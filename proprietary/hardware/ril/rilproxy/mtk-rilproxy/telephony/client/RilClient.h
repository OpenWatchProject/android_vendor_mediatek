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

#ifndef __RIL_CLIENT_H__
#define __RIL_CLIENT_H__

#include <stddef.h>
#include <utils/Thread.h>
#include "utils/RefBase.h"
#include "utils/Errors.h"
#include "RilClientConstants.h"
#include <telephony/record_stream.h>
#include <binder/Parcel.h>

using namespace android;

typedef enum {
    CLIENT_STATE_UNKNOWN,
    CLIENT_INITIALIZING,
    CLIENT_ACTIVE,
    CLIENT_DEACTIVE,
    CLIENT_CLOSED,
} RilClientState;

class RilClient {

public:

    RilClient(int identity, char* socketName);
    virtual ~RilClient();
    void setClientState(RilClientState state);

    int identity;
    char* socketName;
    int commandFd;
    RecordStream *stream;
    int listenFd;
    RilClientState clientState;

    class StateActivityThread: public Thread
    {
    public:
        StateActivityThread(RilClient* client);
        virtual ~StateActivityThread();

    protected:
        RilClient* client;
        virtual bool threadLoop();
    };

    sp<StateActivityThread> activityThread;

    static char* clientStateToString(RilClientState state);
    void clientStateCallback();
    void handleStateInitializing();
    void handleStateActive();
    void handleStateDeactive();
    void handleStateClosed();

    // Override, if you have other imprementation
    virtual void processCommands(void *buffer, size_t buflen, int clientId);
    virtual void handleUnsolicited(int slotId, int unsolResponse, const void *data,
                                size_t datalen);
    virtual void addHeaderToResponse(Parcel* p, int slotId);

};

#endif /* __RP_RADIO_CONTROLLER_H__ */