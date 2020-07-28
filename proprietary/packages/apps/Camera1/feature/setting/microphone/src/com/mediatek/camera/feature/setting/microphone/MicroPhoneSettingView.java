/*
 * Copyright Statement:
 *
 *   This software/firmware and related documentation ("MediaTek Software") are
 *   protected under relevant copyright laws. The information contained herein is
 *   confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 *   the prior written permission of MediaTek inc. and/or its licensors, any
 *   reproduction, modification, use or disclosure of MediaTek Software, and
 *   information contained herein, in whole or in part, shall be strictly
 *   prohibited.
 *
 *   MediaTek Inc. (C) 2016. All rights reserved.
 *
 *   BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *   THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *   RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 *   ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 *   WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 *   WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 *   NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 *   RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *   INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 *   TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 *   RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 *   OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 *   SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 *   RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 *   STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 *   ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 *   RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 *   MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 *   CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *   The following software/firmware and/or related documentation ("MediaTek
 *   Software") have been modified by MediaTek Inc. All revisions are subject to
 *   any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.camera.feature.setting.microphone;

import android.preference.Preference;
import android.preference.PreferenceFragment;

import com.mediatek.camera.R;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.preference.SwitchPreference;
import com.mediatek.camera.common.setting.ICameraSettingView;

/**
 * MicroPhone setting view.
 */

public class MicroPhoneSettingView implements ICameraSettingView {
    private static final LogUtil.Tag TAG =
            new LogUtil.Tag(MicroPhoneSettingView.class.getSimpleName());
    private static final String KEY_MICROPHONE = "key_microphone";
    private SwitchPreference mPref;
    private OnMicroViewListener mViewListener;
    private boolean mChecked;
    private boolean mEnabled;

    /**
     * Listener with MicroPhone view.
     */
    interface OnMicroViewListener {
        void onItemViewClick(boolean isOn);
        boolean onCachedValue();
    }

    @Override
    public void loadView(PreferenceFragment fragment) {
        LogHelper.d(TAG, "[loadView] + ");
        fragment.addPreferencesFromResource(R.xml.microphone_preference);
        mPref = (SwitchPreference) fragment.findPreference(KEY_MICROPHONE);
        mPref.setRootPreference(fragment.getPreferenceScreen());
        mPref.setId(R.id.microphone_setting);
        mPref.setContentDescription(fragment.getActivity().getResources()
                .getString(R.string.microphone_content_description));
        mPref.setOnPreferenceChangeListener(new Preference.OnPreferenceChangeListener() {
            @Override
            public boolean onPreferenceChange(Preference preference, Object object) {
                boolean value = (Boolean) object;
                mChecked = value;
                mViewListener.onItemViewClick(value);
                return true;
            }
        });
        mPref.setChecked(mViewListener.onCachedValue());
        mPref.setEnabled(mEnabled);
        LogHelper.d(TAG, "[loadView] - ");
    }

    @Override
    public void refreshView() {
        if (mPref != null) {
            mPref.setChecked(mChecked);
            mPref.setEnabled(mEnabled);
        }
    }

    @Override
    public void unloadView() {
        LogHelper.d(TAG, "[unloadView]");
    }

    @Override
    public void setEnabled(boolean enabled) {
        mEnabled = enabled;
    }

    @Override
    public boolean isEnabled() {
        return mEnabled;
    }

    /**
     * This is to set microphone view update listener.
     * @param  viewListener the microphone view listener.
     */
    public void setMicroViewListener(OnMicroViewListener viewListener) {
        mViewListener = viewListener;
    }
    /**
     * Set microphone state.
     *
     * @param checked True means microphone is opened, false means microphone is closed.
     */
    public void setChecked(boolean checked) {
        mChecked = checked;
        refreshView();
    }
}
