/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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

#ifndef __RTC_DATA_UTILS_H__
#define __RTC_DATA_UTILS_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "rfx_properties.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OPERATOR_OP12 "OP12"
#define SUPPORT_TEMPORARY_DISABLE_IMS 0

/*****************************************************************************
 * Class RtcDataUtils
 *****************************************************************************/

class RtcDataUtils {
public:
    RtcDataUtils() {}
    virtual ~RtcDataUtils() {}
    static bool isOp12Support();
    static bool isSupportTemporaryDisableIms();
    static bool isSupportTempDataSwitchFromOem();
    static bool isPreferredDataMode();
};

inline bool RtcDataUtils::isOp12Support() {
    bool ret = false;
    char optr_value[RFX_PROPERTY_VALUE_MAX] = {0};
    rfx_property_get("persist.vendor.operator.optr", optr_value, "0");
    if (strcmp(optr_value, OPERATOR_OP12) == 0) {
        ret = true;
    }
    return ret;
}

inline bool RtcDataUtils::isSupportTemporaryDisableIms() {
    bool ret = false;
    if (SUPPORT_TEMPORARY_DISABLE_IMS) {
        ret = true;
    }
    return ret;
}

// Customized data incall: Switch default data to voice call sim
// It will be enable ONLY for platform which support this feature by:
// 1. Java framework will switch default data to voice call sim through setDefaultData
// 2. When call start, it will send allow data to voice call sim without disallow original data SIM
// 3. When call end, it will send disallow data to voice call sim without allow data on original
//    data SIM
// So RILD need to handle this scenario specially:
// 1. Support error handling for allow data on both slot (disallow for one slot then allow
//    data on another).
// 2. Check call state and set temp data switch state for special handling.
// 3. Support restore data allow state and data connection when call end.
inline bool RtcDataUtils::isSupportTempDataSwitchFromOem() {
    char support_value[RFX_PROPERTY_VALUE_MAX] = {0};
    rfx_property_get("persist.vendor.radio.data_incall_ext",
            support_value, "0");
    int value = atoi(support_value);
    return value == 1;
}

inline bool RtcDataUtils::isPreferredDataMode() {
    char preferredDataMode[RFX_PROPERTY_VALUE_MAX] = {0};
    rfx_property_get("vendor.ril.data.preferred_data_mode", preferredDataMode, "0");
    return atoi(preferredDataMode) == 1 ? true : false;
}

#endif /* __RTC_DATA_UTILS_H__ */
