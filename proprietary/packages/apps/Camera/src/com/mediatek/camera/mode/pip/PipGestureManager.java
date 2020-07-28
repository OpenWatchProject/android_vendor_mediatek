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
package com.mediatek.camera.mode.pip;

import android.app.Activity;
import android.content.Context;
import android.graphics.Point;
import android.graphics.RectF;
import android.view.Display;
import android.view.WindowManager;

import com.mediatek.camera.mode.pip.pipwrapping.AnimationRect;
import com.mediatek.camera.mode.pip.pipwrapping.GLUtil;
import com.mediatek.camera.mode.pip.pipwrapping.PIPOperator.PIPCustomization;
import com.mediatek.camera.util.Log;

public class PipGestureManager {
    public static final String TAG = "PipGestureManager";

    private Context mContext;
    // top rectangle attributes
    // records top graphic rectangle's position
    private AnimationRect mTopGraphicRect = null;
    // synchronize read and write mTopGraphicRect
    private static Object mSyncTopGraphicRect = new Object();
    private final int RECT_TO_TOP; // dp
    private float mCurrentRectToTop;

    private float mRotatedRotation = 0;
    private int mKeepLastOrientation = 0;
    private int mKeepLastDisplayRotation = 0;
    // top graphic rectangle animation (translate, scale, rotate)
    public static final int ANIMATION_TRANSLATE = 0;
    public static final int ANIMATION_SCALE = 1;
    public static final int ANIMATION_ROTATE = 2;
    private float mXScale = 1f;
    private float mYScale = 1f;
    private boolean isTranslateAnimation = false;
    private boolean isScaleRotateAnimation = false;

    private int mPreviewWidth = 0;
    private int mPreviewHeight = 0;

    private float mRotation = 0;
    private int mKeepPreviewOrientation = 0;
    private int mCurrentPreviewOrientation = 0;

    private Listener            mListener;
    private int                 mEditButtonSize = 0;
    private RectF               mEditButtonRect;

    // this listener is used to communicate with RendererManager
    public interface Listener {
        int getGSensorOrientation();
        int getButtomGraphicCameraId();
        void notifyTopGraphicIsEdited();
        void switchPIP();
    }

    public PipGestureManager(Context context, Listener listener) {
        Log.d(TAG, "PIPGestureManager");
        mContext = context;
//        if (context.getDisplayRotation() % 180 == 0) {
//            RECT_TO_TOP = mContext.getModePicker().getViewHeight();
//        } else {
//            RECT_TO_TOP = mContext.getModePicker().getViewWidth();
//        }
        //TODO add rect to top by modepicker
        RECT_TO_TOP = 100;
        mListener = listener;
        mTopGraphicRect = new AnimationRect();
        mEditButtonRect = new RectF();
    }

    public void setPreviewOrientation(int orientation) {
        mCurrentPreviewOrientation = orientation;
    }

