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
package com.android.camera.bridge;

import java.io.FileDescriptor;
import java.util.HashMap;
import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;

import com.android.camera.CameraActivity;
import com.android.camera.FeatureSwitcher;
import com.android.camera.FileSaver;
import com.android.camera.ParametersHelper;
import com.android.camera.R;
import com.android.camera.Storage;
import com.android.camera.manager.EffectViewManager.EffectListener;
import com.android.camera.manager.IndicatorManager;
import com.android.camera.manager.InfoManager;
import com.android.camera.manager.ModePicker;
import com.android.camera.manager.ModePicker.OnModeChangedListener;
import com.android.camera.manager.OnScreenHint;
import com.android.camera.manager.PickerManager;
import com.android.camera.manager.PickerManager.PickerListener;
import com.android.camera.manager.EffectViewManager;
import com.android.camera.manager.RemainingManager;
import com.android.camera.manager.ReviewManager;
import com.android.camera.manager.RotateDialog;
import com.android.camera.manager.RotateProgress;
import com.android.camera.manager.SettingManager;
import com.android.camera.manager.SettingManager.SettingListener;
import com.android.camera.manager.ShutterManager;
import com.android.camera.manager.SubSettingManager;
import com.android.camera.manager.ThumbnailViewManager;
import com.android.camera.manager.ViewManager;
import com.android.camera.manager.ZoomManager;
import com.android.camera.ui.FaceBeautyEntryView;
import com.android.camera.ui.ShutterButton;
import com.android.camera.ui.ShutterButton.OnShutterButtonListener;

import android.app.Activity;
import android.content.ContentResolver;
import android.content.Intent;
import android.media.CamcorderProfile;
import android.net.Uri;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.view.SurfaceView;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.ImageView;

import com.mediatek.camera.ICameraMode.CameraModeType;
import com.mediatek.camera.platform.ICameraAppUi;
import com.mediatek.camera.platform.ICameraAppUi.GestureListener;
import com.mediatek.camera.platform.ICameraView;
import com.mediatek.camera.platform.IModuleCtrl;
import com.mediatek.camera.ISettingCtrl;
import com.mediatek.camera.setting.SettingConstants;
import com.mediatek.camera.setting.preference.ListPreference;
import com.mediatek.camera.util.Log;

public class CameraAppUiImpl implements ICameraAppUi {
    private static final String TAG = "CameraAppUiImpl";

    private static final int MSG_SHOW_ONSCREEN_INDICATOR = 1000;
    private static final int MSG_DELAY_SHOW_ONSCREEN_INDICATOR = 1001;
    private static final int MSG_SHOW_REMAINING = 1002;
    private static final int DELAY_MSG_SHOW_ONSCREEN_TIME = 1 * 1000;
    private static final int DELAY_MSG_SHOW_ONSCREEN_VIEW = 3 * 1000;

    /**
     * tag for whether auto back to VFB mode. if true means last time leave out
     * VFB mode by the preview not have face in 5s,so next time when detected
     * face will automatic into VFB mode
     */
    private boolean mIsNeedBackToVFBMode = false;

    /**
     * use for tag current whether in camera preview or in Gallery
     */
    private boolean mIsInCameraPreview = true;

    /**
     * used for tag for video shutter button can click,if WFD is connected,
     * this tag will be set to false
     */
    private boolean mIsVideoShutterButtonEanble = true;

    private final CameraActivity mCameraActivity;
    private ShutterManager mShutterManager;
    private ModePicker mModePicker;
    private ThumbnailViewManager mThumbnailManager;
    private PickerManager mPickerManager;
    private IndicatorManager mIndicatorManager;
    private RemainingManager mRemainingManager;
    private SettingManager mSettingManager;
    private InfoManager mInfoManager;
    private ZoomManager mZoomManager;
    private MainHandler mMainHandler;
    private RotateProgress mRotateProgress;
    private ReviewManager mReviewManager;
    private RotateDialog mRotateDialog;
    private EffectViewManager mEffectManager;
    private FaceBeautyEntryView mFaceBeautyEntryView;
    private SubSettingManager mSubSettingManager; // For tablet
    private ViewState mCurrentViewState = ViewState.VIEW_STATE_NORMAL;
    private ViewState mRestoreViewState = ViewState.VIEW_STATE_NORMAL;
    private OnScreenHint mRotateToast;
    private List<ViewManager> mViewManagers = new CopyOnWriteArrayList<ViewManager>();
    private HashMap<CommonUiType, ICameraView> mCameraViewArray =
            new HashMap<CommonUiType, ICameraView>();

    private ViewGroup mViewLayerBottom;
    private ViewGroup mViewLayerNormal;
    private ViewGroup mViewLayerTop;
    private ViewGroup mViewLayerShutter;
    private ViewGroup mViewLayerSetting;
    private ViewGroup mViewLayerOverlay;

    // restore old surface view window's alpha value
    private static final float TRANSPARENT = 0.0f;
    private float     mOldSurfaceViewAlphaValue = 1.0f;

    private EffectListenerImpl mEffectListener = new EffectListenerImpl();

    public CameraAppUiImpl(CameraActivity context) {
        Log.d(TAG, "[CameraAppUiImpl] constructor... ");
        mCameraActivity = context;
        mMainHandler = new MainHandler(context.getMainLooper());
    }

    public void createCommonView() {
        mShutterManager = new ShutterManager(mCameraActivity);
        mInfoManager = new InfoManager(mCameraActivity);
        mRotateProgress = new RotateProgress(mCameraActivity);
        mRemainingManager = new RemainingManager(mCameraActivity);
        mPickerManager = new PickerManager(mCameraActivity);
        mIndicatorManager = new IndicatorManager(mCameraActivity);
        mReviewManager = new ReviewManager(mCameraActivity);
        mRotateDialog = new RotateDialog(mCameraActivity);
        mZoomManager = new ZoomManager(mCameraActivity);
        mThumbnailManager = new ThumbnailViewManager(mCameraActivity);
        if (FeatureSwitcher.isVfbEnable()) {
            mFaceBeautyEntryView = new FaceBeautyEntryView(mCameraActivity); //add for FB entry
        }

        mSettingManager = new SettingManager(mCameraActivity);
        mEffectManager = new EffectViewManager(mCameraActivity, mEffectListener);
        // For tablet
        if (FeatureSwitcher.isSubSettingEnabled()) {
            mSubSettingManager = new SubSettingManager(mCameraActivity);
        }
    }

