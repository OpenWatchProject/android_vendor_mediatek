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
 * MediaTek Inc. (C) 2015. All rights reserved.
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

package com.android.camera.v2.app;

import android.app.Activity;
import android.content.res.Configuration;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Matrix;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.view.Surface;
import android.view.View;
import android.view.View.OnLayoutChangeListener;
import android.view.ViewGroup;
import android.widget.FrameLayout;

import com.android.camera.R;
import com.android.camera.v2.app.AppController.OkCancelClickListener;
import com.android.camera.v2.app.AppController.PlayButtonClickListener;
import com.android.camera.v2.app.AppController.RetakeButtonClickListener;
import com.android.camera.v2.app.AppController.ShutterEventsListener;
import com.android.camera.v2.app.PreviewManager.SurfaceCallback;
import com.android.camera.v2.app.SettingAgent.SettingChangedListener;
import com.android.camera.v2.ui.PreviewStatusListener;
import com.android.camera.v2.ui.PreviewStatusListener.OnGestureListener;
import com.android.camera.v2.ui.PreviewStatusListener.OnPreviewTouchedListener;
import com.android.camera.v2.uimanager.IndicatorManager;
import com.android.camera.v2.uimanager.InfoManager;
import com.android.camera.v2.uimanager.ModePicker;
import com.android.camera.v2.uimanager.ModePicker.OnModeChangedListener;
import com.android.camera.v2.uimanager.OnScreenHint;
import com.android.camera.v2.uimanager.PickerManager;
import com.android.camera.v2.uimanager.PickerManager.OnPickedListener;
import com.android.camera.v2.uimanager.RemainingManager;
import com.android.camera.v2.uimanager.ReviewManager;
import com.android.camera.v2.uimanager.ReviewManager.OnPlayButtonClickListener;
import com.android.camera.v2.uimanager.ReviewManager.OnRetakeButtonClickListener;
import com.android.camera.v2.uimanager.RotateDialog;
import com.android.camera.v2.uimanager.RotateProgress;
import com.android.camera.v2.uimanager.SettingManager;
import com.android.camera.v2.uimanager.SettingManager.OnSettingChangedListener;
import com.android.camera.v2.uimanager.SettingManager.OnSettingStatusListener;
import com.android.camera.v2.uimanager.ShutterManager;
import com.android.camera.v2.uimanager.ShutterManager.OnOkCancelButtonClickListener;
import com.android.camera.v2.uimanager.ShutterManager.OnShutterButtonListener;
import com.android.camera.v2.uimanager.ThumbnailManager;
import com.android.camera.v2.uimanager.ThumbnailManager.OnThumbnailClickListener;
import com.android.camera.v2.uimanager.preference.PreferenceManager;
import com.android.camera.v2.util.CameraUtil;
import com.android.camera.v2.util.SettingKeys;
import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;
import com.mediatek.camera.v2.exif.Exif;

import java.util.HashMap;
import java.util.Map;

/**
 * CameraAppUI centralizes control of views shared across modules. Whereas module specific views
 * will be handled in each Module UI. For example, we can now bring the flash animation and capture
 * animation up from each module to app level, as these animations are largely the same for all
 * modules.
 *
 * This class also serves to disambiguate touch events. It recognizes all the swipe gestures that
 * happen on the preview by attaching a touch listener to a full-screen view on top of preview
 * TextureView. Since CameraAppUI has knowledge of how swipe from each direction should be handled,
 * it can then redirect these events to appropriate recipient views.
 */
public class CameraAppUI implements SurfaceCallback {
    private static final Tag TAG = new Tag(CameraAppUI.class.getSimpleName());
    private final AppController mAppController;
    private Activity mCameraActivity;
    private FrameLayout mModuleUI;
    private MainHandler mMainHandler;

    // Preview is fully visible.
    public static final int VISIBILITY_UNCOVERED = 0;
    // Preview is covered by e.g. the transparent mode drawer. */
    public static final int VISIBILITY_COVERED = 1;
    private int mPreviewVisibility = VISIBILITY_UNCOVERED;

    // ui managers parent view groups
    private ViewGroup mViewLayerBottom;
    private ViewGroup mViewLayerNormal;
    private ViewGroup mViewLayerTop;
    private ViewGroup mViewLayerShutter;
    private ViewGroup mViewLayerSetting;
    private ViewGroup mViewLayerOverlay;

    private int mOrientation;
    private int mOrientationCompensation;
    // gestures
    private final GestureManager mGestureManager;
    private boolean mSwipeEnabled = true;
    private boolean mStopShowCommonUi = false;

