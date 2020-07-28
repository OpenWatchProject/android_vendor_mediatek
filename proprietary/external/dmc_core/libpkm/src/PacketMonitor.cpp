/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2018. All rights reserved.
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

/***************************************************************************************************
 * @file PacketMonitor.cpp
 * @brief The source file defines the class of PacketMonitor
 **************************************************************************************************/
#include "PacketMonitor.h"
#include "PacketMonitorTypes.h"
#include "Logs.h"
#include "apm_kpi_ota_packet.h"
#include "apm_msg_defs.h"

#include <ap_monitor.h>
#include <vendor/mediatek/hardware/apmonitor/2.0/IApmService.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <memory.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>

#include <errno.h>
#include <pthread.h>
#include <linux/capability.h>
#include <sys/capability.h>
#include <sys/system_properties.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/un.h>      // for Unix domain sockets */
#include <pcap.h>
#include <pcap/sll.h>
#include <strings.h>
#include <assert.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/ip.h>  // For IP header
#include <netinet/ip6.h> // For IP header
#include <netinet/udp.h>
#include <netinet/tcp.h>

extern "C" {
    #include <interface.h>
    #include <extract.h>
}

#include <vector>

// #define CAPTURE_DEBUG
#define PCAP_TIMEOUT_1S     1000
#define SNAPLEN             1518
#define SPI_LENGTH          21
#define DNS_HDR_LENGTH      12

#define ETYPE_IKE_SA_INIT   34
#define IPV6_FIX_HEADER_LENGTH  40

#define MESSAGE_KEY_ADD_EVENT   0x2001

#define MAX_SIM_COUNT   (4)
#define LEN_OF_PROP_NON_IMS_PDN_INFO_NAME   (26)
#define PROP_IMS_PDN_INFO "vendor.ims.eims.pdn.info"
#define PROP_NON_IMS_PDN_INFO "vendor.ril.data.pdn_info%d"
#define PROP_AUTO_SETUP_IMS "ro.vendor.md_auto_setup_ims"
#define IMS_APN "ims"

#define PARAM_PDN_TYPE  (0)
#define PARAM_SIM_ID    (1)
#define PARAM_INTF_ID   (2)
#define PARAM_NON_IMS_PDN_TYPE  (0)
#define PARAM_NON_IMS_INTF_ID   (1)

#define MAX_SIM_SIZE    (4)

#define DECRYPT_RESULT_OK       0x0
#define DECRYPT_KEY_NOT_FOUND   0x2101
#define DECRYPT_FAIL            0x2102
#define DECRYPT_STATUS_ERROR    0x2103

#define IS_MODE(p) ((mParam & p) == p)
#define IS_MODE_PARAM(p, q) (((q) & p) == p)

using vendor::mediatek::hardware::apmonitor::V2_0::PacketEvent_t;

using ::android::hardware::hidl_vec;

using namespace com::mediatek::diagnostic;
using namespace libmdmonitor;
using namespace std;

// Log TAG
const static char* TAG = "PKM-Monitor";

typedef struct
{
    unsigned short pkttype;
    unsigned short ifindex;
    #define IFNAMSIZE 16
    char ifname[IFNAMSIZE];
    unsigned char *ippacket;
    unsigned char ipversion;
    unsigned char iplength;
    unsigned int packet_length;
    unsigned int caplen;
} mdmi_header_capturepackets;

struct info_to_capture_callback{
    pcap_t  *handle;
    PacketMonitor *me;
};

typedef struct {
    EncryptionType encType;
    char info1[SPI_SIZE];
    char info2[SPI_SIZE];
} AddKeyEvent;

/***************************************************************************************************
 * @Function Constructor of Message
 * @brief It will reset all internal fields in Message
 **************************************************************************************************/
Message::Message():what(0), event(0), data(NULL) {
    // Message Constructor: DO NOTHING
}

/***************************************************************************************************
 * @Function Destructor of Message
 * @brief It will free all internal fields in Message
 **************************************************************************************************/
Message::~Message() {

    this->what = 0;
    this->event = 0;
    if(this->data != NULL) {
        free(this->data);
        this->data = NULL;
    }
}

/***************************************************************************************************
 * @Function Constructor of PacketMonitor
 * @brief Initalization variable of PacketMonitor
 **************************************************************************************************/
PacketMonitor::PacketMonitor(): mIsRunning(false),
                                mIsStopping(false), mMainThread(NULL), mHandle(NULL), mCount(0),
                                mSa(NULL), mMdm(NULL), mParam(0)
{
    PKMM_LOGD("[PacketMonitor] created");
    mIsMdLegacy = isLegacy();
}

/***************************************************************************************************
 * @Function Destructor of PacketMonitor
 * @brief Finalization of PacketMonitor
 **************************************************************************************************/
PacketMonitor::~PacketMonitor()
{
    PKMM_LOGD("[PacketMonitor] deleted");
}

/***************************************************************************************************
 * @Function Creator of PacketMonitor
 * @brief It will check parameters and create a new instance of PacketMonitor
 **************************************************************************************************/
PacketMonitor * PacketMonitor::create() {

    PacketMonitor *monitor = new PacketMonitor;
    Int64 param = apmGetSessionParam();
    monitor->mParam = param;

    PKMM_LOGD("[PacketMonitor] Mode: %llx", param);
    if(IS_MODE_PARAM(PARAMETER_IGNORE_DECRYPTION, param)) {
        PKMM_LOGD("[PacketMonitor] Mode of ignoring encryption is enabled");
    }
    else {
        PKMM_LOGD("[PacketMonitor] Mode of ignoring encryption is disabled");
    }

    return monitor;
}

/***************************************************************************************************
 * @Function Start monitor
 * @brief Start PacketMonitor
 **************************************************************************************************/
int PacketMonitor::start() {

    std::unique_lock<std::mutex> locker(mLifecycleMutex);
    if(mIsStopping) {
        return PKM_ERROR_SERVICE_STOPPING;
    }

    if(mIsRunning) {
        return PKM_ERROR_SERVICE_RUNNING;
    }

    PKMM_LOGD("[PacketMonitor] start PacketMonitor");
    mIsRunning = true;
    mMainThread = new thread(&(PacketMonitor::runMainThread), this);
    mMainThread->join();

    return PKM_OK;
}

/***************************************************************************************************
 * @Function Stop monitor
 * @brief Stop PacketMonitor
 **************************************************************************************************/
void PacketMonitor::stop() {

    bool stop = false;
    {
        std::unique_lock<std::mutex> locker(mLifecycleMutex);
        if(mIsRunning) {
            mIsRunning = false;
            mIsStopping = true;
            mCv.notify_one();
            stop = true;
        }
    }

    if(stop) {
        PKMM_LOGD("[PacketMonitor] stop PacketMonitor");
        if(mMainThread != NULL) {
            mMainThread->join();
            delete(mMainThread);
            mMainThread = NULL;
        }
    }
}

/***************************************************************************************************
 * @Function onLtecsrRtpEvent
 * @brief Receive RTP Portd from MDM
 **************************************************************************************************/
