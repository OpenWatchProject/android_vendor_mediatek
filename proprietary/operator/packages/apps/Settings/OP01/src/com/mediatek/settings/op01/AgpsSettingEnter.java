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

import android.app.ActionBar;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.ContentQueryMap;
import android.content.ContentResolver;
import android.content.DialogInterface;
import android.content.Intent;
import android.database.Cursor;
import android.os.Bundle;
import android.os.SystemProperties;
import android.preference.CheckBoxPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.provider.Settings;
import android.util.Log;

import com.mediatek.lbs.em2.utils.AgpsInterface;

import java.io.IOException;
import java.util.Observable;
import java.util.Observer;

public class AgpsSettingEnter extends PreferenceActivity {

    private static final String TAG = "AgpsEpoSettingEnter";

    // / M: Agps enable and agps settings preference key
    private static final String KEY_AGPS_ENABLER = "agps_enabler";
    private static final String KEY_AGPS_SETTINGS = "agps_settings";

    // / M: Agps enable and agps settings preference
    private AgpsInterface agpsInterface;
    private CheckBoxPreference mAgpsCB;
    private Preference mAgpsPref;

    // Agps enable confirm dialog
    private static final int CONFIRM_AGPS_DIALOG_ID = 1;
    private ContentQueryMap mContentQueryMap;
    private Observer mSettingsObserver;
    private Cursor mSettingsCursor;
    private ContentResolver mContentResolver;

    public void onCreate(Bundle savedInstanceState) {
        Log.i(TAG, "oncreate doValidcheck");
        super.onCreate(savedInstanceState);
        mContentResolver = getContentResolver();
        createPreferenceHierarchy();
        getActionBar().setTitle(R.string.gps_settings_title);
    }

    @Override
    protected boolean isValidFragment(String fragmentName) {
        return false;
    }

    @Override
    public void onStart() {
        super.onStart();

        getActionBar().setDisplayOptions(
                ActionBar.DISPLAY_SHOW_CUSTOM, ActionBar.DISPLAY_SHOW_CUSTOM);

        // listen for Location Manager settings changes
        mSettingsCursor = mContentResolver.query(
                Settings.Secure.CONTENT_URI, null, "(" + Settings.System.NAME
                        + "=?)",
                new String[] { Settings.Secure.LOCATION_PROVIDERS_ALLOWED },
                null);
        mContentQueryMap = new ContentQueryMap(mSettingsCursor,
                Settings.System.NAME, true, null);

    }

    @Override
    public void onResume() {
        super.onResume();

        updateLocationToggles();

        if (mSettingsObserver == null) {
            mSettingsObserver = new Observer() {
                public void update(Observable o, Object arg) {
                    updateLocationToggles();
                }
            };
        }

        mContentQueryMap.addObserver(mSettingsObserver);
    }

    @Override
    public void onStop() {
        super.onStop();
        getActionBar().setDisplayOptions(0, ActionBar.DISPLAY_SHOW_CUSTOM);
        getActionBar().setCustomView(null);

        if (mSettingsObserver != null) {
            mContentQueryMap.deleteObserver(mSettingsObserver);
        }
        if (mSettingsCursor != null) {
            mSettingsCursor.close();
        }
    }

    private void createPreferenceHierarchy() {
        if (SystemProperties.get("ro.vendor.mtk_agps_app").equals("1")
                && (SystemProperties.get("ro.vendor.mtk_gps_support").equals("1") || SystemProperties.get("ro.kernel.qemu").equals("1"))) {

            try {
                agpsInterface = new AgpsInterface();
            } catch (IOException e) {
                Log.d("@M_" + TAG, "agpsInterface exception: " + e);
                e.printStackTrace();
            }

            if (agpsInterface == null) {
                Log.d("@M_" + TAG, "agpsInterface fail");
                return;
            }

            addPreferencesFromResource(R.xml.agps_setting);
            mAgpsPref = findPreference(KEY_AGPS_SETTINGS);
            mAgpsCB = (CheckBoxPreference) findPreference(KEY_AGPS_ENABLER);
        }
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen,
            Preference preference) {

        if (preference == mAgpsCB) {
            if (mAgpsCB.isChecked()) {
                mAgpsCB.setChecked(false);
                showDialog(CONFIRM_AGPS_DIALOG_ID);
            } else {
                agpsInterface.setAgpsEnabled(false);
                Log.d("@M_" + TAG, "agps_sky onPreferenceTreeClick setAgpsEnabled(false)");
            }
        } else if (preference == mAgpsPref) {
            startActivity(new Intent(this, AgpsSettings.class));
        } else {
            // If we didn't handle it, let preferences handle it.
            return super.onPreferenceTreeClick(preferenceScreen, preference);
        }

        return true;
    }

    /*
     * Creates toggles for each available location provider
     */
    private void updateLocationToggles() {
        if (mAgpsCB != null && agpsInterface != null) {
            Log.d("@M_" + TAG, "[agps_sky] agpsSettingsEnter screen check Agps status = "
                + agpsInterface.getAgpsConfig().agpsSetting.agpsEnable);
            mAgpsCB.setChecked(agpsInterface.getAgpsConfig().agpsSetting.agpsEnable);
            mAgpsCB.setEnabled(true);
        }
    }

    public Dialog onCreateDialog(int id) {

        Dialog dialog = null;
        switch (id) {
        case CONFIRM_AGPS_DIALOG_ID:
            dialog = new AlertDialog.Builder(this)
                    .setTitle(R.string.agps_enable_confirm_title)
                    .setMessage(R.string.agps_enable_confirm)
                    .setIcon(com.android.internal.R.drawable.ic_dialog_alert)
                    .setPositiveButton(R.string.agps_enable_confirm_allow,
                            new DialogInterface.OnClickListener() {
                                public void onClick(
                                        DialogInterface dialoginterface, int i) {
                                    mAgpsCB.setChecked(true);
                                    agpsInterface.setAgpsEnabled(true);
                                    Log.d("@M_" + TAG, "agps_sky onCreateDialog setPositiveButton onClick "
                                        + "setAgpsEnabled(true)");
                                }
                            })
                    .setNegativeButton(R.string.agps_enable_confirm_deny,
                            new DialogInterface.OnClickListener() {
                                public void onClick(
                                        DialogInterface dialoginterface, int i) {
                                    // / M: see cr [alps00339992] enabe a-gps
                                    // should be disabeld when you tap cancel
                                    Log.d("@M_" + TAG, "agps_sky mAgpsMgr.getStatus()"
                                            + agpsInterface.getAgpsConfig().agpsSetting.agpsEnable);
                                    if (!agpsInterface.getAgpsConfig().agpsSetting.agpsEnable) {
                                        mAgpsCB.setChecked(false);
                                        Log.d("@M_" + TAG, "agps_sky onCreateDialog setNegativeButton onClick "
                                        + "setChecked(false)");
                                    }
                                    Log.i("@M_" + TAG, "DenyDenyDeny");
                                }
                            }).create();
            break;
        default:
            break;
        }
        return dialog;
    }

}