    private static final int DELAY_MSG_SHOW_INDICATOR_UI = 3000;
    private static final int DOWN_SAMPLE_FACTOR = 4;
    // InfoManager, RemainingManager,IndicatorManager
    // interact MSG
    private static final int MSG_SHOW_INFO_UI = 1000;
    private static final int MSG_SHOW_INDICATOR_UI = 1001;
    private static final int MSG_SHOW_REMAINING_UI = 1002;
    private static final int MSG_HIDE_INFO_UI = 1003;
    private static final int PICTURE_SIZE_4M_HEIGHT = 1440;
    // default on screen view show time duration
    private static final int MSG_DEFAULT_SHOW_ONSCREEN_VIEW_DURATION = 3 * 1000;
    private static final int SHOW_INFO_DURATION_MS = 5 * 1000;
    // Ui Managers
    private ShutterManager mShutterManager;
    private ShutterEventsListener mPhotoShutterListener;
    private SettingManager mSettingManager;
    private PickerManager mPickerManager;
    private ModePicker mModePicker;
    private IndicatorManager mIndicatorManager;
    private RemainingManager mRemainingManager;
    private ThumbnailManager mThumbnailManager;
    private ReviewManager mReviewManager;
    private RotateDialog mRotateDialog;
    private RotateProgress mRotateProgress;
    private PreferenceManager mPreferenceManager;
    private InfoManager mInfoManager;
    private OnScreenHint mOnScreenHint;

    private SettingAgent mSettingAgent;
    private boolean  mIsSecureCamera = false;

    private OnShutterButtonListener mPhotoShutterCallback = new OnShutterButtonListener() {
        @Override
        public void onLongPressed() {
            if (mPhotoShutterListener != null) {
                mPhotoShutterListener.onShutterLongPressed();
            }
        }

        @Override
        public void onFocused(boolean pressed) {
            if (pressed) {
                if (mPhotoShutterListener != null) {
                    mPhotoShutterListener.onShutterPressed();
                }
            } else {
                if (mPhotoShutterListener != null) {
                    mPhotoShutterListener.onShutterReleased();
                }
            }
        }

        @Override
        public void onPressed() {
            if (mSettingManager != null) {
                mSettingManager.collapse(true);
            }
            if (mPhotoShutterListener != null) {
                mPhotoShutterListener.onShutterClicked();
            }
        }
    };

    private ShutterEventsListener mVideoShutterListener;
    private OnShutterButtonListener mVideoShutterCallback = new OnShutterButtonListener() {
        @Override
        public void onLongPressed() {
            if (mVideoShutterListener != null) {
                mVideoShutterListener.onShutterLongPressed();
            }
        }

        @Override
        public void onFocused(boolean pressed) {
            if (pressed) {
                if (mVideoShutterListener != null) {
                    mVideoShutterListener.onShutterPressed();
                }
            } else {
                if (mVideoShutterListener != null) {
                    mVideoShutterListener.onShutterReleased();
                }
            }
        }

        @Override
        public void onPressed() {
            if (mPickerManager != null) {
                mPickerManager.setEnable(false);
            }
            if (mSettingManager != null) {
                mSettingManager.collapse(true);
            }
            if (mVideoShutterListener != null) {
                mVideoShutterListener.onShutterClicked();
            }
        }
    };

    private OkCancelClickListener mOkCancelClickListener;
    private OnOkCancelButtonClickListener mOnOkCancelButtonClickListener =
            new OnOkCancelButtonClickListener() {

        @Override
        public void onOkClick() {
            LogHelper.i(TAG, "[onOkClick]...");
            mOkCancelClickListener.onOkClick();
        }

        @Override
        public void onCancelClick() {
            LogHelper.i(TAG, "[onCancelClick]...");
            mOkCancelClickListener.onCancelClick();
        }
    };

    private OnPreviewTouchedListener mOnPreviewTouchedListener = new OnPreviewTouchedListener() {

        @Override
        public boolean onPreviewTouched() {
            return mSettingManager.collapse(false);
        }
    };

    private OnSettingChangedListener mSettingChangedListener = new OnSettingChangedListener() {

        @Override
        public void onSettingChanged(String key, String value) {
            LogHelper.i(TAG, "[onSettingChanged], key:" + key + ", value:" + value);
            mSettingAgent.doSettingChange(key, value);
        }

        @Override
        public void onSettingRestored() {
            Runnable runnable = new Runnable() {

                @Override
                public void run() {
                    if (mSettingManager != null) {
                        mSettingManager.collapse(true);
                    }

                    if (mModePicker != null) {
                        mModePicker.restoreToNormalMode();
                    }

                    if (mPreferenceManager != null) {
                        mPreferenceManager.restoreSetting();
                    }
                }
            };
            if (mRotateDialog != null) {
                mRotateDialog.showAlertDialog(null,
                        mCameraActivity.getString(R.string.confirm_restore_message),
                        mCameraActivity.getString(android.R.string.cancel), null,
                        mCameraActivity.getString(android.R.string.ok), runnable);
            }
        }
    };

    private OnSettingStatusListener mOnSettingStatusListener = new OnSettingStatusListener() {

        @Override
        public void onShown() {
            LogHelper.i(TAG, "[onShown]...");
            if (mModePicker != null) {
                mModePicker.hide();
            }

            if (mPickerManager != null) {
                mPickerManager.hide();
            }

            if (mThumbnailManager != null) {
                mThumbnailManager.hide();
            }

            mPreviewVisibility = VISIBILITY_COVERED;
            mAppController.onPreviewVisibilityChanged(mPreviewVisibility);
        }

        @Override
        public void onHidden() {
            LogHelper.i(TAG, "[onHidden]...");
            if (mModePicker != null) {
                mModePicker.show();
            }

            if (mPickerManager != null) {
                mPickerManager.show();
            }

            if (mThumbnailManager != null) {
                mThumbnailManager.show();
            }

            mPreviewVisibility = VISIBILITY_UNCOVERED;
            mAppController.onPreviewVisibilityChanged(mPreviewVisibility);
        }
    };

