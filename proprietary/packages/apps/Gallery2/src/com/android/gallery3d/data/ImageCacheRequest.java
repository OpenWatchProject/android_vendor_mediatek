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

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;

import com.android.gallery3d.R;
import com.android.gallery3d.app.GalleryApp;
import com.android.gallery3d.common.BitmapUtils;
import com.android.gallery3d.data.BytesBufferPool.BytesBuffer;
import com.android.gallery3d.util.ThreadPool.Job;
import com.android.gallery3d.util.ThreadPool.JobContext;
import com.mediatek.gallery3d.adapter.FeatureHelper;
import com.mediatek.gallery3d.adapter.FeatureManager;
import com.mediatek.gallery3d.layout.FancyHelper;
import com.mediatek.gallery3d.util.Log;
import com.mediatek.gallerybasic.base.ExtItem;
import com.mediatek.gallerybasic.base.IDecodeOptionsProcessor;
import com.mediatek.gallerybasic.base.MediaData;
import com.mediatek.gallerybasic.base.ThumbType;
import com.mediatek.gallerybasic.util.DebugUtils;
import com.mediatek.gallerybasic.util.ExtFieldsUtils;
import com.mediatek.galleryportable.TraceHelper;

abstract class ImageCacheRequest implements Job<Bitmap> {
    private static final String TAG = "Gallery2/ImageCacheRequest";

    protected GalleryApp mApplication;
    private Path mPath;
    private int mType;
    private int mTargetSize;
    private long mTimeModified;

    public ImageCacheRequest(GalleryApp application,
            Path path, long timeModified, int type, int targetSize) {
        mApplication = application;
        mPath = path;
        mType = type;
        mTargetSize = targetSize;
        mTimeModified = timeModified;
    }

    private String debugTag() {
        return mPath + "," + mTimeModified + "," +
                ((mType == MediaItem.TYPE_THUMBNAIL) ? "THUMB" :
                (mType == MediaItem.TYPE_MICROTHUMBNAIL) ? "MICROTHUMB" : "?");
    }

