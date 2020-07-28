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
package com.mediatek.camera.feature.setting.videoquality;

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
 * Video quality setting view.
 */
public class VideoQualitySettingView implements ICameraSettingView,
                       VideoQualitySelector.OnItemClickListener {
    private static final LogUtil.Tag TAG =
            new LogUtil.Tag(VideoQualitySettingView.class.getSimpleName());
    private List<String> mEntryValues = new ArrayList<>();

    private VideoQualitySelector mQualitySelector;
    private OnValueChangeListener mListener;
    private VideoQuality mVideoQuality;
    private String mSelectedValue;
    private boolean mEnabled;
    private Activity mActivity;
    private Preference mPref;
    private String mSummary;
    private String mKey;

    /**
     * Listener to listen video quality value changed.
     */
    public interface OnValueChangeListener {
        /**
         * Callback when video quality value changed.
         * @param value The changed video quality, such as "1920x1080".
         */
        void onValueChanged(String value);
    }
    /**
     * Video quality setting view constructor.
     * @param key The key of video quality
     * @param videoQuality the video quality
     */
    public VideoQualitySettingView(String key, VideoQuality videoQuality) {
        mKey = key;
        mVideoQuality = videoQuality;
    }

    @Override
    public void loadView(PreferenceFragment fragment) {
        mActivity = fragment.getActivity();
        if (mQualitySelector == null) {
            mQualitySelector = new VideoQualitySelector();
            mQualitySelector.setOnItemClickListener(this);

        }
        mQualitySelector.setActivity(mActivity);
        mQualitySelector.setCurrentID(Integer.parseInt(mVideoQuality.getCameraId()));
        mQualitySelector.setValue(mSelectedValue);
        mQualitySelector.setEntryValues(mEntryValues);
        fragment.addPreferencesFromResource(R.xml.videoquality_preference);
        mPref = (Preference) fragment.findPreference(mKey);
        mPref.setRootPreference(fragment.getPreferenceScreen());
        mPref.setId(R.id.video_quality_setting);
        mPref.setContentDescription(mActivity.getResources()
                .getString(R.string.video_quality_content_description));
        mPref.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {

            @Override
            public boolean onPreferenceClick(android.preference.Preference preference) {
                FragmentTransaction transaction = mActivity.getFragmentManager()
                        .beginTransaction();
                transaction.addToBackStack(null);
                transaction.replace(R.id.setting_container,
                        mQualitySelector, "video_quality_selector").commit();
                return true;
            }
        });
        mPref.setEnabled(mEnabled);
        mSummary = VideoQualityHelper.getCurrentResolution(
                Integer.parseInt(mVideoQuality.getCameraId()), mSelectedValue);
    }

    @Override
    public void refreshView() {
        if (mPref != null) {
            mPref.setSummary(mSummary);
            mPref.setEnabled(mEnabled);
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
     * Set listener to listen the changed video quality value.
     * @param listener The instance of {@link OnValueChangeListener}.
     */
    public void setOnValueChangeListener(OnValueChangeListener listener) {
        mListener = listener;
    }

    /**
     * Set the default selected value.
     * @param value The default selected value.
     */
    public void setValue(String value) {
        mSelectedValue = value;
    }

    /**
     * Set the video quality supported.
     * @param entryValues The video quality supported.
     */
    public void setEntryValues(List<String> entryValues) {
        mEntryValues = entryValues;
    }

    /**
     * Callback when item clicked.
     * @param value The video quality clicked.
     */
    @Override
    public void onItemClick(String value) {
        mSelectedValue = value;
        mSummary = VideoQualityHelper.getCurrentResolution(
                Integer.parseInt(mVideoQuality.getCameraId()), value);
        mListener.onValueChanged(value);
    }
}