    private OnPickedListener mOnPickedListener = new OnPickedListener() {

        @Override
        public void onPicked(String key, String value) {
            LogHelper.i(TAG, "[onPicked], key:" + key + ", value:" + value);
            if (SettingKeys.KEY_HDR.equals(key)) {
                Map<String, String> changedSettings = new HashMap<String, String>();
                changedSettings.put(key, value);
                mSettingAgent.doSettingChange(key, value);
                if ("on".equals(value)) {
                    showInfo(mCameraActivity.getString(R.string.hdr_guide_capture),
                            SHOW_INFO_DURATION_MS);
                }
            } else if (SettingKeys.KEY_CAMERA_ID.equals(key)) {
                mSettingManager.collapseImmediately();
                mIndicatorManager.hide();
                setAllCommonViewEnable(false);
                mAppController.onCameraPicked(value);
            } else {
                mSettingAgent.doSettingChange(key, value);
            }
        }
    };

    private OnModeChangedListener mOnModeChangedListener = new OnModeChangedListener() {

        @Override
        public void onModeChanged(Map<String, String> changedModes) {
            LogHelper.i(TAG, "[onModeChanged], changedModes:" + changedModes);
            setAllCommonViewEnable(false);
            mAppController.onModeChanged(changedModes);
        }

        @Override
        public void onRestoreToNomalMode(Map<String, String> changedModes) {
            LogHelper.i(TAG, "[onModeChanged], changedModes:" + changedModes);
            mAppController.onModeChanged(changedModes);
        }
    };

    private OnThumbnailClickListener mOnThumbnailClickListener = new OnThumbnailClickListener() {

        @Override
        public void onThumbnailClick() {
            mAppController.gotoGallery();
        }
    };

    private PlayButtonClickListener mPlayButtonClickListener;
    private OnPlayButtonClickListener mOnPlayButtonClickListener =
            new OnPlayButtonClickListener() {

        @Override
        public void onPlayButtonClick() {
            LogHelper.i(TAG, "[onPlayButtonClick]...");
            mPlayButtonClickListener.onPlay();
        }
    };

    private RetakeButtonClickListener mRetakeButtonClickListener;
    private OnRetakeButtonClickListener mOnRetakeButtonClickListener =
            new OnRetakeButtonClickListener() {

        @Override
        public void onRetakeButtonClick() {
            LogHelper.i(TAG, "[onRetakeButtonClick]...");
            mRetakeButtonClickListener.onRetake();
        }
    };

    // preview
    private PreviewManager mPreviewManager;
    private PreviewStatusListener mPreviewStatusListener;

    public CameraAppUI(AppController controller) {
        LogHelper.i(TAG, "[CameraAppUI]+");
        mAppController = controller;
        mCameraActivity = mAppController.getActivity();
        mPreferenceManager = mAppController.getPreferenceManager();
        mGestureManager = mAppController.getGestureManager();
        mGestureManager.registerPreviewTouchListener(mOnPreviewTouchedListener);
        mPreviewManager = mAppController.getPreviewManager();
        mMainHandler = new MainHandler(mCameraActivity.getMainLooper());
    }

    @Override
    public void surfaceAvailable(Surface surface, int width, int height) {
        if (mPreviewStatusListener != null) {
            mPreviewStatusListener.surfaceAvailable(surface, width, height);
        }
    }

    @Override
    public void surfaceDestroyed(Surface surface) {
        if (mPreviewStatusListener != null) {
            mPreviewStatusListener.surfaceDestroyed(surface);
        }
    }

    @Override
    public void surfaceSizeChanged(Surface surface, int width, int height) {
        if (mPreviewStatusListener != null) {
            mPreviewStatusListener.surfaceSizeChanged(surface, width, height);
        }
    }

    /********************************** life cycle manager *********************************/
    public void init(View root, boolean isSecureCamera, boolean isCaptureIntent) {
        LogHelper.i(TAG, "[init]+");
        mIsSecureCamera = isSecureCamera;
        initializeCommonUIManagers();
    }

    public void onCreate() {

    }

    public void onResume() {
        mPreviewManager.resume();
        if (mThumbnailManager != null) {
            mThumbnailManager.onResume();
        }
    }

    public void onPause() {
        mPreviewManager.pause();
        if (mThumbnailManager != null) {
            mThumbnailManager.onPause();
        }
        if (mSettingManager != null) {
            mSettingManager.collapse(true);
        }
        if (mIndicatorManager != null) {
            mIndicatorManager.hide();
        }
        if (mInfoManager != null) {
            mInfoManager.hide();
        }
    }

    public void onDestroy() {
        if (mThumbnailManager != null) {
            mThumbnailManager.onDestroy();
        }
    }

