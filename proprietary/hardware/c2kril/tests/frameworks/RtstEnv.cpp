/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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
#include <gtest/gtest.h>
#include <cutils/properties.h>
#include <string.h>
#include <sys/time.h>
#include "RtstEnv.h"
#include "RtstLog.h"
#include "atchannel.h"

/*****************************************************************************
 * Define
 *****************************************************************************/
#define TAG "RTF"

/*****************************************************************************
 * External Functions
 *****************************************************************************/
extern "C" int rilNwMain(int request, void *data, size_t datalen, RIL_Token t);
extern "C" int rilDataMain(int request, void *data, size_t datalen, RIL_Token t);
extern "C" int rilCcMain(int request, void *data, size_t datalen, RIL_Token t);
extern "C" int rilSimMain(int request, void *data, size_t datalen, RIL_Token t);
extern "C" int rilSmsMain(int request, void *data, size_t datalen, RIL_Token t);
extern "C" int rilUtkMain(int request, void *data, size_t datalen, RIL_Token t);
extern "C" int rilOemMain(int request, void *data, size_t datalen, RIL_Token t);
extern "C" int rilNwUnsolicited(const char *s, const char *sms_pdu);
extern "C" int rilDataUnsolicited(const char *s, const char *sms_pdu);
extern "C" int rilCcUnsolicited(const char *s, const char *sms_pdu);
extern "C" int rilSimUnsolicited(const char *s, const char *sms_pdu);
extern "C" int rilSmsUnsolicited(const char *s, const char *sms_pdu);
extern "C" int rilUtkUnsolicited(const char *s, const char *sms_pdu);
extern "C" int rilOemUnsolicited(const char *s, const char *sms_pdu);
extern "C" void onRequestComplete(RIL_Token t, RIL_Errno e, void *response, size_t responselen);

/*****************************************************************************
 * Static functions
 *****************************************************************************/
