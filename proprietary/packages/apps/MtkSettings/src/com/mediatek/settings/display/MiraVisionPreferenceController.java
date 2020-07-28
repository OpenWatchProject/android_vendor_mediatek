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
import android.content.Intent;
import androidx.preference.Preference;

import com.android.settings.core.PreferenceControllerMixin;
import com.android.settingslib.core.AbstractPreferenceController;
import com.mediatek.settings.FeatureOption;

public class MiraVisionPreferenceController extends AbstractPreferenceController
        implements PreferenceControllerMixin {

    private static final String KEY_MIRA_VISION = "mira_vision";

    public MiraVisionPreferenceController(Context context) {
        super(context);
    }

    @Override
    public boolean isAvailable() {
        return (FeatureOption.MTK_MIRAVISION_SETTING_SUPPORT
                && !(android.os.UserHandle.myUserId() != 0 && FeatureOption.MTK_PRODUCT_IS_TABLET));
    }

    @Override
    public String getPreferenceKey() {
        return KEY_MIRA_VISION;
    }
}
