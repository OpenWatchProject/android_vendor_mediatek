/*
 * Copyright (C) 2017 The Android Open Source Project
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


import android.content.Context;
import android.os.SystemProperties;
import androidx.preference.Preference;

import com.android.settings.R;
import com.android.settings.core.PreferenceControllerMixin;
import com.android.settingslib.core.AbstractPreferenceController;
import com.android.settingslib.Utils;
import com.mediatek.settings.FeatureOption;
import android.telephony.TelephonyManager;
import android.util.Log;
import java.util.Locale;

public class BasebandVersion2PreferenceController extends AbstractPreferenceController
        implements PreferenceControllerMixin {

    private static final String CDMA_BASEBAND_PROPERTY = "vendor.cdma.version.baseband";
    private static final String KEY_BASEBAND_VERSION_2 = "baseband_version_2";
    private final static String TAG = "BasebandVersion2PreferenceController";

    public BasebandVersion2PreferenceController(Context context) {
        super(context);
    }

    @Override
    public boolean isAvailable() {
        return !Utils.isWifiOnly(mContext) && needShowCdmaBasebandVersion();
    }

    @Override
    public String getPreferenceKey() {
        return KEY_BASEBAND_VERSION_2;
    }

    @Override
    public void updateState(Preference preference) {
        super.updateState(preference);
        preference.setSummary(SystemProperties.get(CDMA_BASEBAND_PROPERTY,
                mContext.getResources().getString(R.string.device_info_default)));
        preference.setTitle(this.getCustomizedBasebandTitle(mContext, getPreferenceKey()));
    }

    public static boolean needShowCdmaBasebandVersion() {
        // If C2K support but only one modem (C2K modem merged), do not show CDMA baseband version

        boolean isC2kSupport = false;
        String networkType = TelephonyManager.getTelephonyProperty(0, "ro.telephony.default_network", "");
        Log.d(TAG, "DBBG networkType : " + networkType);
        if (networkType.equals("7") || networkType.equals("8") || networkType.equals("10"))
            isC2kSupport = true;

        Log.d(TAG, "isC2kSupport : " + isC2kSupport);
        Log.d(TAG, "onlyOneModem : " + onlyOneModem());
        return (isC2kSupport && !onlyOneModem());
    }

    private static boolean onlyOneModem() {
        String value = SystemProperties.get("ro.vendor.mtk_ril_mode", "");
        return value.contains("1rild");
    }

    public static String getCustomizedBasebandTitle(Context context, String prefKey) {
        String basebandVerStr = context.getResources().getString(R.string.baseband_version);
        String customizedTitle = basebandVerStr;
        String country = Locale.getDefault().getCountry();
        boolean chineseLocale = country.equals(Locale.CHINA.getCountry())
                || country.equals(Locale.TAIWAN.getCountry());
        if (prefKey == KEY_BASEBAND_VERSION_2) {
            customizedTitle = (chineseLocale ? "CDMA" : "CDMA ") + basebandVerStr;
        } else {
            customizedTitle = (chineseLocale ? "GSM" : "GSM ") + basebandVerStr;
        }
        return customizedTitle;
    }
}
