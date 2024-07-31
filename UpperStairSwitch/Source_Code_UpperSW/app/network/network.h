/*
 * network.h
 *
 *  Created on: Jan 4, 2024
 *      Author: TRAN DOAN MANH
 */

#ifndef SOURCE_APP_NETWORK_NETWORK_H_
#define SOURCE_APP_NETWORK_NETWORK_H_

// Define enum type for all states of system
typedef enum
{
	POWER_ON_STATE,
	REPORT_STATE,
	IDLE_STATE,
	REBOOT_STATE,
}system_state;

// Define enum type for all states of network
typedef enum
{
	NETWORK_JOIN_SUCCESS,
	NETWORK_JOIN_FALL,
	NETWORK_HAS_PARENT,
	NETWORK_OUT_NETWORK,
	NETWORK_LOST_PARENT,
}network_state;

// Event to join a network
EmberEventControl joinNetworkEventControl;
void joinNetworkEventHandler(void);

// All functions to initialize and find network
void NETWORK_FindAndJoin(void);
void NETWORK_StopFindAndJoin(void);
typedef void (*networkCallbackFunction)(network_state );
void networkInit(networkCallbackFunction );

#endif /* SOURCE_APP_NETWORK_NETWORK_H_ */
