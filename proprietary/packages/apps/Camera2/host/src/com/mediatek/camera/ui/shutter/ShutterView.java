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
package com.mediatek.camera.ui.shutter;

import android.content.Context;
import android.graphics.drawable.Drawable;
import android.util.AttributeSet;
import android.view.View;
import android.widget.RelativeLayout;
import android.widget.TextView;

import com.mediatek.camera.R;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;

/**
 * A shutter button view, it has a text view and image view.
 */
class ShutterView extends RelativeLayout {

    /**
     * Shutter text clicked listener.
     */
    public interface OnShutterTextClicked {
        /**
         * Shutter text clicked callback.
         * @param index the index of the shutter position.
         */
        void onShutterTextClicked(int index);
    }

    private final static int THRESHOLD_VALUE = 2;
    private LogUtil.Tag mTag;
    private TextView mName;
    private String mType;
    private ShutterButton mShutter;
    private int mCenterX;
    private int mScrollDistance;

    private OnShutterTextClicked mTextClickedListener;

    /**
     * Constructor that is called when inflating a face view from XML.
     * @param context The Context the view is running in.
     * @param attrs The attributes of the XML tag that is inflating the view.
     * @see #View(Context, AttributeSet)
     */
    public ShutterView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public void setType(String type) {
        mType = type;
        mTag = new LogUtil.Tag(ShutterView.class.getSimpleName() + "_" + type);
        LogHelper.d(mTag, "setType " + type);
    }

    public void setName(String name) {
        mName.setText(name);
        LogHelper.d(mTag, "setName " + name);
        mShutter.setContentDescription(name);
    }

    public void setDrawable(Drawable drawable) {
        mShutter.setImageDrawable(drawable);
    }

    public void setOnShutterButtonListener(ShutterButton.OnShutterButtonListener listener) {
        mShutter.setOnShutterButtonListener(listener);
    }

    public void setOnShutterTextClickedListener(OnShutterTextClicked listener) {
        mTextClickedListener = listener;
    }

    public String getName() {
        return mName.getText().toString();
    }

    public String getType() {
        return mType;
    }

    public void onScrolled(int scrollX, int parentCenterX, int scrollDistance) {
        updateShutterView(scrollX, parentCenterX, scrollDistance);
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        if (mName != null && mName.getMeasuredHeight() != 0
                && mShutter != null && mShutter.getMeasuredHeight() != 0) {
            int defaultHeight = mName.getMeasuredHeight() + mShutter.getMeasuredHeight();
            int defaultWidth = Math.max(mName.getMeasuredWidth(), mShutter.getMeasuredWidth());
            if (defaultWidth != 0 && defaultHeight != 0) {
                final int newWidthMeasureSpec =
                        MeasureSpec.makeMeasureSpec(defaultWidth, MeasureSpec.EXACTLY);
                final int newHeightMeasureSpec =
                        MeasureSpec.makeMeasureSpec(defaultHeight, MeasureSpec.EXACTLY);
                super.onMeasure(newWidthMeasureSpec, newHeightMeasureSpec);
            } else {
                super.onMeasure(widthMeasureSpec, heightMeasureSpec);
            }
        } else {
            super.onMeasure(widthMeasureSpec, heightMeasureSpec);
        }
    }

    @Override
    protected void onLayout(boolean changed, int l, int t, int r, int b) {
        super.onLayout(changed, l, t, r, b);
        mCenterX = (r - l + 1) / 2 + l;
        if (getParent() != null) {
            int parentCenterX = (((View) getParent()).getWidth() + 1) / 2;
            updateShutterView(((View) getParent()).getScrollX(), parentCenterX, 0);
        }
    }

    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();
        mName = (TextView) findViewById(R.id.shutter_text);
        mShutter = (ShutterButton) findViewById(R.id.shutter_button);
        mName.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mTextClickedListener != null) {
                    mTextClickedListener.onShutterTextClicked((int) getTag());
                }
            }
        });
    }

    @Override
    public void setEnabled(boolean enabled) {
        if (mShutter != null) {
            mShutter.setEnabled(enabled);
            mShutter.setClickable(enabled);
        }
        if (mName != null) {
            mName.setEnabled(enabled);
            mName.setClickable(enabled);
        }
    }

    public void setTextEnabled(boolean enabled) {
        if (mName != null) {
            mName.setEnabled(enabled);
            mName.setClickable(enabled);
        }
    }

    private int measureDimension(int defaultSize, int measureSpec) {
        int result = defaultSize;

        int specMode = MeasureSpec.getMode(measureSpec);
        int specSize = MeasureSpec.getSize(measureSpec);

        if (specMode == MeasureSpec.EXACTLY) {
            result = specSize;
        } else if (specMode == MeasureSpec.AT_MOST) {
            result = Math.min(defaultSize, specSize);
        } else {
            result = defaultSize;
        }
        return result;
    }

    private void updateShutterView(int scrollX, int parentCenterX, int scrollDistance) {
        int centerX = mCenterX - scrollX;
        int diff = Math.abs(centerX - parentCenterX);
        mScrollDistance = scrollDistance;

        if (scrollDistance == 0) {
            if (diff <= THRESHOLD_VALUE) {
                mShutter.setAlpha(1.0f);
                mShutter.setEnabled(true);
            } else {
                mShutter.setEnabled(false);
                mShutter.setAlpha(0.0f);
            }
        } else {
            if (diff <= THRESHOLD_VALUE) {
                mShutter.setAlpha(1.0f);
                mShutter.setEnabled(true);
            } else if (diff < scrollDistance) {
                float alpha = 1.0f * diff / scrollDistance;
                mShutter.setAlpha(1.0f - alpha);
            } else if (diff >= scrollDistance) {
                mShutter.setEnabled(false);
                mShutter.setAlpha(0.0f);
            }
        }
    }
}
