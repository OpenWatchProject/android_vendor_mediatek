/*
 *   Copyright Statement:
 *
 *     This software/firmware and related documentation ("MediaTek Software") are
 *     protected under relevant copyright laws. The information contained herein is
 *     confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 *     the prior written permission of MediaTek inc. and/or its licensors, any
 *     reproduction, modification, use or disclosure of MediaTek Software, and
 *     information contained herein, in whole or in part, shall be strictly
 *     prohibited.
 *
 *     MediaTek Inc. (C) 2016. All rights reserved.
 *
 *     BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *    THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *     RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 *     ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 *     WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 *     WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 *     NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 *     RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 *     TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 *     RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 *     OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 *     SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 *     RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 *     STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 *     ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 *     RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 *     MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 *     CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     The following software/firmware and/or related documentation ("MediaTek
 *     Software") have been modified by MediaTek Inc. All revisions are subject to
 *     any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.camera.common.app;

import android.app.Activity;
import android.net.Uri;
import android.view.KeyEvent;

import com.mediatek.camera.common.IAppUi;

import javax.annotation.Nonnull;

/**
 * The controller at app level.
 */
public interface IApp {
    int DEFAULT_PRIORITY = Integer.MAX_VALUE;
    /**
     * The interface used to notify key events.
     */
    interface KeyEventListener {
        /**
         * Called when a key was pressed down and not handled by any of the views
         * inside of the activity.
         *
         * @param keyCode key code.
         * @param event key event.
         * @return Return <code>true</code> to prevent this event from being propagated
         * further, or <code>false</code> to indicate that you have not handled this event
         * and it should continue to be propagated.
         */
        boolean onKeyDown(int keyCode, KeyEvent event);

        /**
         * Called when a key was released and not handled by any of the views
         * inside of the activity.
         *
         * @param keyCode key code.
         * @param event key event.
         * @return Return <code>true</code> to prevent this event from being propagated
         * further, or <code>false</code> to indicate that you have not handled this event
         * and it should continue to be propagated.
         */
        boolean onKeyUp(int keyCode, KeyEvent event);
    }

    /**
     * The interface used to notify back pressed event.
     */
    interface BackPressedListener {
        /**
         * Called when back key is pressed.
         *
         * @return Whether the back key event is processed.
         */
        boolean onBackPressed();
    }

    /**
     * G-sensor orientation change listener, it will notify the gsensor new
     * orientation value when it has been changed.
     */
    interface OnOrientationChangeListener {
        /**
         * Notify the new orientation value, the value will be one of 0,90,180,270.
         * @param orientation The changed orientation value.
         */
        void onOrientationChanged(int orientation);
    }

    /**
     * Get the activity.
     * @return the instance of activity.
     */
    @Nonnull
    Activity getActivity();

    /**
     * Get the instance of IAppUi, This method requires a thread-safe.
     *
     * @return an instance of IAppUi.
     */
    @Nonnull
    IAppUi getAppUi();

    /**
     * Keeps the screen turned on.
     *
     * @param enabled whether to keep the screen on.
     */
    void enableKeepScreenOn(boolean enabled);

    /**
     * Notifies the app of the newly captured media from mode.
     * @param uri the newly uri.
     * @param needNotify need notify to others or not.
     */
    void notifyNewMedia(@Nonnull Uri uri,  boolean needNotify);

    /**
     * Notifies the camera icon is selected.
     *
     * @param newCameraId new camera id.
     * @return true if can do camera selected, false if will ignore camera selected.
     */
    boolean notifyCameraSelected(@Nonnull String newCameraId);

    /**
     * Register KeyEventListener, this method is thread-safe.
     *
     * @param keyEventListener the registered KeyEventListener.
     * @param priority the priority to dispatch listener.
     */
    void registerKeyEventListener(@Nonnull KeyEventListener keyEventListener, int priority);

    /**
     * Unregister KeyEventListener.
     *
     * @param keyEventListener the listener to be unregistered.
     */
    void unRegisterKeyEventListener(@Nonnull KeyEventListener keyEventListener);

    /**
     * Register BackPressedListener, this method is thread-safe.
     *
     * @param backPressedListener the registered BackPressedListener.
     * @param priority            the priority to dispatch listener.
     */
    void registerBackPressedListener(
            @Nonnull BackPressedListener backPressedListener, int priority);

    /**
     * Unregistered BackPressedListener.
     *
     * @param backPressedListener the listener to be unregistered.
     */
    void unRegisterBackPressedListener(@Nonnull BackPressedListener backPressedListener);

    /**
     * Register OnOrientationChangeListener.
     * @param listener The listener.
     */
    void registerOnOrientationChangeListener(OnOrientationChangeListener listener);

    /**
     * Unregister OnOrientationChangeListener.
     * @param listener The listener.
     */
    void unregisterOnOrientationChangeListener(OnOrientationChangeListener listener);

    /**
     * enable GSensor Orientation.
     */
    void enableGSensorOrientation();

    /**
     * disable GSensor Orientation.
     */
    void disableGSensorOrientation();

    /**
     * Get current gsensor orientation, the value will be one of 0,90,180,270.
     * @return  The orientation value.
     */
    int getGSensorOrientation();
}