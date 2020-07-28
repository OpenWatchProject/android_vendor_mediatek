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

#ifndef __RMC_RCS_DATA_THREAD_CONTROLLER_H__
#define __RMC_RCS_DATA_THREAD_CONTROLLER_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RfxBaseHandler.h"
#include <pthread.h>
#include "volte_core_interface.h"

/*****************************************************************************
 * Define
 *****************************************************************************/
#define RCS_RIL_SOCKET_NAME    "@/rcs_rild"
#define UA_NUM (1) //rcs
#define MAX_CLIENT (UA_NUM * 2) //one for reg, one for sip msg each
#define UA_CHANNEL_HEADER_LEN      (3 * sizeof(int))//(offsetof(Channel_Data_t, data))
#define MD_CHANNEL_HEADER_LEN      (5 * sizeof(int))//(offsetof(VoLTE_Event_IMS_HIF_Data, data))
#define CHANNEL_BUFFER_SIZE     65535   // align SIP_TX_RECV_BUFF_SIZE & SIP_TCP_RECEIVE_BUFFER_SIZE
#define RCS_RIL_SHARE_DATA_STATUS_RECV_NONE      (0)
#define RCS_RIL_SHARE_DATA_STATUS_RECV_DATA      (1)

/*****************************************************************************
 * typedef
 *****************************************************************************/
typedef struct _Ril_Channel_Obj {
    int                             fd;
    pthread_mutex_t                 mutex;
    void                           *channels;
    int                             channels_size;
    int                             type;          /* VoLTE_Stack_UA_Type_e */
} Ril_Channel_Obj_t;

typedef struct _Ril_Channel_Client {
    Ril_Channel_Obj_t                channel;
    int                              buf_len;
    char                             buf[CHANNEL_BUFFER_SIZE];
} Ril_Channel_Client_t;


/*****************************************************************************
 * Class RmcRcsSharedMemory
 *****************************************************************************/
class RmcRcsSharedMemory: virtual public RefBase {

    public:
        RmcRcsSharedMemory(void);
        virtual ~RmcRcsSharedMemory();

        void setState(int state);
        int getState(void);
        bool checkState(int want_state);
        void setSize(int len);
        void setData(char* data, int len);
        int getSize(void);
        void getData(char** data);
        void clearData();
        void lock(const char* user);
        void unlock(const char* user);
        void wait(const char* user, int stay_state);
        void signal(const char* user);

    private:
        pthread_mutex_t     mLock;
        pthread_mutex_t    *mPLock;
        pthread_cond_t      mCond;
        int                 mDataReadyCount;
        int                 mDatalen;//recv data length from md
        char*               mData; //recv data from md
};


/*****************************************************************************
 * Class RmcRcsDataThreadController
 *****************************************************************************/
class RmcRcsDataThreadController: virtual public RefBase {

    public:
        RmcRcsDataThreadController();
        virtual ~RmcRcsDataThreadController();
        static void init();
        static void* rilServerThread(void *arg); // process data from UA
        static void* shareMemoryThread(void *arg); //process data from md
        static void sendMsgToShareMemory(char* outBuffer, int length, const char* user);
        static bool isLogEnable();

    private:
        static void rilServerRead(Channel_Data_t *msgData, Ril_Channel_Obj_t *channel);
        static void startThread(); //only be called once when init
        static int fsend(int fd, char *data, int len);
        static Ril_Channel_Obj_t *getClientObjbyUaType(int uaType);
        static void *convertUAData(Channel_Data_t *_data, int uaType, int acctId);
        static int sendDataToUA(Channel_Data_t *data, int uaType);
        static sp<RmcRcsSharedMemory> getSharedMem(void);

        static pthread_t  sRilServerThd; //process UA data
        static pthread_t  sShareMemoryThd; //process Md data
        static sp<RmcRcsSharedMemory> sShareMemory;
        static pthread_mutex_t sMutex;
        static Ril_Channel_Obj_t *sChannelObj;
        static RmcRcsDataThreadController *sInstance;
};

#endif /* __RMC_RCS_DATA_THREAD_CONTROLLER_H__ */
