/*
 * Copyright Statement:
 *
 *   This software/firmware and related documentation ("MediaTek Software") are
 *   protected under relevant copyright laws. The information contained herein is
 *   confidential and proprietary to MediaTek Inc. and/or its licensor. Without
 *   the prior written permission of MediaTek inc. and/or its licensor, any
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
 *   NON-INFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
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
package com.mediatek.camera.feature.setting.picturesize;

import android.preference.CheckBoxPreference;
import android.preference.Preference;
import android.preference.PreferenceGroup;

import com.mediatek.camera.BuildConfig;
import com.mediatek.camera.ui.SettingFragment;

import junit.framework.Assert;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.annotation.Config;
import org.robolectric.util.FragmentTestUtil;

import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.when;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;

/**
 * The unit test for {@link PictureSizeSettingView}.
 */
@RunWith(RobolectricTestRunner.class)
@Config(constants = BuildConfig.class, sdk = 22)
public class PictureSizeSettingViewTest {
    private SettingFragment mSettingFragment;
    private Map<String, String> mSizeSummaryMap = new HashMap();
    private PictureSize mMockPictureSize = mock(PictureSize.class);

    /**
     * Set up an instance of {@link SettingFragment} it will be used by all tests.
     */
    @Before
    public void setUpSettingFragment() {
        mSizeSummaryMap.put("5120x3840", "20M(4:3)");
        mSizeSummaryMap.put("5280x2992", "16M(16:9)");
        mSizeSummaryMap.put("5120x2880", "15M(16:9)");
        mSizeSummaryMap.put("4160x3120", "13M(4:3)");
        mSizeSummaryMap.put("4096x2304", "9M(16:9)");
        mSizeSummaryMap.put("2560x1920", "5M(4:3)");
        mSizeSummaryMap.put("2560x1440", "4M(16:9)");
        mSizeSummaryMap.put("2560x1712", "4M(3:2)");
        mSizeSummaryMap.put("1920x1080", "2M(16:9)");
        mSizeSummaryMap.put("1600x1200", "2M(4:3)");
        mSizeSummaryMap.put("1280x960", "1M(4:3)");
        mSizeSummaryMap.put("1280x720", "1M(16:9)");
        mSizeSummaryMap.put("1024x768", "1M(4:3)");
        mSizeSummaryMap.put("800x600", "SVGA"); // 4:3
        mSizeSummaryMap.put("800x480", "WVGA"); // 5:3
        mSizeSummaryMap.put("640x480", "VGA"); // 4:3
        mSizeSummaryMap.put("400x240", "WQVGA"); // 5:3
        mSizeSummaryMap.put("320x240", "QVGA"); // 4:3
    }

    /**
     * Test ListPreference's summary, it's summary should equals to pre-defined.
     * Case 1: picture size < 0.5M, show related screen.
     * Case 2: picture size >= 1M, show xM, follow rounding.
     */
    @Test
    public void testSummary() {
        mSettingFragment = new SettingFragment();
        FragmentTestUtil.startFragment(mSettingFragment);

        Set<Map.Entry<String, String>> summaryEntry = mSizeSummaryMap.entrySet();
        PictureSizeSettingView pictureSizeSettingView = getPictureSizeSettingView(
                new ArrayList<>(mSizeSummaryMap.keySet()), "320x240");

        List<Double> desiredAspectRatios = new ArrayList<>();
        desiredAspectRatios.add(16d / 9);
        desiredAspectRatios.add(5d / 3);
        desiredAspectRatios.add(4d / 3);
        desiredAspectRatios.add(3d / 2);
        PictureSizeHelper.setDesiredAspectRatios(desiredAspectRatios);

        Preference preference = mSettingFragment.findPreference(mMockPictureSize.getKey());

        for (Map.Entry<String, String> entry : summaryEntry) {
            pictureSizeSettingView.onItemClick(entry.getKey());
            pictureSizeSettingView.refreshView();
            Assert.assertEquals(entry.getValue(), preference.getSummary());
        }
    }

    /**
     * Test the picture sizes are shown according to the requested aspect ratios.
     */
    @Test
    public void testPictureSizeSelectorFilterByRatio() {
        List<Double> desiredAspectRatios = new ArrayList<>();
        desiredAspectRatios.add(16d / 9);
        desiredAspectRatios.add(4d / 3);
        PictureSizeHelper.setDesiredAspectRatios(desiredAspectRatios);

        PictureSizeSelector sizeSelector = new PictureSizeSelector();

        sizeSelector.setEntryValues(new ArrayList<>(mSizeSummaryMap.keySet()));
        sizeSelector.setValue("4160x3120");

        FragmentTestUtil.startFragment(sizeSelector);

        Map<String, String> expectedTitleAndSummary = new HashMap<>();
        expectedTitleAndSummary.put("5120x3840", "20M(4:3)");
        expectedTitleAndSummary.put("5280x2992", "16M(16:9)");
        expectedTitleAndSummary.put("5120x2880", "15M(16:9)");
        expectedTitleAndSummary.put("4160x3120", "13M(4:3)");
        expectedTitleAndSummary.put("4096x2304", "9M(16:9)");
        expectedTitleAndSummary.put("2560x1920", "5M(4:3)");
        expectedTitleAndSummary.put("2560x1440", "4M(16:9)");
        expectedTitleAndSummary.put("1920x1080", "2M(16:9)");
        expectedTitleAndSummary.put("1600x1200", "2M(4:3)");
        expectedTitleAndSummary.put("1280x960", "1M(4:3)");
        expectedTitleAndSummary.put("1280x720", "1M(16:9)");
        expectedTitleAndSummary.put("1024x768", "1M(4:3)");
        expectedTitleAndSummary.put("800x600", "SVGA"); // 4:3
        expectedTitleAndSummary.put("640x480", "VGA"); // 4:3
        expectedTitleAndSummary.put("320x240", "QVGA"); // 4:3
        List<String> expectedTitle = new ArrayList<>(expectedTitleAndSummary.values());
        List<String> expectedSummary = new ArrayList<>(expectedTitleAndSummary.keySet());

        PreferenceGroup preferenceGroup = sizeSelector.getPreferenceScreen();
        int count = preferenceGroup.getPreferenceCount();
        Assert.assertEquals(expectedTitleAndSummary.size(), count);

        int selectedCount = 0;
        CheckBoxPreference selectedPreference = null;
        for (int i = 0; i < count; i++) {
            CheckBoxPreference preference = (CheckBoxPreference) preferenceGroup.getPreference(i);
            if (preference.isChecked()) {
                selectedCount ++;
                selectedPreference = preference;
            }
            Assert.assertEquals(expectedTitle.get(i), preference.getTitle());
            Assert.assertEquals(expectedSummary.get(i), preference.getSummary());
        }
        Assert.assertTrue(selectedCount == 1);
        Assert.assertEquals("4160x3120", selectedPreference.getSummary());
    }

    private PictureSizeSettingView getPictureSizeSettingView(List<String> pictureSizes,
                                                 String currentPictureSize) {
        when(mMockPictureSize.getKey()).thenReturn("key_picture_size");
        PictureSizeSettingView pictureSizeView
                = new PictureSizeSettingView(mMockPictureSize.getKey());
        pictureSizeView.setValue(currentPictureSize);
        pictureSizeView.setEntryValues(pictureSizes);
        pictureSizeView.loadView(mSettingFragment);
        pictureSizeView.refreshView();
        return pictureSizeView;
    }
}