    public void onConfigurationChanged(Configuration newConfig) {
        if (mModePicker != null) {
            mModePicker.reInflate();
        }

        if (mShutterManager != null) {
            mShutterManager.reInflate();
        }

        if (mSettingManager != null) {
            mSettingManager.reInflate();
        }

        if (mPickerManager != null) {
            mPickerManager.reInflate();
        }

        if (mThumbnailManager != null) {
            mThumbnailManager.reInflate();
        }

        if (mIndicatorManager != null) {
            mIndicatorManager.reInflate();
        }

        if (mInfoManager != null) {
            mInfoManager.reInflate();
        }

        if (mRemainingManager != null) {
            mRemainingManager.reInflate();
        }
        mOrientationCompensation = (mOrientation +
                CameraUtil.getDisplayRotation(mCameraActivity)) % 360;
        rotateLayers(mOrientationCompensation);
    }

    public void setSettingAgent(SettingAgent settingAgent) {
        mSettingAgent = settingAgent;
        mSettingAgent.registerSettingChangedListener(new SettingChangedListener() {
            // callback setting result after setting changed.
            @Override
            public void onSettingResult(Map<String, String> values,
                    Map<String, String> overrideValues) {
                if (mPreferenceManager != null) {
                    mPreferenceManager.updateSettingResult(values, overrideValues);
                }

                mMainHandler.post(new Runnable() {

                    @Override
                    public void run() {
                        if (mSettingManager != null) {
                            mSettingManager.refresh();
                        }

                        if (mIndicatorManager != null) {
                            mIndicatorManager.refresh();
                        }

                        if (mPickerManager != null) {
                            mPickerManager.refresh();
                        }
                    }
                });
            }
        }, mMainHandler);
    }

    /**
     * Called when the back key is pressed.
     *
     * @return Whether the UI responded to the key event.
     */
    public boolean onBackPressed() {
        // call view manager's onBackPressed() order is need to considered.
        return mRotateProgress.onBackPressed() || mRotateDialog.onBackPressed()
                || mSettingManager.onBackPressed();
    }

    public void setSwipeEnabled(boolean enabled) {
        // TODO consider remove this variable
        mSwipeEnabled = enabled;
        if (mGestureManager != null) {
            mGestureManager.setScrollEnable(enabled);
        }
    }

    // FIXME consider all common ui manager
    public void setAllCommonViewEnable(boolean enable) {
        LogHelper.i(TAG, "[setAllCommonViewEnable], enable:" + enable);
        mModePicker.setEnable(enable);
        mSettingManager.setEnable(enable);
        mPickerManager.setEnable(enable);
        mShutterManager.setEnable(enable);
        if (!(mStopShowCommonUi && enable)) {
            // stop to make thumbnail manager clickable.
            mThumbnailManager.setEnable(enable);
        }
    }

    public void setAllCommonViewButShutterVisible(boolean visible) {
        if (visible) {
            mSettingManager.show();
            mModePicker.show();
            mIndicatorManager.show();
            mPickerManager.show();
            mThumbnailManager.show();
        } else {
            mSettingManager.hide();
            mModePicker.hide();
            mIndicatorManager.hide();
            mPickerManager.hide();
            mThumbnailManager.hide();
        }
    }

    public Uri getThumbnailUri() {
        return mThumbnailManager.getThumbnailUri();
     }

    public String getThumbnailMimeType() {
        return mThumbnailManager.getThumbnailMimeType();
    }

    public int getPreviewVisibility() {
        LogHelper.i(TAG, "getPreviewVisibility : " + mPreviewVisibility);
        return mPreviewVisibility;
    }

    public void updateSecureThumbnail(boolean needShow) {
        if (mThumbnailManager != null) {
            mThumbnailManager.updateNeedShowThumbnail(needShow);
        }
    }
    /********************************** module ui manager *********************************/

    /**
     * Called indirectly from each module in their initialization to get a view group to inflate the
     * module specific views in.
     *
     * @return a view group for modules to attach views to
     */
    public FrameLayout getModuleRootView() {
        LogHelper.i(TAG, "[getModuleRootView]+");
        return mModuleUI;
    }

    /**
     * This inflates generic_module layout, which contains all the shared views across modules. Then
     * each module inflates their own views in the given view group.
     */
    public void prepareModuleUI() {
        LogHelper.i(TAG, "[prepareModuleUI]+");
        mPreviewManager.setSurfaceCallback(this);
    }

    /**
     * Remove all the module specific views.
     */
    public void clearModuleUI() {
        LogHelper.i(TAG, "[clearModuleUI]+");

    }

    /********************************** preview status *********************************/
    public void setPreviewStatusListener(PreviewStatusListener previewStatusListener) {
        mPreviewStatusListener = previewStatusListener;
        if (mPreviewStatusListener != null) {
            onPreviewListenerChanged();
        }
    }

    public View getPreviewView() {
        return mPreviewManager.getPreviewView();
    }

    public void setOnLayoutChangedListener(OnLayoutChangeListener listener) {
        mPreviewManager.setOnLayoutChangeListener(listener);
    }

    public void addPreviewAreaSizeChangedListener(
            PreviewStatusListener.OnPreviewAreaChangedListener listener) {
        mPreviewManager.addPreviewAreaSizeChangedListener(listener);
    }

