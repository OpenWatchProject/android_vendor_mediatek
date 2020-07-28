/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2019. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

/***************************************************************************************************
 * @file PacketMonitor.h
 * @brief The header file defines the class of PacketMonitor
 **************************************************************************************************/
#ifndef __PKM_PACKET_MONITOR_H__
#define __PKM_PACKET_MONITOR_H__

#include "libpkm.h"
#include "SecurityAssociation.h"
#include "MdmAdapter.h"

#include <pthread.h>
#include <pcap.h>

#include <thread>
#include <mutex>
#include <queue>
#include <vector>
#include <unordered_map>
#include <libmdmonitor.h>

#include <ap_monitor.h>
#include <vendor/mediatek/hardware/apmonitor/2.0/IApmService.h>

#include "MdmAdapter.h"
#include "SecurityAssociation.h"

using namespace std;
using namespace libmdmonitor;
using com::mediatek::diagnostic::SecurityAssociation;
using com::mediatek::diagnostic::MdmReceiver;
using com::mediatek::diagnostic::MdmEventCallback;

namespace com {
    namespace mediatek {
        namespace diagnostic {

            typedef enum
            {
                PacketEncryptionTypeNone = -1,
                PacketEncryptionTypeIKev2Unencrypted,
                PacketEncryptionTypeIKev2,
                PacketEncryptionTypeEsp,
                PacketEncryptionTypeEspUdp,
                PacketEncryptionTypeNattKeepAlive,

            } PacketEncryptionType;

            typedef enum
            {
                SCENARIO_NONE = 0,              // Internet or VoLTE without encryption
                SCENARIO_ESP_1_TUNNEL = 1,      // VoWifi without encryption
                SCENARIO_ESP_1_TRANSPORT = 2,   // VoLTE with encryption
                SCENARIO_ESP_2 = 3,             // VoWifi with IPsec encryption (2 Layer encryption)
                SCENARIO_WIFI_NONE = 4,         // WiFI Internet Packet

            } SCENARIO;

            typedef enum
            {
                IPv4 = 4,
                IPv6 = 6
            } IP_VERSION;

            typedef enum
            {
                PROTOCOL_IPV6_HOPOPT = 0x00,
                PROTOCOL_TCP = 0x06,
                PROTOCOL_UDP = 0x11,
                PROTOCOL_ESP = 0x32,
                PROTOCOL_IP = 0x04,

            } PROTOCOL;

            typedef enum
            {
                INVALID = -1,
                UPSTREAM = 1,
                DOWNSTREAM,

            } DIRECTION;

            typedef enum
            {
                PORT_DNS = 53,
                PORT_ISAKMP = 500,
                PORT_ISAKMP_NATT = 4500,
                PORT_SIP = 5060,

            } PORT;

            typedef enum
            {
                UNICAST_TO_US = 0,
                BROADCAST = 1,
                MULTICAST = 2,
                SEND_BY_US = 4,

            } PKITTYPE;

            typedef struct {

                const uint8_t *packet;

                int offset;
                int ipLen;
                int len;
                int hdrLen;

                uint8_t version;
                uint16_t protocol;
                uint16_t espNext;

                int sim;
                int source;
                uint8_t direction;
                UInt64 timestamp;
                int count;

                PacketEncryptionType packetEncType;
                EncryptionType encType;
                SCENARIO scenario;

                char srcIp[64];
                char dstIp[64];

                char info1[MAX_SPI];
                char info2[MAX_SPI];
                int initiator;

            } IpPacketInfo;

            typedef struct {
                bool decrypted;

                UInt8 *packet;
                int size;
                int hdr_size;
                int count;
                int src;
                int sim;
                int direction;
                UInt64 timestamp;

                EncryptionType encType;
                char info1[MAX_SPI];
                char info2[MAX_SPI];

            } PendingPacket;

            class Message {
                public:
                    Message();
                    virtual ~Message();
                    int what;
                    int event;
                    void *data;
            };

            class RtpPorts {
                public:
                    RtpPorts(int src, int dest);
                    virtual ~RtpPorts() {};
                    bool equals(int src, int dest);
                private:
                    int mSrc;
                    int mDest;
            };

            class PacketMonitor : public MdmEventCallback {
                public:
                    PacketMonitor();
                    virtual ~PacketMonitor();

                    // PacketMonitor Lifecycle
                    static PacketMonitor * create();
                    int start();
                    void stop();

                    void onLtecsrRtpEvent(uint8_t event, uint16_t src, uint16_t dest);
                    void onAddKeyEvent(EncryptionType type, char *spi1, char *spi2);

                private:

                    // PacketMonitor Lifecycle
                    bool mIsRunning;   // Flag of Running
                    bool mIsStopping;  // Flag of Stopping
                    thread *mMainThread;
                    queue<Message *> mQueue;
                    mutex mMainMutex;
                    mutex mLifecycleMutex;
                    condition_variable mCv;
                    pcap_t * mHandle;
                    int mCount;

                    // MDM Receiver & SA
                    SecurityAssociation *mSa;
                    MdmReceiver *mMdm;

                    // RTP List
                    mutex mMutexRtp;
                    vector <RtpPorts *> mRtpList;

                    Int64 mParam;

                    // Pending Packets
                    vector<PendingPacket *> mPendingList;
                    mutex mMutexPendingQueue;

                    // MD Type
                    bool mIsMdLegacy;

                private:

                    void sendMessage(Message *message);

                    // Packet Monitor Lifecycle
                    static void runMainThread(void *data);
                    void looper();
                    void handleMessage(Message *message);

                    static void *runQueryNwPackets(void *arg);
                    static void nwHandler(u_char *user, const u_char* header, const u_char *packet);
                    void packetsReceiver(const u_char* mtk_header, const u_char *packet);
                    void packetsHandler(const u_char *packet, int size, int hdr_size, int count,
                                        int src, int sim, int direction, UInt64 timestamp,
                                        bool decrypted, bool clone);

                    void setCapability();

                    bool isRtpPort(int src, int dest);
                    bool extractInfo(IpPacketInfo *info);
                    void addPendingQueue(IpPacketInfo *info, bool decrypted);
                    void sendPacket(const u_char *packet, int size, int ipHeaderLen,
                                    int src, int sim, int direction, int version, UInt16 protocol,
                                    int type, int encType, APM_MSGID id,
                                    Int64 timestamp, Int32 num,
                                    int appPacketOffset, int appPacketSize, int appPacketHeaderLen);

                    int decryptPacket(IpPacketInfo *info);
                    void handleKeyAddEvent(void *data);

                    bool mapNetwork(char *nwif, int *sim, bool *isVoLTE);
                    bool mapImsNetwork(char *nwif, int *sim);
                    bool mapImsNetworkInternal(char *line, int ifid, int *sim);
                    bool mapNonImsNetwork(char *nwif, int *sim);
                    bool mapNonImsNetworkInternal(char *line, int ifid);
                    bool isLegacy();
            };
        }
    }
}

#endif
