package com.mediatek.gallery3d.adapter;

import android.graphics.Bitmap;

import com.android.gallery3d.app.GalleryAppImpl;
import com.android.gallery3d.ui.PositionController;
import com.android.gallery3d.util.ThreadPool;
import com.android.gallery3d.util.ThreadPool.Job;
import com.android.gallery3d.util.ThreadPool.JobContext;
import com.mediatek.gallery3d.util.Log;
import com.mediatek.gallerybasic.base.MediaData;
import com.mediatek.gallerybasic.base.Work;
import com.mediatek.gallerybasic.platform.Platform;
import com.mediatek.gallerybasic.util.DecodeSpecLimitor;

public class PlatformImpl implements Platform {
    private static final String TAG = "MtkGallery2/PlatformImpl";

    private ThreadPoolBridge mThreadPoolBridge;

    public PlatformImpl(GalleryAppImpl app) {
        mThreadPoolBridge = new ThreadPoolBridge<Bitmap>(app);
    }

    public boolean isOutOfDecodeSpec(long fileSize, int width, int height,
                                     String mimeType) {
        return DecodeSpecLimitor.isOutOfSpecLimit(fileSize, width, height, mimeType);
    }

    public void submitJob(Work work) {
        mThreadPoolBridge.submit(work);
    }

    private static class ThreadPoolBridge<T> {
        private ThreadPool mThreadPool;

        public ThreadPoolBridge(GalleryAppImpl app) {
            mThreadPool = app.getThreadPool();
            Log.d(TAG, "<ThreadPoolBridge> mThreadPool " + mThreadPool);
        }

        public void submit(Work work) {
            mThreadPool.submit(new BridgeJob(work));
        }

        private class BridgeJob implements Job<T> {
            private Work mWork;

            public BridgeJob(Work work) {
                mWork = work;
            }

            @Override
            public T run(JobContext jc) {
                if (mWork != null && !mWork.isCanceled()) {
                    return (T) (mWork.run());
                }
                return null;
            }
        }
    }

    public float getMinScaleLimit(MediaData data) {
        return PositionController.SCALE_LIMIT;
    }
}