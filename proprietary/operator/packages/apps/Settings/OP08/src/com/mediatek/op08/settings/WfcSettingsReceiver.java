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

package com.mediatek.op08.settings;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.wifi.WifiManager;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.preference.PreferenceManager;
import android.provider.Settings;
import android.telephony.SubscriptionManager;
import android.util.Log;

import com.android.ims.ImsManager;
import com.android.internal.telephony.ITelephony;
import com.mediatek.provider.MtkSettingsExt;


/** Broadcast receiver to receive SIM_CHANGE & WIFI_STATE_CHANGE intents.
 * & take action accordingly for WFC Settings
 */
public class WfcSettingsReceiver extends BroadcastReceiver {

    private static final String TAG = "Op08WfcSettingsReceiver";
    private static final String FIRST_TIME = "first_time_in_wfcSetting";
    private static final String WIFI_CONNECTED_FIRST_TIME = "wifi_connected_first_time";
    private static final String KEY_ISIM_PRESENT = "persist.sys.wfc_isimAppPresent";
    // Hotknot intents which tells hotKnot wifi mac discovery start & finish
    private static final String HOTKNOT_DISCOVERY_START_ACTION
            = "com.mediatek.hotknot.DISCOVERY_START_ACTION";
    private static final String HOTKNOT_DISCOVERY_END_ACTION
            = "com.mediatek.hotknot.DISCOVERY_END_ACTION";
    private static final String LISTEN_WIFI_STATE_CHANGE_INTENT_FLAG
            = "listen_wifi_state_change_intent_flag";
    private static final String WFC_MODE_DIALOG_ACTION = "mediatek.settings.SHOW_WFC_MODE_DIALOG";

    @Override
    public void onReceive(Context context, Intent intent) {

        if (!ImsManager.isWfcEnabledByPlatform(context)) {
            return;
        }

        String action = intent.getAction();
        Log.v(TAG, action);
        if (WifiManager.WIFI_STATE_CHANGED_ACTION.equals(action)
                && getWifiStateChangeListenFlag(context)) {
            int wifiState = intent.getIntExtra(WifiManager.EXTRA_WIFI_STATE,
                    WifiManager.WIFI_STATE_UNKNOWN);
            Log.v(TAG, "wifiState:" + wifiState);
            if (wifiState == WifiManager.WIFI_STATE_ENABLING) {
                SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(context);
                if (sp.getBoolean(WIFI_CONNECTED_FIRST_TIME, true)) {
                    sp.edit().putBoolean(WIFI_CONNECTED_FIRST_TIME, false).commit();
                    Log.v(TAG, "wifi_connected_first_time after commit"
                            + sp.getBoolean(WIFI_CONNECTED_FIRST_TIME, true));
                } else {
                    return;
                }

                Log.v(TAG, "launching WFC wifi dialog activity");
                Intent i = new Intent(context, WfcWifiDialogActivity.class);
                i.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                context.startActivity(i);
            }
        } else if (action.equals(HOTKNOT_DISCOVERY_START_ACTION)) {
        //ALPS02164418: On boot Hotknot turns Wifi ON to read MAC address from MD.
        // After getting MAC, it turns Wifi OFF. This turning of Wifi ON causes
        // showing of WFC Wifi first time ON Alert, while User has not turned wifi ON.
        // To prevent this, HotKnot intimates other user of its MAC discovery start &
        // end via HOTKNOT_DISCOVERY_START_ACTION & HOTKNOT_DISCOVERY_END_ACTION intents,
        // During this period WfcRecevier will ignore Wifi_state_change intents.
            Log.v(TAG, "Turn OFF wifiStateChange listen flag");
            setWifiStateChangeListenFlag(context, false);
        } else if (action.equals(HOTKNOT_DISCOVERY_END_ACTION)) {
            Log.v(TAG, "Turn ON wifiStateChange listen flag");
            setWifiStateChangeListenFlag(context, true);
        }
    }

    private void setWifiStateChangeListenFlag(Context context, boolean flag) {
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(context);
        sp.edit().putBoolean(LISTEN_WIFI_STATE_CHANGE_INTENT_FLAG, flag).commit();
    }

    private boolean getWifiStateChangeListenFlag(Context context) {
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(context);
        return sp.getBoolean(LISTEN_WIFI_STATE_CHANGE_INTENT_FLAG, true);
    }
}
