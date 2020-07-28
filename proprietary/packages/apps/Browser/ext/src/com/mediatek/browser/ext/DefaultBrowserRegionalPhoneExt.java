package com.mediatek.browser.ext;

import android.content.ContentValues;
import android.content.Context;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.res.Resources;
import android.content.res.TypedArray;
import android.content.SharedPreferences;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Build;
import android.os.SystemProperties;
import android.preference.PreferenceManager;
import android.util.Log;

import com.android.browser.provider.BrowserContract;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;

public class DefaultBrowserRegionalPhoneExt implements IBrowserRegionalPhoneExt {

    private static final String TAG = "DefaultBrowserRegionalPhoneExt";
    private static final String GOOGLE_URL = "https://www.google.com/";
    private static final long FIXED_ID_ROOT = 1;
    private static final String mApp = "com.android.browser";

    // M : flag for auto on/off log
    public final static boolean DEBUG =
        (!Build.TYPE.equals("user")) ? true :
            SystemProperties.getBoolean("ro.mtk_browser_debug_enablelog", false);

    @Override
    public String getSearchEngine(SharedPreferences pref, Context context) {
        Log.i("@M_" + TAG, "Enter: " + "updateSearchEngine" + " --default implement");
        return null;
    }

    @Override
    public void updateBookmarks(Context context) {
        if (!needUpdateBookmarks(context)) {
            Log.i("@M_" + TAG, "Enter: " + "updateBookmarks" + " --default implement");
        } else {
            new UpdateBookmarkTask(context).execute();
        }
    }

    private boolean needUpdateBookmarks(Context context) {
        /*
        SharedPreferences p = PreferenceManager.getDefaultSharedPreferences(context);
        String sysProperty = SystemProperties.get("persist.vendor.operator.optr");
        Log.d(TAG, "system property = " + sysProperty);
        String currentOperator = p.getString("operator_bookmarks", "OPNONE");
        Log.d(TAG, "currentOperator = " + currentOperator);
        boolean opUpdated = !(sysProperty.equals(currentOperator));
        if (opUpdated) {
            opUpdated = currentOperator.equals("OP03");
        }
        SharedPreferences.Editor editor = p.edit();
        editor.putString("operator_bookmarks", sysProperty);
        editor.commit();
        return opUpdated;
        */
        return false;
    }

    private static class UpdateBookmarkTask extends AsyncTask<Void, Void, Void> {
        Context mContext;

        public UpdateBookmarkTask(Context context) {
            mContext = context;
        }

        @Override
        protected Void doInBackground(Void... unused) {
            removeOperatorBookmarks(mContext);
            addOMBookmarks(mContext);
            return null;
        }

        private void removeOperatorBookmarks(Context context) {
            Resources res = null;
            Uri bookmarksUri = BrowserContract.Bookmarks.CONTENT_URI;
            try {
                res = context.getPackageManager().getResourcesForApplication(mApp);
                int resourceId = res.getIdentifier(mApp + ":array/bookmarks_for_op03",null,null);
                Log.d(TAG , "OP03 resourceId = " + resourceId);
                CharSequence[] bookmarks = null;
                if (0 != resourceId) {
                    bookmarks = res.getTextArray(resourceId);
                }
                if (bookmarks != null) {
                    Log.d(TAG, " OP03 bookmarks size = " + bookmarks.length);
                    int size = (bookmarks.length / 2);
                    String[] mEntriesString = new String[size];
                    for (int i = 0; i < size ; i++) {
                        CharSequence ch = bookmarks[2 * i + 1];
                        mEntriesString[i] = ch.toString();
                    }
                    int delCount = context.getContentResolver().delete(bookmarksUri
                        , "url" + makeInQueryString(size), mEntriesString);
                    Log.d(TAG, "Delete count = " + delCount);
                }
            } catch (NameNotFoundException e) {
                e.printStackTrace();
            }
        }

