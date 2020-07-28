package com.mediatek.gallerygif;

import android.content.Context;
import android.graphics.Bitmap;
import android.os.ParcelFileDescriptor;

import com.mediatek.gallerybasic.base.MediaData;
import com.mediatek.gallerybasic.base.Player;
import com.mediatek.gallerybasic.base.ThumbType;
import com.mediatek.gallerybasic.base.Work;
import com.mediatek.gallerybasic.gl.GLIdleExecuter;
import com.mediatek.gallerybasic.gl.GLIdleExecuter.GLIdleCmd;
import com.mediatek.gallerybasic.gl.MBitmapTexture;
import com.mediatek.gallerybasic.gl.MGLCanvas;
import com.mediatek.gallerybasic.gl.MTexture;
import com.mediatek.gallerybasic.platform.PlatformHelper;
import com.mediatek.gallerybasic.util.BitmapUtils;
import com.mediatek.gallerybasic.util.Log;
import com.mediatek.gallerybasic.util.Utils;

import java.io.FileDescriptor;
import java.io.FileNotFoundException;

public class GifPlayer extends Player {
    private static final String TAG = "MtkGallery2/GifPlayer";
    public static final float RESIZE_RATIO = 3.0f;
    public static final int FRAME_COUNT_MAX = 20;

    private Object mLock = new Object();
    private ThumbType mThumbType;
    private GifDecoderWrapper mGifDecoderWrapper;
    private ParcelFileDescriptor mFD;
    private int mFrameCount;
    private int mCurrentFrameDuration;
    private int mCurrentFrameIndex;
    private int mWidth;
    private int mHeight;
    private boolean mIsPlaying = false;

    private Bitmap mTempBitmap;
    private Bitmap mNextBitmap;
    private MBitmapTexture mTexture;
    private DecodeJob mCurrentJob;

    private boolean mIsCancelled;
    private GLIdleExecuter mGlIdleExecuter;
    private int mTargetSize;
    private byte[] mEncryptBuffer = null;

    // To avoid multi thread issue, decode and recycle need use a same lock,
    // but decode job maybe waste too much time if decode some broken gif, and hold
    // the lock for a long time. it will cause recycle job can not get the lock
    // immediately.
    // We need recycle gif decoder when onPause executed, but the onPause flow will
    // be executed in GLThread. To avoid blocking UI, we cannot recycle in GLThread,
    // but in sub thread.
    private Work<Void> recycleJob = new Work<Void>() {
        @Override
        public boolean isCanceled() {
            return false;
        }

        @Override
        public Void run() {
            synchronized (mLock) {
                if (mGifDecoderWrapper != null) {
                    mGifDecoderWrapper.close();
                    mGifDecoderWrapper = null;
                }
            }
            return null;
        }
    };

    public GifPlayer(Context context, MediaData data, OutputType outputType, ThumbType type,
            GLIdleExecuter glIdleExecuter) {
        super(context, data, outputType);
        mWidth = mMediaData.width;
        mHeight = mMediaData.height;
        mThumbType = type;
        mGlIdleExecuter = glIdleExecuter;
    }

    @Override
    protected boolean onPrepare() {
        return true;
    }

    @Override
    protected synchronized void onRelease() {
        Log.d(TAG, "<onRelease> caption = " + mMediaData.caption);
        removeAllMessages();
        if (mGlIdleExecuter != null) {
            mGlIdleExecuter.addOnGLIdleCmd(new GLIdleCmd() {

                @Override
                public boolean onGLIdle(MGLCanvas canvas) {
                    if (mTexture != null) {
                        mTexture.recycle();
                        mTexture = null;
                    }
                    return false;
                }
            });
        } else {
            if (mTexture != null) {
                mTexture.recycle();
                mTexture = null;
            }
        }
    }

