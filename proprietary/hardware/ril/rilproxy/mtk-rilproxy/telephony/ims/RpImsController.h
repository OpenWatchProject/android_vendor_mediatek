/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2015. All rights reserved.
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

#ifndef __RP_IMS_CONTROLLER_H__
#define __RP_IMS_CONTROLLER_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include <utils/Vector.h>
#include "RfxAction.h"
#include "RfxController.h"
#include "RfxObject.h"
#include "RfxTimer.h"

/*****************************************************************************
 * Class RpImsController
 *****************************************************************************/



class RpImsController : public RfxController {
    RFX_DECLARE_CLASS(RpImsController);  // Required: declare this class

public:
    RpImsController();

    virtual ~RpImsController();

    void requestImsDeregister(int slotId, const sp<RfxAction>& action);


// Override
protected:

    virtual bool onHandleRequest(const sp<RfxMessage>& message);
    virtual bool onHandleUrc(const sp<RfxMessage>& message);
    virtual bool onHandleResponse(const sp<RfxMessage>& message);
    virtual void onInit();
    virtual void onDeinit();

private:
    // Private functions
    void handleImsDeRegRequest(const sp<RfxMessage>& message);
    void handleImsDeRegResponse(const sp<RfxMessage>& message);
    void handleImsEnableStartUrc(const sp<RfxMessage>& message);
    void handleImsRegStateRequest(const sp<RfxMessage>& message);
    void handleImsRegStateResponse(const sp<RfxMessage>& message);
    void handleImsDisableDoneUrc(const sp<RfxMessage>& message);
    void handleImsRegistartionInfoUrc(const sp<RfxMessage>& message);
    void handleImsDeRegDoneUrc(const sp<RfxMessage>& message);
    void handleImsCallStatusChange(const sp<RfxMessage>& message);
    void sendImsDeRegRequest();
    void replyImsDeregConfirmAction();
    void onRadioStateChanged(RfxStatusKeyEnum key,
            RfxVariant oldValue, RfxVariant value);
    /// M: add for op09 default volte setting @{
    void initOp09Ims();
    void deinitOp09Ims();
    void sync_mims_capa();
    void onMainCapabilitySlotChanged(
             RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant new_value);
    void onCardTypeChanged(RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant value);
    void onNoIccIdTimeout();
    void setDefaultVolteState(int slot_id, char new_iccid[],int card_type);
    void setVolteStateProperty(int slot_id, bool isEnable);
    bool isOp09SimCard(int slot_id, char icc_id[], int card_type);
    void sendDefaultVolteStateUrc(int slot_id, int value);
    static const char* givePrintableStr(const char* iccId);
    /// @}

private:
    // Private members
    bool mIsInImsCall;
    bool mIsImsRegistered;
    bool mIsImsEnabled;
    /// M: add for op09 default volte setting @{
    TimerHandle mNoIccidTimerHandle;
    int mNoIccidRetryCount;
    int mMainSlotId;
    bool mIsBootUp;
    bool mIsSimSwitch;
    static char sLastBootIccId[4][21];
    static int sLastBootVolteState;
    static bool sInitDone;
    /// @}
    Vector<sp<RfxAction>> mDeregConfirmActionQueue;
};

#endif /* __RP_IMS_CONTROLLER_H__ */

