package com.mediatek.gallery3d.video;

import android.content.Context;
/**
 * DRM extension for host app. IMovieDrmCallback will be used after user do some operations.
 */
public interface IMovieDrmExtension {
    /**
     * Callback for user operations.
     */
    public interface IMovieDrmCallback {
        /**
         * Final user will continue current operation after consumed current DRM right.
         */
        void onContinue();
        /**
         * Final user will not consume DRM right.
         */
        void onStop();
    }
    /**
     * If current video file is DRM file, callback will be used and this function will return true.
     * Otherwise, callback will not be used and return false.
     * @param context
     * @param item
     * @param callback
     * @return
     */
    boolean handleDrmFile(final Context context,
                          final IMovieItem item,
                          final IMovieDrmCallback callback);
    /**
     * Current video file is permitted to be shared or not.
     * @param context
     * @param item
     * @return
     */
    boolean canShare(Context context, IMovieItem item);
}
