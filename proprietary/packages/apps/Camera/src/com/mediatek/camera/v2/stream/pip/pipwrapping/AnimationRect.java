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
package com.mediatek.camera.v2.stream.pip.pipwrapping;

import android.graphics.Matrix;
import android.graphics.RectF;

import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;

public class AnimationRect {
    private static final Tag TAG = new Tag(AnimationRect.class.getSimpleName());
    private static final float    MAX_SCALE_VALUE = PIPCustomization.TOP_GRAPHIC_MAX_SCALE_VALUE;
    private static final float    MIN_SCALE_VALUE = PIPCustomization.TOP_GRAPHIC_MIN_SCALE_VALUE;
    private static float mRotationLimitedMax = PIPCustomization.TOP_GRAPHIC_MAX_ROTATE_VALUE;
    private static float mRotationLimitedMin = -PIPCustomization.TOP_GRAPHIC_MAX_ROTATE_VALUE;
    private float                 mCurrentScaleValue = 1.0f;
    private Matrix                mAnimationMatrix;
    private float                 mOriginalDistance = 0f;
    private RectF                 mRectF;
    private int                   mPreviewWidth = -1;
    private int                   mPreviewHeight = -1;
    private float                 mCurrentRotationValue = 0f;
    private float[]               leftTop = new float[] { 0f, 0f };
    private float[]               rightTop = new float[] { 0f, 0f };
    private float[]               leftBottom = new float[] { 0f, 0f };
    private float[]               rightBottom = new float[] { 0f, 0f };
    private boolean               mIsHighlightEnable = false;

    public float getOriginalDistance() {
        return mOriginalDistance;
    }

    public void setOriginalDistance(float originalDistance) {
        mOriginalDistance = originalDistance;
    }

    public float getCurrentScaleValue() {
        return mCurrentScaleValue;
    }

    public void setCurrentScaleValue(float currentScaleValue) {
        mCurrentScaleValue = currentScaleValue;
    }

    public float[] getLeftTop() {
        return leftTop;
    }

    public void setLeftTop(float[] lefttop) {
        leftTop[0] = lefttop[0];
        leftTop[1] = lefttop[1];
    }

    public float[] getRightTop() {
        return rightTop;
    }

    public void setRightTop(float[] righttop) {
        rightTop[0] = righttop[0];
        rightTop[1] = righttop[1];
    }

    public float[] getLeftBottom() {
        return leftBottom;
    }

    public void setLeftBottom(float[] leftbottom) {
        leftBottom[0] = leftbottom[0];
        leftBottom[1] = leftbottom[1];
    }

    public float[] getRightBottom() {
        return rightBottom;
    }

    public void setRightBottom(float[] rightbottom) {
        rightBottom[0] = rightbottom[0];
        rightBottom[1] = rightbottom[1];
    }

    private void setVetex(float left, float top, float right, float bottom) {
        leftTop[0] = left;
        leftTop[1] = top;
        rightTop[0] = right;
        rightTop[1] = top;
        leftBottom[0] = left;
        leftBottom[1] = bottom;
        rightBottom[0] = right;
        rightBottom[1] = bottom;
    }

    public AnimationRect() {
        mAnimationMatrix = new Matrix();
        mRectF = new RectF();
    }

    public int getPreviewWidth() {
        return mPreviewWidth;
    }

    public int getPreviewHeight() {
        return mPreviewHeight;
    }

    public void setRendererSize(int width, int height) {
        // reduce edge / 2
        mPreviewWidth = width;
        mPreviewHeight = height;
    }

    public void initialize(float mLeft, float mTop, float mRight, float mBottom) {
        mRectF.set(mLeft, mTop, mRight, mBottom);
        setVetex(mRectF.left, mRectF.top, mRectF.right, mRectF.bottom);
        mOriginalDistance = (float) Math.sqrt((centerX() - rightBottom[0])
                * (centerX() - rightBottom[0]) + (centerY() - rightBottom[1])
                * (centerY() - rightBottom[1]));
    }

