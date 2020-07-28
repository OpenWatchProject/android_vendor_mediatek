package com.mediatek.gallerybasic.base;

import android.app.Activity;

/**
 * If you want to do something in activity life cycle, implement this interface.
 */
public interface IActivityCallback {
    /**
     * Callback in Activity.onCreate.
     * @param activity
     *            Current gallery activity
     */
    public void onCreate(Activity activity);

    /**
     * Callback in Activity.onStart.
     * @param activity
     *            Current gallery activity
     */
    public void onStart(Activity activity);

    /**
     * Callback in Activity.onResume.
     * @param activity
     *            Current gallery activity
     */
    public void onResume(Activity activity);

    /**
     * Callback in Activity.onPause.
     * @param activity
     *            Current gallery activity
     */
    public void onPause(Activity activity);

    /**
     * Callback in Activity.onStop.
     * @param activity
     *            Current gallery activity
     */
    public void onStop(Activity activity);

    /**
     * Callback in Activity.onDestroy.
     * @param activity
     *            Current gallery activity
     */
    public void onDestroy(Activity activity);
}
