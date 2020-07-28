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
#include "SuplMsgDispatcher.h"
#include <list>
#include "SmsMessage.h"
#include "ConcentratedSms.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <unistd.h>
#include <cutils/jstring.h>
#include "RfxLog.h"

#define RFX_LOG_TAG   "SuplMsgDispatcher"

RFX_IMPLEMENT_CLASS("SuplMsgDispatcher", SuplMsgDispatcher, RfxObject);

int SuplMsgDispatcher::sSuplExistenceState = SUPL_EXISTENCE_UNKNOWN;
int SuplMsgDispatcher::sSuplExistenceDetectionCount = 0;
/*****************************************************************************
 * Class SuplMsgDispatcher
 *****************************************************************************/
void SuplMsgDispatcher::onDeinit() {
    list<ConcentratedSms*>::iterator iter;
    for (iter = mConcSmsList.begin(); iter != mConcSmsList.end(); iter++) {
        ConcentratedSms *concSms = *iter;
        RFX_OBJ_CLOSE(concSms);
    }
    mConcSmsList.clear();
}

void SuplMsgDispatcher::dispatchSuplMsg(string content) {
    int length = content.length();
    RFX_LOG_D(RFX_LOG_TAG, "dispatchSuplMsg is content length: %d", length);
    BYTE *pdu = PhoneNumberUtils::hexStringToBytes(content);
    if (pdu == NULL) {
        return;
    }
    SmsMessage *msg = SmsMessage::createFromPdu(pdu, length/2);
    RFX_LOG_D(RFX_LOG_TAG, "dispatchSuplMsg begin!");
    if (msg->isConcentratedSms()) {
        RFX_LOG_D(RFX_LOG_TAG, "dispatchSuplMsg is concentrated message!");
        if (msg->isWapush()) {
            // save
            RFX_LOG_D(RFX_LOG_TAG, "dispatchSuplMsg is wap push message!");
            ConcentratedSms *concSms = NULL;
            concSms = findConcSms(msg->getRefNumber());
            if (concSms != NULL) {
                RFX_LOG_D(RFX_LOG_TAG, "dispatchSuplMsg find existing wap push message!");
                concSms->addSegment(msg);
            } else {
                RFX_OBJ_CREATE_EX(concSms, ConcentratedSms, this, (msg));
                mConcSmsList.push_back(concSms);
                concSms->mTimeOutSignal.connect(this, &SuplMsgDispatcher::onConcSmsTimeout);
            }
            bool handled = notifyConcMsg2Mnl(concSms);
            if (handled) {
                mConcSmsList.remove(concSms);
                RFX_OBJ_CLOSE(concSms);
            }
        } else {
            RFX_LOG_D(RFX_LOG_TAG, "dispatchSuplMsg notify sms to mnl!");
            notifySms2Mnl(msg);
            delete msg;
        }
    } else {
        RFX_LOG_D(RFX_LOG_TAG, "dispatchSuplMsg not concentrated sms!");
        if (!notifySms2Mnl(msg)) {
            RFX_LOG_D(RFX_LOG_TAG, "dispatchSuplMsg not sms, check wap push!");
            if (msg->isWapush()) {
                RFX_LOG_D(RFX_LOG_TAG, "dispatchSuplMsg is wap push message!");
                WappushMessage *wapMsg =
                        new WappushMessage(msg->getUserData(), msg->getUserDataLength());
                wapMsg->parsePdu();
                notifyWappush2Mnl(wapMsg);
                delete wapMsg;
            }
        }
        // To do:
        // free memory
        delete msg;
    }
    return;
}

void SuplMsgDispatcher::onConcSmsTimeout(int ref) {
    RFX_LOG_D(RFX_LOG_TAG, "onConcSmsTimeout ref: %d", ref);
    ConcentratedSms *concSms = findConcSms(ref);
    if (concSms != NULL) {
        notifyConcMsg2Mnl(concSms);
        mConcSmsList.remove(concSms);
        RFX_OBJ_CLOSE(concSms);
    }
}

ConcentratedSms* SuplMsgDispatcher::findConcSms(int ref) {
    RFX_LOG_D(RFX_LOG_TAG, "findConcSms ref: %d", ref);
    list<ConcentratedSms*>::iterator iter;
    for (iter = mConcSmsList.begin(); iter != mConcSmsList.end(); iter++) {
        ConcentratedSms *concSms = *iter;
        RFX_LOG_D(RFX_LOG_TAG, "findConcSms getRefNumber: %d", concSms->getRefNumber());
        if (concSms->getRefNumber() == ref) {
            return concSms;
        }
    }
    return NULL;
}

bool SuplMsgDispatcher::notifySms2Mnl(SmsMessage *msg) {
    RFX_LOG_D(RFX_LOG_TAG, "notifySms2Mnl is sms for supl: %d", msg->isSmsForSUPL());
    if (msg->isSmsForSUPL()) {
        BYTE *data = msg->getUserData();
        int length = msg->getUserDataLength();
        hal2mnl_ni_message((char *)data, length);
        return true;
    }
    return false;
}

