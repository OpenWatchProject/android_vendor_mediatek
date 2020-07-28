package com.android.camera.v2.ui;

import android.content.Context;
import android.content.res.Configuration;
import android.util.AttributeSet;
import android.view.SurfaceView;
import android.widget.FrameLayout;

import com.android.camera.v2.util.CameraUtil;
import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;

public class FixedAspectSurfaceView extends SurfaceView {
    private static final Tag TAG = new Tag(FixedAspectSurfaceView.class.getSimpleName());
    public static final double          ASPECT_TOLERANCE = 0.03;
    private double                      mAspectRatio = 0.0;
    private int                         mPreviewWidth = 0;
    private int                         mPreviewHeight = 0;
    private boolean                     mIsNeedLockSizeChange = false;

    public FixedAspectSurfaceView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    // Add onMeasure is for full screen crop preview
    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        int previewWidth = MeasureSpec.getSize(widthMeasureSpec);
        int previewHeight = MeasureSpec.getSize(heightMeasureSpec);
        LogHelper.d(TAG, "onMeasure previewWidth = " + previewWidth +
                " previewHeight = " + previewHeight);
        boolean widthLonger = previewWidth > previewHeight;
        int longSide = (widthLonger ? previewWidth : previewHeight);
        int shortSide = (widthLonger ? previewHeight : previewWidth);
        if (mAspectRatio > 0) {
            double fullScreenRatio = CameraUtil.findFullscreenRatio(getContext());
            if (Math.abs((mAspectRatio - fullScreenRatio)) <= ASPECT_TOLERANCE) {
                // full screen preview case
                LogHelper.d(TAG, "full screen case");
                if (longSide < shortSide * mAspectRatio) {
                    longSide = Math.round((float) (shortSide * mAspectRatio) / 2) * 2;
                } else {
                    shortSide = Math.round((float) (longSide / mAspectRatio) / 2) * 2;
                }
            } else {
                // standard (4:3) preview case
                LogHelper.d(TAG, "4:3 case");
                if (longSide > shortSide * mAspectRatio) {
                    longSide = Math.round((float) (shortSide * mAspectRatio) / 2) * 2;
                } else {
                    shortSide = Math.round((float) (longSide / mAspectRatio) / 2) * 2;
                }
            }
        }
        if (widthLonger) {
            previewWidth = longSide;
            previewHeight = shortSide;
        } else {
            previewWidth = shortSide;
            previewHeight = longSide;
        }
        if (!mIsNeedLockSizeChange) {
            mPreviewWidth = previewWidth;
            mPreviewHeight = previewHeight;
        }

        boolean originalPreviewIsLandscape = (mPreviewWidth > mPreviewHeight);
        boolean configurationIsLandscape =
               (getContext().getResources().getConfiguration().orientation ==
               Configuration.ORIENTATION_LANDSCAPE);

        // if configuration is changed, swap to view's configuration
        if (originalPreviewIsLandscape != configurationIsLandscape) {
            int originalPreviewWidht = previewWidth;
            previewWidth = previewHeight;
            previewHeight = originalPreviewWidht;
        }

        LogHelper.d(TAG, "originalPreviewIsLandscape = " + originalPreviewIsLandscape
                + ",configurationIsLandscape = " + configurationIsLandscape + ",mPreviewWidth = "
                + mPreviewWidth + ",mPreviewHeight = " + mPreviewHeight);

        setMeasuredDimension(previewWidth, previewHeight);
        LogHelper.d(TAG, "After onMeasure  aspectRatio = " + mAspectRatio +
                " previewWidth = " + previewWidth + " previewHeight = " + previewHeight);
    }

    /**
     * set new preview aspect ratio to notify SurfaceView onMeasure again
     * Note: this method must be called on UI Thread
     * @param aspectRatio
     * @return true: layout will change; false: layout will not change
     */
    public boolean setAspectRatio(double aspectRatio) {
        LogHelper.d(TAG, "setAspectRatio aspectRatio = " + aspectRatio);
        if (mAspectRatio != aspectRatio) {
            mAspectRatio = aspectRatio;
            requestLayout();
            return true;
        }
        return false;
    }

    /**
     * hide surface view by setting size to 1x1
     * when swap to gallery complete, should hide surface view
     * Note: this method must be called on UI Thread
     */
    public void shrink() {
        if (mIsNeedLockSizeChange) {
            return;
        }
        mIsNeedLockSizeChange = true;
        setLayoutSize(2, 2);
    }

    /**restore surface view's size
     * when swap from gallery to camera, should expand surface view immediately
     * Note: this method must be called on UI Thread
     */
    public void expand() {
        LogHelper.d(TAG, "expand mPreviewWidth = " + mPreviewWidth +
                " mPreviewHeight = " + mPreviewHeight);
        if (mPreviewWidth <= 2 || mPreviewHeight <= 2 || !mIsNeedLockSizeChange) {
            return;
        }
        boolean originalPreviewIsLandscape = (mPreviewWidth > mPreviewHeight);
        boolean configurationIsLandscape =
                (getContext().getResources().getConfiguration().orientation
                        == Configuration.ORIENTATION_LANDSCAPE);
        // if configuration is changed, swap to view's configuration
        if (originalPreviewIsLandscape != configurationIsLandscape) {
            int originalPreviewWidht = mPreviewWidth;
            mPreviewWidth = mPreviewHeight;
            mPreviewHeight = originalPreviewWidht;
        }
        mIsNeedLockSizeChange = false;
        setLayoutSize(mPreviewWidth, mPreviewHeight);
    }

    private void setLayoutSize(int width, int height) {
        LogHelper.d(TAG, "setLayoutSize mPreviewWidth = " + mPreviewWidth
                + " width = " + width
                + " mPreviewHeight = " + mPreviewHeight
                + " height = " + height);
        if (width <= 0 || height <= 0 || mPreviewWidth <= 0 || mPreviewWidth <= 0) {
            return;
        }
        FrameLayout.LayoutParams p = (FrameLayout.LayoutParams) getLayoutParams();
        if (p.width != width || p.height != height) {
            p.width = width;
            p.height = height;
            p.setMargins(mPreviewWidth - width, mPreviewHeight - height, 0, 0);
            setLayoutParams(p);
        }
    }
}
