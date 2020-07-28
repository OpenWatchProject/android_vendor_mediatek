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

#ifndef __RFX_IMS_CALL_INFO_H
#define __RFX_IMS_CALL_INFO_H

/*****************************************************************************
 * Include
 *****************************************************************************/
#include <vector>

#include "Rfx.h"
#include "utils/String8.h"
#include "utils/Vector.h"

using ::android::String8;
using ::android::Vector;

/*****************************************************************************
 * Class RfxImsCallInfo
 *****************************************************************************/
class RfxImsCallInfo {
public:

    enum {
        STATE_ESTABLISHING,
        STATE_DIALING,
        STATE_ALERTING,
        STATE_ESTABLISHED,
        STATE_HELD,
        STATE_INCOMING,
        STATE_WAITING,
        STATE_TERMINATING,
        STATE_TERMINATED,
    };

    enum {
       IMS_VOICE = 20,
       IMS_VIDEO = 21,
       IMS_VOICE_CONF = 22,
       IMS_VIDEO_CONF = 23,
       IMS_VOICE_CONF_PARTS = 24,
       IMS_VIDEO_CONF_PARTS = 25,
    };

    RfxImsCallInfo();

    RfxImsCallInfo(int callId, int callState,
            String8 number, bool isConference, bool isConferenceHost, bool isEcc, bool isVideo, bool isMT);
    explicit RfxImsCallInfo(RfxImsCallInfo* call);

    ~RfxImsCallInfo();

public:

    int getCallId();
    int getCallState();
    String8 getNumber();
    bool isConference();
    bool isConferenceHost();
    bool isEcc();
    bool isVideo();
    bool isMT();
    String8 getName();
    Vector<RfxImsCallInfo*> getParticipantList();
    void setCallId(int callId);
    void setCallState(int callState);
    void setNumber(String8 number);
    void setIsConference(bool isConference);
    void setIsConferenceHost(bool isConferenceHost);
    void setIsEcc(bool isEcc);
    void setIsVideo(bool isVideo);
    void setIsMT(bool isMT);
    void setName(String8 name);
    String8 toString();
    RIL_CallState getRilCallState();
    bool updateByCallMode(int callMode);
    void addParticipant(RfxImsCallInfo* call);
    void removeParticipant(int index);
    void removeParticipant(RfxImsCallInfo* call);
    int getCallMode();

private:

    int mCallId;
    int mCallState;
    String8 mNumber;
    bool mIsConference;
    bool mIsConferenceHost;
    bool mIsEcc;
    bool mIsVideo;
    bool mIsMT;
    String8 mName;
    Vector<RfxImsCallInfo*> mConferenceParticipant;

public:

    bool equalTo(const RfxImsCallInfo &other) const {
        return (mCallId == other.mCallId)
                && (mCallState == other.mCallState)
                && (mNumber == other.mNumber)
                && (mIsConference == other.mIsConference)
                && (mIsConferenceHost == other.mIsConferenceHost)
                && (mIsEcc == other.mIsEcc)
                && (mIsVideo == other.mIsVideo)
                && (mIsMT == other.mIsMT)
                && (mName == other.mName);
    }

    RfxImsCallInfo &operator = (const RfxImsCallInfo &other) {
        mCallId = other.mCallId;
        mCallState = other.mCallState;
        mNumber = other.mNumber;
        mIsConference = other.mIsConference;
        mIsConferenceHost = other.mIsConferenceHost;
        mIsEcc = other.mIsEcc;
        mIsVideo = other.mIsVideo;
        mIsMT = other.mIsMT;
        mName = other.mName;
        return *this;
    }

    bool operator == (const RfxImsCallInfo &other) {
        return equalTo(other);
    }

    bool operator != (const RfxImsCallInfo &other) {
        return !equalTo(other);
    }
};

#endif /* __RFX_IMS_CALL_INFO_H */
