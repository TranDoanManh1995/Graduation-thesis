/*
 * receive_process.h
 *
 *  Created on: Mar 17, 2024
 *      Author: TRAN DOAN MANH
 */

#ifndef SOURCE_APP_RECEIVE_PROCESS_RECEIVE_PROCESS_H_
#define SOURCE_APP_RECEIVE_PROCESS_RECEIVE_PROCESS_H_

// To distinguish NodeID of switches
#define UPSTAIR									0x02
#define DOWNSTAIR								0x04

// Report messenger format code
#define STAIR_LIGHT_CODE						0x01 				// Choose to start from 0x01 to remove noise (0x00)
#define TEMP_VALUE_CODE							0x02
#define HUM_VALUE_CODE							0x03
#define LIGHT_INTENSITY_CODE					0x04
#define NETWORK_UP_CODE							0x05
#define NETWORK_DOWN_CODE						0x06
#define CONTROL_MODE_CODE						0x07

// Check status of sending by USART2
EmberStatus statusSendUSART2;

// NodeID of switches
uint16_t nodeID_upstair, nodeID_downstair, leave_srcNodeID;

// Process received command
void process_LeaveNetwork(uint16_t leave_NodeID);
void process_BasicCluster(EmberAfClusterCommand* cmd);
void process_OnOffCluster(EmberAfClusterCommand* cmd);
void process_TemperatureMeasurementCluster(EmberAfClusterCommand* cmd);
void process_RelativeHumidityMeasurementCluster(EmberAfClusterCommand* cmd);
void process_IlluminanceMeasurementCluster(EmberAfClusterCommand* cmd);

#endif /* SOURCE_APP_RECEIVE_PROCESS_RECEIVE_PROCESS_H_ */
