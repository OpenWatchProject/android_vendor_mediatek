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
 *   MediaTek Inc. (C) 2017. All rights reserved.
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
package com.mediatek.camera.feature.setting.shutterspeed;

import android.app.Activity;
import android.app.FragmentTransaction;
import android.preference.PreferenceFragment;

import com.mediatek.camera.R;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.preference.Preference;
import com.mediatek.camera.common.setting.ICameraSettingView;

import java.util.ArrayList;
import java.util.List;

/**
 * Shutter speed setting view.
 */
public class ShutterSpeedSettingView implements ICameraSettingView,
        ShutterSpeedSelector.OnItemClickListener {
    private static final LogUtil.Tag TAG =
            new LogUtil.Tag(ShutterSpeedSettingView.class.getSimpleName());

    private Activity mActivity;
    private Preference mPref;
    private OnValueChangeListener mListener;
    private String mKey;
    private String mSelectedValue;
    private List<String> mEntries = new ArrayList<>();
    private List<String> mEntryValues = new ArrayList<>();
    private String mAutoEntry = null;
    private ShutterSpeedSelector mSelector;
    private boolean mEnabled;

    /**
     * Listener to listen shutter speed value changed.
     */
    public interface OnValueChangeListener {
        /**
         * Callback when shutter speed value changed.
         *
         * @param value The new shutter speed.
         */
        void onValueChanged(String value);
    }

    /**
     * Shutter speed setting view constructor.
     *
     * @param key      The key of shutter speed.
     * @param activity The camera activity.
     */
    public ShutterSpeedSettingView(String key, Activity activity) {
        mKey = key;
        mActivity = activity;
        mAutoEntry = activity.getResources().getString(R.string
                .shutter_speed_entry_auto);
    }

    @Override
    public void loadView(PreferenceFragment fragment) {
        LogHelper.d(TAG, "[loadView]");
        if (mSelector == null) {
            mSelector = new ShutterSpeedSelector();
            mSelector.setOnItemClickListener(this);
        }

        fragment.addPreferencesFromResource(R.xml.shutter_speed_preference);
        mPref = (Preference) fragment.findPreference(mKey);
        mPref.setRootPreference(fragment.getPreferenceScreen());
        mPref.setId(R.id.shutter_speed_setting);
        mPref.setContentDescription(mActivity.getResources()
                .getString(R.string.shutter_speed_content_description));
        mPref.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {

            @Override
            public boolean onPreferenceClick(android.preference.Preference preference) {
                mSelector.setValue(mSelectedValue);
                mSelector.setEntriesAndEntryValues(mEntries, mEntryValues);

                FragmentTransaction transaction = mActivity.getFragmentManager()
                        .beginTransaction();
                transaction.addToBackStack(null);
                transaction.replace(R.id.setting_container,
                        mSelector, "shutter_speed_selector").commit();
                return true;
            }
        });
        mPref.setEnabled(mEnabled);
    }

    @Override
    public void refreshView() {
        if (mPref != null) {
            mPref.setSummary(getSummary());
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

    @Override
    public void onItemClick(String value) {
        setValue(value);
        if (mListener != null) {
            mListener.onValueChanged(value);
        }
    }

    /**
     * Set listener to listen the changed shutter speed value.
     *
     * @param listener The instance of {@link OnValueChangeListener}.
     */
    public void setOnValueChangeListener(OnValueChangeListener listener) {
        mListener = listener;
    }

    /**
     * Set the default selected value.
     *
     * @param value The default selected value.
     */
    public void setValue(String value) {
        mSelectedValue = value;
    }

    /**
     * Set the shutter speed supported values.
     *
     * @param entryValues The values shutter speed supported.
     */
    public void setEntryValues(List<String> entryValues) {
        mEntries.clear();
        mEntryValues.clear();

        mEntries.addAll(entryValues);
        mEntryValues.addAll(entryValues);
        mEntries.set(0, mAutoEntry);
    }

    private String getSummary() {
        if (ShutterSpeedHelper.ONE_SECONDS.equals(mSelectedValue)) {
            return mActivity.getString(R.string.shutter_speed_entry_1);
        } else if (ShutterSpeedHelper.TWO_SECONDS.equals(mSelectedValue)) {
            return mActivity.getString(R.string.shutter_speed_entry_2);
        } else if (ShutterSpeedHelper.FOUR_SECONDS.equals(mSelectedValue)) {
            return mActivity.getString(R.string.shutter_speed_entry_4);
        } else if (ShutterSpeedHelper.EIGHT_SECONDS.equals(mSelectedValue)) {
            return mActivity.getString(R.string.shutter_speed_entry_8);
        } else if (ShutterSpeedHelper.THIRTY_SIX_SECONDS.equals(mSelectedValue)) {
            return mActivity.getString(R.string.shutter_speed_entry_16);
        } else {
            return mActivity.getString(R.string.shutter_speed_entry_auto);
        }
    }
}
