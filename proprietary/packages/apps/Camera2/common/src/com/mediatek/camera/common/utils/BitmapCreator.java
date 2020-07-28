/*
 * Copyright Statement:
 *
 *   This software/firmware and related documentation ("MediaTek Software") are
 *   protected under relevant copyright laws. The information contained herein is
 *   confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 *   the prior written permission of MediaTek inc. and/or its licensors, any
 *   reproduction, modification, use or disclosure of MediaTek Software, and
 *   information contained herein, in whole or in part, shall be strictly
 *   prohibited.
 *
 *   MediaTek Inc. (C) 2016. All rights reserved.
 *
 *   BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *   THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *   RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 *   ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 *   WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 *   WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 *   NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 *   RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *   INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 *   TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 *   RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 *   OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 *   SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 *   RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 *   STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 *   ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 *   RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 *   MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 *   CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *   The following software/firmware and/or related documentation ("MediaTek
 *   Software") have been modified by MediaTek Inc. All revisions are subject to
 *   any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.camera.common.utils;

import android.content.ContentResolver;
import android.content.ContentUris;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Matrix;
import android.graphics.Rect;
import android.graphics.YuvImage;
import android.media.CameraProfile;
import android.media.MediaMetadataRetriever;
import android.net.Uri;
import android.os.Build;
import android.os.Environment;
import android.provider.MediaStore;
import android.provider.MediaStore.Files.FileColumns;
import android.provider.MediaStore.Images;
import android.provider.MediaStore.Images.ImageColumns;
import android.provider.MediaStore.Video;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil.Tag;
import com.mediatek.camera.common.exif.ExifInterface;
import com.mediatek.camera.portability.SystemProperties;
import com.mediatek.camera.portability.storage.StorageManagerExt;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileDescriptor;
import java.io.FileOutputStream;
import java.io.IOException;
import java.lang.reflect.Method;
import java.util.Locale;

/**
 * create bitmap for image or video.
 */
public class BitmapCreator {
    private static final Tag TAG = new Tag(BitmapCreator.class.getSimpleName());
    private static final String FOLDER_PATH = "/" + Environment.DIRECTORY_DCIM + "/Camera";
    private static final String DCIM_CAMERA_FOLDER_ABSOLUTE_PATH =
            Environment.getExternalStoragePublicDirectory(
                    Environment.DIRECTORY_DCIM).toString() + "/Camera";
    private static final String CLASS_NAME = "com.mediatek.storage.StorageManagerEx";
    private static final String METHOD_NAME = "getDefaultPath";

    private static Uri sUri = null;
    private static String sMountPoint;
    private static Method sGetDefaultPath;

    static {
        try {
            Class cls = Class.forName(CLASS_NAME);
            if (cls != null) {
                sGetDefaultPath = cls.getDeclaredMethod(METHOD_NAME);
            }
            if (sGetDefaultPath != null) {
                sGetDefaultPath.setAccessible(true);
            }
        } catch (NoSuchMethodException e) {
            LogHelper.e(TAG, "NoSuchMethodException: " + METHOD_NAME);
        } catch (ClassNotFoundException e) {
            LogHelper.e(TAG, "ClassNotFoundException: " + CLASS_NAME);
        }
    }

    /**
     * media data structure needed by thumbnail.
     */
    private static class Media {
        public final long id;
        public final int orientation;
        public final long dateTaken;
        public final Uri uri;
        public final int mediaType;
        public final String filePath;

        public Media(long id, int orientation, long dateTaken, Uri uri,
                int mediaType, String filePath) {
            this.id = id;
            this.orientation = orientation;
            this.dateTaken = dateTaken;
            this.uri = uri;
            this.mediaType = mediaType;
            this.filePath = filePath;
        }

        @Override
        public String toString() {
            return new StringBuilder().append("Media(id=").append(id)
                    .append(", orientation=").append(orientation)
                    .append(", dateTaken=").append(dateTaken).append(", uri=")
                    .append(uri).append(", mediaType=").append(mediaType)
                    .append(", filePath=").append(filePath).append(")")
                    .toString();
        }
    }

