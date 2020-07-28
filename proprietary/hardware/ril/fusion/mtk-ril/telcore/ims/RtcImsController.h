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

#ifndef __RFX_IMS_CONTROLLER_H__
#define __RFX_IMS_CONTROLLER_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RfxController.h"
#include "RfxTimer.h"
#include "RfxIntsData.h"
#include "RfxAction.h"

/*****************************************************************************
 * Class RfxController
 *****************************************************************************/

class RtcImsController : public RfxController {
    // Required: declare this class
    RFX_DECLARE_CLASS(RtcImsController);

public:
    RtcImsController();
    virtual ~RtcImsController();

    void requestImsDisable(int slotId, const sp<RfxAction>& action);
    void requestImsResume(int slotId, const sp<RfxAction>& action);

// Override
protected:
    virtual void onInit();
    virtual void onDeinit();
    virtual bool onHandleRequest(const sp<RfxMessage>& message);
    virtual bool onHandleUrc(const sp<RfxMessage>& message);
    virtual bool onHandleResponse(const sp<RfxMessage>& message);
    virtual bool onCheckIfRejectMessage(
        const sp<RfxMessage>& message, bool isModemPowerOff, int radioState);
    virtual bool onPreviewMessage(const sp<RfxMessage>& message);
    virtual bool onCheckIfResumeMessage(const sp<RfxMessage>& message);

private:
    // Private functions
    void handleImsRegStateRequest(const sp<RfxMessage>& message);
    void handleImsRegStateResponse(const sp<RfxMessage>& message);
    void onRadioStateChanged(RfxStatusKeyEnum key,
            RfxVariant oldValue, RfxVariant value);
    bool responseToRilj(const sp<RfxMessage>& msg);
    /// M: add for op09 default volte setting @{
    void initOp09Ims();
    void deinitOp09Ims();
    void onMainCapabilitySlotChanged(
             RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant new_value);
    void onCardTypeChanged(RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant value);
    void onNoIccIdTimeout();
    void setDefaultVolteState(int slot_id, char new_iccid[], int card_type);
    void setVolteStateProperty(int slot_id, bool isEnable);
    bool isOp09SimCard(int slot_id, char icc_id[], int card_type);
    void sendDefaultVolteStateUrc(int slot_id, int value);
    void handleSetImsConfigRequest(const sp<RfxMessage>& message);
    void handleSetImsConfigResponse(const sp<RfxMessage>& message);
    void setVolteSettingStatus(const sp<RfxMessage>& message);
    static const char* givePrintableStr(const char* iccId);
    /// @}
    void updateImsRegStatus();
    void handleImsDisableDoneUrc(const sp<RfxMessage>& message);
    void handleXui(const sp<RfxMessage>& response);
    void cacheimscfgparams(const sp<RfxMessage>& message);
    void mmsoverruleimscfg(const sp<RfxMessage>& message);
    void sendCachedXui();
    void handleImsRegInfoInd(const sp<RfxMessage>& message);
    void sendCachedImsRegInfo();

private:
    // Private members
    /// M: add for op09 default volte setting @{
    TimerHandle mNoIccidTimerHandle;
    int mNoIccidRetryCount;
    int mMainSlotId;
    bool mIsBootUp;
    bool mIsSimSwitch;
    bool mIsImsDisabling;
    static char sLastBootIccId[4][21];
    static int sLastBootVolteState;
    static bool sInitDone;
    /// @}
    static bool mImsDisByMms[4];
    static int imscfgcache[4][6]; //[slot][params]
    sp<RfxMessage> mCachedXuiUrc;
};

#endif /* __RFX_IMS_CONTROLLER_H__ */
