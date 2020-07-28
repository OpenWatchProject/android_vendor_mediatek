/*
 * Copyright (C) 2013 The Android Open Source Project
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

import android.graphics.RectF;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.View;

/**
 * This interface defines a listener that watches preview status, including SurfaceView
 * change and preview gestures.
 */
public interface PreviewStatusListener {
    public interface OnGestureListener {
        /**
          * Notified when a tap occurs with the down that triggered it.
          * This will be triggered immediately for
          * every down event. All other events should be preceded by this.
          *
         * @param x touch x's position
         * @param y touch y's position
         *
         * @return true intercept onDown event, false no need intercept onDown event.
         */
        public boolean onDown(float x, float y);
        public boolean onUp();
        public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY);
        public boolean onScroll(float dx, float dy, float totalX, float totalY);
        public boolean onSingleTapUp(float x, float y);
        public boolean onSingleTapConfirmed(float x, float y);
        public boolean onDoubleTap(float x, float y);
        public boolean onScale(float focusX, float focusY, float scale);
        public boolean onScaleBegin(float focusX, float focusY);
        public boolean onLongPress(float x, float y);
    }
    public interface OnPreviewTouchedListener {
        /**
         * This gets called on any preview touch event.
         */
        public boolean onPreviewTouched();
    }
    /**
     * This listener gets notified when the actual preview frame changes due
     * to a transform matrix being applied to the TextureView
     */
    public interface OnPreviewAreaChangedListener {
        public void onPreviewAreaChanged(RectF previewArea);
    }

    /**
     * This listener gets notified when the preview aspect ratio is changed.
     */
    public interface OnPreviewAspectRatioChangedListener {
        public void onPreviewAspectRatioChanged(float aspectRatio);
    }

    /**
     * This callback notify preview surface is available.
     * @param surface preview surface
     * @param width surface's width
     * @param height surface's height
     */
    public void surfaceAvailable(Surface surface, int width, int height);
    /**
     * When preview surface destroy, this callback will be called.
     * @param surface the destroyed preview surface.
     */
    public void surfaceDestroyed(Surface surface);
    /**
     * This callback is called when surface's size changed.
     * @param surface the size changed surface.
     * @param width new width of the surface
     * @param height new height of the surface
     */
    public void surfaceSizeChanged(Surface surface, int width, int height);
    /**
     * The preview status listener needs to provide an
     * {@link android.view.GestureDetector.OnGestureListener} in order to listen
     * to the touch events that happen on preview.
     *
     * @return a listener that listens to touch events
     */
    public OnGestureListener getGestureListener();

    /**
     * An {@link OnPreviewTouchedListener} can be provided in addition to
     * or instead of a {@link OnGestureListener}
     * for listening to touch events on the preview.  The listener is called whenever
     * there is a touch event on the Preview.
     */
    public OnPreviewTouchedListener getTouchListener();

    /**
     * Gets called when preview TextureView gets a layout change call.
     */
    public void onPreviewLayoutChanged(View v, int left, int top, int right,
            int bottom, int oldLeft, int oldTop, int oldRight, int oldBottom);


//    /**
//     * The preview status listener needs to know for the specific module whether
//     * preview TextureView should automatically adjust its transform matrix based
//     * on the current aspect ratio, width and height of the TextureView.
//     *
//     * @return whether transform matrix should be automatically adjusted
//     */
//    public boolean shouldAutoAdjustTransformMatrixOnLayout();
//
//    /**
//     * The preview status listener needs to know for the specific module whether
//     * bottom bar should be automatically adjusted when preview has changed size
//     * or orientation.
//     *
//     * @return whether bottom bar should be automatically adjusted
//     */
//    public boolean shouldAutoAdjustBottomBar();
//
//    /**
//     * Gets called when the preview is flipped (i.e. 180-degree rotated).
//     */
//    public void onPreviewFlipped();
}
