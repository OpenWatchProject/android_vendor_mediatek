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

import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.os.Bundle;
import android.preference.CheckBoxPreference;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceScreen;
import android.util.Log;
import android.preference.PreferenceActivity;

import com.mediatek.lbs.em2.utils.AgpsInterface;

import java.io.IOException;

public class AgpsSettings extends PreferenceActivity implements Preference.OnPreferenceChangeListener {
    private static final String XLOGTAG = "Settings/Agps";

    private static final String KEY_ABOUT_AGPS = "about_agps";
    private static final String NETWORK_INITIATE = "Network_Initiate";
    // only local or local + Roaming
    private static final String NETWORK_USED = "Network_Used";

    private CheckBoxPreference mNetworkInitiateCB;
    private ListPreference mNetworkUsedListPref;
    private Preference mAboutPref;

    private static final int ABOUT_AGPS_DIALOG_ID = 0;
    private static final int ROAMING_ALERT_DIALOG_ID = 1;
    private static final int SLOT_ALL = -1;

    private AgpsInterface agpsInterface;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        log("onCreate");

        try {
            agpsInterface = new AgpsInterface();
        } catch (IOException e) {
            log("agpsInterface exception: " + e);
            e.printStackTrace();
        }

        if (agpsInterface == null) {
            log("ERR: getSystemService failed agpsInterface=" + agpsInterface);
            return;
        }

        addPreferencesFromResource(R.xml.agps_settings);
        initPreference();
    }

    @Override
    protected boolean isValidFragment(String fragmentName) {
        return false;
    }

    @Override
    public void onPause() {
        super.onPause();
        log("^_^ onPause");
    }

    @Override
    public void onResume() {
        super.onResume();
        log("onResume");
        updatePage();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        log("^_^ onDestroy");
    }

    private void updatePage() {
        log("agps_sky ^_^ updatePage isroaming = " + agpsInterface.getAgpsConfig().upSetting.roaming);
        if (agpsInterface.getAgpsConfig().upSetting.roaming) {
            mNetworkUsedListPref.setSummary(R.string.Network_Local_and_Roaming_Summary);
            mNetworkUsedListPref.setValueIndex(1);
        } else {
            mNetworkUsedListPref.setSummary(R.string.Network_Only_Local_Summary);
            mNetworkUsedListPref.setValueIndex(0);
        }
        log("agps_sky updatePage " + agpsInterface.getAgpsConfig().curSuplProfile.name);
        mNetworkInitiateCB.setChecked(agpsInterface.getAgpsConfig().upSetting.niRequest);
        log("agps_sky updatePage niRequest = " + agpsInterface.getAgpsConfig().upSetting.niRequest);
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference) {
        super.onPreferenceTreeClick(preferenceScreen, preference);

        if ((preference.getKey()).equals(NETWORK_INITIATE)) {
            CheckBoxPreference niCheckBox = (CheckBoxPreference) preference;
            agpsInterface.setAllowNI(niCheckBox.isChecked());
            log("agps_sky onPreferenceTreeClick set NETWORK_INITIATE = " + niCheckBox.isChecked());
        } else if (mAboutPref != null && mAboutPref.getKey().equals(preference.getKey())) {
            showDialog(ABOUT_AGPS_DIALOG_ID);
        }
        return false;
    }

    private void initPreference() {
        mNetworkInitiateCB = (CheckBoxPreference) findPreference(NETWORK_INITIATE);
        mNetworkUsedListPref = (ListPreference) findPreference(NETWORK_USED);
        mNetworkUsedListPref.setOnPreferenceChangeListener(this);
        // About A-GPS
        mAboutPref = (Preference) findPreference(KEY_ABOUT_AGPS);
    }

    /**
     * onPreferenceChange
     *
     * @param preference  Preference
     * @param value  Object
     * @return boolean
     */
    public boolean onPreferenceChange(Preference preference, Object value) {

        final String key = preference.getKey();

        if (mNetworkUsedListPref.getKey().equals(key)) {
            int index = mNetworkUsedListPref.findIndexOfValue(value.toString());
            if (index == 0) {
                agpsInterface.setAllowRoaming(false);
                log("agps_sky onPreferenceChange setAllowRoaming(false)");
                updatePage();
            } else if (index == 1) {
                log("agps_sky onPreferenceChange roaming = " + agpsInterface.getAgpsConfig().upSetting.roaming);
                if (!agpsInterface.getAgpsConfig().upSetting.roaming) {
                    showDialog(ROAMING_ALERT_DIALOG_ID);
                }
            }
        }
        return true;
    }

    /**
     * onCreateDialog invoke when create a dialog
     *
     * @param id
     *            int
     * @return Dialog object
     */
    public Dialog onCreateDialog(int id) {
        Dialog dialog = null;
        if (id == ABOUT_AGPS_DIALOG_ID) {
            dialog = new AlertDialog.Builder(this).setTitle(R.string.about_agps_title)
                    .setIcon(com.android.internal.R.drawable.ic_dialog_info).setMessage(R.string.about_agps_message)
                    .setPositiveButton(R.string.agps_OK, null).create();
        } else if (id == ROAMING_ALERT_DIALOG_ID) {
            dialog = new AlertDialog.Builder(this).setTitle(R.string.Network_Roaming_dialog_title)
                    .setIcon(com.android.internal.R.drawable.ic_dialog_alert)
                    .setMessage(R.string.Network_Roaming_dialog_content)
                    .setPositiveButton(R.string.agps_OK, new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialoginterface, int i) {
                            log("agps_sky onCreateDialog setPositiveButton setAllowRoaming(true)");
                            agpsInterface.setAllowRoaming(true);
                            updatePage();
                        }
                    }).setNegativeButton(R.string.agps_enable_confirm_deny, new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialoginterface, int i) {
                            updatePage();
                        }
                    }).create();
            updatePage();
        } else {
            log("WARNING: onCreateDialog unknown id recv");
        }
        return dialog;
    }

    private void log(String msg) {
        Log.d("@M_" + XLOGTAG, "[AGPS Setting] " + msg);
    }

}
