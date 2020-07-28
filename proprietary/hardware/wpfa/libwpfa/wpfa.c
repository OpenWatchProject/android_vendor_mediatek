/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <mtk_log.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "wpfa.h"
#include "FilterParsingUtilis.h"
#include "WpfaCppUtils.h"
#include "RuleRegisterUtilis.h"
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/netfilter.h>
#include <linux/netfilter/nfnetlink.h>
#include <linux/netfilter/nfnetlink_queue.h>
#include <linux/netlink.h>


//Add this for poll() usage
#include <sys/poll.h>
#include <errno.h>
#define ARRAY_SIZE(x)   (sizeof(x) / sizeof(*(x)))

pthread_t mReaderPacketThread, mA2mRingBufferReaderThread;

int dump_hex(unsigned char *data, int len);

void initIptables(void);
size_t initReaderLoop();
int deInitReaderLoop();
int recvPacketFromFilter(char *buff);
int runCBwithBuffer(int numbytes, char *buff, unsigned char *payload);
void filterVerHanshake();

/*
 *  global vars used in call back function of netlink parsing,
 *  without modifying open source libmnl call back parameters.
 */
/*************** libmnl global vars ***************/
unsigned char *toDeliveredPayload;
uint16_t toDeliveredPayloadLen;

/*
 *  These two var are for records current mnl_socket,
 *  only one instance for each in wpfa.
 */
unsigned int portid;  // nlink's portId
static struct mnl_socket *mMnlsocket;
/*************** libmnl global vars ***************/

struct mnl_socket {
    int fd;
    struct sockaddr_nl addr;
};

/*
 *  Define libmnl call back function type.
 */
typedef int (*mnl_attr_cb_t)(const struct nlattr *attr, void *data);
typedef int (*mnl_cb_t)(const struct nlmsghdr *nlh, void *data);

//for dynamic link libmnl's functions
struct mnl_socket * (*fp_mnl_socket_open) (int bus);
int (*fp_mnl_socket_bind) (struct mnl_socket *mMnlsocket, unsigned int groups, pid_t pid);
unsigned int (*fp_mnl_socket_get_portid) (const struct mnl_socket *mMnlsocket);
int (*fp_mnl_socket_setsockopt) (const struct mnl_socket *mMnlsocket, int type, void *buf, socklen_t len);
struct nlmsghdr * (*fp_mnl_nlmsg_put_header) (void *buf);
void * (*fp_mnl_nlmsg_put_extra_header) (struct nlmsghdr *nlh, size_t size);
void (*fp_mnl_attr_put) (struct nlmsghdr *nlh, uint16_t type, size_t len, const void *data);
void (*fp_mnl_attr_put_u32) (struct nlmsghdr *nlh, uint16_t type, uint32_t data);
ssize_t (*fp_mnl_socket_sendto) (const struct mnl_socket *mMnlsocket, const void *buf, size_t len);

int (*fp_mnl_attr_parse) (const struct nlmsghdr *nlh, unsigned int offset, mnl_attr_cb_t cb, void *data);
void * (*fp_mnl_nlmsg_get_payload) (const struct nlmsghdr *nlh);
void * (*fp_mnl_attr_get_payload) (const struct nlattr *attr);
uint16_t (*fp_mnl_attr_get_payload_len) (const struct nlattr *attr);

uint16_t (*fp_mnl_attr_get_type) (const struct nlattr *attr);
int (*fp_mnl_attr_type_valid) (const struct nlattr *attr, uint16_t max);

int (*fp_mnl_socket_close) (struct mnl_socket *mMnlsocket);

ssize_t (*fp_mnl_socket_recvfrom) (const struct mnl_socket *mMnlsocket, void *buf, size_t bufsiz);
int (*fp_mnl_cb_run) (const void *buf, size_t numbytes, unsigned int seq,
        unsigned int portid, mnl_cb_t cb_data, void *data);

