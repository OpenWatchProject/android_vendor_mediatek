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

#ifndef __RP_REDIAL_HANDLER_H__
#define __RP_REDIAL_HANDLER_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include <binder/Parcel.h>
#include <cutils/properties.h>

#include "RfxController.h"
#include "RfxStatusDefs.h"
#include "RfxTimer.h"
#include "RfxRootController.h"
#include "nw/RpNwRatController.h"

/*****************************************************************************
 * MACRO
 *****************************************************************************/
#define ECPI_MSG_TYPE_CALL_CONNECT      6
#define ECPI_MSG_TYPE_CALL_ID_ASSIGN    130
#define ECPI_MSG_TYPE_CALL_DISCONNECT   133

#define REDIAL_RIL_PARCEL_HEADER        (2 * sizeof(int))
#define REDIAL_NET_TYPE_GLOBAL_MODE     PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA
#define REDIAL_NET_TYPE_NONE            0
#define REDIAL_NET_TYPE_RESUME_RAT_MODE -1
#define REDIAL_NET_TYPE_RESUME_RAT_MODE_PENDING -2

#define CALL_DISC_CAUSE_REDIAL_MD1                      107
#define CALL_DISC_CAUSE_REDIAL_MD3                      2001
#define CALL_DISC_CAUSE_REDIAL_MD3_WITHOUT_TRY_VOLTE    2002
#define CALL_DISC_CAUSE_NEED_CHANGE_TO_GLOBAL_MODE      2003

#define REDIAL_SBP_ID_VZW       12
#define REDIAL_SBP_ID_SPRINT    20
#define REDIAL_TC_ID_COMMON     0
#define REDIAL_TC_ID_01         1

#define IMS_SOCKET              1605

#define RP_REDIAL_TAG "RpRedialHandler"

#define REDIAL_ASSERT(_c)       assert(_c)
#define REDIAL_DBG_ASSERT(_c)   assert(_c)

#define CALL_COUNT_POS 3

extern "C" const char * requestToString(int request);
/*****************************************************************************
 * ENUM
 *****************************************************************************/
enum RedialState {
    REDIAL_NONE     = 0,
    REDIAL_TO_MD1   = 1,
    REDIAL_TO_MD3   = 2,
    REDIAL_CHANGE_GLOBAL_MODE = 3,
};

enum HvolteMode {
    HVOLTE_MODE_SRLTE = 0,
    HVOLTE_MODE_LTE_ONLY = 1,
    HVOLTE_MODE_NONE = 10,
};

enum E911NetworkType {
    E911_RAT_NONE   = 0,
    E911_RAT_GSM    = 1,
    E911_RAT_UMTS   = 2,
    E911_RAT_LTE    = 4,
    E911_RAT_C2K    = 16,
    E911_RAT_CHANGE_GLOBAL_MODE = 100,
};

enum E911CallDomain {
    E911_CS_DOMAIN  = 0,
    E911_PS_DOMAIN  = 1,
    E911_WWAN_TIMER_EXPIRE = 2, // Try to dial ECC to WiFi
};

enum EmergencyMode {
    EMERGENCY_MODE_END      = 0,
    EMERGENCY_MODE_BEGIN    = 1,
};

enum EmergencyCallBackMode {
    E911_ECBM_NONE          = 0,
    E911_ECBM_ENTER_C2K     = 1,
    E911_ECBM_ENTER_GSM_LTE = 2,
};

enum E911CallState {
    E911_CALL_ENTER         = 0,
    E911_CALL_EXIT_WITH_ECB = 1,
    E911_CALL_EXIT          = 2,
    E911_CALL_ENTER_ECBM    = 3
};

/*****************************************************************************
 * Class MoCallContext
 *****************************************************************************/
class MoCallContext {
    public:
        MoCallContext(char *callNumber, bool isEcc, int slotId, int msgId);
        MoCallContext(Parcel& p, bool isEcc, int slotId, int msgId);

        Parcel* getDialParcel();
        int     getCallId();
        int     getSlotId();
        int     getMsgId();
        void    setCallId(int callId);
        bool    isEccCall();

    private:
        Parcel  mDialParcel;
        int     mCallId;
        int     mSlotId;
        int     mMsgId;
        bool    mIsEmergencyCall;
};

/*****************************************************************************
 * Class RpRedialHandler
 *****************************************************************************/
class RpRedialHandler : public RfxController {
    RFX_DECLARE_CLASS(RpRedialHandler);  // Required: declare this class

public:
    RpRedialHandler();

