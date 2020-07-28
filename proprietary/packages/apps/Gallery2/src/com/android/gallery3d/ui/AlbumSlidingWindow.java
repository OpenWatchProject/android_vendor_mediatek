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

import android.graphics.Bitmap;
import android.os.Message;

import com.android.gallery3d.app.AbstractGalleryActivity;
import com.android.gallery3d.app.AlbumDataLoader;
import com.android.gallery3d.common.Utils;
import com.android.gallery3d.data.MediaItem;
import com.android.gallery3d.data.MediaObject;
import com.android.gallery3d.data.MediaObject.PanoramaSupportCallback;
import com.android.gallery3d.data.Path;
import com.android.gallery3d.glrenderer.Texture;
import com.android.gallery3d.glrenderer.TiledTexture;
import com.android.gallery3d.util.Future;
import com.android.gallery3d.util.FutureListener;
import com.android.gallery3d.util.JobLimiter;
import com.android.gallery3d.util.ThreadPool;


public class AlbumSlidingWindow implements AlbumDataLoader.DataListener {
    @SuppressWarnings("unused")
    private static final String TAG = "Gallery2/AlbumSlidingWindow";

    private static final int MSG_UPDATE_ENTRY = 0;
    /// M: [PERF.MODIFY] @{
    /*private static final int JOB_LIMIT = 2;*/
     // Dynamic control thread number according to CPU Cores from constant number
     private static final int JOB_LIMIT = ThreadPool.PARALLEL_THREAD_NUM; //2;
    /// @}
    public static interface Listener {
        public void onSizeChanged(int size);
        public void onContentChanged();
    }

    public static class AlbumEntry {
        public MediaItem item;
        public Path path;
        public boolean isPanorama;
        public int rotation;
        public int mediaType;
        public boolean isWaitDisplayed;
        public TiledTexture bitmapTexture;
        public Texture content;
        private BitmapLoader contentLoader;
        private PanoSupportListener mPanoSupportListener;
    }

    private final AlbumDataLoader mSource;
    private final AlbumEntry mData[];
    private final SynchronizedHandler mHandler;
    private final JobLimiter mThreadPool;
    /// M: [BEHAVIOR.ADD] mVideoMicroThumbDecoder specializes on video thumbnail decoding
    private final JobLimiter mVideoMicroThumbDecoder;
    private final TiledTexture.Uploader mTileUploader;

    private int mSize;

    private int mContentStart = 0;
    private int mContentEnd = 0;

    private int mActiveStart = 0;
    private int mActiveEnd = 0;

    private Listener mListener;

    private int mActiveRequestCount = 0;
    private boolean mIsActive = false;

    private class PanoSupportListener implements PanoramaSupportCallback {
        public final AlbumEntry mEntry;
        public PanoSupportListener (AlbumEntry entry) {
            mEntry = entry;
        }
        @Override
        public void panoramaInfoAvailable(MediaObject mediaObject, boolean isPanorama,
                boolean isPanorama360) {
            if (mEntry != null) mEntry.isPanorama = isPanorama;
        }
    }

    public AlbumSlidingWindow(AbstractGalleryActivity activity,
            AlbumDataLoader source, int cacheSize) {
        source.setDataListener(this);
        mSource = source;
        mData = new AlbumEntry[cacheSize];
        mSize = source.size();

        mHandler = new SynchronizedHandler(activity.getGLRoot()) {
            @Override
            public void handleMessage(Message message) {
                Utils.assertTrue(message.what == MSG_UPDATE_ENTRY);
                ((ThumbnailLoader) message.obj).updateEntry();
            }
        };

        mThreadPool = new JobLimiter(activity.getThreadPool(), JOB_LIMIT);
        /// M: [BEHAVIOR.ADD] mVideoMicroThumbDecoder specializes on video thumbnail decoding @{
        final int VIDEO_MICRO_THUMB_DECODER_JOB_LIMIT = 2;
        mVideoMicroThumbDecoder = new JobLimiter(activity.getThreadPool(),
                VIDEO_MICRO_THUMB_DECODER_JOB_LIMIT);
        /// @}
        mTileUploader = new TiledTexture.Uploader(activity.getGLRoot());
    }

