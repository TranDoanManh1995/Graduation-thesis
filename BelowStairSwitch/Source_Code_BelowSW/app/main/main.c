/*
 * main.c
 *
 *  Created on: May 20, 2024
 *      Author: TRAN DOAN MANH
 */

// All libraries
#include "app/framework/include/af.h"
#include "source/mid/button_user/button_user.h"
#include "source/mid/led_user/led_user.h"
#include "source/app/network/network.h"
#include "source/driver/iadc_user/iadc_user.h"
#include "source/driver/temphum_user/temphum_user.h"
#include "zigbee-framework/zigbee-device-common.h"
#include "app/framework/plugin/find-and-bind-target/find-and-bind-target.h"
#include "app/framework/plugin/find-and-bind-initiator/find-and-bind-initiator.h"
#include "source/mid/kalman/kalman.h"
#include "main.h"
#include "source/app/receive_process/receive_process.h"
#include "source/driver/pir_user/pir_user.h"
#include <stdlib.h>

boolean networkReady = false;

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

	// Initialize LED, Button, Network, IADC, I2C and PIR
	ledInit();
	buttonInit(process_pressnumber);
	networkInit(process_NetworkEvent);
	IADC__init();
	I2C_init();
	PIR_Init(process_motion);

	//Initialize Kalman filter
	KalmanFilterInit(90, 90, 0.01);

	// Trigger event to process the state of system
	systemState = POWER_ON_STATE;
	emberEventControlSetActive(mainStateEventControl);
}

/*
 * @func mainStateEventHandler
 * @brief To process all states of system
 * @param None
 * @retval None
 */
void mainStateEventHandler(void)
{
	// To check if the program has jumped into this function or not
	emberAfCorePrintln("MainStateEventHandler !");

	// State variable of a network joining node
	EmberNetworkStatus nwkStatusCurrent;

	// Inactivate event
	emberEventControlSetInactive(mainStateEventControl);

	// Process all states of system
	switch(systemState)
	{
	case POWER_ON_STATE:
		// To get current joining state of a node
		nwkStatusCurrent = emberAfNetworkState();
		// Process all states of node
		if(nwkStatusCurrent == EMBER_JOINED_NETWORK)
		{
			emberAfCorePrintln("EMBER_JOINED_NETWORK !");
			// Send ModelID for HC
			SEND_ReportInfoToZC(1);
			// Display the network status on circuit
			toggleLed(LED_2, ledPink, 3, 300, 300);
		}
		else if(nwkStatusCurrent == EMBER_NO_NETWORK)
		{
			emberAfCorePrintln("EMBER_NO_NETWORK !");
			// Display the network status on circuit
			toggleLed(LED_2, ledRed, 3, 300, 300);
			// Find and join a network
			NETWORK_FindAndJoin();
		}
		systemState = IDLE_STATE;
		break;

	case REPORT_STATE:
		emberAfCorePrintln("REPORT_STATE !");
		systemState = IDLE_STATE;
		// Send ModelID for HC
		SEND_ReportInfoToZC(1);
		// Get nodeId itself
		itself_nodeId = emberAfGetNodeId();
		// Trigger light sensor reading event
		emberEventControlSetActive(readLightSensorEventControl);
		emberEventControlSetActive(readTempHumSensorEventControl);
		// Binding
		SW_binding = false;
		PIR_binding = false;
		// Disable interrupt for PIR
		PIR_Enable(false);
		// Control mode
		control_mode = AUTO;
		select_controlMode(5, MANUAL, ledRed, false);
		break;

	case IDLE_STATE:
		systemState = IDLE_STATE;
		break;

	case REBOOT_STATE:
		systemState = IDLE_STATE;
		uint8_t contents[ZDO_MESSAGE_OVERHEAD + 1];
		contents[1] = 0x00;
		// Send ZDO Leave Response for HC
		(void) emberSendZigDevRequest(0x0000, LEAVE_RESPONSE,
										EMBER_AF_DEFAULT_APS_OPTIONS, contents, sizeof(contents));
		// Clear binding table, leave network and reboot device
		emberClearBindingTable();
		nwkStatusCurrent = emberAfNetworkState();
		if(nwkStatusCurrent == EMBER_JOINED_NETWORK)
		{
			emberLeaveNetwork();
		}
		// Reboot after 5000ms
		emberEventControlSetDelayMS(halRebootEventControl, 5000);
		break;

	default:
		break;
	}
}

