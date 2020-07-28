// Copyright 2011 Google Inc.
// All Rights Reserved.

package com.android.mms;

import android.content.ContentProvider;
import android.content.ContentValues;
import android.content.Context;
import android.content.UriMatcher;
import android.database.Cursor;
import android.database.MatrixCursor;
import android.net.Uri;
import android.os.ParcelFileDescriptor;
import android.text.TextUtils;
import android.util.Log;
import android.widget.Toast;

import com.android.mms.util.MmsLog;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
/**
 * The TempFileProvider manages a uri, backed by a file, for passing to the camera app for
 * capturing pictures and videos and storing the data in a file in the messaging app.
 */
public class TempFileProvider extends ContentProvider {
    private static String TAG = "TempFileProvider";

    /**
     * The content:// style URL for this table
     */
    public static final Uri SCRAP_CONTENT_URI = Uri.parse("content://mms_temp_file/scrapSpace");
    public static final Uri SCRAP_VIDEO_URI = Uri.parse("content://mms_temp_file/scrapSpaceVideo");

    private static final int MMS_SCRAP_SPACE = 1;
    private static final int MMS_SCRAP_SPACE_VIDEO = 2;
    private static final UriMatcher sURLMatcher = new UriMatcher(UriMatcher.NO_MATCH);
    static {
        sURLMatcher.addURI("mms_temp_file", "scrapSpace", MMS_SCRAP_SPACE);
        sURLMatcher.addURI("mms_temp_file", "scrapSpaceVideo", MMS_SCRAP_SPACE_VIDEO);
    }

    public static final int TEMP_FILENAME_LENGTH = 9;

    private static final String DATA = "_data";

    private static final String[] COLUMNS = new String[] { DATA };

    @Override
    public boolean onCreate() {
        return true;
    }

    @Override
    public Cursor query(Uri uri, String[] projection,
            String selection, String[] selectionArgs, String sortOrder) {
        if (projection != null && projection.length == 1 && projection[0].equals(DATA)) {
            Log.d(TAG, "query begin");
            int match = sURLMatcher.match(uri);
            MatrixCursor cursor = new MatrixCursor(COLUMNS, 1);
            Object[] row = new Object[1];
            switch (match) {
                case MMS_SCRAP_SPACE:
                    row[0] = getScrapPath(getContext(), ".temp.jpg");
                    cursor.addRow(row);
                    break;
                case MMS_SCRAP_SPACE_VIDEO:
                    row[0] = getScrapPath(getContext(), ".temp.3gp");
                    cursor.addRow(row);
                    break;
            }
            return cursor;
        } else {
            return null;
        }

    }

    @Override
    public Uri insert(Uri uri, ContentValues values) {
        return null;
    }

    @Override
    public int delete(Uri uri, String selection, String[] selectionArgs) {
        return 0;
    }

    @Override
    public int update(Uri uri, ContentValues values,
            String selection, String[] selectionArgs) {
        return 0;
    }

    private ParcelFileDescriptor getTempStoreFd(String mode, String name) {
        String fileName = getScrapPath(getContext(), name);
        /// M: Code analyze 002, fix bug unknown, If the filename is null,it
        // should run null @{
        if (fileName == null) {
            return null;
        }
        /// @}
        ParcelFileDescriptor pfd = null;

        try {
            File file = new File(fileName);
            int modeFlags;
            Log.d(TAG, "getTempStoreFd == " + mode);
            if (mode.equals("r")) {
                modeFlags = ParcelFileDescriptor.MODE_READ_ONLY;
            } else {
                modeFlags = ParcelFileDescriptor.MODE_READ_WRITE
                            | ParcelFileDescriptor.MODE_CREATE
                            | ParcelFileDescriptor.MODE_TRUNCATE;
            }
            pfd = ParcelFileDescriptor.open(file, modeFlags);
        } catch (Exception ex) {
            Log.e(TAG, "getTempStoreFd: error creating pfd for " + fileName, ex);
        }

        return pfd;
    }

    @Override
    public String getType(Uri uri) {
        return "*/*";
    }

    @Override
    public ParcelFileDescriptor openFile(Uri uri, String mode) throws FileNotFoundException {
        // if the url is "content://mms/takePictureTempStore", then it means the requester
        // wants a file descriptor to write image data to.
        Log.d(TAG, "openFile begin" + mode);
        ParcelFileDescriptor fd = null;
        int match = sURLMatcher.match(uri);

        if (Log.isLoggable(TAG, Log.VERBOSE)) {
            MmsLog.dpi(TAG, "openFile: uri=" + uri + ", mode=" + mode);
        }

        switch (match) {
            case MMS_SCRAP_SPACE:
                fd = getTempStoreFd(mode, ".temp.jpg");
                break;
            case MMS_SCRAP_SPACE_VIDEO:
                fd = getTempStoreFd(mode, ".temp.3gp");
                break;
        }

        return fd;
    }


    /**
     * This is the scrap file we use to store the media attachment when the user
     * chooses to capture a photo to be attached . We pass {#link@Uri} to the Camera app,
     * which streams the captured image to the uri. Internally we write the media content
     * to this file. It's named '.temp.jpg' so Gallery won't pick it up.
     */
    public static String getScrapPath(Context context, String fileName) {
        String path = "";
        File baseDir = context.getExternalCacheDir();
        if (baseDir == null) {
            Log.e(TAG, "unable to find directory");
            return null;
        }
        if (!baseDir.exists() && !baseDir.mkdirs()) {
            MmsLog.wpi(TAG, "getScrapPath: create failed:" + baseDir.getPath());
            return null;
        }
        path = baseDir.toString() + "/" + fileName;
        MmsLog.dpi(TAG, "getScrapPath: " + path);
        return path;
    }

