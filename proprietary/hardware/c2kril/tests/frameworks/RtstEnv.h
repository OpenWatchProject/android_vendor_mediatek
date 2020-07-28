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
#ifndef __RTST_ENV_H__
#define __RTST_ENV_H__
/*****************************************************************************
 * Include
 *****************************************************************************/
#include <Parcel.h>
#include <gtest/gtest.h>
#include <utils/Looper.h>
#include <utils/threads.h>
#include <map>
#include "RtstSocket.h"
#include "RtstUtils.h"
#include "RtstGRil.h"
#include "RtstMRil.h"
#include "RtstData.h"


/*****************************************************************************
 * Class Declaration
 *****************************************************************************/
class RtstEnv;

/*****************************************************************************
 * Name Space
 *****************************************************************************/
using ::android::Parcel;
using ::android::sp;
using ::android::wp;
using ::android::LooperCallback;
using ::android::Looper;
using ::android::Thread;
using ::android::status_t;
using ::android::Vector;
using ::testing::Environment;

/*****************************************************************************
 * Define
 *****************************************************************************/
#define RTST_MAX_SIM_COUNT 4

/*****************************************************************************
 * Typedef
 *****************************************************************************/

/**
 * The following struct
 *   -RequestInfo
 * are copied from ril.cpp in google libril.
 * We need these type to emulate the RIL request from RILJ in
 * test framework.
 */
typedef struct RequestInfo {
    int32_t token;      //this is not RIL_Token
    CommandInfo *pCI;
    struct RequestInfo *p_next;
    char cancelled;
    char local;         // responses to local commands do not go back to command process
    RIL_SOCKET_ID socket_id;
    int wasAckSent;    // Indicates whether an ack was sent earlier
    RILChannelId cid;
} RequestInfo;


/*****************************************************************************
 * class RtstCallback
 *****************************************************************************/
/*
 * The looper callback when RfxTest thread IN fd is readable
 */
class RtstCallback: public LooperCallback {
// Constructor / Destructor
public:
    // Constructor
    RtstCallback(
        RtstEnv *env // [IN] The test Env
    ): m_env(env){}

    // Destructor
    virtual ~RtstCallback() {}

// Override
protected:
    int handleEvent(int fd, int events, void* data);

// Implement
private:
    void handleRilReq(int fd);
    void handleAtUrc(int fd, int slot);
    // reference of test env
    RtstEnv* m_env;
};

/*****************************************************************************
 * Class RtstThread
 *****************************************************************************/
/*
 * The Thread we can monitor the fd
 */
class RtstThread : public Thread {
public:
    // Constructor
    RtstThread();

    // Destructor
    virtual ~RtstThread() {
        if (m_fds != NULL) {
            delete [] m_fds;
        }
    }

    // Create the test thread
    //
    // RETURNS: the pointer of the thread
    static sp<RtstThread> create(
        int fds[],   // [IN] the fds to monitor
        int fdNum    // [IN] the number of fd
    );

    // Get the Looper of the AGPS working thread
    // RETURNS: the looper of the thead
    sp<Looper> getLooper();

    // Set the Fd to be monitored
    //
    // RETURNS: void
    void setFds(
        int fds[],  // the fds to be monitored
        int fdNum   // The number of the fd
    );

    // Set the looper callback
    //
    // RETURNS: void
    void setCallback(
        sp<LooperCallback> callback // [IN] the looper callback
    );

// Override
protected:
    virtual bool threadLoop();
    virtual status_t readyToRun();

// Implement
private:
    // the looper that is attached to this thread
    sp<Looper> m_looper;

    // The reference of the looper callback when input fd readable
    sp<LooperCallback> m_looperCallback;

    int* m_fds;
    int m_fdNum;
};

/*****************************************************************************
 * Classe RtstEnv
 *****************************************************************************/
/*
 * Test Enviroment is used when testing the RIL
 */
class RtstEnv : public Environment {
// External Method
public:
    // Get the instance of RtstEnv
    //
    // RETURNS: the pointer of RtstEnv
    static RtstEnv *get() {
        return (RtstEnv *)s_env;
    }

// External Method
public:
    // Send a RIL request to vendor ril
    //
    // RETURNS: void
    void sendRilRequest(
        int requestId,    // [IN] ril request id
        int slotId,       // [IN] slot ID
        Parcel &data      // [IN] the data of this request id
    );

    // Send a AT response to vendor ril
    //
    // RETURNS: void
    void sendAtResponse(
        int channelId,   // [IN] AT command channel ID
        int slotId,      // [IN] slot ID
        const char *rsp  // [IN] AT response string
    );


    // Send a URC string to vendor ril
    //
    // RETURNS: void
    void sendUrcString(
        int channelId,   // [IN] AT command channel ID
        int slotId,      // [IN] slot ID
        const char *urc  // [IN] urc string
    );