    public void removePreviewAreaSizeChangedListener(
            PreviewStatusListener.OnPreviewAreaChangedListener listener) {
        mPreviewManager.removePreviewAreaSizeChangedListener(listener);
    }

    public void updatePreviewSize(int previewWidth, int previewHeight) {
        mPreviewManager.updatePreviewSize(previewWidth, previewHeight);
    }

    public void onPreviewStarted() {
        mPreviewManager.onPreviewStarted();
    }

    /********************************** shutter manager *********************************/

    public void setShutterEventListener(ShutterEventsListener listener, boolean videoShutter) {
        if (videoShutter) {
            mVideoShutterListener = listener;
            mShutterManager.addShutterButtonListener(mVideoShutterCallback, true);
        } else {
            mPhotoShutterListener = listener;
            mShutterManager.addShutterButtonListener(mPhotoShutterCallback, false);
        }
    }

    public void setOkCancelClickListener(OkCancelClickListener listener) {
        mOkCancelClickListener = listener;
        mShutterManager.setOnOkCancelButtonClickListener(mOnOkCancelButtonClickListener);
    }

    public void setShutterButtonEnabled(boolean enabled, boolean videoShutter) {
        if (mShutterManager != null) {
            mShutterManager.setShutterButtonEnabled(enabled, videoShutter);
        }
    }

    public boolean isShutterButtonEnabled(boolean videoShutter) {
        if (mShutterManager != null) {
            return mShutterManager.isShutterButtonEnabled(videoShutter);
        }
        return false;
    }

    /**
     * Trigger video / photo shutter button's click event
     *
     * @param clickVideoButton
     *            true, click video button; false, click photo button.
     */
    public void performShutterButtonClick(boolean clickVideoButton) {
        mShutterManager.performShutterButtonClick(clickVideoButton);
    }

    public void switchShutterButtonImageResource(int imageResourceId, boolean isVideoButton) {
        mShutterManager.switchShutterButtonImageResource(imageResourceId, isVideoButton);
    }

    public void switchShutterButtonLayout(int layoutId) {
        mShutterManager.switchShutterButtonLayout(layoutId);
    }

    public void onOrientationChanged(int orientation) {
        int newOrientation = CameraUtil.roundOrientation(orientation, mOrientation);
        if (newOrientation == mOrientation) {
            return;
        }

        mOrientation = newOrientation;
        mGestureManager.onOrientationChanged(mOrientation);
        mOrientationCompensation = (mOrientation +
                CameraUtil.getDisplayRotation(mCameraActivity)) % 360;
        LogHelper.i(TAG, "[onOrientationChanged], mOrientation:" + mOrientation + ", "
                + "mOrientationCompensation:" + mOrientationCompensation);
        rotateLayers(mOrientationCompensation);
    }

    public void setPlayButtonClickListener(PlayButtonClickListener listener) {
        mPlayButtonClickListener = listener;
        mReviewManager.setOnPlayButtonClickListener(mOnPlayButtonClickListener);
    }

    public void setRetakeButtonClickListener(RetakeButtonClickListener listener) {
        mRetakeButtonClickListener = listener;
        mReviewManager.setOnRetakeButtonClickListener(mOnRetakeButtonClickListener);
    }

    public void stopShowCommonUI(boolean stop) {
        LogHelper.i(TAG, "[stopShowCommonUI], stop:" + stop);
        mStopShowCommonUi = stop;
    }

    public boolean isSettingViewShowing() {
        if (mSettingManager != null) {
            // TODO how to check setting view is showing
        }
        return false;
    }

    public void showSettingUi() {
        if (mSettingManager != null) {
            mSettingManager.show();
        }
    }

    public void hideSettingUi() {
        if (mSettingManager != null) {
            mSettingManager.collapse(true);
            mSettingManager.hide();
        }
    }

    public void showModeOptionsUi() {
        if (mModePicker != null) {
            mModePicker.show();
        }
    }

    public void hideModeOptionsUi() {
        if (mModePicker != null) {
            mModePicker.hide();
        }
    }

    public void showPickerManagerUi() {
        if (mPickerManager != null) {
            mPickerManager.show();
            mPickerManager.setEnable(true);
        }
    }

    public void hidePickerManagerUi() {
        if (mPickerManager != null) {
            mPickerManager.hide();
            mPickerManager.setEnable(false);
        }
    }

