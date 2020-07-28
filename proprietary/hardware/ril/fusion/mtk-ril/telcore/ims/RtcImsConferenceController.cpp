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

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RtcImsConferenceController.h"
#include "RtcImsConferenceCallMessageHandler.h"
#include "RfxRootController.h"
#include "cc/RtcCallController.h"

#include "RfxImsCallInfo.h"
#include "RfxImsConfData.h"
#include "RfxDialogInfoData.h"
#include "RfxIntsData.h"
#include "RfxRilUtils.h"
#include "RfxStringData.h"
#include "RfxStringsData.h"
#include "RfxVoidData.h"
#include "rfx_properties.h"
#include "cc/RtcCallController.h"

/*****************************************************************************
 * Class RfxController
 *****************************************************************************/

#define RFX_LOG_TAG "RtcImsEvent"
#define MIN_IMS_CALL_MODE 20

RFX_IMPLEMENT_CLASS("RtcImsConferenceController", RtcImsConferenceController, RfxController);

// register request to RfxData
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringsData, RfxVoidData, RFX_MSG_REQUEST_ADD_IMS_CONFERENCE_CALL_MEMBER);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringsData, RfxVoidData, RFX_MSG_REQUEST_REMOVE_IMS_CONFERENCE_CALL_MEMBER);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxVoidData, RFX_MSG_REQUEST_CONFERENCE);

RFX_REGISTER_DATA_TO_URC_ID(RfxStringsData, RFX_MSG_URC_IMS_EVENT_PACKAGE_INDICATION);
RFX_REGISTER_DATA_TO_URC_ID(RfxImsConfData, RFX_MSG_UNSOL_IMS_CONFERENCE_INFO_INDICATION);
RFX_REGISTER_DATA_TO_URC_ID(RfxStringsData, RFX_MSG_UNSOL_LTE_MESSAGE_WAITING_INDICATION);
RFX_REGISTER_DATA_TO_URC_ID(RfxDialogInfoData, RFX_MSG_URC_IMS_DIALOG_INDICATION);


RtcImsConferenceController::RtcImsConferenceController() : mNormalCallsMerge(false),
        mInviteByNumber(false), mIsMerging(false), mEconfCount(0),
        mFakeDisconnectedCallProcessingCount(0) {
}

RtcImsConferenceController::~RtcImsConferenceController() {
}

void RtcImsConferenceController::onInit() {
    // Required: invoke super class implementation
    RfxController::onInit();

    mRtcImsConferenceHandler = new RtcImsConferenceHandler(getSlotId());
    mRtcImsDialogHandler = new RtcImsDialogHandler(getSlotId());

    const int request_id_list[] = {
        RFX_MSG_REQUEST_ADD_IMS_CONFERENCE_CALL_MEMBER,
        RFX_MSG_REQUEST_REMOVE_IMS_CONFERENCE_CALL_MEMBER,
        RFX_MSG_REQUEST_CONFERENCE,
    };

    const int urc_id_list[] = {
        RFX_MSG_URC_IMS_EVENT_PACKAGE_INDICATION,
    };

    // register request & URC id list
    // NOTE. one id can only be registered by one controller
    registerToHandleRequest(request_id_list, sizeof(request_id_list)/sizeof(const int));
    registerToHandleUrc(urc_id_list, sizeof(urc_id_list)/sizeof(const int));
}

void RtcImsConferenceController::onDeinit() {
    logD(RFX_LOG_TAG, "onDeinit");
    RfxController::onDeinit();
    delete mRtcImsConferenceHandler;
    mRtcImsConferenceHandler = NULL;
    delete mRtcImsDialogHandler;
    mRtcImsDialogHandler = NULL;
}

bool RtcImsConferenceController::onHandleRequest(const sp<RfxMessage>& message) {
    return handleMessgae(message);
}

bool RtcImsConferenceController::onHandleUrc(const sp<RfxMessage>& message) {
    return handleUrc(message);
}

bool RtcImsConferenceController::onHandleResponse(const sp<RfxMessage>& message) {
    return handleResponse(message);
}

