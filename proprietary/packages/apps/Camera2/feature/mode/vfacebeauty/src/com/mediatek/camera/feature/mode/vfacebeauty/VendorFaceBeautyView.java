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
 *     MediaTek Inc. (C) 2019. All rights reserved.
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

package com.mediatek.camera.feature.mode.vfacebeauty;

import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.support.annotation.NonNull;
import android.support.v4.content.ContextCompat;
import android.view.View;
import android.view.ViewGroup;
import android.widget.LinearLayout;
import android.widget.SeekBar;

import com.mediatek.camera.R;
import com.mediatek.camera.common.IAppUi;
import com.mediatek.camera.common.ICameraContext;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.relation.DataStore;
import com.mediatek.camera.feature.mode.facebeauty.utils.Util;
import com.mediatek.camera.feature.mode.facebeauty.widget.RotateImageView;
import com.mediatek.camera.feature.mode.facebeauty.widget.VerticalSeekBar;
import com.mediatek.campostalgo.FeatureParam;
import com.mediatek.campostalgo.FeaturePipeConfig;

import java.util.ArrayList;

import static com.mediatek.camera.feature.mode.vfacebeauty.VendorFaceBeautyDeviceController.MTK_POSTALGO_VENDOR_FACE_BEAUTY_BEAUTY_ALL;
import static com.mediatek.camera.feature.mode.vfacebeauty.VendorFaceBeautyDeviceController.MTK_POSTALGO_VENDOR_FACE_BEAUTY_BIG_EYE;
import static com.mediatek.camera.feature.mode.vfacebeauty.VendorFaceBeautyDeviceController.MTK_POSTALGO_VENDOR_FACE_BEAUTY_BIG_NOSE;
import static com.mediatek.camera.feature.mode.vfacebeauty.VendorFaceBeautyDeviceController.MTK_POSTALGO_VENDOR_FACE_BEAUTY_BRIGHT_EYE;
import static com.mediatek.camera.feature.mode.vfacebeauty.VendorFaceBeautyDeviceController.MTK_POSTALGO_VENDOR_FACE_BEAUTY_BRIGHT_WHITE;
import static com.mediatek.camera.feature.mode.vfacebeauty.VendorFaceBeautyDeviceController.MTK_POSTALGO_VENDOR_FACE_BEAUTY_PHYSICAL_ID;
import static com.mediatek.camera.feature.mode.vfacebeauty.VendorFaceBeautyDeviceController.MTK_POSTALGO_VENDOR_FACE_BEAUTY_SMALL_FACE;
import static com.mediatek.camera.feature.mode.vfacebeauty.VendorFaceBeautyDeviceController.MTK_POSTALGO_VENDOR_FACE_BEAUTY_SMOOTH_FACE;

public class VendorFaceBeautyView implements View.OnClickListener {
    private static final LogUtil.Tag TAG
                             = new LogUtil.Tag(VendorFaceBeautyView.class.getSimpleName());
    private View mRootView;
    private View mFaceView;
    private ViewGroup mParentViewGroup;

    private int mCameraId = 0;
    private ICameraContext mICameraContext;

    private IApp mApp;

    private String mEffectsKey = null;
    private String mEffectsValue = "-1";

    private VendorFaceBeautyInfo mFaceBeautyInfo;

    private int mSupportedDuration = 100;
    private int mSupportedMaxValue = 0;
    private int mCurrentViewIndex = 0;

    private ArrayList<Integer> mFaceBeautyPropertiesValue = new ArrayList<Integer>();

    private LinearLayout mBgLinearLayout;
    private VerticalSeekBar mAdjustmentValueIndicator;
    private RotateImageView[] mFaceBeautyImageViews = new RotateImageView[NUMBER_FACE_BEAUTY_ICON];

    private static final int FACE_BEAUTY_SMALL_FACE = 0;
    private static final int FACE_BEAUTY_BIG_EYE = 1;
    private static final int FACE_BEAUTY_BRIGHT_EYE = 2;
    private static final int FACE_BEAUTY_BIG_NOSE = 3;
    private static final int FACE_BEAUTY_SMOOTH_FACE = 4;
    private static final int FACE_BEAUTY_BRIGHT_WHITE = 5;
    private static final int FACE_BEAUTY_BEAUTY_ALL = 6;
    private static final int FACE_BEAUTY_MODIFY_ICON = 7;
    private static final int FACE_BEAUTY_ICON = 8;


    private static final int SUPPORTED_FB_PROPERTIES_MAX_NUMBER = 4;

