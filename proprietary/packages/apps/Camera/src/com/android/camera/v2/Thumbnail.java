/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.camera.v2;

import android.content.ContentResolver;
import android.content.ContentUris;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Matrix;
import android.media.MediaMetadataRetriever;
import android.net.Uri;
import android.os.ParcelFileDescriptor;
import android.provider.MediaStore;
import android.provider.MediaStore.Files.FileColumns;
import android.provider.MediaStore.Images;
import android.provider.MediaStore.Images.ImageColumns;
import android.provider.MediaStore.Video;

import com.android.camera.v2.util.CameraUtil;
import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileDescriptor;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.Locale;

/**
 *  this class is defined for thumbnail reference actions.
 */
public class Thumbnail {
    private static final Tag TAG = new Tag(Thumbnail.class.getSimpleName());
    private static final String                     LAST_THUMB_FILENAME = "last_thumb";
    private static final long                       UNKNOWNID = -1;
    private static final int                        BUFSIZE = 4096;
    private static int                              mOrientation = 0;

    // whether this thumbnail is read from file
    private boolean                                 mFromFile = false;
    private long                                    mThumbnailId = UNKNOWNID;
    private long                                    mDateTaken = 0;
    private String                                  mFilePath;
    private Uri                                     mUri;
    private Bitmap                                  mBitmap;

    // Camera, VideoCamera, and Panorama share the same thumbnail. Use sLock
    // to serialize the storage access.
    private static Object sLock                     = new Object();

    private Thumbnail(Uri uri, Bitmap bitmap, int orientation, long id,
            long dataTaken, String filePath) {
        mUri = uri;
        mBitmap = rotateImage(bitmap, orientation);
        mOrientation = orientation;
        mThumbnailId = id;
        mDateTaken = dataTaken;
        mFilePath = filePath;
    }
    public String getFilePath() {
        return mFilePath;
    }

    public Uri getUri() {
        return mUri;
    }

    public Bitmap getBitmap() {
        return mBitmap;
    }

    public void setFromFile(boolean fromFile) {
        mFromFile = fromFile;
    }

    public boolean fromFile() {
        return mFromFile;
    }

    private static Bitmap rotateImage(Bitmap bitmap, int orientation) {
        if (orientation != 0) {
            // We only rotate the thumbnail once even if we get OOM.
            Matrix m = new Matrix();
            m.setRotate(orientation, bitmap.getWidth() * 0.5f, bitmap.getHeight() * 0.5f);

            try {
                Bitmap rotated = Bitmap.createBitmap(bitmap, 0, 0, bitmap.getWidth(),
                        bitmap.getHeight(), m, true);
                // If the rotated bitmap is the original bitmap, then it
                // should not be recycled.
                if (rotated != bitmap) {
                    bitmap.recycle();
                }
                return rotated;
            } catch (IllegalArgumentException t) {
                LogHelper.w(TAG, "Failed to rotate thumbnail", t);
            }
        }
        return bitmap;
    }

    // Stores the bitmap to the specified file.
    public void saveLastThumbnailToFile(File filesDir) {
        File file = new File(filesDir, LAST_THUMB_FILENAME);
        FileOutputStream f = null;
        BufferedOutputStream b = null;
        DataOutputStream d = null;
        synchronized (sLock) {
            try {
                f = new FileOutputStream(file);
                b = new BufferedOutputStream(f, BUFSIZE);
                d = new DataOutputStream(b);
                d.writeUTF(mUri.toString());
                mBitmap.compress(Bitmap.CompressFormat.JPEG, 90, d);
                d.close();
            } catch (IOException e) {
                LogHelper.e(TAG, "Fail to store bitmap. path=" + file.getPath(), e);
            } finally {
                CameraUtil.closeSilently(f);
                CameraUtil.closeSilently(b);
                CameraUtil.closeSilently(d);
            }
        }
    }

    // Delete the specified file which saved the thumbnail bitmap.
    public static void deleteFrom(File filesDir) {
        synchronized (sLock) {
            File file = new File(filesDir, LAST_THUMB_FILENAME);
            file.delete();
        }
    }
    /**
     * Loads the data from the specified file.
     * @param storageFilePath used to get bucket id
     * @param filesDir file director
     * @param resolver content resolver
     * @return the last thumbnail from file, null if failure or the Uri is invalid.
    */
    public static Thumbnail getLastThumbnailFromFile(
            String storageFilePath, File filesDir, ContentResolver resolver) {
        File file = new File(filesDir, LAST_THUMB_FILENAME);
        Uri uri = null;
        Bitmap bitmap = null;
        FileInputStream f = null;
        BufferedInputStream b = null;
        DataInputStream d = null;
        synchronized (sLock) {
            try {
                f = new FileInputStream(file);
                b = new BufferedInputStream(f, BUFSIZE);
                d = new DataInputStream(b);
                uri = Uri.parse(d.readUTF());
                if (!isUriValid(uri, resolver, storageFilePath)) {
                    d.close();
                    return null;
                }
                bitmap = BitmapFactory.decodeStream(d);
                d.close();
            } catch (IOException e) {
                LogHelper.i(TAG, "Fail to load bitmap. " + e);
                return null;
            } catch (OutOfMemoryError e) {
                LogHelper.e(TAG, "loadFrom file fail", e);
                return null;
            } finally {
                CameraUtil.closeSilently(f);
                CameraUtil.closeSilently(b);
                CameraUtil.closeSilently(d);
            }
        }
        Thumbnail thumbnail = createThumbnail(uri, bitmap, 0);
        if (thumbnail != null) {
            thumbnail.setFromFile(true);
        }
        return thumbnail;
    }

