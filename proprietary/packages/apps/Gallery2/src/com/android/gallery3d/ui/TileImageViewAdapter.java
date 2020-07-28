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

package com.android.gallery3d.ui;

import android.annotation.TargetApi;
import android.graphics.Bitmap;
import android.graphics.Bitmap.Config;
import android.graphics.BitmapFactory;
import android.graphics.BitmapRegionDecoder;
import android.graphics.Canvas;
import android.graphics.Rect;

import com.android.gallery3d.common.ApiHelper;
import com.android.gallery3d.common.Utils;
import com.android.gallery3d.data.MediaItem;
import com.android.photos.data.GalleryBitmapPool;
import com.mediatek.gallery3d.adapter.FeatureManager;
import com.mediatek.gallerybasic.base.IDecodeOptionsProcessor;
import com.mediatek.gallerybasic.util.DebugUtils;
import com.mediatek.gallerybasic.util.BitmapUtils;
import java.util.concurrent.locks.ReadWriteLock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

public class TileImageViewAdapter implements TileImageView.TileSource {
    private static final String TAG = "Gallery2/TileImageViewAdapter";
    protected ScreenNail mScreenNail;
    protected boolean mOwnScreenNail;
    protected BitmapRegionDecoder mRegionDecoder;
    protected int mImageWidth;
    protected int mImageHeight;
    protected int mLevelCount;

    public TileImageViewAdapter() {
    }

    public synchronized void clear() {
        mScreenNail = null;
        mImageWidth = 0;
        mImageHeight = 0;
        mLevelCount = 0;
        mRegionDecoder = null;
    }

    // Caller is responsible to recycle the ScreenNail
    public synchronized void setScreenNail(
            ScreenNail screenNail, int width, int height) {
        Utils.checkNotNull(screenNail);
        mScreenNail = screenNail;
        mImageWidth = width;
        mImageHeight = height;
        mRegionDecoder = null;
        mLevelCount = 0;
    }

    public synchronized void setRegionDecoder(BitmapRegionDecoder decoder) {
        mRegionDecoder = Utils.checkNotNull(decoder);
        mImageWidth = decoder.getWidth();
        mImageHeight = decoder.getHeight();
        mLevelCount = calculateLevelCount();
    }

    /**
     * M: [PERF.ADD]
     * Set region decoder to tile provider,
     * The width and height is come from cache.
     * @param decoder
     * @param width
     * @param height
     */
    public synchronized void setRegionDecoder(BitmapRegionDecoder decoder, int width, int height) {
        mRegionDecoder = Utils.checkNotNull(decoder);
        mImageWidth = width;
        mImageHeight = height;
        mLevelCount = calculateLevelCount();
    }

    private int calculateLevelCount() {
        return Math.max(0, Utils.ceilLog2(
                (float) mImageWidth / mScreenNail.getWidth()));
    }

