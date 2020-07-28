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
 * MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.pq;

import android.annotation.ProductApi;
import android.util.Log;
import android.os.SystemProperties;

public class PictureQuality {

    static boolean sLibStatus = true;

    /**
     * @internal
     */
    public static final int MODE_NORMAL = 0x000000;

    /**
     * @internal
     */
    public static final int MODE_CAMERA = 0x000001;
    public static final int MODE_VIDEO = 0x000002;
    /**
     * @internal
     */
    public static final int MODE_MASK   = 0x000001;

    public static final int PIC_MODE_STANDARD = 0;
    public static final int PIC_MODE_VIVID = 1;
    public static final int PIC_MODE_USER_DEF = 2;

    public static final int DCHIST_INFO_NUM = 20;
    public static class Hist {
        public int info[];

        public Hist() {
            info = new int[DCHIST_INFO_NUM];
            for (int i = 0; i < DCHIST_INFO_NUM; i++) {
                set(i, 0);
            }
        }

        public void set(int index, int value) {
            if ((0 <= index) && (index < DCHIST_INFO_NUM)) {
                this.info[index] = value;
            }
        }
    }

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

    public static final int GAMMA_LUT_SIZE = 512;
    public static class GammaLut {
        public int hwid;
        public int lut[];

        public GammaLut() {
            lut = new int[GAMMA_LUT_SIZE];
            for (int i = 0; i < GAMMA_LUT_SIZE; i++) {
                set(i, 0);
            }
        }

        public void set(int index, int value) {
            if ((0 <= index) && (index < GAMMA_LUT_SIZE)) {
                this.lut[index] = value;
            }
        }
    }

    static {
        try {
            Log.v("JNI_PQ", "loadLibrary");
            System.loadLibrary("jni_pq");
        } catch (UnsatisfiedLinkError e) {
            Log.e("JNI_PQ", "UnsatisfiedLinkError");
            sLibStatus = false;
        }
    }

    public static boolean getLibStatus() {
        return sLibStatus;
    }

    public static final int CAPABILITY_MASK_COLOR       = 0x00000001;
    public static final int CAPABILITY_MASK_SHARPNESS   = 0x00000002;
    public static final int CAPABILITY_MASK_GAMMA       = 0x00000004;
    public static final int CAPABILITY_MASK_DC          = 0x00000008;
    public static final int CAPABILITY_MASK_OD          = 0x00000010;
    public static int getCapability() {
        return nativeGetCapability();
    }


    public static String setMode(int mode, int step)
    {
        if(mode == PictureQuality.MODE_CAMERA)
        {
            nativeSetCameraPreviewMode(step);
        }
        else if (mode == PictureQuality.MODE_VIDEO)
        {
            nativeSetVideoPlaybackMode(step);
        }
        else
        {
            nativeSetGalleryNormalMode(step);
        }

        return null;
    }

    @ProductApi
    public static String setMode(int mode)
    {
        if(mode == PictureQuality.MODE_CAMERA)
        {
            nativeSetCameraPreviewMode(getDefaultOnTransitionStep());
        }
        else if (mode == PictureQuality.MODE_VIDEO)
        {
            nativeSetVideoPlaybackMode(getDefaultOnTransitionStep());
        }
        else
        {
            nativeSetGalleryNormalMode(getDefaultOnTransitionStep());
        }

        return null;
    }

    public static Hist getDynamicContrastHistogram(byte[] srcBuffer, int srcWidth, int srcHeight) {
        Hist outHist = new Hist();
        nativeGetDynamicContrastHistogram(srcBuffer, srcWidth, srcHeight, outHist);
        return outHist;
    }

    // enable PQ: 0 is disable, 1 is enable
    public static boolean enablePQ(int isEnable)
    {
        return nativeEnablePQ(isEnable);
    }

    // enable Color: 0 is disable, 1 is enable
    public static boolean enableColor(int isEnable)
    {
        return nativeEnableColor(isEnable);
    }

