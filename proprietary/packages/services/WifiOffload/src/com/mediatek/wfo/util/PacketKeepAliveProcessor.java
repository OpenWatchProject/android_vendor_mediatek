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
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
package com.mediatek.wfo.util;

import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.Iterator;

import android.content.Intent;
import android.net.ConnectivityManager;
import android.net.ConnectivityManager.PacketKeepalive;
import android.net.ConnectivityManager.PacketKeepaliveCallback;
import android.net.Network;
import android.net.NetworkRequest;
import android.os.Handler;
import android.os.Message;
import android.os.SystemProperties;
import android.telephony.Rlog;
import android.text.TextUtils;

import com.mediatek.wfo.impl.WifiPdnHandler;

public class PacketKeepAliveProcessor {
    final static String TAG = "PacketKeepAliveProcessor";

    ConnectivityManager mConnectivityManager;
    Handler mWifiPdnHandler;
    ArrayList<KeepAliveInfo> mKeepAlives = new ArrayList<KeepAliveInfo> ();
    final static String PROPERTY_ENABLE_OFFLOAD = "persist.vendor.mtk_enable_keep_alive_offload";

    public class KeepAliveConfig {
        boolean enable;
        int interval;
        InetAddress srcIp;
        int srcPort;
        InetAddress dstIp;
        int dstPort;

        public KeepAliveConfig() {
            Rlog.d(TAG, "KeepAliveConfig default ctor");
            enable = false;
            interval = 0;
            srcIp = null;
            srcPort = 0;
            dstIp = null;
            dstPort = 0;
        }

        public String getSrcIp() {
            return srcIp.getHostAddress();
        }

        public int getSrcPort() {
            return srcPort;
        }

        public String getDstIp() {
            return dstIp.getHostAddress();
        }

        public int getDstPort() {
            return dstPort;
        }

        public KeepAliveConfig(boolean enable, int interval,
                String srcIp, int srcPort, String dstIp, int dstPort)
                throws UnknownHostException {
            this.enable = enable;
            this.interval = interval;
            this.srcIp = InetAddress.getByName(srcIp);
            this.srcPort = srcPort;
            this.dstIp = InetAddress.getByName(dstIp);
            this.dstPort = dstPort;
        }

        public boolean isEnabled() {
            return enable;
        }

        public boolean isEnabledAndAvailable() {
            return enable && interval >= 20
                && dstPort == PacketKeepalive.NATT_PORT; // NATT_PORT = 4500;
        }

        @Override
        public String toString() {
            return "enable: " + enable + " interval: " + interval
                + " srcIp: " + maskString(srcIp.getHostAddress()) + " srcPort: " + srcPort
                + " dstIp: " + maskString(dstIp.getHostAddress()) + " dstPort: " + dstPort;
        }

        @Override
        public boolean equals(Object o) {
            if (this == o)  return true;
            if (o == null)  return false;
            if (getClass() != o.getClass()) return false;
            KeepAliveConfig kac = (KeepAliveConfig) o;
            return interval == kac.interval
                && srcIp.equals(kac.srcIp)
                && srcPort == kac.srcPort
                && dstIp.equals(kac.dstIp)
                && dstPort == kac.dstPort;
        }
    }

    class WfcKeepAliveCallback extends PacketKeepaliveCallback{
        KeepAliveConfig config;
        boolean mIsStarted = false;

        public WfcKeepAliveCallback(KeepAliveConfig config) {
            this.config = config;
        }

        public boolean isStarted() {
            return mIsStarted;
        }

        @Override
        public void onStarted() {
            Rlog.d(TAG, "keepAlive is started!");
            mIsStarted = true;
            mWifiPdnHandler.sendMessage(mWifiPdnHandler.obtainMessage(
                        WifiPdnHandler.EVENT_SET_NATT_STATUS, 0, 0, config));
        }

        @Override
        public void onStopped() {
            Rlog.d(TAG, "keepAlive is stopped!");
            mIsStarted = false;
        }

        @Override
        public void onError(final int error) {
            Rlog.d(TAG, "keepAlive is failed, error: " + error
                    + ", " + errorToString(error));
            mIsStarted = false;
        }

        private String errorToString(int error) {
            String str = "unknown";
            switch(error) {
                case PacketKeepalive.ERROR_INVALID_NETWORK:
                    str = "ERROR_INVALID_NETWORK";
                    break;
                case PacketKeepalive.ERROR_INVALID_IP_ADDRESS:
                    str = "ERROR_INVALID_IP_ADDRESS";
                    break;
                case PacketKeepalive.ERROR_INVALID_PORT:
                    str = "ERROR_INVALID_PORT";
                    break;
                case PacketKeepalive.ERROR_INVALID_LENGTH:
                    str = "ERROR_INVALID_LENGTH";
                    break;
                case PacketKeepalive.ERROR_INVALID_INTERVAL:
                    str = "ERROR_INVALID_INTERVAL";
                    break;
                case PacketKeepalive.ERROR_HARDWARE_UNSUPPORTED:
                    str = "ERROR_HARDWARE_UNSUPPORTED";
                    break;
                case PacketKeepalive.ERROR_HARDWARE_ERROR:
                    str = "ERROR_HARDWARE_ERROR";
                    break;
            }
            return str;
        }
    }

    public class KeepAliveInfo {
        PacketKeepalive ka;
        KeepAliveConfig config;
        WfcKeepAliveCallback callback;

        public KeepAliveInfo(PacketKeepalive pka, KeepAliveConfig c, WfcKeepAliveCallback callback) {
            this.ka = pka;
            this.config = c;
            this.callback = callback;
        }

