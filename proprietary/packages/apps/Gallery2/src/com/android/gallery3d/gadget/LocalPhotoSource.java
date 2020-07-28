/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2011 The Android Open Source Project
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

package com.android.gallery3d.gadget;

import android.content.ContentResolver;
import android.content.Context;
import android.database.ContentObserver;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.net.Uri;
import android.os.Environment;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.provider.MediaStore.Images;
import android.provider.MediaStore.Images.Media;

import com.android.gallery3d.app.GalleryApp;
import com.android.gallery3d.common.Utils;
import com.android.gallery3d.data.ContentListener;
import com.android.gallery3d.data.DataManager;
import com.android.gallery3d.data.LocalImage;
import com.android.gallery3d.data.MediaItem;
import com.android.gallery3d.data.Path;
import com.android.gallery3d.util.GalleryUtils;
import com.mediatek.gallery3d.util.Log;
import com.mediatek.gallerybasic.base.MediaFilterSetting;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Random;

public class LocalPhotoSource implements WidgetSource {

    @SuppressWarnings("unused")
    private static final String TAG = "Gallery2/LocalPhotoSource";

    private static final int MAX_PHOTO_COUNT = 128;

    /* Static fields used to query for the correct set of images */
    private static final Uri CONTENT_URI = Media.EXTERNAL_CONTENT_URI;
    private static final String DATE_TAKEN = Media.DATE_TAKEN;
    private static final String[] PROJECTION = {Media._ID};
    private static final String[] COUNT_PROJECTION = {"count(*)"};
    /* We don't want to include the download directory */
    /// M: [FEATURE.MODIFY] <DRM> @{
    /* private static final String SELECTION =
            String.format("%s != %s", Media.BUCKET_ID, getDownloadBucketId());*/
    private static String SELECTION;
    /// @}
    private static final String ORDER = String.format("%s DESC", DATE_TAKEN);

    private Context mContext;
    private ArrayList<Long> mPhotos = new ArrayList<Long>();
    private ContentListener mContentListener;
    private ContentObserver mContentObserver;
    private boolean mContentDirty = true;
    private DataManager mDataManager;
    private static final Path LOCAL_IMAGE_ROOT = Path.fromString("/local/image/item");

    /// M: [BUG.ADD] @{
    private static final int MILLI_SECOND_WHEN_WAIT_LOOPER = 5;
    /// @}

    public LocalPhotoSource(Context context) {
        /// M: [FEATURE.ADD] <DRM> @{
        SELECTION = MediaFilterSetting.getExtWhereClause(String.format(
                "%s != %s", Media.BUCKET_ID, getDownloadBucketId()));
        /// @}
        mContext = context;
        mDataManager = ((GalleryApp) context.getApplicationContext()).getDataManager();
        /// M: [BUG.ADD] @{
        mHandlerThread = new HandlerThread("LocalPhotoSource-HandlerThread",
                android.os.Process.THREAD_PRIORITY_BACKGROUND);
        mHandlerThread.start();
        Looper looper = mHandlerThread.getLooper();
        // if HandlerThread run slowly, looper may be null, wait until looper !=
        // null
        while (looper == null) {
            Log.d(TAG, "<LocalPhotoSource> looper is null, wait 5 ms");
            try {
                Thread.sleep(MILLI_SECOND_WHEN_WAIT_LOOPER);
            } catch (InterruptedException e) {
                Log.d(TAG, "<LocalPhotoSource> Thread.sleep InterruptedException");
            }
            looper = mHandlerThread.getLooper();
        }
        /// @}
        /// M: [BUG.MODIFY] @{
        /*mContentObserver = new ContentObserver(new Handler()) {*/
        mContentObserver = new ContentObserver(new Handler(looper)) {
        /// @}
            @Override
            public void onChange(boolean selfChange) {
                /// M: [DEBUG.ADD] @{
                Log.d(TAG, "<LocalPhotoSource> ContentObserver.onChange: selfchange=" + selfChange
                        + ", listener=" + mContentListener);
                /// @}
                mContentDirty = true;
                if (mContentListener != null) mContentListener.onContentDirty();
            }
        };
        mContext.getContentResolver()
                .registerContentObserver(CONTENT_URI, true, mContentObserver);
        /// M: [DEBUG.ADD] @{
        Log.d(TAG, "<LocalPhotoSource>content observer registered!!");
        /// @}
    }

