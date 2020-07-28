/*
 *   Copyright Statement:
 *
 *     This software/firmware and related documentation ("MediaTek Software") are
 *     protected under relevant copyright laws. The information contained herein is
 *     confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 *     the prior written permission of MediaTek inc. and/or its licensors, any
 *     reproduction, modification, use or disclosure of MediaTek Software, and
 *     information contained herein, in whole or in part, shall be strictly
 *     prohibited.
 *
 *     MediaTek Inc. (C) 2016. All rights reserved.
 *
 *     BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *    THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *     RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 *     ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 *     WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 *     WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 *     NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 *     RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 *     TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 *     RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 *     OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 *     SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 *     RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 *     STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 *     ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 *     RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 *     MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 *     CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     The following software/firmware and/or related documentation ("MediaTek
 *     Software") have been modified by MediaTek Inc. All revisions are subject to
 *     any receiver's applicable license agreements with MediaTek Inc.
 */
package com.mediatek.camera.feature.setting.facedetection;

import android.graphics.RectF;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.view.View;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.widget.FrameLayout;

import com.mediatek.camera.R;
import com.mediatek.camera.common.IAppUi;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.relation.StatusMonitor;
import com.mediatek.camera.common.widget.PreviewFrameLayout;
import com.mediatek.camera.portability.SystemProperties;

/**
 * This is for face view ctrl.
 */
@SuppressWarnings("deprecation")
public class FaceViewCtrl implements StatusMonitor.StatusChangeListener {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(FaceViewCtrl.class.getSimpleName());
    private static final String ROI_DEBUG_PROPERTY = "vendor.mtk.camera.app.3a.debug";

    private static final String FOCUS_STATE_KEY = "key_focus_state";
    private static final String FOCUS_STATE_PASSIVE_SCAN = "PASSIVE_SCAN";
    private static final String FOCUS_STATE_PASSIVE_FOCUSED = "PASSIVE_FOCUSED";
    private static final String FOCUS_STATE_PASSIVE_UNFOCUSED = "PASSIVE_UNFOCUSED";

    private static final int FACE_VIEW_PRIORITY = 10;
    private static final int HIDE_VIEW_DELAY_WHEN_NO_FACE = 1000;
    private static final int HIDE_VIEW_TIMEOUT_WAIT_AF_SCAN = 1500;
    private static final int HIDE_VIEW_TIMEOUT_WAIT_AF_DONE = 3000;

    private static final int MSG_FACE_VIEW_INIT = 0;
    private static final int MSG_FACE_VIEW_UNINIT = 1;
    private static final int MSG_FACE_VIEW_HIDE = 2;
    private static final int MSG_FACE_VIEW_NONE = 3;
    private static final int MSG_FACE_VIEW_AREA_SIZE = 4;
    private static final int MSG_FACE_VIEW_ORIENTATION_UPDATE = 5;
    private static final int MSG_FACE_UPDATE = 6;
    private static final int MSG_AUTO_FOCUS_CHANGED = 7;

    private MainHandler mMainHandler;
    private PreviewFrameLayout mRootViewGroup;
    private FrameLayout mFaceLayout;
    private FaceView mFaceView;
    private IApp mApp;
    private IAppUi mAppUi;
    private int mFaceNum;
    private boolean mIsEnable = true;
    private Animation mFaceExitAnim;
    private boolean mHideViewWhenFaceCountNotChange = true;
    private FaceViewState mFaceViewState = FaceViewState.STATE_UNINIT;
    private WaitFocusState mWaitFocusState = WaitFocusState.WAIT_NOTHING;

    @Override
    public void onStatusChanged(String key, String value) {
        if (!key.equals(FOCUS_STATE_KEY)) {
            return;
        }
        if (value.equals(FOCUS_STATE_PASSIVE_SCAN)
                || value.equals(FOCUS_STATE_PASSIVE_FOCUSED)
                || value.equals(FOCUS_STATE_PASSIVE_UNFOCUSED)) {
            mMainHandler.obtainMessage(MSG_AUTO_FOCUS_CHANGED, value).sendToTarget();
        }
    }

    /**
     * Face View state.
     */
    private enum FaceViewState {
        STATE_INIT,
        STATE_UNINIT
    }

    private enum WaitFocusState {
        WAIT_PASSIVE_SCAN,
        WAIT_PASSIVE_DONE,
        WAIT_NOTHING
    }

    /**
     * Init the view.
     *
     * @param app The camera activity.
     */
    public void init(IApp app) {
        mApp = app;
        mAppUi = app.getAppUi();
        mMainHandler = new MainHandler(app.getActivity().getMainLooper());
        mMainHandler.sendEmptyMessage(MSG_FACE_VIEW_INIT);
        mFaceExitAnim = AnimationUtils.loadAnimation(app.getActivity(), R.anim.face_exit);

        if (SystemProperties.getInt(ROI_DEBUG_PROPERTY, 0) == 1) {
            LogHelper.d(TAG, "[init] roi debug mode, set mHideViewWhenFaceCountNotChange = false");
            mHideViewWhenFaceCountNotChange = false;
        }
    }