    public static boolean enableContentColor(int isEnable)
    {
        return nativeEnableContentColor(isEnable);
    }

    // enable Sharpness: 0 is disable, 1 is enable
    public static boolean enableSharpness(int isEnable)
    {
        return nativeEnableSharpness(isEnable);
    }

    // enable Dynamic Contrast: 0 is disable, 1 is enable
    public static boolean enableDynamicContrast(int isEnable)
    {
        return nativeEnableDynamicContrast(isEnable);
    }

    public static boolean enableDynamicSharpness(int isEnable)
    {
        return nativeEnableDynamicSharpness(isEnable);
    }

    // enable Color Effect: 0 is disable, 1 is enable
    public static boolean enableColorEffect(int isEnable)
    {
        return nativeEnableColorEffect(isEnable);
    }

    // enable Gamma: 0 is disable, 1 is enable
    public static boolean enableGamma(int isEnable)
    {
        return nativeEnableGamma(isEnable);
    }

    // enable OD Demo: 0 is disable, 1 is enable
    public static boolean enableOD(int isEnable) {
        return nativeEnableOD(isEnable);
    }

    public static boolean enableISOAdaptiveSharpness(int isEnable) {
        return nativeEnableISOAdaptiveSharpness(isEnable);
    }

    public static boolean enableUltraResolution(int isEnable) {
        return nativeEnableUltraResolution(isEnable);
    }

    // Picture Mode: STANDARD / VIVID / USER_DEF
    public static int getPictureMode()
    {
        return nativeGetPictureMode();
    }

    // Picture Mode: STANDARD / VIVID / USER_DEF
    public static boolean setPictureMode(int mode, int step)
    {
        return nativeSetPictureMode(mode, step);
    }

    public static boolean setPictureMode(int mode)
    {
        return nativeSetPictureMode(mode, getDefaultOffTransitionStep());
    }

    // COLOR ROI.
    public static boolean setColorRegion(int isEnable, int startX, int startY,
            int endX, int endY)
    {
        return nativeSetColorRegion(isEnable, startX, startY, endX, endY);
    }

    // Contrast
    public static Range getContrastIndexRange() {
        Range r = new Range();
        nativeGetContrastIndexRange(r);
        return r;
    }

    public static int getContrastIndex() {
        return nativeGetContrastIndex();
    }

    public static void setContrastIndex(int index, int step) {
        nativeSetContrastIndex(index, step);
    }

    public static void setContrastIndex(int index) {
        nativeSetContrastIndex(index, getDefaultOffTransitionStep());
    }

    // Saturation
    public static Range getSaturationIndexRange() {
        Range r = new Range();
        nativeGetSaturationIndexRange(r);
        return r;
    }

    public static int getSaturationIndex() {
        return nativeGetSaturationIndex();
    }

    public static void setSaturationIndex(int index, int step) {
        nativeSetSaturationIndex(index, step);
    }

    public static void setSaturationIndex(int index) {
        nativeSetSaturationIndex(index, getDefaultOffTransitionStep());
    }

    // PicBrightness
    public static Range getPicBrightnessIndexRange() {
        Range r = new Range();
        nativeGetPicBrightnessIndexRange(r);
        return r;
    }

    public static int getPicBrightnessIndex() {
        return nativeGetPicBrightnessIndex();
    }

    public static void setPicBrightnessIndex(int index, int step) {
        nativeSetPicBrightnessIndex(index, step);
    }

    public static void setPicBrightnessIndex(int index) {
        nativeSetPicBrightnessIndex(index, getDefaultOffTransitionStep());
    }

    // Sharpness
    public static Range getSharpnessIndexRange() {
        Range r = new Range();
        nativeGetSharpnessIndexRange(r);
        return r;
    }

    public static int getSharpnessIndex() {
        return nativeGetSharpnessIndex();
    }

