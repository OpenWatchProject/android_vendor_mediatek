package com.mediatek.galleryfeature.drm;

import android.content.ContentResolver;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteException;
import android.drm.DrmManagerClient;
import android.drm.DrmStore;
import android.graphics.Bitmap;
import android.net.Uri;
import android.provider.MediaStore;
import android.os.SystemProperties;


import com.mediatek.gallerybasic.gl.MGLCanvas;
import com.mediatek.gallerybasic.gl.MTexture;
import com.mediatek.gallerybasic.util.Log;
import com.mediatek.gallerybasic.util.Utils;
import com.mediatek.galleryportable.SystemPropertyUtils;
import com.mediatek.omadrm.OmaDrmStore;
import com.mediatek.omadrm.OmaDrmUtils;

import java.lang.reflect.Field;

public class DrmHelper {
    private static final String TAG = "MtkGallery2/DrmHelper";
    public static final String CTA_DATA_PROTECTION_SUFFIX = ".mudp";
    public static final String DRM_SUFFIX_DCF = ".dcf";
    public static final String DRM_SUFFIX_FL = ".fl";
    public static final String DRM_SUFFIX_DM = ".dm";

    public static final String PLACE_HOLDER_COLOR = "#333333";
    public static final String DRM_PROPERTY = "ro.vendor.mtk_oma_drm_support";
    public static final String CTA_PROPERTY = "ro.vendor.mtk_cta_set";
    private static final String DRM_URI_FIELD_NAME = "DRM_CONTENT_URI";
    private static final String DEFAULT_DRM_URI = "drm_content_uri";
    private static String sDrmContentUri = null;
    public static boolean sSupportDrm;
    public static boolean sSupportCTA;
    public static boolean sSupportDrmPlugin;
    private volatile static DrmManagerClient sClient = null;

    static {
        sSupportDrm = true;//SystemPropertyUtils.get(DRM_PROPERTY).equals("1");
        sSupportCTA = SystemPropertyUtils.get(CTA_PROPERTY).equals("1");
        sSupportDrmPlugin = SystemPropertyUtils.get(DRM_PROPERTY).equals("1");
    }

    public static DrmManagerClient getOmaDrmClient(Context context) {
        if (sClient == null) {
            synchronized (DrmManagerClient.class) {
                if (sClient == null) {
                    sClient = new DrmManagerClient(context);
                }
            }
        }
        return sClient;
    }

    public static boolean checkRightsStatus(Context context, String filePath, int action) {
        if (null == context || null == filePath || filePath.equals("")) {
            Log.e(TAG, "<checkRightsStatus> invalid arguments, context = "
                    + context + ", file path = " + filePath);
            return false;
        }
        int valid = getOmaDrmClient(context).checkRightsStatus(filePath, action);
        return valid == DrmStore.RightsStatus.RIGHTS_VALID;
    }

    public static boolean hasRightsToShow(Context context, String filePath, boolean isVideo) {
        int action = isVideo ? DrmStore.Action.PLAY : DrmStore.Action.DISPLAY;
        return checkRightsStatus(context, filePath, action);
    }

    public static boolean isFLDrm(int drmMethod) {
        return drmMethod == OmaDrmStore.Method.FL;
    }

    public static boolean isDrmFile(String filePath) {
      boolean result = false;
      if (filePath != null &&
          (filePath.toLowerCase().endsWith(DRM_SUFFIX_DCF) ||
           filePath.toLowerCase().endsWith(DRM_SUFFIX_FL) ||
           filePath.toLowerCase().endsWith(DRM_SUFFIX_DM))) {
         result = true;
      }
      return result;
    }

    public static boolean isDataProtectionFile(String filePath) {
        return filePath != null && filePath.toLowerCase().endsWith(CTA_DATA_PROTECTION_SUFFIX);
    }

