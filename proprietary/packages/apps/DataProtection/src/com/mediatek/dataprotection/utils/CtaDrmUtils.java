package com.mediatek.dataprotection.utils;

import android.drm.DrmInfoRequest;
import android.drm.DrmManagerClient;
import android.drm.DrmInfo;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.media.MediaMetadataRetriever;

import com.mediatek.dcfdecoder.DcfDecoder;
import com.mediatek.dataprotection.utils.Log;

import java.io.FileDescriptor;
import java.io.UnsupportedEncodingException;

public class CtaDrmUtils {
    public static final String TAG = "CtaDrmUtils";

    public static final String MIME_TYPE_CTA5_MESSAGE = "application/vnd.mtk.cta5.message";

    public static final String KEY_ACTION = "action";

    public static final String KEY_CTA5_CLEAR_FD = "CTA5clearFd";
    public static final String KEY_CTA5_CIPHER_FD = "CTA5cipherFd";
    public static final String KEY_CTA5_FD = "CTA5Fd";
    public static final String KEY_CTA5_KEY = "CTA5key";
    public static final String KEY_CTA5_OLDKEY = "CTA5oldKey";
    public static final String KEY_CTA5_NEWKEY = "CTA5newKey";
    public static final String KEY_CTA5_RAW_MIME = "CTA5rawMime";
    public static final String KEY_CTA5_TOKEN = "CTA5Token";
    public static final String KEY_CTA5_FILEPATH = "CTA5FilePath";

    public static final String ACTION_CTA5_ENCRYPT = "CTA5Encrypt";
    public static final String ACTION_CTA5_DECRYPT = "CTA5Decrypt";
    public static final String ACTION_CTA5_SETKEY = "CTA5SetKey";
    public static final String ACTION_CTA5_CHANGEPASSWORD = "CTA5ChangePassword";
    public static final String ACTION_CTA5_GETPROGESS = "CTA5GetProgress";
    public static final String ACTION_CTA5_CANCEL = "CTA5Cancel";
    public static final String ACTION_CTA5_ISCTAFILE = "CTA5IsCtaFile";
    public static final String ACTION_CTA5_GETTOKEN = "CTA5Gettoken";
    public static final String ACTION_CTA5_CHECKTOKEN = "CTA5Checktoken";
    public static final String ACTION_CTA5_CLEARTOKEN = "CTA5Cleartoken";
    public static final String CTA5_ERRORTOKEN = "error";

    /**
     * No error.
     */
    public static int CTA_ERROR_NONE = 0;
    /**
     * A normal error.
     */
    public static int CTA_ERROR_GENRIC = -1;
    /**
     * The key is wrong.
     */
    public static int CTA_ERROR_BADKEY = -2;
    /**
     * The space is not enough.
     */
    public static int CTA_ERROR_NOSPACE = -3;
    /**
     * A encrypt/decrypt process is done which cannot be cancled.
     */
    public static int CTA_ERROR_CANCEL = -5;
    /**
     * Input is invalid
     */
    public static int CTA_ERROR_INVALID_INPUT = -6;
    /**
     * Enrypt or Decrypt error.
     */
    public static int CTA_ERROR = -7;
    /**
     * Encrypt or Decrypt done.
     */
    public static int CTA_DONE = 100;
    /**
     * Encrypt or Decrypt canceled.
     */
    public static int CTA_CANCEL_DONE = 101;
    /**
     * Encrypt or Decrypt updating.
     */
    public static int CTA_UPDATING = 102;

    /**
     * Multimedia file encrypt done.
     */
    public static int CTA_MULTI_MEDIA_ENCRYPT_DONE = 110;

    /**
     * Multimedia file decrypt done.
     */
    public static int CTA_MULTI_MEDIA_DECRYPT_DONE = 111;

    public static final String RESULT_SUCCESS = "success";
    public static final String RESULT_FAILURE = "failure";

    private DrmManagerClient mClient;

    public CtaDrmUtils(DrmManagerClient client) {
        mClient = client;
    }

    /**
     * Set key used to decrypt/encrypt
     *
     * @param key A key used to decrypt/encrypt cta file
     * return {@link CTA_ERROR_NONE} if sucessfully, otherwise return {@link CTA_ERROR_GENRIC}
     */
    public static int setKey(DrmManagerClient client, byte[] key)
    {
        Log.d(TAG, "setKey()");
        DrmInfoRequest request = new DrmInfoRequest(DrmInfoRequest.TYPE_SET_OMA_DRM_INFO,
                MIME_TYPE_CTA5_MESSAGE);
        request.put(KEY_ACTION, ACTION_CTA5_SETKEY);
        request.put(KEY_CTA5_KEY, key);
        DrmInfo info = client.acquireDrmInfo(request);
        return CTA_ERROR_NONE;
    }