bool RtcImsConferenceController::onCheckIfRejectMessage(
        const sp<RfxMessage>& message, bool isModemPowerOff, int radioState) {

    int isWfcSupport = RfxRilUtils::isWfcSupport();

    if (isWfcSupport == 1 && !isModemPowerOff) {
        return false;
    }

    return RfxController::onCheckIfRejectMessage(message, isModemPowerOff, radioState);
}

bool RtcImsConferenceController::handleMessgae(const sp<RfxMessage>& message) {
    int msg_id = message->getId();
    //logD(RFX_LOG_TAG, "onHandleRequest: %s", RFX_ID_TO_STR(msg_id));
    bool needSendRequestToMcl = true;

    switch (msg_id) {
        case RFX_MSG_REQUEST_ADD_IMS_CONFERENCE_CALL_MEMBER:
            handleAddMember(message);
            break;
        case RFX_MSG_REQUEST_REMOVE_IMS_CONFERENCE_CALL_MEMBER:
            needSendRequestToMcl = false;
            handleRemoveMember(message);
            break;
        case RFX_MSG_REQUEST_CONFERENCE:
            handleMergeConferenece(message);
            if (mOriginalMergeMessage != NULL) {
                needSendRequestToMcl = false;
            }
            break;
        default:
            break;
    }
    if (needSendRequestToMcl) {
        requestToMcl(message);
    }
    return true;
}

bool RtcImsConferenceController::handleResponse(const sp<RfxMessage>& message) {
    int msg_id = message->getId();
    //logD(RFX_LOG_TAG, "onHandleResponse: %s", RFX_ID_TO_STR(msg_id));
    bool needSendRespone = true;
    switch (msg_id) {
        case RFX_MSG_REQUEST_ADD_IMS_CONFERENCE_CALL_MEMBER:
        case RFX_MSG_REQUEST_CONFERENCE:
            needSendRespone = handleAddMemberResponse(message);
            break;
        case RFX_MSG_REQUEST_REMOVE_IMS_CONFERENCE_CALL_MEMBER:
            handleRemoveMemberResponse(message);
            break;
        default:
            break;
    }
    if (needSendRespone) {
        responseToRilj(message);
    }
    return true;
}

bool RtcImsConferenceController::handleUrc(const sp<RfxMessage>& message) {
    int msg_id = message->getId();
    RfxStringsData* data = (RfxStringsData*)message->getData();
    char** params = (char**)data->getData();
    logD(RFX_LOG_TAG, "onHandleUrc: %s", RFX_ID_TO_STR(msg_id));
    bool ret = false;
    switch (msg_id) {
        case RFX_MSG_UNSOL_CALL_INFO_INDICATION:
            handleImsCallInfoUpdate(message);
            break;
        case RFX_MSG_UNSOL_ECONF_RESULT_INDICATION:
            handleEconfUpdate(message);
            break;
        case RFX_MSG_URC_IMS_EVENT_PACKAGE_INDICATION:
            handleImsEventPackageIndication(message);
            ret = true;
            break;
        default:
            break;
    }
    return ret;
}

void RtcImsConferenceController::handleConferenceStart() {
    if (mRtcImsConferenceHandler != NULL) {
        mRtcImsConferenceHandler->startConference();
    }
}

void RtcImsConferenceController::handleOneKeyConference(const sp<RfxMessage>& message) {
    RfxStringsData* data = (RfxStringsData*)message->getData();
    char** params = (char**)data->getData();
    int numOfMembers = atoi(params[1]);
    // get number sequence
    int offSet = 2;
    vector<string> members;
    for (int i = 0; i < numOfMembers; ++i) {
        char* addr = params[i + offSet];
        if (addr != NULL) {
            string memberNum(addr);
            members.push_back(memberNum);
        }
    }
    if (mRtcImsConferenceHandler != NULL) {
        mRtcImsConferenceHandler->tryOneKeyAddLocalList(members);
        mRtcImsConferenceHandler->startConference();
    }
}

void RtcImsConferenceController::handleOneKeyConferenceFail(const sp<RfxMessage>& message) {
    RFX_UNUSED(message);

    if (mRtcImsConferenceHandler != NULL) {
        mRtcImsConferenceHandler->closeConference();
    }
    mCachedCEPMessage = NULL;
}

