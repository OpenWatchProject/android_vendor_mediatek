/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
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
package com.mediatek.rcse.plugin.phone;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.res.AssetFileDescriptor;
import android.database.Cursor;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Environment;
import android.provider.MediaStore;
import android.text.TextUtils;
import android.widget.Toast;
import android.Manifest;
import android.content.pm.PackageManager;

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.plugin.message.PluginUtils;
import com.mediatek.rcse.settings.AppSettings;

import com.mediatek.rcs.R;
import com.mediatek.rcse.service.MediatekFactory;
import com.mediatek.rcse.settings.RcsSettings;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;

/**
 * This class defined as a proxy activity for file transfer.
 */
public class RichcallProxyActivity extends Activity {
    /**
     * The Constant TAG.
     */
    private static final String TAG = "RichcallProxyActivity";
    /* package *//**
                  * The Constant IMAGE_SHARING_SELECTION.
                  */
    static final String IMAGE_SHARING_SELECTION =
            "com.mediatek.rcse.plugin.phone.IMAGE_SHARING_SELECTION";
    /* package *//**
                  * The Constant CONTACT.
                  */
    static final String CONTACT = "contact";
    /* package *//**
                  * The Constant CONTACT_DISPLAYNAME.
                  */
    static final String CONTACT_DISPLAYNAME =
            "contactDisplayname";
    /* package *//**
                  * The Constant SESSION_ID.
                  */
    static final String SESSION_ID = "sessionId";
    /* package *//**
                  * The Constant VIDEO_TYPE.
                  */
    static final String VIDEO_TYPE = "videotype";
    /* package *//**
                  * The Constant VIDEO_WIDTH.
                  */
    static final String VIDEO_WIDTH = "videowidth";
    /* package *//**
                  * The Constant VIDEO_HEIGHT.
                  */
    static final String VIDEO_HEIGHT = "videoheight";
    /* package *//**
                  * The Constant MEDIA_TYPE.
                  */
    static final String MEDIA_TYPE = "mediatype";
    /**
     * The Constant GALLERY_TYPE.
     */
    private static final String GALLERY_TYPE = "image/*";
    /**
     * The Constant GALLERY_VIDEO_TYPE.
     */
    private static final String GALLERY_VIDEO_TYPE = "video/*";
    /**
     * The Constant SUPPORT_WIDTH.
     */
    private static final int SUPPORT_WIDTH = 176;
    /**
     * The Constant SUPPORT_HEIGHT.
     */
    private static final int SUPPORT_HEIGHT = 144;
    /**
     * The Constant SUPPORT_FORMAT_H264.
     */
    private static final String SUPPORT_FORMAT_H264 = "avc";
    /**
     * The m camera temp file uri.
     */
    private Uri mCameraTempFileUri = null;
    /**
     * The Constant FORMAT_SUPPORTED.
     */
    private static final boolean FORMAT_SUPPORTED = true;
    /**
     * The Constant FORMAT_NOT_SUPPORTED.
     */
    private static final boolean FORMAT_NOT_SUPPORTED = false;
    /**
     * The is format supported.
     */
    private static boolean sFormatSupported = FORMAT_SUPPORTED;
    /**
     * The Constant REQUEST_CODE_CAMERA.
     */
    public static final int REQUEST_CODE_CAMERA = 10;
    /**
     * The Constant REQUEST_CODE_GALLERY.
     */
    public static final int REQUEST_CODE_GALLERY = 11;
    /**
     * The Constant REQUEST_CODE_VIDEO.
     */
    private static final int REQUEST_CODE_VIDEO = 12;
    /* package *//**
                  * The Constant IMAGE_NAME.
                  */
    static final String IMAGE_NAME = "filename";
    /* package *//**
                  * The Constant IMAGE_SIZE.
                  */
    static final String IMAGE_SIZE = "filesize";
    /* package *//**
                  * The Constant IMAGE_TYPE.
                  */
    static final String IMAGE_TYPE = "filetype";
    /**
     * The Constant THUMBNAIL_TYPE.
     */
    static final String THUMBNAIL_TYPE = "thumbnail";
    /**
     * The Constant SDCARDDIEFILE.
     */
    private static final File SDCARDDIEFILE = Environment
            .getExternalStorageDirectory();
    /**
     * The Constant SLASH.
     */
    private static final String SLASH = "/";
    /**
     * The Constant RCSE_FILE_DIR.
     */
    private static String RCSE_FILE_DIR = SDCARDDIEFILE + SLASH
            + "Joyn";
    /**
     * The Constant RCSE_TEMP_FILE_DIR.
     */
    private static String RCSE_TEMP_FILE_DIR = RCSE_FILE_DIR
            + SLASH + "temp";
    /**
     * The Constant RCSE_TEMP_FILE_NAME_HEADER.
     */
    private static final String RCSE_TEMP_FILE_NAME_HEADER = "tmp_joyn_";
    /**
     * The Constant JPEG_SUFFIX.
     */
    private static final String JPEG_SUFFIX = ".jpg";
    /**
     * The Constant SELECT_TYPE.
     */
    public static final String SELECT_TYPE = "selectionType";
    /**
     * The Constant SELECT_TYPE_GALLERY.
     */
    public static final String SELECT_TYPE_GALLERY = "Gallery";
    /**
     * The Constant SELECT_TYPE_CAMERA.
     */
    public static final String SELECT_TYPE_CAMERA = "Camera";
    /**
     * The Constant SELECT_TYPE_VIDEO.
     */
    public static final String SELECT_TYPE_VIDEO = "Video";
    /**
     * The Constant FILE_SCHEMA.
     */
    private static final String FILE_SCHEMA = "file://";
    /**
     * The Constant CONTENT_SCHEMA.
     */
    private static final String CONTENT_SCHEMA = "content://";
    /**
     * The Constant VCARD_SCHEMA.
     */
    private static final String VCARD_SCHEMA =
            "content://com.android.contacts/contacts/as_vcard";
    /**
     * The Constant ALL_VCARD_SCHEMA.
     */
    private static final String ALL_VCARD_SCHEMA =
            "content://com.android.contacts/contacts/as_multi_vcard/";
    /**
     * The Constant READABLE_RIGHT.
     */
    private static final String READABLE_RIGHT = "r";
    /**
     * The Constant VCARD_SUFFIX.
     */
    private static final String VCARD_SUFFIX = ".vcf";
    /**
     * The Constant VCALENDAR_SCHEMA.
     */
    private static final String VCALENDAR_SCHEMA =
            "content://com.mediatek.calendarimporter/";
    /**
     * The Constant VCALENDAR_DATA_TYPE.
     */
    private static final String VCALENDAR_DATA_TYPE = "text/x-vcalendar";
    /**
     * The Constant VCALENDAR_SUFFIX.
     */
    private static final String VCALENDAR_SUFFIX = ".vcs";

