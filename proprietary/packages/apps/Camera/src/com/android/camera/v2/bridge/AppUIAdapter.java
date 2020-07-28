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

package com.android.camera.v2.bridge;

import com.android.camera.v2.app.AppController;
import com.android.camera.v2.app.CameraAppUI;
import com.mediatek.camera.v2.platform.app.AppUi;
import com.mediatek.camera.v2.platform.app.AppUi.OkCancelClickListener;
import com.mediatek.camera.v2.platform.app.AppUi.PlayButtonClickListener;
import com.mediatek.camera.v2.platform.app.AppUi.RetakeButtonClickListener;

import android.graphics.Bitmap;
import android.widget.FrameLayout;

/**
 *  This adapter is used to adapt AppUi calls from module level to app level.
 */
public class AppUIAdapter implements AppUi {
    private final AppController             mAppController;
    private final CameraAppUI               mCameraAppUi;
    private ShutterEventAdapter             mPhotoShutterEventAdapter;
    private ShutterEventAdapter             mVideoShutterEventAdapter;
    private OkCancelClickAdapter            mOkCancelClickAdapter;
    private PlayButtonClickAdapter          mPlayButtonClickAdapter;
    private RetakeButtonClickAdapter        mRetakeButtonClickAdapter;

    public AppUIAdapter(AppController app) {
        mAppController = app;
        mCameraAppUi = app.getCameraAppUI();
    }

    @Override
    public FrameLayout getModuleLayoutRoot() {
        return mAppController.getModuleLayoutRoot();
    }

    @Override
    public void setSwipeEnabled(boolean enabled) {
        mCameraAppUi.setSwipeEnabled(enabled);
    }

    @Override
    public void setAllCommonViewEnable(boolean enable) {
        mCameraAppUi.setAllCommonViewEnable(enable);
    }

    @Override
    public void setAllCommonViewButShutterVisible(boolean visible) {
        mCameraAppUi.setAllCommonViewButShutterVisible(visible);
    }

    @Override
    public int getPreviewVisibility() {
        int remotePreviewVisibility = mCameraAppUi.getPreviewVisibility();
        int localPreviewVisibility;
        switch (remotePreviewVisibility) {
        case CameraAppUI.VISIBILITY_UNCOVERED:
            localPreviewVisibility = AppUi.PREVIEW_VISIBILITY_UNCOVERED;
            break;
        case CameraAppUI.VISIBILITY_COVERED:
            localPreviewVisibility = AppUi.PREVIEW_VISIBILITY_COVERED;
            break;
        default:
            localPreviewVisibility = AppUi.PREVIEW_VISIBILITY_UNCOVERED;
            break;
        }
        return localPreviewVisibility;
    }

    @Override
    public void setShutterEventListener(ShutterEventsListener eventListener,
            boolean videoShutter) {
        if (videoShutter) {
            mVideoShutterEventAdapter = new ShutterEventAdapter(eventListener);
            mAppController.setShutterEventListener(mVideoShutterEventAdapter, true);
        } else {
            mPhotoShutterEventAdapter = new ShutterEventAdapter(eventListener);
            mAppController.setShutterEventListener(mPhotoShutterEventAdapter, false);
        }
    }

    @Override
    public void setOkCancelClickListener(OkCancelClickListener listener) {
        if (mOkCancelClickAdapter == null) {
            mOkCancelClickAdapter = new OkCancelClickAdapter(listener);
        }
        mAppController.setOkCancelClickListener(mOkCancelClickAdapter);
    }

    @Override
    public boolean isShutterButtonEnabled(boolean videoShutter) {
        return mAppController.isShutterButtonEnabled(videoShutter);
    }

    @Override
    public void setShutterButtonEnabled(boolean enabled, boolean videoShutter) {
        mAppController.setShutterButtonEnabled(enabled, videoShutter);
    }

    @Override
    public void performShutterButtonClick(boolean videoShutter) {
        mAppController.performShutterButtonClick(videoShutter);
    }

    @Override
    public void switchShutterButtonImageResource(int imageResourceId, boolean isVideoButton) {
        mCameraAppUi.switchShutterButtonImageResource(imageResourceId, isVideoButton);
    }

    @Override
    public void switchShutterButtonLayout(int layoutId) {
        mCameraAppUi.switchShutterButtonLayout(layoutId);
    }

    @Override
    public void setPlayButtonClickListener(PlayButtonClickListener listener) {
        if (mPlayButtonClickAdapter == null) {
            mPlayButtonClickAdapter = new PlayButtonClickAdapter(listener);
        }
        mAppController.setPlayButtonClickListener(mPlayButtonClickAdapter);
    }


    @Override
    public void setRetakeButtonClickListener(RetakeButtonClickListener listener) {
        if (mRetakeButtonClickAdapter == null) {
            mRetakeButtonClickAdapter = new RetakeButtonClickAdapter(listener);
        }
        mAppController.setRetakeButtonClickListener(mRetakeButtonClickAdapter);
    }

    @Override
    public void stopShowCommonUI(boolean stop) {
        mCameraAppUi.stopShowCommonUI(stop);
    }

    @Override
    public boolean isSettingViewShowing() {
        return mAppController.getCameraAppUI().isSettingViewShowing();
    }

    @Override
    public void showSettingUi() {
        mAppController.getCameraAppUI().showSettingUi();
    }