    /**
     * /**
     * This method used to trigger camera switch.
     * @param force If it is true, force do camera switch.
     */
    public void performCameraPickerBtnClick(boolean force) {
        LogHelper.i(TAG, "[performCameraPickerBtnClick], force:" + force);
        if (mPickerManager != null
                && (mPickerManager.isEnable() || force)) {
            mPickerManager.performCameraPickerBtnClick();
        }
    }
    /**
    * Show thumbnail UI.
    */
    public void showThumbnailManagerUi() {
        if (mThumbnailManager != null) {
            mThumbnailManager.show();
        }
    }
    /**
     * Enable or disable thumbnail UI.
     * @param enable True to enable thumbnail UI, false to disable thumbnail UI.
     */
    public void setThumbnailManagerEnable(boolean enable) {
        if (mThumbnailManager != null) {
            mThumbnailManager.setEnable(enable);
        }
    }
    /**
     * Force to update thumbnail, Like default storage changed.
     */
    public void forceUpdateThumbnail() {
        if (mThumbnailManager != null) {
            mThumbnailManager.forceUpdate();
        }
    }
    /**
     * Hide thumbnail UI.
     */
    public void hideThumbnailManagerUi() {
        if (mThumbnailManager != null) {
            mThumbnailManager.hide();
        }
    }
    /**
     * Show indicator UI.
     */
    public void showIndicatorManagerUi() {
        if (mIndicatorManager != null) {
            mIndicatorManager.show();
        }
    }
    /**
     * Hide indicator UI.
     */
    public void hideIndicatorManagerUi() {
        if (mIndicatorManager != null) {
            mIndicatorManager.hide();
        }
    }

    public void showInfo(final String text) {
        showInfo(text, MSG_DEFAULT_SHOW_ONSCREEN_VIEW_DURATION/** default show time 3000ms**/);
    }

    public void showInfo(final CharSequence text, int showMs) {
        LogHelper.i(TAG, "[showInfo], text:" + text + ", showMs:" + showMs);
        mMainHandler.removeMessages(MSG_SHOW_INFO_UI);
        // if remaining manager is showing, show info after 1000ms
        // else show info directly
        if (mRemainingManager.isShowing()) {
            Message msg = mMainHandler.obtainMessage(MSG_SHOW_INFO_UI, showMs, 0, text);
            mMainHandler.sendMessageDelayed(msg, 1000);
        } else {
            doShowInfo(text, showMs);
        }
    }

    /**
     * Dismiss info. If parameter onlyDismissInfo is true, only need to dismiss info view,
     * otherwise, besides of dismissing info view, other views needs to show if it is possible.
     *
     * @param onlyDismiss
     */
    public void dismissInfo(boolean onlyDismissInfo) {
        if (mInfoManager != null) {
            mMainHandler.removeMessages(MSG_SHOW_INFO_UI);
            mMainHandler.removeMessages(MSG_HIDE_INFO_UI);
            doDismissInfo(onlyDismissInfo);
        }
    }

    public void notifyPreferenceReady() {
        LogHelper.i(TAG, "[notifyPreferenceReady]...");
        if (!mStopShowCommonUi && !mReviewManager.isShowing()) {
            // avoid to ui will be shown when switch camera during recording,
            // like pip recording.
            mShutterManager.show();
            mSettingManager.show();
            mPickerManager.notifyPreferenceReady();
            mPickerManager.show();
            mModePicker.show();
            mRemainingManager.show();
            mInfoManager.hide();
            mMainHandler.sendEmptyMessageDelayed(MSG_SHOW_INDICATOR_UI,
                    DELAY_MSG_SHOW_INDICATOR_UI);
        }
        setAllCommonViewEnable(true);
        rotateLayers(mOrientationCompensation);
    }

    public void notifyMediaSaved(Uri uri) {
        if (mThumbnailManager != null) {
            mThumbnailManager.notifyFileSaved(uri);
        }
    }

