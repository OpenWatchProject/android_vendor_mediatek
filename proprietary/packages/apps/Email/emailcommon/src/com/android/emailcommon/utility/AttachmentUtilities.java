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

package com.android.emailcommon.utility;

import android.app.DownloadManager;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.res.AssetFileDescriptor;
import android.database.Cursor;
import android.media.MediaScannerConnection;
import android.net.Uri;
import android.os.Binder;
import android.os.Bundle;
import android.os.Environment;
import android.os.ParcelFileDescriptor;
import android.text.TextUtils;
import android.webkit.MimeTypeMap;

import com.android.emailcommon.Logging;
import com.android.emailcommon.provider.EmailContent;
import com.android.emailcommon.provider.EmailContent.Body;
import com.android.emailcommon.provider.EmailContent.BodyColumns;
import com.android.emailcommon.provider.Mailbox;
import com.android.emailcommon.provider.EmailContent.Attachment;
import com.android.emailcommon.provider.EmailContent.AttachmentColumns;
import com.android.emailcommon.provider.EmailContent.Message;
import com.android.emailcommon.provider.EmailContent.MessageColumns;
import com.android.mail.providers.UIProvider;
import com.android.mail.utils.LogUtils;

import android.drm.DrmManagerClient;
import com.mediatek.storage.StorageManagerEx;
import android.content.ContentValues;

import org.apache.commons.io.IOUtils;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.HashMap;
import java.util.List;
import java.util.regex.Pattern;

public class AttachmentUtilities {
    /// M: Log tag
    public static final String TAG = LogUtils.TAG;

    public static final String FORMAT_RAW = "RAW";
    public static final String FORMAT_THUMBNAIL = "THUMBNAIL";
    public static final String GENERAL_MIME_TYPE = "application/octet-stream";
    ///M: For attachment in eml file.
    private static final String EML_ATTACHMENT_PROVIDER = "com.android.email.provider.eml.attachment";

    public static class Columns {
        public static final String _ID = "_id";
        public static final String DATA = "_data";
        public static final String DISPLAY_NAME = "_display_name";
        public static final String SIZE = "_size";
    }

    /// M: synchronize lock for forward attachment @{
    public static final Object SYNCHRONIZE_LOCK_FOR_FORWARD_ATTACHMENT = new Object();
    /// @}

    /// M: add id column to identify the attachment
    private static final String[] ATTACHMENT_CACHED_FILE_PROJECTION = new String[] {
            AttachmentColumns.CACHED_FILE, AttachmentColumns._ID};

    /// M: The mime type of file with .dcf extension (DRM sd file)
    public static final String MIME_DCF = "application/dcf";
    /// M:  The mime type for adding to downloads database system
    public static final String MIME_TYPE_DRM_CONTENT = "application/vnd.oma.drm.content";

    /// M: Result of caching external attachment @{
    public static final int CACHE_FAIL = -1;
    public static final int CACHE_NO_NEED = 0;
    public static final int CACHE_SUCCESS = 1;
    /// @}

    /**
     * M: Used by exchange to pass the account id to email process.And the email
     * process would use this id to delete attachments associated the message
     * which would be deleted from database.
     *
     * @{
     */
    public static final String KEY_ACCOUNT_ID = "account_id";
    /**  @}  */

    /** M: drm single instance*/
    private static DrmManagerClient sDrmClientInstance = null;

    /**
     * M: get Drm client single instance.
     * Even it is a single instance, need release it when not use it any more.
     */
    private static synchronized DrmManagerClient getDrmClientInstance(
            Context context) {
        if (sDrmClientInstance == null) {
            LogUtils.w(LogUtils.TAG, "getDrmClientInstance, create new instance");
            sDrmClientInstance = new DrmManagerClient(context);
        }
        return sDrmClientInstance;
    }

    /**
     * The MIME type(s) of attachments we're willing to send via attachments.
     *
     * Any attachments may be added via Intents with Intent.ACTION_SEND or ACTION_SEND_MULTIPLE.
     */
    public static final String[] ACCEPTABLE_ATTACHMENT_SEND_INTENT_TYPES = new String[] {
        "*/*",
    };
    /**
     * The MIME type(s) of attachments we're willing to send from the internal UI.
     *
     * NOTE:  At the moment it is not possible to open a chooser with a list of filter types, so
     * the chooser is only opened with the first item in the list.
     */
    public static final String[] ACCEPTABLE_ATTACHMENT_SEND_UI_TYPES = new String[] {
        "image/*",
        "video/*",
    };
    /**
     * The MIME type(s) of attachments we're willing to view.
     */
    public static final String[] ACCEPTABLE_ATTACHMENT_VIEW_TYPES = new String[] {
        "*/*",
    };
    /**
     * The MIME type(s) of attachments we're not willing to view.
     */
    public static final String[] UNACCEPTABLE_ATTACHMENT_VIEW_TYPES = new String[] {
    };
    /**
     * The MIME type(s) of attachments we're willing to download to SD.
     */
    public static final String[] ACCEPTABLE_ATTACHMENT_DOWNLOAD_TYPES = new String[] {
        "*/*",
    };
    /**
     * The MIME type(s) of attachments we're not willing to download to SD.
     */
    public static final String[] UNACCEPTABLE_ATTACHMENT_DOWNLOAD_TYPES = new String[] {
    };
    /**
     * Filename extensions of attachments we're never willing to download (potential malware).
     * Entries in this list are compared to the end of the lower-cased filename, so they must
     * be lower case, and should not include a "."
     */
    public static final String[] UNACCEPTABLE_ATTACHMENT_EXTENSIONS = new String[] {
        // File types that contain malware
        "ade", "adp", "bat", "chm", "cmd", "com", "cpl", "dll", "exe",
        "hta", "ins", "isp", "jse", "lib", "mde", "msc", "msp",
        "mst", "pif", "scr", "sct", "shb", "sys", "vb", "vbe",
        "vbs", "vxd", "wsc", "wsf", "wsh",
        // File types of common compression/container formats (again, to avoid malware)
        "zip", "gz", "z", "tar", "tgz", "bz2",
    };
    /**
     * Filename extensions of attachments that can be installed.
     * Entries in this list are compared to the end of the lower-cased filename, so they must
     * be lower case, and should not include a "."
     */
    public static final String[] INSTALLABLE_ATTACHMENT_EXTENSIONS = new String[] {
        "apk",
    };
    /**
     * The maximum size of an attachment we're willing to download (either View or Save)
     * Attachments that are base64 encoded (most) will be about 1.375x their actual size
     * so we should probably factor that in. A 5MB attachment will generally be around
     * 6.8MB downloaded but only 5MB saved.
     */
    public static final int MAX_ATTACHMENT_DOWNLOAD_SIZE = (5 * 1024 * 1024);
    /**
     * The maximum size of an attachment we're willing to upload (measured as stored on disk).
     * Attachments that are base64 encoded (most) will be about 1.375x their actual size
     * so we should probably factor that in. A 5MB attachment will generally be around
     * 6.8MB uploaded.
     */
    public static final int MAX_ATTACHMENT_UPLOAD_SIZE = (5 * 1024 * 1024);

