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
 * File name:  RfxAction.cpp
 * Author: Jun Liu (MTK80064)
 * Description:
 *  Define the implementation of action.
 */

/*****************************************************************************
 * Include
 *****************************************************************************/

#include "RfxAction.h"

#define RFX_LOG_TAG "RfxAction"
/*****************************************************************************
 * Class RfxAction
 *****************************************************************************/
#ifdef RFX_OBJ_DEBUG
Vector<RfxDebugInfo*> *RfxAction::s_root_action_debug_info = NULL;

RfxAction::RfxAction() {
    if (RfxDebugInfo::isRfxDebugInfoEnabled()) {
        m_debug_info = new RfxDebugInfo(static_cast<IRfxDebugLogger *>(this), (void *)this);
        if (s_root_action_debug_info == NULL) {
            s_root_action_debug_info = new Vector<RfxDebugInfo*>();
        }
        s_root_action_debug_info->add(m_debug_info);
    } else {
        m_debug_info = NULL;
    }
}

RfxAction::~RfxAction() {
    if (m_debug_info != NULL) {
        size_t size = s_root_action_debug_info->size();
        for (size_t i = 0; i < size; i++) {
            const RfxDebugInfo *item = s_root_action_debug_info->itemAt(i);
            if (item == m_debug_info) {
                s_root_action_debug_info->removeAt(i);
                break;
            }
        }
        delete(m_debug_info);
        m_debug_info = NULL;
    }
}

void RfxAction::dumpActionList() {
    size_t size = RfxAction::s_root_action_debug_info->size();
    RFX_LOG_D(RFX_DEBUG_INFO_TAG, "dumpActionList() Action count is %zu", size);
    for (size_t i = 0; i < size; i++) {
        const RfxDebugInfo *item = RfxAction::s_root_action_debug_info->itemAt(i);
        item->getLogger()->dump();
    }
}

void RfxAction::dumpActionConstructionCallStack(void *action_address) {
    size_t size = RfxAction::s_root_action_debug_info->size();
    for (size_t i = 0; i < size; i++) {
        const RfxDebugInfo *item = RfxAction::s_root_action_debug_info->itemAt(i);
        if (item->getUserData() == action_address) {
            RFX_LOG_D(RFX_DEBUG_INFO_TAG, "dumpActionConstructionCallStack() Found action, start to dump callstack");
            item->dump();
            return;
        }
    }
    RFX_LOG_D(RFX_DEBUG_INFO_TAG, "dumpActionConstructionCallStack() Can't find action %p", action_address);
}

#endif //#ifdef RFX_OBJ_DEBUG