    public static final int THUMBNAIL_NOT_FOUND = 0;
    public static final int THUMBNAIL_FOUND = 1;
    // The media is deleted while we are getting its thumbnail from media
    // provider.
    public static final int THUMBNAIL_DELETED = 2;

    private static class Media {
        public Media(long id, int orientation, long dateTaken, Uri uri,
                int mediaType, String filePath) {
            this.id = id;
            this.orientation = orientation;
            this.dateTaken = dateTaken;
            this.uri = uri;
            this.mediaType = mediaType;
            this.filePath = filePath;
        }

        public final long id;
        public final int orientation;
        public final long dateTaken;
        public final Uri uri;
        public final int mediaType;
        public final String filePath;

        @Override
        public String toString() {
            return new StringBuilder().append("Media(id=").append(id).append(", orientation=")
                    .append(orientation).append(", dateTaken=").append(dateTaken).append(", uri=")
                    .append(uri).append(", mediaType=")
                    .append(mediaType)
                    .append(", filePath=")
                    .append(filePath).append(")")
                    .toString();
        }
    }

    public static Thumbnail createThumbnail(byte[] jpeg, int orientation,
            int inSampleSize, Uri uri) {
        // Create the thumbnail.
        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inSampleSize = inSampleSize;
        Bitmap bitmap = null;
        try {
            bitmap = BitmapFactory.decodeByteArray(jpeg, 0, jpeg.length, options);
        } catch (OutOfMemoryError e) {
            LogHelper.e(TAG, "createThumbnail fail", e);
            return null;
        }
        return createThumbnail(uri, bitmap, orientation);
    }

    // mtk migration start
    public static Bitmap create2DFileFromBitmap(Bitmap bitmap, int stereo3DType) {

        // split
/*        if (stereo3DType == MediaStore.Images.Media.STEREO_TYPE_SIDE_BY_SIDE) {
            bitmap = Bitmap.createBitmap(bitmap, 0, 0, bitmap.getWidth() / 2, bitmap.getHeight());
        } else if (stereo3DType == MediaStore.Images.Media.STEREO_TYPE_TOP_BOTTOM) {
            bitmap = Bitmap.createBitmap(bitmap, 0, 0, bitmap.getWidth(), bitmap.getHeight() / 2);
        }*/
        return bitmap;
    }

    public static Bitmap decodeLastPictureThumb(String filePath, int inSampleSize) {
        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inSampleSize = inSampleSize;
        Bitmap lastPictureThumb = BitmapFactory.decodeFile(filePath, options);
        LogHelper.d(TAG, "lastPictureThumb = " + lastPictureThumb + "!!!; file path" + filePath);
        return lastPictureThumb;
    }

    public static Thumbnail createThumbnail(String filePath, int orientation, int inSampleSize,
            Uri uri, int stereo3DType) {
        Bitmap bitmap = decodeLastPictureThumb(filePath, inSampleSize);
        if (bitmap == null) {
            LogHelper.e(TAG, "Failed to create thumbnail from decodeLastPictureThumb");
            return null;
        }
        // get 2D image if it is 3D file
        bitmap = create2DFileFromBitmap(bitmap, stereo3DType);
        return createThumbnail(uri, bitmap, orientation);
    }

    public static Thumbnail createThumbnail(String filePath, int orientation, int inSampleSize,
            Uri uri) {
        Bitmap bitmap = decodeLastPictureThumb(filePath, inSampleSize);
        return createThumbnail(uri, bitmap, orientation);
    }

    // mtk migration end

    public static Bitmap createVideoThumbnailBitmap(FileDescriptor fd, int targetWidth) {
        return createVideoThumbnailBitmap(null, fd, targetWidth);
    }

    public static Bitmap createVideoThumbnailBitmap(String filePath, int targetWidth) {
        return createVideoThumbnailBitmap(filePath, null, targetWidth);
    }

