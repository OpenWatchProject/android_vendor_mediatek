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
#ifndef __SOCKETWRAPPER_H__
#define __SOCKETWRAPPER_H__

#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <utils/RefBase.h>
#include <utils/threads.h>
#include <assert.h>

#define CONNECT_UDP

using namespace android;

#define VOLTE_IPADDR_LENGTH 16
#define VOLTE_LOOP_EBI 100
#define VOLTE_WIFI_EBI 200
#define VOLTE_VT_DEV_NAME "/dev/ccci_imsv"
#define VOLTE_AV_DEV_NAME "/dev/ccci_imsdc"

enum VoLTE_Event_IP_Version_e {
    VoLTE_Event_IPv4 = 0,
    VoLTE_Event_IPv6,
    /*add here*/
    VoLTE_Event_IPv_Max,
};


#define SOCKETWRAPPER_LOGI(fmt, arg...)  ALOGI("" fmt, ##arg);
#define SOCKETWRAPPER_LOGW(fmt, arg...)  ALOGW("" fmt, ##arg);
#define SOCKETWRAPPER_LOGE(fmt, arg...)  ALOGE("" fmt, ##arg);

#define SOCKETWRAPPER_ASSERT(expr, M, ...) while(!(expr)) { SOCKETWRAPPER_LOGE("@@@ ASSERT @@@: (%s:%d)" M, __FILE__, __LINE__, ##__VA_ARGS__); fflush(stdout); LOG_ALWAYS_FATAL("ASSERT!!!!"); assert(0); exit(-11);}

typedef int (*Sock_RxCB_t)(void* cookie, const sp<ABuffer>& buffer);

typedef struct Sock_param {
    /*IPv4 or IPv6*/
    uint32_t protocol_version;

    /*Socket FD*/
    int32_t  sockfd;

    uint32_t  dscp;
    uint32_t  priority;

    bool isBlock;

    uint32_t sendBufferSize;    /* 0 for ignore */
    uint32_t receiveBufferSize; /* 0 for ignore */

    uint32_t network_id;
    char ifname[16];
    uid_t uid;
    uint32_t tag;
    uint8_t  local_address[VOLTE_IPADDR_LENGTH];
    uint16_t local_port;

    uint8_t  peer_address[VOLTE_IPADDR_LENGTH];
    uint16_t peer_port;
} Sock_param_t;

class SocketWrapper : public RefBase
{
public:
    SocketWrapper();
    virtual ~SocketWrapper();
    int setParam(Sock_param_t param);
    int getParam(Sock_param_t *param);
    int setRxCallBack(void *cookie, Sock_RxCB_t rx_cb);
    int writeSock(const sp<ABuffer>& buffer);
private:
    int readSock(int fd, const sp<ABuffer>& buffer);
    int setSock(void);
    int createSock(void);
    int dumpAddr(struct sockaddr *addr_ptr);
    void setUdpConnect(void);

    Sock_param_t mParam;
    Sock_RxCB_t mRxCb;
    void        *mCookie;
    bool        m_bStarted;
    bool        m_bSelfCreate;
    pthread_t   m_Tid;
    static void * receiveThread(void *pParam);
    /* for thread wakeup */
    int mReadPipe;
    int mWritePipe;
    uint64_t mSendDataUasage;
    uint64_t mrecvDataUasage;
    bool mError;

    //for debug
    unsigned int mWriteCount;
    unsigned int mWriteFail;
    unsigned int mReceiveCount;
};
#endif /* __SOCKETWRAPPER_H__ */