    private static Uri sUri;
    public static Uri getAttachmentUri(long accountId, long id) {
        if (sUri == null) {
            sUri = Uri.parse(Attachment.ATTACHMENT_PROVIDER_URI_PREFIX);
        }
        return sUri.buildUpon()
                .appendPath(Long.toString(accountId))
                .appendPath(Long.toString(id))
                .appendPath(FORMAT_RAW)
                .build();
    }

    // exposed for testing
    public static Uri getAttachmentThumbnailUri(long accountId, long id, long width, long height) {
        if (sUri == null) {
            sUri = Uri.parse(Attachment.ATTACHMENT_PROVIDER_URI_PREFIX);
        }
        return sUri.buildUpon()
                .appendPath(Long.toString(accountId))
                .appendPath(Long.toString(id))
                .appendPath(FORMAT_THUMBNAIL)
                .appendPath(Long.toString(width))
                .appendPath(Long.toString(height))
                .build();
    }

    /**
     * M: Return the filename for a given attachment.  This should be used when delete cached
     * attachments which are exported from external.
     * This does not create or write the file, or even the directories.  It simply builds
     * the filename that should be used.
     * return NULL with invalid attachment
     */
    public static File getAttachmentFilename(Context context, long accountId, String attachmentUri) {
        if (TextUtils.isEmpty(attachmentUri)) {
            return null;
        }
        List<String> segments = Uri.parse(attachmentUri).getPathSegments();
        String fileId = segments.get(1);
        return getAttachmentFilename(context, accountId, Long.valueOf(fileId));
    }

    /**
     * Return the filename for a given attachment.  This should be used by any code that is
     * going to *write* attachments.
     *
     * This does not create or write the file, or even the directories.  It simply builds
     * the filename that should be used.
     */
    public static File getAttachmentFilename(Context context, long accountId, long attachmentId) {
        return new File(getAttachmentDirectory(context, accountId), Long.toString(attachmentId));
    }

    /**
     * Return the directory for a given attachment.  This should be used by any code that is
     * going to *write* attachments.
     *
     * This does not create or write the directory.  It simply builds the pathname that should be
     * used.
     */
    public static File getAttachmentDirectory(Context context, long accountId) {
        return context.getDatabasePath(accountId + ".db_att");
    }

    /**
     * Helper to convert unknown or unmapped attachments to something useful based on filename
     * extensions. The mime type is inferred based upon the table below. It's not perfect, but
     * it helps.
     *
     * <pre>
     *                   |---------------------------------------------------------|
     *                   |                  E X T E N S I O N                      |
     *                   |---------------------------------------------------------|
     *                   | .eml        | known(.png) | unknown(.abc) | none        |
     * | M |-----------------------------------------------------------------------|
     * | I | none        | msg/rfc822  | image/png   | app/abc       | app/oct-str |
     * | M |-------------| (always     |             |               |             |
     * | E | app/oct-str |  overrides  |             |               |             |
     * | T |-------------|             |             |-----------------------------|
     * | Y | text/plain  |             |             | text/plain                  |
     * | P |-------------|             |-------------------------------------------|
     * | E | any/type    |             | any/type                                  |
     * |---|-----------------------------------------------------------------------|
     * </pre>
     *
     * NOTE: Since mime types on Android are case-*sensitive*, return values are always in
     * lower case.
     *
     * @param fileName The given filename
     * @param mimeType The given mime type
     * @return A likely mime type for the attachment
     */
    public static String inferMimeType(final String fileName, final String mimeType) {
        String resultType = null;
        ///M: move getFilenameExtension to UnifiedEmail
        String fileExtension = Utilities.getFilenameExtension(fileName);
        boolean isTextPlain = "text/plain".equalsIgnoreCase(mimeType);

        if ("eml".equals(fileExtension)) {
            resultType = "message/rfc822";
        } else {
            boolean isGenericType =
                    isTextPlain || "application/octet-stream".equalsIgnoreCase(mimeType);
            // If the given mime type is non-empty and non-generic, return it
            if (isGenericType || TextUtils.isEmpty(mimeType)) {
                if (!TextUtils.isEmpty(fileExtension)) {
                    // Otherwise, try to find a mime type based upon the file extension
                    resultType = MimeTypeMap.getSingleton().getMimeTypeFromExtension(fileExtension);
                    if (TextUtils.isEmpty(resultType)) {
                        // Finally, if original mimetype is text/plain, use it; otherwise synthesize
                        resultType = isTextPlain ? mimeType : "application/" + fileExtension;
                    }
                }
            } else {
                resultType = mimeType;
            }
        }

        // No good guess could be made; use an appropriate generic type
        if (TextUtils.isEmpty(resultType)) {
            resultType = isTextPlain ? "text/plain" : "application/octet-stream";
        }
        return resultType.toLowerCase();
    }

