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

import android.content.ContentResolver;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapRegionDecoder;
import android.net.Uri;
import android.provider.MediaStore.Video;
import android.provider.MediaStore.Video.VideoColumns;

import com.android.gallery3d.app.GalleryApp;
import com.android.gallery3d.common.BitmapUtils;
import com.android.gallery3d.util.GalleryUtils;
import com.android.gallery3d.util.ThreadPool.Job;
import com.android.gallery3d.util.ThreadPool.JobContext;
import com.android.gallery3d.util.UpdateHelper;

import com.mediatek.gallery3d.adapter.FeatureHelper;
import com.mediatek.gallery3d.adapter.MediaDataParser;
import com.mediatek.gallery3d.adapter.PhotoPlayFacade;
import com.mediatek.gallery3d.layout.FancyHelper;
import com.mediatek.gallery3d.util.SystemPropertyUtils;
import com.mediatek.gallerybasic.base.ExtFields;
import com.mediatek.gallerybasic.base.ExtItem;
import com.mediatek.gallerybasic.base.ExtItem.Thumbnail;
import com.mediatek.gallerybasic.util.ExtFieldsUtils;
import com.mediatek.galleryportable.PerfServiceUtils;
import com.mediatek.galleryportable.TraceHelper;

// LocalVideo represents a video in the local storage.
public class LocalVideo extends LocalMediaItem {
    private static final String TAG = "Gallery2/LocalVideo";
    static final Path ITEM_PATH = Path.fromString("/local/video/item");

    /// M: [FEATURE.MODIFY] @{
    /*// Must preserve order between these indices and the order of the terms in
     // the following PROJECTION array.
     private static final int INDEX_ID = 0;
     private static final int INDEX_CAPTION = 1;
     private static final int INDEX_MIME_TYPE = 2;
     private static final int INDEX_LATITUDE = 3;
     private static final int INDEX_LONGITUDE = 4;
     private static final int INDEX_DATE_TAKEN = 5;
     private static final int INDEX_DATE_ADDED = 6;
     private static final int INDEX_DATE_MODIFIED = 7;
     private static final int INDEX_DATA = 8;
     private static final int INDEX_DURATION = 9;
     private static final int INDEX_BUCKET_ID = 10;
     private static final int INDEX_SIZE = 11;
     private static final int INDEX_RESOLUTION = 12;*/

    /// M: [FEATURE.ADD] fancy layout @{
    private boolean mIsFancyLayoutSupported = FancyHelper.isFancyLayoutSupported();
    /// @}
    public static final int INDEX_ID = 0;
    public static final int INDEX_CAPTION = 1;
    public static final int INDEX_MIME_TYPE = 2;
    public static final int INDEX_LATITUDE = 3;
    public static final int INDEX_LONGITUDE = 4;
    public static final int INDEX_DATE_TAKEN = 5;
    public static final int INDEX_DATE_ADDED = 6;
    public static final int INDEX_DATE_MODIFIED = 7;
    public static final int INDEX_DATA = 8;
    public static final int INDEX_DURATION = 9;
    public static final int INDEX_BUCKET_ID = 10;
    public static final int INDEX_SIZE = 11;
    public static final int INDEX_RESOLUTION = 12;
    /// @}

    private static final String[] PROJECTION = new String[] {
            VideoColumns._ID,
            VideoColumns.TITLE,
            VideoColumns.MIME_TYPE,
            VideoColumns.LATITUDE,
            VideoColumns.LONGITUDE,
            VideoColumns.DATE_TAKEN,
            VideoColumns.DATE_ADDED,
            VideoColumns.DATE_MODIFIED,
            VideoColumns.DATA,
            VideoColumns.DURATION,
            VideoColumns.BUCKET_ID,
            VideoColumns.SIZE,
            VideoColumns.RESOLUTION,
    };

    private final GalleryApp mApplication;

    public int durationInSec;

