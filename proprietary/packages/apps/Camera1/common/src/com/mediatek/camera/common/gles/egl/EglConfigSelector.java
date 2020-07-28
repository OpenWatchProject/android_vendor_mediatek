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

package com.mediatek.camera.common.gles.egl;

import android.annotation.TargetApi;
import android.graphics.ImageFormat;
import android.graphics.PixelFormat;
import android.opengl.EGL14;
import android.opengl.EGLConfig;
import android.opengl.EGLDisplay;
import android.os.Build;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil.Tag;

import java.util.ArrayList;
import java.util.Arrays;

/**
 * This class is used to create a specific EGLConfig.
*/
@TargetApi(Build.VERSION_CODES.JELLY_BEAN_MR2)
public class EglConfigSelector {
    private static final Tag TAG = new Tag(EglConfigSelector.class.getSimpleName());
    private static final int EGL_RECORDABLE_ANDROID = 0x3142;
    private static final int EGL_OPENGL_ES2_BIT = 4;
    private EGLConfigChooser mEGLConfigChooser;
    private int mSelectedPixelFormat = -1;
    private ArrayList<Integer> mSupportedOutputFormats = new ArrayList<>();

    /**
     * Set supported output formats.
     *
     * @param formats supported output formats.
     */
    public void setSupportedFormats(int[] formats) {
        LogHelper.d(TAG, "[setSupportedFormats] setSupportedFormats,format: "
                + ((formats == null) ? "null" : Arrays.toString(formats)));
        for (int format : formats) {
            mSupportedOutputFormats.add(format);
        }
    }

    /**
      * Install a custom EGLConfigChooser.
      * <p>If this method is
      * called, it must be called before GL thread initialize.
      * <p>
      * If no setEGLConfigChooser method is called, then by default the
      * view will choose a config as close to 16-bit RGB as possible, with
      * a depth buffer as close to 16 bits as possible.
      *
      * @param configChooser set self-defined EGLConfigChooser.
      */
     public void setEGLConfigChooser(EGLConfigChooser configChooser) {
         mEGLConfigChooser = configChooser;
     }

    /**
     * Install a config chooser which will choose a config
     * with at least the specified component sizes, and as close
     * to the specified component sizes as possible.
     * <p>If this method is
     * called, it must be called before GL thread initialize.
     * <p>
     * If no setEGLConfigChooser method is called, then by default the
     * view will choose a config as close to 16-bit RGB as possible, with
     * a depth buffer as close to 16 bits as possible.
     *
     * @param redSize red size.
     * @param greenSize green size.
     * @param blueSize blue size.
     * @param alphaSize alpha size.
     * @param depthSize depth size.
     * @param stencilSize stencil size.
     */
    public void setEGLConfigChooser(
            int redSize,
            int greenSize,
            int blueSize,
            int alphaSize,
            int depthSize,
            int stencilSize) {
        setEGLConfigChooser(new ComponentSizeChooser(
                redSize,
                greenSize,
                blueSize,
                alphaSize,
                depthSize,
                stencilSize));
    }

    /**
     * Choose EGL14 EGLConfig.
     *
     * @param display the specified EGLDisplay.
     * @param recording whether is recording surface.
     * @return an selected EGLConfig.
     */
    public EGLConfig chooseConfigEGL14(EGLDisplay display, boolean recording) {
        if (mEGLConfigChooser == null) {
            mEGLConfigChooser = new SimpleEGLConfigChooser();
        }
        if (mSupportedOutputFormats.size() <= 0) {
            mSupportedOutputFormats.add(PixelFormat.RGBA_8888);
        }
        return mEGLConfigChooser.chooseConfigEGL14(display, recording);
    }

    public int getSelectedPixelFormat() {
        return mSelectedPixelFormat;
    }

    /**
     * An interface for choosing an EGLConfig configuration from a list of
     * potential configurations.
     */
    private interface EGLConfigChooser {
        /**
         * Choose a configuration from the list. Implementors typically
         * implement this method by calling
         * {@link EGL14#eglChooseConfig} and iterating through the results. Please consult the
         * EGL specification available from The Khronos Group to learn how to call eglChooseConfig.
         */
        EGLConfig chooseConfigEGL14(EGLDisplay display, boolean recording);
    }

    /**
     * An EglConfig format enum, used to identify you want to choose which type of format.
     */
    private enum EglConfigFormat {
        YUV,
        RGB,
        RGBA
    }