/*
 * @func process_NetworkEvent
 * @brief To process all states of network
 * @param networkResult - the state of network node
 * @retval None
 */
void process_NetworkEvent(network_state networkResult)
{
	// To check if the program has jumped into this function or not
	emberAfCorePrintln("process_NetworkEvent !");

	// Process the state of network node
	switch(networkResult)
	{
	case NETWORK_HAS_PARENT:
		emberAfCorePrintln("NETWORK_HAS_PARENT !");
		toggleLed(LED_2, ledPink, 5, 300, 300);
		networkReady = true;
		systemState = REPORT_STATE;
		emberEventControlSetDelayMS(mainStateEventControl, 1000);
		break;

	case NETWORK_JOIN_FALL:
		emberAfCorePrintln("NETWORK_JOIN_FALL !");
		systemState = IDLE_STATE;
		emberEventControlSetDelayMS(mainStateEventControl, 1000);
		break;

	case NETWORK_JOIN_SUCCESS:
		emberAfCorePrintln("NETWORK_JOIN_SUCCESS !");
		toggleLed(LED_2, ledBlue, 3, 300, 300);
		networkReady = true;
		systemState = REPORT_STATE;
		emberEventControlSetDelayMS(mainStateEventControl, 1000);
		break;

	case NETWORK_LOST_PARENT:
		emberAfCorePrintln("NETWORK_LOST_PARENT !");
		toggleLed(LED_2, ledYellow, 3, 300, 300);
		systemState = IDLE_STATE;
		emberEventControlSetDelayMS(mainStateEventControl, 1000);
		break;

	case NETWORK_OUT_NETWORK:
		if(networkReady)
		{
			emberAfCorePrintln("NETWORK_OUT_NETWORK !");
			toggleLed(LED_2, ledYellow, 5, 300, 300);
			systemState = REBOOT_STATE;
			emberEventControlSetDelayMS(mainStateEventControl, 3000);
		}
		break;

	default:
		break;
	}
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
		if(index_button == BUTTON_2)
		{
			emberAfCorePrintln("Button 2 is pressed once!");
			state_stairLight = GPIO_PinOutGet(LED_PORT_2, LED_BLUE_PIN_2);
//			emberAfCorePrintln("The state of stair light: %d", state_stairLight);
			if(state_stairLight == 1)
			{
				// Turn on the stair light
				control_stairLight(LED_ON, 1, itself_nodeId, ZCL_ON_COMMAND_ID);
			}
			else if (state_stairLight == 0)
			{
				// Turn off the stair light
				control_stairLight(LED_OFF, 1, itself_nodeId, ZCL_OFF_COMMAND_ID);
			}
		}
		break;

	case press_2:
		if(index_button == BUTTON_2)
		{
			// AUTO or MANUAL
			if(control_mode == MANUAL)
			{
				// To select the MANUAL mode
				select_controlMode(5, MANUAL, ledRed, false);
				// Report the control mode to the remaining switch
				control_onoffStairLight_usingBinding(itself_nodeId, 5, 2, ZCL_ON_OFF_CLUSTER_ID, ZCL_OFF_COMMAND_ID);
			}
			else if(control_mode == AUTO)
			{
				// To select the AUTO mode
				select_controlMode(5, AUTO, ledBlue, true);
				// Report the control mode to the remaining switch
				control_onoffStairLight_usingBinding(itself_nodeId, 5, 2, ZCL_ON_OFF_CLUSTER_ID, ZCL_ON_COMMAND_ID);
			}
		}
		break;

	case press_3:
		if(index_button == BUTTON_2)
		{
			emberAfCorePrintln("Button 2 is pressed three times!");
			if(SW_binding == false)
			{
				SW_binding = true;
				// Activated target
				status_binding = emberAfPluginFindAndBindTargetStart(1);
				// Check opening target
				if(status_binding == EMBER_ZCL_STATUS_SUCCESS)
				{
					toggleLed(LED_2, ledGreen, 1, 300, 300);
				}
			}
			else if(SW_binding == true)
			{
				SW_binding = false;
				// Activated initiator
				status_binding = emberAfPluginFindAndBindInitiatorStart(1);
				// Check opening initiator
				if(status_binding == EMBER_ZCL_STATUS_SUCCESS)
				{
					toggleLed(LED_2, ledGreen, 2, 300, 300);
				}
			}
		}
		break;

	case press_4:
		if(index_button == BUTTON_2)
		{
			emberAfCorePrintln("Button 2 is pressed twice!");
			if(PIR_binding == false)
			{
				PIR_binding = true;
				// Activated target
				status_binding = emberAfPluginFindAndBindTargetStart(5);
				// Check opening target
				if(status_binding == EMBER_ZCL_STATUS_SUCCESS)
				{
					toggleLed(LED_2, ledBlue, 1, 300, 300);
				}
			}
			else if(PIR_binding == true)
			{
				PIR_binding = false;
				// Activated initiator
				status_binding = emberAfPluginFindAndBindInitiatorStart(5);
				// Check opening initiator
				if(status_binding == EMBER_ZCL_STATUS_SUCCESS)
				{
					toggleLed(LED_2, ledBlue, 2, 300, 300);
				}
			}
		}
		break;

	case press_7:
		if(index_button == BUTTON_2)
		{
			emberAfCorePrintln("Button 2 is pressed seven times!");
			toggleLed(LED_2, ledYellow, 5, 300, 300);
			systemState = REBOOT_STATE;
			emberEventControlSetDelayMS(mainStateEventControl, 3000);
		}
		break;

	default:
		break;
	}
}