    /**
     * Used to destroy the face view.
     */
    public void unInit() {
        mMainHandler.sendEmptyMessage(MSG_FACE_VIEW_UNINIT);
    }

    /**
     * For the preview area changed, face will update the face coordinate.
     *
     * @param newPreviewArea the preview area.
     */
    public void onPreviewAreaChanged(RectF newPreviewArea) {
        mMainHandler.obtainMessage(MSG_FACE_VIEW_AREA_SIZE, newPreviewArea).sendToTarget();
    }

    /**
     * Enable face view to update
     *
     * @param enableView true, if need to enable, or false.
     */
    public void enableFaceView(boolean enableView) {
        mIsEnable = enableView;
        if (!enableView) {
            if (mFaceView != null) {
                mFaceView.resetReallyShown();
            }
            mMainHandler.sendEmptyMessage(MSG_FACE_VIEW_HIDE);
        }
    }

    /**
     * The face callback handle.
     *
     * @param faces the detected faces.
     */
    public void onDetectedFaceUpdate(Face[] faces) {
        if (!mIsEnable) {
            return;
        }
        if (faces != null && faces.length > 0) {
            mMainHandler.obtainMessage(MSG_FACE_UPDATE, faces).sendToTarget();
        } else {
            mMainHandler.obtainMessage(MSG_FACE_VIEW_NONE).sendToTarget();
        }
    }

    /**
     * For the preview state, when stop it should hide face view.
     *
     * @param isPreviewStarted is previewing or not.
     */
    public void onPreviewStatus(boolean isPreviewStarted) {
        if (!isPreviewStarted) {
            mMainHandler.sendEmptyMessage(MSG_FACE_VIEW_HIDE);
        }
    }

    /**
     * when activity orientation changed, the face will be updated.
     *
     * @param orientation the orientation for display.
     * @param cameraId    camera id.
     */
    public void updateFaceDisplayOrientation(int orientation, int cameraId) {
        mMainHandler.obtainMessage(MSG_FACE_VIEW_ORIENTATION_UPDATE,
                orientation, cameraId).sendToTarget();
    }

