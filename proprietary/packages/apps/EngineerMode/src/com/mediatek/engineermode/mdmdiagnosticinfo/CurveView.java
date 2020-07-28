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

package com.mediatek.engineermode.mdmdiagnosticinfo;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.DashPathEffect;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.RectF;
import android.util.AttributeSet;
import android.view.GestureDetector;
import android.view.MotionEvent;
import android.view.ScaleGestureDetector;
import android.view.View;

import com.mediatek.engineermode.Elog;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;

/**
 * Custom view to show a curve.
 */
public class CurveView extends View {
    private static final String TAG = "EmInfo";

    private boolean mScaling;

    class GestureListener implements GestureDetector.OnGestureListener,
            GestureDetector.OnDoubleTapListener, ScaleGestureDetector.OnScaleGestureListener {
        private static final int DIRECTION_UNKNOWN = 0;
        private static final int DIRECTION_HORIZONTAL = 1;
        private static final int DIRECTION_VERTICAL = 2;

        private float[] mValues = new float[9];
        private int mScaleDirection;
        private int mScrollDirection;

        @Override
        public boolean onScale(ScaleGestureDetector detector) {
            float scale = detector.getScaleFactor();
            //Elog.v(TAG, "[CurveView] onScale " + scale);
            float scaleX = scale;
            float scaleY = scale;
            //Elog.v(TAG, "[CurveView] mScaleDirection: " + mScaleDirection);
            scaleX = scaleX < (getWidth() / (float) mViewRect.width()) ?
                    scaleX : (getWidth() / (float) mViewRect.width());
            scaleY = scaleY < (getHeight() / (float) mViewRect.height()) ?
                    scaleY : (getHeight() / (float) mViewRect.height());
            if (mScaleDirection == DIRECTION_UNKNOWN) {
                //Elog.v(TAG, "[CurveView] getCurrentSpanX: " + detector.getCurrentSpanX());
                //Elog.v(TAG, "[CurveView] getCurrentSpanY: " + detector.getCurrentSpanY());
                if (detector.getCurrentSpanX() > detector.getCurrentSpanY()) {
                    mScaleDirection = DIRECTION_HORIZONTAL;
                    scaleY = 1;
                } else {
                    mScaleDirection = DIRECTION_VERTICAL;
                    scaleX = 1;
                }
            }
            doScale(scaleX, scaleY, detector.getFocusX(), detector.getFocusY());
            snapBack();
            updateAxisLabel();
            CurveView.this.invalidate();
            return true;
        }

        private void doScale(float scaleX, float scaleY, float x, float y) {
            //Elog.v(TAG, "[CurveView] doScale " + scaleX + "," + scaleY);
            if (mMatrix != null) {
                mMatrix.postScale(scaleX, scaleY, x, y);
            }
        }

        @Override
        public boolean onScaleBegin(ScaleGestureDetector detector) {
            //Elog.v(TAG, "[CurveView] onScaleBegin");
            mScaling = true;
            return true;
        }

        @Override
        public void onScaleEnd(ScaleGestureDetector detector) {
            //Elog.v(TAG, "[CurveView] onScaleEnd");
            mScaling = false;
            mScaleDirection = DIRECTION_UNKNOWN;
        }

        @Override
        public boolean onScroll(MotionEvent e1, MotionEvent e2, float distanceX, float distanceY) {
            //Elog.v(TAG, "[CurveView] onScroll mScaling = " + mScaling);
            if (!mScaling) {
                //Elog.v(TAG, "[CurveView] mScrollDirection: " + mScrollDirection);
                if (mScrollDirection == DIRECTION_UNKNOWN) {
                    //Elog.v(TAG, "[CurveView] distanceX: " + distanceX);
                    //Elog.v(TAG, "[CurveView] distanceY: " + distanceY);
                    if (Math.abs(distanceX) > Math.abs(distanceY)) {
                        mScrollDirection = DIRECTION_HORIZONTAL;
                    } else {
                        mScrollDirection = DIRECTION_VERTICAL;
                    }
                }
                if (mScrollDirection == DIRECTION_HORIZONTAL) {
                    distanceY = 0;
                }
                if (mScrollDirection == DIRECTION_VERTICAL) {
                    distanceX = 0;
                }

                doTranslate(distanceX, distanceY);
                snapBack();
                CurveView.this.invalidate();
                return true;
            }
            return false;
        }

        private void doTranslate(float distanceX, float distanceY) {
            //Elog.v(TAG, "[CurveView] doTranslate " + distanceX + ", " + distanceY);
            if (mMatrix != null) {
                mMatrix.postTranslate(-distanceX, -distanceY);
            }
        }

        private void snapBack() {
            if (mMatrix == null) {
                return;
            }
            float scaleX = 1;
            float scaleY = 1;
            float[] p1 = new float[] {mXAxis.min, mYAxis.min};
            float[] p2 = new float[] {mXAxis.max, mYAxis.max};
            mMatrix.mapPoints(p1);
            mMatrix.mapPoints(p2);
            if (Math.abs(p1[0] - p2[0]) < mViewRect.width()) {
                scaleX = mViewRect.width() / Math.abs(p1[0] - p2[0]);
            }
            if (Math.abs(p1[1] - p2[1]) < mViewRect.height()) {
                scaleY = mViewRect.height() / Math.abs(p1[1] - p2[1]);
            }
            //Elog.v(TAG, "[CurveView] p1 " + p1[0] + "," + p1[1]);
            //Elog.v(TAG, "[CurveView] p2 " + p2[0] + "," + p2[1]);
            //Elog.v(TAG, "[CurveView] snap back scale " + scaleX + "," + scaleY);
            mMatrix.postScale(scaleX, scaleY);

            float transX = 0;
            float transY = 0;
            p1 = new float[] {mXAxis.min, mYAxis.min};
            p2 = new float[] {mXAxis.max, mYAxis.max};
            mMatrix.mapPoints(p1);
            mMatrix.mapPoints(p2);
            if (p1[0] > mViewRect.left) {
                transX = -(p1[0] - mViewRect.left);
            } else if (p2[0] < mViewRect.right) {
                transX = -(p2[0] - mViewRect.right);
            }
            if (p1[1] < mViewRect.bottom) {
                transY = -(p1[1] - mViewRect.bottom);
            } else if (p2[1] > mViewRect.top) {
                transY = -(p2[1] - mViewRect.top);
            }
            //Elog.v(TAG, "[CurveView] p1 " + p1[0] + "," + p1[1]);
            //Elog.v(TAG, "[CurveView] p2 " + p2[0] + "," + p2[1]);
            //Elog.v(TAG, "[CurveView] snap back " + transX + "," + transY);
            mMatrix.postTranslate(transX, transY);
        }

        @Override
        public boolean onDown(MotionEvent e) {
            //Elog.v(TAG, "[CurveView] onDown");
            mScrollDirection = DIRECTION_UNKNOWN;
            return true;
        }

        @Override
        public void onShowPress(MotionEvent e) {
        }

        @Override
        public boolean onSingleTapUp(MotionEvent e) {
            return true;
        }

        @Override
        public void onLongPress(MotionEvent e) {
        }

        @Override
        public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY) {
            return true;
        }

