/*
 * Copyright (C) 2007 The Android Open Source Project
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

package com.android.providers.telephony;

import android.annotation.NonNull;
import android.app.AppOpsManager;
import android.content.ContentProvider;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.UriMatcher;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteException;
import android.database.sqlite.SQLiteOpenHelper;
import android.database.sqlite.SQLiteQueryBuilder;
import android.database.sqlite.SqliteWrapper;
import android.net.Uri;
import android.os.Binder;
import android.os.FileUtils;
import android.os.ParcelFileDescriptor;
import android.os.UserHandle;
import android.provider.BaseColumns;
import android.provider.Telephony;
import android.provider.Telephony.CanonicalAddressesColumns;
import android.provider.Telephony.Mms;
import android.provider.Telephony.Mms.Addr;
import android.provider.Telephony.Mms.Inbox;
import android.provider.Telephony.Mms.Part;
import android.provider.Telephony.Mms.Rate;
import android.provider.Telephony.MmsSms;
import android.provider.Telephony.Threads;
import android.text.TextUtils;
import android.util.Log;

import com.google.android.mms.pdu.CharacterSets;
import com.google.android.mms.pdu.PduHeaders;
import com.google.android.mms.util.DownloadDrmHelper;
import mediatek.telephony.MtkTelephony;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.Set;
import java.util.HashSet;


/**
 * The class to provide base facility to access MMS related content,
 * which is stored in a SQLite database and in the file system.
 */
public class MmsProvider extends ContentProvider {
    static final String TABLE_PDU  = "pdu";
    static final String TABLE_ADDR = "addr";
    static final String TABLE_PART = "part";
    static final String TABLE_RATE = "rate";
    static final String TABLE_DRM  = "drm";
    static final String TABLE_WORDS = "words";
    static final String VIEW_PDU_RESTRICTED = "pdu_restricted";

    // The name of parts directory. The full dir is "app_parts".
    static final String PARTS_DIR_NAME = "parts";

    /// M: Code analyze 001, fix bug ALPS00046358, improve multi-delete speed by use batch
    /// processing. reference from page http://www.erpgear.com/show.php?contentid=1111.
    private static final String FOR_MULTIDELETE = "ForMultiDelete";
    /// M: Code analyze 002, fix bug ALPS00262044, not show out unread message
    /// icon after restore messages. notify mms application about unread messages
    /// number after insert operation.
    private static boolean notifyUnread = false;

    private static final Set<String> COLUMNS = new HashSet<String>();

    /// M: for subject. @{
    private static final long ROW_ID_BEGIN_SUBJECT = 2 << 24;
    static final int TABLE_TO_USE_SUBJECT = 4;
    /// @}

    private static final String[] ADDR_PDU_COLUMNS = {
            BaseColumns._ID, Addr.MSG_ID, Addr.CONTACT_ID, Addr.ADDRESS, Addr.TYPE, Addr.CHARSET,
            "pdu_id", "delivery_status", Mms.READ_STATUS
    };

    static {
        for (int i = 0; i < ADDR_PDU_COLUMNS.length; i++) {
            COLUMNS.add(ADDR_PDU_COLUMNS[i]);
        }
    }

    @Override
    public boolean onCreate() {
        setAppOps(AppOpsManager.OP_READ_SMS, AppOpsManager.OP_WRITE_SMS);
        mOpenHelper = MmsSmsDatabaseHelper.getInstanceForCe(getContext());
        TelephonyBackupAgent.DeferredSmsMmsRestoreService.startIfFilesExist(getContext());
        return true;
    }

    /**
     * Return the proper view of "pdu" table for the current access status.
     *
     * @param accessRestricted If the access is restricted
     * @return the table/view name of the mms data
     */
    public static String getPduTable(boolean accessRestricted) {
        return accessRestricted ? VIEW_PDU_RESTRICTED : TABLE_PDU;
    }

