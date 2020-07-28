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

#ifndef __RP_NW_RAT_CONTROLLER_H__
#define __RP_NW_RAT_CONTROLLER_H__

/*****************************************************************************
 * Include
 *****************************************************************************/

#include "RfxController.h"
#include "RfxAction.h"
#include "RpNwDefs.h"
#include "IRpNwRatSwitchCallback.h"
#include "power/RpRadioController.h"
#include "modecontroller/RpCdmaLteModeController.h"

/* Define the pending rat switch record. */
typedef struct {
    int prefNwType = -1;
    AppFamilyType appFamType;
    NwsMode nwsMode;
    VolteState volteState = VOLTE_OFF;
    RatSwitchCaller ratSwitchCaller;
    sp<RfxAction> action;
    sp<RfxMessage> message;
} PendingRatSwitchRecord;

typedef struct {
    int prefNwType = -1;
    AppFamilyType appFamType = APP_FAM_UNKNOWN;
    NwsMode nwsMode = NWS_MODE_UNKNOWN;
    VolteState volteState = VOLTE_OFF;
    int erat = -1;
    int evdomode = -1;
    int ectmode = -1;
    int prefmode = -1;
    bool md1Radio = false;
    bool md3Radio = false;
    SVLTE_RAT_MODE svlteMode = SVLTE_RAT_MODE_INVALID;
    RatSwitchCaller ratSwitchCaller = RAT_SWITCH_UNKNOWN;
} ModemSettings;

class RpBaseNwRatSwitchHandler;
class RpNwWwopRatController;

/*****************************************************************************
 * Class RpNwRatController
 *****************************************************************************/

class RpNwRatController: public RfxController {
RFX_DECLARE_CLASS(RpNwRatController); // Required: declare this class

public:
    RpNwRatController();
    virtual ~RpNwRatController();

    /* Get functions. */
    AppFamilyType getAppFamilyType();
    // -1 : unknown; or map RIL_PreferredNetworkType.
    int getPreferredNetworkType();
    NwsMode getNwsMode();
    VolteState getVolteState();

    /* Set functions to do network mode switch. */
    void setPreferredNetworkType(const int prefNwType, const sp<RfxAction>& action);
    void setPreferredNetworkType(const RatSwitchInfo ratSwtichInfo);
    void setPreferredNetworkType(const int prefNwType, const sp<RfxMessage>& message);
    void setNwsMode(const NwsMode nwsMode, const sp<RfxAction>& action);