void PacketMonitor::onLtecsrRtpEvent(uint8_t event, uint16_t src, uint16_t dest) {

    PKMM_LOGD("onLtecsrRtpEvent(%d)", event);
    std::unique_lock<std::mutex> locker(mMutexRtp);
    if (event == 1) {
        for (int i = 0, size = mRtpList.size(); i != size; i++) {
            if (mRtpList[i]->equals(src, dest)) {
                // Do nothing if RTP ports are already present
                break;
            }
        }

        PKMM_LOGI("onLtecsrRtpEvent: Add(%u, %u) in list", src, dest);

        RtpPorts *port = new RtpPorts(src, dest);
        mRtpList.push_back(port);
    } else {
        for (int i = 0, size = mRtpList.size(); i != size; i++) {
            if (mRtpList[i]->equals(src, dest)) {
                delete mRtpList[i];
                mRtpList.erase(mRtpList.begin() + i);
                break;
            }
        }
    }
}

/***************************************************************************************************
 * @Function onAddKeyEvent
 * @brief Event of AddKey from MDM
 **************************************************************************************************/
void PacketMonitor::onAddKeyEvent(EncryptionType type, char *spi1, char *spi2) {

    PKMM_LOGI("onAddKeyEvent: Add(%s, %s) in list", spi1, spi2);

    AddKeyEvent *event = new AddKeyEvent;
    event->encType = type;
    if(spi1 == NULL) {}
    else {
        memcpy(event->info1, spi1, SPI_SIZE);
    }

    if(spi2 == NULL) {}
    else {
        memcpy(event->info2, spi2, SPI_SIZE);
    }

    Message *msg = new Message;
    msg->what = 0;
    msg->event = MESSAGE_KEY_ADD_EVENT;
    msg->data = event;

    sendMessage(msg);
}

/***************************************************************************************************
 * @Function isRtpPort
 * @brief Check if port is the same as RTP ports in list
 **************************************************************************************************/
bool PacketMonitor::isRtpPort(int src, int dest) {

    PKMM_LOGI("isRtpPort: src_port: %d, dst_port: %d", (int)src, (int)dest);
    std::unique_lock<std::mutex> locker(mMutexRtp);
    for (int i = 0, size = mRtpList.size(); i != size; i++) {
        if(mRtpList[i]->equals(src, dest)) {
            return true;
        }
    }

    return false;
}

/***************************************************************************************************
 * @Function runPacketMonitorThread
 * @brief Run tasks of packet monitor
 **************************************************************************************************/
void* PacketMonitor::runQueryNwPackets(void *arg)
{
        char errbuf[PCAP_ERRBUF_SIZE] = {0} ; /* Error string */
        pcap_handler callback;                /* Callback called on packet receive */

        struct info_to_capture_callback handle_info = {NULL, NULL};
        u_char *userdata;
        fd_set fd_wait;
        struct timeval st;
        int t;
#ifdef CAPTURE_DEBUG
        int filename_len = 0;
#endif

        PacketMonitor *me = static_cast<PacketMonitor*>(arg);

        /* Open the session */
        PKM_LOGD("[QueryNwPackets] start");
#ifdef CAPTURE_DEBUG
        me->mHandle = pcap_open_offline("/sdcard/tcpdump.cap", errbuf);
#else
        me->mHandle = pcap_open_live("any", SNAPLEN, 1, PCAP_TIMEOUT_1S, errbuf);
#endif
        if(me->mHandle == NULL)
        {
            PKMM_LOGE("[QueryNwPackets] pcap_open_live failed %s", errbuf);
            return NULL;
        }

        handle_info.handle = me->mHandle;
        handle_info.me = me;
        userdata = (u_char*)&handle_info;

        // Capture till STOP is received
        callback = (pcap_handler)me->nwHandler;
#ifdef CAPTURE_DEBUG
        ret_status = pcap_loop(me->mHandle, 0, callback, userdata);
#else
        // Set pcap_dispatch to non-block mode
        if(pcap_setnonblock(me->mHandle, 1, errbuf) == 1) {
            PKM_LOGE("[QueryNwPackets] pcap_setnonblock failed %s", errbuf);
            pcap_close(me->mHandle);
            me->mHandle = NULL;
            return NULL;
        }

        while(me->mIsRunning) {

            /* If you include STDIN_FILENO, be sure to read from it when you get
               traffic from it, or select will return every time it is called,
               since there will still be traffic waiting there.
             */

            FD_ZERO(&fd_wait);
            /* FD_SET(STDIN_FILENO, &fd_wait);*/
            FD_SET(pcap_fileno(me->mHandle), &fd_wait);

            st.tv_sec  = 1;   // 1 = 1 second
            st.tv_usec = 0;   // 1000 = 1 second

            PKM_LOGD("[QueryNwPackets] start select");
            t = select(FD_SETSIZE, &fd_wait, NULL, NULL, &st);
            PKM_LOGD("[QueryNwPackets] end select");
            switch(t)
            {
                case -1: // Something went really wrong

                    PKM_LOGE("[QueryNwPackets] Capture packet error");
                    pcap_close(me->mHandle);
                    me->mHandle = NULL;
                    return NULL;

                case  0: // We timed out, no trafffic
                    PKM_LOGD("[QueryNwPackets] No Traffic");
                    break;

                default: // We got traffic
                    PKM_LOGD("[QueryNwPackets] pcap_dispatch");
                    pcap_dispatch(me->mHandle, me->mCount, callback, userdata);
            }

            /* Put your service routine here.. is called when select times out
               or when a packet is processed
             */

        } /* End of while */


#endif

        if (me->mHandle) {
            /* And close the session */
            pcap_close(me->mHandle);
            me->mHandle = NULL;
        }

        PKM_LOGD("[QueryNwPackets] stop");
        return NULL;
}

void PacketMonitor::nwHandler(u_char *user, const u_char* header, const u_char *packet)
{
    struct info_to_capture_callback *handle_info = (struct info_to_capture_callback*) user;
    PacketMonitor *me = handle_info->me;

    if (NULL != me)
        me->packetsReceiver(header, packet);
}

/***************************************************************************************************
 * Internal Handler to process packet (Callback of pcap_dispatch)
 * @param mtk_header MtkPCab Header
 * @param packet Full Packet (Include Linux Part & IP Packet)
 *
 **************************************************************************************************/