    private static final int PERMISSION_REQUEST_CODE_IPMSG_SHARE_FILE = 910;

    /* (non-Javadoc)
     * @see android.app.Activity#onCreate(android.os.Bundle)
     */
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Logger.v(TAG, "onCreate");
        super.onCreate(savedInstanceState);        
        Intent intent = getIntent();
        mCameraTempFileUri = intent.getParcelableExtra("imageUri");
        Logger.v(TAG, "onCreate mCameraTempFileUri =" + mCameraTempFileUri);
        startFile(intent);
    }

    public static String getSDCardPath(Context c) {
        Logger.v(TAG, "getSDCardPath");
        File sdDir = null;
        String sdStatus = Environment.getExternalStorageState();
        if (TextUtils.isEmpty(sdStatus)) {
            return c.getFilesDir().getAbsolutePath();
        }
        boolean sdCardExist = sdStatus.equals(android.os.Environment.MEDIA_MOUNTED);
        if (sdCardExist) {
            sdDir = Environment.getExternalStorageDirectory();
            Logger.v(TAG, "getSDCardPath sdCardExist path" + sdDir.toString());
            return sdDir.toString();
        }
        String path = c.getFilesDir().getAbsolutePath();
        Logger.v(TAG, "getSDCardPath sdCardExist no exist path" + path);
        return path;
    }
    
    public boolean isNeverGrantedPermission(String permission) {
        return !(this.shouldShowRequestPermissionRationale(permission));
    }
    
    @Override
    public void onRequestPermissionsResult(final int requestCode,
            final String permissions[], final int[] grantResults) {          
        if (grantResults.length <= 0
                || grantResults[0] != PackageManager.PERMISSION_GRANTED) {
            Logger.d(TAG, "onRequestPermissionsResult not granted");
            if(isNeverGrantedPermission(permissions[0])) {
                Toast.makeText(getApplicationContext(), "Permission denied.You can change them in Settings->Apps.", Toast.LENGTH_LONG).show();
            }
            finish();
            return;
        } else {
            Logger.d(TAG, "onRequestPermissionsResult granted");
        }
       if (requestCode == PERMISSION_REQUEST_CODE_IPMSG_SHARE_FILE) {
           startFile(getIntent());
       } 
    }
    
    private void startFile(Intent intent) {        
        String selectionType = intent.getStringExtra(SELECT_TYPE);
        if (SELECT_TYPE_GALLERY.equalsIgnoreCase(selectionType)) {
            startGallery();
        } else if (SELECT_TYPE_CAMERA.equalsIgnoreCase(selectionType)) {
            startCamera();
        } else if (SELECT_TYPE_VIDEO.equalsIgnoreCase(selectionType)) {
            startVideoGallery();
        } else {
            Logger.v(TAG, "intent is of unkown type");
        }
    }
    
    public boolean hasPermission(final String permission) {      
       
        final int permissionState = this.checkSelfPermission(permission);
        Logger.v("ImagesharingPlugin", "hasPermission() : permission = " + permission + " permissionState = " + permissionState);
        return permissionState == PackageManager.PERMISSION_GRANTED;

    }
    
    /* (non-Javadoc)
     * @see android.app.Activity#onActivityResult(int, int, android.content.Intent)
     */
    @Override
    protected void onActivityResult(int requestCode, int resultCode,
            Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        new ImageProcessingAsyncTask(requestCode, resultCode, data)
                .execute();
    }

    /**
     * The Class ImageProcessingAsyncTask.
     */
    private class ImageProcessingAsyncTask extends
            AsyncTask<Void, Void, String> {
        /**
         * The m request code.
         */
        private int mRequestCode = -1;
        /**
         * The m result code.
         */
        private int mResultCode = -1;
        /**
         * The m data.
         */
        private Intent mData = null;
        /**
         * The m duration.
         */
        private long mDuration = 0;
        /**
         * The m encoding.
         */
        private String mEncoding = "h264";
        /**
         * The m width.
         */
        private int mWidth = 0;
        /**
         * The m height.
         */
        private int mHeight = 0;

        /**
         * Instantiates a new image processing async task.
         *
         * @param requestCode the request code
         * @param resultCode the result code
         * @param data the data
         */
        public ImageProcessingAsyncTask(int requestCode,
                int resultCode, Intent data) {
            mRequestCode = requestCode;
            mResultCode = resultCode;
            mData = data;
        }
        /**
         * Check selected image file.
         *
         * @param fileName the file name
         * @param which the which
         * @return the string
         */
        private String checkSelectedImageFile(String fileName,
                int which) {
            File file = new File(fileName);
            long fileSize = file.length();
            long maxFileSize = RcsSettings.getInstance()
                    .getMaxImageSharingSize() * 1024;
            long warningFileSize = RcsSettings.getInstance()
                    .getWarningMaxImageSharingSize() * 1024;
            boolean shouldWarning = false;
            boolean shouldRepick = false;
            if (warningFileSize != 0 && fileSize >= warningFileSize) {
                shouldWarning = true;
            }
            if (maxFileSize != 0 && fileSize >= maxFileSize) {
                shouldRepick = true;
            }
            Logger.d(TAG, "checkSelectedImageFile() maxFileSize: "
                    + maxFileSize + " warningFileSize: "
                    + warningFileSize + " shouldWarning: "
                    + shouldWarning + " shouldRepick: "
                    + shouldRepick);
            if (shouldRepick) {
                sendRepicDialogIntent(which);
                return null;
            } else if (shouldWarning) {
                Context context = MediatekFactory
                        .getApplicationContext();
                if (context != null) {
                    boolean remind = AppSettings.getInstance()
                            .restoreRemindWarningLargeImageFlag();
                    Logger.w(TAG, "checkSelectedImageFile() remind: "
                            + remind);
                    if (remind) {
                        sendWarnDialogIntent(fileName);
                        return null;
                    } else {
                        return fileName;
                    }
                } else {
                    return fileName;
                }
            } else {
                return fileName;
            }
        }
        /**
         * Gets the file name from file schema.
         *
         * @param uri the uri
         * @return the file name from file schema
         */
        private String getFileNameFromFileSchema(String uri) {
            return uri.substring(FILE_SCHEMA.length(), uri.length());
        }
        /**
         * Gets the file name from content schema.
         *
         * @param uri the uri
         * @param entryType the entry_type
         * @return the file name from content schema
         */
        private String getFileNameFromContentSchema(Uri uri,
                int entryType) {
            Cursor cursor = null;
            if (entryType == REQUEST_CODE_GALLERY) {
                cursor = RichcallProxyActivity.this
                        .getContentResolver()
                        .query(uri,
                                new String[] { MediaStore.Images.ImageColumns.DATA },
                                null, null, null);
            } else if (entryType == REQUEST_CODE_VIDEO) {
                cursor = RichcallProxyActivity.this
                        .getContentResolver()
                        .query(uri,
                                new String[] {
                                        MediaStore.Video.VideoColumns.DATA,
                                        MediaStore.Video.VideoColumns.DURATION },
                                null, null, null);
            }
            String fileName = null;
            if (cursor != null) {
                cursor.moveToFirst();
                fileName = cursor
                        .getString(cursor
                                .getColumnIndex(MediaStore.Images.ImageColumns.DATA));
                cursor.close();
            }
            return fileName;
        }
        /**
         * Gets the file name from vcard.
         *
         * @param uri the uri
         * @return the file name from vcard
         */
        private String getFileNameFromVcard(Uri uri) {
            String fileName = RCSE_TEMP_FILE_DIR
                    + System.currentTimeMillis() + VCARD_SUFFIX;
            try {
                AssetFileDescriptor fd = RichcallProxyActivity.this
                        .getContentResolver()
                        .openAssetFileDescriptor(uri, READABLE_RIGHT);
                FileInputStream fis = fd.createInputStream();
                byte[] data = new byte[fis.available()];
                fis.read(data);
                fis.close();
                File dir = new File(RCSE_TEMP_FILE_DIR);
                if (!dir.exists()) {
                    if (dir.mkdir()) {
                        Logger.e(TAG,
                                "getFileNameFromVcard()-create dir failed");
                        File file = new File(fileName);
                        file.setWritable(true);
                        file.setReadable(true);
                        FileOutputStream fos = new FileOutputStream(
                                file);
                        fos.write(data);
                        fos.close();
                    }
                }
            } catch (FileNotFoundException fileNotFoundException) {
                Logger.e(TAG,
                        "getFileNameFromVcard()-fileNotFoundException");
                fileNotFoundException.printStackTrace();
                fileName = null;
            } catch (IOException iOException) {
                Logger.e(TAG,
                        "getFileNameFromVcard()-iOException while accessing the stream");
                iOException.printStackTrace();
                fileName = null;
            } finally {
                return fileName;
            }
        }
        /**
         * Gets the file name from vcalendar.
         *
         * @param uri the uri
         * @return the file name from vcalendar
         */
        private String getFileNameFromVcalendar(Uri uri) {
            String fileName = RCSE_TEMP_FILE_DIR
                    + System.currentTimeMillis() + VCALENDAR_SUFFIX;
            try {
                AssetFileDescriptor fd = RichcallProxyActivity.this
                        .getContentResolver()
                        .openAssetFileDescriptor(uri, READABLE_RIGHT);
                FileInputStream fis = fd.createInputStream();
                byte[] data = new byte[fis.available()];
                fis.read(data);
                fis.close();
                File dir = new File(RCSE_TEMP_FILE_DIR);
                if (!dir.exists()) {
                    if (dir.mkdir()) {
                        File file = new File(fileName);
                        file.setWritable(true);
                        file.setReadable(true);
                        FileOutputStream fos = new FileOutputStream(
                                file);
                        fos.write(data);
                        fos.close();
                    }
                }
            } catch (FileNotFoundException fileNotFoundException) {
                Logger.e(TAG,
                        "getFileNameFromVcalendar()-fileNotFoundException");
                fileNotFoundException.printStackTrace();
                fileName = null;
            } catch (IOException iOException) {
                Logger.e(TAG,
                        "getFileNameFromVcalendar()-iOException while accessing the stream");
                iOException.printStackTrace();
                fileName = null;
            } finally {
                return fileName;
            }
        }
        /**
         * Gets the file full path from uri.
         *
         * @param uri the uri
         * @param entryType the entry_type
         * @return the file full path from uri
         */
        private String getFileFullPathFromUri(Uri uri, int entryType) {
            String fileName = null;
            if (uri != null) {
                String uriString = Uri.decode(uri.toString());
                Logger.d(TAG, "getFileFullPathFromUri()-The uri is:["
                        + uriString + "]");
                if (uriString != null
                        && uriString.startsWith(FILE_SCHEMA)) {
                    fileName = getFileNameFromFileSchema(uriString);
                } else if (uriString != null
                        && uriString.startsWith(CONTENT_SCHEMA)) {
                    fileName = getFileNameFromContentSchema(uri,
                            entryType);
                } else if (uriString != null
                        && (uriString.startsWith(VCARD_SCHEMA) || uriString
                                .startsWith(ALL_VCARD_SCHEMA))) {
                    fileName = getFileNameFromVcard(uri);
                } else if (uriString != null
                        && uriString.startsWith(VCALENDAR_SCHEMA)) {
                    fileName = getFileNameFromVcalendar(uri);
                } else {
                    Logger.e(
                            TAG,
                            "getFileFullPathFromUri()-uriString = "
                                    + uriString
                                    + ",is not start with file:// and content://");
                }
            }
            return fileName;
        }
        /**
         * Check selected video file.
         *
         * @param fileFullName the file full name
         * @return the string
         */
        private String checkSelectedVideoFile(String fileFullName) {
            return null;
        }
        /* (non-Javadoc)
         * @see android.os.AsyncTask#doInBackground(Params[])
         */
        @Override
        protected String doInBackground(Void... params) {
            String fileFullName = null;
            Uri uri = null;
            if (mResultCode == RESULT_CANCELED) {
                Logger.d(TAG,
                        "ImageProcessingAsyncTask mResultCode is RESULT_CANCELED");
                return null;
            }
            switch (mRequestCode) {
            case REQUEST_CODE_CAMERA: {
                fileFullName = getFileFullPathFromUri(
                        mCameraTempFileUri, REQUEST_CODE_CAMERA);
                if (fileFullName != null) {
                    fileFullName = checkSelectedImageFile(
                            fileFullName, REQUEST_CODE_CAMERA);
                } else {
                    return null;
                }
                break;
            }
            case REQUEST_CODE_GALLERY: {
                if (mData != null) {
                    // Get image filename
                    uri = mData.getData();
                    if (uri != null) {
                        fileFullName = getFileFullPathFromUri(uri,
                                REQUEST_CODE_GALLERY);
                        if (fileFullName != null) {
                            fileFullName = checkSelectedImageFile(
                                    fileFullName,
                                    REQUEST_CODE_GALLERY);
                        } else {
                            return null;
                        }
                    } else {
                        return null;
                    }
                } else {
                    Logger.w(TAG,
                            "mData is null,that is onActivityResult() get a null intent");
                }
                break;
            }
            case REQUEST_CODE_VIDEO: {
                if (mData != null) {
                    // Get video filename
                    uri = mData.getData();
                    if (uri != null) {
                        fileFullName = getFileFullPathFromUri(uri,
                                REQUEST_CODE_VIDEO);
                        if (fileFullName != null) {
                            fileFullName = checkSelectedVideoFile(fileFullName);
                        } else {
                            return null;
                        }
                    } else {
                        return null;
                    }
                } else {
                    Logger.w(TAG,
                            "mData is null,that is onActivityResult() get a null intent");
                }
                break;
            }
            default: {
                Logger.w(TAG, "unkown result");
                break;
            }
            }
            return fileFullName;
        }
        /* (non-Javadoc)
         * @see android.os.AsyncTask#onPostExecute(java.lang.Object)
         */
        @Override
        protected void onPostExecute(String fileName) {
            Logger.v(TAG,
                    "ImageProcessingAsyncTask onPostExecute() entry, with fileName = "
                            + fileName);
            if (mResultCode != RESULT_CANCELED) {
                switch (mRequestCode) {
                case REQUEST_CODE_VIDEO:
                    if (fileName == null) {
                        if (sFormatSupported) {
                            Toast.makeText(
                                    getApplicationContext(),
                                    R.string.video_sharing_not_support,
                                    Toast.LENGTH_SHORT).show();
                        } else {
                            sFormatSupported = FORMAT_SUPPORTED;
                            Toast.makeText(
                                    getApplicationContext(),
                                    R.string.video_file_format_not_support,
                                    Toast.LENGTH_SHORT).show();
                        }
                    }
                    break;
                case REQUEST_CODE_GALLERY:
                case REQUEST_CODE_CAMERA:
                    if (fileName != null) {
                        startImageFile(fileName);
                    }
                    break;
                default:
                    break;
                }
            } else {
                Logger.d(TAG, "onPostExecute, user canceled!");
            }
            finish();
        }
    }

    /**
     * Creates the directory.
     *
     * @param path the path
     * @return true, if successful
     */
    private boolean createDirectory(String path) {
        File dir = new File(path);
        if (!dir.exists()) {
            if (!dir.mkdirs()) {
                return false;
            }
        }
        return true;
    }
    /**
     * Start camera.
     */
    private void startCamera() {
        Logger.d(TAG, "startCamera entryRCSE_FILE_DIR " + RCSE_FILE_DIR + "RCSE_TEMP_FILE_DIR" + RCSE_TEMP_FILE_DIR);
        Intent intent = new Intent(MediaStore.ACTION_IMAGE_CAPTURE);
        intent.putExtra(MediaStore.EXTRA_OUTPUT, mCameraTempFileUri);
        this.startActivityForResult(intent, REQUEST_CODE_CAMERA);
        Logger.d(TAG, "startCamera exit");
    }
    /**
     * Start gallery.
     */
    private void startGallery() {
        Logger.d(TAG, "startGallery entry");
        Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
        intent.setType(GALLERY_TYPE);
        startActivityForResult(intent, REQUEST_CODE_GALLERY);
        Logger.d(TAG, "startGallery exit");
    }
    /**
     * Start video gallery.
     */
    private void startVideoGallery() {
        Logger.d(TAG, "startVideoGallery entry");
        Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
        intent.setType(GALLERY_VIDEO_TYPE);
        startActivityForResult(intent, REQUEST_CODE_VIDEO);
        Logger.d(TAG, "startVideoGallery exit");
    }
    public static boolean checkIfDRMFile(String fileName) {
        Logger.d(TAG, "checkIfDRMFile() entry with fileName = " + fileName);
        int pos = fileName.lastIndexOf(".");
        Logger.d(TAG, "getMimeType(): pos = " + pos);
        if (pos < 0 || pos > fileName.length()) {
            return false;
        }
        final String fileNameExtension = fileName.substring(pos); 
        if(fileNameExtension.equals(".dcf"))
            return true;
        return false;
    }
    /**
     * Start image file.
     *
     * @param fileName the file name
     */
    private void startImageFile(String fileName) {
        if (fileName == null) {
            finish();
            Logger.d(TAG, "onPostExecute fileName is null");
            return;
        }
        if (checkIfDRMFile(fileName)) {            
            Logger.d(TAG, "onPostExecute fileName drm");
            Toast.makeText(
                    getApplicationContext(),
                    PluginUtils
                            .getStringInRcse(R.string.file_formats_not_support),
                    Toast.LENGTH_LONG).show();
            finish();
            return;
        }
        Intent intent = new Intent(
                ImageSharingPlugin.IMAGE_SHARING_START_ACTION);
        intent.putExtra(ImageSharingPlugin.IMAGE_NAME, fileName);
        Context context = MediatekFactory.getApplicationContext();
        if (context == null) {
            Logger.v(
                    TAG,
                    "MediatekFactory.getApplicationContext() return null," +
                    " so call getApplicationContext instead");
            context = getApplicationContext();
        }
        context.sendBroadcast(intent, "com.mediatek.rcse.permission.RCS_ACCESS_MESSAGE");
    }
    /**
     * Send repic dialog intent.
     *
     * @param selectType the select_type
     */
    private void sendRepicDialogIntent(int selectType) {
        Intent intent = new Intent(
                ImageSharingPlugin.IMAGE_SHARING_REPIC_ACTION);
        intent.putExtra(ImageSharingPlugin.SELECT_TYPE, selectType);
        Context context = MediatekFactory.getApplicationContext();
        if (context == null) {
            Logger.v(
                    TAG,
                    "MediatekFactory.getApplicationContext() return null" +
                    ", so call getApplicationContext instead");
            context = getApplicationContext();
        }
        context.sendBroadcast(intent);
    }
    /**
     * Send warn dialog intent.
     *
     * @param fileName the file name
     */
    private void sendWarnDialogIntent(String fileName) {
        if (fileName == null) {
            finish();
            Logger.d(TAG, "sendWarnDialogIntent fileName is null");
            return;
        }
        Intent intent = new Intent(
                ImageSharingPlugin.IMAGE_SHARING_WARN_ACTION);
        intent.putExtra(ImageSharingPlugin.IMAGE_NAME, fileName);
        Context context = MediatekFactory.getApplicationContext();
        if (context == null) {
            Logger.v(
                    TAG,
                    "MediatekFactory.getApplicationContext() return null," +
                    " so call getApplicationContext instead");
            context = getApplicationContext();
        }
        context.sendBroadcast(intent, "com.mediatek.rcse.permission.RCS_ACCESS_MESSAGE");
    }
}
