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

package com.mediatek.camera.feature.mode.panorama;

import android.content.pm.ActivityInfo;
import android.graphics.Matrix;
import android.view.View;
import android.view.ViewGroup;

import com.mediatek.camera.R;
import com.mediatek.camera.common.IAppUi;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.utils.CameraUtil;
import com.mediatek.camera.common.widget.Rotatable;
import com.mediatek.camera.common.widget.RotateImageView;
import com.mediatek.camera.common.widget.RotateLayout;
import com.mediatek.camera.common.widget.RotateLayout.OnSizeChangedListener;

import javax.annotation.Nonnull;

/**
 * The view for panorama feature.
 */
public class PanoramaView {
    private static final LogUtil.Tag TAG = new LogUtil.
            Tag(PanoramaView.class.getSimpleName());
    private static final int SHOW_INFO_LENGTH_LONG = 5 * 1000;

    private RotateLayout mScreenProgressLayout;
    private NaviLineImageView mNaviLine;
    private ProgressIndicator mProgressIndicator;
    private AnimationController mAnimationController;
    private RotateImageView mCancelButton;
    private RotateImageView mSaveButton;

    private View mRootView;
    private View mPanoView;
    private ViewGroup mParentViewGroup;
    private ViewGroup mDirectionSigns[] = new ViewGroup[4]; // up,down,left,right
    private ViewGroup mCenterIndicator;
    private ViewGroup mCollimatedArrowsDrawable;

    private static final boolean ANIMATION = true;
    private boolean mIsCapturing = false;

    private Matrix mSensorMatrix[];
    private Matrix mDisplayMatrix = new Matrix();

    private static final int DIRECTION_RIGHT = 0;
    private static final int DIRECTION_LEFT = 1;
    private static final int DIRECTION_UP = 2;
    private static final int DIRECTION_DOWN = 3;
    private static final int DIRECTION_UNKNOWN = 4;

    private static final int TARGET_DISTANCE_HORIZONTAL = 160;
    private static final int TARGET_DISTANCE_VERTICAL = 120;

    private static final int[] DIRECTIONS = {DIRECTION_RIGHT, DIRECTION_DOWN, DIRECTION_LEFT,
            DIRECTION_UP};
    private static final int DIRECTIONS_COUNT = DIRECTIONS.length;

    private int mSensorDirection = DIRECTION_UNKNOWN;
    private int mDisplayOrientation;

    private int mHalfArrowHeight = 0;
    private int mHalfArrowLength = 0;

    private int mPreviewWidth = 0;
    private int mPreviewHeight = 0;

    private static final int BLOCK_NUM = 9;
    private int mBlockSizes[] = {17, 15, 13, 12, 11, 12, 13, 15, 17};

    private int mDistanceHorizontal = 0;
    private int mDistanceVertical = 0;
    private int mCameraId = 0;

    private IApp mApp;
    private OnSaveButtonClickedListener mOnSaveButtonClickedListener;
    private OnCancelButtonClickedListener mOnCancleButtonClickedListener;

    private IAppUi.HintInfo mGuideHint;

    /**
     * The constructor of panorama view.
     *
     * @param app      the instance of IApp.
     * @param cameraId the camera id.
     */
    public PanoramaView(@Nonnull IApp app, int cameraId) {
        LogHelper.d(TAG, "[PanoramaView]constructor...");
        mApp = app;
        mCameraId = cameraId;
        mParentViewGroup = mApp.getAppUi().getModeRootView();
    }

    /**
     * Init panorama view.
     */
    public void init() {
        LogHelper.i(TAG, "[init]");
        getView();
        mGuideHint = new IAppUi.HintInfo();
        int id = mApp.getActivity().getResources().getIdentifier("hint_text_background",
                "drawable", mApp.getActivity().getPackageName());
        mGuideHint.mBackground = mApp.getActivity().getDrawable(id);
        mGuideHint.mType = IAppUi.HintType.TYPE_AUTO_HIDE;
        mGuideHint.mDelayTime = SHOW_INFO_LENGTH_LONG;
    }

