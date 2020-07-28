/*
 * Copyright (C) 2014 MediaTek Inc.
 * Modification based on code covered by the mentioned copyright
 * and/or permission notice(s).
 */
/*
 * Copyright (C) 2012 The Android Open Source Project
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

package com.android.camera;

import android.content.Context;
import android.graphics.Matrix;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import com.mediatek.camera.platform.ICameraAppUi.GestureListener;

// This class aggregates three gesture detectors: GestureDetector,
// ScaleGestureDetector, and DownUpDetector.
public class GestureDispatcher implements GestureRecognizer.Listener,
        CameraActivity.OnOrientationListener {
    private static final String TAG = "GestureDispatcher";

    private static final int LONG_PRESS = 0;
    private static final int SINGLE_TAPUP = 1;
    private View mSingleTapArea;
    private View mLongPressArea;
    private GestureListener mGestureListener;
    private GestureDispatcherListener mGestureDispatcherListener;
    private CameraActivity mCameraActivity;
    private boolean mIgnorGestureForZooming;
    private int mGsensorOrientation = 0;

    public GestureDispatcher(Context context) {
        mCameraActivity = (CameraActivity) context;
        mCameraActivity.addOnOrientationListener(this);
    }

    public interface GestureDispatcherListener {
        public boolean onDown(float x, float y, int width, int height);
        public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY);
        public boolean onScroll(float dx, float dy, float totalX, float totalY);
        public boolean onSingleTapUp(float x, float y);
        public boolean onSingleTapConfirmed(float x, float y);
        public boolean onUp();
        public boolean onDoubleTap(float x, float y);
        public boolean onScale(float focusX, float focusY, float scale);
        public boolean onScaleBegin(float focusX, float focusY);
        public boolean onLongPress(float x, float y);
    }

    // Return true if the tap is consumed.
    @Override
    public boolean onSingleTapUp(float x, float y) {
        Log.d(TAG, "[onSingleTapUp] (" + x + ", " + y + ")");
        if (FeatureSwitcher.isSupportDoubleTapUp()) {
            return false;
        }
        float[] pts = getPointMapCompensation(x, y);
        Log.d(TAG, "[onSingleTapUp] zoomlistener = (" + mGestureListener);
        if (mGestureListener != null && mGestureListener.onSingleTapUp(pts[0], pts[1])) {
            return false;
        }
        return onSingleTapUp(Math.round(pts[0]), Math.round(pts[1]));
    }

    @Override
    public void onLongPress(float x, float y) {
        Log.d(TAG, "onLongPress(" + x + ", " + y + ")");
        float[] pts = getPointMapCompensation(x, y);
        if (mGestureListener != null && mGestureListener.onLongPress(pts[0], pts[1])) {
            return;
        }
        onLongPress(Math.round(pts[0]), Math.round(pts[1]));
    }

    @Override
    public boolean onSingleTapConfirmed(float x, float y) {
        Log.d(TAG, "onSingleTapConfirmed(" + x + ", " + y + ")");
        float[] pts = getPointMapCompensation(x, y);
        return false;
    }

    @Override
    public boolean onDoubleTap(float x, float y) {
        if (!FeatureSwitcher.isSupportDoubleTapUp()) {
            return false;
        }
        float[] pts = getPointMapCompensation(x, y);
        Log.d(TAG, "onDoubleTap(" + x + ", " + y);
        if (mGestureListener != null && mGestureListener.onDoubleTap(pts[0], pts[1])) {
            return false;
        }
        if (mGestureDispatcherListener != null) {
            return mGestureDispatcherListener.onDoubleTap(pts[0], pts[1]);
        }
        return true;
    }

    @Override
    public boolean onScroll(float dx, float dy, float totalX, float totalY) {
        Log.d(TAG, "onScroll(" + dx + ", " + dy + ", " + totalX + ", " + totalY
                + ")");
        if (mIgnorGestureForZooming) {
            return false;
        }
        if (mGestureListener != null
                && mGestureListener.onScroll(dx, dy, totalX, totalY)) {
            return false;
        }
        if (mGestureDispatcherListener != null) {
            return mGestureDispatcherListener.onScroll(dx, dy, totalX, totalY);
        }
        return false;
    }

    @Override
    public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX,
            float velocityY) {
        Log.d(TAG, "[onFling] (" + velocityX + ", " + velocityY + ")");
        if (mIgnorGestureForZooming) {
            return false;
        }
        if (mGestureListener != null
                && mGestureListener.onFling(e1, e2, velocityX, velocityY)) {
            return false;
        }
        if (mGestureDispatcherListener != null) {
            return mGestureDispatcherListener.onFling(e1, e2, velocityX, velocityY);
        }
        return false;
    }

    @Override
    public boolean onScaleBegin(float focusX, float focusY) {
        Log.d(TAG, "onScaleBegin(" + focusX + ", " + focusY + ")");
        // remember that a zooming gesture has just ended
        mIgnorGestureForZooming = true;
        if (mGestureListener != null) {
            mGestureListener.onScaleBegin(focusX, focusY);
        }
        if (mGestureDispatcherListener != null) {
            return mGestureDispatcherListener.onScaleBegin(focusX, focusY);
        }
        return true;
    }

    @Override
    public boolean onScale(float focusX, float focusY, float scale) {
        Log.d(TAG, "onScale(" + focusX + ", " + focusY + ", " + scale);
        if (mGestureListener != null
                && mGestureListener.onScale(focusX, focusY, scale)) {
            return false;
        }
        if (mGestureDispatcherListener != null) {
            return mGestureDispatcherListener.onScale(focusX, focusY, scale);
        }
       return true;
    }

    @Override
    public void onScaleEnd() {
        Log.d(TAG, "onScaleEnd()");
    }

    @Override
    public void onDown(float x, float y) {
        Log.d(TAG, "onDown()");
        mIgnorGestureForZooming = false;
        float[] vertex = computeVertex(x, y);
        if (mGestureListener != null
                && mGestureListener.onDown(vertex[0], vertex[1], (int) vertex[2],
                        (int) vertex[3])) {
            return;
        }
        if (mGestureDispatcherListener != null) {
            mGestureDispatcherListener.onDown(vertex[0], vertex[1], (int) vertex[2],
                        (int) vertex[3]);
        }
    }

    @Override
    public void onUp() {
        Log.d(TAG, "onUp");
        if (mGestureListener != null && mGestureListener.onUp()) {
            return;
        }
    }

    @Override
    public void onOrientationChanged(int orientationCompensation) {
        mGsensorOrientation = orientationCompensation;
    }

    public void setGestureListener(GestureListener listener) {
        mGestureListener = listener;
    }

    public void setGestureDispatcherListener(GestureDispatcherListener listener) {
        mGestureDispatcherListener = listener;
    }

    public void setSingleTapUpListener(View singleTapArea) {
        mSingleTapArea = singleTapArea;
    }

    public void setLongPressListener(View singleTapArea) {
        mLongPressArea = singleTapArea;
    }

    public float[] computeVertex(float x, float y) {
        float[] vertexAndRect = new float[] { 0f, 0f, 0f, 0f };
        int w = mCameraActivity.getUnCropWidth();
        int h = mCameraActivity.getUnCropHeight();
        vertexAndRect[0] = x;
        vertexAndRect[1] = y;
        vertexAndRect[2] = w;
        vertexAndRect[3] = h;
        return vertexAndRect;
    }

    private float[] getPointMapCompensation(float x, float y) {
        Matrix inv = new Matrix();
        float[] pts = new float[] { x, y };
        inv.mapPoints(pts);
        return pts;
    }

    private boolean onSingleTapUp(int x, int y) {
        Log.d(TAG, "onSingleTapUp x = " + x + " y= " + y);
        // Ignore if listener is null or the camera control is invisible.
        return onTouchScreen(x, y, mSingleTapArea, SINGLE_TAPUP);
    }

    private boolean onLongPress(int x, int y) {
        // Ignore if listener is null or the camera control is invisible.
        return onTouchScreen(x, y, mLongPressArea, LONG_PRESS);
    }

    private boolean onTouchScreen(int x, int y, View area, int index) {
        // Ignore if listener is null or the camera control is invisible.
        if (area == null) {
            return false;
        }
        View surfaceView = (View) mCameraActivity.getPreviewSurfaceView();
        if (surfaceView == null) {
            Log.d(TAG, "onTouchScreen surfaceView is null");
            return false;
        }
        int[] relativeLocation = Util.getRelativeLocation(surfaceView, area);
        x -= relativeLocation[0];
        y -= relativeLocation[1];
        if (x >= 0 && x < area.getWidth() && y >= 0 && y < area.getHeight()) {
            if (index == LONG_PRESS) {
                mCameraActivity.onLongPress(area, x, y);
            } else {
                mCameraActivity.onSingleTapUp(area, x, y);
            }
            return true;
        }
        mCameraActivity.onSingleTapUpBorder(area, x, y);
        return true;
    }
}