    public void initializeCommonView() {
        mModePicker = mCameraActivity.getModePicker();

        mCameraViewArray.put(CommonUiType.SHUTTER, new CameraViewImpl(mShutterManager));
        mCameraViewArray.put(CommonUiType.MODE_PICKER, new CameraViewImpl(mModePicker));
        mCameraViewArray.put(CommonUiType.THUMBNAIL, new CameraViewImpl(mThumbnailManager));
        mCameraViewArray.put(CommonUiType.PICKER, new CameraViewImpl(mPickerManager));
        mCameraViewArray.put(CommonUiType.INDICATOR, new CameraViewImpl(mIndicatorManager));
        mCameraViewArray.put(CommonUiType.REMAINING, new CameraViewImpl(mRemainingManager));
        mCameraViewArray.put(CommonUiType.INFO, new CameraViewImpl(mInfoManager));
        mCameraViewArray.put(CommonUiType.REVIEW, new CameraViewImpl(mReviewManager));
        mCameraViewArray.put(CommonUiType.ROTATE_PROGRESS, new CameraViewImpl(mRotateProgress));
        mCameraViewArray.put(CommonUiType.ROTATE_DIALOG, new CameraViewImpl(mRotateDialog));
        mCameraViewArray.put(CommonUiType.ZOOM, new CameraViewImpl(mZoomManager));
        mCameraViewArray.put(CommonUiType.SETTING, new CameraViewImpl(mSettingManager));
        if (mFaceBeautyEntryView != null) {
            mCameraViewArray.put(CommonUiType.FACE_BEAUTY_ENTRY, new CameraViewImpl(
                    mFaceBeautyEntryView));
        }
    }

    public void initializeAfterPreview() {
        mModePicker.setCurrentMode(mCameraActivity.getCurrentMode());
    }
    @Override
    public ICameraView getCameraView(CommonUiType type) {
        return mCameraViewArray.get(type);
    }

    @Override
    public ImageView getVideoShutter() {
        return mShutterManager.getVideoShutter();
    }

    @Override
    public ImageView getPhotoShutter() {
        return mShutterManager.getPhotoShutter();
    }

    @Override
    public void switchShutterType(ShutterButtonType type) {
        switch (type) {
        case SHUTTER_TYPE_PHOTO_VIDEO:
            mShutterManager.switchShutter(ShutterManager.SHUTTER_TYPE_PHOTO_VIDEO);
            break;

        case SHUTTER_TYPE_PHOTO:
            mShutterManager.switchShutter(ShutterManager.SHUTTER_TYPE_PHOTO);
            break;

        case SHUTTER_TYPE_VIDEO:
            mShutterManager.switchShutter(ShutterManager.SHUTTER_TYPE_VIDEO);
            break;

        case SHUTTER_TYPE_OK_CANCEL:
            mShutterManager.switchShutter(ShutterManager.SHUTTER_TYPE_OK_CANCEL);
            break;

        case SHUTTER_TYPE_CANCEL:
            mShutterManager.switchShutter(ShutterManager.SHUTTER_TYPE_CANCEL);
            break;

        case SHUTTER_TYPE_CANCEL_VIDEO:
            mShutterManager.switchShutter(ShutterManager.SHUTTER_TYPE_CANCEL_VIDEO);
            break;

        case SHUTTER_TYPE_SLOW_VIDEO:
            mShutterManager.switchShutter(ShutterManager.SHUTTER_TYPE_SLOW_VIDEO);
            break;

        default:
            break;
        }
    }

    public ShutterButtonType getShutterType() {
        int type = mShutterManager.getShutterType();
        switch (type) {
        case ShutterManager.SHUTTER_TYPE_PHOTO_VIDEO:
            return ShutterButtonType.SHUTTER_TYPE_PHOTO_VIDEO;

        case ShutterManager.SHUTTER_TYPE_PHOTO:
            return ShutterButtonType.SHUTTER_TYPE_PHOTO;

        case ShutterManager.SHUTTER_TYPE_VIDEO:
            return ShutterButtonType.SHUTTER_TYPE_VIDEO;

        case ShutterManager.SHUTTER_TYPE_OK_CANCEL:
            return ShutterButtonType.SHUTTER_TYPE_OK_CANCEL;

        case ShutterManager.SHUTTER_TYPE_CANCEL:
            return ShutterButtonType.SHUTTER_TYPE_CANCEL;

        case ShutterManager.SHUTTER_TYPE_CANCEL_VIDEO:
            return ShutterButtonType.SHUTTER_TYPE_CANCEL_VIDEO;

        case ShutterManager.SHUTTER_TYPE_SLOW_VIDEO:
            return ShutterButtonType.SHUTTER_TYPE_SLOW_VIDEO;

        default:
            Log.w(TAG, "[getShutterType] illegal type:" + type);
            return null;
        }
    }

    @Override
    public void setCurrentMode(CameraModeType mode) {
        Log.d(TAG, "[setCurrentMode] mode = " + mode + ",curMode:" + mModePicker.getCurrentMode());

        int modePickerMode = getModePickerMode(mode);
        if (modePickerMode == mModePicker.getCurrentMode()) {
            return;
        }
        //disable change mode when onModechange
        mModePicker.setEnabled(false);
        mModePicker.setCurrentMode(modePickerMode);
        mModePicker.setEnabled(true);
    }

    @Override
    public void setThumbnailRefreshInterval(int ms) {
        mThumbnailManager.setRefreshInterval(ms);
    }

    @Override
    public void forceThumbnailUpdate() {
        mThumbnailManager.forceUpdate();
    }

    @Override
    public Uri getThumbnailUri() {
       return mThumbnailManager.getThumbnailUri();
    }

    @Override
    public String getThumbnailMimeType() {
        return mThumbnailManager.getThumbnailMimeType();
    }

    @Override
    public void updateThumbnailViewWithYuv(byte[] yuvData, int yuvWidth,
            int yuvHeight, int orientation, int imageFormat) {
       mThumbnailManager.updateThumbnailViewWithYuv(yuvData, yuvWidth,
               yuvHeight, orientation, imageFormat);
    }

    @Override
    public void notifyNewMedia(Uri uri, boolean needNotify) {
        if (uri != null && needNotify) {
            ContentResolver cr = mCameraActivity.getContentResolver();
            String mimeType = cr.getType(uri);
            if (mimeType != null) {
                if (mimeType.startsWith("image/")) {
                    Log.i(TAG, "[notifyNewMedia] uri is "+uri);
                    mCameraActivity.sendBroadcast(
                            new Intent("android.hardware.action.NEW_PICTURE", uri));
                } else if (mimeType.startsWith("video/")) {
                    Log.i(TAG, "[notifyNewMedia] uri is "+uri);
                    mCameraActivity.sendBroadcast(
                            new Intent("android.hardware.action.NEW_VIDEO", uri));
                }
            }
        }
    }