        public KeepAliveConfig getConfig() {
            return config;
        }

        public void stop() {
            Rlog.d(TAG, "KeepAliveInfo.stop, config: " + config
                    + ", isStarted: " + callback.isStarted());
            if (callback.isStarted()) {
                ka.stop();
            }
        }
    }

    public PacketKeepAliveProcessor(ConnectivityManager cm, Handler handler) {
        mConnectivityManager = cm;
        mWifiPdnHandler = handler;
    }

    public void notifyWifiDisconnect() {
        stopAllPacketKeepAlive();
    }

    public void handleKeepAliveChanged(String[] result) {
        if (!isWifiOffloadEnabledFromSystemProperty()) {
            Rlog.d(TAG, "mtk_enable_keep_alive_offload is disabled by System Property.");
            return;
        }

        Rlog.d(TAG, "handleKeepAliveChanged");
        KeepAliveConfig config = parseKeepAliveConfig(result);
        if (config == null) {
            Rlog.e(TAG, "KeepAliveConfig is null, return directly");
            return;
        }
        if (config.isEnabledAndAvailable()) {
            startPacketKeepAlive(config);
        } else {
            stopPacketKeepAlive(config);
        }
    }

    private boolean isWifiOffloadEnabledFromSystemProperty() {
        return "1".equals(SystemProperties.get(PROPERTY_ENABLE_OFFLOAD, "1"));
    }

    private String getStringFromArray(String[] result) {
        if (result == null) return "null";
        String ret = "";
        for (int i = 0 ; i < result.length ; i++) {
            ret += result[i] + " ";
        }
        return ret;
    }

    /**
     * If parsing is error, then return a default KeepAliveConfig.
     */
    private KeepAliveConfig parseKeepAliveConfig(String[] result) {
        if (null == result) {
            Rlog.d(TAG, "parseKeepAliveConfig() result is null");
            return new KeepAliveConfig();
        }

        KeepAliveConfig config = null;
        try {
            boolean enable = (Integer.parseInt(result[0]) == 1);
            int interval = Integer.parseInt(result[1]);
            String srcIp = result[2];
            int srcPort = Integer.parseInt(result[3]);
            String dstIp = result[4];
            int dstPort = Integer.parseInt(result[5]);
            config = new KeepAliveConfig(enable, interval,
                    srcIp, srcPort, dstIp, dstPort);
        } catch(Exception e) {
            Rlog.e(TAG, "parseKeepAliveConfig() exception: " + e.toString());
        }
        Rlog.d(TAG, "parseKeepAliveConfig() config: " + config);
        return config;
    }

    void startPacketKeepAlive(KeepAliveConfig config) {
        if (mConnectivityManager == null) {
            Rlog.e(TAG, "ConnectivityManager is null");
            return;
        }

        if (mKeepAlives.size() >= 3) {
            Rlog.e(TAG, "startNattKeepalive cannot start! Due to mKeepAlives.size() >=3, size: "
                    + mKeepAlives.size());
            return;
        }

        WfcKeepAliveCallback callback = new WfcKeepAliveCallback(config);
        Network network = mConnectivityManager.getActiveNetwork();
        if (isActiveWifiNetwork(network)) {
            PacketKeepalive ka = mConnectivityManager.startNattKeepalive(
                    network, config.interval, callback, config.srcIp, config.srcPort,
                    config.dstIp);
            KeepAliveInfo kai = new KeepAliveInfo(ka, config, callback);
            mKeepAlives.add(kai);
        } else {
            Rlog.e(TAG, "It's not a active wifi network, network: " + network);
            return;
        }
    }

    void stopPacketKeepAlive(KeepAliveConfig config) {
        Rlog.d(TAG, "stopPacketKeepAlive");
        if (mConnectivityManager == null) {
            Rlog.e(TAG, "ConnectivityManager is null");
            return;
        }
        KeepAliveInfo foundKai = findConfigInList(config);
        if (foundKai != null) {
            stopAndRemoveKeepAlive(foundKai);
        }
    }

    private boolean isActiveWifiNetwork(Network network) {
        return network != null
            && mConnectivityManager.getNetworkInfo(network)!= null
            && mConnectivityManager.getNetworkInfo(network).getType()
                    == ConnectivityManager.TYPE_WIFI;
    }

    void stopAllPacketKeepAlive() {
        Rlog.d(TAG, "stopAllPacketKeepAlive");
        Iterator<KeepAliveInfo> iter = mKeepAlives.iterator();
        while(iter.hasNext()) {
            KeepAliveInfo kai = iter.next();
            kai.stop();
            iter.remove();
        }
    }

    KeepAliveInfo findConfigInList(KeepAliveConfig config) {
        for(KeepAliveInfo kai : mKeepAlives) {
            if (kai.getConfig().equals(config)) {
                Rlog.d(TAG, "findConfigInList found config: " + config);
                return kai;
            }
        }
        return null;
    }

    void stopAndRemoveKeepAlive(KeepAliveInfo kai) {
        kai.stop();
        mKeepAlives.remove(kai);
    }

    private String maskString(String s) {
        StringBuilder sb = new StringBuilder();
        if (TextUtils.isEmpty(s)) {
            return s;
        }
        int maskLength = s.length()/2;
        if (maskLength < 1) {
            sb.append("*");
            return (sb.toString());
        }
        for(int i = 0; i < maskLength ; i++) {
            sb.append("*");
        }
        return (sb.toString() + s.substring(maskLength));
    }

}
