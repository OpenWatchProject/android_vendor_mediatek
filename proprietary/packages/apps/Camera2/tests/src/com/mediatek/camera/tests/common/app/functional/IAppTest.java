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
package com.mediatek.camera.tests.common.app.functional;

import android.support.test.uiautomator.UiDevice;
import android.view.KeyEvent;
import android.view.WindowManager;

import com.mediatek.camera.CameraActivity;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.app.IApp.BackPressedListener;
import com.mediatek.camera.common.app.IApp.KeyEventListener;
import com.mediatek.camera.tests.CameraTestCaseBase;

import org.mockito.ArgumentCaptor;

import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

/**
 * Functional tests for {@link com.mediatek.camera.common.app.IApp}.
 */
public class IAppTest extends CameraTestCaseBase<CameraActivity> {
    private static final int DEFAULT_PRIORITY = 1;
    private static final int CHECK_CLEAR_SCREEN_ON_FLAG_DELAY_MS = 132000; // 2.2min
    private static final int CHECK_ADD_SCREEN_ON_FLAG_DELAY_MS = 1000;
    private IApp mApp;
    private UiDevice mUiDevice;

    /**
     * Constructor of IAppTest.
     */
    public IAppTest() {
        super(CameraActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mApp = (CameraActivity) mActivity;
        mUiDevice = UiDevice.getInstance(mInstrumentation);
    }

    /**
     * Test activity is not null.
     *
     * @throws Exception test with exception.
     */
    public void testGetActivity() throws Exception {
        assertNotNull("[testGetActivity] activity is null!!!", mApp.getActivity());
    }

    /**
     * Test app ui is not null.
     *
     * @throws Exception test with exception.
     */
    public void testGetAppUi() throws Exception {
        assertNotNull("[testGetAppUi] Activity's IAppUi instance is null!!!", mApp.getAppUi());
    }

    /**
     * Test BackPressedListener, when register with the same priority, all listeners can receive
     * call backs.
     *
     * @throws Exception test with exception.
     */
    public void testBackPressedListenerWithSamePriority() throws Exception {
        BackPressedListener backPressedListener1 = mock(BackPressedListener.class);
        BackPressedListener backPressedListener2 = mock(BackPressedListener.class);

        mApp.registerBackPressedListener(backPressedListener1, DEFAULT_PRIORITY);
        mApp.registerBackPressedListener(backPressedListener2, DEFAULT_PRIORITY);
        when(backPressedListener1.onBackPressed()).thenReturn(false);
        when(backPressedListener2.onBackPressed()).thenReturn(false);

        mUiDevice.pressBack();
        mUiDevice.waitForIdle();

        verify(backPressedListener1, times(1)).onBackPressed();
        verify(backPressedListener2, times(1)).onBackPressed();
    }

    /**
     * Test KeyEventListener, when register with the same priority, all listeners can receive
     * onKeyDown and onKeyUp call backs.
     *
     * @throws Exception test with exception.
     */
    public void testKeyEventListenerWithSamePriority() throws Exception {
        ArgumentCaptor<Integer> keyCodeCapture = ArgumentCaptor.forClass(Integer.class);
        ArgumentCaptor<KeyEvent> keyEventCapture = ArgumentCaptor.forClass(KeyEvent.class);
        KeyEventListener keyEventListener1 = mock(KeyEventListener.class);
        KeyEventListener keyEventListener2 = mock(KeyEventListener.class);

        mApp.registerKeyEventListener(keyEventListener1, DEFAULT_PRIORITY);
        mApp.registerKeyEventListener(keyEventListener2, DEFAULT_PRIORITY);
        when(keyEventListener1.onKeyDown(keyCodeCapture.capture(),
                keyEventCapture.capture())).thenReturn(false);
        when(keyEventListener1.onKeyUp(keyCodeCapture.capture(),
                keyEventCapture.capture())).thenReturn(false);
        when(keyEventListener2.onKeyDown(keyCodeCapture.capture(),
                keyEventCapture.capture())).thenReturn(false);
        when(keyEventListener2.onKeyUp(keyCodeCapture.capture(),
                keyEventCapture.capture())).thenReturn(false);

        mUiDevice.pressKeyCode(KeyEvent.KEYCODE_VOLUME_DOWN);
        mUiDevice.waitForIdle();

        verify(keyEventListener1, times(1)).onKeyDown(keyCodeCapture.capture(),
                keyEventCapture.capture());
        assertEquals(KeyEvent.KEYCODE_VOLUME_DOWN, (int) keyCodeCapture.getValue());
        verify(keyEventListener1, times(1)).onKeyUp(keyCodeCapture.capture(),
                keyEventCapture.capture());
        assertEquals(KeyEvent.KEYCODE_VOLUME_DOWN, (int) keyCodeCapture.getValue());

        verify(keyEventListener2, times(1)).onKeyDown(keyCodeCapture.capture(),
                keyEventCapture.capture());
        assertEquals(KeyEvent.KEYCODE_VOLUME_DOWN, (int) keyCodeCapture.getValue());
        verify(keyEventListener2, times(1)).onKeyUp(keyCodeCapture.capture(),
                keyEventCapture.capture());
        assertEquals(KeyEvent.KEYCODE_VOLUME_DOWN, (int) keyCodeCapture.getValue());
    }

    /**
     * Test OnOrientationChangeListener,  all listeners can receive call backs.
     *
     * @throws Exception test with exception.
     */
    public void testOnOrientationChangeListener() throws Exception {
        //TODO how to trigger an g-sensor event?
    }

    /**
     * Test BackPressedListener with different priority, when high priority break down the callback
     * low priority's listeners can not receive call back.
     *
     * @throws Exception test with exception.
     */
    public void testBackPressedListenerWithDifferentPriority() throws Exception {
        BackPressedListener backPressedListener1 = mock(BackPressedListener.class);
        BackPressedListener backPressedListener2 = mock(BackPressedListener.class);

        mApp.registerBackPressedListener(backPressedListener1, DEFAULT_PRIORITY);
        mApp.registerBackPressedListener(backPressedListener2, DEFAULT_PRIORITY + 1);
        when(backPressedListener1.onBackPressed()).thenReturn(true);
        when(backPressedListener2.onBackPressed()).thenReturn(false);

        mUiDevice.pressBack();
        mUiDevice.waitForIdle();

        verify(backPressedListener1, times(1)).onBackPressed();
        verify(backPressedListener2, never()).onBackPressed();
    }

    /**
     * Test KeyEventListener with different priority, when high priority break down the callback
     * low priority's listeners can not receive call back.
     *
     * @throws Exception test with exception.
     */
    public void testKeyEventListenerWithDifferentPriority() throws Exception {
        ArgumentCaptor<Integer> keyCodeCapture = ArgumentCaptor.forClass(Integer.class);
        ArgumentCaptor<KeyEvent> keyEventCapture = ArgumentCaptor.forClass(KeyEvent.class);
        KeyEventListener keyEventListener1 = mock(KeyEventListener.class);
        KeyEventListener keyEventListener2 = mock(KeyEventListener.class);

        mApp.registerKeyEventListener(keyEventListener1, DEFAULT_PRIORITY);
        mApp.registerKeyEventListener(keyEventListener2, DEFAULT_PRIORITY + 1);
        when(keyEventListener1.onKeyDown(keyCodeCapture.capture(),
                keyEventCapture.capture())).thenReturn(true);
        when(keyEventListener1.onKeyUp(keyCodeCapture.capture(),
                keyEventCapture.capture())).thenReturn(true);
        when(keyEventListener2.onKeyDown(keyCodeCapture.capture(),
                keyEventCapture.capture())).thenReturn(false);
        when(keyEventListener2.onKeyUp(keyCodeCapture.capture(),
                keyEventCapture.capture())).thenReturn(false);

        mUiDevice.pressKeyCode(KeyEvent.KEYCODE_VOLUME_DOWN);
        mUiDevice.waitForIdle();

        verify(keyEventListener1, times(1)).onKeyDown(keyCodeCapture.capture(),
                keyEventCapture.capture());
        assertEquals(KeyEvent.KEYCODE_VOLUME_DOWN, (int) keyCodeCapture.getValue());
        verify(keyEventListener1, times(1)).onKeyUp(keyCodeCapture.capture(),
                keyEventCapture.capture());
        assertEquals(KeyEvent.KEYCODE_VOLUME_DOWN, (int) keyCodeCapture.getValue());

        verify(keyEventListener2, never()).onKeyDown(keyCodeCapture.capture(),
                keyEventCapture.capture());
        verify(keyEventListener2, never()).onKeyUp(keyCodeCapture.capture(),
                keyEventCapture.capture());
    }

    /**
     * Test unRegister BackPressedListener, verify unRegistered listener can not receive callback.
     * @throws Exception test with exception.
     */
    public void testUnregisterBackPressedListener() throws Exception {
        BackPressedListener backPressedListener1 = mock(BackPressedListener.class);
        BackPressedListener backPressedListener2 = mock(BackPressedListener.class);

        mApp.registerBackPressedListener(backPressedListener1, DEFAULT_PRIORITY);
        mApp.registerBackPressedListener(backPressedListener2, DEFAULT_PRIORITY + 1);
        mApp.unRegisterBackPressedListener(backPressedListener2);
        when(backPressedListener1.onBackPressed()).thenReturn(false);
        when(backPressedListener2.onBackPressed()).thenReturn(false);

        mUiDevice.pressBack();
        mUiDevice.waitForIdle();

        verify(backPressedListener1, times(1)).onBackPressed();
        verify(backPressedListener2, never()).onBackPressed();
    }

    /**
     * Test unRegister OnOrientationChangeListener, verify unRegistered listener can not
     * receive callback.
     * @throws Exception test with exception.
     */
    public void testUnregisterOnOrientationChangeListener() throws Exception {
        //TODO how to trigger an g-sensor event?
    }

    /**
     * Test unRegister KeyEventListener, verify unRegistered listener can not receive callback.
     * @throws Exception test with exception.
     */
    public void testUnregisterKeyEventListener() throws Exception {
        ArgumentCaptor<Integer> keyCodeCapture = ArgumentCaptor.forClass(Integer.class);
        ArgumentCaptor<KeyEvent> keyEventCapture = ArgumentCaptor.forClass(KeyEvent.class);
        KeyEventListener keyEventListener1 = mock(KeyEventListener.class);
        KeyEventListener keyEventListener2 = mock(KeyEventListener.class);

        mApp.registerKeyEventListener(keyEventListener1, DEFAULT_PRIORITY);
        mApp.registerKeyEventListener(keyEventListener2, DEFAULT_PRIORITY + 1);
        mApp.unRegisterKeyEventListener(keyEventListener2);
        when(keyEventListener1.onKeyDown(keyCodeCapture.capture(),
                keyEventCapture.capture())).thenReturn(false);
        when(keyEventListener1.onKeyUp(keyCodeCapture.capture(),
                keyEventCapture.capture())).thenReturn(false);
        when(keyEventListener2.onKeyDown(keyCodeCapture.capture(),
                keyEventCapture.capture())).thenReturn(false);
        when(keyEventListener2.onKeyUp(keyCodeCapture.capture(),
                keyEventCapture.capture())).thenReturn(false);

        mUiDevice.pressKeyCode(KeyEvent.KEYCODE_VOLUME_DOWN);
        mUiDevice.waitForIdle();

        verify(keyEventListener1, times(1)).onKeyDown(keyCodeCapture.capture(),
                keyEventCapture.capture());
        assertEquals(KeyEvent.KEYCODE_VOLUME_DOWN, (int) keyCodeCapture.getValue());
        verify(keyEventListener1, times(1)).onKeyUp(keyCodeCapture.capture(),
                keyEventCapture.capture());
        assertEquals(KeyEvent.KEYCODE_VOLUME_DOWN, (int) keyCodeCapture.getValue());

        verify(keyEventListener2, never()).onKeyDown(keyCodeCapture.capture(),
                keyEventCapture.capture());
        verify(keyEventListener2, never()).onKeyUp(keyCodeCapture.capture(),
                keyEventCapture.capture());
    }

    /**
     * When enable keep screen on, check FLAG_KEEP_SCREEN_ON is added in
     * window manager's flags.
     *
     * @throws Exception test with exception.
     */
    public void testEnableKeepScreenOn() throws Exception {
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mApp.enableKeepScreenOn(true);
            }
        });
        Thread.sleep(CHECK_ADD_SCREEN_ON_FLAG_DELAY_MS);
        int windowFlags = mActivity.getWindow().getAttributes().flags;
        int screenOnFlags = setFlags(windowFlags, WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON,
                WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        assertEquals(windowFlags, screenOnFlags);
    }

    /**
     * When disable keep screen on after 2min, can not find FLAG_KEEP_SCREEN_ON in window manger's
     * flags.
     *
     * @throws Exception test with exception.
     */
    public void testDisableKeepScreenOn() throws Exception {
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mApp.enableKeepScreenOn(false);
            }
        });
        Thread.sleep(CHECK_CLEAR_SCREEN_ON_FLAG_DELAY_MS);
        int windowFlags = mActivity.getWindow().getAttributes().flags;
        int clearScreenOnFlags = setFlags(windowFlags, 0,
                WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        assertEquals(windowFlags, clearScreenOnFlags);
    }

    /**
     * Test new photo broadcast is sent, the only take effects before android N.
     * In android N, we check new photo broadcast is not sent.
     *
     * @throws Exception test with exception.
     */
    public void testNotifyNewPhoto() throws Exception {
        //TODO how to generate a photo uri for test?
    }

    /**
     * Test new video broadcast is sent, the only take effects before android N.
     * In android N, we check new video broadcast is not sent.
     *
     * @throws Exception test with exception.
     */
    public void testNotifyNewVideo() throws Exception {
        //TODO how to generate a video uri for test?
    }

    private int setFlags(int target, int flags, int mask) {
        return (target & ~mask) | (flags & mask);
    }
}