    @Override
    public void setCameraId(int cameraId) {
        mPickerManager.setCameraId(cameraId);
    }

    @Override
    public void updatePreference() {
        mPickerManager.onPreferenceReady();
    }

    public void updateSnapShotUIView(boolean enabled) {
        mCameraActivity.showBorder(enabled);
        mZoomManager.setEnabled(!enabled);
        mShutterManager.getPhotoShutter().setEnabled(!enabled);
    }

    /**
     * set dng on or off state.
     * @param state "on" or "off".
     */
    public void setDngState(String state) {
        mRemainingManager.setDngState(state);
    }

    public void showRemainHint() {
        mRemainingManager.showHint();
    }

    public void clearRemainAvaliableSpace() {
        mRemainingManager.clearAvaliableSpace();
    }

    public boolean showRemainIfNeed() {
        return mRemainingManager.showIfNeed();
    }

    public void notifyParametersReady() {
        Log.d(TAG, "[notifyParametersReady]");
        if (ViewState.VIEW_STATE_RECORDING == getViewState()
                || ViewState.VIEW_STATE_PRE_RECORDING == getViewState()) {
            return;
        }
        if (isEffectConditionSatisfied()) {
            mEffectManager.show();
        } else {
            mEffectManager.hide();
        }
    }

    @Override
    public long updateRemainStorage() {
       return mRemainingManager.updateStorage();
    }

    public void setSettingCtrl(ISettingCtrl settingCtrl) {
        mSettingManager.setSettingController(settingCtrl);
        mShutterManager.setSettingController(settingCtrl);

        if (FeatureSwitcher.isSubSettingEnabled()) {
            mSubSettingManager.setSettingController(settingCtrl);
        }
    }

    public void showText(CharSequence text) {
        mInfoManager.showText(text);
    }

    public boolean collapseSetting(boolean force) {
        return mSettingManager.collapse(force);
    }

    public boolean collapseSubSetting(boolean force) {
        return mSubSettingManager.collapse(force);
    }


    public boolean performSettingClick() {
        return mSettingManager.handleMenuEvent();
    }

    public boolean isSettingShowing() {
        return mSettingManager.isShowSettingContainer();
    }

    public void setSettingListener(SettingListener listener) {
        mSettingManager.setListener(listener);
    }

    public void setSubSettingListener(SettingListener listener) {
        mSubSettingManager.setListener(listener);
    }

    @Override
    public void setReviewCompensation(int compensation) {
        mReviewManager.setOrientationCompensation(compensation);
    }

    @Override
    public void changeZoomForQuality() {
        mZoomManager.changeZoomForQuality();
    }

    @Override
    public void onDetectedSceneMode(int scene, boolean suggestedHdr) {
        mIndicatorManager.onDetectedSceneMode(scene);
        ListPreference hdrPref = mCameraActivity.getISettingCtrl().getListPreference(
                SettingConstants.KEY_HDR);
        // No need to force enable and disable hdr when hdr is not supported
        if (hdrPref == null) {
            return;
        }
        if (suggestedHdr) {
            mPickerManager.forceEnable(SettingConstants.KEY_HDR);
            showToast(mCameraActivity.getString(R.string.asd_hdr_guide));
        } else {
            mPickerManager.cancelForcedEnable(SettingConstants.KEY_HDR);
            hideToast();
        }
    }

    @Override
    public void restoreSceneMode() {
        mPickerManager.cancelForcedEnable(SettingConstants.KEY_HDR);
        hideToast();
        mIndicatorManager.restoreSceneMode();
    }
    @Override
    public void setSwipeEnabled(boolean enabled) {
    }

    @Override
    public void setGestureListener(GestureListener listener) {
        mCameraActivity.setGestureListener(listener);
    }

    @Override
    public ViewGroup getBottomViewLayer() {
        return mViewLayerBottom;
    }

    @Override
    public ViewGroup getNormalViewLayer() {
        return mViewLayerNormal;
    }

    @Override
    public void changeBackToVFBModeStatues(boolean isNeed) {
        Log.d(TAG, "[changeBackToVFBModeStatues] isNeed = " + isNeed);
        mIsNeedBackToVFBMode = isNeed;
    }

    @Override
    public boolean isNeedBackToVFBMode() {
        return mIsNeedBackToVFBMode;
    }

    @Override
    public void updateFaceBeatuyEntryViewVisible(boolean visible) {
        Log.d(TAG, "[updateFaceBeatuyEntryViewVisible] visible = " + visible
                + ",mIsInCameraPreview = " + mIsInCameraPreview + ",mIsNeedBackToVFBMode = "
                + mIsNeedBackToVFBMode);
        if (mFaceBeautyEntryView != null) {
            if (visible && mIsInCameraPreview) {
                /**
                 * Case One: when into Camera->Gallery,but current the FD is
                 * also running, so this case not need to show the FB relative
                 * UI.
                 *
                 * Case Two: when last time leave VFB mode by automatic[the
                 * preview have no face in 5s], so the tag:mIsNeedBackToVFBMode
                 * will be set true, and now the preview have face, in this case
                 * not need show the FB entry view,directly into VFB mode
                 */
                if (mIsNeedBackToVFBMode) {
                    changeBackToVFBModeStatues(false);
                    setCurrentMode(CameraModeType.EXT_MODE_FACE_BEAUTY);
                } else {
                    mFaceBeautyEntryView.show();
                }
            } else {
                mFaceBeautyEntryView.hide();
            }
        }
    }

    @Override
    public void showRemaining() {
        mMainHandler.removeMessages(MSG_SHOW_ONSCREEN_INDICATOR);
        //doShowRemaining(false);
        mMainHandler.obtainMessage(MSG_SHOW_REMAINING, false).sendToTarget();
    }

    public void refreshModeRelated() {
        // should update ModePicker mode firstly.
        mModePicker.refresh();
        mPickerManager.refresh();
        mShutterManager.refresh();
    }

    @Override
    public void setReviewListener(OnClickListener retatekListener, OnClickListener playListener) {
        mReviewManager.setReviewListener(retatekListener, playListener);
    }

    @Override
    public void showReview(String filePath, FileDescriptor fd) {
        if (null != filePath) {
            setViewState(ViewState.VIEW_STATE_REVIEW);
            mReviewManager.show(filePath);
        } else if (null != fd) {
            setViewState(ViewState.VIEW_STATE_REVIEW);
            mReviewManager.show(fd);
        } else {
            setViewState(ViewState.VIEW_STATE_REVIEW);
            mReviewManager.show();
        }
    }

