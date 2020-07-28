/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2008 Esmertec AG.
 * Copyright (C) 2008 The Android Open Source Project
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

package com.android.mms.ui;

import android.content.ContentResolver;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SqliteWrapper;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Matrix;
import android.graphics.Bitmap.CompressFormat;
import android.media.ExifInterface;
import android.net.Uri;
import android.provider.MediaStore.Images;
import android.provider.Telephony.Mms.Part;
import android.text.TextUtils;
import android.util.Log;
import android.webkit.MimeTypeMap;

import com.android.mms.LogTag;
import com.android.mms.MmsApp;
import com.android.mms.model.ImageModel;
import com.android.mms.util.MmsLog;
import com.google.android.mms.pdu.PduPart;

import com.mediatek.mms.util.DrmUtilsEx;
import com.mediatek.storage.StorageManagerEx;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;


/// M: fix bug ALPS397146, clear on-disk Thumbnail cache
/// when same filePath and diff uri or  same uri and differnet degree @{
class ImageData {
    public ImageData(String filePath, String imageUri, int degree) {
        mFilePath = filePath;
        mImageUri = imageUri;
        mDegree = degree;
    }
    public String mFilePath;
    public String mImageUri;
    public int mDegree;
};
/// @}

public class UriImage {
    private static final String TAG = "Mms/image";
    private static final boolean DEBUG = false;
    private static final boolean LOCAL_LOGV = true;

    private final Context mContext;
    private final Uri mUri;
    private String mContentType;
    private String mPath;
    private String mSrc;
    private int mWidth;
    private int mHeight;
    private static final ArrayList<ImageData> sImageArray = new ArrayList<ImageData>();

    /// M:
    private static final String JPEGCONTENTTYPE = "image/jpeg";

    /// M: fix bug ALPS397146, a flag whether reSizedImage
    private static boolean sIsResize = false;

    public static boolean getIsResize() {
        return sIsResize;
    }

    public static void setIsResize(boolean isResize) {
        sIsResize = isResize;
    }

    public UriImage(Context context, Uri uri) {
        if ((null == context) || (null == uri)) {
            throw new IllegalArgumentException();
        }

        MmsLog.dpi(TAG, "UriImage init : " + uri);
        String scheme = uri.getScheme();
        if (scheme.equals("content")) {
            initFromContentUri(context, uri);
        } else if (uri.getScheme().equals("file")) {
            initFromFile(context, uri);
        }

        mContext = context;
        mUri = uri;

        decodeBoundsInfo();

        if (LOCAL_LOGV) {
            MmsLog.vpi(TAG, "UriImage uri: " + uri + " mPath: " + mPath + " mWidth: " + mWidth +
                    " mHeight: " + mHeight);
        }
    }

    private Context mContextTemp;
    private void initFromFile(Context context, Uri uri) {
        mContextTemp = context;
        mPath = uri.getPath();
        MimeTypeMap mimeTypeMap = MimeTypeMap.getSingleton();
        /// M: Code analyze 004, For fix bug ALPS00104512, personal use on device. @{
        String extension = MimeTypeMap.getFileExtensionFromUrl(mPath).toLowerCase();
        /// @}
        if (TextUtils.isEmpty(extension)) {
            // getMimeTypeFromExtension() doesn't handle spaces in filenames nor can it handle
            // urlEncoded strings. Let's try one last time at finding the extension.
            int dotPos = mPath.lastIndexOf('.');
            if (0 <= dotPos) {
                extension = mPath.substring(dotPos + 1);
                /// M: Code analyze 004, For fix bug ALPS00104512, personal use on device. @{
                extension = extension.toLowerCase();
                /// @}
            }
        }
        mContentType = mimeTypeMap.getMimeTypeFromExtension(extension);
        /// M: Code analyze 005, new feature, used for drm. @{
        if (mContentType == null) {
            mContentType = DrmUtilsEx.getDrmContentType(null,
                    extension, mPath, "application/octet-stream");
        }
        /// @}
        MmsLog.ipi(TAG, "ImageModel got file path: " + mPath + " extension: " + extension
                + " mContentType: " + mContentType);
        // It's ok if mContentType is null. Eventually we'll show a toast telling the
        // user the picture couldn't be attached.

        buildSrcFromPath();
    }

