/*
 *   Copyright Statement:
 *
 *     This software/firmware and related documentation ("MediaTek Software") are
 *     protected under relevant copyright laws. The information contained herein is
 *     confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 *     the prior written permission of MediaTek inc. and/or its licensors, any
 *     reproduction, modification, use or disclosure of MediaTek Software, and
 *     information contained herein, in whole or in part, shall be strictly
 *     prohibited.
 *
 *     MediaTek Inc. (C) 2016. All rights reserved.
 *
 *     BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *    THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *     RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 *     ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 *     WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 *     WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 *     NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 *     RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 *     TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 *     RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 *     OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 *     SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 *     RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 *     STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 *     ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 *     RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 *     MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 *     CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     The following software/firmware and/or related documentation ("MediaTek
 *     Software") have been modified by MediaTek Inc. All revisions are subject to
 *     any receiver's applicable license agreements with MediaTek Inc.
 */
package com.mediatek.camera.feature.setting.selftimer;

import android.app.Activity;
import android.app.FragmentTransaction;
import android.preference.PreferenceFragment;
import android.preference.PreferenceGroup;

import com.mediatek.camera.R;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.preference.Preference;
import com.mediatek.camera.common.setting.ICameraSettingView;

import java.util.ArrayList;
import java.util.List;

/**
 * This class is for self timer feature setting view.
 */

public class SelfTimerSettingView  implements ICameraSettingView {
    private static final LogUtil.Tag TAG =
            new LogUtil.Tag(SelfTimerSettingView.class.getSimpleName());

    private String mSelectedValue;
    private List<String> mEntryValues = new ArrayList<>();
    private ISelfTimerViewListener.OnValueChangeListener mOnValueChangeListener;
    private Preference mSelfTimerPreference;
    private SelfTimerSelector mSelfTimerSelector;
    private Activity mContext;
    private boolean mEnabled;

    @Override
    public void loadView(PreferenceFragment fragment) {
        fragment.addPreferencesFromResource(R.xml.self_timer_preference);
        mContext = fragment.getActivity();

        if (mSelfTimerSelector == null) {
            mSelfTimerSelector = new SelfTimerSelector();
            mSelfTimerSelector.setOnItemClickListener(mOnItemClickListener);
        }

        mSelfTimerPreference = (Preference) fragment
                .findPreference(ISelfTimerViewListener.KEY_SELF_TIMER);
        mSelfTimerPreference.setRootPreference(fragment.getPreferenceScreen());
        mSelfTimerPreference.setId(R.id.self_timer_setting);
        mSelfTimerPreference.setContentDescription(fragment.getActivity().getResources()
                .getString(R.string.self_timer_content_description));
        mSelfTimerPreference.setSummary(getSummary());
        mSelfTimerPreference.setOnPreferenceClickListener(
                new Preference.OnPreferenceClickListener() {
            @Override
            public boolean onPreferenceClick(android.preference.Preference preference) {
                mSelfTimerSelector.setValue(mSelectedValue);
                mSelfTimerSelector.setEntryValues(mEntryValues);

                FragmentTransaction transaction = mContext.getFragmentManager()
                        .beginTransaction();
                transaction.addToBackStack(null);
                transaction.replace(R.id.setting_container,
                        mSelfTimerSelector, "self_timer_selector").commit();
                return true;
            }
        });
        mSelfTimerPreference.setEnabled(mEnabled);
    }

    @Override
    public void refreshView() {
        if (mSelfTimerPreference != null) {
            LogHelper.d(TAG, "[refreshView]");
            mSelfTimerPreference.setSummary(getSummary());
            mSelfTimerPreference.setEnabled(mEnabled);
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
     * Set listener to listen the changed self timer value.
     * @param listener The instance of {@link ISelfTimerViewListener.OnValueChangeListener}.
     */
    public void setOnValueChangeListener(ISelfTimerViewListener.OnValueChangeListener listener) {
        mOnValueChangeListener = listener;
    }

    /**
     * Set the default selected value.
     * @param value The default selected value.
     */
    public void setValue(String value) {
        mSelectedValue = value;
    }

    /**
     * Set the self timer supported.
     * @param entryValues The self timer supported.
     */
    public void setEntryValues(List<String> entryValues) {
        mEntryValues = entryValues;
    }

    private ISelfTimerViewListener.OnItemClickListener mOnItemClickListener
            = new ISelfTimerViewListener.OnItemClickListener() {
        @Override
        public void onItemClick(String value) {
            mSelectedValue = value;
            if (mOnValueChangeListener != null) {
                mOnValueChangeListener.onValueChanged(value);
            }
        }
    };

    private String getSummary() {
        if (ISelfTimerViewListener.TEN_SECONDS.equals(mSelectedValue)) {
            return mContext.getString(R.string.self_timer_entry_10);
        } else if (ISelfTimerViewListener.TWO_SECONDS.equals(mSelectedValue)) {
            return mContext.getString(R.string.self_timer_entry_2);
        } else {
            return mContext.getString(R.string.self_timer_entry_off);
        }
    }
}
