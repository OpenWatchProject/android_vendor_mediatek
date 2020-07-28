package com.mediatek.gallerybasic.base;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;

/**
 * If you want to support load other image format in filter show, implement this interface.
 */
public interface IFilterShowImageLoader {
    /**
     * Callback when load image in filter show.
     * @param context
     *            The activity context
     * @param uri
     *            The image uri
     * @param o
     *            Decode options
     * @return Decode result
     */
    public Bitmap loadBitmap(Context context, Uri uri, BitmapFactory.Options o);
}
