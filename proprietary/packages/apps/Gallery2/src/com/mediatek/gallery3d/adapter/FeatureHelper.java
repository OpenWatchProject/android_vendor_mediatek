package com.mediatek.gallery3d.adapter;

import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.UriMatcher;
import android.database.Cursor;
import android.media.ExifInterface;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.storage.StorageManager;
import android.os.storage.StorageVolume;
import android.provider.MediaStore;
import android.provider.MediaStore.Images.Media;
import android.support.v4.os.EnvironmentCompat;

import com.android.gallery3d.app.AbstractGalleryActivity;
import com.android.gallery3d.app.PhotoPage;
import com.android.gallery3d.data.MediaDetails;
import com.android.gallery3d.data.MediaItem;
import com.android.gallery3d.data.MediaObject;
import com.android.gallery3d.data.Path;
import com.android.gallery3d.glrenderer.GLCanvas;
import com.android.gallery3d.glrenderer.Texture;

import com.mediatek.gallery3d.util.Log;
import com.mediatek.gallerybasic.base.ExtItem.SupportOperation;
import com.mediatek.gallerybasic.base.MediaData;
import com.mediatek.gallerybasic.base.ThumbType;
import com.mediatek.galleryportable.StorageManagerUtils;
import com.mediatek.galleryportable.PerfServiceUtils;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.concurrent.atomic.AtomicInteger;

public class FeatureHelper {
    private static final String TAG = "MtkGallery2/FeatureHelper";

    public static final String EXTRA_ENABLE_VIDEO_LIST = "mediatek.intent.extra.ENABLE_VIDEO_LIST";
    // Add for video thumbnail performance.
    public static final int BOOST_VIDEO_DECODE_TIME_OUT = 300;

    private static final String CACHE_SUFFIX = "/Android/data/com.android.gallery3d/cache";
    private static StorageManager sStorageManager = null;
    // Add for first launch performance.
    private static final int BOOST_POLICY_TIME_OUT = 1500;

    private static final HashMap<SupportOperation, Integer> sSpMap =
            new HashMap<SupportOperation, Integer>();

    static {
        sSpMap.put(SupportOperation.DELETE, MediaObject.SUPPORT_DELETE);
        sSpMap.put(SupportOperation.ROTATE, MediaObject.SUPPORT_ROTATE);
        sSpMap.put(SupportOperation.SHARE, MediaObject.SUPPORT_SHARE);
        sSpMap.put(SupportOperation.CROP, MediaObject.SUPPORT_CROP);
        sSpMap.put(SupportOperation.SHOW_ON_MAP,
                MediaObject.SUPPORT_SHOW_ON_MAP);
        sSpMap.put(SupportOperation.SETAS, MediaObject.SUPPORT_SETAS);
        sSpMap.put(SupportOperation.FULL_IMAGE, MediaObject.SUPPORT_FULL_IMAGE);
        sSpMap.put(SupportOperation.PLAY, MediaObject.SUPPORT_PLAY);
        sSpMap.put(SupportOperation.CACHE, MediaObject.SUPPORT_CACHE);
        sSpMap.put(SupportOperation.EDIT, MediaObject.SUPPORT_EDIT);
        sSpMap.put(SupportOperation.INFO, MediaObject.SUPPORT_INFO);
        sSpMap.put(SupportOperation.TRIM, MediaObject.SUPPORT_TRIM);
        sSpMap.put(SupportOperation.UNLOCK, MediaObject.SUPPORT_UNLOCK);
        sSpMap.put(SupportOperation.BACK, MediaObject.SUPPORT_BACK);
        sSpMap.put(SupportOperation.ACTION, MediaObject.SUPPORT_ACTION);
        sSpMap.put(SupportOperation.CAMERA_SHORTCUT,
                MediaObject.SUPPORT_CAMERA_SHORTCUT);
        sSpMap.put(SupportOperation.MUTE, MediaObject.SUPPORT_MUTE);
        sSpMap.put(SupportOperation.PRINT, MediaObject.SUPPORT_PRINT);
    }

    public static int mergeSupportOperations(int originSp,
                                             ArrayList<SupportOperation> exSp,
                                             ArrayList<SupportOperation> exNotSp) {
        if (exSp != null && exSp.size() != 0) {
            int size = exSp.size();
            for (int i = 0; i < size; i++) {
                originSp |= sSpMap.get(exSp.get(i));
            }
        }
        if (exNotSp != null && exNotSp.size() != 0) {
            int size = exNotSp.size();
            for (int i = 0; i < size; i++) {
                originSp &= ~sSpMap.get(exNotSp.get(i));
            }
        }
        return originSp;
    }

