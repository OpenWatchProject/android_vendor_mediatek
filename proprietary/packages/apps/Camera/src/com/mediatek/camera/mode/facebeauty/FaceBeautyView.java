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

package com.mediatek.camera.mode.facebeauty;

import android.app.Activity;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.LinearLayout;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;

import com.android.camera.R;

import com.mediatek.camera.ICameraMode.CameraModeType;
import com.mediatek.camera.mode.facebeauty.FaceBeautyParametersHelper.ParameterListener;
import com.mediatek.camera.platform.ICameraAppUi;
import com.mediatek.camera.platform.ICameraAppUi.ViewState;
import com.mediatek.camera.platform.IModuleCtrl;
import com.mediatek.camera.ui.CameraView;
import com.mediatek.camera.ui.RotateImageView;
import com.mediatek.camera.util.Log;
import com.mediatek.camera.util.Util;

import java.util.ArrayList;

/**
 * The FaceBeautyView.java equals former :FaceBeautyIndicatorManager.java
 */
public class FaceBeautyView extends CameraView implements OnClickListener {
    private static final String TAG = "FaceBeautyView";

    private static final int FACE_BEAUTY_WRINKLE_REMOVE = 0;
    private static final int FACE_BEAUTY_WHITENING = 1;
    private static final int FACE_BEAUTY_BEAUTY_SHAPE = 2; // here shape means
    // slim
    private static final int FACE_BEAUTY_BIG_EYES_NOSE = 3;
    private static final int FACE_BEAUTY_MODIFY_ICON = 4;
    private static final int FACE_BEAUTY_ICON = 5;

    // use for hander the effects item hide
    private static final int DISAPPEAR_VFB_UI_TIME = 5000;
    private static final int DISAPPEAR_VFB_UI = 0;
    // Decoupling: 4 will be replaced by parameters
    private int SUPPORTED_FB_EFFECTS_NUMBER = 0;

    // 6 means all the number of icons in the preview
    private static final int NUMBER_FACE_BEAUTY_ICON = 6;
    private static final int SUPPORTED_FB_PROPERTIES_MIN_NUMBER = 2;
    private static final int SUPPORTED_FB_PROPERTIES_MAX_NUMBER = 4;

    private static final int[] FACE_BEAUTY_ICONS_NORMAL = new int[NUMBER_FACE_BEAUTY_ICON];
    private static final int[] FACE_BEAUTY_ICONS_HIGHTLIGHT = new int[NUMBER_FACE_BEAUTY_ICON];

    // Because current face.length is 0 always callback ,if not always callback
    // ,will not use this
    private boolean mIsTimeOutMechanismRunning = false;

    /**
     * when FaceBeautyMode send MSG show the FB icon but if current is in
     * setting change, FaceBeautyMode will receive a parameters Ready MSG so
     * will notify view show.but this case is in the setting,not need show the
     * view if the msg:ON_CAMERA_PARAMETERS_READY split to
     * ON_CAMERA_PARAMETERS_READY and ON_CAMERA_PARAMETERS_CHANGE and the change
     * MSG used for setting change,so can not use mIsShowSetting
     */
    private boolean mIsShowSetting = false;
    private boolean mIsInPictureTakenProgress = false;

    /**
     * this tag is used for judge whether in camera preview for example:camera
     * -> Gallery->play video,now when play the video,camera will execute
     * onPause(),and when the finished play,camera will onResume,so this time
     * FaceBeautyMode will receive onCameraOpen and onCameraParameters Ready
     * MSG,so will notify FaceBeautyView ,but this view will show the VFB UI,so
     * in this case[not in Camera preview] not need show the UI if
     * FaceBeautyView not show the UI,so this not use
     */
    private boolean mIsInCameraPreview = true;

    private int mSupportedDuration = 0;
    private int mSupportedMaxValue = 0;
    private int mCurrentViewIndex = 0;

    private String mEffectsKey = null;
    private String mEffectsValue = null;

    private ICameraAppUi mICameraAppUi;
    private IModuleCtrl mIModuleCtrl;