    virtual ~RpRedialHandler();

    bool notifyRilRequest(const sp<RfxMessage>& msg);
    const sp<RfxMessage>  notifyRilResponse(const sp<RfxMessage>& msg);
    bool notifyRilUrc(const sp<RfxMessage>& msg);
    bool checkRedialFeatureEnabled(int sbpId, int tcId);
    bool isDialFromIms();
    RedialState getRedialState();
    EmergencyCallBackMode getEcbm();

    // Utility function
    static char const * boolToString(bool value);
    static char const * redialStateToString(RedialState value);
    static char const * deviceModeToString(int deviceMode);
    static char const * radioTechToString(int radio);
    static char const * hvolteModeToString(HvolteMode mode);
    static char* strdupReadString(Parcel &p);
    static void  writeStringToParcel(Parcel &p, const char *s);

// Override
protected:
    virtual bool onHandleAtciRequest(const sp<RfxMessage>& msg);
    virtual bool onHandleAtciResponse(const sp<RfxMessage>& msg);
    virtual void onInit();
    virtual void onDeinit();

private:
    void onGlobalModeSwitchDone(int slotId);
    void onGlobalModeResumeDone(int slotId);
    void onAudioModeSwitchDone();
    void onCapabilityChanged(RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant value);
    void onRatModeSwitch(RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant value);
    void onCsCallCountChanged(RfxStatusKeyEnum key, RfxVariant oldValue, RfxVariant newValue);
    bool handleDialRequest(const sp<RfxMessage>& msg);
    bool handleAtciDialRequest(const char* data, int dataLength, int slotId);
    bool handleAtciCdmaDialRequest(const char* data, int dataLength, int slotId);
    bool handleCallInfoInd(const sp<RfxMessage>& msg);
    bool handleCallStateChanged(const sp<RfxMessage>& msg);
    bool handleEnterEcbm(const sp<RfxMessage>& msg);
    bool handleExitEcbm(const sp<RfxMessage>& msg);
    bool handleExitEmergencyMode(const sp<RfxMessage>& msg);
    bool handleUpdateHvolteMode(const sp<RfxMessage>& msg);
    bool handleSrvcc(const sp<RfxMessage>& msg);
    bool handleSetHvolteModeRsp(const sp<RfxMessage>& msg);
    bool handleHangUp(const sp<RfxMessage>& msg);
    bool processCallConnect(const sp<RfxMessage>& msg);
    bool processCallDisconnect(const sp<RfxMessage>& msg, int discCause);
    void deferMsg(const sp<RfxMessage>& msg);
    void resumeMsg();
    const sp<RfxMessage> getDeferredMsg(int msgId);

    // Overwrite
    bool responseToRilj(const sp<RfxMessage>& msg);

    bool checkDropUrc(const sp<RfxMessage>& msg);
    void setRedialState(RedialState state);
    void setEmergencyMode(bool emergencyMode, int slotId);
    void setEcbm(EmergencyCallBackMode ecbm);
    void setUserHangUping(bool userHangUping);
    void setRatMode(int ratMode, int slotId);
    void setPreRatMode(int ratMode);
    void setHvolteMode(HvolteMode mode, int slotId);
    void sendHvolteModeReq(int mode, int subMode);
    void sendOemHookString(char* atCmdLine, RILD_RadioTechnology_Group sendTarget);
    void notifyEmbmsE911Status(EmergencyMode mode, int slotId);
    sp<RfxMessage> getRedialRequest(RILD_RadioTechnology_Group dialTarget, bool isVolteTried);
    int  getDeviceMode(int slotId);
    int  getGlobalMode();
    void freeMoCallObject();
    int getCallCount(const sp<RfxMessage>& message);

public:

private:
    /* mPreRatMode is used to store the preferred network type before entering global mode.
     *  mPreRatMode: Integer
     *  0: Not switch to global mode
     *  1~xx: previous preferred network type(in global mode)
     *  -1: Resume from global mode to previous preferred network type
     */
    int     mPreRatMode;
    bool    mInEmergencyMode;
    bool    mIsUserHangUping;
    bool    mShouldDropGetCurrentCalls;
    EmergencyCallBackMode   mEcbm;

    RedialState     mRedialState;
    HvolteMode      mHvolteMode;
    MoCallContext   *mMoCall;

    Vector<RfxSuspendedMsgEntry> mSuspendedMsgQueue;
};

#endif /* __RP_REDIAL_HANDLER_H__ */