    @Override
    public void hideReview() {
        mReviewManager.hide();
        restoreViewState();
    }

    @Override
    public void showProgress(String msg) {
        setViewState(ViewState.VIEW_STATE_SAVING);
        mRotateProgress.showProgress(msg);
    }

    @Override
    public void dismissProgress() {
        mRotateProgress.hide();
    }

    @Override
    public boolean isShowingProgress() {
        return mRotateProgress.isShowing();
    }

    @Override
    public void showInfo(final String text) {
        showInfo(text, DELAY_MSG_SHOW_ONSCREEN_VIEW);
    }

    @Override
    public void showInfo(final CharSequence text, int showMs) {
        mMainHandler.removeMessages(MSG_SHOW_ONSCREEN_INDICATOR);
        doShowInfo(text, showMs);
    }

    @Override
    public void dismissInfo() {
        mMainHandler.removeMessages(MSG_SHOW_ONSCREEN_INDICATOR);
        mMainHandler.sendEmptyMessage(MSG_SHOW_ONSCREEN_INDICATOR);
    }

    @Override
    public boolean collapseViewManager(boolean force) {
        boolean handle = false;
        // hide dialog if it's showing.
        if (mRotateDialog.isShowing() && !force) {
            mRotateDialog.collapse(force);
            handle = true;
        } else {
            for (ViewManager manager : mViewManagers) {
                handle = manager.collapse(force);
                if (!force && handle) {
                    break; // just collapse one sub list
                }
            }
        }
        Log.d(TAG, "collapseViewManager(" + force + ") return " + handle);
        return handle;
    }

    @Override
    public void restoreViewState() {
        setViewState(ViewState.VIEW_STATE_NORMAL);
    }

    @Override
    public void setPhotoShutterEnabled(boolean enable) {
        mShutterManager.setPhotoShutterEnabled(enable);
    }


    @Override
    public void updateVideoShutterStatues(boolean enable) {
        mIsVideoShutterButtonEanble = enable;
    }

    @Override
    public void setVideoShutterEnabled(boolean enable) {
        mShutterManager.setVideoShutterEnabled(enable ? mIsVideoShutterButtonEanble : false);
    }

    @Override
    public void setOkButtonEnabled(boolean enable) {
        mShutterManager.setOkButtonEnabled(enable);

    }

    @Override
    public int getPreviewFrameWidth() {
        return mCameraActivity.getPreviewFrameWidth();
    }

    @Override
    public int getUnCropHeight() {
        return mCameraActivity.getUnCropHeight();
    }

    @Override
    public int getUnCropWidth() {
        return mCameraActivity.getUnCropWidth();
    }

    @Override
    public int getPreviewFrameHeight() {
        return mCameraActivity.getPreviewFrameHeight();
    }

    @Override
    public View getPreviewFrameLayout() {
        return mCameraActivity.getPreviewFrameLayout();
    }

    public void hideAllViews() {
        if (mCameraActivity.isNonePickIntent() && mCameraActivity.getParameters() != null) {
            // if mParameters is not ready, it should not show ModePicker,
            // otherwise JE
            // would pop up when change capture mode
            mModePicker.hide();
            mThumbnailManager.hide();
        }
        mSettingManager.hide();
        mIndicatorManager.hide();
        mPickerManager.hide();
        mRemainingManager.hide();
        mEffectManager.hide();
        mMainHandler.removeMessages(MSG_SHOW_ONSCREEN_INDICATOR);
    }

    @Override
    public void showAllViews() {
        if (mCameraActivity.isNonePickIntent() && mCameraActivity.getParameters() != null) {
            // if mParameters is not ready, it should not show ModePicker,
            // otherwise JE
            // would pop up when change capture mode
            mModePicker.show();
            mThumbnailManager.show();
        }
        mSettingManager.show();
        mIndicatorManager.show();
        mPickerManager.show();
        mRemainingManager.show();
        if (isEffectConditionSatisfied()) {
            mEffectManager.show();
        }
    }

