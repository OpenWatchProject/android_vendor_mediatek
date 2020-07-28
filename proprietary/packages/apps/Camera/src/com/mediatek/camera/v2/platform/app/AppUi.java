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

package com.mediatek.camera.v2.platform.app;

import android.graphics.Bitmap;
import android.widget.FrameLayout;

/**
 * The common ui controller used by module.
 * <p>
 * Contains the following APIs:
 * <p>
 * <li>Shutter Button UI</li>
 * <li>Setting UI</li>
 */
public interface AppUi {
    /** Preview is not covered. */
    public static final int PREVIEW_VISIBILITY_UNCOVERED = 0;
    /** Preview is covered by e.g. the transparent mode drawer. */
    public static final int PREVIEW_VISIBILITY_COVERED = 1;
    /**
     * An interface which defines the shutter events listener.
     */
    public interface ShutterEventsListener {
        /**
         * Called when the shutter state is changed to pressed.
         */
        public void onShutterPressed();

        /**
         * Called when the shutter state is changed to released.
         */
        public void onShutterReleased();

        /**
         * Called when the shutter is clicked.
         */
        public void onShutterClicked();

        /**
         * Called when the shutter is long pressed.
         */
        public void onShutterLongPressed();
    }

    /**
     * 3rd party launch camera, after capture or recording, ok and cancel button
     * will be shown in shutter manager.
     *
     */
    public interface OkCancelClickListener {
        public void onOkClick();
        public void onCancelClick();
    }

    /**
     * 3rd party launch camera or video, after capture or recording, play button and
     * retake button will be shown in review ui.
     *
     */
    public interface PlayButtonClickListener {
        public void onPlay();
    }

    /**
     * 3rd party launch camera or video,
     * after capture or recording, play button and
     * retake button will be shown in review ui.
     *
     */
    public interface RetakeButtonClickListener {
        public void onRetake();
    }

    /**
     * Returns the {@link android.widget.FrameLayout} as the root of the module layout.
     */
    public FrameLayout getModuleLayoutRoot();
    /**
     * Enable or disable swipe gestures. We want to disable them e.g. while we
     * record a video.
     * @param enabled result
     */
    public void setSwipeEnabled(boolean enabled);

    /**
     * set all common ui's enable status.
     *
     * @param enable false all common ui are disabled but can be seen.
     */
    public void setAllCommonViewEnable(boolean enable);
    /**
     * Set all common view visible but shutter button.
     * @param visible result value
     */
    public void setAllCommonViewButShutterVisible(boolean visible);

    /**
     * whether the preview is visible.
     *
     * @return whether the preview is visible, one of
     *            {@link #PREVIEW_VISIBILITY_VISIBLE}, {@link #PREVIEW_VISIBILITY_COVERED},
     *            {@link #PREVIEW_VISIBILITY_HIDDEN}
     */
    public int getPreviewVisibility();


    /**
     * Set photo / video shutter button event listener.
     * TODO consider refactor this for add/remove shutter event listener
     * @param eventListener listener
     * @param videoShutter whether is video button
     */
    public void setShutterEventListener(ShutterEventsListener eventListener, boolean videoShutter);

    /**
     * Set ok and cancel button click listener.
     * @param listener button listener
     */
    public void setOkCancelClickListener(OkCancelClickListener listener);
    /**
     * Check shutter button enable status.
     * @param videoShutter true check video shutter button whether is enable.
     * @return whether the specified shutter button is enable.
     */
    public boolean isShutterButtonEnabled(boolean videoShutter);
    /**
     * Enable / Disable photo / video shutter button.
     * @param enabled true enable shutter button; false disable shutter button.
     * @param videoShutter true for video button; false for photo button.
     */
    public void setShutterButtonEnabled(boolean enabled, boolean videoShutter);
    /**
     * Trigger video / photo shutter button's click event.
     * @param videoShutter true, click video button; false, click photo button.
     */
    public void performShutterButtonClick(boolean videoShutter);

    /**
     * update shutter button's image resource id.
     * @param imageResourceId image resource id
     * @param isVideoButton whether is video shutter
     */
    public void switchShutterButtonImageResource(int imageResourceId, boolean isVideoButton);
    /**
     * update shutter manager view's layout.
     * @param layoutId layout id
     */
    public void switchShutterButtonLayout(int layoutId);