/*
 * @func SEND_FillBufferGlobalCommand
 * @brief To fill command information into buffer
 * @param clusterID - ID of cluster
 * 		  attributeID - ID of attribute
 * 		  globalCommand - ID of global command
 * 		  *value - ModelID
 * 		  length - length of ModelID
 * 		  dataType - sending data type of ModelID
 * @retval None
 */
void SEND_FillBufferGlobalCommand(EmberAfClusterId clusterID,
		 	 	 	 	 	 	  EmberAfAttributeId attributeID,
								  uint8_t globalCommand,
								  uint8_t *value,
								  uint8_t length,
								  uint8_t dataType)
{
	// To check if the program has jumped into this function or not
//	emberAfCorePrintln("SEND_FillBufferGlobalCommand");

	// Dimension of sent data
	uint8_t data[MAX_DATA_COMMAND_SIZE];

	// Set up data frame to send for HC
	data[0] = (uint8_t)(attributeID & 0x00FF);
	data[1] = (uint8_t)((attributeID & 0xFF00) >> 8);
	data[2] = EMBER_SUCCESS;
	data[3] = (uint8_t)dataType;
	memcpy(&data[4], value, length);

	// Fill command frame into buffer
	(void) emberAfFillExternalBuffer((ZCL_GLOBAL_COMMAND | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT | ZCL_DISABLE_DEFAULT_RESPONSE_MASK),
									  clusterID,
									  globalCommand,
									  "b",
									  data,
									  length + 4);
}

/*
 * @func SEND_SendCommandUnicast
 * @brief To send command
 * @param src_ep - sending endpoint
 * 		  destination - receiving endpoint
 * 		  address - network address of received node
 * @retval None
 */
void SEND_SendCommandUnicast(uint8_t src_ep,
							 uint8_t destination,
							 uint8_t address)
{
	// To check if the program has jumped into this function or not
//	emberAfCorePrintln("SEND_SendCommandUnicast");

	// Set up source and destination endpoint
	emberAfSetCommandEndpoints(src_ep, destination);

	// Send command in Unicast
	(void) emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, address);
}

