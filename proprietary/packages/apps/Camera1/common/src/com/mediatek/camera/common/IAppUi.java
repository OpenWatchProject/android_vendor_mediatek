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

package com.mediatek.camera.common;

import android.graphics.Bitmap;
import android.graphics.drawable.Drawable;
import android.view.View;
import android.view.ViewGroup;

import com.mediatek.camera.common.IAppUiListener.ISurfaceStatusListener;
import com.mediatek.camera.common.IAppUiListener.OnGestureListener;
import com.mediatek.camera.common.IAppUiListener.OnModeChangeListener;
import com.mediatek.camera.common.IAppUiListener.OnPreviewAreaChangedListener;
import com.mediatek.camera.common.IAppUiListener.OnPreviewTouchedListener;
import com.mediatek.camera.common.IAppUiListener.OnShutterButtonListener;
import com.mediatek.camera.common.IAppUiListener.OnThumbnailClickedListener;
import com.mediatek.camera.common.mode.IReviewUI;
import com.mediatek.camera.common.mode.photo.intent.IIntentPhotoUi;
import com.mediatek.camera.common.mode.video.videoui.IVideoUI;
import com.mediatek.camera.common.setting.ICameraSettingView;
import com.mediatek.camera.common.widget.PreviewFrameLayout;

import java.util.List;

import javax.annotation.Nonnull;

/**
 * Camera app level UI interface, define the common APIs of app UI.
 */
public interface IAppUi {

    /**
     * APPUIState is a structure for features to specify their ideal
     * common UI visible state.
     * Once constructed by a feature, this class should be treated as read only.
     * Visible state value reference:
     * {@link android.view.View.VISIBLE,android.view.View.INVISIBLE,android.view.View.GONE}
     * Enable state value is true or false:
     * True if the view module enable click and focus.
     * False if the view module disable click and focus.
     */
     class AppUIState {
        public int mQuickSwitcherVisibleState;
        public int mModeSwitcherVisibleState;
        public int mThumbnailVisibleState;
        public int mShutterButtonVisibleState;
        public int mIndicatorVisibleState;
        public int mPreviewFrameVisibleState;
        public boolean mQuickSwitcherEnabled;
        public boolean mModeSwitcherEnabled;
        public boolean mThumbnailEnabled;
        public boolean mShutterButtonEnabled;
        public boolean mIndicatorEnabled;
    }

    int QUICK_SWITCHER = 0;
    int MODE_SWITCHER = 1;
    int THUMBNAIL = 2;
    int SHUTTER_BUTTON = 3;
    int INDICATOR = 4;
    int PREVIEW_FRAME = 5;
    int GESTURE = 6;
    int SHUTTER_TEXT = 7;
    int SCREEN_HINT = 8;


    int DEFAULT_PRIORITY = Integer.MAX_VALUE;

    /**
     * A item that has a mode information for register in App UI.
     */
    class ModeItem {
        //The view that will be shown on the mode list.
        public Drawable mModeUnselectedIcon;
        public Drawable mModeSelectedIcon;
        //The shutter view of the mode.
        public Drawable mShutterIcon;
        //The priority value, the smaller the value, the higher the priority.
        // the higher priority icon will be shown in the front of the mode list.
        public int mPriority;
        //The mode type. Such as "Picture" or "Video"
        public String mType;
        //Mode name.
        public String mClassName;
        //Mode key, different modes of the same feature only has one key value.
        //For example:
        //Pip feature has two modes, one for picture, another for video, but they should have same
        //key value.
        public String mModeName;
        //Used for judge current mode support which cameras.
        //such as panorama mode just supported in back camera, so will fill {0};
        //such as face beauty support both front camera and back camera, will fill {0,1}
        public String[] mSupportedCameraIds;
    }

    /**
     * Define animation data structure, i will contain the data to do
     * the animation.
     */
    class AnimationData {
        //The preview data.
        public byte[] mData;
        //The preview data format.
        public int mFormat;
        //The preview data width.
        public int mWidth;
        //The preview data height.
        public int mHeight;
        //The preview data orientation.
        public int mOrientation;
        //The preview data is need mirror or not.
        public boolean mIsMirror;
    }

