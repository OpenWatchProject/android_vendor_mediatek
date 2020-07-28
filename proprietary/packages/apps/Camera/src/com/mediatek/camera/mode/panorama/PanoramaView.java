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

package com.mediatek.camera.mode.panorama;

import android.app.Activity;
import android.graphics.Matrix;
import android.view.View;
import android.view.ViewGroup;

import com.android.camera.R;

import com.mediatek.camera.platform.ICameraAppUi;
import com.mediatek.camera.platform.IModuleCtrl;
import com.mediatek.camera.ui.CameraView;
import com.mediatek.camera.ui.ProgressIndicator;
import com.mediatek.camera.ui.Rotatable;
import com.mediatek.camera.ui.UIRotateLayout;
import com.mediatek.camera.ui.UIRotateLayout.OnSizeChangedListener;
import com.mediatek.camera.util.Log;

public class PanoramaView extends CameraView {
    private static final String TAG = "PanoramaView";

    private UIRotateLayout mScreenProgressLayout;
    private NaviLineImageView mNaviLine;
    private ProgressIndicator mProgressIndicator;
    private AnimationController mAnimationController;

    private View mRootView;
    private View mPanoView;
    private ViewGroup mDirectionSigns[] = new ViewGroup[4]; // up,down,left,right
    private ViewGroup mCenterIndicator;
    private ViewGroup mCollimatedArrowsDrawable;

    private static final boolean ANIMATION = true;
    private boolean mS3DMode = false;
    private boolean mNeedInitialize = true;
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
    private static final int PANO_3D_OVERLAP_DISTANCE = 32;
    private static final int PANO_3D_VERTICAL_DISTANCE = 240;
    private static final int NONE_ORIENTATION = -1;

    public static final int PANORAMA_VIEW = 0;

    private static final int[] DIRECTIONS = { DIRECTION_RIGHT, DIRECTION_DOWN, DIRECTION_LEFT,
            DIRECTION_UP };
    private static final int DIRECTIONS_COUNT = DIRECTIONS.length;

    private int mSensorDirection = DIRECTION_UNKNOWN;
    // private int mDisplayDirection = DIRECTION_UNKNOWN;
    private int mDisplayOrientaion;

    private int mHalfArrowHeight = 0;
    private int mHalfArrowLength = 0;

    private int mPreviewWidth = 0;
    private int mPreviewHeight = 0;

    private int mViewCategory;
    private int mHoldOrientation = NONE_ORIENTATION;
    private static final int BLOCK_NUM = 9;
    private int mBlockSizes[] = { 17, 15, 13, 12, 11, 12, 13, 15, 17 };

    private int mDistanceHorizontal = 0;
    private int mDistanceVertical = 0;

    private IModuleCtrl mIMoudleCtrl;

    public PanoramaView(Activity activity) {
        super(activity);
        Log.i(TAG, "[PanoramaView]constructor...");
        mViewCategory = PANORAMA_VIEW;
    }

    @Override
    public void init(Activity activity, ICameraAppUi cameraAppUi, IModuleCtrl moduleCtrl) {
        Log.i(TAG, "[init]...");
        mIMoudleCtrl = moduleCtrl;
        setOrientation(moduleCtrl.getOrientationCompensation());
    }

    @Override
    public void show() {
        Log.i(TAG, "[show]mNeedInitialize=" + mNeedInitialize);
        super.show();
        // display orientation and rotation will be updated when capture,
        // because camera may slip to gallery and rotate the display,then
        // display orientation and
        // rotation changed
        mDisplayOrientaion = mIMoudleCtrl.getDisplayOrientation();

        if (mNeedInitialize) {
            initializeViewManager();
            mNeedInitialize = false;
        }
        showCaptureView();
    }

    /**
     * will be called when app call release() to unload views from view
     * hierarchy.
     */
    @Override
    public void uninit() {
        Log.i(TAG, "[uninit]...");
        super.uninit();
        mNeedInitialize = true;
    }

