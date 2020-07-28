/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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

package com.mediatek.op18.settings;

import com.android.ims.ImsConfig;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.provider.Settings;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.widget.Toast;

import static com.android.internal.telephony.TelephonyIntents.SECRET_CODE_ACTION;

public class VoWifiSettingsReceiverExtOP18 extends BroadcastReceiver {

    private static final String TAG = "VoWifiSettingsReceiverExtOP18";
    //process vowifi on
    private final Uri mVoWifiOnUri = Uri.parse("android_secret_code://8694341");
    //process vowifi off
    private final Uri mVoWifiOffUri = Uri.parse("android_secret_code://8694340");
    //global settings to save
    private final String VOWIFI_ONOFF_SETTING = "vowifi_secretcode_setting";

    @Override
    public void onReceive(Context context, Intent intent) {
        Log.v(TAG, "onReceive: " + intent.getAction());
        String action = intent.getAction();

        if ((action != null) && (SECRET_CODE_ACTION.equals(action))) {
            Uri uri = intent.getData();

            if ((uri != null) && (mVoWifiOnUri.equals(uri))) {
                Log.v(TAG," uri mVoWifiOnUri: "+mVoWifiOnUri);
                Settings.Global.putInt(context
                        .getContentResolver(), VOWIFI_ONOFF_SETTING, 1);
                Settings.System.putInt(context.getContentResolver(),
                     Settings.System.WHEN_TO_MAKE_WIFI_CALLS,
                TelephonyManager.WifiCallingChoices.ALWAYS_USE);
                Settings.System.putInt(context.getContentResolver(),
                     android.provider.Settings.Global.WFC_IMS_MODE,
                ImsConfig.WfcModeFeatureValueConstants.WIFI_PREFERRED);
                Toast.makeText(context, "VoWifi ON, reboot phone",
                Toast.LENGTH_LONG).show();

            } else if ((uri != null) && mVoWifiOffUri.equals(uri)){
                Log.v(TAG," uri mVoWifiOffUri: "+mVoWifiOffUri);
                Settings.Global.putInt(context
                        .getContentResolver(), VOWIFI_ONOFF_SETTING, 0);
                Settings.System.putInt(context.getContentResolver(),
                     Settings.System.WHEN_TO_MAKE_WIFI_CALLS,
                TelephonyManager.WifiCallingChoices.NEVER_USE);
                Toast.makeText(context, "VoWifi OFF, reboot phone",
                Toast.LENGTH_LONG).show();
            } else {
                Log.v(TAG, "unknown uri");
            }

        }
    }
}