        @Override
        public boolean onSingleTapConfirmed(MotionEvent e) {
            return true;
        }

        @Override
        public boolean onDoubleTap(MotionEvent e) {
            return true;
        }

        @Override
        public boolean onDoubleTapEvent(MotionEvent e) {
            return true;
        }
    }

    // coordinate
    private static final int TOP_GAP = 70;
    private static final int BOTTOM_GAP = 20;
    private static final int LEFT_GAP = 5;
    private static final int RIGHT_GAP = 10;
    private static final int LINE_LENGTH = 5;
    private static final int FONT_SIZE = 10;
    private static final int TEXT_OFFSET_X = 5;
    private static final int TEXT_OFFSET_Y = 10;
    private static final int MAX_COUNT = 20;

    class Axis {
        long base;
        long min = 0;
        long max = 0;
        long step = 0;
        long count = 0;
        long lastMax = -1;

        void set(float dataMin, float dataMax, AxisConfig config) {
            // Decide proper axis min/max value
            if (config.configMin) {
                if (config.fixedMin) {
                    dataMin = config.min;
                } else {
                    dataMin = Math.min(config.min, dataMin);
                }
            }
            if (config.configMax) {
                if (config.fixedMax) {
                    dataMax = config.max;
                } else {
                    dataMax = Math.max(config.max, dataMax);
                }
            }
            if (config.configStep) {
                step = config.step;
                min = (long) Math.floor(dataMin / step) * step;
                max = (long) Math.ceil(dataMax / step) * step;
                count = (max - min) / step;
                if (config.configMaxCount && count > config.maxCount) {
                    while (count > config.maxCount) {
                        step = increase(step);
                        min = (long) Math.floor(dataMin / step) * step;
                        max = (long) Math.ceil(dataMax / step) * step;
                        count = (max - min) / step;
                    }
                }
            } else {
                step = 0;
                count = MAX_COUNT + 1;
                while (count > MAX_COUNT) {
                    step = increase(step);
                    min = (long) Math.floor(dataMin / step) * step;
                    max = (long) Math.ceil(dataMax / step) * step;
                    count = (max - min) / step;
                }
                if (count == 0) {
                    count = 1;
                    min = max - step;
                }
            }
            base = config.base;
            Elog.d(TAG, "[CurveView][Axis][set] min: " + min + " max: " + max +
                    " step: " + step+ " count: " + count);
        }
    }

    Axis mXAxis = new Axis();
    Axis mYAxis = new Axis();

    class AxisLabel {
        long min = 0;
        long max = 0;
        long step = 0;
        long count = 0;
        int type;

        void set(Axis axis, AxisConfig config) {
            step = axis.step;
            min = axis.min;
            max = axis.max;
            count = axis.count;
            type = config.type;

            if (type == AxisConfig.TYPE_TIME) {
                int maxWidth = 0;
                String str = "HH:mm:ss";
                Rect rect = new Rect();
                mTextPaint.getTextBounds(str, 0, str.length(), rect);
                if (2 * rect.width() > maxWidth) {
                    maxWidth = 2 * rect.width();
                }

                while (mViewRect.width() / count < maxWidth) {

                    step = increase(step);
                    min = (long) Math.floor(min / step) * step;
                    max = (long) Math.ceil(max / step) * step;
                    count = (max - min) / step;
                }
            }
            if (type == AxisConfig.TYPE_AUTO_SCALE) {
                int maxHeight = 0;
                String str = "100000";
                Rect rect = new Rect();
                mTextPaint.getTextBounds(str, 0, str.length(), rect);
                if (5 * rect.height() > maxHeight) {
                    maxHeight = 5 * rect.height();
                }
                while (mViewRect.height() / count < maxHeight) {

                    step = increase(step);
                    min = (long) Math.floor(min / step) * step;
                    max = (long) Math.ceil(max / step) * step;
                    count = (max - min) / step;
                }
            }
            Elog.d(TAG, "[CurveView][AxisLabel][set] minAxisLabel: " + min +
                    " maxAxisLabel: " + max +
                    " step: " + step+ " count: " + count);
        }
    }

    AxisLabel mXAxisLabel = new AxisLabel();
    AxisLabel mYAxisLabel = new AxisLabel();