    private ArrayList<Integer> mFaceBeautyPropertiesValue = new ArrayList<Integer>();
    private FaceBeautyInfo mFaceBeautyInfo;
    private Handler mHandler;
    private LinearLayout mBgLinearLayout;
    private ParameterListener mListener;
    private RotateImageView[] mFaceBeautyImageViews = new RotateImageView[NUMBER_FACE_BEAUTY_ICON];
    private SeekBar mAdjustmentValueIndicator;
    private View mView;

    static {
        FACE_BEAUTY_ICONS_NORMAL[FACE_BEAUTY_WRINKLE_REMOVE] = R.drawable.fb_smooth_normal;
        FACE_BEAUTY_ICONS_NORMAL[FACE_BEAUTY_WHITENING] = R.drawable.fb_whitening_normal;
        FACE_BEAUTY_ICONS_NORMAL[FACE_BEAUTY_BEAUTY_SHAPE] = R.drawable.fb_sharp_normal;
        FACE_BEAUTY_ICONS_NORMAL[FACE_BEAUTY_BIG_EYES_NOSE] = R.drawable.fb_eye_normal;
        FACE_BEAUTY_ICONS_NORMAL[FACE_BEAUTY_MODIFY_ICON] = R.drawable.fb_setting_normal;
        FACE_BEAUTY_ICONS_NORMAL[FACE_BEAUTY_ICON] = R.drawable.ic_mode_facebeauty_normal;
    }

    static {
        FACE_BEAUTY_ICONS_HIGHTLIGHT[FACE_BEAUTY_WRINKLE_REMOVE] = R.drawable.fb_smooth_presse;
        FACE_BEAUTY_ICONS_HIGHTLIGHT[FACE_BEAUTY_WHITENING] = R.drawable.fb_whitening_presse;
        FACE_BEAUTY_ICONS_HIGHTLIGHT[FACE_BEAUTY_BEAUTY_SHAPE] = R.drawable.fb_sharp_presse;
        FACE_BEAUTY_ICONS_HIGHTLIGHT[FACE_BEAUTY_BIG_EYES_NOSE] = R.drawable.fb_eye_presse;
        FACE_BEAUTY_ICONS_HIGHTLIGHT[FACE_BEAUTY_MODIFY_ICON] = R.drawable.fb_setting_pressed;
        FACE_BEAUTY_ICONS_HIGHTLIGHT[FACE_BEAUTY_ICON] = R.drawable.ic_mode_facebeauty_focus;
    }

    public FaceBeautyView(Activity mActivity) {
        super(mActivity);
        Log.i(TAG, "[FaceBeautyView]constructor...");
        mHandler = new IndicatorHandler(mActivity.getMainLooper());
    }

    @Override
    public void init(Activity activity, ICameraAppUi cameraAppUi,
            IModuleCtrl moduleCtrl) {
        Log.i(TAG, "[init]...");
        mICameraAppUi = cameraAppUi;
        mIModuleCtrl = moduleCtrl;
        setOrientation(mIModuleCtrl.getOrientationCompensation());
        mView = inflate(R.layout.facebeauty_indicator);
        mBgLinearLayout = (LinearLayout) mView.findViewById(R.id.effcts_bg);
        mFaceBeautyImageViews[FACE_BEAUTY_ICON] = (RotateImageView) mView
                .findViewById(R.id.facebeauty_icon);
        mFaceBeautyImageViews[FACE_BEAUTY_MODIFY_ICON] = (RotateImageView) mView
                .findViewById(R.id.facebeauty_modify);
        mFaceBeautyImageViews[FACE_BEAUTY_BIG_EYES_NOSE] = (RotateImageView) mView
                .findViewById(R.id.facebeauty_big_eys_nose);
        mFaceBeautyImageViews[FACE_BEAUTY_BEAUTY_SHAPE] = (RotateImageView) mView
                .findViewById(R.id.facebeauty_beauty_shape);
        mFaceBeautyImageViews[FACE_BEAUTY_WHITENING] = (RotateImageView) mView
                .findViewById(R.id.facebeauty_whitening);
        mFaceBeautyImageViews[FACE_BEAUTY_WRINKLE_REMOVE] = (RotateImageView) mView
                .findViewById(R.id.facebeauty_wrinkle_remove);
        mAdjustmentValueIndicator = (SeekBar) mView
                .findViewById(R.id.facebeauty_changevalue);
        mAdjustmentValueIndicator.setProgressDrawable(mActivity.getResources()
                .getDrawable(R.drawable.bar_bg));
        applyListeners();
        mFaceBeautyInfo = new FaceBeautyInfo(activity, mIModuleCtrl);
    }

