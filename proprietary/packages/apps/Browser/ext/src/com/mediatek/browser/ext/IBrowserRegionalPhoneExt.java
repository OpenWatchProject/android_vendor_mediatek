package com.mediatek.browser.ext;

import android.content.Context;
import android.content.SharedPreferences;

public interface IBrowserRegionalPhoneExt {

    /**
     * Get the search engine through the regional phone manager
     * @param pref the Browser default shared preferences
     * @param context the context
     * @return the default search engine name
     */
    String getSearchEngine(SharedPreferences pref, Context context);

    /**
     * Update the bookmarks through the regional phone manager
     * @param context the context
     */
    void updateBookmarks(Context context);

}