void RtcImsConferenceController::handleAddMember(const sp<RfxMessage>& message) {
    int msg_id = message->getId();
    RfxStringsData* data = (RfxStringsData*)message->getData();
    char** params = (char**)data->getData();
    char* address = params[1];
    int callId = atoi(params[2]);
    Vector<RfxImsCallInfo*> calls =
            getStatusManager(m_slot_id)->getImsCallInfosValue(RFX_STATUS_KEY_IMS_CALL_LIST);
    logD(RFX_LOG_TAG, "handleAddMember: %d", callId);

    if(callId == -1) {  // One-key add member
        mInviteByNumber = true;
        if (mRtcImsConferenceHandler != NULL && address != NULL) {
            mRtcImsConferenceHandler->tryOneKeyAddParticipant(address);
        }
    } else {  // normal add member by merge
        mIsMerging = true;
        mIsAddingMember = true;
        for (int i = 0; i < (int)calls.size(); i++) {
            if (calls[i]->getCallId() == callId && calls[i]->isConference() == false) {
                if (mRtcImsConferenceHandler != NULL && address != NULL) {
                    mRtcImsConferenceHandler->tryAddParticipant(address);
                }
            }
        }
    }
}

void RtcImsConferenceController::handleRemoveMember(const sp<RfxMessage>& message) {
    int msg_id = message->getId();
    RfxStringsData* data = (RfxStringsData*)message->getData();
    char** params = (char**)data->getData();
    char* address = params[1];
    char **newData = (char **) calloc(4, sizeof(char *));
    string newAddressStr;
    string retryAddressStr;
    if (newData == NULL) {
        logE(RFX_LOG_TAG, "handleRemoveMember, newData calloc fail!");
        sp<RfxMessage> responseMsg = RfxMessage::obtainResponse(RIL_E_NO_MEMORY, message, false);
        responseToRilj(responseMsg);
        return;
    }

    if (mRtcImsConferenceHandler != NULL && address != NULL) {
        mRtcImsConferenceHandler->tryRemoveParticipant(address);
        newAddressStr = mRtcImsConferenceHandler->getConfParticipantUri(address, false);
        retryAddressStr = mRtcImsConferenceHandler->getConfParticipantUri(address, true);
    }
    newData[0] = strdup(params[0]);
    newData[1] = strdup(newAddressStr.c_str());
    newData[2] = strdup(params[2]);
    newData[3] = strdup(retryAddressStr.c_str());

    mIsRemovingMember = true;
    sp<RfxMessage> request = RfxMessage::obtainRequest(msg_id,
            RfxStringsData(newData, 4), message, false);
    requestToMcl(request);
    for (int i = 0; i < 4; i++) {
        if (newData[i] != NULL) {
            free(newData[i]);
        }
    }
    free(newData);
}

