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

#include "RfxRcsSendMsgData.h"
#include "RmcRcsDataThreadController.h"

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
#include <netinet/in.h>
#include <arpa/inet.h>

#define RFX_LOG_TAG "RCS RIL THD"

RmcRcsSharedMemory::RmcRcsSharedMemory(void) {
    mLock = PTHREAD_MUTEX_INITIALIZER;
    mPLock = &mLock;
    mCond = PTHREAD_COND_INITIALIZER;
    mDataReadyCount = RCS_RIL_SHARE_DATA_STATUS_RECV_NONE;
    mDatalen = 0;
    mData = NULL;
}

RmcRcsSharedMemory::~RmcRcsSharedMemory() {
    if (mData != NULL) {
        free(mData);
    }
}

void RmcRcsSharedMemory::setState(int state) {

    RFX_LOG_I(RFX_LOG_TAG, "[MEM SET ST] state = %d\n", state);

    mDataReadyCount = state;
}

int RmcRcsSharedMemory::getState() {
    return mDataReadyCount;
}

bool RmcRcsSharedMemory::checkState(int want_state) {

    if (mDataReadyCount == want_state) {
        if (RmcRcsDataThreadController::isLogEnable()) {
            RFX_LOG_I(RFX_LOG_TAG, "[MEM  CHECK] state = %d success \n", mDataReadyCount);
        }
        return true;
    }
    return false;
}

void RmcRcsSharedMemory::setSize(int size) {
    mDatalen = size;
}

void RmcRcsSharedMemory::setData(char* data, int len) {
    mData = (char*) calloc(len, sizeof(char));
    if (mData != NULL) {
        memcpy(mData, data, len);
    }
}

int RmcRcsSharedMemory::getSize(void) {
    return mDatalen;
}

void RmcRcsSharedMemory::getData(char** data) {
    (*data) = mData;
}

void RmcRcsSharedMemory::clearData() {
    free(mData);
}

void RmcRcsSharedMemory::lock(const char* user) {

    pthread_mutex_lock(mPLock);

    if (RmcRcsDataThreadController::isLogEnable()) {
        RFX_LOG_I(RFX_LOG_TAG, "[MEM   LOCK] lock success (%s)", user);
    }
}

void RmcRcsSharedMemory::unlock(const char* user) {

    pthread_mutex_unlock(mPLock);
    if (RmcRcsDataThreadController::isLogEnable()) {
        RFX_LOG_I(RFX_LOG_TAG, "[MEM UNLOCK] unlock success (%s)", user);
    }
}

void RmcRcsSharedMemory::wait(const char* user, int stay_state) {

    while (mDataReadyCount == stay_state) {
        pthread_cond_wait(&mCond, mPLock);
    }

    if (RmcRcsDataThreadController::isLogEnable()) {
        RFX_LOG_I(RFX_LOG_TAG, "[MEM   WAIT] wait success (%s)", user);
    }
}

void RmcRcsSharedMemory::signal(const char* user) {

    pthread_cond_signal(&mCond);
    if (RmcRcsDataThreadController::isLogEnable()) {
        RFX_LOG_I(RFX_LOG_TAG, "[MEM SIGNAL] signal success (%s)", user);
    }
}

sp<RmcRcsSharedMemory> RmcRcsDataThreadController::sShareMemory = new RmcRcsSharedMemory();
pthread_t  RmcRcsDataThreadController::sShareMemoryThd = 0;
pthread_t  RmcRcsDataThreadController::sRilServerThd = 0;
pthread_mutex_t RmcRcsDataThreadController::sMutex = PTHREAD_MUTEX_INITIALIZER;
Ril_Channel_Obj_t *RmcRcsDataThreadController::sChannelObj = NULL;
RmcRcsDataThreadController *RmcRcsDataThreadController::sInstance = NULL;

RmcRcsDataThreadController::RmcRcsDataThreadController() {
}

RmcRcsDataThreadController:: ~RmcRcsDataThreadController() {
}

sp<RmcRcsSharedMemory> RmcRcsDataThreadController::getSharedMem(void) {
    return sShareMemory;
}

