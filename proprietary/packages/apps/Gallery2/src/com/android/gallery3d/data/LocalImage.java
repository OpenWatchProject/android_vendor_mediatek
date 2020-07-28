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

import android.annotation.TargetApi;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.BitmapRegionDecoder;
import android.net.Uri;
import android.os.Build;
import android.provider.MediaStore.Images;
import android.provider.MediaStore.Images.ImageColumns;
import android.provider.MediaStore.MediaColumns;

import com.android.gallery3d.app.GalleryApp;
import com.android.gallery3d.app.PanoramaMetadataSupport;
import com.android.gallery3d.common.ApiHelper;
import com.android.gallery3d.common.BitmapUtils;
import com.android.gallery3d.common.Utils;
import com.android.gallery3d.exif.ExifInterface;
import com.android.gallery3d.exif.ExifTag;
import com.android.gallery3d.filtershow.tools.SaveImage;
import com.android.gallery3d.util.GalleryUtils;
import com.android.gallery3d.util.ThreadPool.Job;
import com.android.gallery3d.util.ThreadPool.JobContext;
import com.android.gallery3d.util.UpdateHelper;
import com.mediatek.gallery3d.adapter.FeatureHelper;
import com.mediatek.gallery3d.adapter.MediaDataParser;
import com.mediatek.gallery3d.adapter.PhotoPlayFacade;
import com.mediatek.gallery3d.layout.FancyHelper;
import com.mediatek.gallery3d.util.FeatureConfig;
import com.mediatek.gallery3d.util.Log;
import com.mediatek.gallerybasic.base.ExtFields;
import com.mediatek.gallerybasic.base.ExtItem;
import com.mediatek.gallerybasic.base.ExtItem.Thumbnail;
import com.mediatek.gallerybasic.base.MediaData;
import com.mediatek.gallerybasic.util.DecodeSpecLimitor;
import com.mediatek.galleryportable.TraceHelper;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;

// LocalImage represents an image in the local storage.
public class LocalImage extends LocalMediaItem {
    private static final String TAG = "Gallery2/LocalImage";

    static final Path ITEM_PATH = Path.fromString("/local/image/item");

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
     private static final int INDEX_ORIENTATION = 9;
     private static final int INDEX_BUCKET_ID = 10;
     private static final int INDEX_SIZE = 11;
     private static final int INDEX_WIDTH = 12;
     private static final int INDEX_HEIGHT = 13;*/
    public static final int INDEX_ID = 0;
    public static final int INDEX_CAPTION = 1;
    public static final int INDEX_MIME_TYPE = 2;
    public static final int INDEX_LATITUDE = 3;
    public static final int INDEX_LONGITUDE = 4;
    public static final int INDEX_DATE_TAKEN = 5;
    public static final int INDEX_DATE_ADDED = 6;
    public static final int INDEX_DATE_MODIFIED = 7;
    public static final int INDEX_DATA = 8;
    public static final int INDEX_ORIENTATION = 9;
    public static final int INDEX_BUCKET_ID = 10;
    public static final int INDEX_SIZE = 11;
    public static final int INDEX_WIDTH = 12;
    public static final int INDEX_HEIGHT = 13;
    /// @}

    static final String[] PROJECTION =  {
            ImageColumns._ID,           // 0
            ImageColumns.TITLE,         // 1
            ImageColumns.MIME_TYPE,     // 2
            ImageColumns.LATITUDE,      // 3
            ImageColumns.LONGITUDE,     // 4
            ImageColumns.DATE_TAKEN,    // 5
            ImageColumns.DATE_ADDED,    // 6
            ImageColumns.DATE_MODIFIED, // 7
            ImageColumns.DATA,          // 8
            ImageColumns.ORIENTATION,   // 9
            ImageColumns.BUCKET_ID,     // 10
            ImageColumns.SIZE,          // 11
            "0",                        // 12
            "0",                         // 13
    };

    static {
        updateWidthAndHeightProjection();
    }