int initDynamicLinkLibmnl(void *dlmnlHandle) {
    fp_mnl_socket_open = dlsym(dlmnlHandle, "mnl_socket_open");
    fp_mnl_socket_bind = dlsym(dlmnlHandle, "mnl_socket_bind");
    fp_mnl_socket_get_portid = dlsym(dlmnlHandle, "mnl_socket_get_portid");
    fp_mnl_socket_setsockopt = dlsym(dlmnlHandle, "mnl_socket_setsockopt");
    fp_mnl_nlmsg_put_header = dlsym(dlmnlHandle, "mnl_nlmsg_put_header");
    fp_mnl_nlmsg_put_extra_header = dlsym(dlmnlHandle, "mnl_nlmsg_put_extra_header");
    fp_mnl_attr_put = dlsym(dlmnlHandle, "mnl_attr_put");
    fp_mnl_attr_put_u32 = dlsym(dlmnlHandle, "mnl_attr_put_u32");
    fp_mnl_socket_sendto = dlsym(dlmnlHandle, "mnl_socket_sendto");

    fp_mnl_attr_parse = dlsym(dlmnlHandle, "mnl_attr_parse");
    fp_mnl_nlmsg_get_payload = dlsym(dlmnlHandle, "mnl_nlmsg_get_payload");
    fp_mnl_attr_get_payload = dlsym(dlmnlHandle, "mnl_attr_get_payload");
    fp_mnl_attr_get_payload_len = dlsym(dlmnlHandle, "mnl_attr_get_payload_len");

    fp_mnl_attr_get_type = dlsym(dlmnlHandle, "mnl_attr_get_type");
    fp_mnl_attr_type_valid = dlsym(dlmnlHandle, "mnl_attr_type_valid");

    fp_mnl_socket_close = dlsym(dlmnlHandle, "mnl_socket_close");

    fp_mnl_socket_recvfrom = dlsym(dlmnlHandle, "mnl_socket_recvfrom");
    fp_mnl_cb_run = dlsym(dlmnlHandle, "mnl_cb_run");

    if (fp_mnl_socket_open == NULL || fp_mnl_socket_bind == NULL ||
            fp_mnl_socket_get_portid == NULL ||
            fp_mnl_socket_setsockopt == NULL || fp_mnl_socket_setsockopt == NULL ||
            fp_mnl_nlmsg_put_header == NULL || fp_mnl_nlmsg_put_extra_header == NULL ||
            fp_mnl_attr_put == NULL || fp_mnl_attr_put_u32 == NULL ||
            fp_mnl_socket_sendto == NULL || fp_mnl_attr_parse == NULL ||
            fp_mnl_nlmsg_get_payload == NULL || fp_mnl_attr_get_payload == NULL ||
            fp_mnl_attr_get_payload_len == NULL || fp_mnl_attr_get_type == NULL ||
            fp_mnl_attr_type_valid == NULL || fp_mnl_socket_close == NULL ||
            fp_mnl_socket_recvfrom == NULL || fp_mnl_cb_run == NULL) {
        mtkLogD(WA_LOG_TAG, "initDynamicLinkLibmnl failed!");
        return -1;
    }

    return 1;
}

int wpfa_dlLoop() {
    void *dlmnlHandle = NULL;
    size_t buf_size;
    int ret = 0;
    char *buf;
    unsigned char *payload;

    mtkLogD(WA_LOG_TAG, "startinitialIptables");
    initIptables();

    char* LibpkFilterPath = "libmnetlink_v104.so";
    dlmnlHandle = dlopen(LibpkFilterPath , RTLD_NOW);
    if (dlmnlHandle == NULL) {
        mtkLogE(WA_LOG_TAG, "dlopen failed: %s", dlerror());
        exit(EXIT_FAILURE);
    }
    mtkLogD(WA_LOG_TAG, "LibpkFilterPath:%s, and dlopen()pass!", LibpkFilterPath);

    initDynamicLinkLibmnl(dlmnlHandle);

    deInitReaderLoop();
    ret = initReaderLoop();
    if (ret > 0) {
        buf_size = MNL_TOTAL_BUFFER_SIZE;
        mtkLogD(WA_LOG_TAG, "initReaderLoop() done, buf_size=%ld", MNL_TOTAL_BUFFER_SIZE);
    } else {
        mtkLogE(WA_LOG_TAG, "initReaderLoop() failed needs retry");
        return -1;
    }

    buf = malloc(buf_size);
    // 6byte: is for 4bytes siZe in the front and 2bytes magic keyword at the end.
    payload = malloc(buf_size+6);
    if (!buf || !payload) {
        mtkLogD(WA_LOG_TAG, "error: allocate receive buffer");
        exit(EXIT_FAILURE);
    }

    mtkLogD(WA_LOG_TAG, "WPFA_DL initialized");
    filterVerHanshake();

    for (;;) {
        ret = recvPacketFromFilter(buf);
        if (ret == -1) {
            mtkLogD(WA_LOG_TAG, "error: fp_recvPacketFromFilter needs retry");
            free(buf);
            free(payload);
            return -1;
        }

        ret = runCBwithBuffer(ret, buf, payload);
        if (ret < 0) {
            mtkLogD(WA_LOG_TAG, "error: runCBwithBuffer needs retry");
            free(buf);
            free(payload);
            return -1;
        }

        // Here var:payload stores the packet from 4 bytes.
        // Need to add size in the front(4bytes) , and guard pattern in
        // the end(2bytes of magic pattern 0xffff).
        memcpy(payload, &ret, 4); // 4 byte for packet length in the front
        payload[ret+4] = 0xff;    // 2 byte at the end for guard pattern
        payload[ret+5] = 0xff;
        dump_hex(payload, ret+6);
        writeRingBuffer(payload, ret+6);
    }
    free(buf);
    free(payload);
}

