package com.mediatek.galleryfeature.drm;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;

import com.mediatek.gallerybasic.base.MediaData;
import com.mediatek.gallerybasic.base.ThumbType;
import com.mediatek.gallerybasic.base.Work;
import com.mediatek.gallerybasic.gl.MBitmapTexture;
import com.mediatek.gallerybasic.gl.MTexture;
import com.mediatek.gallerybasic.platform.PlatformHelper;
import com.mediatek.gallerybasic.util.BitmapUtils;
import com.mediatek.gallerybasic.util.Log;


/**
 * Generate Drm Texture for DrmPlayer.
 */
public class DrmDataAdapter {
    private final static String TAG = "MtkGallery2/DrmDataAdapter";
    private ThumbNailJob mThumbNailJob;
    private ScreenNailJob mScreenNailJob;
    private DataListener mDataListener;
    private MediaData mMediaData;
    private Context mContext;
    private DrmItem mDrmItem;
    private DrmTexture mThumbNailTexture;
    private MBitmapTexture mScreenNailTexture;
    private Object mLock = new Object();

    /**
     * Constructor.
     * @param context
     *            Use the context generate Drm texture.
     * @param mediaData
     *            The Drm data.
     * @param drmItem
     *            The Drm item.
     * @param dataListener
     *            While has decode bitmap, should notify GLthread for new Texture.
     */
    public DrmDataAdapter(Context context, MediaData mediaData, DrmItem drmItem,
            DataListener dataListener) {
        mContext = context;
        mMediaData = mediaData;
        mDrmItem = drmItem;
        mDataListener = dataListener;
    }

    /**
     * Call back by mScreenNailJob and mThumbNailJob, while have finished the job.
     */
    public interface DataListener {
        /**
         * Notify GLthread.
         */
        public void onDataUpdated();

    }

    /**
     * Do decode bitmap and create texture.
     */
    interface Job extends Work<Bitmap> {

        public void cancel();
    }

    /**
     * Create thumbnail job and submit on prepare state.
     */
    public void onPrepare() {
        mThumbNailJob = new ThumbNailJob();
        PlatformHelper.submitJob(mThumbNailJob);
    }

    /**
     * Create thumbnail job and submit on start state.
     */
    public void onStart() {
        mScreenNailJob = new ScreenNailJob();
        PlatformHelper.submitJob(mScreenNailJob);
    }

    /**
     * Cancel screenNail job and recycle screenNail texture.
     */
    public void onPause() {
        if (mScreenNailJob != null) {
            mScreenNailJob.cancel();
            mScreenNailJob = null;
        }
        synchronized (mLock) {
            if (mScreenNailTexture != null) {
                mScreenNailTexture.recycle();
                mScreenNailTexture = null;
            }
        }
    }

    /**
     * Cancel ThumbNail job and recycle mThumbNailTexture texture.
     */
    public void onRelease() {
        Log.d(TAG, " <onRelease> mFilepath = " + mMediaData.filePath);
        if (mThumbNailJob != null) {
            mThumbNailJob.cancel();
            mThumbNailJob = null;
        }
        synchronized (mLock) {
            if (mThumbNailTexture != null) {
                mThumbNailTexture.recycle();
                mThumbNailTexture = null;
            }
        }
    }

    /**
     * Call by GLThread.
     * @return the current texture.
     */
    public MTexture getTexture() {
        synchronized (mLock) {
            if (mScreenNailTexture != null) {
                return mScreenNailTexture;
            } else {
                return mThumbNailTexture;
            }
        }

    }

    /**
     * Save current texture state. While press home key.
     * @return Whether on ScreenNail texture for display or not.
     */
    public boolean isScreenNailReady() {
        synchronized (mLock) {
            return mScreenNailTexture != null;
        }

    }

    /**
     * Call by sub-thread. Decode bitmap for texture.
     */
    private class ThumbNailJob implements Job {
        private boolean mCanceled;

        @Override
        public boolean isCanceled() {
            return mCanceled;
        }

        @Override
        public Bitmap run() {
            Bitmap bitmap = mDrmItem.getDrmThumbnail(ThumbType.MIDDLE);
            if (bitmap != null) {
                if (isCanceled()) {
                    bitmap.recycle();
                    bitmap = null;
                } else {
                    synchronized (mLock) {
                        mThumbNailTexture =
                                new DrmTexture(mContext, bitmap, mMediaData.filePath, "",
                                        mMediaData.isVideo);
                    }
                    if (mDataListener != null) {
                        mDataListener.onDataUpdated();
                    }
                }
            }
            return null;
        }

        @Override
        public void cancel() {
            mCanceled = true;
        }

    }

    /**
     * Call by sub-thread. Decode bitmap for texture.
     */
    class ScreenNailJob implements Job {
        private final static String TAG = "MtkGallery2/ScreenNailJob";
        private boolean mCanceled;

        @Override
        public boolean isCanceled() {
            return mCanceled;
        }

        @Override
        public Bitmap run() {
            Bitmap drmBitmap = null;
            if (!isCanceled() || (mMediaData.filePath != null && !mMediaData.filePath.equals(""))) {
                int targetSize = ThumbType.MIDDLE.getTargetSize();
                int width = mDrmItem.getWidth();
                int height = mDrmItem.getHeight();
                BitmapFactory.Options options = new BitmapFactory.Options();
                if (width > 0 && height > 0) {
                    options.inSampleSize =
                            BitmapUtils.computeSampleSizeLarger(width, height, targetSize);
                }
                drmBitmap = mDrmItem.decodeBitmap(options);
                // 1. Replace background color for alpha image.
                // 2. Define bitmap to ARGB_8888 format for wbmp image(index8 format).
                drmBitmap = BitmapUtils.replaceBackgroundColor(drmBitmap, true);
                Log.d(TAG, " <run> isCanceled = " + isCanceled() + " mFilePath = "
                        + mMediaData.filePath + " drmBitmap = " + drmBitmap);
            }
            if (drmBitmap != null) {
                if (isCanceled()) {
                    drmBitmap.recycle();
                    drmBitmap = null;
                } else {
                    synchronized (mLock) {
                        mScreenNailTexture = new MBitmapTexture(drmBitmap);
                    }
                }
            }
            if (mThumbNailTexture != null) {
                Log.d(TAG, " <run> Notify Display Right Change!");
                mThumbNailTexture.notifyDisplayRightChange();
            }
            if (mDataListener != null) {
                mDataListener.onDataUpdated();
            }
            return null;
        }

        @Override
        public void cancel() {
            mCanceled = true;
        }
    }

}
