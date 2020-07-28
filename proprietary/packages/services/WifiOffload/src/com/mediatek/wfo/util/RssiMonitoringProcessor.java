/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2015. All rights reserved.
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

import android.net.ConnectivityManager;
import android.net.NetworkRequest;
import android.util.Log;

public class RssiMonitoringProcessor {
    class RssiMonitorRequest {
        NetworkRequest mRequest;
        ConnectivityManager.NetworkCallback mCallback;

        RssiMonitorRequest(NetworkRequest request,
                ConnectivityManager.NetworkCallback callback) {
            mRequest = request;
            mCallback = callback;
        }
    }

    /**
     * For Rssi Monotiring, array index is SIM id, each SIM has its own monitoring list.
     */
    private RssiMonitorRequest mRssiMonitoringList[][];
    int mSimCount;
    ConnectivityManager mConnectivityManager;

    final static String TAG = "RssiMonitoringProcessor";

    public RssiMonitoringProcessor (ConnectivityManager connectivityManager) {
        mConnectivityManager = connectivityManager;
    }

    public void initialize(int simCount) {
        mSimCount = simCount;
        mRssiMonitoringList = new RssiMonitorRequest[simCount][];
    }

    /**
    * callback from MAL for RDS to configure RSSI monitor thresholds.
    */
    public void registerRssiMonitoring(int simId, int size, int[] rssiThresholds) {
        if (checkInvalidSimIdx(simId, "onRssiMonitorRequest: invalid SIM id")) return;

        Log.d(TAG, "onRssiMonitorRequest simId: " + simId
                + " size: " + size + " rssiThresholds: " + printIntArray(rssiThresholds));

        if (mConnectivityManager == null) {
            Log.d(TAG, "onRssiMonitorRequest: mConnectivityManager is null");
            return;
        }

        unregisterAllRssiMonitoring(simId);

        RssiMonitorRequest newList[] = new RssiMonitorRequest[size];
        for (int i = 0; i < size; i++) {
            // make an new request & callback.
            NetworkRequest request = new NetworkRequest.Builder()
                    .setSignalStrength(rssiThresholds[i]).build();
            ConnectivityManager.NetworkCallback callback =
                    new ConnectivityManager.NetworkCallback();

            // setup the new request & callback.
            mConnectivityManager.registerNetworkCallback(request, callback);
            Log.d(TAG, "onRssiMonitorRequest registerNetworkCallback with rssi: "
                    + rssiThresholds[i]);
            newList[i] = new RssiMonitorRequest(request, callback);
        }
        mRssiMonitoringList[simId] = newList;
    }

    private String printIntArray(int[] input) {
        if (input == null) return "input is null";
        String s = new String();
        for (int i = 0 ; i < input.length ; i++) {
            if (i == 0) s = "" + input[i];
            else s += "," + input[i];
        }
        return s;
    }

    public void unregisterAllRssiMonitoring(int simId) {
        Log.d(TAG, "unregisterAllRssiMonitoring(), simId:" + simId);

        if (mRssiMonitoringList[simId] != null) {
            for (int i = 0; i < mRssiMonitoringList[simId].length; i++) {
                mConnectivityManager.unregisterNetworkCallback(
                            mRssiMonitoringList[simId][i].mCallback);
            }
            mRssiMonitoringList[simId] = null;
        }
    }

    private boolean checkInvalidSimIdx(int simIdx, String dbgMsg) {
        if (simIdx < 0 || simIdx >= mSimCount) {
            Log.d(TAG, dbgMsg);
            return true;
        }
        return false;
    }
}
