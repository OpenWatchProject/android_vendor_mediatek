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
 * MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.connectivity;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.net.ConnectivityManager.NetworkCallback;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.NetworkRequest;
import android.os.Binder;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.telephony.TelephonyManager;
import android.util.Log;


/**
  *
  * Android service for multi-PDP functoin.
  * Design for keeping PDP connection status in background.
  *
 **/
public class CdsPdpService extends Service {
    private static final String TAG = "CDSINFO/PDP_SRV";

    private static final int MAX_APN_TYPE = 4;

    private static final int EVENT_REQUEST_NETWORK = 1;
    private static final int EVENT_STOP_NETWORK = 2;
    private static final int EVENT_CHECK_NETWORK = 3;

    private static final int[] APN_CAP_LIST = new int[] {
        NetworkCapabilities.NET_CAPABILITY_INTERNET,
        NetworkCapabilities.NET_CAPABILITY_MMS,
        NetworkCapabilities.NET_CAPABILITY_SUPL,
        NetworkCapabilities.NET_CAPABILITY_DUN,
        NetworkCapabilities.NET_CAPABILITY_IMS};

    private static ConnectivityManager sConnMgr;
    private TelephonyManager mTelephonyManager;

    private Context mContext;
    private TestNetworkRequest mNetworkRequests[];

    private static Handler sNetworkHandler;
    public ServiceBinder mBinder = new ServiceBinder();

    /**
      *
      * Utiltiy Class for multiple network request.
      *
      */
    private static class TestNetworkRequest {
        NetworkRequest  mNetworkRequest;
        Network         mCurrentNetwork;
        boolean         mIsRequested;

        NetworkCallback mNetworkCallback = new NetworkCallback() {
            @Override
            public void onAvailable(Network network) {
                mCurrentNetwork = network;
                mIsRequested = true;
                Log.d(TAG, "onAvailable:" + network);
            }

            @Override
            public void onLost(Network network) {
                if (network.equals(mCurrentNetwork)) {
                        sConnMgr.unregisterNetworkCallback(mNetworkCallback);
                        mIsRequested = false;
                        mCurrentNetwork = null;
                }
                Log.d(TAG, "onLost:" + network);
            };
        };
    }

    @Override
    public void onCreate() {
        super.onCreate();

        mContext = this.getApplicationContext();

        sConnMgr = (ConnectivityManager) mContext.getSystemService(
                                            Context.CONNECTIVITY_SERVICE);
        mTelephonyManager = (TelephonyManager) mContext.getSystemService(
                                Context.TELEPHONY_SERVICE);

        createNetworkRequest(MAX_APN_TYPE);

        HandlerThread networkThread = new HandlerThread(TAG);
        networkThread.start();
        sNetworkHandler = new NetworkHandler(networkThread.getLooper());

        Log.i(TAG, "CdsPdpActivity is started");
    }

    @Override
    public IBinder onBind(Intent intent) {
        return mBinder;
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        return Service.START_NOT_STICKY;
    }

    /** Handler to do the network accesses on. */
    private class NetworkHandler extends Handler {

        public NetworkHandler(Looper l) {
            super(l);
        }

        @Override
        public void handleMessage(Message msg) {
            int i = msg.arg1;
            switch (msg.what) {
                case EVENT_REQUEST_NETWORK:

                    sConnMgr.requestNetwork(mNetworkRequests[i].mNetworkRequest,
                        mNetworkRequests[i].mNetworkCallback);
                    mNetworkRequests[i].mIsRequested = true;
                    CdsPdpActivity.sHandler.sendMessage(
                        CdsPdpActivity.sHandler.obtainMessage(CdsPdpActivity.MSG_UI_UPDATE));
                    if (i == 0) {
                        setMobileDataEnabled(true);
                    }
                    break;
                case EVENT_STOP_NETWORK:
                    try {
                        sConnMgr.unregisterNetworkCallback(
                            mNetworkRequests[i].mNetworkCallback);
                    } catch (IllegalArgumentException e) {
                        Log.e(TAG, "e:" + e);
                    }
                    mNetworkRequests[i].mIsRequested = false;
                    mNetworkRequests[i].mCurrentNetwork = null;
                    CdsPdpActivity.sHandler.sendMessage(
                        CdsPdpActivity.sHandler.obtainMessage(CdsPdpActivity.MSG_UI_UPDATE));
                    if (i == 0) {
                        setMobileDataEnabled(false);
                    }
                    break;
                case EVENT_CHECK_NETWORK:
                    break;
                default:
                    break;
            }
        }
    }

