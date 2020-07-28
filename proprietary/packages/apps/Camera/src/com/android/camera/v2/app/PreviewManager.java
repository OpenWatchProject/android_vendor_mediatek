package com.android.camera.v2.app;

import java.util.ArrayList;

import android.graphics.RectF;
import android.view.GestureDetector;
import android.view.Surface;
import android.view.View;
import android.view.View.OnLayoutChangeListener;

import com.android.camera.v2.ui.PreviewStatusListener;
import com.android.camera.v2.ui.PreviewStatusListener.OnPreviewAreaChangedListener;

/**
 *
 */
public abstract class PreviewManager {
    public interface SurfaceCallback {
        /**
         * This callback notify preview surface is available.
         * @param surface preview surface
         * @param width surface's width
         * @param height surface's height
         */
        public void surfaceAvailable(final Surface surface, int width, int height);
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
    }
    protected final ArrayList<OnPreviewAreaChangedListener>
                                mPreviewAreaChangedListeners =
                                new ArrayList<OnPreviewAreaChangedListener>();
    protected SurfaceCallback
                                mSurfaceCallback;
    protected OnLayoutChangeListener
                                mOnLayoutChangeListener;
    protected RectF             mPreviewArea = new RectF();
    protected GestureDetector   mGestureDetector = null;

    public void setSurfaceCallback(SurfaceCallback surfaceCallback) {
        mSurfaceCallback = surfaceCallback;
    }

    public void setOnLayoutChangeListener(OnLayoutChangeListener listener) {
        mOnLayoutChangeListener = listener;
    }
    /**
     * Adds a listener that will get notified when the preview area changed. This
     * can be useful for UI elements or focus view to adjust themselves according
     * to the preview area change.
     * <p/>
     * Note that a listener will only be added once. A newly added listener will receive
     * a notification of current preview area immediately after being added.
     * <p/>
     * This function should be called on the UI thread and listeners will be notified
     * on the UI thread.
     *
     * @param listener the listener that will get notified of preview area change
     */
    public void addPreviewAreaSizeChangedListener(
            PreviewStatusListener.OnPreviewAreaChangedListener listener) {
        if (listener != null && !mPreviewAreaChangedListeners.contains(listener)) {
            mPreviewAreaChangedListeners.add(listener);
            if (mPreviewArea.width() != 0 || mPreviewArea.height() != 0) {
                listener.onPreviewAreaChanged(mPreviewArea);
            }
        }
    }

    /**
     * Removes a listener that gets notified when the preview area changed.
     *
     * @param listener the listener that gets notified of preview area change
     */
    public void removePreviewAreaSizeChangedListener(
            PreviewStatusListener.OnPreviewAreaChangedListener listener) {
        if (listener != null && mPreviewAreaChangedListeners.contains(listener)) {
            mPreviewAreaChangedListeners.remove(listener);
        }
    }

    /**
     * Each module can pass in their own gesture listener through App UI. When a gesture
     * is detected, the {@link GestureRecognizer.Listener} will be notified of
     * the gesture.
     *
     * @param gestureListener a listener from a module that defines how to handle gestures
     */
    public void setGestureListener(View.OnTouchListener gestureListener) {

    }

    /**
     * Gets called from module when preview is started.
     */
    public void onPreviewStarted() {

    }

    public void pause() {

    }

    public void resume() {

    }

    /**
     * update preview size.
     * <p>
     * This function should be called on the UI thread and listeners will be notified
     * on the UI thread.
     * @param width preview width
     * @param height preview height
     */
    public abstract void updatePreviewSize(int width, int height);

    public abstract View getPreviewView();

    /**
     * broadcast preview area changes
     */
    protected void notifyPreviewAreaChanged() {
        for (PreviewStatusListener.OnPreviewAreaChangedListener listener :
            mPreviewAreaChangedListeners) {
            listener.onPreviewAreaChanged(mPreviewArea);
        }
    }
}