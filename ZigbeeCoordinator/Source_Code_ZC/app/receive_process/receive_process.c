/*
 * receive_process.c
 *
 *  Created on: June 03 , 2024
 *      Author: TRAN DOAN MANH
 */

// All libraries
#include "app/framework/include/af.h"
#include "source/app/main/main.h"
#include "receive_process.h"

// Data array to report on Web
uint8_t data_reportJoinNetwork[2] = {[1] = 0xFF};
uint8_t data_reportLeaveNetwork[2] = {[1] = 0x00};
uint8_t data_reportOnStairLight[2] = {STAIR_LIGHT_CODE, 0xFF};
uint8_t data_reportOffstairLight[2] = {STAIR_LIGHT_CODE, 0x00};
uint8_t data_reportTempValue[2] = {TEMP_VALUE_CODE};
uint8_t data_reportHumValue[2] = {HUM_VALUE_CODE};
uint8_t data_reportLightValue[3] = {LIGHT_INTENSITY_CODE};
uint8_t data_reportControlMode[2] = {CONTROL_MODE_CODE};

/*
 * @func emberAfPreMessageReceivedCallback
 * @brief To receive notice of joining and leaving the network
 * @param incomingMessage - pointer variable of the struct type "EmberAfIncomingMessage" to get information of incoming message
 * @retval true/false
 */
bool emberAfPreMessageReceivedCallback(EmberAfIncomingMessage* incomingMessage)
{
	if(incomingMessage->apsFrame->clusterId == LEAVE_RESPONSE)
	{
		leave_srcNodeID = incomingMessage->source;
		process_LeaveNetwork(leave_srcNodeID);
	}
//	else if(incomingMessage->apsFrame->clusterId == END_DEVICE_ANNOUNCE)
//	{
//		emberAfCorePrintln("LEAVE_RESPONSE");
//		status_send = emberSerialWriteData(comPortUsart2, dataJoin, 2);
//		emberAfCorePrintln("Status_sendJoin: 0x%x !", status_send);
//	}

	return false;
}

/*
 * @func emberAfPreCommandReceivedCallback
 * @brief To process ZCL command
 * @param cmd - pointer variable of the struct type "EmberAfClusterCommand" to get information of incoming message
 * @retval true/false
 */
bool emberAfPreCommandReceivedCallback(EmberAfClusterCommand* cmd)
{
	// Check ClusterID
	if(cmd->clusterSpecific == 0)
	{
		switch(cmd->apsFrame->clusterId)
		{
		case ZCL_BASIC_CLUSTER_ID:
			emberAfCorePrintln("ZCL_BASIC_CLUSTER_ID");
			process_BasicCluster(cmd);
			break;

		case ZCL_ON_OFF_CLUSTER_ID:
			emberAfCorePrintln("ZCL_ON_OFF_CLUSTER_ID");
			// To remove default response messenger
			if(cmd->commandId == ZCL_READ_ATTRIBUTES_RESPONSE_COMMAND_ID)
			{
				process_OnOffCluster(cmd);
			}
			break;

		case ZCL_TEMP_MEASUREMENT_CLUSTER_ID:
			emberAfCorePrintln("ZCL_TEMP_MEASUREMENT_CLUSTER_ID");
			process_TemperatureMeasurementCluster(cmd);
			break;

		case ZCL_RELATIVE_HUMIDITY_MEASUREMENT_CLUSTER_ID:
			emberAfCorePrintln("ZCL_RELATIVE_HUMIDITY_MEASUREMENT_CLUSTER_ID");
			process_RelativeHumidityMeasurementCluster(cmd);
			break;

		case ZCL_ILLUM_MEASUREMENT_CLUSTER_ID:
			emberAfCorePrintln("ZCL_ILLUM_MEASUREMENT_CLUSTER_ID");
			process_IlluminanceMeasurementCluster(cmd);
			break;

		default:
			break;
		}
	}

	return false;
}

/*
 * @func process_LeaveNetwork
 * @brief To report leaving Network for Web
 * @param leave_NodeID - NodeID of switch that leaving network
 * @retval None
 */
