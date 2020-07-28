
package com.mediatek.gallery3d.video;

import android.content.Context;
import android.util.AttributeSet;
import android.view.SurfaceHolder;

import com.mediatek.gallery3d.util.Log;
import com.mediatek.galleryportable.MovieViewAdapter;

public class MovieView extends MovieViewAdapter {

    private static final String TAG = "VP_MovieView";
    private static final boolean LOG = true;

    private int mVideoWidth;
    private int mVideoHeight;
    private SurfaceCallback mSurfaceListener;

    private SurfaceHolder.Callback mSHCallback = new SurfaceHolder.Callback() {

        @Override
        public void surfaceDestroyed(SurfaceHolder holder) {
            if (mSurfaceListener != null) {
                mSurfaceListener.onSurfaceDestroyed(holder);
            }
        }

        @Override
        public void surfaceCreated(SurfaceHolder holder) {
            if (mSurfaceListener != null) {
                mSurfaceListener.onSurfaceCreated(holder);
            }
        }

        @Override
        public void surfaceChanged(SurfaceHolder holder, int format, int width,
                int height) {
            if (mSurfaceListener != null) {
                mSurfaceListener
                        .onSurfaceChanged(holder, format, width, height);
            }
        }
    };

    public MovieView(Context context) {
        this(context, null, 0);
    }

    public MovieView(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public MovieView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        initVideoView();
    }

    private void initVideoView() {
        mVideoWidth = 0;
        mVideoHeight = 0;
        getHolder().addCallback(mSHCallback);
        setFocusable(true);
        setFocusableInTouchMode(true);
        requestFocus();
    }

    public void setVideoLayout(int videoWidth, int videoHeight) {
        Log.v(TAG, "setVideoLayout, videoWidth = " + videoWidth
                + ", videoHeight = " + videoHeight);
        this.mVideoWidth = videoWidth;
        this.mVideoHeight = videoHeight;
        requestLayout();
    }

    public void setSurfaceListener(SurfaceCallback surfaceListener) {
        this.mSurfaceListener = surfaceListener;
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        int width = getDefaultSize(mVideoWidth, widthMeasureSpec);
        int height = getDefaultSize(mVideoHeight, heightMeasureSpec);

        if (mVideoWidth > 0 && mVideoHeight > 0) {
            if (mVideoWidth * height > width * mVideoHeight) {
                // Log.d("@@@", "image too tall, correcting");
                height = width * mVideoHeight / mVideoWidth;
            } else if (mVideoWidth * height < width * mVideoHeight) {
                // Log.d("@@@", "image too wide, correcting");
                width = height * mVideoWidth / mVideoHeight;
            }
        }
        if (LOG) {
            Log.v(TAG, "onMeasure[video size = " + mVideoWidth + 'x' + mVideoHeight
                    + "] set view size = " + width + 'x' + height);
        }
        setMeasuredDimension(width, height);
    }

    /**
     * The SurfaceCallback will be invoked in SurfaceHolder.Callback, It's used
     * to interaction with MoviePlayerWrapper, and implemented in
     * MoviePlayerWrapper
     */
    public interface SurfaceCallback {
        public void onSurfaceCreated(SurfaceHolder holder);

        public void onSurfaceChanged(SurfaceHolder holder, int format,
                int width, int height);

        public void onSurfaceDestroyed(SurfaceHolder holder);
    }
}