    @Override
    protected boolean onStart() {
        Log.d(TAG, "<onStart> caption = " + mMediaData.caption);
        if (mEncryptBuffer != null) {
            mGifDecoderWrapper = GifDecoderWrapper
                    .createGifDecoderWrapper(mEncryptBuffer, 0, mEncryptBuffer.length);
        } else if (mMediaData.filePath != null && !mMediaData.filePath.equals("")) {
            mGifDecoderWrapper = GifDecoderWrapper.createGifDecoderWrapper(mMediaData.filePath);
        } else if (mMediaData.uri != null) {
            try {
                mFD = mContext.getContentResolver().openFileDescriptor(mMediaData.uri, "r");
                FileDescriptor fd = mFD.getFileDescriptor();
                mGifDecoderWrapper = GifDecoderWrapper.createGifDecoderWrapper(fd);
            } catch (Exception e) {
                Log.w(TAG, "<onStart> Exception", e);
                Utils.closeSilently(mFD);
                mFD = null;
                return false;
            }
        }
        if (mGifDecoderWrapper == null) {
            return false;
        }
        if (mIsCancelled) {
            recycleDecoderWrapper();
            return false;
        }
        mWidth = mGifDecoderWrapper.getWidth();
        mHeight = mGifDecoderWrapper.getHeight();
        mFrameCount = getGifTotalFrameCount();
        if (mFrameCount <= 0 || mWidth <= 0 || mHeight <= 0) {
            Log.d(TAG, "<onStart> broken gif, path: " + mMediaData.filePath + ", mFrameCount "
                    + mFrameCount + ", mWidth " + mWidth + ", mHeight " + mHeight);
            return false;
        }
        mTargetSize = mThumbType.getTargetSize();
        Log.d(TAG, " The image width and height = " + mWidth + " " + mHeight
                + " mThumbType.getTargetSize() = " + mTargetSize + " mFrameCount = "
                + mFrameCount);
        PlatformHelper.submitJob(new DecodeJob(0));

        mIsPlaying = true;
        mCurrentFrameIndex = 0;
        mCurrentFrameDuration = getGifFrameDuration(mCurrentFrameIndex);
        sendFrameAvailable();
        sendPlayFrameDelayed(0);
        return true;
    }

    @Override
    protected synchronized boolean onPause() {
        Log.d(TAG, "<onPause> caption = " + mMediaData.caption);
        mIsPlaying = false;
        removeAllMessages();
        if (mCurrentJob != null) {
            mCurrentJob.cancel();
            mCurrentJob = null;
        }
        recycleDecoderWrapper();
        if (mNextBitmap != null) {
            mNextBitmap.recycle();
            mNextBitmap = null;
        }
        if (mTempBitmap != null) {
            mTempBitmap.recycle();
            mTempBitmap = null;
        }
        if (mFD != null) {
            Utils.closeSilently(mFD);
            mFD = null;
        }
        return true;
    }

    @Override
    protected boolean onStop() {
        Log.d(TAG, "<onStop> caption = " + mMediaData.caption);
        removeAllMessages();
        mCurrentFrameIndex = 0;
        mCurrentFrameDuration = getGifFrameDuration(mCurrentFrameIndex);
        return true;
    }

    @Override
    public void setBuffer(byte[] buffer) {
        mEncryptBuffer = buffer;
    }

    public int getOutputWidth() {
        if (mTexture != null) {
            return mWidth;
        }
        return 0;
    }

    public int getOutputHeight() {
        if (mTexture != null) {
            return mHeight;
        }
        return 0;
    }

