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

#ifndef __RFX_CALL_CONTROLLER_H__
#define __RFX_CALL_CONTROLLER_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include <map>
#include <vector>
#include <string>

#include "RfxImsCallInfo.h"
#include "ims/RtcImsConferenceCallMessageHandler.h"
#include "RfxPreciseCallState.h"
#include "RfxTimer.h"
#include "RtcRedialController.h"
#include "utils/String16.h"
#include "utils/String8.h"
#include "utils/Vector.h"

using ::android::String16;
using ::android::String8;
using ::android::Vector;

using std::string;

#define MAX_ADDRESS_LEN     40

/*****************************************************************************
 * Class RfxController
 *****************************************************************************/

class RfxStringsData;
class RfxImsCallInfo;

class RtcCallController : public RfxController {
    // Required: declare this class
    RFX_DECLARE_CLASS(RtcCallController);

public:
    RtcCallController();
    virtual ~RtcCallController();
    void onParticipantsUpdate(vector<sp<ConferenceCallUser>> users, bool autoTerminate);
    void onMergeStart();
    void retrieveMergeFail(int activeCallIdBeforeMerge);

    void notifyCallInfoUpdate(const sp<RfxMessage>& message);

// Override
protected:
    virtual void onInit();
    virtual void onDeinit();
    virtual bool onHandleRequest(const sp<RfxMessage>& message);
    virtual bool onHandleUrc(const sp<RfxMessage>& message);
    virtual bool onHandleResponse(const sp<RfxMessage>& message);
    virtual bool onPreviewMessage(const sp<RfxMessage>& message);
    virtual bool onCheckIfResumeMessage(const sp<RfxMessage>& message);
    virtual bool onCheckIfRejectMessage(
        const sp<RfxMessage>& message, bool isModemPowerOff, int radioState);
    virtual void createRedialController();

    void responseDialFailed(const sp<RfxMessage>& message);
    // error handling
    bool rejectDualDialForDSDS();
    // handle Ims call
    void handleImsDialRequest(const sp<RfxMessage>& message);
    void handleImsDialFailResponse(const sp<RfxMessage>& message);
    void imsCallEstablishFailed(int slotId);
    void responseToRilJAndUpdateIsImsCallExist(const sp<RfxMessage>& msg);
    void updateIsImsCallExistToStatusManager(int slotId);

    RtcRedialController *mRedialCtrl;

private:
    void onCsPhoneChanged(RfxStatusKeyEnum key, RfxVariant oldValue, RfxVariant newValue);

    void onServiceStateChanged(RfxStatusKeyEnum key, RfxVariant oldValue, RfxVariant newValue);

    void onRadioStateChanged(RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant value);

    void onCardTypeChanged(RfxStatusKeyEnum key, RfxVariant oldValue, RfxVariant newValue);
    bool isCsCallExistAndNoEccExist();
    bool isCsActiveCallExistAndNoEccExist();

    bool handleCsCallInfoUpdate(const sp<RfxMessage>& msg);

    bool isInChina();

    // handle Ims call
    bool hasImsCall(int slotId);
    bool handleIncomingCall(int slotId, RfxStringsData* data);
    bool handleImsCallInfoUpdate(const sp<RfxMessage>& msg);
    void handleSrvcc(int slotId, const sp<RfxMessage>& msg);
    bool isDuringSrvcc(int slotId);
    void addImsCall(int slotId, RfxImsCallInfo* call);
    void removeImsCall(int slotId, RfxImsCallInfo* call);
    void assignImsCallId(int slotId, int callId);
    bool updateNumberFromEcpiNumOrPau(RfxImsCallInfo* targetCall, string ecpiNumber, string pau);
    bool updateNumberFromPau(RfxImsCallInfo* targetCall, string pau);
    bool updateDisplayNameFromPau(RfxImsCallInfo* targetCall, string pau);
    string getFieldValueFromPau(string pau, string field);
    void clearAllImsCalls(int slotId);
    void releaseEstablishingCall(int slotId);
    bool waitCallRingForMT(const sp<RfxMessage>& msg);
    void handleCallRing(int slotId);
    bool handleImsCallModeChange(const sp<RfxMessage>& msg);
    void clearCallRingCache(int slotId);
    void generateImsConference(int slotId, int callId);
    void handleImsStkCall(int slotId, char** params);