    public LocalVideo(Path path, GalleryApp application, Cursor cursor) {
        super(path, nextVersionNumber());
        mApplication = application;
        loadFromCursor(cursor);
        /// M: [FEATURE.ADD] @{
        synchronized (mMediaDataLock) {
            mMediaData = MediaDataParser.parseLocalVideoMediaData(this, cursor);
            mExtItem = PhotoPlayFacade.getMediaCenter().getItem(mMediaData);
        }
        /// @}
    }

    public LocalVideo(Path path, GalleryApp context, int id) {
        super(path, nextVersionNumber());
        mApplication = context;
        ContentResolver resolver = mApplication.getContentResolver();
        Uri uri = Video.Media.EXTERNAL_CONTENT_URI;
        /// M: [FEATURE.MODIFY] @{
        //Cursor cursor = LocalAlbum.getItemCursor(resolver, uri, PROJECTION, id);
        Cursor cursor = LocalAlbum.getItemCursor(resolver, uri, getProjection(), id);
        /// @}
        if (cursor == null) {
            throw new RuntimeException("cannot get cursor for: " + path);
        }
        try {
            if (cursor.moveToNext()) {
                loadFromCursor(cursor);
            } else {
                throw new RuntimeException("cannot find data for: " + path);
            }
            /// M: [FEATURE.ADD] @{
            synchronized (mMediaDataLock) {
                mMediaData = MediaDataParser.parseLocalVideoMediaData(this, cursor);
                mExtItem = PhotoPlayFacade.getMediaCenter().getItem(mMediaData);
            }
            /// @}
        } finally {
            cursor.close();
        }
    }

    private void loadFromCursor(Cursor cursor) {
        id = cursor.getInt(INDEX_ID);
        caption = cursor.getString(INDEX_CAPTION);
        mimeType = cursor.getString(INDEX_MIME_TYPE);
        latitude = cursor.getDouble(INDEX_LATITUDE);
        longitude = cursor.getDouble(INDEX_LONGITUDE);
        dateTakenInMs = cursor.getLong(INDEX_DATE_TAKEN);
        dateAddedInSec = cursor.getLong(INDEX_DATE_ADDED);
        dateModifiedInSec = cursor.getLong(INDEX_DATE_MODIFIED);
        filePath = cursor.getString(INDEX_DATA);
        durationInSec = cursor.getInt(INDEX_DURATION) / 1000;
        bucketId = cursor.getInt(INDEX_BUCKET_ID);
        fileSize = cursor.getLong(INDEX_SIZE);
        parseResolution(cursor.getString(INDEX_RESOLUTION));
    }

    private void parseResolution(String resolution) {
        if (resolution == null) return;
        int m = resolution.indexOf('x');
        if (m == -1) {
            m = resolution.indexOf("\u00d7");
        }
        if (m == -1) return;
        try {
            int w = Integer.parseInt(resolution.substring(0, m));
            int h = Integer.parseInt(resolution.substring(m + 1));
            width = w;
            height = h;
        } catch (Throwable t) {
            Log.w(TAG, t);
        }
    }