    @Override
    public void setViewState(ViewState state) {
        Log.d(TAG, "[setViewState],mCurrentViewState:" + mCurrentViewState + ",newState:" + state);
        if (mCurrentViewState == state) {
            return;
        }

        if (mSettingManager.isShowSettingContainer()) {
            state = ViewState.VIEW_STATE_SETTING;
        } else if (FeatureSwitcher.isSubSettingEnabled()
                && mSubSettingManager.isShowSettingContainer()) {
            state = ViewState.VIEW_STATE_SUB_SETTING;
        }

        if (state == ViewState.VIEW_STATE_CAMERA_CLOSED) {
            mRestoreViewState = mCurrentViewState;
        }

        if (mCurrentViewState == ViewState.VIEW_STATE_CAMERA_CLOSED
                && state != ViewState.VIEW_STATE_CAMERA_OPENED) {
            Log.d(TAG, "[setViewState] set restore view state mRestoreViewState:"
                    + mRestoreViewState + ",state:" + state);
            mRestoreViewState = state;
            return;
        }

        if (state == ViewState.VIEW_STATE_CAMERA_OPENED) {
            mCurrentViewState = mRestoreViewState;
            Log.d(TAG, "[setViewState] view state:" + mCurrentViewState);
        } else {
            mCurrentViewState = state;
        }

        switch (mCurrentViewState) {
        case VIEW_STATE_NORMAL:
            setViewManagerVisible(true);
            setViewManagerEnable(true);
            mShutterManager.setEnabled(true);
            mShutterManager.setVideoShutterEnabled(mIsVideoShutterButtonEanble);
            mSettingManager.setFileter(true);
            mSettingManager.setAnimationEnabled(true, true);
            // For tablet
            if (FeatureSwitcher.isSubSettingEnabled()) {
                mSubSettingManager.setFileter(true);
                mSubSettingManager.setAnimationEnabled(true, true);
            }
            if ((!mCameraActivity.isVideoMode() || !mCameraActivity.isNonePickIntent())
                    && mCameraActivity.isCameraOpened()) {
                mPickerManager.show();
            }
            if (!mMainHandler.hasMessages(MSG_SHOW_ONSCREEN_INDICATOR)) {
                showIndicator(0);
            } else {
                Log.d(TAG, "[setViewState]mMainHandler has message MSG_SHOW_ONSCREEN_INDICATOR");
            }
            break;

        case VIEW_STATE_CAPTURE:
            mModePicker.hideToast();
            mSettingManager.collapse(true);
            // For tablet
            if (FeatureSwitcher.isSubSettingEnabled()) {
                mSubSettingManager.collapse(true);
            }
            setViewManagerEnable(false);
            mShutterManager.setEnabled(false);
            break;

        case VIEW_STATE_PRE_RECORDING:
            mModePicker.setEnabled(false);
            mPickerManager.setEnabled(false);
            mShutterManager.setEnabled(false);
            mSettingManager.setEnabled(false);
            mEffectManager.setEnabled(false);
            mModePicker.hide();
            mPickerManager.hide();
            mSettingManager.hide();
            mEffectManager.hide();
            break;
        case VIEW_STATE_RECORDING:
            mEffectManager.setEnabled(false);
            mModePicker.hideToast();
            mEffectManager.hide();
            mShutterManager.setEnabled(true);
            // when WFD Connected,then VR,will found the VideoShutter button
            // will be set to enable TODO will check this whether need
            mShutterManager.setVideoShutterEnabled(mIsVideoShutterButtonEanble);
            mSettingManager.collapse(true);
            // For tablet
            if (FeatureSwitcher.isSubSettingEnabled()) {
                mSubSettingManager.collapse(true);
            }
            setViewManagerVisible(false);
            setViewManagerEnable(false);
            mZoomManager.setEnabled(true);
            break;

        case VIEW_STATE_SETTING:
            mModePicker.hide();
            mThumbnailManager.hide();
            mPickerManager.hide();
            mEffectManager.hide();
            setViewManagerEnable(false);
            mSettingManager.setEnabled(true);
            mIndicatorManager.refresh();
            //when is show setting ,need hide the FaceBeauty view
            if (mFaceBeautyEntryView != null) {
                mFaceBeautyEntryView.hide();
            }
            break;

        case VIEW_STATE_SUB_SETTING:
            mModePicker.hide();
            mThumbnailManager.hide();
            mPickerManager.hide();
            setViewManagerEnable(false);
            if (FeatureSwitcher.isSubSettingEnabled()) {
                mSubSettingManager.setEnabled(true);
            }
            break;

        case VIEW_STATE_FOCUSING:
            setViewManagerEnable(false);
            break;

        case VIEW_STATE_SAVING:
            mModePicker.hideToast();
            mShutterManager.setEnabled(false);
            setViewManagerVisible(false);
            setViewManagerEnable(false);
            break;

        case VIEW_STATE_REVIEW:
            mShutterManager.setEnabled(true);
            setViewManagerVisible(false);
            setViewManagerEnable(false);
            mInfoManager.hide();
            break;

        case VIEW_STATE_PICKING:
            mShutterManager.setEnabled(true);
            setViewManagerVisible(false);
            setViewManagerEnable(false);
            break;
        case VIEW_STATE_CONTINUOUS_CAPTURE:
            mIndicatorManager.refresh();
            mModePicker.hideToast();
            mSettingManager.collapse(true);
            // For tablet
            if (FeatureSwitcher.isSubSettingEnabled()) {
                mSubSettingManager.collapse(true);
            }
            setViewManagerVisible(false);
            setViewManagerEnable(false);
            mShutterManager.setVideoShutterEnabled(false);
            break;

        case VIEW_STATE_CAMERA_CLOSED:
            setViewManagerEnable(false);
            mShutterManager.setEnabled(false);
            // Hide indicator for not show indicator before remaining show.
            mIndicatorManager.hide();
            // when switch camera ,need hide the FaceBeauty view
            if (mFaceBeautyEntryView != null) {
                mFaceBeautyEntryView.hide();
            }
            mMainHandler.removeMessages(MSG_SHOW_REMAINING);
            break;

        case VIEW_STATE_LOMOEFFECT_SETTING:
            mModePicker.hide();
            mThumbnailManager.hide();
            mPickerManager.hide();
            setViewManagerEnable(false);
            mSettingManager.hide();
            mIndicatorManager.refresh();
            mShutterManager.setEnabled(false);
            mShutterManager.hide();
            hideToast();
            if (mFaceBeautyEntryView != null) {
                mFaceBeautyEntryView.hide();
            }
            break;

        case VIEW_STATE_HIDE_ALL_VIEW:
            hideAllViews();
            break;

        default:
            break;
        }
    }

    public ViewState getViewState() {
        return mCurrentViewState;
    }

    @Override
    public ICameraView getCameraView(SpecViewType type) {
        return ViewFactory.getInstance().createViewManager(mCameraActivity, type);
    }

    @Override
    public void showToastForShort(int stringId) {
        String message = mCameraActivity.getString(stringId);
        Log.d(TAG, "[showToastForShort](" + message + ")");
        showToastForShort(message);
    }

    public void showToastForShort(String message) {
        Log.d(TAG, "showToast(" + message + ")");
        if (message != null && mCameraActivity.isFullScreen()) {
            if (mRotateToast == null) {
                mRotateToast = OnScreenHint.makeText(mCameraActivity, message);
            } else {
                mRotateToast.setText(message);
            }
            mRotateToast.showToastForShort();
        }
    }

    @Override
    public void setVideoShutterMask(boolean enable) {
        mShutterManager.setVideoShutterMask(enable);
    };

    @Override
    public boolean isNormalViewState() {
        return (mCurrentViewState == ViewState.VIEW_STATE_NORMAL);
    }

    @Override
    public void setCamcorderProfile(CamcorderProfile profile) {
        mRemainingManager.setCamcorderProfile(profile);
    }

    @Override
    public void showToast(int stringId) {
        String message = mCameraActivity.getString(stringId);
        showToast(message);
    }

    @Override
    public void showToast(String message) {
        Log.d(TAG, "showToast(" + message + ")");
        if (message != null && mCameraActivity.isFullScreen()) {
            if (mRotateToast == null) {
                mRotateToast = OnScreenHint.makeText(mCameraActivity, message);
            } else {
                mRotateToast.setText(message);
            }
            mRotateToast.showToast();
        }
    }

    public void hideInfoManager() {
        if (mInfoManager != null) {
            mInfoManager.hide();
        }
    }

    @Override
    public void showRemainingAways() {
        Log.d(TAG, "[showRemainingAways]");
        mMainHandler.removeMessages(MSG_SHOW_ONSCREEN_INDICATOR);
        //doShowRemaining(true);
        mMainHandler.obtainMessage(MSG_SHOW_REMAINING, true).sendToTarget();
    }

    public void applayViewCallbacks() {
        mShutterManager.setShutterListener(mPhotoShutterListener, mVideoShutterListener,
                mCameraActivity.getCameraActor().getOkListener(), mCameraActivity.getCameraActor()
                        .getCancelListener());
    }

