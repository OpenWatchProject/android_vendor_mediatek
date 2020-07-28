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
 * File name:  RfxStatusManager.h
 * Author: Jun Liu (MTK80064)
 * Description:
 *  Define the prototypes of status manager.
 */

#ifndef __RFX_STATUS_MANAGER_H__
#define __RFX_STATUS_MANAGER_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RfxDefs.h"
#include "RfxStatusDefs.h"
#include "RfxVariant.h"
#include "RfxObject.h"
#include "RfxSignal.h"

/*****************************************************************************
 * Typedef
 *****************************************************************************/

typedef RfxCallback3<RfxStatusKeyEnum, RfxVariant, RfxVariant> RfxStatusChangeCallback;

typedef RfxCallback4<int, RfxStatusKeyEnum, RfxVariant, RfxVariant> RfxStatusChangeCallbackEx;


/*****************************************************************************
 * Class RfxStatusManager
 *****************************************************************************/

class RfxStatusManager : public RfxObject {

    RFX_DECLARE_CLASS(RfxStatusManager);

public:

    RfxStatusManager() : m_slot_id(RFX_SLOT_ID_UNKNOWN) {
            for (int i = 0; i < RFX_STATUS_KEY_END_OF_ENUM; i++) {
            m_status_list[i] = NULL;
        }
    }

    explicit RfxStatusManager(int slot_id);

    virtual ~RfxStatusManager() {}

public:

    void registerStatusChanged(const RfxStatusKeyEnum key,
        const RfxStatusChangeCallback &callback);

    void unRegisterStatusChanged(const RfxStatusKeyEnum key,
        const RfxStatusChangeCallback &callback);

    void registerStatusChangedEx(const RfxStatusKeyEnum key,
        const RfxStatusChangeCallbackEx &callback);

    void unRegisterStatusChangedEx(const RfxStatusKeyEnum key,
        const RfxStatusChangeCallbackEx &callback);

    bool getBoolValue(const RfxStatusKeyEnum key, bool default_value) const;

    bool getBoolValue(const RfxStatusKeyEnum key) const;

    void setBoolValue(const RfxStatusKeyEnum key, bool value,
                     bool force_notify = false, bool is_default = false);

    int getIntValue(const RfxStatusKeyEnum key, int default_value) const;

    int getIntValue(const RfxStatusKeyEnum key) const;

    void setIntValue(const RfxStatusKeyEnum key, int value,
                     bool force_notify = false, bool is_default = false);

    float getFloatValue(const RfxStatusKeyEnum key, float default_value) const;

    float getFloatValue(const RfxStatusKeyEnum key) const;

    void setFloatValue(const RfxStatusKeyEnum key, float value,
                     bool force_notify = false, bool is_default = false);

    const String8& getString8Value(const RfxStatusKeyEnum key,
        const String8& default_value) const;

    const String8& getString8Value(const RfxStatusKeyEnum key) const;

    void setString8Value(const RfxStatusKeyEnum key, const String8& value,
                     bool force_notify = false, bool is_default = false);

    const RfxNwServiceState &getServiceStateValue(const RfxStatusKeyEnum key,
            const RfxNwServiceState& default_value) const;

    const RfxNwServiceState &getServiceStateValue(const RfxStatusKeyEnum key) const;

    void setServiceStateValue(const RfxStatusKeyEnum key, const RfxNwServiceState &value,
                     bool force_notify = false, bool is_default = false);

    const RfxSocketState &getSocketStateValue(const RfxStatusKeyEnum key,
            const RfxSocketState& default_value) const;

    const RfxSocketState &getSocketStateValue(const RfxStatusKeyEnum key) const;

    void setSocketStateValue(const RfxStatusKeyEnum key, const RfxSocketState &value,
                     bool force_notify = false, bool is_default = false);

    const RfxCallState &getCallStateValue(const RfxStatusKeyEnum key,
            const RfxCallState& default_value) const;

    const RfxCallState &getCallStateValue(const RfxStatusKeyEnum key) const;

    void setCallStateValue(const RfxStatusKeyEnum key, const RfxCallState &value,
                     bool force_notify = false, bool is_default = false);

    const RfxDataSettings &getDataSettingsValue(const RfxStatusKeyEnum key) const;

    const RfxDataSettings &getDataSettingsValue(const RfxStatusKeyEnum key,
            const RfxDataSettings& default_value) const;

    void setDataSettingsValue(const RfxStatusKeyEnum key, const RfxDataSettings &value,
            bool force_notify = false, bool is_default = false);

    const RfxVariant &getValue(const RfxStatusKeyEnum key,
        const RfxVariant &default_value) const;

    const RfxVariant &getValue(const RfxStatusKeyEnum key) const;

    void setValue(const RfxStatusKeyEnum key, const RfxVariant &value,
                     bool force_notify = false, bool is_default = false);

    int getSlotId() const {return m_slot_id;}

    static const RfxVariant &getDefaultValue(const RfxStatusKeyEnum key);
    static const char *getKeyString(const RfxStatusKeyEnum key);

private:

    typedef struct _StatusListEntry {
        RfxVariant value;
        RfxSignal3<RfxStatusKeyEnum, RfxVariant, RfxVariant> signal;
        RfxSignal4<int, RfxStatusKeyEnum, RfxVariant, RfxVariant> signal_ex;
    } StatusListEntry;

