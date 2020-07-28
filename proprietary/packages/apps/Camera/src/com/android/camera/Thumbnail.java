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

package com.android.camera;

import android.content.ContentResolver;
import android.content.ContentUris;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Matrix;
import android.media.MediaMetadataRetriever;
import android.net.Uri;
import android.provider.MediaStore;
import android.provider.MediaStore.Files.FileColumns;
import android.provider.MediaStore.Images;
import android.provider.MediaStore.Images.ImageColumns;
import android.provider.MediaStore.Video;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileDescriptor;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;

public class Thumbnail {
    private static final String TAG = "Thumbnail";
    public static final long UNKNOWNID = -1;

    private static final String LAST_THUMB_FILENAME = "last_thumb";
    private static final int BUFSIZE = 4096;

    private Uri mUri;
    private Bitmap mBitmap;
    // whether this thumbnail is read from file
    private boolean mFromFile = false;
    private long mThumbnailId = UNKNOWNID;
    private static int mOrientation = 0;
    private long mDateTaken = 0;
    private String mFilePath;

    // Camera, VideoCamera, and Panorama share the same thumbnail. Use sLock
    // to serialize the storage access.
    private static Object sLock = new Object();

    private Thumbnail(Uri uri, Bitmap bitmap, int orientation, long id, long dataTaken,
            String filePath) {
        mUri = uri;
        mBitmap = rotateImage(bitmap, orientation);
        mOrientation = orientation;
        mThumbnailId = id;
        mDateTaken = dataTaken;
        mFilePath = filePath;
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

    public String getFilePath() {
        return mFilePath;
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
                Log.w(TAG, "Failed to rotate thumbnail", t);
            }
        }
        return bitmap;
    }

    public static final int THUMBNAIL_NOT_FOUND = 0;
    public static final int THUMBNAIL_FOUND = 1;
    // The media is deleted while we are getting its thumbnail from media
    // provider.
    public static final int THUMBNAIL_DELETED = 2;

    private static class Media {
        public Media(long id, int orientation, long dateTaken,
                Uri uri, int mediaType, String filePath) {
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
                    .append(uri).append(", mediaType=").append(mediaType)
                    .append(", filePath=").append(filePath).append(")")
                    .toString();
        }
    }

    /**
     * create thumbnail for the image.
     * @param jpeg the image data.
     * @param orientation the image orientation, get from Image info.
     * @param inSampleSize the image size, get from the Image info.
     * @param uri the save image uri.
     * @param filePath the image path.
     * @return the thumbnail the image thumbnail.
     */
    public static Thumbnail createThumbnail(byte[] jpeg, int orientation,
            int inSampleSize, Uri uri, String filePath) {
        // Create the thumbnail.
        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inSampleSize = inSampleSize;
        Bitmap bitmap = null;
        try {
            bitmap = BitmapFactory.decodeByteArray(jpeg, 0, jpeg.length, options);
        } catch (OutOfMemoryError e) {
            Log.e(TAG, "createThumbnail fail", e);
            return null;
        }
        return createThumbnail(uri, bitmap, orientation, filePath);
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
        Log.d(TAG, "lastPictureThumb = " + lastPictureThumb + "!!!; file path" + filePath);
        return lastPictureThumb;
    }

    public static Thumbnail createThumbnail(String filePath, int orientation, int inSampleSize,
            Uri uri, int stereo3DType) {
        Bitmap bitmap = decodeLastPictureThumb(filePath, inSampleSize);
        if (bitmap == null) {
            Log.e(TAG, "Failed to create thumbnail from decodeLastPictureThumb");
            return null;
        }
        // get 2D image if it is 3D file
        bitmap = create2DFileFromBitmap(bitmap, stereo3DType);
        return createThumbnail(uri, bitmap, orientation, filePath);
    }

    public static Thumbnail createThumbnail(String filePath, int orientation, int inSampleSize,
            Uri uri) {
        Bitmap bitmap = decodeLastPictureThumb(filePath, inSampleSize);
        return createThumbnail(uri, bitmap, orientation, filePath);
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
        Log.v(TAG, "bitmap = " + width + "x" + height + "   targetWidth=" + targetWidth);
        if (width > targetWidth) {
            float scale = (float) targetWidth / width;
            int w = Math.round(scale * width);
            int h = Math.round(scale * height);
            Log.v(TAG, "w = " + w + "h" + h);
            bitmap = Bitmap.createScaledBitmap(bitmap, w, h, true);
        }
        return bitmap;
    }

    /**
     * Create thumbnail for the image.
     * @param uri the image uri.
     * @param bitmap the bitmap data for create thumbnail.
     * @param orientation the image orientation.
     * @param filePath the image path.
     * @return thumbnail data.
     */
    public static Thumbnail createThumbnail(Uri uri, Bitmap bitmap,
            int orientation, String filePath) {
        if (bitmap == null) {
            Log.e(TAG, "Failed to create thumbnail from null bitmap");
            return null;
        }
        return new Thumbnail(uri, bitmap, orientation, UNKNOWNID, UNKNOWNID, filePath);
    }

    public static Thumbnail createThumbnail(Uri uri, Bitmap bitmap, int orientation, long id,
            long dateTaken, String filePath) {
        if (bitmap == null) {
            Log.e(TAG, "Failed to create thumbnail from null bitmap");
            return null;
        }
        return new Thumbnail(uri, bitmap, orientation, id, dateTaken, filePath);
    }

    public static int getLastThumbnailFromContentResolver(ContentResolver resolver,
            Thumbnail[] result, Thumbnail oldThumbnail) {
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
                + ImageColumns.BUCKET_ID + '=' + Storage.getBucketId() + ")";
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

                if (oldThumbnail != null) {
                    Log.d(TAG, "id:" + id + ",oldId:" + oldThumbnail.mThumbnailId + ",orientation:"
                            + orientation + ",oldOrientation:" + oldThumbnail.mOrientation
                            + ",dateTaken:" + dateTaken + ",oldDateTaken:"
                            + oldThumbnail.mDateTaken + ",filePath:" + filePath + ",oldFilePath:"
                            + oldThumbnail.mFilePath);
                }
                if (oldThumbnail == null || id != oldThumbnail.mThumbnailId
                        || orientation != oldThumbnail.mOrientation
                        || dateTaken != oldThumbnail.mDateTaken
                        || !filePath.equalsIgnoreCase(oldThumbnail.mFilePath)) {
                    media = new Media(id, cursor.getInt(1), cursor.getLong(2),
                            ContentUris.withAppendedId(baseUri, id), cursor.getInt(4),
                            cursor.getString(3));
                } else {
                    Log.d(TAG, "same file, don't need decode again!");
                    return THUMBNAIL_DELETED;
                }
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        Log.d(TAG, "getLastThumbnailFromContentResolver() media=" + media);
        if (media == null) {
            return THUMBNAIL_NOT_FOUND;
        }
        Bitmap bitmap = null;
        Uri uri = null;
        orientation = media.orientation;
        try {
            if (media.mediaType == FileColumns.MEDIA_TYPE_IMAGE) {
                bitmap = Images.Thumbnails.getThumbnail(resolver, media.id,
                        Images.Thumbnails.MINI_KIND, null);
                uri = ContentUris.withAppendedId(
                        MediaStore.Images.Media.getContentUri("external"), media.id);
            } else if (media.mediaType == FileColumns.MEDIA_TYPE_VIDEO) {
                // For Video type, the thumbnail is rotated, so ap don't care
                // the orientation.
                orientation = 0;
                bitmap = Video.Thumbnails.getThumbnail(resolver, media.id,
                            Video.Thumbnails.MINI_KIND, null);
                uri = ContentUris.withAppendedId(
                        MediaStore.Video.Media.getContentUri("external"), media.id);
            }
            // Ensure database and storage are in sync.
            if (Util.isUriValid(uri, resolver)) {
                result[0] = createThumbnail(uri, bitmap, orientation, id, dateTaken, filePath);
                return THUMBNAIL_FOUND;
            } else {
                Log.d(TAG, "Uri is not valid!");
            }
        } catch (OutOfMemoryError e) {
            Log.e(TAG, "getThumbnail fail", e);
        }
        Log.d(TAG, "Quit getLastThumbnail");
        return THUMBNAIL_DELETED;
    }

    @Override
    public String toString() {
        return new StringBuilder().append("Thumbnail(mUri=").append(mUri).append(", mFromFile=")
                .append(mFromFile).append(", mBitmap=").append(mBitmap).append(")").toString();
    }
}