    /**
     * An abstract base config chooser.
     */
    private abstract class BaseConfigChooser implements EGLConfigChooser {
        public BaseConfigChooser(int[] configSpec) {
            mConfigSpec = configSpec;
        }

        @Override
        public EGLConfig chooseConfigEGL14(
                EGLDisplay display, boolean recording) {
            EGLConfig[] configs = new EGLConfig[100];
            int[] numConfigs = new int[1];
            int surfaceType = EGL14.EGL_WINDOW_BIT | EGL14.EGL_PBUFFER_BIT;
            if (recording) {
                surfaceType = EGL14.EGL_WINDOW_BIT;
                mConfigSpec[mConfigSpec.length - 3] = EGL_RECORDABLE_ANDROID;
                mConfigSpec[mConfigSpec.length - 2] = 1;
            }
            mConfigSpec[mConfigSpec.length - 5] = EGL14.EGL_SURFACE_TYPE;
            mConfigSpec[mConfigSpec.length - 4] = surfaceType;
            if (!EGL14.eglChooseConfig(display, mConfigSpec, 0, configs, 0, configs.length,
                    numConfigs, 0)) {
                throw new RuntimeException("unable to find ES2 EGL config in EGL14");
            }
            EGLConfig config = chooseConfigEGL14(display, configs, numConfigs[0], recording);
            if (config == null) {
                throw new IllegalArgumentException("No config chosen");
            }
            return config;
        }

        abstract EGLConfig chooseConfigEGL14(EGLDisplay display,
                EGLConfig[] configs, int configNum, boolean recording);

        protected int[] mConfigSpec;
    }

    /**
     * Component size chooser.
     */
    private class ComponentSizeChooser extends BaseConfigChooser {
        public ComponentSizeChooser(int redSize, int greenSize, int blueSize,
                int alphaSize, int depthSize, int stencilSize) {
            super(new int[] {
                    EGL14.EGL_RED_SIZE, redSize,
                    EGL14.EGL_GREEN_SIZE, greenSize,
                    EGL14.EGL_BLUE_SIZE, blueSize,
                    EGL14.EGL_ALPHA_SIZE, alphaSize,
                    EGL14.EGL_DEPTH_SIZE, depthSize,
                    EGL14.EGL_STENCIL_SIZE, stencilSize,
                    EGL14.EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                    EGL14.EGL_NONE, 0, // placeholder for surface type
                    EGL14.EGL_NONE, 0, // placeholder for recordable
                    EGL14.EGL_NONE});
            mValue = new int[1];
            mRedSize = redSize;
            mGreenSize = greenSize;
            mBlueSize = blueSize;
            mAlphaSize = alphaSize;
            mDepthSize = depthSize;
            mStencilSize = stencilSize;
            LogHelper.d(TAG, "R:" + mRedSize +
                             ",G:" + mGreenSize +
                             ",B:" + mBlueSize +
                             ",A:" + mAlphaSize +
                             ",Depth:" + mDepthSize +
                             ",Stencil:" + mStencilSize);
        }

        @Override
        EGLConfig chooseConfigEGL14(
                EGLDisplay display,
                EGLConfig[] configs,
                int configNum,
                boolean recording) {
            EGLConfig closestConfig = null;
            // try to find yuv egl format
            if (hasSpecifiedEglConfigFormat(EglConfigFormat.YUV,
                    mSupportedOutputFormats)) {
                closestConfig =
                        findClosestEglConfig(display, configs, configNum, EglConfigFormat.YUV);
            }
            // if yuv egl format config not support, try find a RGB config
            if (hasSpecifiedEglConfigFormat(EglConfigFormat.RGB,
                    mSupportedOutputFormats)) {
                if (closestConfig == null) {
                    closestConfig =
                        findClosestEglConfig(display, configs, configNum, EglConfigFormat.RGB);
                }
            }
            // if yuv RGB format config not support, try find a RGBA config
            if (hasSpecifiedEglConfigFormat(EglConfigFormat.RGBA,
                    mSupportedOutputFormats)) {
                if (closestConfig == null) {
                    closestConfig =
                        findClosestEglConfig(display, configs, configNum, EglConfigFormat.RGBA);
                }
            }
            return closestConfig;
        }

        private int findConfigAttribute(
                EGLDisplay display,
                EGLConfig config,
                int attribute,
                int defaultValue) {
            if (EGL14.eglGetConfigAttrib(display, config, attribute, mValue, 0)) {
                return mValue[0];
            }
            return defaultValue;
        }