    public static void setSharpnessIndex(int index) {
        nativeSetSharpnessIndex(index);
    }

    // Dynamic Contrast: 0 is disable, 1 is enable
    public static Range getDynamicContrastIndexRange() {
        Range r = new Range();
        nativeGetDynamicContrastIndexRange(r);
        return r;
    }

    public static int getDynamicContrastIndex() {
        return nativeGetDynamicContrastIndex();
    }

    public static void setDynamicContrastIndex(int index) {
        nativeSetDynamicContrastIndex(index);
    }

    // Color Effect: 0 is disable, 1 is enable
    public static Range getColorEffectIndexRange() {
        Range r = new Range();
        nativeGetColorEffectIndexRange(r);
        return r;
    }

    public static int getColorEffectIndex() {
        return nativeGetColorEffectIndex();
    }

    public static void setColorEffectIndex(int index) {
        nativeSetColorEffectIndex(index);
    }

    private static final String GAMMA_INDEX_PROPERTY_NAME = "persist.vendor.sys.pq.gamma.index";

    /**
     * Get index range of gamma. The valid gamma index is in [Range.min,
     * Range.max].
     *
     * @see getGammaIndex, setGammaIndex
     */
    public static Range getGammaIndexRange() {
        Range r = new Range();
        nativeGetGammaIndexRange(r);
        return r;
    }

    /**
     * Set gamma index. The index value should be in [Range.min, Range.max].
     *
     * @see getGammaIndexRange, getGammaIndex
     */
    public static void setGammaIndex(int index, int step) {
        nativeSetGammaIndex(index, step);
    }

    public static void setGammaIndex(int index) {
        nativeSetGammaIndex(index, getDefaultOnTransitionStep());
    }

    /**
     * Get current gamma index setting.
     *
     * @see setGammaIndex
     */
    public static int getGammaIndex() {
        return SystemProperties
                .getInt(GAMMA_INDEX_PROPERTY_NAME, getGammaIndexRange().defaultValue);
    }

    private static final String BLUELIGHT_DEFAULT_PROPERTY_NAME
        = "persist.vendor.sys.pq.bluelight.default";
    /**
     * Get index range of blue light defender. The valid index is in [Range.min, Range.max].
     *
     * @see setBlueLightStrength, getBlueLightStrength
     */
    public static Range getBlueLightStrengthRange() {
        Range r = new Range();
        r.set(0, 255, SystemProperties.getInt(BLUELIGHT_DEFAULT_PROPERTY_NAME, 128));
        return r;
    }

    /**
     * Set blue light defender strength. The index strength should be in [Range.min, Range.max].
     *
     * @see getBlueLightStrengthRange, getBlueLightStrength
     */
    public static boolean setBlueLightStrength(int strength, int step) {
        return nativeSetBlueLightStrength(strength, step);
    }

    public static boolean setBlueLightStrength(int strength) {
        return nativeSetBlueLightStrength(strength, getDefaultOffTransitionStep());
    }

    /**
     * Get current blue light defender strength setting.
     *
     * @see setBlueLightStrength
     */
    public static int getBlueLightStrength() {
        return nativeGetBlueLightStrength();
    }

    /**
     * Enable blue light defender.
     */
    public static boolean enableBlueLight(boolean enable, int step) {
        return nativeEnableBlueLight(enable, step);
    }

    public static boolean enableBlueLight(boolean enable) {
        return nativeEnableBlueLight(enable, getDefaultOnTransitionStep());
    }

    /**
     * Check whether blue light defender is enabled.
     */
    public static boolean isBlueLightEnabled() {
        return nativeIsBlueLightEnabled();
    }

    private static final String CHAMELEON_DEFAULT_PROPERTY_NAME
        = "persist.vendor.sys.pq.chameleon.default";
    /**
     * Get index range of chameleon. The valid index is in [Range.min, Range.max].
     *
     * @see setChameleonStrength, getChameleonStrength
     */
    public static Range getChameleonStrengthRange() {
        Range r = new Range();
        r.set(0, 255, SystemProperties.getInt(CHAMELEON_DEFAULT_PROPERTY_NAME, 128));
        return r;
    }

