package com.mediatek.gallery3d.video;

import android.content.Context;
import android.content.Intent;

public interface IMovieListLoader {
    /**
     * Loader listener interface
     */
    public interface LoaderListener {
        /**
         * Will be called after movie list loaded.
         * @param movieList
         */
        void onListLoaded(IMovieList movieList);
    }
    /**
     * Build the movie list from current item.
     * @param context
     * @param intent
     * @param l
     * @param item
     */
    void fillVideoList(Context context, Intent intent, LoaderListener l, IMovieItem item);
    /**
     * Cancel current loading process.
     */
    void cancelList();

}