/*
 * @func SEND_ReportInfoToZC
 * @brief To report ModelID for ZC
 * @param src_ep - sending endpoint
 * @retval None
 */
void SEND_ReportInfoToZC(uint8_t src_ep)
{
	emberAfCorePrintln("SEND_ReportInfoToZC");

	// ModelID to send for HC
	uint8_t modelID[5] = {4, 'D', 'O', 'W', 'N'};

	// Check whether node joins network
	if(emberAfNetworkState() != EMBER_JOINED_NETWORK)
	{
		return;
	}

	// To fill command information into buffer
	SEND_FillBufferGlobalCommand(ZCL_BASIC_CLUSTER_ID,
								 ZCL_MODEL_IDENTIFIER_ATTRIBUTE_ID,
								 ZCL_READ_ATTRIBUTES_RESPONSE_COMMAND_ID,
								 modelID,
								 5,
								 ZCL_CHAR_STRING_ATTRIBUTE_TYPE);

	// Send command
	SEND_SendCommandUnicast(src_ep,
							DESTINATION_ENDPOINT,
							ZC_NETWORK_ADDRESS);
}

/*
 * @func select_controlMode
 * @brief To select the "AUTO" control mode
 * @param src_ep - source endpoint to send he report messenger for ZC
 * 		  controlMode - To select the control mode
 * 		  modeLED - LED to display the control mode
 * 		  PIRinterrupt - active/inactive the interrupt on PIR sensor
 * @retval None
 */
void select_controlMode(uint8_t src_ep, uint8_t controlMode, ledColor modeLED, boolean PIRinterrupt)
{
	// Display the control mode currently
	if(controlMode == AUTO)
	{
		emberAfCorePrintln("AUTO mode !");
		control_mode = MANUAL;
	}
	else if(controlMode == MANUAL)
	{
		emberAfCorePrintln("MANUAL mode !");
		control_mode = AUTO;
	}
	// Report the control mode for ZC
	SEND_ControlModeReport(src_ep, controlMode);
	// Signal light
	turnOffLed(LED_1);
	turnOnLed(LED_1, modeLED);
	// Enable/Disable PIR
	PIR_Enable(PIRinterrupt);
}

/*
 * @func control_stairLight
 * @brief To turn on/off the stair light
 * @param src_ep - source endpoint to send he report messenger for ZC
 * 		  LEDstatus - the status of LED
 * 		  nodeID_itself - the node ID of switch itself
 * 		  commandID - command to control the stair light
 * @retval None
 */
void control_stairLight(uint8_t LEDstatus, uint8_t src_ep, uint16_t nodeID_itself, uint8_t commandID)
{
	// Turn on/off the stair light
	if(LEDstatus == LED_ON)
	{
		emberAfCorePrintln("The stair light is turning on!");
		turnOnLed(LED_2, ledBlue);
	}
	else if(LEDstatus == LED_OFF)
	{
		emberAfCorePrintln("The stair light is turning off!");
		turnOffLed(LED_2);
	}

	// Report the state of LED for ZC
	SEND_OnOffStateReport(src_ep, LEDstatus);
	// Report the state of LED for the remaining switch
	control_onoffStairLight_usingBinding(nodeID_itself, src_ep, 1, ZCL_ON_OFF_CLUSTER_ID, commandID);
}

/*
 * @func SEND_OnOffStateReport
 * @brief To report the state of LED for HC
 * @param src_ep - sending endpoint
 * 		  value - state ON/OFF of LED
 * @retval None
 */
