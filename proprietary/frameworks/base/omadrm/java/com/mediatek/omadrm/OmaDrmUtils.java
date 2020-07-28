/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2013. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
package com.mediatek.omadrm;

import android.app.Activity;
import android.app.ActivityThread;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.app.FragmentManager;
import android.app.FragmentTransaction;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.res.Resources;
import android.database.Cursor;
import android.database.sqlite.SQLiteException;
import android.drm.DrmInfo;
import android.drm.DrmInfoRequest;
import android.drm.DrmManagerClient;
import android.drm.DrmRights;
import android.drm.DrmStore;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Rect;
import android.graphics.drawable.Drawable;
import android.media.MediaFile;
import android.net.Uri;
import android.os.Binder;
import android.os.Build;
import android.os.Bundle;
import android.os.SystemProperties;
import android.provider.MediaStore;
import android.text.TextUtils;
import android.util.Log;

import java.io.File;
import java.io.FileDescriptor;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.RandomAccessFile;
import java.io.UnsupportedEncodingException;
import java.text.SimpleDateFormat;
import java.util.Date;

import com.mediatek.dcfdecoder.DcfDecoder;
import com.mediatek.media.MtkMediaStore;
import com.mediatek.omadrm.OmaDrmStore.MetadatasColumns;


public class OmaDrmUtils {
    public static final String TAG = "OmaDrmUtils";
    private static boolean sIsOmaDrmEnabled;
    // Add for control debug log, only default enable it on eng/userdebug load
    private static final boolean DEBUG = Log.isLoggable(TAG, Log.DEBUG) || "eng".equals(Build.TYPE);
    private static final String CONSUME_DIALOG_TAG = "consume_rights_dialog";
    private static final String PROTECTION_INFO_DIALOG_TAG = "protection_info_dialog";
    private static final String SPACES = " ";
    private static final String LINE_FEED = "\n";
    private static final int HEADER_BUFFER_SIZE = 128;
    private static final Uri FILE_URI = MediaStore.Files.getContentUri("external");
    private static DialogFragment sConsumeDialog = null;
    private static DialogFragment sProtectionInfoDialog = null;
    static {
        sIsOmaDrmEnabled = SystemProperties.getBoolean("ro.vendor.mtk_oma_drm_support", false);
    }

    /**
     * Check whether OMA DRM v1.0 is enabled or not.
     *
     * @return true if OMA DRM feature is enabled, otherwise return false
     */
    public static boolean isOmaDrmEnabled() {
        return true;  // sIsOmaDrmEnabled;
    }

    /**
     * Check whether the given file is oma drm file. The given file's extension must be ".dcf",
     *  if not, mark it as non drm file. then get it metadata to check it is a real oma drm.
     *
     * @param client DrmManagerClient
     * @param path Path to the need check content.
     * @return If file's extension is dcf and metadata is a oma drm, return true, otherwise false.
     */
    public static boolean isDrm(DrmManagerClient client, String path) {
        ContentValues metadata = client.getMetadata(path);
        if (metadata == null) {
            return false;
        }
        Integer isDrm = metadata.getAsInteger(OmaDrmStore.MetadatasColumns.IS_DRM);
        if (isDrm != null && isDrm > 0) {
            return true;
        }
        return false;
    }

    /**
     * Check whether the given file is oma drm file. The given file's extension must be ".dcf",
     *  if not, mark it as non drm file. then get it metadata to check it is a real oma drm.
     *
     * @param client DrmManagerClient
     * @param uri URI of the need check content.
     * @return If file's extension is dcf and metadata is a oma drm, return true, otherwise false.
     */
    public static boolean isDrm(DrmManagerClient client, Uri uri) {
        ContentValues metadata = null;
        try {
            metadata = client.getMetadata(uri);
        } catch (IllegalArgumentException e) {
            Log.e(TAG, "isDrm: getMetadata fail with " + uri, e);
            // Query path from uri failed, try to read header to check whether is drm.
            Context context = ActivityThread.currentApplication();
            boolean isDrm = false;
            if (context != null) {
                InputStream inputStream = null;
                try {
                    inputStream = context.getContentResolver().openInputStream(uri);
                    if (inputStream != null) {
                        byte[] header = new byte[HEADER_BUFFER_SIZE];
                        if (HEADER_BUFFER_SIZE == inputStream.read(header)) {
                            isDrm = isDrm(header);
                        }
                    }
                } catch (IOException ioe1) {
                    Log.e(TAG, "isDrm: IOException fail with " + uri, ioe1);
                } finally {
                    if (inputStream != null) {
                        try {
                            inputStream.close();
                        }
                        catch (Exception ioe2) {
                            Log.e(TAG, "isDrm: close input stream fail with " + ioe2);
                        }
                    }
                }
            }
            Log.d(TAG, "isDrm: check from file with result = " + isDrm);
            return isDrm;
        }
        if (metadata == null) {
            return false;
        }
        Integer isDrm = metadata.getAsInteger(OmaDrmStore.MetadatasColumns.IS_DRM);
        if (isDrm != null && isDrm > 0) {
            return true;
        }
        return false;
    }

    /**
     * Check whether the given file is oma drm file, it will check the given file's header, if
     * it match DRM Content Format(DCF) header, it is a oma drm file.
     *
     * @param path File path
     * @return When given file header is match DCF file header(include right version, valid content
     * type length and valid mimetype), return true, otherwise false.
     */
    public static boolean isDrm(String path) {
        Log.d(TAG, "isDrm: check file " + path);
        RandomAccessFile randomAccessFile = null;
        boolean isDrm = false;
        try {
            randomAccessFile = new RandomAccessFile(new File(path), "r");
            randomAccessFile.seek(0);
            byte[] header = new byte[HEADER_BUFFER_SIZE];
            if (HEADER_BUFFER_SIZE == randomAccessFile.read(header)) {
                isDrm = isDrm(header);
            }
        } catch (IOException e) {
            Log.e(TAG, "isOmaDrmFile: read file with IOException ", e);
            return false;
        } finally {
            if (randomAccessFile != null) {
                try {
                    randomAccessFile.close();
                    randomAccessFile = null;
                } catch (IOException e) {
                    Log.e(TAG, "isOmaDrmFile: close randomAccessFile with IOException ", e);
                }
            }
        }
        Log.d(TAG, "isDrm: " + isDrm);
        return isDrm;
    }

