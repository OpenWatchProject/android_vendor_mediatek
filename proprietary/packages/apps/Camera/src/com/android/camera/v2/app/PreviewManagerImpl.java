package com.android.camera.v2.app;


import android.app.Activity;
import android.graphics.Rect;
import android.os.Handler;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.View;
import android.view.View.OnLayoutChangeListener;
import android.widget.FrameLayout;

import com.android.camera.R;
import com.android.camera.v2.ui.FixedAspectSurfaceView;
import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;

import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;
import junit.framework.Assert;

/**
 *  A preview helper for SurfaceView.
 */
public class PreviewManagerImpl extends PreviewManager {
    private static final Tag TAG = new Tag(PreviewManagerImpl.class.getSimpleName());
    private final Activity mActivity;
    private FrameLayout mCurSurfaceViewLayout;
    private FixedAspectSurfaceView mSurfaceView;
    private boolean mSurfaceAvailable = false;
    private Surface mSurface;
    private View.OnTouchListener mTouchListener;
    private double mPreviewAspectRatio = 0.0d;
    private int mPreviewWidth = 0;
    private int mPreviewHeight = 0;
    private View mSurfaceViewCover;
    private Handler mHandler = new Handler();
    private BlockingQueue<FrameLayout> mFrameLayoutQueue =
            new LinkedBlockingQueue<FrameLayout>();

    private SurfaceHolder.Callback mSurfaceViewCallback = new SurfaceHolder.Callback() {
        @Override
        public void surfaceCreated(SurfaceHolder holder) {
            Rect surfaceFrame = holder.getSurfaceFrame();
            LogHelper.i(TAG, "surfaceCreated mPreviewWidth = " + surfaceFrame.width()
                    + " mPreviewHeight = " + surfaceFrame.height());
        }
        @Override
        public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
            LogHelper.i(TAG, "surfaceChanged width = " + width + " height = " + height);
            if (mSurfaceCallback != null && width == mPreviewWidth && height == mPreviewHeight) {
                mSurface = holder.getSurface();
                if (mSurfaceAvailable) {
                    mSurfaceCallback.surfaceSizeChanged(mSurface, width, height);
                } else {
                    mSurfaceAvailable = true;
                    mSurfaceCallback.surfaceAvailable(mSurface, width, height);
                }
            }
        }

