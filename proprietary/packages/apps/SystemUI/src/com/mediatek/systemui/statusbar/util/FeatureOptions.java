/*
* Copyright (C) 2011-2017 MediaTek Inc.
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
package com.mediatek.systemui.statusbar.util;

import android.os.SystemProperties;
import android.util.Log;


public class FeatureOptions {

    private static final String TAG = "FeatureOptions";
    // GMO support.
    public static final boolean LOW_RAM_SUPPORT = isPropertyEnabledBoolean("ro.config.low_ram");
    // CTA feature support
    public static final boolean MTK_CTA_SET = isPropertyEnabledInt("ro.vendor.mtk_cta_set");

    // Build Type
    public static final String BUILD_TYPE = "ro.build.type";
    public static final String BUILD_TYPE_ENG = "eng";
    public static final String BUILD_TYPE_USERDEBUG = "userdebug";
    public static final String BUILD_TYPE_USER = "user";
    public static final boolean LOG_ENABLE =
            SystemProperties.get(BUILD_TYPE).equals(BUILD_TYPE_ENG) ||
            SystemProperties.get(BUILD_TYPE).equals(BUILD_TYPE_USERDEBUG);
    public static final boolean LOG_ENG_ENABLE =
        SystemProperties.get(BUILD_TYPE).equals(BUILD_TYPE_ENG);
    /**
     *
     * @param propertyString
     * @return true, property is enable.
     */
    private static boolean isPropertyEnabledBoolean(String propertyString) {
        return "true".equals(SystemProperties.get(propertyString, "false"));
    }

    /**
     *
     * @param propertyString
     * @return true, property is enable.
     */
    private static boolean isPropertyEnabledInt(String propertyString) {
        return "1".equals(SystemProperties.get(propertyString));
    }

    /**
     * Whether is Eng Load.
     * @return True if is Eng Load.
     */
    public static final boolean isEngLoad() {
        return SystemProperties.get(BUILD_TYPE).equals(BUILD_TYPE_ENG);
    }
}
