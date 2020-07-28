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
import android.graphics.BitmapRegionDecoder;

import com.android.gallery3d.glrenderer.TiledTexture;
import com.android.gallery3d.ui.ScreenNail;
import com.android.gallery3d.util.GalleryUtils;
import com.android.gallery3d.util.ThreadPool.Job;

import com.mediatek.gallerybasic.base.ExtItem;
import com.mediatek.gallerybasic.base.MediaData;
import com.mediatek.gallerybasic.base.ThumbType;
import com.mediatek.gallerybasic.util.Utils;

// MediaItem represents an image or a video item.
public abstract class MediaItem extends MediaObject {
    // NOTE: These type numbers are stored in the image cache, so it should not
    // not be changed without resetting the cache.
    private static final String TAG = "Gallery2/MediaItem";

    public static final int TYPE_THUMBNAIL = 1;
    public static final int TYPE_MICROTHUMBNAIL = 2;
    /// M: [FEATURE.ADD] fancy layout @{
    public static final int TYPE_FANCYTHUMBNAIL = 3;
    /// @}
    /// M: [BEHAVIOR.ADD] @{
    public static final int TYPE_HIGHQUALITYTHUMBNAIL = 4;
    /// @}
    public static final int CACHED_IMAGE_QUALITY = 95;

    public static final int IMAGE_READY = 0;
    public static final int IMAGE_WAIT = 1;
    public static final int IMAGE_ERROR = -1;

    public static final String MIME_TYPE_JPEG = "image/jpeg";

    private static final int BYTESBUFFE_POOL_SIZE = 4;
    private static final int BYTESBUFFER_SIZE = 200 * 1024;

    private static int sMicrothumbnailTargetSize = 200;
    private static final BytesBufferPool sMicroThumbBufferPool =
            new BytesBufferPool(BYTESBUFFE_POOL_SIZE, BYTESBUFFER_SIZE);

    /// M: [BUG.MODIFY]
    // private static int sThumbnailTargetSize = 640;
    public static int sThumbnailTargetSize = 640;
    /// @}
    /// M: [FEATURE.ADD] fancy layout @{
    protected static int sFancyThumbnailSize = 360;
    /// @}

    /// M: [BEHAVIOR.ADD] for hithQuality bitmap @{
    public static int sHighQualityThumbnailSize = GalleryUtils.sRealResolutionMaxSize;
    /// @}
    // TODO: fix default value for latlng and change this.
    public static final double INVALID_LATLNG = 0f;

    public abstract Job<Bitmap> requestImage(int type);
    public abstract Job<BitmapRegionDecoder> requestLargeImage();

    public MediaItem(Path path, long version) {
        super(path, version);
    }

    public long getDateInMs() {
        return 0;
    }

    public String getName() {
        return null;
    }

    public void getLatLong(double[] latLong) {
        latLong[0] = INVALID_LATLNG;
        latLong[1] = INVALID_LATLNG;
    }

    public String[] getTags() {
        return null;
    }

    public Face[] getFaces() {
        return null;
    }

    // The rotation of the full-resolution image. By default, it returns the value of
    // getRotation().
    public int getFullImageRotation() {
        return getRotation();
    }

    public int getRotation() {
        return 0;
    }

    public long getSize() {
        return 0;
    }

    public abstract String getMimeType();

    public String getFilePath() {
        return "";
    }

    // Returns width and height of the media item.
    // Returns 0, 0 if the information is not available.
    public abstract int getWidth();
    public abstract int getHeight();

    // This is an alternative for requestImage() in PhotoPage. If this
    // is implemented, you don't need to implement requestImage().
    public ScreenNail getScreenNail() {
        return null;
    }

    public static int getTargetSize(int type) {
        switch (type) {
            case TYPE_THUMBNAIL:
                return sThumbnailTargetSize;
            case TYPE_MICROTHUMBNAIL:
                return sMicrothumbnailTargetSize;
            /// M: [FEATURE.ADD] fancy layout @{
            case TYPE_FANCYTHUMBNAIL:
                return sFancyThumbnailSize;
            /// @}
            /// M: [BEHAVIOR.ADD] @{
            case TYPE_HIGHQUALITYTHUMBNAIL:
                return sHighQualityThumbnailSize;
            /// @}
            default:
                throw new RuntimeException(
                    "should only request thumb/microthumb from cache");
        }
    }

