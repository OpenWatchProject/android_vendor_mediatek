/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

#ifndef RILP_SOCKET_H_INCLUDED
#define RILP_SOCKET_H_INCLUDED
#include "RfxObject.h"

extern "C"
void *ril_socket_reader_looper(void *arg);
extern "C"
void *sap_ril_proxy_rild_socket_reader_looper(void *arg);

class RilpSocket{

    /**
    * Socket name.
    */
    const char* name;

    /**
    * Socket id.
    */
    RIL_SOCKET_ID id;

    /**
     * rild group.
     */
    RILD_RadioTechnology_Group group;

    /**
     * Commands socket file descriptor.
     */
     int socketFd = -1;

    /**
     * Socket request loop thread id.
     */
     pthread_t socketThreadId;

    public:
        /**
         * Constructor.
         *
         * @param Socket name.
         * @param Socket id.
         * @param Rild group.
         */
        RilpSocket(const char* socketName, RIL_SOCKET_ID socketId, RILD_RadioTechnology_Group groupId) {
            name = socketName;
            id = socketId;
            group = groupId;
            memset(&socketThreadId, 0, sizeof(pthread_t));
        }

        /**
         * Init socket param, create loop thread to receive socket data
         *
         * @param Socket name.
         * @param Socket id.
         * @param Rild group.
         */
        static void initSocket(const char *socketName,
                RIL_SOCKET_ID socketid,
                RILD_RadioTechnology_Group groupId);

    private:
        friend void *::ril_socket_reader_looper(void *arg);

    private:
        /**
         * connect to socket
         */
        void connectSocket(void);

        /**
         * Receive data from socket continuely
         */
        void readerLooper(void);

        /**
         * Translate group id to readable string
         */
        const char *rildGroupIdtoString(RILD_RadioTechnology_Group group);
};

class SapRilProxyRildSocket {
    /**
    * Socket name.
    */
    const char* name;

    /**
    * Socket id.
    */
    RIL_SOCKET_ID id;

    /**
     * rild group.
     */
    RILD_RadioTechnology_Group group;

    /**
     * Commands socket file descriptor.
     */
     int socketFd = -1;

    /**
     * Socket request loop thread id.
     */
     pthread_t socketThreadId;

    public:
        /**
         * Constructor.
         *
         * @param Socket name.
         * @param Socket id.
         * @param Rild group.
         */
        SapRilProxyRildSocket(const char* socketName, RIL_SOCKET_ID socketId,
                RILD_RadioTechnology_Group groupId) {
            name = socketName;
            id = socketId;
            group = groupId;
            memset(&socketThreadId, 0, sizeof(pthread_t));
        }

        /**
         * Init socket param, create loop thread to receive socket data
         *
         * @param Socket name.
         * @param Socket id.
         * @param Rild group.
         */
        static void initSocket(const char *socketName,
                RIL_SOCKET_ID socketid,
                RILD_RadioTechnology_Group groupId);

    private:
        friend void *::sap_ril_proxy_rild_socket_reader_looper(void *arg);

    private:
        /**
         * connect to socket
         */
        void connectSocket(void);

        /**
         * Receive data from socket continuely
         */
        void readerLooper(void);

        const char* rildGroupIdtoString(RILD_RadioTechnology_Group group);
};

typedef void* (*PthreadPtr)(void*);

#endif /*RILP_UIM_SOCKET_H_INCLUDED*/
