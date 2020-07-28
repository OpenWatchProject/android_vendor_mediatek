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
import android.graphics.Bitmap.Config;
import android.graphics.BitmapFactory.Options;
import android.graphics.BitmapRegionDecoder;
import android.net.Uri;
import android.os.ParcelFileDescriptor;
import android.provider.MediaStore;

import com.android.gallery3d.app.GalleryApp;
import com.android.gallery3d.app.PanoramaMetadataSupport;
import com.android.gallery3d.common.BitmapUtils;
import com.android.gallery3d.common.Utils;
import com.android.gallery3d.util.ThreadPool.CancelListener;
import com.android.gallery3d.util.ThreadPool.Job;
import com.android.gallery3d.util.ThreadPool.JobContext;
import com.mediatek.gallery3d.adapter.FeatureHelper;
import com.mediatek.gallery3d.adapter.MediaDataParser;
import com.mediatek.gallery3d.adapter.PhotoPlayFacade;
import com.mediatek.gallery3d.util.FeatureConfig;
import com.mediatek.gallery3d.util.Log;
import com.mediatek.gallerybasic.base.ExtItem.Thumbnail;
import com.mediatek.gallerybasic.util.DecodeSpecLimitor;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.InputStream;
import java.net.URI;
import java.net.URL;

public class UriImage extends MediaItem {
    private static final String TAG = "Gallery2/UriImage";

    private static final int STATE_INIT = 0;
    private static final int STATE_DOWNLOADING = 1;
    private static final int STATE_DOWNLOADED = 2;
    private static final int STATE_ERROR = -1;

    private final Uri mUri;
    private final String mContentType;

    private DownloadCache.Entry mCacheEntry;
    private ParcelFileDescriptor mFileDescriptor;
    private int mState = STATE_INIT;
    private int mWidth;
    private int mHeight;
    private int mRotation;
    private PanoramaMetadataSupport mPanoramaMetadata = new PanoramaMetadataSupport(this);

    private GalleryApp mApplication;

    public UriImage(GalleryApp application, Path path, Uri uri, String contentType) {
        super(path, nextVersionNumber());
        mUri = uri;
        mApplication = Utils.checkNotNull(application);
        mContentType = contentType;
        /// M: [FEATURE.ADD] @{
        updateMediaData();
        // For Some type image(exp:DRM) should decode bound.
        if (mWidth <= 0 || mHeight <= 0) {
            int width = mExtItem.getWidth();
            int height = mExtItem.getHeight();
            if (width > 0 && height > 0) {
                mWidth = width;
                mHeight = height;
                updateMediaData();
            }
        }
        Log.d(TAG, "<UriImage> mUri " + mUri + " mContentType " + mContentType + " mWidth = "
                + mWidth + " mHeight = " + mHeight);
        /// @}
    }

    @Override
    public Job<Bitmap> requestImage(int type) {
        return new BitmapJob(type);
    }

    @Override
    public Job<BitmapRegionDecoder> requestLargeImage() {
        return new RegionDecoderJob();
    }

    private void openFileOrDownloadTempFile(JobContext jc) {
        int state = openOrDownloadInner(jc);
        synchronized (this) {
            mState = state;
            if (mState != STATE_DOWNLOADED) {
                if (mFileDescriptor != null) {
                    Utils.closeSilently(mFileDescriptor);
                    mFileDescriptor = null;
                }
            }
            notifyAll();
        }
    }

