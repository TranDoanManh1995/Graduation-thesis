/*
 * iadc_user.c
 *
 *  Created on: Dec 6, 2023
 *      Author: TRAN DOAN MANH
 */

/* All libraries ---------------------------------------------------*/
#include "app/framework/include/af.h"
#include <math.h>
#include "iadc_user.h"
#include "em_iadc.h"
#include "em_cmu.h"

// Light value
static volatile uint32_t lightIntensity_value = 0;

/*
 * @func I2C_INIT
 * @brief To configure all specifications of I2C protocol
 * @param None
 * @retval None
 */
void IADC__init(void)
{
	// To check if the program has jumped into this function or not
	emberAfCorePrintln("IADC__init !");

	// Enable clocks to the IADC and GPIO
	CMU_ClockEnable(cmuClock_IADC0, true);
	CMU_ClockEnable(cmuClock_GPIO, true);

	/*----------------- Configuring I2C1 -------------------*/

	// Reset IADC to reset configuration in case it has been modified by other code
	IADC_reset(IADC0);

	// Declare initialization structures of IDAC
	IADC_Init_t 		init = IADC_INIT_DEFAULT;
	IADC_AllConfigs_t 	initAllConfigs = IADC_ALLCONFIGS_DEFAULT;
	IADC_InitSingle_t 	initSingle = IADC_INITSINGLE_DEFAULT;
	IADC_SingleInput_t 	initSingleInput = IADC_SINGLEINPUT_DEFAULT;

	//---------- Modify init configuration ----------
	// Select warmup mode = KeepWarm
	init.warmup = iadcWarmupKeepWarm;

	//---------- Modify initAllConfigs configuration ---------
	// Select voltage source to reference = Analog Power Supply
	initAllConfigs.configs[0].reference = iadcCfgReferenceVddx;
	initAllConfigs.configs[0].vRef = 3000;
	// Select oversampling ratio = 32x
	initAllConfigs.configs[0].osrHighSpeed = iadcCfgOsrHighSpeed32x;

	//---------- Modify initSingle configuration ----------
	//Select buffer FIFO = one-word
	initSingle.dataValidLevel = _IADC_SINGLEFIFOCFG_DVL_VALID1;

	//---------- Modify clock division configuration ----------
	// Select clock for IADC, FSRCO - 20MHz
	CMU_ClockSelectSet(cmuClock_IADCCLK, cmuSelect_FSRCO);
	// Set the HFSCLK prescale value
	init.srcClkPrescale = IADC_calcSrcClkPrescale(IADC0, CLK_SRC_ADC_FREQ, 0);
	// Divides CLK_SRC_ADC to set the CLK_ADC frequency
	initAllConfigs.configs[0].adcClkPrescale = IADC_calcAdcClkPrescale(IADC0,
	                                             	 	 	 	 	   CLK_ADC_FREQ,
																	   0,
																	   iadcCfgModeNormal,
																	   init.srcClkPrescale);

	//---------- To use GPIO as an input for the IADC ----------
	// Allocate the analog bus for ADC0 inputs
	GPIO->CDBUSALLOC |= GPIO_CDBUSALLOC_CDODD0_ADC0;
	// Assign ports and pins to positive and negative inputs in signed-end mode
	initSingleInput.posInput   = iadcPosInputPortCPin5;
	initSingleInput.negInput   = iadcNegInputGnd;

	//---------- Initialize IADC ----------
	// Initialize the IADC
	IADC_init(IADC0, &init, &initAllConfigs);
	// Initialize the Single conversion inputs
	IADC_initSingle(IADC0, &initSingle, &initSingleInput);
}

/*
 * @func LightSensor_AdcPollingRead
 * @brief To read the value of light sensor
 * @param None
 * @retval Light value
 */
uint32_t LightSensor_AdcPollingRead(void)
{
    // Initialize conversion variable
	volatile uint32_t voltage_value = 0;
	volatile uint32_t resistance_value = 0;

	// Start IADC conversion
    IADC_command(IADC0, iadcCmdStartSingle);

    // Wait for conversion to be complete
    while((IADC0->STATUS & (_IADC_STATUS_CONVERTING_MASK | _IADC_STATUS_SINGLEFIFODV_MASK))
    	     != IADC_STATUS_SINGLEFIFODV);

    // Get ADC result
    voltage_value = IADC_pullSingleFifoResult(IADC0).data;
    resistance_value = 10000 * (3300 - voltage_value) / voltage_value;
    lightIntensity_value = (uint32_t)(336 * pow(10, 5) * pow(resistance_value, -1.4));

    return lightIntensity_value;
}
