/*
 * temphum_user.c
 *
 *  Created on: May 20, 2024
 *      Author: TRAN DOAN MANH
 */

/* All libraries ---------------------------------------------------*/
#include "app/framework/include/af.h"
#include "temphum_user.h"
#include "em_i2c.h"
#include "em_gpio.h"
#include "em_cmu.h"

// Addresses of I2C_SLAVE and TEMPHUM register
#define ADDRESS_I2C0_SLAVE				0x80
#define ADDRESS_I2C0_TEMP				0xE3
#define ADDRESS_I2C0_HUM				0xE5

// An array to store the received data
uint8_t i2c_rxBuffer[2];

// A variable to store the value of TEMPHUM sensor
uint16_t sensor_value = 0;

/*
 * @func I2C_INIT
 * @brief To configure all specifications of I2C protocol
 * @param None
 * @retval None
 */
void I2C_init(void)
{
	// To check if the program has jumped into this function or not
	emberAfCorePrintln("I2C_init !");

	// Enable clocks to the I2C and GPIO
	CMU_ClockEnable(cmuClock_I2C0, true);
	CMU_ClockEnable(cmuClock_GPIO, true);

	/*----------------- Configuring pin 0, 1 of GPIOB that act as SCL, SDA of I2C0 -------------------*/

	// Configuring pin 0, 1 of GPIOB at open drain - PB0 (SCL) and PB1 (SDA)
	GPIO_PinModeSet(gpioPortB, 0, gpioModeWiredAndPullUpFilter, 1);
	GPIO_PinModeSet(gpioPortB, 1, gpioModeWiredAndPullUpFilter, 1);

	// Route I2C pins to GPIO
	GPIO->I2CROUTE[0].SCLROUTE = (GPIO->I2CROUTE[0].SCLROUTE & ~_GPIO_I2C_SCLROUTE_MASK)
	                        	 | (gpioPortB << _GPIO_I2C_SCLROUTE_PORT_SHIFT
	                             | (0 << _GPIO_I2C_SCLROUTE_PIN_SHIFT));
	GPIO->I2CROUTE[0].SDAROUTE = (GPIO->I2CROUTE[0].SDAROUTE & ~_GPIO_I2C_SDAROUTE_MASK)
	                             | (gpioPortB << _GPIO_I2C_SDAROUTE_PORT_SHIFT
	                             | (1 << _GPIO_I2C_SDAROUTE_PIN_SHIFT));
	GPIO->I2CROUTE[0].ROUTEEN = GPIO_I2C_ROUTEEN_SDAPEN | GPIO_I2C_ROUTEEN_SCLPEN;

	/*----------------- Configuring I2C1 -------------------*/

	// Use default settings
	I2C_Init_TypeDef i2cInit = I2C_INIT_DEFAULT;

	// Initialize the I2C
	I2C_Init(I2C0, &i2cInit);
}

/*
 * @func ReadValue_TempHumSensor
 * @brief To read the value of TEMPHUM sensor
 * @param
 *       SlaveI2C_Address - The address of slave I2C
 *       TempHum_Register - The address of register TempHum
 *       *rxBuff - An array of received data
 *       numBytes - The bytes of received data
 * @retval None
 */
uint16_t ReadValue_TempHumSensor(uint16_t SlaveI2C_Address, uint8_t TempHum_Register, uint8_t *rxBuff)
{
	// Transfer structure
	I2C_TransferSeq_TypeDef i2cTransfer;
	I2C_TransferReturn_TypeDef result;

	// Initialize I2C transfer
	i2cTransfer.addr          = SlaveI2C_Address;
	i2cTransfer.flags         = I2C_FLAG_WRITE_READ; 	// must write target address before reading
	i2cTransfer.buf[0].data   = &TempHum_Register;
	i2cTransfer.buf[0].len    = 1;
	i2cTransfer.buf[1].data   = rxBuff;
	i2cTransfer.buf[1].len    = 2;

	result = I2C_TransferInit(I2C0, &i2cTransfer);

	// Send data
	while (result == i2cTransferInProgress)
	{
		result = I2C_Transfer(I2C0);
	}

	// To calculate the value of TEMPHUM sensor
//	emberAfCorePrintln("result = %d", result);
//	emberAfCorePrintln("rxBuff0 = %d, rxBuff0 = %d", rxBuff[0], rxBuff[1]);
	if(result == i2cTransferDone)
	{
		sensor_value = (rxBuff[0] << 8) | rxBuff[1];
	}

	return sensor_value;
}

/*
 * @func TemHumSensor_Get_ProcessTemperature
 * @brief To process the temperature data
 * @param None
 * @retval 0-255
 */
uint8_t TemHumSensor_Get_ProcessTemperature(void)
{
	uint8_t processed_temp;
	uint16_t no_precessed_temp;

	//To get the the temperature value of sensor
	no_precessed_temp = ReadValue_TempHumSensor(ADDRESS_I2C0_SLAVE, ADDRESS_I2C0_TEMP, i2c_rxBuffer);

	//To process that value
	processed_temp = (uint8_t) ((175.72 * no_precessed_temp)/65536 - 46.85);

	return processed_temp;
}

/*
 * @func TemHumSensor_Get_ProcessHumidity
 * @brief To process the humidity data
 * @param None
 * @retval 0-255
 */
uint8_t TemHumSensor_Get_ProcessHumidity(void)
{
	uint8_t processed_hum;
	uint16_t no_precessed_hum;

	//To get the the temperature value of sensor
	no_precessed_hum = ReadValue_TempHumSensor(ADDRESS_I2C0_SLAVE, ADDRESS_I2C0_HUM, i2c_rxBuffer);

	//To process that value
	processed_hum = (uint8_t) ((125*no_precessed_hum)/65536 - 6);

	return processed_hum;
}
