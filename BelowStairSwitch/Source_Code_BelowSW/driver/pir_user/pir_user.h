/*
 * pir_user.h
 *
 *  Created on: June 5, 2024
 *      Author: TRAN DOAN MANH
 */

#ifndef SOURCE_MID_PIR_USER_PIR_USER_H_
#define SOURCE_MID_PIR_USER_PIR_USER_H_

EmberEventControl pinDetectEventControl;
void pinDetectEventHandler(void);

// Output GPIO of PIR sensor
#define PIR_PORT				gpioPortC
#define PIR_PIN					(4U)

// State of finding motion
typedef enum{
	PIR_STATE_DEBOUNCE,
	PIR_STATE_WAIT_2S,
	PIR_STATE_WAIT_30S
}STATE_PIR;

// State of PIR
typedef enum{
	PIR_MOTION,
	PIR_UNMOTION
}MOTION_UNMOTION;

// Variable to find motion
STATE_PIR pirState;

// All functions to initialize and use PIR
void PIR_INTSignalHandle(uint8_t pin);
void PIR_Enable(boolean allow_interrupt);
boolean isMotionSignal(void);
typedef void (*motion_process_event)(uint8_t );
void PIR_Init(motion_process_event );

#endif /* SOURCE_MID_PIR_USER_PIR_USER_H_ */