void SEND_OnOffStateReport(uint8_t src_ep, uint8_t value)
{
	// To check if the program has jumped into this function or not
//	emberAfCorePrintln("Report the network state of below stair switch!");

	// To fill command information into buffer
	SEND_FillBufferGlobalCommand(ZCL_ON_OFF_CLUSTER_ID,
								 ZCL_ON_OFF_ATTRIBUTE_ID,
								 ZCL_READ_ATTRIBUTES_RESPONSE_COMMAND_ID,
								 (uint8_t *) &value,
								 1,
								 ZCL_BOOLEAN_ATTRIBUTE_TYPE);

	// To send command
	SEND_SendCommandUnicast(src_ep,
							DESTINATION_ENDPOINT,
							ZC_NETWORK_ADDRESS);
}

/*
 * @func SEND_ControlModeReport
 * @brief To send the control mode into ZC
 * @param src_ep - source endpoint
 * 		  value - control mode
 * @retval None
 */
void SEND_ControlModeReport(uint8_t src_ep, uint8_t value)
{
	// To check if the program has jumped into this function or not
//	emberAfCorePrintln("SEND_LightValueReport !");

	// To fill command information into buffer
	SEND_FillBufferGlobalCommand(ZCL_ON_OFF_CLUSTER_ID,
								 ZCL_ON_OFF_ATTRIBUTE_ID,
								 ZCL_READ_ATTRIBUTES_RESPONSE_COMMAND_ID,
								 (uint8_t *) &value,
								 1,
								 ZCL_BOOLEAN_ATTRIBUTE_TYPE);

	// To send command
	SEND_SendCommandUnicast(src_ep,
							DESTINATION_ENDPOINT,
							ZC_NETWORK_ADDRESS);
}

/*
 * @func readLightSensorEventHandler
 * @brief To read the value of light sensor
 * @param None
 * @retval None
 */
void readLightSensorEventHandler(void)
{
	// To check if the program has jumped into this function or not
//	emberAfCorePrintln("readLightSensorEventHandler !");

	// To set that inactive event
	emberEventControlSetInactive(readLightSensorEventControl);

	// Variables to get the brightness value
	static uint16_t light_value = 0, light_value_kalman_new = 0,  light_value_kalman_old = 0;

	// Update the value of light sensor
	light_value = LightSensor_AdcPollingRead();
	light_value_kalman_new = KalmanFilter_updateEstimate(light_value);

	// Check to report the value of light sensor
	if(abs(light_value_kalman_new - light_value_kalman_old) > 30)
	{
		light_value_kalman_old = light_value_kalman_new;
		emberAfCorePrintln("Light: %d lux!", light_value_kalman_new);
		SEND_LightValueReport(2, light_value_kalman_new);
	}

	// Recall event on cycle
	emberEventControlSetDelayMS(readLightSensorEventControl, CYCLE_READLIGHTVALUE);
}

/*
 * @func SEND_LightValueReport
 * @brief To send the value of light sensor into ZC
 * @param src_ep - source endpoint
 * 		  value - the value of light sensor
 * @retval None
 */
void SEND_LightValueReport(uint8_t src_ep, uint16_t value)
{
	// To check if the program has jumped into this function or not
//	emberAfCorePrintln("SEND_LightValueReport !");

	// To fill command information into buffer
	SEND_FillBufferGlobalCommand(ZCL_ILLUM_MEASUREMENT_CLUSTER_ID,
								 ZCL_ILLUM_MEASURED_VALUE_ATTRIBUTE_ID,
								 ZCL_READ_ATTRIBUTES_RESPONSE_COMMAND_ID,
								 (uint8_t *) &value,
								 2,
								 ZCL_INT16U_ATTRIBUTE_TYPE);

	// To send command
	SEND_SendCommandUnicast(src_ep,
							DESTINATION_ENDPOINT,
							ZC_NETWORK_ADDRESS);
}

/*
 * @func readLightSensorEventHandler
 * @brief To read the value of light sensor
 * @param None
 * @retval None
 */
