package com.mediatek.gallery3d.video;

import android.content.Context;
import android.net.Uri;

import com.android.gallery3d.common.BlobCache;
import com.android.gallery3d.util.CacheManager;

import com.mediatek.gallery3d.util.Log;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;

/**
 * Bookmarker class is used to save video playing information,
 * and recover it at next time
 */
public class Bookmarker {
    private static final String TAG = "VP_Bookmarker";

    private static final String BOOKMARK_CACHE_FILE = "bookmark";
    private static final int BOOKMARK_CACHE_MAX_ENTRIES = 100;
    private static final int BOOKMARK_CACHE_MAX_BYTES = 10 * 1024;
    private static final int BOOKMARK_CACHE_VERSION = 1;

    private static final int HALF_MINUTE = 30 * 1000;
    private static final int TWO_MINUTES = 4 * HALF_MINUTE;

    private final Context mContext;

    public Bookmarker(Context context) {
        mContext = context;
    }

    public void setBookmark(Uri uri, int bookmark, int duration) {
        Log.v(TAG, "setBookmark(" + uri + ", " + bookmark + ", " + duration
                    + ")");
        try {
            // do not record or override bookmark if duration is not valid.
            if (duration < 0) {
                return;
            }
            BlobCache cache = CacheManager.getCache(mContext,
                    BOOKMARK_CACHE_FILE, BOOKMARK_CACHE_MAX_ENTRIES,
                    BOOKMARK_CACHE_MAX_BYTES, BOOKMARK_CACHE_VERSION);

            ByteArrayOutputStream bos = new ByteArrayOutputStream();
            DataOutputStream dos = new DataOutputStream(bos);
            dos.writeUTF(uri.toString());
            dos.writeInt(bookmark);
            dos.writeInt(Math.abs(duration));
            dos.flush();
            cache.insert(uri.hashCode(), bos.toByteArray());
        } catch (Throwable t) {
            Log.w(TAG, "setBookmark failed", t);
        }
    }

    public BookmarkerInfo getBookmark(Uri uri) {
        try {
            BlobCache cache = CacheManager.getCache(mContext,
                    BOOKMARK_CACHE_FILE, BOOKMARK_CACHE_MAX_ENTRIES,
                    BOOKMARK_CACHE_MAX_BYTES, BOOKMARK_CACHE_VERSION);

            byte[] data = cache.lookup(uri.hashCode());
            if (data == null) {
                Log.v(TAG, "getBookmark(" + uri
                            + ") data=null. uri.hashCode()=" + uri.hashCode());
                return null;
            }

            DataInputStream dis = new DataInputStream(new ByteArrayInputStream(
                    data));

            String uriString = DataInputStream.readUTF(dis);
            int bookmark = dis.readInt();
            int duration = dis.readInt();
            Log.v(TAG, "getBookmark(" + uri + ") uriString=" + uriString
                        + ", bookmark=" + bookmark + ", duration=" + duration);
            if (!uriString.equals(uri.toString())) {
                return null;
            }

            if ((bookmark < HALF_MINUTE) || (duration < TWO_MINUTES)
                    || (bookmark > (duration - HALF_MINUTE))) {
                return null;
            }
            return new BookmarkerInfo(bookmark, duration);
        } catch (Throwable t) {
            Log.w(TAG, "getBookmark failed", t);
        }
        return null;
    }

    public class BookmarkerInfo {
        public final int mBookmark;
        public final int mDuration;

        public BookmarkerInfo(int bookmark, int duration) {
            this.mBookmark = bookmark;
            this.mDuration = duration;
        }

        @Override
        public String toString() {
            return new StringBuilder().append("BookmarkInfo(bookmark=")
                    .append(mBookmark).append(", duration=").append(mDuration)
                    .append(")").toString();
        }
    }
}