void* wpfa_dl(void* data) {
    mtkLogD(WA_LOG_TAG, "wpfa_dl starts");
    int ret = 0;
    while(1) {
        //parameters initial
        mReaderPacketThread = 0;
        toDeliveredPayloadLen = 0;

        ret = wpfa_dlLoop();
        if(ret < 0 ) {
            mtkLogD(WA_LOG_TAG, "wpfa_dl encoumter failed, re-init");
            sleep(5);
        }
    }
    return NULL;
}

void* a2mReaderLoop(void* data) {
    mtkLogD(WA_LOG_TAG, "a2mReaderLoop starting");
    readRingBuffer();
    mtkLogD(WA_LOG_TAG, "a2mReaderLoop stopped!");
    return NULL;
}


void startLoops(void) {
    int ret;
    pthread_attr_t dlAttr,a2mAttr;
    pthread_attr_init(&dlAttr);
    pthread_attr_setdetachstate(&dlAttr, PTHREAD_CREATE_DETACHED);

    ret = pthread_create(&mReaderPacketThread, &dlAttr, wpfa_dl, "wpfa_dl_thread");

    if (ret != 0) {
        mtkLogD(WA_LOG_TAG, "[%s] failed to create dl reader thread ret:%d", __FUNCTION__, ret);
    } else {
        mtkLogD(WA_LOG_TAG, "[%s] create dl reader thread OK, mReaderPacketThread:%ld",
                __FUNCTION__, mReaderPacketThread);
    }

    //initial A2M Ring Buffer
    initialA2MRingBuffer();
    pthread_attr_init(&a2mAttr);
    pthread_attr_setdetachstate(&a2mAttr, PTHREAD_CREATE_DETACHED);
    ret = pthread_create(&mA2mRingBufferReaderThread, &a2mAttr, a2mReaderLoop, "wpfa_a2m_thread");

    if (ret != 0) {
        mtkLogD(WA_LOG_TAG, "[%s] failed to create a2m reader thread ret:%d", __FUNCTION__, ret);
    } else {
        mtkLogD(WA_LOG_TAG, "[%s] create a2m reader thread,ret: %d, mA2mRingBufferReaderThread:%ld",
                __FUNCTION__, ret, mA2mRingBufferReaderThread);
    }
}

void initIptables(void) {

    initIptablesChain();

    return;
}

int wpfaInit() {
    mtkLogD(WA_LOG_TAG, "start");
    initialRuleContainer();
    wpfaDriverInit();
    startLoops();
    return 1;
}

void filterVerHanshake() {
    mtkLogD(WA_LOG_TAG, "filterVerHanshake start");
    a2mWpfaInitNotify();
    a2mWpfaVersionNotify();
}

