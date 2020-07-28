/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */
package com.mediatek.camera.feature.setting.shutterspeed;

import android.os.Bundle;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.preference.PreferenceScreen;
import android.support.v7.widget.Toolbar;

import com.mediatek.camera.R;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;

import java.util.ArrayList;
import java.util.List;

/**
 * Shutter speed selector.
 */

public class ShutterSpeedSelector extends PreferenceFragment {
    private static final LogUtil.Tag TAG
            = new LogUtil.Tag(ShutterSpeedSelector.class.getSimpleName());

    private List<String> mEntryValues = new ArrayList<>();
    private List<String> mTitleList = new ArrayList<>();
    private String mSelectedValue = null;
    private Preference.OnPreferenceClickListener mOnPreferenceClickListener
            = new MyOnPreferenceClickListener();
    private OnItemClickListener mListener;

    /**
     * Listener to listen the item clicked.
     */
    public interface OnItemClickListener {
        /**
         * Callback when item clicked.
         *
         * @param value The picture size clicked.
         */
        void onItemClick(String value);
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        LogHelper.d(TAG, "[onCreate]");
        super.onCreate(savedInstanceState);
        filterValuesOnShown();
        Toolbar toolbar = (Toolbar) getActivity().findViewById(R.id.toolbar);
        if (toolbar != null) {
            toolbar.setTitle(getActivity().getResources().getString(R.string.shutter_speed_title));
        }
        addPreferencesFromResource(R.xml.shutter_speed_selector);
        PreferenceScreen screen = getPreferenceScreen();
        for (int i = 0; i < mEntryValues.size(); i++) {
            RadioPreference preference = new RadioPreference(getActivity());
            if (mEntryValues.get(i).equals(mSelectedValue)) {
                preference.setChecked(true);
            }
            preference.setTitle(mTitleList.get(i));
            preference.setOnPreferenceClickListener(mOnPreferenceClickListener);
            screen.addPreference(preference);
        }
    }

    @Override
    public void onResume() {
        LogHelper.d(TAG, "[onResume]");
        super.onResume();
    }

    @Override
    public void onPause() {
        LogHelper.d(TAG, "[onPause]");
        super.onPause();
    }

    @Override
    public void onDestroy() {
        LogHelper.d(TAG, "[onDestroy]");
        super.onDestroy();
    }

    /**
     * Set listener to listen item clicked.
     *
     * @param listener The instance of {@link OnItemClickListener}.
     */
    public void setOnItemClickListener(OnItemClickListener listener) {
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
     * Set shutter speed entries and entry values.
     *
     * @param entries     Shutter speed entries.
     * @param entryValues Shutter speed entry values.
     */
    public void setEntriesAndEntryValues(List<String> entries, List<String> entryValues) {
        mEntryValues = entryValues;
    }

    /**
     * Preference click listener.
     */
    private class MyOnPreferenceClickListener implements Preference.OnPreferenceClickListener {
        @Override
        public boolean onPreferenceClick(Preference preference) {
            String title = (String) preference.getTitle();
            int index = mTitleList.indexOf(title);
            String value = mEntryValues.get(index);
            if (mListener != null) {
                mListener.onItemClick(value);
            }
            mSelectedValue = value;
            getActivity().getFragmentManager().popBackStack();
            return true;
        }
    }

    private void filterValuesOnShown() {
        List<String> tempValues = new ArrayList<>(mEntryValues);
        mEntryValues.clear();
        mTitleList.clear();
        for (int i = 0; i < tempValues.size(); i++) {
            String value = tempValues.get(i);
            String title = getTitlePattern(value);
            mTitleList.add(title);
            mEntryValues.add(value);
        }
    }

    private String getTitlePattern(String value) {
        switch (value) {
            case ShutterSpeedHelper.ONE_SECONDS:
                return getActivity().getString(R.string.shutter_speed_entry_1);
            case ShutterSpeedHelper.TWO_SECONDS:
                return getActivity().getString(R.string.shutter_speed_entry_2);
            case ShutterSpeedHelper.FOUR_SECONDS:
                return getActivity().getString(R.string.shutter_speed_entry_4);
            case ShutterSpeedHelper.EIGHT_SECONDS:
                return getActivity().getString(R.string.shutter_speed_entry_8);
            case ShutterSpeedHelper.THIRTY_SIX_SECONDS:
                return getActivity().getString(R.string.shutter_speed_entry_16);
            default:
                return getActivity().getString(R.string.shutter_speed_entry_auto);
        }
    }
}