    /**
     * decode bitmap from the jpeg data, will decode the full image.
     * @param jpeg
     *            the image data.
     * @param targetWidth
     *            the view width where the bitmap shows.
     * @return the bitmap decode from jpeg data. if jpeg data is null or out of
     *         memory when decode, will return null.
     */
    public static Bitmap decodeBitmapFromJpeg(byte[] jpeg, int targetWidth) {
        if (jpeg != null) {
            ExifInterface exif = getExif(jpeg);
            int orientation = getJpegOrientation(exif);
            int jpegWidth = getJpegWidth(exif);
            int ratio = (int) Math.ceil((double) jpegWidth / targetWidth);
            int inSampleSize = Integer.highestOneBit(ratio);
            BitmapFactory.Options options = new BitmapFactory.Options();
            options.inSampleSize = inSampleSize;
            try {
                Bitmap bitmap = BitmapFactory.decodeByteArray(jpeg, 0,
                        jpeg.length, options);
                return rotateBitmap(bitmap, orientation);
            } catch (OutOfMemoryError e) {
                LogHelper.e(TAG, "decodeBitmapFromJpeg fail", e);
                return null;
            }
        }
        return null;
    }

    /**
     * create bitmap from the jpeg data.
     * @param jpeg
     *            the image data.
     * @param targetWidth
     *            the view width where the bitmap shows.
     * @return the bitmap decode from jpeg data. if jpeg data is null or out of
     *         memory when decode, will return null.
     */
    public static Bitmap createBitmapFromJpeg(byte[] jpeg, int targetWidth) {
        LogHelper.d(TAG, "[createBitmapFromJpeg] jpeg = " + jpeg
                + ", targetWidth = " + targetWidth);
        if (jpeg != null) {
            ExifInterface exif = getExif(jpeg);
            int orientation = getJpegOrientation(exif);
            if (exif != null && exif.hasThumbnail() && exif.getThumbnailBitmap() != null) {
                LogHelper.d(TAG, "create bitmap from exif thumbnail");
                return rotateBitmap(exif.getThumbnailBitmap(), orientation);
            } else {
                int jpegWidth = getJpegWidth(exif);
                int ratio = (int) Math.ceil((double) jpegWidth / targetWidth);
                int inSampleSize = Integer.highestOneBit(ratio);
                BitmapFactory.Options options = new BitmapFactory.Options();
                options.inSampleSize = inSampleSize;
                try {
                    Bitmap bitmap = BitmapFactory.decodeByteArray(jpeg, 0,
                            jpeg.length, options);
                    LogHelper.d(TAG, "[createBitmapFromJpeg] end");
                    return rotateBitmap(bitmap, orientation);
                } catch (OutOfMemoryError e) {
                    LogHelper.e(TAG, "createBitmapFromJpeg fail", e);
                    return null;
                }
            }
        }
        return null;
    }

    public static Bitmap createBitmapFromJpegWithoutExif(byte[] jpeg, int jpegWidth,
                                                         int targetWidth) {
        LogHelper.d(TAG, "[createBitmapFromJpegWithoutExif] jpeg = " + jpeg
                        + ", jpegWidth = " + jpegWidth + ", targetWidth = " + targetWidth);
        if (jpeg == null) {
            return null;
        }

        int ratio = (int) Math.ceil((double) jpegWidth / targetWidth);
        int inSampleSize = Integer.highestOneBit(ratio);
        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inSampleSize = inSampleSize;

        try {
            Bitmap bitmap = BitmapFactory.decodeByteArray(jpeg, 0, jpeg.length, options);
            return bitmap;
        } catch (OutOfMemoryError e) {
            LogHelper.e(TAG, "[createBitmapFromJpegWithoutExif] OutOfMemoryError", e);
            return null;
        }
    }

    /**
     * create bitmap from the YUV data.
     * @param yuvData
     *            the YUV data.
     * @param targetWidth
     *            the view width where the bitmap shows.
     * @param yuvWidth
     *            the width of YUV image.
     * @param yuvHeight
     *            the height of YUV image.
     * @param orientation
     *            the orientation of YUV image.
     * @param imageFormat
     *            the image format of YUV image, must be NV21 OR YUY2.
     * @return the bitmap decode from YUV data.
     */
    public static Bitmap createBitmapFromYuv(byte[] yuvData, int imageFormat,
            int yuvWidth, int yuvHeight, int targetWidth, int orientation) {
        LogHelper.d(TAG, "[createBitmapFromYuv] yuvData = " + yuvData
                + ", yuvWidth = " + yuvWidth + ", yuvHeight = " + yuvHeight
                + ", orientation = " + orientation + ", imageFormat = "
                + imageFormat);
        if (isNeedDumpYuv()) {
            dumpYuv("/sdcard/postView.yuv", yuvData);
        }
        if (yuvData != null) {
            byte[] jpeg = covertYuvDataToJpeg(yuvData, imageFormat, yuvWidth,
                    yuvHeight);
            int ratio = (int) Math.ceil((double) Math.min(yuvWidth, yuvHeight)
                    / targetWidth);
            int inSampleSize = Integer.highestOneBit(ratio);
            BitmapFactory.Options options = new BitmapFactory.Options();
            options.inSampleSize = inSampleSize;
            try {
                Bitmap bitmap = BitmapFactory.decodeByteArray(jpeg, 0,
                        jpeg.length, options);
                LogHelper.d(TAG, "[createBitmapFromYuv] end");
                return rotateBitmap(bitmap, orientation);
            } catch (OutOfMemoryError e) {
                LogHelper.e(TAG, "createBitmapFromYuv fail", e);
                return null;
            }
        }
        return null;
    }