    /**
     * Show panorama view.
     */
    public void show() {
        LogHelper.i(TAG, "[show]");
        if (mRootView == null) {
            mRootView = getView();
        }
        int displayRotation = CameraUtil.getDisplayRotation(mApp.getActivity());
        int displayOrientation = CameraUtil.getDisplayOrientation(displayRotation,
                mCameraId, mApp.getActivity());
        int activityRequestOrientation = mApp.getActivity().getRequestedOrientation();
        if (ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE == activityRequestOrientation ||
                ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE == activityRequestOrientation) {
            mDisplayOrientation = displayOrientation + 90;
        } else {
            mDisplayOrientation = displayOrientation;
        }
        setSaveCancelButtonOrientation(mApp.getGSensorOrientation());
        mRootView.setVisibility(View.VISIBLE);
        showCaptureView();
        mApp.getAppUi().applyAllUIVisibility(View.INVISIBLE);
        mApp.getAppUi().setUIVisibility(IAppUi.SCREEN_HINT, View.VISIBLE);

    }

    /**
     * Hide panorama view.
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
     * will be called when app call release() to unload views from view
     * hierarchy.
     */
    public void unInit() {
        LogHelper.i(TAG, "[unInit]");
        if (mParentViewGroup != null) {
            mRootView = null;
            mParentViewGroup = null;
        }
        mApp.getAppUi().hideScreenHint(mGuideHint);
    }

    /**
     * reset Panorama view.
     */
    public void reset() {
        LogHelper.i(TAG, "[reset] mRootView = " + mRootView + ",mPanoView = " + mPanoView);
        if (mRootView == null) {
            return;
        }
        mPanoView.setVisibility(View.GONE);
        mAnimationController.stopCenterAnimation();
        mCenterIndicator.setVisibility(View.GONE);

        mSensorDirection = DIRECTION_UNKNOWN;
        mNaviLine.setVisibility(View.GONE);
        mCollimatedArrowsDrawable.setVisibility(View.GONE);
        for (int i = 0; i < 4; i++) {
            mDirectionSigns[i].setSelected(false);
            mDirectionSigns[i].setVisibility(View.VISIBLE);
        }
    }

    /**
     * A callback to be invoked when save button clicked.
     */
    interface OnSaveButtonClickedListener {
        /**
         * Called when save button clicked.
         */
        void onSaveButtonClicked();
    }

