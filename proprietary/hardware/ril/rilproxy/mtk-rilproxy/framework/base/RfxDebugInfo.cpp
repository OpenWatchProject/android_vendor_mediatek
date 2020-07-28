/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
/*
 * File name:  RfxDebugInfo.cpp
 * Author: Jun Liu (MTK80064)
 * Description:
 * RIL proxy framework debug info data class
 */

/***************************************************************************** 
 * Include
 *****************************************************************************/

#include <stdlib.h>
#include <cutils/properties.h>
#include "RfxLog.h"
#include "RfxDebugInfo.h"
#include "RfxObject.h"
#include "RfxMessage.h"
#include "RfxAction.h"
#include <libladder.h>
#include <string>

/***************************************************************************** 
 * Define
 *****************************************************************************/
#define RFX_PROPERTY_DEBUG_INFO_ENABLED         "persist.vendor.radio.rfxdbg.enabled"
#define RFX_PROPERTY_DUMP_OBJ_TREE              "persist.vendor.radio.dumpobjtree"
#define RFX_PROPERTY_DUMP_OBJ_CREATION_STACK    "persist.vendor.radio.dumpobjstack"
#define RFX_PROPERTY_DUMP_MSG_LIST              "persist.vendor.radio.dumpmsglist"
#define RFX_PROPERTY_DUMP_ACT_LIST              "persist.vendor.radio.dumpactlist"
#define RFX_PROPERTY_DUMP_ACT_CREATION_STACK    "persist.vendor.radio.dumpactstack"

#define RFX_LOG_TAG "RfxDebugInfo"
/***************************************************************************** 
 * Class RfxDebugInfo
 *****************************************************************************/

bool RfxDebugInfo::s_rfx_debug_info_enabled = false;

void RfxDebugInfo::dump(int level) const {
    if (m_logger) {
        m_logger->dump(level);
    }
    std::string callstack;
    UnwindCurThreadBT(&callstack);
    RFX_LOG_D(RFX_DEBUG_INFO_TAG, "%s", callstack.c_str());
}

void RfxDebugInfo::printLine(const char* string) {
    RFX_LOG_D(RFX_DEBUG_INFO_TAG, "%s", string);
}

#ifdef RFX_OBJ_DEBUG
bool RfxDebugInfo::isRfxDebugInfoEnabled() {
    return s_rfx_debug_info_enabled;
}

void RfxDebugInfo::updateDebugInfoSwitcher() {
    char property_value[PROPERTY_VALUE_MAX] = { 0 };

    property_get(RFX_PROPERTY_DEBUG_INFO_ENABLED, property_value, "0");

    int value = atoi(property_value);

    RFX_LOG_D(RFX_LOG_TAG, "Debug info switcher property_value = %s, value = %d",
                           property_value, value);

    if (value == 1) {
        s_rfx_debug_info_enabled = true;
    } else {
        s_rfx_debug_info_enabled = false;
    }
}

void RfxDebugInfo::dumpIfNeed() {
    if (!isRfxDebugInfoEnabled()) {
        return;
    }

    char property_value[PROPERTY_VALUE_MAX] = { 0 };

    // dump RfxObject tree
    property_get(RFX_PROPERTY_DUMP_OBJ_TREE, property_value, "0");

    int value = atoi(property_value);

    RFX_LOG_D(RFX_LOG_TAG, "Dump RfxObject tree property_value = %s, value = %d",
                           property_value, value);

    if (value == 1) {
        RfxObject::dumpAllObjTree();
        property_set(RFX_PROPERTY_DUMP_OBJ_TREE, "0");
    }

    // dump RfxObject creation stack of specific object address
    property_get(RFX_PROPERTY_DUMP_OBJ_CREATION_STACK, property_value, "0");

    long value16 = strtol(property_value, NULL, 16);

    RFX_LOG_D(RFX_LOG_TAG, "Dump RfxObject creation stack property_value = %s, value = %p",
                           property_value, (void *)value16);

    if (value16 != 0) {
        RfxObject::dumpObjConstructionCallStack((void *)value16);
        property_set(RFX_PROPERTY_DUMP_OBJ_CREATION_STACK, "0");
    }

    // dump RfxMessage list
    property_get(RFX_PROPERTY_DUMP_MSG_LIST, property_value, "0");

    value = atoi(property_value);

    RFX_LOG_D(RFX_LOG_TAG, "Dump Msg list property_value = %s, value = %d",
                           property_value, value);

    if (value == 1) {
        RfxMessage::dumpMsgList();
        property_set(RFX_PROPERTY_DUMP_MSG_LIST, "0");
    }

    // dump RfxAction list
    property_get(RFX_PROPERTY_DUMP_ACT_LIST, property_value, "0");

    value = atoi(property_value);

    RFX_LOG_D(RFX_LOG_TAG, "Dump RfxAction list property_value = %s, value = %d",
                           property_value, value);

    if (value == 1) {
        RfxAction::dumpActionList();
        property_set(RFX_PROPERTY_DUMP_ACT_LIST, "0");
    }

    // dump RfxAction creation stack of specific object address
    property_get(RFX_PROPERTY_DUMP_ACT_CREATION_STACK, property_value, "0");

    value16 = strtol(property_value, NULL, 16);

    RFX_LOG_D(RFX_LOG_TAG, "Dump RfxAction creation stack property_value = %s, value = %p",
                           property_value, (void *)value16);

    if (value16 != 0) {
        RfxAction::dumpActionConstructionCallStack((void *)value16);
        property_set(RFX_PROPERTY_DUMP_ACT_CREATION_STACK, "0");
    }
}
#endif //#ifdef RFX_OBJ_DEBUG