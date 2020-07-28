package com.mediatek.mail.vip.utils;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;

/**
 * Provides static functions to decode bitmaps at the optimal size
 */
public class BitmapUtil {
    private BitmapUtil() {}

    /**
     * Returns Width or Height of the picture, depending on which size is smaller. Doesn't actually
     * decode the picture, so it is pretty efficient to run.
     */
    public static int getSmallerExtentFromBytes(byte[] bytes) {
        final BitmapFactory.Options options = new BitmapFactory.Options();

        // don't actually decode the picture, just return its bounds
        options.inJustDecodeBounds = true;
        BitmapFactory.decodeByteArray(bytes, 0, bytes.length, options);

        // test what the best sample size is
        return Math.min(options.outWidth, options.outHeight);
    }

    /**
     * Finds the optimal sampleSize for loading the picture
     * @param originalSmallerExtent Width or height of the picture, whichever is smaller
     * @param targetExtent Width or height of the target view, whichever is bigger.
     *
     * If either one of the parameters is 0 or smaller, no sampling is applied
     */
    public static int findOptimalSampleSize(int originalSmallerExtent, int targetExtent) {
        // If we don't know sizes, we can't do sampling.
        if (targetExtent < 1) return 1;
        if (originalSmallerExtent < 1) return 1;

        // test what the best sample size is
        int extent = originalSmallerExtent;
        int sampleSize = 1;
        while ((extent >> 1) >= targetExtent) {
            sampleSize <<= 1;
            extent >>= 1;
        }

        return sampleSize;
    }

    /**
     * Decodes the bitmap with the given sample size
     */
    public static Bitmap decodeBitmapFromBytes(byte[] bytes, int sampleSize) {
        final BitmapFactory.Options options;
        if (sampleSize <= 1) {
            options = null;
        } else {
            options = new BitmapFactory.Options();
            options.inSampleSize = sampleSize;
        }
        return BitmapFactory.decodeByteArray(bytes, 0, bytes.length, options);
    }
}
