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
 * File name:  RfxVariant.h
 * Author: Jun Liu (MTK80064)
 * Description:
 *  RfxVariant is a union for the most common types.
 *  It supports these types:
 *  - bool
 *  - int
 *  - float
 *  - String8
 */

#ifndef __RFX_VARIANT_H__
#define __RFX_VARIANT_H__

/*****************************************************************************
 * Include
 *****************************************************************************/

#include "utils/String8.h"
#include "RfxBasics.h"
#include "RfxNwServiceState.h"
#include "RfxSocketState.h"
#include "RfxCallState.h"
#include "RfxDataSettings.h"

using ::android::String8;

/*****************************************************************************
 * Class RfxVariant
 *****************************************************************************/

/*
 * RfxVariant is a union for the common types
 */
class RfxVariant {
// Define
public:
    // Value type enum
    enum ValueTypeEnum {
        // Null type
        DATA_TYPE_NULL,
        // bool type
        DATA_TYPE_BOOL,
        // int type
        DATA_TYPE_INT,
        // float type
        DATA_TYPE_FLOAT,
        // String type
        DATA_TYPE_STRING8,
        // ServiceState type
        DATA_TYPE_SERVICE_STATE,
        // SocketState type
        DATA_TYPE_SOCKET_STATE,
        // CallState type
        DATA_TYPE_CALL_STATE,
        // Data settings
        DATA_TYPE_DATA_SETTINGS,
        DATA_TYPE_END_OF_ENUM
    };

// Constructor / Destructor
public:
    // Default constructor
    RfxVariant() : m_type(DATA_TYPE_NULL) {
    }

    // Copy constructor
    RfxVariant(const RfxVariant &other) : m_type(DATA_TYPE_NULL) {
        initWith(other);
    }

    // Destructor
    ~RfxVariant() {
        deinit();
    }

    /*
     * construct by value in different types
     */
    explicit RfxVariant(bool value);
    explicit RfxVariant(int value);
    explicit RfxVariant(float value);
    explicit RfxVariant(const String8 &value);
    explicit RfxVariant(const RfxNwServiceState &value);
    explicit RfxVariant(const RfxSocketState &value);
    explicit RfxVariant(const RfxCallState &value);
    explicit RfxVariant(const RfxDataSettings &value);

// Method
public:
    // Return the value type of the variant
    ValueTypeEnum get_type() const {
        return m_type;
    }

    // Get value as bool type in the variant
    bool asBool() const;
    // Get value as int type in the variant
    int asInt() const;
    // Get value as float type in the variant
    float asFloat() const;
    // Get value as String8 type in the variant
    const String8 &asString8() const;

    const RfxNwServiceState &asServiceState() const;

    // Get value as SockeState type in the variant
    const RfxSocketState &asSocketState() const;

    const RfxCallState &asCallState() const;

    const RfxDataSettings &asDataSettings() const;

    String8 toString() const;

// Operator
public:
    // operator =
    RfxVariant &operator = (const RfxVariant &other);

    // operator ==
    bool operator == (const RfxVariant &other);

    // operator !=
    bool operator != (const RfxVariant &other);

private:
    // the type of the value stored in the variant
    ValueTypeEnum m_type;

    // the value stored in the variant
    union
    {
        /*
         * Primitive types
         */
        bool     boolValue;
        int      intValue;
        float    floatValue;

        /*
         * Classes
         */
        uint8_t  string8Buffer[sizeof(String8)];
        uint8_t  serviceStateBuffer[sizeof(RfxNwServiceState)];
        uint8_t  socketStateBuffer[sizeof(RfxSocketState)];
        uint8_t  callStateBuffer[sizeof(RfxCallState)];
        uint8_t  dataSettingsBuffer[sizeof(RfxDataSettings)];
    } m_value;

    void initWith(const RfxVariant &other);
    void deinit();
    void assignWith(const RfxVariant &other);
    bool isEqualWith(const RfxVariant &other);
};

inline
RfxVariant::RfxVariant(bool value) : m_type(DATA_TYPE_BOOL)
{
    m_value.boolValue = value;
}

inline
RfxVariant::RfxVariant(int value) : m_type(DATA_TYPE_INT)
{
    m_value.intValue = value;
}

inline
RfxVariant::RfxVariant(float value) : m_type(DATA_TYPE_FLOAT)
{
    m_value.floatValue = value;
}