    // Gets a sub image on a rectangle of the current photo. For example,
    // getTile(1, 50, 50, 100, 3, pool) means to get the region located
    // at (50, 50) with sample level 1 (ie, down sampled by 2^1) and the
    // target tile size (after sampling) 100 with border 3.
    //
    // From this spec, we can infer the actual tile size to be
    // 100 + 3x2 = 106, and the size of the region to be extracted from the
    // photo to be 200 with border 6.
    //
    // As a result, we should decode region (50-6, 50-6, 250+6, 250+6) or
    // (44, 44, 256, 256) from the original photo and down sample it to 106.
    @TargetApi(ApiHelper.VERSION_CODES.HONEYCOMB)
    @Override
    public Bitmap getTile(int level, int x, int y, int tileSize) {
        if (!ApiHelper.HAS_REUSING_BITMAP_IN_BITMAP_REGION_DECODER) {
            return getTileWithoutReusingBitmap(level, x, y, tileSize);
        }

        int t = tileSize << level;

        Rect wantRegion = new Rect(x, y, x + t, y + t);

        boolean needClear;
        BitmapRegionDecoder regionDecoder = null;

        synchronized (this) {
            regionDecoder = mRegionDecoder;
            if (regionDecoder == null) return null;

            // We need to clear a reused bitmap, if wantRegion is not fully
            // within the image.
            needClear = !new Rect(0, 0, mImageWidth, mImageHeight)
                    .contains(wantRegion);
        }

        Bitmap bitmap = GalleryBitmapPool.getInstance().get(tileSize, tileSize);
        if (bitmap != null) {
            if (needClear) bitmap.eraseColor(0);
        } else {
            bitmap = Bitmap.createBitmap(tileSize, tileSize, Config.ARGB_8888);
        }

        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inPreferredConfig = Config.ARGB_8888;
        options.inPreferQualityOverSpeed = true;
        options.inSampleSize =  (1 << level);
        options.inBitmap = bitmap;
        /// M: [FEATURE.ADD] @{
        processOptions(mMimeType, options);
        /// @}
        try {
            /// M: [PERF.MARK] @{
            //  Do region decode in multi-thread, so delete synchronized
            /*
             // In CropImage, we may call the decodeRegion() concurrently.
             synchronized (regionDecoder) {
            */
            /// M: [BUG.ADD] Ensure that decodeRegion function is sync with recycle@{
            mRegionDecoderLock.readLock().lock();
            /// @}
            bitmap = regionDecoder.decodeRegion(wantRegion, options);
            if (DebugUtils.TILE) {
                if (bitmap == null) {
                    Log.d(TAG, "<getTile1> decodeRegion l" + level + "-x" + x + "-y" + y + "-size"
                            + tileSize + ", return null");
                } else {
                    DebugUtils.dumpBitmap(bitmap, "Tile-l" + level + "-x" + x + "-y" + y + "-size"
                            + tileSize + "-" + sTileDumpNum);
                    sTileDumpNum++;
                }
            }
            /// M: [PERF.MARK] @{
            /* } */
        } finally {
            /// M: [BUG.ADD] Ensure that decodeRegion function is sync with recycle@{
            mRegionDecoderLock.readLock().unlock();
            /// @}
            if (options.inBitmap != bitmap && options.inBitmap != null) {
                GalleryBitmapPool.getInstance().put(options.inBitmap);
                options.inBitmap = null;
            }
        }

        if (bitmap == null) {
            Log.w(TAG, "fail in decoding region");
        }

        /// M: [BUG.ADD] Some bitmaps have transparent areas, so clear alpha value @{
        bitmap = replaceBackgroudForTile(bitmap, level, x, y, tileSize);
        /// @}

        return bitmap;
    }

    private Bitmap getTileWithoutReusingBitmap(
            int level, int x, int y, int tileSize) {
        int t = tileSize << level;
        Rect wantRegion = new Rect(x, y, x + t, y + t);

        BitmapRegionDecoder regionDecoder;
        Rect overlapRegion;

        synchronized (this) {
            regionDecoder = mRegionDecoder;
            if (regionDecoder == null) return null;
            overlapRegion = new Rect(0, 0, mImageWidth, mImageHeight);
            Utils.assertTrue(overlapRegion.intersect(wantRegion));
        }

        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inPreferredConfig = Config.ARGB_8888;
        options.inPreferQualityOverSpeed = true;
        options.inSampleSize =  (1 << level);
        Bitmap bitmap = null;
        /// M: [FEATURE.ADD] @{
        processOptions(mMimeType, options);
        /// @}
        /// M: [PERF.MARK] @{
        //  Do region decode in multi-thread, so delete synchronized
        /*
         // In CropImage, we may call the decodeRegion() concurrently.
         synchronized (regionDecoder) {
        */
        /// M: [BUG.MODIFY] @{
        // Ensure that decodeRegion function is sync with recycle
        // bitmap = regionDecoder.decodeRegion(wantRegion, options);
        try {
            mRegionDecoderLock.readLock().lock();
            bitmap = regionDecoder.decodeRegion(wantRegion, options);
        } finally {
            mRegionDecoderLock.readLock().unlock();
        }
        /// @}
        /// M: [PERF.MARK] @{
        /* } */

        if (bitmap == null) {
            Log.w(TAG, "fail in decoding region");
        }

        /// M: [BUG.ADD] Some bitmaps have transparent areas, so clear alpha value @{
        bitmap = replaceBackgroudForTile(bitmap, level, x, y, tileSize);
        /// @}

        if (wantRegion.equals(overlapRegion)) return bitmap;

        Bitmap result = Bitmap.createBitmap(tileSize, tileSize, Config.ARGB_8888);
        Canvas canvas = new Canvas(result);
        canvas.drawBitmap(bitmap,
                (overlapRegion.left - wantRegion.left) >> level,
                (overlapRegion.top - wantRegion.top) >> level, null);
        return result;
    }


