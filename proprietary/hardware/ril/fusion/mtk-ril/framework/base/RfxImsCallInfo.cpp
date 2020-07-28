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
#include "RfxImsCallInfo.h"
#include "RfxLog.h"
#include "RfxRilUtils.h"
#include "MtkPhoneNumberUtils.h"
#define RFX_LOG_TAG "RfxImsCallInfo"

/*****************************************************************************
 * Class RfxImsCallInfo
 *****************************************************************************/

RfxImsCallInfo::RfxImsCallInfo(): mCallId(0),
mCallState(STATE_TERMINATED), mNumber(""), mIsConference(false), mIsConferenceHost(false),
mIsEcc(false), mIsVideo(false), mIsMT(false), mName("") {
}

RfxImsCallInfo::RfxImsCallInfo(int callId, int callState,
        String8 number, bool isConference, bool isConferenceHost, bool isEcc, bool isVideo, bool isMT) {
    mCallId = callId;
    mNumber = number;
    mCallState = callState;
    mIsConference = isConference;
    mIsConferenceHost = isConferenceHost;
    mIsEcc = isEcc;
    mIsVideo = isVideo;
    mIsMT = isMT;
}

RfxImsCallInfo::RfxImsCallInfo(RfxImsCallInfo* call) {
    mCallId = call->getCallId();
    mNumber = call->getNumber();
    mCallState = call->getCallState();
    mIsConference = call->isConference();
    mIsConferenceHost = call->isConferenceHost();
    mIsEcc = call->isEcc();
    mIsVideo = call->isVideo();
    mIsMT = call->isMT();
    mName = call->getName();
}

RfxImsCallInfo::~RfxImsCallInfo() {
    for (int i = 0; i < (int)mConferenceParticipant.size(); i++) {
        RfxImsCallInfo* participant = mConferenceParticipant[i];
        mConferenceParticipant.erase(mConferenceParticipant.begin() + i);
        delete participant;
    }
}

int RfxImsCallInfo::getCallId() {
    return mCallId;
}

int RfxImsCallInfo::getCallState() {
    return mCallState;
}

String8 RfxImsCallInfo::getNumber() {
    return mNumber;
}

bool RfxImsCallInfo::isConference() {
    return mIsConference;
}

bool RfxImsCallInfo::isConferenceHost() {
    return mIsConferenceHost;
}

bool RfxImsCallInfo::isEcc() {
    return mIsEcc;
}

bool RfxImsCallInfo::isVideo() {
    return mIsVideo;
}

bool RfxImsCallInfo::isMT() {
    return mIsMT;
}

String8 RfxImsCallInfo::getName() {
    return mName;
}

Vector<RfxImsCallInfo*> RfxImsCallInfo::getParticipantList() {
    return mConferenceParticipant;
}

void RfxImsCallInfo::setCallId(int callId) {
    mCallId = callId;
}

void RfxImsCallInfo::setCallState(int callState) {
    mCallState = callState;
    for (int i = 0; i < (int)mConferenceParticipant.size(); i++) {
        RfxImsCallInfo* participant = mConferenceParticipant[i];
        participant->setCallState(callState);
    }
}

void RfxImsCallInfo::setNumber(String8 number) {
    mNumber = number;
}

void RfxImsCallInfo::setIsConference(bool isConference) {
    mIsConference = isConference;
}

void RfxImsCallInfo::setIsConferenceHost(bool isConferenceHost) {
    mIsConferenceHost = isConferenceHost;
}

void RfxImsCallInfo::setIsEcc(bool isEcc) {
    mIsEcc = isEcc;
}

void RfxImsCallInfo::setIsVideo(bool isVideo) {
    mIsVideo = isVideo;
}

void RfxImsCallInfo::setIsMT(bool isMT) {
    mIsMT = isMT;
}
void RfxImsCallInfo::setName(String8 name) {
    mName = name;
}

