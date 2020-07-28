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

#ifndef __RP_IR_CONTROLLER_H__
#define __RP_IR_CONTROLLER_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RfxAction.h"
#include "RfxController.h"
#include "RfxTimer.h"
#include <telephony/mtk_ril.h>
#include "RpNwDefs.h"
#include "RpNwRatController.h"
#include "IRpNwRatSwitchCallback.h"
#include "RpIrUtils.h"
#include "cutils/properties.h"
#include <string>

using namespace std;

using ::android::Vector;

/*****************************************************************************
 * Class RpIrController
 *****************************************************************************/
class RpIrLwgHandler;
class RpIrCdmaHandler;
class RpIrStrategy;
class RpIrNwRatSwitchCallback;
class RpIrController : public RfxController{
    RFX_DECLARE_CLASS(RpIrController);

public:
    static int sNoServiceDelayTime;
    RpIrStrategy* mPtrSwitchStrategy;

private:
    RpIrLwgHandler* mPtrLwgHandler;
    RpIrCdmaHandler* mPtrCdmaHandler;

    bool mIsEnabled;
    bool mIsGmssEnable;
    bool mIsSwitchingTo3GMode;
    bool mIsIrControllerState;
    bool mIsVolteStateChanged;

    VolteState mVolteState;
    NwsMode mNwsMode;
    GmssInfo mGmssInfo;

    RpIrNwRatSwitchCallback *mNwRatSwitchCallback;
    RpNwRatController* mNwRatController;

    int mLastValidCdmaCardType;
    String8 mLastValidImsi;

public:
    RpIrController();
    virtual ~RpIrController();

    bool setIrEnableState(bool isEnabled, bool is3GPrefNwType);
    void setIrControllerEnableState(bool isEnabled);
    bool getIrControllerEnableState();
    void set3GIrEnableState(bool isEnabled);
    bool isCtSimCard();
    bool needEnable3GIrForSpecialCase();
    void setNwsMode(NwsMode nwsMode, const sp<RfxAction>& action);
    NwsMode getNwsMode();
    void setVolteState(VolteState volteState) {
        mVolteState = volteState;
    }
    VolteState getVolteState() {
        return mVolteState;
    }
    void setVolteStateChanged(bool isVolteStateChanged) {
        mIsVolteStateChanged = isVolteStateChanged;
    }
    bool getVolteStateChanged() {
        return mIsVolteStateChanged;
    }
    bool needWaitImsi();
    bool isWwop();
    void resetSuspendNetwork();

    // Handle URC for GMSS RAT changed
    void setGmssEnableState(bool isEnabled);
    bool getGmssEnableState();
    void onNwsModeChange(NwsMode curMode);
    void setIsSwitchingTo3GMode(bool isSwitchingTo3GMode) {
        mIsSwitchingTo3GMode = isSwitchingTo3GMode;
    }
    String8 getLastValidImsi() {
        return mLastValidImsi;
    }
    int getLastValidCdmaCardType() {
        return mLastValidCdmaCardType;
    }
    void setIrControllerState(bool isIrControllerState) {
        mIsIrControllerState = isIrControllerState;
    }
    bool getIrControllerState() {
        return mIsIrControllerState;
    }
    RpNwRatController* getNwRatController() {
        return mNwRatController;
    }

protected:
    virtual bool onHandleUrc(const sp<RfxMessage>& message);
    virtual void onInit();

private:
    void dispose();

    // Handle URC for GMSS RAT changed
    void processGmssRatChanged(const sp<RfxMessage>& msg);
    void onGmssRatChanged(int mcc);
    void onCdmaCardTypeChanged(RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant value);
    void onImsiChanged(RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant value);
    void onImsiChangedForGmss(RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant value);
    void onCdmaSocketSlotChanged(RfxStatusKeyEnum key, RfxVariant oldValue, RfxVariant newValue);
    void onSimStateChanged(RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant value);
    NwsMode getNwsModeByMcc(int mcc);
    NwsMode getNwsModeByEccRetry();
};

#endif /* __RP_IR_CONTROLLER_H__ */