    /**
     * M: Not suitable for MTK solution
     * Extract and return filename's extension, converted to lower case, and not including the "."
     *
     * @return extension, or null if not found (or null/empty filename)
     */
    /*public static String getFilenameExtension(String fileName) {
        String extension = null;
        if (!TextUtils.isEmpty(fileName)) {
            int lastDot = fileName.lastIndexOf('.');
            if ((lastDot > 0) && (lastDot < fileName.length() - 1)) {
                extension = fileName.substring(lastDot + 1).toLowerCase();
            }
        }
        return extension;
    }*/

    /**
     * Resolve attachment id to content URI.  Returns the resolved content URI (from the attachment
     * DB) or, if not found, simply returns the incoming value.
     *
     * @param attachmentUri
     * @return resolved content URI
     *
     * TODO:  Throws an SQLite exception on a missing DB file (e.g. unknown URI) instead of just
     * returning the incoming uri, as it should.
     */
    public static Uri resolveAttachmentIdToContentUri(ContentResolver resolver, Uri attachmentUri) {
        Cursor c = resolver.query(attachmentUri,
                new String[] { Columns.DATA },
                null, null, null);
        if (c != null) {
            try {
                if (c.moveToFirst()) {
                    final String strUri = c.getString(0);
                    if (strUri != null) {
                        return Uri.parse(strUri);
                    }
                }
            } finally {
                c.close();
            }
        }
        return attachmentUri;
    }

    /**
     * In support of deleting a message, find all attachments and delete associated attachment
     * files.
     * @param context
     * @param accountId the account for the message
     * @param messageId the message
     */
    public static void deleteAllAttachmentFiles(Context context, long accountId, long messageId) {
        Uri uri = ContentUris.withAppendedId(Attachment.MESSAGE_ID_URI, messageId);
        Cursor c = context.getContentResolver().query(uri, Attachment.ID_CONTENTURI_ACCOUNT_PROJECTION,
                null, null, null);
        try {
            while (c.moveToNext()) {
                long attachmentId = c.getLong(Attachment.ID_PROJECTION_COLUMN);
                File attachmentFile = getAttachmentFilename(context, accountId, attachmentId);
                // Note, delete() throws no exceptions for basic FS errors (e.g. file not found)
                // it just returns false, which we ignore, and proceed to the next file.
                // This entire loop is best-effort only.
                if (!attachmentFile.delete()) {
                    LogUtils.d(TAG, "Can't delete the attachment: %s, it may reference another attchment, it's ok",
                            attachmentFile.toString());
                }
            }
        } finally {
            c.close();
        }
    }

    /**
     * M: Delete all attachment files with selection
     * @note Only use the attachment id to generate attachment files, so if the attachment is referencing
     * other message's attachment file, don't delete it
     * @param context
     * @param accountId
     * @param messageId
     * @param selection
     * @param selectionArgs
     */
    public static void delAllAttFilesWithSelection(Context context, long accountId, long messageId,
            String selection, String[] selectionArgs) {
        Uri uri = ContentUris.withAppendedId(Attachment.MESSAGE_ID_URI, messageId);
        Cursor c = context.getContentResolver().query(uri, Attachment.ID_CONTENTURI_ACCOUNT_PROJECTION,
                selection, selectionArgs, null);
        try {
            while (c.moveToNext()) {
                long attachmentId = c.getLong(Attachment.ID_PROJECTION_COLUMN);
                LogUtils.d(TAG, "delete attachment: " + accountId + ".att//" + attachmentId);
                File attachmentFile = getAttachmentFilename(context, accountId, attachmentId);
                // Note, delete() throws no exceptions for basic FS errors (e.g. file not found)
                // it just returns false, which we ignore, and proceed to the next file.
                // This entire loop is best-effort only.
                if (!attachmentFile.delete()) {
                    LogUtils.d(TAG, "Can't delete the attachment: %s, it may reference another attchment, it's ok",
                            attachmentFile.toString());
                }
            }
        } finally {
            c.close();
        }
    }

    /**
     * In support of deleting a message, find all attachments and delete associated cached
     * attachment files.
     * @param context
     * @param accountId the account for the message
     * @param messageId the message
     */
    public static void deleteAllCachedAttachmentFiles(Context context, long accountId,
            long messageId) {
        final Uri uri = ContentUris.withAppendedId(Attachment.MESSAGE_ID_URI, messageId);
        final Cursor c = context.getContentResolver().query(uri, ATTACHMENT_CACHED_FILE_PROJECTION,
                null, null, null);
        /// M: update cached file field in provider
        ContentValues cv = new ContentValues();
        cv.put(AttachmentColumns.CACHED_FILE, "");
        try {
            while (c.moveToNext()) {
                final String fileName = c.getString(0);
                if (!TextUtils.isEmpty(fileName)) {
                    final File cachedFile = new File(fileName);
                    // Note, delete() throws no exceptions for basic FS errors (e.g. file not found)
                    // it just returns false, which we ignore, and proceed to the next file.
                    // This entire loop is best-effort only.
                    cachedFile.delete();
                    /// M: delete cached field in db
                    final long attId = c.getLong(1);
                    final Uri attUri = ContentUris.withAppendedId(
                            Attachment.CONTENT_URI, attId);
                    int count = context.getContentResolver().update(attUri, cv,
                            null, null);
                    LogUtils.d(TAG, "Delete Cached Files num = " + count);
                }
            }

        } finally {
            c.close();
        }
    }