    @Override
    public void hide() {
        Log.i(TAG, "[hide]...");
        hideEffectsIconAndSeekBar();
        updateModifyIconStatus(false);
        hideFaceBeautyIcon();
        super.hide();
    }

    @Override
    protected View getView() {
        Log.i(TAG, "[getView].view = " + mView);
        return mView;

    }

    @Override
    public void show() {
        Log.i(TAG, "[show]...,mIsShowSetting = " + mIsShowSetting
                + ",mIsInCameraPreview = " + mIsInCameraPreview);
        if (!mIsShowSetting && mIsInCameraPreview) {
            super.show();
            intoVfbMode();
        }
    }

    @Override
    public void setListener(Object obj) {
        mListener = (ParameterListener) obj;
    }

    @Override
    public boolean update(int type, Object... args) {
        if (FaceBeautyMode.INFO_FACE_DETECTED != type
                && FaceBeautyMode.ORIENTATION_CHANGED != type) {
            Log.i(TAG, "[update] type = " + type);
        }
        boolean value = false;
        switch (type) {

        case FaceBeautyMode.ON_CAMERA_CLOSED:
            // when back to camera, the auto back to photoMode not need
            removeBackToNormalMsg();
            break;

        case FaceBeautyMode.ON_CAMERA_PARAMETERS_READY:
            prepareVFB();
            break;

        case FaceBeautyMode.INFO_FACE_DETECTED:
            updateUI((Integer) args[0]);
            break;

        case FaceBeautyMode.ORIENTATION_CHANGED:
            Util.setOrientation(mView, (Integer) args[0], true);
            if (mFaceBeautyInfo != null) {
                mFaceBeautyInfo.onOrientationChanged((Integer) args[0]);
            }
            break;

        case FaceBeautyMode.ON_FULL_SCREEN_CHANGED:
            mIsInCameraPreview = (Boolean) args[0];
                Log.i(TAG, "ON_FULL_SCREEN_CHANGED, mIsInCameraPreview = "
                        + mIsInCameraPreview);
                if (mIsInCameraPreview) {
                show();
            } else {
                // because when effect is showing, we have hide the ALLViews,so
                // need show the views
                // otherwise back to Camera,you will found all the UI is hide
                if (isEffectsShowing()) {
                    mICameraAppUi.showAllViews();
                }
                hide();
            }
            break;

        case FaceBeautyMode.ON_BACK_PRESSED:
            if (isEffectsShowing()) {
                onModifyIconClick();
                value = true;
            } else {
                // when back to camera, the auto back to photoMode not need
                removeBackToNormalMsg();
            }
            break;

        case FaceBeautyMode.HIDE_EFFECTS_ITEM:
            if (isEffectsShowing()) {
                onModifyIconClick();
            }
            break;

        case FaceBeautyMode.ON_SETTING_BUTTON_CLICK:
            mIsShowSetting = (Boolean) args[0];

            Log.i(TAG, "ON_SETTING_BUTTON_CLICK,mIsShowSetting =  " + mIsShowSetting);

            if (mIsShowSetting) {
                hide();
            } else {
                show();
            }
            break;

        case FaceBeautyMode.ON_LEAVE_FACE_BEAUTY_MODE:
            hide();
            uninit();
            break;

        case FaceBeautyMode.REMVOE_BACK_TO_NORMAL:
            // this case also need reset the automatic back to VFB mode
            removeBackToNormalMsg();
            break;

        case FaceBeautyMode.ON_SELFTIMER_CAPTUEING:
            Log.i(TAG, "[ON_SELFTIMER_CAPTUEING] args[0] = "
                    + (Boolean) args[0] + ", mIsInPictureTakenProgress = "
                    + mIsInPictureTakenProgress);
            if ((Boolean) args[0]) {
                hide();
                removeBackToNormalMsg();
            } else {
                if (!mIsInPictureTakenProgress) {
                    show();
                }
            }
            break;

        case FaceBeautyMode.IN_PICTURE_TAKEN_PROGRESS:
            mIsInPictureTakenProgress = (Boolean) args[0];
            Log.i(TAG, "mIsInPictureTakenProgress = " + mIsInPictureTakenProgress
                    + ",mIsTimeOutMechanismRunning = " + mIsTimeOutMechanismRunning);
            if (mIsInPictureTakenProgress) {
                hide();
                removeBackToNormalMsg();
            } else {
                show();
            }
            break;

        default:
            break;
        }

        return value;
    }