    private void buildSrcFromPath() {
        /// M: Code analyze 002, For fix bug ALPS00243850, check if this image
        // is from temp dir captured by camera.if Yes, change the name for
        // coflict if two image are got from the temp dir,they are the same name
        // before saveDraft @{
        File mTempFile = StorageManagerEx.getExternalCacheDir(mContextTemp.getPackageName());
        if ( mPath == null ||
             (mTempFile != null && mPath.equals(mTempFile.getAbsolutePath() + "/" + ".temp.jpg"))) {
            mSrc = "image" + Long.toString(System.currentTimeMillis()) + ".jpg";
        } else {
            mSrc = mPath.substring(mPath.lastIndexOf('/') + 1);
        }
        /// @}

        if (mSrc.startsWith(".") && mSrc.length() > 1) {
            mSrc = mSrc.substring(1);
        }

        // Some MMSCs appear to have problems with filenames
        // containing a space.  So just replace them with
        // underscores in the name, which is typically not
        // visible to the user anyway.
        mSrc = mSrc.replace(' ', '_');
        MmsLog.ipi(TAG, "ImageModel got mSrc: " + mSrc);
    }

    private void initFromContentUri(Context context, Uri uri) {
        mContextTemp = context;
        /// M: google jb.mr1 patch
        ContentResolver resolver = context.getContentResolver();
        Cursor c = null;
        /// M: fix bug ALPS01239875, resolve JE for email Uri
        if (uri.toString().contains("com.android.email")) {
            c = SqliteWrapper.query(context, resolver, uri,
                    new String[] {"_id", Images.Media.DATA, Images.Media.DISPLAY_NAME},
                    null, null, null);
        } else {
            try {
                    c = SqliteWrapper.query(context, resolver, uri, null, null, null, null);
                } catch (SecurityException ex) {
                    Log.e(TAG, ex.getMessage(), ex);
                    if (mSrc == null) {
                        buildSrcFromPath();
                    }
                    mPath= null;
                    mContentType= null;
                    return;
                }
        }

        mSrc = null;
        if (c == null) {
            throw new IllegalArgumentException("Query on " + uri + " returns null result.");
        }

        try {
            if ((c.getCount() != 1) || !c.moveToFirst()) {
                throw new IllegalArgumentException(
                        "Query on " + uri + " returns 0 or multiple rows.");
            }

            String filePath;
            if (ImageModel.isMmsUri(uri)) {
                filePath = c.getString(c.getColumnIndexOrThrow(Part.FILENAME));
                if (TextUtils.isEmpty(filePath)) {
                    filePath = c.getString(
                            c.getColumnIndexOrThrow(Part._DATA));
                }
                mContentType = c.getString(
                        c.getColumnIndexOrThrow(Part.CONTENT_TYPE));
            } else {
                /// M: Code analyze 003, For fix bug ALPS00278013, send the Mms
                // with a image from phone to email,but the image name changed
                // to number and the suffix is lost when received. @{
                try {
                    filePath = c.getString(c.getColumnIndexOrThrow(Images.Media.DATA));
                } catch (IllegalArgumentException e) {
                    filePath = uri.getPath();
                }
                if (TextUtils.isEmpty(filePath)) {
                    filePath = uri.getPath();
                }
                /// @}
                try {
                    mContentType = c.getString(
                                c.getColumnIndexOrThrow(Images.Media.MIME_TYPE)); // mime_type
                } catch (IllegalArgumentException e) {
                    /// M: google jb.mr1 patch, cannot get the
                    /// mime_type from the "mime_type" column,
                    /// then try getting it by calling ContentResolver.getType(uri); @{
                    try {
                        mContentType = c.getString(c.getColumnIndexOrThrow("mimetype"));
                    } catch (IllegalArgumentException ex) {
                        mContentType = resolver.getType(uri);
                        MmsLog.epi(TAG, "initFromContentUri: "
                                + uri + ", resolver.getType => " + mContentType);
                    }
                    /// @}
                }

                // use the original filename if possible
                int nameIndex = c.getColumnIndex(Images.Media.DISPLAY_NAME);
                if (nameIndex != -1) {
                    mSrc = c.getString(nameIndex);
                    if (!TextUtils.isEmpty(mSrc)) {
                        // Some MMSCs appear to have problems with filenames
                        // containing a space.  So just replace them with
                        // underscores in the name, which is typically not
                        // visible to the user anyway.
                        mSrc = mSrc.replace(' ', '_');
                    } else {
                        mSrc = null;
                    }
                }
            }
            mPath = filePath;
            MmsLog.ipi(TAG, "ImageModel file path: " + mPath + " mContentType: " + mContentType);

            /// M:
            if (mPath == null) {
                return;
            }
            if (mSrc == null) {
                buildSrcFromPath();
            }
        } catch (IllegalArgumentException e) {
            MmsLog.epi(TAG, "initFromContentUri couldn't load image uri: " + uri, e);
        } finally {
            c.close();
        }
    }

