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
import android.support.test.uiautomator.StaleObjectException;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.CheckerOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.operator.PauseResumeByHomeKeyOperator;
import com.mediatek.camera.tests.v3.operator.SwitchPageOperator;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

import java.util.List;

/**
 * Check the setting item that has second level setting. The second level setting's title
 * must be same with the setting item title in the first level list. The setting's summary
 * must be same with the selected value.
 */

public class SettingItemSummaryChecker extends CheckerOne {
    private static final LogUtil.Tag TAG = Utils.getTestTag(
            SettingItemSummaryChecker.class.getSimpleName());

    private static final String VIDEO_QUALITY_TITLE = "Video quality";

    private boolean mWithPauseAndResumedAfterChanged = false;

    @Override
    protected void doCheck() {
        UiObject2 settingListView = Utils.findObject(By.res("android:id/list"));
        // Get all the item that has summary view in the first level setting list.
        // The view that has summary view means it has second level setting list.
        List<UiObject2> childViewList = settingListView.findObjects(By.res("android:id/summary"));
        int count = childViewList.size();
        LogHelper.d(TAG, "[doCheck], child count:" + count);
        // Loop check the summary is updated after its value changed.
        for (int i = 0; i < count; i++) {
            doCheckSummary(i);
        }
    }

    @Override
    public Page getPageBeforeCheck() {
        return Page.SETTINGS;
    }

    @Override
    public String getDescription() {
        return "check every setting summary will be updated right when its value is changed";
    }

    /**
     * Set flag to indicate it needs to check value after paused and resumed.
     *
     * @return This instance.
     */
    public SettingItemSummaryChecker withPauseAndResumeAfterChangeValue() {
        mWithPauseAndResumedAfterChanged = true;
        return this;
    }

    private void doCheckSummary(int index) {
        UiObject2 settingListView = Utils.findObject(By.res("android:id/list"));
        List<UiObject2> summaryViewList = settingListView.findObjects(By.res("android:id/summary"));
        UiObject2 summaryView = summaryViewList.get(index);
        String defaultOption = summaryView.getText();

        UiObject2 checkedView = summaryView.getParent();
        UiObject2 titleView = checkedView.findObject(By.res("android:id/title"));
        String title = titleView.getText();
        if (VIDEO_QUALITY_TITLE.equals(title)) {
            LogHelper.d(TAG, "[doCheckSummary], setting[" + title
                    + "] skip check summary, because its summary" +
                    " update different with other setting");
            return;
        }
        LogHelper.d(TAG, "[doCheckSummary], setting[" + title
                + "] default option:" + defaultOption);
        // click item to open second level setting list.
        checkedView.click();

        // select an non-default option.
        UiObject2 selectedOptionView = selectOption(null);
        UiObject2 selectedOptionTitleView = selectedOptionView
                .findObject(By.res("android:id/title"));
        String selectedOptionTitle = selectedOptionTitleView.getText();
        LogHelper.d(TAG, "[doCheckSummary], setting:" + title
                + ", select:" + selectedOptionTitle);
        selectedOptionView.click();

        if (mWithPauseAndResumedAfterChanged) {
            // pause and resume camera and go to setting list page.
            new PauseResumeByHomeKeyOperator().operate(0);
            new SwitchPageOperator().operate(SwitchPageOperator.INDEX_SETTINGS);
        }

        // back to first level setting list.
        titleView = Utils.findObject(By.res("android:id/title").text(title));
        checkedView = titleView.getParent();
        summaryView = checkedView.findObject(By.res("android:id/summary"));
        Utils.assertRightNow(selectedOptionTitle.equals(summaryView.getText()));

        // restore default option
        settingListView = Utils.findObject(By.res("android:id/list"));
        summaryViewList = settingListView.findObjects(By.res("android:id/summary"));
        summaryView = summaryViewList.get(index);
        checkedView = summaryView.getParent();
        checkedView.click();

        selectedOptionView = selectOption(defaultOption);
        selectedOptionView.click();
    }

    private UiObject2 selectOption(String selectTitle) {
        UiObject2 selectedOptionView = null;
        try {
            Utils.getUiDevice().waitForWindowUpdate(null, Utils.TIME_OUT_SHORT);
            UiObject2 optionsListView = Utils.findObject(By.res("android:id/list"));
            List<UiObject2> childViewList = optionsListView.getChildren();
            for (int i = 0; i < childViewList.size(); i++) {
                UiObject2 optionView = childViewList.get(i);
                UiObject2 titleView = optionView.findObject(By.res("android:id/title"));
                UiObject2 checkBoxView = optionView.findObject(By.res("android:id/checkbox"));
                if (selectTitle == null) {
                    if (!checkBoxView.isChecked()) {
                        selectedOptionView = optionView;
                        break;
                    }
                } else if (titleView.getText().equals(selectTitle)) {
                    selectedOptionView = optionView;
                    break;
                }
            }
        } catch (StaleObjectException e) {
            Utils.assertRightNow(false);
        }
        return selectedOptionView;
    }
}