    public static ThumbType convertToThumbType(int type) {
        switch (type) {
            case MediaItem.TYPE_THUMBNAIL:
                return ThumbType.MIDDLE;
            case MediaItem.TYPE_MICROTHUMBNAIL:
                return ThumbType.MICRO;
            case MediaItem.TYPE_FANCYTHUMBNAIL:
                return ThumbType.FANCY;
            case MediaItem.TYPE_HIGHQUALITYTHUMBNAIL:
                return ThumbType.HIGHQUALITY;
            default:
                Log.e(TAG, "<covertToThumbType> not support type");
                assert (false);
                return null;
        }
    }

    public static void setExtBundle(AbstractGalleryActivity activity,
                                    Intent intent, Bundle data, Path path) {
        /// [Split up into camera and gallery] @{
        data.putBoolean(PhotoPage.KEY_LAUNCH_FROM_CAMERA,
                intent.getBooleanExtra(PhotoPage.KEY_LAUNCH_FROM_CAMERA, false));
        /// @}
        MediaObject object = activity.getDataManager().getMediaObject(path);
        if (object instanceof MediaItem) {
            MediaItem item = (MediaItem) object;
            MediaData md = item.getMediaData();
            /// M: [FEATURE.ADD] [Camera independent from Gallery] @{
            // Add for launch from secure camera
            if (intent.getExtras() != null
                    && intent.getBooleanExtra(PhotoPage.IS_SECURE_CAMERA, false)
                    && intent.getExtras().getSerializable(PhotoPage.SECURE_ALBUM) != null) {
                data.putSerializable(PhotoPage.SECURE_ALBUM,
                        intent.getExtras().getSerializable(PhotoPage.SECURE_ALBUM));
                data.putString(PhotoPage.KEY_MEDIA_SET_PATH,
                        intent.getStringExtra(PhotoPage.SECURE_PATH));
                data.putBoolean(PhotoPage.IS_SECURE_CAMERA,
                        intent.getBooleanExtra(PhotoPage.IS_SECURE_CAMERA, false));
            }
            /// @}
        }
    }

    public static Uri tryContentMediaUri(Context context, Uri uri) {
        if (null == uri) {
            return null;
        }

        String scheme = uri.getScheme();
        UriMatcher uriMatcher = new UriMatcher(UriMatcher.NO_MATCH);
        uriMatcher.addURI(MediaStore.AUTHORITY, "external/file/#", 1);
        if (uriMatcher.match(uri) == 1) {
            return getUriById(context, uri);
        }
        if (!ContentResolver.SCHEME_FILE.equals(scheme)) {
            return uri;
        } else {
            String path = uri.getPath();
            Log.d(TAG, "<tryContentMediaUri> for " + path);
            if (!new File(path).exists()) {
                return null;
            }
        }
        return getUriByPath(context, uri);
    }

    private static Uri getUriByPath(Context context, Uri uri) {
        Cursor cursor = null;
        try {
            // for file kinds of uri, query media database
            cursor = Media.query(context.getContentResolver(),
                    MediaStore.Files.getContentUri("external"), new String[]{
                            Media._ID, Media.MIME_TYPE, Media.BUCKET_ID}, "_data=(?)",
                    new String[]{uri.getPath()}, null); // " bucket_id ASC, _id ASC");
            if (null != cursor && cursor.moveToNext()) {
                long id = cursor.getLong(0);
                String mimeType = cursor.getString(1);
                String contentUri = null;
                Uri resultUri = null;
                if (mimeType == null) {
                    Log.e(TAG, "<getUriByPath> mimeType == null, Please check " +
                            " the uri" + uri);
                    return null;
                }
                if (mimeType.startsWith("image/")) {
                    contentUri = Media.getContentUri("external").toString();
                } else if (mimeType.startsWith("video/")) {
                    contentUri = MediaStore.Video.Media.getContentUri("external").toString();
                } else {
                    Log.d(TAG, "<getUriByPath> id = " + id + ", mimeType = " + mimeType
                            + ", not begin with image/ or video/, return uri " + uri);
                    return uri;
                }
                resultUri = Uri.parse(contentUri + "/" + id);
                Log.d(TAG, "<getUriByPath> got " + resultUri);
                return resultUri;
            } else {
                Log.w(TAG, "<getUriByPath> fail to convert " + uri);
                return uri;
            }
        } finally {
            if (null != cursor) {
                cursor.close();
                cursor = null;
            }
        }
    }

