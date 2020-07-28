/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.miravision.setting;

import android.content.Context;
import android.util.Log;

import com.mediatek.miravision.ui.AalSettingsFragment;
import com.mediatek.miravision.ui.OverDriveFragment;
import com.mediatek.miravision.ui.PictureModeFragment;
import com.mediatek.miravision.utils.Utils;

import com.mediatek.pq.PictureQuality;

/**
 * Collection of utility functions used in this package.
 */
public class MiraVisionJni {

    public static final String TAG = "MiraVisionJni";
    public static final int PIC_MODE_STANDARD = 0;
    public static final int PIC_MODE_VIVID = 1;
    public static final int PIC_MODE_USER_DEF = 2;
    static boolean sLibStatus = true;

    public static class Range {
        public int min;
        public int max;
        public int defaultValue;

        public Range() {
            set(0, 0, 0);
        }

        public void set(int min, int max, int defaultValue) {
            this.min = min;
            this.max = max;
            this.defaultValue = defaultValue;
        }
    }

    private MiraVisionJni() {
    }

    static {
        try {
            Log.v("MiraVisionJni", "loadLibrary");
            System.loadLibrary("MiraVision_jni");
        } catch (UnsatisfiedLinkError e) {
            Log.e("MiraVisionJni", "UnsatisfiedLinkError");
            sLibStatus = false;
        }
    }

    public static boolean getLibStatus() {
        return sLibStatus;
    }

    // enable PQ COLOR: 0 is disable, 1 is enable
    public static boolean nativeEnablePQColor(int isEnable) {
        Log.v("MiraVisionJni", "nativeEnablePQColor");
        return PictureQuality.enableColor(isEnable);
    }

    // Picture Mode: STANDARD / VIVID / USER_DEF
    public static int nativeGetPictureMode() {
        Log.v("MiraVisionJni", "nativeGetPictureMode");
        return PictureQuality.getPictureMode();
    }

    // Picture Mode: STANDARD / VIVID / USER_DEF
    public static boolean nativeSetPictureMode(int mode) {
        Log.v("MiraVisionJni", "nativeSetPictureMode");
        return PictureQuality.setPictureMode(mode);
    }

    // PQ COLOR ROI.
    public static boolean nativeSetPQColorRegion(int isEnable, int startX, int startY, int endX,
            int endY) {
        Log.v("MiraVisionJni", "nativeSetPQColorRegion");
        return PictureQuality.setColorRegion(isEnable, startX, startY, endX, endY);
    }

    // Contrast
    // private static native void nativeGetContrastIndexRange(Range r);

    public static Range getContrastIndexRange() {
        Range r = new Range();
        PictureQuality.Range PQrange;

        Log.v("MiraVisionJni", "getContrastIndexRange");
        PQrange = PictureQuality.getContrastIndexRange();

        r.set(PQrange.min, PQrange.max, PQrange.defaultValue);

        return r;
    }

    // private static native int nativeGetContrastIndex();

    public static int getContrastIndex() {
        return PictureQuality.getContrastIndex();
    }

    // private static native void nativeSetContrastIndex(int index);

    public static void setContrastIndex(int index) {
        PictureQuality.setContrastIndex(index);
    }

    // Saturation
    // private static native void nativeGetSaturationIndexRange(Range r);

    public static Range getSaturationIndexRange() {
        Range r = new Range();
        PictureQuality.Range PQrange;

        Log.v("MiraVisionJni", "getSaturationIndexRange");
        PQrange = PictureQuality.getSaturationIndexRange();

        r.set(PQrange.min, PQrange.max, PQrange.defaultValue);

        return r;
    }

    // private static native int nativeGetSaturationIndex();

    public static int getSaturationIndex() {
        return PictureQuality.getSaturationIndex();
    }

    // private static native void nativeSetSaturationIndex(int index);

    public static void setSaturationIndex(int index) {
        PictureQuality.setSaturationIndex(index);
    }

    // PicBrightness
    // private static native void nativeGetPicBrightnessIndexRange(Range r);

    public static Range getPicBrightnessIndexRange() {
        Range r = new Range();
        PictureQuality.Range PQrange;

        Log.v("MiraVisionJni", "getPicBrightnessIndexRange");
        PQrange = PictureQuality.getPicBrightnessIndexRange();

        r.set(PQrange.min, PQrange.max, PQrange.defaultValue);

        return r;
    }

