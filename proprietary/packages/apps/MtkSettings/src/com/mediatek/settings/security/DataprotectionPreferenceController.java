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
package com.mediatek.settings.security;

import android.content.Context;
import android.content.Intent;
import android.support.v7.preference.Preference;

import com.android.settingslib.core.AbstractPreferenceController;

import java.util.List;

public class DataprotectionPreferenceController extends AbstractPreferenceController {

    private static final String KEY_DATA_SETTING = "data_protection_key";
    private static final String DATA_MAIN_ACTIVITY =
            "com.mediatek.dataprotection.ACTION_START_MAIN";

    public DataprotectionPreferenceController(Context context) {
        super(context);
    }

    @Override
    public boolean isAvailable() {
        Intent intent = new Intent(DATA_MAIN_ACTIVITY);
        return mContext.getPackageManager().resolveActivity(intent, 0) != null;
    }

    @Override
    public String getPreferenceKey() {
        return KEY_DATA_SETTING;
    }
}