    @Override
    public ScreenNail getScreenNail() {
        return mScreenNail;
    }

    @Override
    public int getImageHeight() {
        return mImageHeight;
    }

    @Override
    public int getImageWidth() {
        return mImageWidth;
    }

    @Override
    public int getLevelCount() {
        return mLevelCount;
    }
    //********************************************************************
    //*                              MTK                                 *
    //********************************************************************
    private static int sTileDumpNum = 0;
    // Ensure that decodeRegion function is sync with recycle
    private ReadWriteLock mRegionDecoderLock = new ReentrantReadWriteLock();

    public String mMimeType = null;
    public TileImageViewAdapter(
            Bitmap bitmap, BitmapRegionDecoder regionDecoder) {
        Utils.checkNotNull(bitmap);
        updateScreenNail(new BitmapScreenNail(bitmap), true);
        mRegionDecoder = regionDecoder;
        mImageWidth = regionDecoder.getWidth();
        mImageHeight = regionDecoder.getHeight();
        mLevelCount = calculateLevelCount();
    }

    private void updateScreenNail(ScreenNail screenNail, boolean own) {
        if (mScreenNail != null && mOwnScreenNail) {
            mScreenNail.recycle();
        }
        mScreenNail = screenNail;
        mOwnScreenNail = own;
    }

    /**
     * Clear all info in TileImageViewAdapter, add recycle RegionDecoder before clear.
     */
    public synchronized void clearAndRecycle() {
        mScreenNail = null;
        mImageWidth = 0;
        mImageHeight = 0;
        mLevelCount = 0;
        // When BitmapRegionDecoder is create from FileDescriptor, even if
        // FileDescriptor is closed, BitmapRegionDecoder is still holding the file.
        // In order to release file as soon as possible, call
        // BitmapRegionDecoder.recycle() to release file descriptor.
        // mRegionDecoder = null;
        if (mRegionDecoder != null) {
            mRegionDecoderLock.writeLock().lock();
            mRegionDecoder.recycle();
            mRegionDecoderLock.writeLock().unlock();
            mRegionDecoder = null;
        }
    }

    /// M: [FEATURE.ADD] plugin @{
    public void updateWidthAndHeight(MediaItem item) {
        if (item != null) {
            mImageWidth = item.getWidth();
            mImageHeight = item.getHeight();
            Log.d(TAG, "<updateWidthAndHeight> mImageWidth " + mImageWidth
                    + ", mImageHeight " + mImageHeight);
        }
    }
    /// @}

    private static IDecodeOptionsProcessor[] sOptionsProcessors;
    private boolean processOptions(String mimeType, BitmapFactory.Options options) {
        if (sOptionsProcessors == null) {
            sOptionsProcessors =
                    (IDecodeOptionsProcessor[]) FeatureManager.getInstance().getImplement(
                            IDecodeOptionsProcessor.class);
        }
        boolean changed = false;
        for (IDecodeOptionsProcessor processor : sOptionsProcessors) {
            changed = changed || processor.processRegionDecodeOptions(mimeType, options);
        }
        return changed;
    }

    /// Only replace the rect which contains valid pixel data, not replace the whole bitmap
    private Bitmap
            replaceBackgroudForTile(Bitmap bitmap, int level, int x, int y, int tileSize) {
        int t = tileSize << level;
        Rect wantRegion = new Rect(x, y, x + t, y + t);
        int resWidth = wantRegion.right > mImageWidth ? (mImageWidth - x) >> level : tileSize;
        int resHeight =
                wantRegion.bottom > mImageHeight ? (mImageHeight - y) >> level : tileSize;
        return BitmapUtils.replaceBackgroundColor(bitmap, true, new Rect(0, 0, resWidth,
                resHeight));
    }
}
