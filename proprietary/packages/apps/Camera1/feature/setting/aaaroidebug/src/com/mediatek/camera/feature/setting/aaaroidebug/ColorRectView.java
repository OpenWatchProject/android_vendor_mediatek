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

package com.mediatek.camera.feature.setting.aaaroidebug;

import android.content.Context;
import android.content.res.TypedArray;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.util.AttributeSet;
import android.view.View;

import com.mediatek.camera.R;
import com.mediatek.camera.common.debug.LogUtil;

/**
 * View to show a frame rect with a center point.
 */
public class ColorRectView extends View {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(ColorRectView.class.getSimpleName());
    private static final int DEFAULT_STROKE_WIDTH = 5;
    private static final int DEFAULT_COLOR = Color.RED;
    private int mColor;
    private Paint mRectPaint;
    private Paint mPointPaint;
    private Rect[] mRects;

    /**
     * Default constructor of parent, init color and paint.
     *
     * @param context Current context
     * @param attrs   The attribute
     */
    public ColorRectView(Context context, AttributeSet attrs) {
        super(context, attrs);

        TypedArray typedArray = context.obtainStyledAttributes(attrs, R.styleable.colorRectView);
        mColor = typedArray.getColor(R.styleable.colorRectView_color, DEFAULT_COLOR);

        mRectPaint = new Paint();
        mRectPaint.setColor(mColor);
        mRectPaint.setStrokeWidth(
                typedArray.getColor(R.styleable.colorRectView_lineWidth, DEFAULT_STROKE_WIDTH));
        mRectPaint.setStyle(Paint.Style.STROKE);

        mPointPaint = new Paint();
        mPointPaint.setColor(mColor);
        mPointPaint.setStrokeWidth(
                typedArray.getColor(R.styleable.colorRectView_lineWidth, DEFAULT_STROKE_WIDTH) * 2);
        mPointPaint.setStyle(Paint.Style.FILL);
    }

    /**
     * Set color of rect and its center point.
     *
     * @param color Color to be set.
     */
    public void setColor(int color) {
        mColor = color;
        mRectPaint.setColor(mColor);
        mPointPaint.setColor(mColor);
    }

    /**
     * The rect related to UI.
     *
     * @param rect Rect ar.
     */
    public void setRects(Rect[] rect) {
        if (!isRectArrayEqual(rect, mRects)) {
            mRects = rect;
            invalidate();
        }
    }

    private static boolean isRectArrayEqual(Rect[] rect1, Rect[] rect2) {
        if (rect1 == null && rect2 == null) {
            return true;
        }

        if (rect1 == null && rect2 != null) {
            return false;
        }

        if (rect1 != null && rect2 == null) {
            return false;
        }

        if (rect1.length != rect2.length) {
            return false;
        }

        for (int i = 0; i < rect1.length; i++) {
            if (rect1[i] == null && rect2[i] != null) {
                return false;
            }
            if (!rect1[i].equals(rect2[i])) {
                return false;
            }
        }

        return true;
    }

    @Override
    protected void onDraw(Canvas canvas) {
        if (mRects != null) {
            for (Rect rect : mRects) {
                canvas.drawRect(rect, mRectPaint);
                canvas.drawPoint(rect.centerX(), rect.centerY(), mPointPaint);
            }
        }
        super.onDraw(canvas);
    }
}