    // forceRelease
    bool hasPendingHangupRequest(bool isForegnd);
    bool hasPendingHangupRequest(int hangupCallId);

    // call state cache
    void handleCdmaFlashRequest(const sp<RfxMessage>& message);
    void handleCdmaCallWait();
    void handleGetCurrentCallsResponse(const sp<RfxMessage>& message);
    Vector<RfxPreciseCallState*>* parsePreciseCallState(RIL_Call ** pp_calls, int count);
    void updateDisconnecting(Vector<RfxPreciseCallState*>* origList, int hangupCallId);
    void updateDisconnecting(Vector<RfxPreciseCallState*>* origList, bool isForegnd);
    void updateDisconnected(
            Vector<RfxPreciseCallState*>* oldList, Vector<RfxPreciseCallState*>* newList);
    void freePreciseCallStateList(Vector<RfxPreciseCallState*>* list);
    void updatePreciseCallStateList(
            RfxPreciseCallState* preciseCallState, Vector<RfxPreciseCallState*>* list);

    // error handling
    RIL_CALL_INFO_TYPE shouldRejectIncomingCall(int callMode);
    void approveIncoming(int callId, int seqNo, bool approve);

    bool canHandleCsDialRequest(const sp<RfxMessage>& message);

    bool canHandleHangupForegndOrBackgndRequest(const sp<RfxMessage>& message);

    void unsetEccModeLocally();
    /* ALPS03346578: Emergency dial can be handled after receiving response of
        RFX_MSG_REQUEST_SET_ECC_MODE */
    bool canHandleEmergencyDialRequest(const sp<RfxMessage>& message);
    RfxImsCallInfo* getCallByCallId(int slotId, int callId);
    void updateCallCount();
    int getValidImsCallCount();

    int getPendingMTCallId();

    bool handlePendingMTCallMode(const sp<RfxMessage>& msg);
    void handleAsyncCallControlResult(const sp<RfxMessage>& message);
    void handleAsyncCallControlResponse(const sp<RfxMessage>& message);
    void handleAsyncImsCallControlRequest(const sp<RfxMessage>& message);
    bool shouldDoAsyncImsCallControl();

    void handleImsEventPackageIndication(const sp<RfxMessage>& msg);
    void hangupIncomingCall();
    bool handleSetCallIndicationRequest(const sp<RfxMessage>& msg);
    bool reportFakeImsRingingCall(const sp<RfxMessage>& msg, int seqNo);
    void maybeSendNoEcbm(bool isForce);

    void handleAutoAnswer(const sp<RfxMessage>& msg);
    void onAutoAnswerTimer();
    void handleSpeechCodecInfo(const sp<RfxMessage>& msg);
    void handleImsGetCurrentCalls(const sp<RfxMessage>& msg);
    void handleImsHangupWaitingOrBackground(const sp<RfxMessage>& msg);
    void handleImsHangupFgResumeBg(const sp<RfxMessage>& msg);
    void imsCallToRilCall(int &index, RfxImsCallInfo* call, RIL_Call* p_calls);
    void checkIfResumeBg(int slotId);
    void handleImsSwitchFgBg(const sp<RfxMessage>& msg);
    bool canHandleRequestDuringComboOperations(const sp<RfxMessage>& message);
    Vector<RfxImsCallInfo*> getWaitingCall(Vector<RfxImsCallInfo*> calls);
    Vector<RfxImsCallInfo*> getHoldingCall(Vector<RfxImsCallInfo*> calls);
    Vector<RfxImsCallInfo*> getActiveCall(Vector<RfxImsCallInfo*> calls);
    RfxImsCallInfo* getImsCallByCallId(Vector<RfxImsCallInfo*> calls, int callId);
    Vector<RfxImsCallInfo*> getConferenceHostCall(Vector<RfxImsCallInfo*> calls);
    void dumpImsCallList(Vector<RfxImsCallInfo*> calls);
    void handleEconfResult(const sp<RfxMessage>& msg);
    bool canHandleImsGetCurrentCallsRequest(const sp<RfxMessage>& message);
    bool allowImsIncomingOnAnotherSlot();
    void hangupIncomingCallOnAllSlot();
    bool handleClirSpecial(const sp<RfxMessage>& msg);
    bool canHandleAnswerRequest(const sp<RfxMessage>& message);
    bool onHandleRequestForImsFakeMode(const sp<RfxMessage>& message);
    bool onHandleResponseForImsFakeMode(const sp<RfxMessage>& message);
    void resetImsFakeMode();
    void onAutoApproveTimer();

