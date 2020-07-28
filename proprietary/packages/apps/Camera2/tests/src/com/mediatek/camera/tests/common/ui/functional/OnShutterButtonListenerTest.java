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
package com.mediatek.camera.tests.common.ui.functional;

import android.support.test.uiautomator.UiDevice;
import android.support.test.uiautomator.UiObject;
import android.support.test.uiautomator.UiObjectNotFoundException;
import android.support.test.uiautomator.UiSelector;

import com.mediatek.camera.CameraActivity;
import com.mediatek.camera.common.IAppUi;
import com.mediatek.camera.common.IAppUiListener;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.tests.CameraTestCaseBase;
import com.mediatek.camera.tests.Log;
import com.mediatek.camera.tests.Utils;

import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.timeout;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

/**
 * Function test for {@link com.mediatek.camera.common.IAppUiListener.OnShutterButtonListener}.
 */

public class OnShutterButtonListenerTest extends CameraTestCaseBase<CameraActivity> {
    private static final String TAG = OnShutterButtonListenerTest.class.getSimpleName();
    private static final int DEFAULT_PRIORITY = Integer.MAX_VALUE;
    private static final int HIGHEST_PRIORITY = 0;
    private static final int SET_PRIORITY = 1;
    private static final int TIMEOUT_VALUE = 2000;
    private IApp mApp;
    private IAppUi mAppUi;
    private UiDevice mUiDevice;
    private UiObject mPhotoShutterBtn;

