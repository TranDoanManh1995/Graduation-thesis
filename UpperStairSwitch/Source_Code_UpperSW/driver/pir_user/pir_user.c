/*
 * pir_user.c
 *
 *  Created on: June 5, 2024
 *      Author: TRAN DOAN MANH
 */

// All libraries
#include "app/framework/include/af.h"
#include "source/app/main/main.h"
#include "gpiointerrupt.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "pir_user.h"

// Register the function pointer to process when happening interrupt
motion_process_event process_motionCallbackFunc = NULL;

/*
 * @func PIR_Enable
 * @brief To enable or disable interrupt
 * @param true or false
 * @retval None
 */
void PIR_Enable(boolean allow_interrupt)
{
	// Check if this function is performed
//	emberAfCorePrintln("PIR_Enable !");

	//Allow interrupt or not
	if(allow_interrupt == 1)
	{
		GPIO_ExtIntConfig(PIR_PORT, PIR_PIN, PIR_PIN, true, false, true);
	}
	else if (allow_interrupt == 0)
	{
		GPIO_ExtIntConfig(PIR_PORT, PIR_PIN, PIR_PIN, false, false, false);
	}
}

/*
 * @func PIR_Init
 * @brief To initialize PIR
 * @param None
 * @retval None
 */
void PIR_Init(motion_process_event process_motion)
{
	// Check if this function is performed
	emberAfCorePrintln("PIR_Init !");

	// Register a function to process motion
	if(process_motion != NULL)
	{
		process_motionCallbackFunc = process_motion;
	}

	// Initialize GPIO
	GPIOINT_Init();

	// To turn on the clock pulse on the GPIO port
	CMU_ClockEnable(cmuClock_GPIO, true);

	// To Configure at input mode
	GPIO_PinModeSet(PIR_PORT,
					PIR_PIN,
					gpioModeInputPullFilter,
					1);

	// Register callback before setting up and enabling pin interrupt
	GPIOINT_CallbackRegister(PIR_PIN,
							 PIR_INTSignalHandle);
}

/*
 * @func PIR_INTSignalHandle
 * @brief To process after happening interrupt
 * @param pin - GPIO of PIR_OUT
 * @retval None
 */
void PIR_INTSignalHandle(uint8_t pin)
{
	// Check if this function is performed
//	emberAfCorePrintln("PIR_INTSignalHandle !");

	// Check pin
	if(pin != PIR_PIN) return;

	// Check motion to process
	if(isMotionSignal())
	{
		pirState = PIR_STATE_DEBOUNCE;
		PIR_Enable(false);

		// Delay 200ms to avoid bounce
		emberEventControlSetInactive(pinDetectEventControl);
		emberEventControlSetDelayMS(pinDetectEventControl, 200);
	}
}

/*
 * @func pinDetectEventHandle
 * @brief To process motion event
 * @param None
 * @retval None
 */
void pinDetectEventHandler(void)
{
	// Check if this function is performed
//	emberAfCorePrintln("pinDetectEventHandler !");

	// Inactivate detecting motion event
	emberEventControlSetInactive(pinDetectEventControl);

	// Process all status of motion
	switch(pirState)
	{
	case PIR_STATE_DEBOUNCE:
		if(isMotionSignal())
		{
			emberAfCorePrintln("PIR_DETECT_MOTION !");
			pirState = PIR_STATE_WAIT_2S;
			if(process_motionCallbackFunc != NULL)
			{
				process_motionCallbackFunc(PIR_MOTION);
			}
			emberEventControlSetDelayMS(pinDetectEventControl, 2000);
		}
		else
		{
			PIR_Enable(true);
		}
		break;

	case PIR_STATE_WAIT_2S:
		emberAfCorePrintln("PIR_STATE_WAIT_2S !");
		pirState = PIR_STATE_WAIT_30S;
		// AUTO or MANUAL
		if(control_mode == MANUAL)
		{
			PIR_Enable(true);
		}
		emberEventControlSetDelayMS(pinDetectEventControl, 30000);
		break;

	case PIR_STATE_WAIT_30S:
		if(process_motionCallbackFunc != NULL)
		{
			emberAfCorePrintln("PIR_DETECT_UNMOTION !");
			process_motionCallbackFunc(PIR_UNMOTION);
		}
		break;

	default:
		break;
	}
}

/*
 * @func isMotionSignal
 * @brief To check value of PIROUT
 * @param None
 * @retval boolean
*/
boolean isMotionSignal(void)
{
	// state variable
	static boolean state_PIROUT;

	// Get the state of PIROUT
	state_PIROUT = GPIO_PinInGet(PIR_PORT, PIR_PIN);

	return state_PIROUT;
}