    /**
     * Get image thumbnail.
     * @param client DrmManagerClient instance.
     * @param path which file should be used
     * @param options width,height of this bitmap
     * @return thumbnail of the bitmap
     */
    public static Bitmap getImageThumbnail(DrmManagerClient client,
            String path, BitmapFactory.Options options) {
        Log.d(TAG, "getImageThumbnail");
        Bitmap result = null;
        if (null == client) {
            Log.e(TAG, "DrmManagerClient instance is null.");
            return result;
        }
        int width = 96;
        int height = 96;
        if (null == path) {
            Log.e(TAG, "getImageThumbnail() path is null");
            return result;
        }
        /*
         * buffer may be nul
         */
        if (client.canHandle(path, null)) {
            // check if this file is drm and can get decrypted buffer
            DcfDecoder dcfDecoder = new DcfDecoder();
            byte[] buffer = dcfDecoder.forceDecryptFile(path, false);
            if (null == buffer) {
                Log.e(TAG, "getImageThumbnail() file is not exist:" + path);
                return null;
            }
            BitmapFactory.Options opts = new BitmapFactory.Options();
            opts.inJustDecodeBounds = true;
            BitmapFactory.decodeByteArray(buffer, 0, buffer.length, opts);

            opts.inJustDecodeBounds = false;
            // calc
            int h = opts.outHeight;
            int w = opts.outWidth;
            int beW = w / width;
            int beH = h / height;
            int be = (beW < beH) ? beH : beW;
            opts.inSampleSize = be;
            // not show "DRM License expired" when no rights to decode image
            if (isDrmFile(buffer)) {
                opts.inSampleSize |= 0x100;
            }
            Bitmap btm = BitmapFactory.decodeByteArray(buffer, 0, buffer.length, opts);
            return btm;
        }
        return result;
    }

    /**
     * Get video thumbnail
     * @param client DrmManagerClient instance.
     * @param path which file should be used
     * @param targetWidth width of this thumbnail
     * @return thumbnail of the video
     */
    public static Bitmap getVideoThumbnail(DrmManagerClient client,
            String path, int targetWidth) {
        Log.v(TAG, "getVideoThumbnail honge ,targetWidth: " + targetWidth);
        Bitmap bitmap = null;
        if (null == client) {
            Log.e(TAG, "DrmManagerClient instance is null.");
            return bitmap;
        }
        if (!client.canHandle(path, null)) {
            Log.w(TAG, "getVideoThumbnail honge ,not drm file");
            return bitmap;
        }
        MediaMetadataRetriever retriever = new MediaMetadataRetriever();
        try {
            if (path != null) {
                retriever.setDataSource(path);
            }
            bitmap = retriever.getFrameAtTime(-1);
        } catch (IllegalArgumentException ex) {
            // Assume this is a corrupt video file
        } catch (RuntimeException ex) {
            // Assume this is a corrupt video file.
        } finally {
            try {
                retriever.release();
            } catch (RuntimeException ex) {
                // Ignore failures while cleaning up.
                Log.e(TAG, "retriever release error.");
            }
        }
        if (bitmap == null)
            return null;

        // Scale down the bitmap if it is bigger than we need.
        int width = bitmap.getWidth();
        int height = bitmap.getHeight();
        if (width > targetWidth) {
            float scale = (float) targetWidth / width;
            int w = Math.round(scale * width);
            int h = Math.round(scale * height);
            bitmap = Bitmap.createScaledBitmap(bitmap, w, h, true);
        }
        return bitmap;
    }

    /**
     * get token from native
     */
    public static String getToken(DrmManagerClient client, String filePath) {
        Log.s(TAG, "getToken filePath:" + filePath);
        String result = null;
        DrmInfoRequest request = new DrmInfoRequest(DrmInfoRequest.TYPE_GET_OMA_DRM_INFO,
                MIME_TYPE_CTA5_MESSAGE);
        request.put(KEY_ACTION, ACTION_CTA5_GETTOKEN);
        request.put(KEY_CTA5_FILEPATH, filePath);
        DrmInfo info = client.acquireDrmInfo(request);
        result = getResultFromDrmInfo(info);
        return result;
    }

