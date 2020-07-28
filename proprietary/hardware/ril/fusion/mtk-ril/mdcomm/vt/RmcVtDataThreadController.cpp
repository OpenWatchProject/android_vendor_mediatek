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

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RfxVoidData.h"
#include "RfxIntsData.h"

#include "RfxVtSendMsgData.h"
#include "RmcVtMsgParser.h"
#include "RmcVtDataThreadController.h"
#include "RmcVtReqHandler.h"

// for send MCL request to myself
#include "RfxMclDispatcherThread.h"
#include "RfxMclMessage.h"
#include "RfxMessageId.h"

// for socket / thread
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cutils/sockets.h>
#include <sys/socket.h>
#include <sys/un.h>

#define RFX_LOG_TAG "VT RIL THD"

RmcVtSharedMemory::RmcVtSharedMemory(void) {
    mLock = PTHREAD_MUTEX_INITIALIZER;
    mPLock = &mLock;
    mCond = PTHREAD_COND_INITIALIZER;
    mDataReadyCount = VT_RIL_SHARE_DATA_STATUS_RECV_NONE;
}

RmcVtSharedMemory::~RmcVtSharedMemory() {
}

void RmcVtSharedMemory::setState(int state) {
    if (RmcVtReqHandler::isVTLogEnable()) {
        RFX_LOG_I(RFX_LOG_TAG, "[MEM SET ST] state = %d\n", state);
    }

    mDataReadyCount = state;
}

int RmcVtSharedMemory::getState() {
    return mDataReadyCount;
}

bool RmcVtSharedMemory::checkState(int want_state) {

    if (mDataReadyCount == want_state) {
        if (RmcVtReqHandler::isVTLogEnable()) {
            RFX_LOG_I(RFX_LOG_TAG, "[MEM  CHECK] state = %d success \n", mDataReadyCount);
        }
        return true;
    }
    return false;
}

void RmcVtSharedMemory::setSlotId(int id) {
    mSharedMsg.slot_id = id;
}

void RmcVtSharedMemory::setSize(int size) {
    mSharedMsg.size = size;
}

void RmcVtSharedMemory::setData(char* data, int len) {
    mSharedMsg.data = (char*) calloc(len + 1, sizeof(char));
    memcpy(mSharedMsg.data, data, len);
}

int RmcVtSharedMemory::getSlotId(void) {
    return mSharedMsg.slot_id;
}

int RmcVtSharedMemory::getSize(void) {
    return mSharedMsg.size;
}

void RmcVtSharedMemory::getData(char** data) {
    (*data) = mSharedMsg.data;
}

void RmcVtSharedMemory::clearData() {
    free(mSharedMsg.data);
}

void RmcVtSharedMemory::lock(const char* user) {

    pthread_mutex_lock(mPLock);

    if (RmcVtReqHandler::isVTLogEnable()) {
        RFX_LOG_I(RFX_LOG_TAG, "[MEM   LOCK] lock success (%s)", user);
    }
}

void RmcVtSharedMemory::unlock(const char* user) {

    pthread_mutex_unlock(mPLock);
    if (RmcVtReqHandler::isVTLogEnable()) {
        RFX_LOG_I(RFX_LOG_TAG, "[MEM UNLOCK] unlock success (%s)", user);
    }
}

void RmcVtSharedMemory::wait(const char* user, int stay_state) {

    while (mDataReadyCount == stay_state) {
        pthread_cond_wait(&mCond, mPLock);
    }

    if (RmcVtReqHandler::isVTLogEnable()) {
        RFX_LOG_I(RFX_LOG_TAG, "[MEM   WAIT] wait success (%s)", user);
    }
}

void RmcVtSharedMemory::signal(const char* user) {

    pthread_cond_signal(&mCond);
    if (RmcVtReqHandler::isVTLogEnable()) {
        RFX_LOG_I(RFX_LOG_TAG, "[MEM SIGNAL] signal success (%s)", user);
    }
}

sp<RmcVtSharedMemory> RmcVtDataThreadController::sShareMemmory = new RmcVtSharedMemory();
pthread_t  RmcVtDataThreadController::sVtRilThd = 0;
pthread_t  RmcVtDataThreadController::sImcbRilThd = 0;
int RmcVtDataThreadController::sVtRilFd = 0;
int RmcVtDataThreadController::sVtsFd = 0;
int RmcVtDataThreadController::sIsVtConnected = 0;

