/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2009 The Android Open Source Project
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

package com.android.browser;

import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.net.Uri;
import android.os.AsyncTask;
import android.preference.PreferenceManager;
import com.android.browser.provider.BrowserContract;
import com.android.browser.provider.BrowserContract.Combined;
import com.android.browser.provider.BrowserContract.Images;
import android.text.TextUtils;
import android.util.Log;
import android.webkit.WebIconDatabase;
import android.widget.Toast;


import java.io.ByteArrayOutputStream;

/**
 *  This class is purely to have a common place for adding/deleting bookmarks.
 */
public class Bookmarks {
    // We only want the user to be able to bookmark content that
    // the browser can handle directly.
    private static final String acceptableBookmarkSchemes[] = {
            "http:",
            "https:",
            "about:",
            "data:",
            "javascript:",
            "file:",
            "content:",
            "rtsp:"
    };

    /// M: add for debug @ {
    private static final boolean DEBUG = Browser.DEBUG;
    /// @ }
    private final static String LOGTAG = "Bookmarks";
    private static final String XLOGTAG = "browser/Bookmarks";
    /**
     *  Add a bookmark to the database.
     *  @param context Context of the calling Activity.  This is used to make
     *          Toast confirming that the bookmark has been added.  If the
     *          caller provides null, the Toast will not be shown.
     *  @param url URL of the website to be bookmarked.
     *  @param name Provided name for the bookmark.
     *  @param thumbnail A thumbnail for the bookmark.
     *  @param retainIcon Whether to retain the page's icon in the icon database.
     *          This will usually be <code>true</code> except when bookmarks are
     *          added by a settings restore agent.
     *  @param parent ID of the parent folder.
     */
    /* package */ static void addBookmark(Context context, boolean showToast, String url,
            String name, Bitmap thumbnail, long parent) {
        // Want to append to the beginning of the list
        ContentValues values = new ContentValues();
        deleteSameTitle(context, name, parent);
        deleteSameUrl(context, url, parent);
        try {
            //Google redundancy code.
            //SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
            values.put(BrowserContract.Bookmarks.TITLE, name);
            values.put(BrowserContract.Bookmarks.URL, url);
            values.put(BrowserContract.Bookmarks.IS_FOLDER, 0);
            values.put(BrowserContract.Bookmarks.THUMBNAIL,
                    bitmapToBytes(thumbnail));
            values.put(BrowserContract.Bookmarks.PARENT, parent);
            context.getContentResolver().insert(BrowserContract.Bookmarks.CONTENT_URI, values);
        } catch (IllegalStateException e) {
            Log.e(LOGTAG, "addBookmark", e);
        }
        if (showToast) {
            Toast.makeText(context, R.string.added_to_bookmarks,
                    Toast.LENGTH_LONG).show();
        }
    }

    /**
     * M: delete bookmark if exist same url
     */
    private static void deleteSameUrl(Context context, String url, long parent) {
        if (DEBUG) {
            Log.d(XLOGTAG, "deleteSameUrl url:" + url);
        }
        if (url == null || url.length() == 0) {
            return;
        }
        ContentValues values = new ContentValues();
        values.put(BrowserContract.Bookmarks.IS_DELETED, 1);
        int count = context.getContentResolver().update(
                BrowserContract.Bookmarks.CONTENT_URI, values,
                BrowserContract.Bookmarks.URL + " =? AND " + BrowserContract.Bookmarks.PARENT
                        + " =? AND " + BrowserContract.Bookmarks.IS_DELETED + " =?", new String[] {
                        url, String.valueOf(parent), String.valueOf(0)
                });
        Log.d(XLOGTAG, "same url delete :" + count);
    }

    /**
     * M: delete bookmark if exist same name
     */
    private static void deleteSameTitle(Context context, String name, long parent) {
        Log.d(XLOGTAG, "deleteSameTitle title:" + name);
        if (name == null || name.length() == 0) {
            return;
        }
        ContentValues values = new ContentValues();
        values.put(BrowserContract.Bookmarks.IS_DELETED, 1);
        int count = context.getContentResolver().update(
                BrowserContract.Bookmarks.CONTENT_URI, values,
                BrowserContract.Bookmarks.TITLE + " =? AND " + BrowserContract.Bookmarks.PARENT
                        + " =? AND " + BrowserContract.Bookmarks.IS_DELETED + " =? AND "
                        + BrowserContract.Bookmarks.IS_FOLDER + " =0 ", new String[] {
                        name, String.valueOf(parent), String.valueOf(0)
                });
        Log.d(XLOGTAG, "same title delete :" + count);
    }

    /**
     *  Remove a bookmark from the database.  If the url is a visited site, it
     *  will remain in the database, but only as a history item, and not as a
     *  bookmarked site.
     *  @param context Context of the calling Activity.  This is used to make
     *          Toast confirming that the bookmark has been removed and to
     *          lookup the correct content uri.  It must not be null.
     *  @param cr The ContentResolver being used to remove the bookmark.
     *  @param url URL of the website to be removed.
     */
    /* package */ static void removeFromBookmarks(Context context,
            ContentResolver cr, String url, String title) {
        Cursor cursor = null;
        try {
            Uri uri = BookmarkUtils.getBookmarksUri(context);
            cursor = cr.query(uri,
                    new String[] { BrowserContract.Bookmarks._ID },
                    BrowserContract.Bookmarks.URL + " = ? AND " +
                            BrowserContract.Bookmarks.TITLE + " = ?",
                    new String[] { url, title },
                    null);

            if (!cursor.moveToFirst()) {
                return;
            }

            // Remove from bookmarks
            WebIconDatabase.getInstance().releaseIconForPageUrl(url);
            uri = ContentUris.withAppendedId(BrowserContract.Bookmarks.CONTENT_URI,
                    cursor.getLong(0));
            cr.delete(uri, null, null);
            if (context != null) {
                Toast.makeText(context, R.string.removed_from_bookmarks,
                        Toast.LENGTH_LONG).show();
            }
        } catch (IllegalStateException e) {
            Log.e(LOGTAG, "removeFromBookmarks", e);
        } finally {
            if (cursor != null) cursor.close();
        }
    }

