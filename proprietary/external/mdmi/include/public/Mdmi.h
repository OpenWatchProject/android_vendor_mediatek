/**
* Copyright (C) 2015 Verizon. All Rights Reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

// Contains the general definitions for the MDMI device interface
// Version 2.1
// https://github.com/GSMATerminals/TSG-Standard-Diag-Public/blob/master/MDMI.h

#ifndef _MDMI_H_
#define _MDMI_H_

#ifdef  __cplusplus
extern "C" {
#endif

//errors
#define MDMI_NO_ERROR                               0
#define MDMI_ERROR_GENERIC                          1000
#define MDMI_ERROR_NOT_INITIALIZED                  1001
#define MDMI_ERROR_INVALID_SESSION                  1002
#define MDMI_ERROR_TOO_MANY_SESSIONS                1003
#define MDMI_ERROR_INVALID_OPERATION                1004
#define MDMI_ERROR_NO_SUCH_ADDRESS                  1005
#define MDMI_ERROR_INVALID_OID                      1006
#define MDMI_ERROR_OID_READ_ONLY                    1007
#define MDMI_ERROR_OID_NOT_FOUND                    1008
#define MDMI_ERROR_OID_NOT_ACCESIBLE                1009
#define MDMI_ERROR_VALUE_WRONG_TYPE                 1010
#define MDMI_ERROR_ALREADY_REGISTERED               1011
#define MDMI_ERROR_INVALID_ARGUMENT                 1012
#define MDMI_ERROR_ALREADY_INITIALZIED              1013
#define MDMI_ERROR_PERMISSION_DENIED_SIGCHK_FAIL    1014
#define MDMI_ERROR_PERMISSION_DENIED_GROUPID_FAIL   1015

typedef unsigned char   MdmiTypeUInt8;
typedef unsigned int    MdmiTypeUInt32;

typedef MdmiTypeUInt32 MdmiSession;
typedef MdmiTypeUInt32 MdmiError;

typedef struct {
    MdmiTypeUInt32 length;
    MdmiTypeUInt32* ids;
} MdmiObjectId;

typedef MdmiObjectId MdmiObjectName;

typedef struct {
    MdmiTypeUInt32 length;
    MdmiTypeUInt8* data;
} MdmiValue;

typedef struct
{
	// Size of this struct
	MdmiTypeUInt32 size;

	// Total number of events received.
	MdmiTypeUInt32 totalEvents;

	// Number of events that were discarded, usually due to
	// recipient not being slow.
	MdmiTypeUInt32 droppedEvents;

	// The maximum buffer size allocated for events (in bytes)
	MdmiTypeUInt32 eventBufferCapacity;

	// The current buffer capacity in use (in bytes)
	MdmiTypeUInt32 eventBufferInUse;
} MdmiSessionStats;

//The callback function that will be used when a subscribed event occurs.
//Arguments:
// - session: identifies the session
// - eventName: OID of the event. 
//              The owner of this struct is the MDMI library and it should not be modified by the caller
// - eventValue: Optional value of the event. If the trap does not have a value, NULL will be provided.
//               The owner of this struct is the MDMI library and it should not be modified by the caller
// - state: Optional user defined state, associated with the callback function
typedef void(*MdmiEventCallback)(MdmiSession session, const MdmiObjectName* eventName, const MdmiValue* eventValue, void* state);

//Creates an MDMI session
//Arguments:
// - address: (in) address of the MDMI device to open. May be set to NULL if the system has only one MDMI capable device
// - session: (out) session object that will be set upon success
//Returns: MDMI_NO_ERROR on success otherwise an error
MdmiError MdmiCreateSession(const wchar_t* address, MdmiSession* session);

//Closes an MDMI session
//Arguments:
// - session: (in) session object that will be closed
//Returns: MDMI_NO_ERROR on success otherwise an error
MdmiError MdmiCloseSession(MdmiSession session);

//Gets a value identified by its OID from the device
//Arguments:
// - session: (in) identifies the session
// - name: (in) OID of the value to be read
// - value: (in / out) pointer to the MdmiValue struct. 
//          If the read is successful, the actual value will be read into this structure.
//          Upon return from this function, the ownership of this struct will the caller and it must be freed to prevent memory leaks
//Returns: MDMI_NO_ERROR on success otherwise an error
// Example:
MdmiError MdmiGet(MdmiSession session, const MdmiObjectName* name, MdmiValue* value);

//Sets a value
//Arguments:
// - session: (in) identifies the session
// - name: (in) OID of the value to set
// - value: (in) value to be set
//Returns: MDMI_NO_ERROR on success otherwise an error
MdmiError MdmiSet(MdmiSession session, const MdmiObjectName* name, const MdmiValue* value);

//Invokes a command
//Arguments:
// - session: (in) identifies the session
// - name: (in) OID of the command to invoke
// - value: (in) optional value of the command. Can be NULL
//Returns: MDMI_NO_ERROR on success otherwise an error
MdmiError MdmiInvoke(MdmiSession session, const MdmiObjectName* name, const MdmiValue* value);

//Sets the callback for subscribed events.
//When an event occurs, this callback will be called for each event.
//Arguments:
// - session: (in) identifies the session
// - callback: (in) The callback function pointer. This value will replace previous value. Setting this value to NULL will stop event callbacks.
// - state: (in) Optional state that will be passed when callback function is called
//Returns: MDMI_NO_ERROR on success otherwise an error
MdmiError MdmiSetEventCallback(MdmiSession session, MdmiEventCallback callback, void* state);

//Subscribes to an event identified by its OID.
//Arguments:
// - session: (in) identifies the session
// - eventName: (in) identifies the event to be registered.
//Returns: MDMI_NO_ERROR on success otherwise an error
MdmiError MdmiSubscribe(MdmiSession session, const MdmiObjectName* eventName);

//Unsubscribes from an event identified by its OID
//Arguments:
// - session: (in) identifies the session
// - eventName: (in) identifies the event to be deregistered.
//Returns: MDMI_NO_ERROR on success otherwise an error
MdmiError MdmiUnsubscribe(MdmiSession session, const MdmiObjectName* eventName);

//Gets the statistics of the session.
//Arguments:
// - session: (in) identifies the session
// - stats: (in/out) The statistics to be returned.
//Returns: MDMI_NO_ERROR on success otherwise an error
MdmiError MdmiGetSessionStats(MdmiSession session, MdmiSessionStats* stats);

#ifdef  __cplusplus
}
#endif

#endif

