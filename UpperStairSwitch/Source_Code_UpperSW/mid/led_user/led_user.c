/*
 * led_user.c
 *
 *  Created on: Jan 4, 2024
 *      Author: TRAN DOAN MANH
 */

#include "app/framework/include/af.h"
#include "led_user.h"

// Event to process LED
EmberEventControl	led1ToggleEventControl, led2ToggleEventControl;
void led1ToggleEventHandler(void);
void led2ToggleEventHandler(void);

// Array to initialize and control LED
led_t				ledArray[LED_RGB_COUNT][LED_RGB_ELEMENT] = {LED_RGB_1, LED_RGB_2};
ledActionInfo_t		ledAction[LED_RGB_COUNT];

/*
 * @func ledInit
 * @brief To initialize LED at output_pushpull mode
 * @param None
 * @retval None
 */
void ledInit(void)
{
	// Check if function is performed
	emberAfCorePrintln("ledInit !");

	// Enable clock pulse for GPIO
	CMU_ClockEnable(cmuClock_GPIO, true);

	// initialize all LEDs at output_pushpull mode
	for(int i=0; i<LED_RGB_COUNT; i++)
	{
		for(int j=0; j<LED_RGB_ELEMENT; j++)
		{
			// Configure GPIO mode
			GPIO_PinModeSet(ledArray[i][j].port,
							ledArray[i][j].pin,
							gpioModePushPull,
							0);
		}
	}

	//Off LED after Init
	turnOffLed(LED_1);
	turnOffLed(LED_2);
}

/*
 * @func turnOffLed
 * @brief To turn off LED
 * @param index - index of LED
 * @retval None
 */
void turnOffLed(ledNumber index)
{
	for(int j=0; j< LED_RGB_ELEMENT; j++)
	{
		// Turn off LED
		GPIO_PinOutSet(ledArray[index][j].port, ledArray[index][j].pin);
	}
}

/*
 * @func turnOnLed
 * @brief To turn on LED
 * @param index - index of LED
 * 		  color - color of LED
 * @retval None
 */
void turnOnLed(ledNumber index, ledColor color)
{
	for(int j=0; j<LED_RGB_ELEMENT; j++)
	{
		if(((color >> j) & 0x01) == 1)
		{
			// Turn on LED
			GPIO_PinOutClear(ledArray[index][j].port, ledArray[index][j].pin);
		}
		else
		{
			// Turn off LED
			GPIO_PinOutSet(ledArray[index][j].port, ledArray[index][j].pin);
		}
	}
}

/*
 * @func toggleLed
 * @brief To configure at blink LED
 * @param index - index of LED
 * 		  color - color of LED
 *		  toggleTime - number of LED blinks
 *		  onTimeMS - time at high pulse of a cycle (milisecond)
 *		  offTimeMs - time at low pulse of a cycle (milisecond)
 * @retval None
 */
void toggleLed(ledNumber index, ledColor color, uint8_t toggleTime, uint32_t onTimeMs, uint32_t offTimeMs)
{
	// Configure at blink mode
	ledAction[index].ledBlinkMode = LED_TOGGLE;
	ledAction[index].color = color;
	ledAction[index].ledAct.ledBlink.onTime = onTimeMs;
	ledAction[index].ledAct.ledBlink.offTime = offTimeMs;
	ledAction[index].ledAct.ledBlink.blinkTime = toggleTime * 2;

	// Trigger event to control LED
	if(index == 0)
	{
		emberEventControlSetActive(led1ToggleEventControl);
	}
	else if(index == 1)
	{
		emberEventControlSetActive(led2ToggleEventControl);
	}
}

/*
 * @func led1ToggleEventHandle
 * @brief To blink LED_1
 * @param None
 * @retval None
 */
void led1ToggleEventHandler(void)
{
	// Inactivate LED_1 blink event
	emberEventControlSetInactive(led1ToggleEventControl);

	// Control LED_1 blink
	if(ledAction[LED_1].ledAct.ledBlink.blinkTime != 0)
	{
		if((ledAction[LED_1].ledAct.ledBlink.blinkTime % 2) == 0)
		{
			for(int i=0; i<LED_RGB_ELEMENT; i++)
			{
				if(((ledAction[LED_1].color >> i) & 0x01) == 1)
				{
					GPIO_PinOutClear(ledArray[LED_1][i].port, ledArray[LED_1][i].pin);
				}
				else
				{
					GPIO_PinOutSet(ledArray[LED_1][i].port, ledArray[LED_1][i].pin);
				}
			}

			emberEventControlSetDelayMS(led1ToggleEventControl, ledAction[LED_1].ledAct.ledBlink.onTime);
		}
		else
		{
			for(int j=0; j<LED_RGB_ELEMENT; j++)
			{
				GPIO_PinOutSet(ledArray[LED_1][j].port, ledArray[LED_1][j].pin);
			}

			emberEventControlSetDelayMS(led1ToggleEventControl, ledAction[LED_1].ledAct.ledBlink.offTime);
		}

		ledAction[LED_1].ledAct.ledBlink.blinkTime--;
	}
	else
	{
		ledAction[LED_1].ledBlinkMode = LED_FREE;

		for(int j=0; j<LED_RGB_ELEMENT; j++)
		{
			GPIO_PinOutSet(ledArray[LED_1][j].port, ledArray[LED_1][j].pin);
		}
	}
}

/*
 * @func led2ToggleEventHandle
 * @brief To blink LED_2
 * @param None
 * @retval None
 */
void led2ToggleEventHandler(void)
{
	// Inactivate LED_2 blink event
	emberEventControlSetInactive(led2ToggleEventControl);

	// Control LED_2 blink
	if(ledAction[LED_2].ledAct.ledBlink.blinkTime != 0)
	{
		if((ledAction[LED_2].ledAct.ledBlink.blinkTime % 2) == 0)
		{
			for(int i=0; i<LED_RGB_ELEMENT; i++)
			{
				if(((ledAction[LED_2].color >> i) & 0x01) == 1)
				{
					GPIO_PinOutClear(ledArray[LED_2][i].port, ledArray[LED_2][i].pin);
				}
				else
				{
					GPIO_PinOutSet(ledArray[LED_2][i].port, ledArray[LED_2][i].pin);
				}
			}

			emberEventControlSetDelayMS(led2ToggleEventControl, ledAction[LED_2].ledAct.ledBlink.onTime); //meaning of that function?
		}
		else
		{
			for(int j=0; j<LED_RGB_ELEMENT; j++)
			{
				GPIO_PinOutSet(ledArray[LED_2][j].port, ledArray[LED_2][j].pin);
			}

			emberEventControlSetDelayMS(led2ToggleEventControl, ledAction[LED_2].ledAct.ledBlink.offTime);
		}

		ledAction[LED_2].ledAct.ledBlink.blinkTime--;
	}
	else
	{
		ledAction[LED_2].ledBlinkMode = LED_FREE;

		for(int j=0; j<LED_RGB_ELEMENT; j++)
		{
			GPIO_PinOutSet(ledArray[LED_2][j].port, ledArray[LED_2][j].pin);
		}
	}
}