void PacketMonitor::packetsReceiver(const u_char* mtk_header, const u_char *packet)
{
    // Structure info_to_capture_callback *handle_info = (struct info_to_capture_callback*) user;
    mdmi_header_capturepackets *ext = (mdmi_header_capturepackets *)mtk_header;

    if(!mIsRunning) {
        PKM_LOGE("[Received] service is stopping, ignore packets");
        return;
    }

    // Check Direction
    int direction = DIRECTION_UNKNOW;
    if(ext->pkttype == UNICAST_TO_US) {
        direction = DIRECTION_DOWNSTREAM;
    }
    else if(ext->pkttype == SEND_BY_US) {
        direction = DIRECTION_UPSTREAM;
    }
    else {
        PKM_LOGE("[Received][%d][Exception] Invalid direction: %d", ext->pkttype, direction);
        return;
    }

    if(mCount >= INT_MAX) {
        mCount = 0;
    }

    ++mCount;

    // Get timestamp
    struct timeval tv;
    gettimeofday(&tv, NULL);

    UInt64 timestamp =
            (UInt64)(tv.tv_sec)*1000 +
            (UInt64)(tv.tv_usec)/1000;

    const char* wifiInterface = "wlan";
    const char* lteInterface = "ccmni";

    // Find SIM & Packet Type
    int sim = 0;
    int src = SOURCE_UNKNOWN;
    if(!strncmp(ext->ifname, lteInterface, 5)) {
        if(mapImsNetwork(ext->ifname, &sim)) {
            src = SOURCE_VOLTE;
        }
        else {
            src = SOURCE_NONVOLTE;
            if(!mapNonImsNetwork(ext->ifname, &sim)) {
                PKM_LOGE("[Received][%d][Exception] Cannot find SIM-Id", mCount);
            }
        }
    }
    else if(!strncmp(ext->ifname, wifiInterface, 4)) {
        src = SOURCE_WIFI;
    }
    else {
        PKM_LOGE("[Received][Exception] Skipped NIC: %s", ext->ifname);
        mCount--;
        return; // Interface other than LTE or Wifi (No need to concern)
    }

    // Extract IP Packet
    int ver = packet[0] >> 4;
    int ip_len = 0;
    int ip_hdr_len = 0;

    // Compute Protocol, IP Length & IP Length
    switch(ver) {
        case IPv4:
        {
            struct iphdr* ipv4_hdr = (struct iphdr*)packet;
            ip_len = EXTRACT_16BITS(&ipv4_hdr->tot_len);
            ip_hdr_len = (ipv4_hdr->ihl & 0x0F) * 4;

            if(ip_hdr_len < 20) {
                PKM_LOGE("[Received][%d][Exception] IPv4 Packet header size is incorrect", mCount);
                return;
            }

            break;
        }
        case IPv6:
        {
            struct ip6_hdr* ipv6_hdr = (struct ip6_hdr*)packet;
            ip_hdr_len = IPV6_FIX_HEADER_LENGTH;
            ip_len = EXTRACT_16BITS(&ipv6_hdr->ip6_plen) + ip_hdr_len;

            break;
        }
        default:
            PKM_LOGE("[Received][%d][Exception] Packet is invalid", mCount);
            return;
    }

    // Check source packet size
    if(ip_len > ext->packet_length) {
        PKM_LOGE("[Received][%d][Exception] Size of packet is not incorrect", mCount);
        return;
    }

    PKMM_LOGD("[Received][%d][%s]:IPv%d, src:%d, size:%d, ip_hdr:%d", mCount, ext->ifname,
                                                                      ver, src, ip_len, ip_hdr_len);

    // Handle Packet
    packetsHandler(packet, ip_len, ip_hdr_len, mCount, src, sim,
                   direction, timestamp, false, false);
}

/***************************************************************************************************
 * Handle Packets
 * @param packet Packet
 * @param size IP packet size
 * @param hdr_size IP header size
 * @param count Packet number
 * @param src Source of packet
 *
 **************************************************************************************************/
