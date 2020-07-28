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
 * MediaTek Inc. (C) 2014. All rights reserved.
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

package com.mediatek.camera.v2.stream.pip.pipwrapping;

import android.graphics.Bitmap;
import android.graphics.PixelFormat;
import android.opengl.EGL14;

import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLDisplay;

/**
 * This class is used to create a specific EGLConfig.
*/
public class PipEGLConfigWrapper {
    private static final Tag TAG = new Tag(PipEGLConfigWrapper.class.getSimpleName());
    private static final int              EGL_RECORDABLE_ANDROID = 0x3142;
    private static final int              EGL_OPENGL_ES2_BIT = 4;
    private EGLConfigChooser              mEGLConfigChooser;
    private int                           mPixelFormat;
    private Bitmap.Config                 mBitmapConfig;
    private static PipEGLConfigWrapper    sPipEGLConfigWrapper;

    /**
     * An interface for choosing an EGLConfig configuration from a list of
     * potential configurations.
     * <p>
     * This interface must be implemented by clients wishing to call
     * {@link PipEGLConfigWrapper#setEGLConfigChooser(EGLConfigChooser)}
     */
    public interface EGLConfigChooser {
        /**
         * Choose a configuration from the list. Implementors typically
         * implement this method by calling
         * {@link EGL10#eglChooseConfig} and iterating through the results. Please consult the
         * EGL specification available from The Khronos Group to learn how to call eglChooseConfig.
         * @param egl the EGL10 for the current display.
         * @param display the current display.
         * @return the chosen configuration.
         */
        EGLConfig chooseConfig(EGL10 egl, EGLDisplay display);
        android.opengl.EGLConfig chooseConfigEGL14(android.opengl.EGLDisplay display,
                boolean recording);
    }

    public static PipEGLConfigWrapper getInstance() {
        if (sPipEGLConfigWrapper == null) {
            sPipEGLConfigWrapper = new PipEGLConfigWrapper();
        }
        return sPipEGLConfigWrapper;
    }

    private PipEGLConfigWrapper() {
        mEGLConfigChooser = new SimpleEGLConfigChooser(false);
    }

    /**
      * Install a custom EGLConfigChooser.
      * <p>If this method is
      * called, it must be called before GL thread initialize.
      * <p>
      * If no setEGLConfigChooser method is called, then by default the
      * view will choose a config as close to 16-bit RGB as possible, with
      * a depth buffer as close to 16 bits as possible.
      * @param configChooser
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
     */
    public void setEGLConfigChooser(int redSize, int greenSize, int blueSize,
            int alphaSize, int depthSize, int stencilSize) {
        setEGLConfigChooser(new ComponentSizeChooser(redSize, greenSize,
                blueSize, alphaSize, depthSize, stencilSize));
    }

    /**
     * Get current EGLConfigChooser to choose a closest EGLConfig for GL Thread initialization.
     * @return EGLConfigChooser A closest EGLConfig
     */
    public EGLConfigChooser getEGLConfigChooser() {
        return mEGLConfigChooser;
    }

    public int getPixelFormat() {
        return mPixelFormat;
    }

    public Bitmap.Config getBitmapConfig() {
        return mBitmapConfig;
    }

