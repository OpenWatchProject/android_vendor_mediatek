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
package com.mediatek.camera.tests.v2.common.mode;

import android.support.test.InstrumentationRegistry;
import android.support.test.runner.AndroidJUnit4;
import android.support.test.uiautomator.By;
import android.support.test.uiautomator.UiCollection;
import android.support.test.uiautomator.UiDevice;
import android.support.test.uiautomator.UiObject;
import android.support.test.uiautomator.UiObject2;
import android.support.test.uiautomator.UiObjectNotFoundException;
import android.support.test.uiautomator.UiSelector;
import android.support.test.uiautomator.Until;

import com.mediatek.camera.tests.Log;
import com.mediatek.camera.tests.v2.Utils;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;

import static org.hamcrest.Matchers.greaterThan;
import static org.hamcrest.Matchers.is;
import static org.hamcrest.Matchers.notNullValue;
import static org.junit.Assert.assertThat;

/**
 * Camera mode feature sanity test cases.
 */
@RunWith(AndroidJUnit4.class)
public class ModeSanityTestCase {
    private static final String TAG = ModeSanityTestCase.class.getSimpleName();
    private static final String MODE_RES_ID = Utils.RES_ID_PREFIX + "mode";
    private static final String MODE_RES_List_ID = Utils.RES_ID_PREFIX + "mode_list";
    private static final String FACING_SWITCHER_RES_ID = Utils.RES_ID_PREFIX + "camera_switcher";
    private UiDevice mUiDevice;

    /**
     * Launch mtk camera app and wait for preview ready.
     */
    @Before
    public void launchCameraApp() {
        mUiDevice = UiDevice.getInstance(InstrumentationRegistry.getInstrumentation());
        Utils.launchMtkCameraAppFromHomeScreen();
        assertThat(Utils.waitForPreviewReady(), is(true));
    }

    /**
     * In back facing to switch all modes.
     */
    @Test
    public void switchAllModesInBackFacing() {
        UiObject2 modeObject = mUiDevice.findObject(By.res(MODE_RES_ID));
        if (modeObject == null) {
            Log.w(TAG, "Can not find mode icon, there may be only has normal photo & video mode!");
            return;
        }
        switchToFacing("back");
        modeObject.click();
        mUiDevice.wait(Until.hasObject(By.res(MODE_RES_List_ID)), Utils.OPERATION_TIMEOUT);
        doSwitchAllMode();
    }

    /**
     * In front facing to switch all modes.
     */
    @Test
    public void switchAllModesInFrontFacing() {
        UiObject2 modeObject = mUiDevice.findObject(By.res(MODE_RES_ID));
        if (modeObject == null) {
            Log.w(TAG, "Can not find mode icon, there may be only has normal photo & video mode!");
            return;
        }
        // in some projects, there may has no front cameras, skip this test
        if (switchToFacing("front")) {
            modeObject.findObject(By.res(MODE_RES_ID)).click();
            mUiDevice.wait(Until.hasObject(By.res(MODE_RES_List_ID)), Utils.OPERATION_TIMEOUT);
            doSwitchAllMode();
        }
    }

    /**
     * Test step:
     * open mode list, for each mode do following operation:
     * 1.switch to one mode.
     * 2.in this mode, switch photo & video.
     */
    private void doSwitchAllMode() {
        Log.i(TAG, "[doSwitchAllMode]+");
        UiCollection modeCollection =
                new UiCollection(new UiSelector().resourceId(MODE_RES_List_ID));
        int modeCount = 0;
        try {
            modeCount = modeCollection.getChildCount();
        } catch (UiObjectNotFoundException e) {
            e.printStackTrace();
        }
        Log.i(TAG, "[doSwitchAllMode] count:" + modeCount);
        assertThat(modeCount, greaterThan(0));
        for (int i = 0; i < modeCount; i++) {
            try {
                UiObject mode = modeCollection.getChild(
                        new UiSelector().className("android.view.ViewGroup").instance(i));
                assertThat(mode, notNullValue());
                mode.clickAndWaitForNewWindow();
                assertThat(Utils.waitForPreviewReady(), is(true));
                switchModeInShutters();
                mUiDevice.findObject(By.res(MODE_RES_ID)).click();
            } catch (UiObjectNotFoundException e) {
                e.printStackTrace();
            }
        }
        Log.i(TAG, "[doSwitchAllMode]-");
    }

    /**
     * Some modes switched by sweep shutters.
     */
    private void switchModeInShutters() {
        UiCollection shutterRootCollection =
                new UiCollection(new UiSelector().resourceId(Utils.SHUTTER_ROOT_RES_ID));
        int shutterCount;
        try {
            shutterCount = shutterRootCollection.getChildCount();
            if (shutterCount <= 1) {
                return;
            }
            // switch to non-first shutter mode one by one
            for (int i = 1; i < shutterCount; i++) {
                UiObject shutterName = shutterRootCollection.getChild(
                        new UiSelector().className("android.widget.RelativeLayout").instance(i));
                UiObject shutterTextView = shutterName.getChild(
                        new UiSelector().className("android.widget.TextView"));
                shutterTextView.clickAndWaitForNewWindow();
                assertThat(Utils.waitForPreviewReady(), is(true));
            }
            // back to first shutter mode
            UiObject shutterMode = shutterRootCollection.getChild(
                    new UiSelector().className("android.widget.RelativeLayout").instance(0));
            UiObject shutterTextView = shutterMode.getChild(
                    new UiSelector().className("android.widget.TextView"));
            shutterTextView.click();
            assertThat(Utils.waitForPreviewReady(), is(true));
        } catch (UiObjectNotFoundException e) {
            e.printStackTrace();
            Log.w(TAG, "switchBetweenPhotoAndVideo with ui object not found!");
        }
    }

    private boolean switchToFacing(String facing) {
        Log.i(TAG, "[switchToFacing]+ facing:" + facing);
        UiObject2 facingSwitchObject = mUiDevice.findObject(By.res(FACING_SWITCHER_RES_ID));
        // in some projects, there has only one camera device.
        if (facingSwitchObject == null) {
            Log.w(TAG, "Can not find switch icon, there may be only has one camera device!");
            return false;
        }
        String currentFacing = facingSwitchObject.getContentDescription();
        if (facing.equals(currentFacing)) {
            return true;
        }
        facingSwitchObject.click();
        mUiDevice.wait(Until.hasObject(By.desc(facing)), Utils.OPERATION_TIMEOUT);
        assertThat(Utils.waitForPreviewReady(), is(true));
        Log.i(TAG, "[switchToFacing]-");
        return true;
    }
}