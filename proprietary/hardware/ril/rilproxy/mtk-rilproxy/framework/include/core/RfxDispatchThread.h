/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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
/*
 * RfxDiaptchThread.h
 *
 *  Created on: 2015/8/04
 *  Author: MTK10602
 *
 */

#ifndef __RFX_DISPATCH_THREAD_H__
#define __RFX_DISPATCH_THREAD_H__

#include <utils/threads.h>
#include "RfxObject.h"
#include "RfxMessage.h"
#include "RfxDispatchQueue.h"
#include "RfxMainThread.h"
#include "RfxRilAdapter.h"
#include "RfxTransferUtils.h"
#include "RfxDefaultDestUtils.h"
#include "RfxLog.h"

using ::android::Thread;
using ::android::sp;

typedef void* (*PthreadPtr)(void*);

extern "C" void *rfx_process_request_messages_loop(void *arg);
extern "C" void *rfx_process_response_messages_loop(void *arg);
extern "C" void *rfx_process_urc_messages_loop(void *arg);
extern "C" void *rfx_process_status_sync_messages_loop(void *arg);

typedef struct MessageObj {
    sp<RfxMessage> msg;
    struct MessageObj *p_next;
} MessageObj;
static Dispatch_queue<MessageObj> dispatchRequestQueue;
static Dispatch_queue<MessageObj> dispatchResponseQueue;
static Dispatch_queue<MessageObj> dispatchUrcQueue;
static Dispatch_queue<MessageObj> dispatchStatusSyncQueue;
static Dispatch_queue<MessageObj> pendingQueue;
MessageObj* createMessageObj(const sp<RfxMessage>& message);

class RfxDispatchThread : public Thread {
private:

    RfxDispatchThread();

    virtual ~RfxDispatchThread();

public:

    typedef struct {
        int requestNumber;
        void(*dispatchFunction) (Parcel &p, struct RequestInfo *pRI);
        int(*responseFunction) (Parcel &p, void *response, size_t responselen);
    } CommandInfo;

    typedef struct RequestInfo {
        int32_t token;      // this is not RIL_Token
        CommandInfo *pCI;
        struct RequestInfo *p_next;
        char cancelled;
        char local;         // responses to local commands do not go back to command process
        RIL_SOCKET_ID socket_id;
    } RequestInfo;

    static RfxDispatchThread* init();

    void enqueueRequestMessage(int request, void *data, size_t datalen, RIL_Token t,
            RIL_SOCKET_ID socket_id);
    void enqueueRequestMessageOld(int request, int slotId, RILD_RadioTechnology_Group dest,
            int token, void *data);
    void enqueueRequestMessage(int request, int slotId, RILD_RadioTechnology_Group dest,
            int token, void *data, int clientId);
    void enqueueRequestAckMessage(RILD_RadioTechnology_Group source, int slotId, int token,
            Parcel *parcel);
    void enqueueResponseMessage(RILD_RadioTechnology_Group source, int slotId, int token,
            RIL_Errno error, Parcel *parcel);
    void enqueueUrcMessage(RILD_RadioTechnology_Group source, int slotId, int urcId,
            Parcel *parcel);
    static void addMessageToPendingQueue(const sp<RfxMessage>& message);
    static void clearPendingQueue();
    static void removeMessageFromPendingQueue(int token);
    static void updateConnectionState(RIL_SOCKET_ID socket_id, int isConnected);

private:

    virtual bool threadLoop();

    void processRequestMessageLooper();
    void processResponseMessageLooper();
    void processUrcMessageLooper();
    void processStatusSyncMessageLooper();

    friend void *::rfx_process_request_messages_loop(void *arg);
    friend void *::rfx_process_response_messages_loop(void *arg);
    friend void *::rfx_process_urc_messages_loop(void *arg);
    friend void *::rfx_process_status_sync_messages_loop(void *arg);

private:

    static RfxDispatchThread *s_self;

    pthread_t requestThreadId;
    pthread_t responseThreadId;
    pthread_t urcThreadId;
    pthread_t statusSyncThreadId;

};
#endif /* __RFX_DISPATCH_THREAD_H__ */