    public float adjustScaleDistance(float newDistance) {
        if (newDistance < 3 * mOriginalDistance / 4) {
            return 3 * mOriginalDistance / 4;
        } else if (newDistance > mOriginalDistance * 4 / 3) {
            return mOriginalDistance * 4 / 3;
        }
        return newDistance;
    }

    public void translate(float dx, float dy, boolean checkTranslate) {
        mAnimationMatrix.reset();
        mAnimationMatrix.setTranslate(dx, dy);
        mAnimationMatrix.mapRect(mRectF);
        if (checkTranslate) {
            checkTranslate();
        }
        setVetex(mRectF.left, mRectF.top, mRectF.right, mRectF.bottom);
    }

    public RectF getRectF() {
        return mRectF;
    }

    public void setCurrrentRotationValue(float rotation) {
        mCurrentRotationValue = rotation;
    }

    public float getCurrrentRotationValue() {
        return mCurrentRotationValue;
    }

    public void setHighLightEnable(boolean highlight) {
        mIsHighlightEnable = highlight;
    }

    public boolean getHighLightStatus() {
        return mIsHighlightEnable;
    }

    public float centerX() {
        return (rightTop[0] + leftBottom[0]) / 2;
    }

    public float centerY() {
        return (rightTop[1] + leftBottom[1]) / 2;
    }

    private float getMaxScaleValue() {
        float maxScaleValue = Math.min(getXMaxScaleValue(), getYMaxScaleValue());
        LogHelper.d(TAG, "getMaxScaleValue maxScaleValue = "
                + maxScaleValue + " mCurrentScaleValue = "
                + mCurrentScaleValue);
        maxScaleValue = mCurrentScaleValue * maxScaleValue;
        return maxScaleValue > MAX_SCALE_VALUE ? MAX_SCALE_VALUE : maxScaleValue;
    }

    private float getMinScaleValue() {
        return MIN_SCALE_VALUE;
    }

    private float getXMaxScaleValue() {
        return Math.min(
                (float) Math.sqrt((centerX() * centerX())
                        / ((centerX() - leftTop[0]) * (centerX() - leftTop[0]))),
                (float) Math.sqrt(((mPreviewWidth - centerX()) * (mPreviewWidth - centerX()))
                        / ((rightBottom[0] - centerX())) * ((rightBottom[0] - centerX()))));
    }

    private float getYMaxScaleValue() {
        return Math.min(
                (float) Math.sqrt((centerY() * centerY())
                        / ((centerY() - leftTop[1]) * (centerY() - leftTop[1]))),
                (float) Math.sqrt(((mPreviewHeight - centerY()) * (mPreviewHeight - centerY()))
                        / ((rightBottom[1] - centerY())) * ((rightBottom[1] - centerY()))));
    }

    public void scale(float scale, boolean checkScale) {
        LogHelper.d(TAG, "Before setScale scale = " + scale
                + " getMaxScaleValue = " + getMaxScaleValue()
                + " getMinScaleValue = " + getMinScaleValue());
        if (checkScale) {
            float scaleValue = mCurrentScaleValue * scale;
            // check max scale value
            if (scale > 1) {
                scale = scaleValue > getMaxScaleValue() ? 1f : scale;
            }
            // check minimal scale value
            if (scale < 1) {
                scale = scaleValue < getMinScaleValue() ? 1f : scale;
            }
            mCurrentScaleValue = mCurrentScaleValue * scale;
        }
        LogHelper.d(TAG, "setScale mCurrentScaleValue = " + mCurrentScaleValue);
        mAnimationMatrix.reset();
        mAnimationMatrix.setScale(scale, scale, mRectF.centerX(), mRectF.centerY());
        mAnimationMatrix.mapRect(mRectF);
        setVetex(mRectF.left, mRectF.top, mRectF.right, mRectF.bottom);
        mOriginalDistance = (float) Math.sqrt((centerX() - rightBottom[0])
                * (centerX() - rightBottom[0]) + (centerY() - rightBottom[1])
                * (centerY() - rightBottom[1]));
        LogHelper.d(TAG, "After setScale scale = " + scale);
    }

