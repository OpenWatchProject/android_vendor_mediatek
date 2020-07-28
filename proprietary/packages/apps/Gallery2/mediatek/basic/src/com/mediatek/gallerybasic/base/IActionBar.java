package com.mediatek.gallerybasic.base;

import android.app.ActionBar;
import android.view.Menu;
import android.view.MenuItem;

/**
 * If you want to add more operations in single image view, implement this interface. It provides
 * chances to add/update/click menu item in full view.
 */
public interface IActionBar {
    /**
     * Callback when {@link Activity#onCreateOptionsMenu(Menu)} && current view is single image
     * view.
     * @param actionBar
     *            ActionBar of current Activity
     * @param menu
     *            Options menu of current Activity
     */
    public void onCreateOptionsMenu(ActionBar actionBar, Menu menu);

    /**
     * Callback when {@link Activity#onPrepareOptionsMenu(Menu)} && current view is single image
     * view.
     * @param menu
     *            Options menu of current Activity
     * @param data
     *            Current image data
     */
    public void onPrepareOptionsMenu(Menu menu, MediaData data);

    /**
     * Callback when {@link Activity#onOptionsItemSelected(MenuItem)} && current view is single
     * image view.
     * @param item
     *            The selected menu item
     * @param data
     *            Current image data
     * @return Return false to allow other menu processing to proceed, true to consume it here
     */
    public boolean onOptionsItemSelected(MenuItem item, MediaData data);

}
