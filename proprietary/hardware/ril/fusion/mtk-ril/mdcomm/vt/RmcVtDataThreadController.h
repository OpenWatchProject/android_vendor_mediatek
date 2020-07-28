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

#ifndef __RMC_VT_DATA_THREAD_CONTROLLER_H__
#define __RMC_VT_DATA_THREAD_CONTROLLER_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RfxBaseHandler.h"
#include <pthread.h>

/*****************************************************************************
 * Define
 *****************************************************************************/
#define VT_RIL_SHARE_DATA_STATUS_RECV_NONE      (0)
#define VT_RIL_SHARE_DATA_STATUS_RECV_DATA      (1)

/*****************************************************************************
 * Class RmcVtReqHandlerSharedMemory
 *****************************************************************************/
class RmcVtSharedMemory: virtual public RefBase {

    public:
        RmcVtSharedMemory(void);
        virtual ~RmcVtSharedMemory();

        void setState(int state);
        int getState(void);
        bool checkState(int want_state);
        void setSlotId(int id);
        void setSize(int size);
        void setData(char* data, int len);
        int getSlotId(void);
        int getSize(void);
        void getData(char** data);
        void clearData();
        void lock(const char* user);
        void unlock(const char* user);
        void wait(const char* user, int stay_state);
        void signal(const char* user);

    private:
        pthread_mutex_t      mLock;
        pthread_mutex_t     *mPLock;
        pthread_cond_t       mCond;
        int                  mDataReadyCount;
        RIL_VT_SERVICE_MSG   mSharedMsg;

};

/*****************************************************************************
 * Class RmcVtDataThreadController
 *****************************************************************************/
class RmcVtDataThreadController: virtual public RefBase {

    public:
        RmcVtDataThreadController(void);
        virtual ~RmcVtDataThreadController();

        void start();

        static void *RIL_IMCB_THREAD(void *arg);
        static void *VT_RIL_THREAD(void *arg);
        static sp<RmcVtSharedMemory> getSharedMem(void);

    private:
        static int ril_vt_looper();
        static int ril_vt_recv(int fd, void* buffer, int size);

        static bool isImsMessage(int msgId);
        static void handleMessage(int msgId, int length, void* data, int slotId);
        static void handleImsMessage(int size, char* outBuffer, int slotId);

        static sp<RmcVtSharedMemory> sShareMemmory;

        static pthread_t  sVtRilThd;
        static pthread_t  sImcbRilThd;

        static int sVtRilFd;
        static int sVtsFd;

        static int sIsVtConnected;


};

#endif
