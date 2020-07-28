/*
 * Copyright (C) 2011 The Android Open Source Project
 * Copyright (C) 2013 Android Open Kang Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.debug.loggerui;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.LinearGradient;
import android.graphics.Paint;
import android.graphics.Path;
import android.graphics.Rect;
import android.graphics.Shader;
import android.util.AttributeSet;
import android.util.DisplayMetrics;
import android.widget.LinearLayout;

/**
 * @author MTK81255
 *
 */
public class LinearColorBar extends LinearLayout {

    private int mLeftColor = 0xff0099cc;
    // blue
    private int mMiddleColor = 0xff0099cc;
    // gray
    private int mRightColor = 0xff888888;

    private float mRedRatio;
    private float mYellowRatio;

    private boolean mShowingGreen;

    final Rect mRect = new Rect();
    final Paint mPaint = new Paint();

    float mInterestingLeftRadio;
    float mInterestingRightRadio;
    int mLineWidth;

    final Path mColorPath = new Path();
    final Path mEdgePath = new Path();
    final Paint mColorGradientPaint = new Paint();
    final Paint mEdgeGradientPaint = new Paint();

    /**
     * @param context
     *            Context
     * @param attrs
     *            AttributeSet
     */
    public LinearColorBar(Context context, AttributeSet attrs) {
        super(context, attrs);
        setWillNotDraw(false);
        mPaint.setStyle(Paint.Style.FILL);
        mColorGradientPaint.setStyle(Paint.Style.FILL);
        mColorGradientPaint.setAntiAlias(true);
        mEdgeGradientPaint.setStyle(Paint.Style.STROKE);
        mLineWidth = getResources().getDisplayMetrics().densityDpi >= DisplayMetrics.DENSITY_HIGH
                ? 2 : 1;
        mEdgeGradientPaint.setStrokeWidth(mLineWidth);
        mEdgeGradientPaint.setAntiAlias(true);

    }

    /**
     * @param red
     *            float
     * @param yellow
     *            float
     * @param green
     *            float
     */
    public void setRatios(float red, float yellow, float green) {
        mRedRatio = red;
        mYellowRatio = yellow;
        invalidate();
    }

    /**
     * @param showingGreen
     *            boolean
     */
    public void setShowingGreen(boolean showingGreen) {
        if (mShowingGreen != showingGreen) {
            mShowingGreen = showingGreen;
            invalidate();
        }
    }

    /**
     * @param left
     *            float
     * @param right
     *            float
     */
    public void setGradientPaint(float left, float right) {
        // Utils.logd(TAG, "Left = " + left + "; Right = " + right);
        mInterestingLeftRadio = left;
        mInterestingRightRadio = right;
    }

    private void updateIndicator() {
        int off = getPaddingTop() - getPaddingBottom();
        if (off < 0) {
            off = 0;
        }
        mRect.top = off;
        mRect.bottom = getHeight();
        if (mShowingGreen) {
            mColorGradientPaint.setShader(new LinearGradient(0, 0, 0, off - 2,
                    mRightColor & 0xffffff, mRightColor, Shader.TileMode.CLAMP));
        } else {
            mColorGradientPaint.setShader(new LinearGradient(0, 0, 0, off - 2,
                    mMiddleColor & 0xffffff, mMiddleColor, Shader.TileMode.CLAMP));
        }
        mEdgeGradientPaint.setShader(new LinearGradient(0, 0, 0, off / 2, 0x00a0a0a0, 0xffa0a0a0,
                Shader.TileMode.CLAMP));
    }

    @Override
    protected void onSizeChanged(int w, int h, int oldw, int oldh) {
        super.onSizeChanged(w, h, oldw, oldh);
        updateIndicator();
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);

        int width = getWidth();

        int left = 0;

        int right = left + (int) (width * mRedRatio);
        int right2 = right + (int) (width * mYellowRatio);

        mColorPath.reset();
        mEdgePath.reset();
        if (mInterestingLeftRadio < mInterestingRightRadio) {
            int indicatorLeft = (int) (width * mInterestingLeftRadio);
            int indicatorRight = (int) (width * mInterestingRightRadio);
            final int midTopY = mRect.top;
            final int midBottomY = 0;
            final int xoff = 2;
            mColorPath.moveTo(indicatorLeft, mRect.top);
            mColorPath.cubicTo(indicatorLeft, midBottomY, -xoff, midTopY, -xoff, 0);
            mColorPath.lineTo(width + xoff - 1, 0);
            mColorPath.cubicTo(width + xoff - 1, midTopY, indicatorRight, midBottomY,
                    indicatorRight, mRect.top);
            mColorPath.close();
            final float lineOffset = mLineWidth + .5f;
            mEdgePath.moveTo(-xoff + lineOffset, 0);
            mEdgePath.cubicTo(-xoff + lineOffset, midTopY, indicatorLeft + lineOffset, midBottomY,
                    indicatorLeft + lineOffset, mRect.top);
            mEdgePath.moveTo(width + xoff - 1 - lineOffset, 0);
            mEdgePath.cubicTo(width + xoff - 1 - lineOffset, midTopY, indicatorRight - lineOffset,
                    midBottomY, indicatorRight - lineOffset, mRect.top);
        }

        if (!mEdgePath.isEmpty()) {
            canvas.drawPath(mEdgePath, mEdgeGradientPaint);
            canvas.drawPath(mColorPath, mColorGradientPaint);
        }

        if (left < right) {
            mRect.left = left;
            mRect.right = right;
            mPaint.setColor(mLeftColor);
            canvas.drawRect(mRect, mPaint);
            width -= (right - left);
            left = right;
        }

        right = right2;

        if (left < right) {
            mRect.left = left;
            mRect.right = right;
            mPaint.setColor(mMiddleColor);
            canvas.drawRect(mRect, mPaint);
            width -= (right - left);
            left = right;
        }

        right = left + width;
        if (left < right) {
            mRect.left = left;
            mRect.right = right;
            mPaint.setColor(mRightColor);
            canvas.drawRect(mRect, mPaint);
        }
    }
}