    @Override
    public int getViewHeight() {
        return 0;
    }

    @Override
    public int getViewWidth() {
        return 0;
    }

    @Override
    public boolean isEnabled() {
        return false;
    }

    @Override
    public boolean isShowing() {
        return false;
    }

    @Override
    public void onClick(View view) {
        // First:get the click view index,because need show the effects name
        for (int i = 0; i < NUMBER_FACE_BEAUTY_ICON; i++) {
            if (mFaceBeautyImageViews[i] == view) {
                mCurrentViewIndex = i;
                break;
            }
        }

        // Second:highlight the effect's image Resource which is clicked
        // also need set the correct effect value
        for (int i = 0; i < SUPPORTED_FB_EFFECTS_NUMBER; i++) {
            if (mCurrentViewIndex == i) {
                mFaceBeautyImageViews[i]
                        .setImageResource(FACE_BEAUTY_ICONS_HIGHTLIGHT[i]);
                // set the effects value
                int progerss = mFaceBeautyPropertiesValue.get(i);
                setProgressValue(progerss);
            } else {
                mFaceBeautyImageViews[i]
                        .setImageResource(FACE_BEAUTY_ICONS_NORMAL[i]);
            }
        }
        Log.d(TAG, "[onClick]mCurrentViewIndex = " + mCurrentViewIndex);

        switch (mCurrentViewIndex) {
        case FACE_BEAUTY_WRINKLE_REMOVE:
            mEffectsKey = Util.KEY_FACE_BEAUTY_SMOOTH;
            break;

        case FACE_BEAUTY_WHITENING:
            mEffectsKey = Util.KEY_FACE_BEAUTY_SKIN_COLOR;
            break;

        case FACE_BEAUTY_BEAUTY_SHAPE:
            mEffectsKey = Util.KEY_FACE_BEAUTY_SHARP;
            break;

        case FACE_BEAUTY_BIG_EYES_NOSE:
            mEffectsKey = Util.KEY_FACE_BEAUTY_BIG_EYES;
            break;

        case FACE_BEAUTY_MODIFY_ICON:
            onModifyIconClick();
            break;

        case FACE_BEAUTY_ICON:
            onFaceBeautyIconClick();
            break;

        default:
            Log.i(TAG, "[onClick]click is not the facebeauty imageviews,need check");
            break;
        }
        // current not show the toast of the view
        showEffectsToast(view, mCurrentViewIndex);
    }

    private void applyListeners() {
        for (int i = 0; i < NUMBER_FACE_BEAUTY_ICON; i++) {
            if (null != mFaceBeautyImageViews[i]) {
                mFaceBeautyImageViews[i].setOnClickListener(this);
            }
        }
        if (mAdjustmentValueIndicator != null) {
            mAdjustmentValueIndicator
                    .setOnSeekBarChangeListener(mHorientiaonlSeekBarLisenter);
        }
    }