void readTempHumSensorEventHandler(void)
{
	// To check if the program has jumped into this function or not
//	emberAfCorePrintln("readTempHumSensorEventHandler !");

	// To set that inactive event
	emberEventControlSetInactive(readTempHumSensorEventControl);

	// Variables to get the brightness value
	static uint8_t newtemp_value = 0, oldtemp_value = 0, newhum_value = 0, oldhum_value;

	// Update the value of TEMPHUM sensor
	newtemp_value = TemHumSensor_Get_ProcessTemperature();
	newhum_value = TemHumSensor_Get_ProcessHumidity();
	if(newhum_value > 100) newhum_value = 100;

	if(abs(newtemp_value - oldtemp_value) > 2)
	{
		oldtemp_value = newtemp_value;
		emberAfCorePrintln("Temp: %d oC!", newtemp_value);
		SEND_TempValueReport(4, newtemp_value);
	}

	if(abs(newhum_value - oldhum_value) > 2)
	{
		oldhum_value = newhum_value;
		emberAfCorePrintln("Hum: %d %%!", newhum_value);
		SEND_HumValueReport(3, newhum_value);
	}

	// Recall event on cycle
	emberEventControlSetDelayMS(readTempHumSensorEventControl, CYCLE_READTEMPHUMVALUE);
}

/*
 * @func SEND_TempValueReport
 * @brief To send the value of temperature sensor into ZC
 * @param src_ep - source endpoint
 * 		  value - the value of temp sensor
 * @retval None
 */
void SEND_TempValueReport(uint8_t src_ep, uint8_t value)
{
	// To check if the program has jumped into this function or not
//	emberAfCorePrintln("SEND_TempValueReport !");

	// To fill command information into buffer
	SEND_FillBufferGlobalCommand(ZCL_TEMP_MEASUREMENT_CLUSTER_ID,
								 ZCL_TEMP_MEASURED_VALUE_ATTRIBUTE_ID,
								 ZCL_READ_ATTRIBUTES_RESPONSE_COMMAND_ID,
								 (uint8_t *) &value,
								 1,
								 ZCL_INT8U_ATTRIBUTE_TYPE);

	// To send command
	SEND_SendCommandUnicast(src_ep,
							DESTINATION_ENDPOINT,
							ZC_NETWORK_ADDRESS);
}

/*
 * @func SEND_HumValueReport
 * @brief To send the value of light humidity into ZC
 * @param src_ep - source endpoint
 * 		  value - the value of light sensor
 * @retval None
 */
void SEND_HumValueReport(uint8_t src_ep, uint8_t value)
{
	// To check if the program has jumped into this function or not
//	emberAfCorePrintln("SEND_HumValueReport !");

	// To fill command information into buffer
	SEND_FillBufferGlobalCommand(ZCL_RELATIVE_HUMIDITY_MEASUREMENT_CLUSTER_ID,
								 ZCL_RELATIVE_HUMIDITY_MEASURED_VALUE_ATTRIBUTE_ID,
								 ZCL_READ_ATTRIBUTES_RESPONSE_COMMAND_ID,
								 (uint8_t *) &value,
								 1,
								 ZCL_INT8U_ATTRIBUTE_TYPE);

	// To send command
	SEND_SendCommandUnicast(src_ep,
							DESTINATION_ENDPOINT,
							ZC_NETWORK_ADDRESS);
}

/*
 * @func halRebootEventHandler
 * @brief To reboot device after leaving network
 * @param None
 * @retval None
 */
void halRebootEventHandler(void)
{
	// To check if the program has jumped into this function or not
	emberAfCorePrintln("halRebootEventHandler !");

	// To set that inactive event
	emberEventControlSetInactive(halRebootEventControl);

	// Reboot device
	halReboot();
}

/*
 * @func control_onoffStairLight_usingBinding
 * @brief To process binding to control LED
 * @param itself_nodeId - nodeId of device
 * 		  ep_local - local endpoint
 * 		  ep_remote - remote endpoint
 * 		  cluster_id - cluster ID
 * 		  id_command - identifier of command to control LED
 * @retval true/false
 */
