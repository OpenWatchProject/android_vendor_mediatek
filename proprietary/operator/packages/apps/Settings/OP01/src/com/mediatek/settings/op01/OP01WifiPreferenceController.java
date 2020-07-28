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

package com.mediatek.settings.op01;

import android.content.ContentResolver;
import android.content.Context;
import android.content.res.Resources;
import android.net.wifi.WifiManager;
import android.provider.Settings.System;

import androidx.preference.ListPreference;
import androidx.preference.Preference;
import androidx.preference.Preference.OnPreferenceChangeListener;
import androidx.preference.PreferenceScreen;
import android.util.Log;

import com.android.settingslib.core.AbstractPreferenceController;
import com.android.settingslib.core.lifecycle.Lifecycle;
import com.android.settingslib.core.lifecycle.LifecycleObserver;
import com.android.settingslib.core.lifecycle.events.OnPause;
import com.android.settingslib.core.lifecycle.events.OnResume;

import com.mediatek.provider.MtkSettingsExt;

public class OP01WifiPreferenceController extends AbstractPreferenceController implements LifecycleObserver, OnResume, OnPause{

    private Context mContext;
    private Context mOP01Context;

    private ListPreference mConnectTypePref;
    private ListPreference mConnectReminderPref;

    private static final int WIFI_CONNECT_REMINDER_ALWAYS = 0;
    private static final String KEY_CONNECT_TYPE = "connect_type";
    private static final String KEY_CONNECT_REMINDER = "connect_reminder";

    private String TAG = "OP01WifiPreferenceController";
    private WifiManager mWifiManager;
    private ContentResolver mContentResolver;

    public OP01WifiPreferenceController(Context context, Context op01Context, Lifecycle lifecycle) {
        super(context);
        mContext = context;
        mOP01Context = op01Context;
        mWifiManager = ((WifiManager) mContext.getSystemService(Context.WIFI_SERVICE));
        mContentResolver = context.getContentResolver();
        lifecycle.addObserver(this);
    }

    /**
     * Displays preference in this controller.
     */
    public void displayPreference(PreferenceScreen screen) {
        Log.i(TAG, "displayPreference");
        initConnectView(screen);
    }

    @Override
    public String getPreferenceKey() {
        return null;
    }

    @Override
    public boolean isAvailable() {
        return false;
    }

    public void initConnectView(PreferenceScreen screen) {
        Log.i(TAG, "initConnectView");
        mConnectTypePref = new ListPreference(screen.getPreferenceManager()
                .getContext());
        mConnectTypePref.setTitle(mOP01Context
                .getString(R.string.wifi_connect_type_title));
        mConnectTypePref.setDialogTitle(mOP01Context
                .getString(R.string.wifi_connect_type_title));
        mConnectTypePref.setEntries(mOP01Context.getResources().getTextArray(
                R.array.wifi_connect_type_entries));
        mConnectTypePref.setEntryValues(mOP01Context.getResources()
                .getTextArray(R.array.wifi_connect_type_values));
        mConnectTypePref.setKey(KEY_CONNECT_TYPE);
        mConnectTypePref
                .setOnPreferenceChangeListener(mPreferenceChangeListener);
        screen.addPreference(mConnectTypePref);

        mConnectReminderPref = new ListPreference(screen.getPreferenceManager()
                .getContext());
        mConnectReminderPref.setTitle(mOP01Context
                .getString(R.string.wifi_reminder_frequency_title));
        mConnectReminderPref.setDialogTitle(mOP01Context
                .getString(R.string.wifi_reminder_frequency_title));
        mConnectReminderPref.setEntries(mOP01Context.getResources()
                .getTextArray(R.array.wifi_reminder_entries));
        mConnectReminderPref.setEntryValues(mOP01Context.getResources()
                .getTextArray(R.array.wifi_reminder_values));
        mConnectReminderPref.setKey(KEY_CONNECT_REMINDER);
        mConnectReminderPref
                .setOnPreferenceChangeListener(mPreferenceChangeListener);
        screen.addPreference(mConnectReminderPref);

    }