    // use for hander the effects item hide
    private static final int DISAPPEAR_VFB_UI_TIME = 5000;
    private static final int DISAPPEAR_VFB_UI = 0;

    // Decoupling: 4 will be replaced by parameters
    private int SUPPORTED_FB_EFFECTS_NUMBER = 7;

    // 6 means all the number of icons in the preview
    private static final int NUMBER_FACE_BEAUTY_ICON = 9;

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

    private Handler mHandler;
    private Handler mModeHandler;
    protected DataStore mDataStore;
    // add for vFB begin
    public static final String KEY_FACE_BEAUTY_SMALL_FACE = "pref_facebeauty_small_face_key";
    public static final String KEY_FACE_BEAUTY_BIG_EYE = "pref_facebeauty_big_eye_key";
    public static final String KEY_FACE_BEAUTY_BRIGHT_EYE = "pref_facebeauty_bright_eye_key";
    public static final String KEY_FACE_BEAUTY_BIG_NOSE = "pref_facebeauty_big_nose_key";
    public static final String KEY_FACE_BEAUTY_SMOOTH_FACE = "pref_facebeauty_smooth_face_key";
    public static final String KEY_FACE_BEAUTY_BRIGHT_WHITE = "pref_facebeauty_bright_white_key";
    public static final String KEY_FACE_BEAUTY_BEAUTY_ALL = "pref_facebeauty_beauty_all_key";

    public static final String KEY_CAMERA_FACE_BEAUTY_MULTI_MODE_KEY =
            "pref_face_beauty_multi_mode_key";

    private static final int[] FACE_BEAUTY_ICONS_NORMAL = new int[NUMBER_FACE_BEAUTY_ICON];
    private static final int[] FACE_BEAUTY_ICONS_HIGHTLIGHT = new int[NUMBER_FACE_BEAUTY_ICON];

    static {
        FACE_BEAUTY_ICONS_NORMAL[FACE_BEAUTY_SMALL_FACE] = R.drawable.ic_fb_3_slimfacelevel;
        FACE_BEAUTY_ICONS_NORMAL[FACE_BEAUTY_BIG_EYE] = R.drawable.ic_fb_3_enlargeeyelevel;
        FACE_BEAUTY_ICONS_NORMAL[FACE_BEAUTY_BRIGHT_EYE] = R.drawable.ic_fb_3_bright_eyes;
        FACE_BEAUTY_ICONS_NORMAL[FACE_BEAUTY_BIG_NOSE] = R.drawable.ic_fb_3_nose;
        FACE_BEAUTY_ICONS_NORMAL[FACE_BEAUTY_SMOOTH_FACE] = R.drawable.ic_fb_3_microdermabrasion;
        FACE_BEAUTY_ICONS_NORMAL[FACE_BEAUTY_BRIGHT_WHITE] = R.drawable.ic_fb_3_brightlevel;
        FACE_BEAUTY_ICONS_NORMAL[FACE_BEAUTY_BEAUTY_ALL] = R.drawable.ic_fb_3_complex;
        FACE_BEAUTY_ICONS_NORMAL[FACE_BEAUTY_MODIFY_ICON] = R.drawable.ic_fb_3_hide_on;
        FACE_BEAUTY_ICONS_NORMAL[FACE_BEAUTY_ICON] = R.drawable.ic_mode_facebeauty_normal;
    }

    static {
        FACE_BEAUTY_ICONS_HIGHTLIGHT[FACE_BEAUTY_SMALL_FACE]
                    = R.drawable.ic_fb_3_slimfacelevel_highlight;
        FACE_BEAUTY_ICONS_HIGHTLIGHT[FACE_BEAUTY_BIG_EYE]
                    = R.drawable.ic_fb_3_enlargeeyelevel_highlight;
        FACE_BEAUTY_ICONS_HIGHTLIGHT[FACE_BEAUTY_BRIGHT_EYE]
                    = R.drawable.ic_fb_3_bright_eyes_highlight;
        FACE_BEAUTY_ICONS_HIGHTLIGHT[FACE_BEAUTY_BIG_NOSE] = R.drawable.ic_fb_3_nose_highlight;
        FACE_BEAUTY_ICONS_HIGHTLIGHT[FACE_BEAUTY_SMOOTH_FACE]
                    = R.drawable.ic_fb_3_microdermabrasion_highlight;
        FACE_BEAUTY_ICONS_HIGHTLIGHT[FACE_BEAUTY_BRIGHT_WHITE]
                    = R.drawable.ic_fb_3_brightlevel_highlight;
        FACE_BEAUTY_ICONS_HIGHTLIGHT[FACE_BEAUTY_BEAUTY_ALL] = R.drawable.ic_fb_3_complex_highlight;
        FACE_BEAUTY_ICONS_HIGHTLIGHT[FACE_BEAUTY_MODIFY_ICON]
                    = R.drawable.ic_fb_3_hide_off_highlight;
        FACE_BEAUTY_ICONS_HIGHTLIGHT[FACE_BEAUTY_ICON] = R.drawable.ic_mode_facebeauty_focus;
    }

