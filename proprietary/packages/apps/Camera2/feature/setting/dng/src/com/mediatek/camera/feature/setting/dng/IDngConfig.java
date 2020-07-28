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

package com.mediatek.camera.feature.setting.dng;

import android.util.Size;

import java.util.List;

/**
 * It is to control the Dng flow.
 */

public interface IDngConfig {

    public static final String KEY_DNG = "key_dng";
    public static final String DNG_ON = "on";
    public static final String DNG_OFF = "off";

    /**
     * It is used to listen the dng config, dng support state, capture data callback,
     * and the display orientation.
     * @param l the listener.
     */
    public void setDngValueUpdateListener(OnDngValueUpdateListener l);

    /**
     * listener with dng value changed.
     */
    public interface OnDngValueUpdateListener {

        /**
         * To update the dng supported state.
         * @param dngList dng support state list.
         * @param isSupport support or not.
         */
        public void onDngValueUpdate(List<String> dngList, boolean isSupport);

        /**
         * For dng image save flow.
         * @param data the dng data to be saved.
         * @param rawSize the raw size.
         */
        public void onSaveDngImage(byte[] data, Size rawSize);

        /**
         * For update the display orientation.
         * @return the display orientation.
         */
        public int onDisplayOrientationUpdate();

        /**
         * To update the state whether image data is converting to dng.
         * @param isBusy true means dng is converting, dng creator busy.
         */
        public void onDngCreatorStateUpdate(boolean isBusy);
    }

    /**
     * It is for change setting vaule request when post restriction.
     */
    public void requestChangeOverrideValues();

    /**
     * set Dng on or off.
     * @param isOn true when dng on.
     * @param isTakeShutter true when take picture.
     */
    public void setDngStatus(boolean isOn, boolean isTakeShutter);

    /**
     * notify the Dng override value.
     * @param isOn the current value.
     */
    public void notifyOverrideValue(boolean isOn);

    /**
     * Used to relese device resource.
     */
    public void onModeClosed();
}
