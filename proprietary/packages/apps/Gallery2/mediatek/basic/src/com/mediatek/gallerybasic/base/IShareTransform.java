package com.mediatek.gallerybasic.base;

import android.app.Activity;
import android.net.Uri;

import java.util.ArrayList;

/**
 * If you want to replace the share content in standard proccessing, implement this interface. It
 * provides the chances to transform the share content and show/hide the transform ui.
 */
public interface IShareTransform {
    /**
     * Callback before onTransform, run on UI thread, you can update UI here.
     * @param activity
     *            Current activity
     */
    public void onStartTransform(Activity activity);

    /**
     * Callback after onTransform, run on UI thread, you can update UI here.
     * @param activity
     *            Current activity
     */
    public void onStopTransform(Activity activity);

    /**
     * Call to transform original share content to new, not run on UI thread.
     * @param activity
     *            Current activity> If you want to show some error info like toast, use Activity to
     *            do this.
     * @param data
     *            The data list to share
     * @return The new share uri list after transform
     */
    public ArrayList<Uri> onTransform(Activity activity, MediaData[] data);
}