    /**
     * In support of deleting a mailbox, find all messages and delete their attachments.
     *
     * @param context
     * @param accountId the account for the mailbox
     * @param mailboxId the mailbox for the messages
     */
    public static void deleteAllMailboxAttachmentFiles(Context context, long accountId,
            long mailboxId) {
        Cursor c = context.getContentResolver().query(Message.CONTENT_URI,
                Message.ID_COLUMN_PROJECTION, MessageColumns.MAILBOX_KEY + "=?",
                new String[] { Long.toString(mailboxId) }, null);
        try {
            while (c.moveToNext()) {
                long messageId = c.getLong(Message.ID_PROJECTION_COLUMN);
                deleteAllAttachmentFiles(context, accountId, messageId);
            }
        } finally {
            c.close();
        }
    }

    /**
     * M: In support of deleting all attachment files older than a specific time for a message, and
     * update DB
     * @NOTE attachment files download after the specified time won't be deleted
     * @NOTE attachment files which is being forwarded won't be deleted
     * @param context
     * @param messageId
     * @param time specified time for determine whether to delete or not
     */
    public static void deleteMsgAttachmentFiles(Context context, long messageId,
            long time) {
        Uri uri = ContentUris.withAppendedId(Attachment.MESSAGE_ID_URI, messageId);
        Cursor c = context.getContentResolver().query(uri,
                Attachment.ID_CONTENTURI_ACCOUNT_PROJECTION,
                null, null, null);
        if (c == null) {
            return;
        }
        try {
            Message message = Message.restoreMessageWithId(context, messageId);
            // This gonna be a very odd exception that should not happen in normal case
            if (message == null) {
                return;
            }
            Mailbox outbox = Mailbox.restoreMailboxOfType(context, message.mAccountKey,
                    Mailbox.TYPE_OUTBOX);
            Mailbox draft = Mailbox.restoreMailboxOfType(context, message.mAccountKey,
                    Mailbox.TYPE_DRAFTS);

            String draftOutboxMsgSelection = null;
            String refSelection = null;
            while (c.moveToNext()) {
                long attachmentId = c.getLong(Attachment.ID_PROJECTION_COLUMN);
                long accountId = c.getLong(Attachment.ID_CONTENTURI_ACCOUNT_PROJECTION_COLUMN_ACCOUNT);
                String contentUri = c.getString(Attachment.ID_CONTENTURI_ACCOUNT_PROJECTION_COLUMN_CONTENTURI);
                File attachmentFile = getAttachmentFilename(context, accountId, attachmentId);
                if (attachmentFile.exists() && attachmentFile.lastModified() < time) {
                    // We should check whether this attachment file has been referenced by other
                    // messages, eg. this message is forwarded by another message, if so, we can't
                    // delete it
                    // @NOTE we should query the reference synchronously, to make write/read DB in a
                    // right order
                    synchronized (SYNCHRONIZE_LOCK_FOR_FORWARD_ATTACHMENT) {
                        boolean referenced = false;
                        if (draftOutboxMsgSelection == null) {
                            draftOutboxMsgSelection = "SELECT " + Message.RECORD_ID + " FROM "
                                    + Message.TABLE_NAME + " WHERE (" + MessageColumns.MAILBOX_KEY
                                    + " IN (" + (outbox != null ? outbox.mId : null) + ", "
                                    + (draft != null ? draft.mId : null) + "))";
                        }
                        if (refSelection == null) {
                            refSelection = "(" + AttachmentColumns.CONTENT_URI + " = ?) AND ("
                                    + AttachmentColumns.MESSAGE_KEY + " IN ("
                                    + draftOutboxMsgSelection + "))";
                        }
                        Cursor refCur = context.getContentResolver().query(Attachment.CONTENT_URI,
                                Attachment.ID_PROJECTION, refSelection, new String[] {contentUri},
                                null);
                        if (refCur != null) {
                            try {
                                if (refCur.getCount() > 0) {
                                    referenced = true;
                                }
                            } finally {
                                refCur.close();
                            }
                        }
                        if (!referenced) {
                            boolean isSuccess = attachmentFile.delete();
                            LogUtils.d(TAG, "delete attachment file: " + attachmentFile.toString()
                                    + " Success: " + isSuccess);
                            if (isSuccess) {
                                ContentValues cv = new ContentValues();
                                cv.putNull(AttachmentColumns.CONTENT_URI);
                                cv.put(AttachmentColumns.UI_STATE, UIProvider.AttachmentState.NOT_SAVED);
                                Attachment.update(context, Attachment.CONTENT_URI, attachmentId, cv);
                                LogUtils.d(TAG, "Update atta    chment content uri to null for: " + attachmentId);
                            }
                        }
                    }
                }
            }
        } finally {
            c.close();
        }
    }