    private int openOrDownloadInner(JobContext jc) {
        String scheme = mUri.getScheme();
        if (ContentResolver.SCHEME_CONTENT.equals(scheme)
                || ContentResolver.SCHEME_ANDROID_RESOURCE.equals(scheme)
                || ContentResolver.SCHEME_FILE.equals(scheme)) {
            /// M: [FEATURE.ADD] <CTA Data Protection> @{
            if (mMediaData.filePath != null &&
                    mMediaData.filePath.toLowerCase().endsWith(".mudp")) {
                Log.d(TAG, "<openOrDownloadInner> return STATE_DOWNLOADED");
                return STATE_DOWNLOADED;
            }
            /// @}
            try {
                if (MIME_TYPE_JPEG.equalsIgnoreCase(mContentType)) {
                    InputStream is = mApplication.getContentResolver()
                            .openInputStream(mUri);
                    mRotation = Exif.getOrientation(is);
                    Utils.closeSilently(is);
                /// M: [BUG.ADD] read DNG EXIF details. @{
                } else if (MIME_TYPE_DNG.equalsIgnoreCase(mContentType)) {
                    InputStream is = mApplication.getContentResolver()
                            .openInputStream(mUri);
                    mRotation = FeatureHelper.getOrientationFromExif(mMediaData.filePath, is);
                    Utils.closeSilently(is);
                }
                /// @}
                mFileDescriptor = mApplication.getContentResolver()
                        .openFileDescriptor(mUri, "r");
                if (jc.isCancelled()) return STATE_INIT;
                return STATE_DOWNLOADED;
            } catch (FileNotFoundException e) {
                Log.w(TAG, "fail to open: " + mUri, e);
                return STATE_ERROR;
            }
        } else {
            try {
                /// M: [BUG.ADD] disable setAs Action for URL image. @{
                mIsUrlImage = true;
                /// @}
                URL url = new URI(mUri.toString()).toURL();
                /// M: [BUG.MODIFY] @{
                /* mCacheEntry = mApplication.getDownloadCache().download(jc, url); */
                DownloadCache downloadCache = mApplication.getDownloadCache();
                if (downloadCache == null) {
                    Log.w(TAG, "<openOrDownloadInner> failed to get DownloadCache");
                    return STATE_ERROR;
                }
                mCacheEntry = downloadCache.download(jc, url);
                /// @}
                if (jc.isCancelled()) return STATE_INIT;
                if (mCacheEntry == null) {
                    Log.w(TAG, "download failed " + url);
                    return STATE_ERROR;
                }
                if (MIME_TYPE_JPEG.equalsIgnoreCase(mContentType)) {
                    InputStream is = new FileInputStream(mCacheEntry.cacheFile);
                    mRotation = Exif.getOrientation(is);
                    Utils.closeSilently(is);
                }
                mFileDescriptor = ParcelFileDescriptor.open(
                        mCacheEntry.cacheFile, ParcelFileDescriptor.MODE_READ_ONLY);
                return STATE_DOWNLOADED;
            } catch (Throwable t) {
                Log.w(TAG, "download error", t);
                return STATE_ERROR;
            }
        }
    }

    private boolean prepareInputFile(JobContext jc) {
        jc.setCancelListener(new CancelListener() {
            @Override
            public void onCancel() {
                synchronized (this) {
                    notifyAll();
                }
            }
        });

        while (true) {
            synchronized (this) {
                if (jc.isCancelled()) return false;
                if (mState == STATE_INIT) {
                    mState = STATE_DOWNLOADING;
                    // Then leave the synchronized block and continue.
                } else if (mState == STATE_ERROR) {
                    return false;
                } else if (mState == STATE_DOWNLOADED) {
                    return true;
                } else /* if (mState == STATE_DOWNLOADING) */ {
                    try {
                        wait();
                    } catch (InterruptedException ex) {
                        // ignored.
                    }
                    continue;
                }
            }
            // This is only reached for STATE_INIT->STATE_DOWNLOADING
            openFileOrDownloadTempFile(jc);
        }
    }

    private class RegionDecoderJob implements Job<BitmapRegionDecoder> {
        @Override
        public BitmapRegionDecoder run(JobContext jc) {
            if (!prepareInputFile(jc)) return null;
            /// M: [BUG.ADD] check decode spec @{
            if (mMediaData != null
                    && DecodeSpecLimitor.isOutOfSpecLimit(getUriImageFileSize(mUri),
                            mMediaData.width, mMediaData.height, mContentType)) {
                releaseInputFile();
                Log.d(TAG, "<RegionDecoderJob.run> out of spec limit, abort decoding!");
                return null;
            }
            /// @}
            BitmapRegionDecoder decoder = DecodeUtils.createBitmapRegionDecoder(
                    jc, mFileDescriptor.getFileDescriptor(), false);
            if (decoder == null) {
                /// M: [BUG.ADD] Release input file before return @{
                releaseInputFile();
                /// @}
                return null;
            }
            int width = decoder.getWidth();
            int height = decoder.getHeight();
            /// M: [FEATURE.ADD] @{
            if (width > 0 && height > 0 && width != mWidth && height != mHeight) {
                mWidth = width;
                mHeight = height;
                updateMediaData();
            }

            /// @}
            /// M: [BUG.ADD] Release input file before return @{
            releaseInputFile();
            /// @}
            return decoder;
        }
    }

    private class BitmapJob implements Job<Bitmap> {
        private int mType;

        protected BitmapJob(int type) {
            mType = type;
        }

