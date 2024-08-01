/*
 * temphum_user.h
 *
 *  Created on: May 20, 2024
 *      Author: TRAN DOAN MANH
 */

#ifndef SOURCE_MID_TEMPHUM_USER_TEMPHUM_USER_H_
#define SOURCE_MID_TEMPHUM_USER_TEMPHUM_USER_H_

//Functions to initialize IADC and read the value of TEMPHUM sensor
void I2C_init(void);
uint16_t ReadValue_TempHumSensor(uint16_t SlaveI2C_Address, uint8_t TempHum_Register, uint8_t *rxBuff);
uint8_t TemHumSensor_Get_ProcessTemperature(void);
uint8_t TemHumSensor_Get_ProcessHumidity(void);

#endif /* SOURCE_MID_TEMPHUM_USER_TEMPHUM_USER_H_ */
