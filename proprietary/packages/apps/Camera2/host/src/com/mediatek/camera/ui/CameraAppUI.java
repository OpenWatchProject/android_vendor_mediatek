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

package com.mediatek.camera.ui;

import android.app.FragmentManager;
import android.app.FragmentTransaction;
import android.content.res.Configuration;
import android.graphics.Bitmap;
import android.graphics.Point;
import android.graphics.Rect;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.provider.MediaStore;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.OrientationEventListener;
import android.view.View;
import android.view.ViewGroup;
import android.widget.FrameLayout;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.mediatek.camera.R;
import com.mediatek.camera.common.IAppUi;
import com.mediatek.camera.common.IAppUiListener.ISurfaceStatusListener;
import com.mediatek.camera.common.IAppUiListener.OnGestureListener;
import com.mediatek.camera.common.IAppUiListener.OnModeChangeListener;
import com.mediatek.camera.common.IAppUiListener.OnPreviewAreaChangedListener;
import com.mediatek.camera.common.IAppUiListener.OnPreviewTouchedListener;
import com.mediatek.camera.common.IAppUiListener.OnShutterButtonListener;
import com.mediatek.camera.common.IAppUiListener.OnThumbnailClickedListener;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.mode.IReviewUI;
import com.mediatek.camera.common.mode.photo.intent.IIntentPhotoUi;
import com.mediatek.camera.common.mode.video.videoui.IVideoUI;
import com.mediatek.camera.common.setting.ICameraSettingView;
import com.mediatek.camera.common.utils.CameraUtil;
import com.mediatek.camera.common.widget.PreviewFrameLayout;
import com.mediatek.camera.common.widget.RotateLayout;
import com.mediatek.camera.gesture.GestureManager;
import com.mediatek.camera.ui.modepicker.ModePickerManager;
import com.mediatek.camera.ui.modepicker.ModeProvider;
import com.mediatek.camera.ui.photo.IntentPhotoUi;
import com.mediatek.camera.ui.preview.PreviewManager;
import com.mediatek.camera.ui.shutter.ShutterButtonManager;
import com.mediatek.camera.ui.video.VideoUI;

import java.util.ArrayList;
import java.util.List;

/**
 * CameraAppUI centralizes control of views shared across modules. Whereas module
 * specific views will be handled in each Module UI. For example, we can now
 * bring the flash animation and capture animation up from each module to app
 * level, as these animations are largely the same for all modules.
 *
 * This class also serves to disambiguate touch events. It recognizes all the
 * swipe gestures that happen on the preview by attaching a touch listener to
 * a full-screen view on top of preview TextureView. Since CameraAppUI has knowledge
 * of how swipe from each direction should be handled, it can then redirect these
 * events to appropriate recipient views.
 */
public class CameraAppUI implements IAppUi {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(CameraAppUI.class.getSimpleName());
    private final IApp mApp;

    private GestureManager mGestureManager;
    private ShutterButtonManager mShutterManager;
    private ThumbnailViewManager mThumbnailViewManager;
    private PreviewManager mPreviewManager;
    private ModePickerManager mModePickerManager;
    private QuickSwitcherManager mQuickSwitcherManager;
    private IndicatorViewManager mIndicatorViewManager;
    private SettingFragment mSettingFragment;
    private EffectViewManager mEffectViewManager;
    private OnScreenHintManager mOnScreenHintManager;
    private AnimationManager mAnimationManager;

    private final List<IViewManager> mViewManagers;

    private OnModeChangeListener mModeChangeListener;

    private ViewGroup mSavingDialog;

    private String mCurrentModeName;
    private String mCurrentCameraId = "0";
    private String mCurrentModeType;

    private ModeProvider mModeProvider;
    private Handler mConfigUIHandler = new ConfigUIHandler();
    private static final int APPLY_ALL_UI_VISIBILITY = 0;
    private static final int APPLY_ALL_UI_ENABLED = 1;
    private static final int SET_UI_VISIBILITY = 2;
    private static final int SET_UI_ENABLED = 3;

