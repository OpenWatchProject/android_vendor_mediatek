/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.engineermode.desenseat;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.util.AttributeSet;
import android.view.View;

import java.util.ArrayList;

/**
 * Custom view to show a curve.
 */
public class CurveView extends View {
    private static final String TAG = "DesenseAT/CurveView";

    private static final int[] COLOR = {
        Color.GREEN,
        Color.BLUE,
        Color.MAGENTA,
        Color.BLACK,
        Color.RED
    };
    // coordinate
    private static final int TOP_GAP = 10;
    private static final int BOTTOM_GAP = 20;
    private static final int LEFT_GAP = 20;
    private static final int RIGHT_GAP = 10;
    private static final int FONT_SIZE = 10;
    private static final int TEXT_OFFSET_X = 20;
    private static final int TEXT_OFFSET_Y = 10;
    private static final int MAX_COUNT = 20;

    private ArrayList<float[]> mData = null;
    private int mXMin = 0;
    private int mYMin = 0;
    private int mXMax = 0;
    private int mYMax = 0;
    private int mXStep = 0;
    private int mYStep = 0;
    private int mXCount = 0;
    private int mYCount = 0;

    /**
     * Constuctor.
     *
     * @param context
     *              the context
     */
    public CurveView(Context context) {
        super(context);
    }

    /**
     * Constuctor.
     *
     * @param context
     *              the context
     * @param attrs
     *              attributes
     */
    public CurveView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    void setData(TestResult result) {
        if (result == null) {
            return;
        }

        mData = new ArrayList<float[]>();

        for (int k = 0; k < DesenseAtActivity.MAX_SV_COUNT; k++) {
            TestResult.Entry entry = result.subResults.get(k);
            if (entry == null) {
                mData.add(null);
            } else {
                float[] data = new float[entry.data.size()];
                for (int i = 0; i < data.length; i++) {
                    data[i] = entry.data.get(i);
                }
                mData.add(data);
            }
        }

        calculateAxis();
        invalidate();
    }

    void setDataList(ArrayList<float[]> data) {

        mData = data;
        calculateAxis();
        invalidate();
    }

    private void calculateAxis() {
        // Find x/y min/max value of all points
        float xMin = Float.POSITIVE_INFINITY;
        float yMin = Float.POSITIVE_INFINITY;
        float xMax = Float.NEGATIVE_INFINITY;
        float yMax = Float.NEGATIVE_INFINITY;
        for (float[] data : mData) {
            if (data == null) {
                continue;
            }
            for (int i = 0; i < data.length; i += 2) {
                float x = data[i];
                float y = data[i + 1];
                if (x < xMin) {
                    xMin = x;
                }
                if (y < yMin) {
                    yMin = y;
                }
                if (x > xMax) {
                    xMax = x;
                }
                if (y > yMax) {
                    yMax = y;
                }
            }
        }

        // Decide proper X-axis min/max value
        mXStep = 0;
        mXCount = MAX_COUNT + 1;
        while (mXCount > MAX_COUNT) {
            mXStep = increase(mXStep);
            mXMin = (int) Math.floor(xMin / mXStep) * mXStep;
            mXMax = (int) Math.ceil(xMax / mXStep) * mXStep;
            mXCount = (mXMax - mXMin) / mXStep;
        }
        if (mXCount == 0) {
            mXCount = 1;
            mXMin = mXMax - mXStep;
        }

        // Decide proper Y-axis min/max value
        mYStep = 0;
        mYCount = MAX_COUNT + 1;
        while (mYCount > MAX_COUNT) {
            mYStep = increase(mYStep);
            mYMin = (int) Math.floor(yMin / mYStep) * mYStep;
            mYMax = (int) Math.ceil(yMax / mYStep) * mYStep;
            mYCount = (mYMax - mYMin) / mYStep;
        }
        if (mYCount == 0) {
            mYCount = 1;
            mYMin = mYMax - mYStep;
        }
    }

    private int increase(int value) {
        // 1 -> 2 -> 5 -> 10 -> 20 -> 50 -> 100 -> ...
        if (value == 0) {
            value = 1;
        } else if (String.valueOf(value).startsWith("1") || String.valueOf(value).startsWith("5")) {
            value *= 2;
        } else {
            value /= 2;
            value *= 5;
        }
        return value;
    }

    @Override
    protected void onMeasure(int widthSpec, int heightSpec) {
        int width = MeasureSpec.getSize(widthSpec);
        int height = MeasureSpec.getSize(heightSpec);
        int size = width > height ? height : width;
        setMeasuredDimension(size, size);
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        if (mData == null || mData.size() == 0) {
            return;
        }
        final int width = getWidth();
        final int height = getHeight();
        final float originX = LEFT_GAP;
        final float originY = height - BOTTOM_GAP;

        Paint paint = new Paint();
        paint.setStyle(Paint.Style.STROKE);
        paint.setAntiAlias(true);
        paint.setColor(Color.BLACK);
        paint.setTextSize(FONT_SIZE);

        // Convert data to view coordicate
        Matrix m = new Matrix();
        m.postScale((float) (width - LEFT_GAP - RIGHT_GAP) / (mXMax - mXMin),
                (float) -(height - BOTTOM_GAP - TOP_GAP) / (mYMax - mYMin), mXMin, mYMin);
        m.postTranslate(originX - mXMin, originY - mYMin);

        // Draw background
        canvas.drawColor(Color.WHITE);

        // Draw text
        for (int i = 0; i <= mXCount; i++) {
            int x = mXMin + mXStep * i;
            int y = mYMin;
            float[] p = new float[] {x, y};
            m.mapPoints(p);
            p[1] += TEXT_OFFSET_Y;
            canvas.drawText(Integer.toString(x), p[0], p[1], paint);
        }
        for (int i = 0; i <= mYCount; i++) {
            int x = mXMin;
            int y = mYMin + mYStep * i;
            float[] p = new float[] {x, y};
            m.mapPoints(p);
            p[0] -= TEXT_OFFSET_X;
            canvas.drawText(Integer.toString(y), p[0], p[1], paint);
        }

        canvas.concat(m);

        // Draw X axis
        canvas.drawLine(mXMin, mYMin, mXMax, mYMin, paint);

        // Draw Y axis
        canvas.drawLine(mXMin, mYMin, mXMin, mYMax, paint);

        // Draw other lines
        paint.setColor(Color.LTGRAY);
        for (int x = mXMin + mXStep; x <= mXMax; x += mXStep) {
            canvas.drawLine(x, mYMin, x, mYMax, paint);
        }
        for (int y = mYMin + mYStep; y <= mYMax; y += mYStep) {
            canvas.drawLine(mXMin, y, mXMax, y, paint);
        }

        // Draw curve
        paint.setStrokeWidth((float) 0.02);

        int size = mData.size();
        for (int k = 0; k < size; k++) {
            float[] data = mData.get(k);
            if (data == null) {
                continue;
            }
            paint.setColor(COLOR[k]);
            if (data.length > 2) {
                canvas.drawLines(data, paint);
                canvas.drawLines(data, 2, data.length - 2, paint);
            } else {
                canvas.drawPoints(data, paint);
            }
        }
    }
}