    private OnPreferenceChangeListener mPreferenceChangeListener = new OnPreferenceChangeListener() {
        @Override
        public boolean onPreferenceChange(Preference preference, Object newValue) {
            String key = preference.getKey();
            Log.d("@M_" + TAG, "key=" + key);

            if (KEY_CONNECT_TYPE.equals(key)) {
                Log.d("@M_" + TAG, "Wifi connect type is " + newValue);
                try {
                    System.putInt(mOP01Context.getContentResolver(),
                            MtkSettingsExt.System.WIFI_CONNECT_TYPE, Integer
                                    .parseInt(((String) newValue)));
                    if (mConnectTypePref != null) {
                        CharSequence[] array = mOP01Context
                                .getResources()
                                .getTextArray(R.array.wifi_connect_type_entries);
                        mConnectTypePref.setSummary((String) array[Integer
                                .parseInt(((String) newValue))]);
                    }
                } catch (NumberFormatException e) {
                    Log.d("@M_" + TAG, "set Wifi connect type error ");
                    return false;
                }
                try {
                    System.putInt(mOP01Context.getContentResolver(),
                            MtkSettingsExt.System.WIFI_SELECT_SSID_TYPE, Integer
                                    .parseInt(((String) newValue)));
                } catch (NumberFormatException e) {
                    Log.d("@M_" + TAG, "set Wifi SSID reselect type error ");
                    return false;
                }
            } else if (KEY_CONNECT_REMINDER.equals(key)) {
                try {
                    System.putInt(mOP01Context.getContentResolver(),
                            MtkSettingsExt.System.WIFI_CONNECT_REMINDER, Integer
                                    .parseInt(((String) newValue)));
                    if (mConnectReminderPref != null) {
                        CharSequence[] array = mOP01Context.getResources()
                                .getTextArray(R.array.wifi_reminder_entries);
                        mConnectReminderPref.setSummary((String) array[Integer
                                .parseInt(((String) newValue))]);
                    }
                } catch (NumberFormatException e) {
                    Log.d("@M_" + TAG, "set Wifi connect type error ");
                    return false;
                }
            }
            return true;
        }
    };
    
    @Override
    public void onResume() {
        Log.i(TAG, "onResume");
        updateConnectViewValue();
    }
    @Override
    public void onPause() {
        Log.i(TAG, "onPause");
    }

    private void updateConnectViewValue(){
        Log.i(TAG, "updateConnectViewValue");
        if (mConnectTypePref != null) {
            int value = System.getInt(mContentResolver,
                    MtkSettingsExt.System.WIFI_CONNECT_TYPE,
                MtkSettingsExt.System.WIFI_CONNECT_TYPE_AUTO);
            mConnectTypePref.setValue(String.valueOf(value));
            CharSequence[] array =
                mOP01Context.getResources().getTextArray(R.array.wifi_connect_type_entries);
            mConnectTypePref.setSummary((String) array[value]);

            int value1 = System.getInt(mContentResolver,
                    MtkSettingsExt.System.WIFI_SELECT_SSID_TYPE,
                MtkSettingsExt.System.WIFI_SELECT_SSID_AUTO);
            if (value != value1) {
                System.putInt(mContentResolver, MtkSettingsExt.System.WIFI_SELECT_SSID_TYPE, value);
            }
        }

        if (mConnectReminderPref != null) {
            int value = System.getInt(mContentResolver,
                    MtkSettingsExt.System.WIFI_CONNECT_REMINDER,
                WIFI_CONNECT_REMINDER_ALWAYS);
            mConnectReminderPref.setValue(String.valueOf(value));
            CharSequence[] array =
                mOP01Context.getResources().getTextArray(R.array.wifi_reminder_entries);
            mConnectReminderPref.setSummary((String) array[value]);
        }
    }

}