void process_LeaveNetwork(uint16_t leave_NodeID)
{
	if(leave_NodeID == nodeID_upstair)
	{
		data_reportLeaveNetwork[0] = NETWORK_UP_CODE;
		statusSendUSART2 = emberSerialWriteData(comPortUsart2, data_reportLeaveNetwork, 2);
		emberAfCorePrintln("Status_sendUpstairLeave: 0x%x !", statusSendUSART2);
	}
	else if(leave_NodeID == nodeID_downstair)
	{
		data_reportLeaveNetwork[0] = NETWORK_DOWN_CODE;
		statusSendUSART2 = emberSerialWriteData(comPortUsart2, data_reportLeaveNetwork, 2);
		emberAfCorePrintln("Status_sendDownstairLeave: 0x%x !", statusSendUSART2);
	}
}

/*
 * @func process_BasicCluster
 * @brief To get the modelID value to identify upper/below switch
 * @param cmd - pointer variable of the struct type "EmberAfClusterCommand" to get information of incoming message
 * @retval None
 */
void process_BasicCluster(EmberAfClusterCommand* cmd)
{
	// Variable to point to the location to get the data
	static uint8_t payloadOffset;
	payloadOffset = (cmd -> payloadStartIndex) + 4;

	// Variable to get the modelID of switch
	uint8_t* modelID;

	// Get the modelID of switch and report joining the network
	modelID = emberAfGetString(cmd->buffer, payloadOffset, cmd->bufLen);
//	emberAfCorePrintln("ModelID: %d!", modelID[0]);
	switch(modelID[0])
	{
	case UPSTAIR:
		if(modelID[1] == 'U' && modelID[2] == 'P')
		{
			nodeID_upstair = cmd -> source;
			emberAfCorePrintln("NodeID of upstair: 0x%2x!", nodeID_upstair);
			// Report joining the network
			data_reportJoinNetwork[0] = NETWORK_UP_CODE;
			statusSendUSART2 = emberSerialWriteData(comPortUsart2, data_reportJoinNetwork, 2);
			emberAfCorePrintln("Status_sendUpstairJoin: 0x%x !", statusSendUSART2);
		}
		break;

	case DOWNSTAIR:
		if(modelID[1] == 'D' && modelID[2] == 'O' && modelID[3] == 'W' && modelID[4] == 'N')
		{
			nodeID_downstair = cmd -> source;
			emberAfCorePrintln("NodeID of downstair: 0x%2x!", nodeID_downstair);
			// Report joining the network
			data_reportJoinNetwork[0] = NETWORK_DOWN_CODE;
			statusSendUSART2 = emberSerialWriteData(comPortUsart2, data_reportJoinNetwork, 2);
			emberAfCorePrintln("Status_sendDownstairJoin: 0x%x !", statusSendUSART2);
		}
		break;

	default:
		break;
	}
}

/*
 * @func process_OnOffCluster
 * @brief To get the On/Off value of stair light is reported from ZR device and report that value to Web
 * @param cmd - pointer variable of the struct type "EmberAfClusterCommand" to get information of incoming message
 * @retval None
 */
void process_OnOffCluster(EmberAfClusterCommand* cmd)
{
	// Variable to point to the location to get the data
	static uint8_t payloadOffset;
	payloadOffset = (cmd -> payloadStartIndex) + 4;

	// Variable to get the state of stair light
	static uint8_t light_state;
	static uint8_t controlMode;

	if(cmd->apsFrame->sourceEndpoint == 1)
	{
		// Get the state of stair light
		light_state = emberAfGetInt8u(cmd->buffer, payloadOffset, cmd->bufLen);
		emberAfCorePrintln("The state of stair light: %d\n", light_state);
		// Report the state of stair light to Web
		if(light_state == 1)
		{
			statusSendUSART2 = emberSerialWriteData(comPortUsart2, data_reportOnStairLight, 2);
			emberAfCorePrintln("Status_sendOn: 0x%x !", statusSendUSART2);
		}
		else if(light_state == 0)
		{
			statusSendUSART2 = emberSerialWriteData(comPortUsart2, data_reportOffstairLight, 2);
			emberAfCorePrintln("Status_sendOff: 0x%x !", statusSendUSART2);
		}
	}
	else if(cmd->apsFrame->sourceEndpoint == 5)
	{
		// Get the control mode
		controlMode = emberAfGetInt8u(cmd->buffer, payloadOffset, cmd->bufLen);
		emberAfCorePrintln("The control mode: %d\n", controlMode);
		// Report the control mode to Web
		if(controlMode == 1)
		{
			data_reportControlMode[1] = 0xFF;
			statusSendUSART2 = emberSerialWriteData(comPortUsart2, data_reportControlMode, 2);
			emberAfCorePrintln("Auto mode: 0x%x !", statusSendUSART2);
		}
		else if(controlMode == 0)
		{
			data_reportControlMode[1] = 0x0;
			statusSendUSART2 = emberSerialWriteData(comPortUsart2, data_reportControlMode, 2);
			emberAfCorePrintln("Manual mode: 0x%x !", statusSendUSART2);
		}
	}
}