    public void clearViewCallbacks() {
        mShutterManager.setShutterListener(null, null, null, null);
    }

    public void setModeChangeListener(OnModeChangedListener listener) {
        mModePicker.setListener(listener);
    }

    public void setThumbnailFileSaver(FileSaver saver) {
        mThumbnailManager.addFileSaver(saver);
    }

    public void resetSettings() {
        mSettingManager.resetSettings();
    }
    public void showSetting() {
        mSettingManager.showSetting();
    }
    @Override
    public void hideToast() {
        Log.d(TAG, "[hideToast]");
        if (mRotateToast != null) {
            mRotateToast.cancel();
        }
    }

    @Override
    public void showAlertDialog(String title, String msg, String button1Text, final Runnable r1,
            String button2Text, final Runnable r2) {
        mRotateDialog.showAlertDialog(title, msg, button1Text, r1, button2Text, r2);
    }

    public void showIndicator(int delayMs) {
        Log.d(TAG, "[showIndicator] (" + delayMs + ")");
        mMainHandler.removeMessages(MSG_SHOW_ONSCREEN_INDICATOR);
        if (delayMs > 0) {
            mMainHandler.sendEmptyMessageDelayed(MSG_SHOW_ONSCREEN_INDICATOR, delayMs);
        } else {
            mMainHandler.sendEmptyMessage(MSG_SHOW_ONSCREEN_INDICATOR);
        }
    }

    public boolean addViewManager(ViewManager viewManager) {
        if (!mViewManagers.contains(viewManager)) {
            return mViewManagers.add(viewManager);
        }
        return false;
    }

    public boolean removeViewManager(ViewManager viewManager) {
        return mViewManagers.remove(viewManager);
    }

    public void dismissAlertDialog() {
        mRotateDialog.hide();
    }

    public void addFileSaver(FileSaver saver) {
        mThumbnailManager.addFileSaver(saver);
    }

    public void removeFileSaver(FileSaver saver) {
        mThumbnailManager.removeFileSaver(saver);
    }

    public void initializeViewGroup() {
        mViewLayerBottom = (ViewGroup) mCameraActivity.findViewById(R.id.view_layer_bottom);
        mViewLayerNormal = (ViewGroup) mCameraActivity.findViewById(R.id.view_layer_normal);
        mViewLayerTop = (ViewGroup) mCameraActivity.findViewById(R.id.view_layer_top);
        mViewLayerShutter = (ViewGroup) mCameraActivity.findViewById(R.id.view_layer_shutter);
        mViewLayerSetting = (ViewGroup) mCameraActivity.findViewById(R.id.view_layer_setting);
        mViewLayerOverlay = (ViewGroup) mCameraActivity.findViewById(R.id.view_layer_overlay);
    }

    public void removeAllView() {
        if (mViewLayerBottom != null) {
            mViewLayerBottom.removeAllViews();
        }
        if (mViewLayerNormal != null) {
            mViewLayerNormal.removeAllViews();
        }
        if (mViewLayerShutter != null) {
            mViewLayerShutter.removeAllViews();
        }
        if (mViewLayerSetting != null) {
            mViewLayerSetting.removeAllViews();
        }
        if (mViewLayerOverlay != null) {
            mViewLayerOverlay.removeAllViews();
        }
    }

    public void addView(View view, int layer) {
        ViewGroup group = getViewLayer(layer);
        if (group != null) {
            group.addView(view);
        }
    }

    public void removeView(View view, int layer) {
        ViewGroup group = getViewLayer(layer);
        if (group != null) {
            group.removeView(view);
        }
    }

    public View inflate(int layoutId, int layer) {
        // mViewLayerNormal, mViewLayerBottom and mViewLayerTop are same
        // ViewGroup.
        // Here just use one to inflate child view.
        return mCameraActivity.getLayoutInflater().inflate(layoutId, getViewLayer(layer), false);
    }

    public void resetZoom() {
        mZoomManager.resetZoom();
    }

    public void setZoomParameter() {
        mZoomManager.setZoomParameter();
    }

    public void checkViewManagerConfiguration() {
        for (ViewManager manager : mViewManagers) {
            manager.checkConfiguration();
        }
    }

    public void onConfigurationChanged() {
        for (ViewManager manager : mViewManagers) {
            manager.reInflate();
        }
        //because onScreenHint is not extends view manager,
        //so when camera -> Gallery ,rotate device,back to camera
        //this time the screen hint need re-inflate
        //but current screen Hint not the interface of inflate,
        //so just set the mRotateToast to null,when next time need show
        //a toast,will re-inflate
        mRotateToast = null;
    }

    public void setCurrentMode(int mode) {
        mModePicker.setCurrentMode(mode);
    }

    public void setPickerListener(PickerListener listener) {
        mPickerManager.setListener(listener);
    }

    private OnShutterButtonListener mPhotoShutterListener = new OnShutterButtonListener() {

        @Override
        public void onShutterButtonLongPressed(ShutterButton button) {
            Log.d(TAG, "[photo.onShutterButtonLongPressed] (" + button + ")");
            mSettingManager.collapse(true);
            // For tablet
            if (FeatureSwitcher.isSubSettingEnabled()) {
                mSubSettingManager.collapse(true);
            }
            OnShutterButtonListener listener = mCameraActivity.getCameraActor()
                    .getPhotoShutterButtonListener();
            if (listener != null) {
                listener.onShutterButtonLongPressed(button);
            }
        }

        @Override
        public void onShutterButtonFocus(ShutterButton button, boolean pressed) {
            Log.d(TAG, "[photo.onShutterButtonFocus] (" + button + ", " + pressed + ")");
            mSettingManager.cancleHideAnimation();
            mSettingManager.collapse(true);
            // For tablet
            if (FeatureSwitcher.isSubSettingEnabled()) {
                mSubSettingManager.collapse(true);
            }
            OnShutterButtonListener listener = mCameraActivity.getCameraActor()
                    .getPhotoShutterButtonListener();
            if (listener != null) {
                listener.onShutterButtonFocus(button, pressed);
            }
        }

        @Override
        public void onShutterButtonClick(ShutterButton button) {
            Log.d(TAG,
                    "[photo.onShutterButtonClick](" + button + ")isFullScreen()="
                            + mCameraActivity.isFullScreen());

            if (mCameraActivity.isFullScreen()) {
                mSettingManager.collapse(true);
                // For tablet
                if (FeatureSwitcher.isSubSettingEnabled()) {
                    mSubSettingManager.collapse(true);
                }
                OnShutterButtonListener listener = mCameraActivity.getCameraActor()
                        .getPhotoShutterButtonListener();
                if (listener != null) {
                    listener.onShutterButtonClick(button);
                }
            }
        }
    };