void PacketMonitor::packetsHandler(const u_char *packet, int size, int hdr_size, int count, int src,
                                   int sim, int direction, UInt64 timestamp,
                                   bool decrypted, bool clone)
{
    PKMM_LOGD("[Decode][%d], sim: %d, direct:%d, decrypt:%d, clone:%d",count, sim, direction,
                                                                       decrypted, clone);

    int rc = 0;
    IpPacketInfo info = {0};

    info.packet = packet;
    info.len = size;
    info.sim = sim;
    info.source = src;
    info.direction = direction;
    info.timestamp = timestamp;
    info.count = count;

    if(!extractInfo(&info)) {
        PKMM_LOGD("[Decode][%d][Exception] Extract fail", count);
        return;
    }

    PKMM_LOGD("ExtractInfo: type = %d, info1 = %s, info2 = %s, initiator = %d", info.encType,
                                                                                info.info1,
                                                                                info.info2,
                                                                                info.initiator);

    switch(info.packetEncType) {
        case PacketEncryptionTypeNone:
        case PacketEncryptionTypeIKev2Unencrypted:
            break;

        case PacketEncryptionTypeEsp:
        case PacketEncryptionTypeEspUdp:
        case PacketEncryptionTypeIKev2:
        {
            // Check if No Decryption Mode
            if(IS_MODE(PARAMETER_IGNORE_DECRYPTION)) {}
            else {
                rc = decryptPacket(&info);
                if(!clone && DECRYPT_KEY_NOT_FOUND == rc) {
                    addPendingQueue(&info, false);
                    return;
                }
                else if(DECRYPT_RESULT_OK == rc) {}
                else {
                    PKM_LOGD("[Decode][%d][Exception]  Decrypt fail", count);
                }
            }
            break;
        }

        case PacketEncryptionTypeNattKeepAlive:
        {
            if(apmShouldSubmitKpi((APM_MSGID)APM_MSG_PKM_WIFI_NATT_KEEP_ALIVE)) {

                // Send Packets
                sendPacket(packet, size, hdr_size, src, sim, direction, info.version,
                           info.protocol,
                           PACKET_TYPE_NATT_KEEP_ALIVE, ENCRYPTION_NONE,
                           (APM_MSGID)APM_MSG_PKM_WIFI_NATT_KEEP_ALIVE,
                           timestamp, count,
                           hdr_size, size - hdr_size, 0);
            }

            return;
        }
    }

    int next = 0;
    uint8_t *pkt = NULL;
    if(info.espNext > 0) {
        // Cursor to payload
        next = info.espNext;
        pkt = (uint8_t *)(info.packet + info.offset);
    }
    else {
        // Original Packet with Encryption
        next = info.protocol;
        pkt = (uint8_t *)(info.packet + info.hdrLen);
    }

    if(PROTOCOL_IP == next) {
        if(!extractInfo(&info)) {
            return;
        }

        next = info.protocol;
        PKMM_LOGD("ExtractInfo: type = %d, info1 = %s, info2 = %s, initiator = %d", info.encType,
                                                                                    info.info1,
                                                                                    info.info2,
                                                                                    info.initiator);
    }

    int ipPayloadOffset = pkt - packet;
    int ipPayloadSize = size - ipPayloadOffset;

    PKMM_LOGD("[Decode][%d] payload_offset: %d, payload_size: %d", count, ipPayloadOffset,
                                                                          ipPayloadSize);

    // Decode IP Payload
    uint16_t srcPort =0;
    uint16_t dstPort = 0;
    int transportPacketHdrSize = 0;
    bool isNotTcpUdp = false;

    // Category Packet's Application Type and its header
    int packetType = PACKET_TYPE_UNKNOWN;
    switch(next) {
        case PROTOCOL_TCP: {
            packetType = PACKET_TYPE_TCP;
            struct tcphdr *tcph = (struct tcphdr*)pkt;
            transportPacketHdrSize = tcph->doff * 4;
            srcPort = ntohs(tcph->source);
            dstPort = ntohs(tcph->dest);

            break;
        }
        case PROTOCOL_UDP: {
            packetType = PACKET_TYPE_UDP;
            struct udphdr *udph = (struct udphdr*)pkt;
            transportPacketHdrSize = 8;
            srcPort = ntohs(udph->source);
            dstPort = ntohs(udph->dest);

            break;
        }
        case PROTOCOL_ESP: {
            packetType = PACKET_TYPE_ESP;
            isNotTcpUdp = true;
            transportPacketHdrSize = 8;

            break;
        }
        default:
            isNotTcpUdp = true;
            transportPacketHdrSize = 0;
            packetType = 0x10000 | (next & 0xFFFF);
    }

    PKMM_LOGD("[Decode][%d] Port:(%d, %d), size: %d", count, srcPort, dstPort,
                                                      transportPacketHdrSize);

    int appPacketOffset = pkt - packet + transportPacketHdrSize;
    int appPacketSize = size - appPacketOffset;
    int appPacketHeader = 0;

#define IS_PORT(p) (srcPort == (p) || dstPort == (p))

    if(isNotTcpUdp) {
        // Nothing
    }
    else if (IS_PORT(PORT_DNS)) {                                 // DNS-OTA-Message
        packetType = PACKET_TYPE_DNS;
        appPacketHeader = DNS_HDR_LENGTH;

    }
    else if(IS_PORT(PORT_SIP)) {                                  // SIP-OTA-Message
        if(appPacketSize == 0) { // If Port = SIP, but payload is empty, use upper protocol instead
            packetType = next;   // Replace to TCP/UDP
            appPacketOffset = ipPayloadOffset;
            appPacketSize = ipPayloadSize;
            appPacketHeader = transportPacketHdrSize;
        }
        else {
            packetType = PACKET_TYPE_SIP;
        }
    }
    else if(IS_PORT(PORT_ISAKMP) || IS_PORT(PORT_ISAKMP_NATT)) {  // ISAKMP-OTA-Message
        packetType = PACKET_TYPE_ISAKMP;
    }
    else if (isRtpPort(srcPort, dstPort)) {                       // RTP-OTA-Message
        packetType = PACEKT_TYPE_RTP;
        uint8_t *rtp = (uint8_t *) (pkt + transportPacketHdrSize);

        // RTP Header = Fix length(12) + Additional
        appPacketHeader = 12 + ((rtp[0] & 0x0F) * 4);
    }
    else {
        packetType = next;   // Replace to TCP/UDP
        appPacketOffset = ipPayloadOffset;
        appPacketSize = ipPayloadSize;
        appPacketHeader = transportPacketHdrSize;
    }

    PKMM_LOGD("[Decode][%d] type: %d, appPacket: offset: %d, size: %d, header_size: %d", count,
                                    packetType, appPacketOffset, appPacketSize,appPacketHeader);

    // If no property, use scenario to judge source
    if(SOURCE_CELLUAR == src) {
        if(SCENARIO_ESP_1_TRANSPORT == info.scenario) {
            src = SOURCE_VOLTE;
        }
        else {
            if(PACKET_TYPE_SIP == packetType || PACEKT_TYPE_RTP == packetType) {
                src = SOURCE_VOLTE;
            }
            else {
                src = SOURCE_NONVOLTE;
            }
        }
    }

    // Encryption Category
    int encType = ENCRYPTION_NONE;
    switch(info.scenario) {
        case SCENARIO_ESP_1_TRANSPORT:
            encType = ENCRYPTION_ESP_1_TRANSPORT;
            break;
        case SCENARIO_ESP_1_TUNNEL:
            encType = ENCRYPTION_ESP_1_TURNNEL;
            break;
        case SCENARIO_ESP_2:
            encType = ENCRYPTION_ESP_2;
            break;
    }

    // Filter out message by source
    APM_MSGID id = 0;
    switch(src) {
        case SOURCE_VOLTE:
            switch(packetType) {
                case PACEKT_TYPE_RTP:
                    id = (APM_MSGID) APM_MSG_PKM_VOLTE_RTP;
                    if(!apmShouldSubmitKpi(id)) {
                        PKMM_LOGD("[Decode][%d] VoLTE/RTP is not subscribe", count);
                        return;
                    }
                    break;
                case PACKET_TYPE_DNS:
                    id = (APM_MSGID) APM_MSG_PKM_VOLTE_DNS;
                    if(!apmShouldSubmitKpi(id)) {
                        PKMM_LOGD("[Decode][%d] VoLTE/DNS is not subscribe", count);
                        return;
                    }
                    break;
                case PACKET_TYPE_SIP:
                    id = (APM_MSGID) APM_MSG_PKM_VOLTE_SIP;
                    if(!apmShouldSubmitKpi(id)) {
                        PKMM_LOGD("[Decode][%d] VoLTE/SIP is not subscribe", count);
                        return;
                    }
                    break;
                default:
                    id = (APM_MSGID) APM_MSG_PKM_VOLTE_OTHERS;
                    if(!apmShouldSubmitKpi(id)) {
                        PKMM_LOGD("[Decode][%d] VoLTE/Others is not subscribe", count);
                        return;
                    }
                    break;
            }
            break;
        case SOURCE_NONVOLTE:
            switch(packetType) {
                case PACEKT_TYPE_RTP:
                    id = (APM_MSGID) APM_MSG_PKM_NONVOLTE_INTERNET;
                    if(!apmShouldSubmitKpi(id)) {
                        PKMM_LOGD("[Decode][%d] Non-VoLTE/RTP is not subscribe", count);
                        return;
                    }
                    break;
                case PACKET_TYPE_DNS:
                    id = (APM_MSGID) APM_MSG_PKM_NONVOLTE_DNS;
                    if(!apmShouldSubmitKpi(id)) {
                        PKMM_LOGD("[Decode][%d] Non-VoLTE/DNS is not subscribe", count);
                        return;
                    }
                    break;
                case PACKET_TYPE_SIP:
                    id = (APM_MSGID) APM_MSG_PKM_NONVOLTE_INTERNET;
                    if(!apmShouldSubmitKpi(id)) {
                        PKMM_LOGD("[Decode][%d] Non-VoLTE/SIP is not subscribe", count);
                        return;
                    }
                    break;
                default:
                    id = (APM_MSGID) APM_MSG_PKM_NONVOLTE_INTERNET;
                    if(!apmShouldSubmitKpi(id)) {
                        PKMM_LOGD("[Decode][%d] Non-VoLTE/Other is not subscribe", count);
                        return;
                    }
                    break;
            }

            break;
        case SOURCE_WIFI:
            switch(packetType) {
                case PACEKT_TYPE_RTP:
                    id = (APM_MSGID) APM_MSG_PKM_WIFI_RTP;
                    if(!apmShouldSubmitKpi(id)) {
                        PKMM_LOGD("[Decode][%d] WI-FI/RTP is not subscribe", count);
                        return;
                    }
                    break;
                case PACKET_TYPE_DNS:
                    id = (APM_MSGID) APM_MSG_PKM_WIFI_DNS;
                    if(!apmShouldSubmitKpi(id)) {
                        PKMM_LOGD("[Decode][%d] WI-FI/DNS is not subscribe", count);
                        return;
                    }
                    break;
                case PACKET_TYPE_SIP:
                    id = (APM_MSGID) APM_MSG_PKM_WIFI_SIP;
                    if(!apmShouldSubmitKpi(id)) {
                        PKMM_LOGD("[Decode][%d] WI-FI/SIP is not subscribe", count);
                        return;
                    }
                    break;
                case PACKET_TYPE_ISAKMP:
                    id = (APM_MSGID) APM_MSG_PKM_WIFI_ISAKMP;
                    if(!apmShouldSubmitKpi(id)) {
                        PKMM_LOGD("[Decode][%d] ISAKMP is not subscribe", count);
                        return;
                    }
                    break;
                case PACKET_TYPE_NATT_KEEP_ALIVE:
                    id = (APM_MSGID) APM_MSG_PKM_WIFI_NATT_KEEP_ALIVE;
                    if(!apmShouldSubmitKpi(id)) {
                        PKMM_LOGD("[Decode][%d] WI-FI/NATT is not subscribe", count);
                        return;
                    }
                    break;
                default:
                    id = (APM_MSGID) APM_MSG_PKM_WIFI_INTERNET;
                    if(!apmShouldSubmitKpi(id)) {
                        PKMM_LOGD("[Decode][%d] WI-FI/Others is not subscribe", count);
                        return;
                    }
                    break;
            }
            break;
        default:
            return;
    }

    // Send Packets
    sendPacket(packet, size, hdr_size, src, sim, direction, info.version,
               info.protocol, packetType, encType, id, timestamp,
               count, appPacketOffset, appPacketSize, appPacketHeader);


    // UT Check List ===============================================================================
    // MDMI: Message Id                      Status
    //     - IpOtaPacketWithPayload          PASS
    //     - IP-OTA-Packet                   PASS
    //     - sip-OTA-Message                 PASS (Some Messages with timing issues)
    //     - DNS-OTA-Message                 PASS
    //     - RTP-OTA-Message                 PASS
    //     - wifiIpOtaPacketWithPayload
    //     - wifi-ip-OTA-Packet
    //     - WIFI-DNS-OTA-Message
    //     - wifi-ISAKMP-OTA-Message
    //     - WIFI-RTP-OTA-Message
    //     - WIFI-SIP-OTA-Message
    // =============================================================================================


}

