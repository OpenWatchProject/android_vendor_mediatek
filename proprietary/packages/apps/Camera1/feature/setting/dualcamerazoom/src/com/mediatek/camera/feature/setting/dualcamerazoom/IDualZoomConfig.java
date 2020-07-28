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

package com.mediatek.camera.feature.setting.dualcamerazoom;

import com.mediatek.camera.portability.SystemProperties;

import java.util.List;

/**
 * This is for zoom control flow to perform zoom.
 */

public interface IDualZoomConfig {

    public static final String KEY_DUAL_ZOOM = "key_dual_zoom";
    // Parameter and view are valid.
    public static final String ZOOM_ON = "on";
    // Parameter and view are invalid.
    public static final String ZOOM_OFF = "off";
    // only parameter valid.
    public static final String ZOOM_LIMIT = "limit";
    public static final String PATTERN = "%.1f";
    public static final int RATIO_VALUE = 100;
    public static final String DEFAULT_RATIO = "1.0X";

    public static final float ZOOM_MIN_VALUE = 1.0f;
    public static final float ZOOM_MIN_TELE_VALUE = 1.25f;
    public static final float DEFAULT_VALUE = -1.0f;
    public static final int DEFAULT_ZOOM_RATIO = 6;
    public static final float ZOOM_MAX_VALUE = 3.0f;

    //Scale type name
    public static final String TYPE_DRAG = "drag";
    public static final String TYPE_PINCH = "pinch";
    public static final String TYPE_OTHER = "other";

    /**
     * This is to register zoom changed listener.
     *
     * @param zoomUpdateListener the zoom update listener.
     */
    public void setZoomUpdateListener(OnZoomLevelUpdateListener zoomUpdateListener);

    /**
     * This is onScale to prepare the zoom level.
     *
     * @param distanceRatio the zoom distance ratio.
     */
    public void onScalePerformed(double distanceRatio);

    /**
     * This is onScale to prepare the zoom level.
     *
     * @param isPinch true if pinch out/in, or bar.
     */
    public void onScaleType(boolean isPinch);

    /**
     * This is onScale to prepare the zoom level.
     *
     * @param typeName pinch or scale or other.
     */
    public void onScaleTypeName(String typeName);

    /**
     * This is to notify scale status.
     *
     * @param isSwitch is switch w/t or not.
     * @param isInit is view init or not.
     * @return true, if need request change value; false, not request.
     */
    public boolean onScaleStatus(boolean isSwitch, boolean isInit);

    /**
     * listener zoom level change.
     */
    public interface OnZoomLevelUpdateListener {
        /**
         * for zoom level update.
         *
         * @param level the zoom level
         */
        public void onZoomLevelUpdate(int level);

        /**
         * for slow motion zoom ratio update.
         *
         * @param ratio the zoom level
         */
        public void onZoomRatioUpdate(float ratio);

        /**
         * This is to get the override value by other feature.
         *
         * @return the override value.
         */
        public String onGetOverrideValue();

        /**
         * This is to get the sensor.
         *
         * @return true, if is tele sensor.
         */
        public boolean isZoomTele();

        /**
         * Single mode such as panorama/slow motion.
         *
         * @return true, if is single mode.
         */
        public boolean isSingleMode();

        /**
         * Get ratios supported.
         *
         * @param  ratios the supported ratios.
         */
        public void updateRatiosSupported(List<Integer> ratios);

        /**
         * Update switch ratio
         * @param switchRatio the supported switch ratio.
         */
        public void updateSwitchRatioSupported(int switchRatio);

        /**
         * Get max zoom supported.
         *
         * @param  maxZoom the supported max zoom value.
         */
        public void updateMaxZoomSupported(float maxZoom);
    }
}
