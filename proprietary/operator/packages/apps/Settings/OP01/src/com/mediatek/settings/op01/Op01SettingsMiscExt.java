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

import java.util.List;

import android.content.Context;
import androidx.preference.Preference;
import androidx.preference.PreferenceScreen;
import android.util.Log;

import com.android.settingslib.core.AbstractPreferenceController;
import com.android.settingslib.core.lifecycle.Lifecycle;

import com.mediatek.settings.ext.DefaultSettingsMiscExt;

/**
 * Other settings info plugin.
 */
public class Op01SettingsMiscExt extends DefaultSettingsMiscExt {

    private static final String KEY_GPS_SETTINGS_BUTTON = "gps_settings_button";
    private static final String KEY_RECENT_LOCATION = "recent_location_requests_see_all_button";
    private static final String TAG = "OP01SettingsMiscExt";
    private Context mContext;
    private Preference mAgpsEnterPref;

    /**
     * Init context.
     * @param context The Context
     */
    public Op01SettingsMiscExt(Context context) {
        super(context);
        mContext = context;
        mContext.setTheme(R.style.SettingsPluginBase);
        Log.d(TAG, "SettingsMiscExt");
    }

    @Override
    public void addPreferenceController(Object controllers,
            Object preferenceController) {
        Log.i(TAG, "addAGPRSPreferenceController");
        List<AbstractPreferenceController> con =  (List<AbstractPreferenceController>)controllers;
        AbstractPreferenceController agprsPreferenceController
            = (OP01AGPRSPreferenceController)preferenceController;
        con.add(agprsPreferenceController);
    }

    @Override
    public Object createPreferenceController(Context context, Object lifecycle) {
        Log.i(TAG, "createAGPRSPreferenceController");
        Lifecycle cycle = (Lifecycle) lifecycle;
        OP01AGPRSPreferenceController preferenceController
              = new OP01AGPRSPreferenceController(context, mContext, cycle);
        return preferenceController;
    }

    @Override
    public void customizeAGPRS(PreferenceScreen preferenceScreen) {
        Log.i(TAG, "customizeAGPRS add preference");
        mAgpsEnterPref = new Preference(preferenceScreen.getPreferenceManager().getContext());
        mAgpsEnterPref.setTitle(mContext.getResources().getString(R.string.gps_settings_title));
        mAgpsEnterPref.setKey(KEY_GPS_SETTINGS_BUTTON);
        //mAgpsEnterPref.setOnPreferenceClickListener(mPreferenceclickListener);
        mAgpsEnterPref.setOrder(-1);
        mAgpsEnterPref.setEnabled(false);
        preferenceScreen.addPreference(mAgpsEnterPref);
    }
}
