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

package com.mediatek.engineermode.mdmcomponent;

import android.content.Context;
import android.content.SharedPreferences;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.graphics.Paint.Style;
import android.graphics.Path;
import android.graphics.Rect;
import android.graphics.RectF;
import android.graphics.PathEffect;
import android.graphics.DashPathEffect;
import android.util.AttributeSet;
import android.view.GestureDetector;
import android.view.MotionEvent;
import android.view.ScaleGestureDetector;
import android.view.View;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;


import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;

/**
 * Custom view to show a curve.
 */
public class CurveViewEx extends View {
    private static final String TAG = "EmInfo/CurveViewEx";
    private static final String COMPONENT_RSRPSINR_CONFIG_SHAREPRE=
        "telephony_rsrpsinr_config_settings";
    Context mContext = null;

    private static final int STRONG_POINT1_X_DEFAULT = -80;
    private static final int STRONG_POINT1_Y_DEFAULT = 30;
    private static final int STRONG_POINT2_X_DEFAULT = -80;
    private static final int STRONG_POINT2_Y_DEFAULT = 20;
    private static final int STRONG_POINT3_X_DEFAULT = -30;
    private static final int STRONG_POINT3_Y_DEFAULT = 20;

    private static final int MEDIUMWEAK_POINT1_X_DEFAULT = -140;
    private static final int MEDIUMWEAK_POINT1_Y_DEFAULT = 10;
    private static final int MEDIUMWEAK_POINT2_X_DEFAULT = -90;
    private static final int MEDIUMWEAK_POINT2_Y_DEFAULT = 10;
    private static final int MEDIUMWEAK_POINT3_X_DEFAULT = -90;
    private static final int MEDIUMWEAK_POINT3_Y_DEFAULT = -20;

    private static final int WEAK_POINT1_X_DEFAULT = -140;
    private static final int WEAK_POINT1_Y_DEFAULT = 5;
    private static final int WEAK_POINT2_X_DEFAULT = -100;
    private static final int WEAK_POINT2_Y_DEFAULT = 5;
    private static final int WEAK_POINT3_X_DEFAULT = -100;
    private static final int WEAK_POINT3_Y_DEFAULT = -20;

    private static int strongPoint1X = STRONG_POINT1_X_DEFAULT;
    private static int strongPoint1Y = STRONG_POINT1_Y_DEFAULT;
    private static int strongPoint2X = STRONG_POINT2_X_DEFAULT;
    private static int strongPoint2Y = STRONG_POINT2_Y_DEFAULT;
    private static int strongPoint3X = STRONG_POINT3_X_DEFAULT;
    private static int strongPoint3Y = STRONG_POINT3_Y_DEFAULT;
    private static int mediumWeakPoint1X = MEDIUMWEAK_POINT1_X_DEFAULT;
    private static int mediumWeakPoint1Y = MEDIUMWEAK_POINT1_Y_DEFAULT;
    private static int mediumWeakPoint2X = MEDIUMWEAK_POINT2_X_DEFAULT;
    private static int mediumWeakPoint2Y = MEDIUMWEAK_POINT2_Y_DEFAULT;
    private static int mediumWeakPoint3X = MEDIUMWEAK_POINT3_X_DEFAULT;
    private static int mediumWeakPoint3Y = MEDIUMWEAK_POINT3_Y_DEFAULT;
    private static int weakPoint1X = WEAK_POINT1_X_DEFAULT;
    private static int weakPoint1Y = WEAK_POINT1_Y_DEFAULT;
    private static int weakPoint2X = WEAK_POINT2_X_DEFAULT;
    private static int weakPoint2Y = WEAK_POINT2_Y_DEFAULT;
    private static int weakPoint3X = WEAK_POINT3_X_DEFAULT;
    private static int weakPoint3Y = WEAK_POINT3_Y_DEFAULT;

    private boolean mScaling;

