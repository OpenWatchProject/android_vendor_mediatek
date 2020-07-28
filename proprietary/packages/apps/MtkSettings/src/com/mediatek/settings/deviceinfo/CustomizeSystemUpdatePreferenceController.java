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
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.os.UserManager;
import androidx.preference.Preference;
import androidx.preference.PreferenceScreen;
import android.util.Log;

import com.android.settings.core.PreferenceControllerMixin;
import com.android.settingslib.core.AbstractPreferenceController;
import com.mediatek.settings.FeatureOption;

import java.util.List;

public class CustomizeSystemUpdatePreferenceController extends AbstractPreferenceController
        implements PreferenceControllerMixin {

    private static final String TAG = "CustSysUpdatePrefContr";

    private static final String KEY_CUSTOMIZE_SYSTEM_UPDATE_SETTINGS = "mtk_system_update";

    private final UserManager mUm;

    public CustomizeSystemUpdatePreferenceController(Context context, UserManager um) {
        super(context);
        mUm = um;
    }

    @Override
    public boolean isAvailable() {
        return mUm.isAdminUser() && isCustomizedSystemUpdateAvalible();
    }

    public static boolean isCustomizedSystemUpdateAvalible() {
        return FeatureOption.MTK_SYSTEM_UPDATE_SUPPORT;
    }

    @Override
    public String getPreferenceKey() {
        return KEY_CUSTOMIZE_SYSTEM_UPDATE_SETTINGS;
    }

    @Override
    public void displayPreference(PreferenceScreen screen) {
        if (isAvailable()) {
            updatePreferenceToSpecificActivityOrRemove(mContext, screen, getPreferenceKey());
        } else {
            setVisible(screen, getPreferenceKey(), false /* visible */);
        }
    }

    private void updatePreferenceToSpecificActivityOrRemove(Context context,
            PreferenceScreen screen, String preferenceKey) {
        Preference preference = screen.findPreference(preferenceKey);
        if (preference == null) {
            return;
        }

        Intent intent = null;
        if (FeatureOption.MTK_SYSTEM_UPDATE_SUPPORT) {
            intent = new Intent(Intent.ACTION_MAIN, null);
            intent.setClassName("com.mediatek.systemupdate",
                    "com.mediatek.systemupdate.MainEntry");
        }
        if (intent != null) {
            // Find the activity that is in the system image
            PackageManager pm = context.getPackageManager();
            List<ResolveInfo> list = pm.queryIntentActivities(intent, 0);
            int listSize = list.size();
            for (int i = 0; i < listSize; i++) {
                ResolveInfo resolveInfo = list.get(i);
                if ((resolveInfo.activityInfo.applicationInfo.flags & ApplicationInfo.FLAG_SYSTEM)
                        != 0) {
                    // Replace the intent with this specific activity
                    preference.setIntent(new Intent().setClassName(
                            resolveInfo.activityInfo.packageName,
                            resolveInfo.activityInfo.name));
                    CharSequence title = resolveInfo.loadLabel(pm);
                    preference.setTitle(title);
                    Log.d(TAG, "KEY_MTK_SYSTEM_UPDATE : " + title);
                    return;
                }
            }
        }
        // Did not find a matching activity, so remove the preference
        screen.removePreference(preference);
    }

    @Override
    public boolean handlePreferenceTreeClick(Preference preference) {
        if (KEY_CUSTOMIZE_SYSTEM_UPDATE_SETTINGS.equals(preference.getKey())) {
            systemUpdateEntrance(preference);
            return true;
        }
        return false;
    }

    private void systemUpdateEntrance(Preference preference) {
        if (FeatureOption.MTK_SYSTEM_UPDATE_SUPPORT) {
            startActivity("com.mediatek.systemupdate",
                    "com.mediatek.systemupdate.MainEntry");
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