    @Override
    public Cursor query(Uri uri, String[] projection,
            String selection, String[] selectionArgs, String sortOrder) {
        // First check if a restricted view of the "pdu" table should be used based on the
        // caller's identity. Only system, phone or the default sms app can have full access
        // of mms data. For other apps, we present a restricted view which only contains sent
        // or received messages, without wap pushes.
        final boolean accessRestricted = ProviderUtil.isAccessRestricted(
                getContext(), getCallingPackage(), Binder.getCallingUid());
        final String pduTable = getPduTable(accessRestricted);

        SQLiteQueryBuilder qb = new SQLiteQueryBuilder();
        MmsSmsProvider.MmsProviderLog.dpi(TAG, "query begin, uri = " + uri
                + ", selection = " + selection);
        // Generate the body of the query.
        int match = sURLMatcher.match(uri);
        if (LOCAL_LOGV) {
            MmsSmsProvider.MmsProviderLog.vpi(TAG, "Query uri=" + uri + ", match=" + match);
        } else {
            Log.v(TAG, "Query begin, match = " + match);
        }

        switch (match) {
            case MMS_ALL:
                constructQueryForBox(qb, Mms.MESSAGE_BOX_ALL, pduTable);
                break;
            case MMS_INBOX:
                constructQueryForBox(qb, Mms.MESSAGE_BOX_INBOX, pduTable);
                break;
            case MMS_SENT:
                constructQueryForBox(qb, Mms.MESSAGE_BOX_SENT, pduTable);
                break;
            case MMS_DRAFTS:
                constructQueryForBox(qb, Mms.MESSAGE_BOX_DRAFTS, pduTable);
                break;
            case MMS_OUTBOX:
                constructQueryForBox(qb, Mms.MESSAGE_BOX_OUTBOX, pduTable);
                break;
            case MMS_ALL_ID:
                qb.setTables(pduTable);
                qb.appendWhere(Mms._ID + "=" + uri.getPathSegments().get(0));
                break;
            case MMS_INBOX_ID:
            case MMS_SENT_ID:
            case MMS_DRAFTS_ID:
            case MMS_OUTBOX_ID:
                qb.setTables(pduTable);
                qb.appendWhere(Mms._ID + "=" + uri.getPathSegments().get(1));
                qb.appendWhere(" AND " + Mms.MESSAGE_BOX + "="
                        + getMessageBoxByMatch(match));
                break;
            case MMS_ALL_PART:
                qb.setTables(TABLE_PART);
                break;
            case MMS_MSG_PART:
                qb.setTables(TABLE_PART);
                qb.appendWhere(Part.MSG_ID + "=" + uri.getPathSegments().get(0));
                break;
            case MMS_PART_ID:
                qb.setTables(TABLE_PART);
                qb.appendWhere(Part._ID + "=" + uri.getPathSegments().get(1));
                break;
            case MMS_MSG_ADDR:
                qb.setTables(TABLE_ADDR);
                qb.appendWhere(Addr.MSG_ID + "=" + uri.getPathSegments().get(0));
                break;
            case MMS_REPORT_STATUS:
                /*
                   SELECT DISTINCT address,
                                   T.delivery_status AS delivery_status,
                                   T.read_status AS read_status
                   FROM addr
                   INNER JOIN (SELECT P1._id AS id1, P2._id AS id2, P3._id AS id3,
                                      ifnull(P2.st, 0) AS delivery_status,
                                      ifnull(P3.read_status, 0) AS read_status
                               FROM pdu P1
                               INNER JOIN pdu P2
                               ON P1.m_id = P2.m_id AND P2.m_type = 134
                               LEFT JOIN pdu P3
                               ON P1.m_id = P3.m_id AND P3.m_type = 136
                               UNION
                               SELECT P1._id AS id1, P2._id AS id2, P3._id AS id3,
                                      ifnull(P2.st, 0) AS delivery_status,
                                      ifnull(P3.read_status, 0) AS read_status
                               FROM pdu P1
                               INNER JOIN pdu P3
                               ON P1.m_id = P3.m_id AND P3.m_type = 136
                               LEFT JOIN pdu P2
                               ON P1.m_id = P2.m_id AND P2.m_type = 134) T
                   ON (msg_id = id2 AND type = 151)
                   OR (msg_id = id3 AND type = 137)
                   WHERE T.id1 = ?;
                 */
                /*
                qb.setTables(TABLE_ADDR + " INNER JOIN "
                        + "(SELECT P1._id AS id1, P2._id AS id2, P3._id AS id3, "
                        + "ifnull(P2.st, 0) AS delivery_status, "
                        + "ifnull(P3.read_status, 0) AS read_status "
                        + "FROM " + pduTable + " P1 INNER JOIN " + pduTable + " P2 "
                        + "ON P1.m_id=P2.m_id AND P2.m_type=134 "
                        + "LEFT JOIN " + pduTable + " P3 "
                        + "ON P1.m_id=P3.m_id AND P3.m_type=136 "
                        + "UNION "
                        + "SELECT P1._id AS id1, P2._id AS id2, P3._id AS id3, "
                        + "ifnull(P2.st, 0) AS delivery_status, "
                        + "ifnull(P3.read_status, 0) AS read_status "
                        + "FROM " + pduTable + " P1 INNER JOIN " + pduTable + " P3 "
                        + "ON P1.m_id=P3.m_id AND P3.m_type=136 "
                        + "LEFT JOIN " + pduTable + " P2 "
                        + "ON P1.m_id=P2.m_id AND P2.m_type=134) T "
                        + "ON (msg_id=id2 AND type=151) OR (msg_id=id3 AND type=137)");
                qb.appendWhere("T.id1 = " + uri.getLastPathSegment());
                qb.setDistinct(true);
                */
                 SQLiteQueryBuilder deliveryQueryBuilder = new SQLiteQueryBuilder();
                 SQLiteQueryBuilder readQueryBuilder = new SQLiteQueryBuilder();
                 SQLiteQueryBuilder unionQueryBuilder = new SQLiteQueryBuilder();
                 deliveryQueryBuilder.setTables(" addr inner join ("
                         + " select _id as pdu_id, ifnull(st,0) as delivery_status,"
                         + "ifnull(read_status,0) as read_status from pdu "
                         + " where (m_type=134) and (pdu_id in (select _id from pdu where m_id = "
                         + "(select m_id from pdu where _id = "
                         + uri.getLastPathSegment() + ")))"
                         + ") on ( addr.msg_id=pdu_id and addr.type=151)");
                 readQueryBuilder.setTables(" addr inner join ("
                         + " select _id as pdu_id, ifnull(st,0) as delivery_status,"
                         + " ifnull(read_status,0) as read_status from pdu "
                         + " where (m_type=136) and (pdu_id in (select _id from pdu where m_id = "
                         + "(select m_id from pdu where _id = "
                         + uri.getLastPathSegment() + ")))"
                         + ") on ( addr.msg_id=pdu_id and addr.type=137)");

                 String[] idColumn = new String[] { Addr.ADDRESS, "delivery_status", "read_status"};
                 String deliverySubQuery = deliveryQueryBuilder.buildUnionSubQuery(
                                  "status", ADDR_PDU_COLUMNS, COLUMNS, 0, "delivery",
                                  null, null, null);
                 String readSubQuery = readQueryBuilder.buildUnionSubQuery(
                                  "status", ADDR_PDU_COLUMNS, COLUMNS, 0, "readreport",
                                  null, null, null);
                 String unionQuery = null;
                 unionQuery = unionQueryBuilder.buildUnionQuery(
                                 new String[] { deliverySubQuery, readSubQuery}, null, null);
                 MmsSmsProvider.MmsProviderLog.dpi(TAG, "unionQuery = " + unionQuery);
                 qb.setTables("(" + unionQuery + ")");
                break;
            case MMS_REPORT_REQUEST:
                /*
                   SELECT address, d_rpt, rr
                   FROM addr join pdu on pdu._id = addr.msg_id
                   WHERE pdu._id = messageId AND addr.type = 151
                 */
                qb.setTables(TABLE_ADDR + " join " +
                        pduTable + " on " + pduTable + "._id = addr.msg_id");
                qb.appendWhere(pduTable + "._id = " + uri.getLastPathSegment());
                qb.appendWhere(" AND " + TABLE_ADDR + ".type = " + PduHeaders.TO);
                break;
            case MMS_SENDING_RATE:
                qb.setTables(TABLE_RATE);
                break;
            case MMS_DRM_STORAGE_ID:
                qb.setTables(TABLE_DRM);
                qb.appendWhere(BaseColumns._ID + "=" + uri.getLastPathSegment());
                break;
            case MMS_THREADS:
                qb.setTables(pduTable + " group by thread_id");
                break;
            default:
                MmsSmsProvider.MmsProviderLog.epi(TAG, "query: invalid request: " + uri);
                return null;
        }

        String finalSortOrder = null;
        if (TextUtils.isEmpty(sortOrder)) {
            if (qb.getTables().equals(pduTable)) {
                finalSortOrder = Mms.DATE + " DESC";
            } else if (qb.getTables().equals(TABLE_PART)) {
                finalSortOrder = Part.SEQ;
            }
        } else {
            finalSortOrder = sortOrder;
        }

        Cursor ret;
        try {
            SQLiteDatabase db = mOpenHelper.getReadableDatabase();
            ret = qb.query(db, projection, selection,
                    selectionArgs, null, null, finalSortOrder);
        } catch (SQLiteException e) {
            MmsSmsProvider.MmsProviderLog.epi(TAG, "returning NULL cursor, query: " + uri, e);
            return null;
        }

        Log.d(TAG, "query end, cursor count =" + ret.getCount());
        // TODO: Does this need to be a URI for this provider.
        ret.setNotificationUri(getContext().getContentResolver(), uri);
        return ret;
    }

    private void constructQueryForBox(SQLiteQueryBuilder qb, int msgBox, String pduTable) {
        qb.setTables(pduTable);

        if (msgBox != Mms.MESSAGE_BOX_ALL) {
            qb.appendWhere(Mms.MESSAGE_BOX + "=" + msgBox);
        }
    }

    @Override
    public String getType(Uri uri) {
        int match = sURLMatcher.match(uri);
        switch (match) {
            case MMS_ALL:
            case MMS_INBOX:
            case MMS_SENT:
            case MMS_DRAFTS:
            case MMS_OUTBOX:
                return VND_ANDROID_DIR_MMS;
            case MMS_ALL_ID:
            case MMS_INBOX_ID:
            case MMS_SENT_ID:
            case MMS_DRAFTS_ID:
            case MMS_OUTBOX_ID:
                return VND_ANDROID_MMS;
            case MMS_PART_ID: {
                Cursor cursor = mOpenHelper.getReadableDatabase().query(
                        TABLE_PART, new String[] { Part.CONTENT_TYPE },
                        Part._ID + " = ?", new String[] { uri.getLastPathSegment() },
                        null, null, null);
                if (cursor != null) {
                    try {
                        if ((cursor.getCount() == 1) && cursor.moveToFirst()) {
                            return cursor.getString(0);
                        } else {
                            MmsSmsProvider.MmsProviderLog.epi(TAG, "cursor.count() != 1: " + uri);
                        }
                    } finally {
                        cursor.close();
                    }
                } else {
                    MmsSmsProvider.MmsProviderLog.epi(TAG, "cursor == null: " + uri);
                }
                return "*/*";
            }
            case MMS_ALL_PART:
            case MMS_MSG_PART:
            case MMS_MSG_ADDR:
            default:
                return "*/*";
        }
    }