    /**
     * Encrypt clear file to chipher file. It's a asynchronize method.
     *
     * @param clear_fd A file descriptor used to be encrypted
     * @param cipher_fd A file descriptor used to be stored chipher content
     * @return {@link CTA_ERROR_NONE} if encrypt successfully,
     *         otherwise return {@link CTA_ERROR_NOSPACE}, {@link CTA_ERROR_CANCLED_BY_USER}
     */
    public static int encrypt(DrmManagerClient client, FileDescriptor clear_fd,
            FileDescriptor cipher_fd) {
        Log.d(TAG, "encrypt() : cipher_fd = " + cipher_fd + ", clear_fd = " + clear_fd);
        if (clear_fd == null || cipher_fd == null) {
            Log.e(TAG, "encrypt bad input parameters");
            return CTA_ERROR_INVALID_INPUT;
        }
        DrmInfoRequest request = new DrmInfoRequest(DrmInfoRequest.TYPE_SET_OMA_DRM_INFO,
                MIME_TYPE_CTA5_MESSAGE);
        request.put(KEY_ACTION, ACTION_CTA5_ENCRYPT);
        request.put(KEY_CTA5_CLEAR_FD, clear_fd);
        request.put(KEY_CTA5_CIPHER_FD, cipher_fd);
        DrmInfo info = client.acquireDrmInfo(request);
        return CTA_ERROR_NONE;
    }

    /**
     * Encrypt clear file to chipher file. It's a asynchronize method.
     *
     * @param clear_fd A file descriptor used to be encrypted
     * @param cipher_fd A file descriptor used to be stored chipher content
     * @param mime The mime type of clear file
     * @return {@link CTA_ERROR_NONE} if encrypt successfully,
     *         otherwise return {@link CTA_ERROR_NOSPACE}, {@link CTA_ERROR_CANCLED_BY_USER}
     */
    public static int encrypt(DrmManagerClient client, FileDescriptor clear_fd,
            FileDescriptor cipher_fd, String mime) {
        Log.d(TAG, "encrypt() : cipher_fd = " + cipher_fd + ", clear_fd = "
            + clear_fd + "mime = " + mime);
        if (clear_fd == null || cipher_fd == null) {
            Log.e(TAG, "encrypt bad input parameters");
            return CTA_ERROR_INVALID_INPUT;
        }
        if (mime == null) {
            return encrypt(client, clear_fd, cipher_fd);
        }
        DrmInfoRequest request = new DrmInfoRequest(DrmInfoRequest.TYPE_SET_OMA_DRM_INFO,
                MIME_TYPE_CTA5_MESSAGE);
        request.put(KEY_ACTION, ACTION_CTA5_ENCRYPT);
        request.put(KEY_CTA5_CLEAR_FD, clear_fd);
        request.put(KEY_CTA5_CIPHER_FD, cipher_fd);
        request.put(KEY_CTA5_RAW_MIME, mime);
        DrmInfo info = client.acquireDrmInfo(request);

        return CTA_ERROR_NONE;
    }

    /**
     * Decrypt chipher file to clear file. It's a asynchronize method.
     *
     * @param cipher_fd A file descriptor used to be decrypted
     * @param clear_fd A file descriptor used to be stored clear content
     * @return {@link CTA_ERROR_NONE} if encrypt successfully,
     *         otherwise return {@link CTA_ERROR_BADKEY}, {@link CTA_ERROR_NOSPACE},
     *         {@link CTA_ERROR_CANCLED_BY_USER}
     */
    public static int decrypt(DrmManagerClient client,
            FileDescriptor cipher_fd, FileDescriptor clear_fd) {
        Log.d(TAG, "decrypt() : cipher_fd = " + cipher_fd + ", clear_fd = " + clear_fd);
        if (clear_fd == null || cipher_fd == null) {
            Log.e(TAG, "encrypt bad input parameters");
            return CTA_ERROR_INVALID_INPUT;
        }
        DrmInfoRequest request = new DrmInfoRequest(DrmInfoRequest.TYPE_SET_OMA_DRM_INFO,
                MIME_TYPE_CTA5_MESSAGE);
        request.put(KEY_ACTION, ACTION_CTA5_DECRYPT);
        request.put(KEY_CTA5_CLEAR_FD, clear_fd);
        request.put(KEY_CTA5_CIPHER_FD, cipher_fd);
        DrmInfo info = client.acquireDrmInfo(request);
        return CTA_ERROR_NONE;
    }

