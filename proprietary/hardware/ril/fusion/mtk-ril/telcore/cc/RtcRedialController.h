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

#ifndef __RTC_REDIAL_CONTROLLER_H__
#define __RTC_REDIAL_CONTROLLER_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RfxController.h"
#include "RfxDefs.h"
#include "RfxRedialData.h"
#include "RfxRootController.h"
#include "RfxStatusDefs.h"


/*****************************************************************************
 * MACRO
 *****************************************************************************/
#define LOG_REDIAL_TAG "RtcRedialController"

#define ECPI_MSG_TYPE_CALL_CONNECT      6
#define ECPI_MSG_TYPE_CALL_ID_ASSIGN    130
#define ECPI_MSG_TYPE_CALL_DISCONNECT   133

#define REDIAL_RIL_PARCEL_HEADER        (2 * sizeof(int))
#define REDIAL_NET_TYPE_GLOBAL_MODE     PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA
#define REDIAL_NET_TYPE_NONE            0
#define REDIAL_NET_TYPE_RESUME_RAT_MODE -1

#define RAT_OPRATION_RESUME_RAT_MODE    0
#define RAT_OPRATION_SWITCH_GLOBAL_MODE 1

#define CALL_DISC_CAUSE_NEED_CHANGE_TO_GLOBAL_MODE 2003

#define EMERGENCY_CALLBACK_MODE_NONE     -1
#define EMERGENCY_CALLBACK_MODE_ENTER    1
#define EMERGENCY_CALLBACK_MODE_EXIT     0


/*****************************************************************************
 * Const
 *****************************************************************************/
static const bool DBG = false;

/*****************************************************************************
 * TYPE
 *****************************************************************************/
enum RedialState {
    REDIAL_NONE     = 0,
    REDIAL_DIALING  = 1,
    REDIAL_DIALED   = 2,
    REDIAL_DONE     = 3,
    REDIAL_END      = 100,
};

enum RatState {
    REDIAL_RAT_NONE     = 0,
    REDIAL_RAT_CHANGED  = 1,
    REDIAL_RAT_RESUME   = 2,
    REDIAL_RAT_END      = 100,
};

enum RedialRatOp {
    REDIAL_RAT_OP_NONE          = 0,
    REDIAL_RAT_OP_GLOBAL        = 1,
    REDIAL_RAT_OP_RESUME        = 2,
    REDIAL_RAT_OP_RESUME_PENDING = 3,
    REDIAL_RAT_OP_END           = 100,
};


/*****************************************************************************
 * Class MoCallContext
 *****************************************************************************/
class MoCallContext {
public:
    explicit MoCallContext(const sp<RfxMessage>& msg, bool isEcc = true);
    explicit MoCallContext(char *callNumber, bool isEcc = true);
    virtual ~MoCallContext();

    int getCallId() const {
        return mCallId;
    }

    int getClientId() const {
        return mClientId;
    }

    bool isEccCall() const {
        return mIsEcc;
    }

    char *getNumber() const;
    RfxRedialData *getRedialData();
    void setCallId(int callId);


private:
    bool    mIsEcc;
    int     mCallId;
    int     mClientId;
    char    *mNumber;
    RfxDialData *mDialData;
    RfxRedialData *mRedialData;
};


/*****************************************************************************
 * Class RtcRedialController
 *****************************************************************************/
class RtcRedialController : public RfxController {
    RFX_DECLARE_CLASS(RtcRedialController);

public:
    RtcRedialController();
    virtual ~RtcRedialController();
    virtual bool isEmergencyModeSupported(int slotId);

protected:
    virtual void onInit();
    virtual void onDeinit();
    virtual bool onHandleRequest(const sp<RfxMessage>& message);
    virtual bool onHandleUrc(const sp<RfxMessage>& message);
    virtual bool onHandleResponse(const sp<RfxMessage>& message);
    virtual bool onHandleAtciRequest(const sp<RfxMessage>& msg);
    virtual bool onHandleAtciResponse(const sp<RfxMessage>& msg);
    virtual bool onCheckIfRejectMessage(
        const sp<RfxMessage>& message, bool isModemPowerOff, int radioState);

    void resetController();
    void increaseEmcsCount();
    void setEmergencyMode(bool emergencyMode);

public:
    bool notifyRilRequest(const sp<RfxMessage>& message);
    bool notifyRilResponse(const sp<RfxMessage>& message);
    bool notifyRilUrc(const sp<RfxMessage>& message);
    void setEmergencyModeExt(bool emergencyMode);
    void setEccCallId(int callId); // for 380 case.

private:
    void handleEmergencyDial(const sp<RfxMessage>& message);
    bool handleAtciDialRequest(const char* data, int dataLength);
    bool handleCallInfoUpdate(const sp<RfxMessage>& message);
    void handleEmergencyCallbackMode(int mode);

    void ForceReleaseEmergency();
    void checkMoCallAndFree();
    bool checkFeatureEnabled();
    void setUserHangUping(bool userHangUping);
    void setRedialState(RedialState state);
    void setRatMode(RedialRatOp ratOp);
    void destroyMoCallContext();
    bool canReleaseEmergencySession();
    bool decreaseEmcsCountAndIsZero();


    int getGlobalRatMode();
    int getCurrentRatMode();
    void onRatModeSwitchDone(RedialRatOp ratOp);
    sp<RfxMessage> getRedialRequest();
    void redial();

    void deferMsg(const sp<RfxMessage>& msg);
    void resumeDeferMsg();
    void clearDeferMsg();
    void onCallCountChanged(RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant value);

    void setPendingExitEmergencySession(bool pending);
    bool isPendingExitEmergencySession();

protected:
    MoCallContext *mMoCall;
    bool    mInEmergencyMode;
    bool    mIsEccModeSent;  // means EMCS sent earlier under flight mode

private:

    bool    mIsUserHangUping;
    bool    mPendingExitES;
    int     mEcbm;
    int     mEmcsCount;

    RedialState     mRedialState;
    RatState        mRatState;
    RedialRatOp     mRatOp;

    Vector<RfxSuspendedMsgEntry> mSuspendedMsgQueue;
};

const char *boolToString(bool value);

#endif /* __RTC_REDIAL_CONTROLLER_H__ */