    /**
     * Define the animation type.
     */
    enum AnimationType {
        TYPE_SWITCH_CAMERA,
        TYPE_CAPTURE,
        TYPE_SWITCH_MODE
    }

    /**
     * Screen hint type.
     */
    enum HintType {
        TYPE_ALWAYS_TOP,
        TYPE_AUTO_HIDE,
        TYPE_ALWAYS_BOTTOM
    }

    /**
     * Screen hint information definition.
     */
    class HintInfo {
        public HintType mType;
        public String  mHintText;
        public Drawable mBackground;
        public int mDelayTime;
    }

    /**
     * Show screen hint.
     * @param info The hint information.
     */
    void showScreenHint(HintInfo info);

    /**
     * Hide screen hint.
     * @param info The hint information.
     */
    void hideScreenHint(HintInfo info);
    /**
     * Called indirectly from each feature in their initialization to get a view group
     * to inflate the module specific views in.
     *
     * @return a view group for modules to attach views to
     */
     ViewGroup getModeRootView();

    /**
     * Get shutter root view.
     * @return the shutter root view.
     */
    View getShutterRootView();

    /**
     * Get preview frame layout, it is a parent view for focus and face detection view.
     * @return preview frame layout.
     */
     PreviewFrameLayout getPreviewFrameLayout();

    /**
     * When preview started, notify the event to camera app UI.
     * @param cameraId current preview camera id.
     */
     void onPreviewStarted(String cameraId);

    /**
     * When switch camera started, notify the event to camera app UI.
     * @param cameraId To be previewing camera id.
     */
     void onCameraSelected(String cameraId);

    /**
     * Config UI module visibility.
     * It will post a Runnable() to UI thread looper, latency will happened when invoke the
     * function in UI thread directly.
     * @param module Selected UI module.
     *               The UI module can be {@link QUICK_SWITCHER,MODE_SWITCHER, ...}
     * @param visibility Visible state value reference:
     * {@link android.view.View.VISIBLE,android.view.View.INVISIBLE,android.view.View.GONE}
     */
    void setUIVisibility(int module, int visibility);

    /**
     * Config UI module enable state.
     * It will post a Runnable() to UI thread looper, latency will happened when invoke the
     * function in UI thread directly.
     * @param module Selected UI module.
     *               The UI module can be {@link QUICK_SWITCHER,MODE_SWITCHER, ...}
     * @param enabled enabled Enable state value is true or false:
     * True: if the view module enable click and focus.
     * False: if the view module disable click and focus.
     */
    void setUIEnabled(int module, boolean enabled);

    /**
     * Config all common ui visibility.
     * It will post a Runnable() to UI thread looper, latency will happened when invoke the
     * function in UI thread directly.
     * @param visibility Visible state value reference:
     * {@link android.view.View.VISIBLE,android.view.View.INVISIBLE,android.view.View.GONE}
     */
     void applyAllUIVisibility(int visibility);

    /**
     * Config all common ui enable state.
     * It will post a Runnable() to UI thread looper, latency will happened when invoke the
     * function in UI thread directly.
     * @param enabled Enable state value is true or false:
     * True: if the view module enable click and focus.
     * False: if the view module disable click and focus.
     */
     void applyAllUIEnabled(boolean enabled);

    /**
     * Config UI module visibility.
     * It has no latency and take effect immediately. Use it in UI thread, otherwise error will
     * be happened.
     * @param module Selected UI module.
     *               The UI module can be {@link QUICK_SWITCHER,MODE_SWITCHER, ...}
     * @param visibility Visible state value reference:
     * {@link android.view.View.VISIBLE,android.view.View.INVISIBLE,android.view.View.GONE}
     */
     void setUIVisibilityImmediately(int module, int visibility);