    private void createNetworkRequest(int count) {
        mNetworkRequests = new TestNetworkRequest[count + 1];

        for (int i = 0; i < count; i++) {
            mNetworkRequests[i] = new TestNetworkRequest();
            mNetworkRequests[i].mNetworkRequest = new NetworkRequest.Builder().
                            addCapability(APN_CAP_LIST[i]).
                            addTransportType(NetworkCapabilities.TRANSPORT_CELLULAR).
                            build();
            mNetworkRequests[i].mIsRequested = false;

        }
        updateNetworkInfo();
    }

    private void updateNetworkInfo() {
        Network[] networks = sConnMgr.getAllNetworks();
        for (int i = 0; i < networks.length; i ++) {
            syncNetworkStatus(sConnMgr.getNetworkCapabilities(networks[i]), networks[i]);
        }
    }

    private void syncNetworkStatus(NetworkCapabilities nwCap, Network network) {
        for (int i = 0; i < MAX_APN_TYPE; i++) {
            NetworkCapabilities nc =  mNetworkRequests[i].mNetworkRequest.networkCapabilities;
            Log.i(TAG, "cap1:" + nwCap);
            Log.i(TAG, "cap2:" + nc);
            if (nc.satisfiedByNetworkCapabilities(nwCap)) {
                mNetworkRequests[i].mCurrentNetwork = network;
                mNetworkRequests[i].mIsRequested = true;
            }
        }
    }

    private void setMobileDataEnabled(boolean enabled) {
        Log.i(TAG, "setMobileDataEnabled");
        mTelephonyManager.setDataEnabled(enabled);
    }

    /**
      * This is an IPC communication binder between activity and service.
      * Provide service APIs for activity to use.
      *
    */
    public class ServiceBinder extends Binder {

        /**
         *
         * API to setup PDP connection.
         * @param i indicates which PDP to setup.
         * @throws InterruptedException if Interrupted exception is occurred.
         *
         */
        public void startNetworkRequest(int i) throws InterruptedException {
            if (!mNetworkRequests[i].mIsRequested) {
                sNetworkHandler.obtainMessage(EVENT_REQUEST_NETWORK, i , 0).sendToTarget();
            } else {
                Log.e(TAG, "Ingore");
            }
        }

       /**
         *
         * API to teardown PDP connection.
         * @param i indicates which PDP to teardown.
         * @throws InterruptedException if Interrupted exception is occurred.
         *
         */
        public void stopNetworkRequest(int i) throws InterruptedException {
            if (mNetworkRequests[i].mIsRequested) {
                sNetworkHandler.obtainMessage(EVENT_STOP_NETWORK, i, 0).sendToTarget();
            } else {
                Log.e(TAG, "Ingore");
            }
        }

       /**
         *
         * API to check PDP connection.
         * @param i indicates which PDP to check.
         * @return indicate the PDP is active or not.
         * @throws InterruptedException if Interrupted exception is occurred.
         *
         */
        public boolean checkNetworkReqeust(int i) throws InterruptedException {
            if (mNetworkRequests[i].mIsRequested) {
                return true;
            } else if (mNetworkRequests[i].mCurrentNetwork != null) {
                return true;
            }
            return false;
        }

       /**
         *
         * API to return Network object.
         * @param i indicates which PDP to use.
         * @return indicate the PDP is active or not.
         * @throws InterruptedException if Interrupted exception is occurred.
         *
         */
        public Network getNetwork(int i) throws InterruptedException {
            return mNetworkRequests[i].mCurrentNetwork;
        }

    }
}