    @Override
    public Uri insert(Uri uri, ContentValues values) {
        final int callerUid = Binder.getCallingUid();
        final String callerPkg = getCallingPackage();
        MmsSmsProvider.MmsProviderLog.dpi(TAG, "insert begin, uri = "
                + uri + ", values = " + values);
        int msgBox = Mms.MESSAGE_BOX_ALL;
        boolean notify = true;
        /// M: Code analyze 002, fix bug ALPS00262044, not show out unread message
        /// icon after restore messages. notify mms application about unread messages
        /// number after insert operation.
        notifyUnread = true;
        int match = sURLMatcher.match(uri);
        if (LOCAL_LOGV) {
            MmsSmsProvider.MmsProviderLog.vpi(TAG, "Insert uri=" + uri + ", match=" + match + ", values = " + values);
        }

        String table = TABLE_PDU;
        switch (match) {
            case MMS_ALL:
                Object msgBoxObj = values.getAsInteger(Mms.MESSAGE_BOX);
                if (msgBoxObj != null) {
                    msgBox = (Integer) msgBoxObj;
                }
                else {
                    // default to inbox
                    msgBox = Mms.MESSAGE_BOX_INBOX;
                }
                break;
            case MMS_INBOX:
                /// M: Code analyze 004, fix bug ALPS00249113, if query result is not updated
                /// completely, the result will show abnormal. adjust notify time while
                /// store messages. @{
                if (values.containsKey("need_notify")) {
                    notify = values.getAsBoolean("need_notify");
                }
                /// @}
                msgBox = Mms.MESSAGE_BOX_INBOX;
                break;
            case MMS_SENT:
                msgBox = Mms.MESSAGE_BOX_SENT;
                break;
            case MMS_DRAFTS:
                msgBox = Mms.MESSAGE_BOX_DRAFTS;
                break;
            case MMS_OUTBOX:
                msgBox = Mms.MESSAGE_BOX_OUTBOX;
                break;
            case MMS_MSG_PART:
                notify = false;
                table = TABLE_PART;
                break;
            case MMS_MSG_ADDR:
                notify = false;
                table = TABLE_ADDR;
                break;
            case MMS_SENDING_RATE:
                notify = false;
                table = TABLE_RATE;
                break;
            case MMS_DRM_STORAGE:
                notify = false;
                table = TABLE_DRM;
                break;
            /// M: Code analyze 005, fix bug ALPS00275452, count the attachments size. @{
            case MMS_ATTACHMENT_SIZE:
                long size = getAttachmentsSize();
                uri = uri.buildUpon().appendQueryParameter("size", String.valueOf(size)).build();
                return uri;
            /// @}
            default:
                MmsSmsProvider.MmsProviderLog.epi(TAG, "insert: invalid request: " + uri);
                return null;
        }

        SQLiteDatabase db = mOpenHelper.getWritableDatabase();
        ContentValues finalValues;
        Uri res = Mms.CONTENT_URI;
        Uri caseSpecificUri = null;
        long rowId;
        /// M: Code analyze 004, fix bug ALPS00249113, if query result is not updated
        /// completely, the result will show abnormal. adjust notify time while
        /// store messages. @{
        if (values.containsKey("need_notify")) {
            values.remove("need_notify");
        }
        /// @}
        if (table.equals(TABLE_PDU)) {
            boolean addDate = !values.containsKey(Mms.DATE);
            boolean addMsgBox = !values.containsKey(Mms.MESSAGE_BOX);

            // Filter keys we don't support yet.
            filterUnsupportedKeys(values);

            // TODO: Should initialValues be validated, e.g. if it
            // missed some significant keys?
            finalValues = new ContentValues(values);

            long timeInMillis = System.currentTimeMillis();

            if (addDate) {
                finalValues.put(Mms.DATE, timeInMillis / 1000L);
            }

            if (addMsgBox && (msgBox != Mms.MESSAGE_BOX_ALL)) {
                finalValues.put(Mms.MESSAGE_BOX, msgBox);
            }

            if (msgBox != Mms.MESSAGE_BOX_INBOX) {
                // Mark all non-inbox messages read.
                finalValues.put(Mms.READ, 1);
            }

            // thread_id
            Long threadId = values.getAsLong(Mms.THREAD_ID);
            String address = values.getAsString(CanonicalAddressesColumns.ADDRESS);

            if (((threadId == null) || (threadId == 0)) && (!TextUtils.isEmpty(address))) {
                /// M: Code analyze 006, fix bug ALPS00231431, thread should not be delete when
                /// receive mms notification, update status of threads table. @{
                long token = Binder.clearCallingIdentity();
                try {
                    threadId = getOrCreateThreadIdInternal(getContext(), address);
                } finally {
                    Binder.restoreCallingIdentity(token);
                }
                finalValues.put(Mms.THREAD_ID, threadId);
                /// @}
            }

            if (ProviderUtil.shouldSetCreator(finalValues, callerUid)) {
                // Only SYSTEM or PHONE can set CREATOR
                // If caller is not SYSTEM or PHONE, or SYSTEM or PHONE does not set CREATOR
                // set CREATOR using the truth on caller.
                // Note: Inferring package name from UID may include unrelated package names
                finalValues.put(Telephony.Mms.CREATOR, callerPkg);
            }

            if ((rowId = db.insert(table, null, finalValues)) <= 0) {
                Log.e(TAG, "MmsProvider.insert: failed!");
                return null;
            }

            // Notify change when an MMS is received.
            if (msgBox == Mms.MESSAGE_BOX_INBOX) {
                caseSpecificUri = ContentUris.withAppendedId(Mms.Inbox.CONTENT_URI, rowId);
            }

            /// M: Code analyze 006, fix bug ALPS00231431, thread should not be delete when
            /// receive mms notification, update status of threads table.
            setThreadStatus(db, values, 0);
            res = Uri.parse(res + "/" + rowId);

            /// M: insert subject into words table. @{
            ContentValues cv = new ContentValues();
            cv.put(Telephony.MmsSms.WordsTable.ID, ROW_ID_BEGIN_SUBJECT + rowId);
            String sub = finalValues.getAsString(Mms.SUBJECT);
            if (finalValues.containsKey(Mms.SUBJECT_CHARSET)
                    && finalValues.getAsInteger(Mms.SUBJECT_CHARSET) != null) {
                int charset = finalValues.getAsInteger(Mms.SUBJECT_CHARSET);
                sub = transformSubjectByCharset(sub, charset);
            }
            cv.put(Telephony.MmsSms.WordsTable.INDEXED_TEXT, sub);
            cv.put(Telephony.MmsSms.WordsTable.SOURCE_ROW_ID, rowId);
            cv.put(Telephony.MmsSms.WordsTable.TABLE_ID, TABLE_TO_USE_SUBJECT);
            db.insert(TABLE_WORDS, Telephony.MmsSms.WordsTable.INDEXED_TEXT, cv);
            /// @}

        } else if (table.equals(TABLE_ADDR)) {
            finalValues = new ContentValues(values);
            finalValues.put(Addr.MSG_ID, uri.getPathSegments().get(0));

            ArrayList<String> addresses = null;
            if (values.containsKey("addresses")) {
                addresses = values.getStringArrayList("addresses");
                values.remove("addresses");
            }

            rowId = 0;
            if (addresses != null && addresses.size() > 0) {
                ContentValues v = new ContentValues(4);
                db.beginTransaction();
                try {
                    for (int index = 0 ; index < addresses.size(); ) {
                        values.clear(); // Clear all values first.
                        v.put(Addr.MSG_ID, uri.getPathSegments().get(0));
                        v.put(Addr.ADDRESS, addresses.get(index++));
                        v.put(Addr.CHARSET, addresses.get(index++));
                        v.put(Addr.TYPE, addresses.get(index++));
                        rowId = db.insert(table, null, v);
                    }
                    db.setTransactionSuccessful();
                } finally {
                    db.endTransaction();
                }
            } else if ((rowId = db.insert(table, null, finalValues)) <= 0) {
                Log.e(TAG, "Failed to insert address");
                return null;
            }

            res = Uri.parse(res + "/addr/" + rowId);
        } else if (table.equals(TABLE_PART)) {
            boolean containsDataPath = values != null && values.containsKey(Part._DATA);
            finalValues = new ContentValues(values);

            if (match == MMS_MSG_PART) {
                finalValues.put(Part.MSG_ID, uri.getPathSegments().get(0));
            }

            String contentType = values.getAsString("ct");

            // text/plain and app application/smil store their "data" inline in the
            // table so there's no need to create the file
            boolean plainText = false;
            boolean smilText = false;
            if ("text/plain".equals(contentType)) {
                if (containsDataPath) {
                    Log.e(TAG, "insert: can't insert text/plain with _data");
                    return null;
                }
                plainText = true;
            } else if ("application/smil".equals(contentType)) {
                if (containsDataPath) {
                    Log.e(TAG, "insert: can't insert application/smil with _data");
                    return null;
                }
                smilText = true;
            }
            if (!plainText && !smilText) {
                String path;
                if (containsDataPath) {
                    // The _data column is filled internally in MmsProvider or from the
                    // TelephonyBackupAgent, so this check is just to avoid it from being
                    // inadvertently set. This is not supposed to be a protection against malicious
                    // attack, since sql injection could still be attempted to bypass the check.
                    // On the other hand, the MmsProvider does verify that the _data column has an
                    // allowed value before opening any uri/files.
                    if (!"com.android.providers.telephony".equals(callerPkg)) {
                        Log.e(TAG, "insert: can't insert _data");
                        return null;
                    }
                    try {
                        path = values.getAsString(Part._DATA);
                        final String partsDirPath = getContext()
                                .getDir(PARTS_DIR_NAME, 0).getCanonicalPath();
                        if (!new File(path).getCanonicalPath().startsWith(partsDirPath)) {
                            Log.e(TAG, "insert: path "
                                    + path
                                    + " does not start with "
                                    + partsDirPath);
                            // Don't care return value
                            return null;
                        }
                    } catch (IOException e) {
                        Log.e(TAG, "insert part: create path failed " + e, e);
                        return null;
                    }
                } else {
                    // Use the filename if possible, otherwise use the current time as the name.
                    String contentLocation = values.getAsString("cl");
                    if (!TextUtils.isEmpty(contentLocation)) {
                        File f = new File(contentLocation);
                        contentLocation = "_" + f.getName();
                    } else {
                        contentLocation = "";
                    }

                    // Generate the '_data' field of the part with default
                    // permission settings.

                    /// M: Code analyze 007, fix bug ALPS00334881, the JE pops up when you add the
                    /// image to message. do not use contentLocation as part of path name. @{
                    /// M: we do not use contentLocation, this maybe a long string
                    ///    the total file length maybe too long, it can not be created!
                    path = getContext().getDir(PARTS_DIR_NAME, 0).getPath()
                            + "/PART_" + System.currentTimeMillis(); // + contentLocation;

                    ///M: add .dcf extension to the filename if it is dcf file @{
                    if (contentLocation.contains(".dcf")) {
                        path = path + ".dcf";
                    }

                    if (DownloadDrmHelper.isDrmConvertNeeded(contentType)) {
                        // Adds the .fl extension to the filename if contentType is
                        // "application/vnd.oma.drm.message"
                        // path = DownloadDrmHelper.modifyDrmFwLockFileExtension(path);
                        path = path + ".fl";
                    }
                }

                finalValues.put(Part._DATA, path);

                File partFile = new File(path);
                if (!partFile.exists()) {
                    try {
                        if (!partFile.createNewFile()) {
                            throw new IllegalStateException(
                                    "Unable to create new partFile: " + path);
                        }
                        // Give everyone rw permission until we encrypt the file
                        // (in PduPersister.persistData). Once the file is encrypted, the
                        // permissions will be set to 0644.
                        int result = FileUtils.setPermissions(path, 0666, -1, -1);
                        if (LOCAL_LOGV) {
                            Log.d(TAG, "MmsProvider.insert setPermissions result: " + result);
                        }
                    } catch (IOException e) {
                        Log.e(TAG, "createNewFile", e);
                        throw new IllegalStateException(
                                "Unable to create new partFile: " + path);
                    }
                }
            }

            if ((rowId = db.insert(table, null, finalValues)) <= 0) {
                Log.e(TAG, "MmsProvider.insert: failed!");
                return null;
            }

            res = Uri.parse(res + "/part/" + rowId);

            // Don't use a trigger for updating the words table because of a bug
            // in FTS3.  The bug is such that the call to get the last inserted
            // row is incorrect.
            if (plainText) {
                // Update the words table with a corresponding row.  The words table
                // allows us to search for words quickly, without scanning the whole
                // table;
                ContentValues cv = new ContentValues();

                // we're using the row id of the part table row but we're also using ids
                // from the sms table so this divides the space into two large chunks.
                // The row ids from the part table start at 2 << 32.
                cv.put(Telephony.MmsSms.WordsTable.ID, (2L << 32) + rowId);
                cv.put(Telephony.MmsSms.WordsTable.INDEXED_TEXT, values.getAsString("text"));
                cv.put(Telephony.MmsSms.WordsTable.SOURCE_ROW_ID, rowId);
                cv.put(Telephony.MmsSms.WordsTable.TABLE_ID, 2);
                db.insert(TABLE_WORDS, Telephony.MmsSms.WordsTable.INDEXED_TEXT, cv);
            }

        } else if (table.equals(TABLE_RATE)) {
            long now = values.getAsLong(Rate.SENT_TIME);
            long oneHourAgo = now - 1000 * 60 * 60;
            // Delete all unused rows (time earlier than one hour ago).
            db.delete(table, Rate.SENT_TIME + "<=" + oneHourAgo, null);
            db.insert(table, null, values);
        } else if (table.equals(TABLE_DRM)) {
            String path = getContext().getDir(PARTS_DIR_NAME, 0).getPath()
                    + "/PART_" + System.currentTimeMillis();
            finalValues = new ContentValues(1);
            finalValues.put("_data", path);

            File partFile = new File(path);
            if (!partFile.exists()) {
                try {
                    if (!partFile.createNewFile()) {
                        throw new IllegalStateException(
                                "Unable to create new file: " + path);
                    }
                } catch (IOException e) {
                    Log.e(TAG, "createNewFile", e);
                    throw new IllegalStateException(
                            "Unable to create new file: " + path);
                }
            }

            if ((rowId = db.insert(table, null, finalValues)) <= 0) {
                Log.e(TAG, "MmsProvider.insert: failed!");
                return null;
            }
            res = Uri.parse(res + "/drm/" + rowId);
        } else {
            throw new AssertionError("Unknown table type: " + table);
        }

        if (notify) {
            /// M: Code analyze 002, fix bug ALPS00262044, not show out unread message
            /// icon after restore messages. notify mms application about unread messages
            /// number after insert operation.
            notifyUnread = false;
            notifyChange(uri, caseSpecificUri);
        }
        Log.d(TAG, "insert succeed, uri = " + res);
        return res;
    }