    @Override
    public Bitmap run(JobContext jc) {
        ImageCacheService cacheService = mApplication.getImageCacheService();

        /// M: [FEATURE.ADD] @{
        // Some special items do not want to cache thumbnail,
        // or do not want get thumbnail from Gallery cache
        boolean needToReadCache = true;
        boolean needToWriteCache = true;
        ExtItem extItem = null;
        String mimeType = null;
        if (mPath.getObject() instanceof MediaItem) {
            MediaItem item = ((MediaItem) mPath.getObject());
            if (item != null) {
                extItem = item.getExtItem();
                mimeType = item.getMimeType();
                ThumbType thumbType = FeatureHelper.convertToThumbType(mType);
                needToReadCache = (mType != MediaItem.TYPE_HIGHQUALITYTHUMBNAIL && extItem
                        .isNeedToGetThumbFromCache(thumbType));
                needToWriteCache = (mType != MediaItem.TYPE_HIGHQUALITYTHUMBNAIL && extItem
                        .isNeedToCacheThumb(thumbType));
            }
        }
        if (extItem == null) {
            Log.w(TAG, "extItem is null, maybe because the MediaObject has recycled by GC");
        }

        if (needToReadCache) {
        /// @}
            BytesBuffer buffer = MediaItem.getBytesBufferPool().get();
            try {
                /// M: [DEBUG.ADD] @{
                TraceHelper.beginSection(">>>>ImageCacheRequest-getImageData");
                /// @}
                boolean found = cacheService.getImageData(mPath, mTimeModified, mType, buffer);
                /// M: [DEBUG.ADD] @{
                TraceHelper.endSection();
                // @}
                /// M: [BUG.ADD] if cache image out of data, re-decode it @{
                if ((ImageCacheService.sForceObsoletePath != null)
                        && (ImageCacheService.sForceObsoletePath.equals(mPath
                                .toString()))) {
                    found = false;
                    ImageCacheService.sForceObsoletePath = null;
                }
                /// @}
                if (jc.isCancelled()) return null;
                if (found) {
                    /// M: [DEBUG.ADD] @{
                    TraceHelper.beginSection(">>>>ImageCacheRequest-decodeFromCache");
                    /// @}
                    BitmapFactory.Options options = new BitmapFactory.Options();
                    /// M: [FEATURE.ADD] @{
                    processOptions(mimeType, options);
                    /// @}
                    Bitmap bitmap;
                    if (mType == MediaItem.TYPE_MICROTHUMBNAIL) {
                        bitmap = DecodeUtils.decodeUsingPool(jc, buffer.data,
                                buffer.offset, buffer.length, options);
                    } else {
                        bitmap = DecodeUtils.decodeUsingPool(jc, buffer.data,
                                buffer.offset, buffer.length, options);
                    }
                    if (bitmap == null && !jc.isCancelled()) {
                        Log.w(TAG, "decode cached failed " + debugTag());
                    }
                    /// M: [DEBUG.ADD] @{
                    /// dump Skia decoded cache Bitmap for debug @{
                    if (DebugUtils.DUMP) {
                        if (bitmap == null) {
                            bitmap = Bitmap.createBitmap(DUMP_BITMAP_DEFAULT_SIZE,
                                    DUMP_BITMAP_DEFAULT_SIZE, Bitmap.Config.RGB_565);
                            dumpBitmap(bitmap, mCacheBitmap);
                            bitmap = null;
                        } else {
                            dumpBitmap(bitmap, mCacheBitmap);
                        }
                    }
                    /// @}
                    /// M: [DEBUG.ADD] @{
                    TraceHelper.endSection();
                    /// @}
                    return bitmap;
                }
            } finally {
                MediaItem.getBytesBufferPool().recycle(buffer);
            }
        /// M: [FEATURE.ADD] @{
        }
        /// @}
        /// M: [DEBUG.ADD] @{
        TraceHelper.beginSection(">>>>ImageCacheRequest-decodeFromOriginal");
        /// @}
        Bitmap bitmap = onDecodeOriginal(jc, mType);
        /// M: [DEBUG.ADD] @{
        TraceHelper.endSection();
        /// @}
        if (jc.isCancelled()) return null;

        if (bitmap == null) {
            Log.w(TAG, "decode orig failed " + debugTag());
            return null;
        }

        /// M: [DEBUG.ADD] @{
        /// dump Skia decoded origin Bitmap for debug @{
        if (DebugUtils.DUMP) {
            dumpBitmap(bitmap, mOriginBitmap);
        }
        /// @}

        /// M: [DEBUG.ADD] @{
        TraceHelper.beginSection(">>>>ImageCacheRequest-resizeAndCrop");
        /// @}
        /// M: [FEATURE.MODIFY] fancy layout @{
        if (FancyHelper.isFancyLayoutSupported() && mType == MediaItem.TYPE_FANCYTHUMBNAIL) {
            MediaObject object = mApplication.getDataManager().getMediaObject(mPath);
            if (object != null && object instanceof MediaItem) {
                bitmap = resizeAndCropFancyThumbnail(bitmap, (MediaItem) object, mTargetSize);
            }
        /// @}
        } else if (mType == MediaItem.TYPE_MICROTHUMBNAIL) {
            bitmap = BitmapUtils.resizeAndCropCenter(bitmap, mTargetSize, true);
        } else {
            bitmap = BitmapUtils.resizeDownBySideLength(bitmap, mTargetSize, true);
            /// M: [BUG.ADD] if the width or height is odd value,
            // after decode with PQ enable, output bitmap is a little scale
            // so we should align it to even before compress to cache @{
            bitmap = BitmapUtils.alignBitmapToEven(bitmap, true);
            /// @}
        }
        /// M: [DEBUG.ADD] @{
        TraceHelper.endSection();
        /// @}
        if (jc.isCancelled()) return null;

        /// M: [FEATURE.ADD] @{
        if (needToWriteCache) {
        /// @}
            /// M: [DEBUG.ADD] @{
            TraceHelper.beginSection(">>>>ImageCacheRequest-compressToBytes");
            /// @}
            byte[] array = BitmapUtils.compressToBytes(bitmap);
            /// M: [DEBUG.ADD] @{
            TraceHelper.endSection();
            /// @}
            if (jc.isCancelled()) return null;

            /// M: [DEBUG.ADD] @{
            TraceHelper.beginSection(">>>>ImageCacheRequest-writeToCache");
            /// @}
            cacheService.putImageData(mPath, mTimeModified, mType, array);
            /// M: [DEBUG.ADD] @{
            TraceHelper.endSection();
            /// @}

            /// M: [FEATURE.ADD] @{
            // Cache has not PQ effect, so have to decode PQ effected bitmap
            // for first launch
            BitmapFactory.Options options = new BitmapFactory.Options();
            if (processOptions(mimeType, options)) {
                bitmap.recycle();
                TraceHelper.beginSection(">>>>ImageCacheRequest-decodeFromCacheWithPQ");
                bitmap = BitmapFactory.decodeByteArray(array, 0, array.length, options);
                TraceHelper.endSection();
            }
            /// @}
        /// M: [FEATURE.ADD] @{
        }
        /// @}
        return bitmap;
    }

