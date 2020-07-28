/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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

#include <mtk_log.h>
#include "WpfaDriver.h"
#include "WpfaCcciSender.h"
#include "WpfaDriverUtilis.h"


#define WPFA_D_LOG_TAG "WpfaCcciSender"

void WpfaCcciSender::CcciMsgHandler::handleMessage(const Message& message) {
    WPFA_UNUSED(message);
    sender->processMessage(msg);
}

WpfaCcciSender::WpfaCcciSender(int fd) : m_fd(fd) {
    mtkLogD(WPFA_D_LOG_TAG, "init");
    mNeedWaitLooper = true;
    sem_init(&mWaitLooperSem, 0, 0);
    memset(&m_threadId, 0, sizeof(pthread_t));
}

void WpfaCcciSender::enqueueCcciMessage(const sp<WpfaDriverMessage>& msg) {
    waitLooper();
    mtkLogD(WPFA_D_LOG_TAG, "enqueueCcciMessage: %s", msg->toString().string());
    WpfaCcciSender* sender = this;
    sp<MessageHandler> handler = new CcciMsgHandler(sender, msg);
    m_looper->sendMessage(handler, m_dummy_msg);
}

void WpfaCcciSender::enqueueCcciMessageFront(const sp<WpfaDriverMessage>& msg) {
    waitLooper();
    mtkLogD(WPFA_D_LOG_TAG, "enqueueCcciMessageFront: %s", msg->toString().string());
    WpfaCcciSender* sender = this;
    sp<MessageHandler> handler = new CcciMsgHandler(sender, msg);
    m_looper->sendMessageAtTime(0, handler, m_dummy_msg);
}

void WpfaCcciSender::processMessage(const sp<WpfaDriverMessage>& msg) {
    int retval = 0;
    int length_write = 0;
    int try_cnt = 0;
    const int k_max_try_cnt = 20;

    // TODO: remark, only for UT
    mtkLogD(WPFA_D_LOG_TAG, "processMessage: %s", msg->toString().string());

    ccci_msg_send_t *mCcciMsgSend = (ccci_msg_send_t *) calloc(1, sizeof(ccci_msg_send_t));

    retval = driverMsgToCcciMsg(msg, mCcciMsgSend);
    if (retval != 0) {
        mtkLogE(WPFA_D_LOG_TAG, "driverMsgToCcciMsg fail!!");
        free(mCcciMsgSend);
        return;
    }

    // send message from CCCI
    for (try_cnt = 0; try_cnt < k_max_try_cnt; try_cnt++) {
        length_write = write(m_fd,
                             mCcciMsgSend->buffer,
                             mCcciMsgSend->buffer_size);
        if (length_write == mCcciMsgSend->buffer_size) {
            retval = 0;
            break;
        }

        retval = -1;
        mtkLogD(WPFA_D_LOG_TAG,"write ccci, try_cnt: %d, msg_id: 0x%x, length_write: %d",
                try_cnt, mCcciMsgSend->msg.hdr.msg_id, length_write);
        usleep(2 * 1000);
    }

    free(mCcciMsgSend);

    // test mode
    if (WPFA_DRIVER_TEST_MODE_ENABLE) {
        uint16_t msgId = msg->getMsgId();
        WpfaDriverUtilis *mWpfaDriverUtilis = WpfaDriverUtilis::getInstance();
        if ((mWpfaDriverUtilis != NULL) && (msgId == MSG_A2M_WPFA_VERSION)) {
            mWpfaDriverUtilis->testStartNormal(msg);
        }
    }
}


