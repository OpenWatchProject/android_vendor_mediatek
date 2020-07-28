package com.mediatek.gallerybasic.util;

import android.content.Context;
import android.graphics.BitmapFactory;
import android.net.Uri;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;

public class DecodeSpecLimitor {
    private static final String TAG = "MtkGallery2/DecodeSpecLimitor";
    public static boolean sIsLowRamDevice = false;

    // GIF: None LCA:20MB, LCA:10MB
    private final static int MAX_GIF_FILE_SIZE = sIsLowRamDevice ? (10 * 1024 * 1024)
            : (20 * 1024 * 1024);
    private final static long MAX_GIF_FRAME_PIXEL_SIZE = (long) (1.5f * 1024 * 1024); // 1.5MB
    private final static String MIME_GIF = "image/gif";

    // BMP & WBMP: NONE-LCA file size < 52MB, LCA file size < 6MB
    private final static int MAX_BMP_FILE_SIZE = sIsLowRamDevice ? (6 * 1024 * 1024)
            : (52 * 1024 * 1024);

    public static boolean isOutOfSpecLimit(long fileSize, int width,
            int height, String mimeType) {
        return isOutOfSpecInteral(fileSize, width, height, mimeType);
    }

    public static boolean isOutOfSpecLimit(Context context, Uri uri) {
        if (context == null || uri == null)
            return false;
        InputStream inputStream = null;
        try {
            inputStream = context.getContentResolver().openInputStream(uri);
        } catch (FileNotFoundException e) {
            return false;
        }
        if (inputStream == null)
            return false;

        // get file size
        int fileSize;
        try {
            fileSize = inputStream.available();
        } catch (IOException e) {
            return false;
        }

        // get MimeType & width & height
        BitmapFactory.Options option = new BitmapFactory.Options();
        option.inJustDecodeBounds = true;
        BitmapFactory.decodeStream(inputStream, null, option);

        // close stream
        try {
            inputStream.close();
        } catch (IOException e) {
        }

        boolean res = isOutOfSpecInteral(fileSize, option.outWidth,
                option.outHeight, option.outMimeType);
        if (res == true) {
            Log.d(TAG, "<isOutOfSpecLimit> uri " + uri
                    + ", out of spec limit");
        }
        return res;
    }

    private static boolean isOutOfSpecInteral(long fileSize, int width,
            int height, String mimeType) {
        return isOutOfGifSpec(fileSize, (long) width * (long) height, mimeType)
                || isOutOfBmpSpec(fileSize, mimeType);
    }

    private static boolean isOutOfGifSpec(long fileSize, long framePixelSize,
            String mimeType) {
        return mimeType != null
                && mimeType.equals(MIME_GIF)
                && (fileSize > MAX_GIF_FILE_SIZE || framePixelSize > MAX_GIF_FRAME_PIXEL_SIZE);
    }

    private static boolean isOutOfBmpSpec(long fileSize, String mimeType) {
        return mimeType != null && mimeType.endsWith("bmp")
                && fileSize > MAX_BMP_FILE_SIZE;
    }
}