    public static BytesBufferPool getBytesBufferPool() {
        return sMicroThumbBufferPool;
    }

    public static void setThumbnailSizes(int size, int microSize) {
        sThumbnailTargetSize = size;

        /// M: [BUG.MODIFY] @{
        /*if (sMicrothumbnailTargetSize != microSize) {
         sMicrothumbnailTargetSize = microSize;
         }*/
        ///for 720p resolution, if thumbnail size equal 256,
        // thumbnail will be divided into four tiles, performance will be degrade
        if (sMicrothumbnailTargetSize != microSize) {
            sMicrothumbnailTargetSize = ((microSize % 256 == 0) ? microSize - 2
                    * TiledTexture.BORDER_SIZE : microSize);
        }

        /// M: [BUG.ADD] @{
        // When device ram = 1G, screen resolution is FHD, memory size is not
        // enough to run gallery normally. In this case, degrade size of
        // thumbnail
        if (Utils.getDeviceRam() > 0 && Utils.getDeviceRam() <= RAM_IN_KB_1G) {
            sThumbnailTargetSize = Math.min(sThumbnailTargetSize, MAX_THUMBNAIL_SIZE_1G_RAM);
            sMicrothumbnailTargetSize = Math.min(sMicrothumbnailTargetSize,
                    MAX_MICRO_THUMBNAIL_SIZE_1G_RAM);
        }
        /// @}

        Log.d(TAG, "<setThumbnailSizes> sThumbnailTargetSize = "
                + sThumbnailTargetSize + ", sMicrothumbnailTargetSize = "
                + sMicrothumbnailTargetSize);
        /// @}
    }

    //********************************************************************
    //*                              MTK                                 *
    //********************************************************************

    // the max thumbnail size of 1G ram device, set as that of 720p device
    private static final int MAX_THUMBNAIL_SIZE_1G_RAM = 592;
    private static final int MAX_MICRO_THUMBNAIL_SIZE_1G_RAM = 236;
    private static final int MAX_FANCYTHUMBNAIL_SIZE_1G_RAM = 240;
    private static final long RAM_IN_KB_1G = 1024 * 1024;

    protected ExtItem mExtItem = null;
    protected MediaData mMediaData = null;
    protected Object mMediaDataLock = new Object();

    public ExtItem getExtItem() {
        return mExtItem;
    }

    public MediaData getMediaData() {
        synchronized (mMediaDataLock) {
            return mMediaData;
        }
    }

    /// M: [FEATURE.ADD] fancy layout @{
    public static void setFancyThumbnailSizes(int fancyThumbnailSize) {
        sFancyThumbnailSize = fancyThumbnailSize;

        // When device ram = 1G, screen resolution is FHD, memory size is not
        // enough to run gallery normally. In this case, degrade size of
        // thumbnail
        if (Utils.getDeviceRam() > 0 && Utils.getDeviceRam() <= RAM_IN_KB_1G) {
            sFancyThumbnailSize = Math.min(sFancyThumbnailSize, MAX_FANCYTHUMBNAIL_SIZE_1G_RAM);
        }

        ThumbType.FANCY.setTargetSize(sFancyThumbnailSize);
        Log.d(TAG, "<setFancyThumbnailSizes> <Fancy> sFancyThumbnailSize = "
                + sFancyThumbnailSize);
    }
    /// @}

    /// M: [BEHAVIOR.ADD] @{
    // use high-quality screennail instead of region decoder for extremely large image
    protected static final int REGION_DECODER_PICTURE_SIZE_LIMIT = 12 * 1024 * 1024;
    /// @}

    /// M: [BUG.ADD] read DNG EXIF details. @{
    public static final String MIME_TYPE_DNG = "image/x-adobe-dng";
    /// @}
}