    /**
     * Config UI module enable state.
     * It has no latency and take effect immediately. Use it in UI thread, otherwise error will
     * be happened.
     * @param module Selected UI module.
     *               The UI module can be {@link QUICK_SWITCHER,MODE_SWITCHER, ...}
     * @param enabled enabled Enable state value is true or false:
     * True: if the view module enable click and focus.
     * False: if the view module disable click and focus.
     */
     void setUIEnabledImmediately(int module, boolean enabled);

    /**
     * Config all common ui visibility.
     * It has no latency and take effect immediately. Use it in UI thread, otherwise error will
     * be happened.
     * @param visibility Visible state value reference:
     * {@link android.view.View.VISIBLE,android.view.View.INVISIBLE,android.view.View.GONE}
     */
     void applyAllUIVisibilityImmediately(int visibility);

    /**
     * Config all common ui enable state.
     * It has no latency and take effect immediately. Use it in UI thread, otherwise error will
     * be happened.
     * @param enabled Enable state value is true or false:
     * True: if the view module enable click and focus.
     * False: if the view module disable click and focus.
     */
     void applyAllUIEnabledImmediately(boolean enabled);

    /**
     * Clear the Status previous listener that set when update preview size.
     * @param listener The listener need to be used.
     */
     void clearPreviewStatusListener(ISurfaceStatusListener listener);

    /**
     * Register preview touched listener.
     * @param listener The listener need to be registered.
     */
     void registerOnPreviewTouchedListener(OnPreviewTouchedListener listener);

    /**
     * Unregister preview touched listener.
     * @param listener The listener need to be unregistered.
     */
     void unregisterOnPreviewTouchedListener(OnPreviewTouchedListener listener);
    /**
     * Register preview area changed listener.
     * @param listener The listener need to be registered.
     */
    void registerOnPreviewAreaChangedListener(OnPreviewAreaChangedListener listener);

    /**
     * Unregister preview area changed listener.
     * @param listener The listener need to be unregistered.
     */
    void unregisterOnPreviewAreaChangedListener(OnPreviewAreaChangedListener listener);


    /**
     * Register gesture listener with a priority, the high priority listener can receiver
     * gesture info befor the low priority. high priority listener can consume one gesture
     * message by return true{@link OnGestureListener},and the low priority one can not receive
     * the message.
     * @param listener The listener need to be registered.
     * @param priority  Listener's priority, it is a int value
     *                  the smaller the value, the higher the priority.
     */
     void registerGestureListener(OnGestureListener listener, int priority);

    /**
     * Unregister gesture listener.
     * @param listener The listener need to be unregistered.
     */
     void unregisterGestureListener(OnGestureListener listener);

    /**
     * Register shutter button listener. the high priority listener can receiver
     * gesture info befor the low priority. high priority listener can consume one gesture
     * message by return true{@link OnShutterButtonListener},and the low priority one can not
     * receive the message.
     * @param listener The listener need to be registered.
     * @param priority  Listener's priority, it is a int value
     *                  the smaller the value, the higher the priority.
     */
     void registerOnShutterButtonListener(OnShutterButtonListener listener, int priority);

    /**
     * Unregister shutter button listener.
     * @param listener The listener need to be unregistered.
     */
      void unregisterOnShutterButtonListener(OnShutterButtonListener listener);

    /**
     * Set thumbnail clicked listener.
     * @param listener The listener need to be registered.
     */
     void setThumbnailClickedListener(OnThumbnailClickedListener listener);

    /**
     * Add view to quick switcher with specified priority.
     * @param view The view register to quick switcher.
     * @param priority The priority that the registered view sort order.
     */
     void addToQuickSwitcher(View view, int priority);

    /**
     * Remove view from quick switcher.
     * @param view The view removed from quick switcher.
     */
     void removeFromQuickSwitcher(View view);

    /**
     * Add view to indicator view with specified priority.
     * @param view The view register to quick switcher.
     * @param priority The priority that the registered view sort order.
     */
    void addToIndicatorView(View view, int priority);

    /**
     * Remove view from indicator view.
     * @param view The view removed from quick switcher.
     */
    void removeFromIndicatorView(View view);
    /**
     * Set mode change listener.
     *
     * @param listener mode change listener instance.
     */
     void setModeChangeListener(OnModeChangeListener listener);

