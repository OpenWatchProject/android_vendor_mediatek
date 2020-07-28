/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
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
#include "RtcConcatSms.h"
#include "RtcSmsMessage.h"

/*****************************************************************************
 * Class RtcConCatSmsRoot
 *****************************************************************************/
RFX_IMPLEMENT_CLASS("RtcConCatSmsRoot", RtcConCatSmsRoot, RfxObject);

RtcConCatSmsSender* RtcConCatSmsRoot::getSmsSender(const String8 & address) {
    if (address.isEmpty()) {
       return NULL;
    }
    RfxObject *obj = getFirstChildObj();
    RtcConCatSmsSender *sender = NULL;
    while (obj != NULL) {
        sender = (RtcConCatSmsSender *)obj;
        if (sender->isSameAs(address)) {
            return sender;
        }
        obj = obj->getNextObj();
    }
    RFX_OBJ_CREATE_EX(sender, RtcConCatSmsSender, this, (address));
    return sender;
}

void RtcConCatSmsRoot::cleanUpObj() {
    RfxObject *obj = getFirstChildObj();
    RtcConCatSmsSender *sender = NULL;
    Vector<RtcConCatSmsSender *> cleanObj;
    while (obj != NULL) {
        sender = (RtcConCatSmsSender *)obj;
        sender->cleanUpObj();
        if (sender->getChildCount() == 0) {
            cleanObj.push(sender);
        }
        obj = obj->getNextObj();
    }
    Vector<RtcConCatSmsSender *>::iterator it;
    for (it = cleanObj.begin(); it != cleanObj.end(); it++) {
        RFX_OBJ_CLOSE(*it);
    }
}

/*****************************************************************************
 * RtcConCatSmsSender
 *****************************************************************************/
RFX_IMPLEMENT_CLASS("RtcConCatSmsSender", RtcConCatSmsSender, RfxObject);

RtcConCatSmsGroup *RtcConCatSmsSender::getSmsGroup(int refNumber, int messageCount) {
    if (refNumber < 0 || messageCount < 0) {
        return NULL;
    }
    RfxObject *obj = getFirstChildObj();
    RtcConCatSmsGroup *group = NULL;
    while (obj != NULL) {
        group = (RtcConCatSmsGroup *)obj;
        if (group->isSameAs(refNumber, messageCount)) {
            return group;
        }
        obj = obj->getNextObj();
    }
    RFX_OBJ_CREATE_EX(group, RtcConCatSmsGroup, this, (refNumber, messageCount));
    return group;
}

void RtcConCatSmsSender::cleanUpObj() {
    RfxObject *obj = getFirstChildObj();
    RtcConCatSmsGroup *group = NULL;
    Vector<RtcConCatSmsGroup *> cleanObj;
    while (obj != NULL) {
        group = (RtcConCatSmsGroup *)obj;
        if (group->allPartsReady() || group->isExpire()) {
            cleanObj.push(group);
        }
        obj = obj->getNextObj();
    }
    Vector<RtcConCatSmsGroup *>::iterator it;
    for (it = cleanObj.begin(); it != cleanObj.end(); it++) {
        RFX_OBJ_CLOSE(*it);
    }
}

/*****************************************************************************
 * RtcConCatSmsGroup
 *****************************************************************************/
RFX_IMPLEMENT_CLASS("RtcConCatSmsGroup", RtcConCatSmsGroup, RfxObject);

RtcConCatSmsPart *RtcConCatSmsGroup::getSmsPart(int seqNumber) {
    if (seqNumber <= 0 || seqNumber > mMessageCount) {
        return NULL;
    }
    RfxObject *obj = getFirstChildObj();
    RtcConCatSmsPart *part = NULL;
    while (obj != NULL) {
        part = (RtcConCatSmsPart *)obj;
        if (part->isSameAs(seqNumber)) {
            return part;
        }
        obj = obj->getNextObj();
    }
    RFX_OBJ_CREATE_EX(part, RtcConCatSmsPart, this, (seqNumber));
    return part;
}

bool RtcConCatSmsGroup::isExpire() {
    nsecs_t now = systemTime(SYSTEM_TIME_MONOTONIC);
    if ((now - mTimeStamp) > seconds_to_nanoseconds(60 * 3)) {
        return true;
    }
    return false;
}

/*****************************************************************************
 * RtcConCatSmsPart
 *****************************************************************************/
RFX_IMPLEMENT_CLASS("RtcConCatSmsPart", RtcConCatSmsPart, RfxObject);

RtcConCatSmsPart::~RtcConCatSmsPart() {
    if (mConvertedMessage != NULL) {
        delete mConvertedMessage;
    }
    if (mMessage != NULL) {
        delete mMessage;
    }
}

const RtcSmsMessage &RtcConCatSmsPart::getMessage()  const {
    if (mIs3Gpp) {
        return *((RtcGsmSmsMessage *)mMessage);
    } else  {
        return *((RtcCdmaSmsMessage *)mMessage);
    }
}

RtcSmsMessage *RtcConCatSmsPart::getConvertedMessage() {
    if (mConvertedMessage != NULL) {
        return mConvertedMessage;
    }
    RtcConCatSmsGroup *group = (RtcConCatSmsGroup *)getParent();
    if (group != NULL) {
        RtcConCatSmsPart *part = (RtcConCatSmsPart *)group->getFirstChildObj();
        if ((part != NULL) && (part != this) && (!part->isSameFormatAs(this))) {
            if (part->isFormat3Gpp()) {
                mConvertedMessage = new RtcGsmSmsMessage(
                        (const RtcGsmSmsMessage &)part->getMessage(),
                        getMessage().getEncodingType(),
                        getMessage().getNumField(),
                        getMessage().getUserDataPayload());
            } else {
                mConvertedMessage = new RtcCdmaSmsMessage(
                        (const RtcCdmaSmsMessage &)part->getMessage(),
                        getMessage().getEncodingType(),
                        getMessage().getNumField(),
                        getMessage().getUserDataPayload());
            }
        }
    }
    return mConvertedMessage;
}