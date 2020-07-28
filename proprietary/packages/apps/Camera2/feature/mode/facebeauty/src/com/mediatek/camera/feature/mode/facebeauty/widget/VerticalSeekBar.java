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
 *     MediaTek Inc. (C) 2019. All rights reserved.
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
package com.mediatek.camera.feature.mode.facebeauty.widget;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.RectF;
import android.util.AttributeSet;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;

import com.mediatek.camera.R;

public class VerticalSeekBar extends View {
    private Context context;
    private int height;
    private int width;
    private Paint paint;
    private int maxProgress = 100;
    private int progress = 50;

    protected Bitmap mThumb;
    private int intrinsicHeight;
    private int intrinsicWidth;
    private boolean isInnerClick;
    private float downX;
    private float downY;

    private int locationX;
    private int locationY = -1;

    private int mInnerProgressWidth = 4;
    private int mInnerProgressWidthPx;

    private int unSelectColor = 0xcc888888;
    private RectF mDestRect;
    /**
     * slide orientation
     * 0 from bottom to top
     * 1 from top to bottom
     */
    private int orientation = 0;

    /**
     * set unSelect bar color
     *
     * @param uNSelectColor
     */
    public void setUnSelectColor(int uNSelectColor) {
        this.unSelectColor = uNSelectColor;
    }

    /**
     * slide orientation
     * 0 from bottom to top
     * 1 from top to bottom
     *
     * @param orientation
     */
    public void setOrientation(int orientation) {
        this.orientation = orientation;
        invalidate();
    }

    private int selectColor = 0xaa0980ED;

    /**
     * set selected bar color
     *
     * @param selectColor
     */
    public void setSelectColor(int selectColor) {
        this.selectColor = selectColor;
    }

    /**
     * set the width of the bar, the unit is PX
     *
     * @param mInnerProgressWidthPx
     */
    public void setmInnerProgressWidthPx(int mInnerProgressWidthPx) {
        this.mInnerProgressWidthPx = mInnerProgressWidthPx;
    }

    /**
     * set the width of the bar, the unit is DP
     *
     * @param mInnerProgressWidth
     */
    public void setmInnerProgressWidth(int mInnerProgressWidth) {
        this.mInnerProgressWidth = mInnerProgressWidth;
        mInnerProgressWidthPx = dip2px(context, mInnerProgressWidth);
    }


    /**
     * set thumb resource
     *
     * @param id
     */
    public void setThumb(int id) {

        mThumb = BitmapFactory.decodeResource(getResources(), id);
        intrinsicHeight = mThumb.getHeight();
        intrinsicWidth = mThumb.getWidth();
        mDestRect.set(0, 0, intrinsicWidth, intrinsicHeight);
        invalidate();
    }

    /**
     * set thumb size ,the unit is DP
     *
     * @param width
     * @param height
     */
    public void setThumbSize(int width, int height) {
        setThumbSizePx(dip2px(context, width), dip2px(context, height));
    }

    /**
     * set thumb size ,the unit is PX
     *
     * @param width
     * @param height
     */
    public void setThumbSizePx(int width, int height) {
        intrinsicHeight = width;
        intrinsicWidth = height;
        mDestRect.set(0, 0, width, height);
        invalidate();
    }


    /**
     * Simple constructor to use when creating a view from code.
     *
     * @param context The Context the view is running in, through which it can
     *                access the current theme, resources, etc.
     */
    public VerticalSeekBar(Context context) {
        super(context);
        init(context, null, 0);
    }

    /**
     * Constructor that is called when inflating a view from XML. This is called
     * when a view is being constructed from an XML file, supplying attributes
     * that were specified in the XML file. This version uses a default style of
     * 0, so the only attribute values applied are those in the Context's Theme
     * and the given AttributeSet.
     * <p>
     * <p>
     * The method onFinishInflate() will be called after all children have been
     * added.
     *
     * @param context The Context the view is running in, through which it can
     *                access the current theme, resources, etc.
     * @param attrs   The attributes of the XML tag that is inflating the view.
     */
    public VerticalSeekBar(Context context, AttributeSet attrs) {
        super(context, attrs);
        init(context, attrs, 0);
    }

    public VerticalSeekBar(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        init(context, attrs, defStyleAttr);
    }

    /**
     * inir bar
     *
     * @param context
     * @param attrs
     * @param defStyleAttr
     */
    private void init(Context context, AttributeSet attrs, int defStyleAttr) {
        this.context = context;
        paint = new Paint();
        mThumb = BitmapFactory.decodeResource(getResources(), R.drawable.ic_bar_round);
        intrinsicHeight = mThumb.getHeight();
        intrinsicWidth = mThumb.getWidth();
        mDestRect = new RectF(0, 0, intrinsicWidth, intrinsicHeight);
        mInnerProgressWidthPx = dip2px(context, mInnerProgressWidth);
    }


