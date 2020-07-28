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
import android.util.AttributeSet;
import android.view.View;
import android.view.ViewGroup;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;

/**
 * A RotateLayout is designed to display a single item and provides the
 * capabilities to rotate the item.
 */
public class RotateLayout extends ViewGroup implements Rotatable {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(RotateLayout.class.getSimpleName());
    private OnSizeChangedListener mListener;
    private int mOrientation;
    protected View mChild;

    /**
     * The RotateLayout constructor.
     *
     * @param context The Context the view is running in, through which it can
     *                access the current theme, resources, etc.
     * @param attrs   The attributes of the XML tag that is inflating the view.
     */
    public RotateLayout(Context context, AttributeSet attrs) {
        super(context, attrs);
        // The transparent background here is a workaround of the render issue
        // happened when the view is rotated as the device's orientation
        // changed. The view looks fine in landscape. After rotation, the view
        // is invisible.
        setBackgroundResource(android.R.color.transparent);
    }

    /**
     * The RotateLayout constructor.
     *
     * @param context The Context the view is running in, through which it can
     *                access the current theme, resources, etc.
     */
    public RotateLayout(Context context) {
        super(context);
        setBackgroundResource(android.R.color.transparent);
    }


    /**
     * A callback to be invoked when the preview frame's size changes.
     */
    public interface OnSizeChangedListener {
        /**
         * the callback to be invoked when the preview frame's size changes.
         *
         * @param width  The width of preview frame.
         * @param height The height of preview frame.
         */
        void onSizeChanged(int width, int height);
    }

    @Override
    public void onFinishInflate() {
        super.onFinishInflate();
        mChild = getChildAt(0);
        mChild.setPivotX(0);
        mChild.setPivotY(0);
    }

    @Override
    protected void onLayout(boolean change, int left, int top, int right, int bottom) {
        if (mChild == null) {
            return;
        }
        int width = right - left;
        int height = bottom - top;
        switch (mOrientation) {
            case 0:
            case 180:
                mChild.layout(0, 0, width, height);
                break;
            case 90:
            case 270:
                mChild.layout(0, 0, height, width);
                break;
            default:
                break;
        }
    }

    @Override
    protected void onMeasure(int widthSpec, int heightSpec) {
        int w = 0;
        int h = 0;
        if (mChild == null) {
            setMeasuredDimension(w, h);
            return;
        }
        switch (mOrientation) {
            case 0:
            case 180:
                measureChild(mChild, widthSpec, heightSpec);
                w = mChild.getMeasuredWidth();
                h = mChild.getMeasuredHeight();
                break;
            case 90:
            case 270:
                measureChild(mChild, heightSpec, widthSpec);
                w = mChild.getMeasuredHeight();
                h = mChild.getMeasuredWidth();
                break;
            default:
                break;
        }
        setMeasuredDimension(w, h);

        switch (mOrientation) {
            case 0:
                mChild.setTranslationX(0);
                mChild.setTranslationY(0);
                break;
            case 90:
                mChild.setTranslationX(0);
                mChild.setTranslationY(h);
                break;
            case 180:
                mChild.setTranslationX(w);
                mChild.setTranslationY(h);
                break;
            case 270:
                mChild.setTranslationX(w);
                mChild.setTranslationY(0);
                break;
            default:
                break;
        }
        mChild.setRotation(-mOrientation);
    }

    @Override
    public boolean shouldDelayChildPressedState() {
        return false;
    }

    // Rotate the view counter-clockwise
    @Override
    public void setOrientation(int orientation, boolean animation) {
        if (orientation != 0 && orientation != 90 && orientation != 180 && orientation != 270) {
            LogHelper.w(TAG, "setOrientation : Not support orientation = " + orientation);
            return;
        }
        orientation = orientation % 360;
        if (mOrientation == orientation) {
            return;
        }
        mOrientation = orientation;
        requestLayout();
    }

    public int getOrientation() {
        return mOrientation;
    }

    public void setOnSizeChangedListener(OnSizeChangedListener listener) {
        mListener = listener;
    }

    @Override
    protected void onSizeChanged(int w, int h, int oldw, int oldh) {
        if (mListener != null) {
            mListener.onSizeChanged(w, h);
        }
    }
}