    public void setListener(Listener listener) {
        mListener = listener;
    }

    public AlbumEntry get(int slotIndex) {
        if (!isActiveSlot(slotIndex)) {
            Utils.fail("invalid slot: %s outsides (%s, %s)",
                    slotIndex, mActiveStart, mActiveEnd);
        }
        return mData[slotIndex % mData.length];
    }

    public boolean isActiveSlot(int slotIndex) {
        return slotIndex >= mActiveStart && slotIndex < mActiveEnd;
    }

    private void setContentWindow(int contentStart, int contentEnd) {
        if (contentStart == mContentStart && contentEnd == mContentEnd) return;

        if (!mIsActive) {
            mContentStart = contentStart;
            mContentEnd = contentEnd;
            mSource.setActiveWindow(contentStart, contentEnd);
            return;
        }

        if (contentStart >= mContentEnd || mContentStart >= contentEnd) {
            for (int i = mContentStart, n = mContentEnd; i < n; ++i) {
                freeSlotContent(i);
            }
            mSource.setActiveWindow(contentStart, contentEnd);
            for (int i = contentStart; i < contentEnd; ++i) {
                prepareSlotContent(i);
            }
        } else {
            for (int i = mContentStart; i < contentStart; ++i) {
                freeSlotContent(i);
            }
            for (int i = contentEnd, n = mContentEnd; i < n; ++i) {
                freeSlotContent(i);
            }
            mSource.setActiveWindow(contentStart, contentEnd);
            for (int i = contentStart, n = mContentStart; i < n; ++i) {
                prepareSlotContent(i);
            }
            for (int i = mContentEnd; i < contentEnd; ++i) {
                prepareSlotContent(i);
            }
        }

        mContentStart = contentStart;
        mContentEnd = contentEnd;
    }

    public void setActiveWindow(int start, int end) {
        if (!(start <= end && end - start <= mData.length && end <= mSize)) {
            Utils.fail("%s, %s, %s, %s", start, end, mData.length, mSize);
        }
        AlbumEntry data[] = mData;

        mActiveStart = start;
        mActiveEnd = end;

        int contentStart = Utils.clamp((start + end) / 2 - data.length / 2,
                0, Math.max(0, mSize - data.length));
        int contentEnd = Math.min(contentStart + data.length, mSize);
        setContentWindow(contentStart, contentEnd);
        updateTextureUploadQueue();
        if (mIsActive) updateAllImageRequests();
    }

    private void uploadBgTextureInSlot(int index) {
        if (index < mContentEnd && index >= mContentStart) {
            AlbumEntry entry = mData[index % mData.length];
            if (entry.bitmapTexture != null) {
                mTileUploader.addTexture(entry.bitmapTexture);
            }
        }
    }

    private void updateTextureUploadQueue() {
        if (!mIsActive) return;
        mTileUploader.clear();

        // add foreground textures
        for (int i = mActiveStart, n = mActiveEnd; i < n; ++i) {
            AlbumEntry entry = mData[i % mData.length];
            if (entry.bitmapTexture != null) {
                mTileUploader.addTexture(entry.bitmapTexture);
            }
        }

        // add background textures
        int range = Math.max(
                (mContentEnd - mActiveEnd), (mActiveStart - mContentStart));
        for (int i = 0; i < range; ++i) {
            uploadBgTextureInSlot(mActiveEnd + i);
            uploadBgTextureInSlot(mActiveStart - i - 1);
        }
    }