/*
 * @func process_TemperatureMeasurementCluster
 * @brief To get the temperature value is reported from ZR device and report that value to Web
 * @param cmd - pointer variable of the struct type "EmberAfClusterCommand" to get information of incoming message
 * @retval None
 */
void process_TemperatureMeasurementCluster(EmberAfClusterCommand* cmd)
{
	// Variable to point to the location to get the data
	static uint8_t payloadOffset;
	payloadOffset = (cmd -> payloadStartIndex) + 4;

	// Variable to get the temperature value
	static uint8_t temp_value;

	// Get the temperature value
	temp_value = emberAfGetInt8u(cmd->buffer, payloadOffset, cmd->bufLen);
	emberAfCorePrintln("The temperature value: %d\n", temp_value);

	// Report the temperature value to Web
	data_reportTempValue[1] = temp_value;
	statusSendUSART2 = emberSerialWriteData(comPortUsart2, data_reportTempValue, 2);
	emberAfCorePrintln("Status_sendTemp: 0x%x !", statusSendUSART2);
}

/*
 * @func process_RelativeHumidityMeasurementCluster
 * @brief To get the relative humidity value is reported from ZR device and report that value to Web
 * @param cmd - pointer variable of the struct type "EmberAfClusterCommand" to get information of incoming message
 * @retval None
 */
void process_RelativeHumidityMeasurementCluster(EmberAfClusterCommand* cmd)
{
	// Variable to point to the location to get the data
	static uint8_t payloadOffset;
	payloadOffset = (cmd -> payloadStartIndex) + 4;

	// Variable to get the relative humidity value
	static uint8_t hum_value;

	// Get the relative humidity value
	hum_value = emberAfGetInt8u(cmd->buffer, payloadOffset, cmd->bufLen);
	emberAfCorePrintln("The relative humidity value: %d\n", hum_value);

	// Report the relative humidity value to Web
	data_reportHumValue[1] = hum_value;
	statusSendUSART2 = emberSerialWriteData(comPortUsart2, data_reportHumValue, 2);
	emberAfCorePrintln("Status_sendHum: 0x%x !", statusSendUSART2);
}

/*
 * @func process_IlluminanceMeasurementCluster
 * @brief To get the light intensity value is reported from ZR device and report that value to Web
 * @param cmd - pointer variable of the struct type "EmberAfClusterCommand" to get information of incoming message
 * @retval None
 */
void process_IlluminanceMeasurementCluster(EmberAfClusterCommand* cmd)
{
	// Variable to point to the location to get the data
	static uint8_t payloadOffset;
	payloadOffset = (cmd -> payloadStartIndex) + 4;

	// Variable to get the light intensity value
	static uint16_t light_intensity;

	// Get the light intensity value
	light_intensity = emberAfGetInt16u(cmd->buffer, payloadOffset, cmd->bufLen);
	emberAfCorePrintln("The light intensity value: %d\n", light_intensity);

	// Report the light intensity value to Web
	data_reportLightValue[1] = light_intensity & 0x00FF;
	data_reportLightValue[2] = (light_intensity >> 8) & 0x00FF;
	statusSendUSART2 = emberSerialWriteData(comPortUsart2, data_reportLightValue, 3);
	emberAfCorePrintln("Status_sendLight: 0x%x !", statusSendUSART2);
}