    private final OnOrientationChangeListenerImpl mOrientationChangeListener;
    /**
     * Constructor of cameraAppUi.
     * @param app The {@link IApp} implementer.
     */
    public CameraAppUI(IApp app) {
        mApp = app;
        mOrientationChangeListener = new OnOrientationChangeListenerImpl();
        mViewManagers = new ArrayList<>();
    }
    /**
     * Called when activity's onCreate() is invoked.
     */
    public void onCreate() {

        ViewGroup rootView = (ViewGroup) mApp.getActivity()
                .findViewById(R.id.app_ui_root);

        ViewGroup parentView = (ViewGroup) mApp.getActivity().getLayoutInflater()
                .inflate(R.layout.camera_ui_root, rootView, true);

        View appUI = parentView.findViewById(R.id.camera_ui_root);
        if (CameraUtil.isHasNavigationBar(mApp.getActivity())) {
            //get navigation bar height.
            int navigationBarHeight = CameraUtil.getNavigationBarHeight(mApp.getActivity());
            //set root view bottom margin to let the UI above the navigation bar.
            FrameLayout.LayoutParams params =
                    (FrameLayout.LayoutParams) appUI.getLayoutParams();
            if (CameraUtil.isTablet()) {
                int displayRotation = CameraUtil.getDisplayRotation(mApp.getActivity());
               LogHelper.d(TAG, " onCreate displayRotation  " + displayRotation);
                if (displayRotation == 90 || displayRotation == 270) {
                    params.leftMargin += navigationBarHeight;
                    appUI.setLayoutParams(params);
                } else {
                    params.bottomMargin += navigationBarHeight;
                    appUI.setLayoutParams(params);
                }
            } else {
                params.bottomMargin += navigationBarHeight;
                appUI.setLayoutParams(params);
            }
        }
        mModeProvider = new ModeProvider(mApp.getActivity());
        String action = mApp.getActivity().getIntent().getAction();
        mGestureManager = new GestureManager(mApp.getActivity());
        mAnimationManager = new AnimationManager(mApp, this);

        mShutterManager = new ShutterButtonManager(mApp, parentView);
        mShutterManager.setVisibility(View.VISIBLE);
        mShutterManager.setOnShutterChangedListener(new OnShutterChangeListenerImpl());
        mViewManagers.add(mShutterManager);

        if (!(MediaStore.ACTION_IMAGE_CAPTURE.equals(action)
                || MediaStore.ACTION_VIDEO_CAPTURE.equals(action))) {
            mThumbnailViewManager = new ThumbnailViewManager(mApp, parentView);
            mViewManagers.add(mThumbnailViewManager);
            mThumbnailViewManager.setVisibility(View.VISIBLE);
        }

        mPreviewManager = new PreviewManager(mApp);
        //Set gesture listener to receive touch event.
        mPreviewManager.setOnTouchListener(new OnTouchListenerImpl());

        mModePickerManager = new ModePickerManager(mApp, parentView);
        mModePickerManager.setSettingClickedListener(new OnSettingClickedListenerImpl());
        mModePickerManager.setModeChangeListener(new OnModeChangedListenerImpl());
        mModePickerManager.setVisibility(View.VISIBLE);
        mViewManagers.add(mModePickerManager);

        mQuickSwitcherManager = new QuickSwitcherManager(mApp, parentView);
        mQuickSwitcherManager.setVisibility(View.VISIBLE);
        mViewManagers.add(mQuickSwitcherManager);

        mIndicatorViewManager = new IndicatorViewManager(mApp, parentView);
        mIndicatorViewManager.setVisibility(View.VISIBLE);
        mViewManagers.add(mIndicatorViewManager);

        mSettingFragment = new SettingFragment();
        mSettingFragment.setStateListener(new SettingStateListener());
        layoutSettingUI();

        mEffectViewManager = new EffectViewManager(mApp, parentView);
        mEffectViewManager.setVisibility(View.VISIBLE);
        mViewManagers.add(mEffectViewManager);

        mOnScreenHintManager = new OnScreenHintManager(mApp, parentView);
        //call each manager's onCreate()
        for (int count = 0; count < mViewManagers.size(); count ++) {
            mViewManagers.get(count).onCreate();
        }
        mApp.registerOnOrientationChangeListener(mOrientationChangeListener);
        // [Add for CCT tool] Receive keycode and enable/disable ZSD @{
        mApp.registerKeyEventListener(getKeyEventListener(), IApp.DEFAULT_PRIORITY - 1);
        // @}
    }

    /**
     * Called when activity's onResume() is invoked.
     */
    public void onResume() {
        RotateLayout root = (RotateLayout) mApp.getActivity().findViewById(R.id.app_ui);
        Configuration newConfig = mApp.getActivity().getResources().getConfiguration();
        hideAlertDialog();
        LogHelper.d(TAG, "onResume orientation = " + newConfig.orientation);
        if (root != null) {
            if (newConfig.orientation == Configuration.ORIENTATION_PORTRAIT) {
                root.setOrientation(0, false);
            } else if (newConfig.orientation == Configuration.ORIENTATION_LANDSCAPE) {
                root.setOrientation(90, false);
            }
        }
        //call each manager's onResume()
        for (int count = 0; count < mViewManagers.size(); count ++) {
            mViewManagers.get(count).onResume();
        }
    }

