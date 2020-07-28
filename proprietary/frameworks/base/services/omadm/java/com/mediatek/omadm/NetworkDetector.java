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

package com.mediatek.omadm;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.mediatek.omadm.OmadmServiceImpl;

public class NetworkDetector {
    private final static String TAG = "NetworkDetector";

    private final static int NW_CONN_WIFI = 0;
    private final static int NW_CONN_ADMIN = 1;
    private final static int NW_DATA_ROAM = 2;
    private final OmadmServiceImpl mOmadmSrv;
    private TelephonyManager mTelephonyManager = null;
    private ConnectivityManager mConnectivityManager = null;
    private BroadcastReceiver mNetEventsReceiver = null;

    public NetworkDetector(Context context, OmadmServiceImpl omadmServiceImpl) {
        mOmadmSrv = omadmServiceImpl;
        mTelephonyManager = (TelephonyManager)context.getSystemService(Context.TELEPHONY_SERVICE);
        mConnectivityManager =
          (ConnectivityManager)context.getSystemService(Context.CONNECTIVITY_SERVICE);
    }

    private void defaultStateNotify() {
        if (mConnectivityManager == null) {
            Log.e(TAG, "defaultStateNotify(): NULL ConnectivityManager!!!");
            return;
        }

        NetworkInfo networkInfo = mConnectivityManager.getActiveNetworkInfo();
        Feature feature = Feature.getFeature(networkInfo, mTelephonyManager.getPhoneType());
        int features[] = {
                NW_CONN_WIFI, NW_CONN_ADMIN, NW_DATA_ROAM
        };

        for (int type : features) {
            if (feature != null && type == feature.type) {
                Log.d(TAG, "Notify omadm NM with type: "+feature.type+", status: "+feature.status);
                if (NW_CONN_ADMIN == feature.type) {
                    mOmadmSrv.notifyOmadmNetworkManager(NW_DATA_ROAM, networkInfo.isRoaming());
                }
                mOmadmSrv.notifyOmadmNetworkManager(feature.type, feature.status);
            } else {
                Log.d(TAG, "Notify omadm NM with type: " + type + ", status: false");
                mOmadmSrv.notifyOmadmNetworkManager(type, false);
            }
        }
    }

    private void networkChanged(NetworkInfo nwInfo) {
        Log.d(TAG, "networkChanged()");
        if (mTelephonyManager == null) {
            Log.e(TAG, "onReceive() NULL TelephonyManager !!!");
            return;
        }

        int type = nwInfo.getType();
        Log.d(TAG, "onReceive() - handle for type: " + type);
        Feature feature = Feature.getFeature(nwInfo, mTelephonyManager.getPhoneType());

        if (nwInfo != null && feature != null) {
            if (NW_CONN_ADMIN == feature.type) {
                mOmadmSrv.notifyOmadmNetworkManager(
                  NW_DATA_ROAM, mTelephonyManager.isNetworkRoaming());
            }
            mOmadmSrv.notifyOmadmNetworkManager(feature.type, feature.status);
        } else {
            Log.d(TAG, "onReceive() - nothing to handle for type: " + type);
        }
    }

    public void register(Context context) {
        defaultStateNotify();
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(ConnectivityManager.CONNECTIVITY_ACTION);
        mNetEventsReceiver = new NwEvntBroadcastReceiver();
        context.registerReceiver(mNetEventsReceiver, intentFilter);
    }

    public void unregister(Context context) {
        if (mNetEventsReceiver != null) {
            context.unregisterReceiver(mNetEventsReceiver);
            mNetEventsReceiver = null;
        }
    }

    private class NwEvntBroadcastReceiver extends BroadcastReceiver {

        @Override
        public void onReceive(Context context, Intent intent) {
            Log.d(TAG, "NwEvntBroadcastReceiver onReceive()");

            NetworkInfo networkInfo =
              (NetworkInfo) intent.getParcelableExtra(ConnectivityManager.EXTRA_NETWORK_INFO);
            networkChanged(networkInfo);
        }
    }

    private static class Feature {
        public int type;
        public boolean status;

        public Feature(int type, boolean status) {
            this.type = type;
            this.status = status;
        }

        public static Feature getFeature(NetworkInfo networkInfo, int phoneType) {
            if (null == networkInfo) {
                Log.d(TAG, "networkInfo is null");
                return null;
            }
            int type = networkInfo.getType();
            Log.d(TAG, "Phone type: " + phoneType + " Network type: " + type
                  + " NetworkInfo content: " + networkInfo.toString());
            switch (type) {
                case ConnectivityManager.TYPE_MOBILE_FOTA:
                    return new Feature(NW_CONN_ADMIN, networkInfo.isConnected());
                case ConnectivityManager.TYPE_MOBILE:
                    switch (phoneType) {
                        case TelephonyManager.PHONE_TYPE_CDMA:
                            // Do nothing for CDMA phone type like Verizon devices
                            break;
                        case TelephonyManager.PHONE_TYPE_GSM:
                            return new Feature(NW_CONN_ADMIN,
                                    networkInfo.isConnected());
                    }
                    break;
                case ConnectivityManager.TYPE_WIFI:
                    return new Feature(NW_CONN_WIFI, networkInfo.isConnected());
            }
            return null;
        }
    }
}