    @TargetApi(Build.VERSION_CODES.JELLY_BEAN)
    private static void updateWidthAndHeightProjection() {
        if (ApiHelper.HAS_MEDIA_COLUMNS_WIDTH_AND_HEIGHT) {
            PROJECTION[INDEX_WIDTH] = MediaColumns.WIDTH;
            PROJECTION[INDEX_HEIGHT] = MediaColumns.HEIGHT;
        }
    }

    private final GalleryApp mApplication;

    public int rotation;

    private PanoramaMetadataSupport mPanoramaMetadata = new PanoramaMetadataSupport(this);

    public LocalImage(Path path, GalleryApp application, Cursor cursor) {
        super(path, nextVersionNumber());
        mApplication = application;
        loadFromCursor(cursor);
        /// M: [FEATURE.ADD] @{
        updateMediaData(cursor);
        /// @}
    }

    public LocalImage(Path path, GalleryApp application, int id) {
        super(path, nextVersionNumber());
        mApplication = application;
        ContentResolver resolver = mApplication.getContentResolver();
        Uri uri = Images.Media.EXTERNAL_CONTENT_URI;
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
            updateMediaData(cursor);
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
        rotation = cursor.getInt(INDEX_ORIENTATION);
        bucketId = cursor.getInt(INDEX_BUCKET_ID);
        fileSize = cursor.getLong(INDEX_SIZE);
        width = cursor.getInt(INDEX_WIDTH);
        height = cursor.getInt(INDEX_HEIGHT);
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
        rotation = uh.update(rotation, cursor.getInt(INDEX_ORIENTATION));
        bucketId = uh.update(bucketId, cursor.getInt(INDEX_BUCKET_ID));
        fileSize = uh.update(fileSize, cursor.getLong(INDEX_SIZE));
        width = uh.update(width, cursor.getInt(INDEX_WIDTH));
        height = uh.update(height, cursor.getInt(INDEX_HEIGHT));
        /// M: [FEATURE.MODIFY] @{
        /*return uh.isUpdated();*/
        return updateMediaData(cursor) || uh.isUpdated();
        /// @}
    }

    @Override
    public Job<Bitmap> requestImage(int type) {
        /// M: [FEATURE.MODIFY] @{
        /*return new LocalImageRequest(mApplication, mPath, dateModifiedInSec,
         type, filePath);*/
        return new LocalImageRequest(mApplication, mPath, dateModifiedInSec, type, filePath,
                mExtItem, mMediaData);
        /// @}
    }

    public static class LocalImageRequest extends ImageCacheRequest {
        private String mLocalFilePath;

        LocalImageRequest(GalleryApp application, Path path, long timeModified,
                int type, String localFilePath) {
            super(application, path, timeModified, type,
                    MediaItem.getTargetSize(type));
            mLocalFilePath = localFilePath;
        }

        /// M: [FEATURE.ADD] @{
        private ExtItem mData;
        private MediaData mMediaData;
        private boolean mIsCameraRollCover;
        private boolean mIsScreenShotCover;

        LocalImageRequest(GalleryApp application, Path path, long timeModified,
                int type, String localFilePath, ExtItem data,
                MediaData mediaData) {
            super(application, path, timeModified, type, MediaItem.getTargetSize(type));
            mLocalFilePath = localFilePath;
            mData = data;
            mMediaData = mediaData;
            mIsCameraRollCover = isCameraRollCover(application, mMediaData, path);
            mIsScreenShotCover = isScreenShotCover(application, mMediaData, path);
        }
        /// @}