RmcVtDataThreadController::RmcVtDataThreadController(void) {

    sShareMemmory->setState(VT_RIL_SHARE_DATA_STATUS_RECV_NONE);
}

RmcVtDataThreadController:: ~RmcVtDataThreadController() {
}

sp<RmcVtSharedMemory> RmcVtDataThreadController::getSharedMem(void) {
    return sShareMemmory;
}

void RmcVtDataThreadController::start(void) {

    pthread_attr_t  attr;
    int ret;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    ret = pthread_create(&sVtRilThd, &(attr), VT_RIL_THREAD, NULL);
    pthread_attr_destroy(&attr);
    if (ret < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[THD CTRLER] Fail to create ril VT thread!");
    }

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    ret = pthread_create(&sImcbRilThd, &(attr), RIL_IMCB_THREAD, NULL);
    pthread_attr_destroy(&attr);
    if (ret < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[THD CTRLER] Fail to create imcb ril thread!");
    }

}

void * RmcVtDataThreadController::RIL_IMCB_THREAD(void *arg) {

    RFX_LOG_I(RFX_LOG_TAG, "[RIL IMCB THD] Start");

    RFX_UNUSED(arg);

    while(1) {

        if (sIsVtConnected == 0) {
            //RFX_LOG_D(RFX_LOG_TAG, "[RIL IMCB THD] sIsVtConnected : %d", sIsVtConnected);
            usleep(500 * 1000);
            continue;
        }

        sShareMemmory->lock("ril-imcb thread");

        sShareMemmory->wait("ril-imcb thread", VT_RIL_SHARE_DATA_STATUS_RECV_NONE);

        char *data;
        int id;
        int size;

        sShareMemmory->getData(&data);
        id = (* ((int*)data));
        size = (* ((int*)(data + 4)));

        RFX_LOG_D(RFX_LOG_TAG, "[RIL IMCB THD] id = %d, size = %d", id, size);

        if (send(sVtsFd, (const void*)&id, sizeof(int), 0) != sizeof(int)) {
            RFX_LOG_E(RFX_LOG_TAG, "[RIL IMCB THD] send fail (type) / fd: %d, errCode: %d", sVtsFd, errno);

            sShareMemmory->clearData();
            sShareMemmory->setState(VT_RIL_SHARE_DATA_STATUS_RECV_NONE);
            sShareMemmory->unlock("ril-imcb thread");
            continue;
        }

        if (send(sVtsFd, (const void*)&size, sizeof(int), 0) != sizeof(int)) {
            RFX_LOG_E(RFX_LOG_TAG, "[RIL IMCB THD] send fail (len) / fd: %d, errCode: %d", sVtsFd, errno);

            sShareMemmory->clearData();
            sShareMemmory->setState(VT_RIL_SHARE_DATA_STATUS_RECV_NONE);
            sShareMemmory->unlock("ril-imcb thread");
            continue;
        }

        if (send(sVtsFd, (const void*)(data + 8), size, 0) != size) {
            RFX_LOG_E(RFX_LOG_TAG, "[RIL IMCB THD] send fail (data) / fd: %d, errCode: %d", sVtsFd, errno);

            sShareMemmory->clearData();
            sShareMemmory->setState(VT_RIL_SHARE_DATA_STATUS_RECV_NONE);
            sShareMemmory->unlock("ril-imcb thread");
            continue;
        }

        if (RmcVtReqHandler::isVTLogEnable()) {
            RFX_LOG_D(RFX_LOG_TAG, "[RIL IMCB THD] Send Data to VTS");
        }

        sShareMemmory->clearData();

        sShareMemmory->setState(VT_RIL_SHARE_DATA_STATUS_RECV_NONE);

        sShareMemmory->unlock("ril-imcb thread");

    }
    RFX_LOG_E(RFX_LOG_TAG, "[RIL IMCB THD] end of readerLooper, should not happen");

    return 0;
}

