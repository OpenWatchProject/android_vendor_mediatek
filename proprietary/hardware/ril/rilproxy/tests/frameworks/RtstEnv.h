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
#include "RfxVariant.h"
#include "RfxStatusDefs.h"
#include "RtstData.h"
#include "RfxClassInfo.h"
#include "RtstGRil.h"

/*****************************************************************************
 * Class Declaration
 *****************************************************************************/
class RtstEnv;
struct RadioImpl;

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
#define MAX_RFX_SLOT_COUNT 4
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
} RequestInfo;


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

    void sendRilResponse(
        int requestId,
        int slotId,
        Parcel &data,
        RIL_Errno error,
        RILD_RadioTechnology_Group source,
        int serial);

    void sendRilUrc(
        int urcId,
        int slotId,
        RILD_RadioTechnology_Group source,
        Parcel &data);

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

    bool getExpectedReqToCdmaModem(
        int &reqId,
        int &token,
        Parcel &p);


    bool getExpectedReqToGsmModem(
        int slotId,
        int &reqId,
        int &token,
        Parcel &p);

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

    const RtstFd& getCdmaRilSocket1() {
        return m_cdmaRilSocket.getSocket1();
    }

    const RtstFd& getCdmaRilSocket2() {
        return m_cdmaRilSocket.getSocket2();
    }

    // Get the Gsm RIL socket for UT framework
    //
    // RETURNS: RIL socket
    const RtstFd& getGsmRilSocket1(
        int slotId                       // [IN] slot ID
    ) {
        return m_gsmRilSocketPairs[slotId]->getSocket1();
    }

    // Get the Gsm RIL socket for vendor RIL
    //
    // RETURNS: RIL socket
    const RtstFd& getGsmRilSocket2(
        int slotId                       // [IN] slot ID
    ) {
        return m_gsmRilSocketPairs[slotId]->getSocket2();
    }


    void cleanCdmaRilSocket() {
        RtstUtils::cleanSocketBuffer(getCdmaRilSocket1(), m_timeout);
    }

    void cleanGsmRilSocket() {
        RtstUtils::cleanSocketBuffer(getGsmRilSocket1(0), m_timeout);
        RtstUtils::cleanSocketBuffer(getGsmRilSocket1(0), m_timeout);
    }

    // Set the status
    //
    // RETURNS: void
    void setStatus(
        int slot,                     // [IN] slot ID
        const RfxStatusKeyEnum key,   // [IN] key of the status
        const RfxVariant &value       // [IN] value of the status
    );

    // free the request Info
    //
    // RETURNS: void
    void releaseRequestInfo();

    // Init the vendor ril
    //
    // RETURNS: void
    void init();

    void deinitFd();

    void setFdTimeoutValue(int timeout = 1000) {
        m_timeout = timeout;
    }

// Overide
protected:
    virtual void SetUp() {
        static int count = 1;
        RFX_LOG_D(RTST_TAG, "Global Setup %d", count++);
        setFdTimeoutValue();
    }
    virtual void TearDown() {
        static int count = 1;
        RFX_LOG_D(RTST_TAG, "Global TearDown %d", count++);
    }

// Implementation
private:
    void deinit();
    void initFd();
    bool isRilSocket1Fd(int fd, int *slotId);


    static ::testing::Environment * const s_env;


    Vector<RequestInfo *> m_requestInfo;
    Vector<RtstSocketPair* > m_rilSocketPairs;
    RtstSocketPair m_cdmaRilSocket;
    Vector<RtstSocketPair* > m_gsmRilSocketPairs;

    int m_timeout;
};

#endif /* __RTST_ENV_H__ */