#ifndef _HANDLE_SERVERCLIENT_H
#define _HANDLE_SERVERCLIENT_H

// Report messenger format code
#define STAIR_LIGHT_CODE						0x01
#define TEMP_VALUE_CODE							0x02
#define HUM_VALUE_CODE							0x03
#define LIGHT_INTENSITY_CODE					0x04
#define NETWORK_UP_CODE							0x05
#define NETWORK_DOWN_CODE						0x06
#define CONTROL_MODE_CODE						0x07

// Functions to start and stop WebServer
void start_webserver(void);
void stop_webserver(void);

// All functions for USART2
void USART2_RX_Events_task(void *pvParameters);
void USART2_init(void);

#endif