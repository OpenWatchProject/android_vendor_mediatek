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
 * MediaTek Inc. (C) 2016. All rights reserved.
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
package com.mediatek.camera.feature.setting.antiflicker;

import android.os.Bundle;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.preference.PreferenceScreen;
import android.support.v7.widget.Toolbar;

import com.mediatek.camera.R;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.feature.setting.picturesize.RadioPreference;

import java.util.List;

/**
 * Self timer selector.
 */

public class AntiFlickerSelector extends PreferenceFragment {
    private static final LogUtil.Tag TAG
            = new LogUtil.Tag(AntiFlickerSelector.class.getSimpleName());

    private List<String> mEntries;
    private List<String> mEntryValues;
    private String mSelectedValue = null;
    private Preference.OnPreferenceClickListener mOnPreferenceClickListener
            = new MyPreferenceClickListener();
    private OnItemClickListener mListener;

    /**
     * Listener to listen the item clicked.
     */
    public interface OnItemClickListener {
        /**
         * Callback when item clicked.
         *
         * @param value The value clicked.
         */
        void onItemClick(String value);
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        Toolbar toolbar = (Toolbar) getActivity().findViewById(R.id.toolbar);
        if (toolbar != null) {
            toolbar.setTitle(getActivity().getString(R.string.pref_camera_antibanding_title));
        }
        addPreferencesFromResource(R.xml.anti_flicker_selector_preference);
        PreferenceScreen screen = getPreferenceScreen();
        for (int i = 0 ; i < mEntryValues.size(); i++) {
            RadioPreference preference = new RadioPreference(getActivity());
            if (mEntryValues.get(i).equals(mSelectedValue)) {
                preference.setChecked(true);
            }
            preference.setTitle(mEntries.get(i));
            preference.setOnPreferenceClickListener(mOnPreferenceClickListener);
            screen.addPreference(preference);
        }
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
     * Set anti flicker entries and entry values.
     *
     * @param entries Anti flicker entries.
     * @param entryValues Anti flicker entry values.
     */
    public void setEntriesAndEntryValues(List<String> entries,
                                         List<String> entryValues) {
        mEntries = entries;
        mEntryValues = entryValues;
    }

    /**
     * Self timer preference click listener.
     */
    private class MyPreferenceClickListener implements Preference.OnPreferenceClickListener {
        @Override
        public boolean onPreferenceClick(Preference preference) {
            String title = (String) preference.getTitle();
            int index = mEntries.indexOf(title);
            String value = mEntryValues.get(index);
            mListener.onItemClick(value);

            mSelectedValue = value;
            getActivity().getFragmentManager().popBackStack();
            return true;
        }
    }
}