    public static final int VALUE_FACE_BEAUTY_NO_EFFECTS = -1;

    private FeatureParam param;

    /**
     * The constructor of panorama view.
     *
     * @param app      the instance of IApp.
     * @param cameraId the camera id.
     */
    public VendorFaceBeautyView(@NonNull IApp app, int cameraId, ICameraContext mICameraContext
            , Handler mModeHandler) {
        LogHelper.d(TAG, "[FaceBeautyView]constructor...");
        mApp = app;
        mCameraId = cameraId;
        mParentViewGroup = app.getAppUi().getModeRootView();
        mHandler = new IndicatorHandler(mApp.getActivity().getMainLooper());
        this.mModeHandler = mModeHandler;
        this.mICameraContext = mICameraContext;
        mDataStore = mICameraContext.getDataStore();
    }

    /**
     * init FaceBeauty view.
     */
    public void init() {
        LogHelper.i(TAG, "[init]");
        getView();
    }

    /**
     * show FaceBeauty view.
     */
    public void show() {
        LogHelper.i(TAG, "[show]");
        if (mRootView == null) {
            mRootView = getView();
        }
        mRootView.setVisibility(View.VISIBLE);
        mBgLinearLayout.setVisibility(View.VISIBLE);
        mApp.getAppUi().applyAllUIVisibility(View.VISIBLE);
        mApp.getAppUi().setUIVisibility(IAppUi.SCREEN_HINT, View.VISIBLE);
        LogHelper.i(TAG, "[show]...,mIsShowSetting = " + mIsShowSetting
                + ",mIsInCameraPreview = " + mIsInCameraPreview);
        if (!mIsShowSetting && mIsInCameraPreview) {
            intoVfbMode();
        }
    }

    /**
     * hide FaceBeauty view.
     */
    public void hide() {
        LogHelper.i(TAG, "[hide]");
        if (mRootView == null) {
            return;
        }
        mRootView.setVisibility(View.GONE);
        mApp.getAppUi().applyAllUIVisibility(View.VISIBLE);
    }

    /**
     * 186       * will be called when app call release() to unload views from view
     * 187       * hierarchy.
     * 188
     */
    public void unInit() {
        LogHelper.i(TAG, "[unInit]");
        if (mParentViewGroup != null) {
            mParentViewGroup.removeView(mRootView);
            mRootView = null;
            mParentViewGroup = null;
        }
    }

    /**
     * reset FaceBeauty view.
     */
    public void reset() {
        LogHelper.i(TAG, "[reset] mRootView = " + mRootView + ",mPanoView = " + mFaceView);
        if (mRootView == null) {
            return;
        }
    }


    public void setmCameraId(int mCameraId) {
        this.mCameraId = mCameraId;
    }

