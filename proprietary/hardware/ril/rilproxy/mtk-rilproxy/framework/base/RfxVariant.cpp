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
 * File name:  rfx_variant.cpp
 * Author: Jun Liu (MTK80064)
 * Description:
 *  RfxVariant is a union for the most common types.
 *  It supports these types:
 *  - bool
 *  - int
 *  - float
 *  - String8
 */


/*****************************************************************************
 * Include
 *****************************************************************************/

#include "RfxVariant.h"

/*****************************************************************************
 * Class RfxVariant
 *****************************************************************************/

RfxVariant &RfxVariant::operator = (const RfxVariant &other) {
    if (m_type == other.m_type) {
        assignWith(other);
    }
    else {
        deinit();
        initWith(other);
    }

    return *this;
}

bool RfxVariant::operator == (const RfxVariant &other) {
    return isEqualWith(other);
}

bool RfxVariant::operator != (const RfxVariant &other) {
    return !isEqualWith(other);
}

bool RfxVariant::isEqualWith(const RfxVariant &other) {
    if (m_type == other.m_type) {
        bool ret = false;
        switch (m_type)
        {
        case DATA_TYPE_NULL:
            ret = true;
            break;

        case DATA_TYPE_BOOL:
            ret = (m_value.boolValue == other.asBool());
            break;

        case DATA_TYPE_INT:
            ret = (m_value.intValue == other.asInt());
            break;

        case DATA_TYPE_FLOAT:
            ret = (m_value.floatValue == other.asFloat());
            break;

        case DATA_TYPE_STRING8:
            ret = ((*(String8*)(m_value.string8Buffer)) == other.asString8());
            break;

        case DATA_TYPE_SERVICE_STATE:
            ret = ((*(RfxNwServiceState*)(m_value.serviceStateBuffer)) == other.asServiceState());
            break;

        case DATA_TYPE_SOCKET_STATE:
            ret = ((*(RfxSocketState*)(m_value.socketStateBuffer)) == other.asSocketState());
            break;

        case DATA_TYPE_CALL_STATE:
            ret = ((*(RfxCallState*)(m_value.callStateBuffer)) == other.asCallState());
            break;

        case DATA_TYPE_DATA_SETTINGS:
            ret = ((*(RfxDataSettings*)(m_value.dataSettingsBuffer)) == other.asDataSettings());
            break;

        default:
            RFX_ASSERT(0);
        }
        return ret;
    } else {
        return false;
    }
}


void RfxVariant::initWith(const RfxVariant &other) {
    RFX_ASSERT(m_type == DATA_TYPE_NULL);

    // Assign new type
    m_type = other.m_type;

    switch (other.m_type) {
    case DATA_TYPE_NULL:
        // Do nothing
        break;

    case DATA_TYPE_BOOL:
        m_value.boolValue = other.asBool();
        break;

    case DATA_TYPE_INT:
        m_value.intValue = other.asInt();
        break;

    case DATA_TYPE_FLOAT:
        m_value.floatValue = other.asFloat();
        break;

    case DATA_TYPE_STRING8:
        new (m_value.string8Buffer) String8(other.asString8());
        break;

    case DATA_TYPE_SERVICE_STATE:
        new (m_value.serviceStateBuffer) RfxNwServiceState(other.asServiceState());
        break;

    case DATA_TYPE_SOCKET_STATE:
        new (m_value.socketStateBuffer) RfxSocketState(other.asSocketState());
        break;

    case DATA_TYPE_CALL_STATE:
        new (m_value.callStateBuffer) RfxCallState(other.asCallState());
        break;

    case DATA_TYPE_DATA_SETTINGS:
        new (m_value.dataSettingsBuffer) RfxDataSettings(other.asDataSettings());
        break;

    default:
        RFX_ASSERT(0);
    }
}


void RfxVariant::deinit() {
    // Call the destructor
    switch (m_type) {
    case DATA_TYPE_NULL:
    case DATA_TYPE_BOOL:
    case DATA_TYPE_INT:
    case DATA_TYPE_FLOAT:
        // no destructor for primitive types, do nothing
        break;

    case DATA_TYPE_STRING8:
        ((String8 *)m_value.string8Buffer)->~String8();
        break;

    case DATA_TYPE_SERVICE_STATE:
        ((RfxNwServiceState *)m_value.serviceStateBuffer)->~RfxNwServiceState();
        break;

    case DATA_TYPE_SOCKET_STATE:
        ((RfxSocketState *)m_value.socketStateBuffer)->~RfxSocketState();
        break;

    case DATA_TYPE_CALL_STATE:
        ((RfxCallState *)m_value.callStateBuffer)->~RfxCallState();
        break;

    case DATA_TYPE_DATA_SETTINGS:
        ((RfxDataSettings *)m_value.dataSettingsBuffer)->~RfxDataSettings();
        break;

    default:
        RFX_ASSERT(0);
    }

    m_type = DATA_TYPE_NULL;
}


void RfxVariant::assignWith(const RfxVariant &other) {
    RFX_ASSERT(m_type == other.m_type);

    switch (m_type)
    {
    case DATA_TYPE_NULL:
        // do nothing
        break;

    case DATA_TYPE_BOOL:
        m_value.boolValue = other.m_value.boolValue;
        break;

    case DATA_TYPE_INT:
        m_value.intValue = other.m_value.intValue;
        break;

    case DATA_TYPE_FLOAT:
        m_value.floatValue = other.m_value.floatValue;
        break;

    case DATA_TYPE_STRING8:
        *(String8 *)m_value.string8Buffer = *(String8 *)other.m_value.string8Buffer;
        break;

    case DATA_TYPE_SERVICE_STATE:
        *(RfxNwServiceState *)m_value.serviceStateBuffer = *(RfxNwServiceState *)other.m_value.serviceStateBuffer;
        break;

    case DATA_TYPE_SOCKET_STATE:
        *(RfxSocketState *)m_value.socketStateBuffer = *(RfxSocketState *)other.m_value.socketStateBuffer;
        break;

    case DATA_TYPE_CALL_STATE:
        *(RfxCallState *)m_value.callStateBuffer = *(RfxCallState *)other.m_value.callStateBuffer;
        break;

    case DATA_TYPE_DATA_SETTINGS:
        *(RfxDataSettings *)m_value.dataSettingsBuffer = *(RfxDataSettings *)other.m_value.dataSettingsBuffer;
        break;

    default:
        RFX_ASSERT(0);
    }
}

String8 RfxVariant::toString() const {
    switch (m_type)
    {
    case DATA_TYPE_NULL:
        // do nothing
        break;

    case DATA_TYPE_BOOL:
        return String8::format("%s", m_value.boolValue ? "true" : "false");
        break;

    case DATA_TYPE_INT:
        return String8::format("%d", m_value.intValue);
        break;

    case DATA_TYPE_FLOAT:
        return String8::format("%.2f", m_value.floatValue);
        break;

    case DATA_TYPE_STRING8:
        return *(String8 *)m_value.string8Buffer;
        break;

    case DATA_TYPE_SERVICE_STATE:
        return ((RfxNwServiceState *)m_value.serviceStateBuffer)->toString();
        break;

    case DATA_TYPE_SOCKET_STATE:
        return ((RfxSocketState *)m_value.socketStateBuffer)->toString();
        break;

    case DATA_TYPE_CALL_STATE:
        return ((RfxCallState *)m_value.callStateBuffer)->toString();
        break;

    case DATA_TYPE_DATA_SETTINGS:
        return ((RfxDataSettings *)m_value.dataSettingsBuffer)->toString();
        break;

    default:
        RFX_ASSERT(0);
    }
    return String8("");
}

