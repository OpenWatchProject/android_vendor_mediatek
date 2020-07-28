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

#include "WpfaCcciReader.h"
#include "WpfaControlMsgHandler.h"

#define WPFA_D_LOG_TAG "WpfaCcciReader"


WpfaCcciReader::WpfaCcciReader(int fd) : mFd(fd) {
        mtkLogD(WPFA_D_LOG_TAG, "init");
        memset(&m_threadId, 0, sizeof(pthread_t));
}

bool WpfaCcciReader::threadLoop() {
    m_threadId = pthread_self();
    readerLoopForCcciData();
    return true;
}

void WpfaCcciReader::readerLoopForCcciData() {
    int count = 0;
    int bodySize = 0;
    uint16_t msgId = 0;
    uint16_t msgParam = 0;
    WpfaCcciDataHeader ccciDataHeader;
    ccci_msg_body_t *body = NULL;
    ccci_msg_ul_ip_pkt_body_t *IpPktbody = NULL;

    ccci_msg_hdr_t *header = (ccci_msg_hdr_t *) calloc(1, CCCI_HEADER_SIZE);
    if (header == NULL) {
        mtkLogE(WPFA_D_LOG_TAG, "OOM");
        return;
    }
    mtkLogD(WPFA_D_LOG_TAG, "readerLoop run!");

    for (;;) {
        // get header
        memset(header, 0, sizeof(ccci_msg_hdr_t));
        do {
            count = read(mFd, header, sizeof(ccci_msg_hdr_t));
            if (count < 0 && errno != EINTR) {
                mtkLogE(WPFA_D_LOG_TAG, "header read end: %d (err: %d - %s)\n", count,
                        errno, strerror(errno));
                free(header);
                return;
            } else {
                mtkLogD(WPFA_D_LOG_TAG, "header read end: %d\n", count);
            }
        } while (count < 0 && errno == EINTR);

        // For non-ViLte project, imspty will transform to dev/null
        // It will trigger read with count=0, the data can be ignored
        if (count == 0) {
            continue;
        }

        ccciDataHeader = WpfaCcciDataHeaderEncoder::decodeHeader(header);
        mtkLogD(WPFA_D_LOG_TAG, "ccciDataHeader msgId: %d",
                ccciDataHeader.getMsgId());

        // for all read indication, receive the msg size data is size of ccci_msg_body_t
        body = (ccci_msg_body_t *) calloc(1, sizeof(ccci_msg_body_t));
        if (body == NULL) {
            mtkLogE(WPFA_D_LOG_TAG, "OOM");
            free(header);
            return;
        }

        // get size by mseeage Id
        //bodySize = WpfaDriverMessage::getCcciMsgBodySize(msgId);
        bodySize = sizeof(ccci_msg_body_t);

        // get message body if the message is not UL IP PKT
        if (ccciDataHeader.getMsgType() != CCCI_UL_IP_PKT_MSG) {
            do {
                count = read(mFd, body, bodySize);
                if (count < 0 && errno != EINTR) {
                    mtkLogE(WPFA_D_LOG_TAG, "body read end: %d (err: %d - %s)\n", count,
                            errno, strerror(errno));
                    free(body);
                    free(header);
                    return;
                } else {
                    mtkLogD(WPFA_D_LOG_TAG, "body read end: count=%d\n", count);
                }
            } while (count < 0 && errno == EINTR);
        }

        if (ccciDataHeader.getMsgType() == CCCI_CTRL_MSG) {
            handleShmCtrlEvent(ccciDataHeader);
            continue;
        } else if (ccciDataHeader.getMsgType() == CCCI_IP_TABLE_MSG) {
            // get message id
            msgId = ccciDataHeader.getMsgId();

            // create data object
            switch (msgId) {
                case MSG_M2A_REG_DL_FILTER:
                    handleFilterRuleCtrlEvent(ccciDataHeader, WpfaDriverRegFilterData(body, 1));
                    break;

                case MSG_M2A_DEREG_DL_FILTER:
                    handleFilterRuleCtrlEvent(ccciDataHeader, WpfaDriverDeRegFilterData(body, 1));
                    break;

                case MSG_M2A_WPFA_VERSION:
                    handleFilterRuleCtrlEvent(ccciDataHeader, WpfaDriverVersionData(body, 1));
                    break;

                default:
                    mtkLogE(WPFA_D_LOG_TAG, "can't hadle this msg id!");
                    // todo: assert
            }

            // send message to dispatcher
            //handleFilterRuleCtrlEvent(ccciDataHeader, body);
            free(body);
        } else if (ccciDataHeader.getMsgType() == CCCI_UL_IP_PKT_MSG) {
            // MD UL AP Path Via TTY instead of SHM (For M2 RD IT purpose)
            // get message id
            msgId = ccciDataHeader.getMsgId();
            msgParam = ccciDataHeader.getParams();

            mtkLogD(WPFA_D_LOG_TAG, "UP_IP_PKT size=%d", msgParam);

            IpPktbody = (ccci_msg_ul_ip_pkt_body_t *) calloc(1, sizeof(ccci_msg_ul_ip_pkt_body_t));
            if (IpPktbody == NULL) {
                mtkLogE(WPFA_D_LOG_TAG, "OOM");
                free(header);
                return;
            }

            // get message body
            do {
                count = read(mFd, IpPktbody, msgParam);
                if (count < 0 && errno != EINTR) {
                    mtkLogE(WPFA_D_LOG_TAG, "body read end: %d (err: %d - %s)\n", count,
                            errno, strerror(errno));
                    free(IpPktbody);
                    free(header);
                    return;
                } else {
                    mtkLogD(WPFA_D_LOG_TAG, "body read end: count=%d\n", count);
                }
            } while (count < 0 && errno == EINTR);

            switch (msgId) {
                case MSG_M2A_UL_IP_PKT:
                    handleFilterRuleCtrlEvent(ccciDataHeader, WpfaDriverULIpPkt(IpPktbody, 1));
                    break;
                default:
                    mtkLogE(WPFA_D_LOG_TAG, "can't hadle this msg id!");
            }
            free(IpPktbody);

        } else {
            mtkLogE(WPFA_D_LOG_TAG, "cannot handle this msgType: %d", ccciDataHeader.getMsgType());
            free(header);
            return;
        }
    }
    free(header);
}

void WpfaCcciReader::handleFilterRuleCtrlEvent(WpfaCcciDataHeader header, WpfaDriverBaseData data) {
    // create Message and sent to WpfaControlMsgHandler
    int msgId = header.getMsgId();
    mtkLogD(WPFA_D_LOG_TAG, "handleFilterRuleCtrlEvent, msgId:%d", msgId);

    sp<WpfaDriverMessage> msg = WpfaDriverMessage::obtainMessage(
            header.getMsgId(),
            header.getTid(),
            header.getMsgType(),
            header.getParams(),
            data);
    WpfaControlMsgHandler::enqueueDriverMessage(msg);
}

void WpfaCcciReader::handleShmCtrlEvent(WpfaCcciDataHeader header) {
    // create Message(only header) and sent to WpfaControlMsgHandler
    mtkLogD(WPFA_D_LOG_TAG, "handleShmCtrlEvent, msgId:%d", header.getMsgId());
    sp<WpfaDriverMessage> msg = WpfaDriverMessage::obtainMessage(
            header.getMsgId(),
            header.getTid(),
            header.getMsgType(),
            header.getParams());
    WpfaControlMsgHandler::enqueueDriverMessage(msg);
}
