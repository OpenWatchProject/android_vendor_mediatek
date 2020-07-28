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
package com.mediatek.settings.display;

import android.content.Context;
import androidx.preference.Preference;
import androidx.preference.PreferenceScreen;
import android.util.Log;

import com.android.settings.core.PreferenceControllerMixin;
import com.android.settings.R;
import com.android.settingslib.core.AbstractPreferenceController;

import com.mediatek.hdmi.HdimReflectionHelper;

public class HdmiPreferenceController extends AbstractPreferenceController implements
        PreferenceControllerMixin {

    private static final String TAG = "HdmiPrefContr";
    private static final String KEY_HDMI_PREF = "hdmi_settings";

    private Object mHdmiManager;

    public HdmiPreferenceController(Context context) {
        super(context);
    }

    @Override
    public boolean isAvailable() {
        Class contextClass = Context.class;
        mHdmiManager = HdimReflectionHelper.getHdmiService();
        return mHdmiManager != null;
    }

    @Override
    public String getPreferenceKey() {
        return KEY_HDMI_PREF;
    }

    @Override
    public void displayPreference(PreferenceScreen screen) {
        if (!isAvailable()) {
            setVisible(screen, getPreferenceKey(), false /* visible */);
            return;
        }
        Preference preference = screen.findPreference(getPreferenceKey());
        if (mHdmiManager != null) {
            String hdmi = mContext.getString(R.string.hdmi_replace_hdmi);
            int typeMhl = HdimReflectionHelper.getHdmiDisplayTypeConstant("DISPLAY_TYPE_MHL");
            int typeSlimport = HdimReflectionHelper.getHdmiDisplayTypeConstant("DISPLAY_TYPE_SLIMPORT");
            int displayType = HdimReflectionHelper.getHdmiDisplayType(mHdmiManager);
            if (displayType == typeMhl) {
                String mhl = mContext.getString(R.string.hdmi_replace_mhl);
                preference.setTitle(preference.getTitle().toString().replaceAll(hdmi, mhl));
                preference.setSummary(preference.getSummary().toString().replaceAll(hdmi, mhl));
            } else if (displayType == typeSlimport) {
                String slimPort = mContext.getString(R.string.slimport_replace_hdmi);
                preference.setTitle(preference.getTitle().toString().replaceAll(hdmi, slimPort));
                preference
                        .setSummary(preference.getSummary().toString().replaceAll(hdmi, slimPort));
            }
        }
    }

}