void RtcImsConferenceController::handleMergeConferenece(const sp<RfxMessage>& message) {
    mIsMerging = true;
    Vector<RfxImsCallInfo*> calls =
            getStatusManager(m_slot_id)->getImsCallInfosValue(RFX_STATUS_KEY_IMS_CALL_LIST);

    int confCallId = 0;
    Vector<string> numbers;
    Vector<string> callIds;

  for (int i = 0; i < (int)calls.size(); i++) {
        if (calls[i]->isConferenceHost() == true) {
            confCallId = calls[i]->getCallId();
        }
        if (calls[i]->isConference() == false) {
            numbers.push_back(calls[i]->getNumber().string());
            callIds.push_back(to_string(calls[i]->getCallId()));
        }
    }

    // Handle Confernece call & Normal call merge.
    if (confCallId > 0 && callIds.size() > 0) {
        char **newData = (char **) calloc(3, sizeof(char *));
        if (newData == NULL) {
            logE(RFX_LOG_TAG, "handleMergeConferenece, newData calloc fail!");
            sp<RfxMessage> responseMsg = RfxMessage::obtainResponse(RIL_E_NO_MEMORY, message, false);
            responseToRilj(responseMsg);
            return;
        }
        newData[0] = strdup(to_string(confCallId).c_str());
        newData[1] = strdup(numbers[0].c_str());
        newData[2] = strdup(callIds[0].c_str());
        sp<RfxMessage> request = RfxMessage::obtainRequest(
                RFX_MSG_REQUEST_ADD_IMS_CONFERENCE_CALL_MEMBER,
                RfxStringsData(newData, 3), message, false);
        handleAddMember(request);
        mOriginalMergeMessage = RfxMessage::obtainRequest(
                m_slot_id, RFX_MSG_REQUEST_CONFERENCE, message);
        requestToMcl(request);

        for (int i = 0; i < 3; i++) {
            if (newData[i] != NULL) {
                free(newData[i]);
            }
        }
        free(newData);
        return;
    } else {
        // Handle Normal call & Normal call merge.
        mNormalCallsMerge = true;
        mEconfCount = 0;
        Vector<string> numbers;
        for (int i = 0; i < (int)calls.size(); i++) {
            RfxImsCallInfo* call = calls[i];
            if (call->isConference() == false) {
                numbers.push_back(call->getNumber().string());
            }
            if (call->getCallState() == RfxImsCallInfo::STATE_ESTABLISHED) {
                mActiveCallIdBeforeMerge = call->getCallId();
            }
        }
        if (mRtcImsConferenceHandler != NULL && numbers.size() >= 2) {
            mRtcImsConferenceHandler->firstMerge(callIds[0], callIds[1], numbers[0], numbers[1]);
        }
    }
}

void RtcImsConferenceController::handleImsEventPackageIndication(const sp<RfxMessage>& message) {
    /*
     * +EIMSEVTPKG: <call_id>,<type>,<urc_index>,<total_urc_count>,<data>
     * <call_id>:  0~255
     * <type>: 1 = Conference Event Package; 2 = Dialog Event Package; 3 = Message Waiting Event Package
     * <urc_index>: 1~255, the index of URC part
     * <total_urc_count>: 1~255
     * <data>: xml raw data, max length = 1950
     */

    RfxStringsData* data = (RfxStringsData*)message->getData();
    char** params = (char**)data->getData();
    int msgType = atoi(params[1]);
    int index = atoi(params[2]);
    int count = atoi(params[3]);

    if (msgType == 1) {
        // For multiple Urc case, only cache the last.
        if ((mIsAddingMember || mIsRemovingMember) && index == count) {
            logD(RFX_LOG_TAG, "handleImsEventPackageIndication: cache CEP");
            mCachedCEPMessage = message;
        } else {
            mRtcImsConferenceHandler->handleImsConfCallMessage(message);
        }
        return;
    } else if (msgType == 2) {
        mRtcImsDialogHandler->handleImsDialogMessage(message);
        return;
    } else if (msgType == 3) {
        sp<RfxMessage> msg = RfxMessage::obtainUrc(message->getSlotId(),
                RFX_MSG_UNSOL_LTE_MESSAGE_WAITING_INDICATION, message, true);
        responseToRilj(msg);
    }
}

void RtcImsConferenceController::handleImsCallInfoUpdate(const sp<RfxMessage>& message) {
    /* +ECPI: <call_id>, <msg_type>, <is_ibt>,
    *        <is_tch>, <dir>, <call_mode>, <number>, <type>, "<pau>", [<cause>] */
    RfxStringsData* data = (RfxStringsData*)message->getData();
    char** params = (char**)data->getData();
    int callId = atoi(params[0]);
    char* number = params[6];
    int msgType = atoi(params[1]);
    if (msgType == 133) {
        Vector<RfxImsCallInfo*> calls = getStatusManager(m_slot_id)->getImsCallInfosValue(
                RFX_STATUS_KEY_IMS_CALL_LIST);
        int count = (int)calls.size();
        for (int i = 0; i < count; i++) {
            if (calls[i]->getCallId() == callId && calls[i]->isConferenceHost() == true) {
                mCachedCEPMessage = NULL;
                mIsAddingMember = false;
                mIsRemovingMember = false;
                if (mRtcImsConferenceHandler != NULL) {
                    mRtcImsConferenceHandler->closeConference();
                    mFakeDisconnectedCallProcessingCount = 0;
                    mFakeDisconnectedCallIdList.clear();
                    return;
                }
            }
        }
        if (count == 0 || (count == 1 && calls[0]->getCallId() == callId)) {
            mCachedCEPMessage = NULL;
            mIsAddingMember = false;
            mIsRemovingMember = false;
            if (mRtcImsConferenceHandler != NULL) {
                mRtcImsConferenceHandler->closeConference();
                mFakeDisconnectedCallProcessingCount = 0;
                mFakeDisconnectedCallIdList.clear();
            }
        }
    }
}

