/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2010 The Android Open Source Project
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

package com.android.camera.v2.ui;

import android.content.Context;
import android.util.AttributeSet;
import android.view.View;
import android.view.ViewGroup;

import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;

// A RotateLayout is designed to display a single item and provides the
// capabilities to rotate the item.
public class RotateLayout extends ViewGroup implements Rotatable {
    private static final Tag TAG = new Tag(RotateLayout.class.getSimpleName());

    private OnSizeChangedListener mListener;
    private int mOrientation;
    protected View mChild;

    public RotateLayout(Context context, AttributeSet attrs) {
        super(context, attrs);
        // The transparent background here is a workaround of the render issue
        // happened when the view is rotated as the device's orientation
        // changed. The view looks fine in landscape. After rotation, the view
        // is invisible.
        setBackgroundResource(android.R.color.transparent);
    }

    /** A callback to be invoked when the preview frame's size changes. */
    public interface OnSizeChangedListener {
        void onSizeChanged(int width, int height);
    }

    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();
        mChild = getChildAt(0);
        mChild.setPivotX(0);
        mChild.setPivotY(0);
    }

    @Override
    protected void onLayout(boolean change, int left, int top, int right, int bottom) {
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
        LogHelper.v(TAG, "setOrientation(" + orientation + ", " + animation + ") mOrientation="
                + mOrientation);
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
        LogHelper.d(TAG, "onSizeChanged(" + w + ", " + h + ", " + oldh + ", " + oldh + ") " + this);
        if (mListener != null) {
            mListener.onSizeChanged(w, h);
        }
    }
}
