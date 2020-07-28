/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2014. All rights reserved.
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

package com.mediatek.wfo.ril;

import android.os.Handler;
import android.os.Message;

public interface MwiCommandsInterface {

    // Actions Interfaces below

    void setWifiEnabled(String ifName,
            int isWifiEnabled, int isFlightModeOn, Message response);


    void setWifiAssociated(String ifName,
            boolean associated, String ssid, String apMac, int MtuSize, String ueMac, Message response);

    void setWfcConfig(int setting, String ifName,
            String value, Message response);

    void setWfcConfig_WifiUeMac(String ifName, String value, Message response);

    void setWifiSignalLevel(int rssi, int snr, Message response);


    void setWifiIpAddress(String ifName, String ipv4Addr, String ipv6Addr,
            int ipv4PrefixLen, int ipv6PrefixLen, String ipv4Gateway, String ipv6Gateway,
            int dnsCount, String dnsAddresses, Message response);

    void setLocationInfo(String accountId, String broadcastFlag,
            String latitude, String longitude, String accuracy, String method, String city,
            String state, String zip, String countryCode, String ueWlanMac, Message response);


    void setEmergencyAddressId(String aid, Message response);


    void setNattKeepAliveStatus(String ifName, boolean enable,
            String srcIp, int srcPort,
            String dstIp, int dstPort, Message response);

    void setWifiPingResult(int rat, int latency, int pktloss, Message response);

    void notifyEPDGScreenState(int state, Message result);

    // URC Listener / Registrant Interfaces below

    void registerRssiThresholdChanged(Handler h, int what, Object obj);

    void unregisterRssiThresholdChanged(Handler h);

    void registerWifiPdnActivated(Handler h, int what, Object obj);

    void unregisterWifiPdnActivate(Handler h);

    void registerWifiPdnError(Handler h, int what, Object obj);

    void unregisterWifiPdnError(Handler h);

    void registerWifiPdnHandover(Handler h, int what, Object obj);

    void unregisterWifiPdnHandover(Handler h);

    void registerWifiPdnRoveOut(Handler h, int what, Object obj);

    void unregisterWifiPdnRoveOut(Handler h);

    void registerRequestGeoLocation(Handler h, int what, Object obj);

    void unregisterRequestGeoLocation(Handler h);

    void registerWfcPdnStateChanged(Handler h, int what, Object obj);

    void unregisterWfcPdnStateChanged(Handler h);

    void registerWifiPingRequest(Handler h, int what, Object obj);

    void unregisterWifiPingRequest(Handler h);

    void registerWifiPdnOos(Handler h, int what, Object obj);

    void unregisterWifiPdnOos(Handler h);

    void registerWifiLock(Handler h, int what, Object obj);

    void unregisterWifiLock(Handler h);

    void registerNattKeepAliveChanged(Handler h, int what, Object obj);

    void unrgisterNattKeepAliveChanged(Handler h);
}