//    float[] mXAxisLabelPoints;
//    float[] mYAxisLabelPoints;

    public static class AxisConfig {
        static int TYPE_NORMAL = 0;
        static int TYPE_TIME = 1;
        static int TYPE_AUTO_SCALE = 3;
        boolean configMin;
        boolean configMax;
        boolean configStep;
        boolean fixedMin;
        boolean fixedMax;
        boolean fixedStep;
        boolean configMaxCount = false;
        long min;
        long max;
        long step;
        long maxCount;
        long base;
        int type;
        HashMap<Integer, String> customLabel;
    }

    AxisConfig mXAxisConfig = null;
    AxisConfig mYAxisConfig = null;

    boolean test = false;
    public void setAxisConfig(AxisConfig xConfig, AxisConfig yConfig) {
        mXAxisConfig = xConfig;
        mYAxisConfig = yConfig;
        test = true;
        invalidate();
    }

    public static class Config {
        static int TYPE_CIRCLE = 0;
        static int TYPE_CROSS = 1;
        static int LINE_SOLID = 0;
        static int LINE_DASH = 1;
        int nodeType;
        int lineType;
        int lineWidth;
        int color = Color.BLUE;
        String name;
        int newLineThreadshold;
    }

    private ArrayList<Config> mConfigs;

    public void setConfig(Config[] configs) {
        mConfigs = new ArrayList<Config>();
        for (int i = 0; i < configs.length; i++) {
            mConfigs.add(configs[i]);
        }
    }

    private ArrayList<float[]> mData = null;