    public boolean update(int type, Object... args) {
        if (VendorFaceBeautyMode.INFO_FACE_DETECTED != type
                && VendorFaceBeautyMode.ORIENTATION_CHANGED != type) {
            LogHelper.i(TAG, "[update] type = " + type);
        }
        boolean value = false;
        switch (type) {

            case VendorFaceBeautyMode.ON_CAMERA_CLOSED:
                // when back to camera, the auto back to photoMode not need
                removeBackToNormalMsg();
                break;

            case VendorFaceBeautyMode.ON_CAMERA_PARAMETERS_READY:
                prepareVFB();
                break;

            case VendorFaceBeautyMode.INFO_FACE_DETECTED:
                updateUI((Integer) args[0]);
                break;

            case VendorFaceBeautyMode.ORIENTATION_CHANGED:
                Util.setOrientation(mRootView, (Integer) args[0], true);
                if (mFaceBeautyInfo != null) {
                    mFaceBeautyInfo.onOrientationChanged((Integer) args[0]);
                }
                break;

            case VendorFaceBeautyMode.ON_FULL_SCREEN_CHANGED:
                mIsInCameraPreview = (Boolean) args[0];
                LogHelper.i(TAG, "ON_FULL_SCREEN_CHANGED, mIsInCameraPreview = "
                        + mIsInCameraPreview);
                if (mIsInCameraPreview) {
                    show();
                } else {
                    // because when effect is showing, we have hide the ALLViews,so
                    // need show the views
                    // otherwise back to Camera,you will found all the UI is hide
                    if (isEffectsShowing()) {
                        mApp.getAppUi().applyAllUIVisibility(View.VISIBLE);
                    }
                    hide();
                }
                break;

            case VendorFaceBeautyMode.ON_BACK_PRESSED:
                if (isEffectsShowing()) {
                    onModifyIconClick();
                    value = true;
                } else {
                    // when back to camera, the auto back to photoMode not need
                    removeBackToNormalMsg();
                }
                break;

            case VendorFaceBeautyMode.HIDE_EFFECTS_ITEM:
                if (isEffectsShowing()) {
                    onModifyIconClick();
                }
                break;

            case VendorFaceBeautyMode.ON_SETTING_BUTTON_CLICK:
                mIsShowSetting = (Boolean) args[0];

                LogHelper.i(TAG, "ON_SETTING_BUTTON_CLICK,mIsShowSetting =  " + mIsShowSetting);

                if (mIsShowSetting) {
                    hide();
                } else {
                    show();
                }
                break;

            case VendorFaceBeautyMode.ON_LEAVE_FACE_BEAUTY_MODE:
                hide();
                unInit();
                break;

            case VendorFaceBeautyMode.REMVOE_BACK_TO_NORMAL:
                // this case also need reset the automatic back to VFB mode
                removeBackToNormalMsg();
                break;

            case VendorFaceBeautyMode.ON_SELFTIMER_CAPTUEING:
                LogHelper.i(TAG, "[ON_SELFTIMER_CAPTUEING] args[0] = "
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

            case VendorFaceBeautyMode.IN_PICTURE_TAKEN_PROGRESS:
                mIsInPictureTakenProgress = (Boolean) args[0];
                LogHelper.i(TAG, "mIsInPictureTakenProgress = " + mIsInPictureTakenProgress
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

    /**
     * will be called if app want to show current view which hasn't been
     * created.
     *
     * @return
     */
    private View getView() {
        View viewLayout
              = mApp.getActivity().getLayoutInflater().inflate(R.layout.vendor_facebeauty_indicator,
                                                               mParentViewGroup, true);
        mRootView = viewLayout.findViewById(R.id.facebeauty_indicator);
        initializeViewManager();
        return mRootView;
    }

    private void initializeViewManager() {
        mBgLinearLayout = (LinearLayout) mRootView.findViewById(R.id.effcts_bg);
        mFaceBeautyImageViews[FACE_BEAUTY_ICON] = (RotateImageView) mRootView
                .findViewById(R.id.facebeauty_icon);
        mFaceBeautyImageViews[FACE_BEAUTY_MODIFY_ICON] = (RotateImageView) mRootView
                .findViewById(R.id.facebeauty_modify);
        mFaceBeautyImageViews[FACE_BEAUTY_SMALL_FACE] = (RotateImageView) mRootView
                .findViewById(R.id.facebeauty_small_face);
        mFaceBeautyImageViews[FACE_BEAUTY_BIG_EYE] = (RotateImageView) mRootView
                .findViewById(R.id.facebeauty_big_eye);
        mFaceBeautyImageViews[FACE_BEAUTY_BRIGHT_EYE] = (RotateImageView) mRootView
                .findViewById(R.id.facebeauty_bright_eye);
        mFaceBeautyImageViews[FACE_BEAUTY_BIG_NOSE] = (RotateImageView) mRootView
                .findViewById(R.id.facebeauty_big_nose);
        mFaceBeautyImageViews[FACE_BEAUTY_SMOOTH_FACE] = (RotateImageView) mRootView
                .findViewById(R.id.facebeauty_smooth_face);
        mFaceBeautyImageViews[FACE_BEAUTY_BRIGHT_WHITE] = (RotateImageView) mRootView
                .findViewById(R.id.facebeauty_bright_white);
        mFaceBeautyImageViews[FACE_BEAUTY_BEAUTY_ALL] = (RotateImageView) mRootView
                .findViewById(R.id.facebeauty_beauty_all);
        mAdjustmentValueIndicator = mRootView
                .findViewById(R.id.facebeauty_changevalue);
        mAdjustmentValueIndicator.setThumbSizePx(50, 50);
        mAdjustmentValueIndicator.setOrientation(0);
        mAdjustmentValueIndicator.setUnSelectColor(ContextCompat.getColor(mApp.getActivity(),
                R.color.thumb_unSelected));
        mAdjustmentValueIndicator.setSelectColor(ContextCompat.getColor(mApp.getActivity(),
                R.color.thumb_selected));
        applyListeners();
        mFaceBeautyInfo = new VendorFaceBeautyInfo(mApp.getActivity(), mApp);
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
        LogHelper.d(TAG, "[onClick]mCurrentViewIndex = " + mCurrentViewIndex);

        switch (mCurrentViewIndex) {
            case FACE_BEAUTY_SMALL_FACE:
                mEffectsKey = KEY_FACE_BEAUTY_SMALL_FACE;
                break;

            case FACE_BEAUTY_BIG_EYE:
                mEffectsKey = KEY_FACE_BEAUTY_BIG_EYE;
                break;

            case FACE_BEAUTY_BRIGHT_EYE:
                mEffectsKey = KEY_FACE_BEAUTY_BRIGHT_EYE;
                break;

            case FACE_BEAUTY_BIG_NOSE:
                mEffectsKey = KEY_FACE_BEAUTY_BIG_NOSE;
                break;

            case FACE_BEAUTY_SMOOTH_FACE:
                mEffectsKey = KEY_FACE_BEAUTY_SMOOTH_FACE;
                break;

            case FACE_BEAUTY_BRIGHT_WHITE:
                mEffectsKey = KEY_FACE_BEAUTY_BRIGHT_WHITE;
                break;

            case FACE_BEAUTY_BEAUTY_ALL:
                mEffectsKey = KEY_FACE_BEAUTY_BEAUTY_ALL;
                break;

            case FACE_BEAUTY_MODIFY_ICON:
                onModifyIconClick();
                break;

            case FACE_BEAUTY_ICON:
                onFaceBeautyIconClick();
                break;

            default:
                LogHelper.i(TAG, "[onClick]click is not the facebeauty imageviews,need check");
                break;
        }
        // current not show the toast of the view
        showEffectsToast(view, mCurrentViewIndex);
        //
        mModeHandler.post(new Runnable() {
            @Override
            public void run() {
                onEffectsIconClick();
            }
        });

    }

    private void applyListeners() {
        for (int i = 0; i < NUMBER_FACE_BEAUTY_ICON; i++) {
            if (null != mFaceBeautyImageViews[i]) {
                mFaceBeautyImageViews[i].setOnClickListener(this);
            }
        }
        if (mAdjustmentValueIndicator != null) {
            mAdjustmentValueIndicator
                    .setOnSlideChangeListener(mVerticalSeekBarSlideChangeListener);
        }
    }

    // when click the effects modify icon will run follow
    private void onModifyIconClick() {
        LogHelper.d(TAG, "[onModifyIconClick],isFaceBeautyEffectsShowing = "
                + isEffectsShowing());
        if (isEffectsShowing()) {
            // if current is showing and click the modify icon,need hide the
            // common views ,such as ModePicker/thumbnail/picker/settings item
            //mICameraAppUi.setViewState(ViewState.VIEW_STATE_NORMAL);
            mApp.getAppUi().applyAllUIVisibility(View.VISIBLE);
            hideEffectsIconAndSeekBar();
        } else {
            LogHelper.i(TAG, "onModifyIconClick, AppUI = " + mApp.getAppUi());
            //mApp.getAppUi().applyAllUIVisibility(View.INVISIBLE);
            //mICameraAppUi.setViewState(ViewState.VIEW_STATE_HIDE_ALL_VIEW);
            if (mBgLinearLayout != null) {
                mBgLinearLayout.setBackgroundResource(R.drawable.bg_icon);
            }
            showFaceBeautyEffects();
            // initialize the parameters
            mEffectsKey = KEY_FACE_BEAUTY_SMALL_FACE;
            // show default string
            showEffectsToast(mFaceBeautyImageViews[FACE_BEAUTY_SMALL_FACE],
                    FACE_BEAUTY_SMALL_FACE);
            // need set current values
            setProgressValue(mFaceBeautyPropertiesValue
                    .get(FACE_BEAUTY_SMALL_FACE));
        }
    }

    private boolean isEffectsShowing() {
        boolean isEffectsShowing = View.VISIBLE == mFaceBeautyImageViews[FACE_BEAUTY_SMALL_FACE]
                .getVisibility();
        LogHelper.d(TAG, "isEffectsShowing = " + isEffectsShowing);

        return isEffectsShowing;
    }

    // when click the effects icon will run follow
    private void onEffectsIconClick() {
        if (mCurrentViewIndex == FACE_BEAUTY_BEAUTY_ALL) {
            configBeautyAllMetaParams(mFaceBeautyPropertiesValue.get(mCurrentViewIndex));
        } else if (mCurrentViewIndex < SUPPORTED_FB_EFFECTS_NUMBER) {
            configMetaParams(getmEffectsKey(mCurrentViewIndex),
                    mFaceBeautyPropertiesValue.get(mCurrentViewIndex));
        }
    }

    private void hideEffectsIconAndSeekBar() {
        LogHelper.d(TAG, "[hideEffectsIconAndSeekBar]mSupporteFBEffectsNumber = "
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
        LogHelper.d(TAG, "[showFaceBeautyEffects]...");
        // default first effects is wrinkle Remove effects
        mFaceBeautyImageViews[FACE_BEAUTY_SMALL_FACE]
                .setImageResource(FACE_BEAUTY_ICONS_HIGHTLIGHT[FACE_BEAUTY_SMALL_FACE]);
        mFaceBeautyImageViews[FACE_BEAUTY_SMALL_FACE]
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
             i < SUPPORTED_FB_PROPERTIES_MAX_NUMBER; i++) {
            mFaceBeautyImageViews[i].setVisibility(View.GONE);
        }
        // also need to show SeekBar
        if (mAdjustmentValueIndicator != null) {
            mAdjustmentValueIndicator.setMaxProgress(mSupportedDuration);
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
        LogHelper.d(TAG, "[hideToast()]");
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
        return value;
    }

    private void onFaceBeautyIconClick() {
        LogHelper.d(TAG, "[onFaceBeautyIconClick]isFaceBeautyModifyIconShowing = "
                + isModifyIconShowing());
        if (!isModifyIconShowing()) {
            intoVfbMode();
        } else {
            leaveVfbMode();
        }
    }

    private void intoVfbMode() {
        LogHelper.d(TAG, "[intoVfbMode]");
        mFaceBeautyImageViews[FACE_BEAUTY_MODIFY_ICON]
                .setImageResource(FACE_BEAUTY_ICONS_NORMAL[FACE_BEAUTY_MODIFY_ICON]);
        updateModifyIconStatus(true);
        mFaceBeautyImageViews[FACE_BEAUTY_ICON]
                .setImageResource(FACE_BEAUTY_ICONS_HIGHTLIGHT[FACE_BEAUTY_ICON]);
        mFaceBeautyImageViews[FACE_BEAUTY_ICON].setVisibility(View.VISIBLE);
        /*update(FaceBeautyMode.ORIENTATION_CHANGED,
                CameraUtil.getDisplayRotation(mApp.getActivity()));*/
        hideEffectsItems();
        hideSeekBar();
    }

    private void leaveVfbMode() {
        // when isFaceBeautyModifyIconShowing = true,means the icon is
        // showing ,need hide the
        // face beauty effects and modify values Seekbar
        LogHelper.d(TAG, "[leaveVfbMode]");
        updateModifyIconStatus(false);
        mFaceBeautyImageViews[FACE_BEAUTY_ICON]
                .setImageResource(FACE_BEAUTY_ICONS_NORMAL[FACE_BEAUTY_ICON]);
        mFaceBeautyImageViews[FACE_BEAUTY_ICON].setVisibility(View.VISIBLE);
        hideEffectsIconAndSeekBar();
        //mICameraAppUi.setCurrentMode(CameraModeType.EXT_MODE_PHOTO);
    }

    private void updateUI(int length) {
        // LogHelper.i(TAG, "[updateUI],face length = " + length);
        /*boolean enable = mListener.canShowFbIcon(length);
        if (enable) {
            mIsTimeOutMechanismRunning = false;
            mView.setEnabled(true);
            showFaceBeautyIcon();
            mHandler.removeMessages(DISAPPEAR_VFB_UI);
        } else if (length == 0 && !mIsTimeOutMechanismRunning) {
            if (isModifyIconShowing()) {
                if (mFaceBeautyImageViews[FACE_BEAUTY_MODIFY_ICON].isEnabled()) {
                    LogHelper.i(TAG, "will send msg: DISAPPEAR_VFB_UI");
                    mView.setEnabled(false);
                    mHandler.removeMessages(DISAPPEAR_VFB_UI);
                    mHandler.sendEmptyMessageDelayed(DISAPPEAR_VFB_UI,
                            DISAPPEAR_VFB_UI_TIME);
                    mIsTimeOutMechanismRunning = true;
                }
            } else {
                hideFaceBeautyIcon();
            }
        }*/
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
            LogHelper.d(TAG, "showFaceBeautyIcon....");
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
        LogHelper.d(TAG, "[isModifyIconShowing]isModifyIconShowing = "
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
        LogHelper.d(TAG, "[updateModifyIconStatus]isFaceBeautyModifyIconShowing = "
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

    private void updateEffectsChache(int value) {
        if (mEffectsKey != null && mCurrentViewIndex >= 0
                && mCurrentViewIndex < SUPPORTED_FB_EFFECTS_NUMBER
                && value != mFaceBeautyPropertiesValue.get(mCurrentViewIndex)) {
            mFaceBeautyPropertiesValue.set(mCurrentViewIndex, value);
        }
        LogHelper.d(TAG, "[updateEffectsChache],targetValue = " + value);
    }

    private void setEffectsValueParameters(int progress) {
        mEffectsValue = Integer.toString(convertToParamertersValue(progress));
        LogHelper.d(TAG, "[setEffectsValueParameters] progress = " + progress
                + ",mCurrentViewIndex = " + mCurrentViewIndex
                + ",will set parameters value = " + mEffectsValue);

        mModeHandler.post(new Runnable() {
            @Override
            public void run() {
                // set the value to parameters to devices
                configMetaParams(getmEffectsKey(mCurrentViewIndex), progress);
            }
        });
        mDataStore.setValue(getmEffectsKey(mCurrentViewIndex),
                Integer.toString(progress),
                mDataStore.getGlobalScope(), false);
    }

    private void prepareVFB() {
        LogHelper.i(TAG, "[prepareVFB]");
        /*if (mListener.isMultiFbMode()) {
            SUPPORTED_FB_EFFECTS_NUMBER = SUPPORTED_FB_PROPERTIES_MIN_NUMBER;
        } else {
            SUPPORTED_FB_EFFECTS_NUMBER = SUPPORTED_FB_PROPERTIES_MAX_NUMBER;
        }*/
        // first need clear the effects value;
        mFaceBeautyPropertiesValue.clear();
        for (int i = 0; i < SUPPORTED_FB_EFFECTS_NUMBER; i++) {
            int value = Integer.parseInt(mDataStore.getValue(getmEffectsKey(i),
                    "0", mDataStore.getGlobalScope()));
            mFaceBeautyPropertiesValue.add(value);
        }
        // get the supported max effects
        mSupportedMaxValue = 100;
        // set the effects duration: Max - Min
        mSupportedDuration = mSupportedMaxValue - 0;
    }

    // 5s timeout mechanism
    // This is used to the 5s timeout mechanism Read
    protected class IndicatorHandler extends Handler {
        public IndicatorHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            LogHelper.i(TAG, "[handleMessage]msg.what = " + msg.what
                    + ", mIsTimeOutMechanismRunning = "
                    + mIsTimeOutMechanismRunning);
            if (!mIsTimeOutMechanismRunning) {
                LogHelper.i(TAG, "Time out mechanism not running ,so return ");
                return;
            }
            switch (msg.what) {
                case DISAPPEAR_VFB_UI:
                    hide();
                    mApp.getAppUi().applyAllUIVisibility(View.VISIBLE);
                    //mICameraAppUi.changeBackToVFBModeStatues(true);
                    //mICameraAppUi.showAllViews();
                    //mICameraAppUi.setCurrentMode(CameraModeType.EXT_MODE_PHOTO);
                    break;

                default:
                    break;
            }
        }
    }

    private void removeBackToNormalMsg() {
        LogHelper.i(TAG, "[removeMsg]:DISAPPEAR_VFB_UI");
        if (mHandler != null) {
            mHandler.removeMessages(DISAPPEAR_VFB_UI);
            mIsTimeOutMechanismRunning = false;
        }
    }

    private void configMetaParams(String type, int value) {
        if (type.equals(MTK_POSTALGO_VENDOR_FACE_BEAUTY_BEAUTY_ALL)) {
            configBeautyAllMetaParams(value);
        } else {
            param = new FeatureParam();
            param.appendInt(type, value);
            param.appendInt(MTK_POSTALGO_VENDOR_FACE_BEAUTY_PHYSICAL_ID, mCameraId);
            param.appendInt(MTK_POSTALGO_VENDOR_FACE_BEAUTY_BEAUTY_ALL,
                    VALUE_FACE_BEAUTY_NO_EFFECTS);
            mICameraContext.getCamPostAlgo().configParams(FeaturePipeConfig.INDEX_PREVIEW, param);
            mICameraContext.getCamPostAlgo().configParams(FeaturePipeConfig.INDEX_CAPTURE, param);
        }
    }

    private void configBeautyAllMetaParams(int beautyAllValue) {
        param = new FeatureParam();
        param.appendInt(MTK_POSTALGO_VENDOR_FACE_BEAUTY_SMALL_FACE, VALUE_FACE_BEAUTY_NO_EFFECTS);
        param.appendInt(MTK_POSTALGO_VENDOR_FACE_BEAUTY_BIG_EYE, VALUE_FACE_BEAUTY_NO_EFFECTS);
        param.appendInt(MTK_POSTALGO_VENDOR_FACE_BEAUTY_BRIGHT_EYE, VALUE_FACE_BEAUTY_NO_EFFECTS);
        param.appendInt(MTK_POSTALGO_VENDOR_FACE_BEAUTY_BIG_NOSE, VALUE_FACE_BEAUTY_NO_EFFECTS);
        param.appendInt(MTK_POSTALGO_VENDOR_FACE_BEAUTY_SMOOTH_FACE, VALUE_FACE_BEAUTY_NO_EFFECTS);
        param.appendInt(MTK_POSTALGO_VENDOR_FACE_BEAUTY_BRIGHT_WHITE, VALUE_FACE_BEAUTY_NO_EFFECTS);
        param.appendInt(MTK_POSTALGO_VENDOR_FACE_BEAUTY_BEAUTY_ALL,
                beautyAllValue);
        param.appendInt(MTK_POSTALGO_VENDOR_FACE_BEAUTY_PHYSICAL_ID, mCameraId);
        mICameraContext.getCamPostAlgo().configParams(FeaturePipeConfig.INDEX_PREVIEW, param);
        mICameraContext.getCamPostAlgo().configParams(FeaturePipeConfig.INDEX_CAPTURE, param);
    }

    private String getmEffectsKey(int index) {
        switch (index) {
            case FACE_BEAUTY_SMALL_FACE:
                return MTK_POSTALGO_VENDOR_FACE_BEAUTY_SMALL_FACE;
            case FACE_BEAUTY_BIG_EYE:
                return MTK_POSTALGO_VENDOR_FACE_BEAUTY_BIG_EYE;
            case FACE_BEAUTY_BRIGHT_EYE:
                return MTK_POSTALGO_VENDOR_FACE_BEAUTY_BRIGHT_EYE;
            case FACE_BEAUTY_BIG_NOSE:
                return MTK_POSTALGO_VENDOR_FACE_BEAUTY_BIG_NOSE;
            case FACE_BEAUTY_SMOOTH_FACE:
                return MTK_POSTALGO_VENDOR_FACE_BEAUTY_SMOOTH_FACE;
            case FACE_BEAUTY_BRIGHT_WHITE:
                return MTK_POSTALGO_VENDOR_FACE_BEAUTY_BRIGHT_WHITE;
            case FACE_BEAUTY_BEAUTY_ALL:
                return MTK_POSTALGO_VENDOR_FACE_BEAUTY_BEAUTY_ALL;
            default:
                return MTK_POSTALGO_VENDOR_FACE_BEAUTY_SMALL_FACE;
        }
    }

    private VerticalSeekBar.SlideChangeListener mVerticalSeekBarSlideChangeListener
            = new VerticalSeekBar.SlideChangeListener() {
        @Override
        public void onStart(VerticalSeekBar slideView, int progress) {

        }

        @Override
        public void onProgress(VerticalSeekBar slideView, int progress) {
            LogHelper.d(TAG, "[onProgressChanged]progress is  =" + progress);
            mAdjustmentValueIndicator.setProgress(progress);
            mCurrentViewIndex = mCurrentViewIndex % SUPPORTED_FB_EFFECTS_NUMBER;
            if (progress % 20 == 0)
                setEffectsValueParameters(progress);
        }

        @Override
        public void onStop(VerticalSeekBar slideView, int progress) {
            LogHelper.d(TAG, "[onStopTrackingTouch]index = " + mCurrentViewIndex
                    + ",Progress value is = " + mEffectsValue);
            updateEffectsChache(Integer.valueOf(mEffectsValue));
        }
    };

}
