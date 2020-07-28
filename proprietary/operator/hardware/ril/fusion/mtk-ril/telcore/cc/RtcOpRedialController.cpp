/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
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
#include "RtcOpRedialController.h"
#include "nw/RtcRatSwitchController.h"
#include "rfx_properties.h"


/*****************************************************************************
 * Class RtcOpRedialController
 *****************************************************************************/
RFX_IMPLEMENT_CLASS("RtcOpRedialController", RtcOpRedialController, RtcRedialController);

#define RFX_LOG_TAG "RtcOpRedial"

RtcOpRedialController::RtcOpRedialController() {
}

RtcOpRedialController::~RtcOpRedialController() {
    logD(RFX_LOG_TAG, "RtcOpRedialController");
}

void RtcOpRedialController::onInit() {
    RtcRedialController::onInit();
    logD(RFX_LOG_TAG, "onInit !!");
}

void RtcOpRedialController::onDeinit() {
    logD(RFX_LOG_TAG, "onDeinit");
    RtcRedialController::onDeinit();
}

bool RtcOpRedialController::isEmergencyModeSupported(int slotId) {
    if (RfxRilUtils::isEmciSupport()) {
        return true;
    }

    char optr[RFX_PROPERTY_VALUE_MAX] = { 0 };
    rfx_property_get("persist.vendor.operator.optr", optr, "0");
    // Vzw CDMA-less , AT&T supports ECM
    if (strcmp(optr, "OP12") == 0 || strcmp(optr, "OP07") == 0) {
        return true;
    }

    RtcModeSwitchController* modeSwitchController =
            (RtcModeSwitchController *)findController(RFX_OBJ_CLASS_INFO(RtcModeSwitchController));
    // non-TMO, 5M
    if (strcmp(optr, "OP08") != 0 &&
            modeSwitchController->getCCapabilitySlotId() == slotId) {
        return true;
    }

    logD(RFX_LOG_TAG, "isEmergencyModeSupported = false ");
    return false;
}