    private int getMessageBoxByMatch(int match) {
        switch (match) {
            case MMS_INBOX_ID:
            case MMS_INBOX:
                return Mms.MESSAGE_BOX_INBOX;
            case MMS_SENT_ID:
            case MMS_SENT:
                return Mms.MESSAGE_BOX_SENT;
            case MMS_DRAFTS_ID:
            case MMS_DRAFTS:
                return Mms.MESSAGE_BOX_DRAFTS;
            case MMS_OUTBOX_ID:
            case MMS_OUTBOX:
                return Mms.MESSAGE_BOX_OUTBOX;
            default:
                throw new IllegalArgumentException("bad Arg: " + match);
        }
    }

    @Override
    public int delete(Uri uri, String selection,
            String[] selectionArgs) {
        int match = sURLMatcher.match(uri);
        MmsSmsProvider.MmsProviderLog.dpi(TAG, "delete begin, uri = " + uri + ", selection = " + selection);

        String table = null, extraSelection = null;
        boolean notify = false;

        switch (match) {
            case MMS_ALL_ID:
            case MMS_INBOX_ID:
            case MMS_SENT_ID:
            case MMS_DRAFTS_ID:
            case MMS_OUTBOX_ID:
                notify = true;
                table = TABLE_PDU;
                extraSelection = Mms._ID + "=" + uri.getLastPathSegment();
                if (match != MMS_ALL_ID) {
                    extraSelection += " AND " + Mms.MESSAGE_BOX + "=" + getMessageBoxByMatch(match);
                }
                break;
            case MMS_ALL:
                if (selection != null && selection.equals(FOR_MULTIDELETE)) {
                    selection = "_id IN " + SmsProvider.getSmsIdsFromArgs(selectionArgs);
                    selectionArgs = null;
                }
            case MMS_INBOX:
            case MMS_SENT:
            case MMS_DRAFTS:
            case MMS_OUTBOX:
                notify = true;
                table = TABLE_PDU;
                if (match != MMS_ALL) {
                    int msgBox = getMessageBoxByMatch(match);
                    extraSelection = Mms.MESSAGE_BOX + "=" + msgBox;
                }
                break;
            case MMS_ALL_PART:
                table = TABLE_PART;
                break;
            case MMS_MSG_PART:
                table = TABLE_PART;
                extraSelection = Part.MSG_ID + "=" + uri.getPathSegments().get(0);
                break;
            case MMS_PART_ID:
                table = TABLE_PART;
                extraSelection = Part._ID + "=" + uri.getPathSegments().get(1);
                break;
            case MMS_MSG_ADDR:
                table = TABLE_ADDR;
                extraSelection = Addr.MSG_ID + "=" + uri.getPathSegments().get(0);
                break;
            case MMS_DRM_STORAGE:
                table = TABLE_DRM;
                break;
            default:
                MmsSmsProvider.MmsProviderLog.wpi(TAG, "No match for URI '" + uri + "'");
                return 0;
        }

        String finalSelection = concatSelections(selection, extraSelection);
        SQLiteDatabase db = mOpenHelper.getWritableDatabase();
        int deletedRows = 0;

        if (TABLE_PDU.equals(table)) {
            deletedRows = deleteMessages(getContext(), db, finalSelection,
                                         selectionArgs, uri, true);
        } else if (TABLE_PART.equals(table)) {
            deletedRows = deleteParts(db, finalSelection, selectionArgs);
        } else if (TABLE_DRM.equals(table)) {
            deletedRows = deleteTempDrmData(db, finalSelection, selectionArgs);
        } else {
            deletedRows = db.delete(table, finalSelection, selectionArgs);
        }

        if ((deletedRows > 0) && notify) {
            notifyChange(uri, null);
        }
        Log.d(TAG, "delete end, affectedRows = " + deletedRows);
        return deletedRows;
    }

