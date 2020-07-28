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
package com.mediatek.camera.v2.stream.pip;

import android.app.Activity;
import android.content.Context;
import android.graphics.Point;
import android.graphics.RectF;
import android.hardware.display.DisplayManager;
import android.util.Size;
import android.view.Display;
import android.view.Surface;
import android.view.WindowManager;

import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;
import com.mediatek.camera.v2.stream.pip.pipwrapping.AnimationRect;
import com.mediatek.camera.v2.stream.pip.pipwrapping.GLUtil;
import com.mediatek.camera.v2.stream.pip.pipwrapping.PIPCustomization;

import junit.framework.Assert;

/**
 * This class is used to compute pip top graphic's position.
 * <p>
 * Note: we will base on preview size to compute top graphic's position.
 * Not preview view area, But gestures are based on preview area,
 * so we need make a transform from 'preview area
 * gesture' to 'preview size gesture'.
 */
public class PipGestureImpl implements IPipGesture {
    private static final Tag TAG = new Tag(PipGestureImpl.class.getSimpleName());
    public static final int                ANIMATION_TRANSLATE = 0;
    public static final int                ANIMATION_SCALE_ROTATE = 1;
    // synchronize read and write mTopGraphicRect.
    private static Object                  mSyncTopGraphicRect = new Object();
    private final Activity                 mActivity;
    private int                            mDisplayRotation = -1;
    private final GestureCallback          mCallback;
    // top rectangle attributes records top graphic rectangle's position
    private final AnimationRect            mTopGraphicRectInPortrait;
    private final RectF                    mEditSquareRectF;

    private int                            mEditSquareSideLength = 0;
    private int                            mPreviewWidth = 0;
    private int                            mPreviewHeight = 0;
    // used to transform gesture
    private RectF                          mPreviewArea = null;
    private float                          mXScale = 1f;
    private float                          mYScale = 1f;

    private final int                      RECT_TO_TOP; // dp
    private float                          mRelativeRectToTop;
    private int                            mCurrentGsensorOrientation = -1;

    private boolean                        isTranslateAnimationEnable = false;
    private boolean                        isScaleRotateAnimationEnable = false;

    public PipGestureImpl(Activity activity, GestureCallback callback) {
        Assert.assertNotNull(activity);
        Assert.assertNotNull(callback);
        mActivity                    = activity;
        mCallback                    = callback;
        mTopGraphicRectInPortrait    = new AnimationRect();
        mEditSquareRectF             = new RectF();
        RECT_TO_TOP                  = 100;
    }

    @Override
    public void open() {
        mDisplayRotation = getDisplayRotation(mActivity);
        ((DisplayManager) mActivity
                .getSystemService(Context.DISPLAY_SERVICE))
                .registerDisplayListener(mDisplayListener, null);
    }

    @Override
    public void release() {
        ((DisplayManager) mActivity
                .getSystemService(Context.DISPLAY_SERVICE))
                .unregisterDisplayListener(mDisplayListener);
    }

    @Override
    public void setPreviewSize(Size previewSize) {
        Assert.assertNotNull(previewSize);
        LogHelper.d(TAG, "[setPreviewSize]+ : " + previewSize.getWidth() + " x "
                + previewSize.getHeight());
        int width = Math.min(previewSize.getWidth(), previewSize.getHeight());
        int height = Math.max(previewSize.getWidth(), previewSize.getHeight());
        if (mPreviewWidth == width && mPreviewHeight == height) {
            LogHelper.i(TAG, "[setPreviewSize]- skip for the same size : " + width + "x" + height);
            return;
        }

        // always update edit square's side length
        mEditSquareSideLength = Math.min(width, height) /
                PIPCustomization.TOP_GRAPHIC_EDIT_BUTTON_RELATIVE_VALUE;
        if (mPreviewWidth == 0 || mPreviewHeight == 0) {
            WindowManager wm = (WindowManager) mActivity.getSystemService(Context.WINDOW_SERVICE);
            Display display = wm.getDefaultDisplay();
            Point point = new Point();
            display.getRealSize(point);
            int maxEdge = Math.max(point.x, point.y);
            mRelativeRectToTop = RECT_TO_TOP * Math.max(width, height) / (float) maxEdge; //height

            float[] topGraphicPosition = GLUtil.createTopRightRect(
                    width, height, mRelativeRectToTop);
            mTopGraphicRectInPortrait.setRendererSize(Math.min(width, height),
                    Math.max(width, height));
            mTopGraphicRectInPortrait.initialize(topGraphicPosition[0], /**left**/
                                       topGraphicPosition[1], /**top**/
                                       topGraphicPosition[6], /**right**/
                                       topGraphicPosition[7]  /**bottom**/);
        } else {
            mTopGraphicRectInPortrait.changePortraitCooridnateSystem(width, height);
        }
        mPreviewWidth = width;
        mPreviewHeight = height;
        if (mPreviewArea != null) {
            mXScale = mPreviewWidth / Math.min(mPreviewArea.width(), mPreviewArea.height());
            mYScale = mPreviewHeight / Math.max(mPreviewArea.width(), mPreviewArea.height());
        }
        LogHelper.d(TAG, "[setPreviewSize]-");
    }

