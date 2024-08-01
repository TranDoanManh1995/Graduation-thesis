/*
 * iadc_user.h
 *
 *  Created on: Dec 6, 2023
 *      Author: TRAN DOAN MANH
 */

#ifndef SOURCE_MID_IADC_USER_IADC_USER_H_
#define SOURCE_MID_IADC_USER_IADC_USER_H_

// Set CLK_ADC to 10 MHz
#define CLK_SRC_ADC_FREQ        20000000  	// CLK_SRC_ADC
#define CLK_ADC_FREQ            10000000  	// CLK_ADC - 10 MHz max in normal mode

//Functions to initialize IADC and read the value of light sensor
void IADC__init(void);
uint32_t LightSensor_AdcPollingRead(void);

#endif /* SOURCE_MID_IADC_USER_IADC_USER_H_ */