    typedef struct _StatusDefaultValueEntry {
        RfxStatusKeyEnum key;
        const char *key_string;
        RfxVariant value;
    } StatusDefaultValueEntry;

    RFX_STATUS_DECLARE_DEFAULT_VALUE_TABLE;

private:

    int m_slot_id;
    StatusListEntry *m_status_list[RFX_STATUS_KEY_END_OF_ENUM];
};

inline
bool RfxStatusManager::getBoolValue(const RfxStatusKeyEnum key,
    bool default_value) const {
    return getValue(key, RfxVariant(default_value)).asBool();
}

inline
bool RfxStatusManager::getBoolValue(const RfxStatusKeyEnum key) const {
    return getValue(key).asBool();
}


inline
void RfxStatusManager::setBoolValue(const RfxStatusKeyEnum key, bool value,
                                    bool force_notify, bool is_default) {
    setValue(key, RfxVariant(value), force_notify, is_default);
}

inline
int RfxStatusManager::getIntValue(const RfxStatusKeyEnum key,
    int default_value) const {
    return getValue(key, RfxVariant(default_value)).asInt();
}

inline
int RfxStatusManager::getIntValue(const RfxStatusKeyEnum key) const {
    return getValue(key).asInt();
}


inline
void RfxStatusManager::setIntValue(const RfxStatusKeyEnum key, int value,
                                   bool force_notify, bool is_default) {
    setValue(key, RfxVariant(value), force_notify, is_default);
}

inline
float RfxStatusManager::getFloatValue(const RfxStatusKeyEnum key,
    float default_value) const {
    return getValue(key, RfxVariant(default_value)).asFloat();
}

inline
float RfxStatusManager::getFloatValue(const RfxStatusKeyEnum key) const {
    return getValue(key).asFloat();
}


inline
void RfxStatusManager::setFloatValue(const RfxStatusKeyEnum key, float value,
                                     bool force_notify, bool is_default) {
    setValue(key, RfxVariant(value), force_notify, is_default);
}

inline
const String8& RfxStatusManager::getString8Value(const RfxStatusKeyEnum key,
        const String8& default_value) const {
    return getValue(key, RfxVariant(default_value)).asString8();
}

inline
const String8& RfxStatusManager::getString8Value(const RfxStatusKeyEnum key) const {
    return getValue(key).asString8();
}

inline
void RfxStatusManager::setString8Value(const RfxStatusKeyEnum key, const String8& value,
                                       bool force_notify, bool is_default) {
    setValue(key, RfxVariant(value), force_notify, is_default);
}

inline
const RfxNwServiceState& RfxStatusManager::getServiceStateValue(const RfxStatusKeyEnum key,
        const RfxNwServiceState& default_value) const {
    return getValue(key, RfxVariant(default_value)).asServiceState();
}

inline
const RfxNwServiceState& RfxStatusManager::getServiceStateValue(const RfxStatusKeyEnum key) const {
    return getValue(key).asServiceState();
}

inline
void RfxStatusManager::setServiceStateValue(const RfxStatusKeyEnum key,
    const RfxNwServiceState& value, bool force_notify, bool is_default) {
    setValue(key, RfxVariant(value), force_notify, is_default);
}

inline
const RfxSocketState& RfxStatusManager::getSocketStateValue(const RfxStatusKeyEnum key,
        const RfxSocketState& default_value) const {
    return getValue(key, RfxVariant(default_value)).asSocketState();
}

inline
const RfxSocketState& RfxStatusManager::getSocketStateValue(const RfxStatusKeyEnum key) const {
    return getValue(key).asSocketState();
}

inline
void RfxStatusManager::setSocketStateValue(const RfxStatusKeyEnum key,
    const RfxSocketState& value, bool force_notify, bool is_default) {
    setValue(key, RfxVariant(value), force_notify, is_default);
}

inline
const RfxCallState& RfxStatusManager::getCallStateValue(const RfxStatusKeyEnum key,
        const RfxCallState& default_value) const {
    return getValue(key, RfxVariant(default_value)).asCallState();
}

inline
const RfxCallState& RfxStatusManager::getCallStateValue(const RfxStatusKeyEnum key) const {
    return getValue(key).asCallState();
}

inline
void RfxStatusManager::setCallStateValue(const RfxStatusKeyEnum key,
    const RfxCallState& value, bool force_notify, bool is_default) {
    setValue(key, RfxVariant(value), force_notify, is_default);
}

inline
const RfxDataSettings& RfxStatusManager::getDataSettingsValue(const RfxStatusKeyEnum key,
        const RfxDataSettings& default_value) const {
    return getValue(key, RfxVariant(default_value)).asDataSettings();
}

inline
const RfxDataSettings& RfxStatusManager::getDataSettingsValue(const RfxStatusKeyEnum key) const {
    return getValue(key).asDataSettings();
}

inline
void RfxStatusManager::setDataSettingsValue(const RfxStatusKeyEnum key,
    const RfxDataSettings& value, bool force_notify, bool is_default) {
    setValue(key, RfxVariant(value), force_notify, is_default);
}

#endif /* __RFX_STATUS_MANAGER_H__ */