    /**
     * In support of deleting or wiping an account, delete all related attachments.
     *
     * @param context
     * @param accountId the account to scrub
     */
    public static void deleteAllAccountAttachmentFiles(Context context, long accountId) {
        File[] files = getAttachmentDirectory(context, accountId).listFiles();
        if (files == null) return;
        for (File file : files) {
            boolean result = file.delete();
            if (!result) {
                LogUtils.e(Logging.LOG_TAG, "Failed to delete attachment file " + file.getName());
            }
        }
    }

    private static long copyFile(InputStream in, OutputStream out) throws IOException {
        long size = IOUtils.copy(in, out);
        in.close();
        out.flush();
        out.close();
        return size;
    }

    /**
     * M: Copy attachment form internal to external.Do it in background and delete the internal file
     *
     * @param context
     * @param uri
     */
    public static void copyAttachmentFromInternalToExternal(final Context context,
            final Uri attUri, final Uri contentUri) {
        copyAttachmentFromInternalToExternal(context, attUri, contentUri, true, true, null);
    }

    /**
     * M: Copy attachment form internal to external.Do it in background but not delete the internal file
     *
     * @param context
     * @param uri
     */
    public static void copyAttachmentFromInternalToExternal(final Context context,
            final Uri attUri, final Uri contentUri, final CopyAttachmentCallback cb) {
        copyAttachmentFromInternalToExternal(context, attUri, contentUri, false, false, cb);
    }

    /**
     * M: Copy attachment form internal to external.Do it in background.
     *
     * @param context
     * @param uri
     * @param deleteSource if we need to delete the cache file after copy completed
     * @param updateDb If we need to update the attachment info in db
     * @param CopyAttachmentCallback when copy completed, do something, if null, do nothing
     */
    public static void copyAttachmentFromInternalToExternal(final Context context,
            final Uri attUri, final Uri contentUri, final boolean deleteSource,
            final boolean updateDb, final CopyAttachmentCallback cb) {
        new EmailAsyncTask<Void, Void, String>(null /* no cancel */) {
            @Override
            protected String doInBackground(Void... params) {
                String resultUri = null;
                ///M: for attachment in eml file, we need process it in this special way. @{
                if (contentUri.toString().contains(EML_ATTACHMENT_PROVIDER)) {
                    ContentValues cv = new ContentValues();
                    cv.put(UIProvider.AttachmentColumns.STATE, UIProvider.AttachmentState.SAVED);
                    cv.put(UIProvider.AttachmentColumns.DESTINATION, UIProvider.AttachmentDestination.EXTERNAL);
                    context.getContentResolver().update(contentUri, cv, null, null);
                /// @}
                } else {
                    long id = Long.parseLong(attUri.getLastPathSegment());
                    EmailContent.Attachment dbAttachment = EmailContent.Attachment.restoreAttachmentWithId(context, id);
                    /// M: Judge dbAttachment to fix seldom NPE.
                    if (dbAttachment == null) {
                        LogUtils.d(LogUtils.TAG, "copyAttachmentFromInToEx: dbAttachment is null");
                        return resultUri;
                    }
                    /// @}
                    try {
                        InputStream in = context.getContentResolver().openInputStream(contentUri);
                        dbAttachment.mUiDestination = UIProvider.AttachmentDestination.EXTERNAL;
                        resultUri = saveAttachment(context, in, dbAttachment, updateDb);
                        File internalAttachment = AttachmentUtilities.getAttachmentFilename(context, dbAttachment.mAccountKey, id);
                        if (deleteSource) {
                            if (internalAttachment != null && !internalAttachment.delete()) {
                                LogUtils.d(LogUtils.TAG, " copyAttachmentFromInternalToExternal : delete raw attachment failed. %s",
                                        dbAttachment.mFileName);
                            }
                        }
                        LogUtils.d(LogUtils.TAG, " copyAttachmentFromInternalToExternal : copy internal attachment to external. %s",
                                dbAttachment.mFileName);
                    } catch (IOException ioe) {
                        LogUtils.d(LogUtils.TAG, " IO exception when copy internal attachment to external.");
                    }
                }
                return resultUri;
            }

            @Override
            protected void onSuccess(String resultUri) {
                if (null != cb) {
                    cb.onCopyCompleted(resultUri);
                }
            }
        } .executeParallel((Void []) null);
    }

