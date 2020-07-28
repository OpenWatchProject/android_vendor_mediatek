/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2016. All rights reserved.
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
#include "RfxRilAdapter.h"
#include "Rfx.h"
#include "RtstEnv.h"
#include "RfxBasics.h"
#include "RfxObject.h"
#include "RfxRootController.h"
#include "RtstHandler.h"
#include <sys/time.h>
#include "RtstMRil.h"

/*****************************************************************************
 * Define
 *****************************************************************************/
#define TAG "RTF"

extern RfxDispatchThread *dispatchThread;
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
#else
static request(
        int request,
        void *data,
        size_t datalen,
        RIL_Token t) {
    RIL_SOCKET_ID socket_id = RIL_SOCKET_1
#endif
    RFX_LOG_D(TAG, "rfx_enqueue_request_message %d", request);
    RequestInfo *pRI = (RequestInfo *)t;
    rfx_enqueue_request_message(request, data, datalen, t, socket_id);
}

/*****************************************************************************
 * class RtstCallback
 *****************************************************************************/

/*****************************************************************************
 * class RtstEnv
 *****************************************************************************/
::testing::Environment * const RtstEnv::s_env
    = ::testing::AddGlobalTestEnvironment(new RtstEnv());


void RtstEnv::initFd() {
    RFX_LOG_D(TAG, "RtstEnv::initFd");
    for (int i = 0; i < SIM_COUNT; i++) {
        RtstSocketPair *ril = new RtstSocketPair();
        m_rilSocketPairs.push(ril);
        RtstSocketPair *gsmRil = new RtstSocketPair();
        m_gsmRilSocketPairs.push(gsmRil);
    }
}


void RtstEnv::deinitFd() {
    Vector<RtstSocketPair *>::iterator it1;
    for (it1 = m_rilSocketPairs.begin(); it1 != m_rilSocketPairs.end();) {
        delete (*it1);
        it1 = m_rilSocketPairs.erase(it1);
    }
    m_rilSocketPairs.clear();
    for (it1 = m_gsmRilSocketPairs.begin(); it1 != m_gsmRilSocketPairs.end();) {
        delete (*it1);
        it1 = m_gsmRilSocketPairs.erase(it1);
    }
    m_gsmRilSocketPairs.clear();
}


void RtstEnv::init() {
    static bool inited = false;
    RFX_LOG_D(TAG, "RtstEnv::init");
    if (!inited) {
        initFd();
        rfx_init();
        inited = true;
        RtstGRil::setOnRequestCallback(request);
        RtstGRil::setVersion(RIL_VERSION);
        RtstMRil::setRilEnv();
        RFX_OBJ_GET_INSTANCE(RfxRilAdapter)->setSocket(
            RFX_SLOT_ID_0, RADIO_TECH_GROUP_C2K, getCdmaRilSocket2().getFd());
        RFX_OBJ_GET_INSTANCE(RfxRilAdapter)->setSocket(
            RFX_SLOT_ID_0, RADIO_TECH_GROUP_GSM, getGsmRilSocket2(RFX_SLOT_ID_0).getFd());
    }
}


void RtstEnv::deinit() {
    deinitFd();
}


void RtstEnv::sendRilRequest(int id, int slotId, Parcel &data) {
    RFX_LOG_D(TAG, "sendRilRequest id = %d, slotId = %d", id, slotId);
    RequestInfo *pRI = new RequestInfo();
    static int token = 0;
    pRI->token = token++;
    pRI->pCI = RtstGRil::getCommandInfo(id);
    pRI->p_next = NULL;
    pRI->socket_id = (RIL_SOCKET_ID)slotId;
    pRI->pCI->dispatchFunction(data, pRI);
    m_requestInfo.push(pRI);
}



bool RtstEnv::getExpectedRilRsp(int slotId, int &reqId, int &error, Parcel &p) {
    const RtstFd &s = getRilSocket1(slotId);
    int ret = RtstUtils::pollWait(s.getFd(), m_timeout);
    if (ret <=0 ) {
        return false;
    }
    unsigned char buf[1024];
    s.read(buf, 4);
    Parcel q;
    q.setData(buf, 4);
    q.setDataPosition(0);
    int size;
    q.readInt32(&size);
    int len = s.read(buf, size);
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
    s.read(buf, 4);
    Parcel q;
    q.setData(buf, 4);
    q.setDataPosition(0);
    int size;
    q.readInt32(&size);
    int len = s.read(buf, size);
    p.setData(buf, len);
    p.setDataPosition(0);
    p.readInt32(&urcId);

    return true;
}

void RtstEnv::sendRilResponse(
    int requestId,
    int slotId,
    Parcel &data,
    RIL_Errno error,
    RILD_RadioTechnology_Group source,
    int serial) {
    RFX_UNUSED(requestId);
    RFX_LOG_D(TAG, "sendRilResponse (requestId = %d, source = %d)", requestId, source);
    dispatchThread->enqueueResponseMessage(source, slotId, serial, error, &data);
}

void RtstEnv::sendRilUrc(int urcId, int slotId, RILD_RadioTechnology_Group source, Parcel &data) {
    RFX_LOG_D(TAG, "sendRilUrc (requestId = %d, slot = %d, source = %d)", urcId, slotId, source);
    dispatchThread->enqueueUrcMessage(source, slotId, urcId, &data);
}


bool RtstEnv::getExpectedReqToCdmaModem(int &reqId, int &token, Parcel &p) {
    const RtstFd &s = getCdmaRilSocket1();
    int ret = RtstUtils::pollWait(s.getFd(), m_timeout);
    if (ret <=0 ) {
        return false;
    }
    unsigned char buff[2048];
    uint8_t dataLength[4];
    s.read(dataLength, sizeof(dataLength));
    int size =  ((dataLength[2] << 8) & 0xFF00) | dataLength[3];
    RFX_LOG_D(TAG, "c2kRildRead (size = %d)", size);
    s.read(buff, size);
    p.setData(buff, size);
    p.setDataPosition(0);
    status_t status;
    status = p.readInt32(&reqId);
    status = p.readInt32 (&token);
    return true;
}

bool RtstEnv::getExpectedReqToGsmModem(int slotId, int &reqId, int &token, Parcel &p) {
    const RtstFd &s = getGsmRilSocket1(slotId);
    int ret = RtstUtils::pollWait(s.getFd(), m_timeout);
    if (ret <=0 ) {
        return false;
    }
    unsigned char buff[2048];
    uint8_t dataLength[4];
    s.read(dataLength, sizeof(dataLength));
    int size =  ((dataLength[2] << 8) & 0xFF00) | dataLength[3];
    RFX_LOG_D(TAG, "gsmRildRead slot = %d (size = %d)", slotId, size);
    s.read(buff, size);
    p.setData(buff, size);
    p.setDataPosition(0);
    status_t status;
    status = p.readInt32(&reqId);
    status = p.readInt32 (&token);
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


void writeRILSocket2(int slot, Parcel &p) {
    RtstEnv::get()->getRilSocket2(slot)
            .write((void *)p.data(), p.dataSize());
}

void RtstEnv::setStatus(int slot,  const RfxStatusKeyEnum key, const RfxVariant &value) {
    RFX_LOG_D(TAG, "setStatus slot = %d, key = %d, value = %s", slot, key,
            value.toString().string());
    RfxRootController *root = RFX_OBJ_GET_INSTANCE(RfxRootController);
    root->getStatusManager(slot)->setValue(key, value, false, true);
}