    private static byte[] bitmapToBytes(Bitmap bm) {
        if (bm == null) {
            return null;
        }

        final ByteArrayOutputStream os = new ByteArrayOutputStream();
        bm.compress(Bitmap.CompressFormat.PNG, 100, os);
        return os.toByteArray();
    }

    /* package */ static boolean urlHasAcceptableScheme(String url) {
        if (url == null) {
            return false;
        }

        for (int i = 0; i < acceptableBookmarkSchemes.length; i++) {
            if (url.startsWith(acceptableBookmarkSchemes[i])) {
                return true;
            }
        }
        return false;
    }
    /// M: modify for bookmarks without slash @ {
    private static String modifyUrl(String url) {
        if (url.endsWith("/")) {
            return url.substring(0, url.length() - 1);
        } else {
            return url;
        }
    }

    static final String QUERY_BOOKMARKS_WHERE =
            Combined.URL + " == ? OR " +
            Combined.URL + " == ? OR " +
            Combined.URL + " == ? OR " +
            Combined.URL + " == ?";

    public static Cursor queryCombinedForUrl(ContentResolver cr,
            String originalUrl, String url) {
        if (cr == null || url == null) {
            return null;
        }

        // If originalUrl is null, just set it to url.
        if (originalUrl == null) {
            originalUrl = url;
        }

        String modifyOriginalUrl = modifyUrl(originalUrl);
        String modifyUrl = modifyUrl(url);

        // Look for both the original url and the actual url. This takes in to
        // account redirects.

        final String[] selArgs = new String[] { originalUrl, url, modifyOriginalUrl, modifyUrl};
        final String[] projection = new String[] { Combined.URL };
        return cr.query(Combined.CONTENT_URI, projection, QUERY_BOOKMARKS_WHERE, selArgs, null);
    }
    /// @ }
    // Strip the query from the given url.
    static String removeQuery(String url) {
        if (url == null) {
            return null;
        }
        int query = url.indexOf('?');
        String noQuery = url;
        if (query != -1) {
            noQuery = url.substring(0, query);
        }
        return noQuery;
    }

    /**
     * Update the bookmark's favicon. This is a convenience method for updating
     * a bookmark favicon for the originalUrl and url of the passed in WebView.
     * @param cr The ContentResolver to use.
     * @param originalUrl The original url before any redirects.
     * @param url The current url.
     * @param favicon The favicon bitmap to write to the db.
     */
    /* package */ static void updateFavicon(final ContentResolver cr,
            final String originalUrl, final String url, final Bitmap favicon) {
        new AsyncTask<Void, Void, Void>() {
            @Override
            protected Void doInBackground(Void... unused) {
                final ByteArrayOutputStream os = new ByteArrayOutputStream();
                favicon.compress(Bitmap.CompressFormat.PNG, 100, os);

                // The Images update will insert if it doesn't exist
                ContentValues values = new ContentValues();
                values.put(Images.FAVICON, os.toByteArray());
                updateImages(cr, originalUrl, values);
                updateImages(cr, url, values);
                return null;
            }

            private void updateImages(final ContentResolver cr,
                    final String url, ContentValues values) {
                String iurl = removeQuery(url);
                if (!TextUtils.isEmpty(iurl)) {
                    values.put(Images.URL, iurl);
                    cr.update(BrowserContract.Images.CONTENT_URI, values, null, null);
                }
            }
        }.execute();
    }

    /**
     * M: get the bookmark id by name or url
     */
    /*packages*/ static int getIdByNameOrUrl(ContentResolver cr, String name, String url,
                                             long parentId, long currentId) {
        String where = BrowserContract.Bookmarks.PARENT + " = ?"
        + " AND (" + BrowserContract.Bookmarks.TITLE + " = ?"
        + " OR " + BrowserContract.Bookmarks.URL + " = ?"
        + " OR " + BrowserContract.Bookmarks.URL + " = ?)"
        + " AND " + BrowserContract.Bookmarks.IS_FOLDER + "= 0";
        if (currentId > 0) {
            where = where + " AND " + BrowserContract.Bookmarks._ID + " <> " + currentId;
        }
        Log.v(XLOGTAG, "getIdByNameOrUrl() sql:" + where);
        String[] projection = {BrowserContract.Bookmarks._ID};
        Cursor cursor = cr.query(BrowserContract.Bookmarks.CONTENT_URI, projection, where,
                new String[] {String.valueOf(parentId), name, url,
                url.endsWith("/") ? url.substring(0, url.lastIndexOf("/")) : (url + "/") },
                BrowserContract.Bookmarks._ID + " DESC");
        if (cursor != null) {
            try {
                if (cursor.moveToFirst()) {
                    return cursor.getInt(0);
                }
            } finally {
                cursor.close();
            }
        }
        return -1;
    }
}