    private static Uri getUriById(Context context, Uri uri) {
        Cursor cursor = null;
        Cursor resultCursor = null;
        try {
            cursor = Media.query(context.getContentResolver(),
                    MediaStore.Files.getContentUri("external"), new String[]{
                            Media._ID, Media.MIME_TYPE, Media.BUCKET_ID}, "_id=(?)",
                    new String[]{uri.getLastPathSegment()}, null);
            if (null != cursor && cursor.moveToNext()) {
                long id = cursor.getLong(0);
                String mimeType = cursor.getString(1);
                String contentUri = null;
                Uri resultUri = null;
                if (mimeType.startsWith("image/")) {
                    contentUri = Media.getContentUri("external").toString();
                } else if (mimeType.startsWith("video/")) {
                    contentUri = MediaStore.Video.Media.getContentUri("external").toString();
                } else {
                    Log.d(TAG, "<getUriById> id = " + id + ", mimeType = " + mimeType
                            + ", not begin with image/ or video/, return uri " + uri);
                    return uri;
                }
                resultUri = Uri.parse(contentUri + "/" + id);
                resultCursor = context.getContentResolver().query(
                        resultUri, null, null, null, null);
                if (null != resultCursor && resultCursor.moveToNext()) {
                    Log.d(TAG, "<getUriById> got " + resultUri);
                    return resultUri;
                } else {
                    Log.w(TAG, "<getUriById> fail to convert " + uri);
                    return uri;
                }
            } else {
                Log.w(TAG, "<getUriById> fail to convert " + uri);
                return uri;
            }
        } finally {
            if (null != cursor) {
                cursor.close();
                cursor = null;
            }
            if (null != resultCursor) {
                resultCursor.close();
                resultCursor = null;
            }
        }
    }

    public static File getExternalCacheDir(Context context) {
        if (context == null) {
            Log.e(TAG, "<getExternalCacheDir> context is null, return null");
            return null;
        }
        String internalStoragePath = StorageManagerUtils.getStorageForCache(context);
        if (internalStoragePath == null || internalStoragePath.equals("")) {
            Log.e(TAG, "<getExternalCacheDir> internalStoragePath is null, return null");
            return null;
        }
        // get cache directory on internal storage or phone storage
        String cachePath = internalStoragePath + CACHE_SUFFIX;
        Log.d(TAG, "<getExternalCacheDir> return external cache dir is " + cachePath);
        File result = new File(cachePath);
        if (result.exists()) {
            return result;
        }
        if (result.mkdirs()) {
            return result;
        }
        Log.e(TAG, "<getExternalCacheDir> Fail to create external cache dir, return null");
        return null;
    }

    public static String getDefaultPath() {
        String path = StorageManagerUtils.getDefaultPath();
        return path;
    }