    public abstract Bitmap onDecodeOriginal(JobContext jc, int targetSize);

    //********************************************************************
    //*                              MTK                                 *
    //********************************************************************
    private static final int DEGREE_90 = 90;
    private static final int DEGREE_270 = 270;
    /// M: [DEBUG.ADD] @{
    //dump skia decode bitmap
    private static final int DUMP_BITMAP_DEFAULT_SIZE = 200;
    private final String mCacheBitmap = "_CacheBitmap";
    private final String mOriginBitmap = "_OriginBitmap";

    private void dumpBitmap(Bitmap bitmap, String source) {
        long dumpStart = System.currentTimeMillis();
        String fileType;
        if (mType == MediaItem.TYPE_MICROTHUMBNAIL) {
            fileType = "MicroTNail";
        } else if (mType == MediaItem.TYPE_FANCYTHUMBNAIL) {
            fileType = "FancyTNail";
        } else {
            fileType = "TNail";
        }
        MediaItem item = (MediaItem) mPath.getObject();
        if (item != null) {
            String string = item.getName() + source + fileType;
            Log.d(TAG, "<dumpBitmap> string " + string);
            DebugUtils.dumpBitmap(bitmap, string);
            Log.d(TAG, "<dumpBitmap> Dump Bitmap time "
                    + (System.currentTimeMillis() - dumpStart));
        }
    }
    /// @}

    /// M: [FEATURE.ADD] fancy layout @{
    public Bitmap resizeAndCropFancyThumbnail(Bitmap bitmap, MediaItem item,
            int targetSize) {
        MediaData data = item.getMediaData();
        int rotation = data.isVideo ?
                ((LocalVideo) item).getOrientation() : item.getRotation();

        if (isScreenShotCover(mApplication, data, item.getPath())) {
            Log.d(TAG, "<resizeAndCropFancyThumbnail> ScreenShotCover");
            return bitmap;
        } else if (rotation == ExtFieldsUtils.NOT_SUPPORT_VIDEO_ROTATION) {
            // just crop center on none-mtk platform if current item is video
            Log.d(TAG, "<resizeAndCropFancyThumbnail> crop center on none-mtk platform if video");
            bitmap = FancyHelper.resizeAndCropCenter(bitmap,  2 * targetSize, true);
            return bitmap;
        }
        boolean isCameraRollCover = isCameraRollCover(mApplication, data, item.getPath());
        int screenWidth = FancyHelper.getScreenWidthAtFancyMode();
        if (isCameraRollCover && FancyHelper.isLandItem(item)) {
            if (data.isVideo) {
                bitmap = FancyHelper.resizeByWidthOrLength(bitmap, 2 * targetSize, true, true);
            } else if (rotation == DEGREE_90 || rotation == DEGREE_270) {
                if (item.getWidth() != 0 &&
                        (float) item.getHeight() / (float) item.getWidth()
                        > FancyHelper.FANCY_CROP_RATIO_CAMERA) {
                    bitmap = FancyHelper.resizeAndCropCenter(bitmap,
                            Math.round(screenWidth / FancyHelper.FANCY_CROP_RATIO_CAMERA),
                            screenWidth, true, true);
                } else {
                    bitmap = FancyHelper.resizeAndCropCenter(bitmap,
                            Math.round(screenWidth / FancyHelper.FANCY_CROP_RATIO_CAMERA),
                            screenWidth, false, true);
                }
            } else {
                if (item.getHeight() != 0 &&
                        (float) item.getWidth() / (float) item.getHeight()
                        > FancyHelper.FANCY_CROP_RATIO_CAMERA) {
                    bitmap = FancyHelper.resizeAndCropCenter(bitmap, screenWidth,
                            Math.round(screenWidth / FancyHelper.FANCY_CROP_RATIO_CAMERA),
                            false, true);
                } else {
                    bitmap = FancyHelper.resizeAndCropCenter(bitmap, screenWidth,
                            Math.round(screenWidth / FancyHelper.FANCY_CROP_RATIO_CAMERA),
                            true, true);
                }
            }
        } else if (rotation == DEGREE_90 || rotation == DEGREE_270) {
            if (item.getHeight() != 0
                    && ((float) item.getWidth() / (float) item.getHeight()
                            > FancyHelper.FANCY_CROP_RATIO)) {
                // for normal picture, resize to fancy thumbnail target size
                // but for certain picture(height is very greater than
                // width), it needs be cropped
                // and displayed to half screen size, so resize to half
                // screen size
                bitmap = FancyHelper.resizeAndCropCenter(bitmap,
                        Math.round(screenWidth / 2 * FancyHelper.FANCY_CROP_RATIO),
                        screenWidth / 2, false, true);
            } else if (item.getHeight() != 0
                    && ((float) item.getWidth() / (float) item.getHeight()
                            < FancyHelper.FANCY_CROP_RATIO_LAND)) {
                bitmap = FancyHelper.resizeAndCropCenter(bitmap,
                        Math.round(screenWidth / 2 * FancyHelper.FANCY_CROP_RATIO_LAND),
                        screenWidth / 2, true, true);
            } else if (isCameraRollCover && data.isVideo) {
                bitmap = FancyHelper.resizeByWidthOrLength(bitmap, 2 * targetSize, true, true);
            } else {
                bitmap = FancyHelper.resizeByWidthOrLength(bitmap, targetSize,
                        false, true);
            }
        } else {
            if (item.getWidth() != 0
                    && ((float) item.getHeight() / (float) item.getWidth()
                            > FancyHelper.FANCY_CROP_RATIO)) {
                // for normal picture, resize to fancy thumbnail target size
                // but for certain picture(height is very greater than
                // width), it needs be cropped
                // and displayed to half screen size, so resize to half
                // screen size
                bitmap = FancyHelper.resizeAndCropCenter(bitmap, screenWidth / 2,
                        Math.round(screenWidth / 2 * FancyHelper.FANCY_CROP_RATIO),
                        true, true);
            } else if (item.getWidth() != 0
                    && ((float) item.getHeight() / (float) item.getWidth()
                            < FancyHelper.FANCY_CROP_RATIO_LAND)) {
                bitmap = FancyHelper.resizeAndCropCenter(bitmap, screenWidth / 2,
                        Math.round(screenWidth / 2 * FancyHelper.FANCY_CROP_RATIO_LAND),
                        false,
                        true);
            } else {
                bitmap = FancyHelper.resizeByWidthOrLength(bitmap, targetSize,
                        true, true);
            }
        }
        return bitmap;
    }

