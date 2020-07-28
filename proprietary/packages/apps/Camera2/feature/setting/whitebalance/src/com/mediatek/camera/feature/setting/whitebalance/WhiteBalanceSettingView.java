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
package com.mediatek.camera.feature.setting.whitebalance;

import android.app.Activity;
import android.app.FragmentTransaction;
import android.content.res.TypedArray;
import android.preference.PreferenceFragment;
import android.preference.PreferenceGroup;

import com.mediatek.camera.R;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.preference.Preference;
import com.mediatek.camera.common.setting.ICameraSettingView;

import java.util.ArrayList;
import java.util.List;

/**
 * White balance setting view.
 */

public class WhiteBalanceSettingView implements ICameraSettingView,
        WhiteBalanceSelector.OnItemClickListener {
    private static final LogUtil.Tag TAG
            = new LogUtil.Tag(WhiteBalanceSettingView.class.getSimpleName());
    private List<String> mOriginalEntries = new ArrayList<>();
    private List<String> mOriginalEntryValues = new ArrayList<>();
    private List<Integer> mOriginalIcons = new ArrayList<>();
    private List<String> mEntries = new ArrayList<>();
    private List<String> mEntryValues = new ArrayList<>();
    private List<Integer> mIcons = new ArrayList<>();

    private Activity mActivity;
    private String mKey = null;
    private String mSummary = null;
    private String mSelectedValue = null;
    private boolean mEnabled;

    private OnValueChangeListener mListener;
    private Preference mPreference;
    private WhiteBalanceSelector mSelector;

    @Override
    public void onItemClick(String value) {
        setValue(value);
        if (mListener != null) {
            mListener.onValueChanged(value);
        }
    }

    /**
     * Listener to listen White balance value changed.
     */
    public interface OnValueChangeListener {
        /**
         * Callback when White balance value changed.
         *
         * @param value The changed White balance.
         */
        void onValueChanged(String value);
    }

    /**
     * Setting view constructor.
     *
     * @param activity The instance of camera activity.
     * @param key The scene mode setting key.
     */
    public WhiteBalanceSettingView(Activity activity, String key) {
        mActivity = activity;
        mKey = key;
        String[] originalEntriesInArray = mActivity.getResources()
                .getStringArray(R.array.white_balance_entries);
        String[] originalEntryValuesInArray = mActivity.getResources()
                .getStringArray(R.array.white_balance_entryvalues);

        TypedArray array = mActivity.getResources().obtainTypedArray(R.array.white_balance_icons);
        int n = array.length();
        int[] originalIconsInArray = new int[n];
        for (int i = 0; i < n; ++i) {
            originalIconsInArray[i] = array.getResourceId(i, 0);
        }
        array.recycle();

        for (String value : originalEntriesInArray) {
            mOriginalEntries.add(value);
        }
        for (String value : originalEntryValuesInArray) {
            mOriginalEntryValues.add(value);
        }
        for (int icon : originalIconsInArray) {
            mOriginalIcons.add(icon);
        }
    }

    @Override
    public void loadView(PreferenceFragment fragment) {
        if (mSelector == null) {
            mSelector = new WhiteBalanceSelector();
            mSelector.setOnItemClickListener(this);
        }

        fragment.addPreferencesFromResource(R.xml.white_balance_preference);
        mPreference = (Preference) fragment.getPreferenceManager().findPreference(mKey);
        mPreference.setRootPreference(fragment.getPreferenceScreen());
        mPreference.setId(R.id.white_balance_setting);
        mPreference.setContentDescription(fragment.getActivity().getResources()
                .getString(R.string.pref_camera_whitebalance_content_description));
        mPreference.setSummary(mSummary);
        mPreference.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
            @Override
            public boolean onPreferenceClick(android.preference.Preference preference) {
                mSelector.setSelectedValue(mSelectedValue);
                mSelector.setEntriesAndEntryValues(mEntries, mEntryValues, mIcons);

                FragmentTransaction transaction = mActivity.getFragmentManager()
                        .beginTransaction();
                transaction.addToBackStack(null);
                transaction.replace(R.id.setting_container,
                        mSelector, "white_balance_selector").commit();
                return true;
            }
        });
        mPreference.setEnabled(mEnabled);
    }

    @Override
    public void refreshView() {
        if (mPreference != null) {
            mPreference.setSummary(mSummary);
            mPreference.setEnabled(mEnabled);
        }
    }

    @Override
    public void unloadView() {

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
     * Set listener to listen the changed White balance value.
     *
     * @param listener The instance of {@link OnValueChangeListener}.
     */
    public void setOnValueChangeListener(OnValueChangeListener listener) {
        mListener = listener;
    }

    /**
     * Set current White balance entry values.
     *
     * @param entryValues Current entry values.
     */
    public void setEntryValues(List<String> entryValues) {
        mEntries.clear();
        mEntryValues.clear();
        mIcons.clear();

        for (int i = 0; i < mOriginalEntryValues.size(); i++) {
            String originalEntryValue = mOriginalEntryValues.get(i);
            for (int j = 0; j < entryValues.size(); j++) {
                String entryValue = entryValues.get(j);
                if (entryValue.equals(originalEntryValue)) {
                    mEntryValues.add(entryValue);
                    mEntries.add(mOriginalEntries.get(i));
                    mIcons.add(mOriginalIcons.get(i));
                    break;
                }
            }
        }
    }

    /**
     * Set current White balance value.
     *
     * @param value Current White balance value.
     */
    public void setValue(String value) {
        mSelectedValue = value;
        int index = mEntryValues.indexOf(mSelectedValue);
        if (index >= 0) {
            mSummary = mEntries.get(index);
        }
    }
}
