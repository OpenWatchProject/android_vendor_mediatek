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
package com.mediatek.camera.platform;

import java.io.FileDescriptor;

import android.media.CamcorderProfile;
import android.net.Uri;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.ImageView;

import com.mediatek.camera.ICameraMode.CameraModeType;

public interface ICameraAppUi {

    public enum CommonUiType {
        SHUTTER,
        MODE_PICKER,
        THUMBNAIL,
        PICKER,
        INDICATOR,
        REMAINING,
        INFO,       //info manager
        REVIEW,     //review manager
        ROTATE_PROGRESS,
        ROTATE_DIALOG,
        ZOOM,
        SETTING,
        FACE_BEAUTY_ENTRY,
    }

    public enum SpecViewType {
        MODE_FACE_BEAUTY,
        MODE_PANORAMA,
        MODE_PIP,
        MODE_STEREO,
        MODE_SLOW_MOTION,
        ADDITION_CONTINUE_SHOT,
        ADDITION_EFFECT,
        ADDITION_OBJECT_TRACKING,
    }

    public enum ShutterButtonType {
        SHUTTER_TYPE_PHOTO_VIDEO,
        SHUTTER_TYPE_PHOTO,
        SHUTTER_TYPE_VIDEO,
        SHUTTER_TYPE_OK_CANCEL,
        SHUTTER_TYPE_CANCEL,
        SHUTTER_TYPE_CANCEL_VIDEO,
        SHUTTER_TYPE_SLOW_VIDEO,
    }

    public enum ViewState {
        VIEW_STATE_NORMAL,
        VIEW_STATE_CAPTURE,
        VIEW_STATE_PRE_RECORDING,
        VIEW_STATE_RECORDING,
        VIEW_STATE_SETTING,
        VIEW_STATE_SUB_SETTING,
        VIEW_STATE_FOCUSING,
        VIEW_STATE_SAVING,
        VIEW_STATE_REVIEW,
        VIEW_STATE_CAMERA_OPENED,
        VIEW_STATE_CAMERA_CLOSED,
        VIEW_STATE_PICKING,
        VIEW_STATE_CONTINUOUS_CAPTURE,
        VIEW_STATE_LOMOEFFECT_SETTING,
        VIEW_STATE_HIDE_ALL_VIEW,
    }

    // TODO: API naming should refernece to GestureDetector.java under framework
    public interface GestureListener {
        public boolean onDown(float x, float y, int width, int height);

        public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY);

        public boolean onScroll(float dx, float dy, float totalX, float totalY);

        public boolean onSingleTapUp(float x, float y);

        public boolean onSingleTapConfirmed(float x, float y);

        public boolean onUp();

        public boolean onDoubleTap(float x, float y);

        public boolean onScale(float focusX, float focusY, float scale);

        public boolean onScaleBegin(float focusX, float focusY);

