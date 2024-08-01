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
#include "zigbee-framework/zigbee-device-common.h"
#include "app/framework/plugin/find-and-bind-target/find-and-bind-target.h"
#include "app/framework/plugin/find-and-bind-initiator/find-and-bind-initiator.h"
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

	// Initialize LED, Button, Network and PIR
	ledInit();
	buttonInit(process_pressnumber);
	networkInit(process_NetworkEvent);
	PIR_Init(process_motion);

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
		// Binding
		SW_binding = false;
		PIR_binding = false;
		// Disable interrupt for PIR
		PIR_Enable(false);
		// Control mode
		control_mode = AUTO;
		select_controlMode(MANUAL, ledRed, false);
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
			if(state_stairLight == 0)
			{
				emberAfCorePrintln("Turning off the stair light!");
				// Report the LED control command for the remaining switch
				control_onoffStairLight_usingBinding(itself_nodeId, 1, 1, ZCL_ON_OFF_CLUSTER_ID, ZCL_OFF_COMMAND_ID);
			}
			else if (state_stairLight == 1)
			{
				emberAfCorePrintln("Turning on the stair light!");
				// Report the LED control command for the remaining switch
				control_onoffStairLight_usingBinding(itself_nodeId, 1, 1, ZCL_ON_OFF_CLUSTER_ID, ZCL_ON_COMMAND_ID);
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
				select_controlMode(MANUAL, ledRed, false);
				// Report the control mode to the remaining switch
				control_onoffStairLight_usingBinding(itself_nodeId, 2, 5, ZCL_ON_OFF_CLUSTER_ID, ZCL_OFF_COMMAND_ID);
			}
			else if(control_mode == AUTO)
			{
				// To select the AUTO mode
				select_controlMode(AUTO, ledBlue, true);
				// Report the control mode to the remaining switch
				control_onoffStairLight_usingBinding(itself_nodeId, 2, 5, ZCL_ON_OFF_CLUSTER_ID, ZCL_ON_COMMAND_ID);
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
				// Activated initiator
				emberAfPluginFindAndBindInitiatorStart(1);
				// Check opening initiator
				if(status_binding == EMBER_ZCL_STATUS_SUCCESS)
				{
					toggleLed(LED_2, ledGreen, 2, 300, 300);
				}
			}
			else if(SW_binding == true)
			{
				SW_binding = false;
				// Activated target
				emberAfPluginFindAndBindTargetStart(1);
				// Check opening target
				if(status_binding == EMBER_ZCL_STATUS_SUCCESS)
				{
					toggleLed(LED_2, ledGreen, 1, 300, 300);
				}
			}
		}
		break;

	case press_4:
		if(index_button == BUTTON_2)
		{
			emberAfCorePrintln("Button 2 is four times!");
			if(PIR_binding == false)
			{
				PIR_binding = true;
				// Activated initiator
				emberAfPluginFindAndBindInitiatorStart(2);
				// Check opening initiator
				if(status_binding == EMBER_ZCL_STATUS_SUCCESS)
				{
					toggleLed(LED_2, ledBlue, 2, 300, 300);
				}
			}
			else if(PIR_binding == true)
			{
				PIR_binding = false;
				// Activated target
				emberAfPluginFindAndBindTargetStart(2);
				// Check opening target
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
	emberAfCorePrintln("SEND_FillBufferGlobalCommand");

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
	emberAfCorePrintln("SEND_SendCommandUnicast");

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
	uint8_t modelID[3] = {2, 'U', 'P'};

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
								 3,
								 ZCL_CHAR_STRING_ATTRIBUTE_TYPE);

	// Send command
	SEND_SendCommandUnicast(src_ep,
							DESTINATION_ENDPOINT,
							ZC_NETWORK_ADDRESS);
}

/*
 * @func select_controlMode
 * @brief To select the "AUTO" control mode
 * @param controlMode - To select the control mode
 * 		  modeLED - LED to display the control mode
 * 		  PIRinterrupt - active/inactive the interrupt on PIR sensor
 * @retval None
 */
void select_controlMode(uint8_t controlMode, ledColor modeLED, boolean PIRinterrupt)
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
	// Signal light
	turnOffLed(LED_1);
	turnOnLed(LED_1, modeLED);
	// Enable/Disable PIR
	PIR_Enable(PIRinterrupt);
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
	emberAfCorePrintln("Report the network state of below stair switch!");

	SEND_FillBufferGlobalCommand(ZCL_ON_OFF_CLUSTER_ID,
								 ZCL_ON_OFF_ATTRIBUTE_ID,
								 ZCL_READ_ATTRIBUTES_RESPONSE_COMMAND_ID,
								 (uint8_t *) &value,
								 1,
								 ZCL_BOOLEAN_ATTRIBUTE_TYPE);

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
	emberAfCorePrintln("control_onoffStairLight_usingBinding");

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

	// Process MOTION, UNMOTION
	switch(pirAction)
	{
	case PIR_MOTION:
		// Report motion to the remaining switch
		control_onoffStairLight_usingBinding(itself_nodeId, 2, 5, ZCL_IAS_ZONE_CLUSTER_ID, ZCL_ZONE_ENROLL_RESPONSE_COMMAND_ID);
		// Turning on the stair light
		if(state_stairLight == 1)
		{
			emberAfCorePrintln("Turning on the stair light!");
			// Report the LED control command for the remaining switch
			control_onoffStairLight_usingBinding(itself_nodeId, 1, 1, ZCL_ON_OFF_CLUSTER_ID, ZCL_ON_COMMAND_ID);
		}
//		toggleLed(LED_1, ledYellow, 1, 300, 300);
		break;

	case PIR_UNMOTION:
		// Turning off the stair light
		emberAfCorePrintln("Turning off the stair light !");
		break;

	default:
		break;
	}
}