    /**
     * Decrypt chipher file to clear file. It's a asynchronize method
     *
     * @param chipher_fd A file descriptor used to be decrypted
     * @param clear_fd A file descriptor used to be stored clear content
     * @param key The key used to decrypt file
     * @return {@link CTA_ERROR_NONE} if encrypt successfully,
     *         otherwise return {@link CTA_ERROR_BADKEY}, {@link CTA_ERROR_NOSPACE},
     *         {@link CTA_ERROR_CANCLED_BY_USER}
     */
    public static int decrypt(DrmManagerClient client,
            FileDescriptor cipher_fd, FileDescriptor clear_fd, byte[] key) {
        Log.d(TAG, "decrypt() : cipher_fd = " + cipher_fd + ", clear_fd = "
            + clear_fd);
        if (clear_fd == null || cipher_fd == null) {
            Log.e(TAG, "encrypt bad input parameters");
            return CTA_ERROR_INVALID_INPUT;
        }
        DrmInfoRequest request = new DrmInfoRequest(DrmInfoRequest.TYPE_SET_OMA_DRM_INFO,
                 MIME_TYPE_CTA5_MESSAGE);
        request.put(KEY_ACTION, ACTION_CTA5_DECRYPT);
        request.put(KEY_CTA5_CLEAR_FD, clear_fd);
        request.put(KEY_CTA5_CIPHER_FD, cipher_fd);
        request.put(KEY_CTA5_KEY, key);
        DrmInfo info = client.acquireDrmInfo(request);
        return CTA_ERROR_NONE;
    }

    /**
     * Change password
     *
     * @param fd which file's key will be changed
     * @param oldKey The old key
     * @param newKey The new key
     * @return {@link CTA_ERROR_NONE} if sucessfully, otherwise return {@link CTA_ERROR_GENRIC}
     */
    public static int changePassword(DrmManagerClient client,
            FileDescriptor fd, byte[] oldKey, byte[] newKey) {
        Log.d(TAG, "changePassword()");
        DrmInfoRequest request = new DrmInfoRequest(DrmInfoRequest.TYPE_SET_OMA_DRM_INFO,
                MIME_TYPE_CTA5_MESSAGE);
        request.put(KEY_ACTION, ACTION_CTA5_CHANGEPASSWORD);
        request.put(KEY_CTA5_FD, fd);
        request.put(KEY_CTA5_OLDKEY, oldKey);
        request.put(KEY_CTA5_NEWKEY, newKey);
        DrmInfo info = client.acquireDrmInfo(request);
        return CTA_ERROR_NONE;
    }

    /**
     * Cancel a encrypt/decrypt process
     * @param fd The CTA file
     * @return {@link CTA_ERROR_NONE} if sucessfully, otherwise return {@link CTA_ERROR_DONE}
     */
    public static int cancel(DrmManagerClient client, FileDescriptor fd) {
        Log.d(TAG, "cancel() fd " + fd.valid() + ",toString " + fd.toString());
        DrmInfoRequest request = new DrmInfoRequest(DrmInfoRequest.TYPE_SET_OMA_DRM_INFO,
            MIME_TYPE_CTA5_MESSAGE);
        request.put(KEY_ACTION, ACTION_CTA5_CANCEL);
        request.put(KEY_CTA5_FD, fd);

        DrmInfo info = client.acquireDrmInfo(request);
        String message = getResultFromDrmInfo(info);
        int result = RESULT_SUCCESS.equals(message) ? CTA_ERROR_NONE : CTA_ERROR_CANCEL;
        return result;
    }

    // get the result from DrmInfo, sucess or fail
    private static String getResultFromDrmInfo(DrmInfo info) {
        // get message from returned DrmInfo
        byte[] data = null;
        if (info != null) {
            data = info.getData();
        }
        String message = "";
        if (null != data) {
            try {
                // the information shall be in format of ASCII string
                message = new String(data, "US-ASCII");
            } catch (UnsupportedEncodingException e) {
                Log.e(TAG, "Unsupported hongen encoding type of the returned DrmInfo data");
                message = "";
            }
        }
        return message;
    }

 private static boolean isDrmFile(byte[] header) {
        if (header == null) {
            return false;
        }
        // Structure: version(1), content type length(1), content uri lenght(1),
        //            content type(<128), content uri(<128)
        int version = header[0];
        if (version != 1) {
            Log.d(TAG, "isDrmFile: version is not dcf version 1, no oma drm file");
            return false;
        }
        int contentTypeLen = header[1];
        int contentUriLen = header[2];
        if (contentTypeLen <=0 || (contentTypeLen + 3) > 128
                || contentUriLen <= 0 || contentUriLen > 128 ) {
            Log.d(TAG, "isDrmFile: content type or uri len invalid, not oma drm file, contentType["
                    + contentTypeLen + "] contentUri[" + contentUriLen + "]");
            return false;
        }
        String contentType = new String(header, 3, contentTypeLen);
        if (contentType == null || !contentType.contains("/")) {
            Log.d(TAG, "isDrmFile: content type not right, not oma drm file");
            return false;
        }
        Log.d(TAG, "this is a oma drm file: " + contentType);
        return true;
    }
}