static void nfq_send_verdict(int queue_num, uint32_t id, int verdictCmd) {
    char buf[MNL_SOCKET_BUFFER_SIZE];
    struct nlmsghdr *nlh;

    nlh = fp_mnl_nlmsg_put_header(buf);
    nlh->nlmsg_type    = (NFNL_SUBSYS_QUEUE << 8) | NFQNL_MSG_VERDICT;
    nlh->nlmsg_flags = NLM_F_REQUEST;

    struct nfgenmsg *nfg = fp_mnl_nlmsg_put_extra_header(nlh, sizeof(*nfg));
    nfg->nfgen_family = AF_UNSPEC;
    nfg->version = NFNETLINK_V0;
    nfg->res_id = htons(queue_num);

    struct nfqnl_msg_verdict_hdr verdictHdr;
    verdictHdr.verdict = htonl(verdictCmd);
    verdictHdr.id = htonl(id);
    fp_mnl_attr_put(nlh, NFQA_VERDICT_HDR, sizeof(verdictHdr), &verdictHdr);
    mtkLogD(MNL_LOG_TAG, "verdicting id = %d", id);

    if (fp_mnl_socket_sendto(mMnlsocket, nlh, nlh->nlmsg_len) < 0) {
        mtkLogD(MNL_LOG_TAG, "error: mnl_socket_send");
        exit(EXIT_FAILURE);
    }
}

static int parseAttrCb(const struct nlattr *attr, void *data) {
    const struct nlattr **tb = data;
    int type = fp_mnl_attr_get_type(attr);
    //ignore not valid attributes
    if (fp_mnl_attr_type_valid(attr, NFQA_MAX) < 0) {
        return 1;
    }
    tb[type] = attr;
    return 1;
}

static int mnlQueueCallback(const struct nlmsghdr *nlh, void *data) {
    struct nfqnl_msg_packet_hdr *packetHeader = NULL;
    struct nlattr *attr[NFQA_MAX+1] = {};
    uint32_t id = 0;
    struct nfgenmsg *nfg;
    unsigned char *packetpayload;
    toDeliveredPayloadLen = 0;
    mtkLogD(MNL_LOG_TAG, "enter mnlQueueCallback\n");

    if (fp_mnl_attr_parse(nlh, sizeof(struct nfgenmsg),
            parseAttrCb, attr) < 0) {
        mtkLogD(MNL_LOG_TAG, "error: problems parsing");
        return -1;
    }

    nfg = fp_mnl_nlmsg_get_payload(nlh);

    if (attr[NFQA_PACKET_HDR] == NULL) {
        mtkLogD(MNL_LOG_TAG, "error: can't get NFQA_PACKET_HDR\n");
        return -1;
    }
    packetHeader = fp_mnl_attr_get_payload(attr[NFQA_PACKET_HDR]);

    toDeliveredPayloadLen = fp_mnl_attr_get_payload_len(attr[NFQA_PAYLOAD]);
    packetpayload = fp_mnl_attr_get_payload(attr[NFQA_PAYLOAD]);

    id = ntohl(packetHeader->packet_id);
    mtkLogD(MNL_LOG_TAG, "queue_num=%d, nfgen_family = %d , version = %d, packet received (id=%u hw=0x%04x hook=%u, payload len %u",
        ntohs(nfg->res_id) , nfg->nfgen_family , nfg->version,
        id, ntohs(packetHeader->hw_protocol), packetHeader->hook, toDeliveredPayloadLen);

    memcpy(toDeliveredPayload+4, packetpayload, toDeliveredPayloadLen); // shift 4 bytes

    dump_hex(packetpayload,toDeliveredPayloadLen);

    if (ntohs(nfg->res_id) == atoi(REGISTER_NFQUEUE_QNUM)) {
        nfq_send_verdict(ntohs(nfg->res_id), id, NF_DROP);
    } else {
        nfq_send_verdict(ntohs(nfg->res_id), id, NF_ACCEPT);
    }

    return 1;
}