    @Override
    protected boolean updateFromCursor(Cursor cursor) {
        UpdateHelper uh = new UpdateHelper();
        id = uh.update(id, cursor.getInt(INDEX_ID));
        caption = uh.update(caption, cursor.getString(INDEX_CAPTION));
        mimeType = uh.update(mimeType, cursor.getString(INDEX_MIME_TYPE));
        latitude = uh.update(latitude, cursor.getDouble(INDEX_LATITUDE));
        longitude = uh.update(longitude, cursor.getDouble(INDEX_LONGITUDE));
        dateTakenInMs = uh.update(
                dateTakenInMs, cursor.getLong(INDEX_DATE_TAKEN));
        dateAddedInSec = uh.update(
                dateAddedInSec, cursor.getLong(INDEX_DATE_ADDED));
        dateModifiedInSec = uh.update(
                dateModifiedInSec, cursor.getLong(INDEX_DATE_MODIFIED));
        filePath = uh.update(filePath, cursor.getString(INDEX_DATA));
        durationInSec = uh.update(
                durationInSec, cursor.getInt(INDEX_DURATION) / 1000);
        bucketId = uh.update(bucketId, cursor.getInt(INDEX_BUCKET_ID));
        fileSize = uh.update(fileSize, cursor.getLong(INDEX_SIZE));
        /// M: [FEATURE.ADD] @{
        synchronized (mMediaDataLock) {
            mMediaData = MediaDataParser.parseLocalVideoMediaData(this, cursor);
            mExtItem = PhotoPlayFacade.getMediaCenter().getItem(mMediaData);
        }
        /// @}
        return uh.isUpdated();
    }

    @Override
    public Job<Bitmap> requestImage(int type) {
        /// M: [FEATURE.MODIFY] @{
        /*return new LocalVideoRequest(mApplication, getPath(), dateModifiedInSec,
         type, filePath);*/
        return new LocalVideoRequest(mApplication, getPath(), dateModifiedInSec, type,
                filePath, mExtItem);
        /// @}
    }

    public static class LocalVideoRequest extends ImageCacheRequest {
        private String mLocalFilePath;

        LocalVideoRequest(GalleryApp application, Path path, long timeModified,
                int type, String localFilePath) {
            /// M: [BUG.MODIFY] set thumbnail size to 640 if thumbnail is smaller than 640 @{
            /*
            super(application, path, timeModified, type,
                    MediaItem.getTargetSize(type));
            */
            super(application, path, timeModified, type,
            type == MediaItem.TYPE_THUMBNAIL
                    ? Math.max(MediaItem.getTargetSize(type), MediaItem.sThumbnailTargetSize)
                            : MediaItem.getTargetSize(type));
            /// @}
            mLocalFilePath = localFilePath;
        }

        /// M: [FEATURE.ADD] @{
        private ExtItem mData;
        LocalVideoRequest(GalleryApp application, Path path, long timeModified, int type,
                String localFilePath, ExtItem item) {
            super(application, path, timeModified, type, MediaItem.getTargetSize(type));
            mLocalFilePath = localFilePath;
            mData = item;
        }
        /// @}

        @Override
        public Bitmap onDecodeOriginal(JobContext jc, int type) {
            /// M: [DEBUG.ADD] @{
            TraceHelper.beginSection(">>>>LocalVideo-onDecodeOriginal");
            /// @}
            /// M: [FEATURE.ADD] @{
            Thumbnail thumb = mData.getThumbnail(FeatureHelper
                    .convertToThumbType(type));
            if (thumb != null && thumb.mNeedClearCache) {
                clearCache();
            }
            if (thumb != null && thumb.mBitmap != null) {
                return thumb.mBitmap;
            }
            if (thumb != null && thumb.mBitmap == null
                    && thumb.mStillNeedDecode == false) {
                return null;
            }
            /// @}
            /// M: [DEBUG.ADD] @{
            long logTimeBefore;
            logTimeBefore = System.currentTimeMillis();
            Log.d(TAG, "create video thumb begins at" + logTimeBefore);
            /// @}

            /// M: [PERF.ADD] Gallery boost for video decode @{
            PerfServiceUtils.boostEnableTimeoutMs(BOOST_VIDEO_DECODE_TIME_OUT);
            Log.d(TAG, "<boostEnableTimeoutMs> Video decode boost(ms): " +
                    BOOST_VIDEO_DECODE_TIME_OUT);
            /// @}
            Bitmap bitmap = BitmapUtils.createVideoThumbnail(mLocalFilePath);

            /// M: [DEBUG.ADD] @{
            Log.d(TAG, "create video thumb costs "
                    + (System.currentTimeMillis() - logTimeBefore));
            /// @}
            if (bitmap == null || jc.isCancelled()) return null;
            /// M: [DEBUG.ADD] @{
            TraceHelper.endSection();
            /// @}
            return bitmap;
        }
    }