    /**
     * Handler let some task execute in main thread.
     */
    private class MainHandler extends Handler {
        public MainHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_FACE_VIEW_HIDE:
                    hideView();
                    break;
                case MSG_FACE_VIEW_INIT:
                    initFaceView();
                    break;
                case MSG_FACE_VIEW_UNINIT:
                    unInitFaceView();
                    break;
                case MSG_FACE_UPDATE:
                    updateFacesViewByFace((Face[]) msg.obj);
                    break;
                case MSG_FACE_VIEW_NONE:
                    mFaceNum = 0;
                    hideView();
                    break;
                case MSG_FACE_VIEW_AREA_SIZE:
                    setFaceViewPreviewSize((RectF) msg.obj);
                    break;
                case MSG_FACE_VIEW_ORIENTATION_UPDATE:
                    setFaceViewDisplayOrientation(msg.arg1, msg.arg2);
                    break;
                case MSG_AUTO_FOCUS_CHANGED:
                    updateFacesViewByFocus((String) msg.obj);
                    break;
                default:
                    break;
            }
        }
    }

    private void setFaceViewDisplayOrientation(int orientation, int cameraId) {
        updateViewDisplayOrientation(orientation, cameraId);
    }

    private void setFaceViewPreviewSize(RectF previewArea) {
        int width = Math.abs((int) previewArea.right - (int) previewArea.left);
        int height = Math.abs((int) previewArea.top - (int) previewArea.bottom);
        updateViewPreviewSize(width, height);
    }

    private void updateFacesViewByFace(Face[] faces) {
        if (!mIsEnable) {
            LogHelper.d(TAG, "[updateFacesViewByFace] mIsEnable is false, ignore this time");
            return;
        }
        if (faces != null && faces.length > 0
                && mFaceViewState == FaceViewState.STATE_INIT) {
            // Check if face view has really been shown, if not , not hide view this time.
            // Why to do this check?
            // Maybe higher priority view is shown when face view wants to show, after higher
            // priority view is not shown, maybe face num is not changed too, it's time to hide
            // face view. So face view has no chance to show out.
            if (mHideViewWhenFaceCountNotChange && faces.length == mFaceNum
                    && mFaceView.hasReallyShown()) {
                // if face view is hide now, not send message, only update wait state
                if (mFaceView.getVisibility() != View.VISIBLE) {
                    mMainHandler.removeMessages(MSG_FACE_VIEW_HIDE);
                    mWaitFocusState = WaitFocusState.WAIT_NOTHING;
                } else if (!mMainHandler.hasMessages(MSG_FACE_VIEW_HIDE)) {
                    // if there is not hide msg in queue, send delay message to hide
                    mMainHandler.removeMessages(MSG_FACE_VIEW_HIDE);
                    LogHelper.d(TAG, "[updateFacesViewByFace] new face num = " + faces.length +
                            ", clear hide msg, send hide msg delay "
                            + HIDE_VIEW_TIMEOUT_WAIT_AF_SCAN + " ms");
                    mMainHandler.sendEmptyMessageDelayed(MSG_FACE_VIEW_HIDE,
                            HIDE_VIEW_TIMEOUT_WAIT_AF_SCAN);
                }
            } else {
                LogHelper.d(TAG, "[updateFacesViewByFace] new face num = " + faces.length +
                        ", clear hide msg, show view right now");
                mMainHandler.removeMessages(MSG_FACE_VIEW_HIDE);
                mWaitFocusState = WaitFocusState.WAIT_PASSIVE_SCAN;
                showView();
                mFaceView.resetReallyShown();
            }

            mFaceView.setFaces(faces);
            mFaceNum = faces.length;
        }
    }

    private void updateFacesViewByFocus(String focusState) {
        LogHelper.d(TAG, "[updateFacesViewByFocus] enter, focusState = " + focusState
                + ", mWaitFocusState = " + mWaitFocusState);
        if (!mIsEnable) {
            LogHelper.d(TAG, "[updateFacesViewByFocus] mIsEnable is false, ignore this time");
            return;
        }

        if (mFaceNum <= 0) {
            LogHelper.d(TAG, "[updateFacesViewByFocus] face num <= 0, ignore this time");
            return;
        }

        if (mFaceViewState != FaceViewState.STATE_INIT) {
            LogHelper.d(TAG, "[updateFacesViewByFocus] face view not init, ignore this time");
            return;
        }

        if (focusState.equals(FOCUS_STATE_PASSIVE_SCAN)
                && mWaitFocusState == WaitFocusState.WAIT_PASSIVE_SCAN) {
            mWaitFocusState = WaitFocusState.WAIT_PASSIVE_DONE;
            LogHelper.d(TAG, "[updateFacesViewByFocus] clear hide msg, send hide msg delay " +
                    HIDE_VIEW_TIMEOUT_WAIT_AF_DONE + " ms");
            mMainHandler.removeMessages(MSG_FACE_VIEW_HIDE);
            mMainHandler.sendEmptyMessageDelayed(MSG_FACE_VIEW_HIDE,
                    HIDE_VIEW_TIMEOUT_WAIT_AF_DONE);
        } else if ((focusState.equals(FOCUS_STATE_PASSIVE_FOCUSED)
                || focusState.equals(FOCUS_STATE_PASSIVE_UNFOCUSED))
                && mWaitFocusState == WaitFocusState.WAIT_PASSIVE_DONE) {
            mWaitFocusState = WaitFocusState.WAIT_NOTHING;
            LogHelper.d(TAG, "[updateFacesViewByFocus] clear hide msg, hide view right now");
            mMainHandler.removeMessages(MSG_FACE_VIEW_HIDE);
            hideView();
        }
        LogHelper.d(TAG, "[updateFacesViewByFocus] exit, mWaitFocusState = " + mWaitFocusState);
    }

    private void initFaceView() {
        mRootViewGroup = mAppUi.getPreviewFrameLayout();
        mFaceLayout = (FrameLayout) mApp.getActivity().getLayoutInflater().inflate(
                R.layout.face_view, mRootViewGroup, true);
        mFaceView = (FaceView) mFaceLayout.findViewById(R.id.face_view);
        mFaceViewState = FaceViewState.STATE_INIT;
        mRootViewGroup.registerView(mFaceView, FACE_VIEW_PRIORITY);
    }

    private void showView() {
        if (mFaceView != null && mFaceView.getVisibility() != View.VISIBLE) {
            LogHelper.d(TAG, "[showView]");
            mFaceView.setVisibility(View.VISIBLE);
        }
    }

    private void hideView() {
        if (mFaceView != null && mFaceView.getVisibility() == View.VISIBLE) {
            LogHelper.d(TAG, "[hideView]");
            mFaceExitAnim.reset();
            mFaceView.clearAnimation();
            mFaceView.startAnimation(mFaceExitAnim);
            mFaceView.setVisibility(View.INVISIBLE);
        }
    }

    private void unInitFaceView() {
        mRootViewGroup.unRegisterView(mFaceView);
        mFaceView.setVisibility(View.GONE);
        mRootViewGroup.removeView(mFaceView);
        mFaceViewState = FaceViewState.STATE_UNINIT;
        mFaceView = null;
    }

    private void updateViewDisplayOrientation(int displayOrientation, int cameraId) {
        if (mFaceView != null) {
            mFaceView.setDisplayOrientation(displayOrientation, cameraId);
        }
    }

    private void updateViewPreviewSize(int width, int height) {
        if (mFaceView != null) {
            mFaceView.updatePreviewSize(width, height);
        }
    }
}