//used for mutli-queue registered.
int registerByQueueNum(uint32_t queue_num) {
    struct nlmsghdr *nlh;
    char *buf;
    mtkLogD(MNL_LOG_TAG,  "registerByQueueNum, queue_num=%d",queue_num);
    buf = malloc(MNL_TOTAL_BUFFER_SIZE);
    if (!buf) {
        mtkLogD(MNL_LOG_TAG,  "error: allocate buffer in registerByQueueNum");
        return -1;
    }
    mtkLogD(MNL_LOG_TAG,  "registerByQueueNum: NFQNL_CFG_CMD_BIND");
    //NFQNL_CFG_CMD_BIND
    nlh = fp_mnl_nlmsg_put_header(buf);
    nlh->nlmsg_type    = (NFNL_SUBSYS_QUEUE << 8) | NFQNL_MSG_CONFIG;
    nlh->nlmsg_flags = NLM_F_REQUEST;

    struct nfgenmsg *nfg = fp_mnl_nlmsg_put_extra_header(nlh, sizeof(*nfg));
    nfg->nfgen_family = AF_UNSPEC;
    nfg->version = NFNETLINK_V0;
    nfg->res_id = htons(queue_num);

    struct nfqnl_msg_config_cmd configCmd;
    configCmd.command = NFQNL_CFG_CMD_BIND;
    configCmd.pf = AF_INET;
    fp_mnl_attr_put(nlh, NFQA_CFG_CMD, sizeof(configCmd), &configCmd);

    if (fp_mnl_socket_sendto(mMnlsocket, nlh, nlh->nlmsg_len) < 0) {
        mtkLogD(MNL_LOG_TAG,  "error: NFQNL_CFG_CMD_BIND, queue_num = %d", queue_num);
        free(buf);
        return -1;
        // exit(EXIT_FAILURE);
    }
    mtkLogD(MNL_LOG_TAG,  "registerByQueueNum: NFQNL_COPY_PACKET");

    //NFQNL_COPY_PACKET, range: 0xffff
    nlh = fp_mnl_nlmsg_put_header(buf);
    nlh->nlmsg_type    = (NFNL_SUBSYS_QUEUE << 8) | NFQNL_MSG_CONFIG;
    nlh->nlmsg_flags = NLM_F_REQUEST;

    struct nfgenmsg *nfg2 = fp_mnl_nlmsg_put_extra_header(nlh, sizeof(*nfg2));
    nfg2->nfgen_family = AF_UNSPEC;
    nfg2->version = NFNETLINK_V0;
    nfg2->res_id = htons(queue_num);

    struct nfqnl_msg_config_params configParams;
    configParams.copy_mode = NFQNL_COPY_PACKET;
    configParams.copy_range = htonl(0xffff);
    fp_mnl_attr_put(nlh, NFQA_CFG_PARAMS, sizeof(configParams), &configParams);
    mtkLogD(MNL_LOG_TAG,  "registerByQueueNum3");
    fp_mnl_attr_put_u32(nlh, NFQA_CFG_FLAGS, htonl(NFQA_CFG_F_GSO));
    fp_mnl_attr_put_u32(nlh, NFQA_CFG_MASK, htonl(NFQA_CFG_F_GSO));
    mtkLogD(MNL_LOG_TAG,  "main5");

    if (fp_mnl_socket_sendto(mMnlsocket, nlh, nlh->nlmsg_len) < 0) {
        mtkLogD(MNL_LOG_TAG,  "error: NFQNL_COPY_PACKET, queue_num = %d", queue_num);
        free(buf);
        return -1;
        // exit(EXIT_FAILURE);
    }
    free(buf);
    return 1;
}

size_t initReaderLoop() {
    int ret;
    mtkLogD(MNL_LOG_TAG,  "main1");

    mMnlsocket = fp_mnl_socket_open(NETLINK_NETFILTER);
    if (mMnlsocket == NULL) {
        mtkLogD(MNL_LOG_TAG,  "error: mnl_socket_open");
        return -1;
        // exit(EXIT_FAILURE);
    }

    mtkLogD(MNL_LOG_TAG,  "main2");

    if (fp_mnl_socket_bind(mMnlsocket, 0, MNL_SOCKET_AUTOPID) < 0) {
        mtkLogD(MNL_LOG_TAG,  "error: mnl_socket_bind");
        return -1;
        // exit(EXIT_FAILURE);
    }
    portid = fp_mnl_socket_get_portid(mMnlsocket);

    mtkLogD(MNL_LOG_TAG,  "main4");
    if (registerByQueueNum(atoi(REGISTER_NFQUEUE_QNUM)) != 1 ||
            registerByQueueNum(atoi(REGISTER_ICMP_NFQUEUE_QNUM)) != 1) {
        mtkLogD(MNL_LOG_TAG,  "error when registerByQueueNum");
        return -1;
    }

    //Turn ENOBUFS off.
    ret = 1;
    fp_mnl_socket_setsockopt(mMnlsocket, NETLINK_NO_ENOBUFS, &ret, sizeof(int));

    mtkLogD(MNL_LOG_TAG,  "initReaderLoop done");

    return 1;
}