    /**
     * M: check the give file header whether is a drm file, include OMA DRM.
     *
     * @param header file header with 128 bytes
     * @return if it is drm file, return true, otherwise false.
     *
     * @hide
     */
    private static boolean isDrm(byte[] header) {
        if (header == null || header.length < HEADER_BUFFER_SIZE) {
            return false;
        }

        // OMA DRM
        // Structure: version(1), content type length(1), content uri lenght(1),
        //            content type(<128), content uri(<128)
        int version = header[0];
        if (version != 1) {
            Log.d(TAG, "isDrmFile: version is not dcf version 1, no oma drm file");
            return false;
        }
        int contentTypeLen = header[1];
        int contentUriLen = header[2];
        if (contentTypeLen <=0 || (contentTypeLen + 3) > HEADER_BUFFER_SIZE
                || contentUriLen <= 0 || contentUriLen > HEADER_BUFFER_SIZE ) {
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

    public static boolean canBeForwarded(DrmManagerClient client, String path) {
        int status = client.checkRightsStatus(path, DrmStore.Action.TRANSFER);
        return status == DrmStore.RightsStatus.RIGHTS_VALID;
    }

    public static boolean canBeForwarded(DrmManagerClient client, Uri uri) {
        int status = client.checkRightsStatus(uri, DrmStore.Action.TRANSFER);
        return status == DrmStore.RightsStatus.RIGHTS_VALID;
    }

    /**
     * Install drm file to device, include message, content and rights files.
     *
     * @param client DrmManagerClient instance
     * @param path Path to the content from which you want to install to device
     * @return If install success, return true, otherwise false
     */
    public static boolean installDrmToDevice(DrmManagerClient client, String path) {
        if (DEBUG) Log.d(TAG, "installDrmToDevice: path = " + path);

        if (TextUtils.isEmpty(path)) {
            Log.e(TAG, "installDrmToDevice : Given path is not valid");
            return false;
        }

        File file = new File(path);
        if (!file.exists()) {
            Log.e(TAG, "installDrmToDevice : Given file is not exist");
            return false;
        }

        // 1. If it's a rights file, call save rights in DrmManagerClient to install to device.
        DrmRights rights = null;
        if (path.endsWith(OmaDrmStore.DrmFileExtension.EXTENSION_RIGHTS_XML)) {
            rights = new DrmRights(file, OmaDrmStore.DrmObjectMimeType.MIME_TYPE_RIGHTS_XML);
        } else if (path.endsWith(OmaDrmStore.DrmFileExtension.EXTENSION_RIGHTS_WBXML)) {
            rights = new DrmRights(file, OmaDrmStore.DrmObjectMimeType.MIME_TYPE_RIGHTS_WBXML);
        }
        if (rights != null) {
            int result = DrmManagerClient.ERROR_UNKNOWN;
            try {
                result = client.saveRights(rights, null, null);
            } catch (IOException e) {
                Log.e(TAG, "installDrmToDevice : save rights with", e);
            }
            Log.d(TAG, "installDrmToDevice : save rights with result " + result);
            return result == DrmManagerClient.ERROR_NONE;
        }
        // 2. If it's a message or content file, call acquireDrmInfo to install to device.
        String dcfPath = generateDcfFilePath(path);
        File dcfFile = new File(dcfPath);
        FileOutputStream dcfStream = null;
        boolean success = false;
        try {
            if (!dcfFile.exists()) {
                dcfFile.createNewFile();
            }
            dcfStream = new FileOutputStream(dcfPath);
            DrmInfoRequest request = new DrmInfoRequest(DrmInfoRequest.TYPE_SET_OMA_DRM_INFO,
                    OmaDrmStore.DrmObjectMimeType.MIME_TYPE_DRM_MESSAGE);
            request.put(OmaDrmInfoRequest.KEY_ACTION,
                    OmaDrmInfoRequest.ACTION_INSTALL_DRM_TO_DEVICE);
            // use dm path, open fd in plugin
            request.put(OmaDrmInfoRequest.KEY_DATA, path);
            // use dcf fd(open "xx.dcf.tmp" as fd)
            request.put(OmaDrmInfoRequest.KEY_FILEDESCRIPTOR, formatFdToString(dcfStream.getFD()));
            Log.e(TAG, "client.acquireDrmInfo OmaDrmUtils 1 ");
            DrmInfo drmInfo = client.acquireDrmInfo(request); // install to device
            String result = getResultFromDrmInfo(drmInfo);
            success = OmaDrmInfoRequest.DrmRequestResult.RESULT_SUCCESS.equals(result);
        } catch (IOException e) {
            Log.e(TAG, "installDrmTodevice with ", e);
            success = false;
        } finally {
            if (dcfStream != null) {
                try {
                    dcfStream.close();
                } catch (IOException e) {
                    Log.e(TAG, "close dcfStream with ", e);
                }
            }
        }

        // After finishing install to device, we need check whether install success and do:
        // SUCCESS -> rename tmp file to dcf file("xx.dcf.tmp" -> "xx.dcf") and delete old dm file
        // FAILED  -> delete create dcf tmp file
        File dmFile = file;
        if (success) {
            // delete old dm file
            dmFile.delete();
            String newPath = dcfPath.substring(0, dcfPath.lastIndexOf("."));
            // rename ".dcf.tmp" to ".dcf"
            if (!dcfFile.renameTo(new File(newPath))) {
                Log.e(TAG, "installDrmTodevice failed due to rename [" + dcfPath
                        + "] -> [" + newPath + "] failed");
                return false;
            }
            dcfPath = dcfFile.getPath();
        } else {
            dcfFile.delete(); // delete install failed dcf file
        }

        if (DEBUG) Log.d(TAG, "installDrmTodevice: [" + path + "] -> [" + dcfPath + "] " + success);
        return success;
    }

    public static int getActionByMimetype(String mimetype) {
        int action = DrmStore.Action.DEFAULT;
        if (TextUtils.isEmpty(mimetype)) {
            action = DrmStore.Action.DEFAULT;
        } else if (mimetype.startsWith(OmaDrmStore.MimePrefix.IMAGE)) {
            action = DrmStore.Action.DISPLAY;
        } else if (mimetype.startsWith(OmaDrmStore.MimePrefix.VIDEO)
            || mimetype.startsWith(OmaDrmStore.MimePrefix.AUDIO)) {
            action = DrmStore.Action.PLAY;
        }

        if (DEBUG) Log.d(TAG, "getActionByMimetype: mimetype=" + mimetype + ", action=" + action);
        return action;
    }

    /**
     * Overlay a background with drm lock icon, due to rights status to overlap right lock icon
     * it will overlap red lock icon for invalid rights and green lock icon for valid rights, if
     * path is null, always overlap red lock icon.
     *
     * @param client DrmManagerClient instance
     * @param res The application resource
     * @param bgBitmap Background bitmap which will be overlap the drm lock icon if need
     * @param path Path of drm protected content, if path is null, always overlap
     *             lock icon with drm_red_lock
     * @return Bitmap New bitmap with overlaied icon
     */
    public static Bitmap overlapLockIcon(DrmManagerClient client, Resources res,
            Bitmap bgBitmap, String path) {
        if (DEBUG) Log.d(TAG, "overlapLockIcon(res): path = " + path);
        // If path is null, overlap red lock icon, otherwise need check it rights status,
        // overlap red lock icon(invalid rights) and green lock icon(invalid rights)
        int rightsStatus = DrmStore.RightsStatus.RIGHTS_INVALID;
        if (path != null) {
            rightsStatus = client.checkRightsStatus(path);
        }
        return overlapLockIcon(client, res, bgBitmap, rightsStatus);
    }

    public static int checkRightsStatusByFd(DrmManagerClient client, FileDescriptor fd) {
        DrmInfoRequest drmInfoRequest = new DrmInfoRequest(DrmInfoRequest.TYPE_SET_OMA_DRM_INFO,
                OmaDrmStore.DrmObjectMimeType.MIME_TYPE_DRM_CONTENT);
        drmInfoRequest.put(OmaDrmInfoRequest.KEY_ACTION,
                OmaDrmInfoRequest.ACTION_CHECK_RIGHTS_STATUS_BY_FD);
        drmInfoRequest.put(OmaDrmInfoRequest.KEY_FILEDESCRIPTOR, formatFdToString(fd));
        Log.e(TAG, "client.acquireDrmInfo OmaDrmUtils 2 ");
        DrmInfo drmInfo = client.acquireDrmInfo(drmInfoRequest);
        int rightsStatus = DrmStore.RightsStatus.RIGHTS_INVALID;
        String result = getResultFromDrmInfo(drmInfo);
        try {
            rightsStatus = Integer.parseInt(result);
        } catch (NumberFormatException e) {
            Log.e(TAG, "checkRightsStatusByFd with " + e);
        }
        if (DEBUG) Log.d(TAG, "checkRightsStatusByFd: rightsStatus = " + result);
        return rightsStatus;
    }

    public static Bitmap getOriginalLockIcon(DrmManagerClient client,
        Resources res,  String path) {
        int rightsStatus = DrmStore.RightsStatus.RIGHTS_INVALID;
        if (path != null) {
            ContentValues metadata = client.getMetadata(path);
            if (DEBUG) Log.d(TAG, "getOriginalLockIcon: metadata = " + metadata);
            // get metadata fail, just return original bgBitmap
            if (metadata == null) {
               return null;
            }
            Integer isDrm = metadata.getAsInteger(OmaDrmStore.MetadatasColumns.IS_DRM);
            // not a drm file, just return original bgBitmap
            if (isDrm == null || isDrm <= 0) {
                return null;
            }

            String mimetype = metadata.getAsString(OmaDrmStore.MetadatasColumns.DRM_MIME_TYPE);
            if (!TextUtils.isEmpty(mimetype)) {
                rightsStatus = client.checkRightsStatus(path, getActionByMimetype(mimetype));
            }
        }

        int lockId = (rightsStatus == DrmStore.RightsStatus.RIGHTS_VALID) ?
                com.mediatek.internal.R.drawable.drm_green_lock :
                com.mediatek.internal.R.drawable.drm_red_lock;

        return BitmapFactory.decodeResource(res, lockId);
    }

    /**
     * Overlay a background with drm lock icon
     *
     * @param client DrmManagerClient instance
     * @param res The application resource
     * @param bgId Background icon resource id
     * @param path Path of drm protected content
     * @return Bitmap New bitmap with overlaid icon
     */
    public static Bitmap overlapLockIcon(DrmManagerClient client, Resources res,
            int bgId, String path) {
        Bitmap bgBitmap = BitmapFactory.decodeResource(res, bgId);
        return overlapLockIcon(client, res, bgBitmap, path);
    }

    /**
     * Overlay a background with drm lock icon, due to rights status to overlap right lock icon
     * it will overlap red lock icon for invalid rights and green lock icon for valid rights, if
     * path is null, always overlap red lock icon.
     *
     * @param client DrmManagerClient instance
     * @param context The application context
     * @param bgId Background icon resource id
     * @param path Path of drm protected content
     *
     * @return Bitmap New bitmap with overlaid icon
     */
    public static Bitmap overlapLockIcon(DrmManagerClient client, Context context,
            int bgId, String path) {
        if (DEBUG) Log.d(TAG, "overlapLockIcon: path " + path);
        Resources res = context.getResources();
        Bitmap bgBitmap = BitmapFactory.decodeResource(res, bgId);
        int rightsStatus = DrmStore.RightsStatus.RIGHTS_INVALID;
        if (path != null) {
            rightsStatus = client.checkRightsStatus(path);
        }
        return overlapLockIcon(client, res, bgBitmap, rightsStatus);
    }

    public static Bitmap overlapLockIcon(DrmManagerClient client, Context context,
            int bgId, Uri uri) {
        if (DEBUG) Log.d(TAG, "overlapLockIcon: uri " + uri);
        Resources res = context.getResources();
        Bitmap bgBitmap = BitmapFactory.decodeResource(res, bgId);
        int rightsStatus = DrmStore.RightsStatus.RIGHTS_INVALID;
        if (uri != null) {
            rightsStatus = client.checkRightsStatus(uri);
        }
        return overlapLockIcon(client, res, bgBitmap, rightsStatus);
    }

    public static Bitmap overlapLockIcon(DrmManagerClient client, Context context,
            int bgId, FileDescriptor fd) {
        if (DEBUG) Log.d(TAG, "overlapLockIcon: fd " + fd);
        Resources res = context.getResources();
        Bitmap bgBitmap = BitmapFactory.decodeResource(res, bgId);
        int rightsStatus = checkRightsStatusByFd(client, fd);
        return overlapLockIcon(client, res, bgBitmap, rightsStatus);
    }

    public static Bitmap overlapLockIcon(DrmManagerClient client, Context context,
            Bitmap bgBitmap, String path) {
        if (DEBUG) Log.d(TAG, "overlapLockIcon: path " + path);
        Resources res = context.getResources();
        int rightsStatus = DrmStore.RightsStatus.RIGHTS_INVALID;
        if (path != null) {
            rightsStatus = client.checkRightsStatus(path);
        }
        return overlapLockIcon(client, res, bgBitmap, rightsStatus);
    }

    public static Bitmap overlapLockIcon(DrmManagerClient client, Context context,
            Bitmap bgBitmap, Uri uri) {
        if (DEBUG) Log.d(TAG, "overlapLockIcon: uri " + uri);
        Resources res = context.getResources();
        int rightsStatus = DrmStore.RightsStatus.RIGHTS_INVALID;
        if (uri != null) {
            rightsStatus = client.checkRightsStatus(uri);
        }
        return overlapLockIcon(client, res, bgBitmap, rightsStatus);
    }

    public static Bitmap overlapLockIcon(DrmManagerClient client, Context context,
            Bitmap bgBitmap, FileDescriptor fd) {
        if (DEBUG) Log.d(TAG, "overlapLockIcon: fd " + fd);
        Resources res = context.getResources();
        int rightsStatus = checkRightsStatusByFd(client, fd);
        return overlapLockIcon(client, res, bgBitmap, rightsStatus);
    }

    private static Bitmap overlapLockIcon(DrmManagerClient client, Resources res,
            Bitmap bgBitmap, int rightsStatus) {

        int lockId = (rightsStatus == DrmStore.RightsStatus.RIGHTS_VALID) ?
                com.mediatek.internal.R.drawable.drm_green_lock :
                com.mediatek.internal.R.drawable.drm_red_lock;
        Drawable front = res.getDrawable(lockId);

        Bitmap overlayBitmap = Bitmap.createBitmap(bgBitmap.getWidth(),
                bgBitmap.getHeight(), bgBitmap.getConfig());
        Canvas overlayCanvas = new Canvas(overlayBitmap);
        // make sure the bitmap is valid otherwise we use an empty one
        if (!bgBitmap.isRecycled()) {
            overlayCanvas.drawBitmap(bgBitmap, 0, 0, null);
        }
        int overlayWidth = front.getIntrinsicWidth();
        int overlayHeight = front.getIntrinsicHeight();
        int left = bgBitmap.getWidth() - overlayWidth;
        int top = bgBitmap.getHeight() - overlayHeight;
        Rect newBounds = new Rect(left, top, left + overlayWidth, top + overlayHeight);
        front.setBounds(newBounds);
        front.draw(overlayCanvas);
        return overlayBitmap;
    }

    public static void showConsumerDialog(Context context, DrmManagerClient client, Uri uri,
            DialogInterface.OnClickListener onClickListener) {
        showConsumerDialog(context, client, convertUriToPath(context, uri), onClickListener);
    }

    public static void showConsumerDialog(final Context context, final DrmManagerClient client,
            final String path, final DialogInterface.OnClickListener onClickListener) {
        if (DEBUG) Log.d(TAG, "showConsumerDialog: path = " + path);

        // When match below cases, callback to calling app to play directly, only when rights is
        // valid need show consume dialog to let user choose whether to play or not. {@
        // 1. listener is null
        if (onClickListener == null) {
            Log.e(TAG, "showConsumerDialog, onClickListener is null.");
            return;
        }

        // 2. OMA DRM is disable
        if (!sIsOmaDrmEnabled) {
            Log.d(TAG, "showConsumerDialog, oma drm disable.");
            onClickListener.onClick(null, DialogInterface.BUTTON_POSITIVE);
            return;
        }

        // 3. Given path is invalid
        if (TextUtils.isEmpty(path)) {
            Log.e(TAG, "showConsumerDialog: Given path is invalid");
            onClickListener.onClick(null, DialogInterface.BUTTON_POSITIVE);
            return;
        }

        // 4. Must use activity context to show consume dialog
        if (!(context instanceof Activity)) {
            Log.e(TAG, "showConsumerDialog : not an acitivty context");
            onClickListener.onClick(null, DialogInterface.BUTTON_NEGATIVE);
            return;
        }

        // 5. this is not a drm file
        ContentValues metadata = client.getMetadata(path);
        if (DEBUG) Log.d(TAG, "showConsumerDialog: metadata = " + metadata);
        if (metadata == null) {
            Log.d(TAG, "showConsumerDialog, get metadata is null, it's not drm file");
            onClickListener.onClick(null, DialogInterface.BUTTON_POSITIVE);
            return;
        }
        Integer isDrm = metadata.getAsInteger(OmaDrmStore.MetadatasColumns.IS_DRM);
        if (isDrm == null || isDrm <= 0) {
            Log.d(TAG, "showConsumerDialog, get metadata is null, it's not drm file");
            onClickListener.onClick(null, DialogInterface.BUTTON_POSITIVE);
            return;
        }

        // 6. rights is invalid
        final String mimetype = metadata.getAsString(MetadatasColumns.DRM_MIME_TYPE);
        int action = getActionByMimetype(mimetype);
        int rightsStatus = client.checkRightsStatus(path, action);
        if (rightsStatus != DrmStore.RightsStatus.RIGHTS_VALID) {
            if (DEBUG) Log.d(TAG, "showConsumerDialog: rights is invalid, play directly");
            onClickListener.onClick(null, DialogInterface.BUTTON_POSITIVE);
            return;
        }
        // @}

        final String cid = metadata.getAsString(MetadatasColumns.DRM_CONTENT_URI);
        final DialogInterface.OnClickListener listener = new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int which) {
                    if (which == DialogInterface.BUTTON_POSITIVE) {
                        if (mimetype.startsWith(OmaDrmStore.MimePrefix.IMAGE)
                                || mimetype.startsWith(OmaDrmStore.MimePrefix.VIDEO)) {
                            markAsConsumeInAppClient(client, cid);
                        }
                    }
                    if (sConsumeDialog != null || dialog == null) {
                    onClickListener.onClick(dialog, which);
                    }
                    sConsumeDialog = null;
                }
            };

        ContentValues constraints = client.getConstraints(path, action);
        if (DEBUG) Log.d(TAG, "showConsumerDialog : constraints = " + constraints);
        // If value is null ,play it directly, drm plug in will show expired dialog. TODO need check
        // whether this case happen or not
        if (constraints == null || constraints.size() == 0) {
            Log.e(TAG, "showConsumerDialog : constraints is null, no rights");
            listener.onClick(null, DialogInterface.BUTTON_POSITIVE);
            return;
        }

        Resources res = context.getResources();
        StringBuilder message = new StringBuilder();
        // Get 5 types constraints and check which type it is: count, interval and time
        Long maxCount = constraints.getAsLong(DrmStore.ConstraintsColumns.MAX_REPEAT_COUNT);
        Long remainCount = constraints.getAsLong(
                DrmStore.ConstraintsColumns.REMAINING_REPEAT_COUNT);
        Long startTime = constraints.getAsLong(DrmStore.ConstraintsColumns.LICENSE_START_TIME);
        Long expireTime = constraints.getAsLong(DrmStore.ConstraintsColumns.LICENSE_EXPIRY_TIME);
        Long availableTime = constraints.getAsLong(
                DrmStore.ConstraintsColumns.LICENSE_AVAILABLE_TIME);

        // Below may never happen, because native plugin will return all constraints if exist one
        if (maxCount == null || startTime == null || availableTime == null) {
            Log.w(TAG, "showConsumerDialog: max count or start time or available time is null");
            listener.onClick(null, DialogInterface.BUTTON_POSITIVE);
            return;
        }
        // When first time use for count and interval(time type couldn't div whether is first use)
        // and left two and less times for count need show consume dialog to user.
        if (maxCount > 0) {
            // 1. Count type(maxCount>0)
            if (remainCount == maxCount) {
                // 1.1 First time use, show dialog
                message.append(res.getString(com.mediatek.internal.R.string.drm_first_time_use))
                        .append(SPACES)
                        .append(res.getString(com.mediatek.internal.R.string.drm_use_now))
                        .append(LINE_FEED)
                        .append(res.getString(com.mediatek.internal.R.string.drm_use_left))
                        .append(SPACES)
                        .append(remainCount);
            } else if (remainCount <= 2) {
                // 1.2 Only left 2 time or less, show dialog
                message.append(res.getString(com.mediatek.internal.R.string.drm_use_left))
                        .append(SPACES)
                        .append(remainCount)
                        .append(LINE_FEED)
                        .append(res.getString(com.mediatek.internal.R.string.drm_use_now));
            } else {
                // 1.3 Other cases, play directly
                listener.onClick(null, DialogInterface.BUTTON_POSITIVE);
                return;
            }
        } else if (availableTime > 0) {
            // 2. Interval type(availableTime>0)
            if (startTime == -1 && expireTime == -1) {
                // 2.1 First time use
                String time = toTimeString(availableTime);
                message.append(res.getString(
                        com.mediatek.internal.R.string.drm_consume_interval, time))
                        .append(SPACES)
                        .append(res.getString(com.mediatek.internal.R.string.drm_use_now));
            } else {
                // 2.2 Other cases, play directly
                listener.onClick(null, DialogInterface.BUTTON_POSITIVE);
                return;
            }
        } else {
            // 3. Other cases, play directly
            listener.onClick(null, DialogInterface.BUTTON_POSITIVE);
            return;
        }

        if (DEBUG) Log.d(TAG, "showConsumerDialog with message: " + message);
        final AlertDialog.Builder builder = new AlertDialog.Builder(context);
        builder.setIcon(android.R.drawable.ic_dialog_info);
        builder.setTitle(com.mediatek.internal.R.string.drm_consume_title);
        builder.setPositiveButton(android.R.string.ok, listener);
        builder.setNegativeButton(android.R.string.cancel, listener);
        builder.setMessage(message);

        DialogFragment dialogFragment = ConsumeDialogFragment.newInstance(message);
        ((ConsumeDialogFragment) dialogFragment).setOnClickListener(listener);

        // Dismiss old dialog
        if (sConsumeDialog != null) {
            sConsumeDialog.dismissAllowingStateLoss();
        }
        sConsumeDialog = dialogFragment;

        FragmentManager fm = ((Activity) context).getFragmentManager();
        FragmentTransaction ft = fm.beginTransaction();
        ft.add(sConsumeDialog, CONSUME_DIALOG_TAG);
        ft.commitAllowingStateLoss();
        if (DEBUG) Log.d(TAG, "showConsumerDialog: begin show dialog fragment");
    }