        private EGLConfig findClosestEglConfig(
                EGLDisplay display,
                EGLConfig[] configs,
                int configNum,
                EglConfigFormat configFormat) {
            EGLConfig closestConfig = null;
            int closestDistance = 1000;
            for (int i = 0; i < configNum; i++) {
                int d = findConfigAttribute(display, configs[i],
                        EGL14.EGL_DEPTH_SIZE, 0);
                int s = findConfigAttribute(display, configs[i],
                        EGL14.EGL_STENCIL_SIZE, 0);
                int visualId = findConfigAttribute(display, configs[i],
                        EGL14.EGL_NATIVE_VISUAL_ID, 0);
                int surfaceType = findConfigAttribute(display, configs[i],
                        EGL14.EGL_SURFACE_TYPE, 0);
                if (d >= mDepthSize && s >= mStencilSize) {
                    int r = findConfigAttribute(display, configs[i],
                            EGL14.EGL_RED_SIZE, 0);
                    int g = findConfigAttribute(display, configs[i],
                            EGL14.EGL_GREEN_SIZE, 0);
                    int b = findConfigAttribute(display, configs[i],
                            EGL14.EGL_BLUE_SIZE, 0);
                    int a = findConfigAttribute(display, configs[i],
                            EGL14.EGL_ALPHA_SIZE, 0);
                    int distance =
                              Math.abs(r - mRedSize)
                            + Math.abs(g - mGreenSize)
                            + Math.abs(b - mBlueSize)
                            + Math.abs(a - mAlphaSize);
                    LogHelper.d(TAG, "Try to find EglConfig, want format:" + configFormat
                            + " r: " + r
                            + " g: " + g
                            + " b: " + b
                            + " a: " + a
                            + " visual id = " + visualId
                            + " surfaceType = " + surfaceType
                            + " depth = " + d
                            + " stencil = " + s
                            + " distance = " + distance);
                    if (isInSupportedFormats(visualId) &&
                            isVisualIdValidate(visualId, configFormat) &&
                            distance < closestDistance) {
                        closestDistance = distance;
                        closestConfig = configs[i];
                        mSelectedPixelFormat = visualId;
                    }
                }
            }
            LogHelper.d(TAG, "Find format: " + mSelectedPixelFormat);
            return closestConfig;
        }

        private boolean isVisualIdValidate(
                int visualId,
                EglConfigFormat configFormat) {
            switch (configFormat) {
            case YUV:
                return isYuvFormat(visualId);
            case RGB:
                return isRGBFormat(visualId);
            case RGBA:
                return visualId == PixelFormat.RGBA_8888;
            default:
                return false;
            }

        }

        private boolean hasSpecifiedEglConfigFormat(
                EglConfigFormat specifiedEglConfigFormat,
                ArrayList<Integer> supportedFormats) {
            for (int format: supportedFormats) {
                switch (specifiedEglConfigFormat) {
                    case YUV:
                        if (isYuvFormat(format)) {
                            return true;
                        }
                        break;
                    case RGB:
                        if (isRGBFormat(format)) {
                            return true;
                        }
                        break;
                    case RGBA:
                        if (format == PixelFormat.RGBA_8888) {
                            return true;
                        }
                        break;
                    default:
                        return false;
                }
            }
            return false;
        }

        private boolean isYuvFormat(int visualId) {
            switch (visualId) {
            case ImageFormat.YV12:
            case ImageFormat.YUV_420_888:
            case ImageFormat.NV21:
                return true;
            default:
                return false;
            }
        }

        private boolean isRGBFormat(int visualId) {
            switch (visualId) {
            case PixelFormat.RGB_565:
            case PixelFormat.RGB_888:
                return true;
            default:
                return false;
            }
        }

        private int[] mValue;
        protected int mRedSize;
        protected int mGreenSize;
        protected int mBlueSize;
        protected int mAlphaSize;
        protected int mDepthSize;
        protected int mStencilSize;
    }

    /**
     * An inner implemented simple config chooser.
     */
    private class SimpleEGLConfigChooser extends ComponentSizeChooser {
        public SimpleEGLConfigChooser() {
            super(8, 8, 8, 0, 0, 0);
        }
    }

    private boolean isInSupportedFormats(int format) {
        return mSupportedOutputFormats.contains(format);
    }
}