    /**
     * Set chameleon strength. The index strength should be in [Range.min, Range.max].
     *
     * @see setChameleonStrength, getChameleonStrength
     */
    public static boolean setChameleonStrength(int strength, int step) {
        return nativeSetChameleonStrength(strength, step);
    }

    public static boolean setChameleonStrength(int strength) {
        return nativeSetChameleonStrength(strength, getDefaultOffTransitionStep());
    }

    /**
     * Get current chameleon strength setting.
     *
     * @see setChameleonStrength
     */
    public static int getChameleonStrength() {
        return nativeGetChameleonStrength();
    }

    /**
     * Enable chameleon.
     */
    public static boolean enableChameleon(boolean enable, int step) {
        return nativeEnableChameleon(enable, step);
    }

    public static boolean enableChameleon(boolean enable) {
        return nativeEnableChameleon(enable, getDefaultOnTransitionStep());
    }

    /**
     * Check whether chameleon is enabled.
     */
    public static boolean isChameleonEnabled() {
        return nativeIsChameleonEnabled();
    }

    /**
     * Get default step for transition off .
     *
     * @see getDefaultOffTransitionStep
     */
    public static int getDefaultOffTransitionStep() {
        return nativeGetDefaultOffTransitionStep();
    }

    /**
     * Get default transition step .
     *
     * @see getDefaultOffTransitionStep
     */
    public static int getDefaultOnTransitionStep() {
        return nativeGetDefaultOnTransitionStep();
    }

    /**
     * Set Global PQ Switch.
     *
     * @see getGlobalPQSwitch
     */
    public static boolean setGlobalPQSwitch(int globalPQSwitch) {
        return nativeSetGlobalPQSwitch(globalPQSwitch);
    }

    /**
     * Get current global PQ Switch.
     *
     * @see setGlobalPQSwitch
     */
    public static int getGlobalPQSwitch() {
        return nativeGetGlobalPQSwitch();
    }

    /**
     * Set Global PQ Strength.
     *
     * @see getGlobalPQStrength
     */
    public static boolean setGlobalPQStrength(int globalPQStrength) {
        return nativeSetGlobalPQStrength(globalPQStrength);
    }

    /**
     * Get current global PQ Sthength.
     *
     * @see setGlobalPQStrength
     */
    public static int getGlobalPQStrength() {
        return nativeGetGlobalPQStrength();
    }

    /**
     * Get current global PQ Sthength Range.
     *
     */
    public static int getGlobalPQStrengthRange() {
        return nativeGetGlobalPQStrengthRange();
    }

    /**
     * Enable VideoHDR.
     */
    public static boolean enableVideoHDR(boolean enable) {
        return nativeEnableVideoHDR(enable);
    }

    /**
     * Check whether VideoHDR is enabled.
     */
    public static boolean isVideoHDREnabled() {
        return nativeIsVideoHDREnabled();
    }

    /**
     * enable MDP DRE
     */
     public static boolean enableMdpDRE(int enable) {
         return nativeEnableMdpDRE(enable);
    }

    /**
     * enable MDP CCORR
     */
     public static boolean enableMdpCCORR(int enable) {
         return nativeEnableMdpCCORR(enable);
    }

    /**
     * Get AAL function
     */
    public static int getAALFunction() {
        return nativeGetAALFunction();
    }

    /**
     * Set AAL function
     */
    public static void setAALFunction(int func) {
        nativeSetAALFunction(func);
    }

    public static void setAALFunctionProperty(int func) {
        nativeSetAALFunctionProperty(func);
    }

    /**
     * Set ESS Smart Backlight Strength
     */
    public static void setSmartBacklightStrength(int value) {
        nativeSetSmartBacklightStrength(value);
    }

