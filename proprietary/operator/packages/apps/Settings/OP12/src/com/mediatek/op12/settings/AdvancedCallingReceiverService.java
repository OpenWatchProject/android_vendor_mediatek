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

package com.mediatek.op12.settings;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.IBinder;
import android.provider.Settings;
import android.util.Log;

import com.android.ims.ImsConfig;
import com.android.ims.ImsException;
import com.android.ims.ImsManager;
import com.mediatek.ims.config.ImsConfigContract;


public class AdvancedCallingReceiverService extends Service {
    private static final String TAG = "Op12AdvancedCallingReceiverService";
    private Context mContext;
    private boolean mEnablePlatform;

    @Override
    public IBinder onBind(Intent arg0) {
        // TODO Auto-generated method stub
        return null;
    }

    @Override
    public void onCreate() {
        Log.d(TAG, "onCreate");
        mContext = getApplicationContext();
        mEnablePlatform = ImsManager.isVtEnabledByPlatform(mContext);
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        boolean isVolteProvisioned = false;
        boolean isLVCEnabled = true;
        if (intent != null) {
            String action = intent.getAction();
            Log.d(TAG, "onStartCommand: action:" + action);
            Context context = AdvancedCallingReceiverService.this;
            int phoneId = intent.getIntExtra(ImsConfigContract.EXTRA_PHONE_ID, 0);
            ImsConfig imsConfig = null;
            ImsManager imsManager = ImsManager.getInstance(context, phoneId);
            if (imsManager != null) {
                try {
                    imsConfig = imsManager.getConfigInterface();
                    if (imsConfig != null) {
                        isVolteProvisioned = ImsManager.isVolteProvisionedOnDevice(mContext);
                        isLVCEnabled = (1 == imsConfig.getProvisionedValue(
                                 ImsConfig.ConfigConstants.LVC_SETTING_ENABLED));
                        boolean advancedCallingSetting = Settings.Global.getInt(mContext
                                .getContentResolver(), "KEY_ADVANCED_CALLING", 0) == 1;
                        boolean volteSetting = isVolteProvisioned && advancedCallingSetting;
                        ImsManager.setEnhanced4gLteModeSetting(context, volteSetting);
                        boolean videoCallingSetting = Settings.Global.getInt(mContext
                                .getContentResolver(), "KEY_CALL_OPTIONS", 1) == 1;
                        Log.d(TAG, "isVolteProvisioned:" + isVolteProvisioned + " isLVCEnabled:" +
                                 isLVCEnabled + " videoCallingSetting:" + videoCallingSetting +
                                 "volteSetting:" + volteSetting);
                        if (mEnablePlatform) {
                            boolean setVT = volteSetting && isLVCEnabled && isInternetConnected()
                                             && videoCallingSetting;
                            Log.d(TAG, "set VT:" + setVT);
                            ImsManager.setVtSetting(context, setVT);
                        }
                    }
                } catch (ImsException e) {
                    Log.e(TAG, "Advanced settings not updated, ImsConfig null");
                    e.printStackTrace();
                }
            } else {
                Log.e(TAG, "Advanced  settings not updated, ImsManager null");
            }
        }
        return Service.START_STICKY;
    }

    private boolean isInternetConnected() {
    ConnectivityManager connectivityMgr = (ConnectivityManager) mContext
            .getSystemService(Context.CONNECTIVITY_SERVICE);
    NetworkInfo wifi = connectivityMgr.getNetworkInfo(ConnectivityManager.TYPE_WIFI);
    NetworkInfo mobile = connectivityMgr.getNetworkInfo(ConnectivityManager.TYPE_MOBILE);
    // Check if wifi or mobile network is available or not. If any of them is
    // available or connected then it will return true, otherwise false;

    if (wifi != null) {
        if (wifi.isConnected()) {
            Log.d(TAG, "isInternetConnected: Wifi is connected");
            return true;
        }
    }
    if (mobile != null) {
        if (mobile.isConnected()) {
            Log.d(TAG, "isInternetConnected: Mobile data is connected");
            return true;
        }
    }
    Log.d(TAG, "isInternetConnected: Internet not Connected");
    return false;
  }
}