    @Override
    public void hideSettingUi() {
        mCameraAppUi.hideSettingUi();
    }

    @Override
    public void showModeOptionsUi() {
        mCameraAppUi.showModeOptionsUi();
    }

    @Override
    public void hideModeOptionsUi() {
        mCameraAppUi.hideModeOptionsUi();
    }

    @Override
    public void showPickerManagerUi() {
        mCameraAppUi.showPickerManagerUi();
    }

    @Override
    public void hidePickerManagerUi() {
        mCameraAppUi.hidePickerManagerUi();
    }

    @Override
    public void performCameraPickerBtnClick(boolean force) {
        mCameraAppUi.performCameraPickerBtnClick(force);
    }

    @Override
    public void showThumbnailManagerUi() {
        mCameraAppUi.showThumbnailManagerUi();
    }

    @Override
    public void hideThumbnailManagerUi() {
        mCameraAppUi.hideThumbnailManagerUi();
    }

    @Override
    public void forceUpdateThumbnail() {
        mCameraAppUi.forceUpdateThumbnail();
    }
    @Override
    public void setThumbnailManagerEnable(boolean enable) {
        mCameraAppUi.setThumbnailManagerEnable(enable);
    }

    @Override
    public void showIndicatorManagerUi() {
        mCameraAppUi.showIndicatorManagerUi();
    }

    @Override
    public void hideIndicatorManagerUi() {
        mCameraAppUi.hideIndicatorManagerUi();
    }

    @Override
    public void showInfo(String text) {
        mCameraAppUi.showInfo(text);
    }

    @Override
    public void showInfo(CharSequence text, int showMs) {
        mCameraAppUi.showInfo(text, showMs);
    }

    @Override
    public void dismissInfo(boolean onlyDismissInfo) {
        mCameraAppUi.dismissInfo(onlyDismissInfo);
    }

    @Override
    public void showHint(String message) {
        mCameraAppUi.showHint(message);
    }
    @Override
    public void hideHint() {
        mCameraAppUi.hideHint();
    }

    @Override
    public void showSavingProgress(String text) {
        mCameraAppUi.showSavingProgress(text);
    }

    @Override
    public void dismissSavingProgress() {
        mCameraAppUi.dismissSavingProgress();
    }

    @Override
    public void showReviewView(Bitmap bitmap) {
        mCameraAppUi.showReviewManager(bitmap);
    }

    @Override
    public void showReviewView(byte[] jpegData, int height) {
        mCameraAppUi.showReviewManager(jpegData, height);
    }

    @Override
    public void hideReviewView() {
        mCameraAppUi.hideReviewManager();
    }

    @Override
    public void showLeftCounts(int bytePerCount, boolean showAlways) {
        mCameraAppUi.showLeftCounts(bytePerCount, showAlways);
    }

    @Override
    public void showLeftTime(long bytePerMs) {
        mCameraAppUi.showLeftTime(bytePerMs);
    }

    @Override
    public void updateAsdDetectedScene(String scene) {
        mCameraAppUi.updateAsdDetectedScene(scene);
    }

    @Override
    public void showShutterButton() {
        mCameraAppUi.showShutterButtonView();
    }

    @Override
    public void hideShutterButton() {
        mCameraAppUi.hideShutterButtonView();
    }

    /**
     *  This adapter is used to adapt shutter events from App level to Module level.
     */
    private class ShutterEventAdapter implements
                    com.android.camera.v2.app.AppController.ShutterEventsListener {
        private final ShutterEventsListener mShutterEventListener;
        public ShutterEventAdapter(ShutterEventsListener shutterListener) {
            mShutterEventListener = shutterListener;
        }

        @Override
        public void onShutterPressed() {
            mShutterEventListener.onShutterPressed();
        }

        @Override
        public void onShutterReleased() {
            mShutterEventListener.onShutterReleased();
        }

        @Override
        public void onShutterClicked() {
            mShutterEventListener.onShutterClicked();
        }

        @Override
        public void onShutterLongPressed() {
            mShutterEventListener.onShutterLongPressed();
        }
    }

    private class OkCancelClickAdapter implements
                    com.android.camera.v2.app.AppController.OkCancelClickListener {
        private OkCancelClickListener mOkCancelClickListener;
        public OkCancelClickAdapter(OkCancelClickListener listener) {
            mOkCancelClickListener = listener;
        }
        @Override
        public void onOkClick() {
            mOkCancelClickListener.onOkClick();
        }

        @Override
        public void onCancelClick() {
            mOkCancelClickListener.onCancelClick();
        }

    }

    private class PlayButtonClickAdapter implements
                    com.android.camera.v2.app.AppController.PlayButtonClickListener {
        private PlayButtonClickListener mPlayButtonClickListener;

        public PlayButtonClickAdapter(PlayButtonClickListener listener) {
            mPlayButtonClickListener = listener;
        }

        @Override
        public void onPlay() {
            mPlayButtonClickListener.onPlay();
        }

    }

    private class RetakeButtonClickAdapter implements
                    com.android.camera.v2.app.AppController.RetakeButtonClickListener {
        private RetakeButtonClickListener mRetakeButtonClickListener;

        public RetakeButtonClickAdapter(RetakeButtonClickListener listener) {
            mRetakeButtonClickListener = listener;
        }

        @Override
        public void onRetake() {
            mRetakeButtonClickListener.onRetake();
        }
    }
}