void* RmcVtDataThreadController::VT_RIL_THREAD(void *arg) {

    RFX_LOG_I(RFX_LOG_TAG, "[VT RIL THD] Start");

    RFX_UNUSED(arg);

    // create server FD
    sVtRilFd = android_get_control_socket("volte_imsvt1");
    if (sVtRilFd < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[VT RIL THD] failed to get socket, errno: %d, %s", errno, strerror(errno));
        return 0;
    }

    RFX_LOG_I(RFX_LOG_TAG, "[VT RIL THD] socket fd: %d", sVtRilFd);

    // listen client
    int ret = listen(sVtRilFd, 1);
    if (ret < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[VT RIL THD] failed to listen, errno: %d, %s", errno, strerror(errno));
    }

    struct sockaddr_un peerAddr;
    socklen_t sockLen = sizeof(peerAddr);
    RFX_LOG_I(RFX_LOG_TAG, "[VT RIL THD] start readerLooper");

    // accept client and start looper to read/write data
    // we should have only one client
    while (1) {

        sVtsFd = accept(sVtRilFd, (sockaddr *)&peerAddr, &sockLen);

        if (sVtsFd < 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[VT RIL THD] failed to accept, errno: %d, %s", errno, strerror(errno));
            usleep(200 * 1000);
            continue;
        }

        int flags = fcntl(sVtsFd, F_GETFL);
        int ret = fcntl(sVtsFd , F_SETFL, flags | O_NONBLOCK);
        if (ret < 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[VT RIL THD] failed to set O_NONBLOCK, errno: %d, %s", errno, strerror(errno));
        }

        sIsVtConnected = 1;
        RFX_LOG_I(RFX_LOG_TAG, "[VT RIL THD] accepted, sVtsFd: %d, sIsVtConnected: %d",
                sIsVtConnected, sVtsFd);

        // start looper
        ril_vt_looper();

        close(sVtsFd);

        sIsVtConnected = 0;
        RFX_LOG_I(RFX_LOG_TAG, "[VT RIL THD] sIsVtConnected: %d", sIsVtConnected);
    }

    RFX_LOG_E(RFX_LOG_TAG, "[VT RIL THD] end of readerLooper, should not happen");

    return 0;
}

int RmcVtDataThreadController::ril_vt_looper() {

    if (RmcVtReqHandler::isVTLogEnable()) {
        RFX_LOG_D(RFX_LOG_TAG, "[VT RIL THD] sVtsFd: %d\n", sVtsFd);
    }

    while (1) {

        // =============================================================
        // the case vt service send msg to IMCB
        // =============================================================

        int ret;
        fd_set rfds;

        FD_ZERO(&rfds);
        FD_SET(sVtsFd, &rfds);

        ret = select(sVtsFd + 1, &rfds, NULL, NULL, NULL);

        if (ret == -1) {
            if (errno == EINTR || errno == EAGAIN) {
                continue;
            }
            RFX_LOG_D(RFX_LOG_TAG, "[VT RIL THD] Fail to select in readline. error: %d", errno);
            break;

        } else if (ret == 0) {

            //RFX_LOG_D(RFX_LOG_TAG, "[VT RIL LOP] No data from vt service socket.");
            continue;

        }

        if (sVtsFd > 0 && FD_ISSET(sVtsFd, &rfds)) {

            int slot_id;
            int msg_type;
            int recv_length = 0;
            char* outBuffer = NULL;

            ret = ril_vt_recv(sVtsFd, reinterpret_cast<void *>(&msg_type), sizeof(msg_type));
            if (ret < 0) {
                break;
            }

            ret = ril_vt_recv(sVtsFd, reinterpret_cast<void *>(&recv_length), sizeof(recv_length));
            if (ret < 0) {
                break;
            }

            outBuffer = (char* ) calloc(recv_length + 1 + 8, sizeof(char));
            (* ((int*)outBuffer))       = msg_type;
            (* ((int*)outBuffer + 1))   = recv_length;

            RFX_LOG_D(RFX_LOG_TAG, "[VT RIL THD] msg_type = %d, recv_length = %d",
                    (* ((int*)outBuffer)), (* ((int*)outBuffer + 1)));

            ret = ril_vt_recv(sVtsFd, reinterpret_cast<void *>(outBuffer + 8), recv_length);
            if (ret < 0) {
                break;
            }

            // get slot ID from msg
            RmcVtMsgParser* parser = RmcVtMsgParser::getInstance();
            parser->startSession(outBuffer + 8, msg_type);
            slot_id = parser->getSlot();
            parser->stopSession();

            if (isImsMessage(msg_type)) {
                //send to IMCB
                handleImsMessage((recv_length + 8), outBuffer, slot_id);
            } else {
                //send to L4
                RFX_LOG_I(RFX_LOG_TAG, "msg to L4, send AT command");
                handleMessage(msg_type, recv_length, (void*)(outBuffer + 8), slot_id);
            }

            free(outBuffer);
        }
    }
    return 0;
}


