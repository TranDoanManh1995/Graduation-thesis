/*
 * button_user.h
 *
 *  Created on: Jan 2, 2024
 *      Author: TRAN DOAN MANH
 */

#ifndef SOURCE_MID_BUTTON_USER_BUTTON_USER_H_
#define SOURCE_MID_BUTTON_USER_BUTTON_USER_H_

#include "stdbool.h"

#define BUTTON_COUNT							2
#define GPIO_DOUT								1
#define BUTTON_DEBOUNCE							5
#define BUTTON_CHECK_RELEASE_MS					500
#define BUTTON_PRESS							0
#define BUTTON_1								0
#define BUTTON_2								1
#define BUTTON_CHECK_RELEASE_MS					500

// Pin and Port of BUTTON
#define BUTTON_1_PORT							gpioPortD
#define BUTTON_1_PIN							(4U)
#define BUTTON_2_PORT							gpioPortD
#define BUTTON_2_PIN							(3U)

// Array to initialize and process BUTTON
#define BUTTON_INIT								{{BUTTON_1_PORT, BUTTON_1_PIN}, {BUTTON_2_PORT, BUTTON_2_PIN}}

// Define enum type for number of pressing BUTTON
typedef enum{
	press_1	= 1,
	press_2,
	press_3,
	press_4,
	press_5,
	press_6,
	press_7
}BUTTON_Event_t;

// Define structure type to initialize and process BUTTON
typedef struct {
  GPIO_Port_TypeDef		port;
  unsigned int			pin;
  bool					state;
  uint8_t				pressCount;
  bool 					press;
}BUTTONx_t;

// All functions to initialize and process BUTTON
typedef void (*BUTTON_pressEvent_t)(uint8_t index, BUTTON_Event_t pressEvent);
void buttonInit(BUTTON_pressEvent_t);
void testInit(void);
void halInternalTesrIrs(uint8_t pin);

#endif /* SOURCE_MID_BUTTON_USER_BUTTON_USER_H_ */