int WpfaCcciSender::driverMsgToCcciMsg(const sp<WpfaDriverMessage>& driverMsg,
        ccci_msg_send_t *ccciMsgSend) {
    int retValue = 0; // 0: success, 1: fail

    ccci_msg_hdr_t *header = &ccciMsgSend->msg.hdr;
    //ccci_msg_body_t *body = &ccciMsgSend->msg.body;

    header->msg_id = driverMsg->getMsgId();
    header->t_id = driverMsg->getTid();
    header->msg_type = driverMsg->getType();
    header->param_16bit = driverMsg->getParams();

    mtkLogD(WPFA_D_LOG_TAG, "driverMsgToCcciMsg Header msgId=%d tid=%d type=%d",
                header->msg_id, header->t_id,header->msg_type);

    if (header->msg_type == CCCI_CTRL_MSG) {
        // date size to be writed
        //ccciMsgSend->buffer_size = CCCI_HEADER_SIZE;
        ccciMsgSend->buffer_size = sizeof(ccci_msg_t);
        mtkLogD(WPFA_D_LOG_TAG, "ccciMsgSend->buffer_size=%d", ccciMsgSend->buffer_size);
    } else if (header->msg_type == CCCI_IP_TABLE_MSG) {
        uint16_t msgId = header->msg_id;
        switch (msgId) {
            case MSG_A2M_WPFA_INIT: {
                // no mseeage bode only header
                //ccciMsgSend->buffer_size = CCCI_HEADER_SIZE;
                break;
            }
            case MSG_A2M_WPFA_VERSION: {
                const wifiproxy_ap_md_filter_ver_t *verMsg =
                        (const wifiproxy_ap_md_filter_ver_t *)driverMsg->getData()->getData();
                wifiproxy_ap_md_filter_ver_t *version = &ccciMsgSend->msg.body.u.version;
                version->ap_filter_ver = verMsg->ap_filter_ver;
                version->md_filter_ver = verMsg->md_filter_ver;
                version->dl_buffer_size = verMsg->dl_buffer_size;
                version->ul_buffer_size = verMsg->ul_buffer_size;
                //ccciMsgSend->buffer_size = CCCI_HEADER_SIZE + CCCI_BODY_FILTER_VER_SIZE;
                mtkLogD(WPFA_D_LOG_TAG, "version Body apVer=%d mdVer=%d",
                            version->ap_filter_ver, version->md_filter_ver);
                break;
            }
            case MSG_A2M_REG_REPLY: {
                const wifiproxy_a2m_reg_reply_t *regAcceptMsg =
                        (const wifiproxy_a2m_reg_reply_t *)driverMsg->getData()->getData();
                wifiproxy_a2m_reg_reply_t *regAccept = &ccciMsgSend->msg.body.u.reg_reply_id;
                regAccept->fid = regAcceptMsg->fid;
                regAccept->error_cause = regAcceptMsg->error_cause;
                //ccciMsgSend->buffer_size = CCCI_HEADER_SIZE + CCCI_BODY_REG_ACCEPT_SIZE;
                mtkLogD(WPFA_D_LOG_TAG, "regAccept Body fid=%d error_cause=%d", regAccept->fid,
                            regAccept->error_cause);
                break;
            }
            case MSG_A2M_DEREG_REPLY: {
                const wifiproxy_a2m_dereg_reply_t *deRegAcceptMsg =
                        (const wifiproxy_a2m_dereg_reply_t *)driverMsg->getData()->getData();
                wifiproxy_a2m_dereg_reply_t *deRegAccept = &ccciMsgSend->msg.body.u.dereg_reply_id;
                deRegAccept->fid = deRegAcceptMsg->fid;
                deRegAccept->error_cause = deRegAcceptMsg->error_cause;
                //ccciMsgSend->buffer_size = CCCI_HEADER_SIZE + CCCI_BODY_REG_ACCEPT_SIZE;
                mtkLogD(WPFA_D_LOG_TAG, "deRegAccept Body fid=%d error_cause=%d", deRegAccept->fid,
                            deRegAccept->error_cause);
                break;
            }
            default: {
                mtkLogE(WPFA_D_LOG_TAG, "can't hadle this msg_id=%d", driverMsg->getMsgId());
                retValue = 1;
            }
        }
        ccciMsgSend->buffer_size = sizeof(ccci_msg_t);
        mtkLogD(WPFA_D_LOG_TAG, "ccciMsgSend->buffer_size=%d", ccciMsgSend->buffer_size);

    } else {
        mtkLogE(WPFA_D_LOG_TAG, "cannot handle msgType: %d", driverMsg->getType());
        retValue = 1;
    }
    return retValue;
}

bool WpfaCcciSender::threadLoop() {
    mtkLogD(WPFA_D_LOG_TAG, "threadLoop. WpfaCcciSender init");
    m_looper = Looper::prepare(0);
    sem_post(&mWaitLooperSem);
    m_threadId = pthread_self();

    int result;
    do {
        result = m_looper->pollAll(-1);
        mtkLogD(WPFA_D_LOG_TAG,"threadLoop, result = %d", result);
    } while (result == Looper::POLL_WAKE || result == Looper::POLL_CALLBACK);

    WPFA_D_ASSERT(0); // Can't go here
    return true;
}

sp<Looper> WpfaCcciSender::waitLooper() {
    mWaitLooperMutex.lock();
    if (mNeedWaitLooper) {
        mtkLogD(WPFA_D_LOG_TAG,"waitLooper() begin");
        sem_wait(&mWaitLooperSem);
        mNeedWaitLooper = false;
        sem_destroy(&mWaitLooperSem);
        mtkLogD(WPFA_D_LOG_TAG,"waitLooper() end");
    }
    mWaitLooperMutex.unlock();
    return m_looper;
}