void RtcImsConferenceController::handleEconfUpdate(const sp<RfxMessage>& message) {
    // +ECONF:<conf_call_id>,<op>,<num>,<result>,<cause>[,<joined_call_id>]
    RfxStringsData* data = (RfxStringsData*)message->getData();
    char** params = (char**)data->getData();
    int result = atoi(params[3]);
    char* joined_call_id = params[5];

    if (mNormalCallsMerge) {
        mEconfCount++;
        if (result == 0 && mRtcImsConferenceHandler != NULL && joined_call_id != NULL) {
            mRtcImsConferenceHandler->addFirstMergeParticipant(joined_call_id);
        }
        if (mEconfCount == 2) {
            if (mRtcImsConferenceHandler != NULL) {
                if (result == 0) {
                    handleSpecificConferenceMode();
                    modifyParticipantComplete();
                } else {
                    mRtcImsConferenceHandler->modifyParticipantFailed();
                }
            }
            mEconfCount = 0;
            mNormalCallsMerge = false;
            mIsMerging = false;
        }
    } else {
        mIsMerging = false;
        if (mRtcImsConferenceHandler != NULL) {
            if (result == 0) {
                modifyParticipantComplete();
            } else {
                mRtcImsConferenceHandler->modifyParticipantFailed();
            }
        }
        tryhandleCachedCEP();
    }
}

bool RtcImsConferenceController::handleAddMemberResponse(const sp<RfxMessage>& response) {
    RtcCallController *ctrl = (RtcCallController *) findController(m_slot_id,
                    RFX_OBJ_CLASS_INFO(RtcCallController));
    bool needSendRespone = true;

    if (response->getError() != RIL_E_SUCCESS) {
        tryhandleCachedCEP();
        mInviteByNumber = false;
        mIsMerging = false;
        if (mRtcImsConferenceHandler != NULL) {
            mRtcImsConferenceHandler->modifyParticipantFailed();
        }
        needSendRespone = tryhandleAddMemberByMerge(response);
        if (mNormalCallsMerge) {
            ctrl->retrieveMergeFail(mActiveCallIdBeforeMerge);
            mNormalCallsMerge = false;
            mActiveCallIdBeforeMerge = 0;
        }
        return needSendRespone;
    }
    if (mInviteByNumber) {
        mInviteByNumber = false;
        // modifyParticipantComplete();
    } else {
        ctrl->onMergeStart();
    }
    needSendRespone = tryhandleAddMemberByMerge(response);
    return needSendRespone;
}

void RtcImsConferenceController::handleRemoveMemberResponse(const sp<RfxMessage>& response) {
    if (response->getError() != RIL_E_SUCCESS) {
        tryhandleCachedCEP();
        mIsMerging = false;
        if (mRtcImsConferenceHandler != NULL) {
            mRtcImsConferenceHandler->modifyParticipantFailed();
        }
        return;
    }

    if (mRtcImsConferenceHandler != NULL) {
        modifyParticipantComplete();
    }
    tryhandleCachedCEP();
}

void RtcImsConferenceController::modifyParticipantComplete() {
    bool ret = mRtcImsConferenceHandler->modifyParticipantComplete();
    if (ret) {
        onParticipantsUpdate(false);
    }
    logD(RFX_LOG_TAG, "start  Timer");
    mTimeoutHandle = RfxTimer::start(RfxCallback0(this,
            &RtcImsConferenceController::onTimeout), ms2ns(5000));
}