    /**
     * M: Cache attachment files which would be shared from external
     * @param context the context
     * @param attachment attachment cached in message
     * @param attachmentFds attachment's fd from external
     * @return
     */
    public static int cacheAttachmentInternal(Context context, Attachment attachment,
            Bundle attachmentFds, HashMap<String, String> uriMap) {
        InputStream inputStream = null;
        String originalUri = attachment.getContentUri();
        // Only new added external attachments need to be cached, ignore internal ones
        if ((!TextUtils.isEmpty(originalUri) && originalUri.startsWith(Attachment.ATTACHMENT_PROVIDER_URI_PREFIX))/** internal */
                || (TextUtils.isEmpty(originalUri) && attachment.mLocation != null)/** download/smart forward */
                || (!TextUtils.isEmpty(attachment.getCachedFileUri())
                        && attachment.getCachedFileUri().startsWith(Attachment.ATTACHMENT_PROVIDER_URI_PREFIX))/** cached */) {
            LogUtils.d(TAG, "No necessary to cache attachment: %s", attachment);
            return CACHE_NO_NEED;
        }
        try {
            String cachedUri = uriMap.get(originalUri);
            boolean needRecordUri = false;
            if (cachedUri == null) {
                needRecordUri = true;
                ParcelFileDescriptor fileDescriptor = null;
                // for vcf, we have to use AssetFileDescriptor to access it.
                AssetFileDescriptor vcfFileDescriptor = null;

                if (attachmentFds != null && originalUri != null) {
                    if (originalUri.toString().startsWith(UIProvider.ATTACHMENT_CONTACT_URI_PREFIX)) {
                        // for vcf, we have to use AssetFileDescriptor to access it.
                        vcfFileDescriptor = (AssetFileDescriptor) attachmentFds.getParcelable(originalUri.toString());
                    } else {
                        fileDescriptor = (ParcelFileDescriptor) attachmentFds.getParcelable(originalUri.toString());
                    }
                }
                if (fileDescriptor != null) {
                    // Get the input stream from the file descriptor
                    inputStream = new FileInputStream(fileDescriptor.getFileDescriptor());
                } else if (vcfFileDescriptor != null) {
                    inputStream = new FileInputStream(vcfFileDescriptor.getFileDescriptor());
                } else {
                    // Attempt to open the file
                    inputStream = context.getContentResolver().openInputStream(Uri.parse(originalUri));
                }
            } else {
                inputStream = context.getContentResolver().openInputStream(Uri.parse(cachedUri));
            }
            cacheAttachment(context, inputStream, attachment);
            if (needRecordUri) {
                uriMap.put(originalUri, attachment.getContentUri());
                LogUtils.d(TAG, "Map uri < %s > to < %s >", originalUri, attachment.getContentUri());
            }
        } catch (SecurityException sex) {
            LogUtils.w(TAG, "Security Exception: %s", sex);
            return CACHE_FAIL;
        } catch (FileNotFoundException ex) {
            LogUtils.w(TAG, "File not found: %s", ex);
            return CACHE_FAIL;
        } finally {
            try {
                if (inputStream != null) {
                    inputStream.close();
                }
            } catch (IOException e) {
                LogUtils.w(TAG, e, "Failed to close stream");
            }
        }
        return CACHE_SUCCESS;
    }

    /**
     * M: Cache attachment files which would be shared from external
     * @param context
     * @param in inputStream of caching file
     * @param attachment attachment need to be cached
     */
    public static void cacheAttachment(Context context, InputStream in, Attachment attachment) {
        saveAttachment(context, in, attachment, true, true);
    }

    /**
     * M: Wrap the original saveAttachment
     * @param context
     * @param in
     * @param attachment
     */
    public static void saveAttachment(Context context, InputStream in, Attachment attachment) {
        saveAttachment(context, in, attachment, false, true);
    }

    /**
     * M: Just copy the file but not update the database, also do not show the notification
     * when copy completed. Current if do not update database, then also do not show notification,
     * Just use the updateDb to control
     * @param context
     * @param in
     * @param attachment
     * @param updateDb If true, update database when copy completed
     * @return The saved file's uri
     */
    public static String saveAttachment(Context context, InputStream in, Attachment attachment, boolean updateDb) {
        return saveAttachment(context, in, attachment, false, updateDb);
    }