bool SuplMsgDispatcher::notifyWappush2Mnl(WappushMessage *wapMsg) {
    RFX_LOG_D(RFX_LOG_TAG, "notifyWappush2Mnl is wap push for supl: %d", wapMsg->isWapushForSUPL());
    if (wapMsg->isWapushForSUPL()) {
        BYTE *data = wapMsg->getUserData();
        int length = wapMsg->getUserDataLength();
        hal2mnl_ni_message((char *)data, length);
        return true;
    }
    return false;
}

bool SuplMsgDispatcher::notifyConcMsg2Mnl(ConcentratedSms* msg) {
    RFX_LOG_D(RFX_LOG_TAG, "notifyConcMsg2Mnl isAllSegmentsReceived: %d",
        msg->isAllSegmentsReceived());
    if (msg->isAllSegmentsReceived()) {
        if (msg->isWappush()) {
            RFX_LOG_D(RFX_LOG_TAG, "notifyConcMsg2Mnl isWappush: %d", msg->isWappush());
            msg->parseWappushPdu();
            if (msg->isWapushForSUPL()) {
                RFX_LOG_D(RFX_LOG_TAG, "notifyConcMsg2Mnl isWapushForSUPL: %d", msg->isWapushForSUPL());
                // notify AGPS
                BYTE* userData = msg->getWappushMsgUserData();
                int length = msg->getWappushMsgUserDataLength();
                hal2mnl_ni_message((char *)userData, length);
                return true;
            }
        } else {
            // to do:
        }
    }
    return false;
}

void SuplMsgDispatcher::hal2mnl_ni_message(char* msg, int len) {
    // LOGD("hal2mnl_ni_message  len=%d", len);
    char buff[HAL_MNL_BUFF_SIZE] = {0};
    int offset = 0;
    put_int(buff, &offset, HAL_MNL_INTERFACE_VERSION);
    put_int(buff, &offset, HAL2MNL_NI_MESSAGE);
    put_binary(buff, &offset, msg, len);
    int ret = safe_sendto(MTK_HAL2MNL, buff, offset);
    if (SUPL_EXISTENCE_UNKNOWN == sSuplExistenceState) {
        if (-1 == ret) {
            if (++sSuplExistenceDetectionCount > SUPL_EXISTENCE_MAX_DETECTION) {
                sSuplExistenceState = SUPL_EXISTENCE_NO;
                RFX_LOG_D(RFX_LOG_TAG, "ril2mnl_ni_message SuplExistence: no");
            }
        } else {
            sSuplExistenceState = SUPL_EXISTENCE_YES;
            RFX_LOG_D(RFX_LOG_TAG, "ril2mnl_ni_message SuplExistence: yes");
        }
    }
}

void SuplMsgDispatcher::put_byte(char* buff, int* offset, const char input) {
    *((char*)&buff[*offset]) = input;
    *offset += 1;
}

void SuplMsgDispatcher::put_short(char* buff, int* offset, const short input) {
    put_byte(buff, offset, input & 0xff);
    put_byte(buff, offset, (input >> 8) & 0xff);
}

void SuplMsgDispatcher::put_int(char* buff, int* offset, const int input) {
    put_short(buff, offset, input & 0xffff);
    put_short(buff, offset, (input >> 16) & 0xffff);
}

void SuplMsgDispatcher::put_binary(char* buff, int* offset, const char* input, const int len) {
    put_int(buff, offset, len);
    if (len > 0) {
        memcpy(&buff[*offset], input, len);
        *offset += len;
    }
}

int SuplMsgDispatcher::safe_sendto(const char* path, const char* buff, int len) {
    int ret = 0;
    RFX_LOG_D(RFX_LOG_TAG, "safe_sendto path: %s, len: %d", path, len);
    struct sockaddr_un addr;
    int retry = 10;
    int fd = socket(PF_LOCAL, SOCK_DGRAM, 0);
    if (fd < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "safe_sendto faled reason[%s]:%d", strerror(errno), errno);
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_path[0] = 0;
    memcpy(addr.sun_path + 1, path, strlen(path));
    addr.sun_family = AF_UNIX;

    while ((ret = sendto(fd, buff, len, 0,
        (const struct sockaddr *)&addr, sizeof(addr))) == -1) {
        RFX_LOG_E(RFX_LOG_TAG, "safe_sendto faled reason[%s]:%d", strerror(errno), errno);
        if (errno == EINTR) continue;
        if (errno == EAGAIN) {
            if (retry-- > 0) {
                usleep(100 * 1000);
                continue;
            }
        }
        break;
    }
    close(fd);

    return ret;
}

bool SuplMsgDispatcher::doesSuplExist(void) {
    return (SUPL_EXISTENCE_NO != sSuplExistenceState);
}