    private void decodeBoundsInfo() {
        if (DrmUtilsEx.checkHasDrmContent(mContext, null, mUri)) {
            mWidth = -1;
            mHeight = -1;
            return;
        }
        InputStream input = null;
        try {
            input = mContext.getContentResolver().openInputStream(mUri);
            BitmapFactory.Options opt = new BitmapFactory.Options();
            opt.inJustDecodeBounds = true;
            BitmapFactory.decodeStream(input, null, opt);
            mWidth = opt.outWidth;
            mHeight = opt.outHeight;
        } catch (FileNotFoundException e) {
            // Ignore
            Log.e(TAG, "IOException caught while opening stream", e);
            mWidth = 0;
            mHeight = 0;
        } catch (SecurityException e) {
            Log.e(TAG, "SecurityException caught while opening stream", e);
            mWidth = 0;
            mHeight = 0;
        } finally {
            if (null != input) {
                try {
                    input.close();
                } catch (IOException e) {
                    // Ignore
                    Log.e(TAG, "IOException caught while closing stream", e);
                }
            }
        }
    }

    public static void clearImageArray() {
        synchronized (sImageArray) {
            sImageArray.clear();
        }
    }

    public String getContentType() {
        return mContentType;
    }

    public String getSrc() {
        return mSrc;
    }

    public String getPath() {
        return mPath;
    }

    public int getWidth() {
        return mWidth;
    }

    public int getHeight() {
        return mHeight;
    }

    /**
     * Get a version of this image resized to fit the given dimension and byte-size limits. Note
     * that the content type of the resulting PduPart may not be the same as the content type of
     * this UriImage; always call {@link PduPart#getContentType()} to get the new content type.
     *
     * @param widthLimit The width limit, in pixels
     * @param heightLimit The height limit, in pixels
     * @param byteLimit The binary size limit, in bytes
     * @return A new PduPart containing the resized image data
     */
    public PduPart getResizedImageAsPart(int widthLimit, int heightLimit, int byteLimit) {
        PduPart part = new PduPart();

        byte[] data =  getResizedImageData(mWidth, mHeight,
                widthLimit, heightLimit, byteLimit, mUri, mContext);
        if (data == null) {
            if (LOCAL_LOGV) {
                Log.v(TAG, "Resize image failed.");
            }
            return null;
        }
        mContentType = JPEGCONTENTTYPE;
        part.setData(data);
        // getResizedImageData ALWAYS compresses to JPEG, regardless of the original content type
        //part.setContentType(ContentType.IMAGE_JPEG.getBytes());
        part.setContentType(mContentType.getBytes());
        if (!TextUtils.isEmpty(mSrc)) {
            part.setFilename(mSrc.getBytes());
        }
        return part;
    }

    private static final int NUMBER_OF_RESIZE_ATTEMPTS = 4;