    @Override
    public void reset() {
        Log.i(TAG, "[reset] mViewCategory = " + mViewCategory + ",mRootView = " + mRootView
                + ",mPanoView = " + mPanoView);
        if (mRootView == null) {
            return;
        }
        mPanoView.setVisibility(View.GONE);
        mAnimationController.stopCenterAnimation();
        mCenterIndicator.setVisibility(View.GONE);

        if (mViewCategory == PANORAMA_VIEW) {
            mSensorDirection = DIRECTION_UNKNOWN;
            mNaviLine.setVisibility(View.GONE);
            mCollimatedArrowsDrawable.setVisibility(View.GONE);
            for (int i = 0; i < 4; i++) {
                mDirectionSigns[i].setSelected(false);
                mDirectionSigns[i].setVisibility(View.VISIBLE);
            }
        }
    }

    @Override
    public boolean update(int type, Object... args) {
        Log.i(TAG, "[update] type =" + type);
        switch (type) {
        case PanoramaMode.INFO_UPDATE_PROGRESS:
            int num = Integer.parseInt(args[0].toString());
            setViewsForNext(num);
            break;

        case PanoramaMode.INFO_UPDATE_MOVING:
            if (args[0] != null && args[1] != null && args[2] != null) {
                int xy = Integer.parseInt(args[0].toString());
                int direction = Integer.parseInt(args[1].toString());
                boolean show = Boolean.parseBoolean(args[2].toString());
                updateMovingUI(xy, direction, show);
            }
            break;

        case PanoramaMode.INFO_START_ANIMATION:
            startCenterAnimation();
            break;

        case PanoramaMode.INFO_IN_CAPTURING:
            mIsCapturing = true;
            break;

        case PanoramaMode.INFO_OUTOF_CAPTURING:
            mIsCapturing = false;
            break;

        default:
            break;
        }

        return true;
    }

    @Override
    public void onOrientationChanged(int orientation) {
        Log.i(TAG, "[onOrientationChangedis]...mIsCapturing = " + mIsCapturing);
        if (!mIsCapturing) {
            super.onOrientationChanged(orientation);
            mHoldOrientation = NONE_ORIENTATION;
            // in 3D Mode, the layout lock as 270
            if (mS3DMode) {
                Log.i(TAG, "[onOrientationChanged]orientation = " + orientation);
                return;
            }
            if (mProgressIndicator != null) {
                mProgressIndicator.setOrientation(orientation);
            }
        } else {
            mHoldOrientation = orientation;
        }
    }

    /**
     * will be called if app want to show current view which hasn't been
     * created.
     *
     * @return
     */
    @Override
    protected View getView() {
        View view = inflate(R.layout.pano_preview);
        mRootView = view.findViewById(R.id.pano_frame_layout);
        return view;
    }

    private void initializeViewManager() {
        mPanoView = mRootView.findViewById(R.id.pano_view);

        mScreenProgressLayout = (UIRotateLayout) mRootView.findViewById(R.id.on_screen_progress);
        mCenterIndicator = (ViewGroup) mRootView.findViewById(R.id.center_indicator);
        mDirectionSigns[DIRECTION_RIGHT] = (ViewGroup) mRootView.findViewById(R.id.pano_right);
        mDirectionSigns[DIRECTION_LEFT] = (ViewGroup) mRootView.findViewById(R.id.pano_left);
        mDirectionSigns[DIRECTION_UP] = (ViewGroup) mRootView.findViewById(R.id.pano_up);
        mDirectionSigns[DIRECTION_DOWN] = (ViewGroup) mRootView.findViewById(R.id.pano_down);
        mAnimationController = new AnimationController(mDirectionSigns,
                (ViewGroup) mCenterIndicator.getChildAt(0));

        mDistanceHorizontal = mS3DMode ? PANO_3D_OVERLAP_DISTANCE : TARGET_DISTANCE_HORIZONTAL;
        mDistanceVertical = mS3DMode ? PANO_3D_VERTICAL_DISTANCE : TARGET_DISTANCE_VERTICAL;
        if (mViewCategory == PANORAMA_VIEW) {
            mNaviLine = (NaviLineImageView) mRootView.findViewById(R.id.navi_line);
            mCollimatedArrowsDrawable = (ViewGroup) mRootView
                    .findViewById(R.id.static_center_indicator);

            mProgressIndicator = new ProgressIndicator(getContext(), BLOCK_NUM, mBlockSizes);
            mProgressIndicator.setVisibility(View.GONE);
            mScreenProgressLayout.setOrientation(getOrientation(), true);
            mProgressIndicator.setOrientation(getOrientation());

            prepareSensorMatrix();
        }
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
        // reset orientation,since camera state is snapinprogress at last time.
        if (mHoldOrientation != NONE_ORIENTATION) {
            onOrientationChanged(mHoldOrientation);
        }
        if (mS3DMode) {
            for (int i = 0; i < 4; i++) {
                mDirectionSigns[i].setVisibility(View.INVISIBLE);
            }
            mCenterIndicator.setVisibility(View.VISIBLE);

            mAnimationController.startCenterAnimation();
        } else {
            mCenterIndicator.setVisibility(View.GONE);
        }
        mPanoView.setVisibility(View.VISIBLE);
        mProgressIndicator.setProgress(0);
        mProgressIndicator.setVisibility(View.VISIBLE);
    }