    /**
     * Constructor of OnShutterButtonListenerTest.
     */
    public OnShutterButtonListenerTest() {
        super(CameraActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mApp = (CameraActivity) mActivity;
        mAppUi = mApp.getAppUi();
        mUiDevice = UiDevice.getInstance(mInstrumentation);
    }

    /**
     * Test app ui is not null.
     *
     * @throws Exception test with exception.
     */
    public void testGetAppUi() throws Exception {
        assertNotNull("[testGetAppUi] Activity's IAppUi instance is null!!!", mAppUi);
    }

    /**
     * Test ShutterButtonListener single click event, the registered listener can receive
     * call backs.
     *
     * @throws Exception test with exception.
     */
    public void testShutterButtonListenerSingleClick() throws Exception {
        IAppUiListener.OnShutterButtonListener onShutterButtonListener
                = mock(IAppUiListener.OnShutterButtonListener.class);

        mAppUi.registerOnShutterButtonListener(onShutterButtonListener, SET_PRIORITY);
        when(onShutterButtonListener.onShutterButtonClick()).thenReturn(false);

        mPhotoShutterBtn = mUiDevice.findObject(Utils.SHUTTER_BUTTON_SELECTOR);
        clickOnUiObject(mPhotoShutterBtn);
        mUiDevice.waitForIdle();

        verify(onShutterButtonListener, timeout(TIMEOUT_VALUE).times(1)).onShutterButtonClick();
    }

    /**
     * Test ShutterButtonListener single click event, only with highest priority registered listener
     * can receive call back.
     *
     * @throws Exception test with exception.
     */
    public void testShutterButtonListenerWithHighest() throws Exception {
        IAppUiListener.OnShutterButtonListener onShutterButtonListener
                = mock(IAppUiListener.OnShutterButtonListener.class);

        mAppUi.registerOnShutterButtonListener(onShutterButtonListener, HIGHEST_PRIORITY);
        when(onShutterButtonListener.onShutterButtonFocus(true)).thenReturn(false);
        when(onShutterButtonListener.onShutterButtonClick()).thenReturn(true);
        when(onShutterButtonListener.onShutterButtonFocus(false)).thenReturn(false);

        mPhotoShutterBtn = mUiDevice.findObject(Utils.SHUTTER_BUTTON_SELECTOR);
        clickOnUiObject(mPhotoShutterBtn);
        mUiDevice.waitForIdle();

        verify(onShutterButtonListener, timeout(TIMEOUT_VALUE).times(1))
                .onShutterButtonFocus(true);
        verify(onShutterButtonListener, timeout(TIMEOUT_VALUE).times(1))
                .onShutterButtonClick();
        verify(onShutterButtonListener, timeout(TIMEOUT_VALUE).times(1))
                .onShutterButtonFocus(false);
    }

    /**
     * Test ShutterButtonListener long click event, the registered listener can receive
     * call backs.
     *
     * @throws Exception test with exception.
     */
    public void testShutterButtonListenerLongClick() throws Exception {
        IAppUiListener.OnShutterButtonListener onShutterButtonListener
                = mock(IAppUiListener.OnShutterButtonListener.class);

        mAppUi.registerOnShutterButtonListener(onShutterButtonListener, HIGHEST_PRIORITY);
        when(onShutterButtonListener.onShutterButtonFocus(true)).thenReturn(false);
        when(onShutterButtonListener.onShutterButtonLongPressed()).thenReturn(false);
        when(onShutterButtonListener.onShutterButtonFocus(false)).thenReturn(false);

        mPhotoShutterBtn = mUiDevice.findObject(Utils.SHUTTER_BUTTON_SELECTOR);
        longClickOnUiObject(mPhotoShutterBtn);
        mUiDevice.waitForIdle();

        verify(onShutterButtonListener, timeout(TIMEOUT_VALUE).times(1))
                .onShutterButtonFocus(true);
        verify(onShutterButtonListener, timeout(TIMEOUT_VALUE).times(1))
                .onShutterButtonLongPressed();
        verify(onShutterButtonListener, timeout(TIMEOUT_VALUE).times(1))
                .onShutterButtonFocus(false);
    }

    /**
     * Test ShutterButtonListener, when register with the same priority, all listeners can receive
     * call backs.
     *
     * @throws Exception test with exception.
     */
    public void testShutterButtonListenerWithSamePriority() throws Exception {
        IAppUiListener.OnShutterButtonListener onShutterButtonListener1
                = mock(IAppUiListener.OnShutterButtonListener.class);
        IAppUiListener.OnShutterButtonListener onShutterButtonListener2
                = mock(IAppUiListener.OnShutterButtonListener.class);

        mAppUi.registerOnShutterButtonListener(onShutterButtonListener1, SET_PRIORITY);
        mAppUi.registerOnShutterButtonListener(onShutterButtonListener2, SET_PRIORITY);
        when(onShutterButtonListener1.onShutterButtonClick()).thenReturn(false);
        when(onShutterButtonListener2.onShutterButtonClick()).thenReturn(false);

        mPhotoShutterBtn = mUiDevice.findObject(Utils.SHUTTER_BUTTON_SELECTOR);
        clickOnUiObject(mPhotoShutterBtn);
        mUiDevice.waitForIdle();

        verify(onShutterButtonListener1, timeout(TIMEOUT_VALUE).times(1)).onShutterButtonClick();
        verify(onShutterButtonListener2, timeout(TIMEOUT_VALUE).times(1)).onShutterButtonClick();
    }

    /**
     * Test ShutterButtonListener with different priority,
     * when high priority break down the callback
     * low priority's listeners can not receive call back.
     *
     * @throws Exception test with exception.
     */
    public void testShutterButtonListenerWithDifferentPriority() throws Exception {
        IAppUiListener.OnShutterButtonListener onShutterButtonListener1
                = mock(IAppUiListener.OnShutterButtonListener.class);
        IAppUiListener.OnShutterButtonListener onShutterButtonListener2
                = mock(IAppUiListener.OnShutterButtonListener.class);

        mAppUi.registerOnShutterButtonListener(onShutterButtonListener1, SET_PRIORITY);
        mAppUi.registerOnShutterButtonListener(onShutterButtonListener2, SET_PRIORITY + 1);
        when(onShutterButtonListener1.onShutterButtonClick()).thenReturn(false);
        when(onShutterButtonListener2.onShutterButtonClick()).thenReturn(false);

        mPhotoShutterBtn = mUiDevice.findObject(Utils.SHUTTER_BUTTON_SELECTOR);
        clickOnUiObject(mPhotoShutterBtn);
        mUiDevice.waitForIdle();

        verify(onShutterButtonListener1, timeout(TIMEOUT_VALUE).times(1)).onShutterButtonClick();
        verify(onShutterButtonListener2, timeout(TIMEOUT_VALUE).times(1)).onShutterButtonClick();
    }

    /**
     * Test unRegister ShutterButtonListener, verify unRegistered listener can not receive callback.
     * @throws Exception test with exception.
     */
    public void testUnregisterShutterButtonListener() throws Exception {
        IAppUiListener.OnShutterButtonListener onShutterButtonListener1
                = mock(IAppUiListener.OnShutterButtonListener.class);
        IAppUiListener.OnShutterButtonListener onShutterButtonListener2
                = mock(IAppUiListener.OnShutterButtonListener.class);

        mAppUi.registerOnShutterButtonListener(onShutterButtonListener1, SET_PRIORITY);
        mAppUi.registerOnShutterButtonListener(onShutterButtonListener2, SET_PRIORITY + 1);
        mAppUi.unregisterOnShutterButtonListener(onShutterButtonListener1);
        when(onShutterButtonListener1.onShutterButtonClick()).thenReturn(false);
        when(onShutterButtonListener2.onShutterButtonClick()).thenReturn(false);

        mPhotoShutterBtn = mUiDevice.findObject(Utils.SHUTTER_BUTTON_SELECTOR);
        clickOnUiObject(mPhotoShutterBtn);
        mUiDevice.waitForIdle();

        verify(onShutterButtonListener1, never()).onShutterButtonClick();
        verify(onShutterButtonListener2, timeout(TIMEOUT_VALUE).times(1)).onShutterButtonClick();
    }

    private boolean clickOnUiObject(UiObject uiObject) {
        if (uiObject.exists()) {
            try {
                uiObject.clickAndWaitForNewWindow();
            } catch (UiObjectNotFoundException e) {
                Log.e(TAG, "UiObjectNotFoundException");
            }
            return true;
        } else {
            return false;
        }
    }

    private boolean longClickOnUiObject(UiObject uiObject) {
        if (uiObject.exists()) {
            try {
                uiObject.swipeDown(100);
            } catch (UiObjectNotFoundException e) {
                Log.e(TAG, "UiObjectNotFoundException");
            }
            return true;
        } else {
            return false;
        }
    }
}