    /**
     * Set DRE Readability Level
     */
    public static void setReadabilityLevel(int value) {
        nativeSetReadabilityLevel(value);
    }

    /**
     * Set DRE Low Backlight Readability Level
     */
    public static void setLowBLReadabilityLevel(int value) {
        nativeSetLowBLReadabilityLevel(value);
    }

    /**
     * Set ESS LEDMinStep.
     *
     */
    public static boolean setESSLEDMinStep(int value) {
        return nativeSetESSLEDMinStep(value);
    }

    /**
     * Get current ESS LEDMinStep
     *
     */
    public static int getESSLEDMinStep() {
        return nativeGetESSLEDMinStep();
    }

    /**
     * Set ESS OLEDMinStep.
     *
     */
    public static boolean setESSOLEDMinStep(int value) {
        return nativeSetESSOLEDMinStep(value);
    }

    /**
     * Get current ESS OLEDMinStep
     *
     */
    public static int getESSOLEDMinStep() {
        return nativeGetESSOLEDMinStep();
    }


    /**
     * Set current ExternalPanelNits setting.
     * Only can be used when isVideoHDREnabled() is true in external display scenario.
     */
    public static boolean setExternalPanelNits(int externalPanelNits) {
        return nativeSetExternalPanelNits(externalPanelNits);
    }

    /**
     * Get current ExternalPanelNits setting.
     * Only can be used when isVideoHDREnabled() is true in external display scenario.
     */
    public static int getExternalPanelNits() {
        return nativeGetExternalPanelNits();
    }

    /**
     * Set R, G, B channel gain value.
     */
    public static boolean setRGBGain(double r_gain, double g_gain, double b_gain, int step) {
        int r_gain_normalize = (int)(r_gain * 1024);
        int g_gain_normalize = (int)(g_gain * 1024);
        int b_gain_normalize = (int)(b_gain * 1024);

        return nativeSetRGBGain(r_gain_normalize, g_gain_normalize, b_gain_normalize, step);
    }

    /**
     * Get R, G, B channel gain value.
    */
    public static double[] getRGBGain() {
        double[] arr = new double[3];
        int[] arr_int = new int[3];
        /* initial */
        arr_int[0] = 1024;
        arr_int[1] = 1024;
        arr_int[2] = 1024;

        arr_int = nativeGetRGBGain();

        arr[0] = (double)arr_int[0] / 1024;
        arr[1] = (double)arr_int[1] / 1024;
        arr[2] = (double)arr_int[2] / 1024;

        return arr;
    }

    /**
     * Set Ccorr Matrix
     */
    public static boolean setCcorrMatrix(double[] matrix, int step) {
        int[] resultMatrix = new int[9];

        if (matrix != null && matrix.length != 9) {
            throw new IllegalArgumentException("Expected length: 9 (3x3 matrix)"
                    + ", actual length: " + matrix.length);
        }

        for (int i = 0; i < resultMatrix.length; i++) {
            resultMatrix[i] = (int)(matrix[i] * 1024);
        }

        return nativeSetCcorrMatrix(resultMatrix, step);
    }

    private static native int nativeGetCapability();

    private static native void nativeSetCameraPreviewMode(int step);
    private static native void nativeSetGalleryNormalMode(int step);
    private static native void nativeSetVideoPlaybackMode(int step);
    private static native void nativeGetDynamicContrastHistogram(byte[] srcBuffer, int srcWidth, int srcHeight, Hist outHist);