void control_onoffStairLight_usingBinding(EmberNodeId itself_nodeId,
										  uint8_t ep_local,
										  uint8_t ep_remote,
										  uint16_t cluster_id,
										  uint8_t id_command)
{
	// To check if the program has jumped into this function or not
	emberAfCorePrintln("control_onoffStairLight_usingBinding !");

	// Variables to get information in binding table
	EmberBindingTableEntry inf_row_bindingtable;			// Information of each row of binding table

	// Process data to control target in binding table
	for(uint8_t i = 0; i < EMBER_BINDING_TABLE_SIZE; i++)
	{
		// Check if index value of binding table exists
		if(emberGetBinding(i, &inf_row_bindingtable) == EMBER_SUCCESS)
		{
			// Filter information to control target correctly
			if(inf_row_bindingtable.type == EMBER_UNICAST_BINDING
					&& inf_row_bindingtable.local == ep_local
					&& inf_row_bindingtable.remote == ep_remote
					&& inf_row_bindingtable.clusterId == cluster_id
					&& itself_nodeId != emberGetBindingRemoteNodeId(i))
			{
				// Control target that has the same clusterId
				switch(cluster_id)
				{
				case ZCL_ON_OFF_CLUSTER_ID:
					// Fill data into buffer
					emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND \
					                           | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER), \
					                           ZCL_ON_OFF_CLUSTER_ID, \
											   id_command, \
					                           "");
					// Set up destination and source endpoint
					emberAfSetCommandEndpoints(inf_row_bindingtable.local, inf_row_bindingtable.remote);
					emberAfCorePrintln("ep_local: %d, ep_remote: %d", inf_row_bindingtable.local, inf_row_bindingtable.remote);
					// Use Unicast to send message
					(void) emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, emberGetBindingRemoteNodeId(i));
					emberAfCorePrintln("node_remote: %d", emberGetBindingRemoteNodeId(i));
					break;

				case ZCL_IAS_ZONE_CLUSTER_ID:
					// Fill data into buffer
					emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND \
					                           | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER), \
											   ZCL_IAS_ZONE_CLUSTER_ID, \
											   id_command, \
					                           "");
					// Set up destination and source endpoint
					emberAfSetCommandEndpoints(inf_row_bindingtable.local, inf_row_bindingtable.remote);
					emberAfCorePrintln("ep_local: %d, ep_remote: %d", inf_row_bindingtable.local, inf_row_bindingtable.remote);
					// Use Unicast to send message
					(void) emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, emberGetBindingRemoteNodeId(i));
					emberAfCorePrintln("node_remote: %d", emberGetBindingRemoteNodeId(i));
					break;

				default:
					break;
				}
			}
		}
		else if(emberGetBinding(i, &inf_row_bindingtable) != EMBER_SUCCESS) break;
	}
}

/*
 * @func process_motion
 * @brief To process motion
 * @param pirAction - Motion or Unmotion
 * @retval None
 */
void process_motion(uint8_t pirAction)
{
	// To check if the program has jumped into this function or not
//	emberAfCorePrintln("process_motion !");

	// Get the status of stair light
	state_stairLight = GPIO_PinOutGet(LED_PORT_2, LED_BLUE_PIN_2);

	// Process MOTION, UNMOTION
	switch(pirAction)
	{
	case PIR_MOTION:
		// Report motion to the remaining switch
		control_onoffStairLight_usingBinding(itself_nodeId, 5, 2, ZCL_IAS_ZONE_CLUSTER_ID, ZCL_ZONE_ENROLL_RESPONSE_COMMAND_ID);
		// Turning on the stair light
		if(state_stairLight == 1)
		{
			control_stairLight(LED_ON, 1, itself_nodeId, ZCL_ON_COMMAND_ID);
		}
//		toggleLed(LED_1, ledYellow, 1, 300, 300);
		break;

	case PIR_UNMOTION:
		// Turning off the stair light
		if(state_stairLight == 0)
		{
			control_stairLight(LED_OFF, 1, itself_nodeId, ZCL_OFF_COMMAND_ID);
		}
		break;

	default:
		break;
	}
}
