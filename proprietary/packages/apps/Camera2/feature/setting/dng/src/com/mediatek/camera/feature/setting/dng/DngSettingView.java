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

package com.mediatek.camera.feature.setting.dng;

import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.preference.PreferenceGroup;

import com.mediatek.camera.R;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.preference.SwitchPreference;
import com.mediatek.camera.common.setting.ICameraSettingView;
import com.mediatek.camera.common.setting.ISettingManager;

import java.util.List;

/**
 * Dng setting view.
 */

public class DngSettingView implements ICameraSettingView {
    private static final LogUtil.Tag TAG =
            new LogUtil.Tag(DngSettingView.class.getSimpleName());
    private SwitchPreference mPref;
    private DngViewCtrl.OnDngSettingViewListener mDngViewListener;
    private ISettingManager.SettingDevice2Requester mSettingDevice2Requester;
    private List<String> mEntryValues;
    private boolean mEnabled;


    @Override
    public void loadView(PreferenceFragment fragment) {
        LogHelper.d(TAG, "[loadView]");
        fragment.addPreferencesFromResource(R.xml.dng_preference);
        mPref = (SwitchPreference) fragment.findPreference(IDngConfig.KEY_DNG);
        mPref.setRootPreference(fragment.getPreferenceScreen());
        mPref.setId(R.id.dng_setting);
        mPref.setOnPreferenceChangeListener(new Preference.OnPreferenceChangeListener() {
            @Override
            public boolean onPreferenceChange(Preference preference, Object o) {
                if (mSettingDevice2Requester != null
                        && mSettingDevice2Requester.getCurrentCaptureSession() == null) {
                   return false;
                }
                boolean value = (Boolean) o;
                if (value) {
                    mPref.setContentDescription(fragment.getActivity().getResources()
                            .getString(R.string.dng_content_description_on));
                } else {
                    mPref.setContentDescription(fragment.getActivity().getResources()
                            .getString(R.string.dng_content_description_off));
                }
                mDngViewListener.onItemViewClick(value);
                return true;
            }
        });
        mPref.setChecked(mDngViewListener.onUpdatedValue());
        mPref.setEnabled(mEnabled);
        if (mDngViewListener.onUpdatedValue()) {
            mPref.setContentDescription(fragment.getActivity().getResources()
                    .getString(R.string.dng_content_description_on));
        } else {
            mPref.setContentDescription(fragment.getActivity().getResources()
                    .getString(R.string.dng_content_description_off));
        }
    }

    @Override
    public void refreshView() {
        if (mPref == null) {
            return;
        }
        mPref.setChecked(mDngViewListener.onUpdatedValue());
        mPref.setEnabled(mEnabled);
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
     * This is to set dng view update listener.
     * @param  dngViewListener the dng view listener.
     */
    public void setDngViewListener(DngViewCtrl.OnDngSettingViewListener dngViewListener) {
        mDngViewListener = dngViewListener;
    }

    /**
     * Set current dng entry values.
     *
     * @param entryValue Dng entry values.
     */
    public void setEntryValue(List<String> entryValue) {
        mEntryValues = entryValue;
    }

    /**
     * Set setting device2 request for hal3,because hal3 change dng should check device state.
     * @param settingRequester the setting request.
     */
    public void setSettingRequester(ISettingManager.SettingDevice2Requester settingRequester) {
        mSettingDevice2Requester = settingRequester;
    }
}