    @Override
    protected void onSizeChanged(int w, int h, int oldw, int oldh) {
        super.onSizeChanged(w, h, oldw, oldh);
        height = getMeasuredHeight();
        width = getMeasuredWidth();
        if (locationY == -1) {
            locationX = width / 2;
            locationY = height / 2;

            Log.i("xiaozhu", locationY + ":" + height);
        }

    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        switch (event.getAction()) {
            case MotionEvent.ACTION_DOWN:
                //determine whether the click point is in the thumb
                isInnerClick = isInnerMthum(event);
                if (isInnerClick) {
                    if (listener != null) {
                        listener.onStart(this, progress);
                    }
                }
                downX = event.getX();
                downY = event.getY();

                break;
            case MotionEvent.ACTION_MOVE:
                if (isInnerClick) {
                    locationY = (int) event.getY();
                    fixLocationY();

                    progress =
                            (int) (maxProgress - (locationY - intrinsicHeight * 0.5)
                                    / (height - intrinsicHeight) * maxProgress);
                    if (orientation == 1) {
                        progress = maxProgress - progress;
                    }
                    downY = event.getY();
                    downX = event.getX();
                    if (listener != null) {
                        listener.onProgress(this, progress);
                    }
                    invalidate();
                }
                break;
            case MotionEvent.ACTION_UP:
                if (isInnerClick) {
                    if (listener != null) {
                        listener.onStop(this, progress);
                    }
                }
                break;
        }
        return true;
    }

    private void fixLocationY() {
        if (locationY <= intrinsicHeight / 2) {
            locationY = intrinsicHeight / 2;
        } else if (locationY >= height - intrinsicHeight / 2) {
            locationY = height - intrinsicHeight / 2;
        }
    }

    /**
     * //determine whether the click point is in the thumb
     *
     * @param event
     * @return
     */
    private boolean isInnerMthum(MotionEvent event) {
        return event.getX() >= width / 2 - intrinsicWidth / 2 && event.getX() <= width
                / 2 + intrinsicWidth / 2 && event.getY() >= locationY - intrinsicHeight
                / 2 && event.getY() <= locationY + intrinsicHeight / 2;
    }

    @Override
    protected void onDraw(Canvas canvas) {
        if (orientation == 0) {
            locationY =
                    (int) (intrinsicHeight * 0.5f + (maxProgress - progress)
                            * (height - intrinsicHeight) / maxProgress);
        } else {
            locationY =
                    (int) (intrinsicHeight * 0.5f + (progress)
                            * (height - intrinsicHeight) / maxProgress);
        }
        paint.setColor(orientation == 0 ? unSelectColor : selectColor);
        canvas.drawRect(width / 2 - mInnerProgressWidthPx / 2, mDestRect.height() / 2,
                width / 2 + mInnerProgressWidthPx / 2, locationY, paint);
        paint.setColor(orientation == 0 ? selectColor : unSelectColor);
        canvas.drawRect(width / 2 - mInnerProgressWidthPx / 2, locationY,
                width / 2 + mInnerProgressWidthPx / 2
                , height - mDestRect.height() / 2, paint);
        canvas.save();
        canvas.translate(width / 2 - mDestRect.width() / 2
                , locationY - mDestRect.height() / 2);
        canvas.drawBitmap(mThumb, null, mDestRect, new Paint());
        canvas.restore();
        super.onDraw(canvas);
    }

    public void setProgress(int progress) {
        if (height == 0) {
            height = getMeasuredHeight();
        }

        this.progress = progress;

        invalidate();
    }

    public int getProgress() {
        return progress;
    }

    @Override
    protected void onDetachedFromWindow() {
        if (mThumb != null) {
            mThumb.recycle();
        }
        super.onDetachedFromWindow();
    }


    public void setMaxProgress(int maxProgress) {
        this.maxProgress = maxProgress;
    }

    public int getMaxProgress() {
        return maxProgress;
    }

    private SlideChangeListener listener;

    public void setOnSlideChangeListener(SlideChangeListener l) {
        this.listener = l;
    }

    //add listener
    public interface SlideChangeListener {
        /**
         * start slide
         *
         * @param slideView
         * @param progress
         */
        void onStart(VerticalSeekBar slideView, int progress);

        /**
         * sliding
         *
         * @param slideView
         * @param progress
         */
        void onProgress(VerticalSeekBar slideView, int progress);

        /**
         * stop slide
         *
         * @param slideView
         * @param progress
         */
        void onStop(VerticalSeekBar slideView, int progress);
    }

    public static int dip2px(final Context context, final float dpValue) {
        final DisplayMetrics metrics = context.getResources().getDisplayMetrics();
        final float scale = metrics.density;
        return (int) ((dpValue * scale) + 0.5f);
    }


}