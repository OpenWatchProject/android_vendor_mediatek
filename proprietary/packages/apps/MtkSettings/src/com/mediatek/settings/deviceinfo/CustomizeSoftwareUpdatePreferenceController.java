/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.mediatek.settings.deviceinfo;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.os.UserManager;
import androidx.preference.Preference;
import android.util.Log;

import com.android.settings.core.PreferenceControllerMixin;
import com.android.settingslib.core.AbstractPreferenceController;
import com.mediatek.settings.FeatureOption;

public class CustomizeSoftwareUpdatePreferenceController extends AbstractPreferenceController
        implements PreferenceControllerMixin {

    private static final String TAG = "CustSwUpdatePrefContr";

    private static final String KEY_CUSTOMIZE_SOFTWARE_UPDATE_SETTINGS = "mtk_software_update";

    private final UserManager mUm;

    public CustomizeSoftwareUpdatePreferenceController(Context context, UserManager um) {
        super(context);
        mUm = um;
    }

    @Override
    public boolean isAvailable() {
        return true;//mUm.isAdminUser() && isCustomizedSoftwareUpdateAvalible();
    }

    public static boolean isCustomizedSoftwareUpdateAvalible() {
        return true;//FeatureOption.MTK_MDM_SCOMO || FeatureOption.MTK_SCOMO_ENTRY;
    }

    @Override
    public String getPreferenceKey() {
        return KEY_CUSTOMIZE_SOFTWARE_UPDATE_SETTINGS;
    }

    @Override
    public boolean handlePreferenceTreeClick(Preference preference) {
        if (KEY_CUSTOMIZE_SOFTWARE_UPDATE_SETTINGS.equals(preference.getKey())) {
            softwareUpdateEntrance(preference);
            return true;
        }
        return false;
    }

    private void softwareUpdateEntrance(Preference preference) {
        if (FeatureOption.MTK_MDM_SCOMO) {
            startActivity("com.mediatek.mediatekdm",
                    "com.mediatek.mediatekdm.scomo.DmScomoActivity");
        } else if (FeatureOption.MTK_SCOMO_ENTRY) {
            startActivity("com.mediatek.dm",
                    "com.mediatek.dm.scomo.DmScomoActivity");
        }
    }

    private void startActivity(String className, String activityName) {
        Intent intent = new Intent(Intent.ACTION_MAIN, null);
        ComponentName cn = new ComponentName(className, activityName);
        intent.setComponent(cn);
        if (mContext.getPackageManager().resolveActivity(intent, 0) != null) {
            mContext.startActivity(intent);
        } else {
            Log.d(TAG, "Unable to start activity " + intent.toString());
        }
    }

}
