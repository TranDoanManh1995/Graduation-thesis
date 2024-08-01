/*
 * main.c
 *
 *  Created on: Jun 4, 2024
 *      Author: TRAN DOAN MANH
 */

// All libraries
#include "app/framework/include/af.h"
#include "source/mid/button_user/button_user.h"
#include "source/mid/led_user/led_user.h"
#include "app/framework/plugin/network-creator/network-creator.h"
#include "app/framework/plugin/network-creator-security/network-creator-security.h"
#include "app/util/zigbee-framework/zigbee-device-common.h"
#include "source/app/receive_process/receive_process.h"
#include "stack/include/network-formation.h"
#include "main.h"
#include <string.h>
#include <stdlib.h>

// Array to compare with received data
char createNW[10] = "CREATE";
char openNW[10] = "OPEN";
char stopNW[10] = "STOP";
char allLeaveNW[10] = "ALL";
char upperLeaveNW[10] = "UPPER";
char belowLeaveNW[10] = "BELOW";
char autoMode[10] = "AUTO";
char manualMode[10] = "MANUAL";
char upperSWturnOn[10] = "ONUP";
char upperSWturnOff[10] = "OFFUP";
char belowSWturnOn[10] = "ONDOWN";
char belowSWturnOff[10] = "OFFDOWN";

/** @brief Main Init
 *
 * This function is called from the application's main function. It gives the
 * application a chance to do any initialization required at system startup.
 * Any code that you would normally put into the top of the application's
 * main() routine should be put into this function.
        Note: No callback
 * in the Application Framework is associated with resource cleanup. If you
 * are implementing your application on a Unix host where resource cleanup is
 * a consideration, we expect that you will use the standard Posix system
 * calls, including the use of atexit() and handlers for signals such as
 * SIGTERM, SIGINT, SIGCHLD, SIGPIPE and so on. If you use the signal()
 * function to register your signal handler, please mind the returned value
 * which may be an Application Framework function. If the return value is
 * non-null, please make sure that you call the returned function from your
 * handler to avoid negating the resource cleanup of the Application Framework
 * itself.
 *
 */
void emberAfMainInitCallback(void)
{
	// To check if the program has jumped into this function or not
	emberAfCorePrintln("Main Init Callback !");

	// Initialize LED, Button
	ledInit();
	buttonInit(process_pressnumber);

	// Trigger event to check the Zigbee network
	emberEventControlSetActive(checkNetworkEventControl);

	// Trigger event to read data using USART2
	emberEventControlSetDelayMS(readDataUSART2EventControl, CYCLE_READ_USARTDATA);
}

/*
 * @func checkNetworkEventHandler
 * @brief To check the Zigbee network
 * @param None
 * @retval None
 */
void checkNetworkEventHandler(void)
{
	// To check if the program has jumped into this function or not
	emberAfCorePrintln("checkNetworkEventHandler !");

	// State variable of a network joining node
	EmberNetworkStatus nwkStatusCurrent;

	// Inactivate event
	emberEventControlSetInactive(checkNetworkEventControl);

	// To get current joining state of a node
	nwkStatusCurrent = emberAfNetworkState();

	// To check the Zigbee network and process
	if(nwkStatusCurrent == EMBER_NO_NETWORK)
	{
		emberAfCorePrintln("EMBER_NO_NETWORK !");
		// Display the network status on circuit
		toggleLed(LED_1, ledRed, 3, 300, 300);
	}
}

/** @brief Stack Status
 *
 * This function is called by the application framework from the stack status
 * handler.  This callbacks provides applications an opportunity to be notified
 * of changes to the stack status and take appropriate action.  The return code
 * from this callback is ignored by the framework.  The framework will always
 * process the stack status after the callback returns.
 *
 * @param status   Ver.: always
 */
bool emberAfStackStatusCallback(EmberStatus status)
{
	// To check if the program has jumped into this function or not
	emberAfCorePrintln("emberAfStackStatusCallback !");

	// Check the state of network to process
	if(status == EMBER_NETWORK_UP)
	{
		emberAfCorePrintln("EMBER_JOINED_NETWORK !");
		// Display the network status on circuit
		toggleLed(LED_1, ledBlue, 3, 300, 300);
	}
	else
	{
		emberAfCorePrintln("EMBER_UNJOINED_NETWORK !");
		// Display the network status on circuit
		toggleLed(LED_1, ledRed, 3, 300, 300);
	}

	// This value is ignored by the framework.
	return false;
}

/*
 * @func process_pressnumber
 * @brief To process BUTTON
 * @param index_button - BUTTON_1 or BUTTON_2
 * 		  pressnumber - number of pressing button
 * @retval None
 */
