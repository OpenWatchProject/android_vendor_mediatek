package com.mediatek.gallerybasic.base;

import android.graphics.BitmapFactory;

/**
 * If you want to change the bitmap options in decode flow, implement this interface. It provides
 * two chances to change the options: <br>
 * 1. before decode thumbnail <br>
 * 2. before decode region
 */
public interface IDecodeOptionsProcessor {
    /**
     * Process bitmap options before decode thumbnail.
     * @param mimeType
     *            The image type
     * @param options
     *            The decode options
     * @return If the decode options is modified
     */
    public boolean processThumbDecodeOptions(String mimeType, BitmapFactory.Options options);

    /**
     * Process bitmap options before decode region.
     * @param mimeType
     *            The image type
     * @param options
     *            The decode options
     * @return If the decode options is modified
     */
    public boolean processRegionDecodeOptions(String mimeType, BitmapFactory.Options options);
}