    @Override
    public void onPreviewAreaChanged(RectF previewArea) {
        LogHelper.d(TAG, "onPreviewAreaChanged previewArea:" + previewArea);
        mPreviewArea = previewArea;
        if (mPreviewWidth > 0 && mPreviewHeight > 0) {
            mXScale = mPreviewWidth / Math.min(mPreviewArea.width(), mPreviewArea.height());
            mYScale = mPreviewHeight / Math.max(mPreviewArea.width(), mPreviewArea.height());
        }
    }

    @Override
    public boolean onDown(float x, float y) {
        LogHelper.d(TAG, "[onDown]+ x:" + x + " y:" + y);
        if (mPreviewArea == null) {
            LogHelper.i(TAG, "[onDown]- with mPreviewArea is null!");
            return false;
        }
        // convert to portrait
        switch (mDisplayRotation) {
        case 0:
        case 180:
            break;
        case 90:
        case 270:
            float temp = y;
            y = x;
            x = mPreviewArea.height() - temp;
            break;
        }
        x = mXScale * x;
        y = mYScale * y;
        isTranslateAnimationEnable = mTopGraphicRectInPortrait.getRectF().contains(x, y);
        isScaleRotateAnimationEnable = mEditSquareRectF.contains(x, y);

        mTopGraphicRectInPortrait.setHighLightEnable(isTranslateAnimationEnable ||
                isScaleRotateAnimationEnable);
        if (isTranslateAnimationEnable || isScaleRotateAnimationEnable) {
            mCallback.onTopGraphicTouched();
        }
        LogHelper.d(TAG, "[onDown]- x:" + x + " y: " + y +
                "isTranslateAnimationEnable = " + isTranslateAnimationEnable +
                " isScaleRotateAnimationEnable = " + isScaleRotateAnimationEnable);
        return isTranslateAnimationEnable || isScaleRotateAnimationEnable;
    }

    @Override
    public boolean onScroll(float dx, float dy, float totalX, float totalY) {
        if (mPreviewArea == null || !isTranslateAnimationEnable && !isScaleRotateAnimationEnable) {
            return false;
        }
        switch (mDisplayRotation) {
        case 0:
            break;
        case 90:
        case 270:
            float temp = dx;
            dx = -dy;
            dy = temp;
            break;
        case 180:
            dx = -dx;
            dy = -dy;
            break;
        }
        dx = dx * mXScale;
        dy = dy * mYScale;
        if (isScaleRotateAnimationEnable) {
            initVertexData(-dx, -dy, ANIMATION_SCALE_ROTATE);
        } else if (isTranslateAnimationEnable) {
            initVertexData(-dx, -dy, ANIMATION_TRANSLATE);
        }
        return isTranslateAnimationEnable || isScaleRotateAnimationEnable;
    }

    @Override
    public boolean onSingleTapUp(float x, float y) {
        if (isTranslateAnimationEnable && !isScaleRotateAnimationEnable) {
            mCallback.onTopGraphicSingleTapUp();
        }
        return isTranslateAnimationEnable || isScaleRotateAnimationEnable;
    }

    @Override
    public boolean onLongPress(float x, float y) {
        if (isTranslateAnimationEnable && !isScaleRotateAnimationEnable) {
            mCallback.onTopGraphicSingleTapUp();
        }
        return isTranslateAnimationEnable || isScaleRotateAnimationEnable;
    }

    @Override
    public boolean onUp() {
        LogHelper.d(TAG, "onUp");
        mTopGraphicRectInPortrait.setHighLightEnable(false);
        isScaleRotateAnimationEnable = false;
        isScaleRotateAnimationEnable = false;
        return false;
    }

    @Override
    public AnimationRect getTopGraphicRect(int gsensorOrientation) {
        synchronized (mSyncTopGraphicRect) {
            mCurrentGsensorOrientation = gsensorOrientation;
            AnimationRect animationRect = mTopGraphicRectInPortrait.copy();
            animationRect.rotate(-gsensorOrientation + animationRect.getCurrrentRotationValue());
            float centerX = animationRect.getRightBottom()[0];
            float centerY = animationRect.getRightBottom()[1];
            if (mDisplayRotation == 270) {
                centerX = animationRect.getLeftTop()[0];
                centerY = animationRect.getLeftTop()[1];
            }
            updateEditSquare(
                    centerX,
                    centerY,
                    mEditSquareSideLength);
            animationRect = checkDisplayRotation(mDisplayRotation, animationRect);
            return animationRect;
        }
    }