/***************************************************************************************************
 * addPendingQueue
 * @param info IP packet information
 *
 **************************************************************************************************/
void PacketMonitor::addPendingQueue(IpPacketInfo *info, bool decrypted) {

    std::unique_lock<std::mutex> locker(mMutexPendingQueue);

    PendingPacket *pp = new PendingPacket;
    if (pp == NULL) {
         PKMM_LOGE("addPendingQueue() new PendingPacket failed!");
         return;
    }
    pp->decrypted = decrypted;
    pp->packet = (UInt8 *)malloc(sizeof(UInt8) * (info->len + 1));
    if (pp->packet == NULL) {
         PKMM_LOGE("addPendingQueue() malloc PendingPacket.packet failed!");
         delete pp;
         pp = NULL;
         return;
    }
    memcpy(pp->packet, info->packet, info->len);
    pp->size = info->len;
    pp->hdr_size = info->hdrLen;
    pp->sim = info->sim;
    pp->src = info->source;
    pp->direction = info->direction;
    pp->timestamp = info->timestamp;
    pp->count = info->count;

    pp->encType = info->encType;

    memset(pp->info1, 0, MAX_SPI);
    memcpy(pp->info1, info->info1, SPI_SIZE);
    memset(pp->info2, 0, MAX_SPI);
    memcpy(pp->info2, info->info2, SPI_SIZE);
    PKMM_LOGD("Save pending packet, spi1=%s, spi2=%s", pp->info1, pp->info2);

    mPendingList.push_back(pp);

}

void PacketMonitor::setCapability()
{
    struct __user_cap_header_struct header;
    struct __user_cap_data_struct cap;
    header.version = _LINUX_CAPABILITY_VERSION;
    header.pid = 0;

    PKMM_LOGD("set_capability");

    if(capget(&header, &cap) < 0) {
        PKMM_LOGE("capget failed : %s", strerror(errno));
        return;
    }

    cap.effective = cap.permitted = 1 << CAP_NET_RAW | 1 << CAP_NET_ADMIN
            | 1 << CAP_NET_BIND_SERVICE | 1 << CAP_SYS_BOOT;

    if (capset(&header, &cap) < 0) {
        PKMM_LOGE("capset failed : %s", strerror(errno));
    }
}

/***************************************************************************************************
 * @Function runMainThread
 * @brief Run tasks of main (life-cycle control, error handling)
 **************************************************************************************************/
void PacketMonitor::runMainThread(void *data) {

    PacketMonitor *instance = static_cast<PacketMonitor *>(data);
    instance->looper();
}

/***************************************************************************************************
 * @Function Looper
 * @brief Main thread of PacketMonitor
 **************************************************************************************************/
void PacketMonitor::looper() {

    PKM_LOGD("Start PKM Service ... v0.010");

    // Register & Start Receive MDM event
    mSa = new SecurityAssociation();
    mMdm = new MdmReceiver(mSa, this);

    // Start capturing OTA packets
    thread monitor(&(PacketMonitor::runQueryNwPackets), this);

    while(mIsRunning) {

        Message *message = NULL;
        std::unique_lock<std::mutex> locker(mMainMutex);
        while (mQueue.empty()) {
            if (mIsRunning) {
                PKMM_LOGD("looper WAIT");
                mCv.wait(locker);
                PKMM_LOGD("looper AWAKE");
            }
            else {
                break;
            }
        }

        if(mIsRunning) {
            message = mQueue.front();
            mQueue.pop();

            handleMessage(message);
            delete(message);
        }
        else {
            break;
        }
    }

    PKMM_LOGD("Dequeue all ...");
    while (!mQueue.empty()) {
        Message *message = mQueue.front();
        mQueue.pop();
        delete(message);
        PKMM_LOGD("Remove message");
    }

    // Release Resources
    delete(mMdm);

    // Wait for closing thread of packet monitor
    monitor.join();

    // Release Resources
    delete(mSa);

    mIsStopping = false;
    PKMM_LOGD("Stop PKM Service ...");
}

/***************************************************************************************************
 * @Function Loop
 * @brief Send message to message queue
 **************************************************************************************************/
void PacketMonitor::sendMessage(Message *message) {

    PKMM_LOGD("Send message");
    if(mIsRunning) {}
    else {
        PKMM_LOGD("[sendMessage] Service is stopped.");
        return;
    }

    PKMM_LOGD("[sendMessage] Enqueue message");
    std::unique_lock<mutex> locker(mMainMutex);
    mQueue.push(message);
    mCv.notify_one();

}

/***************************************************************************************************
 * @Function handleMessage
 * @brief Handle message from queue
 * @param msg message
 **************************************************************************************************/
void PacketMonitor::handleMessage(Message *msg) {

    switch(msg->event) {
        case MESSAGE_KEY_ADD_EVENT:
            if(msg->data != NULL) {
                handleKeyAddEvent(msg->data);
            }
            break;
    }

}

/***************************************************************************************************
 * @Function handleKeyAddEvent
 * @brief Handle message of AddKeyEvent from queue
 * @param data raw data
 **************************************************************************************************/
void PacketMonitor::handleKeyAddEvent(void *data) {

    AddKeyEvent *evt = (AddKeyEvent *) data;
    PKMM_LOGD("handleKeyAddEvent, spi1=%s, spi2=%s", evt->info1, evt->info2);
    vector<PendingPacket *> packets;

    mMutexPendingQueue.lock();
    PKMM_LOGD("handleKeyAddEvent, size = %uld", mPendingList.size());
    for (int i = 0, size = mPendingList.size(); i < size; i++) {

        packets.push_back(mPendingList[i]);


        /**
        if(mPendingList[i]->encType == evt->encType) {
            switch(evt->encType) {
                case EncryptionTypeEsp:
                    if (!strcasecmp(mPendingList[i]->info1, evt->info1))
                    {
                        packets.push_back(mPendingList[i]);
                        mPendingList.erase(mPendingList.begin() + i);
                    }
                    else {
                        i++;
                    }
                    break;
                case EncryptionTypeIKev2:
                    if (!strcasecmp(mPendingList[i]->info1, evt->info1) &&
                        !strcasecmp(mPendingList[i]->info2, evt->info2))
                    {
                        packets.push_back(mPendingList[i]);
                        mPendingList.erase(mPendingList.begin() + i);
                    }
                    else {
                        i++;
                    }
                    break;
                default:
                    i++;
            }
        }*/
    }

    mPendingList.clear();
    mMutexPendingQueue.unlock();

    PKMM_LOGD("handleKeyAddEvent, take action size = %d, origin size = %d", packets.size(), mPendingList.size());
    for(int i = 0, size = packets.size(); i < size; i ++) {
        if(packets[i] != NULL) {
            PKMM_LOGD("handleKeyAddEvent and take action");
            PendingPacket *pp = packets[i];

            // Handle Packet
            packetsHandler(pp->packet, pp->size, pp->hdr_size, pp->count, pp->src, pp->sim,
                           pp->direction, pp->timestamp, pp->decrypted, true);

            if(pp->packet != NULL) {
                free(pp->packet);
                pp->packet = NULL;
            }

            delete(pp);
        }
    }
    packets.clear();


}