void RmcRcsDataThreadController::init() {
    pthread_mutex_lock(&sMutex);
    if (sInstance == NULL) {
        RFX_LOG_I(RFX_LOG_TAG, "RmcRcsDataThreadController init, create instance");
        sInstance = new RmcRcsDataThreadController();
        startThread();
    }
    pthread_mutex_unlock(&sMutex);
}

void RmcRcsDataThreadController::startThread() {

    pthread_attr_t  attr;
    int ret;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    ret = pthread_create(&sRilServerThd, &(attr), rilServerThread, NULL);
    pthread_attr_destroy(&attr);
    if (ret < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "Fail to create RCS ril thread!");
    }

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    ret = pthread_create(&sShareMemoryThd, &(attr), shareMemoryThread, NULL);
    pthread_attr_destroy(&attr);
    if (ret < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "Fail to create RCS share memory thread!");
    }
}

void* RmcRcsDataThreadController::rilServerThread(void *arg) {

    RFX_LOG_I(RFX_LOG_TAG, "[Ril Srv Thd] rilServerThread Start");
    RFX_UNUSED(arg);
    int ret = 0;

    //allocate memory
    sChannelObj = (Ril_Channel_Obj_t *)malloc(sizeof(Ril_Channel_Obj_t));
    if (!sChannelObj) {
        RFX_LOG_E(RFX_LOG_TAG, "[Ril Srv Thd] Can't allocate the memory");
        goto free_mem;
    }
    memset(sChannelObj, 0, sizeof(Ril_Channel_Obj_t));

    sChannelObj->channels_size = MAX_CLIENT;

    sChannelObj->channels = (Ril_Channel_Client_t *)malloc(sizeof(Ril_Channel_Client_t) * sChannelObj->channels_size);
    if (!sChannelObj->channels) {
        RFX_LOG_E(RFX_LOG_TAG, "[Ril Srv Thd] Can't allocate the memory");
        goto free_mem;
    }
    memset(sChannelObj->channels, 0, sizeof(Ril_Channel_Client_t) * sChannelObj->channels_size);
    //set fd as initial value -1
    for (int i = 0; i < sChannelObj->channels_size; ++i) {
        Ril_Channel_Client_t *clients = (Ril_Channel_Client_t *)sChannelObj->channels;
        clients[i].channel.fd = -1;
    }

    // create server FD
    sChannelObj->fd = android_get_control_socket(RCS_RIL_SOCKET_NAME);

    //do retry if init.rc didn't define socket
    if (sChannelObj->fd < 0) {
        struct sockaddr_un my_addr;
        RFX_LOG_D(RFX_LOG_TAG, "init.rc didn't define, create socket manually");
        memset(&my_addr, 0, sizeof(struct sockaddr_un));
        my_addr.sun_family = AF_UNIX;

        RFX_LOG_D(RFX_LOG_TAG, "socketName is %s", RCS_RIL_SOCKET_NAME);
        strncpy(my_addr.sun_path, RCS_RIL_SOCKET_NAME, sizeof(my_addr.sun_path));
        my_addr.sun_path[0] = 0;

        sChannelObj->fd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (sChannelObj->fd < 0) {
            RFX_LOG_E(RFX_LOG_TAG, "manually listen fail, closed");
            goto free_mem;
        }

        int ret = ::bind(sChannelObj->fd, (struct sockaddr *) &my_addr,
                sizeof(struct sockaddr_un));
        if (ret < 0) {
            RFX_LOG_E(RFX_LOG_TAG, "bind fail, ret = %d, errno = %d, set state to close", ret, errno);
            close(sChannelObj->fd);
            sChannelObj->fd = -1;
        }
    }

    if (sChannelObj->fd < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[Ril Srv Thd] failed to get socket, errno: %d, %s", errno, strerror(errno));
        goto free_mem;
    }

    RFX_LOG_I(RFX_LOG_TAG, "[Ril Srv Thd] socket fd: %d", sChannelObj->fd);

    // listen client
    ret = listen(sChannelObj->fd, sChannelObj->channels_size);
    if (ret < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[Ril Srv Thd] failed to listen, errno: %d, %s", errno, strerror(errno));
        goto free_mem;
    }

    pthread_mutex_init(&(sChannelObj->mutex), 0);

    // accept client and start looper to read/write data
    while (1) {
        fd_set  iofds;
        int i = 0;
        int ret = 0;
        int max_fd = sChannelObj->fd;
        int local_errno;
        int bad_fd_handle = 0;
        Ril_Channel_Client_t *clients = (Ril_Channel_Client_t *)sChannelObj->channels;

        FD_ZERO(&iofds);
        FD_SET(sChannelObj->fd, &iofds);

        //set client fd
        for (i = 0; i < sChannelObj->channels_size; ++i) {
            if (clients[i].channel.fd >= 0) {
                if (clients[i].channel.fd > max_fd) {
                    max_fd = clients[i].channel.fd;
                }
                FD_SET(clients[i].channel.fd, &iofds);
            }
        }

        ret = select(max_fd + 1, &iofds, 0, 0, NULL);
        local_errno = errno;

        if (ret < 0) {
            if (local_errno == EINTR) {
                continue;
            }

            RFX_LOG_E(RFX_LOG_TAG, "[Ril Srv Thd] Select fail, ret = %d, errno: %d, %s", ret, errno, strerror(errno));

            if (local_errno == 9) {
                //Bad file descriptor(9)
                for (i = 0; i < sChannelObj->channels_size; ++i) {
                    Ril_Channel_Client_t *ccc = &(clients[i]);
                    if (ccc->channel.fd >= 0) {
                        if (FD_ISSET(ccc->channel.fd, &iofds)) {
                            int tmp_client_fd = ccc->channel.fd;
                            RFX_LOG_E(RFX_LOG_TAG, "[Ril Srv Thd] Bad file descriptor [fd:%d, idx:%d]", ccc->channel.fd, i);
                            ccc->channel.fd = -1;
                            shutdown(tmp_client_fd, SHUT_RDWR);
                            close(tmp_client_fd);
                            rilServerRead(0, &(ccc->channel));
                            bad_fd_handle = 1;
                            break;
                        }
                    }
                }
            }

            if (bad_fd_handle == 1) {
                continue;
            } else {
                goto free_mem;
            }
        } else if (ret == 0) {
            continue;
        }

        if (FD_ISSET(sChannelObj->fd, &iofds)) {
            int tmp_fd = 0;
            int tmp_index = -1;
            struct sockaddr_in tmp_addr;
            socklen_t tmp_addrlen = sizeof(struct sockaddr);

            tmp_fd = accept(sChannelObj->fd, (sockaddr *)&tmp_addr, (socklen_t *)&tmp_addrlen);
            if (tmp_fd < 0) {
                RFX_LOG_E(RFX_LOG_TAG, "[Ril Srv Thd] accept error, errno = %s(%d)", strerror(errno), errno);
                continue;
            }

            for (i = 0 ; i < sChannelObj->channels_size; ++i) {
                if (clients[i].channel.fd < 0) {
                    pthread_mutex_init(&(clients[i].channel.mutex), 0);
                    clients[i].channel.fd = tmp_fd;
                    tmp_index = i;
                    break;
                }
            }

            if (tmp_index < 0) {
                shutdown(tmp_fd, SHUT_RDWR);
                close(tmp_fd);
                RFX_LOG_E(RFX_LOG_TAG, "[Ril Srv Thd] Connection pool is full");
            } else {
                RFX_LOG_E(RFX_LOG_TAG, "[Ril Srv Thd] Connection from %s:%d [fd:%d, idx:%d]",
                    inet_ntoa(tmp_addr.sin_addr), ntohs(tmp_addr.sin_port), tmp_fd, tmp_index);
            }
        }

        for (i = 0 ; i < sChannelObj->channels_size; ++i) {
            Ril_Channel_Client_t *ccc = &(clients[i]);

            if (ccc->channel.fd >= 0) {
                if (FD_ISSET(ccc->channel.fd, &iofds)) {
                    ret = recv(ccc->channel.fd, ccc->buf + ccc->buf_len, sizeof(ccc->buf) - ccc->buf_len, 0);
                    if (ret > 0) {
                        ccc->buf_len += ret;
                        while (ccc->buf_len >= (int)UA_CHANNEL_HEADER_LEN) {
                            Channel_Data_t recv_data;

                            memcpy(&recv_data, ccc->buf, UA_CHANNEL_HEADER_LEN);
                            RFX_LOG_I(RFX_LOG_TAG, "[Ril Srv Thd] data->type = %d, data->len = %d, data->flag = %d, ccc->buf_len = %d",
                                recv_data.type, recv_data.len, recv_data.flag, ccc->buf_len);
                            if (recv_data.len == 0) {
                                recv_data.data = 0;
                                rilServerRead(&recv_data, &(ccc->channel));
                                ccc->buf_len -= (UA_CHANNEL_HEADER_LEN);
                                memmove(ccc->buf, ccc->buf + UA_CHANNEL_HEADER_LEN, ccc->buf_len);
                            } else if (recv_data.len <= (int)(ccc->buf_len - UA_CHANNEL_HEADER_LEN)) {
                                recv_data.data = (char *)calloc(recv_data.len + 1, sizeof(char));
                                if (recv_data.data) {
                                    memcpy(recv_data.data, ccc->buf + UA_CHANNEL_HEADER_LEN, recv_data.len);
                                    rilServerRead(&recv_data, &(ccc->channel));
                                    free(recv_data.data);
                                } else {
                                    RFX_LOG_E(RFX_LOG_TAG, "[Ril Srv Thd] Can't allocate memory");
                                }
                                ccc->buf_len -= (UA_CHANNEL_HEADER_LEN + recv_data.len);
                                memmove(ccc->buf, ccc->buf + UA_CHANNEL_HEADER_LEN+recv_data.len, ccc->buf_len);
                            } else {
                                break;
                            }
                        }
                    } else if (ret == 0) {
                        int tmp_client_fd = ccc->channel.fd;

                        if (sizeof(ccc->buf) - ccc->buf_len == 0) {
                            RFX_LOG_E(RFX_LOG_TAG, "[Ril Srv Thd] ****** channel buffer full ******");
                        }
                        RFX_LOG_E(RFX_LOG_TAG, "[Ril Srv Thd] Client disconnect [fd:%d, idx:%d]", ccc->channel.fd, i);
                        ccc->channel.fd = -1;
                        shutdown(tmp_client_fd, SHUT_RDWR);
                        close(tmp_client_fd);
                        rilServerRead(0, &(ccc->channel));
                    } else {
                        if (errno != EINTR) {
                            int tmp_client_fd = ccc->channel.fd;

                            RFX_LOG_E(RFX_LOG_TAG, "[Ril Srv Thd] Read data errorm [fd:%d, idx:%d], ret = %d, errno = %s(%d)",
                                ccc->channel.fd, i, ret, strerror(errno), errno);
                            ccc->channel.fd = -1;
                            shutdown(tmp_client_fd, SHUT_RDWR);
                            close(tmp_client_fd);
                            rilServerRead(0, &(ccc->channel));
                        }
                    }
                }
            }
        }
    }

    RFX_LOG_E(RFX_LOG_TAG, "[Ril Srv Thd] rilServerThread exit");

    //close server fd
    close(sChannelObj->fd);
    sChannelObj->fd = -1;

free_mem :
    if (sChannelObj && sChannelObj->channels) free(sChannelObj->channels);
    if (sChannelObj) {
        if (sChannelObj->fd >= 0) close(sChannelObj->fd);
        free(sChannelObj);
    }
    RFX_LOG_E(RFX_LOG_TAG, "[Ril Srv Thd] end of server thread, should not happen");

    return 0;
}