    /**
     * Show left counts of image can be taken.
     *
     * @param bytePerCount
     *            The bytes one image occupy.
     * @param showAlways
     *            If this is true, always show left counts of image can be taken. If this is false,
     *            only show left counts when counts less than 100L.
     */
    public void showLeftCounts(final int bytePerCount, final boolean showAlways) {
        LogHelper.i(TAG, "[showLeftCounts], bytePerCount:"
                + bytePerCount + ", showAlways:" + showAlways);
        mCameraActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (mRemainingManager != null) {
                    mMainHandler.removeMessages(MSG_SHOW_INDICATOR_UI);
                    mIndicatorManager.hide();
                    mInfoManager.hide();
                    mRemainingManager.showLeftCounts(bytePerCount, showAlways);
                    mMainHandler.sendEmptyMessageDelayed(MSG_SHOW_INDICATOR_UI,
                            DELAY_MSG_SHOW_INDICATOR_UI);
                }
            }
        });
    }

    /**
     * Show left times can be recorded
     *
     * @param bytePerMs
     *            The bytes of one millisecond recording
     */
    public void showLeftTime(long bytePerMs) {
        LogHelper.i(TAG, "[showLeftTime], bytePerMs:" + bytePerMs);
        if (mRemainingManager != null) {
            mMainHandler.removeMessages(MSG_SHOW_INDICATOR_UI);
            mRemainingManager.showLeftTime(bytePerMs);
            mMainHandler
                    .sendEmptyMessageDelayed(MSG_SHOW_INDICATOR_UI, DELAY_MSG_SHOW_INDICATOR_UI);
        }
    }

    public void updateAsdDetectedScene(final String scene) {
        mIndicatorManager.updateAsdDetectedScene(scene);
        boolean needShowHdrGuide = "hdr".equals(scene) || "backlight-portrait".equals(scene);
        if (needShowHdrGuide) {
            // When hdr scene is detected by asd, it should show toast to notify
            // user this scene is better to capture using HDR and show hdr icon for
            // user to open HDR.
            showHint(mCameraActivity.getString(R.string.asd_hdr_guide));
            mPickerManager.forceEnablePickerButton(SettingKeys.KEY_HDR);
        } else {
            hideHint();
            mPickerManager.forceDisablePickerButton(SettingKeys.KEY_HDR);
        }
    }

    /**
     * Show the shutter button view.
     */
    public void showShutterButtonView() {
        mShutterManager.show();
    }

    /**
     * Hide the shutter button view,such as Cross mount
     * scan view is showing,need hide the view.
     */
    public void hideShutterButtonView() {
        mShutterManager.hide();
    }

    /**
     * Show saving dialog.
     * @param msg
     *            The content dialog show.
     */
    public void showSavingProgress(String msg) {
        if (mRotateProgress != null) {
            mRotateProgress.showProgress(msg);
        }
    }

    /**
     * Dismiss saving dialog
     */
    public void dismissSavingProgress() {
        if (mRotateProgress != null) {
            mRotateProgress.hide();
        }
    }

    /**
     * Set review image to ReviewManager and show.
     *
     * @param bitmap
     */
    public void showReviewManager(Bitmap bitmap) {
        if (mReviewManager != null) {
            mReviewManager.setReviewImage(bitmap);
        }
        if (mPickerManager != null) {
            mPickerManager.hide();
        }

        if (mSettingManager != null) {
            mSettingManager.hide();
        }
        // Don't show face detection and focus box in review UI.
        View fdView = mCameraActivity.findViewById(R.id.face_detection_view);
        if (fdView != null) {
            fdView.setVisibility(View.GONE);
        }
        View focusView = mCameraActivity.findViewById(R.id.focus_indicator_rotate_layout);
        if (focusView != null) {
            focusView.setVisibility(View.GONE);
        }
    }
    /**
     * show review view for 3th ap use camera.
     * @param jpagData image data
     * @param height image size
     */
    public void showReviewManager(byte[] jpagData, int height) {
        DecodeTask decodeTask = new DecodeTask(jpagData, false, height);
        decodeTask.execute();
    }

    public void hideReviewManager() {
        if (mReviewManager != null) {
            mReviewManager.hide();
        }
        if (mPickerManager != null) {
            mPickerManager.show();
        }

        if (mSettingManager != null) {
            mSettingManager.show();
        }
        // Recover face detection and focus box when hide review UI.
        View fdView = mCameraActivity.findViewById(R.id.face_detection_view);
        if (fdView != null) {
            fdView.setVisibility(View.VISIBLE);
        }
        View focusView = mCameraActivity.findViewById(R.id.focus_indicator_rotate_layout);
        if (focusView != null) {
            focusView.setVisibility(View.VISIBLE);
        }
    }

    /**
     * Show hint to notify user.
     * @param message The message needed to show.
     */
    public void showHint(String message) {
        mOnScreenHint.showHint(message);
    }

    /**
     * Hide hint.
     */
    public void hideHint() {
        mOnScreenHint.hideHint();
    }

    private void initializeViewGroup() {
        mViewLayerBottom = (ViewGroup) mCameraActivity.findViewById(R.id.view_layer_bottom);
        mViewLayerNormal = (ViewGroup) mCameraActivity.findViewById(R.id.view_layer_normal);
        mViewLayerTop = (ViewGroup) mCameraActivity.findViewById(R.id.view_layer_top);
        mViewLayerShutter = (ViewGroup) mCameraActivity.findViewById(R.id.view_layer_shutter);
        mViewLayerSetting = (ViewGroup) mCameraActivity.findViewById(R.id.view_layer_setting);
        mViewLayerOverlay = (ViewGroup) mCameraActivity.findViewById(R.id.view_layer_overlay);
        mModuleUI = (FrameLayout) mCameraActivity.findViewById(R.id.view_layer_module);
    }

    private void initializeCommonUIManagers() {
        initializeViewGroup();

        mShutterManager = new ShutterManager(mCameraActivity, mViewLayerShutter);

        mSettingManager = new SettingManager(mCameraActivity, mViewLayerSetting,
                mPreferenceManager);
        mSettingManager.setSettingChangedListener(mSettingChangedListener);
        mSettingManager.setSettingStatusListener(mOnSettingStatusListener);

        mPickerManager = new PickerManager(mCameraActivity, mViewLayerNormal, mPreferenceManager);
        mPickerManager.setOnPickedListener(mOnPickedListener);

        mModePicker = new ModePicker(mCameraActivity, mViewLayerNormal, mPreferenceManager);
        mModePicker.setOnModeChangedListener(mOnModeChangedListener);

        mIndicatorManager = new IndicatorManager(mCameraActivity, mViewLayerNormal,
                mPreferenceManager);

        mRemainingManager = new RemainingManager(mAppController, mCameraActivity, mViewLayerNormal,
                mPreferenceManager);

        mThumbnailManager = new ThumbnailManager(mAppController,
                mCameraActivity, mViewLayerNormal, mIsSecureCamera);
        mThumbnailManager.setOnThumbnailClickListener(mOnThumbnailClickListener);
        mThumbnailManager.show();

        mReviewManager = new ReviewManager(mCameraActivity, mViewLayerBottom);

        mRotateDialog = new RotateDialog(mCameraActivity, mViewLayerOverlay);
        mRotateProgress = new RotateProgress(mCameraActivity, mViewLayerOverlay);
        mInfoManager = new InfoManager(mCameraActivity, mViewLayerNormal);

        mOnScreenHint = new OnScreenHint(mCameraActivity, mViewLayerNormal);

        rotateLayers(mOrientation);
    }

    private void rotateLayers(int orientation) {
        // set orientation as tag, so its child views can get orientation from
        // this tag.
        mViewLayerBottom.setTag(orientation);
        mViewLayerNormal.setTag(orientation);
        mViewLayerTop.setTag(orientation);
        mViewLayerShutter.setTag(orientation);
        mViewLayerSetting.setTag(orientation);
        mViewLayerOverlay.setTag(orientation);

        CameraUtil.setOrientation(mViewLayerBottom, orientation, true);
        CameraUtil.setOrientation(mViewLayerNormal, orientation, true);
        CameraUtil.setOrientation(mViewLayerTop, orientation, true);
        CameraUtil.setOrientation(mViewLayerShutter, orientation, true);
        CameraUtil.setOrientation(mViewLayerSetting, orientation, true);
        CameraUtil.setOrientation(mViewLayerOverlay, orientation, true);
    }

    /**
     * When the PreviewStatusListener changes, listeners need to be set on the following app ui
     * elements:
     */
    private void onPreviewListenerChanged() {
        OnGestureListener gestureListener = mPreviewStatusListener.getGestureListener();
        OnPreviewTouchedListener touchListener = mPreviewStatusListener.getTouchListener();
        if (mGestureManager != null) {
            mGestureManager.setPreviewGestureListener(gestureListener);
            mGestureManager.registerPreviewTouchListener(touchListener);
        }
    }

    private final class MainHandler extends Handler {

        public MainHandler(Looper mainLooper) {
            super(mainLooper);
        }

        @Override
        public void handleMessage(Message msg) {
            LogHelper.i(TAG, "msg id=" + msg.what);
            switch (msg.what) {
                case MSG_SHOW_INDICATOR_UI:
                    doShowIndicator();
                    break;

                case MSG_SHOW_INFO_UI:
                    doShowInfo((CharSequence) msg.obj, msg.arg1);
                    break;

                case MSG_SHOW_REMAINING_UI:
                    break;

                case MSG_HIDE_INFO_UI:
                    doDismissInfo(false);
                    break;
                default:
                    break;
            }
        }
    }

    private void doShowIndicator() {
        if (mRemainingManager != null) {
            mRemainingManager.hide();
        }

        if (mIndicatorManager != null && !mStopShowCommonUi) {
            mIndicatorManager.show();
        }
    }

    private void doShowInfo(CharSequence text, int showViewMs) {
        // hide remaining manager, picker manager, indicator manager
        mRemainingManager.hide();
        mPickerManager.hide();
        mIndicatorManager.hide();
        // show info manager
        mInfoManager.showText(text);
        // hide info manager after showViewMs(ms)
        if (showViewMs > 0) {
            mMainHandler.sendMessageDelayed(mMainHandler.obtainMessage(MSG_HIDE_INFO_UI),
                    showViewMs);
        }
    }

    /**
     * Dismiss info. If parameter onlyDimiss is true, only need to dismiss info view, otherwise,
     * besides of dismissing info view, other views needs to show if it is possible.
     *
     * @param onlyDismiss
     */
    private void doDismissInfo(boolean onlyDismissInfo) {
        mInfoManager.hide();

        if (!onlyDismissInfo && !mStopShowCommonUi) {
            mPickerManager.show();
            mIndicatorManager.show();
        }

    }

    // ********* add for mms or 3th ap user camera take picture
    private class DecodeTask extends AsyncTask<Void, Void, Bitmap> {
        private final byte[] mData;
        private final boolean mMirror;
        private final int mHeight;

        public DecodeTask(byte[] data, boolean mirror, int height) {
            mData = data;
            mMirror = mirror;
            mHeight = height;
        }

        @Override
        protected Bitmap doInBackground(Void... params) {
            // decode image in background
            final BitmapFactory.Options opts = new BitmapFactory.Options();
            if (mHeight > PICTURE_SIZE_4M_HEIGHT) {
                opts.inSampleSize = DOWN_SAMPLE_FACTOR;
            }
            Bitmap bitmap = BitmapFactory.decodeByteArray(mData, 0, mData.length, opts);
            int exifOrientation = Exif.getOrientation(mData);
            if (exifOrientation != 0 || mMirror) {
                Matrix m = new Matrix();
                if (mMirror) {
                    // flip horzontally
                    m.setScale(-1f, 1f);
                }
                m.preRotate(exifOrientation);
                return Bitmap.createBitmap(bitmap, 0, 0, bitmap.getWidth(), bitmap.getHeight(), m,
                        false);

            }
            return bitmap;
        }

        @Override
        protected void onPostExecute(Bitmap bitmap) {
            showReviewManager(bitmap);
        }
    }
}