void process_pressnumber(uint8_t index_button, BUTTON_Event_t pressnumber)
{
	// To check if the program has jumped into this function or not
	emberAfCorePrintln("process_pressnumber !");

	// Handles the number of button presses
	switch(pressnumber)
	{
	case press_1:
		if(index_button == BUTTON_1)
		{
			emberAfCorePrintln("Button 1 is pressed once!");
			emberAfCorePrintln("OpenNW !");
			emberAfPluginNetworkCreatorSecurityOpenNetwork();
			toggleLed(LED_2, ledGreen, 1, 300, 300);
		}
		else
		{
			emberAfCorePrintln("Button 2 is pressed once!");
			emberAfCorePrintln("UpperLeaveNW !");
			Leave_device_onNetwork(nodeID_upstair);
			toggleLed(LED_2, ledYellow, 1, 300, 300);
		}
		break;

	case press_2:
		if(index_button == BUTTON_1)
		{
			emberAfCorePrintln("Button 1 is pressed twice!");
			emberAfCorePrintln("StopNW !");
			emberAfPluginNetworkCreatorSecurityCloseNetwork();
			toggleLed(LED_2, ledRed, 1, 300, 300);
		}
		else
		{
			emberAfCorePrintln("Button 2 is pressed twice!");
			emberAfCorePrintln("BelowLeaveNW !");
			Leave_device_onNetwork(nodeID_downstair);
			toggleLed(LED_2, ledYellow, 2, 300, 300);
		}

		break;

	case press_3:
		if(index_button == BUTTON_1)
		{
			emberAfCorePrintln("Button 1 is pressed three times!");
			emberAfCorePrintln("CreateNW !");
			emberAfPluginNetworkCreatorNetworkForm(1, 0xABCD, 10, 15);
		}
		else
		{
			emberAfCorePrintln("Button 2 is three times!");
			emberAfCorePrintln("AllLeaveNW !");
			Leave_device_onNetwork(nodeID_upstair);
			Leave_device_onNetwork(nodeID_downstair);
			toggleLed(LED_2, ledYellow, 3, 300, 300);
		}
		break;

	default:
		break;
	}
}

/*
 * @func readDataUSART2EventHandler
 * @brief To process the "readDataUSART2" event
 * @param None
 * @retval None
 */
void readDataUSART2EventHandler(void)
{
	// Inactivate read data using USART event
	emberEventControlSetInactive(readDataUSART2EventControl);

	// Get data and process received data
	receiveDataLength = emberSerialReadAvailable(comPortUsart2);
	if(receiveDataLength != 0)
	{
		statusReceiveUSART2 = emberSerialReadData(comPortUsart2, (uint8_t *)receiveDataUSART2, receiveDataLength, NULL);
		emberAfCorePrintln("Status Receive USART2: 0x%x !", statusReceiveUSART2);
		// Process received data
		process_receivedData_USART2(receiveDataUSART2, receiveDataLength);
	}

	// To perform this event every 1,5 seconds
	emberEventControlSetDelayMS(readDataUSART2EventControl, CYCLE_READ_USARTDATA);
}

/*
 * @func process_receivedData_USART2
 * @brief To process received data
 * @param receivedData - received data
 * 		  receivedLength the length of received data
 * @retval None
 */