    // when click the effects modify icon will run follow
    private void onModifyIconClick() {
        Log.d(TAG, "[onModifyIconClick],isFaceBeautyEffectsShowing = "
                + isEffectsShowing());
        if (isEffectsShowing()) {
            // if current is showing and click the modify icon,need hide the
            // common views ,such as ModePicker/thumbnail/picker/settings item
            mICameraAppUi.setViewState(ViewState.VIEW_STATE_NORMAL);
            hideEffectsIconAndSeekBar();
        } else {
            Log.i(TAG, "onModifyIconClick, mICameraAppUI = " + mICameraAppUi);
            mICameraAppUi.setViewState(ViewState.VIEW_STATE_HIDE_ALL_VIEW);
            if (mBgLinearLayout != null) {
                mBgLinearLayout.setBackgroundResource(R.drawable.bg_icon);
            }
            showFaceBeautyEffects();
            // initialize the parameters
            mEffectsKey = Util.KEY_FACE_BEAUTY_SMOOTH;
            // show default string
            showEffectsToast(mFaceBeautyImageViews[FACE_BEAUTY_WRINKLE_REMOVE],
                    FACE_BEAUTY_WRINKLE_REMOVE);
            // need set current values
            setProgressValue(mFaceBeautyPropertiesValue
                    .get(FACE_BEAUTY_WRINKLE_REMOVE));
        }
    }

    private boolean isEffectsShowing() {
        boolean isEffectsShowing = View.VISIBLE == mFaceBeautyImageViews[FACE_BEAUTY_WRINKLE_REMOVE]
                .getVisibility();
        Log.d(TAG, "isEffectsShowing = " + isEffectsShowing);

        return isEffectsShowing;
    }

    private void hideEffectsIconAndSeekBar() {
        Log.d(TAG, "[hideEffectsIconAndSeekBar]mSupporteFBEffectsNumber = "
                + SUPPORTED_FB_EFFECTS_NUMBER);
        hideEffectsItems();
        hideSeekBar();
        if (mFaceBeautyInfo != null) {
            mFaceBeautyInfo.cancel();
        }
        if (mBgLinearLayout != null) {
            mBgLinearLayout.setBackgroundDrawable(null);
        }

        // change the image resource
        mFaceBeautyImageViews[FACE_BEAUTY_MODIFY_ICON]
                .setImageResource(FACE_BEAUTY_ICONS_NORMAL[FACE_BEAUTY_MODIFY_ICON]);
    }

    private void showFaceBeautyEffects() {
        Log.d(TAG, "[showFaceBeautyEffects]...");
        // default first effects is wrinkle Remove effects
        mFaceBeautyImageViews[FACE_BEAUTY_WRINKLE_REMOVE]
                .setImageResource(FACE_BEAUTY_ICONS_HIGHTLIGHT[FACE_BEAUTY_WRINKLE_REMOVE]);
        mFaceBeautyImageViews[FACE_BEAUTY_WRINKLE_REMOVE]
                .setVisibility(View.VISIBLE);

        mFaceBeautyImageViews[FACE_BEAUTY_MODIFY_ICON]
                .setImageResource(FACE_BEAUTY_ICONS_HIGHTLIGHT[FACE_BEAUTY_MODIFY_ICON]);

        // also need to show the background
        if (mBgLinearLayout != null) {
            mBgLinearLayout.setVisibility(View.VISIBLE);
        }

        // show the left of imageviews
        for (int i = 1; i < SUPPORTED_FB_EFFECTS_NUMBER; i++) {
            mFaceBeautyImageViews[i]
                    .setImageResource(FACE_BEAUTY_ICONS_NORMAL[i]);
            mFaceBeautyImageViews[i].setVisibility(View.VISIBLE);
        }
        // when set the face mode to Mulit-face ->close camera ->reopen camera
        // ->go to FB mdoe
        // will found the effects UI is error,so need set not supported effects
        // view gone. //[this need check whether nead TODO]
        for (int i = SUPPORTED_FB_EFFECTS_NUMBER;
                        i < FaceBeautyMode.SUPPORTED_FB_PROPERTIES_MAX_NUMBER; i++) {
            mFaceBeautyImageViews[i].setVisibility(View.GONE);
        }
        // also need to show SeekBar
        if (mAdjustmentValueIndicator != null) {
            mAdjustmentValueIndicator.setMax(mSupportedDuration);
            mAdjustmentValueIndicator.setVisibility(View.VISIBLE);
        }
    }