    /**
     * Set save button clicked listener.
     *
     * @param listener The listener need to be registered.
     */
    public void setSaveButtonClickedListener(OnSaveButtonClickedListener listener) {
        mOnSaveButtonClickedListener = listener;
        LogHelper.i(TAG, "mSaveButton= " + mSaveButton);
        mSaveButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                LogHelper.i(TAG, "save button clicked, mIsCapturing = " + mIsCapturing);
                if (mOnSaveButtonClickedListener != null && mIsCapturing) {
                    mOnSaveButtonClickedListener.onSaveButtonClicked();
                }
            }
        });
    }

    /**
     * A callback to be invoked when cancel button clicked.
     */
    interface OnCancelButtonClickedListener {
        /**
         * Called when cancel button clicked.
         */
        void onCancelButtonClicked();
    }

    /**
     * Set save cancel clicked listener.
     *
     * @param listener The listener need to be registered.
     */
    public void setCancelButtonClickedListener(OnCancelButtonClickedListener listener) {
        mOnCancleButtonClickedListener = listener;
        mCancelButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                LogHelper.i(TAG, "cancel button clicked, mIsCapturing = " + mIsCapturing);
                if (mOnCancleButtonClickedListener != null && mIsCapturing) {
                    mOnCancleButtonClickedListener.onCancelButtonClicked();
                }
            }
        });
    }

    /**
     * Update panorama view.
     *
     * @param type which action should update for panorama view.
     *             Panorama.INFO_UPDATE_PROGRESS,
     *             Panorama.INFO_UPDATE_MOVING,
     *             Panorama.INFO_START_ANIMATION,
     *             Panorama.INFO_IN_CAPTURING,
     *             Panorama.INFO_OUT_OF_CAPTURING.
     * @param args arguments needed for update action.
     */
    public void update(int type, Object... args) {
        LogHelper.d(TAG, "[update] type =" + type);
        switch (type) {
            case PanoramaMode.INFO_UPDATE_PROGRESS:
                int num = Integer.parseInt(args[0].toString());
                setViewsForNext(num);
                break;

            case PanoramaMode.INFO_UPDATE_MOVING:
                if (args[0] != null && args[1] != null && args[2] != null
                        && mRootView != null && mRootView.isShown()) {
                    int x = Integer.parseInt(args[0].toString());
                    int y = Integer.parseInt(args[1].toString());
                    int direction = Integer.parseInt(args[2].toString());
                    boolean show = Boolean.parseBoolean(args[3].toString());
                    updateMovingUI(x, y, direction, show);
                }
                break;

            case PanoramaMode.INFO_START_ANIMATION:
                startCenterAnimation();
                break;

            case PanoramaMode.INFO_IN_CAPTURING:
                mIsCapturing = true;
                break;

            case PanoramaMode.INFO_OUT_OF_CAPTURING:
                mIsCapturing = false;
                break;

            default:
                break;
        }
    }

    /**
     * show Guide view.
     *
     * @param step The step for guide ui.
     */
    public void showGuideView(int step) {
        int guideId = 0;
        switch (step) {
            case PanoramaMode.GUIDE_SHUTTER:
                guideId = R.string.panorama_guide_shutter;
                break;

            case PanoramaMode.GUIDE_MOVE:
                guideId = R.string.panorama_guide_choose_direction;
                break;

            default:
                break;
        }
        if (guideId != 0) {
            mGuideHint.mHintText = mApp.getActivity().getString(guideId);
            mApp.getAppUi().showScreenHint(mGuideHint);
        }
    }

    /**
     * will be called if app want to show current view which hasn't been
     * created.
     *
     * @return
     */
    private View getView() {
        View viewLayout = mApp.getActivity().getLayoutInflater().inflate(R.layout.pano_preview,
                mParentViewGroup, true);
        mRootView = viewLayout.findViewById(R.id.pano_frame_layout);
        initializeViewManager();
        return mRootView;
    }

    private void initializeViewManager() {
        mPanoView = mRootView.findViewById(R.id.pano_view);

        mScreenProgressLayout = (RotateLayout) mRootView.findViewById(R.id.on_screen_progress);
        mCenterIndicator = (ViewGroup) mRootView.findViewById(R.id.center_indicator);
        mDirectionSigns[DIRECTION_RIGHT] = (ViewGroup) mRootView.findViewById(R.id.pano_right);
        mDirectionSigns[DIRECTION_LEFT] = (ViewGroup) mRootView.findViewById(R.id.pano_left);
        mDirectionSigns[DIRECTION_UP] = (ViewGroup) mRootView.findViewById(R.id.pano_up);
        mDirectionSigns[DIRECTION_DOWN] = (ViewGroup) mRootView.findViewById(R.id.pano_down);
        mAnimationController = new AnimationController(mDirectionSigns,
                (ViewGroup) mCenterIndicator.getChildAt(0));
        mCancelButton = (RotateImageView) mRootView.findViewById(R.id.btn_pano_cancel);
        mSaveButton = (RotateImageView) mRootView.findViewById(R.id.btn_pano_save);

        mDistanceHorizontal = TARGET_DISTANCE_HORIZONTAL;
        mDistanceVertical = TARGET_DISTANCE_VERTICAL;
        mNaviLine = (NaviLineImageView) mRootView.findViewById(R.id.navi_line);
        mCollimatedArrowsDrawable = (ViewGroup) mRootView
                .findViewById(R.id.static_center_indicator);

        mProgressIndicator = new ProgressIndicator(mApp.getActivity(), BLOCK_NUM, mBlockSizes);
        mProgressIndicator.setVisibility(View.GONE);
        mScreenProgressLayout.setOrientation(mDisplayOrientation, true);

        prepareSensorMatrix();
        mScreenProgressLayout.setOnSizeChangedListener(mOnSizeChangedListener);
    }

    private void prepareSensorMatrix() {
        mSensorMatrix = new Matrix[4];

        mSensorMatrix[DIRECTION_LEFT] = new Matrix();
        mSensorMatrix[DIRECTION_LEFT].setScale(-1, -1);
        mSensorMatrix[DIRECTION_LEFT].postTranslate(0, mDistanceVertical);

        mSensorMatrix[DIRECTION_RIGHT] = new Matrix();
        mSensorMatrix[DIRECTION_RIGHT].setScale(-1, -1);
        mSensorMatrix[DIRECTION_RIGHT].postTranslate(mDistanceHorizontal * 2, mDistanceVertical);

        mSensorMatrix[DIRECTION_UP] = new Matrix();
        mSensorMatrix[DIRECTION_UP].setScale(-1, -1);
        mSensorMatrix[DIRECTION_UP].postTranslate(mDistanceHorizontal, 0);

        mSensorMatrix[DIRECTION_DOWN] = new Matrix();
        mSensorMatrix[DIRECTION_DOWN].setScale(-1, -1);
        mSensorMatrix[DIRECTION_DOWN].postTranslate(mDistanceHorizontal, mDistanceVertical * 2);
    }

    private void showCaptureView() {
        mCenterIndicator.setVisibility(View.GONE);
        mPanoView.setVisibility(View.VISIBLE);
        mProgressIndicator.setProgress(0);
        mProgressIndicator.setVisibility(View.VISIBLE);
    }

    private OnSizeChangedListener mOnSizeChangedListener = new OnSizeChangedListener() {
        @Override
        public void onSizeChanged(int width, int height) {
            LogHelper.d(TAG, "[onSizeChanged]width=" + width + " height=" + height);
            mPreviewWidth = Math.max(width, height);
            mPreviewHeight = Math.min(width, height);
        }
    };

    private void setViewsForNext(int imageNum) {
        mProgressIndicator.setProgress(imageNum + 1);

        if (imageNum == 0) {
            mAnimationController.startDirectionAnimation();
        } else {
            mNaviLine.setVisibility(View.INVISIBLE);
            mAnimationController.stopCenterAnimation();
            mCenterIndicator.setVisibility(View.GONE);
            mCollimatedArrowsDrawable.setVisibility(View.VISIBLE);
        }
    }

    private void updateMovingUI(int x, int y, int direction, boolean shown) {
        // direction means sensor towards.
        if (direction == DIRECTION_UNKNOWN || shown || mNaviLine.getWidth() == 0
                || mNaviLine.getHeight() == 0) {
            // if the NaviLine has not been drawn well, return.
            mNaviLine.setVisibility(View.INVISIBLE);
            return;
        }
        updateUIShowingMatrix(x, y, direction);
    }

    private void updateUIShowingMatrix(int x, int y, int direction) {
        // Be sure it's called in onFrame.
        float[] pts = {x, y};
        mSensorMatrix[direction].mapPoints(pts);
        LogHelper.v(TAG, "[updateUIShowingMatrix]Matrix x = " + pts[0] + " y = " + pts[1]);

        prepareTransformMatrix(direction);
        mDisplayMatrix.mapPoints(pts);
        LogHelper.v(TAG, "[updateUIShowingMatrix]DisplayMatrix x = " +
                pts[0] + " y = " + pts[1]);

        int fx = (int) pts[0];
        int fy = (int) pts[1];

        mNaviLine.setLayoutPosition(fx - mHalfArrowHeight, fy - mHalfArrowLength, fx
                + mHalfArrowHeight, fy + mHalfArrowLength);

        updateDirection(direction);
        mNaviLine.setVisibility(View.VISIBLE);
    }

    private void prepareTransformMatrix(int direction) {
        mDisplayMatrix.reset();
        int halfPrewWidth = mPreviewWidth >> 1;
        int halfPrewHeight = mPreviewHeight >> 1;

        // Determine the length / height of the arrow.
        getArrowHL();

        // For simplified calculation of view rectangle, clip arrow length
        // for both view width and height.
        // Arrow may look like this "--------------->"
        float halfViewWidth = (float) halfPrewWidth - mHalfArrowLength;
        float halfViewHeight = (float) halfPrewHeight - mHalfArrowLength;

        mDisplayMatrix.postScale(halfViewWidth / mDistanceHorizontal, halfViewHeight
                / mDistanceVertical);

        switch (mDisplayOrientation) {
            case 270:
                mDisplayMatrix.postTranslate(-halfViewWidth * 2, 0);
                mDisplayMatrix.postRotate(-90);
                break;

            case 0:
                break;

            case 90:
                mDisplayMatrix.postTranslate(0, -halfViewHeight * 2);
                mDisplayMatrix.postRotate(90);
                break;

            case 180:
                mDisplayMatrix.postTranslate(-halfViewWidth * 2,
                        -halfViewHeight * 2);
                mDisplayMatrix.postRotate(180);
                break;

            default:
                break;
        }
        mDisplayMatrix.postTranslate(mHalfArrowLength, mHalfArrowLength);
    }

    private void getArrowHL() {
        if (mHalfArrowHeight == 0) {
            int naviWidth = mNaviLine.getWidth();
            int naviHeight = mNaviLine.getHeight();
            if (naviWidth > naviHeight) {
                mHalfArrowLength = naviWidth >> 1;
                mHalfArrowHeight = naviHeight >> 1;
            } else {
                mHalfArrowHeight = naviWidth >> 1;
                mHalfArrowLength = naviHeight >> 1;
            }
        }
    }

    private void updateDirection(int direction) {
        LogHelper.d(TAG, "[updateDirection]mDisplayOrientation:" + mDisplayOrientation
                + ",mSensorDirection =" + mSensorDirection + ", direction = " + direction);
        int index = 0;
        for (int i = 0; i < DIRECTIONS_COUNT; i++) {
            if (DIRECTIONS[i] == direction) {
                index = i;
                break;
            }
        }
        switch (mDisplayOrientation) {
            case 270:
                direction = DIRECTIONS[(index - 1 + DIRECTIONS_COUNT) % DIRECTIONS_COUNT];
                break;

            case 0:
                break;

            case 90:
                direction = DIRECTIONS[(index + 1) % DIRECTIONS_COUNT];
                break;

            case 180:
                direction = DIRECTIONS[(index + 2) % DIRECTIONS_COUNT];
                break;

            default:
                break;
        }

        if (mSensorDirection != direction) {
            mSensorDirection = direction;
            if (mSensorDirection != DIRECTION_UNKNOWN) {
                setOrientationIndicator(direction);
                mCenterIndicator.setVisibility(View.VISIBLE);

                mAnimationController.startCenterAnimation();
                for (int i = 0; i < 4; i++) {
                    mDirectionSigns[i].setVisibility(View.INVISIBLE);
                }
            } else {
                mCenterIndicator.setVisibility(View.INVISIBLE);
            }
        }
    }

    private void setOrientationIndicator(int direction) {
        LogHelper.d(TAG, "[setOrientationIndicator]direction = " + direction);
        if (direction == DIRECTION_RIGHT) {
            ((Rotatable) mCollimatedArrowsDrawable).setOrientation(0, ANIMATION);
            ((Rotatable) mCenterIndicator).setOrientation(0, ANIMATION);
            mNaviLine.setRotation(-90);
        } else if (direction == DIRECTION_LEFT) {
            ((Rotatable) mCollimatedArrowsDrawable).setOrientation(180, ANIMATION);
            ((Rotatable) mCenterIndicator).setOrientation(180, ANIMATION);
            mNaviLine.setRotation(90);
        } else if (direction == DIRECTION_UP) {
            ((Rotatable) mCollimatedArrowsDrawable).setOrientation(90, ANIMATION);
            ((Rotatable) mCenterIndicator).setOrientation(90, ANIMATION);
            mNaviLine.setRotation(180);
        } else if (direction == DIRECTION_DOWN) {
            ((Rotatable) mCollimatedArrowsDrawable).setOrientation(270, ANIMATION);
            ((Rotatable) mCenterIndicator).setOrientation(270, ANIMATION);
            mNaviLine.setRotation(0);
        }
    }

    private void startCenterAnimation() {
        mCollimatedArrowsDrawable.setVisibility(View.GONE);
        mAnimationController.startCenterAnimation();
        mCenterIndicator.setVisibility(View.VISIBLE);
    }

    private void setSaveCancelButtonOrientation(int orientation) {
        mApp.getActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                CameraUtil.rotateRotateLayoutChildView(mApp.getActivity(), mSaveButton,
                        orientation, false);
                CameraUtil.rotateRotateLayoutChildView(mApp.getActivity(), mCancelButton,
                        orientation, false);
            }
        });
    }
}
