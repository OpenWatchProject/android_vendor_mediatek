/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 */

package com.mediatek.cellbroadcastreceiver;

import java.util.List;
import android.app.Activity;
import android.os.Bundle;
import android.preference.PreferenceFragment;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.util.Log;

/**
 * Settings activity for the cell broadcast receiver.
 */
public class CellBroadcastMainSettings extends Activity {
    private static final String TAG = "[CMAS]CellBroadcastMainSettings";

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        int settingProfileId = CmasConfigManager.getSettingProfileId();
        Log.d(TAG, "onCreate, setting profile id = " + settingProfileId);
        PreferenceFragment fragment = new CellBroadcastCommonSettingFragment();
        switch (settingProfileId) {
        case CmasConfigManager.PROFILE_ID_TW:
            fragment = new CellBroadcastPwsSettingFragment();
            break;
        case CmasConfigManager.PROFILE_ID_TRA:
            fragment = new CellBroadcastTraSettingFragment();
            break;
        case CmasConfigManager.PROFILE_ID_COMMON:
            //dual sim setting
            List<SubscriptionInfo> subInfoList
                = SubscriptionManager.from(this).getActiveSubscriptionInfoList();
            int subCount = (subInfoList != null && !subInfoList.isEmpty()) ? subInfoList.size() : 0;
            Bundle b = new Bundle();
            if (CmasConfigManager.getGeminiSupport()) {
                if (subCount == 2) {
                    fragment = new CellBroadcastGeminiSettingFragment();
                } else if (subCount == 1) {
                    b.putInt("slotid", subInfoList.get(0).getSimSlotIndex());
                } else {
                    b.putInt("slotid", 0);
                }
            } else {
                b.putInt("slotid", 0);
            }
            b.putBoolean("removeCommonSetting", false);
            fragment.setArguments(b);
            break;
        default:
            break;
        }
        getFragmentManager().beginTransaction().add(android.R.id.content, fragment).commit();
    }

}
