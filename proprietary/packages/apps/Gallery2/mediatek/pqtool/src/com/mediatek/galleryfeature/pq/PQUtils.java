package com.mediatek.galleryfeature.pq;

import android.content.ContentResolver;
import android.content.Context;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.net.Uri;
import android.os.Build;
import android.provider.MediaStore.Images.ImageColumns;
import android.util.DisplayMetrics;
import android.view.WindowManager;

import com.mediatek.gallerybasic.util.Log;

import java.io.FileDescriptor;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.lang.reflect.Field;

/**
 * Utils for PQ.
 */
public class PQUtils {
    private static final String TAG = "MtkGallery2/PQUtils";
    private static final int HIGH_RESOLUTION_THRESHOLD = 1920;
    private static final int LOG_THRESHOLD = 31;
    private static final int STEP_INSAMPLE_SIZE = 8;
    public static boolean mHasInitializedField;
    public static Field mField = null;
    /**
     * Call back while query DB.
     */
    interface ContentResolverQueryCallback {
        void onCursorResult(Cursor cursor);
    }

    /**
     * set inPostProc.
     * @param option The decode option.
     */

    public static void initOptions(BitmapFactory.Options option) {
        if (!mHasInitializedField) {
            try {
                Class<? extends BitmapFactory.Options> clazz = option.getClass();
                mField = clazz.getField("inPostProc");
            } catch (NoSuchFieldException e1) {
                Log.e(TAG, "NoSuchFieldException: " + e1);
            }
            mHasInitializedField = true;
        }
        if (mHasInitializedField) {
            if (mField != null) {
                try {
                    mField.set(option, true);
                    Log.v(TAG, "<initOptions> inPostPro = " + mField.getBoolean(option));
                } catch (IllegalAccessException e) {
                    Log.e(TAG, "IllegalAccessException: " + e);
                } catch (IllegalArgumentException e) {
                    Log.e(TAG, "IllegalArgumentException: " + e);
                }
            }
        }
    }

