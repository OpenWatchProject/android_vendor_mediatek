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
 * File name:  rfx_status_manager.cpp
 * Author: Jun Liu (MTK80064)
 * Description:
 *  Implementation of status manager.
 */

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RfxStatusManager.h"
#include "RfxLog.h"
#include "RfxRilUtils.h"

#define RFX_LOG_TAG "RfxSM"

/*****************************************************************************
 * Class RfxStatusManager
 *****************************************************************************/
RFX_IMPLEMENT_CLASS("RfxStatusManager", RfxStatusManager, RfxObject);

RFX_STATUS_IMPLEMENT_DEFAULT_VALUE_TABLE;

RfxStatusManager::RfxStatusManager(int slot_id) :
    m_slot_id(slot_id) {
    for (int i = 0; i < RFX_STATUS_KEY_END_OF_ENUM; i++) {
        m_status_list[i] = NULL;
    }
}

const RfxVariant &RfxStatusManager::getDefaultValue(const RfxStatusKeyEnum key) {

    for (int i = 0; s_default_value_table[i].key != RFX_STATUS_KEY_END_OF_ENUM; i++) {
        if (s_default_value_table[i].key == key) {
            return s_default_value_table[i].value;
        }
    }

    static RfxVariant s_dummy_value = RfxVariant();

    return s_dummy_value;
}

const char *RfxStatusManager::getKeyString(const RfxStatusKeyEnum key) {

    for (int i = 0; s_default_value_table[i].key != RFX_STATUS_KEY_END_OF_ENUM; i++) {
        if (s_default_value_table[i].key == key) {
            return s_default_value_table[i].key_string;
        }
    }

    return "defaultValue";
}


const RfxVariant &RfxStatusManager::getValue(const RfxStatusKeyEnum key,
    const RfxVariant &default_value) const {

    RFX_ASSERT(key > RFX_STATUS_KEY_START && key < RFX_STATUS_KEY_END_OF_ENUM);

    if (m_status_list[key] == NULL ||
            m_status_list[key]->value.get_type() == RfxVariant::DATA_TYPE_NULL) {
        return default_value;
    } else {
        return m_status_list[key]->value;
    }
}

const RfxVariant &RfxStatusManager::getValue(const RfxStatusKeyEnum key) const {

    RFX_ASSERT(key > RFX_STATUS_KEY_START && key < RFX_STATUS_KEY_END_OF_ENUM);

    if (m_status_list[key] == NULL ||
            m_status_list[key]->value.get_type() == RfxVariant::DATA_TYPE_NULL) {
        return getDefaultValue(key);
    } else {
        return m_status_list[key]->value;
    }
}


void RfxStatusManager::setValue(const RfxStatusKeyEnum key, const RfxVariant &value,
                                bool force_notify, bool is_default) {

    RFX_ASSERT(key > RFX_STATUS_KEY_START && key < RFX_STATUS_KEY_END_OF_ENUM);

    if (m_status_list[key] == NULL) {
        m_status_list[key] = new StatusListEntry();
        m_status_list[key]->value = value;
        if (RfxRilUtils::hideStatusLog(key)) {
            RFX_LOG_D(RFX_LOG_TAG, "setValue() slot(%d) key = %s, value = [XXX]",
                    m_slot_id, getKeyString(key));
        } else {
            RFX_LOG_D(RFX_LOG_TAG, "setValue() slot(%d) key = %s, value = [%s]",
                    m_slot_id, getKeyString(key), value.toString().string());
        }
    } else {
        RfxVariant old = m_status_list[key]->value;
        bool notify = false;
        bool is_dif = (old != value);

        if (is_dif) {
            if (RfxRilUtils::hideStatusLog(key)) {
                RFX_LOG_D(RFX_LOG_TAG, "setValue() slot(%d) key = %s, old = [XXX],\
new = [XXX], is_force = %s, is_default = %s",
                        m_slot_id, getKeyString(key),
                        force_notify ? "true" : "false",
                        is_default ? "true" : "false");
            } else {
                RFX_LOG_D(RFX_LOG_TAG, "setValue() slot(%d) key = %s, old = [%s],\
new = [%s], is_force = %s, is_default = %s",
                        m_slot_id, getKeyString(key), old.toString().string(),
                        value.toString().string(),
                        force_notify ? "true" : "false",
                        is_default ? "true" : "false");
            }
        }

        if ((is_dif && !is_default) || force_notify) {
            // if setting default value and not force notify, don't notify
            notify = true;
        }

        if (is_dif) {
            if (old.get_type() == RfxVariant::DATA_TYPE_NULL) {
                old = value;
            }
            m_status_list[key]->value = value;
        }

        if (notify) {
            m_status_list[key]->signal.postEmit(key, old, value);
            m_status_list[key]->signal_ex.postEmit(m_slot_id, key, old, value);
        }
    }
}

void RfxStatusManager::registerStatusChanged(const RfxStatusKeyEnum key,
    const RfxStatusChangeCallback &callback) {

    RFX_ASSERT(key > RFX_STATUS_KEY_START && key < RFX_STATUS_KEY_END_OF_ENUM);
    if (m_status_list[key] == NULL) {
        m_status_list[key] = new StatusListEntry();
        m_status_list[key]->value = getDefaultValue(key);
    }
    m_status_list[key]->signal.connect(callback);
}

void RfxStatusManager::unRegisterStatusChanged(const RfxStatusKeyEnum key,
    const RfxStatusChangeCallback &callback) {

    RFX_ASSERT(key > RFX_STATUS_KEY_START && key < RFX_STATUS_KEY_END_OF_ENUM);

    if (m_status_list[key] != NULL) {
        m_status_list[key]->signal.disconnect(callback);
    }
}

void RfxStatusManager::registerStatusChangedEx(const RfxStatusKeyEnum key,
    const RfxStatusChangeCallbackEx &callback) {

    RFX_ASSERT(key > RFX_STATUS_KEY_START && key < RFX_STATUS_KEY_END_OF_ENUM);
    if (m_status_list[key] == NULL) {
        m_status_list[key] = new StatusListEntry();
        m_status_list[key]->value = getDefaultValue(key);
    }
    m_status_list[key]->signal_ex.connect(callback);
}

void RfxStatusManager::unRegisterStatusChangedEx(const RfxStatusKeyEnum key,
    const RfxStatusChangeCallbackEx &callback) {

    RFX_ASSERT(key > RFX_STATUS_KEY_START && key < RFX_STATUS_KEY_END_OF_ENUM);

    if (m_status_list[key] != NULL) {
        m_status_list[key]->signal_ex.disconnect(callback);
    }
}

