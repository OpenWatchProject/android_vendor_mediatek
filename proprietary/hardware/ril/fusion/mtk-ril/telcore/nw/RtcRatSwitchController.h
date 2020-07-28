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

#ifndef __RTC_RAT_SWITCH_CONTROLLER_H__
#define __RTC_RAT_SWITCH_CONTROLLER_H__

#undef NDEBUG
#ifdef LOG_NDEBUG
#undef LOG_NDEBUG
#endif
/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RfxController.h"
#include "RfxAction.h"
#include "RtcNwDefs.h"
#include "RfxVoidData.h"
#include "RfxRilUtils.h"
#include "RfxStringData.h"
#include "RfxStringsData.h"
#include "RfxIntsData.h"
#include "RfxMessageId.h"
#include "modecontroller/RtcModeSwitchController.h"
#include "capabilityswitch/RtcCapabilitySwitchUtil.h"
#include "wp/RtcWpController.h"
#include <telephony/mtk_ril.h>
#include "rfx_properties.h"
#include <libmtkrilutils.h>

/*****************************************************************************
 * Define
 *****************************************************************************/
typedef struct {
    int prefNwTypeFromRilj = -1;  // RILJ setting
    int prefNwTypeDefault = -1;  // Default setting
    int prefNwType = -1;  // MD setting
    RatSwitchCaller ratSwitchCaller = RAT_SWITCH_UNKNOWN;
    sp<RfxAction> action;
    sp<RfxMessage> message;
} RatSettings;

typedef enum {
    SWITCH_RAT,
    GMSS_RAT
} PHONE_CHANGE_SOURCE;

typedef enum {
    PS_RAT_FAMILY_UNKNOWN = 0,
    PS_RAT_FAMILY_GSM = 1,
    PS_RAT_FAMILY_CDMA = 2,
    PS_RAT_FAMILY_IWLAN = 4,
} PsRatFamily;

/*****************************************************************************
 * Class RfxController
 *****************************************************************************/
class RtcRatSwitchController : public RfxController {
    // Required: declare this class
    RFX_DECLARE_CLASS(RtcRatSwitchController);

public:
    RtcRatSwitchController();
    virtual ~RtcRatSwitchController();
    void setPreferredNetworkType(RatSwitchInfo ratSwtichInfo);
    void setPreferredNetworkType(const int prefNwType, const sp<RfxAction>& action);
    PsRatFamily getPsRatFamily(int radioTechnology);
    void setLocalAbortAvailableNetwork(const sp<RfxAction>& action);
    bool isECBMRestrictedModeSupport();

protected:
    virtual bool onHandleRequest(const sp<RfxMessage>& message);
    virtual bool onHandleUrc(const sp<RfxMessage>& message);
    virtual bool onHandleResponse(const sp<RfxMessage>& message);
    virtual void onInit();
    virtual void onDeInit();

    virtual bool onCheckIfRejectMessage(const sp<RfxMessage>& message,
        bool isModemPowerOff,int radioState);
    virtual bool onPreviewMessage(const sp<RfxMessage>& message);
    virtual bool onCheckIfResumeMessage(const sp<RfxMessage>& message);

private:
    int calculateDefaultNetworkType(RatSwitchInfo ratSwtichInfo);
    void setPreferredNetworkType(const sp<RfxMessage>& message);
    void getPreferredNetworkType(const sp<RfxMessage>& message);
    void requestVoiceRadioTech(const sp<RfxMessage>& message);
    bool onHandleAtciRequest(const sp<RfxMessage>& message);
    void handleGmssRatChanged(const sp<RfxMessage>& message);

    void responseSetPreferredNetworkType(const sp<RfxMessage>& response);
    void responseGetPreferredNetworkType(const sp<RfxMessage>& response);
    void responseGetVoiceRadioTech(const sp<RfxMessage>& response);

    void switchNwRat(int prefNwType,  const RatSwitchCaller ratSwitchCaller,
        const sp<RfxAction>& action, const sp<RfxMessage>& message);
    void configRatMode();
    void queueRatSwitchRecord(int prefNwType, const RatSwitchCaller ratSwitchCaller,
        const sp<RfxAction>& action, const sp<RfxMessage>& message);
    void processPendingRatSwitchRecord();
    bool hasPendingRecord();
    void updateState(int prefNwType, RatSwitchResult switchResult);
    void updatePhoneMode(PHONE_CHANGE_SOURCE source);
    void onApVoiceCallCountChanged(int slotId, RfxStatusKeyEnum key,
        RfxVariant old_value, RfxVariant value);
    void onVolteStateChanged(RfxStatusKeyEnum key, RfxVariant old_value,
        RfxVariant value);
    void onImsRegistrationStatusChanged(RfxStatusKeyEnum key,
        RfxVariant old_value, RfxVariant value);

    int isNetworkScanOngoing();
    bool isAPInCall();
    bool isGsmOnlySimFromMode(RatSwitchInfo ratSwtichInfo);
    bool isNoSimInserted();
    bool isCT4GCard();
    bool isGsmOnlySim();
    bool isCdmaOnlySim();
    bool isCdmaDualModeSimCard();
    bool isSlaveInDualCDMACard();
    bool isNetworkTypeContainLteCdma();
    int getMajorSlotId();
    int getCCapabilitySlotId();
    bool isHvolteSupport();

    void setSlotCapability(int slotId, int val);
    int getSlotCapability(int slotId);
    int getDefaultNetWorkType(int slotId);
    void setPreferredNetWorkTypeToSysProp(int slotId, int val);
    int getPreferredNetWorkTypeFromSysProp(int slotId);
    void setIntSysProp(char *propName, int val);
    int getIntSysProp(char *propName, int defaultVal);
    int isNewSimCard(int slotId);
    int isRafContainsCdma(int raf);
    int isRafContainsGsm(int raf);

    const char *switchCallerToString(int callerEnum);

    int filterPrefNwTypeFromRilj(const int prefNwType);

    void responseGetGmssRatMode(const sp<RfxMessage>& response);
    void getGmssRatMode();
    void handleGmssRat(const sp<RfxMessage>& message);

    int getIccIdCacheIndex(int slotId);
    int getNwTypeCacheByIndex(int index);
    void setNwTypeCacheByIndex(int index, int type);

    void onUiccMccMncChanged(RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant value);
    bool needDisableCdmaRat();
    bool needDisableCdmaRatByMccMnc(const char *mccmnc);

    void onRestrictedModeChanged(RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant value);

public:
    RfxSignal2<int, int> mRatSwitchSignal;
private:
    static bool sIsInSwitching;
    Vector<int> mTokensGetEgmssForHvolte;
    RatSettings mRatSettings;

    RatSettings mPendingInitRatSwitchRecord;
    RatSettings mPendingNormalRatSwitchRecord;
    RatSettings mPendingRestrictedRatSwitchRecord;

    int mDefaultNetworkType;
    int mCurPreferedNetWorkType;
    int mPhoneMode;
    NwsMode mNwsMode;
    int mNetworkTypeIndex;
    int mNetworkTypeCache;
    int mRestrictedCount;
    bool mInRestrictedMode;
    sp<RfxAction> mAbortAction;
};
#endif /* __RTC_RAT_SWITCH_CONTROLLER_H__ */
