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
package com.mediatek.camera.feature.setting.matrixdisplay;

import android.graphics.ImageFormat;
import android.graphics.RectF;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.view.View;
import android.widget.ImageView;

import com.mediatek.camera.R;
import com.mediatek.camera.common.IAppUiListener;
import com.mediatek.camera.common.ICameraContext;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.mode.ICameraMode;
import com.mediatek.camera.common.relation.Relation;
import com.mediatek.camera.common.relation.StatusMonitor;
import com.mediatek.camera.common.setting.ISettingManager;
import com.mediatek.camera.common.setting.SettingBase;
import com.mediatek.camera.common.utils.CameraUtil;
import com.mediatek.camera.common.utils.Size;

import java.util.ArrayList;
import java.util.List;

import javax.annotation.Nonnull;

/**
 * Matrix display setting item.
 */

public class MatrixDisplay extends SettingBase implements
        MatrixDisplayViewManager.ViewStateCallback,
        MatrixDisplayViewManager.ItemClickListener,
        MatrixDisplayHandler.EffectAvailableListener {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(MatrixDisplay.class.getSimpleName());
    private static final String MATRIX_DISPLAY_SHOW_KEY = "key_matrix_display_show";
    private static final String KEY_COLOR_EFFECT = "key_color_effect";
    private static final String KEY_PIP_PHOTO_MODE
            = "com.mediatek.camera.feature.mode.pip.photo.PipPhotoMode";
    private static final String KEY_PIP_VIDEO_MODE
            = "com.mediatek.camera.feature.mode.pip.video.PipVideoMode";
    private static final String VALUE_NONE = "none";

    // preview size > destination buffer size is OK, no need to set higher resolution
    private static final int MAX_SUPPORTED_PREVIEW_SIZE[] = new int[]{960 * 540, 1280 * 720};
    private static final int DELAY_MSG_REMOVE_GRID_MS = 3000;
    private static final int TIME_DELAY_HIDE_DISPLAY_LAYOUT_MS = 500;

    private StatusMonitor.StatusResponder mMatrixDisplayResponder;
    private MatrixDisplayViewManager mViewManager;
    private MatrixDisplayHandler mMatrixDisplayHandler;
    private IMatrixDisplayConfig mDisplayConfig;

    private MainHandler mMainHandler;
    private List<CharSequence> mEffectEntryValues;
    private List<CharSequence>  mEffectEntries;
    private List<String> mSupportedEffects;
    private List<String> mSupportedPreviewSize;
    private String mSelectedEffect;

    private boolean mViewIsShowing = false;
    private String mModeDeviceState;

    private ImageView mImageView;
    private int mPreviewWidth;
    private int mPreviewHeight;
    private int mLayoutWidth;
    private int mLayoutHeight;
    private int mOrientation;

    private String mModeKey;

    @Override
    public void init(IApp app, ICameraContext cameraContext,
                     ISettingManager.SettingController settingController) {
        super.init(app, cameraContext, settingController);
        LogHelper.d(TAG, "[init]");
        app.registerOnOrientationChangeListener(mOrientationListener);
        app.registerBackPressedListener(mBackPressedListener, IApp.DEFAULT_PRIORITY);
        app.getAppUi().registerOnPreviewAreaChangedListener(mPreviewAreaChangedListener);
        if (mMatrixDisplayHandler == null) {
            mMatrixDisplayHandler = new MatrixDisplayHandler();
            mMatrixDisplayHandler.setEffectAvailableListener(MatrixDisplay.this);
        }
        mMainHandler = new MainHandler(mActivity.getMainLooper());
        mMatrixDisplayResponder = mStatusMonitor.getStatusResponder(MATRIX_DISPLAY_SHOW_KEY);
    }

    @Override
    public void updateModeDeviceState(String newState) {
        mModeDeviceState = newState;
    }

    @Override
    public void unInit() {
        LogHelper.d(TAG, "[unInit]");
        mApp.unregisterOnOrientationChangeListener(mOrientationListener);
        mApp.unRegisterBackPressedListener(mBackPressedListener);
        mAppUi.unregisterOnPreviewAreaChangedListener(mPreviewAreaChangedListener);
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (mViewManager != null) {
                    mViewManager.hideView(false, 0);
                    mViewIsShowing = false;
                    mAppUi.applyAllUIVisibilityImmediately(View.VISIBLE);
                    mAppUi.applyAllUIEnabledImmediately(true);
                    mViewManager = null;
                }
            }

        });

        if (mMatrixDisplayHandler != null) {
            mMatrixDisplayHandler.release();
        }
        mDisplayConfig = null;
        mSupportedEffects = null;
    }

    @Override
    public void addViewEntry() {
        LogHelper.d(TAG, "[addViewEntry], mImageView:" + mImageView);
        if (mImageView == null) {
            ImageView imageView = (ImageView) mActivity.getLayoutInflater()
                    .inflate(R.layout.matrix_display_entry_view, null, false);
            imageView.setImageDrawable(mActivity.getResources()
                    .getDrawable(R.drawable.ic_matrix_display_entry));
            imageView.setOnClickListener(mEntryViewClickListener);
            imageView.setVisibility(View.GONE);
            mAppUi.setEffectViewEntry(imageView);
            mImageView = imageView;
        }
    }

    @Override
    public void removeViewEntry() {
        mImageView = null;
        mAppUi.setEffectViewEntry(null);
    }

    @Override
    public void refreshViewEntry() {
        LogHelper.d(TAG, "[refreshViewEntry], entry values:" + getEntryValues());
        // In pip mode, switch between photo and video, bottom setting manager will
        // call refresh view entry, top setting manager will call init values, they
        // are in different thread, so matrix display entry value size may be > 1
        // when execute refresh, which cause matrix display icon be shown in pip mode.
        if (KEY_PIP_PHOTO_MODE.equals(mModeKey)
                || KEY_PIP_VIDEO_MODE.equals(mModeKey)) {
            LogHelper.d(TAG, "[refreshViewEntry], in pip mode, don't refresh, return");
            return;
        }
        if (mImageView != null) {
            if (getEntryValues().size() <= 1) {
                mImageView.setVisibility(View.GONE);
            } else {
                mImageView.setVisibility(View.VISIBLE);
            }
        }
    }

    @Override
    public void onModeOpened(String modeKey, ICameraMode.ModeType modeType) {
        super.onModeOpened(modeKey, modeType);
        mModeKey = modeKey;
    }

    @Override
    public void postRestrictionAfterInitialized() {

    }

    @Override
    public SettingType getSettingType() {
        return SettingType.PHOTO_AND_VIDEO;
    }

    @Override
    public String getKey() {
        return KEY_COLOR_EFFECT;
    }

    @Override
    public IParametersConfigure getParametersConfigure() {
        if (mDisplayConfig == null) {
            MatrixDisplayParametersConfig parametersConfig =
                    new MatrixDisplayParametersConfig(getKey(), mSettingDeviceRequester,
                            new MatrixDisplayParametersConfig.ValueInitializedListener() {
                                @Override
                                public void onValueInitialized(
                                        List<String> supportedEffects,
                                        String defaultEffect,
                                        List<String> supportedPreviewSizes) {
                                    initializeValue(supportedEffects,
                                            defaultEffect, supportedPreviewSizes);
                                }
                            });
            parametersConfig.setPreviewFrameCallback(mMatrixDisplayHandler);
            mDisplayConfig = parametersConfig;
        }
        return (MatrixDisplayParametersConfig) mDisplayConfig;
    }

    @Override
    public ICaptureRequestConfigure getCaptureRequestConfigure() {
        if (mDisplayConfig == null) {
            MatrixDisplayRequestConfig requestConfig =
                    new MatrixDisplayRequestConfig(getKey(), mSettingDevice2Requester,
                            new MatrixDisplayRequestConfig.ValueInitializedListener() {
                                @Override
                                public void onValueInitialized(
                                        List<String> supportedEffects,
                                        String defaultEffect,
                                        List<String> supportedPreviewSizes) {
                                    initializeValue(supportedEffects,
                                            defaultEffect, supportedPreviewSizes);
                                }
                            });
            requestConfig.setPreviewFrameCallback(mMatrixDisplayHandler);
            mDisplayConfig = requestConfig;
        }
        return (MatrixDisplayRequestConfig) mDisplayConfig;
    }

    @Override
    public boolean onItemClicked(String effect) {
        if (mDisplayConfig == null) {
            return false;
        }
        if (!mViewIsShowing) {
            return false;
        }
        mSelectedEffect = effect;
        setValue(mSelectedEffect);
        mDataStore.setValue(KEY_COLOR_EFFECT, effect, getStoreScope(), true);
        exitMatrixDisplay(true, DELAY_MSG_REMOVE_GRID_MS);
        return true;
    }

    @Override
    public void overrideValues(@Nonnull String headerKey, String currentValue,
                               List<String> supportValues) {
        LogHelper.d(TAG, "[overrideValues], headerKey:" + headerKey + ", " +
                "currentValue:" + currentValue + ", supportValues:" + supportValues);
        super.overrideValues(headerKey, currentValue, supportValues);
        if (mDisplayConfig != null) {
            mDisplayConfig.setSelectedEffect(getValue());
        }
    }

    @Override
    public void onViewCreated() {
        LogHelper.d(TAG, "[onViewCreated]");
        mMatrixDisplayHandler.initialize(mPreviewWidth, mPreviewHeight,
                ImageFormat.YV12, mLayoutWidth, mLayoutHeight);
    }

    @Override
    public void onViewScrollOut() {
        if (mViewIsShowing) {
            exitMatrixDisplay(false, 0);
        }
    }

    @Override
    public void onViewHidden() {
        LogHelper.d(TAG, "[onViewHidden]");
        mMatrixDisplayResponder.statusChanged(MATRIX_DISPLAY_SHOW_KEY, String.valueOf(false));
    }

    @Override
    public void onViewDestroyed() {
        LogHelper.d(TAG, "[onViewDestroyed]");
        mMatrixDisplayHandler.release();
    }

    @Override
    public void onEffectAvailable() {
        LogHelper.d(TAG, "[onEffectAvailable]");
        mViewManager.onEffectAvailable();
    }

    /**
     * Initialize effect values when platform supported effects is ready.
     *
     * @param supportedEffects The platform supported effects.
     * @param defaultEffect The default effect is set by native.
     * @param supportedPreviewSize The platform supported preview size.
     */
    public void initializeValue(List<String> supportedEffects, String defaultEffect,
                                List<String> supportedPreviewSize) {
        LogHelper.d(TAG, "[initializeValue], supportedEffects:" + supportedEffects);
        if (supportedEffects != null && supportedEffects.size() != 0) {
            mSelectedEffect = mDataStore.getValue(KEY_COLOR_EFFECT, defaultEffect, getStoreScope());
            // Pip mode operate two camera at the same time, matrix display keep same instance
            // between back and front camera. In pip mode, color effect is limited as none,
            // pip mode override back camera color effect as none and configure it, however matrix
            // display will be initialized in front camera and set the color effect as no-none
            // effect if color effect has set as no-none before. So the back camera may configure
            // no-none effect into parameters.
            if (KEY_PIP_PHOTO_MODE.equals(mModeKey)
                    || KEY_PIP_VIDEO_MODE.equals(mModeKey)) {
                mSelectedEffect = VALUE_NONE;
                LogHelper.d(TAG, "[initializeValue], in pip mode, set effect as none");
            }
            mDisplayConfig.setSelectedEffect(mSelectedEffect);

            setSupportedPlatformValues(supportedEffects);
            setSupportedEntryValues(supportedEffects);
            setEntryValues(supportedEffects);
            setValue(mSelectedEffect);

            mSupportedPreviewSize = supportedPreviewSize;
            mSupportedEffects = supportedEffects;
        }
    }

    private void initializeViewManager() {
        mViewManager = new MatrixDisplayViewManager(mActivity);
        mViewManager.setViewStateCallback(MatrixDisplay.this);
        mViewManager.setItemClickListener(MatrixDisplay.this);
        mViewManager.setSurfaceAvailableListener(mMatrixDisplayHandler);
        mViewManager.setEffectUpdateListener(mMatrixDisplayHandler);
        initEffectEntriesAndEntryValues(mSupportedEffects);
        mViewManager.setEffectEntriesAndEntryValues(mEffectEntries, mEffectEntryValues);
        mViewManager.setLayoutSize(mLayoutWidth, mLayoutHeight);
        mViewManager.setOrientation(mOrientation);
        mViewManager.setDisplayOrientation(getDisplayOrientation());
    }

    private void enterMatrixDisplay() {
        mMatrixDisplayResponder.statusChanged(MATRIX_DISPLAY_SHOW_KEY, String.valueOf(true));
        LogHelper.d(TAG, "[enterMatrixDisplay]");
        mAppUi.applyAllUIEnabledImmediately(false);
        mAppUi.applyAllUIVisibilityImmediately(View.INVISIBLE);

        // set effect color as none
        mDisplayConfig.setSelectedEffect(mEffectEntryValues.get(0).toString());
        computeSuitablePreviewSize();
        mDisplayConfig.setPreviewSize(mPreviewWidth, mPreviewHeight);
        mDisplayConfig.setDisplayStatus(true);

        Relation relation = MatrixDisplayRestriction.getRestrictionGroup()
                .getRelation("on", true);
        mSettingController.postRestriction(relation);

        mDisplayConfig.sendSettingChangeRequest();

        mViewManager.setSelectedEffect(mSelectedEffect);

        mViewManager.setMirror(CameraUtil.isCameraFacingFront(
                mApp.getActivity(), Integer.parseInt(mSettingController.getCameraId())));

        mViewManager.showView();
        mViewIsShowing = true;
    }

    private void exitMatrixDisplay(boolean animation, int delay) {
        LogHelper.d(TAG, "[exitMatrixDisplay]");
        mMatrixDisplayResponder.statusChanged(MATRIX_DISPLAY_SHOW_KEY, String.valueOf(false));
        mViewIsShowing = false;
        mDisplayConfig.setSelectedEffect(mSelectedEffect);
        mDisplayConfig.setDisplayStatus(false);
        Relation relation = MatrixDisplayRestriction.getRestrictionGroup()
                .getRelation("off", true);
        mSettingController.postRestriction(relation);
        mDisplayConfig.sendSettingChangeRequest();

        // It will restart preview when exit matrix display to back to normal preview.
        // In order to don't let user to see the preview is paused during restarting preview,
        // it delays to hide matrix display layout to cover the process of restart preview.
        Message msg = mMainHandler.obtainMessage(MainHandler.MSG_HIDE_MATRIX_DISPLAY_LAYOUT,
                delay, 0, animation);
        mMainHandler.sendMessageDelayed(msg, TIME_DELAY_HIDE_DISPLAY_LAYOUT_MS);
    }

    private View.OnClickListener mEntryViewClickListener = new View.OnClickListener() {
        @Override
        public void onClick(View view) {
            LogHelper.ui(TAG, "[onClick], mViewIsShowing:" + mViewIsShowing);
            if (mViewIsShowing) {
                return;
            }

            if (!view.isEnabled()) {
                LogHelper.d(TAG, "[onClick], view is disabled, return");
                return;
            }

            if (mSupportedEffects == null) {
                LogHelper.d(TAG, "[onClick], supported effect is null, return");
                return;
            }

            if (!ICameraMode.MODE_DEVICE_STATE_PREVIEWING.equals(mModeDeviceState)) {
                LogHelper.d(TAG, "[onClick] mModeDeviceState = " + mModeDeviceState
                        + ", not in " + ICameraMode.MODE_DEVICE_STATE_PREVIEWING
                        + " state, return");
                return;
            }

            if (mViewManager == null) {
                initializeViewManager();
            }
            enterMatrixDisplay();
        }
    };

    private IApp.OnOrientationChangeListener mOrientationListener =
            new IApp.OnOrientationChangeListener() {
                @Override
                public void onOrientationChanged(int orientation) {
                    mOrientation = orientation;
                    if (mViewManager != null) {
                        mViewManager.setOrientation(mOrientation);
                    }
                }
            };

    private IAppUiListener.OnPreviewAreaChangedListener mPreviewAreaChangedListener
            = new IAppUiListener.OnPreviewAreaChangedListener() {
                 @Override
                 public void onPreviewAreaChanged(RectF newPreviewArea, Size previewSize) {
                     int layoutWidth = (int) (newPreviewArea.right - newPreviewArea.left);
                     int layoutHeight = (int) (newPreviewArea.bottom - newPreviewArea.top);
                     LogHelper.d(TAG, "[onPreviewAreaChanged], layoutWidth = " + layoutWidth
                             + ", layoutHeight = " + layoutHeight);
                     if (layoutWidth != mLayoutWidth || layoutHeight != mLayoutHeight) {
                         mLayoutWidth = layoutWidth;
                         mLayoutHeight = layoutHeight;
                         if (mViewManager != null) {
                             mViewManager.setLayoutSize(mLayoutWidth, mLayoutHeight);
                             mViewManager.hideView(false, 0);
                         }
                     }
                 }
            };

    private IApp.BackPressedListener mBackPressedListener
            = new IApp.BackPressedListener() {
                @Override
                public boolean onBackPressed() {
                    LogHelper.d(TAG, "[onBackPressed] mViewIsShowing:" + mViewIsShowing);
                    if (mViewIsShowing) {
                        exitMatrixDisplay(true, DELAY_MSG_REMOVE_GRID_MS);
                        return true;
                    } else {
                        return false;
                    }
                }
            };

    /**
     * Handler in the main thread to update UI.
     */
    private class MainHandler extends Handler {
        private static final int MSG_HIDE_MATRIX_DISPLAY_LAYOUT = 0;

        public MainHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_HIDE_MATRIX_DISPLAY_LAYOUT:
                    if (mViewManager != null) {
                        mViewManager.hideView((Boolean) msg.obj, msg.arg1);
                        mViewIsShowing = false;

                        mAppUi.applyAllUIVisibilityImmediately(View.VISIBLE);
                        mAppUi.applyAllUIEnabledImmediately(true);
                    }
                    break;

                default:
                    break;
            }
        }
    }
    private int getDisplayOrientation() {
        int cameraId = Integer.valueOf(mSettingController.getCameraId());
        int displayRotation = CameraUtil.getDisplayRotation(mActivity);
        int displayOrientation = CameraUtil.getDisplayOrientation(displayRotation, cameraId);
        return displayOrientation;
    }

    private void initEffectEntriesAndEntryValues(List<String> supportedEffects) {
        mEffectEntryValues = new ArrayList<>();
        mEffectEntries = new ArrayList<>();

        String[] originalEffectEntries = mActivity.getResources()
                .getStringArray(R.array.pref_camera_coloreffect_entries);
        String[] originalEffectEntryValues = mActivity.getResources()
                .getStringArray(R.array.pref_camera_coloreffect_entryvalues);

        for (int i = 0; i < originalEffectEntryValues.length; i++) {
            String effect = originalEffectEntryValues[i];
            for (int j = 0; j < supportedEffects.size(); j++) {
                if (effect.equals(supportedEffects.get(j))) {
                    mEffectEntryValues.add(effect);
                    mEffectEntries.add(originalEffectEntries[i]);
                    break;
                }
            }
        }
    }

    private void computeSuitablePreviewSize() {
        double refAspectRatio = (double) Math.max(mLayoutWidth, mLayoutHeight)
                / (double) Math.min(mLayoutWidth, mLayoutHeight);

        int selectedWidth = 0;
        int selectedHeight = 0;
        for (int j = 0; j < MAX_SUPPORTED_PREVIEW_SIZE.length; j++) {
            for (int i = 0; i < mSupportedPreviewSize.size(); i++) {
                String size = mSupportedPreviewSize.get(i);
                int index = size.indexOf('x');
                int width = Integer.parseInt(size.substring(0, index));
                int height = Integer.parseInt(size.substring(index + 1));
                double ratio = (double) width / (double) height;

                if (width % 32 != 0) {
                    continue;
                }
                if (width * height > MAX_SUPPORTED_PREVIEW_SIZE[j]) {
                    continue;
                }
                if (Math.abs(refAspectRatio - ratio) > 0.02) {
                    continue;
                }
                if (width * height > selectedWidth * selectedHeight) {
                    selectedWidth = width;
                    selectedHeight = height;
                }
            }
            if (selectedWidth != 0 && selectedHeight != 0) {
                break;
            }
        }
        mPreviewWidth = selectedWidth;
        mPreviewHeight = selectedHeight;
        LogHelper.d(TAG, "[computeSuitablePreviewSize], preview size for matrix display, Width:"
                + mPreviewWidth + ", mPreviewHeight:" + mPreviewHeight);
    }
}
