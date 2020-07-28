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

#ifndef __RP_CALL_CONTROLLER_H__
#define __RP_CALL_CONTROLLER_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include <cutils/properties.h>
#include <log/log.h>
#include "RfxController.h"
#include "RfxPreciseCallState.h"
#include "RfxTimer.h"
#include "RpAudioControlHandler.h"
#include "RpCallCenterController.h"
#include "RpRedialHandler.h"
#include "utils/String16.h"
#include "utils/Vector.h"

using ::android::String16;
using ::android::Vector;

/*****************************************************************************
 * Class RpCallController
 *****************************************************************************/
extern "C" const char * requestToString(int request);

class RpCallController : public RfxController {
    RFX_DECLARE_CLASS(RpCallController); // Required: declare this class

public:
    RpCallController();
    virtual ~RpCallController();

// Override
protected:
    virtual bool onHandleRequest(const sp<RfxMessage>& request);
    virtual bool onHandleUrc(const sp<RfxMessage>& message);
    virtual bool onHandleResponse(const sp<RfxMessage>& response);
    virtual void onInit();
    virtual void onDeinit();

private:
    void onCsPhoneChanged(RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant value);
    bool onPreHandleRequest(const sp<RfxMessage>& request);
    void onHandleGetCurrentCalls(const sp<RfxMessage>& response);
    void onHandleIncomingCallIndication(const sp<RfxMessage>& message);

    // forceRelease
    bool hasPendingHangupRequest(bool isForegnd);
    bool hasPendingHangupRequest(int hangupCallId);

    // call state cache
    void handleEmergencyDialRequest(const sp<RfxMessage>& request);
    void onServiceStateChanged(RfxStatusKeyEnum key, RfxVariant oldValue, RfxVariant newValue);
    void handleCdmaFlashRequest(const sp<RfxMessage>& request);
    void onNotifyCdmaCallWait();
    void onNotifyPreciseCallState(const sp<RfxMessage>& message);
    Vector<RfxPreciseCallState*>* parsePreciseCallState(Parcel* parcel);
    void updateDisconnecting(Vector<RfxPreciseCallState*>* origList, int hangupCallId);
    void updateDisconnecting(Vector<RfxPreciseCallState*>* origList, bool isForegnd);
    void updateDisconnected(
            Vector<RfxPreciseCallState*>* oldList, Vector<RfxPreciseCallState*>* newList);
    void updatePreciseCallStateList(
            RfxPreciseCallState* preciseCallState, Vector<RfxPreciseCallState*>* list);
    static void freePreciseCallStateList(Vector<RfxPreciseCallState*>* list);

    int getRilRequestSendDomain(const sp<RfxMessage>& request);
    int getCommonRilRequestSendDomain(const sp<RfxMessage>& request);

    void handleAutoAnswer(const sp<RfxMessage>& msg);
    void onAutoAnswerTimer();

    char* strdupReadString(Parcel *p);

private:
    int mCsPhone;
    int mSwitchAntennaSupported;
    int mCallCount;
    int mCsCallCount;
    int mPsCallCount;
    String16 mEccNumber;
    Vector<RfxPreciseCallState*>* mPreciseCallStateList;
    CallRat mCallRat;
    RpRedialHandler *mRedialHandler;
    RpAudioControlHandler *mAudioControlHandler;
    bool mUseLocalCallFailCause;
    int mDialLastError;
    TimerHandle mAutoAnswerTimerHandle;
    int mCallCountForAutoAnswer;

    sp<RfxMessage> mOriginalDialMessage;
};

#endif /* __RP_CALL_CONTROLLER_H__ */