    private static final int DELETE_LIMIT = 100;

    /// M: used to delete one/more pdus in one/more threads.
    /// Both to performance and data integrity. @{
    static int deleteMessages(Context context, SQLiteDatabase db,
            String selection, String[] selectionArgs, Uri uri, boolean isUpdateThread) {
        Log.d(TAG, "deleteMessages, start");
        Cursor cursor = db.query(TABLE_PDU, new String[] { Mms._ID, Mms.THREAD_ID},
                selection, selectionArgs, null, null, null);
        if (cursor == null) {
            return 0;
        }

        int pduCount = cursor.getCount();
        long thread_id = 0;
        Set<Long> threadIds = new HashSet<Long>();
        try {
            if (cursor.getCount() == 0) {
                return 0;
            }
            Set<Long> ids = new HashSet<Long>();
            int count = 0;
            while (cursor.moveToNext()) {
                Long id = cursor.getLong(0);
                thread_id = cursor.getLong(1);
                ids.add(id);
                if (thread_id > 0) {
                    threadIds.add(thread_id);
                }
                count++;
                if (count % 50 > 0 && !cursor.isLast()) {
                    continue;
                }
                String whereClause = Part.MSG_ID + formatInClause(ids);
                Log.d(TAG, "deleteMessages, delete parts where " + whereClause);
                deleteParts(db, whereClause, null);
                ids.clear();
                count = 0;
            }
        } finally {
            cursor.close();
        }
        Log.d(TAG, "deleteMessages, delete all parts end");

        /// M: delete words
        deleteWordsBySelection(db, selection, selectionArgs);

        /// M: To avoid blocking other database operation, set a limit to each delete operation. @{
        int count = 0;
        if (pduCount > 100) {
            String pduSelection = "";
            if (TextUtils.isEmpty(selection)) {
                pduSelection = "_id in (select _id from pdu limit " + DELETE_LIMIT + ")";
            } else {
                pduSelection = "_id in (select _id from pdu where " + selection + " limit "
                        + DELETE_LIMIT + ")";
            }
            int deleteCount = DELETE_LIMIT;
            while (deleteCount > 0) {
                deleteCount = db.delete(TABLE_PDU, pduSelection, selectionArgs);
                count += deleteCount;
                Log.d(TAG, "deleteMessages, delete " + deleteCount + " pdu");
            }
        } else {
            count = db.delete(TABLE_PDU, selection, selectionArgs);
        }
        Log.d(TAG, "deleteMessages, delete pdu end");

        if (count > 0) {
            Intent intent = new Intent(Mms.Intents.CONTENT_CHANGED_ACTION);
            intent.putExtra(Mms.Intents.DELETED_CONTENTS, uri);
            if (LOCAL_LOGV) {
                Log.v(TAG, "Broadcasting intent: " + intent);
            }
            context.sendBroadcast(intent);
            /// M: update threads. @{
            if (isUpdateThread) {
                if (threadIds.size() <= 2) {
                    for (long threadId : threadIds) {
                        MmsSmsDatabaseHelper.updateThread(db, threadId);
                    }
                } else {
                    int i = 0;
                    long[] updateThreads = new long[threadIds.size()];
                    for (long threadId : threadIds) {
                        updateThreads[i++] = threadId;
                    }
                    MmsSmsDatabaseHelper.updateMultiThreads(db, updateThreads);
                }
            }
            /// @}
        }
        return count;
    }

    private static int deleteParts(SQLiteDatabase db, String selection,
            String[] selectionArgs) {
        return deleteDataRows(db, TABLE_PART, selection, selectionArgs);
    }

    private static int deleteTempDrmData(SQLiteDatabase db, String selection,
            String[] selectionArgs) {
        return deleteDataRows(db, TABLE_DRM, selection, selectionArgs);
    }

