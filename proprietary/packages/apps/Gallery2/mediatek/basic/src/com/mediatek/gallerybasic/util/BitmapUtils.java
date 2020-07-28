package com.mediatek.gallerybasic.util;

import android.graphics.Bitmap;
import android.graphics.Bitmap.CompressFormat;
import android.graphics.Bitmap.Config;
import android.graphics.BitmapFactory.Options;
import android.graphics.Canvas;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.graphics.Rect;
import android.os.Build;
import android.os.Build.VERSION_CODES;

import java.io.ByteArrayOutputStream;

public class BitmapUtils {
    private static final String TAG = "MtkGallery2/BitmapUtils";
    public static final int BACKGROUND_COLOR = 0xFFFFFFFF;
    private static final int DEFAULT_JPEG_QUALITY = 100;
    private static final int MAX_INTEGER = 65536;

    public static Bitmap resizeAndCropCenter(Bitmap bitmap, int size,
            boolean recycle) {
        if (bitmap == null) {
            Log.d(TAG, "<resizeAndCropCenter> Input bitmap == null, return null");
            return null;
        }
        int w = bitmap.getWidth();
        int h = bitmap.getHeight();
        if (w == size && h == size) {
            return bitmap;
        }

        // scale the image so that the shorter side equals to the target;
        // the longer side will be center-cropped.
        float scale = (float) size / Math.min(w, h);

        Bitmap target = Bitmap.createBitmap(size, size, getConfig(bitmap));
        int width = Math.round(scale * bitmap.getWidth());
        int height = Math.round(scale * bitmap.getHeight());
        Canvas canvas = new Canvas(target);
        canvas.translate((size - width) / 2f, (size - height) / 2f);
        canvas.scale(scale, scale);
        Paint paint = new Paint(Paint.FILTER_BITMAP_FLAG | Paint.DITHER_FLAG);
        canvas.drawBitmap(bitmap, 0, 0, paint);
        if (recycle) {
            bitmap.recycle();
        }
        return target;
    }

    private static Bitmap.Config getConfig(Bitmap bitmap) {
        Bitmap.Config config = bitmap.getConfig();
        if (config == null) {
            config = Bitmap.Config.ARGB_8888;
        }
        return config;
    }

    // This computes a sample size which makes the longer side at least
    // minSideLength long. If that's not possible, return 1.
    public static int computeSampleSizeLarger(int w, int h, int minSideLength) {
        int initialSize = Math.max(w / minSideLength, h / minSideLength);
        if (initialSize <= 1) {
            return 1;
        }
        return initialSize <= 8 ? Utils.prevPowerOf2(initialSize)
                : initialSize / 8 * 8;
    }

    // Find the min x that 1 / x >= scale
    public static int computeSampleSizeLarger(float scale) {
        int initialSize = (int) Math.floor(1f / scale);
        if (initialSize <= 1) {
            return 1;
        }
        return initialSize <= 8 ? Utils.prevPowerOf2(initialSize)
                : initialSize / 8 * 8;
    }

    // Find the max x that 1 / x <= scale.
    public static int computeSampleSize(float scale) {
        Utils.assertTrue(scale > 0);
        int initialSize = Math.max(1, (int) Math.ceil(1 / scale));
        return initialSize <= 8 ? Utils.nextPowerOf2(initialSize)
                : (initialSize + 7) / 8 * 8;
    }

    public static Bitmap resizeBitmapByScale(Bitmap bitmap, float scale,
            boolean recycle) {
        int width = Math.round(bitmap.getWidth() * scale);
        int height = Math.round(bitmap.getHeight() * scale);
        if (width < 1 || height < 1) {
            Log.d(TAG, "<resizeBitmapByScale> scaled width or height < 1, no need to resize");
            return bitmap;
        }
        if (width == bitmap.getWidth() && height == bitmap.getHeight()) {
            return bitmap;
        }
        Bitmap target = Bitmap.createBitmap(width, height, getConfig(bitmap));
        Canvas canvas = new Canvas(target);
        canvas.scale(scale, scale);
        Paint paint = new Paint(Paint.FILTER_BITMAP_FLAG | Paint.DITHER_FLAG);
        canvas.drawBitmap(bitmap, 0, 0, paint);
        if (recycle) {
            bitmap.recycle();
        }
        return target;
    }

    public static Bitmap resizeDownBySideLength(Bitmap bitmap, int maxLength,
            boolean recycle) {
        if (bitmap == null) {
            Log.d(TAG, "<resizeDownBySideLength> Input bitmap == null, return null");
            return null;
        }
        int srcWidth = bitmap.getWidth();
        int srcHeight = bitmap.getHeight();
        float scale = Math.min((float) maxLength / srcWidth, (float) maxLength
                / srcHeight);
        if (scale >= 1.0f) {
            return bitmap;
        }
        return resizeBitmapByScale(bitmap, scale, recycle);
    }

    public static Bitmap replaceBackgroundColor(Bitmap bitmap, boolean recycleInput) {
        return replaceBackgroundColor(bitmap, BACKGROUND_COLOR, recycleInput);
    }

    public static Bitmap clearAlphaValueIfPng(Bitmap bitmap, String mimeType,
                                              boolean recycleInput) {
        if ("image/png".equals(mimeType)) {
            return BitmapUtils.replaceBackgroundColor(bitmap, BACKGROUND_COLOR, recycleInput);
        } else {
            return bitmap;
        }
    }