    public static void clearProtectionInfoDialog() {
        sProtectionInfoDialog = null;
    }
    public static void showProtectionInfoDialog(Context context, DrmManagerClient client,
            Uri uri) {
        showProtectionInfoDialog(context, client, convertUriToPath(context, uri));
    }

    public static void showProtectionInfoDialog(final Context context, DrmManagerClient client,
            String path) {
        if (DEBUG) Log.d(TAG, "showProtectionInfoDialog: path=" + path + ", context=" + context);

        // 1. OMA DRM is disable
        if (!sIsOmaDrmEnabled) {
            Log.d(TAG, "showProtectionInfoDialog, oma drm is disable");
            return;
        }

        // 2. Given path is invalid
        if (TextUtils.isEmpty(path)) {
            Log.e(TAG, "showProtectionInfoDialog: Given path is invalid");
            return;
        }

        // 3. Must use activity context to show consume dialog
        if (!(context instanceof Activity)) {
            Log.e(TAG, "showConsumerDialog : not an Acitivty context");
            return;
        }

        // 4. This is not a drm file
        ContentValues metadata = client.getMetadata(path);
        if (DEBUG) Log.d(TAG, "showProtectionInfoDialog: metadata = " + metadata);
        if (metadata == null) {
            Log.d(TAG, "showProtectionInfoDialog, get metadata is null, it's not drm file");
            return;
        }
        Integer isDrm = metadata.getAsInteger(OmaDrmStore.MetadatasColumns.IS_DRM);
        if (isDrm == null || isDrm <= 0) {
            Log.d(TAG, "showProtectionInfoDialog, get metadata is null, it's not drm file");
            return;
        }

        // Get constraints to show protection info
        final AlertDialog.Builder builder = new AlertDialog.Builder(context);
        StringBuilder message = new StringBuilder();
        Resources res = context.getResources();
        String mimetype = metadata.getAsString(MetadatasColumns.DRM_MIME_TYPE);
        int action = getActionByMimetype(mimetype);
        ContentValues constraints = client.getConstraints(path, action);
        if (DEBUG) Log.d(TAG, "showProtectionInfoDialog : constraints = " + constraints);
        // 1. First show file name in dialog
        String fileName = MediaFile.getFileTitle(path);
        message.append(fileName).append(LINE_FEED);
        // 2. Check whether can be forwarded(Only sd can forward)
        message.append(res.getString(com.mediatek.internal.R.string.drm_protection_status))
                .append(SPACES);
        int method = metadata.getAsInteger(MetadatasColumns.DRM_METHOD);
        if (method == OmaDrmStore.Method.SD) {
            message.append(res.getString(com.mediatek.internal.R.string.drm_can_forward))
                    .append(LINE_FEED);
        } else {
            message.append(res.getString(com.mediatek.internal.R.string.drm_can_not_forward))
                    .append(LINE_FEED);
        }
        // 2. Show real protection info, if no rights show no available drm license and sd need add
        // renew button, other parse constraints to show
        if (constraints == null || constraints.size() == 0) {
            // Rights invalid with cd/sd
            message.append(res.getString(com.mediatek.internal.R.string.drm_no_license));
            final String rightsIssuer = metadata.getAsString(MetadatasColumns.DRM_RIGHTS_ISSUER);
            if (!TextUtils.isEmpty(rightsIssuer)) {
                builder.setPositiveButton(com.mediatek.internal.R.string.drm_protectioninfo_renew,
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int which) {
                                // Renew rights, start browser to download
                                Intent intent = new Intent(Intent.ACTION_VIEW,
                                        Uri.parse(rightsIssuer));
                                context.startActivity(intent);
                            }
                });
            }
        } else {
            // Get 5 types constraints and check which type it is: count, interval and time
            Long maxCount = constraints.getAsLong(DrmStore.ConstraintsColumns.MAX_REPEAT_COUNT);
            Long remainCount = constraints.getAsLong(
                    DrmStore.ConstraintsColumns.REMAINING_REPEAT_COUNT);
            Long startTime = constraints.getAsLong(DrmStore.ConstraintsColumns.LICENSE_START_TIME);
            Long expireTime = constraints.getAsLong(
                    DrmStore.ConstraintsColumns.LICENSE_EXPIRY_TIME);
            Long availableTime = constraints.getAsLong(
                    DrmStore.ConstraintsColumns.LICENSE_AVAILABLE_TIME);

            // Below may never happen, because native plugin will return all constraints
            // if exist one
            if (maxCount == null || startTime == null || availableTime == null) {
                Log.w(TAG, "showConsumerDialog:max count or start time or available time is null");
                return;
            }

            // Count type
            if (remainCount > 0) {
                message.append(res.getString(com.mediatek.internal.R.string.drm_use_left))
                        .append(SPACES).append(remainCount).append(LINE_FEED);
            }
            // Time type or interval has been used
            if (startTime > 0 && expireTime > 0) {
                message.append(res.getString(com.mediatek.internal.R.string.validity_period))
                        .append(SPACES)
                        .append(toDateTimeString(startTime))
                        .append(" - ")
                        .append(toDateTimeString(expireTime));
            // Interval type
            } else if (availableTime > 0) {
                String time = toTimeString(availableTime);
                message.append(res.getString(com.mediatek.internal.R.string.drm_effective_duration))
                        .append(SPACES)
                        .append(time);
            }
        }

        if (DEBUG) Log.d(TAG, "showProtectionIfoDialog : message = " + message);
        builder.setTitle(com.mediatek.internal.R.string.drm_protectioninfo_title);
        builder.setMessage(message);
        builder.setNeutralButton(android.R.string.ok, null);

        DialogFragment dialogFragment = ProtectionDialogFragment.newInstance(builder);

        // Dismiss old dialog
        if (sProtectionInfoDialog != null) {
            sProtectionInfoDialog.dismissAllowingStateLoss();
        }
        sProtectionInfoDialog = dialogFragment;

        FragmentManager fm = ((Activity) context).getFragmentManager();
        FragmentTransaction ft = fm.beginTransaction();
        ft.add(sProtectionInfoDialog, PROTECTION_INFO_DIALOG_TAG);
        ft.commitAllowingStateLoss();
        if (DEBUG) Log.d(TAG, "showProtectionInfoDialog: begin show dialog fragment");
    }

    private static boolean markAsConsumeInAppClient(DrmManagerClient client, String cid) {
        int pid = Binder.getCallingPid();
        if (DEBUG) Log.d(TAG, "markAsConsumeInAppClient : pid = " + pid + ", cid = " + cid);
        DrmInfoRequest request = new DrmInfoRequest(DrmInfoRequest.TYPE_SET_OMA_DRM_INFO,
                OmaDrmStore.DrmObjectMimeType.MIME_TYPE_DRM_MESSAGE);
        request.put(OmaDrmInfoRequest.KEY_ACTION,
                OmaDrmInfoRequest.ACTION_MARK_AS_CONSUME_IN_APP_CLIENT);
        request.put(OmaDrmInfoRequest.KEY_DATA_1, String.valueOf(pid));
        if (cid != null) {
            request.put(OmaDrmInfoRequest.KEY_DATA_2, cid);
        }
        Log.e(TAG, "client.acquireDrmInfo OmaDrmUtils 3 ");
        DrmInfo info = client.acquireDrmInfo(request);
        String message = getResultFromDrmInfo(info);
        return OmaDrmInfoRequest.DrmRequestResult.RESULT_SUCCESS.equals(message) ? true : false;
    }

    /**
     * get dcf file path according to given dm file path.
     *
     * @param filePath given dm file path
     * @return dcf file path after install success
     */
    private static String generateDcfFilePath(String filePath) {
        Log.v(TAG, "generateDcfFilePath : " + filePath);

        int index = filePath.lastIndexOf(".");
        String raw = null;
        String suffix = ".dcf.tmp";
        if (-1 != index) {
            raw = filePath.substring(0, index);
        } else {
            return null;
        }
        return (raw + suffix);
    }

    /**
     * Convert seconds to hour:min:sec string.
     *
     * @param sec The seconds
     * @return String hour:min:sec string.
     */
    private static String toTimeString(Long sec) {
        final Long SEXAGESIMAL = 60L;
        final Long DECIMALISM = 10L;

        Long hour = sec / (SEXAGESIMAL *  SEXAGESIMAL);
        Long min = (sec - hour * SEXAGESIMAL *  SEXAGESIMAL) / SEXAGESIMAL;
        Long second = sec - hour * SEXAGESIMAL *  SEXAGESIMAL - min * SEXAGESIMAL;

        StringBuilder str = new StringBuilder();
        if (hour < DECIMALISM) {
            str.append("0").append(hour.toString());
        } else {
            str.append(hour.toString());
        }

        str.append(":");
        if (min < DECIMALISM) {
            str.append("0").append(min.toString());
        } else {
            str.append(min.toString());
        }

        str.append(":");
        if (second < DECIMALISM) {
            str.append("0").append(second.toString());
        } else {
            str.append(second.toString());
        }

        return str.toString();
    }

    /**
     * Convert seconds to date time string.
     *
     * @param sec The seconds count from 1970-1-1 00:00:00
     * @return String Date time string
     */
    private static String toDateTimeString(Long sec) {
        Date date = new Date(sec.longValue() * 1000L);
        SimpleDateFormat dateFormat;
        Context context = ActivityThread.currentApplication();
        final boolean use24HourFormat = android.text.format.DateFormat.is24HourFormat(context);
        if (use24HourFormat) {
            dateFormat = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        } else {
            dateFormat = new SimpleDateFormat("yyyy-MM-dd hh:mm:ss");
        }
        Log.e(TAG, "toDateTimeString ");
        String str = dateFormat.format(date);
        return str;
    }

    ///for cta
    /**
     * check token is valid
     */
    public static boolean isTokenValid(DrmManagerClient client,
            String filePath, String token) {
        Log.d(TAG, "isTokenValid filePath:" + filePath);
        boolean result = false;
        DrmInfoRequest request = new DrmInfoRequest(DrmInfoRequest.TYPE_GET_OMA_DRM_INFO,
            OmaDrmStore.DrmObjectMimeType.MIME_TYPE_CTA5_MESSAGE);
        request.put(OmaDrmInfoRequest.KEY_ACTION, OmaDrmInfoRequest.ACTION_CTA5_CHECKTOKEN);
        request.put(OmaDrmInfoRequest.KEY_CTA5_FILEPATH, filePath);
        request.put(OmaDrmInfoRequest.KEY_CTA5_TOKEN, token);
        Log.e(TAG, "client.acquireDrmInfo OmaDrmUtils 4 ");
        DrmInfo info = client.acquireDrmInfo(request);
        String message = getResultFromDrmInfo(info);
        result = OmaDrmInfoRequest.DrmRequestResult.RESULT_SUCCESS.equals(message) ?
            true : false;
        return result;
    }

    /**
     * clear token
     */
    public static boolean clearToken(DrmManagerClient client,
            String filePath, String token) {
        Log.d(TAG, "clearToken filePath:" + filePath);
        boolean result = false;
        DrmInfoRequest request = new DrmInfoRequest(DrmInfoRequest.TYPE_GET_OMA_DRM_INFO,
            OmaDrmStore.DrmObjectMimeType.MIME_TYPE_CTA5_MESSAGE);
        request.put(OmaDrmInfoRequest.KEY_ACTION, OmaDrmInfoRequest.ACTION_CTA5_CLEARTOKEN);
        request.put(OmaDrmInfoRequest.KEY_CTA5_FILEPATH, filePath);
        request.put(OmaDrmInfoRequest.KEY_CTA5_TOKEN, token);
        Log.e(TAG, "client.acquireDrmInfo OmaDrmUtils 5");
        DrmInfo info = client.acquireDrmInfo(request);
        String message = getResultFromDrmInfo(info);
        result = OmaDrmInfoRequest.DrmRequestResult.RESULT_SUCCESS.equals(message) ?
            true : false;
        return result;
    }

    /**
     * @param filePath Drm file path.
     * @param consume Whether consume display right or not.
     * @return Data.
     */
    public static byte[] forceDecryptFile(String filePath, boolean consume) {
        DcfDecoder dcfDecoder = new DcfDecoder();
        return dcfDecoder.forceDecryptFile(filePath, consume);
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

    private static String convertUriToPath(Context context, Uri uri) {
        String path = null;
        if (null != uri) {
            String scheme = uri.getScheme();
            if (null == scheme || scheme.equals("") ||
                    scheme.equals(ContentResolver.SCHEME_FILE)) {
                path = uri.getPath();

            } else if (scheme.equals("http")) {
                path = uri.toString();

            } else if (scheme.equals(ContentResolver.SCHEME_CONTENT)) {
                String[] projection = new String[] {MediaStore.MediaColumns.DATA};
                Cursor cursor = null;
                ContentResolver resolver = context.getContentResolver();
                try {
                    cursor = resolver.query(uri, projection, null,
                            null, null);
                    if (null != cursor && cursor.moveToFirst()) {
                        int index = cursor.getColumnIndex(MediaStore.MediaColumns.DATA);
                        // Check whether the provider support '_data' column, if don't we try
                        // to get cid from header and query path with it from media database
                        if (index != -1) {
                            path = cursor.getString(index);
                        } else {
                            if (null != cursor) {
                                cursor.close();
                                cursor = null;
                            }
                            String contentUri = getContentUri(context, uri);
                            if (contentUri != null) {
                                cursor = resolver.query(FILE_URI,
                                        new String[] { MediaStore.MediaColumns.DATA },
                                        MtkMediaStore.MediaColumns.DRM_CONTENT_URI + "=?",
                                        new String[] { contentUri },
                                        null);
                                if (null != cursor && cursor.moveToFirst()) {
                                    path = cursor.getString(0);
                                }
                            }
                        }
                    }
                } catch (SQLiteException e) {
                    path = uri.getPath();
                } finally {
                    if (null != cursor) {
                        cursor.close();
                    }
                }
            } else {
                path = null;
            }
        }
        if (DEBUG) Log.d(TAG, "convertUriToPath: uri = " + uri + " --> path = " + path);
        return path;
    }

    /**
     * Get content uri(cid) from OMA DRM file header, header structure is:
     * [version][content type len][content uri len][content type][content uri]
     */
    private static String getContentUri(Context context, Uri uri) {
        InputStream inputStream = null;
        String contentUri = null;
        try {
            inputStream = context.getContentResolver().openInputStream(uri);
            if (inputStream != null) {
                byte[] header = new byte[3];
                // Read first 3 byte and check first one whether is drm version 1
                if (3 == inputStream.read(header) && header[0] == 1) {
                    int contentTypeLen = header[1];
                    int contentUriLen = header[2];
                    byte[] buffer = new byte[contentTypeLen + contentUriLen];
                    // content uri is cid:xxxx, need get real cid after ':'
                    if (buffer.length == inputStream.read(buffer)) {
                        contentUri = new String(buffer, contentTypeLen, contentUriLen);
                        contentUri = contentUri.substring(contentUri.indexOf(":") + 1);
                    }
                }
            }
        } catch (IOException ioe1) {
            Log.e(TAG, "getContentUri: IOException fail with " + uri, ioe1);
        } finally {
            if (inputStream != null) {
                try {
                    inputStream.close();
                }
                catch (Exception ioe2) {
                    Log.e(TAG, "getContentUri: close input stream fail with " + ioe2);
                }
            }
        }
        if (DEBUG) Log.d(TAG, "getContentUri: uri = " + uri + ", contentUri = " + contentUri);
        return contentUri;
    }

    /**
     * DrmManagerService only support KEY_FILEDESCRIPTOR as format 'FileDescriptor[fd]'
     */
    private static String formatFdToString(FileDescriptor fd) {
        return "FileDescriptor[" + fd.getInt$() + "]";
    }
}