    private void showEffectsToast(View view, int index) {
        if (index >= 0 && index < SUPPORTED_FB_EFFECTS_NUMBER) {
            if (view.getContentDescription() != null) {
                mFaceBeautyInfo.setText(view.getContentDescription());
                mFaceBeautyInfo.cancel();
                // Margin left have more than 2 bottom: FB/modifyICon
                mFaceBeautyInfo.setTargetId(index, SUPPORTED_FB_EFFECTS_NUMBER + 2);
                mFaceBeautyInfo.showToast();
            }
        }
    }

    private void hideToast() {
        Log.d(TAG, "[hideToast()]");
        if (mFaceBeautyInfo != null) {
            mFaceBeautyInfo.hideToast();
        }
    }

    private void setProgressValue(int value) {
        // because the effects properties list is stored as parameters value
        // so need revert the value
        // but the progress bar is revert the max /min , so not need revert
        mAdjustmentValueIndicator.setProgress(convertToParamertersValue(value));
    }

    private int convertToParamertersValue(int value) {
        // one:in progress bar,the max value is at the end of left,and current
        // max value is 8;
        // but in our UI,the max value is at the begin of right.
        // two:the parameters supported max value is 4 ,min value is -4
        // above that,the parameters value should be :[native max - current
        // progress value]
        return mSupportedMaxValue - value;
    }

    private void onFaceBeautyIconClick() {
        Log.d(TAG, "[onFaceBeautyIconClick]isFaceBeautyModifyIconShowing = "
                + isModifyIconShowing());
        if (!isModifyIconShowing()) {
            intoVfbMode();
        } else {
            leaveVfbMode();
        }
    }

    private void intoVfbMode() {
        Log.d(TAG, "[intoVfbMode]");
        mFaceBeautyImageViews[FACE_BEAUTY_MODIFY_ICON]
                .setImageResource(FACE_BEAUTY_ICONS_NORMAL[FACE_BEAUTY_MODIFY_ICON]);
        updateModifyIconStatus(true);
        mFaceBeautyImageViews[FACE_BEAUTY_ICON]
                .setImageResource(FACE_BEAUTY_ICONS_HIGHTLIGHT[FACE_BEAUTY_ICON]);
        mFaceBeautyImageViews[FACE_BEAUTY_ICON].setVisibility(View.VISIBLE);
        update(FaceBeautyMode.ORIENTATION_CHANGED,
                mIModuleCtrl.getOrientationCompensation());
        hideEffectsItems();
        hideSeekBar();
    }

    private void leaveVfbMode() {
        // when isFaceBeautyModifyIconShowing = true,means the icon is
        // showing ,need hide the
        // face beauty effects and modify values Seekbar
        Log.d(TAG, "[leaveVfbMode]");
        updateModifyIconStatus(false);
        mFaceBeautyImageViews[FACE_BEAUTY_ICON]
                .setImageResource(FACE_BEAUTY_ICONS_NORMAL[FACE_BEAUTY_ICON]);
        mFaceBeautyImageViews[FACE_BEAUTY_ICON].setVisibility(View.VISIBLE);
        hideEffectsIconAndSeekBar();
        mICameraAppUi.setCurrentMode(CameraModeType.EXT_MODE_PHOTO);
    }

    private void updateUI(int length) {
        // Log.i(TAG, "[updateUI],face length = " + length);
        boolean enable = mListener.canShowFbIcon(length);
        if (enable) {
            mIsTimeOutMechanismRunning = false;
            mView.setEnabled(true);
            showFaceBeautyIcon();
            mHandler.removeMessages(DISAPPEAR_VFB_UI);
        } else if (length == 0 && !mIsTimeOutMechanismRunning) {
            if (isModifyIconShowing()) {
                if (mFaceBeautyImageViews[FACE_BEAUTY_MODIFY_ICON].isEnabled()) {
                    Log.i(TAG, "will send msg: DISAPPEAR_VFB_UI");
                    mView.setEnabled(false);
                    mHandler.removeMessages(DISAPPEAR_VFB_UI);
                    mHandler.sendEmptyMessageDelayed(DISAPPEAR_VFB_UI,
                            DISAPPEAR_VFB_UI_TIME);
                    mIsTimeOutMechanismRunning = true;
                }
            } else {
                hideFaceBeautyIcon();
            }
        }
    }