        public boolean onLongPress(float x, float y);
    }

    public ICameraView getCameraView(CommonUiType type);

    public ICameraView getCameraView(SpecViewType mode);

    // CommonUiType --> SHUTTER
    public void switchShutterType(ShutterButtonType type);

    public ShutterButtonType getShutterType();

    public ImageView getVideoShutter();

    public ImageView getPhotoShutter();

    public void setPhotoShutterEnabled(boolean enable);

    public void updateVideoShutterStatues(boolean enable);

    public void setVideoShutterEnabled(boolean enable);

    public void setVideoShutterMask(boolean enable);

    // CommonUiType --> MODE_PICKER
    public void setCurrentMode(CameraModeType mode);

    // CommonUiType --> THUMBNAIL
    public void updatePreference();

    public void setThumbnailRefreshInterval(int ms);

    public void forceThumbnailUpdate();

    public Uri getThumbnailUri();
    public String getThumbnailMimeType();
    // CommonUiType -> PICKER
    public void setCameraId(int cameraId);

    // CommonUiType --> REMAINING
    public void showRemainHint();

    public void clearRemainAvaliableSpace();

    public boolean showRemainIfNeed();

    public long updateRemainStorage();

    public void showRemaining();

    public void showRemainingAways();

    // CommonUiType --> SETTING
    public boolean collapseSetting(boolean force);

    /**
     * Collapse sub setting.
     * @param force If it is true, force to collapse.
     * @return True if it collapse successfully.
     */
    public boolean collapseSubSetting(boolean force);

    public boolean performSettingClick();

    public boolean isSettingShowing();

    // CommonUiType --> REVIEW
    /**
     * set the review's onclick Listener ,such as ok/cancel/retake/play
     */
    public void setReviewListener(OnClickListener retatekListener, OnClickListener playListener);

    public void showReview(String path, FileDescriptor fd);

    public void hideReview();

    public void setReviewCompensation(int compensation);

    /**
     * get the reviewmanger is showing
     *
     * @return true means current is show review UI false means current is not
     *         show review UI
     */

    // CommonUiType --> ROTATE PROGRESS
    /**
     * when save the video or photo,need show saving dialog, such as:Saving...
     *
     */
    public void showProgress(String msg);

    /**
     * dismiss the progress when not need show the progress
     */
    public void dismissProgress();

    public boolean isShowingProgress();

    // CommonUiType --> INFO
    public void showText(CharSequence text);

    public void showInfo(final String text);

    public void showInfo(final CharSequence text, int showMs);

    public void dismissInfo();

    // OnScreenHint -> show toast

    public void showToastForShort(int stringId);

    public void showToast(int stringId);

    public void showToast(String message);

    public void hideToast();

    public void showAlertDialog(String title, String msg, String button1Text, final Runnable r1,
            String button2Text, final Runnable r2);

    public void setSwipeEnabled(boolean enabled);

    public boolean collapseViewManager(boolean force);

    public void restoreViewState();

    /**
     * get current preview frame's width
     */
    public int getPreviewFrameWidth();

    /**
     * get current preview frame's height
     *
     */
    public int getPreviewFrameHeight();

    /**
     * get uncrop preview width
     */
    public int getUnCropWidth();

    /**
     * get uncrop preview height
     *
     */
    public int getUnCropHeight();

    public View getPreviewFrameLayout();

    public void showAllViews();

    public void hideAllViews();

    public void setViewState(ViewState state);

    public ViewState getViewState();

    public boolean isNormalViewState();

    public void setCamcorderProfile(CamcorderProfile profile);

    public void changeZoomForQuality();

  /**
   * The scene detected by ASD.
   * @param scene The scene value detected.
   * @param suggestedHdr If it is true, it suggests to use HDR capture will be
   *     better.
   */
    public void onDetectedSceneMode(int scene, boolean suggestedHdr);

    public void restoreSceneMode();

    public void setGestureListener(GestureListener listener);

    public ViewGroup getBottomViewLayer();

    public ViewGroup getNormalViewLayer();

    public void changeBackToVFBModeStatues(boolean isNeed);

    public void updateFaceBeatuyEntryViewVisible(boolean visible);

    public void updateSnapShotUIView(boolean enabled);

    public void setOkButtonEnabled(boolean enable);

    public boolean isNeedBackToVFBMode();

    /**
     * the interface to update thumbnail view, if the data is YUV from post view
     * callback.
     * @param yuvData the YUV data from post view callback.
     * @param yuvWidth the width of YUV image.
     * @param yuvHeight the height of YUV image.
     * @param orientation the phone orientation set to native.
     * @param imageFormat the image format of YUV image.
     */
    public void updateThumbnailViewWithYuv(byte[] yuvData, int yuvWidth,
                                           int yuvHeight, int orientation, int imageFormat);


    /**
     * Notifies the app of the newly captured media from mode.
     * @param uri the newly uri.
     * @param needNotify need notify to others or not.
     */
    void notifyNewMedia(Uri uri,  boolean needNotify);

}