    void updateRQ(bool RQ);

    void onSendDelayCallInfoTimer();
    void onAutoFakeAcceptTimer();

    CallRat mCallRat;
    CallRat mImsCallRat;
    char* mEccNumber;
    char mEccNumberBuffer[MAX_ADDRESS_LEN + 1];
    int mEccState;
    enum {
        STATE_DIALING,
        STATE_ALERTING,
        STATE_ACTIVE,
        STATE_DISCONNECTED,
    };

    Vector<RfxPreciseCallState*>* mPreciseCallStateList;
    bool mUseLocalCallFailCause;
    int mDialLastError;
    bool mEspeechOnInDialingEcc;
    int mCsCallCount;

    // key = slot, value = call
    std::map<int, Vector<RfxImsCallInfo*>> mImsCalls;
    std::map<int, RfxImsCallInfo*> mEstablishingCall;
    std::map<int, sp<RfxMessage>> mPendingCallInfoForMT;
    std::map<int, bool> mCallRingIndicated;
    std::map<int, int> mSrvccState;
    sp<RfxMessage> mPendingCallControlMessage;
    int mPendingIncomingCallId;
    Vector<RfxImsCallInfo*> mToBeMergedCalls;
    int mAutoAnswerImsIncoming;
    int mAutoAnswerCallId;

    bool mWaitForSwitchCallResp;

    /* ALPS03346578: Emergency dial can be handled after receiving response of
        RFX_MSG_REQUEST_SET_ECC_MODE */
    bool mWaitForSetEccModeResponse;
    int mPendingSrvccCallCount;
    bool mNeedResumeOrAnswerCall;
    int mExpectedToResumeCallId;
    int mSpeechCodec;
    // For combo operations.
    int mCallIdToBeHeld;
    int mCallIdToBeHangup;
    int mCallIdExpectToResumeOrAnswer;
    int mCallIdResuming;
    int mCallIdAnswering;
    sp<RfxMessage> mPendingImsStkCallInfoMessage;
    sp<RfxMessage> mDelaySendCallInfoMessage;
    Vector<int> mCallIdsDisconnecting;

    bool handleDialAsEmergencyDial(const sp<RfxMessage>& message);
    void handleRedialEcc(const sp<RfxMessage>& urc);
    void handleEmergencyBearerSupportNotify(const sp<RfxMessage>& message);
    sp<RfxMessage> mOriginalDialMessage;
    sp<RfxMessage> mImsDialMessage;
    sp<RfxMessage> mHangupWaitingOrBgMessage;
    sp<RfxMessage> mHangupFgResumeBgMessage;
    sp<RfxMessage> mSwitchFgBgMessage;
    int mSeqNo;
    sp<RfxMessage> mOriginalAnswerMessage;
    sp<RfxMessage> mOriginalHangupMessage;
    bool mImsFakeMode;

    TimerHandle mAutoAnswerTimerHandle;
    TimerHandle mAutoApproveTimerHandle;
    static const int AUTO_ANSWER_NONE = 0;
    static const int AUTO_ANSWER_EM   = 1;
    static const int AUTO_ANSWER_FAKE = 2;

    TimerHandle mSendImsCallInfoTimerHandle;

    const char *PAU_NUMBER_FIELD     = "<tel:";
    const char *PAU_NAME_FIELD       = "<name:";
    const char *PAU_SIP_NUMBER_FIELD = "<sip:";
    const char *PAU_END_FLAG_FIELD    = ">";

    const int SRVCC_STATE_NONE      = -1;
    const int SRVCC_STATE_STARTED   = 0;
    const int SRVCC_STATE_COMPLETED = 1;
    const int SRVCC_STATE_FAILED    = 2;
    const int SRVCC_STATE_CANCELED  = 3;

    // Convert CHLD=0 to CHLD=1x for hanging up ringing call
    sp<RfxMessage> mOriginalHangupRingingMessage;
    int findRingingConnection();

    bool mRQ;

    TimerHandle mAutoFakeAcceptTimerHandle;
};

#endif /* __RFX_CALL_CONTROLLER_H__ */