//    private ArrayList<float[]> mPoints = null;
    float mXMin = Float.POSITIVE_INFINITY;
    float mYMin = Float.POSITIVE_INFINITY;
    float mXMax = Float.NEGATIVE_INFINITY;
    float mYMax = Float.NEGATIVE_INFINITY;
    int drawIndex = 0;
    private int mWidth = 0;
    private int mHeight = 0;
    private Matrix mMatrix;
    private GestureDetector mGestureDetector;
    private ScaleGestureDetector mScaleDetector;
    private boolean mAutoScroll = true;
    private RectF mViewRect = new RectF();
    private Paint mPaint;
    private Paint mTextPaint;
    private Paint mTextPaint2;
    private Paint mBoldPaint;
    private Paint mLightPaint;

    /**
     * Constuctor.
     *
     * @param context
     *              the context
     */
    public CurveView(Context context) {
        super(context);
        init(context);
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
        init(context);
    }

    private void init(Context context) {
        GestureListener listener = new GestureListener();
        mGestureDetector = new GestureDetector(context, listener, null, true);
        mScaleDetector = new ScaleGestureDetector(context, listener);

        mPaint = new Paint();
        mPaint.setStyle(Paint.Style.STROKE);
        mPaint.setAntiAlias(true);

        mBoldPaint = new Paint();
        mBoldPaint.setStyle(Paint.Style.STROKE);
        mBoldPaint.setAntiAlias(false);
        mBoldPaint.setColor(Color.BLACK);

        mLightPaint = new Paint();
        mLightPaint.setStyle(Paint.Style.STROKE);
        mLightPaint.setAntiAlias(false);
        mLightPaint.setColor(Color.LTGRAY);

        mTextPaint = new Paint();
        mTextPaint.setStyle(Paint.Style.STROKE);
        mTextPaint.setAntiAlias(true);
        mTextPaint.setColor(Color.BLACK);
        mTextPaint.setTextSize(FONT_SIZE);

        mTextPaint2 = new Paint();
        mTextPaint2.setStyle(Paint.Style.STROKE);
        mTextPaint2.setAntiAlias(true);
        mTextPaint2.setColor(Color.BLACK);
        mTextPaint2.setTextSize(12);
    }

    public void setData(int index, float[] data) {
        if (data == null || data.length <= 1) {
            return;
        }
        if (mWidth == 0 || mHeight == 0) {
            return;
        }
        if (mData == null) {
            mData = new ArrayList<float[]>();
            for (int i = 0; i < mConfigs.size(); i++) {
                mData.add(null);
            }
        }
        //for (int i = 0; i < data.length; i ++) {
        //    Elog.v(TAG, "[CurveView] data[" + i + "] = " + data[i]);
        //}

        mData.set(index, data);
        checkData();
        updateAxis();

        if (mAutoScroll) {
            makeLastVisible();
        }
        if (!mScaling) {
            invalidate();
        }
    }

    private void makeLastVisible() {
        float transX = 0;
        float transY = 0;
        float[] p = new float[] {mXMax, mYMax};
        mMatrix.mapPoints(p);
        if (p[0] > mViewRect.right) {
            transX = -(p[0] - mViewRect.right);
        }
        mMatrix.postTranslate(transX, 0);
        mXAxisLabel.set(mXAxis, mXAxisConfig);
    }

    private void checkData() {
        // Find x/y min/max value of all points
        for (float[] data : mData) {
            if (data == null) {
                continue;
            }
            for (int i = drawIndex; i < data.length; i += 2) {
                float x = data[i];
                float y = data[i + 1];
                if (x < mXMin) {
                    mXMin = x;
                }
                if (y < mYMin) {
                    mYMin = y;
                }
                if (x > mXMax) {
                    mXMax = x;
                }
                if (y > mYMax) {
                    mYMax = y;
                }
            }
            drawIndex = data.length;
        }
    }

    private void updateAxis() {
        // update axis
        mXAxis.set(mXMin, mXMax, mXAxisConfig);
        mYAxis.set(mYMin, mYMax, mYAxisConfig);
        if (mYAxis.lastMax >= mYAxis.max) {
            return;
        }

        int width = 0;
        Rect rect = new Rect();
        if (mYAxisConfig.customLabel != null) {
            for (String str : mYAxisConfig.customLabel.values()) {
                mTextPaint.getTextBounds(str, 0, str.length(), rect);
                if (rect.width() > width) {
                    width = rect.width();
                }
            }
        } else {
            String str = String.valueOf(mYMax);
            mTextPaint.getTextBounds(str, 0, str.length(), rect);
            if (rect.width() > width) {
                width = rect.width();
            }
            mYAxis.lastMax = mYAxis.max;
        }
        mViewRect.set(width + LEFT_GAP, TOP_GAP, mWidth - RIGHT_GAP, mHeight - BOTTOM_GAP);

        if (mMatrix == null ) {
            mMatrix = new Matrix();
            setMatrix();
        } else if (mYAxisConfig.type == AxisConfig.TYPE_AUTO_SCALE) {
            mMatrix = new Matrix();
            setMatrix();
        }
        updateAxisLabel();

    }

    private void setMatrix() {
        mMatrix.postScale((float) (mViewRect.width()) / (mXAxis.max - mXAxis.min),
                (float) -(mViewRect.height()) / (mYAxis.max - mYAxis.min),
                mXAxis.min, mYAxis.min);
        mMatrix.postTranslate(mViewRect.left - mXAxis.min, mViewRect.bottom - mYAxis.min);
    }

    private void updateAxisLabel() {
        mXAxisLabel.set(mXAxis, mXAxisConfig);
        mYAxisLabel.set(mYAxis, mYAxisConfig);
    }

    @Override
    protected void onMeasure(int widthSpec, int heightSpec) {
        super.onMeasure(widthSpec, heightSpec);
        mWidth = MeasureSpec.getSize(widthSpec);
        mHeight = MeasureSpec.getSize(heightSpec);
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        mGestureDetector.onTouchEvent(event);
        mScaleDetector.onTouchEvent(event);
        return true;
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        if (mXAxisConfig == null || mYAxisConfig == null) {
            return;
        }
        if (test) {
            updateAxis();
            test = false;
        }
        final int width = getWidth();
        final int height = getHeight();
        //Elog.v(TAG, "[CurveView] width = " + width);
        //Elog.v(TAG, "[CurveView] height = " + height);
        Matrix m = mMatrix;
        RectF r = mViewRect;
        // decide auto scroll or not
        float[] p = new float[] {mXMax, 0};
        m.mapPoints(p);
        if (p[0] <= width) {
            mAutoScroll = true;
        } else {
            mAutoScroll = false;
        }

        // Draw background
        canvas.drawColor(Color.WHITE);

        // Draw X axis
        canvas.drawLine(r.left, r.bottom, r.right, r.bottom, mBoldPaint);

        // Draw Y axis
        canvas.drawLine(r.left, r.top, r.left, r.bottom, mBoldPaint);

        // Draw X axis calibration
        canvas.save();
        canvas.clipRect(r.left, 0, mWidth, mHeight);
        // Draw long lines
        for (long x = mXAxisLabel.min; x <= mXAxisLabel.max; x += mXAxisLabel.step) {
            p = new float[] {x, 0};
            m.mapPoints(p);
            if (Math.abs(p[0] - r.left) > 1) {
                canvas.drawLine(p[0], r.top, p[0], r.bottom, mLightPaint);
            }
/*
            // Show text
            if (mXAxisLabel.type == AxisConfig.TYPE_TIME) {
                String str = formatTime((x + mXAxis.base) * 1000);
                Rect rect = new Rect();
                mTextPaint.getTextBounds(str, 0, str.length(), rect);
                canvas.drawText(str, p[0] - rect.width() / 2, r.bottom + TEXT_OFFSET_Y, mTextPaint);
            } else {
                canvas.drawText(Long.toString(x), p[0], r.bottom + TEXT_OFFSET_Y, mTextPaint);
            }
*/
        }
        // Draw short lines
        for (long x = mXAxis.min; x <= mXAxis.max; x += mXAxis.step) {
            p = new float[] {x, 0};
            m.mapPoints(p);
            canvas.drawLine(p[0], r.bottom, p[0], r.bottom - 5, mBoldPaint);
        }
        canvas.restore();

        for (long x = mXAxisLabel.min; x <= mXAxisLabel.max; x += mXAxisLabel.step) {
            // Show text
            p = new float[] {x, 0};
            m.mapPoints(p);
            if (mXAxisLabel.type == AxisConfig.TYPE_TIME) {
                String str = formatTime((x + mXAxis.base) * 1000);
                Rect rect = new Rect();
                mTextPaint.getTextBounds(str, 0, str.length(), rect);
                canvas.drawText(str, p[0] - rect.width() / 2, r.bottom + TEXT_OFFSET_Y, mTextPaint);
            } else {
                canvas.drawText(Long.toString(x), p[0], r.bottom + TEXT_OFFSET_Y, mTextPaint);
            }
        }

        // Draw Y axis calibration
        canvas.save();
        canvas.clipRect(0, r.top, mWidth, r.bottom);
        // Draw long lines
        for (long y = mYAxisLabel.min; y <= mYAxisLabel.max; y += mYAxisLabel.step) {
            p = new float[] {0, y};
            m.mapPoints(p);
            if (Math.abs(p[1] - r.bottom) > 1) {
                canvas.drawLine(r.left, p[1], r.right, p[1], mLightPaint);
            }
            // Show Y axis text
            String text = "";
            if (mYAxisConfig.customLabel != null) {
                text = mYAxisConfig.customLabel.get((int) y);
                if (text == null) {
                    text = "";
                }
            } else {
                text = Long.toString(y);
            }
            canvas.drawText(text, LEFT_GAP, p[1], mTextPaint);
        }
        // Draw short lines
        long step = mYAxisConfig.type == mYAxisConfig.TYPE_AUTO_SCALE ? mYAxisLabel.step / 5
                : mYAxis.step;
        for (long y = mYAxis.min; y <= mYAxis.max; y += step) {
            p = new float[] {0, y};
            m.mapPoints(p);
            canvas.drawLine(r.left, p[1], r.left + 5, p[1], mBoldPaint);
        }
        canvas.restore();

        drawSeries(canvas, new RectF(10, 10, mWidth, 80));

        if (mData == null || mData.size() < 1) {
            return;
        }

        // Draw curve
        canvas.save();
        canvas.clipRect(r.left, r.top, mWidth, r.bottom);
        for (int i = 0; i < mData.size(); i++) {
            float[] data = mData.get(i);
            if (data == null) {
                continue;
            }
            mPaint.setColor(mConfigs.get(i).color);
            mPaint.setStrokeWidth(mConfigs.get(i).lineWidth);
            String name = mConfigs.get(i).name;
            mConfigs.get(i).name = name.replaceAll(
                    "\\([0-9]+\\.[0-9]+\\)",
                    "(" + data[data.length - 1] + ")");
            if (mConfigs.get(i).lineType == Config.LINE_DASH) {
                mPaint.setPathEffect(new DashPathEffect(new float[]{4, 2}, 0));
            } else {
                mPaint.setPathEffect(null);
            }
            int nodeType = mConfigs.get(i).nodeType;

            float[] tmp = new float[data.length];
            m.mapPoints(tmp, data);
            // Draw vertex
            for (int j = 0; j < tmp.length; j += 2) {
                if (nodeType == Config.TYPE_CIRCLE) {
                    canvas.drawCircle(tmp[j], tmp[j + 1], 5, mPaint);
                } else if (nodeType == Config.TYPE_CROSS) {
                    canvas.drawLine(tmp[j] - 5, tmp[j + 1] - 5, tmp[j] + 5, tmp[j + 1] + 5, mPaint);
                    canvas.drawLine(tmp[j] + 5, tmp[j + 1] - 5, tmp[j] - 5, tmp[j + 1] + 5, mPaint);
                }
            }
            // Draw lines
            //Elog.v(TAG, "[CurveView] drawLines " + data.length);
            if (data.length > 2) {
                if (mConfigs.get(i).newLineThreadshold == 0) {
                    canvas.drawLines(tmp, mPaint);
                    canvas.drawLines(tmp, 2, tmp.length - 2, mPaint);
                } else {
                    for (int j = 0; j < data.length - 2; j += 2) {
                        if (Math.abs(data[j + 2] - data[j]) < 11) {
                            canvas.drawLine(tmp[j], tmp[j + 1], tmp[j + 2], tmp[j + 3], mPaint);
                        }
                    }
                }
            }
        }
        canvas.restore();
    }

    private void drawSeries(Canvas canvas, RectF rect) {
        canvas.save();
        canvas.clipRect(rect);
/*
        int seriesWidth = 100;
        int textWidth;
        for (int i = 0; i < mConfigs.size(); i++) {
            String str = mConfigs.get(i).name;
            Rect rect = new Rect();
            mTextPaint.getTextBounds(str, 0, str.length(), rect);
            textWidth += rect.width();
        }
        if (seriesWidth * mConfigs.size() + textWidth > mWidth) {
            seriesWidth = (mWidth - textWidth) / mConfigs.size();
        }
*/
        float[] p = new float[2];
        p[0] = rect.left;
        p[1] = (rect.top + rect.bottom) / 2;

        for (int i = 0; i < mConfigs.size(); i++) {
            p[0] += 10;
            mPaint.setColor(mConfigs.get(i).color);
            mPaint.setStrokeWidth(mConfigs.get(i).lineWidth);
            if (mConfigs.get(i).lineType == Config.LINE_DASH) {
                mPaint.setPathEffect(new DashPathEffect(new float[]{4, 2}, 0));
            } else {
                mPaint.setPathEffect(null);
            }

            canvas.drawLine(p[0], p[1], p[0] + 100, p[1], mPaint);

            p[0] += 50;
            int nodeType = mConfigs.get(i).nodeType;
            if (nodeType == Config.TYPE_CIRCLE) {
                canvas.drawCircle(p[0], p[1], 5, mPaint);
            } else if (nodeType == Config.TYPE_CROSS) {
                canvas.drawLine(p[0] - 5, p[1] - 5, p[0] + 5, p[1] + 5, mPaint);
                canvas.drawLine(p[0] + 5, p[1] - 5, p[0] - 5, p[1] + 5, mPaint);
            }

            p[0] += 60;
            String str = mConfigs.get(i).name;
            canvas.drawText(str, p[0], p[1], mTextPaint2);

            Rect r = new Rect();
            mTextPaint2.getTextBounds(str, 0, str.length(), r);
            p[0] += r.width();
        }
//        canvas.drawRect(rect.left, rect.top, p[0] + 10, rect.bottom, mBoldPaint);
        canvas.restore();
    }

    private long increase(long value) {
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

    private String formatTime(long miliSeconds) {
        return new SimpleDateFormat("HH:mm:ss").format(new Date(miliSeconds));
    }
}