    /**
     * Resize and recompress the image such that it fits the given limits. The resulting byte
     * array contains an image in JPEG format, regardless of the original image's content type.
     * @param widthLimit The width limit, in pixels
     * @param heightLimit The height limit, in pixels
     * @param byteLimit The binary size limit, in bytes
     * @return A resized/recompressed version of this image, in JPEG format
     */
    public byte[] getResizedImageData(int width, int height,
            int widthLimit, int heightLimit, int byteLimit, Uri uri, Context context) {
        int outWidth = width;
        int outHeight = height;

        int widthScale = (outWidth - 1) / widthLimit + 1;
        int heightScale = (outHeight - 1) / heightLimit + 1;
        int roateWidthScale = (outWidth - 1) / heightLimit + 1;
        int rotateHeightScale = (outHeight - 1) / widthLimit + 1;
        int scaleFactor = (widthScale > heightScale ? widthScale : heightScale);
        int roateScaleFactor = (roateWidthScale > rotateHeightScale
                ? roateWidthScale : rotateHeightScale);
        if (roateScaleFactor < scaleFactor) {
            scaleFactor = roateScaleFactor;
        }

        if (Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
            Log.v(TAG, "getResizedImageData: wlimit=" + widthLimit +
                    ", hlimit=" + heightLimit + ", sizeLimit=" + byteLimit +
                    ", mWidth=" + width + ", mHeight=" + height +
                    ", initialScaleFactor=" + scaleFactor);
        }

        InputStream input = null;
        /// M: Code analyze 004, For fix bug ALPS00104512, personal use on device. @{
        InputStream inputForRotate = null;
        ByteArrayOutputStream os = null;
        try {
            int attempts = 1;
            boolean resultTooBig = true;
            boolean isMustResize = false;
            do {
                BitmapFactory.Options options = new BitmapFactory.Options();
                options.inSampleSize = scaleFactor;
                input = context.getContentResolver().openInputStream(uri);
                // Don't know why need two copy of inputStream if we use ExifInterface,
                // this is only for getting rotation degree
                inputForRotate = context.getContentResolver().openInputStream(uri);
                int orientation = 0;
                int degree = 0;

                try {
                    if (inputForRotate != null) {
                        ExifInterface exif = new ExifInterface(inputForRotate);
                        if (exif != null) {
                            orientation = exif.getAttributeInt(ExifInterface.TAG_ORIENTATION, 0);
                            degree = getExifRotation(orientation);
                            /// M: fix bug ALPS397146, clear on-disk Thumbnail cache
                            /// when same filePath and diff uri or same uri and differnet degree @{
                            String imageUri = uri.getPath();

                            setIsResize(true);

                            String filePath = "";
                            if (!ImageModel.isMmsUri(uri)) {
                                Cursor c = null;
                                try {
                                    c = SqliteWrapper.query(context, context.getContentResolver(),
                                            uri, null, null, null, null);
                                    if (c != null && c.getCount() == 1 && c.moveToFirst()) {
                                        filePath = c.getString(
                                                c.getColumnIndexOrThrow(Images.Media.DATA));
                                    }
                                    if (TextUtils.isEmpty(filePath)) {
                                        filePath = uri.getPath();
                                    }
                                } catch (IllegalArgumentException e) {
                                    filePath = uri.getPath();
                                } finally {
                                    if (c != null) {
                                        c.close();
                                    }
                                }
                            }
                            synchronized (sImageArray) {
                                Boolean isAdd = true;
                                for (ImageData imageData : sImageArray) {
                                    if (imageData != null
                                            && imageData.mFilePath != null
                                            && imageData.mImageUri != null
                                            && imageData.mFilePath.equals(filePath)
                                            && imageData.mImageUri.equals(imageUri)
                                            && imageData.mDegree == degree) {
                                        isAdd = false;
                                        break;
                                    }
                                    if ((imageData != null
                                            && imageData.mFilePath != null
                                            && imageData.mImageUri != null
                                            && imageData.mFilePath.equals(filePath)
                                            && !imageData.mImageUri.equals(imageUri)) ||
                                            (imageData != null
                                                    && imageData.mImageUri != null
                                                    && imageData.mImageUri.equals(imageUri)
                                                    && imageData.mDegree != degree))  {
                                        MmsApp.getApplication()
                                            .getThumbnailManager().clearBackingStore();
                                        sImageArray.clear();
                                        break;
                                        }
                                }
                                if (isAdd) {
                                    sImageArray.add(new ImageData(filePath, imageUri, degree));
                                }
                            }
                            /// @}
                        }
                    }
                } catch (IOException e) {
                    MmsLog.e(TAG, e.getMessage(), e);
                } finally {
                    if (inputForRotate != null) {
                        try {
                            inputForRotate.close();
                        } catch (IOException e) {
                            MmsLog.e(TAG, e.getMessage(), e);
                        }
                    }
                }

                MmsLog.i(TAG, "image rotation is" + degree + " degree");
                int quality = MessageUtils.IMAGE_COMPRESSION_QUALITY;
                try {
                    Bitmap b = BitmapFactory.decodeStream(input, null, options);
        /// @}
                    if (b == null) {
                        MmsLog.w(TAG, "BitmapFactory.decodeStream return null");
                        return null;
                    }
                    // rotate it to final rotation.
                    b = rotate(b, degree);
                    /// M:BUGFIX:ALPS00534764 @{
                    boolean needReduce = true;
                    if (options.outWidth <= widthLimit && options.outHeight <= heightLimit) {
                        needReduce = false;
                    }
                    if ((needReduce) &&
                            (options.outWidth <= heightLimit && options.outHeight <= widthLimit)) {
                        needReduce = false;
                    }
                    if (needReduce || isMustResize) {
                        if (isMustResize) {
                            isMustResize = false;
                            scaleFactor = 2;
                        }
                        // The decoder does not support the inSampleSize option.
                        // Scale the bitmap using Bitmap library.
                        int scaledWidth = outWidth / scaleFactor;
                        int scaledHeight = outHeight / scaleFactor;

                        if (Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
                            Log.v(TAG, "getResizedImageData: retry scaling using " +
                                    "Bitmap.createScaledBitmap: w=" + scaledWidth +
                                    ", h=" + scaledHeight);
                        }

                        /// M: Code analyze 004, For fix bug ALPS00104512,
                        /// personal use on device. @{
                        b = Bitmap.createScaledBitmap(b, outWidth / scaleFactor,
                                outHeight / scaleFactor, false);
                        /// @}
                        if (b == null) {
                            if (Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
                                Log.v(TAG, "Bitmap.createScaledBitmap returned NULL!");
                            }
                            return null;
                        }
                    }

                    /** M: Compress the image into a JPG. Start with
                     * MessageUtils.IMAGE_COMPRESSION_QUALITY.
                     * In case that the image byte size is still too large reduce the quality in
                     * proportion to the desired byte size. Should the quality fall below
                     * MINIMUM_IMAGE_COMPRESSION_QUALITY skip a
                     * compression attempt and we will enter
                     * the next round with a smaller image to start with.
                     */
                    if (os != null) {
                        try {
                            os.close();
                        } catch (IOException e) {
                            Log.e(TAG, e.getMessage(), e);
                        }
                    }
                    os = new ByteArrayOutputStream();
                    // if (!b.hasAlpha()) {
                    if (mContentType != null) {
                        MmsLog.d(TAG, "Bitmap compress mContentType 1:"+mContentType);
                        if(!mContentType.substring(mContentType.lastIndexOf("/")+1).toUpperCase().
                          contains("PNG")) {
                        MmsLog.d(TAG, "Bitmap compress JPEG_1");
                        b.compress(CompressFormat.JPEG, quality, os);
                        } else {
                            MmsLog.d(TAG, "Bitmap compress PNG_1");
                            b.compress(CompressFormat.PNG, quality, os);
                        }
                    }
                    //b.compress(CompressFormat.JPEG, quality, os);
                    int jpgFileSize = os.size();

                    /// M: Modify for ALPS00778930, if jpgFileSize > byteLimit
                    // we should compress it util jpgFileSize < byteLimit

                    int count = 0;

                    while (jpgFileSize > byteLimit && count < MessageUtils.MAX_COMPRESS_TIMES) {
                        /// M: Code analyze 004, For fix bug ALPS00104512,
                        /// personal use on device. @{
                        int reducedQuality = (quality * byteLimit) / jpgFileSize;
                        if (reducedQuality >= MessageUtils.MINIMUM_IMAGE_COMPRESSION_QUALITY) {
                            quality = reducedQuality;
                        /// @}

                            if (Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
                                Log.v(TAG,
                                        "getResizedImageData: compress(2) w/ quality=" + quality);
                            }
                            if (os != null) {
                                try {
                                    os.close();
                                } catch (IOException e) {
                                    Log.e(TAG, e.getMessage(), e);
                                }
                            }
                            os = new ByteArrayOutputStream();
                            if (mContentType != null) {
                                MmsLog.d(TAG, "Bitmap compress mContentType 2:"+mContentType);
                                if(!mContentType.substring(mContentType.lastIndexOf("/")+1).toUpperCase().
                                     contains("PNG")) {
                                    MmsLog.d(TAG, "Bitmap compress JPEG_2");
                                    b.compress(CompressFormat.JPEG, quality, os);
                                } else {
                                     MmsLog.d(TAG, "Bitmap compress PNG_2");
                                     b.compress(CompressFormat.PNG, quality, os);
                                }
                            //b.compress(CompressFormat.JPEG, quality, os);
                            }
                        } else {
                            MmsLog.i(TAG,
                                    "reducedQuality < MINIMUM_IMAGE_COMPRESSION_QUALITY");
                            break;
                        }
                        jpgFileSize = os.size();
                        count++;
                    }
                    /// M: done with the bitmap, release the memory
                    b.recycle();
                } catch (java.lang.OutOfMemoryError e) {
                    Log.e(TAG, "getResizedImageData - image too big (OutOfMemoryError), will try "
                            + " with smaller scale factor, cur scale factor: " + scaleFactor);
                    /// M: fall through and keep trying with a smaller scale factor.
                }
                if (Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
                    Log.v(TAG, "attempt=" + attempts
                            + " size=" + (os == null ? 0 : os.size())
                            + " width=" + outWidth / scaleFactor
                            + " height=" + outHeight / scaleFactor
                            + " scaleFactor=" + scaleFactor
                            + " quality=" + quality);
                }
                /// M: Code analyze 004, For fix bug ALPS00104512, personal use on device. @{
                scaleFactor *= 2;
                /// @}
                attempts++;
                resultTooBig = os == null || os.size() > byteLimit;
                if (attempts == 3 && resultTooBig
                        && options.outWidth < widthLimit && options.outHeight < heightLimit) {
                    isMustResize = true;
                }
            } while (resultTooBig && attempts < NUMBER_OF_RESIZE_ATTEMPTS);

            if (Log.isLoggable(LogTag.APP, Log.VERBOSE) && resultTooBig) {
                Log.v(TAG, "getResizedImageData returning NULL because the result is too big: "
                    + " requested max: " + byteLimit + " actual: " + os.size());
            }

            return resultTooBig ? null : os.toByteArray();
        } catch (FileNotFoundException e) {
            Log.e(TAG, e.getMessage(), e);
            return null;
        } catch (SecurityException e) {
            Log.e(TAG, e.getMessage(), e);
            return null;
        } catch (java.lang.OutOfMemoryError e) {
            Log.e(TAG, e.getMessage(), e);
            return null;
        /// M: Code analyze 004, For fix bug ALPS00104512, personal use on device. @{
        } finally {
            if (input != null) {
                try {
                    input.close();
                } catch (IOException e) {
                    Log.e(TAG, e.getMessage(), e);
                }
            }
            if (os != null) {
                try {
                    os.close();
                } catch (IOException e) {
                    MmsLog.e(TAG, e.getMessage(), e);
                }
            }
        }
        /// @}
    }

