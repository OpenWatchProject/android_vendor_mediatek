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

#include "ImsCustomizedUtils.h"
#include "rfx_properties.h"
#include <string>
#include <string.h>
#include "RfxRilUtils.h"

/** Customized function - start **/
bool ImsCustomizedUtils::isSupportCustomizedUserAgent() {
    // default value: false, change to true if you would like to enable set user agent.
    return false;
}

String8 ImsCustomizedUtils::getCustomizedUserAgent(String8 format, int slotId) {
    // TODO: Make customized User-Agent by FormatId
    // Format value is came from RFX_STATUS_KEY_CUSTOMIZED_USER_AGENT_FORMAT by carrier config mechanism.
    // Please set the preferred format by MCC/MNC.

    // Carrier config file path is as follow:
    // vendor\mediatek\proprietary\hardware\ril\fusion\libcarrierconfig\config_data\carrier_config_mcc_mnc.h
    // For an example, if you would like set China Mobile SIM to using format "1" parsing rule,
    // then you need to add { RFX_STATUS_KEY_CUSTOMIZED_USER_AGENT_FORMAT, "1"}, into
    // vendor\mediatek\proprietary\hardware\ril\fusion\libcarrierconfig\config_data\carrier_config_460_00.h

    // The parsing rule by format could be customized here.
    // This is only an example code.
    String8 userAgent("XXXOEM");
    String8 cust_prop_key1("ro.aaa.bbb");
    String8 cust_prop_key2("ro.build.version.incremental");
    char cust_prop_value1[RFX_PROPERTY_VALUE_MAX] = {0};
    char cust_prop_value2[RFX_PROPERTY_VALUE_MAX] = {0};

    RfxRilUtils::printLog(DEBUG, String8("ImsCustomizedUtils"),
                String8::format("getCustomizedUserAgent(), format = %s", format.string()), slotId);

    rfx_property_get(cust_prop_key1.string(), cust_prop_value1, "");
    RfxRilUtils::printLog(DEBUG, String8("ImsCustomizedUtils"),
                String8::format("getCustomizedUserAgent(), format = %s", format.string()), slotId);

    rfx_property_get(cust_prop_key2.string(), cust_prop_value2, "");
    RfxRilUtils::printLog(DEBUG, String8("ImsCustomizedUtils"),
                String8::format("getCustomizedUserAgent(), [%s]= %s",
                cust_prop_key2.string(), cust_prop_value2), slotId);

    if (String8("1") == format.string()) {
        // AIS
        userAgent.append(String8::format("-%s SIP/2.0", cust_prop_value1));
    } else if (String8("2") == format.string()) {
        // DT & ee-gb && H3G-GB && SFR
        userAgent.append(String8::format("-%s%s SIP/2.0", cust_prop_value1, cust_prop_value2));
    } else if (String8("3") == format.string()) {
        // KDDI
        userAgent.append(String8::format("-%s", cust_prop_value1));
    } else {
        // No matched
        RfxRilUtils::printLog(DEBUG, String8::format("ImsCustomizedUtils"),
                    String8::format("getCustomizedUserAgent(): not match any format"), slotId);
    }

    RfxRilUtils::printLog(DEBUG, String8::format("ImsCustomizedUtils"),
                String8::format("getCustomizedUserAgent(): %s", userAgent.string()), slotId);

    return userAgent;
}
/** Customized function - end **/