    private void showFaceBeautyIcon() {
        if (null != mFaceBeautyImageViews && !isFBIconShowing()) {
            int resValue = FACE_BEAUTY_ICONS_NORMAL[FACE_BEAUTY_ICON];
            // when modify icon is showing , this time not only show the FB icon
            // also need show the modify icon
            if (isModifyIconShowing()) { // Need Check
                updateModifyIconStatus(true);
            }
            mFaceBeautyImageViews[FACE_BEAUTY_ICON].setImageResource(resValue);
            mFaceBeautyImageViews[FACE_BEAUTY_ICON].setVisibility(View.VISIBLE);
            Log.d(TAG, "showFaceBeautyIcon....");
        }
    }

    private boolean isFBIconShowing() {
        boolean isFBIconShowing = View.VISIBLE == mFaceBeautyImageViews[FACE_BEAUTY_ICON]
                .getVisibility();

        return isFBIconShowing;
    }

    private boolean isModifyIconShowing() {
        boolean isModifyIconShowing = View.VISIBLE == mFaceBeautyImageViews[FACE_BEAUTY_MODIFY_ICON]
                .getVisibility();
        Log.d(TAG, "[isModifyIconShowing]isModifyIconShowing = "
                + isModifyIconShowing);

        return isModifyIconShowing;
    }

    private void hideFaceBeautyIcon() {
        if (null != mFaceBeautyImageViews) {
            mFaceBeautyImageViews[FACE_BEAUTY_ICON]
                    .setVisibility(View.INVISIBLE);
        }
    }

    private void updateModifyIconStatus(boolean visible) {
        if (!visible && mFaceBeautyInfo != null) {
            mFaceBeautyInfo.cancel();
        }
        if (mBgLinearLayout != null) {
            mBgLinearLayout.setBackgroundDrawable(null);
            mBgLinearLayout.setVisibility(View.VISIBLE);
        }
        mFaceBeautyImageViews[FACE_BEAUTY_MODIFY_ICON]
                .setImageResource(FACE_BEAUTY_ICONS_NORMAL[FACE_BEAUTY_MODIFY_ICON]);
        mFaceBeautyImageViews[FACE_BEAUTY_MODIFY_ICON]
                .setVisibility(visible ? View.VISIBLE : View.INVISIBLE);
        Log.d(TAG, "[updateModifyIconStatus]isFaceBeautyModifyIconShowing = "
                + isModifyIconShowing());
    }

    private void hideEffectsItems() {
        for (int i = 0; i < SUPPORTED_FB_EFFECTS_NUMBER; i++) {
            mFaceBeautyImageViews[i].setVisibility(View.GONE);
        }
    }

    private void hideSeekBar() {
        if (mAdjustmentValueIndicator != null) {
            mAdjustmentValueIndicator.setVisibility(View.GONE);
        }
    }

