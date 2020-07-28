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

package com.android.phone;

import android.os.Bundle;
import android.os.PersistableBundle;
import android.os.SystemProperties;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.telephony.CarrierConfigManager;
import android.view.MenuItem;

import com.android.internal.telephony.PhoneConstants;
import com.mediatek.settings.vtss.GsmUmtsVTCBOptions;
import com.mediatek.settings.vtss.GsmUmtsVTCFOptions;

import mediatek.telephony.MtkCarrierConfigManager;

public class GsmUmtsCallOptions extends PreferenceActivity implements
        PhoneGlobals.SubInfoUpdateListener {
    private static final String LOG_TAG = "GsmUmtsCallOptions";
    private final boolean DBG = true; //(PhoneGlobals.DBG_LEVEL >= 2);

    public static final String CALL_FORWARDING_KEY = "call_forwarding_key";
    public static final String CALL_BARRING_KEY = "call_barring_key";
    private static final String ADDITIONAL_GSM_SETTINGS_KEY = "additional_gsm_call_settings_key";

    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);

        addPreferencesFromResource(R.xml.gsm_umts_call_options);

        SubscriptionInfoHelper subInfoHelper = new SubscriptionInfoHelper(this, getIntent());
        subInfoHelper.setActionBarTitle(
                getActionBar(), getResources(), R.string.labelGsmMore_with_label);
        init(getPreferenceScreen(), subInfoHelper);

        if (subInfoHelper.getPhone().getPhoneType() != PhoneConstants.PHONE_TYPE_GSM) {
            //disable the entire screen
            getPreferenceScreen().setEnabled(false);
        }
        /// M: For hot swap @{
        PhoneGlobals.getInstance().addSubInfoUpdateListener(this);
        /// @}
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        final int itemId = item.getItemId();
        if (itemId == android.R.id.home) {
            onBackPressed();
            return true;
        }
        return super.onOptionsItemSelected(item);
    }

    public static void init(PreferenceScreen prefScreen, SubscriptionInfoHelper subInfoHelper) {
        PersistableBundle b = null;
        if (subInfoHelper.hasSubId()) {
            b = PhoneGlobals.getInstance().getCarrierConfigForSubId(subInfoHelper.getSubId());
        } else {
            b = PhoneGlobals.getInstance().getCarrierConfig();
        }

        Preference callForwardingPref = prefScreen.findPreference(CALL_FORWARDING_KEY);
        if (callForwardingPref != null) {
            if (b != null && b.getBoolean(
                    CarrierConfigManager.KEY_CALL_FORWARDING_VISIBILITY_BOOL)) {
                /// M: Add for vilte SS feature. @{
                if (b.getBoolean(MtkCarrierConfigManager.MTK_KEY_SUPPORT_VT_SS_BOOL)
                        || SystemProperties.get("persist.vendor.video.ss_dbg").equals("1")) {
                    callForwardingPref.setIntent(
                            subInfoHelper.getIntent(GsmUmtsVTCFOptions.class));
                } else {
                    callForwardingPref.setIntent(
                            subInfoHelper.getIntent(GsmUmtsCallForwardOptions.class));
                }
                /// @}

            } else {
                prefScreen.removePreference(callForwardingPref);
            }
        }

        Preference additionalGsmSettingsPref =
                prefScreen.findPreference(ADDITIONAL_GSM_SETTINGS_KEY);
        if (additionalGsmSettingsPref != null) {
            if (b != null && (b.getBoolean(
                    CarrierConfigManager.KEY_ADDITIONAL_SETTINGS_CALL_WAITING_VISIBILITY_BOOL)
                    || b.getBoolean(
                    CarrierConfigManager.KEY_ADDITIONAL_SETTINGS_CALLER_ID_VISIBILITY_BOOL))) {
                additionalGsmSettingsPref.setIntent(
                        subInfoHelper.getIntent(GsmUmtsAdditionalCallOptions.class));
            } else {
                prefScreen.removePreference(additionalGsmSettingsPref);
            }
        }

        Preference callBarringPref = prefScreen.findPreference(CALL_BARRING_KEY);
        if (callBarringPref != null) {
            if (b != null && b.getBoolean(CarrierConfigManager.KEY_CALL_BARRING_VISIBILITY_BOOL)) {
                /// M: Add for vilte SS feature. @{
                if (b.getBoolean(MtkCarrierConfigManager.MTK_KEY_SUPPORT_VT_SS_BOOL)
                        || SystemProperties.get("persist.vendor.video.ss_dbg").equals("1")) {
                    callBarringPref.setIntent(
                            subInfoHelper.getIntent(GsmUmtsVTCBOptions.class));
                } else {
                    callBarringPref.setIntent(
                            subInfoHelper.getIntent(GsmUmtsCallBarringOptions.class));
                }
                /// @}
            } else {
                prefScreen.removePreference(callBarringPref);
            }
        }
    }

    /// M: For hot swap @{
    @Override
    protected void onDestroy() {
        PhoneGlobals.getInstance().removeSubInfoUpdateListener(this);
        super.onDestroy();
    }

    @Override
    public void handleSubInfoUpdate() {
        finish();
    }
    /// @}
}