void RmcRcsDataThreadController::rilServerRead(Channel_Data_t *msgData, Ril_Channel_Obj_t *channel) {
    if (!channel) {
        return;
    }
    if (!msgData) {
        RFX_LOG_I(RFX_LOG_TAG, "UA (%d - %p) is disconnected", channel->type, channel);
        //reset type
        channel->type = 0; //unknown type
        return;
    } else {
        if(msgData->type == VOLTE_REQ_SIPTX_CREATE) {
            VoLTE_Stack_Channel_Info_t *info = (VoLTE_Stack_Channel_Info_t *)msgData->data;
            if (info->type == (VoLTE_Stack_UA_RCS_Proxy | VoLTE_UA_MAGIC_NUMBER)) {//rcs
                channel->type = VoLTE_Stack_UA_RCS_Proxy;
            } else if (info->type == (VoLTE_Stack_UA_RCS_Proxy_Sip | VoLTE_UA_MAGIC_NUMBER)) {//rcs_sip
                channel->type = VoLTE_Stack_UA_RCS_Proxy_Sip;
            } else {
                RFX_LOG_E(RFX_LOG_TAG, "wrong type(%d) for VOLTE_REQ_SIPTX_CREATE", info->type);
                return;
            }
        }
        //dispatch to md
        switch (channel->type) {
            case VoLTE_Stack_UA_RCS_Proxy:
            case VoLTE_Stack_UA_RCS_Proxy_Sip:
            {
                // send to RIL FWK
                void *rawData = NULL;
                int rawDataLen = msgData->len + MD_CHANNEL_HEADER_LEN;
                rawData = convertUAData(msgData, channel->type, 0);//default acct_id
                if (rawData) {
                    sp<RfxMclMessage> rcsmsg = RfxMclMessage::obtainEvent(RFX_MSG_EVENT_RCS_SEND_MSG,
                            RfxRcsSendMsgData(rawData, rawDataLen), RIL_CMD_PROXY_2, 0);//todo for slot id
                    RfxMclDispatcherThread::enqueueMclMessage(rcsmsg);
                    free(rawData);
                }
                break;
            }
            default:
                break;
        }
    }
}

