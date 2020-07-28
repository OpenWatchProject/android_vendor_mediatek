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
package com.mediatek.op07.settings;

import android.app.Activity;

import android.content.ComponentName;
import android.content.Context;
import android.os.IBinder;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.util.Log;

import com.android.ims.ImsManager;
//import com.android.settingslib.drawer.SettingsDrawerActivity;
import com.android.settings.core.SettingsBaseActivity;

import com.mediatek.op07.settings.WfcSettingsActivitySecond;
import com.mediatek.settings.ext.DefaultSettingsMiscExt;

import com.mediatek.wfo.IWifiOffloadService;
import com.mediatek.wfo.WifiOffloadManager;

/**
 * For settings small feature misc.
 */
public class Op07SettingsMiscExt extends DefaultSettingsMiscExt {

    private static final String TAG = "Op07SettingsMiscExt";


    private Context mContext;

    /**
     * Constructor method.
     * @param context Settings's context
     */
    public Op07SettingsMiscExt(Context context) {
        super(context);
        mContext = context;
    }

    /**
     * Add/remove WFC tile in settings main page.
     * @param activity SettingsDrawerActivity activity
     * @param isAdmin isAdmin
     * @param somethingChanged If some tile is changed
     * @return true/false
     */
    @Override
    public boolean doUpdateTilesList(Activity activity, boolean isAdmin, boolean somethingChanged) {
    	SettingsBaseActivity settingsActivity = (SettingsBaseActivity) activity;
        boolean enabled = (ImsManager.isWfcEnabledByPlatform(mContext)
                && isEntitlementEnabled());
        Log.d(TAG, "doUpdateTilesList:" + enabled);
        if (enabled == true) {
            somethingChanged = settingsActivity.setTileEnabled(new ComponentName(getPackageName(),
                WfcSettingsActivitySecond.class.getName()), true) || somethingChanged;
        } else {
            somethingChanged = settingsActivity.setTileEnabled(new ComponentName(getPackageName(),
            		WfcSettingsActivitySecond.class.getName()), false) || somethingChanged;
        }
        return somethingChanged;
    }

    @Override
    public void doWosFactoryReset() {
        Log.d(TAG, "doWosFactoryReset");
        IBinder b = ServiceManager.getService(WifiOffloadManager.WFO_SERVICE);
        try {
            IWifiOffloadService wifiOffloadService = IWifiOffloadService.Stub.asInterface(b);
            if (wifiOffloadService != null) {
                wifiOffloadService.factoryReset();
            }
        } catch (RemoteException e) {
            Log.d(TAG, "Fail to connect to WOS");
        }

    }

    public static boolean isEntitlementEnabled() {
        boolean isEntitlementEnabled = (1 == SystemProperties.getInt
                ("persist.vendor.entitlement_enabled", 1) ? true : false);
        Log.d(TAG, "isEntitlementEnabled:" + isEntitlementEnabled);

        return isEntitlementEnabled;

    }
}
