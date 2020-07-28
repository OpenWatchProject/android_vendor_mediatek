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
package com.mediatek.camera.feature.setting.focus;

import android.hardware.Camera;

import java.util.List;

/**
 * Classes implementing this interface will received config update from device.
 */
interface IFocus {
    /**
     * State of lock.
     */
    public static enum LockState {
        /**
         * Indicates not locked.
         */
        STATE_UNLOCKED,
        /**
         * Indicates going to be locked.
         */
        STATE_LOCKING,
        /**
         * Indicates has been locked.
         */
        STATE_LOCKED
    }

    /**
     * State of auto focus mode.
     */
    public static enum AfModeState {
        /**
         * Multi zone af is not supported in current platform.
         */
        STATE_INVALID,
        /**
         * Indicates current auto focus mode is single auto focus.
         */
        STATE_SINGLE,
        /**
         * Indicates current auto focus mode is multi zone auto focus.
         */
        STATE_MULTI
    }

    /**
     * Initialize platform supported values.
     *
     * @param platformSupportedValues The platform supported values which get from original
     *                                parameters.
     */
    void initPlatformSupportedValues(List<String> platformSupportedValues);

    /**
     * Initialize application supported entry values. May be spec required supported values.
     *
     * @param appSupportedEntryValues The application supported values which config by application.
     */
    void initAppSupportedEntryValues(List<String> appSupportedEntryValues);

    /**
     * Initialize focus supported setting values,setting with ui will use this values to show.
     * their choices.The values is the intersection of platform supported values and application
     * supported entry values.
     *
     * @param settingEntryValues The values application finally supported, the setting UI will use
     *                           refers these values to shown.
     */
    void initSettingEntryValues(List<String> settingEntryValues);

    /**
     * Classes implementing this interface will be called when contact device to do focus related
     * things.
     */
    interface Listener {
        /**
         * Whether can do focus or not by the platform.
         *
         * @return True if the device can do focus.
         */
        boolean isFocusCanDo();

        /**
         * Notify to change focus mode , will set the mode to device.
         *
         * @param currentValue The focus mode need change to .
         */
        void updateFocusMode(String currentValue);

        /***
         * Notify to update focus current mode and supported modes.
         *
         * @param currentValue  The focus mode other feature needed.
         * @param supportValues The focus modes other features need to supported.
         */
        void overrideFocusMode(String currentValue, List<String> supportValues);

        /**
         * Notify to update focus area.
         *
         * @param focusArea    The focus area.
         * @param meteringArea The focus metering area.
         */
        void updateFocusArea(List<Camera.Area> focusArea, List<Camera.Area> meteringArea);

        /**
         * Notify to update focus callback.
         */
        // FIXME: need to fix the api later
        void updateFocusCallback();

        /**
         * Notify to do auto focus.
         */
        void autoFocus();

        /**
         * Notify to do continue focus.
         */
        void restoreContinue();

        /**
         * Notify to cancel auto focus.
         */
        void cancelAutoFocus();

        /**
         * Disable to update focus state.eg:do disable to update focus state during capturing.
         *
         * @param disable True if need to disable to update focus state.Otherwise false.
         */
        void disableUpdateFocusState(boolean disable);

        /**
         * Get the current focus mode.
         * @return The current focus mode.
         */
        // FIXME: will remove the API later
        String getCurrentFocusMode();

        /**
         * Reset focus related configuration when needed.
         */
        void resetConfiguration();
    }
}
