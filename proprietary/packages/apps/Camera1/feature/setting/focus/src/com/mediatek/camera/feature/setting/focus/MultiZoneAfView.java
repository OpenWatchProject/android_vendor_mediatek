/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */
package com.mediatek.camera.feature.setting.focus;

import android.animation.ValueAnimator;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Matrix;
import android.graphics.Rect;
import android.graphics.RectF;
import android.graphics.drawable.Drawable;
import android.util.AttributeSet;
import android.view.View;

import com.mediatek.camera.R;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.utils.CameraUtil;
import com.mediatek.camera.portability.SystemProperties;

/**
 * View used to show multi zone af indicators.
 *
 */
public class MultiZoneAfView extends View {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(MultiZoneAfView.class.getSimpleName());

    private static final int[] MZAF_ICON = new int[] { R.drawable.ic_multi_zone_focus_focusing,
            R.drawable.ic_multi_zone_focus_focused };
    private static final String MZAF_INDICATOR_KEY = "vendor.multizone.af.window.ratio";
    private static final String DEFAULT_INDICATOR_RATIO = "0.4";
    private static final int SCALING_UP_TIME = 1000;
    private static final int SCALING_DOWN_TIME = 200;

    private MultiWindow[] mWindows;
    private float mScaleRatio = 0;
    private float mAnimatorRatio = 1.0f;
    private int mDisplayOrientation;
    private int mOrientation;
    private int mPreviewWidth;
    private int mPreviewHeight;
    private boolean mMirror;
    private Matrix mMatrix = new Matrix();
    private RectF mRect = new RectF();
    private Drawable mAfIndicator;
    private ValueAnimator mValueAnimator = new ValueAnimator();
    private Drawable[] mAfStatusIndicators = new Drawable[MZAF_ICON.length];

    /**
     * The constructor.
     *
     * @param context The Context the view is running in, through which it can access the current
     * theme, resources, etc.
     * @param attr The attributes of the XML tag that is inflating the view.
     */
    public MultiZoneAfView(Context context, AttributeSet attr) {
        super(context, attr);
        getViewDrawable();
        mAfIndicator = mAfStatusIndicators[0];
        mScaleRatio = Float.parseFloat(SystemProperties.getString(MZAF_INDICATOR_KEY,
                DEFAULT_INDICATOR_RATIO));
    }

    /**
     * Set mirror infomation.
     *
     * @param mirror
     *            True if current camera is the front camera,otherwise,false.
     */
    public void setMirror(boolean mirror) {
        mMirror = mirror;
    }

    /**
     * Update focus window datas.
     *
     * @param windows
     *            The multi-zone af windows which will be shown.
     */
    public void updateFocusWindows(MultiWindow[] windows) {
        mWindows = windows;
    }

    /**
     * Show multi-zone af window,the animator is depends on whether it is focusing windows or not.
     *
     * @param isFocusingWindow
     *            True if current windows is focusing windows,other wise not.
     */
    public void showWindows(boolean isFocusingWindow) {
        mValueAnimator.cancel();
        if (isFocusingWindow) {
            mValueAnimator = ValueAnimator.ofFloat(1.0f, 1.2f).setDuration(SCALING_UP_TIME);
        } else {
            mValueAnimator = ValueAnimator.ofFloat(mAnimatorRatio, 1.0f).setDuration(
                    SCALING_DOWN_TIME);
        }
        mValueAnimator.addUpdateListener(new ValueAnimator.AnimatorUpdateListener() {
            @Override
            public void onAnimationUpdate(ValueAnimator animation) {
                mAnimatorRatio = (Float) animation.getAnimatedValue();
                // do not draw the windows when scale ratio is bigger than 1,otherwise,the windows
                // will interlace
                if (mAnimatorRatio * mScaleRatio <= 1) {
                    invalidate();
                }
            }
        });
        mValueAnimator.start();
    }

    /**
     * Clear the windows when necessary.
     */
    public void clear() {
        mWindows = null;
        invalidate();
    }

    /**
     * The the orientation.
     * @param orientation The compensation orientation.
     */
    public void setOrientation(int orientation) {
        mOrientation = orientation;
        invalidate();
    }

    /**
     * Set preview size.
     * @param previewWidth The preview width.
     * @param previewHeight The preview height.
     */
    public void setPreviewSize(int previewWidth, int previewHeight) {
        mPreviewWidth = previewWidth;
        mPreviewHeight = previewHeight;
    }