    private abstract class BaseConfigChooser
        implements EGLConfigChooser {
        public BaseConfigChooser(int[] configSpec) {
            mConfigSpec = configSpec;
        }

        public EGLConfig chooseConfig(EGL10 egl, EGLDisplay display) {
            int[] num_config = new int[1];
            if (!egl.eglChooseConfig(display, mConfigSpec, null, 0,
                    num_config)) {
                throw new IllegalArgumentException("eglChooseConfig failed");
            }

            int numConfigs = num_config[0];

            if (numConfigs <= 0) {
                throw new IllegalArgumentException(
                        "No configs match configSpec");
            }

            EGLConfig[] configs = new EGLConfig[numConfigs];
            if (!egl.eglChooseConfig(display, mConfigSpec, configs, numConfigs,
                    num_config)) {
                throw new IllegalArgumentException("eglChooseConfig#2 failed");
            }
            EGLConfig config = chooseConfig(egl, display, configs);
            if (config == null) {
                throw new IllegalArgumentException("No config chosen");
            }
            return config;
        }

        @Override
        public android.opengl.EGLConfig chooseConfigEGL14(
                android.opengl.EGLDisplay display, boolean recording) {
            android.opengl.EGLConfig[] configs = new android.opengl.EGLConfig[100];
            int[] numConfigs = new int[1];
            // The actual surface is generally RGBA or RGBX, so situationally
            // omitting alpha
            // doesn't really help. It can also lead to a huge performance hit on
            // glReadPixels()
            // when reading into a GL_RGBA buffer.
            if (recording) {
                mConfigSpec[mConfigSpec.length - 3] = EGL_RECORDABLE_ANDROID;
                mConfigSpec[mConfigSpec.length - 2] = 1;
            }
            if (!EGL14.eglChooseConfig(display, mConfigSpec, 0, configs, 0, configs.length,
                    numConfigs, 0)) {
                throw new RuntimeException("unable to find ES2 EGL config in EGL14");
            }
            android.opengl.EGLConfig config = chooseConfigEGL14(display, configs, numConfigs[0]);
            if (config == null) {
                throw new IllegalArgumentException("No config chosen");
            }
            return config;
        }

        abstract EGLConfig chooseConfig(EGL10 egl, EGLDisplay display,
                EGLConfig[] configs);
        abstract android.opengl.EGLConfig chooseConfigEGL14(android.opengl.EGLDisplay display,
                android.opengl.EGLConfig[] configs, int config_num);

        protected int[] mConfigSpec;
    }

    private class ComponentSizeChooser extends BaseConfigChooser {
        public ComponentSizeChooser(int redSize, int greenSize, int blueSize,
                int alphaSize, int depthSize, int stencilSize) {
            super(new int[] {
                    EGL10.EGL_RED_SIZE, redSize,
                    EGL10.EGL_GREEN_SIZE, greenSize,
                    EGL10.EGL_BLUE_SIZE, blueSize,
                    EGL10.EGL_ALPHA_SIZE, alphaSize,
                    EGL10.EGL_DEPTH_SIZE, depthSize,
                    EGL10.EGL_STENCIL_SIZE, stencilSize,
                    EGL10.EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                    EGL14.EGL_NONE, 0, // placeholder for recordable [@-3]
                    EGL10.EGL_NONE});
            mValue = new int[1];
            mRedSize = redSize;
            mGreenSize = greenSize;
            mBlueSize = blueSize;
            mAlphaSize = alphaSize;
            mDepthSize = depthSize;
            mStencilSize = stencilSize;
        }

        @Override
        public EGLConfig chooseConfig(EGL10 egl, EGLDisplay display,
                EGLConfig[] configs) {
            EGLConfig closestConfig = null;
            int closestDistance = 1000;
            for (EGLConfig config : configs) {
                int d = findConfigAttrib(egl, display, config,
                        EGL10.EGL_DEPTH_SIZE, 0);
                int s = findConfigAttrib(egl, display, config,
                        EGL10.EGL_STENCIL_SIZE, 0);
                if (d >= mDepthSize && s >= mStencilSize) {
                    int r = findConfigAttrib(egl, display, config,
                            EGL10.EGL_RED_SIZE, 0);
                    int g = findConfigAttrib(egl, display, config,
                            EGL10.EGL_GREEN_SIZE, 0);
                    int b = findConfigAttrib(egl, display, config,
                            EGL10.EGL_BLUE_SIZE, 0);
                    int a = findConfigAttrib(egl, display, config,
                            EGL10.EGL_ALPHA_SIZE, 0);
                    int distance =
                              Math.abs(r - mRedSize)
                            + Math.abs(g - mGreenSize)
                            + Math.abs(b - mBlueSize)
                            + Math.abs(a - mAlphaSize);
                    LogHelper.d(TAG, "Try choose EGL10: depth = " + d
                            + " stencil = " + s
                            + " red = " + r
                            + " green = " + g
                            + " blue = " + b
                            + " alpha = " + a
                            + " distance = " + distance);
                    if (distance < closestDistance) {
                        closestDistance = distance;
                        closestConfig = config;
                        LogHelper.d(TAG, "find closer!");
                    }
                }
            }
            return closestConfig;
        }

