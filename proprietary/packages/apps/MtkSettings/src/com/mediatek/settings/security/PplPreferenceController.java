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

package com.mediatek.settings.security;

import android.content.Context;
import androidx.preference.PreferenceGroup;
import androidx.preference.PreferenceScreen;
import android.util.Log;

import com.android.settingslib.core.AbstractPreferenceController;
import com.android.settingslib.core.lifecycle.Lifecycle;
import com.android.settingslib.core.lifecycle.LifecycleObserver;
import com.android.settingslib.core.lifecycle.events.OnPause;
import com.android.settingslib.core.lifecycle.events.OnResume;

import com.mediatek.settings.ext.IPplSettingsEntryExt;
import com.mediatek.settings.ext.DefaultPplSettingsEntryExt;


public class PplPreferenceController extends AbstractPreferenceController
        implements LifecycleObserver, OnResume, OnPause {

    private static final String TAG = "PPL/PplPrefContr";
    private static final String KEY_PPL_PREF = "privacy_protection_lock";
    private static final String KEY_DEVICE_ADMIN_CATEGORY =
            "security_settings_device_admin_category";

    private IPplSettingsEntryExt mPplSettingsEntryExt;

    public PplPreferenceController(Context context, Lifecycle lifecycle) {
        super(context);
        mPplSettingsEntryExt = DefaultPplSettingsEntryExt.getInstance(context);
        if (lifecycle != null) {
            lifecycle.addObserver(this);
        }
    }

    @Override
    public boolean isAvailable() {
        return mPplSettingsEntryExt != null;
    }

    @Override
    public String getPreferenceKey() {
        return KEY_PPL_PREF;
    }

    @Override
    public void displayPreference(PreferenceScreen screen) {
        if (mPplSettingsEntryExt != null) {
            PreferenceGroup deviceAdminCategory =
                    (PreferenceGroup)screen.findPreference(KEY_DEVICE_ADMIN_CATEGORY);
            mPplSettingsEntryExt.addPplPrf(deviceAdminCategory);
        } else {
            Log.e(TAG, "[displayPreference] mPplSettingsEntryExt should not be null !!!");
        }
    }

    @Override
    public void onResume() {
        if (mPplSettingsEntryExt != null) {
            mPplSettingsEntryExt.enablerResume();
        } else {
            Log.e(TAG, "[onResume] mPplSettingsEntryExt should not be null !!!");
        }
    }

    @Override
    public void onPause() {
        if(mPplSettingsEntryExt != null) {
            mPplSettingsEntryExt.enablerPause();
        } else {
            Log.e(TAG, "[onPause] mPplSettingsEntryExt should not be null !!!");
        }
    }
}
