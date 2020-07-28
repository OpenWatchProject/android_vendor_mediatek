/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2019. All rights reserved.
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

#ifndef __APM_KPI_OTA_PACKET_H__
#define __APM_KPI_OTA_PACKET_H__

#define PARAMETER_IGNORE_DECRYPTION     0x40000000
#define PARAMETER_NONE                  0x00000000

// Code Definition of packet source
#define SOURCE_UNKNOWN                          (0)
#define SOURCE_VOLTE                            (1)
#define SOURCE_NONVOLTE                         (2)
#define SOURCE_CELLUAR                          (3)
#define SOURCE_WIFI                             (4)

// Code Definition of packet type
#define PACKET_TYPE_UNKNOWN                     (0)
#define PACKET_TYPE_OTHERS                      (1)
#define PACKET_TYPE_IP                          (2)
#define PACKET_TYPE_DNS                         (3)
#define PACKET_TYPE_NATT_KEEP_ALIVE             (4)
#define PACKET_TYPE_SIP                         (5)
#define PACKET_TYPE_TCP                         (6)
#define PACKET_TYPE_ISAKMP                      (7)
#define PACEKT_TYPE_RTP                         (8)
#define PACKET_TYPE_UDP                         (17)
#define PACKET_TYPE_ESP                         (50)

// Code Definition of packet direction
#define DIRECTION_UNKNOW                        (0)
#define DIRECTION_DOWNSTREAM                    (1)
#define DIRECTION_UPSTREAM                      (2)

// Code Definition of packet encryption type
#define ENCRYPTION_NONE                         (0)
#define ENCRYPTION_ESP_1_TRANSPORT              (1)
#define ENCRYPTION_ESP_1_TURNNEL                (2)
#define ENCRYPTION_ESP_2                        (3)

// Code Definition of packet IP version
#define IP_VERSION_NONE                         (0)
#define IP_VERSION_V4                           (4)
#define IP_VERSION_V6                           (6)

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Packet data layout
// +-------------------------------------------------------+
// | size                                                  |
// +--------------+----------+-----------------------------+
// | ipHeaderSize |   ...    | appPacketSize               |
// |              |          +---------------------+-------+
// |              |          | appPacketHeaderSize |  ...  |
// +--------------+----------+---------------------+-------+
//                           ^
//                           | appPacketOffset
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma pack(push, 1)
    typedef struct {
        int32_t    version;
        int32_t    application;
        int8_t     encType;
        int8_t     source;
        int8_t     slot;
        int8_t     direction;
        int8_t     ipVersion;
        uint16_t   protocol;
        uint64_t   timestamp;
        int32_t    num; // For Debugging

        int32_t    packetSize;
        int32_t    ipHeaderSize;

        int32_t    appPacketSize;
        int32_t    appPacketOffset;
        int32_t    appPacketHeaderSize;

        // uint8_t*   packetRawData;
    } otaPacket_v1_t;
#pragma pack(pop)

#endif // __APM_KPI_OTA_PACKET_H__