        @Override
        android.opengl.EGLConfig chooseConfigEGL14(
                android.opengl.EGLDisplay display,
                android.opengl.EGLConfig[] configs, int config_num) {
            android.opengl.EGLConfig closestConfig = null;
            LogHelper.d(TAG, "chooseConfigEGL14 config number = " + config_num);
            int closestDistance = 1000;
            for (int i = 0; i < config_num; i++) {
                int d = findConfigAttrib(display, configs[i],
                        EGL10.EGL_DEPTH_SIZE, 0);
                int s = findConfigAttrib(display, configs[i],
                        EGL10.EGL_STENCIL_SIZE, 0);
                if (d >= mDepthSize && s >= mStencilSize) {
                    int r = findConfigAttrib(display, configs[i],
                            EGL10.EGL_RED_SIZE, 0);
                    int g = findConfigAttrib(display, configs[i],
                            EGL10.EGL_GREEN_SIZE, 0);
                    int b = findConfigAttrib(display, configs[i],
                            EGL10.EGL_BLUE_SIZE, 0);
                    int a = findConfigAttrib(display, configs[i],
                            EGL10.EGL_ALPHA_SIZE, 0);
                    int distance =
                              Math.abs(r - mRedSize)
                            + Math.abs(g - mGreenSize)
                            + Math.abs(b - mBlueSize)
                            + Math.abs(a - mAlphaSize);
                    LogHelper.i(TAG, "Try EGL14 choose: depth = " + d
                            + " stencil = " + s
                            + " red = " + r
                            + " green = " + g
                            + " blue = " + b
                            + " alpha = " + a
                            + " distance = " + distance);
                    if (distance < closestDistance) {
                        closestDistance = distance;
                        closestConfig = configs[i];
                        LogHelper.i(TAG, "find closer!");
                    }
                }
            }
            /**
             * current only support RGBX8888 or RGB565,
             * because image reader only supported the following formats:
             * PixelFormat.RGB_565, PixelFormat.RGBA_8888
             * PixelFormat.RGBX_8888, PixelFormat.RGB_888
             *
             * here is a dirty code, must refine in the future.
             */
            int cloestR = findConfigAttrib(display, closestConfig,
                    EGL10.EGL_RED_SIZE, 0);
            int cloestG = findConfigAttrib(display, closestConfig,
                    EGL10.EGL_GREEN_SIZE, 0);
            int cloestB = findConfigAttrib(display, closestConfig,
                    EGL10.EGL_BLUE_SIZE, 0);
            int cloestA = findConfigAttrib(display, closestConfig,
                    EGL10.EGL_ALPHA_SIZE, 0);
            if (cloestR == 8 && cloestG == 8 && cloestB == 8 && cloestA == 8) {
                mPixelFormat = PixelFormat.RGBA_8888;
                mBitmapConfig = Bitmap.Config.ARGB_8888;
            } else if (cloestR == 5 && cloestG == 6 && cloestB == 5) {
                mPixelFormat = PixelFormat.RGB_565;
                mBitmapConfig = Bitmap.Config.RGB_565;
            }
            return closestConfig;
        }

        private int findConfigAttrib(EGL10 egl, EGLDisplay display,
                EGLConfig config, int attribute, int defaultValue) {

            if (egl.eglGetConfigAttrib(display, config, attribute, mValue)) {
                return mValue[0];
            }
            return defaultValue;
        }

        private int findConfigAttrib(android.opengl.EGLDisplay display,
                android.opengl.EGLConfig config, int attribute, int defaultValue) {
            if (EGL14.eglGetConfigAttrib(display, config, attribute, mValue, 0)) {
                return mValue[0];
            }
            return defaultValue;
        }

        private int[] mValue;
        // Subclasses can adjust these values:
        protected int mRedSize;
        protected int mGreenSize;
        protected int mBlueSize;
        protected int mAlphaSize;
        protected int mDepthSize;
        protected int mStencilSize;
    }

    /**
     * This class will choose a supported surface as close to
     * RGBA8888 as possible, with or without a depth buffer.
     */
    private class SimpleEGLConfigChooser extends ComponentSizeChooser {
        public SimpleEGLConfigChooser(boolean withDepthBuffer) {
            super(8, 8, 8, 8, withDepthBuffer ? 16 : 0, 0);
        }
    }
}