    @Override
    public void close() {
        mContext.getContentResolver().unregisterContentObserver(mContentObserver);
        /// M: [BUG.ADD] @{
        mHandlerThread.quit();
        /// @}
    }

    @Override
    public Uri getContentUri(int index) {
        /// M: [BUG.MODIFY] @{
        /*if (index < mPhotos.size()) {
            return CONTENT_URI.buildUpon()
                    .appendPath(String.valueOf(mPhotos.get(index)))
                    .build();
            return null;
        }*/
        synchronized (mPhotos) {
            if (index < mPhotos.size()) {
                return CONTENT_URI.buildUpon()
                        .appendPath(String.valueOf(mPhotos.get(index)))
                        .build();
            }
            return null;
        }
        /// @}

    }

    @Override
    public Bitmap getImage(int index) {
        /// M: [BUG.MODIFY] @{
        /*if (index >= mPhotos.size()) return null;*/
        long id;
        synchronized (mPhotos) {
            if (index >= mPhotos.size()) {
                Log.e(TAG, "getImage: index out of range: " + index + ", size=" + mPhotos.size());
                return null;
            }
            id = mPhotos.get(index);
        }
        /// @}
        MediaItem image = (MediaItem)
                mDataManager.getMediaObject(LOCAL_IMAGE_ROOT.getChild(id));
        /// M: [DEBUG.ADD] @{
        Log.d(TAG, "getImage: id=" + id
                + ", mediaitem=" + (image == null ? "null" : image.getName()));
        /// @}
        if (image == null) return null;

        /// M: [BUG.ADD] @{
        // In some cases, the information of image is not latest.
        // In order to get correct bitmap, update image here
        if (image instanceof LocalImage) {
            try {
                Cursor cursor = mContext.getContentResolver().query(
                        Images.Media.EXTERNAL_CONTENT_URI,
                        /// M: [FEATURE.MODIFY] @{
                        /*LocalImage.PROJECTION, "_id = ?",*/
                        LocalImage.getProjection(), "_id = ?",
                        /// @}
                        new String[] { String.valueOf(((LocalImage) image).id) }, null);
                if (cursor != null && cursor.moveToFirst()) {
                    ((LocalImage) image).updateContent(cursor);
                }
                if (cursor != null) {
                    cursor.close();
                }
            } catch (SecurityException e) {
                Log.d(TAG, "<getImage> SecurityException", e);
                return null;
            }
        }
        /// @}

        return WidgetUtils.createWidgetBitmap(image);
    }

    private int[] getExponentialIndice(int total, int count) {
        Random random = new Random();
        if (count > total) count = total;
        HashSet<Integer> selected = new HashSet<Integer>(count);
        while (selected.size() < count) {
            int row = (int)(-Math.log(random.nextDouble()) * total / 2);
            if (row < total) selected.add(row);
        }
        int values[] = new int[count];
        int index = 0;
        for (int value : selected) {
            values[index++] = value;
        }
        return values;
    }

    private int getPhotoCount(ContentResolver resolver) {
        /// M: [FEATURE.ADD] [Runtime permission] @{
        try {
        /// @}
            Cursor cursor = resolver.query(
                    CONTENT_URI, COUNT_PROJECTION, SELECTION, null, null);
            if (cursor == null) return 0;
            try {
                Utils.assertTrue(cursor.moveToNext());
                return cursor.getInt(0);
            } finally {
                cursor.close();
            }
        /// M: [FEATURE.ADD] [Runtime permission] @{
        } catch (SecurityException e) {
            Log.d(TAG, "<getPhotoCount> SecurityException", e);
            return 0;
        }
        /// @}
    }