    // private static native int nativeGetPicBrightnessIndex();

    public static int getPicBrightnessIndex() {
        return PictureQuality.getPicBrightnessIndex();
    }

    // private static native void nativeSetPicBrightnessIndex(int index);

    public static void setPicBrightnessIndex(int index) {
        PictureQuality.setPicBrightnessIndex(index);
    }

    // Sharpness
    public static boolean nativeSetTuningMode(int mode) {
        // PHASED OUT
        return true;
    }

    // private static native void nativeGetSharpnessIndexRange(Range r);

    public static Range getSharpnessIndexRange() {
        Range r = new Range();
        PictureQuality.Range PQrange;

        Log.v("MiraVisionJni", "getSharpnessIndexRange");
        PQrange = PictureQuality.getSharpnessIndexRange();

        r.set(PQrange.min, PQrange.max, PQrange.defaultValue);

        return r;
    }

    // private static native int nativeGetSharpnessIndex();

    public static int getSharpnessIndex() {
        return PictureQuality.getSharpnessIndex();
    }

    // private static native void nativeSetSharpnessIndex(int index);

    public static void setSharpnessIndex(int index) {
        PictureQuality.setSharpnessIndex(index);
    }

    // Dynamic Contrast: 0 is disable, 1 is enable
    // private static native void nativeGetDynamicContrastIndexRange(Range r);

    public static Range getDynamicContrastIndexRange() {
        Range r = new Range();
        PictureQuality.Range PQrange;

        Log.v("MiraVisionJni", "getDynamicContrastIndexRange");
        PQrange = PictureQuality.getDynamicContrastIndexRange();

        r.set(PQrange.min, PQrange.max, PQrange.defaultValue);

        return r;
    }

    // private static native int nativeGetDynamicContrastIndex();
    public static int getDynamicContrastIndex() {
        return PictureQuality.getDynamicContrastIndex();
    }

    // private static native void nativeSetDynamicContrastIndex(int index);
    public static void setDynamicContrastIndex(int index) {
        PictureQuality.setDynamicContrastIndex(index);
    }

    // private static native int nativeGetColorEffectIndex();
    public static int getColorEffectIndex() {
        return PictureQuality.getColorEffectIndex();
    }

    // private static native void nativeSetColorEffectIndex(int index)
    public static void setColorEffectIndex(int index) {
        PictureQuality.setColorEffectIndex(index);
    }

    // enable OD Demo: 0 is disable, 1 is enable
    public static boolean nativeEnableODDemo(int isEnable) {
        if (isEnable == 2) {

            Log.v("MiraVisionJni", "nativeEnableODDemo, query OD support!");
            int cap;

            cap = PictureQuality.getCapability();

            if ((cap & PictureQuality.CAPABILITY_MASK_OD) == PictureQuality.CAPABILITY_MASK_OD) {
                return true;
            } else {
                return false;
            }
        } else {
            Log.v("MiraVisionJni", "nativeEnableODDemo..");
            PictureQuality.enableOD(isEnable);
            return true;
        }
    }

    // private static final String GAMMA_INDEX_PROPERTY_NAME =
    // "persist.sys.gamma.index";

    // private static native void nativeGetGammaIndexRange(Range r);

    /**
     * Get index range of BlueLight. The valid BlueLight index is in [Range.min,
     * Range.max].
     *
     * @see getBlueLightIndex, setBlueLightIndex
     */
    public static Range getBlueLightIndexRange() {
        Range r = new Range();
        PictureQuality.Range PQrange;
        Log.v("MiraVisionJni", "getBlueLightIndexRange");
        PQrange = PictureQuality.getBlueLightStrengthRange();
        r.set(PQrange.min, PQrange.max, PQrange.defaultValue);
        return r;
    }

    /**
     * Set BlueLigh index. The index value should be in [Range.min, Range.max].
     *
     * @see getBlueLighIndexRange, getBlueLighIndex
     */
    public static void setBlueLightIndex(int index) {
        PictureQuality.setBlueLightStrength(index);
    }

