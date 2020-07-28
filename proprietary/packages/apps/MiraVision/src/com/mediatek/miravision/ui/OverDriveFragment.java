/*
 * Copyright (C) 2011 The Android Open Source Project
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

package com.mediatek.miravision.ui;

import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CompoundButton;

import com.mediatek.miravision.setting.MiraVisionJni;
import com.mediatek.miravision.utils.Utils;

public class OverDriveFragment extends DynamicContrastFragment {

    public static final String SHARED_PREFERENCES_OD_STATUS = "od_status";
    private static final String TAG = "Miravision/OverDriveFragment";
    private static final String VIDEO_NAME = "over_drive.mp4";

    private Utils mUtils;

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        View rootView = super.onCreateView(inflater, container, savedInstanceState);
        mVideoName = VIDEO_NAME;
        mUtils = new Utils(getActivity());
        return rootView;
    }

    @Override
    public void onResume() {
        super.onResume();
        updateOdStatus();
    }

    private void updateOdStatus() {
        Log.d(TAG, "updateOdStatus");
        mActionBarSwitch.setChecked(mUtils.getSharePrefBoolenValue(SHARED_PREFERENCES_OD_STATUS));
    }

    @Override
    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
        Log.d(TAG, "onCheckedChanged " + isChecked);
        MiraVisionJni.nativeEnableODDemo(isChecked ? 1 : 0);
        mUtils.setSharePrefBoolenValue(SHARED_PREFERENCES_OD_STATUS, isChecked);
    }
}