/***************************************************************************************************
 * SendPacket
 * @param packet
 * @param size
 * @param ipHeaderLen
 * @param src
 * @param sim
 * @param direction
 * @param version
 * @param protocol
 * @param type
 * @param encType
 * @param timestamp
 * @param num
 * @param appPacketOffset
 * @param appPacketSize
 * @param appPacketHeaderLen
 *
 **************************************************************************************************/
void PacketMonitor::sendPacket(const u_char * packet, int size, int ipHeaderLen,
                               int src, int sim, int direction, int version, UInt16 protocol,
                               int type, int encType, APM_MSGID id,
                               Int64 timestamp, Int32 num,
                               int appPacketOffset, int appPacketSize, int appPacketHeaderLen)
{
    uint32_t payloadLen = sizeof(otaPacket_v1_t) + size;
    otaPacket_v1_t *pPacketInfo = (otaPacket_v1_t *)malloc(payloadLen);
    if (pPacketInfo == NULL) {
        PKM_LOGE("[sendPacket] Header: Malloc fail");
        return;
    }
    uint8_t *pRawDataOffset = (uint8_t *)pPacketInfo;
    memset(pPacketInfo, 0, sizeof(otaPacket_v1_t));

    pPacketInfo->version = 1;
    pPacketInfo->application = type;
    pPacketInfo->encType = encType;
    pPacketInfo->source = src;
    pPacketInfo->slot = sim;
    pPacketInfo->direction = direction;
    pPacketInfo->ipVersion = version;
    pPacketInfo->protocol = protocol;
    pPacketInfo->timestamp = timestamp;
    pPacketInfo->num = num;

    pPacketInfo->packetSize = size;
    pPacketInfo->ipHeaderSize = ipHeaderLen;

    pPacketInfo->appPacketSize = appPacketSize;
    pPacketInfo->appPacketOffset = appPacketOffset;
    pPacketInfo->appPacketHeaderSize = appPacketHeaderLen;

    // Write packet raw data after otaPacket_v1_t
    pRawDataOffset += sizeof(otaPacket_v1_t);
    memcpy(pRawDataOffset, packet, size);

    apmSubmitKpiST(id, (APM_SIMID)sim, (timeStamp)timestamp, payloadLen, (void *)pPacketInfo);

    // Free
    if (pPacketInfo != NULL) {
        free(pPacketInfo);
    }
}

/***************************************************************************************************
 * Decrypt Packet
 * @param info Information of IP Packets
 *
 **************************************************************************************************/
int PacketMonitor::decryptPacket(IpPacketInfo *info) {

    netdissect_options ndo = {0};
    uint8_t *packet = (uint8_t *)(info->packet + info->offset);
    int ret = -1;
    int blockSize = 0, authSize = 0;

    switch(info->packetEncType) {
        case PacketEncryptionTypeEsp:
        case PacketEncryptionTypeEspUdp: {

            ndo.ndo_espsecret = new char [1024];
            if (!mSa->findEspSecret(EncryptionTypeEsp,
                                    ndo.ndo_espsecret, 1024,
                                    &blockSize, &authSize,
                                    info->info1, info->info2, info->initiator))
            {
                PKMM_LOGE("Decrypt: ESP Security info not found.");
                delete [] ndo.ndo_espsecret;
                return DECRYPT_KEY_NOT_FOUND;
            }

            PKMM_LOGD("Decrypt: ESP secret: %s", ndo.ndo_espsecret);
            ret = mdmi_decrypt(&ndo, packet, info->len);

            PKMM_LOGD("Decrypt: mdmi_decrypt: %d", ret);
            delete [] ndo.ndo_espsecret;

            // ESP Trailer size
            int esp_auth_size = authSize / 8;

            // ESP Header = SPI(4 Bytes) + SEQ(4 Bytes) + IV size
            int esp_hdr_len = 8 + (blockSize / 8);

            // Move cursor to ESP trailer
            uint8_t *esp_last = packet + info->len - esp_auth_size - 2;

            int padding = esp_last[0] & 0xFF;
            info->espNext = esp_last[1] & 0xFF;

            // [IP Header][ESP Header][IV][Payload][ESP Trailer][ESP Authentication]
            int offset = info->hdrLen + esp_hdr_len;
            if(PacketEncryptionTypeEspUdp == info->packetEncType) {
                offset += 8;   // + UDP Header Length
            }

            PKMM_LOGE("offset: %d, hdr_len: %d, esp_hdr_len: %d", info->offset,
                                                                  info->hdrLen,
                                                                  esp_hdr_len);
            info->offset = info->offset + offset;
            info->len = info->len - offset - 2 - padding - esp_auth_size;

            if(SCENARIO_NONE == info->scenario) {
                if(PROTOCOL_ESP == info->espNext || PROTOCOL_IP == info->espNext) {
                    info->scenario = SCENARIO_ESP_1_TUNNEL;
                }
                else {
                    info->scenario = SCENARIO_ESP_1_TRANSPORT;
                }
            }
            else if(SCENARIO_ESP_1_TUNNEL == info->scenario) {
                info->scenario = SCENARIO_ESP_2;
            }
            else {
                PKMM_LOGE("Scenario is not correct: (%d, %d)", info->scenario, info->espNext);
                if(ndo.ndo_sa_list_head != NULL) {
                    free(ndo.ndo_sa_list_head);
                }
                return DECRYPT_STATUS_ERROR;
            }

            PKMM_LOGD("Decrypt: next: %d, padding: %d, offset: %d, len:%d",info->espNext
                                                                          ,padding
                                                                          ,info->offset
                                                                          ,info->len);



            break;
        }

        case PacketEncryptionTypeIKev2: {

            ndo.ndo_espsecret = new char [1024];
            if (!mSa->findEspSecret(EncryptionTypeIKev2, ndo.ndo_espsecret, 1024, NULL, NULL,
                                    info->info1, info->info2, info->initiator))
            {
                PKMM_LOGE("Decrypt: IKev2 Security info not found.");
                delete [] ndo.ndo_espsecret;
                return DECRYPT_KEY_NOT_FOUND;
            }

            PKMM_LOGD("Decrypt: IKev2 secret: %s", ndo.ndo_espsecret);
            ret = mdmi_decrypt(&ndo, packet, info->len);

            delete [] ndo.ndo_espsecret;
            break;
        }

        default:
           break;
    }

    if(ndo.ndo_sa_list_head != NULL) {
        free(ndo.ndo_sa_list_head);
    }

    if(ret == 0) {
        return DECRYPT_RESULT_OK;
    }
    else {
        return DECRYPT_FAIL;
    }
}

