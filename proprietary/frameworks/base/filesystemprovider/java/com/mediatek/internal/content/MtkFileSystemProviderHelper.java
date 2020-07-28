/*
 * Copyright (C) 2017 The Android Open Source Project
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

package com.mediatek.internal.content;

import android.content.ContentResolver;
import android.content.Context;
import android.database.Cursor;
import android.database.MatrixCursor.RowBuilder;
import android.drm.DrmManagerClient;
import android.media.MediaFile;
import android.net.Uri;
import android.os.SystemProperties;
import android.provider.DocumentsContract.Document;
import android.provider.MediaStore;
import android.util.Log;
import android.webkit.MimeTypeMap;
import com.mediatek.media.MtkMediaStore;


import java.io.File;
import java.io.FileNotFoundException;

/**
 * A helper class for {@link android.provider.DocumentsProvider} to perform file operations on local
 * files.
 */
public class MtkFileSystemProviderHelper {

    private static final String TAG = "FileSystemProvider";
    private static final Uri BASE_URI =
            new Uri.Builder().scheme(ContentResolver.SCHEME_CONTENT)
            .authority("com.android.externalstorage.documents").build();
    private static final boolean DEBUG = false;
    private static final boolean LOG_INOTIFY = false;

    private String[] mDefaultProjection;

    private Context mContext = null;

    private static final String[] DEFAULT_DOCUMENT_PROJECTION = new String[] {
            Document.COLUMN_DOCUMENT_ID, Document.COLUMN_MIME_TYPE, Document.COLUMN_DISPLAY_NAME,
            Document.COLUMN_LAST_MODIFIED, Document.COLUMN_FLAGS, Document.COLUMN_SIZE,
                /// M: add to support drm
            MediaStore.MediaColumns.DATA,
            MediaStore.MediaColumns.IS_DRM,
            MtkMediaStore.MediaColumns.DRM_METHOD
    };

    private static final String MIMETYPE_JPEG = "image/jpeg";
    private static final String MIMETYPE_JPG = "image/jpg";
    private static final String MIMETYPE_OCTET_STREAM = "application/octet-stream";

    /// M: add to support drm
    /**
     *
     * @return is MtkDrmApp, or not.
     */
    public MtkFileSystemProviderHelper(Context context) {
        mContext = context;
    }

    public static boolean isMtkDrmApp() {
        return SystemProperties.getBoolean("ro.vendor.mtk_oma_drm_support", false);
    }

    public void supportDRM(File file, RowBuilder row, String docId, String mimeType,
                           File visibleFile)
        throws FileNotFoundException {

        String displayName = file.getName();
        //String mimeType = getTypeForFile(file);
        /// M: add to support drm
        if (isMtkDrmApp() && !file.isDirectory()) {
            final int lastDot = displayName.lastIndexOf('.');
            String extension = null;
            if (lastDot >= 0) {
                extension = displayName.substring(lastDot + 1).toLowerCase();
            }
            if (DEBUG) Log.d(TAG, "includeFile extension = " + extension);
            if (extension != null && extension.equalsIgnoreCase("dcf")) {
                // query drm info from file table of media provider
                Uri fileUri = MediaStore.Files.getContentUri("external");
                if (DEBUG) Log.d(TAG, "includeFile fileUri = " + fileUri);
                String whereClause = MediaStore.MediaColumns.DATA + " = ?";
                String[] projection = new String[]
                    {MediaStore.MediaColumns.IS_DRM, MtkMediaStore.MediaColumns.DRM_METHOD,
                        MediaStore.MediaColumns.MIME_TYPE};
                Cursor drmCursor = null;

                try {
                    /// M: get visible path for DRM files (new in N)
                    //file = getFileForDocId(docId, true);
                    file = visibleFile;
                    if (file != null) {
                        if (DEBUG) Log.d(TAG, "includeFile file.getAbsolutePath() = " +
                                         file.getAbsolutePath());
                        drmCursor = mContext.getContentResolver().query(fileUri, projection,
                                whereClause, new String[]{file.getAbsolutePath()}, null);

                        if (drmCursor != null && drmCursor.moveToFirst()) {
                            int isDrm = drmCursor.getInt
                                (drmCursor.getColumnIndex(MediaStore.MediaColumns.IS_DRM));
                            int drmMethod = drmCursor.getInt
                                (drmCursor.getColumnIndex(MtkMediaStore.MediaColumns.DRM_METHOD));
                            mimeType = drmCursor.getString
                                (drmCursor.getColumnIndex(MediaStore.MediaColumns.MIME_TYPE));
                            row.add(MediaStore.MediaColumns.IS_DRM, isDrm);
                            row.add(MtkMediaStore.MediaColumns.DRM_METHOD, drmMethod);
                            if (DEBUG) Log.d(TAG, "includeFile " + file.getAbsolutePath() +
                                             ", isDrm: " + isDrm + ", drmMethod: " + drmMethod);
                        }
                    } else {
                        Log.d(TAG, "VisibleFile is null");
                    }
                } catch (IllegalStateException e) {
                    if (DEBUG) Log.d(TAG, "includeFile " + file.getAbsolutePath() +
                                     "query media occur error.");
                } finally {
                    if (drmCursor != null) {
                        drmCursor.close();
                    }
                }
            }
        }
        row.add(Document.COLUMN_MIME_TYPE, mimeType);
        row.add(MediaStore.MediaColumns.DATA, file.getAbsolutePath());
    }

    public String getTypeForNameMtk(File file, String name) {
        final int lastDot = name.lastIndexOf('.');
        if (lastDot >= 0) {
            final String extension = name.substring(lastDot + 1).toLowerCase();
            if (extension.equalsIgnoreCase("dcf")) {
                return getTypeForDrmFile(file);
            }
            final String mime = MimeTypeMap.getSingleton().getMimeTypeFromExtension(extension);
            if (mime != null) {
                return mime;
            }
        }

        /// M: Try Media mime if upper fails
        final String mime = MediaFile.getMimeTypeForFile(name);
        if (mime != null) {
            return mime;
        }

        return MIMETYPE_OCTET_STREAM;
    }



    private String getTypeForDrmFile(File file) {
        final DrmManagerClient client = new DrmManagerClient(mContext);
        final String rawFile = file.toString();
        if (DEBUG) {
            Log.d(TAG, "getTypeForFile rawFile = " + rawFile);
        }
        if (client.canHandle(rawFile, null)) {
            return client.getOriginalMimeType(rawFile);
        }
        return MIMETYPE_OCTET_STREAM;
    }

    public String[] getDefaultProjection() {
        return DEFAULT_DOCUMENT_PROJECTION;
    }

}