    /**
     * Invoke the OnModeChangeListener to switch mode.
     * @param newMode The new mode key value.
     */
    void triggerModeChanged(String newMode);

    /**
     * Invoke the onShutterButtonCLicked listener to start a capture event.
     * @param currentPriority Trigger module shutter button listener priority,
     *                        the trigger event will be pass to the modules which shutter listener
     *                        priority is lower than currentPriority value.
     *                        The zero value will pass the click event to all listeners.
     */
    void triggerShutterButtonClick(int currentPriority);

    /**
     * update thumbnailView.
     * @param bitmap
     *            the bitmap matched with the picture or video, such as
     *            orientation, content. suggest thumbnail view size.
     */
    void updateThumbnail(Bitmap bitmap);

    /**
     * get the width of thumbnail view.
     * @return the min value of width and height of thumbnail view.
     */
    int getThumbnailViewWidth();

    /**
     * Register quick switcher icon view, layout position will be decided by the priority.
     */
    void registerQuickIconDone();

    /**
     * Register indicator icon view, layout position will be decided by the priority.
     */
    void registerIndicatorDone();

    /**
     * Register mode icon that shown in the mode list view. The order of the mode icon will be
     * decided by the priority value.
     * @param items The mode information for register.
     */
    void registerMode(List<ModeItem> items);

    /**
     * Update current running mode.
     * Notice: This is used for mode manager to update current mode, do not
     * use it in any other place.
     * @param mode Mode name.
     */
    void updateCurrentMode(String mode);

    /**
     * Set Camera preview size.
     * @param width preview width, must > 0.
     * @param height preview height, must > 0.
     * @param listener Set preview status listener. The new listener will replace the old one.
     */
    void setPreviewSize(int width, int height, ISurfaceStatusListener listener);
    /**
     * used get the video recording ui.
     * @return video recording ui
     */
    IVideoUI getVideoUi();
    /**
     * used get the review ui.
     * @return review ui.
     */
    IReviewUI getReviewUI();
    /**
     * Get an implementation of intent photo ui.
     * @return an instance of IIntentPhotoUi.
     */
    @Nonnull
    IIntentPhotoUi getPhotoUi();

    /**
     * Add setting view instance to setting view list.
     *
     * @param view The instance of {@link ICameraSettingView}.
     */
    void addSettingView(ICameraSettingView view);

    /**
     * Remove setting view instance from setting view list.
     *
     * @param view The instance of {@link ICameraSettingView}.
     */
    void removeSettingView(ICameraSettingView view);

    /**
     * Refresh setting view.
     */
    void refreshSettingView();

    /**
     * Update setting icon visibility to decide setting icon is shown or not.
     */
    void updateSettingIconVisibility();

    /**
     * Start the animation.
     * @param type Animation type. {@link AnimationType}
     * @param data Animation data. {@link AnimationData}
     */
    void animationStart(AnimationType type, AnimationData data);

    /**
     * Stop the animation.
     * @param type Animation type. {@link AnimationType}
     */
    void animationEnd(AnimationType type);

    /**
     * Show saving dialog. The dialog will cover full screen and no ui should show behind it.
     * @param message The dialog's text view message.
     * @param isNeedShowProgress Is need show progress bar or not.
     */
    void showSavingDialog(String message, boolean isNeedShowProgress);

    /**
     * Hide saving dialog. Notify call showSavingDialog() firstly.
     */
    void hideSavingDialog();

    /**
     * Add nine grid view entry.
     *
     * @param view The entry view.
     */
    void setEffectViewEntry(View view);

    /**
     * Attach effect view entry to view tree;
     */
    void attachEffectViewEntry();

    /**
     * Show quick switcher option view, mode picker and quick switch will disappear.
     * @param optionView the option view, it should not attach to any parent view.
     */
    void showQuickSwitcherOption(View optionView);

    /**
     * Hide quick switcher option view, it will remove from the option parent view.
     */
    void hideQuickSwitcherOption();
}
