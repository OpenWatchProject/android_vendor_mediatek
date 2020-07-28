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

package com.mediatek.engineermode.debugtool;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.SharedPreferences;
import android.content.SharedPreferences.OnSharedPreferenceChangeListener;
import android.os.Build;
import android.os.Bundle;
import android.os.IBinder;
import android.os.SystemProperties;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceClickListener;
import android.preference.PreferenceActivity;
import android.preference.PreferenceCategory;
import android.preference.PreferenceScreen;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;

/**
 * @author Aee owner
 *
 */
public class DebugToolboxActivity extends PreferenceActivity implements
        OnSharedPreferenceChangeListener, OnPreferenceClickListener {
    private static final String TAG = "DebugToolboxActivity/Debugutils";

    private boolean mIsBound;

    private AEEControlService mBoundService;

    ListPreference mPrefAeemode;

    ListPreference mPrefDaloption;
    PreferenceScreen mClearDAL;

    PreferenceScreen mCleanData;

    private static final String sAEE_MODE = "persist.vendor.mtk.aeev.mode";

    private static final String sAEE_BUILD_INFO = "ro.vendor.aee.build.info";

    private static final String[] sAEE_MODE_STRING = { "", "MediatekEngineer", "MediatekUser",
            "CustomerEngineer", "CustomerUser" };

    private static final String sDAL_SETTING = "persist.vendor.mtk.aee.dal";

    private static final String[] sDAL_OPTION_STRING = { "EnableDAL", "DisableDAL" };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // Load the preferences from an XML resource
        addPreferencesFromResource(R.xml.preference_debugtoolbox);
        // setContentView(R.layout.main);

        mPrefAeemode =
                (ListPreference) getPreferenceScreen().findPreference(
                        getString(R.string.preference_aeemode));
        String aeebuildinfo = getProperty(sAEE_BUILD_INFO);
        if (aeebuildinfo == null || !aeebuildinfo.equals("mtk")) {
            mPrefAeemode.setEntries(R.array.entries_list_preference_aeemode_oem);
            mPrefAeemode.setEntryValues(R.array.entryvalues_list_preference_aeemode_oem);
        }

        String deviceAEEmode = currentAEEMode();
        mPrefAeemode.setValue(deviceAEEmode);
        mPrefAeemode.setSummary(deviceAEEmode);

        mPrefDaloption =
                (ListPreference) getPreferenceScreen().findPreference(
                        getString(R.string.preference_dal_setting));
        String deviceDalOption = currentDalOption();
        mPrefDaloption.setValue(deviceDalOption);
        mPrefDaloption.setSummary(deviceDalOption);

        if (aeebuildinfo == null || !aeebuildinfo.equals("mtk")) {
            mPrefDaloption.setSelectable(false);
        }

        mClearDAL =
                (PreferenceScreen) getPreferenceScreen().findPreference(
                        getString(R.string.preference_cleardal));
        mClearDAL.setOnPreferenceClickListener(this);

        mCleanData =
                (PreferenceScreen) getPreferenceScreen().findPreference(
                        getString(R.string.preference_cleandata));
        mCleanData.setOnPreferenceClickListener(this);
        if (!Build.TYPE.equals("eng")) {
            // getPreferenceScreen().removePreference(cleanData);
            mCleanData.setEnabled(false);
        }

        PreferenceCategory deviceMaintain =
                (PreferenceCategory) getPreferenceScreen().findPreference(
                        getString(R.string.preferences_device_maintain));
        getPreferenceScreen().removePreference(deviceMaintain);

        doBindService();
    }

    @Override
    protected void onResume() {
        super.onResume();
        // Set up a listener whenever a key changes
        getPreferenceScreen().getSharedPreferences().registerOnSharedPreferenceChangeListener(this);
    }

    @Override
    protected void onPause() {
        super.onPause();
        // Unregister the listener whenever a key changes
        getPreferenceScreen().getSharedPreferences().unregisterOnSharedPreferenceChangeListener(
                this);
    }

    /**
     * @param sharedPreferences SharedPreferences
     * @param key String
     */
    @Override
    public void onSharedPreferenceChanged(SharedPreferences sharedPreferences, String key) {
        if (key.equals(getString(R.string.preference_aeemode))) {
            String modevalue = sharedPreferences.getString(key, null);
            if (modevalue == null) {
                Elog.e(TAG, "AEE working mode is set to NULL.");
                return;
            }
            try {
                mBoundService.changeAEEMode(modevalue);
            } catch (NullPointerException e) {
                Elog.e(TAG, "AEE Service is not started");
            }
            String deviceAEEmode = currentAEEMode();
            if (!deviceAEEmode.equals(modevalue)) {
                mPrefAeemode.setValue(deviceAEEmode);
                String errorString =
                        "Change debug level [" + deviceAEEmode + "-->" + modevalue
                                + "] too fequent, please retry later.";
                Elog.e(TAG, errorString);
                Toast.makeText(this, errorString, Toast.LENGTH_SHORT).show();
            }
            mPrefAeemode.setSummary(deviceAEEmode);
        }

        if (key.equals(getString(R.string.preference_dal_setting))) {
            String dalOption = sharedPreferences.getString(key, null);
            if (dalOption == null) {
                Elog.d(TAG, "DAL setting mode is set to NULL.");
                return;
            }
            try {
                mBoundService.dalSetting(dalOption);
            } catch (NullPointerException e) {
                Elog.e(TAG, "DAL setting error");
            }
            String deviceDalOption = currentDalOption();
            if (!deviceDalOption.equals(dalOption)) {
                mPrefDaloption.setValue(deviceDalOption);
            }
            mPrefDaloption.setSummary(deviceDalOption);
        }
    }

    @Override
    public boolean onPreferenceClick(Preference preference) {
        if (preference.equals(mClearDAL)) {
            try {
                mBoundService.clearDAL();
                return true;
            } catch (NullPointerException e) {
                Elog.e(TAG, "AEE Service is not started");
            }
        } else if (preference.equals(mCleanData)) {
            try {
                mBoundService.cleanData();
                return true;
            } catch (NullPointerException e) {
                Elog.e(TAG, "AEE Service is not started");
            }
        }
        return false;
    }

    private ServiceConnection mConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName className, IBinder service) {
            // This is called when the connection with the service has been
            // established, giving us the service object we can use to
            // interact with the service. Because we have bound to a explicit
            // service that we know is running in our own process, we can
            // cast its IBinder to a concrete class and directly access it.
            mBoundService = ((AEEControlService.LocalBinder) service).getService();
            // Tell the user about this for our demo.
            // Toast.makeText(DebugToolboxActivity.this,
            // R.string.aee_service_connected,
            // Toast.LENGTH_SHORT).show();
        }

        @Override
        public void onServiceDisconnected(ComponentName className) {
            // This is called when the connection with the service has been
            // unexpectedly disconnected -- that is, its process crashed.
            // Because it is running in our same process, we should never
            // see this happen.
            mBoundService = null;
            // Toast.makeText(DebugToolboxActivity.this,
            // R.string.aee_service_disconnected,
            // Toast.LENGTH_SHORT).show();
        }
    };

    void doBindService() {
        // Establish a connection with the service. We use an explicit
        // class name because we want a specific service implementation that
        // we know will be running in our own process (and thus won't be
        // supporting component replacement by other applications).
        bindService(new Intent(DebugToolboxActivity.this, AEEControlService.class), mConnection,
                Context.BIND_AUTO_CREATE);
        mIsBound = true;
    }

    void doUnbindService() {
        if (mIsBound) {
            // Detach our existing connection.
            unbindService(mConnection);
            mIsBound = false;
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        doUnbindService();
    }

    private static String currentAEEMode() {
        String aeemodeIndexString = getProperty(sAEE_MODE);
        if (aeemodeIndexString != null && !aeemodeIndexString.isEmpty()) {
            int aeemodeIndex = Integer.valueOf(aeemodeIndexString);
            if (aeemodeIndex >= 1 && aeemodeIndex <= 4) {
                String aeemode = sAEE_MODE_STRING[aeemodeIndex];
                return aeemode;
            }
        }
        String aeebuildinfo = getProperty(sAEE_BUILD_INFO);
        int aeebuildinfoInt = aeebuildinfo.equals("mtk") ? 2 : 4;
        int robuildtypeInt = Build.TYPE.equals("eng") ? 1 : 0;
        return sAEE_MODE_STRING[aeebuildinfoInt - robuildtypeInt];
    }

    private static String currentDalOption() {
        String dalOptionIndexString = getProperty(sDAL_SETTING);
        if (dalOptionIndexString != null && !dalOptionIndexString.isEmpty()) {
            int dalOptionIndex = Integer.valueOf(dalOptionIndexString);
            if (dalOptionIndex == 0) { // persist.vendor.mtk.aee.dal :0
                return sDAL_OPTION_STRING[1];
            }
            if (dalOptionIndex == 1) { // persist.vendor.mtk.aee.dal :1
                return sDAL_OPTION_STRING[0];
            }
        }
        return "";
    }

    private static String getProperty(String prop) {
        return SystemProperties.get(prop);
    }

    @Override
    protected boolean isValidFragment(String fragmentName) {
        return false;
    }
}
