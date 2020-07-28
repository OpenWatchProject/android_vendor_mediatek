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

package com.mediatek.engineermodecmas;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.preference.CheckBoxPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceManager;
import android.util.Log;
import android.widget.Toast;

import com.android.internal.telephony.PhoneConstants;

public class CmasTabPageActivity extends PreferenceActivity implements
        Preference.OnPreferenceChangeListener {
    private static final String TAG = "EM/CmasSettings";
    private static final String PACKAGE_NAME = "com.mediatek.cellbroadcastreceiver";
    private static final String PREF_NAME = "com.mediatek.cellbroadcastreceiver_preferences";
    private static final String CMAS_RMT_KEY = "enable_cmas_rmt_support";
    private static final String CMAS_EXERCISE_KEY = "enable_cmas_exercise_support";
    private  static final String CMAS_OPERATOR_CHOICE_KEY
                                                = "enable_cmas_operator_choice_support";
    private static final String RMT_KEY = "rmtkey";
    private static final String EXERCISE_KEY = "exerkey";
    private static final String RMT_KEY_ACTION = "com.mediatek.cmasengmode.rmtkey";
    private static final String EXERCISE_KEY_ACTION = "com.mediatek.cmasengmode.exerkey";
    private static final  String OPERATOR_CHOICE_KEY = "operatorchoicekey";
    private static final String OPERATOR_CHOICE_KEY_ACTION
                            = "com.mediatek.cmasengmode.operatorChoicekey";
    private CheckBoxPreference mCheckBoxOperatorChoice;
    private Context mCmasContext;
    private CheckBoxPreference mCheckBoxRmt;
    private CheckBoxPreference mCheckBoxExercise;
    private int slotId = 0;
    private String slotKey = "";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.i("@M_" + TAG, "onCreate()");
        addPreferencesFromResource(R.xml.cmas_setting);
        slotId = getIntent().getIntExtra("SLOTID", 0);
        slotKey = slotId + "_";
        mCheckBoxRmt = (CheckBoxPreference) findPreference(CMAS_RMT_KEY);
        mCheckBoxExercise = (CheckBoxPreference) findPreference(CMAS_EXERCISE_KEY);
        mCheckBoxOperatorChoice
                    = (CheckBoxPreference) findPreference(CMAS_OPERATOR_CHOICE_KEY);
        try {
            mCmasContext = createPackageContext(PACKAGE_NAME, 0);
        } catch (PackageManager.NameNotFoundException e) {
            Log.i("@M_" + TAG, "CMAS App not installed");
            Toast.makeText(this, "CMAS App not installed.", Toast.LENGTH_SHORT).show();
            getPreferenceScreen().setEnabled(false);
            return;
        }

        mCheckBoxRmt.setOnPreferenceChangeListener(this);
        mCheckBoxExercise.setOnPreferenceChangeListener(this);
        mCheckBoxOperatorChoice.setOnPreferenceChangeListener(this);
        Log.i("@M_" + TAG, "onCreate() End");
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.i("@M_" + TAG, "onResume()");

        if (mCmasContext == null) {
            return;
        }

        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);

        boolean rmtValue = prefs.getBoolean(slotKey + CMAS_RMT_KEY, false);
        boolean exerciseValue = prefs.getBoolean(slotKey + CMAS_EXERCISE_KEY, false);
        boolean operatorChoiceValue = prefs.getBoolean(slotKey + CMAS_OPERATOR_CHOICE_KEY, false);

        mCheckBoxRmt.setChecked(rmtValue);
        mCheckBoxExercise.setChecked(exerciseValue);
        mCheckBoxOperatorChoice.setChecked(operatorChoiceValue);
        Log.i("@M_" + TAG, "onResume()" + rmtValue + exerciseValue + operatorChoiceValue);

    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        SharedPreferences.Editor editor = PreferenceManager.
                            getDefaultSharedPreferences(this).edit();
        final Intent intent = new Intent();
        intent.putExtra(PhoneConstants.SLOT_KEY, slotId);
        if (preference.getKey().equals(CMAS_RMT_KEY)) {
            Log.i("@M_" + TAG, "onPreferenceChange(): CMAS RMT " + newValue);
            editor.putBoolean(slotKey + CMAS_RMT_KEY, (Boolean) newValue);
            intent.setAction(RMT_KEY_ACTION);
            intent.putExtra(RMT_KEY, (Boolean) newValue);
        } else if (preference.getKey().equals(CMAS_EXERCISE_KEY)) {
            Log.i("@M_" + TAG, "onPreferenceChange(): CMAS Exercise" + newValue);
            editor.putBoolean(slotKey + CMAS_EXERCISE_KEY, (Boolean) newValue);
            intent.setAction(EXERCISE_KEY_ACTION);
            intent.putExtra(EXERCISE_KEY, (Boolean) newValue);
        } else if (preference.getKey().equals(CMAS_OPERATOR_CHOICE_KEY)) {
            Log.d("@M_" + TAG,
                  "onPreferenceChange(): CMAS_OPERATOR_CHOICE_KEY" + newValue);
            editor.putBoolean(slotKey + CMAS_OPERATOR_CHOICE_KEY, (Boolean) newValue);
            intent.setAction(OPERATOR_CHOICE_KEY_ACTION);
            intent.putExtra(OPERATOR_CHOICE_KEY, (Boolean) newValue);
        }
        editor.commit();
        intent.setPackage("com.mediatek.cellbroadcastreceiver");
        sendBroadcast(intent);
        return true;
    }

}