    public static Bitmap getLockIcon(Context context, String filePath) {
        if (null == context || null == filePath || filePath.equals("")) {
            Log.e(TAG, "<getLockIcon> invalid arguments, context = " + context
                    + ", file path = " + filePath);
            return null;
        }
        if (sSupportDrmPlugin) {
            return OmaDrmUtils.getOriginalLockIcon(DrmHelper.getOmaDrmClient(context), context
                    .getResources(), filePath);
        } else {
            return null;
        }
    }

    public static void clearToken(Context context, String tokenKey, String token) {
        OmaDrmUtils.clearToken(getOmaDrmClient(context), tokenKey, token);
    }

    public static boolean isTokenValid(Context context, String tokenKey, String token) {
        return OmaDrmUtils.isTokenValid(getOmaDrmClient(context), tokenKey, token);
    }

    public static byte[] forceDecryptFile(String filePath, boolean consume) {
        if (null == filePath
                || (!filePath.toLowerCase().endsWith(DRM_SUFFIX_DCF)
                && !filePath.toLowerCase().endsWith(CTA_DATA_PROTECTION_SUFFIX)
                && !filePath.toLowerCase().endsWith(DRM_SUFFIX_FL))) {
            return null;
        }
        return OmaDrmUtils.forceDecryptFile(filePath, consume);
    }

    /**
     * Create dialog for display drm image information. If there has special characters file scheme
     * Uri, getPath function should truncate the Uri. So should use substring function for absolute
     * path.
     *
     * @param context for show protection Info Dialog.
     * @param uri     the uri of the drm image.
     */
    public static void showProtectionInfoDialog(final Context context, final Uri uri) {
        if (null == context || null == uri) {
            Log.e(TAG, "<showProtectionInfoDialog> invalid arguments, context = " + context
                    + ", uri = " + uri);
            return;
        }
        if (ContentResolver.SCHEME_FILE.equals(uri.getScheme())
                && Utils.hasSpecialCharaters(uri)) {
            String filePath = uri.toString().substring(Utils.SIZE_SCHEME_FILE);
            OmaDrmUtils.showProtectionInfoDialog(context, getOmaDrmClient(context), filePath);
        } else {
            OmaDrmUtils.showProtectionInfoDialog(context, getOmaDrmClient(context), uri);
        }
    }

    /**
     * Query filePath by Uri.
     *
     * @param context The context for query.
     * @param uri     The file Uri.
     * @return The file path.
     */
    public static String convertUriToPath(Context context, Uri uri) {
        if (null == context || null == uri) {
            Log.e(TAG, "<convertUriToPath> invalid arguments, context = " + context
                    + ", uri = " + uri);
            return null;
        }
        String path = null;
        String scheme = uri.getScheme();
        if (null == scheme || scheme.equals("")
                || scheme.equals(ContentResolver.SCHEME_FILE)) {
            path = uri.getPath();
        } else if (scheme.equals("http")) {
            path = uri.toString();
        } else if (scheme.equals(ContentResolver.SCHEME_CONTENT)) {
            String[] projection = new String[]{
                    MediaStore.MediaColumns.DATA
            };
            Cursor cursor = null;
            try {
                cursor =
                        context.getContentResolver().query(uri, projection, null, null,
                                null);
                if (null == cursor || 0 == cursor.getCount() || !cursor.moveToFirst()) {
                    throw new IllegalArgumentException("Given Uri could not be found"
                            + " in media store");
                }
                int pathIndex = cursor.getColumnIndexOrThrow(MediaStore.MediaColumns.DATA);
                path = cursor.getString(pathIndex);
            } catch (SQLiteException e) {
                throw new IllegalArgumentException("Given Uri is not formatted in a way "
                        + "so that it can be found in media store.");
            } finally {
                if (null != cursor) {
                    cursor.close();
                }
            }
        } else {
            throw new IllegalArgumentException("Given Uri scheme is not supported");
        }
        return path;
    }

    public static void updateFeatureOption() {
        if (SystemPropertyUtils.get(DRM_PROPERTY).equals("1")) {
            sSupportDrm = true;
        }
        if (SystemPropertyUtils.get(CTA_PROPERTY).equals("1")) {
            sSupportCTA = true;
        }
    }