    public static String getScrapPath(Context context) {
        return getScrapPath(context, ".temp.jpg");
    }

    /**
     * renameScrapFile renames the single scrap file to a new name so newer uses of the scrap
     * file won't overwrite the previously captured data.
     * @param fileExtension file extension for the temp file, typically ".jpg" or ".3gp"
     * @param uniqueIdentifier a separator to add to the file to make it unique,
     *        such as the slide number. This parameter can be empty or null.
     * @return uri of renamed file. If there's an error renaming, null will be returned
     */
    public static Uri renameScrapFile(String fileExtension, String uniqueIdentifier,
            Context context) {
        String filePath = getScrapPath(context);
        // There's only a single scrap file, but there can be several slides. We rename
        // the scrap file to a new scrap file with the slide number as part of the filename.

        // Replace the filename ".temp.jpg" with ".temp#.[jpg | 3gp]" where # is the unique
        // identifier. The content of the file may be a picture or a .3gp video.
        if (uniqueIdentifier == null) {
            uniqueIdentifier = "";
        }
        /// M: Code analyze 004,fix bug unknown,We should check whether the
        // newTempFilePath and filePath is null. @{
        String newTempFilePath = getScrapPath(context, ".temp" + uniqueIdentifier + fileExtension);
        if (newTempFilePath == null || filePath == null) {
            return null;
        }
        File newTempFile = new File(newTempFilePath);
        /// @}
        File oldTempFile = new File(filePath);
        // remove any existing file before rename
        boolean deleted = newTempFile.delete();
        if (!oldTempFile.renameTo(newTempFile)) {
            return null;
        }
        return Uri.fromFile(newTempFile);
    }

    /**
     * Pass in a path to a file and this function will return true if it thinks the path
     * points to one of its scrap files.
     * @param path full path of a file
     * @return true if path is a scrap file path
     */
    public static boolean isTempFile(String path) {
        // An admittedly weak determination of a temp file, but sufficient for current needs.
        // For now, the penalty of returning true for a file that isn't a temp file is simply
        // not storing the file's thumbnail in an on-disk thumbnail cache.
        return path.contains(".temp");
    }

    /// M: Code analyze 003,fix bug ALPS00240011,the video reached size limit
    // can't attached if replace the attachment with video @{
    private void cleanScrapFile(Context context) {
        final String path = getScrapPath(context);
        if (path == null) {
            return;
        }
        final File file = new File(path);
        if (file.exists() && file.length() > 0) {
            if (file.delete()) {
                MmsLog.dpi(TAG, "getScrapPath, delete old file " + path);
            } else {
                MmsLog.dpi(TAG, "getScrapPath, failed to delete old file " + path);
            }
        }
    }
    /// @}

    /// M: Code analyze 005,fix bug ALPS00241707,You can not add capture video
    // to Messaging after you preview it in Gallery. @{
    public static String getScrapVideoPath(Context context) {
        return getScrapPath(context, ".temp.3gp");
    }

    public static Uri getScrapVideoUri(Context context) {
        /// M: Code analyze 001,fix bug ALPS00243850,If user not format the
        // sdcard, there is available space for camera. JE happen when view a
        // MMS with image. @{
        String fileName = getScrapVideoPath(context);
        if (!TextUtils.isEmpty(fileName)) {
            File file = new File(fileName);
            return Uri.fromFile(file);
        }
        return null;
        /// @}
    }

    /**
     * renameScrapFile renames the single scrap file to a new name so newer uses of the scrap
     * file won't overwrite the previously captured video.
     * @param fileExtension file extension for the temp file, ".3gp"
     * @param uniqueIdentifier a separator to add to the file to make it unique,
     *        such as the slide number. This parameter can be empty or null.
     * @return uri of renamed file. If there's an error renaming, null will be returned
     */
    public static Uri renameScrapVideoFile(String fileExtension,
            String uniqueIdentifier, Context context) {
        String filePath = getScrapVideoPath(context);
        // There's only a single scrap file, but there can be several slides. We rename
        // the scrap file to a new scrap file with the slide number as part of the filename.

        // Replace the filename ".temp.3gp" with ".temp#.3gp" where # is the unique
        // identifier. The content of the file is a .3gp video.
        if (uniqueIdentifier == null) {
            uniqueIdentifier = "";
        }
        String newFilePath = getScrapPath(context, ".temp" + uniqueIdentifier + fileExtension);
        if (newFilePath == null || filePath == null) {
            return null;
        }
        File newTempFile = new File(newFilePath);
        File oldTempFile = new File(filePath);
        try {
            TempFileProvider.copyFile(oldTempFile, newTempFile);
        } catch (Exception e) {
            Log.e(TAG, "TempFileProvider copyFile failed");
            Toast.makeText(context, "TempFileProvider copyFile failed", Toast.LENGTH_LONG).show();
        }
        return Uri.fromFile(newTempFile);
    }
    /// @}

    /// M: fix bug ALPS01031577, copy file for avoiding BT share failed
    public static void copyFile(File sourceFile, File targetFile) throws Exception {
        BufferedInputStream inBuff = null;
        BufferedOutputStream outBuff = null;
        try {
            inBuff = new BufferedInputStream(new FileInputStream(sourceFile));
            outBuff = new BufferedOutputStream(new FileOutputStream(targetFile));
            byte[] b = new byte[1024 * 5];
            int len;
            while ((len = inBuff.read(b)) != -1) {
                outBuff.write(b, 0, len);
            }
            outBuff.flush();
        } finally {
            if (inBuff != null)
                inBuff.close();
            if (outBuff != null)
                outBuff.close();
        }
    }

}
