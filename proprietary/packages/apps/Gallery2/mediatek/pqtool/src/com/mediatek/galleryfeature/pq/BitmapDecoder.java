package com.mediatek.galleryfeature.pq;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;

import com.mediatek.gallerybasic.util.Log;

import java.io.FileDescriptor;
import java.io.FileInputStream;
import java.io.IOException;

/**
 * Decode whole bitmap.
 */
public class BitmapDecoder {
    private static final String TAG = "MtkGallery2/BitmapDecoder";
    private static final float SCALE_THRESHOLD = 0.5f;
    private String mPqUri;
    private Context mContext;
    private int mTargetSize;
    private BitmapFactory.Options mOptions = new BitmapFactory.Options();
    private int mSampleSize;

    /**
     * Constructor.
     * @param context for calculate sample and decode bitmap.
     * @param uri the bitmap uri.
     * @param targetSize the bitmap target size.
     */
    public BitmapDecoder(Context context, String uri, int targetSize) {
        mPqUri = uri;
        mContext = context;
        this.mTargetSize = targetSize;
        mSampleSize = PQUtils.calculateInSampleSize(context, uri, targetSize);
    }

    /**
     * Decode whole bitmap for ImageView.
     * @return the whole bitmap.
     */
    public Bitmap decodeScreenNailBitmap() {
        FileDescriptor fd = null;
        FileInputStream fis = null;
        Bitmap mBitmap = null;
        mOptions.inPreferredConfig = Bitmap.Config.ARGB_8888;
        mOptions.inSampleSize = mSampleSize;
        PQUtils.initOptions(mOptions);
        try {
            fis = PQUtils.getFileInputStream(mContext, mPqUri);
            if (fis != null) {
                fd = fis.getFD();
                if (fd != null) {
                    mBitmap = BitmapFactory.decodeFileDescriptor(fd, null,
                            mOptions);
                }
            }
        } catch (IOException e) {
            Log.w(TAG, "<decodeScreenNailBitmap> exception occur, "
                            + e.getMessage());
        } finally {
            if (fis != null) {
                try {
                    fis.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
            if (mBitmap != null) {
                float scale = (float) mTargetSize
                        / Math.max(mBitmap.getWidth(), mBitmap.getHeight());
                if (scale <= SCALE_THRESHOLD) {
                    return PQUtils.resizeBitmapByScale(mBitmap, scale, true);
                }
            }
        }
        return mBitmap;
    }
}