    public void scaleToTranslateY(float scaleY) {
        LogHelper.d(TAG, "setScaleToTranslateY");
        float[] rt = new float[] { rightTop[0], rightTop[1] };
        mAnimationMatrix.reset();
        mAnimationMatrix.setScale(1, scaleY, mRectF.centerX(), mRectF.centerY());
        mAnimationMatrix.mapPoints(rt);
        translate(0, rt[1] - rightTop[1], false);
    }

    public void rotate(float degrees) {
        rotate(degrees, mRectF.centerX(), mRectF.centerY());
    }

    public void rotate(float degrees, float centerX, float centerY) {
        LogHelper.d(TAG, "setRotate");
        setVetex(mRectF.left, mRectF.top, mRectF.right, mRectF.bottom);
        mAnimationMatrix.reset();
        mAnimationMatrix.setRotate(degrees, centerX, centerY);
        mAnimationMatrix.mapPoints(leftTop);
        mAnimationMatrix.mapPoints(rightTop);
        mAnimationMatrix.mapPoints(leftBottom);
        mAnimationMatrix.mapPoints(rightBottom);
        mCurrentRotationValue = degrees;
    }

    private void checkTranslate() {
        if (mPreviewWidth <= 0 || mPreviewHeight <= 0) {
            return;
        }
        // check left
        if (mRectF.left < 0) {
            mAnimationMatrix.reset();
            mAnimationMatrix.setTranslate(-mRectF.left, 0);
            mAnimationMatrix.mapRect(mRectF);
        }
        // check right
        if (mRectF.right > mPreviewWidth) {
            mAnimationMatrix.reset();
            mAnimationMatrix.setTranslate(mPreviewWidth - mRectF.right, 0);
            mAnimationMatrix.mapRect(mRectF);
        }
        // check top
        if (mRectF.top < 0) {
            mAnimationMatrix.reset();
            mAnimationMatrix.setTranslate(0, -mRectF.top);
            mAnimationMatrix.mapRect(mRectF);
        }
        // check bottom
        if (mRectF.bottom > mPreviewHeight) {
            mAnimationMatrix.reset();
            mAnimationMatrix.setTranslate(0, mPreviewHeight - mRectF.bottom);
            mAnimationMatrix.mapRect(mRectF);
        }
    }

    public static void dumpVertex(AnimationRect rect) {
        LogHelper.d(TAG, "Dump Vertex Animation Rect begin");
        LogHelper.d(TAG, "(" + rect.getLeftTop()[0] + " , "
                + rect.getLeftTop()[1] + ")" + " , " + "("
                + rect.getRightTop()[0] + " , " + rect.getRightTop()[1] + ")");
        LogHelper.d(TAG, "(" + rect.getLeftBottom()[0] + " , "
                + rect.getLeftBottom()[1] + ")" + " , "
                + "(" + rect.getRightBottom()[0] + " , " + rect.getRightBottom()[1] + ")");
        LogHelper.d(TAG, "Dump Vertex Animation Rect end");
        LogHelper.d(TAG, "(centerX , centerY) = " + "(" + rect.centerX()
                + " , " + rect.centerY() + ")");
    }

    public AnimationRect copy() {
        AnimationRect resultRect = new AnimationRect();
        resultRect.mCurrentScaleValue = this.mCurrentScaleValue;
        resultRect.mAnimationMatrix.set(this.mAnimationMatrix);
        resultRect.mOriginalDistance = this.mOriginalDistance;
        resultRect.mRectF.set(this.mRectF);
        resultRect.mPreviewWidth = this.mPreviewWidth;
        resultRect.mPreviewHeight = this.mPreviewHeight;
        resultRect.mCurrentRotationValue = this.mCurrentRotationValue;
        resultRect.setLeftTop(this.getLeftTop());
        resultRect.setRightTop(this.getRightTop());
        resultRect.setLeftBottom(this.getLeftBottom());
        resultRect.setRightBottom(this.getRightBottom());
        resultRect.setHighLightEnable(this.mIsHighlightEnable);
        return resultRect;
    }

