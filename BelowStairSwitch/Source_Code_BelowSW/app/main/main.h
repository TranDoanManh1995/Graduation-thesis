/*
 * main.h
 *
 *  Created on: May 20, 2024
 *      Author: TRAN DOAN MANH
 */

#ifndef SOURCE_APP_MAIN_MAIN_H_
#define SOURCE_APP_MAIN_MAIN_H_

#include "source/app/network/network.h"
#include "source/mid/button_user/button_user.h"
#include "source/mid/led_user/led_user.h"

#define DESTINATION_ENDPOINT					1
#define ZC_NETWORK_ADDRESS						0x0000
#define MAX_DATA_COMMAND_SIZE					20

#define CYCLE_READLIGHTVALUE					30000
#define CYCLE_READTEMPHUMVALUE					60000

#define MOTION									0x0001
#define UNMOTION								0

#define AUTO									1
#define MANUAL									0

// Variable about the state of system
system_state systemState;

// Variable about the state of binding
EmberAfStatus status_binding;

// Get the state of LED
boolean state_stairLight;

// Get nodeId itself
EmberNodeId itself_nodeId;

// To bind two switches and two PIRs
boolean SW_binding;
boolean PIR_binding;

// Allow or not PIR
boolean control_mode;

// Event to process the state of system
EmberEventControl mainStateEventControl;
void mainStateEventHandler(void);

// Event to read value of Light sensor and report
EmberEventControl readLightSensorEventControl;
void readLightSensorEventHandler(void);

// Event to read value of Temp_Hum sensor and report
EmberEventControl readTempHumSensorEventControl;
void readTempHumSensorEventHandler(void);

// Event to reboot device
EmberEventControl halRebootEventControl;
void halRebootEventHandler(void);

// All functions to process
void process_NetworkEvent(network_state networkResult);
void process_pressnumber(uint8_t index_button, BUTTON_Event_t pressnumber);
void process_motion(uint8_t pirAction);
void select_controlMode(uint8_t src_ep, uint8_t controlMode, ledColor modeLED, boolean PIRinterrupt);
void control_stairLight(uint8_t LEDstatus, uint8_t src_ep, uint16_t nodeID_itself, uint8_t commandID);
void SEND_ReportInfoToZC(uint8_t src_ep);
void SEND_ControlModeReport(uint8_t src_ep, uint8_t value);
void SEND_FillBufferGlobalCommand(EmberAfClusterId clusterID,
		 	 	 	 	 	 	  EmberAfAttributeId attributeID,
								  uint8_t globalCommand,
								  uint8_t *value,
								  uint8_t length,
								  uint8_t dataType);
void SEND_SendCommandUnicast(uint8_t src_ep,
							 uint8_t destination,
							 uint8_t address);
void SEND_LightValueReport(uint8_t src_ep, uint16_t value);
void SEND_TempValueReport(uint8_t src_ep, uint8_t value);
void SEND_HumValueReport(uint8_t src_ep, uint8_t value);
void SEND_OnOffStateReport(uint8_t src_ep, uint8_t value);
void control_onoffStairLight_usingBinding(EmberNodeId itself_nodeId,
										  uint8_t ep_local,
										  uint8_t ep_remote,
										  uint16_t cluster_id,
										  uint8_t id_command);

#endif /* SOURCE_APP_MAIN_MAIN_H_ */