    @Override
    public Job<BitmapRegionDecoder> requestLargeImage() {
        throw new UnsupportedOperationException("Cannot regquest a large image"
                + " to a local video!");
    }

    @Override
    public int getSupportedOperations() {
        /// M: [FEATURE.MODIFY] @{
        // return SUPPORT_DELETE | SUPPORT_SHARE | SUPPORT_PLAY
        // | SUPPORT_INFO | SUPPORT_TRIM | SUPPORT_MUTE
        int operation = SUPPORT_DELETE | SUPPORT_SHARE | SUPPORT_PLAY | SUPPORT_INFO | SUPPORT_TRIM
                | SUPPORT_MUTE;
        operation = FeatureHelper.mergeSupportOperations(operation,
                mExtItem.getSupportedOperations(),
                mExtItem.getNotSupportedOperations());
        return operation;
        /// @}
    }

    @Override
    public void delete() {
        GalleryUtils.assertNotInRenderThread();
        Uri baseUri = Video.Media.EXTERNAL_CONTENT_URI;
        mApplication.getContentResolver().delete(baseUri, "_id=?",
                new String[]{String.valueOf(id)});
        /// M: [BUG.ADD] @{
        mApplication.getDataManager().broadcastUpdatePicture();
        /// @}
    }

    @Override
    public void rotate(int degrees) {
        // TODO
    }

    @Override
    public Uri getContentUri() {
        Uri baseUri = Video.Media.EXTERNAL_CONTENT_URI;
        return baseUri.buildUpon().appendPath(String.valueOf(id)).build();
    }

    @Override
    public Uri getPlayUri() {
        return getContentUri();
    }

    @Override
    public int getMediaType() {
        return MEDIA_TYPE_VIDEO;
    }

    @Override
    public MediaDetails getDetails() {
        MediaDetails details = super.getDetails();
        int s = durationInSec;
        if (s > 0) {
            details.addDetail(MediaDetails.INDEX_DURATION, GalleryUtils.formatDuration(
                    mApplication.getAndroidContext(), durationInSec));
        }
        return details;
    }

    @Override
    public int getWidth() {
        return width;
    }

    @Override
    public int getHeight() {
        return height;
    }

    @Override
    public String getFilePath() {
        return filePath;
    }

    //********************************************************************
    //*                              MTK                                 *
    //********************************************************************

    /// M: [FEATURE.ADD] fancy layout @{
    private int mOrientation = ExtFieldsUtils.INVALID_VIDEO_ROTATION;

    public int getOrientation() {
        if (mOrientation == ExtFieldsUtils.INVALID_VIDEO_ROTATION) {
            mOrientation = ExtFieldsUtils.getVideoRotation(mMediaData);
        }
        Log.d(TAG, "<getOrientation> <Fancy> mOrientation " + mOrientation + ", " + getName());
        return mOrientation;
    }
    /// @}

    /// M: [FEATURE.ADD] @{
    private static String[] sExtProjection;

    /**
     * Get the projection after extended by features.
     * @return The projection after extended by features
     */
    public static String[] getProjection() {
        if (sExtProjection == null) {
            sExtProjection = ExtFields.getVideoProjection(PROJECTION);
        }
        return sExtProjection;
    }
    /// @}

    /// M: [PERF.ADD] Gallery boost for video decode @{
    private static final String BOOST_VIDEO_KEY = "debug.gallery.videoboost";
    private static final int BOOST_VIDEO_DECODE_TIME_OUT = SystemPropertyUtils.getInt(
            BOOST_VIDEO_KEY, FeatureHelper.BOOST_VIDEO_DECODE_TIME_OUT);
    /// @}
}