    public void changePortraitCooridnateSystem(int newWidth, int newHeight) {
        float portraitWidht = (float) Math.min(newWidth, newHeight);
        float portraitHeight = (float) Math.max(newWidth, newHeight);
        float scaleX = portraitWidht / Math.min(mPreviewWidth, mPreviewHeight);
        float scaleY = portraitHeight / Math.max(mPreviewWidth, mPreviewHeight);
        float centerX = centerX();
        float centerY = centerY();
        float newCenterX = scaleX * centerX;
        float newCenterY = scaleY * centerY;
        // translate to new center
        translate(newCenterX - centerX,
                  newCenterY - centerY,
                  false);
        // scale by animationScaleX
        scale(scaleX, false);
//        scaleToTranslateY(scaleX / scaleY);
        rotate(mCurrentRotationValue);
        setRendererSize((int) portraitWidht, (int) portraitHeight);
    }

    public void changeToLandscapeCooridnateSystem(int width, int height, int rotation) {
        int portraitWidth = Math.min(width, height);
        int portraitHeight = Math.max(width, height);
        changePortraitCooridnateSystem(portraitWidth, portraitHeight);

        float centerX = centerX();
        float centerY = centerY();
        float newCenterX = 0;
        float newCenterY = 0;

        switch (rotation) {
        case 90:
            newCenterX = centerY;
            newCenterY = portraitWidth - centerX;
            break;
        case 270:
            newCenterX = portraitHeight - centerY;
            newCenterY = centerX;
            break;
        default:
            break;
        }

        translate(newCenterX - centerX,
                newCenterY - centerY,
                false);
        rotate(mCurrentRotationValue - rotation);
    }

    public void changeCooridnateSystem(int newWidth, int newHeight, int rotation) {
        float animationScaleX = (float) Math.min(newWidth, newHeight)
                / Math.min(mPreviewWidth, mPreviewHeight);
        float animationScaleY = (float) Math.max(newWidth, newHeight)
                / Math.max(mPreviewWidth, mPreviewHeight);
        // keep original centerX and centerY
        float centerX = centerX();
        float centerY = centerY();
        float tempValue;
        switch (rotation) {
        case 0:
            break;
        case 90:
            tempValue = centerX;
            centerX = centerY;
            centerY = mPreviewWidth - tempValue;
            break;
        case 180:
            break;
        case 270:
            tempValue = centerX;
            centerX = mPreviewHeight - centerY;
            centerY = tempValue;
            break;
        }
        // translate to new coordinate system
        translate(centerX - centerX(), centerY - centerY(), false);
        // translate from old renderer coordinate system to new renderer coordinate system
        translate(centerX * animationScaleX - centerX,
                  centerY * animationScaleY - centerY,
                  false);
        // scale by animationScaleX
        scale(animationScaleX, false);
        // scale to translate by animationScaleY / animationScaleX to match correct top distance
        scaleToTranslateY(animationScaleY / animationScaleX);
        // compute rotation
//        rotate(-rotation);
        float rotationRotate = formatRotationValue(360 - rotation);
//        rotationRotate = AnimationRect.checkRotationLimit(rotationRotate, mCurrentRotationValue);
        // rotate by current orientation
        rotate(mCurrentRotationValue + rotationRotate);
//        rotate(rotationRotate);
    }

    public static float formatRotationValue(float rotation) {
        if (rotation > 180) {
            rotation = rotation - 360;
        }
        if (rotation < -180) {
            rotation = rotation + 360;
        }
        rotation = rotation % 360;
        return rotation;
    }

    public static float checkRotationLimit(float rotation, float rotatedRotation) {
        // same direction should -, reverse direction should +
        boolean rotatedClockwise = rotatedRotation > 0;
        boolean currentRotatedClockwire = rotation > 0;
        float mcurrentRotaedRotation = (rotatedClockwise == currentRotatedClockwire)
                ? rotatedRotation
                : -rotatedRotation;
        rotation -= mcurrentRotaedRotation;
        if (rotation < mRotationLimitedMin) {
            rotation = mRotationLimitedMin;
        }
        if (rotation > mRotationLimitedMax) {
            rotation = mRotationLimitedMax;
        }
        rotation += mcurrentRotaedRotation;
        return rotation;
    }
}