    public void setRendererSize(int width, int height) {
        Log.d(TAG, "setPreviewSize width = " + width + " height = " + height + " oldWidth = "
                + mPreviewWidth + " oldHeight = " + mPreviewHeight + " mTopGraphicRect = "
                + mTopGraphicRect + " mCurrentPreviewOrientation = " + mCurrentPreviewOrientation
                + " mKeepPreviewOrientation = " + mKeepPreviewOrientation);
        if (mTopGraphicRect == null || (width == mPreviewWidth && height == mPreviewHeight)) {
            return;
        }
        synchronized (mSyncTopGraphicRect) {
            WindowManager wm = (WindowManager) mContext.getSystemService(Context.WINDOW_SERVICE);
            Display display = wm.getDefaultDisplay();
            Point point = new Point();
            display.getRealSize(point);
            int maxEdge = Math.max(point.x, point.y);
            if (mPreviewWidth == 0 && mPreviewHeight == 0) {
                mCurrentRectToTop = RECT_TO_TOP * Math.max(width, height) / (float) maxEdge;
                // first time, initialize top graphic rectangle
                float[] topRight = GLUtil.createTopRightRect(width, height, mCurrentRectToTop);
                mTopGraphicRect.setRendererSize(width, height);
                mTopGraphicRect.initialize(topRight[0], topRight[1], topRight[6], topRight[7]);
                mTopGraphicRect.rotate(mRotation);
            } else {
                float animationScaleX = (float) Math.min(width, height)
                        / Math.min(mPreviewWidth, mPreviewHeight);
                float animationScaleY = (float) Math.max(width, height)
                        / Math.max(mPreviewWidth, mPreviewHeight);
                float tempValue = 0f;
                // Translate to new renderer coordinate system(landscape ->
                // portrait or portrait -> landscape)
                mTopGraphicRect.setRendererSize(width, height);
                float centerX = mTopGraphicRect.centerX();
                float centerY = mTopGraphicRect.centerY();
                switch ((mCurrentPreviewOrientation - mKeepPreviewOrientation + 360) % 360) {
                case 0:
                    break;
                case 90:
                    tempValue = centerX;
                    centerX = mPreviewHeight - centerY;
                    centerY = tempValue;
                    break;
                case 180:
                    break;
                case 270:
                    tempValue = centerX;
                    centerX = centerY;
                    centerY = mPreviewWidth - tempValue;
                    break;
                }
                // translate to new coordinate system
                mTopGraphicRect.translate(centerX - mTopGraphicRect.centerX(), centerY
                        - mTopGraphicRect.centerY(), false);
                // translate from old renderer coordinate system to new renderer
                // coordinate system
                mTopGraphicRect.translate(mTopGraphicRect.centerX() * animationScaleX
                        - mTopGraphicRect.centerX(), mTopGraphicRect.centerY() * animationScaleY
                        - mTopGraphicRect.centerY(), false);
                // scale by animationScaleX
                mTopGraphicRect.scale(animationScaleX, false);
                // scale to translate by animationScaleY / animationScaleX to
                // match correct top distance
                mTopGraphicRect.scaleToTranslateY(animationScaleY / animationScaleX);
                // rotate
                mRotation += (mCurrentPreviewOrientation - mKeepPreviewOrientation);
                mRotation = AnimationRect.formatRotationValue(mRotation);
                mRotation = AnimationRect.checkRotationLimit(mRotation, mRotatedRotation);
                mTopGraphicRect.rotate(mRotation);
            }
            mKeepPreviewOrientation = mCurrentPreviewOrientation;
            mPreviewWidth = width;
            mPreviewHeight = height;
            mTopGraphicRect.setRendererSize(width, height);
        }
        mEditButtonSize = Math.min(width, height)
                / PIPCustomization.TOP_GRAPHIC_EDIT_BUTTON_RELATIVE_VALUE;
        initEditButtonRect(mTopGraphicRect.getRightBottom()[0],
                mTopGraphicRect.getRightBottom()[1], mEditButtonSize);
    }

    // when orientation changed, rotate top graphic rectangle
    public void onViewOrientationChanged(int orientation) {
        Log.d(TAG, "onOrientationChanged orientation = " + orientation
                 + " mKeepLastOrientation = " + mKeepLastOrientation);
        synchronized (mSyncTopGraphicRect) {
            if (orientation != mKeepLastOrientation) {
                mRotatedRotation += ((360 - orientation + mKeepLastOrientation) % 360);
                mRotatedRotation = AnimationRect.formatRotationValue(mRotatedRotation);
                rotate(orientation - mKeepLastOrientation);
                mKeepLastOrientation = orientation;
                Log.d(TAG, "onOrientationChanged orientation = " + orientation
                        + " mKeepLastOrientation = " + mKeepLastOrientation
                        + " mRotatedRotation = " + mRotatedRotation);
            }
        }
    }

    public void setDisplayRotation(int displayRotation) {
        Log.d(TAG, "setDisplayRotation displayRotation = " + displayRotation);
        synchronized (mSyncTopGraphicRect) {
            if (displayRotation != mKeepLastDisplayRotation) {
                Log.d(TAG, "setDisplayRotation rotate = "
                        + (mKeepLastDisplayRotation - displayRotation));
                // display rotation changes, should rotate by new
                // displayRotation
                rotate(mKeepLastDisplayRotation - displayRotation);
                /**
                 * when camera's activity can be locked to reverse rotation,
                 * should consider: displayRotation switches between standard
                 * rotation and reverse rotation, should translate to new
                 * position.
                 */
                if (Math.abs(mKeepLastDisplayRotation - displayRotation) >= 180) {
                    Log.d(TAG, "setDisplayRotation" + " translate x = "
                            + (mPreviewWidth - 2 * mTopGraphicRect.centerX()) + " y = "
                            + (mPreviewHeight - 2 * mTopGraphicRect.centerY()));
                    initVertexData(mPreviewWidth - 2 * mTopGraphicRect.centerX(), mPreviewHeight
                            - 2 * mTopGraphicRect.centerY(), ANIMATION_TRANSLATE);
                }
                mKeepLastDisplayRotation = displayRotation;
            }
        }
    }

