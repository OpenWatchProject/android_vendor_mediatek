package com.mediatek.galleryfeature.pq;

import android.content.Context;
import android.graphics.Bitmap;

/**
 * Create load task for bitmap and set bitmap as ImageView.
 */
public class PresentImage {

    private static PresentImage sPresentImage;
    private RenderingRequestListener mListener;
    private LoadBitmapTask mLoadBitmapTask;

    /**
     * Set bitmap while success decode bitmap.
     */
    public interface RenderingRequestListener {
        /**
         * set bitmap for ImageView display.
         * @param bitmap for ImageView.
         * @param uri the image uri.
         * @return whether set image bitmap for ImageView or no.
         */
        public boolean available(Bitmap bitmap, String uri);
    }

    /**
     * Get PresentImage object.
     * @return return the object.
     */
    public static PresentImage getPresentImage() {
        if (null == sPresentImage) {
            sPresentImage = new PresentImage();
        }
        return sPresentImage;
    }

    /**
     * Set listener for ImageView.
     * @param context for init load bitmap task.
     * @param listener transfer the bitmap for ImageView.
     */
    public void setListener(Context context,
            RenderingRequestListener listener) {
        LoadBitmapTask.init(context);
        mListener = listener;
    }

    /**
     * Set bitmap while load bitmap task success decode bitmap.
     * @param bitmap the current bitmap.
     * @param uri the image uri.
     */
    public void setBitmap(Bitmap bitmap, String uri) {
        boolean finished = mListener.available(bitmap, uri);
        if (LoadBitmapTask.needRegionDecode() && finished) {
            loadBitmap(uri);
        }
    }

    /**
     * Start load bitmap task.
     * @param uri the bitmap uri.
     */
    public void loadBitmap(String uri) {
        if (uri == null) {
            return;
        }
        stopLoadBitmap();
        mLoadBitmapTask = new LoadBitmapTask(uri);
        mLoadBitmapTask.execute();
    }

    /**
     * Free all resource.
     */
    public void free() {
        if (mLoadBitmapTask != null) {
            mLoadBitmapTask.free();
        }
    }

    /**
     * stop load bitmap task.
     */
    public void stopLoadBitmap() {
        if (mLoadBitmapTask == null) {
            return;
        }
        mLoadBitmapTask.cancel(true);
    }
}
