/*
 * network.c
 *
 *  Created on: Jan 4, 2024
 *      Author: TRAN DOAN MANH
 */

// All libraries
#include "app/framework/include/af.h"
#include "source/mid/led_user/led_user.h"
#include "network.h"
#include "callback.h"
#include "debug-printing.h"

// Variable to initialize and find network
uint8_t timeFindAndJoin;
networkCallbackFunction networkEventHandler = NULL;

/*
 * @func networkInit
 * @brief To register process function when the state of network changes
 * @param process_NetworkEvent - process function when the state of network changes
 * @retval None
 */
void networkInit(networkCallbackFunction process_NetworkEvent)
{
	// To check if the program has jumped into this function or not
	emberAfCorePrintln("Network Init !");

	// Assign address of pressHandle for function pointer "networkEventHandler"
	if(process_NetworkEvent != NULL)
	{
		networkEventHandler = process_NetworkEvent;
	}
}

/*
 * @func NETWORK_FindAndJoin
 * @brief To initialize network finding event after 2000 ms
 * @param None
 * @retval None
 */
void NETWORK_FindAndJoin(void)
{
	if(emberAfNetworkState() == EMBER_NO_NETWORK)
	{
		// To initialize network finding event after 2000 ms
		emberEventControlSetDelayMS(joinNetworkEventControl, 2000);
	}
}

/*
 * @func joinNetworkEventHandler
 * @brief To find network
 * @param None
 * @retval None
 */
void joinNetworkEventHandler(void)
{
	// To check if the program has jumped into this function or not
	emberAfCorePrintln("joinNetworkEventHandler !");

	// Inactivate network finding event
	emberEventControlSetInactive(joinNetworkEventControl);

	// Find network
	if(emberAfNetworkState() == EMBER_NO_NETWORK)
	{
		emberAfPluginNetworkSteeringStart();
		timeFindAndJoin++;
		toggleLed(LED_2, ledRed, 1, 500, 500);
		emberEventControlSetDelayMS(joinNetworkEventControl, 5000);
	}
}

/*
 * @func NETWORK_StopFindAndJoin
 * @brief To find network
 * @param None
 * @retval None
 */
void NETWORK_StopFindAndJoin(void)
{
	// To check if the program has jumped into this function or not
	emberAfCorePrintln("NETWORK_StopFindAndJoin !");

	// Stop finding network
	emberAfPluginNetworkSteeringStop();
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
	emberAfCorePrintln("emberAfStackStatusCallback\n");

	// Check the state of network to process
	if(status == EMBER_NETWORK_UP)
	{
		if(timeFindAndJoin > 0)
		{
			NETWORK_StopFindAndJoin();
			if(networkEventHandler != NULL)
			{
				networkEventHandler(NETWORK_JOIN_SUCCESS);
			}
			timeFindAndJoin = 0;
		}
		else
		{
			if(networkEventHandler != NULL)
			{
				networkEventHandler(NETWORK_HAS_PARENT);
			}
		}
	}
	else
	{
		if(emberAfNetworkState() == EMBER_NO_NETWORK)
		{
			if(networkEventHandler != NULL)
			{
				networkEventHandler(NETWORK_OUT_NETWORK);
			}
		}
		else if(emberAfNetworkState() == EMBER_JOINED_NETWORK_NO_PARENT)
		{
			networkEventHandler(NETWORK_LOST_PARENT);
		}
	}

	// This value is ignored by the framework.
	return false;
}

