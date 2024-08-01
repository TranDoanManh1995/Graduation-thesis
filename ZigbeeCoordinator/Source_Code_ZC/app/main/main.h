/*
 * main.h
 *
 *  Created on: Jun 4, 2024
 *      Author: TRAN DOAN MANH
 */

#ifndef SOURCE_APP_MAIN_MAIN_H_
#define SOURCE_APP_MAIN_MAIN_H_

#include "source/mid/button_user/button_user.h"

#define CYCLE_READ_USARTDATA			100

// Variables for communicating by USART2
EmberStatus statusReceiveUSART2;
char receiveDataUSART2[10];
uint16_t receiveDataLength;

// Event to read data using USART2
EmberEventControl readDataUSART2EventControl;
void readDataUSART2EventHandler(void);

// Event to check whether the Zigbee Network is created
EmberEventControl checkNetworkEventControl;
void checkNetworkEventHandler(void);

// All functions to process
void Leave_device_onNetwork(uint16_t des_NodeID);
void process_pressnumber(uint8_t index_button, BUTTON_Event_t pressnumber);
void process_receivedData_USART2(char *receivedData, uint16_t receivedLength);
void control_stairLight(uint8_t commandID, uint16_t des_NodeID);
void send_controlModeforSW(uint8_t commandID, uint16_t des_NodeID);

#endif /* SOURCE_APP_MAIN_MAIN_H_ */