    /**
     * M: Update CACHED_FILE instead of CONTENT_URI when cache external file
     * Save the attachment to its final resting place (cache or sd card)
     * @param updateDb If true, update database when copy completed
     */
    private static String saveAttachment(Context context, InputStream in, Attachment attachment,
            boolean cacheFile, boolean updateDb) {
        final Uri uri = ContentUris.withAppendedId(Attachment.CONTENT_URI, attachment.mId);
        final ContentValues cv = new ContentValues();
        final long attachmentId = attachment.mId;
        final long accountId = attachment.mAccountKey;
        String contentUri = null;
        final long size;

        try {
            ContentResolver resolver = context.getContentResolver();
            /// M: we copy to internal storage when uiDestination=cache or is in caching process
            if (attachment.mUiDestination == UIProvider.AttachmentDestination.CACHE || cacheFile) {
                Uri attUri = getAttachmentUri(accountId, attachmentId);
                size = copyFile(in, resolver.openOutputStream(attUri));
                contentUri = attUri.toString();
            } else if (Utilities.isExternalStorageMounted()) {
                if (TextUtils.isEmpty(attachment.mFileName)) {
                    // TODO: This will prevent a crash but does not surface the underlying problem
                    // to the user correctly.
                    LogUtils.w(Logging.LOG_TAG, "Trying to save an attachment with no name: %d",
                            attachmentId);
                    throw new IOException("Can't save an attachment with no name");
                }
                /// M: get the current using storage but not the default storage.
                String attsSavedPath = StorageManagerEx.getDefaultPath();
                File downloads = new File(attsSavedPath, Environment.DIRECTORY_DOWNLOADS);
                /**
                 * M: Adjust the attachment's name is valid or not. If not,
                 * remove the invalid character.Meanwhile dispose the long
                 * filename. The max length is 255. @{
                 */
                if (!downloads.mkdirs()) {
                    Logging.w(Logging.LOG_TAG, " saveAttachment mkdirs failed");
                }
                String filename = attachment.mFileName;
                if (!Utilities.isFileNameValid(filename)) {
                    filename = Utilities.removeFileNameSpecialChar(filename);
                }
                filename = Utilities.getLengthSafeFileName(filename);
                File file = Utilities.createUniqueFile(downloads, filename);
                /** @} */
                size = copyFile(in, new FileOutputStream(file));
                String absolutePath = file.getAbsolutePath();

                // Although the download manager can scan media files, scanning only happens
                // after the user clicks on the item in the Downloads app. So, we run the
                // attachment through the media scanner ourselves so it gets added to
                // gallery / music immediately.
                MediaScannerConnection.scanFile(context, new String[] {absolutePath},
                        null, null);

                /// M: Set attachment size = 0 into DownloadManager will make it crush, skip it
                if (size > 0) {
                    String mimeType = TextUtils.isEmpty(attachment.mMimeType) ?
                        "application/octet-stream" :
                        attachment.mMimeType;
                    /** M: MTK Dependence. For sd drm file, use proper mime type in adding to downloads database
                      * system, then the file can be opened @{ */
                    if (FeatureOption.MTK_DRM_APP
                            && (MIME_DCF.equalsIgnoreCase(attachment.mMimeType)
                            || isDrmAttachment(context, Uri.fromFile(file)))) {
                        //mimeType = OmaDrmStore.DrmObjectMime.MIME_DRM_CONTENT;
                        mimeType = MIME_TYPE_DRM_CONTENT;
                        LogUtils.d(Logging.LOG_TAG,
                                "reset drm mime type, original [%s], new [%s]",
                                attachment.mMimeType, mimeType);
                    }
                    /** @} */
                    LogUtils.d(Logging.LOG_TAG,
                            "save attachment to downloadManager with name: %s, mimeType: %s, length:%d",
                            attachment.mFileName, mimeType, size);
                    try {
                        DownloadManager dm =
                                (DownloadManager) context.getSystemService(Context.DOWNLOAD_SERVICE);
                        /// M: Use the saved file name
                        long id = dm.addCompletedDownload(filename, filename,
                                true /* M: Use media scanner to make sure music/video can also be deleted from music/gallery
                                       when deleting them from download app.*/,
                                mimeType, absolutePath, size,
                                /** M: If update database, then also show notification. otherwise false*/
                                updateDb /* show notification */);
                        contentUri = dm.getUriForDownloadedFile(id).toString();
                        /// M: From Android M, the Uri got back from download
                        // manager starts with scheme "content://", but it needs
                        // a Uri which starts with scheme "file://", so we use
                        // the absolute path to construct a Uri which starts
                        // with scheme "file://"@{
                        LogUtils.d(LogUtils.TAG, "Download manager Uri is %s",
                                contentUri);
                        //contentUri = Uri.fromFile(new File(absolutePath))
                         //       .toString();
                        /// @}
                  } catch (final IllegalArgumentException e) {
                      LogUtils.d(LogUtils.TAG, e, "IAE from DownloadManager while saving attachment");
                      throw new IOException(e);
                  }
               } else {
                   /// M: Did not download yet ? try again.
                   LogUtils.w(Logging.LOG_TAG,
                           "Trying to save an attachment downloaded failed");
                   throw new IOException("attachment did not download yet");
               }
            } else {
                LogUtils.w(Logging.LOG_TAG,
                        "Trying to save an attachment without external storage?");
                throw new IOException();
            }

            // Update the attachment
            cv.put(AttachmentColumns.SIZE, size);
            if (cacheFile) {
                /// M: Update CACHED_FILE for caching external file
                cv.put(AttachmentColumns.CACHED_FILE, contentUri);
            }
            /// M: update content uri
            attachment.setContentUri(contentUri);
            cv.put(AttachmentColumns.CONTENT_URI, contentUri);
            cv.put(AttachmentColumns.UI_STATE, UIProvider.AttachmentState.SAVED);
            /// M: also update destination
            cv.put(AttachmentColumns.UI_DESTINATION, attachment.mUiDestination);
        } catch (IOException e) {
            // Handle failures here...
            cv.put(AttachmentColumns.UI_STATE, UIProvider.AttachmentState.FAILED);
        }
        if (updateDb) {
            context.getContentResolver().update(uri, cv, null, null);
        }

        /// M: TODO need we remove all those code ?
        /*
        /// M: If this is an inline attachment, update the body
        refactorHtmlBodyWithInlineAttachments(context, attachment.mMessageKey,
                new Attachment [] { attachment });
        */
        return contentUri;
    }

    /** M: get the regular expression string with contentId or contentUri
     *
     * @param content contentId or contentUri string
     * @param background Is it a background image content
     * @param contentId Is it a contentId string
     * @return A html image string for regular expression
     */
    public static String getImgHtmlStr(String content, boolean background, boolean contentId) {
        String imgHtmlString = "";
        // Regexp which matches ' src="cid:contentId"'.
        // String contentIdRe = "\\s+(?i)(\"src\"|\"backkground\")=\"cid:(?-i)\\Q" + attachment.mContentId + "\\E\"";
        String srcString = background ? "background" : "src";
        String formatString = contentId ? "\\s+(?i)%s=\"cid:(?-i)\\Q%s\\E\"" : " %s=\"%s\"";
        imgHtmlString = String.format(formatString, srcString, content);
        return imgHtmlString;
    }

