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

#include "RadioConstants.h"
#include "RfxController.h"
#include "RpRadioController.h"
#include "RpCdmaRadioController.h"

/*****************************************************************************
 * Class RfxController
 *****************************************************************************/
#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "RpCdmaRadioCtrl"
#endif
#define ARRAY_LENGTH(array) (sizeof(array)/sizeof(array[0]))

static const int RADIO_POWER_OFF = 0;
static const int RADIO_POWER_ON = 1;

RFX_IMPLEMENT_CLASS("RpCdmaRadioController", RpCdmaRadioController, RfxController);

//TODO get somehing to it
static RpCdmaRadioController* controllerInstance = NULL;

RpCdmaRadioController::RpCdmaRadioController() {
}

RpCdmaRadioController::~RpCdmaRadioController() {
    delete(cdmaPower);
}

void RpCdmaRadioController::onInit() {
    if (controllerInstance == NULL) {
        //super.init
        RLOGD("First Init");
        RfxController::onInit();
        cdmaPower = new int[MAX_SIM_COUNT];
        for (int i = 0; i < SIM_COUNT; i++) {
            cdmaPower[i] = RADIO_POWER_OFF;
        }
        registerForStatusChange();
        controllerInstance = this;
    } else {
        RLOGD("Already Init, don't init again");
    }
}

void RpCdmaRadioController::registerForStatusChange() {
    getNonSlotScopeStatusManager()->registerStatusChanged(RFX_STATUS_KEY_CDMA_SOCKET_SLOT,
        RfxStatusChangeCallback(this, &RpCdmaRadioController::onCdmaSocketSlotChange));
}

RpCdmaRadioController* RpCdmaRadioController::getInstance() {
    if (controllerInstance != NULL) {
        return controllerInstance;
    } else {
        // TODO: should initialize
        return controllerInstance;
    }
}

void RpCdmaRadioController::registerForCdmaCapability(int slotId, bool slotPower) {
    cdmaPower[slotId] = ((slotPower) ? RADIO_POWER_ON : RADIO_POWER_OFF);
    RLOGD("registerForCdmaCapability slot Id %d is %d", slotId, cdmaPower[slotId]);
}

bool RpCdmaRadioController::adjustCdmaRadioPower() {
    bool shouldAdjustPower =  false;
    for (int i = 0; i < SIM_COUNT; i++) {
        if (cdmaPower[i] == RADIO_POWER_ON) {
            shouldAdjustPower = true;
        }
    }
    RLOGD("adjustCdmaRadioPower: %s", (shouldAdjustPower ? "true" : "false"));
    return shouldAdjustPower;
}

bool RpCdmaRadioController::needHandleCdma(int slotId, bool suggestPower) {
    RLOGD("needHandleCdma: slotId=%d suggestPower=%d", slotId, suggestPower);

    RfxSocketState socketState =
        getStatusManager(slotId)->getSocketStateValue(RFX_STATUS_KEY_SOCKET_STATE);
    if (!socketState.getIsCdmaSlot()) {
        RLOGD("needHandleCdma: retuen false because not cdma slot");
        return false;
    }

    bool oldStatus = adjustCdmaRadioPower();
    bool newStatus = false;
    // Just cauculate newStatus without update cdmaPower[]
    for (int i = 0; i < SIM_COUNT; i++) {
        RLOGD("slotId[%d] cdmaPower is %d", i, cdmaPower[i]);
        if (i == slotId) {
            if (suggestPower == RADIO_POWER_ON) {
                newStatus = true;
                break;
            }
        } else {
            if (cdmaPower[i] == RADIO_POWER_ON) {
                newStatus = true;
                break;
            }
        }
    }

    int capability = RIL_CAPABILITY_NONE;
    bool currentCapability = false;
    for (int i = 0; i < SIM_COUNT; i++) {
        capability = getStatusManager(i)->getIntValue(RFX_STATUS_KEY_RADIO_CAPABILITY, RIL_CAPABILITY_NONE);
        RLOGD("slotId[%d] currentCapability is %d", i, capability);
        if (CAPABILITY_HAS_CDMA(capability)) {
            currentCapability = true;
            break;
        }
    }

    RLOGD("needHandleCdma: cdma oldStatus=%d, newStatus=%d, currentCapability=%d",
        oldStatus, newStatus, currentCapability);
    bool needHandleCdma = ((newStatus != oldStatus) || (newStatus != currentCapability));
    RLOGD("needHandleCdma: %d", needHandleCdma);
    return needHandleCdma;
}

void RpCdmaRadioController::handleRadioStateChanged(int32_t radioState) {
    for (int i = 0; i < SIM_COUNT; i++) {
        int oldCapability = getStatusManager(i)->getIntValue(RFX_STATUS_KEY_RADIO_CAPABILITY, RIL_CAPABILITY_NONE);
        if (CAPABILITY_HAS_CDMA(oldCapability) && radioState == RADIO_STATE_OFF) {
            int newCapability = oldCapability - RIL_CAPABILITY_CDMA_ONLY;
            RLOGD("slot %d change capability from %d to %d", i, oldCapability, newCapability);
            getStatusManager(i)->setIntValue(RFX_STATUS_KEY_RADIO_CAPABILITY, newCapability);
        } else if (!CAPABILITY_HAS_CDMA(oldCapability) && radioState == RADIO_STATE_ON && isCdmaSlot(i)) {
            int newCapability = oldCapability + RIL_CAPABILITY_CDMA_ONLY;
            RLOGD("slot %d change capability from %d to %d", i, oldCapability, newCapability);
            getStatusManager(i)->setIntValue(RFX_STATUS_KEY_RADIO_CAPABILITY, newCapability);
        } else {
            RLOGD("Do nothing for slot %d", i);
        }
    }
}

bool RpCdmaRadioController::isCdmaSlot(int slotId) {
    RfxSocketState socketState = getStatusManager(slotId)->getSocketStateValue(RFX_STATUS_KEY_SOCKET_STATE);
    return socketState.getIsCdmaSlot();
}

void RpCdmaRadioController::onCdmaSocketSlotChange(RfxStatusKeyEnum key,
    RfxVariant old_value, RfxVariant value) {
    RFX_UNUSED(key);
    int newCdmaSlot = value.asInt();
    int oldCdmaSlot = old_value.asInt();
    RLOGD("onCdmaSocketSlotChange, oldCdmaSlot:%d, newCdmaSlot:%d", oldCdmaSlot, newCdmaSlot);
    if (newCdmaSlot != oldCdmaSlot) {
        int oldSlotCapability = getStatusManager(oldCdmaSlot)->
            getIntValue(RFX_STATUS_KEY_RADIO_CAPABILITY, RIL_CAPABILITY_NONE);
        int newSlotCapability = getStatusManager(newCdmaSlot)->
            getIntValue(RFX_STATUS_KEY_RADIO_CAPABILITY, RIL_CAPABILITY_NONE);
        if (CAPABILITY_HAS_CDMA(oldSlotCapability)) {
            RLOGD("old slot %d has cdma, exchange capability", oldCdmaSlot);
            getStatusManager(oldCdmaSlot)->
                setIntValue(RFX_STATUS_KEY_RADIO_CAPABILITY, oldSlotCapability - RIL_CAPABILITY_CDMA_ONLY);
            getStatusManager(newCdmaSlot)->
                setIntValue(RFX_STATUS_KEY_RADIO_CAPABILITY, newSlotCapability + RIL_CAPABILITY_CDMA_ONLY);
        } else {
            RLOGD("cdma slot changed, but no slot has cdma capability");
        }
    }
}


