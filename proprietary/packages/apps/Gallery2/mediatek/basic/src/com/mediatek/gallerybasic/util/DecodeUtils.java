package com.mediatek.gallerybasic.util;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.os.Build;
import android.os.ParcelFileDescriptor;

import java.io.FileNotFoundException;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

public class DecodeUtils {
    private static final String TAG = "MtkGallery2/DecodeUtils";

        public static Bitmap decodeBitmap(String filePath, BitmapFactory.Options options) {
            BitmapUtils.setOptionsMutable(options);
            Bitmap bitmap = BitmapFactory.decodeFile(filePath, options);
            return bitmap;
        }

    public static Bitmap decodeBitmap(Context context, Uri uri, BitmapFactory.Options options) {
        Bitmap bitmap = null;
        ParcelFileDescriptor fd = null;
        try {
            fd = context.getContentResolver()
                    .openFileDescriptor(uri, "r");
            BitmapUtils.setOptionsMutable(options);
            bitmap = BitmapFactory.decodeFileDescriptor(fd.getFileDescriptor(), null, options);
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } finally {
            Utils.closeSilently(fd);
        }
        return bitmap;
    }

    private static Bitmap decode(String filePath, int targetSize) {
        if (filePath == null || filePath.equals("")) {
            Log.d(TAG, "<decode> error args, return null");
            return null;
        }
        Bitmap bitmap;
        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inJustDecodeBounds = true;
        BitmapFactory.decodeFile(filePath, options);
        options.inSampleSize = BitmapUtils.computeSampleSizeLarger(
                options.outWidth, options.outHeight, targetSize);

        options.inJustDecodeBounds = false;
        BitmapUtils.setOptionsMutable(options);
        bitmap = BitmapFactory.decodeFile(filePath, options);
        return bitmap;
    }

    public static Bitmap decodeVideoThumbnail(String filePath, BitmapFactory.Options options) {
        // MediaMetadataRetriever is available on API Level 8
        // but is hidden until API Level 10
        Class<?> clazz = null;
        Object instance = null;
        try {
            clazz = Class.forName("android.media.MediaMetadataRetriever");
            instance = clazz.newInstance();

            Method method = clazz.getMethod("setDataSource", String.class);
            method.invoke(instance, filePath);

            // The method name changes between API Level 9 and 10.
            if (Build.VERSION.SDK_INT <= 9) {
                Bitmap videThumbnail = (Bitmap) clazz.getMethod("captureFrame").invoke(instance);
                if (videThumbnail != null) {
                    options.outWidth = videThumbnail.getWidth();
                    options.outHeight = videThumbnail.getHeight();
                }
                return videThumbnail;
            } else {
                byte[] data = (byte[]) clazz.getMethod("getEmbeddedPicture").invoke(instance);
                if (data != null) {
                    Bitmap bitmap = BitmapFactory.decodeByteArray(data, 0, data.length);
                    if (bitmap != null) {
                        return bitmap;
                    }
                }
                Bitmap videThumbnail = (Bitmap) clazz.getMethod("getFrameAtTime").invoke(instance);
                if (videThumbnail != null) {
                    options.outWidth = videThumbnail.getWidth();
                    options.outHeight = videThumbnail.getHeight();
                }
                return videThumbnail;
            }
        } catch (IllegalArgumentException e) {
            Log.e(TAG, "<decodeVideoThumbnail>", e);
        } catch (InstantiationException e) {
            Log.e(TAG, "<decodeVideoThumbnail>", e);
        } catch (InvocationTargetException e) {
            Log.e(TAG, "<decodeVideoThumbnail>", e);
        } catch (ClassNotFoundException e) {
            Log.e(TAG, "<decodeVideoThumbnail>", e);
        } catch (NoSuchMethodException e) {
            Log.e(TAG, "<decodeVideoThumbnail>", e);
        } catch (IllegalAccessException e) {
            Log.e(TAG, "<decodeVideoThumbnail>", e);
        } finally {
            try {
                if (instance != null) {
                    clazz.getMethod("release").invoke(instance);
                }
            } catch (IllegalAccessException e) {
                Log.e(TAG, "<decodeVideoThumbnail> release", e);
            } catch (IllegalArgumentException e) {
                Log.e(TAG, "<decodeVideoThumbnail> release", e);
            } catch (InvocationTargetException e) {
                Log.e(TAG, "<decodeVideoThumbnail> release", e);
            } catch (NoSuchMethodException e) {
                Log.e(TAG, "<decodeVideoThumbnail> release", e);
            }
        }
        return null;
    }
}