        private void addOMBookmarks(Context context) {
            Resources res = null;
            try {
                res = context.getPackageManager().getResourcesForApplication(mApp);
                int resourceId = res.getIdentifier(mApp + ":array/bookmarks_for_yahoo",null,null);
                Log.d(TAG , "addOMBookmarks(), first resourceId = " + resourceId);
                CharSequence[] bookmarks = null;
                if (0 != resourceId) {
                    bookmarks = res.getTextArray(resourceId);
                }
                resourceId = res.getIdentifier(mApp + ":array/bookmark_preloads_for_yahoo"
                                            , null, null);
                Log.d(TAG , "addOMBookmarks(), first Preload resourceId = " + resourceId);
                TypedArray preloads = null;
                if (0 != resourceId) {
                    preloads = res.obtainTypedArray(resourceId);
                }
                if (bookmarks == null || preloads == null) {
                    return;
                }
                int position = addDefaultBookmarks(context, bookmarks, preloads, 2);
                resourceId = res.getIdentifier(mApp + ":array/bookmarks", null, null);
                Log.d(TAG , "addOMBookmarks(), Other resourceId = " + resourceId);
                bookmarks = null;
                if (0 != resourceId) {
                    bookmarks = res.getTextArray(resourceId);
                }
                resourceId = res.getIdentifier(mApp + ":array/bookmark_preloads", null, null);
                Log.d(TAG , "addOMBookmarks(), other Preload resourceId = " + resourceId);
                preloads = null;
                if (0 != resourceId) {
                    preloads = res.obtainTypedArray(resourceId);
                }
                if (bookmarks == null || preloads == null) {
                    return;
                }
                addDefaultBookmarks(context, bookmarks, preloads, position);
            } catch (NameNotFoundException e) {
                e.printStackTrace();
            }
        }

        private int addDefaultBookmarks(Context cntx, CharSequence[] bookmarks,
                                    TypedArray preloads, int position) {
            Resources res = cntx.getResources();
            int size = bookmarks.length;
            if (DEBUG) {
                Log.i(TAG, "bookmarks count = " + size);
            }
            try {
                String parent = Long.toString(FIXED_ID_ROOT);
                String now = Long.toString(System.currentTimeMillis());
                for (int i = 0; i < size; i = i + 2) {
                    Uri newUri = null;
                    boolean isUpdated = false;
                    CharSequence bookmarkDestination = bookmarks[i + 1];
                    int finalPos = 0;
                    if (GOOGLE_URL.equals(bookmarkDestination.toString())) {
                        finalPos = 1;
                    } else {
                        finalPos = position + i;
                    }
                    int thumbId = preloads.getResourceId(i + 1, 0);
                    int favIconId = preloads.getResourceId(i, 0);
                    byte[] thumb = null, favicon = null;
                    try {
                        thumb = readRaw(res, thumbId);
                        favicon = readRaw(res, favIconId);
                    } catch (IOException e) {
                          Log.i(TAG, "IOException for thumb");
                    }
                    ContentValues values = new ContentValues();
                    values.put(BrowserContract.Bookmarks.TITLE, bookmarks[i].toString());
                    values.put(BrowserContract.Bookmarks.URL, bookmarkDestination.toString());
                    values.put(BrowserContract.Bookmarks.IS_FOLDER, 0);
                    values.put(BrowserContract.Bookmarks.THUMBNAIL, thumb);
                    values.put(BrowserContract.Bookmarks.FAVICON, favicon);
                    values.put(BrowserContract.Bookmarks.PARENT, parent);
                    values.put(BrowserContract.Bookmarks.POSITION, (Integer.toString(finalPos)));
                    values.put(BrowserContract.Bookmarks.DATE_CREATED, now);
                    newUri = cntx.getContentResolver().insert(BrowserContract.Bookmarks.CONTENT_URI
                    , values);
                    if (newUri != null) {
                        isUpdated = true;
                    }
                    if (DEBUG) {
                        Log.i(TAG, "for " + i + "update result = " + isUpdated);
                    }
               }
            } catch (ArrayIndexOutOfBoundsException e) {
                Log.i(TAG, "ArrayIndexOutOfBoundsException is caught");
            } finally {
                preloads.recycle();
            }
            return size;
        }

        private String makeInQueryString(int size) {
            StringBuilder sb = new StringBuilder();
            if (size > 0) {
                sb.append(" IN ( ");
                String placeHolder = "";
                for (int i = 0; i < size; i++) {
                    sb.append(placeHolder);
                    sb.append("?");
                    placeHolder = ",";
                }
                sb.append(" )");
            }
            return sb.toString();
        }

        private byte[] readRaw(Resources res, int id) throws IOException {
            if (id == 0) {
                return null;
            }
            InputStream is = res.openRawResource(id);
            try {
                ByteArrayOutputStream bos = new ByteArrayOutputStream();
                byte[] buf = new byte[4096];
                int read;
                while ((read = is.read(buf)) > 0) {
                    bos.write(buf, 0, read);
                }
                bos.flush();
                return bos.toByteArray();
            } finally {
                is.close();
            }
        }
    }
}