    /**
     * Set display orientation.
     * @param orientation The value for activity orientation.
     */
    public void setDisplayOrientation(int orientation) {
        mDisplayOrientation = orientation;
        LogHelper.d(TAG, "mDisplayOrientation=" + orientation);
    }

    @Override
    protected void onDraw(Canvas canvas) {
        if (mWindows != null && mWindows.length > 0) {
            LogHelper.d(TAG, "onDraw length " + mWindows.length + " ,mDisplayOrientation = "
                    + mDisplayOrientation + " ,mOrientation= " + mOrientation + ",mMirror ="
                    + mMirror);
            int rw;
            int rh;
            rw = mPreviewWidth;
            rh = mPreviewHeight;
            if (((rh > rw) && ((mDisplayOrientation == 0) || (mDisplayOrientation == 180)))
                    || ((rh < rw) && ((mDisplayOrientation == 90)
                            || (mDisplayOrientation == 270)))) {
                int temp = rw;
                rw = rh;
                rh = temp;
            }
            // Prepare the matrix.
            CameraUtil.prepareMatrix(mMatrix, mMirror, mDisplayOrientation, rw, rh);

            float dx = (float) (getWidth() - rw) / 2;
            float dy = (float) (getHeight() - rh) / 2;
            Matrix pointMatrix = new Matrix();
            float[] pointes = new float[] { dx, dy };
            // Focus indicator is directional. Rotate the matrix and the canvas
            // so it looks correctly in all orientations.
            canvas.save();
            mMatrix.postRotate(mOrientation); // postRotate is clockwise
            canvas.rotate(-mOrientation); // rotate is counter-clockwise (for
                                          // canvas)
//            pointMatrix.postRotate(mContext.getOrientationCompensation());
            pointMatrix.mapPoints(pointes);
            for (int i = 0; i < mWindows.length; i++) {
                // Transform the coordinates.
                mRect.set(mWindows[i].mBounds);
                dumpRect(mRect, "Original rect");
                mMatrix.mapRect(mRect);
                dumpRect(mRect, "Transformed rect");
                LogHelper.d(TAG, "window[ " + i + " ] result " + mWindows[i].mResult);
                if (mWindows[i].mResult > 0) {
                    mAfIndicator = mAfStatusIndicators[1];
                } else {
                    mAfIndicator = mAfStatusIndicators[0];
                }
                mRect.offset(pointes[0], pointes[1]);
                mAfIndicator.setBounds(scale());
                mAfIndicator.draw(canvas);
            }
            canvas.restore();
        }
        super.onDraw(canvas);
    }

    private Drawable[] getViewDrawable() {
        int length = mAfStatusIndicators.length;
        for (int i = 0; i < length; i++) {
            mAfStatusIndicators[i] = this.getResources().getDrawable(MZAF_ICON[i]);
        }
        return mAfStatusIndicators;
    }

    /**
     * scale the rect by it's center.
     * @return The scaled rect.
     */
    private Rect scale() {
        Rect rect = new Rect();
        float centerX = mRect.centerX();
        float centerY = mRect.centerY();
        float minWidth = Math.min(mRect.width(), mRect.height());
        float left = centerX - minWidth * mAnimatorRatio * mScaleRatio / 2;
        float top = centerY - minWidth * mAnimatorRatio * mScaleRatio / 2;
        float right = centerX + minWidth * mAnimatorRatio * mScaleRatio / 2;
        float bottom = centerY + minWidth * mAnimatorRatio * mScaleRatio / 2;
        rect.set((int) left, (int) top, (int) right, (int) bottom);
        return rect;
    }

    /**
     * Describes a window detected in an image.
     */
    public static final class MultiWindow {
        public Rect mBounds;
        public int mResult;

        /**
         * Create a new window.
         *
         * @param bounds
         *            Bounds of the window.
         * @param result
         *            Result of the window.True if the window is focused,false otherwise.
         */
        public MultiWindow(Rect bounds, int result) {
            mBounds = bounds;
            mResult = result;
        }

        /**
         * Represent the window as a string for debugging purposes.
         *
         * @return The window information as stirng.
         */
        @Override
        public String toString() {
            return String.format("{ bounds: %s, result: %s}", mBounds, mResult);
        }
    }

    private void dumpRect(RectF rect, String msg) {
        LogHelper.d(TAG, msg + "=(" + rect.left + "," + rect.top + "," + rect.right + ","
                + rect.bottom + ")");
    }
}
