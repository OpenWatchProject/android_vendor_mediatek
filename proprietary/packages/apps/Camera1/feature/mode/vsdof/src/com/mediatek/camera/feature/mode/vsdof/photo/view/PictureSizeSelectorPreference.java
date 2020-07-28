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
package com.mediatek.camera.feature.mode.vsdof.photo.view;

import android.os.Bundle;
import android.preference.Preference.OnPreferenceClickListener;
import android.preference.PreferenceFragment;
import android.preference.PreferenceScreen;
import android.support.v7.widget.Toolbar;

import com.mediatek.camera.R;
import com.mediatek.camera.feature.setting.videoquality.RadioPreference;

import java.util.ArrayList;
import java.util.List;

import javax.annotation.Nonnull;

/**
 * sdof picture size selector preference.
 */
public class PictureSizeSelectorPreference extends PreferenceFragment {
    private OnPreferenceClickListener mOnPreferenceClickListener;
    private List<String> mEntryValues = new ArrayList<>();
    private List<String> mTitleList = new ArrayList<>();
    private String mSelectedValue = null;

    /**
     * Set selector on preference click listener.
     * @param preferenceClickListener preference click listener.
     */
    public void setOnPreferenceClickListener(
            @Nonnull OnPreferenceClickListener preferenceClickListener) {
        mOnPreferenceClickListener = preferenceClickListener;
    }

    /**
     * Set the video quality supported.
     * @param entryValues The video quality supported.
     */
    public void setEntryValues(List<String> entryValues) {
        mEntryValues.clear();
        mEntryValues.addAll(entryValues);
    }

    /**
     * Set the video quality title list.
     * @param titleList The video quality supported title list.
     */
    public void setTitleList(List<String> titleList) {
        mTitleList.clear();
        mTitleList.addAll(titleList);
    }

    /**
     * Set the selected value.
     * @param value The default selected value.
     */
    public void setSelectedValue(String value) {
        mSelectedValue = value;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Toolbar toolbar = (Toolbar) getActivity().findViewById(R.id.toolbar);
        toolbar.setTitle(getActivity().getResources().getString(R.string.sdof_picture_size_title));
        addPreferencesFromResource(R.xml.sdof_picture_size_selector_preference);
        PreferenceScreen screen = getPreferenceScreen();
        for (int i = 0 ; i < mEntryValues.size(); i++) {
            RadioPreference preference = new RadioPreference(getActivity());
            if (mEntryValues.get(i).equals(mSelectedValue)) {
                preference.setChecked(true);
            }
            preference.setTitle(mTitleList.get(i));
            preference.setSummary(mEntryValues.get(i));
            preference.setOnPreferenceClickListener(mOnPreferenceClickListener);
            screen.addPreference(preference);
        }
    }
}