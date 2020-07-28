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
import android.preference.PreferenceScreen;

import com.android.internal.telephony.CommandsInterface;
import com.android.phone.CallBarringEditPreference;
import com.android.phone.CallForwardEditPreference;
import com.android.phone.R;
import com.mediatek.internal.telephony.MtkRIL;

public class GsmUmtsVTUtils {
    public static final int VIDEO_SERVICE_CLASS = MtkRIL.SERVICE_CLASS_VIDEO;
    public static final int VOICE_SERVICE_CLASS = CommandsInterface.SERVICE_CLASS_VOICE;
    public static final String SERVICE_CLASS = "service_class";
    public static final int CF_TYPE = 0;
    public static final int CB_TYPE = 1;

    private static final String BUTTON_CFU_KEY   = "button_cfu_key";
    private static final String BUTTON_CFB_KEY   = "button_cfb_key";
    private static final String BUTTON_CFNRY_KEY = "button_cfnry_key";
    private static final String BUTTON_CFNRC_KEY = "button_cfnrc_key";

    private static final String BUTTON_CALL_BARRING_KEY = "button_baoc_key";
    private static final String BUTTON_ALL_OUTING_KEY = "button_baoic_key";
    private static final String BUTTON_OUT_INTERNATIONAL_EXCEPT = "button_baoicxh_key";
    private static final String BUTTON_ALL_INCOMING_KEY = "button_baic_key";
    private static final String BUTTON_ALL_INCOMING_EXCEPT = "button_baicr_key";

    public static void setServiceClass(Intent intent, int serviceClass) {
        intent.putExtra(SERVICE_CLASS, serviceClass);
    }

    public static void setCFServiceClass(PreferenceScreen prefSet, int serviceClass) {
        CallForwardEditPreference buttonCFU;
        buttonCFU = (CallForwardEditPreference) prefSet.findPreference(BUTTON_CFU_KEY);
        buttonCFU.setServiceClass(serviceClass);

        CallForwardEditPreference buttonCFB;
        buttonCFB = (CallForwardEditPreference) prefSet.findPreference(BUTTON_CFB_KEY);
        buttonCFB.setServiceClass(serviceClass);

        CallForwardEditPreference buttonCFNRy;
        buttonCFNRy = (CallForwardEditPreference) prefSet.findPreference(BUTTON_CFNRY_KEY);
        buttonCFNRy.setServiceClass(serviceClass);

        CallForwardEditPreference buttonCFNRc;
        buttonCFNRc = (CallForwardEditPreference) prefSet.findPreference(BUTTON_CFNRC_KEY);
        buttonCFNRc.setServiceClass(serviceClass);
    }

    public static void setCBServiceClass(PreferenceScreen prefSet, int serviceClass) {
        CallBarringEditPreference callAllOutButton;
        callAllOutButton = (CallBarringEditPreference) prefSet
        .findPreference(BUTTON_CALL_BARRING_KEY);
        callAllOutButton.setServiceClass(serviceClass);

        CallBarringEditPreference callInternationalOutButton;
        callInternationalOutButton = (CallBarringEditPreference) prefSet
        .findPreference(BUTTON_ALL_OUTING_KEY);
        callInternationalOutButton.setServiceClass(serviceClass);

        CallBarringEditPreference callInternationalOutButton2;
        callInternationalOutButton2 = (CallBarringEditPreference) prefSet
        .findPreference(BUTTON_OUT_INTERNATIONAL_EXCEPT);
        callInternationalOutButton2.setServiceClass(serviceClass);

        CallBarringEditPreference callInButton;
        callInButton = (CallBarringEditPreference) prefSet
        .findPreference(BUTTON_ALL_INCOMING_KEY);
        callInButton.setServiceClass(serviceClass);

        CallBarringEditPreference callInButton2;
        callInButton2 = (CallBarringEditPreference) prefSet
        .findPreference(BUTTON_ALL_INCOMING_EXCEPT);
        callInButton2.setServiceClass(serviceClass);
    }

    public static int getActionBarResId(int serviceClass, int type) {

        int resId = R.string.actionBarCFVoice;

        if (type == CF_TYPE) {
            resId = R.string.actionBarCFVoice;
            if (serviceClass == VIDEO_SERVICE_CLASS) {
                resId = R.string.actionBarCFVideo;
            }
        } else {
            resId = R.string.actionBarCBVoice;
            if (serviceClass == VIDEO_SERVICE_CLASS) {
                resId = R.string.actionBarCBVideo;
            }
        }
        return resId;
    }
}