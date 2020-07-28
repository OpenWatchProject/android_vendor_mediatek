/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.gba;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.ConnectivityManager;
import android.net.Network;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.telephony.SubscriptionManager;
import android.util.Log;



import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.TelephonyIntents;

import com.mediatek.gba.cache.GbaKeysCache;
import com.mediatek.gba.element.NafId;
import com.mediatek.gba.telephony.TelephonyUtils;

/**
 * implementation for GbaService.
 *
 * @hide
 */
public class GbaService extends Service {
    public static final String TAG = "GbaService";

    private Context mContext;
    private GbaKeysCache mGbaKeysCache = null;
    private int mGbaType = GbaConstant.GBA_NONE;

    private static final int EVENT_SIM_STATE_CHANGED = 0;

    @Override
    public void onCreate() {
        super.onCreate();
        Log.d(TAG, "GbaService: onCreate");
        ServiceManager.addService(TAG, mBinder);

        mContext = this.getBaseContext();

        if (mGbaKeysCache == null) {
            mGbaKeysCache = new GbaKeysCache();
        }

        Log.d(TAG, "Add service for GbaService");
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        if (intent == null) {
            return Service.START_STICKY;
        }

        Log.d(TAG, "GbaService: onStartCommand");
        String action = intent.getAction();
        Log.d(TAG, "Service starting for intent " + action);

        return Service.START_STICKY;
    }

    @Override
    public IBinder onBind(Intent intent) {
        Log.d(TAG, "GbaService: onBind");
        return mBinder;    // clients can't bind to this service
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "Service is onDestroy");
    }


    private IBinder mBinder = new IGbaService.Stub() {

        public int getGbaSupported() {
            return TelephonyUtils.getGbaSupported(mContext,
                    SubscriptionManager.getDefaultSubscriptionId());
        }

        public int getGbaSupportedForSubscriber(int subId) {
            return TelephonyUtils.getGbaSupported(mContext, subId);
        }


        public boolean isGbaKeyExpired(String nafFqdn, byte[] nafSecurProtocolId) {
            boolean bIsKeyExpired = true;

            return isGbaKeyExpiredForSubscriber(nafFqdn, nafSecurProtocolId,
                SubscriptionManager.getDefaultSubscriptionId());
        }

        public boolean isGbaKeyExpiredForSubscriber(
            String nafFqdn, byte[] nafSecurProtocolId, int subId) {
            boolean bIsKeyExpired = true;

            NafId nafId = NafId.createFromNafFqdnAndProtocolId(nafFqdn, nafSecurProtocolId);
            bIsKeyExpired = mGbaKeysCache.isExpiredKey(nafId, subId);

            return bIsKeyExpired;
        }

        public void setNetwork(Network network) {
            if (network != null) {
                ConnectivityManager connMgr =
                        (ConnectivityManager) mContext.getSystemService(Context.CONNECTIVITY_SERVICE);

                connMgr.setProcessDefaultNetwork(network);
            }
        }

        public NafSessionKey runGbaAuthentication(String nafFqdn, byte[] nafSecurProtocolId,
                boolean forceRun) {
            return runGbaAuthenticationForSubscriber(nafFqdn, nafSecurProtocolId, forceRun,
                   SubscriptionManager.getDefaultSubscriptionId());
        }

        public NafSessionKey runGbaAuthenticationForSubscriber(
            String nafFqdn, byte[] nafSecurProtocolId, boolean forceRun, int subId) {

            Log.d(TAG, "runGbaAuthenticationForSubscriber: subId" + subId );

            GbaDebugParam gbaDebugParam = GbaDebugParam.getInstance();
            gbaDebugParam.load();
            Log.d(TAG, "Gba debug params: \n" + gbaDebugParam.toString());

            NafSessionKey nafSessionKey = null;
            NafId nafId = NafId.createFromNafFqdnAndProtocolId(nafFqdn, nafSecurProtocolId);
            boolean isExpiredKey = mGbaKeysCache.isExpiredKey(nafId, subId);

            boolean needForce = gbaDebugParam.getEnableGbaForceRun();

            if (needForce || TelephonyUtils.getTestSIM(subId) == 1) {
                Log.i(TAG, "Need force");
                forceRun = true;
            }

            if (!isExpiredKey && !forceRun) {
                nafSessionKey = mGbaKeysCache.getKeys(nafId, subId);
            } else {
                int gbaType = TelephonyUtils.getGbaSupported(mContext, subId);
                GbaBsfProcedure gbaProcedure = new GbaBsfProcedure(gbaType, subId, mContext);
                try {
                    nafSessionKey = gbaProcedure.perform(nafId);
                    if (nafSessionKey != null) {
                        Log.d(TAG, "nafSessionKey:" + nafSessionKey);
                        mGbaKeysCache.putKeys(nafId, subId, nafSessionKey);
                    }
                } catch (Exception e) {
                    Log.e(TAG, "GBA perform bsf procedure exception occur");
                    e.printStackTrace();
                }
            }

            return nafSessionKey;
        }

        public NafSessionKey getCachedKey(String nafFqdn, byte[] nafSecurProtocolId, int subId) {
            Log.d(TAG, "getCachedKey: nafFqdn=" + nafFqdn + ", subId=" + subId);
            NafSessionKey nafSessionKey = null;
            NafId nafId = NafId.createFromNafFqdnAndProtocolId(nafFqdn, nafSecurProtocolId);
            nafSessionKey = mGbaKeysCache.getKeys(nafId, subId);
            Log.d(TAG, "getCachedKey: nafSessionKey=" + nafSessionKey);
            return nafSessionKey;
        }

        public void updateCachedKey(String nafFqdn, byte[] nafSecurProtocolId, int subId,
                NafSessionKey nafSessionKey) {
            Log.d(TAG, "updateCachedKey: nafFqdn=" + nafFqdn + ", subId=" + subId);
            NafId nafId = NafId.createFromNafFqdnAndProtocolId(nafFqdn, nafSecurProtocolId);
            mGbaKeysCache.putKeys(nafId, subId, nafSessionKey);
        }
    };

/*
    Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case EVENT_SIM_STATE_CHANGED:

                    break;
                default:
                    break;
            }
        }
    };
*/
}