    /**************************Gesture related**********************/
    public boolean onDown(float x, float y, int relativeWidth, int relativeHeight) {
        Log.d(TAG, "onDown x = " + x + " y = " + y + " relativeWidth = " + relativeWidth
                + " relativeHeight = " + relativeHeight);
        switch (GLUtil.getDisplayRotation((Activity) mContext)) {
        case 0:
            break;
        case 90:
            float temp = x;
            x = relativeHeight - y;
            y = temp;
            break;
        case 180:
            x = relativeWidth - x;
            y = relativeHeight - y;
            break;
        case 270:
            float temp2 = x;
            x = y;
            y = relativeWidth - temp2;
            break;
        }
        // map scale, scale display size to original preview size
        boolean mFboPreviewIsLandscape = (GLUtil.getDisplayOrientation(0,
                mListener.getButtomGraphicCameraId()) % 90 == 0);
        boolean mRelativeFrameIsLandscape =
                (GLUtil.getDisplayRotation((Activity) mContext) % 180 == 0);
        if (mFboPreviewIsLandscape != mRelativeFrameIsLandscape) {
            int tempWidth = relativeWidth;
            relativeWidth = relativeHeight;
            relativeHeight = tempWidth;
        }
        mXScale = (float) mPreviewWidth / relativeWidth;
        mYScale = (float) mPreviewHeight / relativeHeight;
        x = mXScale * x;
        y = mYScale * y;
        Log.d(TAG, "scale: mXScale = " + mXScale + "mYScale = " + mYScale);
        // compute animation type
        isTranslateAnimation = mTopGraphicRect.getRectF().contains(x, y);
        isScaleRotateAnimation = mEditButtonRect.contains(x, y);
        mTopGraphicRect.setHighLightEnable(isTranslateAnimation || isScaleRotateAnimation);
        Log.d(TAG, "isTranslateAnimation = " + isTranslateAnimation + " isScaleAnimation = "
                + isScaleRotateAnimation);
        if (isTranslateAnimation || isScaleRotateAnimation) {
            mListener.notifyTopGraphicIsEdited();
        }
        return isTranslateAnimation || isScaleRotateAnimation;
    }

    public boolean onScroll(float dx, float dy, float totalX, float totalY) {
         Log.d(TAG, "before onScroll dx = " + dx + " dy = " + dy + " totalX = "
                 + totalX + " totalY = " + totalY
                 + " isTranslateAnimation = " + isTranslateAnimation
                 + " isScaleAnimation = " + isScaleRotateAnimation);
        if (!isTranslateAnimation && !isScaleRotateAnimation) {
            return false;
        }
        synchronized (mSyncTopGraphicRect) {
            // transform gestures to portrait, because pip gestures
            // are computed always in portrait coordinate.
            switch (mKeepLastDisplayRotation) {
            case 0:
                break;
            case 90:
                float temp2 = dx;
                dx = -dy;
                dy = temp2;
                break;
            case 180:
                dx = -dx;
                dy = -dy;
                break;
            case 270:
                float temp = dx;
                dx = dy;
                dy = -temp;
                break;
            default:
                break;
            }
            dx = dx * mXScale;
            dy = dy * mYScale;
            if (isScaleRotateAnimation) {
                initVertexData(-dx, -dy, ANIMATION_SCALE);
            } else if (isTranslateAnimation) {
                initVertexData(-dx, -dy, ANIMATION_TRANSLATE);
            }
        }
        return isTranslateAnimation || isScaleRotateAnimation;
    }

    public boolean onUp() {
        Log.d(TAG, "onUp");
        mTopGraphicRect.setHighLightEnable(false);
        isScaleRotateAnimation = false;
        isTranslateAnimation = false;
        return false;
    }

    public boolean onSingleTapUp(float x, float y) {
        Log.d(TAG, "onSingleTapUp x = " + x + " y = " + y + " isTranslateAnimation = "
                + isTranslateAnimation);
        if (isTranslateAnimation && !isScaleRotateAnimation) {
            mListener.switchPIP();
        }
        return isTranslateAnimation || isScaleRotateAnimation;
    }

