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

#include "WpfaDriver.h"
#include "WpfaShmReadMsgHandler.h"
#include "WpfaDriverUtilis.h"

/* Raw socket headers*/
#include <netdb.h>            // struct addrinfo
#include <sys/types.h>        // needed for socket(), uint8_t, uint16_t, uint32_t
#include <sys/socket.h>       // needed for socket()
#include <netinet/in.h>       // IPPROTO_HOPOPTS, IPPROTO_TCP, IPPROTO_FRAGMENT, INET6_ADDRSTRLEN
#include <netinet/ip.h>       // IP_MAXPACKET (which is 65535)
#include <netinet/ip6.h>      // struct ip6_hdr
//#define __FAVOR_BSD         // Use BSD format of tcp header
#include <netinet/tcp.h>      // struct tcphdr
#include <arpa/inet.h>        // inet_pton() and inet_ntop()
#include <sys/ioctl.h>        // macro ioctl is defined
//#include <bits/ioctls.h>    // defines values for argument "request" of ioctl.
#include <net/if.h>           // struct ifreq
#include <linux/if_ether.h>   // ETH_P_IP = 0x0800, ETH_P_IPV6 = 0x86DD
#include <linux/if_packet.h>  // struct sockaddr_ll (see man 7 packet)
#include <net/ethernet.h>
#include <cstdio>


#define WPFA_D_LOG_TAG "WpfaShmReadMsgHandler"

using ::android::Mutex;

WpfaShmReadMsgHandler *WpfaShmReadMsgHandler::s_self = NULL;

static sem_t sWaitLooperSem;
static bool sNeedWaitLooper = true;
static Mutex sWaitLooperMutex;


/*****************************************************************************
 * Class RuleReqMsgHandler
 *****************************************************************************/
void WpfaShmReadMsgHandler::ShmReadMsgHandler::handleMessage(
        const Message& message) {
    WPFA_UNUSED(message);
    sender->processMessage(msg);
}

/*****************************************************************************
 * Class WpfaShmReadMsgHandler
 *****************************************************************************/
WpfaShmReadMsgHandler::WpfaShmReadMsgHandler() {
    mDriverAdapter = NULL;
    mTid = 0;
}

void WpfaShmReadMsgHandler::init() {
    mtkLogD(WPFA_D_LOG_TAG, "init");
    sem_init(&sWaitLooperSem, 0, 0);
    s_self = new WpfaShmReadMsgHandler();
    s_self->run("WpfaShmReadMsgHandler init");
}

bool WpfaShmReadMsgHandler::threadLoop() {
    mtkLogD(WPFA_D_LOG_TAG, "threadLoop");
    int err = 0;
    mLooper = Looper::prepare(0);
    sem_post(&sWaitLooperSem);

    int result;
    do {
        result = mLooper->pollAll(-1);
        mtkLogD(WPFA_D_LOG_TAG, "threadLoop, result = %d, err=%d", result, err);
    } while (result == Looper::POLL_WAKE || result == Looper::POLL_CALLBACK);

    WPFA_D_ASSERT(0); // Can't go here
    return true;
}

void WpfaShmReadMsgHandler::enqueueShmReadMessage(const sp<WpfaDriverMessage>& message) {
    //if (!RfxRilUtils::isInLogReductionList(message->getId())) {
        mtkLogD(WPFA_D_LOG_TAG, "enqueueShmReadMessage: %s", message->toString().string());
    //}
    WpfaShmReadMsgHandler* sender = s_self;
    sp<MessageHandler> handler = new ShmReadMsgHandler(sender, message);
    s_self->mLooper->sendMessage(handler, s_self->mDummyMsg);
}

void WpfaShmReadMsgHandler::enqueueShmReadMessageFront(const sp<WpfaDriverMessage>& message) {
    mtkLogD(WPFA_D_LOG_TAG, "enqueueShmReadMessageFront: %s", message->toString().string());
    WpfaShmReadMsgHandler* sender = s_self;
    sp<MessageHandler> handler = new ShmReadMsgHandler(sender, message);
    s_self->mLooper->sendMessageAtTime(0, handler, s_self->mDummyMsg);
}

sp<Looper> WpfaShmReadMsgHandler::waitLooper() {
    WPFA_D_ASSERT(s_self != NULL);
    sWaitLooperMutex.lock();
    if (sNeedWaitLooper) {
        mtkLogD(WPFA_D_LOG_TAG, "waitLooper() begin");
        sem_wait(&sWaitLooperSem);
        sNeedWaitLooper = false;
        sem_destroy(&sWaitLooperSem);
        mtkLogD(WPFA_D_LOG_TAG, "waitLooper() end");
    }
    sWaitLooperMutex.unlock();
    return s_self->mLooper;
}

