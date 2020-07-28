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

/*****************************************************************************
 * Include
 *****************************************************************************/
#include <string.h>
#include "rfx_properties.h"
#include "RmcCapabilitySwitchUtil.h"
#include "RfxLog.h"
#include "RfxRilUtils.h"
#include "RfxIdToStringUtils.h"
#include "RfxMclMessage.h"
#include "RfxMclStatusManager.h"

#define RFX_LOG_TAG "RmcCapa"
/*****************************************************************************
 * Class RmcCapabilitySwitchUtil
 *****************************************************************************/

int RmcCapabilitySwitchUtil::getMaxRadioGeneration(int radio_capability) {
    RFX_LOG_D(RFX_LOG_TAG, "getMaxRadioGeneration, capability=%d", radio_capability);
    if (radio_capability & RAF_LTE) {
        return RADIO_GENERATION_4G;
    } else if (radio_capability & (RAF_WCDMA_GROUP | RAF_EVDO_GROUP)) {
        return RADIO_GENERATION_3G;
    } else {
        return RADIO_GENERATION_2G;
    }
}

int RmcCapabilitySwitchUtil::getMajorSim() {
    return RfxRilUtils::getMajorSim();
}

int RmcCapabilitySwitchUtil::isDisableCapabilitySwitch() {
    char tmp[RFX_PROPERTY_VALUE_MAX] = { 0 };
    rfx_property_get("ro.vendor.mtk_disable_cap_switch", tmp, "0");
    return atoi(tmp);
}

int RmcCapabilitySwitchUtil::getAdjustedRaf(int raf) {
    raf = ((RAF_GSM_GROUP & raf) > 0) ? (RAF_GSM_GROUP | raf) : raf;
    raf = ((RAF_WCDMA_GROUP & raf) > 0) ? (RAF_WCDMA_GROUP | raf) : raf;
    raf = ((RAF_CDMA_GROUP & raf) > 0) ? (RAF_CDMA_GROUP | raf) : raf;
    raf = ((RAF_EVDO_GROUP & raf) > 0) ? (RAF_EVDO_GROUP | raf) : raf;

    return raf;
}

bool RmcCapabilitySwitchUtil::isMessageBeforeCapabilitySwitch(const sp<RfxMclMessage> &msg) {
    if (RfxMclStatusManager::getNonSlotMclStatusManager()->getIntValue(
            RFX_STATUS_KEY_MAIN_CAPABILITY_SLOT, 0) == msg->getMainProtocolSlotId()) {
        RFX_LOG_I(RFX_LOG_TAG, "%s isn't msg before capability switch", RFX_ID_TO_STR(msg->getId()));
        return false;
    }
    RFX_LOG_I(RFX_LOG_TAG, "%s is msg before capability switch", RFX_ID_TO_STR(msg->getId()));
    return true;
}

bool RmcCapabilitySwitchUtil::isDssNoResetSupport() {
    char tmp[RFX_PROPERTY_VALUE_MAX] = { 0 };
    rfx_property_get("vendor.ril.simswitch.no_reset_support", tmp, "0");
    if (strcmp(tmp, "1") == 0) {
        return true;
    }
    return false;
}

bool RmcCapabilitySwitchUtil::isDisableC2kCapability() {
    char tmp[RFX_PROPERTY_VALUE_MAX] = { 0 };
    rfx_property_get("persist.vendor.radio.disable_c2k_cap", tmp, "0");
    if (strcmp(tmp, "1") == 0) {
        RFX_LOG_I(RFX_LOG_TAG, "isDisableC2kCapability == true");
        return true;
    }
    return false;
}

