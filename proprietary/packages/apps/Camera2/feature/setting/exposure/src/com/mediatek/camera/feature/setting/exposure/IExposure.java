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
package com.mediatek.camera.feature.setting.exposure;

import java.util.List;

/**
 * Classes implementing this interface will received exposure config update from device.
 */
public interface IExposure {

    /**
     * Flow of flash request.
     */
    public static enum FlashFlow {
        /**
         * Indicate flash is not aivilable
         */
        FLASH_FLOW_NO_FLASH,
        /**
         * Indicate to do normal flash flow of back camera.
         */
        FLASH_FLOW_NORMAL,
        /**
         * Indicate to do standard external flash flow.
         */
        FLASH_FLOW_PANEL_STANDARD,
        /**
         * Indicate to do customized external flash,in this circumstance,pre-flash and main flash
         * will be fired by according to AE state.
         */
        FLASH_FLOW_PANEL_CUSTOMIZATION
    }

    /**
     * Initialize ExposureCompensation values which user can change from UI.
     * @param values The values user can change.
     */
    void initExposureCompensation(int[] values);

    /**
     * Classes implementing this interface will be called when contact device to exposure related
     * things.
     */
    interface Listener {
        /**
         * Update ev value.
         *
         * @param value The current ev value.
         */
        void updateEv(int value);

        /***
         * Notify to update exposure current value and supported values.
         *
         * @param currentValue  The exposure value other feature needed.
         * @param supportValues The exposure values other features need to supported.
         */
        void overrideExposureValue(String currentValue, List<String> supportValues);

        /**
         * Whether AePretrigger should be considered or not.Only API2 need to consider to do
         * AePretrigger.
         *
         * @return True if need to consider AePretrigger.Otherwise false.
         */
        boolean needConsiderAePretrigger();

        /**
         * Check whether to do capture after AE converted.
         * Note that only API2 need to check AE state before capture.
         * @return True if need to check AE converted before capture.
         */
        boolean checkTodoCapturAfterAeConverted();

        /**
         * Sets the auto-exposure lock state.
         *
         * @param lock new state of the auto-exposure lock. True means that
         *             auto-exposure is locked, false means that the auto-exposure
         *             routine is free to run normally.
         */
        void setAeLock(boolean lock);

        /**
         * Gets the state of the auto-exposure lock.
         *
         * @return State of the auto-exposure lock. Returns true if
         * auto-exposure is currently locked, and false otherwise.
         */
        boolean getAeLock();
    }
}