    private static int deleteDataRows(SQLiteDatabase db, String table,
            String selection, String[] selectionArgs) {
        Cursor cursor = db.query(table, new String[] { "_data" },
                selection, selectionArgs, null, null, null);
        if (cursor == null) {
            // FIXME: This might be an error, ignore it may cause
            // unpredictable result.
            return 0;
        }

        try {
            if (cursor.getCount() == 0) {
                return 0;
            }

            while (cursor.moveToNext()) {
                try {
                    // Delete the associated files saved on file-system.
                    String path = cursor.getString(0);
                    if (path != null) {
                        new File(path).delete();
                    }
                } catch (Throwable ex) {
                    Log.e(TAG, ex.getMessage(), ex);
                }
            }
        } finally {
            cursor.close();
        }

        return db.delete(table, selection, selectionArgs);
    }

    @Override
    public int update(Uri uri, ContentValues values, String selection, String[] selectionArgs) {
        // The _data column is filled internally in MmsProvider, so this check is just to avoid
        // it from being inadvertently set. This is not supposed to be a protection against
        // malicious attack, since sql injection could still be attempted to bypass the check. On
        // the other hand, the MmsProvider does verify that the _data column has an allowed value
        // before opening any uri/files.
        if (values != null && values.containsKey(Part._DATA)) {
            return 0;
        }
        final int callerUid = Binder.getCallingUid();
        final String callerPkg = getCallingPackage();
        int match = sURLMatcher.match(uri);
        if (LOCAL_LOGV) {
            Log.v(TAG, "Update uri=" + uri + ", match=" + match);
        } else {
            Log.d(TAG, "Update begin, match = " + match);
        }
        MmsSmsProvider.MmsProviderLog.dpi(TAG, "update begin, uri = " + uri +
                ", selection = " + selection);
        if (LOCAL_LOGV) {
            Log.v(TAG, "Update values=" + values);
        }
        boolean notify = false;
        String msgId = null;
        String table;
        /// M: fix bug ALPS00293687, update thread table while update messages.
        long oldThreadId = -1;

        switch (match) {
            case MMS_ALL_ID:
            case MMS_INBOX_ID:
            case MMS_SENT_ID:
            case MMS_DRAFTS_ID:
            case MMS_OUTBOX_ID:
                msgId = uri.getLastPathSegment();
            // fall-through
            case MMS_ALL:
            case MMS_INBOX:
            case MMS_SENT:
            case MMS_DRAFTS:
            case MMS_OUTBOX:
                notify = true;
                table = TABLE_PDU;
                break;

            case MMS_MSG_PART:
            case MMS_PART_ID:
                /// M: Code analyze 004, fix bug ALPS00249113, if query result is not updated
                /// completely, the result will show abnormal. adjust notify time while
                /// store messages. @{
                if (values.containsKey("need_notify")) {
                    notify = values.getAsBoolean("need_notify");
                }
                /// @}
                table = TABLE_PART;
                break;

            case MMS_PART_RESET_FILE_PERMISSION:
                String path = getContext().getDir(PARTS_DIR_NAME, 0).getPath() + '/' +
                        uri.getPathSegments().get(1);
                // Reset the file permission back to read for everyone but me.
                int result = FileUtils.setPermissions(path, 0644, -1, -1);
                if (LOCAL_LOGV) {
                    Log.d(TAG, "MmsProvider.update setPermissions result: " + result +
                            " for path: " + path);
                }
                return 0;

            default:
                Log.w(TAG, "Update operation for '" + uri + "' not implemented.");
                return 0;
        }
        /// M: Code analyze 004, fix bug ALPS00249113, if query result is not updated
        /// completely, the result will show abnormal. adjust notify time while
        /// store messages. @{
        if (values.containsKey("need_notify")) {
            values.remove("need_notify");
        }
        /// @}
        String extraSelection = null;
        ContentValues finalValues;
        if (table.equals(TABLE_PDU)) {
            // Filter keys that we don't support yet.
            filterUnsupportedKeys(values);
            if (ProviderUtil.shouldRemoveCreator(values, callerUid)) {
                // CREATOR should not be changed by non-SYSTEM/PHONE apps
                Log.w(TAG, callerPkg + " tries to update CREATOR");
                values.remove(Mms.CREATOR);
            }
            finalValues = new ContentValues(values);

            if (msgId != null) {
                extraSelection = Mms._ID + "=" + msgId;
                /// M: Code analyze 009, fix bug ALPS00293687, update thread table while update
                /// messages. @{
                SQLiteDatabase db = mOpenHelper.getWritableDatabase();
                Cursor cursor = db.query(table, new String[] {"thread_id", Mms._ID},
                        extraSelection, null, null, null, null);
                try {
                     if (cursor != null && cursor.getCount() > 0) {
                         if (cursor.moveToFirst()) {
                             oldThreadId = cursor.getLong(0);
                             /// M: update subject in words table. @{
                             int rowId = cursor.getInt(cursor.getColumnIndex(Mms._ID));
                             if (finalValues.containsKey(Mms.SUBJECT)) {
                                 ContentValues cv = new ContentValues();
                                 cv.put(Telephony.MmsSms.WordsTable.ID,
                                         ROW_ID_BEGIN_SUBJECT + rowId);

                                 String sub = finalValues.getAsString(Mms.SUBJECT);
                                 if (finalValues.containsKey(Mms.SUBJECT_CHARSET)
                                         && finalValues.getAsInteger(Mms.SUBJECT_CHARSET) != null) {
                                     int charset = finalValues.getAsInteger(Mms.SUBJECT_CHARSET);
                                     sub = transformSubjectByCharset(sub, charset);
                                 }

                                 cv.put(Telephony.MmsSms.WordsTable.INDEXED_TEXT, sub);
                                 cv.put(Telephony.MmsSms.WordsTable.SOURCE_ROW_ID, rowId);
                                 cv.put(Telephony.MmsSms.WordsTable.TABLE_ID, TABLE_TO_USE_SUBJECT);
                                 String selectionSubject = Telephony.MmsSms.WordsTable.ID + " = "
                                         + (ROW_ID_BEGIN_SUBJECT + rowId) + " and "
                                         + Telephony.MmsSms.WordsTable.TABLE_ID + " = "
                                         + TABLE_TO_USE_SUBJECT;
                                 db.update(TABLE_WORDS, cv, selectionSubject, null);
                             }
                             /// @}
                         }
                     }
                } finally {
                    cursor.close();
                }
                /// @}
            }
        } else if (table.equals(TABLE_PART)) {
            finalValues = new ContentValues(values);

            switch (match) {
                case MMS_MSG_PART:
                    extraSelection = Part.MSG_ID + "=" + uri.getPathSegments().get(0);
                    break;
                case MMS_PART_ID:
                    extraSelection = Part._ID + "=" + uri.getPathSegments().get(1);
                    break;
                default:
                    break;
            }
        } else {
            return 0;
        }

        String finalSelection = concatSelections(selection, extraSelection);
        SQLiteDatabase db = mOpenHelper.getWritableDatabase();
        int count = db.update(table, finalValues, finalSelection, selectionArgs);
        if (notify && (count > 0)) {
            notifyUnread = false;
            notifyChange(uri, null);
        }
        /// M: Code analyze 009, fix bug ALPS00293687, update thread table while update messages. @{
        if (count > 0) {
            if (table.equals(TABLE_PDU)) {
                if (finalValues.containsKey(Mms.THREAD_ID)) {
                    long newThreadId = finalValues.getAsLong(Mms.THREAD_ID);
                    if (newThreadId != oldThreadId) {
                        MmsSmsDatabaseHelper.updateThread(db, oldThreadId);
                    }
                }
            }
        }
        /// @}
        Log.d(TAG, "update end, affectedRows = " + count);
        return count;
    }

