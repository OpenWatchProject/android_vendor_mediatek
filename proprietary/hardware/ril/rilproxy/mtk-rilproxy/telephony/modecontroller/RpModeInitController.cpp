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
#include "RfxLog.h"
#include "RpCdmaLteModeController.h"
#include "RpModeInitController.h"
#include "util/RpFeatureOptionUtils.h"
#include <cutils/properties.h>


/*****************************************************************************
 * Class RpModeInitController
 *****************************************************************************/

RFX_IMPLEMENT_CLASS("RpModeInitController", RpModeInitController, RfxController);

void RpModeInitController::onInit() {
    RfxController::onInit();
    int cdmaLteModeSlotId = RpCdmaLteModeController::getActiveCdmaLteModeSlotId();
    int cdmaSocketSlotId = RpCdmaLteModeController::getCdmaSocketSlotId();
    ///M: Add for DSDS->SS, correct the c slot value for ss.
    if (RpFeatureOptionUtils::getSimCount() == 1 && cdmaSocketSlotId > CDMALTE_ON_SLOT_0) {
        RFX_LOG_D(RP_CDMALTE_MODE_TAG, "correct c Slot: %d -> 0", cdmaSocketSlotId);
        if (cdmaLteModeSlotId > CDMALTE_ON_SLOT_0) {
            property_set("persist.vendor.radio.svlte_slot", "2,2");
            cdmaLteModeSlotId = CSFB_ON_SLOT;
        }
        RpCdmaLteModeController::setCdmaSocketSlotId(CDMALTE_ON_SLOT_0);
        cdmaSocketSlotId = CDMALTE_ON_SLOT_0;
    }
    if (cdmaLteModeSlotId > -1 && cdmaLteModeSlotId != cdmaSocketSlotId) {
        RFX_LOG_E(RP_CDMALTE_MODE_TAG,"[SMC][onInit] SystemProperties not sync here, revise!");
        RpCdmaLteModeController::setCdmaSocketSlotId(cdmaLteModeSlotId);
        cdmaSocketSlotId = cdmaLteModeSlotId;
    }
    RFX_LOG_D(RP_CDMALTE_MODE_TAG, "[SMC][onInit] cdmaLteModeSlotId = %d, cdmaSocketSlotId = %d",
            cdmaLteModeSlotId, cdmaSocketSlotId);
    getNonSlotScopeStatusManager()->setIntValue(
            RFX_STATUS_KEY_ACTIVE_CDMALTE_MODE_SLOT, cdmaLteModeSlotId, true);
    getNonSlotScopeStatusManager()->setIntValue(
            RFX_STATUS_KEY_CDMA_SOCKET_SLOT, cdmaSocketSlotId, true);
}
