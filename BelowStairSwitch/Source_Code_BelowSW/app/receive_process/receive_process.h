/*
 * receive_process.h
 *
 *  Created on: Mar 17, 2024
 *      Author: TRAN DOAN MANH
 */

#ifndef SOURCE_APP_RECEIVE_PROCESS_RECEIVE_PROCESS_H_
#define SOURCE_APP_RECEIVE_PROCESS_RECEIVE_PROCESS_H_

// Variables for binding
boolean SWBinding;
boolean PIRBinding;

// Process received command
void processOnOffCluster(EmberAfClusterCommand* cmd);
void processIASZoneCluster(EmberAfClusterCommand* cmd);
void processIdentifyCluster(EmberAfClusterCommand* cmd);

#endif /* SOURCE_APP_RECEIVE_PROCESS_RECEIVE_PROCESS_H_ */
