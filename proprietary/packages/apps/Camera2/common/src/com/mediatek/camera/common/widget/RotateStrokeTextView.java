/*
 * Copyright Statement:
 *
 *   This software/firmware and related documentation ("MediaTek Software") are
 *   protected under relevant copyright laws. The information contained herein is
 *   confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 *   the prior written permission of MediaTek inc. and/or its licensors, any
 *   reproduction, modification, use or disclosure of MediaTek Software, and
 *   information contained herein, in whole or in part, shall be strictly
 *   prohibited.
 *
 *   MediaTek Inc. (C) 2016. All rights reserved.
 *
 *   BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *   THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *   RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 *   ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 *   WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 *   WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 *   NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 *   RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *   INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 *   TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 *   RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 *   OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 *   SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 *   RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 *   STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 *   ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 *   RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 *   MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 *   CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *   The following software/firmware and/or related documentation ("MediaTek
 *   Software") have been modified by MediaTek Inc. All revisions are subject to
 *   any receiver's applicable license agreements with MediaTek Inc.
 */
package com.mediatek.camera.common.widget;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.os.Looper;
import android.text.TextPaint;
import android.util.AttributeSet;
import android.view.Gravity;
import android.view.animation.AnimationUtils;
import android.widget.TextView;

import java.lang.reflect.Field;


/**
 * A Stroke Text view which can rotate it's content.
 */

public class RotateStrokeTextView extends TwoStateTextView implements Rotatable {

    private static final float STROKE_WIDTH_DP = 0.5f;
    private static final int STROKE_ALPHA = 127;

    private TextPaint mTextPaint;

    private static final int ANIMATION_SPEED = 270; // 270 deg/sec
    private int mCurrentDegree = 0; // [0, 359]
    private int mStartDegree = 0;
    private int mTargetDegree = 0;

    private boolean mClockwise = false;
    private boolean mEnableAnimation = true;

    private long mAnimationStartTime = 0;
    private long mAnimationEndTime = 0;

    private float mStrokeWidth;


    /**
     * The RotateTextView constructor.
     * @param context
     *            The Context the view is running in, through which it can
     *            access the current theme, resources, etc.
     * @param attrs
     *            The attributes of the XML tag that is inflating the view.
     */
    public RotateStrokeTextView(Context context, AttributeSet attrs) {
        super(context, attrs);
        mStrokeWidth = dip2Px(context, STROKE_WIDTH_DP);
        mTextPaint = this.getPaint();
        setGravity(Gravity.CENTER);
    }

    /**
     * Rotate the view counter-clockwise.
     * @param degree the degree of the text
     * @param animation whether use animation or not.
     */
    @Override
    public void setOrientation(int degree, boolean animation) {
        mEnableAnimation = animation;
        // make sure in the range of [0, 359]
        degree = degree >= 0 ? degree % 360 : degree % 360 + 360;

        if (degree == mTargetDegree) {
            return;
        }

        mTargetDegree = degree;
        if (mEnableAnimation) {
            mStartDegree = mCurrentDegree;
            mAnimationStartTime = AnimationUtils.currentAnimationTimeMillis();

            int diff = mTargetDegree - mCurrentDegree;
            diff = diff >= 0 ? diff : 360 + diff; // make it in range [0, 359]

            // Make it in range [-179, 180]. That's the shorted distance between
            // the
            // two angles
            diff = diff > 180 ? diff - 360 : diff;

            mClockwise = diff >= 0;
            mAnimationEndTime = mAnimationStartTime + Math.abs(diff) * 1000 / ANIMATION_SPEED;
        } else {
            mCurrentDegree = mTargetDegree;
        }
        if(Looper.getMainLooper() != Looper.myLooper()) {
            postInvalidate();
        } else {
            invalidate();
        }

    }

    @Override
    protected void onDraw(Canvas canvas) {
        if (getText() == null) {
            return;
        }
        if (mCurrentDegree != mTargetDegree) {
            long time = AnimationUtils.currentAnimationTimeMillis();
            if (time < mAnimationEndTime) {
                int deltaTime = (int) (time - mAnimationStartTime);
                int degree = mStartDegree + ANIMATION_SPEED * (mClockwise ? deltaTime : -deltaTime)
                        / 1000;
                degree = degree >= 0 ? degree % 360 : degree % 360 + 360;
                mCurrentDegree = degree;
                invalidate();
            } else {
                mCurrentDegree = mTargetDegree;
            }
        }

        int left = getPaddingLeft();
        int top = getPaddingTop();
        int right = getPaddingRight();
        int bottom = getPaddingBottom();
        int width = getWidth() - left - right;
        int height = getHeight() - top - bottom;

        int saveCount = canvas.getSaveCount();

        canvas.translate(left + width / 2, top + height / 2);
        canvas.rotate(-mCurrentDegree);
        canvas.translate(-getWidth() / 2, -getHeight() / 2);


        setTextColorUseReflection(Color.BLACK);

        mTextPaint.setStyle(Paint.Style.STROKE);
        mTextPaint.setAlpha(STROKE_ALPHA);
        mTextPaint.setStrokeWidth(mStrokeWidth);
        super.onDraw(canvas);

        setTextColorUseReflection(Color.WHITE);
        mTextPaint.setStrokeWidth(0);
        mTextPaint.setStyle(Paint.Style.FILL);
        mTextPaint.setAlpha(255);

        super.onDraw(canvas);
        canvas.restoreToCount(saveCount);
    }


    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        super.onMeasure(widthMeasureSpec, heightMeasureSpec);

        int width = getMeasuredWidth();
        int height = getMeasuredHeight();
        int newWidth = Math.max(width, height);
        setMeasuredDimension(newWidth, newWidth);
    }

    private void setTextColorUseReflection(int color) {
        Field textColorField;
        try {
            textColorField = TextView.class.getDeclaredField("mCurTextColor");
            textColorField.setAccessible(true);
            textColorField.set(this, color);
            textColorField.setAccessible(false);
        } catch (NoSuchFieldException e) {
            e.printStackTrace();
        } catch (IllegalArgumentException e) {
            e.printStackTrace();
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        }
        mTextPaint.setColor(color);
    }

    private static float dip2Px(Context context, float dpvalue) {
        final float scale = context.getResources().getDisplayMetrics().density;
        return dpvalue * scale;
    }
}
