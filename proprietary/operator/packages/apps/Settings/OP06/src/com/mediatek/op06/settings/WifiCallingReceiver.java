/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.op06.settings;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.preference.PreferenceManager;
import android.provider.Settings;
import android.telephony.CarrierConfigManager;
import android.util.Log;

import com.android.ims.ImsConfig;

import com.mediatek.ims.config.ImsConfigContract;
import com.mediatek.ims.internal.MtkImsConfig;

/** Broadcast receiver to receive Boot complete intent, ImsConfig intents
 * & invoke service to set WFC Settings in settings Provider during provisioning & de-provisioning.
 */
public class WifiCallingReceiver extends BroadcastReceiver {

    private static final String TAG = "OP06WifiCallingReceiver";
    private static final String FIRST_BOOT = "first_boot";
    private static final String IMS_ENABLE_DEFAULT_VALUE = "ims_enable_default_value";
    private static final String IMS_MODE_DEFAULT_VALUE = "ims_mode_default_value";
    private static final String OMACP_SETTING_ACTION = "com.mediatek.omacp.settings";

    @Override
    public void onReceive(Context context, Intent intent) {

        String action = intent.getAction();
        Log.v(TAG, action);
        if (action.equals(Intent.ACTION_BOOT_COMPLETED)) {
            SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(context);
            /* Add in db only on first boot */
            if (sp.getBoolean(FIRST_BOOT, true)) {
                Editor ed = sp.edit();
                ed.putBoolean(FIRST_BOOT, false);
                // Collect default value of WFC_IMS_ENABLED & WFC_IMS_MODE on First Boot complete.
                // Need to be done here because no other way to get it from SettingsProvider.
                // during first boot, these values are initiliazed in db(some via plugins too).
                ed.putBoolean(IMS_ENABLE_DEFAULT_VALUE, Settings.Global.getInt(context
                        .getContentResolver(), Settings.Global.WFC_IMS_ENABLED,
                        ImsConfig.FeatureValueConstants.OFF) == ImsConfig.FeatureValueConstants.ON ?
                                    true : false);
                ed.putInt(IMS_MODE_DEFAULT_VALUE, Settings.Global.getInt(context
                        .getContentResolver(), Settings.Global.WFC_IMS_MODE,
                        ImsConfig.WfcModeFeatureValueConstants.WIFI_PREFERRED));
                ed.commit();
                Log.d(TAG, "written in sp after first boot");
            }
        } else if (action.equals(ImsConfigContract.ACTION_IMS_CONFIG_CHANGED)) {
            int configId = intent.getIntExtra(ImsConfigContract.EXTRA_CONFIG_ID, -1);
            Log.d(TAG, "configId:" + configId);
            if (MtkImsConfig.ConfigConstants.EPDG_ADDRESS == configId) {
                handleIntent(context, intent);
            }
        } else if (action.equals(ImsConfigContract.ACTION_CONFIG_LOADED) ||
               CarrierConfigManager.ACTION_CARRIER_CONFIG_CHANGED.equals(action) ||
               OMACP_SETTING_ACTION.equals(action)) {
            handleIntent(context, intent);
        }
    }

    private void handleIntent(Context context, Intent intent) {
        /* Need to start a service, because when volte settings are configured via ImsManager,
            * it uses ImsService for this. A service cannot be binded from a broadcast receiver
            */
        intent.setClass(context, WifiCallingReceiverService.class);
        context.startService(intent);
    }
}