    /**
     * Set the listener to listen play button that is in review ui click.
     * @param listener button listener
     */
    public void setPlayButtonClickListener(PlayButtonClickListener listener);


    /**
     * Set the listener to listen retake button that is in review ui click.
     * @param listener button listener
     */
    public void setRetakeButtonClickListener(RetakeButtonClickListener listener);

    /**
     * Stop to show common ui. If parameter stop is true, stop show common ui, otherwise,
     * do not to stop show common ui.
     * @param stop true or false.
     */
    public void stopShowCommonUI(boolean stop);
    /**
     * charge whether setting is showing.
     * @return result
     */
    public boolean isSettingViewShowing();
    /**
     * Show setting ui.
     */
    public void showSettingUi();
    /**
     * Hide setting ui.
     */
    public void hideSettingUi();
    /**
     * Show mode UI.
     */
    public void showModeOptionsUi();
    /**
     * Hide mode UI.
     */
    public void hideModeOptionsUi();
    /**
     * Show picker button UI.
     */
    public void showPickerManagerUi();
    /**
     * Hide picker button UI.
     */
    public void hidePickerManagerUi();
    /**
     * This method used to trigger camera switch.
     * @param force If it is true, force do camera switch.
     */
    public void performCameraPickerBtnClick(boolean force);
    /**
     * Show thumbnail UI.
     */
    public void showThumbnailManagerUi();
    /**
     * Enable or disable thumbnail UI.
     * @param enable True enable thumbnail UI, false disable thumbnail UI.
     */
    public void setThumbnailManagerEnable(boolean enable);
    /**
     * Hide thumbnail UI.
     */
    public void hideThumbnailManagerUi();
    /**
     * Update thumbnail.
     * used for active update thumbnail
     * after storage state change will call it.
     */
    public void forceUpdateThumbnail();
    /**
     * Show indicator UI.
     */
    public void showIndicatorManagerUi();
    /**
     * Hide indicator UI.
     */
    public void hideIndicatorManagerUi();
    /**
     * Show infomation to notify user how to operate. Default,
     * the infomation will show 3000 millisecond.
     * @param text The content going to show.
     */
    public void showInfo(final String text);
    /**
     * Show infomation to notify user how to operate.
     * Default, the infomation will show 3000 millisecond.
     * @param text The content going to show.
     * @param showMs Indicate how long the infomation be shown, unit is a millisecond.
     */
    public void showInfo(final CharSequence text, int showMs);
    /**
     * Only dismiss info UI or not.
     * @param onlyDismissInfo True only dismiss info UI,
     * false except dismiss info UI, other correlative UI will be shown.
     */
    public void dismissInfo(boolean onlyDismissInfo);
    /**
     * Use to show toast.
     * @param message the info want to show.
     */
    public void showHint(String message);
    /**
     * Use to hide toast which is show by showHint.
     */
    public void hideHint();
    /**
     * Show saving dialog.
     * @param text reminding string
     */
    public void showSavingProgress(String text);

    /**
     * Dismiss saving dialog.
     */
    public void dismissSavingProgress();


    /**
     * Set review image to review view and show.
     * @param bitmap review image
     */
    public void showReviewView(Bitmap bitmap);

    /**
     * Show review view.
     * @param jpegData image date
     * @param height image size
     */
    public void showReviewView(byte[] jpegData, int height) ;

    /**
     * Hide review view.
     */
    public void hideReviewView();

    /**
     * Show left counts of image can be taken.
     * @param bytePerCount The bytes one image occupy.
     * @param showAlways If this is true, always show left counts of image can be taken. If
     *        this is false, only show left counts when counts less than 100L. After capture
     *        this method should be called
     */
    public void showLeftCounts(int bytePerCount, boolean showAlways);

    /**
     * Show left times can be recorded.
     * @param bytePerMs The bytes of one millisecond recording
     */
    public void showLeftTime(long bytePerMs);

    /**
     * Update scene value detected by asd.
     * @param scene mode value
     */
    public void updateAsdDetectedScene(String scene);

    /**
     * Show the shutter button view.
     */
    public void showShutterButton();

    /**
     * Hide the shutter button view,such as Cross mount
     * scan view is showing,need hide the view.
     */
    public void hideShutterButton();
}
