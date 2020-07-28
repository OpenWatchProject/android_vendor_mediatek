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

#ifndef __RP_NW_WWOP_RAT_CONTROLLER_H__
#define __RP_NW_WWOP_RAT_CONTROLLER_H__

/*****************************************************************************
 * Include
 *****************************************************************************/

#include "RfxController.h"
#include "RfxAction.h"
#include "RpNwDefs.h"
#include "RpNwRatController.h"
#include "IRpNwRatSwitchCallback.h"
#include "power/RpRadioController.h"
#include "power/RpRadioController.h"

class RpBaseNwRatSwitchHandler;

/*****************************************************************************
 * Class RpNwWwopRatController
 *****************************************************************************/
class RpNwWwopRatController : public RfxController {
RFX_DECLARE_CLASS(RpNwWwopRatController); // Required: declare this class

public:
    RpNwWwopRatController();
    virtual ~RpNwWwopRatController();

protected:
    // Override
    virtual void onInit();
    virtual void onDeinit();
    virtual bool onHandleRequest(const sp<RfxMessage> &message);
    virtual bool onHandleResponse(const sp<RfxMessage> &message);
    virtual bool onHandleUrc(const sp<RfxMessage> &message);

    // For suspend/resume mechanism.
    virtual bool onPreviewMessage(const sp<RfxMessage>& message);
    virtual bool onCheckIfResumeMessage(const sp<RfxMessage>& message);

private:
    void switchNwRat(const AppFamilyType appFamType,
            int prefNwType, NwsMode nwsMode, const RatSwitchCaller ratSwitchCaller,
            const sp<RfxAction> &action, const sp<RfxMessage> &message);
    void queueRatSwitchRecord(const AppFamilyType appFamType,
            int prefNwType, NwsMode nwsMode, const RatSwitchCaller ratSwitchCaller,
            const sp<RfxAction> &action, const sp<RfxMessage> &message);
    void calculateModemSettings(int prefNwType,
            AppFamilyType appFamType, NwsMode nwsMode);
    void controlSwitchFlow(RAT_CTRL_ACTION nextAction);
    void configRadioPower(bool state);
    void onSuggestRadioResult(RfxStatusKeyEnum key,
            RfxVariant oldValue, RfxVariant newValue);
    void onSuggestRadioOffResult(SuggestRadioResult result);
    void onSuggestRadioOnResult(SuggestRadioResult result);
    void configEvdoMode();
    void configSvlteMode();
    void configRatMode();
    void configPhoneMode();
    int  getSuggestedRadioCapability();
    void updateState(NwsMode nwsMode, int prefNwType);
    void handleGetPreferredNwType(const sp<RfxMessage> &message);
    void responseSetPreferredNetworkType(
            const RIL_Errno error, const sp<RfxMessage> &response);
    void responseSetSvlteMode(const sp<RfxMessage> &response);
    void responseGetVoiceRadioTech(const sp<RfxMessage> &response);
    void onRatSwitchStart(const int prefNwType, const NwsMode newNwsMode);
    void onRatSwitchDone(const int prefNwType);
    void onEctModeChangeDone(const int prefNwType);
    void onResponseTimeOut(const sp<RfxMessage> &response);
    void doPendingRatSwitchRecord();
    bool hasPendingRecord();
    void doNwSwitchForEngMode(const sp<RfxAction> &action);
    const char *requestToString(int reqId);
    const char *urcToString(int reqId);
    const char *sourceToString(RILD_RadioTechnology_Group srcId);
    const char *switchCallerToString(int callerEnum);
    int getEnginenerMode();
    bool isNoSimInserted();
    bool isCdmaOnlySim();
    bool isCdmaDualModeSimCard();
    int  getMajorSlotId();
    int getCdmaSlotId();
    void setIntSysProp(const char *propName, int val);
    int  getIntSysProp(const char *propName, int defaultVal);
    int  isNewSimCard(int slotId);
    void setPreferredNetWorkTypeToSysProp(int slotId, int val);
    int  getPreferredNetWorkTypeFromSysProp(int slotId);
    void onRestrictedModeChanged(RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant value);
    int selectNwTypeBySlot(int prefNwType);
    bool isEratExtSupport();

public:
    void setRestrictedNetworkMode(const int prefNwType, const sp<RfxAction>& action);
    void setAppFamilyType(const AppFamilyType appFamType,
            const int prefNwType, int cardState, const sp<RfxAction> &action);
    void setNwsMode(const NwsMode nwsMode, const sp<RfxAction> &action);
    NwsMode getNwsMode();
    AppFamilyType getAppFamilyType();
    int  getPreferredNetworkType();
    void registerRatSwitchCallback(IRpNwRatSwitchCallback *callback);
    void unRegisterRatSwitchCallback(IRpNwRatSwitchCallback *callback);

public:
    RfxSignal1<int> m_rat_switch_singal;
    PendingRatSwitchRecord mPendingInitRatSwitchRecord;
    PendingRatSwitchRecord mPendingNormalRatSwitchRecord;
    PendingRatSwitchRecord mPendingRestrictedRatSwitchRecord;

private:
    // Variables.
    AppFamilyType mCurAppFamilyType;
    int mCurPreferedNetWorkType;
    NwsMode mCurNwsMode;
    int mDesiredNetWorkTypeFromRILJ;
    int mPreferredNetWorkTypeFromRILJ;

    AppFamilyType mNewAppFamilyType;
    int mNewPreferedNetWorkType;
    NwsMode mNewNwsMode;
    RAT_CTRL_ACTION mCtrlAction;

    int mSugggestedResetRadioCapablity;
    ModemSettings mModemSettings;
    sp<RfxAction> mAction;
    sp<RfxMessage> mMessage;

    static bool sIsInSwitching;
    int mRestrictedCount;

    IRpNwRatSwitchCallback *mRpNwRatSwitchListener = NULL;
    RpRadioController *mRpRadioController = NULL;
};

#endif /* __RP_NW_WWOP_RAT_CONTROLLER_H__ */
