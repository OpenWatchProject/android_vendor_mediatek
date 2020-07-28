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
 * MediaTek Inc. (C) 2014. All rights reserved.
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
package com.mediatek.camera.v2.setting.rule;

import android.util.Size;

import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;
import com.mediatek.camera.v2.setting.ISettingRule;
import com.mediatek.camera.v2.setting.SettingCtrl;
import com.mediatek.camera.v2.setting.SettingItem;
import com.mediatek.camera.v2.util.SettingKeys;
import com.mediatek.camera.v2.util.Utils;

import java.util.List;

/**
 * This class is used to contain the rules which can implement in {@link CommonRule}.
 *
 */
public class ExtraRules {
    private static final Tag TAG = new Tag(ExtraRules.class.getSimpleName());

    private SettingCtrl mSettingCtrl;

    /**
     * Keep the instance of {@link SettingCtrl} to get values, native supported
     * information from it.
     * @param settingCtrl The instance of {@link SettingCtrl}.
     */
    public ExtraRules(SettingCtrl settingCtrl) {
        mSettingCtrl = settingCtrl;
    }

    /**
     * Create rules and add them to rule matrix.
     */
    public void createRules() {
        PictureRatioSizeRule pictureRatioSizeRule = new PictureRatioSizeRule();
        mSettingCtrl.addRule(SettingKeys.KEY_PICTURE_RATIO,
                SettingKeys.KEY_PICTURE_SIZE, pictureRatioSizeRule);
    }

    /**
     * Define rule between picture ratio and picture size.
     *
     */
    private class PictureRatioSizeRule implements ISettingRule {

        @Override
        public void execute() {
            LogHelper.d(TAG, "[PictureRatioSizeRule], exectue");
            SettingItem pictureRatioSetting = mSettingCtrl.getSettingItem(
                    SettingKeys.KEY_PICTURE_RATIO);
            String pictureRatio = pictureRatioSetting.getValue();
            SettingItem pictureSizeSetting = mSettingCtrl.getSettingItem(
                    SettingKeys.KEY_PICTURE_SIZE);
            String pictureSize = pictureSizeSetting.getValue();

            List<String> supportedPictureSizes = mSettingCtrl.getSupportedValues(
                    SettingKeys.KEY_PICTURE_SIZE);
            if (supportedPictureSizes == null) {
                LogHelper.e(TAG, "supported picture size is null, return");
                return;
            }
            sortSizesInAscending(supportedPictureSizes);
            List<String> sizesFilteredByRatio = Utils.filterPictureSizesByRatio(
                    supportedPictureSizes, Double.parseDouble(pictureRatio));
            if (!sizesFilteredByRatio.contains(pictureSize)) {
                pictureSize = sizesFilteredByRatio.get(sizesFilteredByRatio.size() - 1);
                pictureSizeSetting.setValue(pictureSize);
            }
            String overrideValue = null;
            if (sizesFilteredByRatio.size() == 1) {
                overrideValue = sizesFilteredByRatio.get(0);
            } else if (sizesFilteredByRatio.size() > 1) {
                String[] values = new String[sizesFilteredByRatio.size()];
                overrideValue = Utils.buildEnableList(sizesFilteredByRatio.toArray(values));
            }
            pictureSizeSetting.setOverrideValue(overrideValue);
        }

        @Override
        public void addLimitation(String condition, List<String> result) {

        }

        private void sortSizesInAscending(List<String> supportedPictureSizes) {
            String maxSizeInString = null;
            Size maxSize = null;
            int maxIndex = 0;
            // Loop (n - 1) times.
            for (int i = 0; i < supportedPictureSizes.size() - 1; i++) {
                maxSizeInString = supportedPictureSizes.get(0);
                maxSize = Utils.getSize(maxSizeInString);
                maxIndex = 0;
                String tempSizeInString = null;
                Size tempSize = null;
                // Find the max size.
                for (int j = 0; j < supportedPictureSizes.size() - i; j++) {
                    tempSizeInString = supportedPictureSizes.get(j);
                    tempSize = Utils.getSize(tempSizeInString);
                    if (tempSize.getWidth() * tempSize.getHeight()
                            > maxSize.getWidth() * maxSize.getHeight()) {
                        maxSizeInString = tempSizeInString;
                        maxSize = tempSize;
                        maxIndex = j;
                    }
                }
                // Place the max size to the end position.
                supportedPictureSizes.set(maxIndex, tempSizeInString);
                supportedPictureSizes.set(supportedPictureSizes.size() - 1 - i, maxSizeInString);
            }
        }
    }
}