        @Override
        public Bitmap run(JobContext jc) {
            if (!prepareInputFile(jc)) return null;
            /// M: [BUG.ADD] check decode spec @{
            if (mMediaData != null
                    && DecodeSpecLimitor.isOutOfSpecLimit(getUriImageFileSize(mUri),
                            mMediaData.width, mMediaData.height, mContentType)) {
                Log.d(TAG, "<BitmapJob.run> out of spec limit, abort decoding!");
                releaseInputFile();
                return null;
            }
            /// @}
            /// M: [BUG.ADD] @{
            // for images which not support RegionDecode,
            // get width and height from decodeBounds.
            // Decode bound for Drm image.
            int width;
            int height;
            Options op = new Options();
            if (mFileDescriptor != null) {
                DecodeUtils.decodeBounds(jc, mFileDescriptor.getFileDescriptor(), op);
                width = op.outWidth;
                height = op.outHeight;
            } else {
                width = mExtItem.getWidth();
                height = mExtItem.getHeight();
            }
            if (width > 0 && height > 0 && width != mWidth && height != mHeight) {
                mWidth = width;
                mHeight = height;
                updateMediaData();
            }
            /// @}
            /// M: [FEATURE.ADD] @{
            Thumbnail thumb = mExtItem.getThumbnail(FeatureHelper.convertToThumbType(mType));
            if (thumb != null && thumb.mBitmap != null) {
                releaseInputFile();
                return thumb.mBitmap;
            }
            if (thumb != null && thumb.mBitmap == null && thumb.mStillNeedDecode == false) {
                releaseInputFile();
                return null;
            }
            /// @}
            int targetSize = MediaItem.getTargetSize(mType);
            Options options = new Options();
            options.inPreferredConfig = Config.ARGB_8888;
            Bitmap bitmap = DecodeUtils.decodeThumbnail(jc,
                    mFileDescriptor.getFileDescriptor(), options, targetSize, mType);

            if (jc.isCancelled() || bitmap == null) {
                /// M: [BUG.ADD] Release input file before return @{
                releaseInputFile();
                /// @}
                return null;
            }

            if (mType == MediaItem.TYPE_MICROTHUMBNAIL) {
                bitmap = BitmapUtils.resizeAndCropCenter(bitmap, targetSize, true);
            } else {
                bitmap = BitmapUtils.resizeDownBySideLength(bitmap, targetSize, true);
            }

            /// M: [BUG.ADD] @{
            // Some png bitmaps have transparent areas, so clear alpha value
            bitmap =
                    com.mediatek.gallerybasic.util.BitmapUtils.clearAlphaValueIfPng(bitmap,
                            mMediaData.mimeType, true);
            /// @}

            /// M: [BUG.ADD] Release input file before return @{
            releaseInputFile();
            /// @}
            return bitmap;
        }
    }

    @Override
    public int getSupportedOperations() {
        /// M: [BUG.MODIFY] disable setAs Action for URL image. @{
        // int supported = SUPPORT_PRINT | SUPPORT_SETAS;
        int supported = SUPPORT_PRINT;
        if (!mIsUrlImage) {
            supported |= SUPPORT_SETAS;
        }
        /// @}
        if (isSharable()) supported |= SUPPORT_SHARE;
        if (BitmapUtils.isSupportedByRegionDecoder(mContentType)) {
            supported |= SUPPORT_EDIT | SUPPORT_FULL_IMAGE;
            /// M: [BUG.ADD] @{
            float scale = (float) sThumbnailTargetSize / Math.max(mWidth, mHeight);
            int thumbWidth = (int) (mWidth * scale);
            if (Math.max(0, Utils.ceilLog2((float) mWidth / thumbWidth)) == 0
                    || (FeatureConfig.sIsLowRamDevice
                            && mWidth * mHeight > REGION_DECODER_PICTURE_SIZE_LIMIT)) {
                // 1. if current item is not bigger than thumbnail size,
                // don't support full image display
                // 2. use high-quality screennail instead of region decoder
                // for extremely large image
                Log.d(TAG, "<getSupportedOperations> item thumbWidth " + thumbWidth
                        + " scale " + scale);
                Log.d(TAG, "<getSupportedOperations> item not support full image, mWidth "
                        + mWidth + " sthumbnailsize " + sThumbnailTargetSize);
                Log.d(TAG, "<getSupportedOperations> sIsLowRamDevice "
                        + FeatureConfig.sIsLowRamDevice + ", mWidth * mHeight is "
                        + mWidth * mHeight);
                supported &= ~SUPPORT_FULL_IMAGE;
            }
            /// @}
        }

        /// M: [FEATURE.ADD] @{
        supported = FeatureHelper.mergeSupportOperations(supported,
                mExtItem.getSupportedOperations(),
                mExtItem.getNotSupportedOperations());
        /// @}

        /// M: [BUG.ADD] From N, Google using file uri in app internal,
        // and using content uri between apps. So UriImage should not support share,
        // setAs and print for file uri.@{
        if (ContentResolver.SCHEME_FILE.equals(mUri.getScheme())) {
            supported &= ~SUPPORT_PRINT;
            supported &= ~SUPPORT_SETAS;
            supported &= ~SUPPORT_SHARE;
        }
        /// @}
        return supported;
    }

