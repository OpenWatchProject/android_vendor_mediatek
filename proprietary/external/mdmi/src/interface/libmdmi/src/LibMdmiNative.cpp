/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2019. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include "Mdmi.h"
#include "Mdmi_defs.h"
#include "CoreNative.h"

#ifdef BUILD_MDMI_LIB_DEBUG
    MDMI_LIB_TYPE gLibType = MDMI_LIB_DEBUG;
#endif
#ifdef BUILD_MDMI_LIB_LTE
    MDMI_LIB_TYPE gLibType = MDMI_LIB_LTE;
#endif
#ifdef BUILD_MDMI_LIB_GSM
    MDMI_LIB_TYPE gLibType = MDMI_LIB_GSM;
#endif
#ifdef BUILD_MDMI_LIB_UMTS
    MDMI_LIB_TYPE gLibType = MDMI_LIB_UMTS;
#endif
#ifdef BUILD_MDMI_LIB_WIFI
    MDMI_LIB_TYPE gLibType = MDMI_LIB_WIFI;
#endif
#ifdef BUILD_MDMI_LIB_IMS
    MDMI_LIB_TYPE gLibType = MDMI_LIB_IMS;
#endif
#ifdef BUILD_MDMI_LIB_COMMANDS
    MDMI_LIB_TYPE gLibType = MDMI_LIB_COMMANDS;
#endif
#ifdef BUILD_MDMI_LIB_WCDMA
    MDMI_LIB_TYPE gLibType = MDMI_LIB_WCDMA;
#endif
#ifdef BUILD_MDMI_LIB_HSUPA
    MDMI_LIB_TYPE gLibType = MDMI_LIB_HSUPA;
#endif
#ifdef BUILD_MDMI_LIB_HSDPA
    MDMI_LIB_TYPE gLibType = MDMI_LIB_HSDPA;
#endif
#ifdef BUILD_MDMI_LIB_EMBMS
    MDMI_LIB_TYPE gLibType = MDMI_LIB_EMBMS;
#endif

//Creates an MDMI session
//Arguments:
// - address: (in) address of the MDMI device to open. May be set to NULL if the system has only one MDMI capable device
// - session: (out) session object that will be set upon success
//Returns: MDMI_NO_ERROR on success otherwise an error
extern "C" MdmiError MdmiCreateSession(const wchar_t* address, MdmiSession* session) {
    return MdmiCoreCreateSession(gLibType, address, session);
}

//Closes an MDMI session
//Arguments:
// - session: (in) session object that will be closed
//Returns: MDMI_NO_ERROR on success otherwise an error
extern "C" MdmiError MdmiCloseSession(MdmiSession session) {
    return MdmiCoreCloseSession(gLibType, session);
}

//Gets a value identified by its OID from the device
//Arguments:
// - session: (in) identifies the session
// - name: (in) OID of the value to be read
// - value: (in / out) pointer to the MdmiValue struct.
//          If the read is successful, the actual value will be read into this structure.
//          Upon return from this function, the ownership of this struct will the caller and it must be freed to prevent memory leaks
//Returns: MDMI_NO_ERROR on success otherwise an error
// Example:
extern "C" MdmiError MdmiGet(MdmiSession session, const MdmiObjectName* name, MdmiValue* value) {
    return MdmiCoreGet(gLibType, session, name, value);
}

//Sets a value
//Arguments:
// - session: (in) identifies the session
// - name: (in) OID of the value to set
// - value: (in) value to be set
//Returns: MDMI_NO_ERROR on success otherwise an error
extern "C" MdmiError MdmiSet(MdmiSession session, const MdmiObjectName* name, const MdmiValue* value) {
    return MdmiCoreSet(gLibType, session, name, value);
}

//Invokes a command
//Arguments:
// - session: (in) identifies the session
// - name: (in) OID of the command to invoke
// - value: (in) optional value of the command. Can be NULL
//Returns: MDMI_NO_ERROR on success otherwise an error
extern "C" MdmiError MdmiInvoke(MdmiSession session, const MdmiObjectName* name, const MdmiValue* value) {
    return MdmiCoreInvoke(gLibType, session, name, value);
}

//Sets the callback for subscribed events.
//When an event occurs, this callback will be called for each event.
//Arguments:
// - session: (in) identifies the session
// - callback: (in) The callback function pointer. This value will replace previous value. Setting this value to NULL will stop event callbacks.
// - state: (in) Optional state that will be passed when callback function is called
//Returns: MDMI_NO_ERROR on success otherwise an error
extern "C" MdmiError MdmiSetEventCallback(MdmiSession session, MdmiEventCallback callback, void* state) {
    return MdmiCoreSetEventCallback(gLibType, session, callback, state);
}

//Subscribes to an event identified by its OID.
//Arguments:
// - session: (in) identifies the session
// - eventName: (in) identifies the event to be registered.
//Returns: MDMI_NO_ERROR on success otherwise an error
extern "C" MdmiError MdmiSubscribe(MdmiSession session, const MdmiObjectName* eventName) {
    return MdmiCoreSubscribe(gLibType, session, eventName);
}

//Unsubscribes from an event identified by its OID
//Arguments:
// - session: (in) identifies the session
// - eventName: (in) identifies the event to be deregistered.
//Returns: MDMI_NO_ERROR on success otherwise an error
extern "C" MdmiError MdmiUnsubscribe(MdmiSession session, const MdmiObjectName* eventName) {
    return MdmiCoreUnsubscribe(gLibType, session, eventName);
}

//Gets the statistics of the session.
//Arguments:
// - session: (in) identifies the session
// - stats: (in/out) The statistics to be returned.
//Returns: MDMI_NO_ERROR on success otherwise an error
extern "C" MdmiError MdmiGetSessionStats(MdmiSession session, MdmiSessionStats* stats) {
    return MdmiCoreGetSessionStats(gLibType, session, stats);
}