/*
 * receive_process.c
 *
 *  Created on: June 03 , 2024
 *      Author: TRAN DOAN MANH
 */

// All libraries
#include "app/framework/include/af.h"
#include "source/app/main/main.h"
#include "source/mid/led_user/led_user.h"
#include "source/driver/pir_user/pir_user.h"
#include "receive_process.h"

/*
 * @func emberAfPreCommandReceivedCallback
 * @brief To process ZCL command
 * @param cmd - pointer variable of the struct type "EmberAfClusterCommand" to get information of incoming message
 * @retval true/false
 */
bool emberAfPreCommandReceivedCallback(EmberAfClusterCommand* cmd)
{
	// Check ClusterID
	if(cmd->clusterSpecific)
	{
		switch(cmd->apsFrame->clusterId)
		{
		case ZCL_ON_OFF_CLUSTER_ID:
			emberAfCorePrintln("ZCL_ON_OFF_CLUSTER_ID");
			processOnOffCluster(cmd);
			break;

		case ZCL_IAS_ZONE_CLUSTER_ID:
			emberAfCorePrintln("ZCL_IAS_ZONE_CLUSTER_ID");
			processIASZoneCluster(cmd);
			break;

		case ZCL_IDENTIFY_CLUSTER_ID:
			emberAfCorePrintln("ZCL_IDENTIFY_CLUSTER_ID");
			processIdentifyCluster(cmd);
			break;

		default:
			break;
		}
	}

	return false;
}

/** @brief Complete
 *
 * This callback is fired by the initiator when the Find and Bind process is
 * complete.
 *
 * @param status Status code describing the completion of the find and bind
 * process Ver.: always
 */
void emberAfPluginFindAndBindInitiatorCompleteCallback(EmberStatus status)
{
	emberAfCorePrintln("Find and Bind Initiator: Complete: 0x%X", status);
	// Display LED when binding between Initiator & Target of two different devices is success
	if(status == EMBER_SUCCESS)
	{
		if(SWBinding == 1)
		{
			emberAfCorePrintln("Binding for SW is success !");
			toggleLed(LED_2, ledGreen, 3, 300, 300);
			SWBinding = 0;
		}
		else if(PIRBinding == 1)
		{
			emberAfCorePrintln("Binding for PIR is success !");
			toggleLed(LED_2, ledBlue, 3, 300, 300);
			PIRBinding = 0;
		}
	}
}

/*
 * @func processOnOffCluster
 * @brief To process the cluster On/Off to use binding and turning ON/OFF led
 * @param cmd - pointer variable of the struct type "EmberAfClusterCommand" to get information of incoming message
 * @retval None
 */
void processOnOffCluster(EmberAfClusterCommand* cmd)
{
	// Get commandID, NodeID of endpoint of incoming message
	uint8_t commandID = cmd -> commandId;
	uint16_t src_nodeId = cmd->source;
	uint8_t src_ep = cmd -> apsFrame -> sourceEndpoint;

	// Process On/Off Led and report state
	switch(commandID)
	{
	case ZCL_ON_COMMAND_ID:
		// Distinguish between control mode and light control
		if(src_ep == 1)
		{
			emberAfCorePrintln("The stair light is turning on!");
			// Receive control from ZC and transmit the remaining switch
			if(src_nodeId == ZC_NodeID)
			{
				// Report the LED control command for the remaining switch
				control_onoffStairLight_usingBinding(itself_nodeId, 1, 1, ZCL_ON_OFF_CLUSTER_ID, ZCL_ON_COMMAND_ID);
			}
			// Get state of stair light
			state_stairLight = 0;
		}
		else if(src_ep == 5)
		{
			// To select the AUTO mode
			select_controlMode(AUTO, ledBlue, true);
		}
		break;

	case ZCL_OFF_COMMAND_ID:
		// Distinguish between control mode and light control
		if(src_ep == 1)
		{
			emberAfCorePrintln("The stair light is turning off!");
			// Receive control from ZC and transmit the remaining switch
			if(src_nodeId == ZC_NodeID)
			{
				// Report the LED control command for the remaining switch
				control_onoffStairLight_usingBinding(itself_nodeId, 1, 1, ZCL_ON_OFF_CLUSTER_ID, ZCL_OFF_COMMAND_ID);
			}
			// Get state of stair light
			state_stairLight = 1;
		}
		else if(src_ep == 5)
		{
			// To select the MANUAL mode
			select_controlMode(MANUAL, ledRed, false);
		}
		break;

	default:
		break;
	}
}

/*
 * @func processIASZoneCluster
 * @brief To process the cluster IAS Zone
 * @param cmd - pointer variable of the struct type "EmberAfClusterCommand" to get information of incoming message
 * @retval None
 */
void processIASZoneCluster(EmberAfClusterCommand* cmd)
{
	// Get commandID of incoming message
	uint8_t commandID = cmd -> commandId;

	// Process On/Off Led and report state
	switch(commandID)
	{
	case ZCL_ZONE_ENROLL_RESPONSE_COMMAND_ID:
		emberAfCorePrintln("Having motion !");
		pirState = PIR_STATE_WAIT_2S;
		emberEventControlSetActive(pinDetectEventControl);
		break;

	default:
		break;
	}
}

/*
 * @func processIdentifyCluster
 * @brief To process the cluster Identify using for Binding
 * @param cmd - pointer variable of the struct type "EmberAfClusterCommand" to get information of incoming message
 * @retval None
 */
void processIdentifyCluster(EmberAfClusterCommand* cmd)
{
	// Get NodeID of endpoint of incoming message
	uint16_t nodeId_target = cmd->source;
	uint8_t src_ep = cmd -> apsFrame -> sourceEndpoint;
	uint8_t des_ep = cmd -> apsFrame -> destinationEndpoint;
//	emberAfCorePrintln("NodeID target: 0x%2x !", nodeId_target);
//	emberAfCorePrintln("Source endpoint: %d !", src_ep);
//	emberAfCorePrintln("Destination endpoint: %d !", des_ep);

	// Display LED when binding between Initiator & Target of two different devices occurs
	if(nodeId_target != itself_nodeId)
	{
		if(src_ep == 1 && des_ep == 1)
		{
			SWBinding = 1;
		}
		else if(src_ep == 5 && des_ep == 2)
		{
			PIRBinding = 1;
		}
	}
}