void WpfaShmReadMsgHandler::processMessage(const sp<WpfaDriverMessage>& msg) {
    uint16_t msgId = msg->getMsgId();
    uint16_t tId = msg->getTid();
    mtkLogD(WPFA_D_LOG_TAG, "processMessage, msgId=%d tId=%d", msgId, tId);
    switch (msgId) {
        case MSG_M2A_DATA_READY:
            onDataReady(tId);
            break;
        case MSG_M2A_REQUEST_DATA_ACK:
            onRequestDataAck(tId);
            break;
        case MSG_M2A_UL_IP_PKT:  // MD UL AP Path Via TTY instead of SHM (For M2 RD IT purpose)
            onRequestULIpPkt(msg);
            break;

        default:
            mtkLogE(WPFA_D_LOG_TAG, "processMessage, type=%d not support", msgId);
    }
}

void WpfaShmReadMsgHandler::onDataReady(uint16_t tId) {
    // check and update mTid
    //if (mTid == MODEM_TID_INIT) {  // for first time get DataReady
        mTid = tId;
        if (MAX_SIZE_RING_BURRER == 0) {
            // loacl ring buffer is not supported
            mtkLogD(WPFA_D_LOG_TAG, "MAX_SIZE_RING_BURRER=0");
            sendMessageToModem(MSG_A2M_REQUEST_DATA, tId);
        } else {
            // check free space of local ring buffer
            if (1) {
                sendMessageToModem(MSG_A2M_REQUEST_DATA, tId);
            } else {
                mtkLogE(WPFA_D_LOG_TAG, "onDataReady, local buffer is full!");
                //need to assert
            }
        }
}

void WpfaShmReadMsgHandler::onRequestDataAck(uint16_t tId) {
    // 1. check option and transaction Id
    // 2. copy data from SHM to local buffer
    //    (if no local buffer then pass readIdx and size to AP[blocking call])
    // 3. send MSG_A2M_REQUEST_DONE to modem
    sendMessageToModem(MSG_A2M_REQUEST_DATA_DONE, tId);
}

int WpfaShmReadMsgHandler::checkDriverAdapterState() {
    int retValue = 0;
    if (mDriverAdapter != NULL) {
        return retValue;
    } else {
        mDriverAdapter = WpfaDriverAdapter::getInstance();
        if (mDriverAdapter == NULL) {
            mtkLogE(WPFA_D_LOG_TAG, "getInstance of WpfaDriverAdapter fail");
            retValue = -1;
        }
    }
    return retValue;
}

int WpfaShmReadMsgHandler::sendMessageToModem(uint16_t msgId, uint16_t tId) {
    int retValue = checkDriverAdapterState();
    if (retValue == 0) {
        sp<WpfaDriverMessage> msg = WpfaDriverMessage::obtainMessage(
        msgId,
        tId,
        CCCI_CTRL_MSG,
        0);
        retValue = mDriverAdapter->sendMsgToControlMsgDispatcher(msg);
    }
    return retValue;
}


/* MD UL AP Path Via TTY instead of SHM (For M2 RD IT purpose) */
void WpfaShmReadMsgHandler::onRequestULIpPkt(const sp<WpfaDriverMessage>& msg) {
    mtkLogE(WPFA_D_LOG_TAG, "Not support onRequestULIpPkt.");
    // int ret = -1;
    // unsigned char val[MAX_UL_IP_PKT_SIZE];
    // char *pos;

    // uint16_t pktSize = msg->getParams();
    // mtkLogD(WPFA_D_LOG_TAG, "onRequestULIpPkt pktSize=%d", pktSize);

    // wifiproxy_m2a_ul_ip_pkt_t* data = (wifiproxy_m2a_ul_ip_pkt_t *)msg->getData()->getData();

    // if (data != NULL) {
    //     //for(int i = 0; i < pktSize; i++) {
    //     //    mtkLogD(WPFA_D_LOG_TAG, "data->pkt[%d]=%x", i, data->pkt[i]);
    //     //}
    //     //pos = (char *)&data->pkt[0];

    //     dump_hex(data->pkt , pktSize);
    //     // ret = sendPktToKernel(data->pkt, pktSize);
    //     if (ret < 0) {
    //         //Error handling...Do nothing
    //         mtkLogE(WPFA_D_LOG_TAG, "onRequestULIpPkt sendPktToKernel fail!!");
    //     } else {
    //         // for test mode
    //         if (WPFA_DRIVER_TEST_MODE_ENABLE) {
    //             WpfaDriverUtilis *mWpfaDriverUtilis = WpfaDriverUtilis::getInstance();
    //             if (mWpfaDriverUtilis != NULL) {
    //                 mWpfaDriverUtilis->notifyEvent(TEST_EVENT_PKT_SEND_TO_KERNEL_SUCCESS);
    //             }
    //         }
    //     }
    // } else {
    //     mtkLogE(WPFA_D_LOG_TAG, "onRequestULIpPkt data is null");
    // }
}