    @Override
    public ParcelFileDescriptor openFile(Uri uri, String mode) throws FileNotFoundException {
        int match = sURLMatcher.match(uri);

        if (Log.isLoggable(TAG, Log.VERBOSE)) {
            MmsSmsProvider.MmsProviderLog.dpi(TAG, "openFile: uri=" + uri
                    + ", mode=" + mode + ", match=" + match);
        }

        if (match != MMS_PART_ID) {
            Log.v(TAG, "openFile " + "openFile return null");
            return null;
        }

        return safeOpenFileHelper(uri, mode);
    }

    @NonNull
    private ParcelFileDescriptor safeOpenFileHelper(
            @NonNull Uri uri, @NonNull String mode) throws FileNotFoundException {
        Cursor c = query(uri, new String[]{"_data"}, null, null, null);
        int count = (c != null) ? c.getCount() : 0;
        if (count != 1) {
            // If there is not exactly one result, throw an appropriate
            // exception.
            if (c != null) {
                c.close();
            }
            if (count == 0) {
                Log.e(TAG, "openfile FileNotFoundException(No entry for)");
                throw new FileNotFoundException("No entry for " + uri);
            }
            Log.e(TAG, "openfile FileNotFoundException(Multiple items at)");
            throw new FileNotFoundException("Multiple items at " + uri);
        }

        c.moveToFirst();
        int i = c.getColumnIndex("_data");
        String path = (i >= 0 ? c.getString(i) : null);
        c.close();

        if (path == null) {
            /// M: test Google patch JE Issue
            MmsSmsProvider.MmsProviderLog.vpi(TAG, "openfile path == null " + path);
            //throw new FileNotFoundException("Column _data not found.");
        }

        File filePath = new File(path);
        try {
            // The MmsProvider shouldn't open a file that isn't MMS data, so we verify that the
            // _data path actually points to MMS data. That safeguards ourselves from callers who
            // inserted or updated a URI (more specifically the _data column) with disallowed paths.
            // TODO(afurtado): provide a more robust mechanism to avoid disallowed _data paths to
            // be inserted/updated in the first place, including via SQL injection.
            if (!filePath.getCanonicalPath()
                    .startsWith(getContext().getDir(PARTS_DIR_NAME, 0).getCanonicalPath())) {
                MmsSmsProvider.MmsProviderLog.epi(TAG, "openFile: path "
                        + filePath.getCanonicalPath()
                        + " does not start with "
                        + getContext().getDir(PARTS_DIR_NAME, 0).getCanonicalPath());
                // Don't care return value
                Log.v(TAG, "openfile !filePath.getCanonicalPath().startsWith()");
                //return null;
            }
        } catch (IOException e) {
            Log.e(TAG, "openFile: create path failed " + e, e);
            return null;
        }

        int modeBits = ParcelFileDescriptor.parseMode(mode);
        return ParcelFileDescriptor.open(filePath, modeBits);
    }

    private void filterUnsupportedKeys(ContentValues values) {
        // Some columns are unsupported.  They should therefore
        // neither be inserted nor updated.  Filter them out.
        values.remove(Mms.DELIVERY_TIME_TOKEN);
        values.remove(Mms.SENDER_VISIBILITY);
        values.remove(Mms.REPLY_CHARGING);
        values.remove(Mms.REPLY_CHARGING_DEADLINE_TOKEN);
        values.remove(Mms.REPLY_CHARGING_DEADLINE);
        values.remove(Mms.REPLY_CHARGING_ID);
        values.remove(Mms.REPLY_CHARGING_SIZE);
        values.remove(Mms.PREVIOUSLY_SENT_BY);
        values.remove(Mms.PREVIOUSLY_SENT_DATE);
        values.remove(Mms.STORE);
        values.remove(Mms.MM_STATE);
        values.remove(Mms.MM_FLAGS_TOKEN);
        values.remove(Mms.MM_FLAGS);
        values.remove(Mms.STORE_STATUS);
        values.remove(Mms.STORE_STATUS_TEXT);
        values.remove(Mms.STORED);
        values.remove(Mms.TOTALS);
        values.remove(Mms.MBOX_TOTALS);
        values.remove(Mms.MBOX_TOTALS_TOKEN);
        values.remove(Mms.QUOTAS);
        values.remove(Mms.MBOX_QUOTAS);
        values.remove(Mms.MBOX_QUOTAS_TOKEN);
        values.remove(Mms.MESSAGE_COUNT);
        values.remove(Mms.START);
        values.remove(Mms.DISTRIBUTION_INDICATOR);
        values.remove(Mms.ELEMENT_DESCRIPTOR);
        values.remove(Mms.LIMIT);
        values.remove(Mms.RECOMMENDED_RETRIEVAL_MODE);
        values.remove(Mms.RECOMMENDED_RETRIEVAL_MODE_TEXT);
        values.remove(Mms.STATUS_TEXT);
        values.remove(Mms.APPLIC_ID);
        values.remove(Mms.REPLY_APPLIC_ID);
        values.remove(Mms.AUX_APPLIC_ID);
        values.remove(Mms.DRM_CONTENT);
        values.remove(Mms.ADAPTATION_ALLOWED);
        values.remove(Mms.REPLACE_ID);
        values.remove(Mms.CANCEL_ID);
        values.remove(Mms.CANCEL_STATUS);

        // Keys shouldn't be inserted or updated.
        values.remove(Mms._ID);
    }

    private void notifyChange(final Uri uri, final Uri caseSpecificUri) {
        final Context context = getContext();
        if (caseSpecificUri != null) {
            context.getContentResolver().notifyChange(
                caseSpecificUri, null, true, UserHandle.USER_ALL);
        }
        context.getContentResolver().notifyChange(
                MmsSms.CONTENT_URI, null, true, UserHandle.USER_ALL);
        context.getContentResolver().notifyChange(uri, null);

        /// M: Code analyze 002, fix bug ALPS00262044, not show out unread message
        /// icon after restore messages. notify mms application about unread messages
        /// number after insert operation.
        if (!notifyUnread) {
            Log.d(TAG, "notifyChange, notify unread change");
            MmsSmsProvider.notifyUnreadMessageNumberChanged(getContext());
        }
        ProviderUtil.notifyIfNotDefaultSmsApp(caseSpecificUri == null ? uri : caseSpecificUri,
                getCallingPackage(), context);
    }

    private final static String TAG = "Mms/Provider/Mms";
    private final static String VND_ANDROID_MMS = "vnd.android/mms";
    private final static String VND_ANDROID_DIR_MMS = "vnd.android-dir/mms";
    private final static boolean DEBUG = false;
    private final static boolean LOCAL_LOGV = false;

    private static final int MMS_ALL                      = 0;
    private static final int MMS_ALL_ID                   = 1;
    private static final int MMS_INBOX                    = 2;
    private static final int MMS_INBOX_ID                 = 3;
    private static final int MMS_SENT                     = 4;
    private static final int MMS_SENT_ID                  = 5;
    private static final int MMS_DRAFTS                   = 6;
    private static final int MMS_DRAFTS_ID                = 7;
    private static final int MMS_OUTBOX                   = 8;
    private static final int MMS_OUTBOX_ID                = 9;
    private static final int MMS_ALL_PART                 = 10;
    private static final int MMS_MSG_PART                 = 11;
    private static final int MMS_PART_ID                  = 12;
    private static final int MMS_MSG_ADDR                 = 13;
    private static final int MMS_SENDING_RATE             = 14;
    private static final int MMS_REPORT_STATUS            = 15;
    private static final int MMS_REPORT_REQUEST           = 16;
    private static final int MMS_DRM_STORAGE              = 17;
    private static final int MMS_DRM_STORAGE_ID           = 18;
    private static final int MMS_THREADS                  = 19;
    private static final int MMS_PART_RESET_FILE_PERMISSION = 20;
    /// M: Code analyze 005, fix bug ALPS00275452, count the attachments size.
    private static final int MMS_ATTACHMENT_SIZE          = 21;
    private static final UriMatcher
            sURLMatcher = new UriMatcher(UriMatcher.NO_MATCH);

