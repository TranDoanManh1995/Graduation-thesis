/*
 * led_user.h
 *
 *  Created on: Jan 4, 2024
 *      Author: TRAN DOAN MANH
 */

#ifndef SOURCE_MID_LED_USER_LED_USER_H_
#define SOURCE_MID_LED_USER_LED_USER_H_

#define LED_FREE				0
#define LED_TOGGLE				1
#define LED_RGB_COUNT			2
#define LED_RGB_ELEMENT			3

// Status of LED
#define LED_ON					1
#define LED_OFF					0

// Pin and Port of LED_1
#define LED_PORT_1				gpioPortA
#define LED_BLUE_PIN_1			(0U)
#define LED_GREEN_PIN_1			(3U)
#define LED_RED_PIN_1			(4U)

// Pin and Port of LED_2
#define LED_PORT_2				gpioPortD
#define LED_BLUE_PIN_2			(2U)
#define LED_GREEN_PIN_2			(1U)
#define LED_RED_PIN_2			(0U)

// Array to initialize and control LED_1, LED_2
#define LED_RGB_1				{{LED_PORT_1,LED_BLUE_PIN_1}, {LED_PORT_1,LED_GREEN_PIN_1}, {LED_PORT_1,LED_RED_PIN_1}}
#define LED_RGB_2				{{LED_PORT_2,LED_BLUE_PIN_2}, {LED_PORT_2,LED_GREEN_PIN_2}, {LED_PORT_2,LED_RED_PIN_2}}

// Define enum type for color of LED
typedef enum
{
	ledOff		= 0x000,
	ledBlue		= BIT(0),				//(1U << 0)
	ledGreen	= BIT(1),				//(1U << 1)
	ledRed		= BIT(2),				//(1U << 2)
	ledPink 	= BIT(0) | BIT(2),		//ledBlue + ledRed
	ledYellow	= BIT(2) | BIT(1)		//ledGreen + ledRed
}ledColor;

// Define enum type for state of LED
typedef enum
{
	red,
	green,
	blue,
	off
}RGB_state;

// Define enum type for position of LED
typedef enum
{
	LED_1,
	LED_2
}ledNumber;

// Define structure type to initialize and control LED
typedef struct
{
	GPIO_Port_TypeDef port;
	unsigned int pin;
}led_t;

// Define structure type for blink LED mode
typedef struct
{
	uint16_t onTime;
	uint16_t offTime;
	uint8_t blinkTime;
}ledBlink_t;

// Define structure type for dim LED mode
typedef struct
{
	boolean dimHigh;
	uint8_t dimLevel;
}ledDim_t;

// Define structure type for action of LED
typedef struct
{
	uint8_t ledBlinkMode;
	ledColor color;

	union
	{
		ledBlink_t ledBlink;
		ledDim_t ledDim;
	}ledAct;

}ledActionInfo_t;

// All functions to initialize and control LED
void ledInit(void);
void turnOffLed(ledNumber index);
void turnOnLed(ledNumber index, ledColor color);
void toggleLed(ledNumber index, ledColor color, uint8_t toggleTime, uint32_t onTimeMs, uint32_t offTimeMs);
void toggleLedHandle(ledNumber index);

#endif /* SOURCE_MID_LED_USER_LED_USER_H_ */