    public static void drawRightBottom(MGLCanvas canvas, MTexture texture, int x, int y, int
            width, int height, float scale) {
        if (null == canvas || null == texture) {
            Log.e(TAG, "<drawRightBottom> invalid arguments, canvas = " + canvas
                    + ", texture = " + texture);
            return;
        }
        int texWidth = (int) (texture.getWidth() * scale);
        int texHeight = (int) (texture.getHeight() * scale);
        texture.draw(canvas, x + width - texWidth, y + height - texHeight, texWidth, texHeight);
    }

    public static String getDrmContentUri() {
        if (sDrmContentUri != null) {
            return sDrmContentUri;
        }
        Log.i(TAG, "<getDrmContentUri> try to get MtkMediaStore$MediaColumns.DRM_CONTENT_URI");
        try {
            Class<?> clazz =
                    DrmHelper.class.getClassLoader().loadClass(
                            "com.mediatek.media.MtkMediaStore$MediaColumns");
            Log.i(TAG, "<getDrmContentUri> clazz: " + clazz);
            Field field = clazz.getField(DRM_URI_FIELD_NAME);
            Object value = field.get(null);
            if (value != null) {
                sDrmContentUri = value.toString();
                Log.i(TAG, "<getDrmContentUri> MtkMediaStore$MediaColumns.DRM_CONTENT_URI: "
                        + sDrmContentUri);
                return sDrmContentUri;
            }
        } catch (ClassNotFoundException e) {
            Log.e(TAG, "<getDrmContentUri> ClassNotFoundException", e);
        } catch (IllegalAccessException e) {
            Log.e(TAG, "<getDrmContentUri> IllegalAccessException", e);
        } catch (IllegalArgumentException e) {
            Log.e(TAG, "<getDrmContentUri> IllegalArgumentException", e);
        } catch (ExceptionInInitializerError e) {
            Log.e(TAG, "<getDrmContentUri> ExceptionInInitializerError", e);
        } catch (NoSuchFieldException e) {
            Log.e(TAG, "<getDrmContentUri> NoSuchFieldException", e);
        } catch (NullPointerException e) {
            Log.e(TAG, "<getDrmContentUri> NullPointerException", e);
        }

        Log.i(TAG, "<getDrmContentUri> try to get MediaStore$MediaColumns.DRM_CONTENT_URI");
        try {
            Class<?> clazz =
                    DrmHelper.class.getClassLoader().loadClass(
                            "android.provider.MediaStore$MediaColumns");
            Log.i(TAG, "<getDrmContentUri> clazz: " + clazz);
            Field field = clazz.getField(DRM_URI_FIELD_NAME);
            Object value = field.get(null);
            if (value != null) {
                sDrmContentUri = value.toString();
                Log.i(TAG, "<getDrmContentUri> MediaStore$MediaColumns.DRM_CONTENT_URI: "
                        + sDrmContentUri);
                return sDrmContentUri;
            }
        } catch (ClassNotFoundException e) {
            Log.e(TAG, "<getDrmContentUri> ClassNotFoundException", e);
        } catch (IllegalAccessException e) {
            Log.e(TAG, "<getDrmContentUri> IllegalAccessException", e);
        } catch (IllegalArgumentException e) {
            Log.e(TAG, "<getDrmContentUri> IllegalArgumentException", e);
        } catch (ExceptionInInitializerError e) {
            Log.e(TAG, "<getDrmContentUri> ExceptionInInitializerError", e);
        } catch (NoSuchFieldException e) {
            Log.e(TAG, "<getDrmContentUri> NoSuchFieldException", e);
        } catch (NullPointerException e) {
            Log.e(TAG, "<getDrmContentUri> NullPointerException", e);
        }
        Log.i(TAG, "<getDrmContentUri> return DEFAULT_DRM_URI: " + DEFAULT_DRM_URI);
        return DEFAULT_DRM_URI;
    }
}