    /**
     * Get current BlueLight index setting.
     *
     * @see setBlueLightIndex
     */
    public static int getBlueLightIndex() {
        return PictureQuality.getBlueLightStrength();
    }

    /**
     * Enable blue light defender.
     */
    public static boolean enableBlueLight(boolean enable) {
        return PictureQuality.enableBlueLight(enable);
    }

    /**
     * Check whether blue light defender is enabled.
     */
    public static boolean isBlueLightEnabled() {
        return PictureQuality.isBlueLightEnabled();
    }

    /**
     * Get index range of gamma. The valid gamma index is in [Range.min,
     * Range.max].
     *
     * @see getGammaIndex, setGammaIndex
     */
    public static Range getGammaIndexRange() {
        Range r = new Range();
        PictureQuality.Range PQrange;

        Log.v("MiraVisionJni", "getGammaIndexRange");
        PQrange = PictureQuality.getGammaIndexRange();

        r.set(PQrange.min, PQrange.max, PQrange.defaultValue);

        return r;
    }

    /**
     * Set gamma index. The index value should be in [Range.min, Range.max].
     *
     * @see getGammaIndexRange, getGammaIndex
     */
    public static void setGammaIndex(int index) {
        PictureQuality.setGammaIndex(index);
    }

    /**
     * Get current gamma index setting.
     *
     * @see setGammaIndex
     */
    public static int getGammaIndex() {
        return PictureQuality.getGammaIndex();
    }

    // private static native void nativeSetGammaIndex(int index);

    public static final int AAL_FUNC_CABC = 0x2;
    public static final int AAL_FUNC_DRE = 0x4;

    private static native void nativeSetAALFunction(int func);

    /**
     * Set AAL function by bit-wise control code.
     */
    public static void setAALFunction(int func) {
        PictureQuality.setAALFunctionProperty(func);
    }

    public static int getAALFunction() {
        ///M:ALPS03683084 change default from AAL_FUNC_CABC | AAL_FUNC_DRE to AAL_FUNC_CABC
        return PictureQuality.getAALFunction();
    }

    public static int getDefaultAALFunction() {
        ///M:ALPS03683084 change default from AAL_FUNC_CABC | AAL_FUNC_DRE to AAL_FUNC_CABC
        return (AAL_FUNC_CABC);
    }

    public static Range getUserBrightnessRange() {
        Range r = new Range();
        r.set(0, 1, 0);
        return r;
    }

    public static void setUserBrightness(int level) {
        // PHASED OUT
    }

    public static void resetPQ(Context context) {
        Log.d(TAG, "resetPQ");
        PictureQuality.setContrastIndex(PictureQuality.getContrastIndexRange().defaultValue);
        PictureQuality.setSaturationIndex(PictureQuality.getSaturationIndexRange().defaultValue);
        PictureQuality.setSharpnessIndex(PictureQuality.getSharpnessIndexRange().defaultValue);
        PictureQuality
                .setPicBrightnessIndex(PictureQuality.getPicBrightnessIndexRange().defaultValue);
        PictureQuality.setGammaIndex(PictureQuality.getGammaIndexRange().defaultValue);
        PictureQuality
              .setDynamicContrastIndex(PictureQuality.getDynamicContrastIndexRange().defaultValue);
        PictureQuality.setColorEffectIndex(PictureQuality.getColorEffectIndexRange().defaultValue);
        // Reset OD
        Utils utils = new Utils(context);
        utils.setSharePrefBoolenValue(OverDriveFragment.SHARED_PREFERENCES_OD_STATUS, false);
        utils.setSharePrefBoolenValue(PictureModeFragment.SHARED_PREFERENCES_USER_MODE_KEY, false);
        PictureQuality.enableOD(0);
        // Reset AAL
        setAALFunction(getDefaultAALFunction());
        // Reset BluLight
        PictureQuality.setBlueLightStrength(
                PictureQuality.getBlueLightStrengthRange().defaultValue);
        PictureQuality.enableBlueLight(false);
        // Reset Chameleon display
        PictureQuality
                .setChameleonStrength(PictureQuality.getChameleonStrengthRange().defaultValue);
        PictureQuality.enableChameleon(false);

        PictureQuality.setPictureMode(PictureQuality.PIC_MODE_STANDARD);
    }
}
