package com.mediatek.gallerybasic.util;

import android.graphics.Bitmap;
import android.graphics.Bitmap.CompressFormat;
import android.os.Environment;

import com.mediatek.galleryportable.SystemPropertyUtils;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;

public class DebugUtils {
    private static final String TAG = "MtkGallery2/DebugUtils";

    public static final boolean DUMP = SystemPropertyUtils.getInt("Gallery_DUMP", 0) == 1 ? true
            : false;
    public static final boolean TILE = SystemPropertyUtils.getInt("Gallery_TILE", 0) == 1 ? true
            : false;
    public static final boolean DEBUG_PLAY_ENGINE = SystemPropertyUtils.getInt(
            "Gallery_DEBUG_PLAY_ENGINE", 0) == 1 ? true : false;
    public static final boolean DEBUG_THUMBNAIL_PLAY_ENGINE = SystemPropertyUtils.getInt(
            "Gallery_DEBUG_ConstrainedEngine", 0) == 1 ? true : false;
    public static final boolean DEBUG_PLAY_RENDER = SystemPropertyUtils.getInt(
            "Gallery_DEBUG_PLAY_RENDER", 0) == 1 ? true : false;
    public static final boolean DEBUG_POSITION_CONTROLLER = SystemPropertyUtils.getInt(
            "Gallery_DEBUG_PC", 0) == 1 ? true : false;
    public static final boolean DEBUG_HIGH_QUALITY_SCREENAIL = SystemPropertyUtils.getInt(
            "Gallery_DEBUG_HQS", 0) == 1 ? true : false;
    public static final boolean DEBUG_RENDER = SystemPropertyUtils.getInt(
            "Gallery_DEBUG_RENDER", 0) == 1 ? true : false;

    private static final String BITAMP_DUMP_FOLDER = "/.GalleryIssue/";
    public static final String BITMAP_DUMP_PATH = Environment
            .getExternalStorageDirectory().toString()
            + BITAMP_DUMP_FOLDER;
    private static final int DEFAULT_COMPRESS_QUALITY = 100;

    public static void dumpBitmap(Bitmap bitmap, String fileName) {
        fileName = fileName + ".png";
        File galleryIssueFilePath = new File(BITMAP_DUMP_PATH);
        if (!galleryIssueFilePath.exists()) {
            Log.d(TAG, "<dumpBitmap> create  galleryIssueFilePath");
            galleryIssueFilePath.mkdir();
        }
        File file = new File(BITMAP_DUMP_PATH, fileName);
        OutputStream fos = null;
        try {
            fos = new FileOutputStream(file);
            bitmap.compress(CompressFormat.PNG, DEFAULT_COMPRESS_QUALITY, fos);
        } catch (IOException e) {
            e.printStackTrace();
            Log.d(TAG, "<dumpBitmap> IOException", e.getCause());
        } finally {
            try {
                if (fos != null) {
                    fos.close();
                }
            } catch (IOException e) {
                e.printStackTrace();
                Log.d(TAG, "<dumpBitmap> close FileOutputStream", e.getCause());
            }
        }
    }
}