    // We would like to request non active slots in the following order:
    // Order:    8 6 4 2                   1 3 5 7
    //         |---------|---------------|---------|
    //                   |<-  active  ->|
    //         |<-------- cached range ----------->|
    private void requestNonactiveImages() {
        int range = Math.max(
                (mContentEnd - mActiveEnd), (mActiveStart - mContentStart));
        for (int i = 0 ;i < range; ++i) {
            requestSlotImage(mActiveEnd + i);
            requestSlotImage(mActiveStart - 1 - i);
        }
    }

    // return whether the request is in progress or not
    private boolean requestSlotImage(int slotIndex) {
        if (slotIndex < mContentStart || slotIndex >= mContentEnd) return false;
        AlbumEntry entry = mData[slotIndex % mData.length];
        if (entry.content != null || entry.item == null) return false;

        // Set up the panorama callback
        entry.mPanoSupportListener = new PanoSupportListener(entry);
        entry.item.getPanoramaSupport(entry.mPanoSupportListener);

        entry.contentLoader.startLoad();
        return entry.contentLoader.isRequestInProgress();
    }

    private void cancelNonactiveImages() {
        int range = Math.max(
                (mContentEnd - mActiveEnd), (mActiveStart - mContentStart));
        for (int i = 0 ;i < range; ++i) {
            cancelSlotImage(mActiveEnd + i);
            cancelSlotImage(mActiveStart - 1 - i);
        }
    }

    private void cancelSlotImage(int slotIndex) {
        if (slotIndex < mContentStart || slotIndex >= mContentEnd) return;
        AlbumEntry item = mData[slotIndex % mData.length];
        if (item.contentLoader != null) item.contentLoader.cancelLoad();
    }

    private void freeSlotContent(int slotIndex) {
        AlbumEntry data[] = mData;
        int index = slotIndex % data.length;
        AlbumEntry entry = data[index];
        if (entry.contentLoader != null) entry.contentLoader.recycle();
        if (entry.bitmapTexture != null) entry.bitmapTexture.recycle();
        data[index] = null;
    }

    private void prepareSlotContent(int slotIndex) {
        AlbumEntry entry = new AlbumEntry();
        MediaItem item = mSource.get(slotIndex); // item could be null;
        entry.item = item;
        entry.mediaType = (item == null)
                ? MediaItem.MEDIA_TYPE_UNKNOWN
                : entry.item.getMediaType();
        entry.path = (item == null) ? null : item.getPath();
        entry.rotation = (item == null) ? 0 : item.getRotation();
        entry.contentLoader = new ThumbnailLoader(slotIndex, entry.item);
        mData[slotIndex % mData.length] = entry;
    }

    private void updateAllImageRequests() {
        mActiveRequestCount = 0;
        for (int i = mActiveStart, n = mActiveEnd; i < n; ++i) {
            if (requestSlotImage(i)) ++mActiveRequestCount;
        }
        if (mActiveRequestCount == 0) {
            requestNonactiveImages();
        } else {
            cancelNonactiveImages();
        }
    }

    private class ThumbnailLoader extends BitmapLoader  {
        private final int mSlotIndex;
        private final MediaItem mItem;

        public ThumbnailLoader(int slotIndex, MediaItem item) {
            mSlotIndex = slotIndex;
            mItem = item;
        }

        @Override
        protected Future<Bitmap> submitBitmapTask(FutureListener<Bitmap> l) {
            /// M: [BEHAVIOR.ADD] mVideoMicroThumbDecoder specializes on video thumbnail decoding @{
            if (MediaObject.MEDIA_TYPE_VIDEO == mItem.getMediaType()) {
                return mVideoMicroThumbDecoder.submit(mItem
                        .requestImage(MediaItem.TYPE_MICROTHUMBNAIL), this);
            }
            /// @}
            return mThreadPool.submit(
                    mItem.requestImage(MediaItem.TYPE_MICROTHUMBNAIL), this);
        }

