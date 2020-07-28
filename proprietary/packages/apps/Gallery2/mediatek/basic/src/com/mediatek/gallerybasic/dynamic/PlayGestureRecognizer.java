package com.mediatek.gallerybasic.dynamic;

import android.content.Context;
import android.view.GestureDetector;
import android.view.MotionEvent;
import android.view.ScaleGestureDetector;

public class PlayGestureRecognizer {
    private static final String TAG = "MtkGallery2/PlayGestureRecognizer";

    public interface Listener {
        public boolean onSingleTapUp(float x, float y);

        public boolean onDoubleTap(float x, float y);

        public boolean onScroll(float dx, float dy, float totalX, float totalY);

        public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX,
                float velocityY);

        public boolean onScaleBegin(float focusX, float focusY);

        public boolean onScale(float focusX, float focusY, float scale);

        public void onScaleEnd();

        public void onDown(float x, float y);

        public void onUp();
    }

    private final GestureDetector mGestureDetector;
    private final ScaleGestureDetector mScaleDetector;
    private final DownUpDetector mDownUpDetector;
    private boolean mScaleEventResult = false;
    private final Listener mListener;

    public PlayGestureRecognizer(Context context, Listener listener) {
        mListener = listener;
        mGestureDetector = new GestureDetector(context,
                new MyGestureListener(), null, true);
        mScaleDetector = new ScaleGestureDetector(context,
                new MyScaleListener());
        mDownUpDetector = new DownUpDetector(new MyDownUpListener());
    }

    public boolean onTouch(MotionEvent event) {
        mScaleEventResult = false;
        boolean result = mGestureDetector.onTouchEvent(event);
        // ScaleGestureDetector.onTouchEvent always return true, so
        // we using mScaleEventResult as flag
        mScaleDetector.onTouchEvent(event);
        mDownUpDetector.onTouchEvent(event);
        return result || mScaleEventResult;
    }

    private class MyGestureListener extends
            GestureDetector.SimpleOnGestureListener {
        @Override
        public boolean onSingleTapUp(MotionEvent e) {
            return mListener.onSingleTapUp(e.getX(), e.getY());
        }

        @Override
        public boolean onDoubleTap(MotionEvent e) {
            return mListener.onDoubleTap(e.getX(), e.getY());
        }

        @Override
        public boolean onScroll(MotionEvent e1, MotionEvent e2, float dx,
                float dy) {
            return mListener.onScroll(dx, dy, e2.getX() - e1.getX(), e2.getY()
                    - e1.getY());
        }

        @Override
        public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX,
                float velocityY) {
            return mListener.onFling(e1, e2, velocityX, velocityY);
        }
    }

    private class MyScaleListener extends
            ScaleGestureDetector.SimpleOnScaleGestureListener {
        @Override
        public boolean onScaleBegin(ScaleGestureDetector detector) {
            mScaleEventResult = mListener.onScaleBegin(detector.getFocusX(),
                    detector.getFocusY());
            return mScaleEventResult;
        }

        @Override
        public boolean onScale(ScaleGestureDetector detector) {
            mScaleEventResult = mListener.onScale(detector.getFocusX(),
                    detector.getFocusY(), detector.getScaleFactor());
            return mScaleEventResult;
        }

        @Override
        public void onScaleEnd(ScaleGestureDetector detector) {
            mListener.onScaleEnd();
        }
    }

    private class MyDownUpListener implements DownUpDetector.DownUpListener {
        @Override
        public void onDown(MotionEvent e) {
            mListener.onDown(e.getX(), e.getY());
        }

        @Override
        public void onUp(MotionEvent e) {
            mListener.onUp();
        }
    }
}