    private OnSizeChangedListener mOnSizeChangedListener = new OnSizeChangedListener() {
        @Override
        public void onSizeChanged(int width, int height) {
            Log.d(TAG, "[onSizeChanged]width=" + width + " height=" + height);
            mPreviewWidth = Math.max(width, height);
            mPreviewHeight = Math.min(width, height);
        }
    };

    private void setViewsForNext(int imageNum) {
        if (!filterViewCategory(PANORAMA_VIEW)) {
            return;
        }

        mProgressIndicator.setProgress(imageNum + 1);

        if (imageNum == 0) {
            if (!mS3DMode) {
                // in 3D Mode, direction animation do not show
                mAnimationController.startDirectionAnimation();
            } else {
                mNaviLine.setVisibility(View.VISIBLE);
            }
        } else {
            mNaviLine.setVisibility(View.INVISIBLE);
            mAnimationController.stopCenterAnimation();
            mCenterIndicator.setVisibility(View.GONE);
            mCollimatedArrowsDrawable.setVisibility(View.VISIBLE);
        }
    }

    private boolean filterViewCategory(int requestCategory) {
        if (mViewCategory != requestCategory) {
            return false;
        }
        return true;
    }

    private void updateMovingUI(int xy, int direction, boolean shown) {
        Log.d(TAG, "[updateMovingUI]xy:" + xy + ",direction:" + direction + ",shown:" + shown);
        if (!filterViewCategory(PANORAMA_VIEW)) {
            return;
        }
        // direction means sensor towards.
        if (direction == DIRECTION_UNKNOWN || shown || mNaviLine.getWidth() == 0
                || mNaviLine.getHeight() == 0) {
            // if the NaviLine has not been drawn well, return.
            mNaviLine.setVisibility(View.INVISIBLE);
            return;
        }
        short x = (short) ((xy & 0xFFFF0000) >> 16);
        short y = (short) (xy & 0x0000FFFF);

        updateUIShowingMatrix(x, y, direction);
    }

    private void updateUIShowingMatrix(int x, int y, int direction) {
        // Be sure it's called in onFrame.
        float[] pts = { x, y };
        mSensorMatrix[direction].mapPoints(pts);
        Log.v(TAG, "[updateUIShowingMatrix]Matrix x = " + pts[0] + " y = " + pts[1]);

        prepareTransformMatrix(direction);
        mDisplayMatrix.mapPoints(pts);
        Log.v(TAG, "[updateUIShowingMatrix]DisplayMatrix x = " + pts[0] + " y = " + pts[1]);

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
        float halfViewWidth = mS3DMode ? 65 * 4 : ((float) halfPrewWidth - mHalfArrowLength);
        float halfViewHeight = (float) halfPrewHeight - mHalfArrowLength;

        mDisplayMatrix.postScale(halfViewWidth / mDistanceHorizontal, halfViewHeight
                / mDistanceVertical);

        switch (mDisplayOrientaion) {
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
            mDisplayMatrix.postTranslate((float) (-halfViewWidth * (mS3DMode ? 2.67 : 2)),
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
        Log.d(TAG, "[updateDirection]mDisplayOrientaion:" + mDisplayOrientaion
                + ",mSensorDirection =" + mSensorDirection);
        int index = 0;
        for (int i = 0; i < DIRECTIONS_COUNT; i++) {
            if (DIRECTIONS[i] == direction) {
                index = i;
                break;
            }
        }
        switch (mDisplayOrientaion) {
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
                // mViewChangedListener.onCaptureBegin();
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
        Log.d(TAG, "[setOrientationIndicator]direction = " + direction);
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
}