void RfxImsCallInfo::addParticipant(RfxImsCallInfo* call) {
    mConferenceParticipant.push_back(call);
}

void RfxImsCallInfo::removeParticipant(int index) {
    delete mConferenceParticipant[index];
    mConferenceParticipant.erase(mConferenceParticipant.begin() + index);
}

void RfxImsCallInfo::removeParticipant(RfxImsCallInfo* call) {
    for (int i = 0; i < (int)mConferenceParticipant.size(); i++) {
        RfxImsCallInfo* participant = mConferenceParticipant[i];
        if (*participant == *call) {
            RFX_LOG_D(RFX_LOG_TAG, "removeParticipant %s",
                    RfxRilUtils::pii(RFX_LOG_TAG, participant->getNumber().string()));
            mConferenceParticipant.erase(mConferenceParticipant.begin() + i);
            delete participant;
            // Expect to remove only 1 participant. NE may happen if remove more than
            // 1 participant in this loop.
            break;
        }
    }
}

String8 RfxImsCallInfo::toString() {
    return String8::format(
            "id=%d, state=%d, number=%s, conf=%d, confHost=%d, ecc=%d, video=%d, mt=%d, name=%s",
            mCallId, mCallState, RfxRilUtils::pii(RFX_LOG_TAG, mNumber.string()), mIsConference,
            mIsConferenceHost, mIsEcc, mIsVideo, mIsMT,
            RfxRilUtils::pii(RFX_LOG_TAG, mName.string()));
}

RIL_CallState RfxImsCallInfo::getRilCallState() {
    switch (mCallState) {
        case STATE_ESTABLISHING:
        case STATE_DIALING:
            return RIL_CALL_DIALING;
        case STATE_ALERTING:
            return RIL_CALL_ALERTING;
        case STATE_ESTABLISHED:
            return RIL_CALL_ACTIVE;
        case STATE_HELD:
            return RIL_CALL_HOLDING;
        case STATE_INCOMING:
            return RIL_CALL_INCOMING;
        case STATE_WAITING:
            return RIL_CALL_WAITING;
        default:
            RFX_LOG_E(RFX_LOG_TAG, "No matching call state");
            return RIL_CALL_ACTIVE;
    }
}

bool RfxImsCallInfo::updateByCallMode(int callMode) {
    int isVideo = false;
    int isConference = false;
    int isConferenceHost = false;
    bool changed = false;

    switch (callMode) {
        case IMS_VOICE:
            break;
        case IMS_VIDEO:
            isVideo = true;
            break;
        case IMS_VOICE_CONF:
            isConference = true;
            isConferenceHost = true;
            break;
        case IMS_VIDEO_CONF:
            isVideo = true;
            isConference = true;
            isConferenceHost = true;
            break;
        case IMS_VOICE_CONF_PARTS:
            isConference = true;
            break;
        case IMS_VIDEO_CONF_PARTS:
            isVideo = true;
            isConference = true;
            break;
        default:
            RFX_LOG_E(RFX_LOG_TAG, "No matching call mode");
            return false;
    }

    if (mIsVideo != isVideo) {
        mIsVideo = isVideo;
        changed = true;
    }

    if (mIsConference != isConference) {
        mIsConference = isConference;
        changed = true;
    }

    if (mIsConferenceHost != isConferenceHost) {
        mIsConferenceHost = isConferenceHost;
        changed = true;
    }

    return changed;
}

int RfxImsCallInfo::getCallMode() {
    if (mIsVideo) {
        if (mIsConference) {
            if (mIsConferenceHost) {
                return IMS_VIDEO_CONF;
            } else {
                return IMS_VIDEO_CONF_PARTS;
            }
        } else {
            return IMS_VIDEO;
        }
    } else {
        if (mIsConference) {
            if (mIsConferenceHost) {
                return IMS_VOICE_CONF;
            } else {
                return IMS_VOICE_CONF_PARTS;
            }
        } else {
            return IMS_VOICE;
        }
    }
}