    private DisplayManager.DisplayListener mDisplayListener =
            new DisplayManager.DisplayListener() {
        @Override
        public void onDisplayAdded(int arg0) {
            // Do nothing.
        }

        @Override
        public void onDisplayChanged(int displayId) {
            mDisplayRotation = getDisplayRotation(mActivity);
            LogHelper.d(TAG, "onDisplayChanged mDisplayRotation:" + mDisplayRotation);
        }

        @Override
        public void onDisplayRemoved(int arg0) {
            // Do nothing.
        }
    };

    private void updateEditSquare(float rCenterX, float rCenterY, float edgeLength) {
        mEditSquareRectF.set(
                rCenterX - edgeLength / 2, /**left**/
                rCenterY - edgeLength / 2, /**top**/
                rCenterX + edgeLength / 2, /**right**/
                rCenterY + edgeLength / 2); /**bottom**/
    }

    private AnimationRect checkDisplayRotation(int displayRotation, AnimationRect animationRect) {
       if (displayRotation == 270) {
            animationRect.translate(
                    mPreviewWidth - 2 * animationRect.centerX(),
                    mPreviewHeight - 2 * animationRect.centerY(),
                    true
                    );
            animationRect.rotate(animationRect.getCurrrentRotationValue());
        }
        return animationRect;
    }

    private int getDisplayRotation(Activity activity) {
        int rotation = activity.getWindowManager().getDefaultDisplay().getRotation();
        switch (rotation) {
            case Surface.ROTATION_0:
                return 0;
            case Surface.ROTATION_90:
                return 90;
            case Surface.ROTATION_180:
                return 180;
            case Surface.ROTATION_270:
                return 270;
        }
        return 0;
    }

    private void initVertexData(float dx, float dy, int animationtype) {
        switch (animationtype) {
        case ANIMATION_TRANSLATE:
            mTopGraphicRectInPortrait.translate(dx, dy, true);
            mTopGraphicRectInPortrait.rotate(mTopGraphicRectInPortrait.getCurrrentRotationValue());
            break;
        case ANIMATION_SCALE_ROTATE:

            AnimationRect animationRect = mTopGraphicRectInPortrait.copy();
            animationRect.rotate(-mCurrentGsensorOrientation +
                    animationRect.getCurrrentRotationValue());

            float newX = animationRect.getRightBottom()[0] + dx;
            float newY = animationRect.getRightBottom()[1] + dy;
            float oldDistance = (float) Math.sqrt(
                    (animationRect.centerX() - animationRect.getRightBottom()[0])
                        * (animationRect.centerX() - animationRect.getRightBottom()[0])
                    + (animationRect.centerY() - animationRect.getRightBottom()[1])
                        * (animationRect.centerY() - animationRect.getRightBottom()[1]));
            float newDistance = (float) Math.sqrt(
                    (animationRect.centerX() - newX)
                        * (animationRect.centerX() - newX)
                    + (animationRect.centerY() - newY)
                        * (animationRect.centerY() - newY));
            float scaleRatio = newDistance / oldDistance;
            float degress = (float) rotateAngle(dx, dy, animationRect);
            // reverse for topgraphicrect reverse
            if (mDisplayRotation == 270) {
                scaleRatio = 1 / scaleRatio;
                degress = - degress;
            }
            //scale
            mTopGraphicRectInPortrait.scale(scaleRatio, true);
            mTopGraphicRectInPortrait.rotate(mTopGraphicRectInPortrait.getCurrrentRotationValue());
            // rotate
            mTopGraphicRectInPortrait.rotate(degress +
                    mTopGraphicRectInPortrait.getCurrrentRotationValue());
            break;
        }
    }

    private double rotateAngle(float dx, float dy, AnimationRect animationRect) {
        double angle = 0, angle1, angle2;
        float centerX = animationRect.centerX();
        float centerY = animationRect.centerY();
        float right = animationRect.getRightBottom()[0];
        float bottom = animationRect.getRightBottom()[1];
        float newRight = right + dx;
        float newBottom = bottom + dy;
        angle1 = Math.atan2(bottom - centerY, right - centerX) * 180 / Math.PI;
        angle2 = Math.atan2(newBottom - centerY, newRight - centerX) * 180 / Math.PI;
        angle1 = (angle1 + 360) % 360;
        angle2 = (angle2 + 360) % 360;
        angle = angle2 - angle1;
        LogHelper.d(TAG, "rotateAngle angle:" + angle);
        return angle;
    }
}