    /**
     * Called when activity's onPause() is invoked.
     */
    public void onPause() {
        //call each manager's onPause()
        for (int count = 0; count < mViewManagers.size(); count ++) {
            mViewManagers.get(count).onPause();
        }
        hideAlertDialog();
        hideSetting();
        mPreviewManager.onPause();
    }

    /**
     * Called when activity's onDestroy() is invoked.
     */
    public void onDestroy() {
        //call each manager's onDestroy()
        for (int count = 0; count < mViewManagers.size(); count ++) {
            mViewManagers.get(count).onDestroy();
        }
        mApp.unregisterOnOrientationChangeListener(mOrientationChangeListener);
        // [Add for CCT tool] Receive keycode and enable/disable ZSD @{
        mApp.unRegisterKeyEventListener(getKeyEventListener());
        // @}
    }

    /**
     * Called by the system when the device configuration changes while your
     * activity is running.
     * @param newConfig The new device configuration.
     */
    public void onConfigurationChanged(Configuration newConfig) {
    }

    /**
     * Update thumbnailView, when the bitmap finished update, is will be recycled
     * immediately, do not use the bitmap again!
     * @param bitmap
     *            the bitmap matched with the picture or video, such as
     *            orientation, content. suggest thumbnail view size.
     */
    public void updateThumbnail(final Bitmap bitmap) {
        if (mThumbnailViewManager != null) {
            mApp.getActivity().runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    mThumbnailViewManager.updateThumbnail(bitmap);
                    if (bitmap != null && !bitmap.isRecycled()) {
                        bitmap.recycle();
                    }
                }
            });
        }
    }

    /**
     * get the width of thumbnail view.
     * @return the min value of width and height of thumbnail view.
     */
    public int getThumbnailViewWidth() {
        if (mThumbnailViewManager != null) {
            return mThumbnailViewManager.getThumbnailViewWidth();
        } else {
            return 0;
        }
    }

    @Override
    public void registerQuickIconDone() {
        mApp.getActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mQuickSwitcherManager.registerQuickIconDone();
            }
        });
    }

    @Override
    public void registerIndicatorDone() {
        mApp.getActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mIndicatorViewManager.registerQuickIconDone();
            }
        });
    }

    @Override
    public void registerMode(List<ModeItem> items) {
        ModeItem item = null;
        mModeProvider.clearAllModes();
        for (int i = 0; i < items.size(); i++) {
            item = items.get(i);
            mModeProvider.registerMode(item);
            if (item.mType.equals("Picture")) {
                mShutterManager.registerShutterButton(mApp.getActivity().getResources()
                        .getDrawable(
                                R.drawable.ic_shutter_photo), "Picture", 0);
            } else if (item.mType.equals("Video")) {
                mShutterManager.registerShutterButton(mApp.getActivity().getResources()
                        .getDrawable(
                                R.drawable.ic_shutter_video), "Video", 1);
            }
        }
        mModePickerManager.registerModeProvider(mModeProvider);

        mApp.getActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mShutterManager.registerDone();
            }
        });
    }

    /**
     * Notice: This is used for mode manager to update current mode, do not
     * use it in any other place.
     */
    @Override
    public void updateCurrentMode(final String modeEntry) {
        LogHelper.d(TAG, "updateCurrentMode mode = " + modeEntry);


        if (mModeProvider != null) {
            ModeItem item = mModeProvider.getMode(modeEntry);
            if (item == null) {
                return;
            }
            if (item.mModeName.equals(mCurrentModeName)) {
                return;
            }
            mCurrentModeName = item.mModeName;
            mCurrentModeType = item.mType;
            final String[] supportTypes;
            supportTypes = mModeProvider.getModeSupportTypes(mCurrentModeName,
                    mCurrentCameraId);
            mModePickerManager.updateCurrentModeItem(item);
            mApp.getActivity().runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    mShutterManager.updateModeSupportType(mCurrentModeType, supportTypes);
                }
            });
        }
    }

    @Override
    public void setPreviewSize(final int width, final int height,
                               final ISurfaceStatusListener listener) {
        mApp.getActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mPreviewManager.updatePreviewSize(width, height, listener);
            }
        });
    }

    /**
     * Called when the activity has detected the user's press of the back
     * key.  The default implementation simply finishes the current activity,
     * but you can override this to do whatever you want.
     * @return Return <code>true</code> to prevent this event from being propagated
     * further, or <code>false</code> to indicate that you have not handled
     * this event and it should continue to be propagated.
     */
    public boolean onBackPressed() {
        return false;
    }

    @Override
    public void showScreenHint(final HintInfo info) {
        mApp.getActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mOnScreenHintManager.showScreenHint(info);
            }
        });
    }

    @Override
    public void hideScreenHint(final HintInfo info) {
        mApp.getActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mOnScreenHintManager.hideScreenHint(info);
            }
        });

    }

    @Override
    public ViewGroup getModeRootView() {
        return (ViewGroup) mApp.getActivity()
                .findViewById(R.id.feature_root);
    }

    @Override
    public View getShutterRootView() {
        if (mShutterManager != null) {
            return mShutterManager.getShutterRootView();
        }
        return null;
    }

    @Override
    public PreviewFrameLayout getPreviewFrameLayout() {
        return mPreviewManager.getPreviewFrameLayout();
    }

    @Override
    public void onPreviewStarted(final String previewCameraId) {
        LogHelper.d(TAG, "onPreviewStarted previewCameraId = " + previewCameraId);
        if (previewCameraId == null) {
            return;
        }
        synchronized (mCurrentCameraId) {
            mCurrentCameraId = previewCameraId;
        }

        mModePickerManager.onPreviewStarted(previewCameraId);
        mApp.getActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                removeTopSurface();
            }
        });
    }

    @Override
    public void onCameraSelected(final String cameraId) {
        synchronized (mCurrentCameraId) {
            mCurrentCameraId = cameraId;
        }
        mModePickerManager.onPreviewStarted(cameraId);

    }

    @Override
    public IVideoUI getVideoUi() {
        return new VideoUI(mApp, getModeRootView());
    }

    @Override
    public IReviewUI getReviewUI() {
        ViewGroup appUI = (ViewGroup) mApp.getActivity().findViewById(R.id.app_ui);
        ViewGroup reviewRoot = (ViewGroup) appUI.getChildAt(0);
        return new ReviewUI(mApp, reviewRoot);
    }

    @Override
    public IIntentPhotoUi getPhotoUi() {
        return new IntentPhotoUi(mApp.getActivity(), getModeRootView(), this);
    }

    @Override
    public void animationStart(final AnimationType type, final AnimationData data) {
        mApp.getActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mAnimationManager.animationStart(type, data);
            }
        });
    }

    @Override
    public void animationEnd(final AnimationType type) {
        mApp.getActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mAnimationManager.animationEnd(type);
            }
        });
    }

    @Override
    public void setUIVisibility(int module, int visibility) {
        if (!isMainThread()) {
            LogHelper.d(TAG, "setUIVisibility + module " + module + " visibility " + visibility);
            Message msg = Message.obtain();
            msg.arg1 = module;
            msg.arg2 = visibility;
            msg.what = SET_UI_VISIBILITY;
            mConfigUIHandler.sendMessage(msg);
            LogHelper.d(TAG, "setUIVisibility - ");
        } else {
            setUIVisibilityImmediately(module, visibility);
        }

    }
    @Override
    public void setUIEnabled(int module, boolean enabled) {
        if (!isMainThread()) {
            LogHelper.d(TAG, "setUIEnabled + module " + module + " enabled " + enabled);
            Message msg = Message.obtain();
            msg.arg1 = module;
            msg.arg2 = enabled ? 1 : 0;
            msg.what = SET_UI_ENABLED;
            mConfigUIHandler.sendMessage(msg);
            LogHelper.d(TAG, "setUIEnabled - ");
        } else {
            setUIEnabledImmediately(module, enabled);
        }
    }


    @Override
    public void applyAllUIVisibility(final int visibility) {
        if (!isMainThread()) {
            LogHelper.d(TAG, "applyAllUIVisibility + visibility " + visibility);
            Message msg = Message.obtain();
            msg.arg1 = visibility;
            msg.what = APPLY_ALL_UI_VISIBILITY;
            mConfigUIHandler.sendMessage(msg);
            LogHelper.d(TAG, "applyAllUIVisibility -");
        } else {
            applyAllUIVisibilityImmediately(visibility);
        }
    }

    @Override
    public void applyAllUIEnabled(final boolean enabled) {
        if (!isMainThread()) {
            LogHelper.d(TAG, "applyAllUIEnabled + enabled " + enabled);
            Message msg = Message.obtain();
            msg.arg1 = enabled ? 1 : 0;
            msg.what = APPLY_ALL_UI_ENABLED;
            mConfigUIHandler.sendMessage(msg);
            LogHelper.d(TAG, "applyAllUIEnabled -");
        } else {
            applyAllUIEnabledImmediately(enabled);
        }
    }

    private void setUIVisibilityImmediately(int module, int visibility) {
        LogHelper.d(TAG, "setUIVisibilityImmediately + module " + module
                                                + " visibility " + visibility);
        configUIVisibility(module, visibility);
    }

    private void setUIEnabledImmediately(int module, boolean enabled) {
        LogHelper.d(TAG, "setUIEnabledImmediately + module " + module + " enabled " + enabled);
        configUIEnabled(module, enabled);
    }

    @Override
    public void updateBrightnessBackGround(boolean visible) {
        LogHelper.d(TAG, "setBackgroundColor visible = " + visible);
        mApp.getActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                 View rootView = mApp.getActivity()
                         .findViewById(R.id.brightness_view);
                 if (visible) {
                     rootView.setVisibility(View.VISIBLE);
                 } else {
                     rootView.setVisibility(View.GONE);
                 }
            }
        });
    }
    private void applyAllUIVisibilityImmediately(int visibility) {
        LogHelper.d(TAG, "applyAllUIVisibilityImmediately + visibility " + visibility);
        mConfigUIHandler.removeMessages(APPLY_ALL_UI_VISIBILITY);
        for (int count = 0; count < mViewManagers.size(); count++) {
            mViewManagers.get(count).setVisibility(visibility);
        }
        getPreviewFrameLayout().setVisibility(visibility);
        mOnScreenHintManager.setVisibility(visibility);
        if (visibility == View.GONE) {
            mQuickSwitcherManager.hideQuickSwitcherImmediately();
        }
    }

    private void applyAllUIEnabledImmediately(boolean enabled) {
        LogHelper.d(TAG, "applyAllUIEnabledImmediately + enabled " + enabled);
        mConfigUIHandler.removeMessages(APPLY_ALL_UI_ENABLED);
        for (int count = 0; count < mViewManagers.size(); count++) {
            mViewManagers.get(count).setEnabled(enabled);
        }
    }

    private boolean isMainThread() {
        return Looper.myLooper() == Looper.getMainLooper();
    }

    @Override
    public void clearPreviewStatusListener(final ISurfaceStatusListener listener) {
        mApp.getActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mPreviewManager.clearPreviewStatusListener(listener);
            }
        });
    }

    @Override
    public void registerOnPreviewTouchedListener(OnPreviewTouchedListener listener) {

    }

    @Override
    public void unregisterOnPreviewTouchedListener(OnPreviewTouchedListener listener) {

    }

    @Override
    public void registerOnPreviewAreaChangedListener(OnPreviewAreaChangedListener listener) {
        mPreviewManager.registerPreviewAreaChangedListener(listener);
    }

    @Override
    public void unregisterOnPreviewAreaChangedListener(OnPreviewAreaChangedListener listener) {
        mPreviewManager.unregisterPreviewAreaChangedListener(listener);
    }

    @Override
    public void registerGestureListener(OnGestureListener listener, int priority) {
        mGestureManager.registerGestureListener(listener, priority);
    }

    @Override
    public void unregisterGestureListener(OnGestureListener listener) {
        mGestureManager.unregisterGestureListener(listener);
    }

    @Override
    public void registerOnShutterButtonListener(OnShutterButtonListener listener, int priority) {
        mShutterManager.registerOnShutterButtonListener(listener, priority);
    }

    @Override
    public void unregisterOnShutterButtonListener(OnShutterButtonListener listener) {
        mShutterManager.unregisterOnShutterButtonListener(listener);
    }

    @Override
    public void setThumbnailClickedListener(OnThumbnailClickedListener listener) {
        if (mThumbnailViewManager != null) {
            mThumbnailViewManager.setThumbnailClickedListener(listener);
        }
    }

    @Override
    public void setModeChangeListener(OnModeChangeListener listener) {
        mModeChangeListener = listener;
    }

    @Override
    public void triggerModeChanged(String newMode) {
        mModePickerManager.modeChanged(newMode);
    }

    @Override
    public void triggerShutterButtonClick(int currentPriority) {
        mShutterManager.triggerShutterButtonClicked(currentPriority);
    }

    @Override
    public void triggerShutterButtonLongPressed(int currentPriority) {
        mShutterManager.triggerShutterButtonLongPressed(currentPriority);
    }

    @Override
    public void addToQuickSwitcher(View view, int priority) {
        mQuickSwitcherManager.addToQuickSwitcher(view, priority);
    }

    @Override
    public void removeFromQuickSwitcher(View view) {
        mQuickSwitcherManager.removeFromQuickSwitcher(view);
    }

    @Override
    public void addToIndicatorView(View view, int priority) {
        mApp.getActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mIndicatorViewManager.addToIndicatorView(view, priority);
            }
        });
    }

    @Override
    public void removeFromIndicatorView(View view) {
        mApp.getActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mIndicatorViewManager.removeFromIndicatorView(view);
            }
        });
    }

    @Override
    public void addSettingView(ICameraSettingView view) {
        mSettingFragment.addSettingView(view);
    }

    @Override
    public void removeSettingView(ICameraSettingView view) {
        mSettingFragment.removeSettingView(view);
    }

    @Override
    public void refreshSettingView() {
        mApp.getActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mSettingFragment.refreshSettingView();
            }
        });
    }

    @Override
    public void updateSettingIconVisibility() {
        boolean visible = mSettingFragment.hasVisibleChild();
        mModePickerManager.setSettingIconVisible(visible);
    }

    @Override
    public void showSavingDialog(String message, boolean isNeedShowProgress) {
        mApp.getActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                ViewGroup root = (ViewGroup) mApp.getActivity().getWindow().getDecorView();
                TextView text;
                if (mSavingDialog == null) {
                    mSavingDialog = (ViewGroup) mApp.getActivity().getLayoutInflater()
                            .inflate(R.layout.rotate_dialog, root, false);
                    View progress = mSavingDialog.findViewById(R.id.dialog_progress);
                    text = (TextView) mSavingDialog.findViewById(R.id.dialog_text);
                    if (isNeedShowProgress) {
                        progress.setVisibility(View.VISIBLE);
                    } else {
                        progress.setVisibility(View.GONE);
                    }
                    if (message != null) {
                        text.setText(message);
                    } else {
                        text.setText(R.string.saving_dialog_default_string);
                    }
                    root.addView(mSavingDialog);
                    int orientation = mApp.getGSensorOrientation();
                    if (orientation != OrientationEventListener.ORIENTATION_UNKNOWN) {
                        int compensation = CameraUtil.getDisplayRotation(mApp.getActivity());
                        orientation = orientation + compensation;
                        CameraUtil.rotateViewOrientation(mSavingDialog, orientation, false);
                    }
                    mSavingDialog.setVisibility(View.VISIBLE);
                } else {
                    text = (TextView) mSavingDialog.findViewById(R.id.dialog_text);
                    text.setText(message);
                }
            }
        });
    }
    @Override
    public void hideSavingDialog() {
        mApp.getActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (mSavingDialog != null) {
                    ViewGroup root = (ViewGroup) mApp.getActivity().getWindow().getDecorView();
                    mSavingDialog.setVisibility(View.GONE);
                    root.removeView(mSavingDialog);
                    mSavingDialog = null;
                }
            }
        });
    }

    @Override
    public void setEffectViewEntry(View view) {
        mEffectViewManager.setViewEntry(view);;
    }

    @Override
    public void attachEffectViewEntry() {
        mApp.getActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mEffectViewManager.attachViewEntry();
            }
        });
    }

    @Override
    public void showQuickSwitcherOption(View optionView) {
        mApp.getActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mQuickSwitcherManager.showQuickSwitcherOption(optionView);
            }
        });
    }

    @Override
    public void hideQuickSwitcherOption() {
        mApp.getActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mQuickSwitcherManager.hideQuickSwitcherOption();
            }
        });
    }

    protected void removeTopSurface() {
        mPreviewManager.removeTopSurface();
    }

    private void layoutSettingUI() {
        LinearLayout settingRootView = (LinearLayout) mApp.getActivity()
                .findViewById(R.id.setting_ui_root);

        if (CameraUtil.isHasNavigationBar(mApp.getActivity())) {
            // Get the preview height don't contain navigation bar height.
            Point size = new Point();
            mApp.getActivity().getWindowManager().getDefaultDisplay().getSize(size);
            LogHelper.d(TAG, "[layoutSettingUI], preview size don't contain navigation:" + size);
            LinearLayout settingContainer = (LinearLayout) settingRootView
                    .findViewById(R.id.setting_container);
            LinearLayout.LayoutParams containerParams =
                    (LinearLayout.LayoutParams) settingContainer.getLayoutParams();
            containerParams.height = size.y;
            settingContainer.setLayoutParams(containerParams);

            LinearLayout settingTail = (LinearLayout) settingRootView
                    .findViewById(R.id.setting_tail);
            //get navigation bar height.
            int navigationBarHeight = CameraUtil.getNavigationBarHeight(mApp.getActivity());
            LogHelper.d(TAG, "[layoutSettingUI], navigationBarHeight:" + navigationBarHeight);
            //set setting tail view height as navigation bar height.
            LinearLayout.LayoutParams params =
                    (LinearLayout.LayoutParams) settingTail.getLayoutParams();
            params.height = navigationBarHeight;
            settingTail.setLayoutParams(params);
        }
    }

    private void showSetting() {
        FragmentTransaction transaction = mApp.getActivity().getFragmentManager()
                .beginTransaction();
        transaction.addToBackStack("setting_fragment");
        transaction.replace(R.id.setting_container, mSettingFragment, "Setting")
                .commitAllowingStateLoss();
    }

    private void hideSetting() {
        mApp.getActivity().getFragmentManager().popBackStackImmediate("setting_fragment",
                FragmentManager.POP_BACK_STACK_INCLUSIVE);
    }

    private void hideAlertDialog() {
        CameraUtil.hideAlertDialog(mApp.getActivity());
    }

    /**
     * Shutter change listener implementer.
     */
    private class OnShutterChangeListenerImpl implements
                                           ShutterButtonManager.OnShutterChangeListener {

        @Override
        public void onShutterTypeChanged(String newShutterType) {
            mCurrentModeType = newShutterType;
            LogHelper.i(TAG, "onShutterTypeChanged mCurrentModeType " + mCurrentModeType);
            ModeItem item = mModeProvider.getModeEntryName(mCurrentModeName, mCurrentModeType);
            mModeChangeListener.onModeSelected(item.mClassName);
        }
    }

    /**
     * Setting state listener implementer.
     */
    private class SettingStateListener implements SettingFragment.StateListener {

        @Override
        public void onCreate() {
            View view = mApp.getActivity().findViewById(R.id.setting_ui_root);
            view.setVisibility(View.VISIBLE);

            applyAllUIVisibility(View.GONE);
            setUIEnabled(SHUTTER_BUTTON, false);
        }

        @Override
        public void onResume() {

        }

        @Override
        public void onPause() {

        }

        @Override
        public void onDestroy() {
            View view = mApp.getActivity().findViewById(R.id.setting_ui_root);
            view.setVisibility(View.GONE);

            applyAllUIVisibility(View.VISIBLE);
            setUIEnabled(SHUTTER_BUTTON, true);
        }
    }

    /**
     * Implementer of onTouch listener.
     */
    private class OnTouchListenerImpl implements View.OnTouchListener {

        @Override
        public boolean onTouch(View view, MotionEvent motionEvent) {
            if (mGestureManager != null) {
                Rect rect = new Rect();
                getShutterRootView().getHitRect(rect);
                Configuration config = mApp.getActivity().getResources().getConfiguration();
                if (config.orientation == Configuration.ORIENTATION_PORTRAIT) {
                    if (motionEvent.getRawY() > rect.top) {
                        //If the touch point is below shutter, ignore it.
                        return true;
                    }
                } else if (config.orientation == Configuration.ORIENTATION_LANDSCAPE) {
                    if (motionEvent.getRawX() > rect.top) {
                        //If the touch point is below shutter, ignore it.
                        return true;
                    }
                }
                mGestureManager.getOnTouchListener().onTouch(view, motionEvent);
            }
            return true;
        }
    }

    /**
     *  Implementer of setting button click listener.
     */
    private class OnSettingClickedListenerImpl implements View.OnClickListener {

        @Override
        public void onClick(View view) {
            if (mSettingFragment.hasVisibleChild()) {
                showSetting();
            }
        }
    }

    /**
     * The implementer of OnModeChangeListener.
     */
    private class OnModeChangedListenerImpl implements ModePickerManager.OnModeChangedListener {

        @Override
        public void onModeChanged(String modeName) {
            mCurrentModeName = modeName;
            ModeItem item = mModeProvider.getModeEntryName(mCurrentModeName, mCurrentModeType);
            mModeChangeListener.onModeSelected(item.mClassName);
            mModePickerManager.updateCurrentModeItem(item);
            String[] supportTypes =
                    mModeProvider.getModeSupportTypes(item.mModeName, mCurrentCameraId);
            mShutterManager.updateModeSupportType(mCurrentModeType, supportTypes);
            mShutterManager.updateCurrentModeShutter(item.mType, item.mShutterIcon);
        }
    }

    /**
     * Implementer of OnOrientationChangeListener.
     */
    private class OnOrientationChangeListenerImpl implements IApp.OnOrientationChangeListener {

        @Override
        public void onOrientationChanged(int orientation) {
            if (mSavingDialog != null) {
                int compensation = CameraUtil.getDisplayRotation(mApp.getActivity());
                orientation = orientation + compensation;
                CameraUtil.rotateViewOrientation(mSavingDialog, orientation, true);
            }
        }
    }

    /**
     * Handler let some task execute in main thread.
     */
    private class ConfigUIHandler extends Handler {

        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            LogHelper.d(TAG, "handleMessage what =  " + msg.what);
            switch (msg.what) {
                case APPLY_ALL_UI_VISIBILITY:
                    //call each manager's setVisibility()
                    int visibility = msg.arg1;
                    for (int count = 0; count < mViewManagers.size(); count++) {
                        mViewManagers.get(count).setVisibility(visibility);
                    }
                    getPreviewFrameLayout().setVisibility(visibility);
                    mOnScreenHintManager.setVisibility(visibility);
                    if (visibility == View.GONE) {
                        mQuickSwitcherManager.hideQuickSwitcherImmediately();
                    }
                    break;
                case APPLY_ALL_UI_ENABLED:
                    //call each manager's setEnabled()
                    boolean enabled = msg.arg1 == 1;
                    for (int count = 0; count < mViewManagers.size(); count++) {
                        mViewManagers.get(count).setEnabled(enabled);
                    }
                    break;
                case SET_UI_VISIBILITY:
                    configUIVisibility(msg.arg1, msg.arg2);
                    break;
                case SET_UI_ENABLED:
                    configUIEnabled(msg.arg1, msg.arg2 == 1);
                default:
                    break;
            }
        }
    }

    private void configUIVisibility(int module, int visibility) {
        LogHelper.d(TAG, "configUIVisibility + module " + module + " visibility " + visibility);
        switch (module) {
            case QUICK_SWITCHER:
                mQuickSwitcherManager.setVisibility(visibility);
                break;
            case MODE_SWITCHER:
                mModePickerManager.setVisibility(visibility);
                break;
            case THUMBNAIL:
                if (mThumbnailViewManager != null) {
                    mThumbnailViewManager.setVisibility(visibility);
                }
                break;
            case SHUTTER_BUTTON:
                mShutterManager.setVisibility(visibility);
                break;
            case INDICATOR:
                mIndicatorViewManager.setVisibility(visibility);
                break;
            case PREVIEW_FRAME:
                getPreviewFrameLayout().setVisibility(visibility);
                break;
            case SCREEN_HINT:
                mOnScreenHintManager.setVisibility(visibility);
            default:
                break;
        }
    }

    private void configUIEnabled(int module, boolean enabled) {
        LogHelper.d(TAG, "configUIEnabled + module " + module + " enabled " + enabled);
        switch (module) {
            case QUICK_SWITCHER:
                mQuickSwitcherManager.setEnabled(enabled);
                break;
            case MODE_SWITCHER:
                mModePickerManager.setEnabled(enabled);
                break;
            case THUMBNAIL:
                if (mThumbnailViewManager != null) {
                    mThumbnailViewManager.setEnabled(enabled);
                }
                break;
            case SHUTTER_BUTTON:
                mShutterManager.setEnabled(enabled);
                break;
            case INDICATOR:
                mIndicatorViewManager.setEnabled(enabled);
                break;
            case PREVIEW_FRAME:
                break;
            case GESTURE:
                mPreviewManager.setEnabled(enabled);
                break;
            case SHUTTER_TEXT:
                mShutterManager.setTextEnabled(enabled);
                break;
            default:
                break;
        }
    }

    private void dumpUIState(AppUIState state) {
        if (state != null) {
            StringBuilder builder = new StringBuilder();
            builder.append("mIndicatorEnabled:")
                    .append(state.mIndicatorEnabled)
                    .append(", mIndicatorVisibleState:")
                    .append(state.mIndicatorVisibleState)
                    .append(", mModeSwitcherEnabled:")
                    .append(state.mModeSwitcherEnabled)
                    .append(", mModeSwitcherVisibleState:")
                    .append(state.mModeSwitcherVisibleState)
                    .append(", mQuickSwitcherEnabled:")
                    .append(state.mQuickSwitcherEnabled)
                    .append(", mQuickSwitcherVisibleState: ")
                    .append(state.mQuickSwitcherVisibleState)
                    .append(", mShutterButtonEnabled:")
                    .append(state.mShutterButtonEnabled)
                    .append(", mShutterButtonVisibleState:")
                    .append(state.mShutterButtonVisibleState)
                    .append(", mThumbnailEnabled:")
                    .append(state.mThumbnailEnabled)
                    .append(", mThumbnailVisibleState:")
                    .append(state.mThumbnailVisibleState)
                    .append(", mPreviewFrameVisibleState:")
                    .append(state.mPreviewFrameVisibleState)
                    .toString();
            LogHelper.i(TAG, "[dumpUIState]: " + builder);
        }
    }

    // [Add for CCT tool] Receive keycode and enable/disable ZSD @{
    public IApp.KeyEventListener getKeyEventListener() {
        return new IApp.KeyEventListener() {
            @Override
            public boolean onKeyDown(int keyCode, KeyEvent event) {
                return false;
            }

            @Override
            public boolean onKeyUp(int keyCode, KeyEvent event) {
                if (!CameraUtil.isSpecialKeyCodeEnabled()) {
                    return false;
                }
                if (!CameraUtil.isNeedInitSetting(keyCode)) {
                    return false;
                }
                showSetting();
                hideSetting();
                return false;
            }
        };
    }
    // @}
}