    /**
     * create bitmap from a video.
     * @param filePath
     *            the video saving path.
     * @param targetWidth
     *            the view width where the bitmap shows.
     * @return the bitmap decode from a video. maybe return null if the frame
     *         get from MediaMetadataRetriever is null.
     */
    public static Bitmap createBitmapFromVideo(String filePath, int targetWidth) {
        return createBitmapFromVideo(filePath, null, targetWidth);
    }

    /**
     * create bitmap from a video.
     * @param fd
     *            the FileDescriptor for the video.
     * @param targetWidth
     *            the view width where the bitmap shows.
     * @return the bitmap decode from a video. maybe return null if the frame
     *         get from MediaMetadataRetriever is null.
     */
    public static Bitmap createBitmapFromVideo(FileDescriptor fd, int targetWidth) {
        return createBitmapFromVideo(null, fd, targetWidth);
    }

    /**
     * get last bitmap from data base.
     * @param resolver
     *            the current contentResolver
     * @return the last bitmap after query the data base.
     */
    public static Bitmap getLastBitmapFromDatabase(ContentResolver resolver) {
        LogHelper.d(TAG, "getLastBitmapFromDatabase() begin.");
        // TODO: if storage is not ready, need return THUMBNAIL_NOT_FOUND;

        Uri baseUri = MediaStore.Files.getContentUri("external");
        Uri query = baseUri.buildUpon().appendQueryParameter("limit", "1")
                .build();
        String[] projection = new String[] { ImageColumns._ID, // image and
                                                               // video
                                                               // shared the _ID
                ImageColumns.ORIENTATION, // image use ORIENTATION
                ImageColumns.DATE_TAKEN, // image and video shared DATE_TAKEN
                ImageColumns.DATA, // video use DATA
                FileColumns.MEDIA_TYPE,
        };
        String selection = "((" + FileColumns.MEDIA_TYPE + "="
                + Integer.toString(FileColumns.MEDIA_TYPE_IMAGE) + " OR "
                + FileColumns.MEDIA_TYPE + "="
                + Integer.toString(FileColumns.MEDIA_TYPE_VIDEO) + " ) AND "
                + ImageColumns.BUCKET_ID + '=' + getBucketId() + ")";
        String order = ImageColumns.DATE_TAKEN + " DESC," + ImageColumns._ID
                + " DESC";

        Cursor cursor = null;
        Media media = null;
        long id = -1;
        int orientation = 0;
        long dateTaken = 0;
        String filePath = null;
        try {
            cursor = resolver.query(query, projection, selection, null, order);
            if (cursor != null && cursor.moveToFirst()) {
                id = cursor.getLong(0);
                orientation = cursor.getInt(1);
                dateTaken = cursor.getLong(2);
                filePath = cursor.getString(3);
                media = new Media(id, orientation, dateTaken,
                        ContentUris.withAppendedId(baseUri, id),
                        cursor.getInt(4), filePath);
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        LogHelper.d(TAG, "getLastBitmapFromDatabase() media=" + media);
        if (media == null) {
            sUri = null;
            return null;
        }
        Bitmap bitmap = null;
        orientation = media.orientation;
        try {
            if (media.mediaType == FileColumns.MEDIA_TYPE_IMAGE) {
                ExifInterface exif = getExif(media.filePath);
                if (exif != null && exif.hasThumbnail() && exif.getThumbnailBitmap() != null) {
                    LogHelper.d(TAG, "get bitmap from exif thumbnail");
                    bitmap = exif.getThumbnailBitmap();
                } else {
                    bitmap = Images.Thumbnails.getThumbnail(resolver, media.id,
                            Images.Thumbnails.MINI_KIND, null);
                }
                sUri = ContentUris.withAppendedId(
                        MediaStore.Images.Media.getContentUri("external"),
                        media.id);
            } else if (media.mediaType == FileColumns.MEDIA_TYPE_VIDEO) {
                // For Video type, the thumbnail is rotated, so AP don't care
                // the orientation.
                orientation = 0;
                bitmap = Video.Thumbnails.getThumbnail(resolver, media.id,
                        Video.Thumbnails.MINI_KIND, null);
                sUri = ContentUris.withAppendedId(
                        MediaStore.Video.Media.getContentUri("external"),
                        media.id);
            }
            return rotateBitmap(bitmap, orientation);
        } catch (OutOfMemoryError e) {
            LogHelper.e(TAG, "getThumbnail fail", e);
        }
        LogHelper.d(TAG, "Quit getLastBitmap");
        return null;
    }

    /**
     * get the latest file's uri after query camera folder.
     * @return The uri of the latest file.
     */
    public static Uri getUriAfterQueryDb() {
        return sUri;
    }

    /**
     * Encode YUV to jpeg, and crop it.
     * @param data the yuv data.
     * @param imageFormat the yuv format.
     * @param yuvWidth the yuv width.
     * @param yuvHeight the yuv height.
     * @return the jpeg data.
     */
    public static byte[] covertYuvDataToJpeg(byte[] data, int imageFormat,
            int yuvWidth, int yuvHeight) {
        byte[] jpeg;
        Rect rect = new Rect(0, 0, yuvWidth, yuvHeight);
        YuvImage yuvImg = new YuvImage(data, imageFormat, yuvWidth, yuvHeight,
                null);
        ByteArrayOutputStream outputstream = new ByteArrayOutputStream();
        int jpegQuality = CameraProfile
                .getJpegEncodingQualityParameter(CameraProfile.QUALITY_HIGH);
        yuvImg.compressToJpeg(rect, jpegQuality, outputstream);
        jpeg = outputstream.toByteArray();
        return jpeg;
    }

    private static Bitmap rotateBitmap(Bitmap bitmap, int orientation) {
        if (orientation != 0) {
            // We only rotate the thumbnail once even if we get OOM.
            Matrix m = new Matrix();
            m.setRotate(orientation, bitmap.getWidth() / 2,
                    bitmap.getHeight() / 2);
            try {
                Bitmap rotated = Bitmap.createBitmap(bitmap, 0, 0,
                        bitmap.getWidth(), bitmap.getHeight(), m, true);
                return rotated;
            } catch (IllegalArgumentException t) {
                LogHelper.w(TAG, "Failed to rotate bitmap", t);
            }
        }
        return bitmap;
    }

    private static ExifInterface getExif(byte[] jpegData) {
        if (jpegData != null) {
            ExifInterface exif = new ExifInterface();
            try {
                exif.readExif(jpegData);
            } catch (IOException e) {
                LogHelper.w(TAG, "Failed to read EXIF data", e);
            }
            return exif;
        }
        LogHelper.w(TAG, "JPEG data is null, can not get exif");
        return null;
    }

    private static ExifInterface getExif(String filePath) {
        if (filePath != null) {
            ExifInterface exif = new ExifInterface();
            try {
                exif.readExif(filePath);
            } catch (IOException e) {
                LogHelper.w(TAG, "Failed to read EXIF data", e);
            }
            return exif;
        }
        LogHelper.w(TAG, "filePath is null, can not get exif");
        return null;
    }

    // Returns the degrees in clockwise. Values are 0, 90, 180, or 270.
    private static int getJpegOrientation(ExifInterface exif) {
        if (exif != null) {
            Integer val = exif.getTagIntValue(ExifInterface.TAG_ORIENTATION);
            if (val == null) {
                return 0;
            } else {
                return ExifInterface.getRotationForOrientationValue(val.shortValue());
            }
        }
        LogHelper.w(TAG, "exif is null, can not get JpegOrientation");
        return 0;
    }

    private static int getJpegWidth(ExifInterface exif) {
        if (exif != null) {
            int width = 0;
            int height = 0;
            Integer widthValue = exif.getTagIntValue(ExifInterface.TAG_IMAGE_WIDTH);
            if (widthValue != null) {
                width = widthValue.intValue();
            }
            Integer heightValue = exif.getTagIntValue(ExifInterface.TAG_IMAGE_LENGTH);
            if (heightValue != null) {
                height = heightValue.intValue();
            }
            return Math.min(width, height);
        }
        LogHelper.w(TAG, "exif is null, can not get JpegWidth");
        return 0;
    }

    private static Bitmap createBitmapFromVideo(String filePath,
            FileDescriptor fd, int targetWidth) {
        Bitmap bitmap = null;
        LogHelper.d(TAG, "[createBitmapFromVideo] filePath = " + filePath
                + ", targetWidth = " + targetWidth);
        MediaMetadataRetriever retriever = new MediaMetadataRetriever();
        try {
            if (filePath != null) {
                retriever.setDataSource(filePath);
            } else {
                retriever.setDataSource(fd);
            }
            // -1 means any frame that the implementation considers as
            // representative may be returned.
            bitmap = retriever.getFrameAtTime(-1);
        } catch (IllegalArgumentException ex) {
            // Assume this is a corrupt video file
            ex.printStackTrace();
        } catch (RuntimeException ex) {
            // Assume this is a corrupt video file.
            ex.printStackTrace();
        } finally {
            try {
                retriever.release();
            } catch (RuntimeException ex) {
                // Ignore failures while cleaning up.
                ex.printStackTrace();
            }
        }
        if (bitmap == null) {
            return null;
        }

        // Scale down the bitmap if it is bigger than we need.
        int width = bitmap.getWidth();
        int height = bitmap.getHeight();
        LogHelper.v(TAG, "[createBitmapFromVideo] bitmap = " + width + "x" + height);
        if (width > targetWidth) {
            float scale = (float) targetWidth / width;
            int w = Math.round(scale * width);
            int h = Math.round(scale * height);
            LogHelper.v(TAG, "[createBitmapFromVideo] w = " + w + "h" + h);
            bitmap = Bitmap.createScaledBitmap(bitmap, w, h, true);
        }
        return bitmap;
    }

    private static boolean isNeedDumpYuv() {
        boolean enable = SystemProperties.getInt(
                "vendor.debug.thumbnailFromYuv.enable", 0) == 1 ? true : false;
        LogHelper.d(TAG, "[isNeedDumpYuv] return :" + enable);
        return enable;
    }

    private static void dumpYuv(String filePath, byte[] data) {
        FileOutputStream out = null;
        try {
            LogHelper.d(TAG, "[dumpYuv] begin");
            out = new FileOutputStream(filePath);
            out.write(data);
            out.close();
        } catch (IOException e) {
            LogHelper.e(TAG, "[dumpYuv]Failed to write image,ex:", e);
        } finally {
            if (out != null) {
                try {
                    out.close();
                } catch (IOException e) {
                    LogHelper.e(TAG, "[dumpYuv]IOException:", e);
                }
            }
        }
        LogHelper.d(TAG, "[dumpYuv] end");
    }

    private static String getBucketId() {
        String directory = getFileDirectory();
        LogHelper.d(TAG, "getBucketId directory = " + directory);
        return String.valueOf(directory.toLowerCase(Locale.ENGLISH).hashCode());
    }

    /**
     * Get file directory for storage user.
     * @return file path
     * For mediatek platform and sdk version is bigger than M
     * camera may be have the permission to write data to sd card ,so in this
     * case if default storage is sd card will return sd card path for camera
     * to use,otherwise will follow google design.
     */
    private static String getFileDirectory() {
        if (isExtendStorageCanUsed()) {
            return sMountPoint + FOLDER_PATH;
        } else {
            return DCIM_CAMERA_FOLDER_ABSOLUTE_PATH;
        }
    }

    private static boolean isExtendStorageCanUsed() {
        return Build.VERSION.SDK_INT >= Build.VERSION_CODES.M && isDefaultPathCanUsed();
    }

    @SuppressWarnings("ResultOfMethodCallIgnored")
    private static boolean isDefaultPathCanUsed() {
        boolean isDefaultPathCanUsed = false;
        if (sGetDefaultPath != null) {
            try {
                sMountPoint = StorageManagerExt.getDefaultPath();
                File dir = new File(sMountPoint + FOLDER_PATH);
                dir.mkdirs();
                boolean isDirectory = dir.isDirectory();
                boolean canWrite = dir.canWrite();
                if (!isDirectory || !canWrite) {
                    isDefaultPathCanUsed = false;
                } else {
                    isDefaultPathCanUsed = true;
                }
            } catch (Exception e) {
                e.printStackTrace();
                isDefaultPathCanUsed = false;
            }
        }
        LogHelper.d(TAG, "[isDefaultPathCanUsed] isDefaultPathCanUsed = " + isDefaultPathCanUsed);
        return isDefaultPathCanUsed;
    }

}