        @Override
        public Bitmap onDecodeOriginal(JobContext jc, final int type) {
            /// M: [FEATURE.ADD] @{
            if (mData != null) {
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
            } else {
                Log.d(TAG,
                        "<onDecodeOriginal> error status, ExtItem is null, localFilePath = "
                                + mLocalFilePath);
            }
            /// @}
            /// M: [BUG.ADD] check decode spec
            if (mMediaData != null && DecodeSpecLimitor.isOutOfSpecLimit(mMediaData.fileSize,
                    mMediaData.width, mMediaData.height, mMediaData.mimeType)) {
                Log.d(TAG, "<LocalImageRequest.onDecodeOriginal> path "
                        + mLocalFilePath
                        + ", out of spec limit, abort decoding!");
                return null;
            }
            /// @}
            /// M: [DEBUG.ADD] @{
            Log.d(TAG, "<LocalImageRequest.onDecodeOriginal> onDecodeOriginal,type:" + type);
            TraceHelper.beginSection(">>>>LocalImage-onDecodeOriginal");
            /// @}
            BitmapFactory.Options options = new BitmapFactory.Options();
            options.inPreferredConfig = Bitmap.Config.ARGB_8888;
            int targetSize = MediaItem.getTargetSize(type);

            /// M: [BUG.MARK] decode thread may be blocking if exif data is broken. @{
            /*
            // try to decode from JPEG EXIF
            if (type == MediaItem.TYPE_MICROTHUMBNAIL) {
                ExifInterface exif = new ExifInterface();
                byte[] thumbData = null;
                try {
                    exif.readExif(mLocalFilePath);
                    thumbData = exif.getThumbnail();
                } catch (FileNotFoundException e) {
                    Log.w(TAG, "failed to find file to read thumbnail: " + mLocalFilePath);
                } catch (IOException e) {
                    Log.w(TAG, "failed to get thumbnail from: " + mLocalFilePath);
                }
                if (thumbData != null) {
                    Bitmap bitmap = DecodeUtils.decodeIfBigEnough(
                            jc, thumbData, options, targetSize);
                    if (bitmap != null) return bitmap;
                }
            }
            */
            /// @}

            /// M: [DEBUG.ADD] @{
            TraceHelper.beginSection(
                    ">>>>LocalImage-onDecodeOriginal-decodeThumbnail");
            Bitmap res = null;
            /// @}
            /// M: [FEATURE.MODIFY] fancy layout @{
            // return DecodeUtils.decodeThumbnail(jc, mLocalFilePath, options, targetSize, type);
            if (type == MediaItem.TYPE_FANCYTHUMBNAIL) {
                if (mIsCameraRollCover || mIsScreenShotCover) {
                    targetSize = FancyHelper.getScreenWidthAtFancyMode();
                    Log.d(TAG, "<onDecodeOri> "
                            + "mIsCameraRollCover or mIsScreenShotCover, targetSize " + targetSize);
                }
                res = FancyHelper.decodeThumbnail(jc, mLocalFilePath, options, targetSize, type);
            } else {
                if (type == MediaItem.TYPE_HIGHQUALITYTHUMBNAIL && mMediaData != null) {
                    processOptions(mMediaData.mimeType, options);
                }
                res = DecodeUtils.decodeThumbnail(jc, mLocalFilePath, options, targetSize, type);
            }
            /// @}
            /// M: [BUG.ADD] @{
            // Some png bitmaps have transparent areas, so clear alpha value
            if (mMediaData != null) {
                res = com.mediatek.gallerybasic.util.BitmapUtils.clearAlphaValueIfPng(res,
                        mMediaData.mimeType, true);
            }
            /// @}
            /// M: [DEBUG.ADD] @{
            TraceHelper.endSection();
            TraceHelper.endSection();
            Log.d(TAG, "<LocalImageRequest.onDecodeOriginal> finish, return bitmap: " + res);
            return res;
            /// @}
        }
    }

    @Override
    public Job<BitmapRegionDecoder> requestLargeImage() {
        return new LocalLargeImageRequest(filePath);
    }

    public static class LocalLargeImageRequest
            implements Job<BitmapRegionDecoder> {
        String mLocalFilePath;

        public LocalLargeImageRequest(String localFilePath) {
            mLocalFilePath = localFilePath;
        }

        @Override
        public BitmapRegionDecoder run(JobContext jc) {
            return DecodeUtils.createBitmapRegionDecoder(jc, mLocalFilePath, false);
        }
    }