    protected boolean isCameraRollCover(GalleryApp application, MediaData data, Path path) {
        if (data == null || path == null) {
            Log.d(TAG, "<isCameraRollCover> media data or path is null");
            return false;
        }
        Path mediaSetPath = FancyHelper.getMediaSetPath(data);
        MediaSet mediaSet = application.getDataManager().getMediaSet(mediaSetPath);

        if (mediaSet != null && mediaSet.isCameraRoll() && mediaSet.getCoverMediaItem() != null
                && mediaSet.getCoverMediaItem().getPath().equalsIgnoreCase(path.toString())) {
            Log.d(TAG, "<isCameraRollCover> " + data.filePath + " is isCameraRollCover");
            return true;
        }
        return false;
    }

    protected boolean isScreenShotCover(GalleryApp application, MediaData data, Path path) {
        if (data == null) {
            Log.d(TAG, "<isScreenShotCover> media data is null");
            return false;
        }
        Path mediaSetPath = FancyHelper.getMediaSetPath(data);
        MediaSet mediaSet = application.getDataManager().getMediaSet(mediaSetPath);
        if (mediaSet == null) {
            Log.d(TAG, "<isScreenShotCover> mediaSet is null");
            return false;
        }
        String albumName = mediaSet.getName() != null ? mediaSet.getName() : "";
        if (!albumName
                .equals(application.getResources().getString(R.string.folder_screenshot))) {
            return false;
        }
        if (mediaSet.getCoverMediaItem() != null
                && mediaSet.getCoverMediaItem().getPath().equalsIgnoreCase(path.toString())) {
            Log.d(TAG, "<isScreenShotCover> " + data.filePath + " is ScreenShotCover");
            return true;
        }
        return false;
    }
    /// @}

    private static IDecodeOptionsProcessor[] sOptionsProcessors;

    protected  boolean
            processOptions(String mimeType, BitmapFactory.Options options) {
        if (sOptionsProcessors == null) {
            sOptionsProcessors =
                    (IDecodeOptionsProcessor[]) FeatureManager.getInstance().getImplement(
                            IDecodeOptionsProcessor.class);
        }
        boolean changed = false;
        for (IDecodeOptionsProcessor processor : sOptionsProcessors) {
            changed = changed || processor.processThumbDecodeOptions(mimeType, options);
        }
        return changed;
    }

    protected void clearCache() {
        Log.d(TAG, "clear cache, path = " + mPath);
        mApplication.getImageCacheService().clearImageData(mPath,
                mTimeModified, MediaItem.TYPE_THUMBNAIL);
    }
}
