/*
* Copyright (C) 2011-2014 Mediatek.inc.
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
package com.mediatek.phone;

import android.os.SystemProperties;
import android.util.Log;

public class PhoneFeatureConstants {

    public static final class FeatureOption {
        private static final String TAG = "FeatureOption";
        private final static String ONE = "1";
        private final static String TWO = "2";

        public static boolean isMtkCtaSet() {
            boolean isSupport = ONE.equals(
                    SystemProperties.get("ro.vendor.mtk_cta_set")) ? true : false;
            Log.d(TAG, "isMtkCtaSet(): " + isSupport);
            return isSupport;
        }
    }
}