    @Override
    public int getSupportedOperations() {
        int operation = SUPPORT_DELETE | SUPPORT_SHARE | SUPPORT_CROP
                | SUPPORT_SETAS | SUPPORT_PRINT | SUPPORT_INFO;
        if (BitmapUtils.isSupportedByRegionDecoder(mimeType)) {
            operation |= SUPPORT_FULL_IMAGE | SUPPORT_EDIT;
            /// M: [BUG.ADD] @{
            float scale = (float) sThumbnailTargetSize / Math.max(width, height);
            int thumbWidth = (int) (width * scale);
            if (Math.max(0, Utils.ceilLog2((float) width / thumbWidth)) == 0
                    || (FeatureConfig.sIsLowRamDevice
                            && width * height > REGION_DECODER_PICTURE_SIZE_LIMIT)) {
                // 1. if current item is not bigger than thumbnail size,
                // don't support full image display
                // 2. use high-quality screennail instead of region decoder
                // for extremely large image
                Log.d(TAG, "<getSupportedOperations> item thumbWidth " + thumbWidth +
                        " scale " + scale);
                Log.d(TAG, "<getSupportedOperations> item not support full image, width " + width +
                        " sthumbnailsize " + sThumbnailTargetSize);
                Log.d(TAG, "<getSupportedOperations> sIsLowRamDevice "
                        + FeatureConfig.sIsLowRamDevice + ", width * height is "
                        + width * height);
                operation &= ~SUPPORT_FULL_IMAGE;
            }
            /// @}
        }

        if (BitmapUtils.isRotationSupported(mimeType)) {
            operation |= SUPPORT_ROTATE;
        }

        if (GalleryUtils.isValidLocation(latitude, longitude)) {
            operation |= SUPPORT_SHOW_ON_MAP;
        }
        /// M: [FEATURE.ADD] @{
        operation = FeatureHelper.mergeSupportOperations(operation,
                mExtItem.getSupportedOperations(),
                mExtItem.getNotSupportedOperations());
        /// @}
        return operation;
    }

    @Override
    public void getPanoramaSupport(PanoramaSupportCallback callback) {
        mPanoramaMetadata.getPanoramaSupport(mApplication, callback);
    }

    @Override
    public void clearCachedPanoramaSupport() {
        mPanoramaMetadata.clearCachedValues();
    }

    @Override
    public void delete() {
        GalleryUtils.assertNotInRenderThread();
        /// M: [FEATURE.ADD] @{
        mExtItem.delete();
        /// @}
        Uri baseUri = Images.Media.EXTERNAL_CONTENT_URI;
        ContentResolver contentResolver = mApplication.getContentResolver();
        SaveImage.deleteAuxFiles(contentResolver, getContentUri());
        contentResolver.delete(baseUri, "_id=?",
                new String[]{String.valueOf(id)});
        /// M: [BUG.ADD] @{
        mApplication.getDataManager().broadcastUpdatePicture();
        /// @}
    }