        @Override
        public void surfaceDestroyed(SurfaceHolder holder) {
            LogHelper.i(TAG, "surfaceDestroyed");
            if (mSurfaceCallback != null) {
                mSurfaceCallback.surfaceDestroyed(holder.getSurface());
            }
        }
    };

    private OnLayoutChangeListener          mOnLayoutChangeCallback = new OnLayoutChangeListener() {
        @Override
        public void onLayoutChange(View v, int left, int top, int right, int bottom, int oldLeft,
                int oldTop, int oldRight, int oldBottom) {
            LogHelper.i(TAG, "onLayoutChange left = " + left +
                                       " top = " + top +
                                       " width = " + (right - left) +
                                       " height = " + (bottom - top));
            mPreviewArea.set(left, top, right, bottom);
            // This method can be called during layout pass. We post a Runnable so
            // that the callbacks won't happen during the layout pass.
            mSurfaceView.post(new Runnable() {
                @Override
                public void run() {
                    notifyPreviewAreaChanged();
                }
            });
            if (mOnLayoutChangeListener != null) {
                mOnLayoutChangeListener.onLayoutChange(v, left, top, right, bottom,
                        oldLeft, oldTop, oldRight, oldBottom);
            }
        }
    };

    public PreviewManagerImpl(Activity activity) {
        mActivity = activity;
    }

    @Override
    public void updatePreviewSize(int width, int height) {
        Assert.assertTrue(width > 0 && height > 0);
        if (mPreviewWidth == width && mPreviewHeight == height) {
            LogHelper.i(TAG, "setPreviewSize skip : width = " + width
                    + " height = " + height);
            return;
        }
        double newAspectRatio = (double) width / height;
        mSurfaceView = null;
        mPreviewWidth = width;
        mPreviewHeight = height;
        mPreviewAspectRatio = (double) Math.max(width, height) / Math.min(width, height);
        // if aspect ration changed, attach a new surface view
        if (mSurfaceView == null) {
            attachSurfaceViewLayout();
            mSurfaceAvailable = false;
        }
        mSurfaceView.getHolder().setFixedSize(mPreviewWidth, mPreviewHeight);
        mSurfaceView.setAspectRatio(mPreviewAspectRatio);
    }

    @Override
    public View getPreviewView() {
        return mSurfaceView;
    }

    @Override
    public void setGestureListener(View.OnTouchListener touchListener) {
        mTouchListener = touchListener;
    }

    @Override
    public void onPreviewStarted() {
        mHandler.removeCallbacks(mDetachRunnable);
        mHandler.post(mDetachRunnable);
    }

    @Override
    public void pause() {
        super.pause();
        if (mSurfaceViewCover != null && mSurfaceViewCover.getVisibility() != View.VISIBLE) {
            mSurfaceViewCover.setVisibility(View.VISIBLE);
        }
        if (mSurfaceView != null) {
            mSurfaceView.shrink();
        }
    }

    @Override
    public void resume() {
        super.resume();
    }

    private void attachSurfaceViewLayout() {
        LogHelper.i(TAG, "[attachSurfaceViewLayout]...");
        if (mSurfaceView == null) {
            // if onPreviewStarted not comes when do attach new surface view,
            // should detach newest surface view (with black buffer),
            // for example: current flow should detach SurfaceView B,
            // in order to not show black screen.
            // A(SurfaceView with image buffer)-->
            //     B(SurfaceView with black buffer)-->
            //         C(new SurfaceView).
            if (mCurSurfaceViewLayout != null) {
                mFrameLayoutQueue.add(mCurSurfaceViewLayout);
            }
            FrameLayout surfaceViewRoot = (FrameLayout)
                    mActivity.findViewById(R.id.camera_preview_container);
            mCurSurfaceViewLayout = (FrameLayout)
                    mActivity.getLayoutInflater().inflate(
                            R.layout.camera_previewsurfaceview_layout, null);
            mSurfaceView = (FixedAspectSurfaceView)
                    mCurSurfaceViewLayout.findViewById(R.id.previewsurfaceview);
            mSurfaceView.setOnTouchListener(new View.OnTouchListener() {
                @Override
                public boolean onTouch(View v, MotionEvent event) {
                    mTouchListener.onTouch(v, event);
                    return true;
                }
            });
            mSurfaceView.addOnLayoutChangeListener(mOnLayoutChangeCallback);
            SurfaceHolder surfaceHolder = mSurfaceView.getHolder();
            surfaceHolder.addCallback(mSurfaceViewCallback);
            surfaceViewRoot.addView(mCurSurfaceViewLayout);
            surfaceViewRoot.setVisibility(View.VISIBLE);
            mSurfaceView.setVisibility(View.VISIBLE);
        }
    }

    private void detachSurfaceViewLayout(FrameLayout frameLayout) {
        LogHelper.i(TAG, "detachSurfaceViewLayout frameLayout = " + frameLayout);
        if (frameLayout != null) {
            FrameLayout surfaceViewRoot = (FrameLayout)
                    mActivity.findViewById(R.id.camera_preview_container);
            surfaceViewRoot.removeViewInLayout(frameLayout);
            frameLayout.setVisibility(View.GONE);
        }
    }

    private Runnable mDetachRunnable = new Runnable() {
        @Override
        public void run() {
            FrameLayout cameraViewContainer = (FrameLayout)
                    mActivity.findViewById(R.id.camera_view_container);
            mSurfaceViewCover = cameraViewContainer.findViewById(R.id.camera_cover);
            if (mSurfaceViewCover != null && mSurfaceViewCover.getVisibility() != View.INVISIBLE) {
                /**
                 * expand surface view when first frame arrives because surface view cover is shown.
                 */
                if (mSurfaceView != null) {
                    mSurfaceView.expand();
                }
                mSurfaceViewCover.setVisibility(View.INVISIBLE);
            }
            int queueSize = mFrameLayoutQueue.size();
            for (int i = 0; i < queueSize; i++) {
                detachSurfaceViewLayout(mFrameLayoutQueue.poll());
            }
        }
    };
}