    @Override
    public void getPanoramaSupport(PanoramaSupportCallback callback) {
        mPanoramaMetadata.getPanoramaSupport(mApplication, callback);
    }

    @Override
    public void clearCachedPanoramaSupport() {
        mPanoramaMetadata.clearCachedValues();
    }

    private boolean isSharable() {
        // We cannot grant read permission to the receiver since we put
        // the data URI in EXTRA_STREAM instead of the data part of an intent
        // And there are issues in MediaUploader and Bluetooth file sender to
        // share a general image data. So, we only share for local file.
        return ContentResolver.SCHEME_FILE.equals(mUri.getScheme());
    }

    @Override
    public int getMediaType() {
        return MEDIA_TYPE_IMAGE;
    }

    @Override
    public Uri getContentUri() {
        return mUri;
    }

    @Override
    public MediaDetails getDetails() {
        MediaDetails details = super.getDetails();
        if (mWidth != 0 && mHeight != 0) {
            details.addDetail(MediaDetails.INDEX_WIDTH, mWidth);
            details.addDetail(MediaDetails.INDEX_HEIGHT, mHeight);
        }
        if (mContentType != null) {
            details.addDetail(MediaDetails.INDEX_MIMETYPE, mContentType);
        }
        if (ContentResolver.SCHEME_FILE.equals(mUri.getScheme())) {
            String filePath = mUri.getPath();
            details.addDetail(MediaDetails.INDEX_PATH, filePath);
            MediaDetails.extractExifInfo(details, filePath);
        }
        return details;
    }

    @Override
    public String getMimeType() {
        return mContentType;
    }

    @Override
    protected void finalize() throws Throwable {
        try {
            if (mFileDescriptor != null) {
                Utils.closeSilently(mFileDescriptor);
            }
        } finally {
            super.finalize();
        }
    }

    @Override
    public int getWidth() {
        /// M: [FEATURE.MARK] @{
        /* return 0; */
        return mWidth;
        /// @}
    }

    @Override
    public int getHeight() {
        /// M: [FEATURE.MARK] @{
        /* return 0; */
        return mHeight;
        /// @}
    }

    @Override
    public int getRotation() {
        return mRotation;
    }

    //********************************************************************
    //*                              MTK                                 *
    //********************************************************************

    public void updateMediaData() {
        synchronized (mMediaDataLock) {
            mMediaData =
                    MediaDataParser.parseUriImageMediaData(this);
            mExtItem = PhotoPlayFacade.getMediaCenter().getItem(mMediaData);
        }
    }

    private long getUriImageFileSize(Uri uri) {
        if (!FeatureHelper.isLocalUri(uri)) {
            return 0;
        }
        String[] proj = { MediaStore.Video.Media.DATA };
        Cursor cursor = null;
        long fileSize = 0;
        String nameFromURI = null;
        try {
            cursor = mApplication.getContentResolver().query(uri, proj, null,
                    null, null);
            if (cursor == null) {
                return 0;
            }
            int colummIndex = cursor
                    .getColumnIndexOrThrow(MediaStore.Video.Media.DATA);
            cursor.moveToFirst();
            nameFromURI = cursor.getString(colummIndex);
        } catch (IllegalArgumentException e) {
            Log.e(TAG, "<getUriImageFileSize> Exception", e);
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        if (nameFromURI != null) {
            File file = new File(nameFromURI);
            if (file.exists()) {
                fileSize = file.length();
            }
        }
        return fileSize;
    }

    // disable setAs Action for URL image
    private boolean mIsUrlImage = false;

    // After open the uri image file which locates on the storage that can eject dynamically,
    // gallery process can not close FileDescriptor immediately, it will extend the
    // time in ejecting status, and gallery process will be killed forced.
    // So release input file immediately after decode.
    private synchronized void releaseInputFile() {
        if (mFileDescriptor != null) {
            Utils.closeSilently(mFileDescriptor);
            mFileDescriptor = null;
        }
        mState = STATE_INIT;
    }
}