void process_receivedData_USART2(char *receivedData, uint16_t receivedLength)
{
	// To check if the program has jumped into this function or not
	emberAfCorePrintln("process_receivedData_USART2 !");

	// Get received data to remove interference
	static char correctlyReceivedData[10];
	memset(correctlyReceivedData, '\0', sizeof(correctlyReceivedData));
	strncpy(correctlyReceivedData, receivedData, receivedLength);
	emberAfCorePrintln("Data: %s !", correctlyReceivedData);

	// Process the received data
	if(strcmp(correctlyReceivedData, createNW) == 0)
	{
		emberAfCorePrintln("CreateNW !");
		emberAfPluginNetworkCreatorNetworkForm(1, 0xABCD, 10, 15);
	}
	else if(strcmp(correctlyReceivedData, openNW) == 0)
	{
		emberAfCorePrintln("OpenNW !");
		emberAfPluginNetworkCreatorSecurityOpenNetwork();
		toggleLed(LED_2, ledGreen, 1, 300, 300);
	}
	else if(strcmp(correctlyReceivedData, stopNW) == 0)
	{
		emberAfCorePrintln("StopNW !");
		emberAfPluginNetworkCreatorSecurityCloseNetwork();
		toggleLed(LED_2, ledRed, 1, 300, 300);
	}
	else if(strcmp(correctlyReceivedData, allLeaveNW) == 0)
	{
		emberAfCorePrintln("AllLeaveNW !");
		Leave_device_onNetwork(nodeID_upstair);
		Leave_device_onNetwork(nodeID_downstair);
		toggleLed(LED_2, ledYellow, 3, 300, 300);
	}
	else if(strcmp(correctlyReceivedData, upperLeaveNW) == 0)
	{
		emberAfCorePrintln("UpperLeaveNW !");
		Leave_device_onNetwork(nodeID_upstair);
		toggleLed(LED_2, ledYellow, 1, 300, 300);
	}
	else if(strcmp(correctlyReceivedData, belowLeaveNW) == 0)
	{
		emberAfCorePrintln("BelowLeaveNW !");
		Leave_device_onNetwork(nodeID_downstair);
		toggleLed(LED_2, ledYellow, 2, 300, 300);
	}
	else if(strcmp(correctlyReceivedData, autoMode) == 0)
	{
		emberAfCorePrintln("AutoMode !");
		send_controlModeforSW(ZCL_ON_COMMAND_ID, nodeID_downstair);
	}
	else if(strcmp(correctlyReceivedData, manualMode) == 0)
	{
		emberAfCorePrintln("ManualMode !");
		send_controlModeforSW(ZCL_OFF_COMMAND_ID, nodeID_downstair);
	}
	else if(strcmp(correctlyReceivedData, upperSWturnOn) == 0)
	{
		emberAfCorePrintln("UpperSWturnOn !");
		control_stairLight(ZCL_ON_COMMAND_ID, nodeID_upstair);
	}
	else if(strcmp(correctlyReceivedData, upperSWturnOff) == 0)
	{
		emberAfCorePrintln("UpperSWturnOff !");
		control_stairLight(ZCL_OFF_COMMAND_ID, nodeID_upstair);
	}
	else if(strcmp(correctlyReceivedData, belowSWturnOn) == 0)
	{
		emberAfCorePrintln("BelowSWturnOn !");
		control_stairLight(ZCL_ON_COMMAND_ID, nodeID_downstair);
	}
	else if(strcmp(correctlyReceivedData, belowSWturnOff) == 0)
	{
		emberAfCorePrintln("BelowSWturnOff!");
		control_stairLight(ZCL_OFF_COMMAND_ID, nodeID_downstair);
	}
}

/*
 * @func send_controlModeforSW
 * @brief To send the control mode for switch
 * @param commandID - command to select the control mode
 * 		  des_NodeID - nodeID of up/down switch
 * @retval None
 */
void send_controlModeforSW(uint8_t commandID, uint16_t des_NodeID)
{
	// Fill data into buffer
	emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND \
		                           | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER), \
		                           ZCL_ON_OFF_CLUSTER_ID, \
								   commandID, \
		                           "");
	// Set up destination and source endpoint
	emberAfSetCommandEndpoints(1, 5);
	// Use Unicast to send message
	(void) emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, des_NodeID);
}

/*
 * @func control_stairLight
 * @brief To turn on/off the stair light
 * @param commandID - command to turn on/off
 * 		  des_NodeID - nodeID of up/down switch
 * @retval None
 */
void control_stairLight(uint8_t commandID, uint16_t des_NodeID)
{
	// Fill data into buffer
	emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND \
		                           | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER), \
		                           ZCL_ON_OFF_CLUSTER_ID, \
								   commandID, \
		                           "");
	// Set up destination and source endpoint
	emberAfSetCommandEndpoints(1, 1);
	// Use Unicast to send message
	(void) emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, des_NodeID);
}

/*
 * @func Leave_device_onNetwork
 * @brief To remove the device from the network
 * @param des_NodeID - NodeID of device that it must leave network
 * @retval None
 */
void Leave_device_onNetwork(uint16_t des_NodeID)
{
	// 64 bit Address
	EmberEUI64 nullEui64 = { 0, 0, 0, 0, 0, 0, 0, 0 };
	// Variable to check the result of function execution
	EmberStatus status;
	// Variable to select exit mode
	uint8_t options = 0;
	options |= EMBER_ZIGBEE_LEAVE_WITHOUT_REJOIN;

	// Send the "Leave" command
	status = emberLeaveRequest(des_NodeID,
							   nullEui64,
							   options,
							   EMBER_APS_OPTION_RETRY | EMBER_APS_OPTION_ENABLE_ROUTE_DISCOVERY);

	// Define the "Leave" command is success or not
	emberAfAppPrintln("Leave %p0x%X", "Request: ", status);
}

	// Three functions to remove a device from the Zigbee network
//	emberSendRemoveDevice(0xFFFF, 0x00000000, 0x00000000);
//	status = emberSendZigbeeLeave(0xFFFF, EMBER_ZIGBEE_LEAVE_WITHOUT_REJOIN);
//	emberAfCorePrintln("status: 0x%x", status);
//	Leave_device_onNetwork(des_NodeID);