    public static String getDefaultStorageState(Context context) {
        if (sStorageManager == null && context == null) {
            return null;
        }
        if (sStorageManager == null) {
            sStorageManager = (StorageManager) context
                    .getSystemService(Context.STORAGE_SERVICE);
        }
        String path = StorageManagerUtils.getDefaultPath();
        if (path == null || path.equals("")) {
            return null;
        }
        String volumeState = null;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            StorageVolume storageVolume = sStorageManager.getStorageVolume(new File(path));
            // add null point check to avoid getting null volume when power off
            if (storageVolume != null) {
                volumeState = storageVolume.getState();
            }
        } else if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
            volumeState = Environment.getStorageState(new File(path));
        } else {
            volumeState = EnvironmentCompat.getStorageState(new File(path));
        }
        Log.v(TAG, "<getDefaultStorageState> default path = " + path
                + ", state = " + volumeState);
        return volumeState;
    }

    private static void drawRightBottom(GLCanvas canvas, Texture tex, int x,
                                        int y, int width, int height, float scale) {
        if (null == tex) {
            return;
        }
        int texWidth = (int) ((float) tex.getWidth() * scale);
        int texHeight = (int) ((float) tex.getHeight() * scale);
        tex.draw(canvas, x + width - texWidth, y + height - texHeight,
                texWidth, texHeight);
    }

    public static boolean isLocalUri(Uri uri) {
        if (uri == null) {
            return false;
        }
        boolean isLocal = ContentResolver.SCHEME_FILE.equals(uri.getScheme());
        isLocal |= ContentResolver.SCHEME_CONTENT.equals(uri.getScheme())
                && MediaStore.AUTHORITY.equals(uri.getAuthority());
        return isLocal;
    }

    public static MediaDetails convertStringArrayToDetails(String[] array) {
        if (array == null || array.length < 1) {
            return null;
        }
        MediaDetails res = new MediaDetails();
        for (int i = 0; i < array.length; i++) {
            if (array[i] != null) {
                res.addDetail(i + 1, array[i]);
            }
        }
        return res;
    }

    // HW limitation @{
    private static final int JPEG_DECODE_LENGTH_MAX = 8192;

    public static boolean isJpegOutOfLimit(String mimeType, int width,
                                           int height) {
        if (mimeType.equals("image/jpeg")
                && (width > JPEG_DECODE_LENGTH_MAX || height > JPEG_DECODE_LENGTH_MAX)) {
            return true;
        }
        return false;
    }
    // @}

    public static boolean isDefaultStorageMounted(Context context) {
        String defaultStorageState = getDefaultStorageState(context);
        if (defaultStorageState == null) {
            defaultStorageState = Environment.getExternalStorageState();
        }
        return Environment.MEDIA_MOUNTED.equalsIgnoreCase(defaultStorageState);
    }

    /// M: [BUG.ADD] read DNG EXIF details. @{
    public static int getOrientationFromExif(String filePath, InputStream is) {
        int orientation = 0;
        int rotation = 0;
        try {
            ExifInterface exif;
            if (filePath != null && !filePath.equals("")) {
                exif = new ExifInterface(filePath);
            } else if (null != is &&
                       android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.N) {
                exif = new ExifInterface(is);
            } else {
                Log.d(TAG, "<getOrientationFromExif> sdk version issue, return 0");
                return 0;
            }
            orientation =
                    exif.getAttributeInt(android.media.ExifInterface.TAG_ORIENTATION,
                            android.media.ExifInterface.ORIENTATION_UNDEFINED);
        } catch (IOException e) {
            Log.e(TAG, "<getOrientationFromExif> IOException", e);
            return 0;
        }
        Log.d(TAG, "<getOrientationFromExif> exif orientation: " + orientation);
        switch (orientation) {
            case android.media.ExifInterface.ORIENTATION_NORMAL:
                rotation = 0;
                break;
            case android.media.ExifInterface.ORIENTATION_ROTATE_90:
                rotation = 90;
                break;
            case android.media.ExifInterface.ORIENTATION_ROTATE_180:
                rotation = 180;
                break;
            case android.media.ExifInterface.ORIENTATION_ROTATE_270:
                rotation = 270;
                break;
            default:
                rotation = 0;
                break;
        }
        Log.d(TAG, "<getOrientationFromExif> rotation: " + rotation);
        return rotation;
    }
    /// @}

    private static final AtomicInteger sNextGeneratedViewId = new AtomicInteger(1);

    public static int generateViewId() {
        for (; ; ) {
            final int result = sNextGeneratedViewId.get();
            // aapt-generated IDs have the high byte nonzero; clamp to the range under that.
            int newValue = result + 1;
            if (newValue > 0x00FFFFFF) newValue = 1; // Roll over to 1, not 0.
            if (sNextGeneratedViewId.compareAndSet(result, newValue)) {
                return result;
            }
        }
    }

    /**
     * Modify CPU boost policy for first launch performance.
     *
     * @param context
     *            GalleryActivity
     */
    public static void modifyBoostPolicy(Context context) {
        if (!isCacheFileExists(context)) {
            Log.i(TAG, "<modifyBoostPolicy> gallery cache not exists, and execute boost policy");
            PerfServiceUtils.boostEnableTimeoutMs(BOOST_POLICY_TIME_OUT);
        }
    }

    /**
     * Check if launch gallery for the first time by check cache exists.
     * @param context
     *            Application context
     * @return cache file exists status
     */
    public static boolean isCacheFileExists(Context context) {
        File cacheDir = getExternalCacheDir(context);
        if (cacheDir == null) {
            return false;
        }
        File[] fs = cacheDir.listFiles();
        if (fs == null || fs.length == 0) {
            return false;
        }
        for (File file : fs) {
            if (file.getName().endsWith("idx")) {
                Log.d(TAG, "<isCacheFileExists> File cache exists!");
                return true;
            }
        }
        return false;
    }
}