    static {
        sURLMatcher.addURI("mms", null,         MMS_ALL);
        sURLMatcher.addURI("mms", "#",          MMS_ALL_ID);
        sURLMatcher.addURI("mms", "inbox",      MMS_INBOX);
        sURLMatcher.addURI("mms", "inbox/#",    MMS_INBOX_ID);
        sURLMatcher.addURI("mms", "sent",       MMS_SENT);
        sURLMatcher.addURI("mms", "sent/#",     MMS_SENT_ID);
        sURLMatcher.addURI("mms", "drafts",     MMS_DRAFTS);
        sURLMatcher.addURI("mms", "drafts/#",   MMS_DRAFTS_ID);
        sURLMatcher.addURI("mms", "outbox",     MMS_OUTBOX);
        sURLMatcher.addURI("mms", "outbox/#",   MMS_OUTBOX_ID);
        sURLMatcher.addURI("mms", "part",       MMS_ALL_PART);
        sURLMatcher.addURI("mms", "#/part",     MMS_MSG_PART);
        sURLMatcher.addURI("mms", "part/#",     MMS_PART_ID);
        sURLMatcher.addURI("mms", "#/addr",     MMS_MSG_ADDR);
        sURLMatcher.addURI("mms", "rate",       MMS_SENDING_RATE);
        sURLMatcher.addURI("mms", "report-status/#",  MMS_REPORT_STATUS);
        sURLMatcher.addURI("mms", "report-request/#", MMS_REPORT_REQUEST);
        sURLMatcher.addURI("mms", "drm",        MMS_DRM_STORAGE);
        sURLMatcher.addURI("mms", "drm/#",      MMS_DRM_STORAGE_ID);
        sURLMatcher.addURI("mms", "threads",    MMS_THREADS);
        /// M: Code analyze 005, fix bug ALPS00275452, count the attachments size.
        sURLMatcher.addURI("mms", "attachment_size", MMS_ATTACHMENT_SIZE);
        sURLMatcher.addURI("mms", "resetFilePerm/*",    MMS_PART_RESET_FILE_PERMISSION);
    }

    private SQLiteOpenHelper mOpenHelper;

    private static String concatSelections(String selection1, String selection2) {
        if (TextUtils.isEmpty(selection1)) {
            return selection2;
        } else if (TextUtils.isEmpty(selection2)) {
            return selection1;
        } else {
            return selection1 + " AND " + selection2;
        }
    }

    /// M: Code analyze 005, fix bug ALPS00275452, count the attachments size. @{
    /// M: get the attachment size
    private long getAttachmentsSize() {
        String[] projs = new String[] {
            Mms.Part._DATA
        };
        // TODO: is there a predefined Uri like Mms.CONTENT_URI?
        final Uri part = Uri.parse("content://mms/part/");
        long token = Binder.clearCallingIdentity();
        Cursor cursor = null;
        try {
            cursor = getContext().getContentResolver().query(part, projs, null, null, null);
        } finally {
            Binder.restoreCallingIdentity(token);
        }
        long size = 0;
        try {
            if (cursor == null || !cursor.moveToFirst()) {
                Log.e(TAG, "getAttachmentsSize, cursor is empty or null");
                return size;
            }
            Log.d(TAG, "getAttachmentsSize, count " + cursor.getCount());
            do {
                final String data = cursor.getString(0);
                if (data != null) {
                    File file = new File(data);
                    if (file.exists()) {
                        size += file.length();
                    }
                }
            } while (cursor.moveToNext());
        } finally {
            Log.d(TAG, "getAttachmentsSize size = " + size);
            if (cursor != null) {
                cursor.close();
            }
        }
        return size;
    }
    /// @}

    /// M: for delete all messages, improve performance. @{
    private static String formatInClause(Set<Long> ids) {
        /* to IN sql */
        if (ids == null || ids.size() == 0) {
            return " IN ()";
        }
        String in = " IN ";
        in += ids.toString();
        in = in.replace('[', '(');
        in = in.replace(']', ')');
        return in;
    }
    // @}

    /// M: Code analyze 006, fix bug ALPS00231431, thread should not be delete when
    /// receive mms notification, update status of threads table. @{
    private void setThreadStatus(SQLiteDatabase db, ContentValues values, int value) {
        ContentValues statusContentValues = new ContentValues(1);
        statusContentValues.put(MtkTelephony.MtkThreads.STATUS, value);
        db.update("threads", statusContentValues, "_id=" + values.getAsLong(Mms.THREAD_ID), null);
    }
    /// @}

    /*
     * M: Subject from pdu is messy code. It should be transformed to
     * right charset to save in words table.
     */
    private String transformSubjectByCharset(String sub, int charset) {
        String result = null;
        if (sub == null || sub.equals("")) {
            return "";
        }
        try {
            String charsetName = CharacterSets.getMimeName(charset);
            result = new String(sub.getBytes("iso-8859-1"), charsetName);
        } catch (UnsupportedEncodingException e) {
            Log.e(TAG, "transformSubjectByCharset UnsupportedEncodingException");
            return sub;
        }
        return result;
    }

    /*
     * M: Delete words about subject from pdu table by selection.
     */
    private static int deleteWordsBySelection(SQLiteDatabase db,
            String selection, String[] selectionArgs) {
        String where = "";
        if (selection != null && !selection.equals("")) {
            where = " where " + selection;
        }
        String subjectSelection = Telephony.MmsSms.WordsTable.SOURCE_ROW_ID
                + " in (select _id from pdu" + where + ")" + " and "
                + Telephony.MmsSms.WordsTable.TABLE_ID
                + " = " + TABLE_TO_USE_SUBJECT;
        int subcount = db.delete(TABLE_WORDS, subjectSelection, selectionArgs);
        return subcount;
    }

    /** M: MTK Add */
    /**
     * Given the recipients list and subject of an unsaved message, return
     * its thread ID. It's used for internal.
     */
    public static long getOrCreateThreadIdInternal(Context context, String recipient) {
        Uri.Builder uriBuilder = Uri.parse("content://mms-sms/threadID").buildUpon();

        if (Telephony.Mms.isEmailAddress(recipient)) {
            recipient = Telephony.Mms.extractAddrSpec(recipient);
        }
        uriBuilder.appendQueryParameter("recipient", recipient);
        Uri uri = uriBuilder.build();

        Cursor cursor = SqliteWrapper.query(context, context.getContentResolver(), uri,
                new String[] {"_id"}, null, null, null);
        if (cursor != null) {
            try {
                if (cursor.moveToFirst()) {
                    return cursor.getLong(0);
                } else {
                    Log.e(TAG, "getOrCreateThreadId returned no rows!");
                }
            } finally {
                cursor.close();
            }
        }

        MmsSmsProvider.MmsProviderLog.epi(TAG, "getOrCreateThreadId failed with uri " + uri.toString());
        throw new IllegalArgumentException("Unable to find or allocate a thread ID.");
    }
}