/***************************************************************************************************
 * Find encryption information from IPsec Packet and extract it
 * @param info IP Packet Info
 *
 **************************************************************************************************/
bool PacketMonitor::extractInfo(IpPacketInfo *info) {

    if(info == NULL) {
        return false;
    }

    uint8_t *packet = (uint8_t *)(info->packet + info->offset);
    info->version = packet[0] >> 4;

    int ip_len = 0;
    int ip_hdr_len = 0;
    int len = info->len;

    // Compute Protocol, IP Length & IP Length
    switch(info->version) {
        case IPv4:
        {
            struct iphdr* ipv4_hdr = (struct iphdr*)packet;
            info->protocol = ipv4_hdr->protocol;
            ip_len = EXTRACT_16BITS(&ipv4_hdr->tot_len);
            ip_hdr_len = (ipv4_hdr->ihl & 0x0F) * 4;

            if(ip_hdr_len < 20) {
                PKMM_LOGE("ExtractInfo: IPv4 Packet header size is incorrect.");
                return false;
            }

            // Decode IP
            uint8_t *addr = (uint8_t*)&ipv4_hdr->saddr;
            snprintf(info->srcIp, sizeof(info->srcIp), "%u.%u.%u.%u", addr[0], addr[1],
                                                                      addr[2], addr[3]);
            addr = (uint8_t*)&ipv4_hdr->daddr;
            snprintf(info->dstIp, sizeof(info->dstIp), "%u.%u.%u.%u", addr[0], addr[1],
                                                                      addr[2], addr[3]);
            break;
        }
        case IPv6:
        {
            struct ip6_hdr* ipv6_hdr = (struct ip6_hdr*)packet;
            info->protocol = ipv6_hdr->ip6_nxt;
            ip_hdr_len = IPV6_FIX_HEADER_LENGTH;
            ip_len = EXTRACT_16BITS(&ipv6_hdr->ip6_plen) + ip_hdr_len;

            // Decode IP
            uint8_t *addr = (uint8_t*)&ipv6_hdr->ip6_src;
            snprintf(info->srcIp, sizeof(info->srcIp), "%u:%u:%u:%u:%u:%u:%u:%u", addr[0], addr[1],
                                                                                  addr[2], addr[3],
                                                                                  addr[4], addr[5],
                                                                                  addr[6], addr[7]);
            addr = (uint8_t*)&ipv6_hdr->ip6_dst;
            snprintf(info->dstIp, sizeof(info->dstIp), "%u:%u:%u:%u:%u:%u:%u:%u", addr[0], addr[1],
                                                                                  addr[2], addr[3],
                                                                                  addr[4], addr[5],
                                                                                  addr[6], addr[7]);
            break;
        }
        default:
            return false;
    }

    // Verify IP Length
    if(len < ip_len || ip_hdr_len > ip_len) {
        PKMM_LOGE("ExtractInfo: IP Packet size is incorrect.");
        return false;
    }

    // Save Header size to IP packet info
    info->hdrLen = ip_hdr_len;

    // Save size of IP packet info
    info->len = ip_len;

    // Save size (Reserve the biggest IP packet size)
    if(info->ipLen == 0) {
        info->ipLen = ip_len;
    }

    // IP Basic Information
    PKMM_LOGD("IPv%d, Number: %d, len: %d ,hdr_len: %d, src_ip: %s, dst_ip: %s", info->version,
                                                                               info->protocol,
                                                                               info->len,
                                                                               info->hdrLen,
                                                                               info->srcIp,
                                                                               info->dstIp);

    info->packetEncType = PacketEncryptionTypeNone;
    info->encType = EncryptionTypeNone;
    if (info->protocol == IPPROTO_ESP) {

        // Extract IPSec ESP Header to find 'SPI'
        struct newesp *esp = (newesp *)(packet + ip_hdr_len);
        snprintf(info->info1, 20, "%08X", EXTRACT_32BITS(&esp->esp_spi));
        info->packetEncType = PacketEncryptionTypeEsp;
        info->encType = EncryptionTypeEsp;

        PKMM_LOGD("ExtractInfo: ESP SPI: %s", info->info1);
        return true;
    }
    else if(IPPROTO_UDP == info->protocol) {

        // Extract UDP Header to find 'Port'
        struct udphdr *udph = (struct udphdr*) (packet + ip_hdr_len);
        const struct isakmp *isakmphdr = (const struct isakmp *)(udph + 1);
        uint16_t dst_port, src_port, udplen;

        udplen = EXTRACT_16BITS(&udph->uh_ulen);
        dst_port = EXTRACT_16BITS(&udph->dest);
        src_port = EXTRACT_16BITS(&udph->source);

        if (dst_port == PORT_ISAKMP) {
            if (ETYPE_IKE_SA_INIT == isakmphdr->etype) {
                info->packetEncType = PacketEncryptionTypeIKev2Unencrypted;
                info->encType = EncryptionTypeIKev2Unencrypted;
                return true;
            }

            info->packetEncType = PacketEncryptionTypeIKev2;
            info->encType = EncryptionTypeIKev2;
            info->initiator = isakmphdr->flags & ISAKMP_FLAG_I ? 1 : 0;

            snprintf(info->info1, 20, "%02X%02X%02X%02X%02X%02X%02X%02X", isakmphdr->i_ck[0],
                                                                          isakmphdr->i_ck[1],
                                                                          isakmphdr->i_ck[2],
                                                                          isakmphdr->i_ck[3],
                                                                          isakmphdr->i_ck[4],
                                                                          isakmphdr->i_ck[5],
                                                                          isakmphdr->i_ck[6],
                                                                          isakmphdr->i_ck[7]);
            snprintf(info->info2, 20, "%02X%02X%02X%02X%02X%02X%02X%02X", isakmphdr->r_ck[0],
                                                                          isakmphdr->r_ck[1],
                                                                          isakmphdr->r_ck[2],
                                                                          isakmphdr->r_ck[3],
                                                                          isakmphdr->r_ck[4],
                                                                          isakmphdr->r_ck[5],
                                                                          isakmphdr->r_ck[6],
                                                                          isakmphdr->r_ck[7]);

            return true;
        }
        else if (PORT_ISAKMP_NATT == dst_port) {
            if (udplen == 9 && *((u_char*)(udph + 1)) == 0xFF) {
                if (mSa->isEspRecordPresent(info->srcIp, info->dstIp)) {
                    info->packetEncType = PacketEncryptionTypeNattKeepAlive;
                    info->encType = EncryptionTypeNone;
                    return true;
                }
                else {
                    PKMM_LOGD("ExtractInfo: PacketEncryptionNone");
                }
            }
            else if((*(unsigned int *)isakmphdr) == 0) {
                info->packetEncType = PacketEncryptionTypeIKev2;
                info->encType = EncryptionTypeIKev2;
                isakmphdr = (struct isakmp *)((u_char *)isakmphdr + 4);

                info->initiator = isakmphdr->flags & ISAKMP_FLAG_I ? 1 : 0;
                snprintf(info->info1, 20, "%02X%02X%02X%02X%02X%02X%02X%02X", isakmphdr->i_ck[0],
                                                                              isakmphdr->i_ck[1],
                                                                              isakmphdr->i_ck[2],
                                                                              isakmphdr->i_ck[3],
                                                                              isakmphdr->i_ck[4],
                                                                              isakmphdr->i_ck[5],
                                                                              isakmphdr->i_ck[6],
                                                                              isakmphdr->i_ck[7]);
                snprintf(info->info2, 20, "%02X%02X%02X%02X%02X%02X%02X%02X", isakmphdr->r_ck[0],
                                                                              isakmphdr->r_ck[1],
                                                                              isakmphdr->r_ck[2],
                                                                              isakmphdr->r_ck[3],
                                                                              isakmphdr->r_ck[4],
                                                                              isakmphdr->r_ck[5],
                                                                              isakmphdr->r_ck[6],
                                                                              isakmphdr->r_ck[7]);
                return true;
            } else {
                info->packetEncType = PacketEncryptionTypeEspUdp;
                info->encType = EncryptionTypeEsp;
                struct newesp *esp = (struct newesp *)(udph + 1);

                snprintf(info->info1, 20, "%08X", EXTRACT_32BITS(&esp->esp_spi));
                PKMM_LOGD("ExtractInfo: ESP UDP SPI: %s", info->info1);
            }
        }
        else {
            // PKMM_LOGD("ExtractInfo: UDP with other port: dst_port = %u, src_port = %u", dst_port,
            //                                                                           src_port);
        }
    }
    else {
        // PKMM_LOGD("extractInfo: Other protocol");
    }

    return true;
}