int WpfaShmReadMsgHandler::dump_hex(unsigned char *data, int len) {
    int i,counter ,rest;
    char * dumpbuffer;
    char  printbuf[1024];

    dumpbuffer = (char*)malloc(16*1024);
    if (!dumpbuffer) {
        mtkLogD(WPFA_D_LOG_TAG, "DUMP_HEX ALLOC memory fail \n");
        return -1;
    }

    if (len >8*1024 ){
        mtkLogD(WPFA_D_LOG_TAG, "trac the packet \n");
        len = 8*1024;
    }

    //memset((void *)dumpbuffer,0,16*1024);
    memset(dumpbuffer, 0, 16*1024);
    //mtkLogD(UPLINK_LOG_TAG, "dumpbuffer size =%d \n",(int)sizeof(*dumpbuffer));

    for (i = 0 ; i < len ; i++) {
       sprintf(&dumpbuffer[i*2],"%02x",data[i]);
    }
    dumpbuffer[i*2] = '\0' ;

    // android log buffer =1024bytes, need to splite the log
    counter = len/300 ;
    rest = len - counter*300 ;

    mtkLogD(WPFA_D_LOG_TAG, " Data Length = %d ,counter =%d ,rest =%d", len ,counter,rest);

    mtkLogD(WPFA_D_LOG_TAG, " NFQUEU Data: ");
    for (i = 0 ; i < counter ; i++) {
        memset(printbuf, 0, sizeof(printbuf));
        memcpy(printbuf ,dumpbuffer+i*600 , 300*2);
        printbuf[600]='\0';
        mtkLogD(WPFA_D_LOG_TAG, "data:%s",printbuf);
        mtkLogD(WPFA_D_LOG_TAG, "~");
    }

    //for rest data
    memset(printbuf, 0, sizeof(printbuf));
    memcpy(printbuf ,dumpbuffer+counter*600 , rest*2);
    printbuf[rest*2]='\0';
    mtkLogD(WPFA_D_LOG_TAG, "%s",printbuf);

    free(dumpbuffer);
    return 1;
}

int WpfaShmReadMsgHandler::getDestAddress(unsigned char *data, int len) {
    struct iphdr *iphdrTemp;
    char *iphdrTempArray;
    char *sourceAddr;
    char *destAddr;
    int i;
    int mdaddr = 0;

    if(len < sizeof(struct iphdr)) {
        return -1;
    }

    iphdrTempArray = (char *)malloc(sizeof(struct iphdr));

    if(!iphdrTempArray){
        mtkLogD(WPFA_D_LOG_TAG, "iphdrTempArray ALLOC memory fail");
    }

    for(i=0; i < sizeof(struct iphdr); i++) {
        iphdrTempArray[i] = data[i];
    }
    iphdrTemp = (struct iphdr*) iphdrTempArray;
    destAddr = (char *)(&iphdrTemp->daddr);
    mtkLogD(WPFA_D_LOG_TAG, "iphdrTemp->daddr= %d , or %d %d %d %d",htonl(iphdrTemp->daddr),
            destAddr[0], destAddr[1], destAddr[2], destAddr[3]);
    mdaddr = htonl(iphdrTemp->daddr);
    free(iphdrTempArray);

    return mdaddr;

}

// Send IPv4 Packet to MD. If needs to support v6, it needs to use PF_PACKET instead.
// Caution!
// This function references some open sources(might has licenses issues),
// and this function ONLY for testing in DEV phase when MD MCIF is not ready.
// Remove it after DEV phase.
int WpfaShmReadMsgHandler::sendPktToKernel(unsigned char *data, int len) {
    return -1;
}

