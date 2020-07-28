/*
 * Copyright (C) 2007-2008 Esmertec AG.
 * Copyright (C) 2007-2008 The Android Open Source Project
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.mediatek.setting;

import android.Manifest;
import android.app.ActionBar;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.Uri;
import android.os.Bundle;
import android.preference.CheckBoxPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceManager;
import android.preference.PreferenceScreen;
import android.provider.Settings;
import android.view.Menu;
import android.view.MenuItem;

import com.android.mms.MmsConfig;
import com.android.mms.R;
import com.android.mms.util.MmsLog;
import com.android.mms.transaction.MessagingNotification;

/**
 * With this activity, users can set preferences for MMS and SMS and
 * can access and manipulate SMS messages stored on the SIM.
 */
public class NotificationPreferenceActivity extends PreferenceActivity {
    private static final String TAG = "NotificationPreferenceActivity";

    private static final boolean DEBUG = false;

    public static final String NOTIFICATION_ENABLED = "pref_key_enable_notifications";
    public static final String NOTIFICATION_RINGTONE_SETTING = "pref_key_ringtone_setting";

    private static final int MENU_RESTORE_DEFAULTS = 1;

    private CheckBoxPreference mEnableNotificationsPref;
    private Preference mRingtoneSettingPref;

    @Override
    protected void onPause() {
        super.onPause();
    }

    @Override
    protected void onResume() {
        super.onResume();

        boolean isSmsEnabled = MmsConfig.isSmsEnabled(this);
        MmsLog.d(TAG, "onResume sms enable? " + isSmsEnabled);
        if (!isSmsEnabled) {
            finish();
            return;
        }

    }

    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        MmsLog.d(TAG, "onCreate");

        ActionBar actionBar = getActionBar();
        actionBar.setTitle(getResources().getString(R.string.actionbar_notification_setting));
        actionBar.setDisplayHomeAsUpEnabled(true);
        setDefaultPreferences(this);
        setMessagePreferences();
    }

    private void setMessagePreferences() {
        addPreferencesFromResource(R.xml.notificationpreferences);
        mRingtoneSettingPref = (Preference) findPreference(NOTIFICATION_RINGTONE_SETTING);
        mEnableNotificationsPref = (CheckBoxPreference) findPreference(NOTIFICATION_ENABLED);
        if (mEnableNotificationsPref.isChecked()) {
            mRingtoneSettingPref.setEnabled(true);
        } else {
            mRingtoneSettingPref.setEnabled(false);
        }
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
        case MENU_RESTORE_DEFAULTS:
            restoreDefaultPreferences();
            return true;
        case android.R.id.home:
            // The user clicked on the Messaging icon in the action bar. Take them back from
            // wherever they came from
            finish();
            return true;
        default:
            break;
        }
        return false;
    }

    public boolean onCreateOptionsMenu(Menu menu) {
        super.onCreateOptionsMenu(menu);
        menu.clear();
        menu.add(0, MENU_RESTORE_DEFAULTS, 0, R.string.restore_default);
        return true;
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen,
            Preference preference) {
        //new notification
        if (preference == mRingtoneSettingPref) {
            String packageName = getPackageName();
            String id = MessagingNotification.getNewMsgNotifyChannelId();
            MmsLog.d(TAG, "Go to system notification setting, packages=" + packageName + ", id = " + id);
            
            Intent intent = new Intent(Settings.ACTION_CHANNEL_NOTIFICATION_SETTINGS);
            intent.putExtra(Settings.EXTRA_APP_PACKAGE, getPackageName());
            intent.putExtra(Settings.EXTRA_CHANNEL_ID, id);
            startActivity(intent);
        } else if (preference == mEnableNotificationsPref) {
            if (mEnableNotificationsPref.isChecked()) {
                mRingtoneSettingPref.setEnabled(true);
            } else {
                mRingtoneSettingPref.setEnabled(false);
            }
        }
        return super.onPreferenceTreeClick(preferenceScreen, preference);
    }

    private void setDefaultPreferences(Context context) {
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        if (!prefs.contains(NOTIFICATION_ENABLED)) {
            // set to default value if not set before
            SharedPreferences.Editor editor =
                    PreferenceManager.getDefaultSharedPreferences(context).edit();
            editor.putBoolean(NOTIFICATION_ENABLED, true);
            editor.apply();
        }
    }

    private void restoreDefaultPreferences() {
        SharedPreferences.Editor editor =
                        PreferenceManager.getDefaultSharedPreferences(
                                NotificationPreferenceActivity.this).edit();
        editor.putBoolean(NOTIFICATION_ENABLED, true);
        editor.apply();
        setPreferenceScreen(null);
        setMessagePreferences();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }
}
