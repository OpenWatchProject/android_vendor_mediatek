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

#ifndef __RIL_OEM_CLIENT_H__
#define __RIL_OEM_CLIENT_H__

#include <stdint.h>
#include "RilClient.h"
#include "RfxMessage.h"
#include "RfxAtLine.h"

class RilOemClient : public RilClient {

protected:
    typedef enum {
        FAILURE = -1,
        SUCCESS = 0,
        NO_ACTION = 1,
    } Result;
    typedef enum {
        GAME_MODE_NONE = 0,
        GAME_MODE_LOADING,
        GAME_MODE_IN_MATCH,
        GAME_MODE_IDLE,
        GAME_MODE_BATTLE,
        GAME_MODE_BOOST,
    } GameMode;

    typedef enum {
        STATE_PAUSED = 0,
        STATE_RESUMED = 1,
        STATE_DESTORYED = 2,
        STATE_DEAD = 3,
        STATE_STOPPED = 4,
        STATE_NUM = 5,
    } AppStatus;

public:
    RilOemClient(int identity, char* socketName);
    virtual ~RilOemClient();

    // Override, if you have other imprementation
    /* oem socket would not handle any urc */
    virtual void handleUnsolicited(int slotId, int unsolResponse, void *data,
            size_t datalen, UrcDispatchRule rule);
    virtual void handleStateActive();
    virtual void requestComplete(RIL_Token token, RIL_Errno e, void *response,
            size_t responselen);

    virtual int handleSpecialRequestWithArgs(int argCount, char** args);

private:
    void freeArgs(char** args, int number);
    void executeThermal(char* arg);
    void executeHangupAll();
    void executeShutDownByThermal(char* arg);
    void executeGameMode(char* arg);
    void executeUpdatePacket(char* arg);
    void executeLowLatencyMode(char* arg);
    void executeWeakSignalOpt(char* arg);
    void executeAppStatus(char* arg);
    void executeQueryCapability(char *arg);
    int mThermalFd = -1;
    int mQueryCapFd = -1;
    int mLowLatencyMode = 0;
    int mGameMode = 0;
    String8 mPhantomPacket;
};
#endif
