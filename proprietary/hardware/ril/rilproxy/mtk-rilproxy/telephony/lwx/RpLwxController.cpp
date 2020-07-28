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
#include "RpLwxController.h"
#include "RfxRootController.h"

#ifdef MTK_NVRAM_SUPPORT
#include <libnvram.h>
#include <Custom_NvRam_LID.h>
#endif

#include <cutils/properties.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef RFX_LOG_TAG
#undef RFX_LOG_TAG
#endif
#define RFX_LOG_TAG "RpLwxController"

#define WIFI_MAC_BYTES          6
#define NARAM_5G_BAND_SUPPORT   0xc5
#define NARAM_5G_BAND_ENABLE    0x106

#define UE_CAPABILITY_LWA      1
#define UE_CAPABILITY_LWI      2
#define UE_CAPABILITY_RCLWI    4
#define UE_CAPABILITY_LWIP     8

#define UE_SUPPORT_LWA      "ro.vendor.mtk_lwa_support"
#define UE_SUPPORT_LWI      "ro.vendor.mtk_lwi_support"
#define UE_SUPPORT_RCLWI    "ro.vendor.mtk_rclwi_support"
#define UE_SUPPORT_LWIP     "ro.vendor.mtk_lwip_support"

/*****************************************************************************
 * Class RfxController
 *****************************************************************************/

RFX_IMPLEMENT_CLASS("RpLwxController", RpLwxController, RfxController);

RpLwxController::RpLwxController() : mLwxCapability(0), mWifiMacAddress(""),
                                    mIsWifi5gSupport(0) {}

RpLwxController::~RpLwxController() {
    logD(RFX_LOG_TAG, "~RpLwxController X!");
}

void RpLwxController::onInit() {
    RfxController::onInit();  // Required: invoke super class implementation

    initLwxSupport();

    if (mLwxCapability != 0) {
        getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_RADIO_STATE,
            RfxStatusChangeCallback(this, &RpLwxController::onRadioStateChanged));
    }
}

void RpLwxController::initLwxSupport() {
    char property_value[PROPERTY_VALUE_MAX] = {0};
    int ue_capability = 0;

    property_get(UE_SUPPORT_LWA, property_value, "0");
    ue_capability = atoi(property_value);
    mLwxCapability |= ue_capability;
    property_get(UE_SUPPORT_LWI, property_value, "0");
    ue_capability = atoi(property_value);
    mLwxCapability |= ue_capability;
    property_get(UE_SUPPORT_RCLWI, property_value, "0");
    ue_capability = atoi(property_value);
    mLwxCapability |= ue_capability;
    property_get(UE_SUPPORT_LWIP, property_value, "0");
    ue_capability = atoi(property_value);
    mLwxCapability |= ue_capability;

    logD(RFX_LOG_TAG, "initLwxSupport:%d", mLwxCapability);
}

bool RpLwxController::initWifiInfo() {
#ifdef MTK_NVRAM_SUPPORT
    ALOGV("initWifiInfo\n");
    F_ID fd;
    bool IsRead = 1;
    int pRecSize =0, pRecNum = 0, wifi5gBandSupported = 0, size = 0;
    char *buff = NULL;
    char wifiMac[WIFI_MAC_BYTES] = { 0 };

    fd = NVM_GetFileDesc(AP_CFG_RDEB_FILE_WIFI_LID, &pRecSize, &pRecNum, IsRead);
    if (fd.iFileDesc == -1) {
        ALOGV("open file Error!\n");
        return false;
    }

    size = pRecSize * pRecNum;
    buff = (char *)malloc(size);

    if (buff == NULL) {
        ALOGV("malloc return NULL!\n");
        NVM_CloseFileDesc(fd);
        return false;
    }

    if (size == read(fd.iFileDesc, buff, size)) {
        logD(RFX_LOG_TAG, "Wi-Fi MAC:%02x%02x%02x%02x%02x%02x",
                            buff[4], buff[5], buff[6], buff[7], buff[8], buff[9]);
        int i = 0;
        for (; i < WIFI_MAC_BYTES - 1; i++) {
                mWifiMacAddress.appendFormat("%02x:", buff[4 + i]);
        }
        mWifiMacAddress.appendFormat("%02x", buff[4 + i]);

        wifi5gBandSupported = buff[NARAM_5G_BAND_SUPPORT] & buff[NARAM_5G_BAND_ENABLE];
        logD(RFX_LOG_TAG, "5G:%d:%d:%d",
            wifi5gBandSupported, buff[NARAM_5G_BAND_SUPPORT], buff[NARAM_5G_BAND_ENABLE]);
        mIsWifi5gSupport = (wifi5gBandSupported != 0) ? 1 : 0;
    } else {
        logE(RFX_LOG_TAG, "Can't read Wi-Fi address");
    }

    free(buff);
    if(!NVM_CloseFileDesc(fd)) {
        ALOGV("close File error!\n");
        return false;
    }
    return true;
#endif
    return false;
}

void RpLwxController::onRadioStateChanged(RfxStatusKeyEnum key,
    RfxVariant old_value, RfxVariant value) {

    RFX_UNUSED(key);
    RFX_UNUSED(old_value);

    RIL_RadioState radioState = (RIL_RadioState) value.asInt();

    if (radioState == RADIO_STATE_OFF) {
        if (initWifiInfo()) {
            sp<RfxMessage> msg = RfxMessage::obtainRequest(getSlotId(),
                        RADIO_TECH_GROUP_GSM, RIL_REQUEST_OEM_HOOK_STRINGS);
            String8 atCmdString;
            atCmdString = "AT+ELWX=";
            atCmdString.appendFormat("%d,\"%s\",%d\n",
                            mLwxCapability, mWifiMacAddress.string(), mIsWifi5gSupport);
            String16 atCmdString2 = String16(atCmdString);
            Parcel* newParcel = msg->getParcel();
            newParcel->writeInt32(2);  // 2 parameters
            newParcel->writeString16(atCmdString2);  // AT command
            newParcel->writeString16(String16(""));  // End of string
            requestToRild(msg);
        }
    }
}