//for Ims Submarine data structure change
void *RmcRcsDataThreadController::convertUAData(Channel_Data_t *_data, int uaType, int acctId){

    Channel_Data_t *data = (Channel_Data_t *)_data;
    char* sendData = NULL;

    sendData = (char* ) calloc(data->len + MD_CHANNEL_HEADER_LEN, sizeof(char));
    if (!sendData) {
        RFX_LOG_E(RFX_LOG_TAG, "convertUAData can't allocate the memory");
        return NULL;
    }

    (* ((int*)sendData)) = data->type;
    (* ((int*)sendData + 1)) = data->len;
    (* ((int*)sendData + 2)) = data->flag;
    (* ((int*)sendData + 3)) = acctId; //acct_id default vaule 0
    (* ((int*)sendData + 4)) = uaType;

    //data.data
    memcpy((sendData + MD_CHANNEL_HEADER_LEN), data->data, data->len);
    return (void*)sendData;
}

//return client channel obj
Ril_Channel_Obj_t *RmcRcsDataThreadController::getClientObjbyUaType(int uaType) {
    int i;
    if (!sChannelObj) {
        return 0;
    }
    if (!sChannelObj->channels) {
        return 0;
    }
    Ril_Channel_Client_t *clients = (Ril_Channel_Client_t *)sChannelObj->channels;
    for (i = 0; i < sChannelObj->channels_size; ++i) {
        Ril_Channel_Client_t *ccc = &(clients[i]);
        if (ccc->channel.fd >= 0 && ccc->channel.type == uaType) {
            return &(ccc->channel);
        }
    }
    return 0;
}

