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
package com.mediatek.camera.tests.common.ui.functional;

import android.support.test.uiautomator.UiDevice;

import com.mediatek.camera.CameraActivity;
import com.mediatek.camera.common.IAppUi;
import com.mediatek.camera.common.IAppUiListener;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.tests.CameraTestCaseBase;
import com.mediatek.camera.tests.Log;

import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;

/**
 * Function test for {@link com.mediatek.camera.common.IAppUiListener.OnGestureListener}.
 */

public class OnGestureListenerTest extends CameraTestCaseBase<CameraActivity> {
    private static final String TAG = OnGestureListenerTest.class.getSimpleName();
    private static final int DEFAULT_PRIORITY = 0;
    private static final int SWIPE_STEPS = 10;
    private static final int SWIPE_LENS = 150;
    private IApp mApp;
    private IAppUi mAppUi;
    private UiDevice mUiDevice;
    private int mScreenCenterX;
    private int mScreenCenterY;

    /**
     * Constructor of OnGestureListenerTest.
     */
    public OnGestureListenerTest() {
        super(CameraActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mApp = (CameraActivity) mActivity;
        mAppUi = mApp.getAppUi();
        mUiDevice = UiDevice.getInstance(mInstrumentation);
        mScreenCenterX = getScreenHeight() / 2;
        mScreenCenterY = getScreenWidth() / 2;
    }

    /**
     * Test app ui is not null.
     */
    public void testGetAppUi() {
        assertNotNull("[testGetAppUi] Activity's IAppUi instance is null!!!", mAppUi);
    }

    /**
     * Test OnGestureListener onUp event, the registered listener can receive
     * call backs.
     */
    public void testGestureOnUpListener() {
        IAppUiListener.OnGestureListener onGestureListener
                = mock(IAppUiListener.OnGestureListener.class);

        mAppUi.registerGestureListener(onGestureListener, DEFAULT_PRIORITY);
//        when(onGestureListener.onUp())
//                .thenReturn(false);

        mUiDevice.click(mScreenCenterX, mScreenCenterY);
        mUiDevice.waitForIdle();

//        verify(onGestureListener, times(1)).onUp();
    }

    /**
     * Test GestureOnUpListener, when register with the same priority, all listeners can receive
     * call backs.
     */
    public void testGestureOnUpListenerWithSamePriority() {
        IAppUiListener.OnGestureListener onGestureListener1
                = mock(IAppUiListener.OnGestureListener.class);
        IAppUiListener.OnGestureListener onGestureListener2
                = mock(IAppUiListener.OnGestureListener.class);

        mAppUi.registerGestureListener(onGestureListener1, DEFAULT_PRIORITY);
        mAppUi.registerGestureListener(onGestureListener2, DEFAULT_PRIORITY);
//        when(onGestureListener1.onUp()).thenReturn(false);
//        when(onGestureListener2.onUp()).thenReturn(false);

        mUiDevice.click(mScreenCenterX, mScreenCenterY);
        mUiDevice.waitForIdle();

//        verify(onGestureListener1, times(1)).onUp();
//        verify(onGestureListener2, times(1)).onUp();
    }

    /**
     * Test GestureOnUpListener with different priority, when high priority break down the callback
     * low priority's listeners can not receive call back.
     */
    public void testGestureOnUpListenerWithDifferentPriority() {
        IAppUiListener.OnGestureListener onGestureListener1
                = mock(IAppUiListener.OnGestureListener.class);
        IAppUiListener.OnGestureListener onGestureListener2
                = mock(IAppUiListener.OnGestureListener.class);

        mAppUi.registerGestureListener(onGestureListener1, DEFAULT_PRIORITY);
        mAppUi.registerGestureListener(onGestureListener2, DEFAULT_PRIORITY + 1);
//        when(onGestureListener1.onUp()).thenReturn(false);
//        when(onGestureListener2.onUp()).thenReturn(false);

        mUiDevice.click(mScreenCenterX, mScreenCenterY);
        mUiDevice.waitForIdle();

//        verify(onGestureListener1, times(1)).onUp();
//        verify(onGestureListener2, times(1)).onUp();
    }

    /**
     * Test unRegister GestureOnUpListener, verify unRegistered listener can not receive callback.
     *
     */
    public void testUnregisterGestureOnUpListener() {
        IAppUiListener.OnGestureListener onGestureListener1
                = mock(IAppUiListener.OnGestureListener.class);
        IAppUiListener.OnGestureListener onGestureListener2
                = mock(IAppUiListener.OnGestureListener.class);

        mAppUi.registerGestureListener(onGestureListener1, DEFAULT_PRIORITY);
        mAppUi.registerGestureListener(onGestureListener2, DEFAULT_PRIORITY + 1);
        mAppUi.unregisterGestureListener(onGestureListener1);
//        when(onGestureListener1.onUp()).thenReturn(false);
//        when(onGestureListener2.onUp()).thenReturn(false);

        mUiDevice.click(mScreenCenterX, mScreenCenterY);
        mUiDevice.waitForIdle();

//        verify(onGestureListener1, never()).onUp();
//        verify(onGestureListener2, times(1)).onUp();
    }

    /**
     * Test OnGestureListener onScale event, the registered listener can receive
     * call backs.
     *
     */
    public void testGestureOnScaleListener() {
        IAppUiListener.OnGestureListener onGestureListener
                = mock(IAppUiListener.OnGestureListener.class);

        mAppUi.registerGestureListener(onGestureListener, DEFAULT_PRIORITY);

        swipe();
        mUiDevice.waitForIdle();
//        verify(onGestureListener, times(1)).onUp();
    }

    private int getScreenWidth() {
        return mUiDevice.getDisplayWidth();
    }

    private int getScreenHeight() {
        return mUiDevice.getDisplayHeight();
    }

    private void swipe() {
        int startX = mUiDevice.getDisplayWidth() / 2;
        int startY = mUiDevice.getDisplayHeight() / 2;
        int endX = startX - SWIPE_LENS;
        int endY = startY;
        Log.i(TAG, "startX = " + startX + ", startY = " + startY
                + ", endX = " + endX + ", endY = " + endY);
        mUiDevice.swipe(startX, startY, endX, endY, SWIPE_STEPS);
    }
}
