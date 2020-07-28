/*
 * Copyright (C) 2006 The Android Open Source Project
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

package com.mediatek.settings.vtss;

import android.content.Intent;
import android.preference.Preference;

import com.android.phone.GsmUmtsCallForwardOptions;
import com.android.phone.R;
import com.android.phone.SubscriptionInfoHelper;

public class GsmUmtsVTCFOptions extends GsmUmtsVTOptions {
    private static final String LOG_TAG = "GsmUmtsVTCFOptions";

    @Override
    protected void setActionBar(SubscriptionInfoHelper subInfoHelper) {
        subInfoHelper.setActionBarTitle(
                getActionBar(), getResources(), R.string.call_forwarding_settings_with_label);
    }

    @Override
    protected void init(SubscriptionInfoHelper subInfoHelper) {
        addPreferencesFromResource(R.xml.mtk_gsm_umts_vt_cf_options);

        Preference voiceCallForwardingPref = getPreferenceScreen().findPreference(VOICE_CALL_KEY);
        Intent voiceIntent = subInfoHelper.getIntent(GsmUmtsCallForwardOptions.class);
        GsmUmtsVTUtils.setServiceClass(voiceIntent, GsmUmtsVTUtils.VOICE_SERVICE_CLASS);
        voiceCallForwardingPref.setIntent(voiceIntent);

        Preference vdieoCallForwardingPref = getPreferenceScreen().findPreference(VIDEO_CALL_KEY);
        Intent videoIntent = subInfoHelper.getIntent(GsmUmtsCallForwardOptions.class);
        GsmUmtsVTUtils.setServiceClass(videoIntent, GsmUmtsVTUtils.VIDEO_SERVICE_CLASS);
        vdieoCallForwardingPref.setIntent(videoIntent);
    }
}