#if defined(ANDROID_MULTI_SIM)
static void request(
        int request,
        void *data,
        size_t datalen,
        RIL_Token t,
        RIL_SOCKET_ID socket_id) {
    RTST_UNUSED(socket_id);
#else
static void request(
        int request,
        void *data,
        size_t datalen,
        RIL_Token t) {
    RIL_SOCKET_ID socket_id = (RIL_SOCKET_ID)0;
#endif
    RTST_LOG_D(TAG, "request entry point %d", request);
    RILChannelCtx *ctx = getRILChannelCtxFromToken(t);
    ctx->id = ((RequestInfo *)t)->cid;
    ctx->fd = socket_id; // use fd to store the slot Id
    if (!(rilNwMain(request, data, datalen, t)
            || rilSmsMain(request, data, datalen, t)
            || rilOemMain(request, data, datalen, t)))
    {
        onRequestComplete(t, RIL_E_REQUEST_NOT_SUPPORTED, NULL, 0);
    }
}



/*****************************************************************************
 * class RtstCallback
 *****************************************************************************/
void RtstCallback::handleRilReq(int fd) {
    RTST_UNUSED(fd);
    const int size = 2048;
    unsigned char buf[size];
    int len = m_env->getReqSocket2().read(buf, size);
    Parcel p;
    p.setData(buf, len);
    p.setDataPosition(0);
    int slotId;
    p.readInt32(&slotId);
    int id;
    p.readInt32(&id);

    RequestInfo *pRI = new RequestInfo();
    static int token = 0;
    pRI->token = token++;
    pRI->pCI = RtstGRil::getCommandInfo(id);
    pRI->p_next = NULL;
    pRI->socket_id = (RIL_SOCKET_ID)slotId;
    pRI->cid = (RILChannelId)pRI->pCI->proxyId;
    pRI->pCI->dispatchFunction(p, pRI);
    m_env->m_requestInfo.push(pRI);
}

void RtstCallback::handleAtUrc(int fd,int slot) {
    const int size = 2048;
    unsigned char buf[size];
    RTST_UNUSED(fd);
    m_env->getAtSocket2(URC_CHANNEL, slot).read(buf, 2048);
    const char *p = strchr((const char *)buf, '\r');
    *((char*)p) = '\0';
    if (!(rilNwUnsolicited((const char*)buf, NULL)
            || rilSmsUnsolicited((const char*)buf, NULL)
            || rilOemUnsolicited((const char*)buf, NULL)))  {
        RTST_LOG_E(TAG, "Unhandled unsolicited result code: %s\n", buf);
    }
}

int RtstCallback::handleEvent(int fd, int events, void* arg) {
    RTST_UNUSED(fd);
    RTST_UNUSED(arg);
    if (events == Looper::EVENT_INPUT) {
        if (fd == m_env->getReqSocket2().getFd()) {
            handleRilReq(fd);
        } else {
            int slot;
            if (m_env->isAtUrcSocket2Fd(fd, &slot)) {
                handleAtUrc(fd, slot);
            }
        }
    }
    return 1;
}

/*****************************************************************************
 * class RtstEnv
 *****************************************************************************/
::testing::Environment * const RtstEnv::s_env
    = ::testing::AddGlobalTestEnvironment(new RtstEnv());

void RtstEnv::initFd(int fd[SIM_COUNT][RTST_MAX_CHANNEL_NUM]) {
    RTST_LOG_D(TAG, "RtstEnv::initFd");
    for (int i = 0; i < SIM_COUNT; i++) {
        RtstSocketPair *ril = new RtstSocketPair();
        m_rilSocketPairs.push(ril);

        Vector<RtstSocketPair *> * atPair = new Vector<RtstSocketPair* >();
        for (int j = 0; j < RTST_MAX_CHANNEL_NUM; j++) {
            RtstSocketPair *at = new RtstSocketPair();
            atPair->push(at);
            fd[i][j] = at->getSocket2().getFd();
        }
        m_atSocketPairs.push(atPair);
    }
}


void RtstEnv::deinitFd() {
    Vector<RtstSocketPair *>::iterator it1;
    for (it1 = m_rilSocketPairs.begin(); it1 != m_rilSocketPairs.end();) {
        delete (*it1);
        it1 = m_rilSocketPairs.erase(it1);
    }
    m_rilSocketPairs.clear();
    Vector<Vector<RtstSocketPair*> *>::iterator it4;
    for (it4 = m_atSocketPairs.begin(); it4 != m_atSocketPairs.end(); it4++) {
        Vector<RtstSocketPair *>::iterator it41;
        for (it41 = (*it4)->begin(); it41 != (*it4)->end(); it41++) {
            delete (*it41);
        }
        (*it4)->clear();
        delete (*it4);
    }
    m_atSocketPairs.clear();
}



void RtstEnv::init() {
    static bool inited = false;
    RTST_LOG_D(TAG, "RtstEnv::init");
    if (!inited) {
        int fd[SIM_COUNT][RTST_MAX_CHANNEL_NUM];
        initFd(fd);
        /// SET MockFlag, ChannelFd, RilEnv
        RtstMRil::setEmulatorMode();
        RtstMRil::setRilEnv();
        RtstMRil::setAtHooker();
        initBootAtRecvThread();
        inited = true;
        RtstGRil::setOnRequestCallback(request);
        RtstGRil::setVersion(RIL_VERSION);
        const RtstFd &s = getInitSocket1();
        unsigned char buf[1024];
        s.read(buf, 1024);
        RTST_LOG_D(TAG, "RtstEnv::init Done");
    }
}

void RtstEnv::initBootAtRecvThread() {
    int fd[1 + SIM_COUNT];
    fd[0] = getReqSocket2().getFd();
    int count = 1;
    for (int i = 0; i < SIM_COUNT; i++) {
        fd[i + 1] = getAtSocket2(URC_CHANNEL, i).getFd();
        count++;
    }
    RTST_LOG_D(TAG, "RtstEnv::initBootAtRecvThread %d ", count);
    sp<RtstThread> t = RtstThread::create(fd, count);
    t->setCallback(new RtstCallback(this));
}


void RtstEnv::deinit() {
    deinitFd();
}


void RtstEnv::sendRilRequest(int id, int slotId, Parcel &data) {
    RTST_LOG_D(TAG, "sendRilRequest id = %d, slotId = %d", id, slotId);
    Parcel p;
    p.writeInt32(slotId);
    p.writeInt32(id);
    p.write(data.data(), data.dataSize());
    RtstEnv::get()->getReqSocket1().write((void *)p.data(), p.dataSize());
}


void RtstEnv::sendAtResponse(int channelId, int slotId, const char *rsp) {
    RTST_LOG_D(TAG, "sendAtResponse channelId = %d, slotId = %d, rsp = %s", channelId, slotId, rsp);
    String8 at = String8(rsp) + String8("\r");
    const RtstFd &s = getAtSocket1(channelId, slotId);
    s.write((void *)at.string(), at.length());
}


void RtstEnv::sendUrcString(int channelId, int slotId, const char *urc) {
    RTST_LOG_D(TAG, "sendUrcString slotId = %d, urc = %s", slotId, urc);
    String8 at = String8(urc) + String8("\r");
    const RtstFd &s = getAtSocket1(channelId, slotId);
    s.write((void *)at.string(), at.length());
}


void RtstEnv::setSysProp(const char *key, const char *value) {
    RTST_UNUSED(key);
    RTST_UNUSED(value);
    RTST_LOG_D(TAG, "setSysProp key = %s, value = %s", key, value);
    //rfx_property_set(key, value);
}



bool RtstEnv::getExpectedAt(int channelId, int slotId, String8 &expectedAt) {
    const RtstFd &s = getAtSocket1(channelId, slotId);
    int ret = RtstUtils::pollWait(s.getFd(), m_timeout);
    if (ret <=0 ) {
        return false;
    }
    char buf[1024];
    s.read(buf, 1024);
    char *p = strchr(buf, '\r');
    *p = '\0';
    expectedAt.setTo(buf);
    return true;
}

bool RtstEnv::getExpectedRilRsp(int slotId, int &reqId, int &error, Parcel &p) {
    const RtstFd &s = getRilSocket1(slotId);
    int ret = RtstUtils::pollWait(s.getFd(), m_timeout);
    if (ret <=0 ) {
        return false;
    }
    unsigned char buf[1024];
    int len = s.read(buf, 1024);
    p.setData(buf, len);
    p.setDataPosition(0);
    p.readInt32(&reqId);
    p.readInt32(&error);
    return true;
}

bool RtstEnv::getExpectedRilUrc(int slotId, int &urcId, Parcel &p) {
    const RtstFd &s = getRilSocket1(slotId);
    int ret = RtstUtils::pollWait(s.getFd(), m_timeout);
    if (ret <=0 ) {
        return false;
    }
    unsigned char buf[1024];
    int len = s.read(buf, 1024);
    p.setData(buf, len);
    p.setDataPosition(0);
    p.readInt32(&urcId);
    return true;
}



bool RtstEnv::getExpectedSysProp(const String8 &key, String8 &value, int delay) {
    RTST_UNUSED(key);
    if (delay != 0) {
        sleep(delay);
    }
    char str[PROPERTY_VALUE_MAX];
    //rfx_property_get(key.string(), str, "");
    value.setTo(str);
    RTST_LOG_D(TAG, "getExpectedSysProp key = %s, value = %s", key.string(), value.string());
    return true;
}

void RtstEnv::releaseRequestInfo() {
    Vector<RequestInfo *>::iterator it;
    for (it = m_requestInfo.begin(); it != m_requestInfo.end();) {
        delete (*it);
        it = m_requestInfo.erase(it);
    }
}

bool RtstEnv::isRilSocket1Fd(int fd, int *slotId) {
    for (int i = 0; i < SIM_COUNT; i++) {
        if (fd == getRilSocket1(i).getFd()) {
            if (slotId != NULL) {
                *slotId = i;
            }
            return true;
        }
    }
    return false;
}

bool RtstEnv::isRilSocket2Fd(int fd, int *slotId) {
    for (int i = 0; i < SIM_COUNT; i++) {
        if (fd == getRilSocket2(i).getFd()) {
            if (slotId != NULL) {
                *slotId = i;
            }
            return true;
        }
    }
    return false;
}

bool RtstEnv::isAtSocket1Fd(int fd,int * slotId) {
    for (int i = 0; i < SIM_COUNT; i++) {
        for (int j = 0; j < RTST_MAX_CHANNEL_NUM; j++) {
            if ((*m_atSocketPairs[i])[j]->getSocket1().getFd() == fd) {
                *slotId = i;
                return true;
            }
        }
    }
    return false;
}

bool RtstEnv::isAtUrcSocket2Fd(int fd, int *slotId) {
    for (int i = 0; i < SIM_COUNT; i++) {
        if ((*m_atSocketPairs[i])[URC_CHANNEL]->getSocket2().getFd() == fd) {
            *slotId = i;
            return true;
        }
    }
    return false;
}



/*****************************************************************************
 * class RtstThread
 *****************************************************************************/
RtstThread::RtstThread() :
    m_looper(NULL),
    m_looperCallback(NULL),
    m_fds(NULL),
    m_fdNum(0) {
}

void RtstThread::setFds(int fds[], int fdNum) {
    m_fds = new int[fdNum];
    m_fdNum = fdNum;
    memcpy(m_fds, fds, fdNum * sizeof(int));
}

void RtstThread::setCallback(sp<LooperCallback> callback) {
    m_looperCallback = callback;
}

sp<RtstThread> RtstThread::create(int fds[], int fdNum) {
    RtstThread *t = new RtstThread();
    if (fdNum > 0) {
        t->setFds(fds, fdNum);
    }
    t->run("RfxTestThread");
    return t;
}


status_t RtstThread::readyToRun() {
    m_looper = Looper::prepare(0);
    for (int i =0 ; i < m_fdNum; i++) {
        m_looper->addFd(m_fds[i],
            Looper::POLL_CALLBACK,
            Looper::EVENT_INPUT,
            m_looperCallback,
            NULL);
    }
    unsigned char buf[1024];
    strcpy((char *)buf, "OK");
    RtstEnv::get()->getInitSocket2().write(buf, 2);
    return android::NO_ERROR;
}


bool RtstThread::threadLoop() {
    m_looper->pollAll(-1);
    return true;
}


sp<Looper> RtstThread::getLooper() {
    return m_looper;
}