    // replace Bitmap's back ground with specified color
    public static Bitmap
            replaceBackgroundColor(Bitmap bitmap, int color, boolean recycleInput) {
        if (null == bitmap) {
            Log.i(TAG, "<replaceBackgroundColor> Input bitmap == null, return null");
            return null;
        }
        return replaceBackgroundColor(bitmap, color, recycleInput, new Rect(0, 0, bitmap
                .getWidth(), bitmap.getHeight()));
    }

    public static Bitmap replaceBackgroundColor(Bitmap bitmap, int color,
                                                boolean recycleInput, Rect replaceRect) {
        if (null == bitmap) {
            Log.d(TAG, "<replaceBackgroundColor> Input bitmap == null, return null");
            return null;
        }
        if (bitmap.getConfig() == Bitmap.Config.RGB_565) {
            Log.d(TAG, "<replaceBackgroundColor> no alpha, return");
            return bitmap;
        }
        // Bitmap has alpha channel, and should be replace its background color
        // 1,create a new bitmap with same dimension and ARGB_8888
        if (bitmap.getWidth() <= 0 || bitmap.getHeight() <= 0) {
            Log.w(TAG, "<replaceBackgroundColor> invalid Bitmap dimension");
            return bitmap;
        }
        Bitmap res = null;
        try {
            res = Bitmap.createBitmap(bitmap.getWidth(), bitmap.getHeight(),
                    Bitmap.Config.ARGB_8888);
        } catch (OutOfMemoryError e) {
            Log.e(TAG, "<replaceBackgroundColor> out of memory", e);
            return bitmap;
        }
        // framework handled out-of-memory exception will lead to null res
        if (res == null) {
            return bitmap;
        }

        // 2,create Canvas to encapulate new Bitmap
        Canvas canvas = new Canvas(res);
        // 3,draw background color
        Paint paint = new Paint();
        paint.setColor(color);
        canvas.drawRect(replaceRect, paint);
        // 4,draw original Bitmap on background
        canvas.drawBitmap(bitmap, new Matrix(), null);
        // 5,recycle original Bitmap if needed
        if (recycleInput) {
            bitmap.recycle();
            bitmap = null;
        }
        // 6,return the output Bitmap
        return res;
    }

    public static Bitmap rotateBitmap(Bitmap source, int rotation,
            boolean recycle) {
        if (rotation == 0 || source == null) {
            return source;
        }
        int w = source.getWidth();
        int h = source.getHeight();
        Matrix m = new Matrix();
        m.postRotate(rotation);
        Bitmap bitmap = Bitmap.createBitmap(source, 0, 0, w, h, m, true);
        if (recycle) {
            source.recycle();
        }
        return bitmap;
    }

    public static void setOptionsMutable(Options options) {
        if (Build.VERSION.SDK_INT >= VERSION_CODES.HONEYCOMB) {
            options.inMutable = true;
        }
    }

    public static Bitmap ensureGLCompatibleBitmap(Bitmap bitmap) {
        if (bitmap == null || bitmap.getConfig() != null) {
            return bitmap;
        }
        Bitmap newBitmap = bitmap.copy(Config.ARGB_8888, false);
        bitmap.recycle();
        return newBitmap;
    }

    /**
     * Compress bitmap to byte array in jpeg format.
     *
     * @param bitmap
     *            The bitmap to be compressed
     * @return Return the byte array
     */
    public static byte[] compressToBytes(Bitmap bitmap) {
        return compressToBytes(bitmap, DEFAULT_JPEG_QUALITY);
    }

    /**
     * Compress bitmap to byte array in jpeg format. It's allowed to specify
     * compressing quality.
     *
     * @param bitmap
     *            The bitmap to be compressed
     * @param quality
     *            The quality when compressing
     * @return Return the byte array
     */
    public static byte[] compressToBytes(Bitmap bitmap, int quality) {
        ByteArrayOutputStream baos = new ByteArrayOutputStream(MAX_INTEGER);
        bitmap.compress(CompressFormat.JPEG, quality, baos);
        return baos.toByteArray();
    }

    public static Bitmap resizeAndCropByScale(Bitmap bitmap, int targetWidth,
            int targetHeight, boolean scaleByWidth, boolean recycle) {
        if (bitmap == null) {
            return bitmap;
        }
        int w = bitmap.getWidth();
        int h = bitmap.getHeight();
        if (w == targetWidth && h == targetHeight) {
            return bitmap;
        }

        float scaleX = (float) targetWidth / w;
        float scaleY = (float) targetHeight / h;
        float scale = scaleByWidth ? scaleX : scaleY;

        Bitmap target = Bitmap.createBitmap(targetWidth, targetHeight,
                getConfig(bitmap));
        w = Math.round(scale * bitmap.getWidth());
        h = Math.round(scale * bitmap.getHeight());
        Canvas canvas = new Canvas(target);
        canvas.translate((targetWidth - w) / 2f, (targetHeight - h) / 2f);
        canvas.scale(scale, scale);
        Paint paint = new Paint(Paint.FILTER_BITMAP_FLAG | Paint.DITHER_FLAG);
        canvas.drawBitmap(bitmap, 0, 0, paint);
        if (recycle) {
            bitmap.recycle();
        }
        return target;
    }

    public static Bitmap replaceBackgroundColor(Bitmap bitmap, boolean recycleInput,
                                                Rect replaceRect) {
        return replaceBackgroundColor(bitmap, BACKGROUND_COLOR, recycleInput, replaceRect);
    }
}