    protected synchronized void onPlayFrame() {
        if (!mIsPlaying) {
            return;
        }
        if (mNextBitmap != null) {
            mNextBitmap.recycle();
            mNextBitmap = null;
        }
        mNextBitmap = mTempBitmap;
        mTempBitmap = null;
        sendFrameAvailable();
        sendPlayFrameDelayed(mCurrentFrameDuration);

        if (mCurrentJob != null) {
            return;
        }

        mCurrentFrameIndex++;
        if (mCurrentFrameIndex >= mFrameCount) {
            mCurrentFrameIndex = 0;
        }
        mCurrentFrameDuration = getGifFrameDuration(mCurrentFrameIndex);
        if (mCurrentFrameDuration != GifDecoderWrapper.INVALID_VALUE) {
            mCurrentJob = new DecodeJob(mCurrentFrameIndex);
            PlatformHelper.submitJob(mCurrentJob);
        }
    }

    public synchronized MTexture getTexture(MGLCanvas canvas) {
        if (mNextBitmap != null) {
            if (mTexture != null) {
                mTexture.recycle();
            }
            mTexture = new MBitmapTexture(mNextBitmap);
            mNextBitmap = null;
        }
        return mTexture;
    }

    @Override
    public void onCancel() {
        mIsCancelled = true;
    }

    private int getGifTotalFrameCount() {
        if (mGifDecoderWrapper == null) {
            return 0;
        }
        return mGifDecoderWrapper.getTotalFrameCount();
    }

    private int getGifFrameDuration(int frameIndex) {
        int index = GifDecoderWrapper.INVALID_VALUE;
        synchronized (mLock) {
            if (mGifDecoderWrapper != null) {
                index = mGifDecoderWrapper.getFrameDuration(frameIndex);
            }
        }
        return index;
    }

    class DecodeJob implements Work<Bitmap> {
        private int mIndex;
        private boolean mCanceled = false;

        public DecodeJob(int index) {
            mIndex = index;
        }

        @Override
        public Bitmap run() {
            Bitmap bitmap = null;
            if (isCanceled() || (mGifDecoderWrapper == null)) {
                Log.d(TAG, "<DecodeJob.onDo> isCanceled() & mGifDecoderWrapper:" +
                        isCanceled() + " " + mGifDecoderWrapper);
                onDoFinished(bitmap);
                return null;
            }
            synchronized (mLock) {
                if (null != mGifDecoderWrapper) {
                    bitmap = mGifDecoderWrapper.getFrameBitmap(mIndex);
                    bitmap = resizeBitmap(bitmap);
                }
            }
            onDoFinished(bitmap);
            return bitmap;
        }

        private void onDoFinished(Bitmap bitmap) {
            synchronized (GifPlayer.this) {
                if (mTempBitmap != null) {
                    mTempBitmap.recycle();
                    mTempBitmap = null;
                }
                if (bitmap != null) {
                    if (!isCanceled()) {
                        mTempBitmap = bitmap;
                    } else {
                        bitmap.recycle();
                        bitmap = null;
                    }
                }
                mCurrentJob = null;
            }
        }

        @Override
        public boolean isCanceled() {
            return mCanceled;
        }

        public void cancel() {
            mCanceled = true;
        }
    }

    private Bitmap resizeBitmap(Bitmap bitmap) {
        if (mWidth == 0) {
            return BitmapUtils.replaceBackgroundColor(bitmap, true);
        }
        float ratio = (float) mHeight / (float) mWidth;

        if (mThumbType == ThumbType.MIDDLE && ratio > RESIZE_RATIO) {
            bitmap = BitmapUtils.resizeDownBySideLength(bitmap, (int) (mTargetSize * RESIZE_RATIO),
                    true);
        } else {
            bitmap = BitmapUtils.resizeDownBySideLength(bitmap, mTargetSize, true);
        }
        bitmap = BitmapUtils.replaceBackgroundColor(bitmap, true);
        return bitmap;
    }

    private void resetFrameCount() {
        if (mFrameCount > FRAME_COUNT_MAX) {
            mFrameCount = FRAME_COUNT_MAX;
            Log.d(TAG, "reset frame count " + mFrameCount);
        }
    }

    private void recycleDecoderWrapper() {
        PlatformHelper.submitJob(recycleJob);
    }
}