    @Override
    public void rotate(int degrees) {
        GalleryUtils.assertNotInRenderThread();
        Uri baseUri = Images.Media.EXTERNAL_CONTENT_URI;
        ContentValues values = new ContentValues();
        int rotation = (this.rotation + degrees) % 360;
        if (rotation < 0) rotation += 360;

        if (mimeType.equalsIgnoreCase("image/jpeg")) {
            ExifInterface exifInterface = new ExifInterface();
            ExifTag tag = exifInterface.buildTag(ExifInterface.TAG_ORIENTATION,
                    ExifInterface.getOrientationValueForRotation(rotation));
            if(tag != null) {
                exifInterface.setTag(tag);
                try {
                    exifInterface.forceRewriteExif(filePath);
                    fileSize = new File(filePath).length();
                    values.put(Images.Media.SIZE, fileSize);
                } catch (FileNotFoundException e) {
                    Log.w(TAG, "cannot find file to set exif: " + filePath);
                } catch (IOException e) {
                    Log.w(TAG, "cannot set exif data: " + filePath);

                    /// M: [BUG.ADD] add Exif header if jpeg image has no Exif header @{
                    try {
                        // suffixCurTime used for the suffix of temporal file
                        String suffixCurTime = Long.toString(System.currentTimeMillis());
                        String newFilePath = filePath + "." + suffixCurTime;

                        exifInterface.writeExif(filePath, newFilePath);
                        File tempFile = new File(newFilePath);
                        Log.d(TAG, "Temporal file's name: " + tempFile.getName());
                        File file = new File(filePath);
                        tempFile.renameTo(file);

                        // write filesize to database
                        values.put(Images.Media.SIZE, file.length());
                    } catch (FileNotFoundException e2) {
                        Log.w(TAG,
                                "cannot find file which has not Exif header: " + filePath);
                    } catch (IOException e2) {
                        Log.w(TAG, "cannot set exif: " + filePath);
                    }
                    /// @}
                }
            } else {
                Log.w(TAG, "Could not build tag: " + ExifInterface.TAG_ORIENTATION);
            }
        }

        values.put(Images.Media.ORIENTATION, rotation);
        /// M: [FEATURE.ADD] fancy layout @{
        if (FancyHelper.isFancyLayoutSupported()) {
            mApplication.getImageCacheService().clearImageData(mPath,
                    dateModifiedInSec, MediaItem.TYPE_FANCYTHUMBNAIL);
            Log.d(TAG, "<rotate> <Fancy> clear FANCYTHUMBNAIL" + mPath);
        }
        /// @}
        mApplication.getContentResolver().update(baseUri, values, "_id=?",
                new String[]{String.valueOf(id)});
    }

    @Override
    public Uri getContentUri() {
        Uri baseUri = Images.Media.EXTERNAL_CONTENT_URI;
        return baseUri.buildUpon().appendPath(String.valueOf(id)).build();
    }

    @Override
    public int getMediaType() {
        return MEDIA_TYPE_IMAGE;
    }

    @Override
    public MediaDetails getDetails() {
        /// M: [FEATURE.ADD] @{
        String[] detailStr = mExtItem.getDetails();
        MediaDetails detailEx = FeatureHelper
                .convertStringArrayToDetails(detailStr);
        if (detailEx != null) {
            return detailEx;
        }
        /// @}
        MediaDetails details = super.getDetails();
        details.addDetail(MediaDetails.INDEX_ORIENTATION, Integer.valueOf(rotation));
        if (MIME_TYPE_JPEG.equals(mimeType)) {
            // ExifInterface returns incorrect values for photos in other format.
            // For example, the width and height of an webp images is always '0'.
            MediaDetails.extractExifInfo(details, filePath);
        /// M: [BUG.ADD] read DNG EXIF details. @{
        } else if (MIME_TYPE_DNG.equalsIgnoreCase(mimeType)) {
            MediaDetails.extractDNGExifInfo(details, filePath);
        }
        /// @}
        return details;
    }

    @Override
    public int getRotation() {
        return rotation;
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

    /**
     * Create new MediaData from Cursor and replace old one.
     * @param cursor
     * @return if current MediaData.mediaType has changed or not
     */
    private boolean updateMediaData(Cursor cursor) {
        MediaData oldMediaData = mMediaData;
        if (cursor == null) {
            Log.d(TAG, "<updateMediaData> other, cursor is null, return", new Throwable());
            return false;
        }
        synchronized (mMediaDataLock) {
            mMediaData = MediaDataParser.parseLocalImageMediaData(cursor);
            mExtItem = PhotoPlayFacade.getMediaCenter().getItem(mMediaData);
        }
        // if mediaType has changed, return true, and delete its cache image data
        if (oldMediaData != null
                && !oldMediaData.mediaType.equals(mMediaData.mediaType)) {
            mApplication.getImageCacheService().clearImageData(mPath,
                    dateModifiedInSec, MediaItem.TYPE_MICROTHUMBNAIL);
            return true;
        }
        return false;
    }

    /// M: [FEATURE.ADD] @{
    private static String[] sExtProjection;

    /**
     * Get the projection after extended by features.
     * @return The projection after extended by features
     */
    public static String[] getProjection() {
        if (sExtProjection == null) {
            sExtProjection = ExtFields.getImageProjection(PROJECTION);
        }
        return sExtProjection;
    }
    /// @}
}