    /** M:
     * corresponding orientation of EXIF to degrees.
     */
    public static int getExifRotation(int orientation) {
        int degrees = 0;
        switch (orientation) {
            case ExifInterface.ORIENTATION_NORMAL:
                degrees = 0;
                break;
            case ExifInterface.ORIENTATION_ROTATE_90:
                degrees = 90;
                break;
            case ExifInterface.ORIENTATION_ROTATE_180:
                degrees = 180;
                break;
            case ExifInterface.ORIENTATION_ROTATE_270:
                degrees = 270;
                break;
            default:
                break;
        }
        return degrees;
    }

    /** M: Rotates the bitmap by the specified degree.
     * If a new bitmap is created, the original bitmap is recycled.
     */
    public static Bitmap rotate(Bitmap b, int degrees) {
        if (degrees != 0 && b != null) {
            Matrix m = new Matrix();
            m.setRotate(degrees,
                    (float) b.getWidth() / 2, (float) b.getHeight() / 2);
            try {
                b.setHasAlpha(true);
                Bitmap b2 = Bitmap.createBitmap(
                        b, 0, 0, b.getWidth(), b.getHeight(), m, true);
                if (b != b2) {
                    b.recycle();
                    b = b2;
                }
            } catch (OutOfMemoryError ex) {
                // We have no memory to rotate. Return the original bitmap.
                MmsLog.e(TAG, "We have no memory to rotate.", ex);
            }
        }
        return b;
    }

    /// M: Code analyze 001, For fix bug ALPS00233419, It can't share the image
    // whose format is ".bin". Composer uses the mime type which has been passed
    // by the user intent. @{
    public void setContentType(String contentType) {
        mContentType = contentType;
    }
    /// @}

    /// M: Code analyze 006, new feature, provide rotation degree of the image
    // for the out class. @{
    public  static int getImageRotationDegree(Context context, Uri uri) {
        if (context == null || uri == null) {
            return 0;
        }
        InputStream inputForRotate = null;
        try {
            inputForRotate = context.getContentResolver().openInputStream(uri);
            if (inputForRotate != null) {
                int orientation = 0;
                ExifInterface exif = new ExifInterface(inputForRotate);
                if (exif != null) {
                    orientation = exif.getAttributeInt(ExifInterface.TAG_ORIENTATION, 0);
                    return getExifRotation(orientation);
                }
            }
            return 0;
        } catch (IOException e) {
            MmsLog.e(TAG, e.getMessage(), e);
            return 0;
        } finally {
            if (inputForRotate != null) {
                try {
                    inputForRotate.close();
                } catch (IOException e) {
                    MmsLog.e(TAG, e.getMessage(), e);
                }
            }
        }
    }
    /// @}
}