void * RmcRcsDataThreadController::shareMemoryThread(void *arg) {

    RFX_LOG_I(RFX_LOG_TAG, "[Share Mem Thd] Start");

    RFX_UNUSED(arg);

    while(1) {

        //if (sIsVtConnected == 0) {
            //RFX_LOG_D(RFX_LOG_TAG, "[RIL IMCB THD] sIsVtConnected : %d", sIsVtConnected);
            //usleep(500 * 1000);
            //continue;
        //}

        sShareMemory->lock("ril-md thread");

        sShareMemory->wait("ril-md thread", RCS_RIL_SHARE_DATA_STATUS_RECV_NONE);

        char *pRecvMsg;
        int size;
        Channel_Data_t recvData;
        int uaType; /* VoLTE_Stack_UA_Type_e */

        memset(&recvData, 0, sizeof(Channel_Data_t));
        sShareMemory->getData(&pRecvMsg);
        size = sShareMemory->getSize();
        recvData.type = (* ((int*)pRecvMsg));
        recvData.len = (* ((int*)pRecvMsg + 1));
        recvData.flag = (* ((int*)pRecvMsg + 2));
        //don't care acct_id
        uaType = (* ((int*)pRecvMsg + 4));

        RFX_LOG_I(RFX_LOG_TAG, "[Share Mem Thd] recv data length = %d, msgId = %d, uaType = %d\n",
            size, recvData.type, uaType);

        if (size != (recvData.len + (int)(MD_CHANNEL_HEADER_LEN))) {

            RFX_LOG_E(RFX_LOG_TAG, "[Share Mem Thd] recv data length is wrong, recvData.type = %d", recvData.type);

            sShareMemory->clearData();
            sShareMemory->setState(RCS_RIL_SHARE_DATA_STATUS_RECV_NONE);
            sShareMemory->unlock("ril-md thread");
            continue;
        }

        if (recvData.len > 0) {
            recvData.data = (char* )calloc(recvData.len, sizeof(char));
            if (!recvData.data) {
                RFX_LOG_E(RFX_LOG_TAG, "[Share Mem Thd] Can't allocate the memory");

                sShareMemory->clearData();
                sShareMemory->setState(RCS_RIL_SHARE_DATA_STATUS_RECV_NONE);
                sShareMemory->unlock("ril-md thread");
                continue;
            }
            memcpy(recvData.data, pRecvMsg + MD_CHANNEL_HEADER_LEN, recvData.len);
        }

        sendDataToUA((Channel_Data_t*)&recvData, uaType);
        if (recvData.data) {
            free(recvData.data);
        }

        sShareMemory->clearData();

        sShareMemory->setState(RCS_RIL_SHARE_DATA_STATUS_RECV_NONE);

        sShareMemory->unlock("ril-md thread");

    }
    RFX_LOG_E(RFX_LOG_TAG, "[Share Mem Thd] end of readerLooper, should not happen");

    return 0;
}