    /**
     * M: Refactor body HTML of Messages which include inline images
     * @param attachment the inline attachment
     */
    public static String refactorHtmlBodyForAttachment(String bodyHtml, Attachment attachment) {
        String contentUri = attachment.getContentUri();

        // If this is an inline attachment, update the body html
        if (contentUri != null && attachment.mContentId != null) {
            if (!TextUtils.isEmpty(bodyHtml)) {
                // for html body, replace CID for inline images
                final boolean forBackground = true;
                final boolean forContentId = true;
                String contentIdRe = getImgHtmlStr(attachment.mContentId, !forBackground, forContentId);
                String srcContentUri = getImgHtmlStr(contentUri, !forBackground, !forContentId);
                String backgroundRe = getImgHtmlStr(attachment.mContentId, forBackground, forContentId);
                String backgroundUri = getImgHtmlStr(contentUri, forBackground, !forContentId);

                // Some Emails with background images will have htmls like:
                // background=cid:image0001:XXXXXXXXXXXXXXX
                // so we need to replace this contentId to contentUri
                bodyHtml = bodyHtml.replaceAll(backgroundRe, backgroundUri);
                /** if it's not inline picture or Application data , we need removed the img tag */
                if ((attachment.mMimeType != null)
                        && (attachment.mMimeType.toLowerCase().startsWith("image") || attachment.mMimeType
                                .toLowerCase().startsWith(GENERAL_MIME_TYPE))) {
                    bodyHtml = bodyHtml.replaceAll(contentIdRe, srcContentUri);
                } else {
                    String imgContentIdRe = getImgHtmlStr(attachment.mContentId, !forBackground, forContentId);
                    if (Pattern.compile(imgContentIdRe, Pattern.CASE_INSENSITIVE).matcher(bodyHtml)
                            .find()) {
                        bodyHtml = bodyHtml.replaceAll(imgContentIdRe, "");
                    } else {
                        bodyHtml = bodyHtml.replaceAll(contentIdRe, srcContentUri);
                    }
                }
            }
        }
        return bodyHtml;
    }

    /**
     * M: Refactor body HTML of Messages which include inline images
     * @param context
     * @param messageId the message's id of which need to be refactor
     * @param attachments the attachments list of this message
     */
    public static void refactorHtmlBodyWithInlineAttachments(Context context, long messageId,
            Attachment[] attachments) {
        if (attachments == null || attachments.length <= 0) {
            // Check attachments first for quick judgment
            return;
        }

        ContentValues cv = new ContentValues();
        Body body = Body.restoreBodyWithMessageId(context, messageId);
        if (body == null || TextUtils.isEmpty(body.mHtmlContent)) {
            // Just exit if body or body.html is unavailable
            return;
        }

        String refactoredHtml = body.mHtmlContent;
        int bodyLength = refactoredHtml.length();
        for (Attachment attachment : attachments) {
            LogUtils.d(TAG, "refactorHtmlBodyWithInlineAttachments attachment %s", attachment);
            if (!TextUtils.isEmpty(refactoredHtml) && attachment.mContentId != null) {
                /**
                 * We use contentId to guess inline pictures so far, it's not very accurate,
                 * but that the simplest way as we know to do it.
                 * TODO: Enhancement inline picture judgment if necessary as we did on JB
                 */
                refactoredHtml = refactorHtmlBodyForAttachment(refactoredHtml, attachment);
            }
        }

        boolean bodyChanged = bodyLength != refactoredHtml.length();
        LogUtils.d(TAG, "refactorHtmlBodyWithInlineAttachments bodyHTML %s",
                bodyChanged ? "Changed" : "Without Inline picture");
        // Look at length change first due to effective judgment
        if (bodyChanged
                || !body.mHtmlContent.equals(refactoredHtml)) {
            // Update Body content when something changed
            cv.put(BodyColumns.HTML_CONTENT, refactoredHtml);
            context.getContentResolver().update(
                    ContentUris.withAppendedId(Body.CONTENT_URI, body.mId), cv, null, null);
        }
    }

    /**
     * M: Refactor body HTML of Messages which include inline images
     * @param context
     * @param messageId the message's id of which need to be refactor
     */
    public static void refactorHtmlBodyWithInlineAttachments(Context context, long messageId) {
        Attachment [] attachments = Attachment.restoreAttachmentsWithMessageId(context, messageId);
        refactorHtmlBodyWithInlineAttachments(context, messageId, attachments);
    }

    /**
     * M: return the uri to the caller when copy is completed
     */
    public interface CopyAttachmentCallback {
        void onCopyCompleted(String uri);
    }

    /**
     * M: Check the given attachment uri is DRM file.
     * Note: only available for File Uri.
     * @param context
     * @param uri, file uri
     */
    private static boolean isDrmAttachment(Context context, Uri fileUri) {
        DrmManagerClient drmClient = getDrmClientInstance(context);
        ContentValues cv = drmClient.getMetadata(fileUri);
        final String key = "isdrm";
        Integer isDrm = ((cv != null && cv.containsKey(key)) ? cv.getAsInteger(key): null);
        if (isDrm != null && isDrm > 0) {
            return true;
        }
        return false;
    }

    /**
     * M: Delete attachments associated with message, if the message was
     * belonged to the exchange account.
     *
     * @param context
     * @param uri
     * @param messageid
     * @{
     */
    public static void deleteAttachmentsOfMessageIfNeed(Context context, Uri uri, String messageId) {
        if (Binder.getCallingUid() == context.getApplicationInfo().uid) {
            return;
        }
        String accountId = uri
                .getQueryParameter(AttachmentUtilities.KEY_ACCOUNT_ID);
        if (accountId == null || accountId.isEmpty()) {
            LogUtils.e(TAG, "failed to delete attachments due to no account id");
            return;
        }
        AttachmentUtilities.deleteAllAttachmentFiles(context,
                Long.valueOf(accountId), Long.valueOf(messageId));
    }
    /** @} */

    /**
     * M: Delete all the attachments belonged to a mail which would be deleted,
     * if the mail box was belonged to a exchange account.
     *
     * @param context
     * @param accountId
     * @param mailBoxId
     * @{
     */
    public static void deleteAllAttachmentsOfMailBoxIfNeed(Context context, long accountId, String mailBoxId) {
        if (Binder.getCallingUid() == context.getApplicationInfo().uid) {
            return;
        }
        AttachmentUtilities.deleteAllMailboxAttachmentFiles(context, accountId,
                Long.valueOf(mailBoxId));
    }
    /** @} */
}
