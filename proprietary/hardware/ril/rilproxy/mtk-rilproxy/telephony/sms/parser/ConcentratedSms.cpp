/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2019. All rights reserved.
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
#include "PduParser.h"
#include "ConcentratedSms.h"
#include "SmsMessage.h"
#include "RfxLog.h"

#define RFX_LOG_TAG   "ConcentratedSms"

/*****************************************************************************
 * Class SmsHeader
 *****************************************************************************/
RFX_IMPLEMENT_CLASS("ConcentratedSms", ConcentratedSms, RfxObject);

ConcentratedSms::ConcentratedSms(SmsMessage *msg) :
        mWapPushUserData(NULL),
        mWappushMsg(NULL),
        mTimeoutHandle(NULL) {
    mTotalSeg = msg->getTotalSegnumber();
    mRefNumber = msg->getRefNumber();
    mIsWapPush = msg->isWapush();
    mMsgList.push_back(msg);
}

ConcentratedSms::ConcentratedSms() :
    mTotalSeg(0),
    mRefNumber(-1),
    mIsWapPush(false),
    mWapPushUserData(NULL),
    mWappushMsg(NULL),
    mTimeoutHandle(NULL) {
}

void ConcentratedSms::onInit() {
    RFX_LOG_D(RFX_LOG_TAG, "ConcentratedSms init start timer!");
    mTimeoutHandle = RfxTimer::start(RfxCallback0(this,
            &ConcentratedSms::onTimeout), ms2ns(60000));
}

void ConcentratedSms::onDeinit() {
    if (mWappushMsg != NULL) {
        delete mWappushMsg;
        mWappushMsg = NULL;
    }
    if (mTimeoutHandle != NULL) {
        RfxTimer::stop(mTimeoutHandle);
        mTimeoutHandle = NULL;
    }
    list<SmsMessage *>::iterator iter;
    for (iter = mMsgList.begin(); iter != mMsgList.end(); iter++) {
        SmsMessage *temp = *iter;
        if (temp != NULL) {
            delete temp;
        }
    }
    mMsgList.clear();
}
int ConcentratedSms::getRefNumber() {
    return mRefNumber;
}

void ConcentratedSms::addSegment(SmsMessage *msg) {
    list<SmsMessage *>::iterator iter;
    for (iter = mMsgList.begin(); iter != mMsgList.end(); iter++) {
        SmsMessage *sms = *iter;
        if ((sms->getRefNumber() == mRefNumber) &&
                (sms->getSeqNumber() == msg->getSeqNumber())) {
            // duplicated SMS, delete it.
            delete msg;
            return;
        }
    }
    mMsgList.push_back(msg);
    mMsgList.sort(ConcentratedSms::compareSms);
    return;
}

bool ConcentratedSms::isAllSegmentsReceived() {
    return (int)mMsgList.size() == mTotalSeg;
}

void ConcentratedSms::parseWappushPdu() {
    int pduLength = 0;
    for (list<SmsMessage *>::iterator iter = mMsgList.begin(); iter != mMsgList.end(); iter++) {
        SmsMessage *sms = *iter;
        pduLength += sms->getUserDataLength();
    }
    BYTE *wapPdu = new BYTE[pduLength];
    int offset = 0;
    for (list<SmsMessage *>::iterator iter = mMsgList.begin(); iter != mMsgList.end(); iter++) {
        SmsMessage *sms = *iter;
        memcpy(wapPdu + offset, sms->getUserData(), sms->getUserDataLength());
        offset +=  sms->getUserDataLength();
    }
    mWappushMsg = new WappushMessage(wapPdu, pduLength);
    mWappushMsg->parsePdu();
    delete[] wapPdu;
}

BYTE* ConcentratedSms::getWappushMsgUserData() {
    if (mWappushMsg != NULL) {
        return mWappushMsg->getUserData();
    }
    return NULL;
}

int ConcentratedSms::getWappushMsgUserDataLength() {
    if (mWappushMsg != NULL) {
        return mWappushMsg->getUserDataLength();
    }
    return 0;
}

bool ConcentratedSms::isWappush() {
    return mIsWapPush;
}

bool ConcentratedSms::isWapushForSUPL() {
    if (mWappushMsg != NULL) {
        return mWappushMsg->isWapushForSUPL();
    }
    return false;
}

bool ConcentratedSms::compareSms(SmsMessage *msg1, SmsMessage *msg2) {
    if (msg1->getSeqNumber() < msg2->getSeqNumber()) {
        return true;
    }
    return false;
}

void ConcentratedSms::onTimeout() {
    if (mTimeoutHandle != NULL) {
        RfxTimer::stop(mTimeoutHandle);
        mTimeoutHandle = NULL;
    }
    mTimeOutSignal.postEmit(mRefNumber);
}