    /**
     * Calculate inSample size for decode bitmap.
     * @param context
     *            for get input stream.
     * @param uri
     *            the uri of bitmap.
     * @param targetSize
     *            the bitmap target size.
     * @return the inSampleSize for decode bitmap.
     */
    public static int calculateInSampleSize(Context context, String uri, int targetSize) {
        FileDescriptor fd = null;
        FileInputStream fis = null;
        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inJustDecodeBounds = true;
        try {
            fis = getFileInputStream(context, uri);
            if (fis != null) {
                fd = fis.getFD();
                if (fd != null) {
                    BitmapFactory.decodeFileDescriptor(fd, null, options);
                }
            }
        } catch (FileNotFoundException e) {
            Log.e(TAG, "<caculateInSampleSize>bitmapfactory decodestream fail");
        } catch (IOException e) {
            Log.e(TAG, "<caculateInSampleSize>bitmapfactory decodestream fail");
        } finally {
            if (fis != null) {
                try {
                    fis.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
        float scale = 1;
        if (options.outWidth > 0 && options.outHeight > 0) {
            scale = (float) targetSize / Math.max(options.outWidth, options.outHeight);
        }
        options.inSampleSize = computeSampleSizeLarger(scale);
        Log.d(TAG, "<caculateInSampleSize> options.inSampleSize=" + options.inSampleSize
                + " width=" + options.outWidth + " height=" + options.outHeight
                + "targetSize=" + targetSize);
        return options.inSampleSize;
    }

    /**
     * Get rotation by query DB.
     * @param context
     *            query DB by the context.
     * @param uri
     *            the image uri.
     * @return the rotation of the image.
     */
    public static int getRotation(Context context, String uri) {
        final int[] rotation = new int[1];
        Log.d(TAG, "<getRotation> Uri.parse(mUri)==" + uri);
        if (ContentResolver.SCHEME_CONTENT.equals(Uri.parse(uri).getScheme())) {
            querySource(context, Uri.parse(uri), new String[] {
                ImageColumns.ORIENTATION
            }, new ContentResolverQueryCallback() {
                public void onCursorResult(Cursor cursor) {
                    rotation[0] = cursor.getInt(0);
                }
            });
        }
        return rotation[0];
    }

    /**
     * Get file InputStream for decode bitmap.
     * @param context
     *            query DB by the context.
     * @param uri
     *            the image uri.
     * @return the FileInputStream object of the image file.
     */
    public static FileInputStream getFileInputStream(Context context, String uri) {
        FileInputStream fis = null;
        try {
            final String[] fullPath = new String[1];
            Log.d(TAG, "<getFileInputStream> Uri.parse(mUri)==" + uri);
            if (ContentResolver.SCHEME_CONTENT.equals(Uri.parse(uri).getScheme())) {
                querySource(context, Uri.parse(uri), new String[] {
                        ImageColumns.DATA, ImageColumns.ORIENTATION
                }, new ContentResolverQueryCallback() {
                    public void onCursorResult(Cursor cursor) {
                        fullPath[0] = cursor.getString(0);
                    }
                });
            } else {
                fullPath[0] = uri;
            }
            Log.d(TAG, "<getFileInputStream> fullPath[0]=" + fullPath[0]);
            fis = new FileInputStream(fullPath[0]);
        } catch (FileNotFoundException e) {
            Log.e(TAG, " <getFileInputStream> FileNotFoundException!");
        }
        return fis;
    }

    private static void querySource(Context context, Uri sourceUri, String[] projection,
                                    ContentResolverQueryCallback callback) {
        ContentResolver contentResolver = context.getContentResolver();
        querySourceFromContentResolver(contentResolver, sourceUri, projection, callback);
    }

    private static void querySourceFromContentResolver(ContentResolver contentResolver,
                                                       Uri sourceUri, String[] projection,
                                                       ContentResolverQueryCallback callback) {
        Cursor cursor = null;
        try {
            cursor = contentResolver.query(sourceUri, projection, null, null, null);
            if ((cursor != null) && cursor.moveToNext()) {
                callback.onCursorResult(cursor);
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
    }

    /**
     * Check the image whether support RegionDecode or no.
     * @param mimeType
     *            the image type.
     * @return whether support RegionDecode or no.
     */
    public static boolean isSupportedByRegionDecoder(String mimeType) {
        if (mimeType == null) {
            return false;
        }
        mimeType = mimeType.toLowerCase();
        return mimeType.startsWith("image/") && (!mimeType.equals("image/gif"));
    }

    /**
     * resize bitmap by scale for display.
     * @param bitmap
     *            resize the bitmap.
     * @param scale
     *            the resize scale.
     * @param recycle
     *            whether should recycle the origin bitmap or not.
     * @return return the resized bitmap.
     */
    public static Bitmap resizeBitmapByScale(Bitmap bitmap, float scale, boolean recycle) {
        int width = Math.round(bitmap.getWidth() * scale);
        int height = Math.round(bitmap.getHeight() * scale);
        if (width < 1 || height < 1) {
            Log.d(TAG, "<resizeBitmapByScale>scaled width or height < 1,"
                    + " no need to resize");
            return bitmap;
        }
        if (width == bitmap.getWidth() && height == bitmap.getHeight()) {
            return bitmap;
        }
        Bitmap target = Bitmap.createBitmap(width, height, PQUtils.getConfig(bitmap));
        Canvas canvas = new Canvas(target);
        canvas.scale(scale, scale);
        Paint paint = new Paint(Paint.FILTER_BITMAP_FLAG | Paint.DITHER_FLAG);
        canvas.drawBitmap(bitmap, 0, 0, paint);
        if (recycle) {
            bitmap.recycle();
        }
        return target;
    }

    /**
     * Calculate log.
     * @param value
     *            the float value.
     * @return return the log value.
     */
    public static int floorLog2(float value) {
        int i;
        for (i = 0; i < LOG_THRESHOLD; i++) {
            if ((1 << i) > value) {
                break;
            }
        }
        return i - 1;
    }

    /**
     * clamp x value.
     * @param x
     *            the x value.
     * @param min
     *            the min value.
     * @param max
     *            the max value.
     * @return the new value.
     */
    public static int clamp(int x, int min, int max) {
        if (x > max) {
            return max;
        }
        if (x < min) {
            return min;
        }
        return x;
    }

    /**
     * Calculate the inSample size.
     * @param scale
     *            the scale.
     * @return the inSample size.
     */
    public static int computeSampleSizeLarger(float scale) {
        int initialSize = (int) Math.floor(1f / scale);
        if (initialSize <= 1) {
            return 1;
        }
        return initialSize <= STEP_INSAMPLE_SIZE ? prevPowerOf2(initialSize) : initialSize
                / STEP_INSAMPLE_SIZE * STEP_INSAMPLE_SIZE;
    }

    /**
     * Calculate the level count by image width and screen.
     * @param imageWidth
     *            the image width.
     * @param screenWidth
     *            current screen width.
     * @return the level of bitmap tile.
     */
    public static int calculateLevelCount(int imageWidth, int screenWidth) {
        return Math.max(0, ceilLog2((float) imageWidth / screenWidth));
    }

    /**
     * Rotate bitmap.
     * @param source
     *            the bitmap that should rotation.
     * @param rotation
     *            the angle.
     * @param recycle
     *            whether should recycle the origin bitmap or not.
     * @return the bitmap that has new rotation.
     */
    public static Bitmap rotateBitmap(Bitmap source, int rotation, boolean recycle) {
        if (rotation == 0) {
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

    /**
     * Check if the bitmap is high resolution.
     * @param context
     *            for get WindowManager.
     * @return whether is high resolution or not.
     */
    public static boolean isHighResolution(Context context) {
        DisplayMetrics metrics = new DisplayMetrics();
        WindowManager wm = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR1) {
            wm.getDefaultDisplay().getRealMetrics(metrics);
        } else {
            wm.getDefaultDisplay().getMetrics(metrics);
        }
        return metrics.heightPixels >= HIGH_RESOLUTION_THRESHOLD
                || metrics.widthPixels >= HIGH_RESOLUTION_THRESHOLD;
    }

    private static int ceilLog2(float value) {
        int i;
        for (i = 0; i < LOG_THRESHOLD; i++) {
            if ((1 << i) >= value) {
                break;
            }
        }
        return i;
    }

    private static int prevPowerOf2(int n) {
        if (n <= 0) {
            throw new IllegalArgumentException();
        }
        return Integer.highestOneBit(n);
    }

    private static Bitmap.Config getConfig(Bitmap bitmap) {
        Bitmap.Config config = bitmap.getConfig();
        if (config == null) {
            config = Bitmap.Config.ARGB_8888;
        }
        return config;
    }
}