//write data to client UA
int RmcRcsDataThreadController::sendDataToUA(Channel_Data_t *data, int uaType) {
    Ril_Channel_Obj_t *obj = 0;
    int ret = 0;
    int id = 0;

    obj = getClientObjbyUaType(uaType);
    if (!obj) {
        RFX_LOG_E(RFX_LOG_TAG, "Client channel obj is NULL for ua connection type(%d)", uaType);
        return -1;
    }

    pthread_mutex_lock(&(obj->mutex));
    id = obj->fd;
    if (id < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "channel fd (%d) < 0, channel = %p", id, obj);
        pthread_mutex_unlock(&(obj->mutex));
        return -1;
    }

    do {
        ret = fsend(id, (char *)data, UA_CHANNEL_HEADER_LEN);
        if (ret != UA_CHANNEL_HEADER_LEN) {
            ret = -1;
            break;
        }

        if (data->len && data->data) {
            ret = fsend(id, (char *)data->data, data->len);
            if (ret != data->len) {
                ret = -1;
                break;
            }
        } else if (data->len || data->data) {
            RFX_LOG_E(RFX_LOG_TAG, "Channel send fail, length or data is null");
            ret = -1;
            break;
        }
        ret = 0;
    } while (0);
    pthread_mutex_unlock(&(obj->mutex));

    return ret;
}

int RmcRcsDataThreadController::fsend(int fd, char *data, int len) {
    int size = 0, ret = 0;
    int write_size = 0;
    int local_errno;

    if (fd <= 0) {
        return -1;
    }

    while (size < len) {
        write_size = len - size;

        ret = send(fd, data + size, write_size, 0);
        local_errno = errno;

        if (ret > 0) {
            size += ret;
        } else {
            if (errno == EINTR) {
                continue;
            }
            else if (errno == EPIPE) {
                ;
            }
            RFX_LOG_E(RFX_LOG_TAG, "Can't write the data to channel, error = (%d), fd = (%d)", local_errno, fd);
            break;
        }
    }
    return size;
}

void RmcRcsDataThreadController::sendMsgToShareMemory(char* outBuffer, int length, const char* user) {
    sp<RmcRcsSharedMemory> Mem  = getSharedMem();

    while (1) {
        if (!Mem->checkState(RCS_RIL_SHARE_DATA_STATUS_RECV_NONE)) {
            // wait finish sending data to UA
            usleep(200);

        } else {
            Mem->lock(user);
            break;
        }
    }

    Mem->setSize(length);
    Mem->setData(outBuffer, length);

    Mem->setState(RCS_RIL_SHARE_DATA_STATUS_RECV_DATA);

    Mem->signal(user);

    Mem->unlock(user);

}

bool RmcRcsDataThreadController::isLogEnable() {
    char enableLog[100] = {0};

    rfx_property_get("persist.vendor.rilrcs.log_enable", enableLog, "0");
    return (atoi(enableLog) == 1)? true: false;
}
