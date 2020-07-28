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
package com.mediatek.camera.feature.mode.vsdof.photo.view;

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
import com.mediatek.camera.common.utils.Size;
import com.mediatek.camera.feature.mode.vsdof.SdofUtil;

import java.util.ArrayList;
import java.util.List;

/**
 * vsdof picture size setting view.
 */
public class SdofPictureSizeSettingView implements ICameraSettingView {
    private static final Tag TAG = new Tag("Sdof_PictureSize");
    private static final String KEY_STEREO_PICTURE_SIZE = "key_stereo_picture_size";
    private final Listener mPictureSizeChangeListener;
    private List<String> mEntryValues = new ArrayList<>();
    private List<String> mTitleList = new ArrayList<>();
    private String mSelectedValue = null;
    private boolean mEnabled;
    private Activity mActivity;
    private DataStore mDataStore;
    private Preference mPictureSizePreference;
    private PictureSizeSelectorPreference mSelectorPreference;
    private PictureSizeSelectorPreferenceClickListener mSelectorPreferenceClickListener =
            new PictureSizeSelectorPreferenceClickListener();

    /**
     * Listener to notify video quality value changed.
     */
    public interface Listener {
        /**
         * Callback when video quality value changed.
         * @param newQuality The changed video quality, such as "1920x1080".
         */
        void onSizeChanged(String newQuality);
    }

    /**
     * Construct with video quality value change listener.
     * @param changeListener video quality value change listener.
     * @param activity camera activity.
     * @param dataStore the data store.
     */
    public SdofPictureSizeSettingView(Listener changeListener,
                                      Activity activity, DataStore dataStore) {
        mPictureSizeChangeListener = changeListener;
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
        initPictureSizeSelectorPreference();
        initPictureSizePreference(fragment);
    }

    @Override
    public void refreshView() {
        String curSize = mDataStore.getValue(KEY_STEREO_PICTURE_SIZE,
                mSelectedValue,
                mDataStore.getGlobalScope());
        if (!mEntryValues.isEmpty()) {
            int index = mEntryValues.indexOf(curSize);
            String value = mTitleList.get(index == -1 ? 0 : index);
            mPictureSizePreference.setSummary(value);
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

    private void createEntryTitles() {
        mTitleList.clear();
        for (String value : mEntryValues) {
            String pictureSizeTitle = SdofUtil.getPixelsAndRatio(value);
            mTitleList.add(pictureSizeTitle);
        }
    }

    private void initPictureSizeSelectorPreference() {
        mSelectorPreference = new PictureSizeSelectorPreference();
        mSelectorPreference.setOnPreferenceClickListener(mSelectorPreferenceClickListener);
        mSelectorPreference.setEntryValues(mEntryValues);
        mSelectorPreference.setTitleList(mTitleList);
        String curSize = mDataStore.getValue(KEY_STEREO_PICTURE_SIZE,
                mSelectedValue,
                mDataStore.getGlobalScope());
        mSelectorPreference.setSelectedValue(curSize);
    }

    private void initPictureSizePreference(PreferenceFragment fragment) {
        fragment.addPreferencesFromResource(R.xml.sdof_picture_size_preference);
        mPictureSizePreference = (Preference)
                fragment.findPreference("sdof_picture_size_preference");
        mPictureSizePreference.setId(R.id.sdof_picture_size_setting);
        mPictureSizePreference.setContentDescription(
                mActivity.getResources().getString(
                        R.string.sdof_picture_size_content_description));
        mPictureSizePreference.setOnPreferenceClickListener(new OnPreferenceClickListener() {
            @Override
            public boolean onPreferenceClick(android.preference.Preference preference) {
                FragmentTransaction transaction = mActivity.getFragmentManager()
                        .beginTransaction();
                transaction.addToBackStack(null);
                transaction.replace(R.id.setting_container,
                        mSelectorPreference, "sdof_picture_size_selector").commit();
                return true;
            }
        });
    }

    /**
     * Picture Size selector preference click listener.
     */
    private class PictureSizeSelectorPreferenceClickListener implements OnPreferenceClickListener {
        @Override
        public boolean onPreferenceClick(android.preference.Preference preference) {
            String title = (String) preference.getTitle();
            int index = mTitleList.indexOf(title);
            String value = mEntryValues.get(index);
            mSelectedValue = value;
            mActivity.getFragmentManager().popBackStack();
            mSelectorPreference.setSelectedValue(mSelectedValue);
            mPictureSizeChangeListener.onSizeChanged(mSelectedValue);
            mDataStore.setValue(KEY_STEREO_PICTURE_SIZE,
                    mSelectedValue,
                    mDataStore.getGlobalScope(), true);
            LogHelper.i(TAG, "On picture size selected, new size:" + mSelectedValue);
            return true;
        }
    }
}