void RtcImsConferenceController::onTimeout() {
    logD(RFX_LOG_TAG, "onTimeout");

    // check if need special handling for conference
    RfxStatusKeyEnum key = RFX_STATUS_KEY_IMS_SPECIFIC_CONFERENCE_MODE;
    if (getStatusManager(m_slot_id)->getString8Value(key) == "1") {
        logD(RFX_LOG_TAG, "onTimeout, KDDI case and no need update paticipant to hide manage menu");
        return;
    }

    if(mRtcImsConferenceHandler != NULL) {
        mRtcImsConferenceHandler->updateConferenceStateWithLocalCache();
    }
}

void RtcImsConferenceController::tryhandleCachedCEP() {
    if (mIsAddingMember || mIsRemovingMember) {
        if (mIsRemovingMember) {
            mIsRemovingMember = false;
        }
        if (mIsAddingMember) {
            mIsAddingMember = false;
        }
        if (mCachedCEPMessage != NULL) {
            if (mRtcImsConferenceHandler != NULL) {
                mRtcImsConferenceHandler->handleImsConfCallMessage(mCachedCEPMessage);
            }
            mCachedCEPMessage = NULL;
        }
    }
}

bool RtcImsConferenceController::tryhandleAddMemberByMerge(const sp<RfxMessage>& response) {
    if (mOriginalMergeMessage != NULL) {
        sp<RfxMessage> newResponse = RfxMessage::obtainResponse(response->getError(),
                mOriginalMergeMessage, false);
        responseToRilj(newResponse);
        mOriginalMergeMessage = NULL;
        return false;
    }
    return true;
}

void RtcImsConferenceController::onParticipantsUpdate(bool autoTerminate) {
    vector<sp<ConferenceCallUser>> users =
            mRtcImsConferenceHandler->getConfParticipantsInfo();
    RtcCallController *ctrl =
            (RtcCallController *) findController(m_slot_id,
                    RFX_OBJ_CLASS_INFO(RtcCallController));
    ctrl->onParticipantsUpdate(users, autoTerminate);

    int count = (int)users.size();
    logD(RFX_LOG_TAG, "count %d, autoTerminate %d", count, autoTerminate);

    if (autoTerminate) return;

    RIL_Conference_Participants* participants = (RIL_Conference_Participants*)
            calloc(1, sizeof(RIL_Conference_Participants) * count);
    if (participants == NULL) {
        logE(RFX_LOG_TAG, "participants calloc fail!");
        return;
    }

    for(int i = 0; i < count; i++) {
        sp<ConferenceCallUser> user = users[i];
        int len = strlen(user->mDisplayText.c_str());
        participants[i].display_text = (char*) calloc(1, len + 1);
        strncpy(participants[i].display_text, user->mDisplayText.c_str(), len);
        len = strlen(user->mEndPoint.c_str());
        participants[i].end_point    = (char*) calloc(1, len + 1);
        strncpy(participants[i].end_point, user->mEndPoint.c_str(), len);
        len = strlen(user->mEntity.c_str());
        participants[i].entity       = (char*) calloc(1, len + 1);
        strncpy(participants[i].entity, user->mEntity.c_str(), len);
        len = strlen(user->mStatus.c_str());
        participants[i].status       = (char*) calloc(1, len + 1);
        strncpy(participants[i].status, user->mStatus.c_str(), len);
        len = strlen(user->mUserAddr.c_str());
        participants[i].useraddr     = (char*) calloc(1, len + 1);
        strncpy(participants[i].useraddr, user->mUserAddr.c_str(), len);
    }
    sp<RfxMessage> msg = RfxMessage::obtainUrc(getSlotId(),
            RFX_MSG_UNSOL_IMS_CONFERENCE_INFO_INDICATION,
            RfxImsConfData((void*)participants, sizeof(RIL_Conference_Participants) * count));
    if (participants != NULL) {
        for (int i = 0; i < count; i++) {
            free(participants[i].useraddr);
            free(participants[i].end_point);
            free(participants[i].display_text);
            free(participants[i].status);
            free(participants[i].entity);
        }
        free(participants);
    }

    responseToRilj(msg);
}

