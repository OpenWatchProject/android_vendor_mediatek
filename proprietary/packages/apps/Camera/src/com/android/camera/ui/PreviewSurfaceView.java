package com.android.camera.ui;

import com.android.camera.FeatureSwitcher;
import com.android.camera.Log;
import com.android.camera.Util;
import com.mediatek.camera.setting.SettingUtils;

import android.content.Context;
import android.content.res.Configuration;
import android.util.AttributeSet;
import android.view.SurfaceView;
import android.widget.FrameLayout;

public class PreviewSurfaceView extends SurfaceView {
    private double                      mAspectRatio = 0.0;
    private static final String         TAG = "PreviewSurfaceView";
    private int                         mPreviewWidth = 0;
    private int                         mPreviewHeight = 0;
    private boolean                     mIsNeedLockSizeChange = false;
    private static final int SHRINK_SIZE = 2; // surface size must be multiple of 2
    private Context mContext;

    public PreviewSurfaceView(Context context, AttributeSet attrs) {
        super(context, attrs);
        mContext = context;
    }

    // Add onMeasure is for full screen crop preview
    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        int previewWidth = MeasureSpec.getSize(widthMeasureSpec);
        int previewHeight = MeasureSpec.getSize(heightMeasureSpec);
        boolean widthLonger = previewWidth > previewHeight;
        int longSide = (widthLonger ? previewWidth : previewHeight);
        int shortSide = (widthLonger ? previewHeight : previewWidth);
        if (mAspectRatio > 0) {
            double fullScreenRatio = SettingUtils.findFullscreenRatio(mContext);
            boolean isMultiWin = FeatureSwitcher.isTablet()
                               && FeatureSwitcher.isMultiWindow()
                               && (previewWidth < SettingUtils.getFullScreenWidth())
                               && (previewHeight < SettingUtils.getFullScreenHeight());
            if (Math.abs((mAspectRatio - fullScreenRatio)) <= Util.ASPECT_TOLERANCE
                && !isMultiWin) {
                // full screen preview case
                Log.d(TAG, "full screen case");
                if (longSide < shortSide * mAspectRatio) {
                    longSide = Math.round((float) (shortSide * mAspectRatio) / 2) * 2;
                } else {
                    shortSide = Math.round((float) (longSide / mAspectRatio) / 2) * 2;
                }
            } else {
                // standard (4:3) or multiwindow preview case
                Log.d(TAG, "4:3 or multiwindow case");
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
        if (mIsNeedLockSizeChange) {
            previewWidth = SHRINK_SIZE;
            previewHeight = SHRINK_SIZE;
        }
        if (!mIsNeedLockSizeChange) {
            mPreviewWidth = previewWidth;
            mPreviewHeight = previewHeight;
        }
        if (Util.isWfdEnabled(getContext()) || FeatureSwitcher.isTablet()) {
            boolean originalPreviewIsLandscape = (mPreviewWidth > mPreviewHeight);
            int orientation = getContext().getResources().getConfiguration().orientation;
            boolean configurationIsLandscape = (orientation == Configuration.ORIENTATION_LANDSCAPE);

            // if configuration is changed, swap to view's configuration
            if (originalPreviewIsLandscape != configurationIsLandscape) {
                int originalPreviewWidht = previewWidth;
                previewWidth = previewHeight;
                previewHeight = originalPreviewWidht;
            }
        }
        setMeasuredDimension(previewWidth, previewHeight);
        Log.d(TAG, "After onMeasure  aspectRatio = " + mAspectRatio + " previewWidth = "
                + previewWidth + " previewHeight = " + previewHeight);
    }

    /**
     * set new preview aspect ratio to notify SurfaceView onMeasure again
     * Note: this method must be called on UI Thread
     * @param aspectRatio
     * @return true: layout will change; false: layout will not change
     */
    public boolean setAspectRatio(double aspectRatio) {
        Log.d(TAG, "setAspectRatio aspectRatio = " + aspectRatio);
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
        setLayoutSize(SHRINK_SIZE, SHRINK_SIZE);
    }

    /**restore surface view's size
     * when swap from gallery to camera, should expand surface view immediately
     * Note: this method must be called on UI Thread
     */
    public void expand() {
        Log.d(TAG, "expand preview (" + mPreviewWidth + " , " + mPreviewHeight + ")");
        if (!mIsNeedLockSizeChange) {
            return;
        }
        boolean originalPreviewIsLandscape = (mPreviewWidth > mPreviewHeight);
        double orientation = getContext().getResources().getConfiguration().orientation;
        boolean configurationIsLandscape = (orientation == Configuration.ORIENTATION_LANDSCAPE);
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
        Log.d(TAG, "setLayoutSize mPreviewWidth = " + mPreviewWidth
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
