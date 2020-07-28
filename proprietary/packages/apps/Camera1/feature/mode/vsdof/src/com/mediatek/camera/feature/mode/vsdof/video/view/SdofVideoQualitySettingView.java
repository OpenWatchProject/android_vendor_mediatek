/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensor. Without
 * the prior written permission of MediaTek inc. and/or its licensor, any
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
 * NON-INFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
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
package com.mediatek.camera.feature.mode.vsdof.video.view;

import android.app.Activity;
import android.app.FragmentTransaction;
import android.preference.Preference.OnPreferenceClickListener;
import android.preference.PreferenceFragment;

import com.mediatek.camera.R;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil.Tag;
import com.mediatek.camera.common.preference.Preference;
import com.mediatek.camera.common.relation.DataStore;
import com.mediatek.camera.common.setting.ICameraSettingView;
import com.mediatek.camera.common.utils.CameraUtil;
import com.mediatek.camera.common.utils.Size;

import java.util.ArrayList;
import java.util.List;

/**
 * PIp video quality setting view.
 */
public class SdofVideoQualitySettingView implements ICameraSettingView {
    private static final Tag TAG = new Tag("Sdof-VideoQuality");
    public static final String KEY_SDOF_VIDEO_QUALITY = "key_sdof_video_quality";
    private static final int QUALITY_FHD = 1920 * 1080;
    private static final int QUALITY_HD = 1440 * 1080;
    private final Listener mVideoQualityChangeListener;
    private List<String> mEntryValues = new ArrayList<>();
    private List<String> mTitleList = new ArrayList<>();
    private String mSelectedValue = null;
    private boolean mEnabled;
    private Activity mActivity;
    private DataStore mDataStore;
    private Preference mVideoQualityPreference;
    private QualitySelectorPreference mSelectorPreference;
    private VideoQualitySelectorPreferenceClickListener mSelectorPreferenceClickListener =
            new VideoQualitySelectorPreferenceClickListener();

    /**
     * Listener to notify video quality value changed.
     */
    public interface Listener {
        /**
         * Callback when video quality value changed.
         * @param newQuality The changed video quality, such as "1920x1080".
         */
        void onQualityChanged(String newQuality);
    }

    /**
     * Construct with video quality value change listener.
     *  @param changeListener video quality value change listener.
     * @param activity camera activity.
     * @param dataStore the data store.
     */
    public SdofVideoQualitySettingView(Listener changeListener,
                                       Activity activity, DataStore dataStore) {
        mVideoQualityChangeListener = changeListener;
        mActivity = activity;
        mDataStore = dataStore;
    }

    /**
     * Set the video quality supported.
     * @param entryValues The video quality supported.
     */
    public void setEntryValues(List<String> entryValues) {
        mEntryValues.clear();
        mEntryValues.addAll(entryValues);
        createEntryTitles();
    }

    /**
     * Set default entry value.
     * @param defaultValue default entry value.
     */
    public void setDefaultValue(String defaultValue) {
        mSelectedValue = defaultValue;
    }

    @Override
    public void loadView(PreferenceFragment fragment) {
        initVideoQualitySelectorPreference();
        initVideoQualityPreference(fragment);
    }

    @Override
    public void refreshView() {
        mVideoQualityPreference.setSummary(mSelectedValue);
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

    private void createEntryTitles() {
        mTitleList.clear();
        for (String value : mEntryValues) {
            Size videoQualitySize = CameraUtil.getSize(value);
            int videoQualityArea = videoQualitySize.getWidth() * videoQualitySize.getHeight();
            String videoQualityTitle = null;
            switch (videoQualityArea) {
                case QUALITY_FHD:
                    videoQualityTitle =
                            mActivity.getResources().getString(R.string.sdof_quality_fhd);
                    break;
                case QUALITY_HD:
                    videoQualityTitle =
                            mActivity.getResources().getString(R.string.sdof_quality_hd);
                    break;
                default:
                    break;
            }
            mTitleList.add(videoQualityTitle);
        }
    }

    private void initVideoQualitySelectorPreference() {
        mSelectorPreference = new QualitySelectorPreference();
        mSelectorPreference.setOnPreferenceClickListener(mSelectorPreferenceClickListener);
        mSelectorPreference.setEntryValues(mEntryValues);
        mSelectorPreference.setTitleList(mTitleList);
        mSelectorPreference.setSelectedValue(mSelectedValue);
    }

    private void initVideoQualityPreference(PreferenceFragment fragment) {
        fragment.addPreferencesFromResource(R.xml.sdof_video_quality_preference);
        mVideoQualityPreference = (Preference)
                fragment.findPreference("sdof_video_quality_preference");
        mVideoQualityPreference.setId(R.id.sdof_video_quality_setting);
        mVideoQualityPreference.setContentDescription(
                mActivity.getResources().getString(
                        R.string.sdof_video_quality_content_description));
        mVideoQualityPreference.setOnPreferenceClickListener(new OnPreferenceClickListener() {
            @Override
            public boolean onPreferenceClick(android.preference.Preference preference) {
                FragmentTransaction transaction = mActivity.getFragmentManager()
                        .beginTransaction();
                transaction.addToBackStack(null);
                transaction.replace(R.id.setting_container,
                        mSelectorPreference, "sdof_video_quality_selector").commit();
                return true;
            }
        });
    }

    /**
     * Video quality selector preference click listener.
     */
    private class VideoQualitySelectorPreferenceClickListener implements OnPreferenceClickListener {
        @Override
        public boolean onPreferenceClick(android.preference.Preference preference) {
            String title = (String) preference.getTitle();
            int index = mTitleList.indexOf(title);
            String value = mEntryValues.get(index);
            mSelectedValue = value;
            mActivity.getFragmentManager().popBackStack();
            mSelectorPreference.setSelectedValue(mSelectedValue);
            mVideoQualityChangeListener.onQualityChanged(mSelectedValue);
            mDataStore.setValue(KEY_SDOF_VIDEO_QUALITY,
                    mSelectedValue,
                    mDataStore.getGlobalScope(), true);
            LogHelper.i(TAG, "On quality selected, new quality:" + mSelectedValue);
            return true;
        }
    }
}