    class GestureListenerEx implements GestureDetector.OnGestureListener,
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
            //Elog.v(TAG, "[CurveViewEx] onScale " + scale);
            float scaleX = scale;
            float scaleY = scale;
            //Elog.v(TAG, "[CurveViewEx] mScaleDirection: " + mScaleDirection);
            if (mScaleDirection == DIRECTION_UNKNOWN) {
                //Elog.v(TAG, "[CurveViewEx] getCurrentSpanX: " + detector.getCurrentSpanX());
                //Elog.v(TAG, "[CurveViewEx] getCurrentSpanY: " + detector.getCurrentSpanY());
                if (detector.getCurrentSpanX() > detector.getCurrentSpanY()) {
                    mScaleDirection = DIRECTION_HORIZONTAL;
                } else {
                    mScaleDirection = DIRECTION_VERTICAL;
                }
            }
            if (mScaleDirection == DIRECTION_HORIZONTAL) {
                scaleY = 1;
            }
            if (mScaleDirection == DIRECTION_VERTICAL) {
                scaleX = 1;
            }
            doScale(scaleX, scaleY, detector.getFocusX(), detector.getFocusY());
            snapBack();
//            updateAxisLabel();
            CurveViewEx.this.invalidate();
            return true;
        }

        private void doScale(float scaleX, float scaleY, float x, float y) {
            //Elog.v(TAG, "[CurveViewEx] doScale " + scaleX + "," + scaleY);
            if (mMatrix != null) {
                mMatrix.postScale(scaleX, scaleY, x, y);
            }
        }

        @Override
        public boolean onScaleBegin(ScaleGestureDetector detector) {
            //Elog.v(TAG, "[CurveViewEx] onScaleBegin");
            mScaling = true;
            return true;
        }

        @Override
        public void onScaleEnd(ScaleGestureDetector detector) {
            //Elog.v(TAG, "[CurveViewEx] onScaleEnd");
            mScaling = false;
            mScaleDirection = DIRECTION_UNKNOWN;
        }

        @Override
        public boolean onScroll(MotionEvent e1, MotionEvent e2, float distanceX, float distanceY) {
            //Elog.v(TAG, "[CurveViewEx] onScroll mScaling = " + mScaling);
            if (!mScaling) {
                //Elog.v(TAG, "[CurveViewEx] mScrollDirection: " + mScrollDirection);
                if (mScrollDirection == DIRECTION_UNKNOWN) {
                    //Elog.v(TAG, "[CurveViewEx] distanceX: " + distanceX);
                    //Elog.v(TAG, "[CurveViewEx] distanceY: " + distanceY);
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
                CurveViewEx.this.invalidate();
                return true;
            }
            return false;
        }

        private void doTranslate(float distanceX, float distanceY) {
            //Elog.v(TAG, "[CurveViewEx] doTranslate " + distanceX + ", " + distanceY);
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
            mMatrix.postTranslate(transX, transY);
        }

        @Override
        public boolean onDown(MotionEvent e) {
            //Elog.v(TAG, "[CurveViewEx] onDown");
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
    private static final int FONT_SIZE = 18;
    private static final int TEXT_OFFSET_X = 16;
    private static final int TEXT_OFFSET_Y = 16;
    private static final int MAX_COUNT = 20;

    class Axis {
        long base;
        long min = 0;
        long max = 0;
        long step = 0;
        long count = 0;

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
            }
            base = config.base;
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
        }
    }

    AxisLabel mXAxisLabel = new AxisLabel();
    AxisLabel mYAxisLabel = new AxisLabel();