    // Set the system prop
    //
    // RETURNS: void
    void setSysProp(
        const char *key,   // [IN] key of the system property
        const char *value  // [IN] value of the system property
    );


    // Check the expectedAt
    //
    // RETURNS: timeout happen if false
    bool getExpectedAt(
        int channelId,            // [IN] AT command channel ID
        int slotId,               // [IN] Slot Id
        String8 &expectedAt       // [OUT] The expected AT command
    );

    // Check the expected RIL response
    //
    // RETURNS: timeout happen if false
    bool getExpectedRilRsp(
        int slotId,          // [IN] The slot ID
        int &reqId,          // [OUT] The expected slot ID
        int &error,          // [OUT] The expected error code
        Parcel &p            // [OUT] The expected data
    );

    // Check the expected RIL URC
    //
    // RETURNS: timeout happen if false
    bool getExpectedRilUrc(
        int slotId,         // [IN] The slot ID
        int &urcId,         // [OUT] The expected urc ID
        Parcel &p           // [OUT] The expected data
    );


    // Check the expected system property
    //
    // RETURNS: reserved
    bool getExpectedSysProp(
        const String8 &key,  // [IN] The key of the system property
        String8 &value,      // [OUT] the expected value of the system property
        int delay = 10       // [IN] default delay 10s to check the value
    );

    // Get the RIL socket for UT framework
    //
    // RETURNS: RIL socket
    const RtstFd& getRilSocket1(
        int slotId                       // [IN] slot ID
    ) {
        return m_rilSocketPairs[slotId]->getSocket1();
    }

    // Get the RIL socket for vendor RIL
    //
    // RETURNS: RIL socket
    const RtstFd& getRilSocket2(
        int slotId                       // [IN] slot ID
    ) {
        return m_rilSocketPairs[slotId]->getSocket2();
    }

    // Get the AT socket for UT framework
    //
    // RETURNS: AT socket
    const RtstFd& getAtSocket1(
        int channelId,               // [IN] channel ID
        int slotId                   // [IN] slot ID
    ) {
        return (*m_atSocketPairs[slotId])[channelId]->getSocket1();
    }

    // Get the AT socket for vendor RIL
    //
    // RETURNS: At socket
    const RtstFd& getAtSocket2(
        int channelId,              // [IN] channel ID
        int slotId                  // [IN] slot ID
    ) {
        return (*m_atSocketPairs[slotId])[channelId]->getSocket2();
    }

    // Get the Init Sync socket 1
    //
    // RETURNS: the init sync socket 1
    const RtstFd& getInitSocket1() {
        return m_initSocket.getSocket1();
    }

    // Get the Init Sync socket 2
    //
    // RETURNS: the init sync socket 2
    const RtstFd& getInitSocket2() {
        return m_initSocket.getSocket2();
    }

    const RtstFd& getReqSocket1() {
        return m_reqSocket.getSocket1();
    }

    const RtstFd& getReqSocket2() {
        return m_reqSocket.getSocket2();
    }

    // free the request Info
    //
    // RETURNS: void
    void releaseRequestInfo();

    // Init the vendor ril
    //
    // RETURNS: void
    void init();

    void deinitFd();


    void cleanRilSocket(int slotId) {
        RtstUtils::cleanSocketBuffer(getRilSocket1(slotId), m_timeout);
    }

    void cleanAllRilSocket() {
        for (int i = 0; i < SIM_COUNT; i++) {
            RtstUtils::cleanSocketBuffer(getRilSocket1(i), m_timeout);
        }
    }

    void setFdTimeoutValue(int timeout = 1000) {
        m_timeout = timeout;
    }

// Overide
protected:
    virtual void SetUp() {
        setFdTimeoutValue();
    }
    virtual void TearDown() {
    }

// Implementation
private:
    void deinit();
    void initBootAtRecvThread();
    bool isRilSocket1Fd(int fd, int *slotId);
    bool isRilSocket2Fd(int fd, int *slotId);
    bool isAtSocket1Fd(int fd, int *slotId);
    bool isAtUrcSocket2Fd(int fd, int *slotId);
    void initFd(int fd[SIM_COUNT][RTST_MAX_CHANNEL_NUM]);
    static ::testing::Environment * const s_env;

    RtstSocketPair  m_initSocket;
    RtstSocketPair  m_reqSocket;
    Vector<RequestInfo *> m_requestInfo;
    Vector<RtstSocketPair* > m_rilSocketPairs;
    Vector<Vector<RtstSocketPair*> *> m_atSocketPairs;

    int m_timeout;
    friend class RtstCallback;
};

#endif /* __RTST_ENV_H__ */