/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2010 The Android Open Source Project
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

package com.android.gallery3d.data;

import android.content.Context;

import com.android.gallery3d.common.BlobCache;
import com.android.gallery3d.common.BlobCache.LookupRequest;
import com.android.gallery3d.common.Utils;
import com.android.gallery3d.data.BytesBufferPool.BytesBuffer;
import com.android.gallery3d.util.CacheManager;
import com.android.gallery3d.util.GalleryUtils;

import java.io.IOException;
import java.nio.ByteBuffer;

public class ImageCacheService {
    @SuppressWarnings("unused")
    private static final String TAG = "Gallery2/ImageCacheService";

    /// M: [BUG.MODIFY] For rename file name @{
    /*    private static final String IMAGE_CACHE_FILE = "imgcache";*/
    private static String IMAGE_CACHE_FILE = "imgcache";
    /// @}
    private static final int IMAGE_CACHE_MAX_ENTRIES = 5000;
    private static final int IMAGE_CACHE_MAX_BYTES = 200 * 1024 * 1024;
    private static final int IMAGE_CACHE_VERSION = 7;

    private BlobCache mCache;

    public ImageCacheService(Context context) {
        /// M: [BUG.ADD] @{
        mContext = context;
        /// @}
        mCache = CacheManager.getCache(context, IMAGE_CACHE_FILE,
                IMAGE_CACHE_MAX_ENTRIES, IMAGE_CACHE_MAX_BYTES,
                IMAGE_CACHE_VERSION);
        // / M: [BUG.ADD] @{
        Log.d(TAG, " <ImageCacheService> IMAGE_CACHE_FILE = "
                + IMAGE_CACHE_FILE + " mCache = " + mCache);
        // / @}
        }

    /**
     * Gets the cached image data for the given <code>path</code>,
     *  <code>timeModified</code> and <code>type</code>.
     *
     * The image data will be stored in <code>buffer.data</code>, started from
     * <code>buffer.offset</code> for <code>buffer.length</code> bytes. If the
     * buffer.data is not big enough, a new byte array will be allocated and returned.
     *
     * @return true if the image data is found; false if not found.
     */
    public boolean getImageData(Path path, long timeModified, int type, BytesBuffer buffer) {
        /// M: [BUG.ADD] @{
        synchronized (mCacheLock) {
            if (mCache == null) {
                Log.e(TAG, "<getImageData>: cache file is null!");
                return false;
            }
        }
        /// @}

        byte[] key = makeKey(path, timeModified, type);
        long cacheKey = Utils.crc64Long(key);
        try {
            LookupRequest request = new LookupRequest();
            request.key = cacheKey;
            request.buffer = buffer.data;
            /// M: [BUG.MODIFY] @{
            /*synchronized (mCache) {
             if (!mCache.lookup(request)) return false;
             }*/
            synchronized (mCacheLock) {
                if (mCache == null) {
                    return false;
                }
                if (!mCache.lookup(request)) return false;
            }
            /// @}
            if (isSameKey(key, request.buffer)) {
                buffer.data = request.buffer;
                buffer.offset = key.length;
                buffer.length = request.length - buffer.offset;
                return true;
            }
        } catch (IOException ex) {
            // ignore.
        }
        return false;
    }

    public void putImageData(Path path, long timeModified, int type, byte[] value) {
        /// M: [BUG.ADD] @{
        synchronized (mCacheLock) {
            if (mCache == null) {
                Log.e(TAG, "<putImageData>: cache file is null!");
                return;
            }
        }
        /// @}
        byte[] key = makeKey(path, timeModified, type);
        long cacheKey = Utils.crc64Long(key);
        ByteBuffer buffer = ByteBuffer.allocate(key.length + value.length);
        buffer.put(key);
        buffer.put(value);
        /// M: [BUG.MODIFY] @{
        /*synchronized (mCache) {
         try {
         mCache.insert(cacheKey, buffer.array());*/
        synchronized (mCacheLock) {
            try {
                if (mCache != null) {
                    mCache.insert(cacheKey, buffer.array());
                }
        /// @}
            } catch (IOException ex) {
                // ignore.
            }
        }
    }

    public void clearImageData(Path path, long timeModified, int type) {
        byte[] key = makeKey(path, timeModified, type);
        long cacheKey = Utils.crc64Long(key);
        /// M: [BUG.MODIFY] @{
        /*
        synchronized (mCache) {*/
        synchronized (mCacheLock) {
        /// @}
            try {
                /// M: [BUG.MODIFY] check mCache is not null, fix JE @{
                /*mCache.clearEntry(cacheKey);*/
                if (mCache != null) {
                    mCache.clearEntry(cacheKey);
                }
                /// @}
            } catch (IOException ex) {
                // ignore.
            }
        }
    }

    private static byte[] makeKey(Path path, long timeModified, int type) {
        return GalleryUtils.getBytes(path.toString() + "+" + timeModified + "+" + type);
    }

    private static boolean isSameKey(byte[] key, byte[] buffer) {
        int n = key.length;
        if (buffer.length < n) {
            return false;
        }
        for (int i = 0; i < n; ++i) {
            if (key[i] != buffer[i]) {
                return false;
            }
        }
        return true;
    }

    //********************************************************************
    //*                              MTK                                 *
    //********************************************************************
    // M: for closing/re-opening cache
    private Context mContext;
    private Object mCacheLock = new Object();
    // force to mark an image cache out of date
    public static volatile String sForceObsoletePath = null;

    // M: for closing/re-opening cache
    public void closeCache() {
        synchronized (mCacheLock) {
            // simply clear the reference,
            // since the BlobCache should already be closed in CacheManager
            mCache = null;
        }
    }

    public void openCache() {
        synchronized (mCacheLock) {
            if (mCache == null) {
                // re-open the cache
                mCache = CacheManager.getCache(mContext, IMAGE_CACHE_FILE,
                        IMAGE_CACHE_MAX_ENTRIES, IMAGE_CACHE_MAX_BYTES,
                        IMAGE_CACHE_VERSION);
            }
        }
    }

    /// create new cache for gallery widget.
    public static void setCacheName(String name) {
        IMAGE_CACHE_FILE = name;
    }
}
