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

package com.mediatek.camera.tests.v3.checker;

import android.support.test.uiautomator.By;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.CheckerOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

import java.util.ArrayList;
import java.util.List;

/**
 * Check the setting item type in the setting list.
 */

public class SettingItemTypeAndOrderChecker extends CheckerOne {
    private static final LogUtil.Tag TAG = Utils.getTestTag(
            SettingItemTypeAndOrderChecker.class.getSimpleName());

    public static final int PHOTO_TYPE = 0;
    public static final int VIDEO_TYPE = 1;

    private int mCheckType;

    private static String[] sPhotoTypeSetting = {
            "Scene mode",
            "Anti-shake",
            "RAW(.DNG)",
            "Self timer",
            "Picture size",
            "ZSD",
            "White balance",
            "ISO",
            "Anti flicker",
            "Image properties",
    };

    private static String[] sVideoTypeSetting = {
            "Scene mode",
            "EIS",
            "Microphone",
            "Audio mode",
            "Video quality",
            "White balance",
            "Anti flicker",
            "Image properties",
    };

    /**
     * Constructor with check type.
     *
     * @param checkType The check type.
     */
    public SettingItemTypeAndOrderChecker(int checkType) {
        mCheckType = checkType;
    }

    @Override
    protected void doCheck() {
        List<String> settingsTitleList = new ArrayList<>();
        boolean isSettingItemsOverall;
        do {
            UiObject2 settingListView = Utils.findObject(By.res("android:id/list"));
            List<UiObject2> titleViewList = settingListView.findObjects(By.res("android:id/title"));

            isSettingItemsOverall = true;
            for (int i = 0; i < titleViewList.size(); i++) {
                String title = titleViewList.get(i).getText();
                if (!settingsTitleList.contains(title)) {
                    settingsTitleList.add(title);
                    isSettingItemsOverall = false;
                }
            }

            Utils.scrollDownOnObject(settingListView);
        } while (!isSettingItemsOverall);

        String[] typeSetting = null;
        if (PHOTO_TYPE == mCheckType) {
            typeSetting = sPhotoTypeSetting;
        } else if (VIDEO_TYPE == mCheckType) {
            typeSetting = sVideoTypeSetting;
        }
        boolean isTypeRight = true;
        for (int i = 0; i < settingsTitleList.size(); i++) {
            String checkTitle = settingsTitleList.get(i);
            boolean isContained = false;
            for (int j = 0; j < typeSetting.length; j++) {
                if (checkTitle.equals(typeSetting[j])) {
                    isContained = true;
                    break;
                }
            }

            if (!isContained) {
                LogHelper.e(TAG, "[doCheck], setting type check failed," +
                        " checkTitle:" + checkTitle);
                isTypeRight = false;
                break;
            }
        }
        Utils.assertRightNow(isTypeRight);

        boolean isInOrder = true;
        if (settingsTitleList.size() < 2) {
            LogHelper.d(TAG, "[doCheck], setting list size less than 2, no need to check order");
            return;
        }

        for (int i = 0; i < settingsTitleList.size() - 1; i++) {
            String checkTitleA = settingsTitleList.get(i);
            String checkTitleB = settingsTitleList.get(i + 1);
            int titleAIndex = 0;
            int titleBIndex = 0;
            for (int j = 0; j < typeSetting.length; j++) {
                if (checkTitleA.equals(typeSetting[j])) {
                    titleAIndex = j;
                }
                if (checkTitleB.equals(typeSetting[j])) {
                    titleBIndex = j;
                }
            }

            if (titleAIndex >= titleBIndex) {
                isInOrder = false;
                LogHelper.e(TAG, "[doCheck], setting[" + checkTitleA
                        + "] and setting[" + checkTitleB + "] order is wrong");
                break;
            }
        }
        Utils.assertRightNow(isInOrder);
    }

    @Override
    public Page getPageBeforeCheck() {
        return Page.SETTINGS;
    }

    @Override
    public String getDescription() {
        if (PHOTO_TYPE == mCheckType) {
            return "check all the setting items are photo type";
        } else if (VIDEO_TYPE == mCheckType) {
            return "check all the setting items are video type";
        }
        return null;
    }
}