//    float[] mXAxisLabelPoints;
//    float[] mYAxisLabelPoints;

    public static class AxisConfig {
        static int TYPE_NORMAL = 0;
        static int TYPE_TIME = 1;
        boolean configMin;
        boolean configMax;
        boolean configStep;
        boolean fixedMin;
        boolean fixedMax;
        boolean fixedStep;
        long min;
        long max;
        long step;
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
        static int TYPE_RHOMBUS = 2;
        static int TYPE_SQUARE = 3;
        static int TYPE_TRIANGLE = 4;
        static int TYPE_NONE = 5;
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
    float mXMin = 0;
    float mYMin = 0;
    float mXMax = -30;
    float mYMax = 0;
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
    private Paint mRegionPaint;
    private int mRegionColor[] = {Color.rgb(43,101,171),
        Color.rgb(204,153,0), Color.rgb(152,152,186)};

    private int mOffsetY = 100;
    private int mStrongX = -80;
    private int mStrongY = 20;
    private int mMediumWeakStartX = -140;
    private int mMediumWeakEndX = -90;
    private int mMediumWeakStartY = -20;
    private int mMediumWeakEndY = 10;
    private int mWeakStartX = -140;
    private int mWeakEndX = -100;
    private int mWeakStartY = -20;
    private int mWeakEndY = 5;
    /**
     * Constuctor.
     *
     * @param context
     *              the context
     */
    public CurveViewEx(Context context) {
        super(context);
        init(context);
        mContext = context;
    }

    /**
     * Constuctor.
     *
     * @param context
     *              the context
     * @param attrs
     *              attributes
     */
    public CurveViewEx(Context context, AttributeSet attrs) {
        super(context, attrs);
        init(context);
        mContext = context;
    }

    private void init(Context context) {
        GestureListenerEx listener = new GestureListenerEx();
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
        mTextPaint2.setTextSize(FONT_SIZE);

        mRegionPaint = new Paint();
        mRegionPaint.setStyle(Paint.Style.STROKE);
        mRegionPaint.setAntiAlias(false);
        mRegionPaint.setTextSize(12);
        mRegionPaint.setStrokeWidth(5);
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
    }

    private void checkData() {
        for (float[] data : mData) {
            if (data == null) {
                continue;
            }
            for (int i = 0; i < data.length; i += 4) {
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
        }
    }

    private void updateAxis() {
        // update axis
        mXAxis.set(mXMin, mXMax, mXAxisConfig);
        mYAxis.set(mYMin, mYMax, mYAxisConfig);

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
            for (long y = mYAxis.min; y <= mYAxis.max; y += mYAxis.step) {
                String str = String.valueOf(y);
                mTextPaint.getTextBounds(str, 0, str.length(), rect);
                if (rect.width() > width) {
                    width = rect.width();
                }
            }
        }
        mViewRect.set(width + LEFT_GAP, TOP_GAP, mWidth - RIGHT_GAP, mHeight - BOTTOM_GAP);

        // Convert data to view coordicate
        if (mMatrix == null) {
            mMatrix = new Matrix();
            mMatrix.postScale((float) (mViewRect.width()) / (mXAxis.max - mXAxis.min),
                    (float) -(mViewRect.height()) / (mYAxis.max - mYAxis.min),
                    mXAxis.min, mYAxis.min);
            mMatrix.postTranslate(mViewRect.left - mXAxis.min, mViewRect.bottom - mYAxis.min);
        }
        updateAxisLabel();
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
            test =false;
        }
        final int width = getWidth();
        final int height = getHeight();
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
                canvas.drawLine(p[0], r.top, p[0], r.bottom, mBoldPaint);
            }
        }
        canvas.restore();

        for (long x = mXAxisLabel.min; x <= mXAxisLabel.max; x += mXAxisLabel.step) {
            // Show text
            p = new float[] {x, 0};
            m.mapPoints(p);
            canvas.drawText(Long.toString(x), p[0] - TEXT_OFFSET_X, r.bottom +
                TEXT_OFFSET_Y, mTextPaint);
        }

        // Draw Y axis calibration
        canvas.save();
        canvas.clipRect(0, r.top - mOffsetY, mWidth, r.bottom);
        // Draw long lines
        for (long y = mYAxisLabel.min; y <= mYAxisLabel.max; y += mYAxisLabel.step) {
            p = new float[] {0, y};
            m.mapPoints(p);
            if (Math.abs(p[1] - r.bottom) > 1) {
                canvas.drawLine(r.left, p[1], r.right, p[1], mBoldPaint);
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
            if( y == mYAxisLabel.max) {
                canvas.drawText(text, LEFT_GAP, p[1] + 20, mTextPaint);
            } else{
                canvas.drawText(text, LEFT_GAP, p[1] - 16, mTextPaint);
            }
        }
        canvas.restore();

        //Draw region
        canvas.save();
        canvas.clipRect(r.left, 0, mWidth, mHeight);
        float[] strongPts = {strongPoint1X,strongPoint1Y,
            strongPoint2X,strongPoint2Y,
            strongPoint2X,strongPoint2Y,
            strongPoint3X,strongPoint3Y};
        //float[] strongRhombus = {mStrongX, (float)(mStrongY+0.5), mStrongX-1, mStrongY,
        //mStrongX, (float)(mStrongY-0.5),mStrongX+1,mStrongY};
        m.mapPoints(strongPts);
        //m.mapPoints(strongRhombus);
        mRegionPaint.setColor(mRegionColor[0]);
        canvas.drawLines(strongPts, mRegionPaint);
        //mRegionPaint.setStyle(Paint.Style.FILL);
        //Path pathRhombus = new Path();
        //pathRhombus.moveTo(strongRhombus[0],strongRhombus[1]);
        //pathRhombus.lineTo(strongRhombus[2],strongRhombus[3]);
        //pathRhombus.lineTo(strongRhombus[4],strongRhombus[5]);
        //pathRhombus.lineTo(strongRhombus[6],strongRhombus[7]);
        //pathRhombus.close();
        //canvas.drawPath(pathRhombus,mRegionPaint);

        float[] mediumWeakPts = {mediumWeakPoint1X,mediumWeakPoint1Y,
            mediumWeakPoint2X,mediumWeakPoint2Y,
            mediumWeakPoint2X,mediumWeakPoint2Y,
            mediumWeakPoint3X,mediumWeakPoint3Y};
        //float[] mediumWeakRect = {mMediumWeakEndX-1,(float)(mMediumWeakEndY+0.5),
        //mMediumWeakEndX+1,(float)(mMediumWeakEndY-0.5)};
        m.mapPoints(mediumWeakPts);
        //m.mapPoints(mediumWeakRect);
        mRegionPaint.setColor(mRegionColor[1]);
        canvas.drawLines(mediumWeakPts, mRegionPaint);
        //canvas.drawRect(mediumWeakRect[0],mediumWeakRect[1],mediumWeakRect[2],
        //mediumWeakRect[3],mRegionPaint);

        float[] weakPts = {weakPoint1X,weakPoint1Y,
            weakPoint2X,weakPoint2Y,
            weakPoint2X,weakPoint2Y,
            weakPoint3X,weakPoint3Y};
        //float[] weakTriangle = {mWeakEndX,(float)(mWeakEndY+0.5),mWeakEndX-1,
        //    (float)(mWeakEndY-0.5),mWeakEndX+1,(float)(mWeakEndY-0.5)};
        m.mapPoints(weakPts);
        //m.mapPoints(weakTriangle);
        mRegionPaint.setColor(mRegionColor[2]);
        canvas.drawLines(weakPts, mRegionPaint);
        //Path pathTriangle = new Path();
        //pathTriangle.moveTo(weakTriangle[0],weakTriangle[1]);
        //pathTriangle.lineTo(weakTriangle[2],weakTriangle[3]);
        //pathTriangle.lineTo(weakTriangle[4],weakTriangle[5]);
        //pathTriangle.close();
        //canvas.drawPath(pathTriangle,mRegionPaint);
        canvas.restore();
        //

        drawSeries(canvas, new RectF(50, 0, mWidth, 86));

        if (mData == null || mData.size() < 1) {
            return;
        }

        // Draw curve
        canvas.save();
        canvas.clipRect(r.left, 0, mWidth, r.bottom);
        float[] pText = new float[2];
        pText[0] = r.left;
        pText[1] = 100;
        canvas.drawText("PCI", pText[0] + 10, pText[1] - 60, mTextPaint2);
        canvas.drawText("EARFCN", pText[0] + 10, pText[1] - 40, mTextPaint2);
        //canvas.drawText("CC0", pText[0] + 100, pText[1] - 60, mTextPaint2);
        //canvas.drawText("CC1", pText[0] + 200, pText[1] - 60, mTextPaint2);
        for (int i = 0; i < mData.size(); i++) {
            float[] data = mData.get(i);
            if (data == null) {
                continue;
            }
            mPaint.setColor(mConfigs.get(i).color);
            mPaint.setStyle(Paint.Style.FILL);
            int nodeType = mConfigs.get(i).nodeType;

            float[] tmp = new float[data.length];
            m.mapPoints(tmp, data);
            // Draw vertex
            for (int j = 0; j < tmp.length; j += 4) {
                if (nodeType == Config.TYPE_CIRCLE) {
                    canvas.drawCircle(tmp[j], tmp[j + 1], 10, mPaint);
                    canvas.drawText(Float.toString(data[j + 2]), pText[0] + 100,
                        pText[1] - 60, mTextPaint2);
                    canvas.drawText(Float.toString(data[j + 3]), pText[0] + 100,
                        pText[1] - 40, mTextPaint2);
                    //canvas.drawText(tmp[j], tmp[j + 1], 10, mPaint);
                } else if (nodeType == Config.TYPE_TRIANGLE) {
                    Path pathTriangleFigure = new Path();
                    pathTriangleFigure.moveTo(tmp[j], tmp[j + 1] - 14);
                    pathTriangleFigure.lineTo(tmp[j] - 14, tmp[j + 1] + 14);
                    pathTriangleFigure.lineTo(tmp[j] + 14, tmp[j + 1] + 14);
                    pathTriangleFigure.close();
                    canvas.drawPath(pathTriangleFigure,mPaint);
                    canvas.drawText(Float.toString(data[j + 2]), pText[0] + 250,
                        pText[1] - 60, mTextPaint2);
                    canvas.drawText(Float.toString(data[j + 3]), pText[0] + 250,
                        pText[1] - 40, mTextPaint2);
                } else if (nodeType == Config.TYPE_CROSS) {
                    canvas.drawLine(tmp[j] - 5, tmp[j + 1] - 5, tmp[j] + 5,
                        tmp[j + 1] + 5, mPaint);
                    canvas.drawLine(tmp[j] + 5, tmp[j + 1] - 5, tmp[j] - 5,
                        tmp[j + 1] + 5, mPaint);
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
        p[1] = (rect.top + rect.bottom) / 2 - 28;

        mPaint.setStyle(Paint.Style.FILL);
        for (int i = 0; i < mConfigs.size(); i++) {
            Elog.v(TAG, "[CurveViewEx] drawSeries i " + i);
            p[0] += 10;
            mPaint.setColor(mConfigs.get(i).color);
            mPaint.setStrokeWidth(mConfigs.get(i).lineWidth);
            mPaint.setPathEffect(null);

            int nodeType = mConfigs.get(i).nodeType;
            if (nodeType == Config.TYPE_NONE) {
                canvas.drawLine(p[0], p[1], p[0] + 100, p[1], mPaint);
            }

            p[0] += 50;
            if (nodeType == Config.TYPE_CIRCLE) {
                canvas.drawCircle(p[0], p[1], 5, mPaint);
            } else if(nodeType == Config.TYPE_NONE) {
                //do nothing
            } else if (nodeType == Config.TYPE_TRIANGLE) {
                Path pathTriangleFigure = new Path();
                pathTriangleFigure.moveTo(p[0], p[1] - 5);
                pathTriangleFigure.lineTo(p[0] - 5, p[1] + 5);
                pathTriangleFigure.lineTo(p[0] + 5, p[1] + 5);
                pathTriangleFigure.close();
                canvas.drawPath(pathTriangleFigure,mPaint);
            } else if (nodeType == Config.TYPE_CROSS) {
                canvas.drawLine(p[0] - 5, p[1] - 5, p[0] + 5, p[1] + 5, mPaint);
                canvas.drawLine(p[0] + 5, p[1] - 5, p[0] - 5, p[1] + 5, mPaint);
            } else if (nodeType == Config.TYPE_RHOMBUS) {
                Path pathRhombusFigure = new Path();
                pathRhombusFigure.moveTo(p[0], p[1] + 5);
                pathRhombusFigure.lineTo(p[0] - 5, p[1]);
                pathRhombusFigure.lineTo(p[0], p[1] - 5);
                pathRhombusFigure.lineTo(p[0] + 5, p[1]);
                pathRhombusFigure.close();
                canvas.drawPath(pathRhombusFigure,mPaint);
            } else if (nodeType == Config.TYPE_SQUARE) {
                canvas.drawRect(p[0] - 5, p[1] - 5,p[0] + 5, p[1] + 5,mPaint);
            }

            p[0] += 60;
            String str = mConfigs.get(i).name;
            if (nodeType == Config.TYPE_CIRCLE || nodeType == Config.TYPE_TRIANGLE) {
                canvas.drawText(str, p[0] - 40, p[1], mTextPaint2);
            } else {
            canvas.drawText(str, p[0], p[1], mTextPaint2);
            }

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
        } else if (String.valueOf(value).startsWith("1") ||
            String.valueOf(value).startsWith("5")) {
            value *= 2;
        } else {
            value /= 2;
            value *= 5;
        }
        return value;
    }

    public void getRSRPSINRConfig() {
        final SharedPreferences rsrpSinrConfigSh = mContext.getSharedPreferences(
        COMPONENT_RSRPSINR_CONFIG_SHAREPRE,android.content.Context.MODE_PRIVATE);
        strongPoint1X = rsrpSinrConfigSh.getInt(mContext.getString(
            R.string.strong_point_1x), STRONG_POINT1_X_DEFAULT);
        strongPoint1Y = rsrpSinrConfigSh.getInt(mContext.getString(
            R.string.strong_point_1y), STRONG_POINT1_Y_DEFAULT);
        strongPoint2X = rsrpSinrConfigSh.getInt(mContext.getString(
            R.string.strong_point_2x), STRONG_POINT2_X_DEFAULT);
        strongPoint2Y = rsrpSinrConfigSh.getInt(mContext.getString(
            R.string.strong_point_2y), STRONG_POINT2_Y_DEFAULT);
        strongPoint3X = rsrpSinrConfigSh.getInt(mContext.getString(
            R.string.strong_point_3x), STRONG_POINT3_X_DEFAULT);
        strongPoint3Y = rsrpSinrConfigSh.getInt(mContext.getString(
            R.string.strong_point_3y), STRONG_POINT3_Y_DEFAULT);
        mediumWeakPoint1X = rsrpSinrConfigSh.getInt(mContext.getString(
            R.string.mediumweak_point_1x), MEDIUMWEAK_POINT1_X_DEFAULT);
        mediumWeakPoint1Y = rsrpSinrConfigSh.getInt(mContext.getString(
            R.string.mediumweak_point_1y), MEDIUMWEAK_POINT1_Y_DEFAULT);
        mediumWeakPoint2X = rsrpSinrConfigSh.getInt(mContext.getString(
            R.string.mediumweak_point_2x), MEDIUMWEAK_POINT2_X_DEFAULT);
        mediumWeakPoint2Y = rsrpSinrConfigSh.getInt(mContext.getString(
            R.string.mediumweak_point_2y), MEDIUMWEAK_POINT2_Y_DEFAULT);
        mediumWeakPoint3X = rsrpSinrConfigSh.getInt(mContext.getString(
            R.string.mediumweak_point_3x), MEDIUMWEAK_POINT3_X_DEFAULT);
        mediumWeakPoint3Y = rsrpSinrConfigSh.getInt(mContext.getString(
            R.string.mediumweak_point_3y), MEDIUMWEAK_POINT3_Y_DEFAULT);
        weakPoint1X = rsrpSinrConfigSh.getInt(mContext.getString(
            R.string.weak_point_1x), WEAK_POINT1_X_DEFAULT);
        weakPoint1Y = rsrpSinrConfigSh.getInt(mContext.getString(
            R.string.weak_point_1y), WEAK_POINT1_Y_DEFAULT);
        weakPoint2X = rsrpSinrConfigSh.getInt(mContext.getString(
            R.string.weak_point_2x), WEAK_POINT2_X_DEFAULT);
        weakPoint2Y = rsrpSinrConfigSh.getInt(mContext.getString(
            R.string.weak_point_2y), WEAK_POINT2_Y_DEFAULT);
        weakPoint3X = rsrpSinrConfigSh.getInt(mContext.getString(
            R.string.weak_point_3x), WEAK_POINT3_X_DEFAULT);
        weakPoint3Y = rsrpSinrConfigSh.getInt(mContext.getString(
            R.string.weak_point_3y), WEAK_POINT3_Y_DEFAULT);
    }
}

