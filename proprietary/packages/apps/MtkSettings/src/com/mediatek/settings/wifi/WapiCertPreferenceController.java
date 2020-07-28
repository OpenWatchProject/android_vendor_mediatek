/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the
 * License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */
package com.mediatek.settings.wifi;

import android.content.Context;
import android.content.ComponentName;
import android.content.pm.PackageManager;
import android.support.v7.preference.Preference;
import android.support.v7.preference.PreferenceScreen;
import android.util.Log;

import com.android.settingslib.core.AbstractPreferenceController;
import com.android.settings.R;

import com.mediatek.settings.FeatureOption;

public class WapiCertPreferenceController extends AbstractPreferenceController {

    private static final String TAG = "WapiCertPreferenceController";
    private static final String KEY_WAPI_CERT_PREF = "wapi_cert_manage";
    private static final String WAPI_CERT_PKG_NAME = "com.wapi.wapicertmanager";
    private static final String WAPI_CERT_CLASS_NAME = "com.wapi.wapicertmanager.WapiCertManagerActivity";

    private Context mContext;

    public WapiCertPreferenceController(Context context) {
        super(context);
        mContext = context;
    }

    @Override
    public boolean isAvailable() {
        return FeatureOption.MTK_WAPI_SUPPORT && isWapiCertPackageExist();
    }

    @Override
    public String getPreferenceKey() {
        return KEY_WAPI_CERT_PREF;
    }

    private boolean isWapiCertPackageExist() {
        if (mContext != null){
            ComponentName componentName = new ComponentName(WAPI_CERT_PKG_NAME, WAPI_CERT_CLASS_NAME);
            try {
                mContext.getPackageManager().getActivityInfo(componentName, 0);
            } catch (PackageManager.NameNotFoundException e) {
                Log.d(TAG, "package exist: false");
                return false;
            }
        }
        Log.d(TAG, "package exist: true");
        return true;
    }

}