        @Override
        protected void onLoadComplete(Bitmap bitmap) {
            mHandler.obtainMessage(MSG_UPDATE_ENTRY, this).sendToTarget();
        }

        public void updateEntry() {
            /// M: [BUG.ADD] abort to updateEntry if page has been paused. @{
            if (!mIsActive) {
                return;
            }
            /// @}
            Bitmap bitmap = getBitmap();
            if (bitmap == null) return; // error or recycled
            AlbumEntry entry = mData[mSlotIndex % mData.length];
            /// M: [BUG.ADD] @{
            if (entry == null) return;
            /// @}
            entry.bitmapTexture = new TiledTexture(bitmap);
            entry.content = entry.bitmapTexture;

            if (isActiveSlot(mSlotIndex)) {
                mTileUploader.addTexture(entry.bitmapTexture);
                --mActiveRequestCount;
                if (mActiveRequestCount == 0) requestNonactiveImages();
                if (mListener != null) mListener.onContentChanged();
            } else {
                mTileUploader.addTexture(entry.bitmapTexture);
            }

            /// M: [TESTCASE.ADD] @{
            mBitmapLoaded = true;
            if (isActiveSlot(mSlotIndex) && mActiveRequestCount == 0) {
                mDecodeFinished = true;
                mDecodeFinishTime = System.currentTimeMillis();
            }
            /// @}
        }
    }

    @Override
    public void onSizeChanged(int size) {
        if (mSize != size) {
            mSize = size;
            if (mListener != null) mListener.onSizeChanged(mSize);
            if (mContentEnd > mSize) mContentEnd = mSize;
            if (mActiveEnd > mSize) mActiveEnd = mSize;
        }
    }

    @Override
    public void onContentChanged(int index) {
        if (index >= mContentStart && index < mContentEnd && mIsActive) {
            freeSlotContent(index);
            prepareSlotContent(index);
            updateAllImageRequests();
            if (mListener != null && isActiveSlot(index)) {
                mListener.onContentChanged();
            }
        }
    }

    public void resume() {
        mIsActive = true;
        TiledTexture.prepareResources();
        for (int i = mContentStart, n = mContentEnd; i < n; ++i) {
            prepareSlotContent(i);
        }
        updateAllImageRequests();
    }

    public void pause() {
        mIsActive = false;
        mTileUploader.clear();
        TiledTexture.freeResources();
        for (int i = mContentStart, n = mContentEnd; i < n; ++i) {
            freeSlotContent(i);
        }
    }

    //********************************************************************
    //*                              MTK                                 *
    //********************************************************************

    public boolean mDecodeFinished = false;
    public long mDecodeFinishTime = 0;

    ///M: [MEMORY.ADD] @{
    /**
     * Recycle the contentLoader.
     *
     * @param entry
     *            The AlbumEntry whose contentLoader will be recycled
     */
    public void recycle(AlbumEntry entry) {
        if (entry.contentLoader != null) {
            entry.contentLoader.recycle();
        }
    }
    /// @}

    // returns true if all active/visible slots are filled
    public boolean isAllActiveSlotsFilled() {
        int start = mActiveStart;
        int end = mActiveEnd;

        if (start < 0 || start >= end) {
            Log.w(TAG, "<isAllActiveSlotFilled> active range not ready yet");
            return false;
        }

        AlbumEntry entry;
        BitmapLoader loader;
        for (int i = start; i < end; ++i) {
            entry = mData[i % mData.length];
            if (entry == null) {
                Log.d(TAG, "<isAllActiveSlotsFilled> slot " + i
                        + " is not loaded, return false");
                return false;
            }
            loader = entry.contentLoader;
            if (loader == null || !loader.isLoadingCompleted()) {
                Log.d(TAG, "<isAllActiveSlotsFilled> slot " + i
                        + " is not loaded, return false");
                return false;
            }
        }

        Log.d(TAG, "<isAllActiveSlotsFilled> return true");
        return true;
    }
}
