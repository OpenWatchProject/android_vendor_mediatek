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

package com.mediatek.presence.utils;


import android.content.Context;
import android.telephony.PhoneStateListener;
import android.telephony.TelephonyManager;
import android.telephony.SubscriptionManager;
import android.net.ConnectivityManager;
import android.net.Network;
import android.net.NetworkRequest;
import android.net.NetworkCapabilities;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.Message;

import com.mediatek.presence.core.ims.rcsua.RcsUaAdapter;
import com.mediatek.presence.platform.AndroidFactory;
import com.mediatek.presence.utils.logger.Logger;

import java.util.Vector;

public class PhoneStateMonitor {

    private PhoneStateListener mPhoneStateListener = null;
    private ConnectivityManager mConnectivityManager = null;
    private TelephonyManager mTelephonyManager = null;


    private Logger logger = Logger.getLogger(this.getClass().getName());


    private Vector<NetowrkStateListener> mNetworkStatelisteners = new Vector<NetowrkStateListener>();
    public static interface NetowrkStateListener{
        public void onNetworkStateChanged(int networkType);
    }

    private Vector<MobileNetowrkListener> mMobileNetworklisteners = new Vector<MobileNetowrkListener>();
    public static interface MobileNetowrkListener{
        public static final int MOBILE_NETWORK_OFF = 0;
        public static final int MOBILE_NETWORK_ON = 1;
        public void onMobileNetworkChanged(int state);
    }

    public PhoneStateMonitor(int slotId) {
        int[] subIds = SubscriptionManager.getSubId(slotId);
        if (subIds == null) {
            logger.error("PhoneStateMonitor init fail. subIds is null");
            return;
        }
        TelephonyManager mTelephonyManager = TelephonyManager.from(AndroidFactory
                         .getApplicationContext()).createForSubscriptionId(subIds[0]);
        mConnectivityManager = (ConnectivityManager) AndroidFactory
                        .getApplicationContext().getSystemService(Context.CONNECTIVITY_SERVICE);

        startMobileNetworkMonitor(subIds[0]);
        startNetworkStateMonitor();
    }

    public void addNetworkStateListener(NetowrkStateListener listener) {
        if (listener == null) {
            logger.error("network listener is null");
            return;
        }
        mNetworkStatelisteners.add(listener);
    }

    public void removeNetworkStateListener(NetowrkStateListener listener){
        if (listener == null) {
            logger.error("network listener is null");
            return;
        }
        mNetworkStatelisteners.remove(listener);
    }
    private void startNetworkStateMonitor() {
          mPhoneStateListener = new PhoneStateListener() {
            @Override
            public void onDataConnectionStateChanged(int state, int networkType) {
                 for(int i=0; i < mNetworkStatelisteners.size(); i++) {
                    NetowrkStateListener listener = (NetowrkStateListener)mNetworkStatelisteners.elementAt(i);
                    listener.onNetworkStateChanged(networkType);
                }
            }
        };
        logger.error("register listener to network framework");
        mTelephonyManager.listen(mPhoneStateListener,
                PhoneStateListener.LISTEN_DATA_CONNECTION_STATE);
    }

    private void stopNetworkStateMonitor() {
        logger.error("deregister listener from network framework");
        mTelephonyManager.listen(mPhoneStateListener,
            PhoneStateListener.LISTEN_NONE);
        mPhoneStateListener = null;
    }

    public void addMobileNetworkListener(MobileNetowrkListener listener) {
        if (listener == null) {
            logger.error("mobile listener is null");
            return;
        }
        mMobileNetworklisteners.add(listener);
    }

    public void removeMobileNetworkListener(MobileNetowrkListener listener) {
        if (listener == null) {
            logger.error("mobile listener is null");
            return;
        }
        mMobileNetworklisteners.remove(listener);
    }

    private void startMobileNetworkMonitor(int subId) {
        NetworkRequest networkRequest = new NetworkRequest.Builder()
                    .addTransportType(NetworkCapabilities.TRANSPORT_CELLULAR)
                    .addCapability(NetworkCapabilities.NET_CAPABILITY_INTERNET)
                    .setNetworkSpecifier(String.valueOf(subId))
                    .build();
        mConnectivityManager.registerNetworkCallback(networkRequest, networkCallback);
    }

    private void stopMobileNetworkMonitor() {
        mConnectivityManager.unregisterNetworkCallback(networkCallback);
    }

    private final ConnectivityManager.NetworkCallback networkCallback = new ConnectivityManager.NetworkCallback() {
        @Override
        public void onAvailable(Network network) {
            logger.debug("onAvailable");
            for(int i=0; i < mMobileNetworklisteners.size(); i++) {
                MobileNetowrkListener listener = (MobileNetowrkListener)mMobileNetworklisteners.elementAt(i);
                listener.onMobileNetworkChanged(MobileNetowrkListener.MOBILE_NETWORK_ON);
            }
        }

        @Override
        public void onLost(Network network) {
            logger.debug("onLost");
            for(int i=0; i < mMobileNetworklisteners.size(); i++) {
                MobileNetowrkListener listener = (MobileNetowrkListener)mMobileNetworklisteners.elementAt(i);
                listener.onMobileNetworkChanged(MobileNetowrkListener.MOBILE_NETWORK_OFF);
            }
        }
    };

    @Override
    public void finalize() {
        logger.debug("finalize");
        stopNetworkStateMonitor();
        stopMobileNetworkMonitor();
    }

}