inline
RfxVariant::RfxVariant(const String8 &value) : m_type(DATA_TYPE_STRING8)
{
    new (m_value.string8Buffer) String8(value);
}

inline
RfxVariant::RfxVariant(const RfxNwServiceState &value) : m_type(DATA_TYPE_SERVICE_STATE)
{
    new (m_value.serviceStateBuffer) RfxNwServiceState(value);
}

inline
RfxVariant::RfxVariant(const RfxSocketState &value) : m_type(DATA_TYPE_SOCKET_STATE)
{
    new (m_value.socketStateBuffer) RfxSocketState(value);
}

inline
RfxVariant::RfxVariant(const RfxCallState &value) : m_type(DATA_TYPE_CALL_STATE)
{
    new (m_value.callStateBuffer) RfxCallState(value);
}

inline
RfxVariant::RfxVariant(const RfxDataSettings &value) : m_type(DATA_TYPE_DATA_SETTINGS)
{
    new (m_value.dataSettingsBuffer) RfxDataSettings(value);
}

inline
bool RfxVariant::asBool() const
{
    RFX_ASSERT(m_type == DATA_TYPE_BOOL);
    return m_value.boolValue;
}

inline
int RfxVariant::asInt() const
{
    RFX_ASSERT(m_type == DATA_TYPE_INT);
    return m_value.intValue;
}

inline
float RfxVariant::asFloat() const
{
    RFX_ASSERT(m_type == DATA_TYPE_FLOAT);
    return m_value.floatValue;
}

inline
const String8 &RfxVariant::asString8() const
{
    RFX_ASSERT(m_type == DATA_TYPE_STRING8);
    return *(String8 *)m_value.string8Buffer;
}

inline
const RfxNwServiceState &RfxVariant::asServiceState() const
{
    RFX_ASSERT(m_type == DATA_TYPE_SERVICE_STATE);
    return *(RfxNwServiceState *)m_value.serviceStateBuffer;
}

inline
const RfxSocketState &RfxVariant::asSocketState() const
{
    RFX_ASSERT(m_type == DATA_TYPE_SOCKET_STATE);
    return *(RfxSocketState *)m_value.socketStateBuffer;
}

inline
const RfxCallState &RfxVariant::asCallState() const
{
    RFX_ASSERT(m_type == DATA_TYPE_CALL_STATE);
    return *(RfxCallState *)m_value.callStateBuffer;
}

inline
const RfxDataSettings &RfxVariant::asDataSettings() const
{
    RFX_ASSERT(m_type == DATA_TYPE_DATA_SETTINGS);
    return *(RfxDataSettings *)m_value.dataSettingsBuffer;
}


/*****************************************************************************
 * Class RfxVariantUtil
 *****************************************************************************/

/*
 * Internal used. RfxVariantUtil provides the type information.
 */
template <class _value_type>
class RfxVariantUtil
{
public:
    typedef _value_type ret_type;
    typedef _value_type arg_type;
};


template <>
class RfxVariantUtil <bool>
{
public:
    typedef bool ret_type;
    typedef bool arg_type;

    inline
    static bool getVariantValue(const RfxVariant &variant)
    {
        return variant.asBool();
    }
};

template <>
class RfxVariantUtil <int>
{
public:
    typedef int ret_type;
    typedef int arg_type;

    inline
    static int getVariantValue(const RfxVariant &variant)
    {
        return variant.asInt();
    }
};

template <>
class RfxVariantUtil <float>
{
public:
    typedef float ret_type;
    typedef float arg_type;

    inline
    static float getVariantValue(const RfxVariant &variant)
    {
        return variant.asFloat();
    }
};

template <>
class RfxVariantUtil <String8>
{
public:
    typedef const String8 &ret_type;
    typedef const String8 &arg_type;

    inline
    static const String8 &getVariantValue(const RfxVariant &variant)
    {
        return variant.asString8();
    }
};

template <>
class RfxVariantUtil <RfxNwServiceState>
{
public:
    typedef const RfxNwServiceState &ret_type;
    typedef const RfxNwServiceState &arg_type;

    inline
    static const RfxNwServiceState &getVariantValue(const RfxVariant &variant)
    {
        return variant.asServiceState();
    }
};


#endif /* __RFX_VARIANT_H__ */