/***************************************************************************************************
 * @Function mapNetwork
 * @brief Map interface, SIM and network type
 * @note
 **************************************************************************************************/
bool PacketMonitor::mapNetwork(char *nwif, int *sim, bool *isVoLTE) {

    *isVoLTE = false;
    if(mapImsNetwork(nwif, sim)) {
        *isVoLTE = true;
        return true;
    }
    else {
        return mapNonImsNetwork(nwif, sim);
    }
}

/***************************************************************************************************
 * @Function mapNonImsNetworkInternal
 * @brief Map interface, SIM and network type
 * @note
 **************************************************************************************************/
bool PacketMonitor::mapNonImsNetworkInternal(char *line, int ifid) {

    int count = 0;
    const char *delim2 = "_";
    char *token = strtok(line, delim2);
    while (token != NULL) {
        switch (count) {
            case PARAM_NON_IMS_PDN_TYPE: {
                break;
            }
            case PARAM_NON_IMS_INTF_ID: {
                if(ifid == atoi(token)) {
                    return true;
                }
                break;
            }
            default: {
                PKM_LOGE("mapNonImsNetworkInternal, invalid param %d", count);
                break;
            }
        }
        token = strtok (NULL, delim2);
        count++;
    }

    return false;
}

/***************************************************************************************************
 * @Function mapImsNetworkInternal
 * @brief Map interface, SIM and network type
 * @note
 **************************************************************************************************/
bool PacketMonitor::mapImsNetworkInternal(char *line, int ifid, int *sim) {

    int count = 0;
    int simId = 0;
    bool volte = false;
    const char *delim2 = "_";
    char *token = strtok(line, delim2);
    while (token != NULL) {
        switch (count) {
            case PARAM_PDN_TYPE: {
                if (strstr(token, IMS_APN) != NULL) {
                    volte = true;
                }
                break;
            }
            case PARAM_SIM_ID: {
                simId = atoi(token);
                if (simId >= MAX_SIM_SIZE) {
                    return false;
                }
                break;
            }
            case PARAM_INTF_ID: {
                if(ifid == atoi(token)) {
                    if(volte) {
                        *sim = simId;
                        return true;
                    }
                }

                break;
            }
            default: {
                PKM_LOGE("mapNetworkInternal, invalid param %d", count);
                break;
            }
        }
        token = strtok (NULL, delim2);
        count++;
    }

    return false;
}

/***************************************************************************************************
 * @Function mapImsNetwork
 * @brief Map interface, SIM and network type
 * @note
 **************************************************************************************************/
bool PacketMonitor::mapImsNetwork(char *nwif, int *sim) {
    if(mIsMdLegacy) {
        PKMM_LOGD("mapImsNetwork: Legacy Platform");
        *sim = 0;
        return (!strcmp(nwif, "ccmni4") || !strcmp(nwif, "ccmni5") || !strcmp(nwif, "ccmni6"));
    }
    else {
        char buffer[PROP_VALUE_MAX] = {'\0'};
        property_get(PROP_IMS_PDN_INFO, buffer, "");

        PKMM_LOGD("mapImsNetwork: %s", buffer);

        if (strlen(buffer) <=1 ) {
            return false;
        }

        const char *delim1 = ",";
        int interfaceId = atoi(nwif + 5);
        char *token = strtok(buffer, delim1);
        char *next = NULL;
        char *tail = NULL;

        while (token != NULL) {
            next = strtok(NULL, delim1);
            if (next != NULL) {
                // 1 is for delim
                tail = buffer + strlen(token) + 1;
            } else {
                tail = NULL;
            }
            if (mapImsNetworkInternal(token, interfaceId, sim)) {
                return true;
            }
            token = strtok(tail, delim1);
        }
    }
    return false;
}

/***************************************************************************************************
 * @Function mapNonImsNetwork
 * @brief Map interface, SIM and network type
 * @note
 **************************************************************************************************/
bool PacketMonitor::mapNonImsNetwork(char *nwif, int *sim) {

    if(mIsMdLegacy) {
        PKMM_LOGD("mapImsNetwork: Legacy Platform");
        return true;
    }
    else {
        char propertyName[LEN_OF_PROP_NON_IMS_PDN_INFO_NAME] = {'\0'};
        char buffer[PROP_VALUE_MAX] = {'\0'};

        char *token = NULL;
        const char *delim = ",";
        int interfaceId = atoi(nwif + 5);

        for(int i = 0; i < MAX_SIM_COUNT; i++) {

            snprintf(propertyName, LEN_OF_PROP_NON_IMS_PDN_INFO_NAME, PROP_NON_IMS_PDN_INFO, i);
            property_get(propertyName, buffer, "");

            if (strlen(buffer) <=1 ) {
                continue;
            }

            PKMM_LOGD("mapNonImsNetwork: %s", buffer);
            token = strtok(buffer, delim);
            while (token != NULL) {
                if(mapNonImsNetworkInternal(token, interfaceId)) {
                    *sim = i;
                    return true;
                }

                token = strtok (NULL, delim);
            }

            memset(propertyName, 0, LEN_OF_PROP_NON_IMS_PDN_INFO_NAME);
            memset(buffer, 0, PROP_VALUE_MAX);
        }
    }

    return false;
}

/***************************************************************************************************
 * @Function isLegacy
 * @brief Check if MD is Legacy
 **************************************************************************************************/
bool PacketMonitor::isLegacy() {
    char buffer[PROP_VALUE_MAX] = {'\0'};
    property_get(PROP_AUTO_SETUP_IMS, buffer, "0");
    return !atoi(buffer);
}

/***************************************************************************************************
 * @Function RtpPort
 * @brief Constructor of RtpPort
 **************************************************************************************************/
RtpPorts::RtpPorts(int src, int dest) {
    mSrc = src;
    mDest = dest;
}

/***************************************************************************************************
 * @Function equals
 * @brief Compare function
 **************************************************************************************************/
bool RtpPorts::equals(int src, int dest) {
    return (src == mSrc && dest == mDest) || (src == mDest && dest == mSrc);
}