    private static native boolean nativeEnablePQ(int isEnable);
    private static native boolean nativeEnableColor(int isEnable);
    private static native boolean nativeEnableContentColor(int isEnable);
    private static native boolean nativeEnableSharpness(int isEnable);
    private static native boolean nativeEnableDynamicContrast(int isEnable);
    private static native boolean nativeEnableDynamicSharpness(int isEnable);
    private static native boolean nativeEnableColorEffect(int isEnable);
    private static native boolean nativeEnableGamma(int isEnable);
    private static native boolean nativeEnableOD(int isEnable);
    private static native boolean nativeEnableISOAdaptiveSharpness(int isEnable);
    private static native boolean nativeEnableUltraResolution(int isEnable);
    private static native int nativeGetPictureMode();
    private static native boolean nativeSetPictureMode(int mode, int step);
    private static native boolean nativeSetColorRegion(int isEnable, int startX, int startY,
            int endX, int endY);
    private static native void nativeGetContrastIndexRange(Range r);
    private static native int nativeGetContrastIndex();
    private static native void nativeSetContrastIndex(int index, int step);
    private static native void nativeGetSaturationIndexRange(Range r);
    private static native int nativeGetSaturationIndex();
    private static native void nativeSetSaturationIndex(int index, int step);
    private static native void nativeGetPicBrightnessIndexRange(Range r);
    private static native int nativeGetPicBrightnessIndex();
    private static native void nativeSetPicBrightnessIndex(int index, int step);
    private static native void nativeGetSharpnessIndexRange(Range r);
    private static native int nativeGetSharpnessIndex();
    private static native void nativeSetSharpnessIndex(int index);
    private static native void nativeGetDynamicContrastIndexRange(Range r);
    private static native int nativeGetDynamicContrastIndex();
    private static native void nativeSetDynamicContrastIndex(int index);
    private static native void nativeGetColorEffectIndexRange(Range r);
    private static native int nativeGetColorEffectIndex();
    private static native void nativeSetColorEffectIndex(int index);
    private static native void nativeGetGammaIndexRange(Range r);
    private static native void nativeSetGammaIndex(int index, int step);
    private static native boolean nativeSetBlueLightStrength(int strength, int step);
    private static native int nativeGetBlueLightStrength();
    private static native boolean nativeEnableBlueLight(boolean isEnable, int step);
    private static native boolean nativeIsBlueLightEnabled();
    private static native boolean nativeSetChameleonStrength(int strength, int step);
    private static native int nativeGetChameleonStrength();
    private static native boolean nativeEnableChameleon(boolean isEnable, int step);
    private static native boolean nativeIsChameleonEnabled();
    private static native int nativeGetDefaultOffTransitionStep();
    private static native int nativeGetDefaultOnTransitionStep();
    private static native boolean nativeSetGlobalPQSwitch(int globalPQSwitch);
    private static native int nativeGetGlobalPQSwitch();
    private static native boolean nativeSetGlobalPQStrength(int globalPQStrength);
    private static native int nativeGetGlobalPQStrength();
    private static native int nativeGetGlobalPQStrengthRange();
    private static native boolean nativeEnableVideoHDR(boolean isEnable);
    private static native boolean nativeIsVideoHDREnabled();
    private static native boolean nativeEnableMdpDRE(int enable);
    private static native boolean nativeEnableMdpCCORR(int enable);
    private static native int nativeGetAALFunction();
    private static native void nativeSetAALFunction(int func);
    private static native void nativeSetAALFunctionProperty(int func);
    private static native void nativeSetSmartBacklightStrength(int value);
    private static native void nativeSetReadabilityLevel(int value);
    private static native void nativeSetLowBLReadabilityLevel(int value);
    private static native int nativeGetESSLEDMinStep();
    private static native boolean nativeSetESSLEDMinStep(int value);
    private static native int nativeGetESSOLEDMinStep();
    private static native boolean nativeSetESSOLEDMinStep(int value);
    private static native boolean nativeSetExternalPanelNits(int externalPanelNits);
    private static native int nativeGetExternalPanelNits();
    private static native boolean nativeSetRGBGain(int r_gain, int g_gain, int b_gain, int step);
    private static native int[] nativeGetRGBGain();
    private static native boolean nativeSetCcorrMatrix(int[] matrix, int step);
}
