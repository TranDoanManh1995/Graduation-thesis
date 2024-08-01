/*
 * button_user.c
 *
 *  Created on: Jan 2, 2024
 *      Author: TRAN DOAN MANH
 */

// All libraries
#include "app/framework/include/af.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "gpiointerrupt.h"
#include "button_user.h"

// Event to process BUTTON releasing
EmberEventControl buttonReleaseEventControl;
void buttonReleaseEventHandle(void);

// Array and functions to initialize and process BUTTON
BUTTONx_t buttonArray[BUTTON_COUNT] = BUTTON_INIT;
BUTTON_pressEvent_t pressCallbackFunc = NULL;
static void halInternalButtonIsr(uint8_t pin);
static uint8_t getButtonIndex(uint8_t pin);

// Variable to define BUTTON_1 or BUTTON_2
uint8_t buttonIndex;

/*
 * @func buttonInit
 * @brief To initialize BUTTON at inputPullupFilter and interrupt mode
 * @param pressHandle - function pointer to process BUTTON immediately after happening interrupt
 * @retval None
 */
void buttonInit(BUTTON_pressEvent_t pressHandle)
{
	emberAfCorePrintln("Button Init !");

	// Assign address of pressHandle for function pointer "pressCallbackFunc"
	if(pressHandle != NULL)
	{
		pressCallbackFunc = pressHandle;
	}

	//To turn on the clock pulse on the GPIO port
	CMU_ClockEnable(cmuClock_GPIO, true);

	GPIOINT_Init();

	//To configure GPIO pin in input mode
	for (uint8_t i = 0; i < BUTTON_COUNT; i++)
	{
		// Configure GPIO mode
		GPIO_PinModeSet(buttonArray[i].port,
						buttonArray[i].pin,
						gpioModeInputPullFilter,
						GPIO_DOUT);

		//Register callback before setting up and enabling pin interrupt
		GPIOINT_CallbackRegister(buttonArray[i].pin,
								 halInternalButtonIsr);

		//Set rising and falling edge interrupts
		GPIO_ExtIntConfig(buttonArray[i].port,
						  buttonArray[i].pin,
						  buttonArray[i].pin,
						  true,
						  true,
						  true);
	 }
}

/*
 * @func halInternalButtonIsr
 * @brief To process BUTTON immediately after happening interrupt
 * @param pin - pin of GPIO happening interrupt
 * @retval None
 */
void halInternalButtonIsr(uint8_t pin)
{
	emberAfCorePrintln("Interrupt!!!");

	// Variables to process BUTTON
	uint8_t buttonStateNow;
	uint8_t buttonStatePrev;
	uint32_t debounce;

	//To define BUTTON_1 or BUTTON_2
	buttonIndex = getButtonIndex(pin);

	//To check if BUTTON is both BUTTON1 and BUTTON2
	if (buttonIndex == -1)	return;

	//To wait for stabilizing button and get state of pin
	buttonStateNow = GPIO_PinInGet(buttonArray[buttonIndex].port, buttonArray[buttonIndex].pin);
	for ( debounce = 0;
		  debounce < BUTTON_DEBOUNCE;
		  debounce = (buttonStateNow == buttonStatePrev) ? debounce + 1 : 0 )
	{
		buttonStatePrev = buttonStateNow;
		buttonStateNow = GPIO_PinInGet(buttonArray[buttonIndex].port, buttonArray[buttonIndex].pin);
	}
	buttonArray[buttonIndex].state = buttonStateNow;
	//emberAfCorePrintln("The state of buttton %d: %d!", buttonIndex, buttonStateNow);

	//To process the state of button
	if (buttonStateNow == BUTTON_PRESS)
	{
		// Calculate the number of pressing BUTTON
		buttonArray[buttonIndex].pressCount++;
		emberAfCorePrintln("presscoutnt: %d", buttonArray[buttonIndex].pressCount);
	}
	else
	{
		// Trigger event to process BUTTON releasing
		emberEventControlSetDelayMS(buttonReleaseEventControl, BUTTON_CHECK_RELEASE_MS);
	}
}

/*
 * @func buttonReleaseEventHandle
 * @brief To process BUTTON releasing event
 * @param None
 * @retval None
 */
void buttonReleaseEventHandle(void)
{
	// Inactivate BUTTON releasing event
	emberEventControlSetInactive(buttonReleaseEventControl);

	//emberAfCorePrintln("The pressCount of buttton %d: %d!", buttonIndex, buttonArray[buttonIndex].pressCount);
	pressCallbackFunc(buttonIndex, buttonArray[buttonIndex].pressCount);
	buttonArray[buttonIndex].pressCount = 0;

//	for (uint8_t i = 0; i < BUTTON_COUNT; i++)
//	{
//		if (buttonArray[i].pressCount != 0)
//		{
//			pressCallbackFunc(i, buttonArray[i].pressCount);
//			buttonArray[i].pressCount = 0;
//		}
//	}
}

/*
 * @func getButtonIndex
 * @brief To get index of BUTTON
 * @param pin - pin of GPIO happening interrupt
 * @retval value of index
 */
static uint8_t getButtonIndex(uint8_t pin)
{
	for(uint8_t i = 0; i < BUTTON_COUNT; i++)
	{
		if (buttonArray[i].pin == pin)
			return i;
	}
	return -1;
}
