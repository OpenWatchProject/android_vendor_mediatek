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

#ifndef __CORE_NATIVE_H__
#define __CORE_NATIVE_H__

// MDMI public interface
#include "Mdmi.h"
// Internal hears
#include "Mdmi_defs.h"

bool checkSessionState(const char *caller, MDMI_LIB_TYPE type, MdmiSession sessionId);

MdmiError MdmiCoreCreateSession(MDMI_LIB_TYPE type, const wchar_t* address, MdmiSession* session);
MdmiError MdmiCoreCloseSession(MDMI_LIB_TYPE type, MdmiSession session);
MdmiError MdmiCoreGet(MDMI_LIB_TYPE type, MdmiSession session, const MdmiObjectName* name, MdmiValue* value);
MdmiError MdmiCoreSet(MDMI_LIB_TYPE type, MdmiSession session, const MdmiObjectName* name, const MdmiValue* value);
MdmiError MdmiCoreInvoke(MDMI_LIB_TYPE type, MdmiSession session, const MdmiObjectName* name, const MdmiValue* value);
MdmiError MdmiCoreSetEventCallback(MDMI_LIB_TYPE type, MdmiSession session, MdmiEventCallback callback, void* state);
MdmiError MdmiCoreSubscribe(MDMI_LIB_TYPE type, MdmiSession session, const MdmiObjectName* eventName);
MdmiError MdmiCoreUnsubscribe(MDMI_LIB_TYPE type, MdmiSession session, const MdmiObjectName* eventName);
MdmiError MdmiCoreGetSessionStats(MDMI_LIB_TYPE type, MdmiSession session, MdmiSessionStats* stats);

#endif