    private OnShutterButtonListener mVideoShutterListener = new OnShutterButtonListener() {

        @Override
        public void onShutterButtonLongPressed(ShutterButton button) {
            Log.d(TAG, "[video.onShutterButtonLongPressed] (" + button + ")");
            mSettingManager.collapse(true);
            // For tablet
            if (FeatureSwitcher.isSubSettingEnabled()) {
                mSubSettingManager.collapse(true);
            }
            int mode = mCameraActivity.getCameraActor().getMode();
            // Disable vsdof video mode
            if (mModePicker.getModeIndex(mode) == ModePicker.MODE_STEREO_CAMERA) {
                return;
            }
            OnShutterButtonListener listener = mCameraActivity.getCameraActor()
                    .getVideoShutterButtonListener();
            if (listener != null) {
                listener.onShutterButtonLongPressed(button);
            }
        }

        @Override
        public void onShutterButtonFocus(ShutterButton button, boolean pressed) {
            Log.d(TAG, "[Video.onShutterButtonFocus] (" + button + ", " + pressed + ")");
            if (pressed && mCameraActivity.isFullScreen()) {
                setSwipeEnabled(false);
            }
            mSettingManager.cancleHideAnimation();
            mSettingManager.collapse(true);
            // For tablet
            if (FeatureSwitcher.isSubSettingEnabled()) {
                mSubSettingManager.collapse(true);
            }
            int mode = mCameraActivity.getCameraActor().getMode();
            // Disable vsdof video mode
            if (mModePicker.getModeIndex(mode) == ModePicker.MODE_STEREO_CAMERA) {
                return;
            }
            OnShutterButtonListener listener = mCameraActivity.getCameraActor()
                    .getVideoShutterButtonListener();
            if (listener != null && mCameraActivity.isCameraOpened()) {
                listener.onShutterButtonFocus(button, pressed);
            }
        }

        @Override
        public void onShutterButtonClick(final ShutterButton button) {
            Log.d(TAG,
                    "[Video.onShutterButtonClick] (" + button + ") isFullScreen()="
                            + mCameraActivity.isFullScreen() + ",isCameraOpened = "
                            + mCameraActivity.isCameraOpened()  + ",Camera State = "
                            + mCameraActivity.getCameraState());
            // when click lomo effect icon and quickly click video shutter
            // button, the lomo effect will be show
            // when recording
            if (mCurrentViewState == ViewState.VIEW_STATE_LOMOEFFECT_SETTING) {
                return;
            }
            mSettingManager.collapse(true);
            // For tablet
            if (FeatureSwitcher.isSubSettingEnabled()) {
                mSubSettingManager.collapse(true);
            }
            // make sure the preview is ready when call it.
            if (mCameraActivity.isFullScreen()
                    && mCameraActivity.isCameraOpened()
                    && (mCameraActivity.getCameraState() == CameraActivity.STATE_IDLE ||
                    mCameraActivity.getCameraState() == CameraActivity.STATE_FOCUSING)) {
                OnShutterButtonListener listener = mCameraActivity
                        .getCameraActor().getVideoShutterButtonListener();
                int mode = mCameraActivity.getCameraActor().getMode();
                // Disable vsdof video mode
                if (mModePicker.getModeIndex(mode) == ModePicker.MODE_STEREO_CAMERA) {
                    return;
                }
                if (listener != null) {
                    listener.onShutterButtonClick(button);
                } else if (mModePicker.getModeIndex(mode) != ModePicker.MODE_VIDEO) {
                    if (Storage.getLeftSpace() > 0) {
                        if (mModePicker.getModeIndex(mode) == ModePicker.MODE_PHOTO_PIP) {
                         // new video pip mode
                            mModePicker.setCurrentMode(ModePicker.MODE_VIDEO_PIP);
                        } else if (mModePicker.getModeIndex(
                                mCameraActivity.getCameraActor().getMode())
                                == ModePicker.MODE_STEREO_CAMERA
                                && ParametersHelper.isVsDofSupported(
                                        mCameraActivity.getParameters())) {
                            mModePicker.setCurrentMode(ModePicker.MODE_VIDEO_STEREO);
                        } else {
                         // new video mode
                            mModePicker.setCurrentMode(ModePicker.MODE_VIDEO);
                        }
                        setViewState(ViewState.VIEW_STATE_PRE_RECORDING);
                        mMainHandler.post(new Runnable() {
                            @Override
                            public void run() {
                                OnShutterButtonListener listener = mCameraActivity
                                        .getCameraActor().getVideoShutterButtonListener();
                                if (listener != null) {
                                    listener.onShutterButtonClick(button);
                                } else {
                                    Log.i(TAG, "error video shutter listener is null");
                                }
                            }
                        });
                    } else {
                        // should set enabld true because onShutterButtonFocus set it false
                        setSwipeEnabled(true);
                    }
                }
            }
        }
    };

    private void doShowIndicator() {
        Log.d(TAG, "[doShowIndicator]");

        mInfoManager.hide();
        mRemainingManager.hide();
        // if Effects is showing,not need show the picker button
        if (mCurrentViewState == ViewState.VIEW_STATE_NORMAL
                && (!mCameraActivity.isVideoMode() || !mCameraActivity.isNonePickIntent())
                && mCameraActivity.isCameraOpened()) {
            mPickerManager.show();
        }
        if (mCurrentViewState != ViewState.VIEW_STATE_SAVING) {
            mIndicatorManager.show();
        }
    }

    private final class MainHandler extends Handler {

        public MainHandler(Looper mainLooper) {
            super(mainLooper);
        }

        @Override
        public void handleMessage(Message msg) {
            Log.d(TAG, "msg id=" + msg.what);
            switch (msg.what) {
            case MSG_SHOW_ONSCREEN_INDICATOR:
                doShowIndicator();
                break;

            case MSG_DELAY_SHOW_ONSCREEN_INDICATOR:
                mRemainingManager.hide();
                mInfoManager.showText((CharSequence) msg.obj);
                showIndicator(msg.arg1);
                break;

            case MSG_SHOW_REMAINING:
                doShowRemaining((Boolean) msg.obj);
                break;

            default:
                break;
            }
        }
    }