    private static Bitmap createVideoThumbnailBitmap(String filePath, FileDescriptor fd,
            int targetWidth) {
        Bitmap bitmap = null;
        MediaMetadataRetriever retriever = new MediaMetadataRetriever();
        try {
            if (filePath != null) {
                retriever.setDataSource(filePath);
            } else {
                retriever.setDataSource(fd);
            }
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
        if (bitmap == null || targetWidth == 0) {
            return bitmap;
        }

        // Scale down the bitmap if it is bigger than we need.
        int width = bitmap.getWidth();
        int height = bitmap.getHeight();
        LogHelper.v(TAG, "bitmap = " + width + "x" + height + "   targetWidth=" + targetWidth);
        if (width > targetWidth) {
            float scale = (float) targetWidth / width;
            int w = Math.round(scale * width);
            int h = Math.round(scale * height);
            LogHelper.v(TAG, "w = " + w + "h" + h);
            bitmap = Bitmap.createScaledBitmap(bitmap, w, h, true);
        }
        return bitmap;
    }

    public static Thumbnail createThumbnail(Uri uri, Bitmap bitmap, int orientation) {
        if (bitmap == null) {
            LogHelper.e(TAG, "Failed to create thumbnail from null bitmap");
            return null;
        }
        return new Thumbnail(uri, bitmap, orientation, UNKNOWNID, UNKNOWNID, null);
    }

    public static Thumbnail createThumbnail(Uri uri, Bitmap bitmap, int orientation, long id,
            long dateTaken, String filePath) {
        if (bitmap == null) {
            LogHelper.e(TAG, "Failed to create thumbnail from null bitmap");
            return null;
        }
        return new Thumbnail(uri, bitmap, orientation, id, dateTaken, filePath);
    }
    /**
     * Loads the data from the specified resolver.
     * @param storageFilePath used to get bucket id.
     * @param resolver content resolver.
     * @param result thumbnail.
     * @return whether the last Thumbnail is get form contentResolver.
    */
    public static int getLastThumbnailFromContentResolver(String storageFilePath,
            ContentResolver resolver, Thumbnail[] result) {
        LogHelper.i(TAG, "getLastThumbnailFromContentResolver() begin.");
        Uri baseUri = MediaStore.Files.getContentUri("external");
        Uri query = baseUri.buildUpon().appendQueryParameter("limit", "1").build();
        String[] projection = new String[] { ImageColumns._ID, // image and video
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
                + ImageColumns.BUCKET_ID + '=' + getBucketId(storageFilePath) + ")";
        String order = ImageColumns.DATE_TAKEN + " DESC," + ImageColumns._ID + " DESC";
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
                media = new Media(id, cursor.getInt(1), cursor.getLong(2),
                        ContentUris.withAppendedId(baseUri, id),
                        cursor.getInt(4), cursor.getString(3));
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        LogHelper.d(TAG, "getLastThumbnailFromContentResolver() media=" + media);
        if (media == null) {
            return THUMBNAIL_NOT_FOUND;
        }
        Bitmap bitmap = null;
        orientation = media.orientation;
        try {
            if (media.mediaType == FileColumns.MEDIA_TYPE_IMAGE) {
                bitmap = Images.Thumbnails.getThumbnail(resolver, media.id,
                        Images.Thumbnails.MINI_KIND, null);
            } else if (media.mediaType == FileColumns.MEDIA_TYPE_VIDEO) {
                // For Video type, the thumbnail is rotated, so ap don't care
                // the orientation.
                orientation = 0;
                bitmap = Video.Thumbnails.getThumbnail(resolver, media.id,
                            Video.Thumbnails.MINI_KIND, null);
            }
            // Ensure database and storage are in sync.
            if (isUriValid(media.uri, resolver, storageFilePath)) {
                result[0] = createThumbnail(media.uri, bitmap,
                        orientation, id, dateTaken, filePath);
                return THUMBNAIL_FOUND;
            } else {
                LogHelper.d(TAG, "Uri is not valid!");
            }
        } catch (OutOfMemoryError e) {
            LogHelper.e(TAG, "getThumbnail fail", e);
        }
        LogHelper.d(TAG, "Quit getLastThumbnail");
        return THUMBNAIL_DELETED;
    }

    @Override
    public String toString() {
        return new StringBuilder().append("Thumbnail(mUri=").append(mUri).append(", mFromFile=")
                .append(mFromFile).append(", mBitmap=").append(mBitmap).append(")").toString();
    }
    private static String getBucketId(String directory) {
        return String.valueOf(directory.toLowerCase(Locale.ENGLISH).hashCode());
    }

    /**
     * judge the uri is valid or not.
     * @param uri file uri.
     * @param resolver content resolver.
     * @param filePath file path.
     * @return the result whether the uri is valid.
     */
     private static boolean isUriValid(Uri uri, ContentResolver resolver, String filePath) {
         if (uri == null) {
             return false;
         }

         try {
             ParcelFileDescriptor pfd = resolver.openFileDescriptor(uri, "r");
             if (pfd == null) {
                 LogHelper.e(TAG, "Fail to open URI. URI=" + uri);
                 return false;
             }
             pfd.close();
         } catch (IOException ex) {
             return false;
         }
         return isMountPointValid(uri, resolver, filePath);
     }

     private static boolean isMountPointValid(Uri uri, ContentResolver resolver, String filePath) {
         String path = "";
         Cursor cursor = resolver.query(uri, new String[] { MediaStore.MediaColumns.DATA }, null,
                 null, null);
         if (cursor != null) {
             try {
                 if (cursor.moveToFirst()) {
                     path = cursor.getString(0);
                 }
             } finally {
                 cursor.close();
             }
         }
         File file = new File(path);
         String parent = file.getParent();
         boolean valid = filePath.equals(parent);
         LogHelper.d(TAG, "isMountPointValid(" + uri + ") path =" + path);
         return valid;
     }

}
