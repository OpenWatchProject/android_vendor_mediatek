/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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
 * have been modified by MediaTek Inc. All revisions are subject to any receiver\'s
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.internal.telephony.cat;

public class BipUtils {
    final public static int MAX_APDU_SIZE = 237;

    final public static int MIN_CHANNEL_ID = 1;
    final public static int MAX_CHANNEL_ID = 1;

    final public static int MAX_CHANNELS_GPRS_ALLOWED = 3;
    final public static int MAX_CHANNELS_CSD_ALLOWED = 1;

    final public static int BEARER_TYPE_UNKNOWN = 0;
    final public static int BEARER_TYPE_CSD = 0x01;
    final public static int BEARER_TYPE_GPRS = 0x02;
    final public static int BEARER_TYPE_DEFAULT = 0x03;
    final public static int BEARER_TYPE_UTRAN = 0x09;
    final public static int BEARER_TYPE_EUTRAN = 0x0B;

    final public static int MIN_BUFFERSIZE_TCP = 255;
    final public static int MAX_BUFFERSIZE_TCP = 1400;
    final public static int DEFAULT_BUFFERSIZE_TCP = 1024;
    final public static int MIN_BUFFERSIZE_UDP = 255;
    final public static int MAX_BUFFERSIZE_UDP = 1400;
    final public static int DEFAULT_BUFFERSIZE_UDP = 1024;

    final public static int TRANSPORT_PROTOCOL_UNKNOWN = 0;
    final public static int TRANSPORT_PROTOCOL_UDP_REMOTE = 0x01;
    final public static int TRANSPORT_PROTOCOL_TCP_REMOTE = 0x02;
    final public static int TRANSPORT_PROTOCOL_SERVER = 0x03;
    final public static int TRANSPORT_PROTOCOL_UDP_LOCAL = 0x04;
    final public static int TRANSPORT_PROTOCOL_TCP_LOCAL = 0x05;

    final public static int ADDRESS_TYPE_UNKNOWN = 0;
    final public static int ADDRESS_TYPE_IPV4 = 0x21;
    final public static int ADDRESS_TYPE_IPV6 = 0x57;

    final public static int ADDRESS_IPV4_LENGTH = 4;
    final public static int ADDRRES_IPV6_LENGTH = 16;

    final public static int CHANNEL_STATUS_UNKNOWN = 0;
    final public static int CHANNEL_STATUS_ONDEMAND = 0x01;
    final public static int CHANNEL_STATUS_CLOSE = 0x02;
    final public static int CHANNEL_STATUS_SERVER_CLOSE = 0x03;
    final public static int CHANNEL_STATUS_OPEN = 0x04;
    final public static int CHANNEL_STATUS_LINK_DROPPED = 0x05; // NW failure or user cancellation
    final public static int CHANNEL_STATUS_TIMEOUT = 0x06;
    final public static int CHANNEL_STATUS_ERROR = 0x07;

    final public static byte TCP_STATUS_CLOSE = (byte) 0;
    final public static byte TCP_STATUS_LISTEN = (byte) 0x40;
    final public static byte TCP_STATUS_ESTABLISHED = (byte) 0x80;

    final public static int LINK_ESTABLISHMENT_MODE_IMMEDIATE = 0;
    final public static int LINK_ESTABLISHMENT_MODE_ONDEMMAND = 1;

    final public static int SEND_DATA_MODE_IMMEDIATE = 1;
    final public static int SEND_DATA_MODE_STORED = 0;

    final public static String KEY_QOS_CID = "cid";
    final public static String KEY_QOS_PRECEDENCE = "precedence";
    final public static String KEY_QOS_DELAY = "delay";
    final public static String KEY_QOS_RELIABILITY = "reliability";
    final public static String KEY_QOS_PEAK = "peak";
    final public static String KEY_QOS_MEAN = "mean";

    public static final int DATA_AVAILABLE_EVENT = 0x09;
    public static final int CHANNEL_STATUS_EVENT = 0x0A;
}