    public boolean onLongPress(float x, float y) {
        if (isTranslateAnimation) {
            mListener.switchPIP();
        }
        return isTranslateAnimation || isScaleRotateAnimation;
    }

    public void rotate(int degrees) {
        Log.d(TAG, "rotate degrees = " + degrees + " mTopGraphicRect = " + mTopGraphicRect);
        if (mTopGraphicRect == null) {
            return;
        }
        mRotation += -degrees;
        mRotation = AnimationRect.formatRotationValue(mRotation);
        mRotation = AnimationRect.checkRotationLimit(mRotation, mRotatedRotation);
        // rotate mVtxRotateMtx
        mTopGraphicRect.rotate(mRotation);
    }

    public AnimationRect getTopGraphicRect() {
        synchronized (mSyncTopGraphicRect) {
            initEditButtonRect(mTopGraphicRect.getRightBottom()[0],
                    mTopGraphicRect.getRightBottom()[1], mEditButtonSize);
            return mTopGraphicRect.copy();
        }
    }

    public float getAnimationRectRotation() {
        synchronized (mSyncTopGraphicRect) {
            Log.d(TAG, "getAnimationRectRotation mRotation = " + mRotation);
            return mRotation;
        }
    }

    private void initVertexData(float dx, float dy, int animationtype) {
        switch (animationtype) {
        case ANIMATION_TRANSLATE:
            mTopGraphicRect.translate(dx, dy, true);
            mTopGraphicRect.rotate(mRotation);
            break;
        case ANIMATION_SCALE:
            // scale
            float newX = mTopGraphicRect.getRightBottom()[0] + dx;
            float newY = mTopGraphicRect.getRightBottom()[1] + dy;
            float oldDistance = (float) Math.sqrt((mTopGraphicRect.centerX() - mTopGraphicRect
                    .getRightBottom()[0])
                    * (mTopGraphicRect.centerX() - mTopGraphicRect.getRightBottom()[0])
                    + (mTopGraphicRect.centerY() - mTopGraphicRect.getRightBottom()[1])
                    * (mTopGraphicRect.centerY() - mTopGraphicRect.getRightBottom()[1]));
            float newDistance = (float) Math.sqrt((mTopGraphicRect.centerX() - newX)
                    * (mTopGraphicRect.centerX() - newX) + (mTopGraphicRect.centerY() - newY)
                    * (mTopGraphicRect.centerY() - newY));
            newDistance = mTopGraphicRect.adjustScaleDistance(newDistance);
            float scaleRatio = newDistance / oldDistance;
            mTopGraphicRect.translate(0, 0, true);
            mTopGraphicRect.scale(scaleRatio, true);
            mTopGraphicRect.rotate(mRotation);
            // rotate
            float degress = 0;
            degress = (float) rotateAngle(dx, dy);
            mRotation += degress;
            mRotation = AnimationRect.formatRotationValue(mRotation);
            mRotation = AnimationRect.checkRotationLimit(mRotation, mRotatedRotation);
            // rotate mVtxRotateMtx
            mTopGraphicRect.rotate(mRotation);
            break;
        case ANIMATION_ROTATE:
            break;
        }
    }

    private double rotateAngle(float dx, float dy) {
        double angle = 0, angle1, angle2;
        float centerX = mTopGraphicRect.centerX();
        float centerY = mTopGraphicRect.centerY();
        float right = mTopGraphicRect.getRightBottom()[0];
        float bottom = mTopGraphicRect.getRightBottom()[1];
        float newRight = right + dx;
        float newBottom = bottom + dy;
        angle1 = Math.atan2(bottom - centerY, right - centerX) * 180 / Math.PI;
        angle2 = Math.atan2(newBottom - centerY, newRight - centerX) * 180 / Math.PI;
        angle1 = (angle1 + 360) % 360;
        angle2 = (angle2 + 360) % 360;
        angle = angle2 - angle1;
        return angle;
    }

    private void initEditButtonRect(float rCenterX, float rCenterY, float edge) {
        Log.d(TAG, "initVertexData rCenterX = " + rCenterX + " rCenterY = " + rCenterY + " edge = "
                + edge);
        mEditButtonRect.set(rCenterX - edge / 2, rCenterY - edge / 2, rCenterX + edge / 2, rCenterY
                + edge / 2);
    }
}