    private boolean isContentSound(int totalCount) {
        if (mPhotos.size() < Math.min(totalCount, MAX_PHOTO_COUNT)) return false;
        if (mPhotos.size() == 0) return true; // totalCount is also 0

        StringBuilder builder = new StringBuilder();
        for (Long imageId : mPhotos) {
            if (builder.length() > 0) builder.append(",");
            builder.append(imageId);
        }

        /// M: [FEATURE.ADD] [Runtime permission] @{
        try {
        /// @}
            Cursor cursor = mContext.getContentResolver().query(
                    CONTENT_URI, COUNT_PROJECTION,
                    String.format("%s in (%s)", Media._ID, builder.toString()),
                    null, null);
            if (cursor == null) return false;
            try {
                Utils.assertTrue(cursor.moveToNext());
                return cursor.getInt(0) == mPhotos.size();
            } finally {
                cursor.close();
            }
        /// M: [FEATURE.ADD] [Runtime permission] @{
        } catch (SecurityException e) {
            Log.d(TAG, "<isContentSound> SecurityException", e);
            return false;
        }
        /// @}
    }

    @Override
    /// M: [BUG.MODIFY] @{
    // reload() and size() maybe called in different thread, so add synchronized
    /*public void reload() {*/
    public synchronized void reload() {
    /// @}
        if (!mContentDirty) return;
        mContentDirty = false;

        ContentResolver resolver = mContext.getContentResolver();
        int photoCount = getPhotoCount(resolver);
        /// M: [DEBUG.ADD] @{
        Log.d(TAG, "<reload> DB photo count=" + photoCount);
        /// @}
        /// M: [BUG.MODIFY] @{
        /*if (isContentSound(photoCount)) return;*/
        synchronized (mPhotos) {
            if (isContentSound(photoCount)) {
                Log.w(TAG, "<reload> content is sound, return and do nothing...");
                return;
            }
        }
        /// @}

        int choosedIds[] = getExponentialIndice(photoCount, MAX_PHOTO_COUNT);
        /// M: [DEBUG.ADD] @{
        Log.d(TAG, "<reload> random ids count=" + choosedIds.length);
        /// @}
        Arrays.sort(choosedIds);
        /// M: [BUG.MODIFY] @{
        // mPhotos.clear();
        ArrayList<Long> temp = new ArrayList<Long>();
        /// @}

        /// M: [FEATURE.ADD] [Runtime permission] @{
        try {
        /// @}
            Cursor cursor = mContext.getContentResolver().query(
                    CONTENT_URI, PROJECTION, SELECTION, null, ORDER);
            /// M: [DEBUG.MARK] @{
            /*if (cursor == null) return;*/
            /// @}
            if (cursor == null) {
                /// M: [DEBUG.ADD] @{
                Log.e(TAG, "<reload> query returns null");
                /// @}
                return;
            }
            try {
                for (int index : choosedIds) {
                    if (cursor.moveToPosition(index)) {
                        /// M: [BUG.MODIFY] @{
                        //mPhotos.add(cursor.getLong(0));
                        temp.add(cursor.getLong(0));
                        /// @}
                    }
                }
            } finally {
                cursor.close();
            }
        /// M: [FEATURE.ADD] [Runtime permission] @{
        } catch (SecurityException e) {
            Log.d(TAG, "<reload> SecurityException", e);
            return;
        }
        /// @}
        /// M: [BUG.MODIFY] @{
        synchronized (mPhotos) {
            mPhotos.clear();
            mPhotos.addAll(temp);
            temp.clear();
        }
        /// @}
        /// M: [DEBUG.ADD] @{
        Log.d(TAG, "<reload>reload result: new photo count=" + mPhotos.size());
        /// @}
    }

    @Override
    /// M: [BUG.MODIFY] @{
    // reload() and size() maybe called in different thread, so add synchronized
    /* public int size()  {
     *  reload();
        return mPhotos.size();
    }*/
    public synchronized int size() {
        reload();
        synchronized (mPhotos) {
            return mPhotos.size();
        }
    }
    /// @}
    /**
     * Builds the bucket ID for the public external storage Downloads directory
     * @return the bucket ID
     */
    private static int getDownloadBucketId() {
        String downloadsPath = Environment
                .getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS)
                .getAbsolutePath();
        return GalleryUtils.getBucketId(downloadsPath);
    }

    @Override
    public void setContentListener(ContentListener listener) {
        mContentListener = listener;
    }

    // ********************************************************************
    //*                              MTK                                 *
    //********************************************************************

    private HandlerThread mHandlerThread = null;

    // [Runtime permission] @{
    @Override
    public void forceNotifyDirty() {
        mContentObserver.onChange(true);
    }
    // @}
}