    private void setViewManagerVisible(boolean visible) {
        if (visible) {
            if (mCameraActivity.isNonePickIntent() && mCameraActivity.getParameters() != null) {
                // if mParameters is not ready, it should not show ModePicker,
                // otherwise JE
                // would pop up when change capture mode
                mModePicker.show();
                mThumbnailManager.show();
            }
            mShutterManager.show();
            mSettingManager.show();
            // For tablet
            if (FeatureSwitcher.isSubSettingEnabled()) {
                mSubSettingManager.show();
            }

            if (isEffectConditionSatisfied()) {
                mEffectManager.show();
            }
        } else {
            mModePicker.hide();
            mPickerManager.hide();
            mSettingManager.hide();
            // For tablet
            if (FeatureSwitcher.isSubSettingEnabled()) {
                mSubSettingManager.hide();
            }
            mEffectManager.hide();
            if (mFaceBeautyEntryView != null) {
                mFaceBeautyEntryView.hide();
            }
        }
    }

    private void setViewManagerEnable(boolean enabled) {
        if (mCameraActivity.isNonePickIntent()) {
            if (!mCameraActivity.isModeChanged()) {
                mModePicker.setEnabled(enabled);
            }
            mThumbnailManager.setEnabled(enabled);
        }
        mSettingManager.setEnabled(enabled);
        mPickerManager.setEnabled(enabled);
        mZoomManager.setEnabled(enabled);
        if (mFaceBeautyEntryView != null) {
            mFaceBeautyEntryView.setEnabled(enabled);
        }
        // For tablet
        if (FeatureSwitcher.isSubSettingEnabled()) {
            mSubSettingManager.setEnabled(enabled);
        }
        mEffectManager.setEnabled(enabled);
    }

    private void doShowInfo(final CharSequence text, final int showMs) {
        Log.d(TAG, "doShowInfo(" + text + ", " + showMs + ")");
        mCameraActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mIndicatorManager.hide();
                mPickerManager.hide();
                mRemainingManager.hide();
                mInfoManager.showText(text);
                if (showMs != -1) {
                    showIndicator(showMs);
                }
            }
        });
    }

    private void doShowRemaining(final boolean showAways) {
        Log.d(TAG, "[doShowRemaining](" + showAways + ")");
        boolean remainingShown = false;
        if (showAways) {
            remainingShown = mRemainingManager.showAways();
        } else {
            remainingShown = mRemainingManager.showIfNeed();
        }
        if (remainingShown) {
            mIndicatorManager.hide();
            mInfoManager.hide();
            if (getViewState() == ViewState.VIEW_STATE_NORMAL
                    && (!mCameraActivity.isVideoMode() || !mCameraActivity.isNonePickIntent())) {
                mPickerManager.show();
            }
            showIndicator(DELAY_MSG_SHOW_ONSCREEN_VIEW);
        }
    }

    private ViewGroup getViewLayer(int layer) {
        Log.d(TAG, "[getViewLayer] layer:" + layer);
        ViewGroup viewLayer = null;
        switch (layer) {
        case ViewManager.VIEW_LAYER_BOTTOM:
            viewLayer = mViewLayerBottom;
            break;
        case ViewManager.VIEW_LAYER_NORMAL:
            viewLayer = mViewLayerNormal;
            break;
        case ViewManager.VIEW_LAYER_TOP:
            viewLayer = mViewLayerTop;
            break;
        case ViewManager.VIEW_LAYER_SHUTTER:
            viewLayer = mViewLayerShutter;
            break;
        case ViewManager.VIEW_LAYER_SETTING:
            viewLayer = mViewLayerSetting;
            break;
        case ViewManager.VIEW_LAYER_OVERLAY:
            viewLayer = mViewLayerOverlay;
            break;
        default:
            throw new RuntimeException("Wrong layer:" + layer);
        }
        return viewLayer;
    }

    private boolean isEffectConditionSatisfied() {
        boolean isSatisfied = false;
        ListPreference pref = mCameraActivity.getISettingCtrl()
                .getListPreference(SettingConstants.KEY_COLOR_EFFECT);
        if (pref != null && pref.isEnabled()
                && mCurrentViewState != ViewState.VIEW_STATE_SETTING
                && FeatureSwitcher.isLomoEffectEnabled()
                && mCameraActivity.isNonePickIntent()
                && ModePicker.MODE_VIDEO != mCameraActivity.getCurrentMode()) {
            isSatisfied = true;
        }
        return isSatisfied;
    }

    private class EffectListenerImpl implements EffectListener {
        @Override
        public boolean onClick() {
            mCameraActivity.getModuleManager().onEffectClick();
            return true;
        }
    }

    private int getModePickerMode(CameraModeType mode) {
        switch (mode) {
        case EXT_MODE_PHOTO:
            return ModePicker.MODE_PHOTO;

        case EXT_MODE_FACE_BEAUTY:
            return ModePicker.MODE_FACE_BEAUTY;

        case EXT_MODE_PANORAMA:
            return ModePicker.MODE_PANORAMA;

        case EXT_MODE_VIDEO:
            return ModePicker.MODE_VIDEO;

        case EXT_MODE_VIDEO_PIP:
            return ModePicker.MODE_VIDEO_PIP;

        default:
            break;
        }
        return ModePicker.MODE_PHOTO;
    }

    private class CameraViewImpl implements ICameraView {

        private ViewManager mViewManager;

        public CameraViewImpl(ViewManager viewManager) {
            mViewManager = viewManager;
        }

        @Override
        public void init(Activity activity, ICameraAppUi cameraAppUi, IModuleCtrl moduleCtrl) {
        }

        @Override
        public void show() {
            mViewManager.show();
        }

        @Override
        public void hide() {
            mViewManager.hide();
        }

        @Override
        public void uninit() {
            // do nothing

        }

        @Override
        public void reset() {
            // do nothing
        }

        @Override
        public void reInflate() {
            // do nothing
        }

        @Override
        public boolean isShowing() {
            return mViewManager.isShowing();
        }

        @Override
        public boolean isEnabled() {
            return mViewManager.isEnabled();
        }

        @Override
        public int getViewHeight() {
            return mViewManager.getViewHeight();
        }

        @Override
        public int getViewWidth() {
            return mViewManager.getViewWidth();
        }

        @Override
        public boolean update(int type, Object... args) {
            return true;
        }

        @Override
        public void refresh() {
            mViewManager.refresh();
        }

        @Override
        public void setEnabled(boolean enabled) {
            mViewManager.setEnabled(enabled);
        }

        @Override
        public void onOrientationChanged(int orientation) {
            //do nothing.
        }

        @Override
        public void setListener(Object obj) {
            // do nothing

        }
    }
}