int RmcVtDataThreadController::ril_vt_recv(int fd, void* buffer, int size) {

    int recvLen = -1;
    int lenRecvd = 0;

    do {

        recvLen = recv(fd, ((char*)buffer + lenRecvd), (size - lenRecvd), 0);

        if (RmcVtReqHandler::isVTLogEnable()) {
            RFX_LOG_E(RFX_LOG_TAG, "[VT RIL RECV] length  = %d", recvLen);
        }

        if (recvLen != (size - lenRecvd)) {
            if (recvLen == -1) {

                if (errno != EAGAIN && errno != EINTR) {

                    RFX_LOG_D(RFX_LOG_TAG, "[VT RIL RECV] fail to recv. errno = %d",  errno);
                    return -1;

                } else {

                    RFX_LOG_E(RFX_LOG_TAG, "[VT RIL RECV] Try again (not error). errno = %d", errno);
                    continue;
                }

            } else if (recvLen == 0) {

                RFX_LOG_E(RFX_LOG_TAG, "[VT RIL RECV] fail to recv. recvLen = 0");
                return -1;

            } else {
                lenRecvd += recvLen;
                RFX_LOG_E(RFX_LOG_TAG, "[VT RIL RECV] Try again (not error), recvLen = %d, lenRecvd = %d. errno = %d", recvLen, lenRecvd, errno);
                continue;
            }

        } else {
            return 0;
        }

    } while (recvLen != (size - lenRecvd));

    return -1;
}

bool RmcVtDataThreadController::isImsMessage(int msgId) {
    switch (msgId) {
        case MSG_ID_WRAP_IMSVT_MD_ANBR_REPORT_REQ: //set ANBR request
            return false;
        default:
            return true;
    }
}

void RmcVtDataThreadController::handleMessage(int msgId, int length, void* data, int slotId) {

    RFX_LOG_I(RFX_LOG_TAG, "[handleMessage] msgId = %d, length = %d, slotId = %d", msgId, length, slotId);

    switch (msgId) {
        case MSG_ID_WRAP_IMSVT_MD_ANBR_REPORT_REQ: {

            int anbr_param [6];
            anbr_param [0] = ((VT_ANBR_REQ*)data)->anbr_config.is_ul;
            anbr_param [1] = ((VT_ANBR_REQ*)data)->anbr_config.ebi;
            anbr_param [2] = ((VT_ANBR_REQ*)data)->anbr_config.bitrate;
            anbr_param [3] = ((VT_ANBR_REQ*)data)->anbr_config.bearer_id;
            anbr_param [4] = ((VT_ANBR_REQ*)data)->anbr_config.pdu_session_id;
            anbr_param [5] = ((VT_ANBR_REQ*)data)->anbr_config.ext_param;

            sp<RfxMclMessage> msg = RfxMclMessage::obtainEvent(RFX_MSG_EVENT_REPORT_ANBR,
                    RfxIntsData(anbr_param, 6), RIL_CMD_PROXY_2, slotId);
            RfxMclDispatcherThread::enqueueMclMessage(msg);

            break;
        }
        default:
            break;
    }
}

void RmcVtDataThreadController::handleImsMessage(int size, char* outBuffer, int slotId) {

    // send to RIL FWK
    RIL_VT_SERVICE_MSG msg_data;
    msg_data.slot_id = slotId;
    msg_data.size   = size;
    msg_data.data   = outBuffer;

    RFX_LOG_D(RFX_LOG_TAG,
            "[VT RIL THD] slot_id = %d, msg_data.size = %d, msg_data.data = %lu",
            msg_data.slot_id, msg_data.size, (unsigned long) msg_data.data);

    // Always using fixed SIM channel
    sp<RfxMclMessage> msg = RfxMclMessage::obtainEvent(RFX_MSG_EVENT_VT_SEND_MSG,
    RfxVtSendMsgData(&msg_data, 1), RIL_CMD_PROXY_2, 0);
    RfxMclDispatcherThread::enqueueMclMessage(msg);
}