    /* Callbacks for network mode switch. */
    void registerRatSwitchCallback(IRpNwRatSwitchCallback* callback);
    void unRegisterRatSwitchCallback(IRpNwRatSwitchCallback* callback);
    void updateState(NwsMode nwsMode, int prefNwType, VolteState volteState);
    void doPendingRatSwitchRecord();
    void queueRatSwitchRecord(const AppFamilyType appFamType, int prefNwType, NwsMode nwsMode,
            VolteState volteState, const RatSwitchCaller ratSwitchCaller,
            const sp<RfxAction>& action, const sp<RfxMessage>& message);
    bool hasPendingRecord();
    void onRatSwitchStart(const int prefNwType, const NwsMode newNwsMode, const VolteState volteState);
    void onRatSwitchDone(const int prefNwType);
    void onEctModeChangeDone(const int prefNwType);
    static bool getSwitchState();
    void clearSuggetRadioCapability ();
    bool isRestrictedModeSupport();
    NwsMode getNwsModeForSwitchCardType();
    bool isEratExtSupport();
    bool isBspPackages();
    bool isCreateWwop();
    bool isWwop();
    RatSwitchCaller getRatSwitchCaller();
    // For NW module used only!
    int getLastPreferredNetworkType();
    int getCapabilitySlotId();
    bool isCtDualVolteSupport();
    bool needConfigRatModeForCtDualVolte();
    bool is4GNetworkMode(int prefNwType);
    void handleRatSwitchResult(const NwsMode nwsMode, const int prefNwType,
            const VolteState volteState, const RatSwitchCaller ratSwitchCaller,
            const RatSwitchResult switchResult, const int errorCode);
    bool hasPendingVoLTERecord();
    int isInCall();

protected:
    // Override
    virtual void onInit();
    virtual void onDeinit();
    virtual bool onHandleRequest(const sp<RfxMessage>& message);
    virtual bool onHandleResponse(const sp<RfxMessage>& message);
    virtual bool onHandleUrc(const sp<RfxMessage>& message);

private:
    // Utility functions
    const char* requestToString(int reqId);
    const char* urcToString(int reqId);
    /* Do NW RAT switch. */
    void switchNwRat(const AppFamilyType appFamType, int prefNwType, NwsMode nwsMode,
            VolteState volteState, const RatSwitchCaller ratSwitchCaller,
            const sp<RfxAction> &action, const sp<RfxMessage> &message);
    int getSuggestedRadioCapability(ModemSettings mdSettings);
    int calculateTargetPreferredNwType(const AppFamilyType appFamType, int prefNwType, NwsMode nwsMode);
    /* For Engineer mode */
    void onRadioStateChange(RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant value);
    void creatRatSwitchHandlerIfNeeded(const AppFamilyType appFamType);
    bool isValidPreferredNwType(const AppFamilyType appFamType, int prefNwType, NwsMode nwsMode);
    int filterPrefNwType(const AppFamilyType appFamilyType, int prefNwType, int capability);
    void doNwSwitchForEngMode(const sp<RfxAction>& action);
    int getEnginenerMode();
    void clearInvalidPendingRecords();
    void onSuggestRadioCapabilityResult(SuggestRadioResult result);
    void onCardTypeChanged(RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant value);
    void setIntSysProp(char *propName, int val);
    int getIntSysProp(char *propName, int defaultVal);
    bool isNoSimInserted();
    int isNewSimCard(int slotId);
    void setPreferredNetWorkTypeToSysProp(int slotId, int val);
    int getPreferredNetWorkTypeFromSysProp(int slotId);
    int getDefaultNetworkType();
    int getPreferredNetWorkTypeFromRILJ();
    VolteState getLastVolteState();
    NwsMode getLastNwsMode();
    NwsMode filterNwsMode(const AppFamilyType appFamType, const NwsMode nwsMode,
            const VolteState volteState);
    VolteState filterVolteState(const int prefNwType, const VolteState volteState,
            const int ratMode);
    int calculateVolteNetworkType(const AppFamilyType appFamType, const NwsMode nwsMode,
            const VolteState volteState);
    void onVolteStateChanged(RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant value);
    void onCTVolteCardTypeChanged(RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant value);
    bool is4GCdmaCard(int cardType);
    bool containsCdma(int cardType);
    bool containsUsim(int cardType);
    void onVoiceCallCountChanged(int slotId, RfxStatusKeyEnum key, RfxVariant old_value,
            RfxVariant value);

public:
    RfxSignal1<int> m_rat_switch_singal;
    PendingRatSwitchRecord mPendingRestrictedRatSwitchRecord;

private:
    // Variables.
    AppFamilyType mCurAppFamilyType;
    int mCurPreferedNetWorkType;
    int mPreferredNetWorkTypeFromRILJ;
    NwsMode mCurNwsMode;
    //
    AppFamilyType mNewAppFamilyType;
    int mNewPreferedNetWorkType;
    NwsMode mNewNwsMode;
    static bool sIsInSwitching;
    int mCapabilitySlotId;
    int mDefaultNetworkType;
    int mCardType;
    // VoLTE
    VolteState mCurVolteState;
    VolteState mNewVolteState;

    RpBaseNwRatSwitchHandler* mNwRatSwitchHandler;
    PendingRatSwitchRecord mPendingInitRatSwitchRecord;
    PendingRatSwitchRecord mPendingNormalRatSwitchRecord;
    IRpNwRatSwitchCallback* mRpNwRatSwitchListener = NULL;
    RpNwWwopRatController *mNwRatController;
};

#endif /* __RP_NW_RAT_CONTROLLER_H__ */