void RtcImsConferenceController::handleSpecificConferenceMode() {
    // check if need special handling for conference
    RfxStatusKeyEnum key = RFX_STATUS_KEY_IMS_SPECIFIC_CONFERENCE_MODE;

    if (getStatusManager(m_slot_id)->getString8Value(key) == "1") {
        logD(RFX_LOG_TAG, "handleEconfUpdate(), special conference mode with mode = 1");

        // KDDI conference handling:
        //   KDDI conference won't terminate the participant after merge complete.
        //   Because user cannot see or do anything for these calls,
        //   AP will just view as terminated and skip the following event for them

        RfxStatusKeyEnum key2 = RFX_STATUS_KEY_IMS_CALL_LIST;
        Vector<RfxImsCallInfo*> calls = getStatusManager(m_slot_id)->getImsCallInfosValue(key2);

        int count = (int)calls.size();

        for (int i = 0; i < count; i++) {
            // +ECPI:<call_id>, <msg_type>, <is_ibt>, <is_tch>, <dir>, <call_mode>[, <number>, <toa>], "",<cause>

            if (calls[i]->isConference() == false) {
                int maxLen = 10;
                char *params[maxLen];
                int callId = calls[i]->getCallId();
                String8 callNumber = calls[i]->getNumber();

                logD(RFX_LOG_TAG, "handleSpecificConferenceMode(), fake disconnect for call id = %d, number = %s",
                        callId, RfxRilUtils::pii(RFX_LOG_TAG, callNumber.string()));

                params[0] = (char *) alloca(3);
                sprintf(params[0], "%d", callId);

                params[1] = (char *) alloca(4);
                sprintf(params[1], "%d", 133);

                // for 133 case, these param is no need
                for (int j=2; j < 5; j++) {
                    params[j] = (char *) alloca(2);
                    sprintf(params[j], "%d", 0);
                }

                // if KDDI support ViLTE, need to chang this
                params[5] = (char *) alloca(3);
                sprintf(params[5], "%d", 20);

                params[6] = (char *) alloca(strlen(callNumber.string()) + 1);
                sprintf(params[6], "%s", callNumber.string());

                // for 133 case, these param is no need
                for (int j=7; j < 10; j++) {
                    params[j] = (char *) alloca(1);
                    sprintf(params[j], "");
                }

                mFakeDisconnectedCallProcessingCount++;
                mFakeDisconnectedCallIdList.push_back(callId);

                RfxStringsData data(params, maxLen);
                sp<RfxMessage> urcToRilj = RfxMessage::obtainUrc(
                                            m_slot_id,
                                            RFX_MSG_UNSOL_CALL_INFO_INDICATION,
                                            data);

                RtcCallController *imsCallCtrl = (RtcCallController *) findController(
                        m_slot_id, RFX_OBJ_CLASS_INFO(RtcCallController));

                imsCallCtrl->notifyCallInfoUpdate(urcToRilj);
            }
        }
    }
}

bool RtcImsConferenceController::needProcessFakeDisconnect() {
    logD(RFX_LOG_TAG, "needProcessFakeDisconnect(),  mFakeDisconnectedCallProcessingCount = %d",
            mFakeDisconnectedCallProcessingCount);

    return (mFakeDisconnectedCallProcessingCount > 0);
}

void RtcImsConferenceController::processFakeDisconnectDone() {
    logD(RFX_LOG_TAG, "processFakeDisconnectDone(),  mFakeDisconnectedCallProcessingCount = %d",
            mFakeDisconnectedCallProcessingCount);

    mFakeDisconnectedCallProcessingCount--;
}

bool RtcImsConferenceController::needSkipDueToFakeDisconnect(int callid) {
    logD(RFX_LOG_TAG, "needSkipDueToFakeDisconnect(),  callid = %d", callid);

    int listSize = mFakeDisconnectedCallIdList.size();
    for (int i = 0; i < listSize; i++) {

        int item = mFakeDisconnectedCallIdList.itemAt(i);
        if (item == callid) {
            logD(RFX_LOG_TAG, "needSkipDueToFakeDisconnect(), Need to skip");
            return true;
        }
    }
    logD(RFX_LOG_TAG, "needSkipDueToFakeDisconnect(), No Need to skip");
    return false;
}
/* IMS Call End */
