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

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.NetworkInfo;
import android.net.NetworkRequest;
import android.os.SystemClock;
import android.os.IBinder;
import android.util.Log;

import com.mediatek.omadm.FotaException;
import com.mediatek.omadm.OmadmServiceImpl;
import com.mediatek.omadm.PalConstDefs;

public class FotaNetworkManager {
    private static final String TAG = "FotaNetworkManager";
    private final Context mContext;
    private Network mNwk;
    private int mFotaReqCnt;
    private final NetworkRequest mNwkReq;
    private ConnectivityManager.NetworkCallback mNwkClbk;
    private volatile ConnectivityManager mConnectivityManager;

    private static final int NETWORK_REQUEST_TIMEOUT_MILLIS = 15 * 1000;
    private static final int NETWORK_ACQUIRE_TIMEOUT_MILLIS = NETWORK_REQUEST_TIMEOUT_MILLIS + 5000;
    private final int mSubId;

    OmadmServiceImpl mOmadmSrv = null;

    private class NetworkReqClbk extends ConnectivityManager.NetworkCallback {
        @Override
        public void onAvailable(Network network) {
            super.onAvailable(network);
            Log.i(TAG, "NetworkCallbackListener.onAvailable: network=" + network);
            synchronized (FotaNetworkManager.this) {
                mNwk = network;
                FotaNetworkManager.this.notifyAll();
            }
        }

        @Override
        public void onLost(Network network) {
            super.onLost(network);
            Log.w(TAG, "NetworkCallbackListener.onLost: network=" + network);
            synchronized (FotaNetworkManager.this) {
                releaseReqLock(this);
                FotaNetworkManager.this.notifyAll();
                if (mOmadmSrv != null) {
                    mOmadmSrv.omadmControllerDispachAdminNetStatus(PalConstDefs.ADMIN_NET_LOST, 0);
                }
                mNwk = null;
            }
        }

        @Override
        public void onUnavailable() {
            super.onUnavailable();
            Log.w(TAG, "NetworkCallbackListener.onUnavailable");
            synchronized (FotaNetworkManager.this) {
                releaseReqLock(this);
                FotaNetworkManager.this.notifyAll();
            }
        }
    }


    private void startNwkReqLock() {
        final ConnectivityManager connectivityManager = getConnectivityManager();
        mNwkClbk = new NetworkReqClbk();
        connectivityManager.requestNetwork(mNwkReq, mNwkClbk, NETWORK_REQUEST_TIMEOUT_MILLIS);
    }

    private void releaseReqLock(ConnectivityManager.NetworkCallback callback) {
        if (callback != null) {
            final ConnectivityManager connectivityManager = getConnectivityManager();
            try {
                connectivityManager.unregisterNetworkCallback(callback);
            } catch (IllegalArgumentException e) {
                Log.w(TAG, "Unregister network callback exception", e);
            }
        }
        resetLocked();
    }

    private void resetLocked() {
        mNwkClbk = null;
        mNwk = null;
        mFotaReqCnt = 0;
    }

    private ConnectivityManager getConnectivityManager() {
        if (mConnectivityManager == null) {
            mConnectivityManager =
              (ConnectivityManager) mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
        }
        return mConnectivityManager;
    }

    public FotaNetworkManager(OmadmServiceImpl service, Context context, int subId) {
        mContext = context;
        mNwkClbk = null;
        mNwk = null;
        mFotaReqCnt = 0;
        mConnectivityManager = null;
        mSubId = subId;
        mOmadmSrv = service;
        mNwkReq = new NetworkRequest.Builder()
                .addTransportType(NetworkCapabilities.TRANSPORT_CELLULAR)
                .addCapability(NetworkCapabilities.NET_CAPABILITY_FOTA)
                .setNetworkSpecifier(Integer.toString(mSubId))
                .build();
    }

    public int acquireNetwork(final String callerID) throws FotaException {
        synchronized (this) {
            mFotaReqCnt += 1;
            if (mNwk != null) {
                Log.d(callerID, "FotaNetworkManager: already available");
                return mNwk.netId;
            }

            if (mNwkClbk == null) {
                Log.d(callerID, "FotaNetworkManager: start new network request");
                startNwkReqLock();
            }

            final long shouldEnd = SystemClock.elapsedRealtime() + NETWORK_ACQUIRE_TIMEOUT_MILLIS;
            long waitTime = NETWORK_ACQUIRE_TIMEOUT_MILLIS;

            while (waitTime > 0) {
                try {
                    this.wait(waitTime);
                } catch (InterruptedException e) {
                    Log.w(callerID, "FotaNetworkManager: acquire network wait interrupted");
                }

                if (mNwk != null) {
                    return mNwk.netId;
                }

                waitTime = shouldEnd - SystemClock.elapsedRealtime();
            }

            Log.e(callerID, "FotaNetworkManager: timed out");
            releaseReqLock(mNwkClbk);

            throw new FotaException("Acquiring network timed out");
        }
    }

    public boolean releaseNetwork(final String callerID) {
        synchronized (this) {
            if (mFotaReqCnt > 0) {
                mFotaReqCnt -= 1;
                Log.d(callerID, "FotaNetworkManager: release, count=" + mFotaReqCnt);
                if (mFotaReqCnt < 1) {
                    Log.d(callerID, "FotaNetworkManager: Release FOTA Network");
                    releaseReqLock(mNwkClbk);
                    return true;
                }
            }
            return false;
        }
    }
    public String getApnName() {
        Network network = null;
        synchronized (this) {
            if (mNwk == null) {
                return null;
            }
            network = mNwk;
        }
        String apnName = null;
        final ConnectivityManager connectivityManager = getConnectivityManager();
        final NetworkInfo FotaNetworkInfo = connectivityManager.getNetworkInfo(network);
        if (FotaNetworkInfo != null) {
            apnName = FotaNetworkInfo.getExtraInfo();
        }
        return apnName;
    }
}