    /**
     * follow is the seekbar's :min  ~ max
     * but UI is               :max  ~ min
     */
    private OnSeekBarChangeListener mHorientiaonlSeekBarLisenter = new OnSeekBarChangeListener() {

        @Override
        public void onStopTrackingTouch(SeekBar seekBar) {
            // current the effect value is parameters style{-4 ~ 4},not progress
            // value style {0 ~8}
            Log.d(TAG, "[onStopTrackingTouch]index = " + mCurrentViewIndex
                    + ",Progress value is = " + mEffectsValue);
            mListener
                    .setVFBSharedPrefences(
                            mCurrentViewIndex == FACE_BEAUTY_BEAUTY_SHAPE ?
                                            FaceBeautyParametersHelper.FACEBEAUTY_SLIM
                                    : mCurrentViewIndex, mEffectsValue);
            // Because current the value is ps style,so need change to progress
            // value
            updateEffectsChache(Integer.valueOf(mEffectsValue));
        }

        @Override
        public void onStartTrackingTouch(SeekBar seekBar) {

        }

        @Override
        public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
            Log.d(TAG, "[onProgressChanged]progress is  =" + progress);
            mAdjustmentValueIndicator.setProgress(progress);
            mCurrentViewIndex = mCurrentViewIndex % SUPPORTED_FB_EFFECTS_NUMBER;
            setEffectsValueParameters(progress);
        }
    };

    private void updateEffectsChache(int value) {
        if (mEffectsKey != null && mCurrentViewIndex >= 0
                && mCurrentViewIndex < SUPPORTED_FB_EFFECTS_NUMBER
                && value != mFaceBeautyPropertiesValue.get(mCurrentViewIndex)) {
            mFaceBeautyPropertiesValue.set(mCurrentViewIndex, value);
        }
        Log.d(TAG, "[updateEffectsChache],targetValue = " + value);
    }

    private void setEffectsValueParameters(int progress) {
        mEffectsValue = Integer.toString(convertToParamertersValue(progress));
        Log.d(TAG, "[setEffectsValueParameters] progress = " + progress
                + ",mCurrentViewIndex = " + mCurrentViewIndex
                + ",will set parameters value = " + mEffectsValue);
        // set the value to parameters to devices
        mListener
                .setParameters(
                        mCurrentViewIndex == FACE_BEAUTY_BEAUTY_SHAPE ?
                                        FaceBeautyParametersHelper.FACEBEAUTY_SLIM
                                : mCurrentViewIndex, mEffectsValue);
    }

    private void prepareVFB() {
        Log.i(TAG, "[prepareVFB]");
        if (mListener.isMultiFbMode()) {
            SUPPORTED_FB_EFFECTS_NUMBER = SUPPORTED_FB_PROPERTIES_MIN_NUMBER;
        } else {
            SUPPORTED_FB_EFFECTS_NUMBER = SUPPORTED_FB_PROPERTIES_MAX_NUMBER;
        }
        // first need clear the effects value;
        mFaceBeautyPropertiesValue.clear();
        int index = -1;
        for (int i = 0; i < SUPPORTED_FB_EFFECTS_NUMBER; i++) {
            // Because in ParametersHelper,slim's index is 4,not use the index
            // of 2[SHAPE]
            // so when the index is 2 mean's slim
            index = i;
            if (2 == i) {
                index = index + 2;
            }
            int value = mListener.getvFbSharedPreferences(index);
            mFaceBeautyPropertiesValue.add(value);

            // Be Care Full [Need Improve][TODO]
            // the i is the parameter's index,must be match with the
            // getvFbSharedPreferences(index);
            mListener.setParameters(i, Integer.toString(value));
        }
        // get the supported max effects
        mSupportedMaxValue = mListener.getMaxLevel(index);
        // set the effects duration: Max - Min
        mSupportedDuration = mSupportedMaxValue - mListener.getMinLevel(index);
    }

    // 5s timeout mechanism
    // This is used to the 5s timeout mechanism Read
    protected class IndicatorHandler extends Handler {
        public IndicatorHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            Log.i(TAG, "[handleMessage]msg.what = " + msg.what
                    + ", mIsTimeOutMechanismRunning = "
                    + mIsTimeOutMechanismRunning);
            if (!mIsTimeOutMechanismRunning) {
                Log.i(TAG, "Time out mechanism not running ,so return ");
                return;
            }
            switch (msg.what) {
            case DISAPPEAR_VFB_UI:
                hide();
                mICameraAppUi.changeBackToVFBModeStatues(true);
                mICameraAppUi.showAllViews();
                mICameraAppUi.setCurrentMode(CameraModeType.EXT_MODE_PHOTO);
                break;

            default:
                break;
            }
        }
    }

    private void removeBackToNormalMsg() {
        Log.i(TAG, "[removeMsg]:DISAPPEAR_VFB_UI");
        if (mHandler != null) {
            mHandler.removeMessages(DISAPPEAR_VFB_UI);
            mIsTimeOutMechanismRunning = false;
        }
    }
}
