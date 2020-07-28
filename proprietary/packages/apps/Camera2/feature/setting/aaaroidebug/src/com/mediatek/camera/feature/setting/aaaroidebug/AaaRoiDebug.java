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
 *   MediaTek Inc. (C) 2017. All rights reserved.
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

package com.mediatek.camera.feature.setting.aaaroidebug;

import android.graphics.Rect;
import android.graphics.RectF;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.widget.FrameLayout;

import com.mediatek.camera.R;
import com.mediatek.camera.common.IAppUiListener;
import com.mediatek.camera.common.ICameraContext;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.setting.ISettingManager;
import com.mediatek.camera.common.setting.SettingBase;
import com.mediatek.camera.common.utils.CameraUtil;
import com.mediatek.camera.common.utils.CoordinatesTransform;
import com.mediatek.camera.common.utils.Size;

/**
 * Receive and show the 3A range, and make debug easily.
 */
public class AaaRoiDebug extends SettingBase implements
        AaaRoiDebugCaptureRequestConfig.DebugInfoListener,
        IApp.OnOrientationChangeListener,
        IAppUiListener.OnPreviewAreaChangedListener {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(AaaRoiDebug.class.getSimpleName());
    private static final String KEY = "key_3a_roi_debug";
    private static final int MSG_INIT_VIEW = 1;
    private static final int MSG_UN_INIT_VIEW = 2;
    private static final int MSG_UPDATE_VIEW_RECT = 3;

    public static final int COUNT = 3;
    public static final int INDEX_AE = 0;
    public static final int INDEX_AF = 1;
    public static final int INDEX_AWB = 2;

    private Handler mMainHandler;
    private Handler mModeHandler;
    private AaaRoiDebugCaptureRequestConfig mCaptureRequestConfig;

    private int mDisplayOrientation;
    private int mPreviewWidth;
    private int mPreviewHeight;
    private boolean mMirror;

    private FrameLayout mDebugLayout;
    private ColorRectView[] mRectViews;
    private Rect[][] mAaaRois;
    private Object mAaaRoisLock = new Object();

    @Override
    public void init(IApp app, ICameraContext cameraContext,
                     ISettingManager.SettingController settingController) {
        super.init(app, cameraContext, settingController);
        synchronized (mAaaRoisLock) {
            mAaaRois = new Rect[COUNT][];
        }
        mModeHandler = new Handler(Looper.myLooper());
        mMainHandler = new MainHandler(app.getActivity().getMainLooper());
        mMainHandler.sendMessage(mMainHandler.obtainMessage(MSG_INIT_VIEW));
        mApp.registerOnOrientationChangeListener(this);
        mApp.getAppUi().registerOnPreviewAreaChangedListener(this);
        updateDisplayOrientation();
    }

    @Override
    public void unInit() {
        mApp.unregisterOnOrientationChangeListener(this);
        mApp.getAppUi().unregisterOnPreviewAreaChangedListener(this);
        mMainHandler.sendMessage(mMainHandler.obtainMessage(MSG_UN_INIT_VIEW));
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
        return KEY;
    }

    @Override
    public IParametersConfigure getParametersConfigure() {
        return null;
    }

    @Override
    public ICaptureRequestConfigure getCaptureRequestConfigure() {
        if (mCaptureRequestConfig == null) {
            mCaptureRequestConfig = new AaaRoiDebugCaptureRequestConfig();
            mCaptureRequestConfig.setDebugInfoListener(this);
        }
        return mCaptureRequestConfig;
    }

    @Override
    public void onRangeUpdate(Rect[] aeRois, Rect[] afRois, Rect[] awbRois, Rect cropRegion) {
        calculateViewRect(new Rect[][]{aeRois, afRois, awbRois}, cropRegion);
        if (!mMainHandler.hasMessages(MSG_UPDATE_VIEW_RECT)) {
            mMainHandler.sendMessage(mMainHandler.obtainMessage(MSG_UPDATE_VIEW_RECT));
        }
    }

    @Override
    public void onOrientationChanged(int orientation) {
        mModeHandler.post(new Runnable() {
            @Override
            public void run() {
                updateDisplayOrientation();
            }
        });
    }

    @Override
    public void onPreviewAreaChanged(RectF newPreviewArea, Size previewSize) {
        mModeHandler.post(new Runnable() {
            @Override
            public void run() {
                mPreviewWidth = Math.abs((int) newPreviewArea.right - (int) newPreviewArea.left);
                mPreviewHeight = Math.abs((int) newPreviewArea.top - (int) newPreviewArea.bottom);
            }
        });
    }

    /**
     * Handler to handle rect view messge.
     */
    class MainHandler extends Handler {

        public MainHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_INIT_VIEW:
                    initView();
                    break;
                case MSG_UN_INIT_VIEW:
                    unInitView();
                    break;
                case MSG_UPDATE_VIEW_RECT:
                    updateViewRect();
                    break;
                default:
                    break;
            }
        }
    }

    private void initView() {
        mDebugLayout = (FrameLayout) mApp.getActivity().getLayoutInflater().inflate(
                R.layout.aaaroidebug_view, mAppUi.getPreviewFrameLayout(), true);
        mRectViews = new ColorRectView[COUNT];
        mRectViews[INDEX_AE] = (ColorRectView) mDebugLayout.findViewById(R.id.aeRect);
        mRectViews[INDEX_AF] = (ColorRectView) mDebugLayout.findViewById(R.id.afRect);
        mRectViews[INDEX_AWB] = (ColorRectView) mDebugLayout.findViewById(R.id.awbRect);
    }

    private void unInitView() {
        mAppUi.getPreviewFrameLayout().removeViewInLayout(mDebugLayout);
    }

    private void calculateViewRect(Rect[][] aaaRois, Rect cropRegion) {
        for (int i = 0; i < aaaRois.length; i++) {
            if (aaaRois[i] == null || aaaRois[i].length == 0) {
                synchronized (mAaaRoisLock) {
                    mAaaRois[i] = null;
                }
            } else {
                Rect[] aRois = new Rect[aaaRois[i].length];
                Rect temp;
                for (int j = 0; j < aaaRois[i].length; j++) {
                    temp = CoordinatesTransform.sensorToNormalizedPreview(aaaRois[i][j],
                            mPreviewWidth, mPreviewHeight, cropRegion);
                    aRois[j] = CoordinatesTransform.normalizedPreviewToUi(temp,
                            mPreviewWidth, mPreviewHeight,
                            mDisplayOrientation, mMirror);
                }
                synchronized (mAaaRoisLock) {
                    mAaaRois[i] = aRois;
                }
            }
        }
    }

    private void updateViewRect() {
        synchronized (mAaaRoisLock) {
            for (int i = 0; i < mAaaRois.length; i++) {
                mRectViews[i].setRects(mAaaRois[i]);
            }
        }
    }

    private void updateDisplayOrientation() {
        mDisplayOrientation = CameraUtil.getDisplayOrientationFromDeviceSpec(
                CameraUtil.getDisplayRotation(mApp.getActivity()),
                Integer.valueOf(mSettingController.getCameraId()),
                mApp.getActivity());
        mMirror = Integer.valueOf(mSettingController.getCameraId()) == 1;
    }
}