int deInitReaderLoop() {
    if (mMnlsocket == NULL) {
        mtkLogD(MNL_LOG_TAG,  "No action: deInitReaderLoop can't get mMnlsocket");
        return 0;
    }
    fp_mnl_socket_close(mMnlsocket);
    mMnlsocket = NULL;
    return 1;
}

int recvPacketFromFilter(char *buff) {
    int ret = 1;
    if (mMnlsocket == NULL) {
        mtkLogD(MNL_LOG_TAG,  "error: recvPacketFromFilter get mMnlsocket");
        return -1;
    }
    mtkLogD(MNL_LOG_TAG,  "enter recvPacketFromFilter\n");
    ret = fp_mnl_socket_recvfrom(mMnlsocket, buff, MNL_TOTAL_BUFFER_SIZE);
    if (ret == -1) {
        mtkLogD(MNL_LOG_TAG,  "error in mnl_socket_recvfrom");
        //return -1;
        //change to return -1 for retry after IT.
        exit(EXIT_FAILURE);
    }
    return ret;
}

int runCBwithBuffer(int numbytes, char *buff, unsigned char *payload) {
    int ret = 1;
    if (mMnlsocket == NULL) {
        mtkLogD(MNL_LOG_TAG,  "error: runCBwithBuffer get mMnlsocket");
        return -1;
    }
    toDeliveredPayload = payload;
    mtkLogD(MNL_LOG_TAG,  "enter runCBwithBuffer\n");
    ret = fp_mnl_cb_run(buff, numbytes, 0, portid, mnlQueueCallback, NULL);
    if (ret < 0) {
        mtkLogD(MNL_LOG_TAG,  "error in mnl_cb_run");
        //return -1;
        //change to return -1 for retry after IT.
        exit(EXIT_FAILURE);
    }
    if (toDeliveredPayloadLen > 0) {
        return toDeliveredPayloadLen;
    }
    return -1;
}

int dump_hex(unsigned char *data, int len) {
    int i, counter, rest;
    char * dumpbuffer;
    char printbuf[1024];

    dumpbuffer = (char*)malloc(16*1024);
    if (!dumpbuffer) {
        mtkLogD(WA_LOG_TAG, "DUMP_HEX ALLOC memory fail \n");
        return -1;;
    }

    if (len >8*1024 ){
        mtkLogD(WA_LOG_TAG, "trac the packet \n");
        len = 8*1024;
    }

    //memset((void *)dumpbuffer,0,16*1024);
    memset(dumpbuffer, 0, 16*1024);

    for (i = 0 ; i < len ; i++) {
       sprintf(&dumpbuffer[i*2],"%02x",data[i]);
    }
    dumpbuffer[i*2] = '\0' ;

    // android log buffer =1024bytes, need to splite the log
    counter = len/300 ;
    rest = len - counter*300 ;

    mtkLogD(WA_LOG_TAG, " Data Length = %d ,counter =%d ,rest =%d", len ,counter,rest);

    mtkLogD(WA_LOG_TAG, " NFQUEU Data: ");
    for (i = 0 ; i < counter ; i++) {
        memset(printbuf, 0, sizeof(printbuf));
        memcpy(printbuf ,dumpbuffer+i*600 , 300*2) ;
        printbuf[600]='\0';
        mtkLogD(WA_LOG_TAG, "data:%s",printbuf);
    }

    //for rest data
    memset(printbuf, 0, sizeof(printbuf));
    memcpy(printbuf, dumpbuffer+counter*600, rest*2) ;
    printbuf[rest*2] = '\0';
    mtkLogD(WA_LOG_TAG, "%s", printbuf);

    free(dumpbuffer);
    return 1;
}


