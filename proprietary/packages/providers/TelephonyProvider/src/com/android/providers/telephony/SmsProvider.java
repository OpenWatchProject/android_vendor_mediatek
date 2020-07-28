/*
 * Copyright (C) 2006 The Android Open Source Project
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
import android.content.ContentProviderOperation;
import android.content.ContentProviderResult;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.OperationApplicationException;
import android.content.UriMatcher;
import android.database.Cursor;
import android.database.DatabaseUtils;
import android.database.MatrixCursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.database.sqlite.SQLiteQueryBuilder;
import android.net.Uri;
import android.os.Binder;
import android.os.UserHandle;
import android.provider.Contacts;
import android.os.SystemProperties;
import android.provider.Telephony;
import android.provider.Telephony.Mms;
import android.provider.Telephony.MmsSms;
import android.provider.Telephony.Sms;
import android.provider.Telephony.TextBasedSmsColumns;
import android.provider.Telephony.CanonicalAddressesColumns;
import android.provider.Telephony.ThreadsColumns;
import android.provider.Telephony.Threads;
import android.telephony.PhoneNumberUtils;
import android.telephony.SmsManager;
import android.telephony.SmsMessage;
import android.telephony.SubscriptionManager;
import android.text.TextUtils;
import android.util.Log;

import com.android.internal.annotations.VisibleForTesting;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.SmsHeader;
import com.google.android.mms.pdu.PduHeaders;
import mediatek.telephony.MtkTelephony;
import mediatek.telephony.MtkTelephony.MtkThreads;
import mediatek.telephony.MtkSmsManager;
import mediatek.telephony.MtkSmsMessage;

import java.nio.CharBuffer;
import java.util.ArrayList;
import java.util.HashMap;

public class SmsProvider extends ContentProvider {
    private static final Uri NOTIFICATION_URI = Uri.parse("content://sms");
    private static final Uri ICC_URI = Uri.parse("content://sms/icc");
    static final String TABLE_SMS = "sms";
    static final String TABLE_RAW = "raw";
    private static final String TABLE_SR_PENDING = "sr_pending";
    private static final String TABLE_WORDS = "words";
    static final String VIEW_SMS_RESTRICTED = "sms_restricted";

    private static final Integer ONE = Integer.valueOf(1);

    private static final String[] CONTACT_QUERY_PROJECTION =
            new String[] { Contacts.Phones.PERSON_ID };
    private static final int PERSON_ID_COLUMN = 0;

    /// M: Code analyze 005, fix bug ALPS00245352, it cost long time to restore messages.
    /// remove useless operation and add transaction while import sms. @{
    private static final int NORMAL_NUMBER_MAX_LENGTH = 15;
    private static final String[] CANONICAL_ADDRESSES_COLUMNS_2 =
            new String[] { CanonicalAddressesColumns._ID,
                           CanonicalAddressesColumns.ADDRESS };
    /// @}

    /// M: Code analyze 002, fix bug ALPS00046358, improve multi-delete speed by use batch
    /// processing. reference from page http://www.erpgear.com/show.php?contentid=1111.
    private static final String FOR_MULTIDELETE = "ForMultiDelete";

    /// M: Code analyze 006, fix bug ALPS00252799, it cost long time to restore messages.
    /// support batch processing while restore messages. @{
    /**
     * Maximum number of operations allowed in a batch
     */
    private static final int MAX_OPERATIONS_PER_PATCH = 50;
    /// @}

    /** Delete any raw messages or message segments marked deleted that are older than an hour. */
    static final long RAW_MESSAGE_EXPIRE_AGE_MS = (long) (60 * 60 * 1000);

    /**
     * These are the columns that are available when reading SMS
     * messages from the ICC.  Columns whose names begin with "is_"
     * have either "true" or "false" as their values.
     */
    private final static String[] ICC_COLUMNS = new String[] {
        // N.B.: These columns must appear in the same order as the
        // calls to add appear in convertIccToSms.
        "service_center_address",       // getServiceCenterAddress
        "address",                      // getDisplayOriginatingAddress
        "message_class",                // getMessageClass
        "body",                         // getDisplayMessageBody
        "date",                         // getTimestampMillis
        "status",                       // getStatusOnIcc
        "index_on_icc",                 // getIndexOnIcc
        "is_status_report",             // isStatusReportMessage
        "transport_type",               // Always "sms".
        "type",                         // Always MESSAGE_TYPE_ALL.
        "locked",                       // Always 0 (false).
        "error_code",                   // Always 0
        "_id",
        /// M: Code analyze 007, fix bug ALPS00042403, should show the sender's number
        /// in manage SIM card. show concatenation sms in one bubble, set incoming sms
        /// on left and sent sms on right, display sender information for every sms.
        "sub_id"                        // sim id
    };

    @Override
    public boolean onCreate() {
        setAppOps(AppOpsManager.OP_READ_SMS, AppOpsManager.OP_WRITE_SMS);
        // So we have two database files. One in de, one in ce. Here only "raw" table is in
        // mDeOpenHelper, other tables are all in mCeOpenHelper.
        mDeOpenHelper = MmsSmsDatabaseHelper.getInstanceForDe(getContext());
        mCeOpenHelper = MmsSmsDatabaseHelper.getInstanceForCe(getContext());
        TelephonyBackupAgent.DeferredSmsMmsRestoreService.startIfFilesExist(getContext());
        return true;
    }

    /**
     * Return the proper view of "sms" table for the current access status.
     *
     * @param accessRestricted If the access is restricted
     * @return the table/view name of the "sms" data
     */
    public static String getSmsTable(boolean accessRestricted) {
        return accessRestricted ? VIEW_SMS_RESTRICTED : TABLE_SMS;
    }

    @Override
    public Cursor query(Uri url, String[] projectionIn, String selection,
            String[] selectionArgs, String sort) {
        // First check if a restricted view of the "sms" table should be used based on the
        // caller's identity. Only system, phone or the default sms app can have full access
        // of sms data. For other apps, we present a restricted view which only contains sent
        // or received messages.
        final boolean accessRestricted = ProviderUtil.isAccessRestricted(
                getContext(), getCallingPackage(), Binder.getCallingUid());
        final String smsTable = getSmsTable(accessRestricted);
        MmsSmsProvider.MmsProviderLog.dpi(TAG, "query begin, url = " + url +
                ", selection = " + selection);
        SQLiteQueryBuilder qb = new SQLiteQueryBuilder();

        // Generate the body of the query.
        int match = sURLMatcher.match(url);
        Log.d(TAG, "query begin, match = " + match);

        SQLiteDatabase db = getReadableDatabase(match);
        switch (match) {
            case SMS_ALL:
                constructQueryForBox(qb, Sms.MESSAGE_TYPE_ALL, smsTable);
                break;

            case SMS_UNDELIVERED:
                constructQueryForUndelivered(qb, smsTable);
                break;

            case SMS_FAILED:
                constructQueryForBox(qb, Sms.MESSAGE_TYPE_FAILED, smsTable);
                break;

            case SMS_QUEUED:
                constructQueryForBox(qb, Sms.MESSAGE_TYPE_QUEUED, smsTable);
                break;

            case SMS_INBOX:
                constructQueryForBox(qb, Sms.MESSAGE_TYPE_INBOX, smsTable);
                break;

            case SMS_SENT:
                constructQueryForBox(qb, Sms.MESSAGE_TYPE_SENT, smsTable);
                break;

            case SMS_DRAFT:
                constructQueryForBox(qb, Sms.MESSAGE_TYPE_DRAFT, smsTable);
                break;

            case SMS_OUTBOX:
                constructQueryForBox(qb, Sms.MESSAGE_TYPE_OUTBOX, smsTable);
                break;

            case SMS_ALL_ID:
                qb.setTables(smsTable);
                qb.appendWhere("(_id = " + url.getPathSegments().get(0) + ")");
                break;

            case SMS_INBOX_ID:
            case SMS_FAILED_ID:
            case SMS_SENT_ID:
            case SMS_DRAFT_ID:
            case SMS_OUTBOX_ID:
                qb.setTables(smsTable);
                qb.appendWhere("(_id = " + url.getPathSegments().get(1) + ")");
                break;

            case SMS_CONVERSATIONS_ID:
                int threadID;

                try {
                    threadID = Integer.parseInt(url.getPathSegments().get(1));
                    if (Log.isLoggable(TAG, Log.VERBOSE)) {
                        Log.d(TAG, "query conversations: threadID=" + threadID);
                    }
                }
                catch (Exception ex) {
                    Log.e(TAG,
                          "Bad conversation thread id: "
                          + url.getPathSegments().get(1));
                    return null;
                }

                qb.setTables(smsTable);
                qb.appendWhere("thread_id = " + threadID);
                break;

            case SMS_CONVERSATIONS:
                qb.setTables(smsTable + ", "
                        + "(SELECT thread_id AS group_thread_id, "
                        + "MAX(date) AS group_date, "
                        + "COUNT(*) AS msg_count "
                        + "FROM " + smsTable + " "
                        + "GROUP BY thread_id) AS groups");
                qb.appendWhere(smsTable + ".thread_id=groups.group_thread_id"
                        + " AND " + smsTable + ".date=groups.group_date");
                final HashMap<String, String> projectionMap = new HashMap<>();
                projectionMap.put(Sms.Conversations.SNIPPET,
                        smsTable + ".body AS snippet");
                projectionMap.put(Sms.Conversations.THREAD_ID,
                        smsTable + ".thread_id AS thread_id");
                projectionMap.put(Sms.Conversations.MESSAGE_COUNT,
                        "groups.msg_count AS msg_count");
                projectionMap.put("delta", null);
                qb.setProjectionMap(projectionMap);
                break;

            case SMS_RAW_MESSAGE:
                // before querying purge old entries with deleted = 1
                purgeDeletedMessagesInRawTable(db);
                qb.setTables("raw");
                break;

            case SMS_STATUS_PENDING:
                qb.setTables("sr_pending");
                break;

            case SMS_ATTACHMENT:
                qb.setTables("attachments");
                break;

            case SMS_ATTACHMENT_ID:
                qb.setTables("attachments");
                qb.appendWhere(
                        "(sms_id = " + url.getPathSegments().get(1) + ")");
                break;

            case SMS_QUERY_THREAD_ID:
                qb.setTables("canonical_addresses");
                if (projectionIn == null) {
                    projectionIn = sIDProjection;
                }
                break;

            case SMS_STATUS_ID:
                qb.setTables(smsTable);
                qb.appendWhere("(_id = " + url.getPathSegments().get(1) + ")");
                break;

            case SMS_ALL_ICC:
                return getAllMessagesFromIcc(url, getSubIdFromUri(url));

            case SMS_ICC:
                String messageIndexString = url.getPathSegments().get(1);

                return getSingleMessageFromIcc(messageIndexString, getSubIdFromUri(url));

            /// M: Code analyze 011, fix bug ALPS00282321, ANR while delete old messages.
            /// use new process of delete. @{
            case SMS_ALL_THREADID:
                /// M: return all the distinct threadid from sms table
                return getAllSmsThreadIds(selection, selectionArgs);

            /// M: New Feature The international card.
            case URI_THREAD_ID:
                String recipient = url.getQueryParameter("recipient");
                return getThreadIdWithoutInsert(recipient, db);

            default:
                MmsSmsProvider.MmsProviderLog.epi(TAG, "Invalid request: " + url);
                return null;
        }

        String orderBy = null;

        if (!TextUtils.isEmpty(sort)) {
            orderBy = sort;
        } else if (qb.getTables().equals(smsTable)) {
            orderBy = Sms.DEFAULT_SORT_ORDER;
        }

        Cursor ret = qb.query(db, projectionIn, selection, selectionArgs,
                              null, null, orderBy);

        // TODO: Since the URLs are a mess, always use content://sms
        ret.setNotificationUri(getContext().getContentResolver(),
                NOTIFICATION_URI);
        return ret;
    }

    private void purgeDeletedMessagesInRawTable(SQLiteDatabase db) {
        long oldTimestamp = System.currentTimeMillis() - RAW_MESSAGE_EXPIRE_AGE_MS;
        int num = db.delete(TABLE_RAW, "deleted = 1 AND date < " + oldTimestamp, null);
        if (Log.isLoggable(TAG, Log.VERBOSE)) {
            Log.d(TAG, "purgeDeletedMessagesInRawTable: num rows older than " + oldTimestamp +
                    " purged: " + num);
        }
    }

    private SQLiteOpenHelper getDBOpenHelper(int match) {
        // Raw table is stored on de database. Other tables are stored in ce database.
        if (match == SMS_RAW_MESSAGE || match == SMS_RAW_MESSAGE_PERMANENT_DELETE) {
            return mDeOpenHelper;
        }
        return mCeOpenHelper;
    }

    /// M: Code analyze 007, fix bug ALPS00042403, should show the sender's number
    /// in manage SIM card. show concatenation sms in one bubble, set incoming sms
    /// on left and sent sms on right, display sender information for every sms. @{
    private Object[] convertIccToSms(MtkSmsMessage message,
            ArrayList<String> concatSmsIndexAndBody, int id,
            int subId) {
        // N.B.: These calls must appear in the same order as the
        // columns appear in ICC_COLUMNS.
        Object[] row = new Object[14];
        row[0] = message.getServiceCenterAddress();

        // check message status and set address
        if ((message.getStatusOnIcc() == SmsManager.STATUS_ON_ICC_READ) ||
               (message.getStatusOnIcc() == SmsManager.STATUS_ON_ICC_UNREAD)) {
            row[1] = message.getDisplayOriginatingAddress();
        } else {
            row[1] = message.getDestinationAddress();
        }

        String concatSmsIndex = null;
        String concatSmsBody = null;
        if (null != concatSmsIndexAndBody) {
            concatSmsIndex = concatSmsIndexAndBody.get(0);
            concatSmsBody = concatSmsIndexAndBody.get(1);
        }

        row[2] = String.valueOf(message.getMessageClass());
        row[3] = concatSmsBody == null ? message.getDisplayMessageBody() : concatSmsBody;
        row[4] = message.getTimestampMillis();
        row[5] = message.getStatusOnIcc();
        row[6] = concatSmsIndex == null ? message.getIndexOnIcc() : concatSmsIndex;
        row[7] = message.isStatusReportMessage();
        row[8] = "sms";
        row[9] = TextBasedSmsColumns.MESSAGE_TYPE_ALL;
        row[10] = 0;      // locked
        row[11] = 0;      // error_code
        row[12] = id;
        row[13] = subId;
        return row;
    }

    /**
     * Return a Cursor containing just one message from the ICC.
     */
    private Cursor getSingleMessageFromIcc(String messageIndexString, int subId) {
        int messageIndex = -1;
        try {
            messageIndex = Integer.parseInt(messageIndexString);
        } catch (NumberFormatException exception) {
            throw new IllegalArgumentException("Bad SMS ICC ID: " + messageIndexString);
        }
        ArrayList<MtkSmsMessage> messages;
        final MtkSmsManager smsManager = MtkSmsManager.getSmsManagerForSubscriptionId(subId);
        // Use phone id to avoid AppOps uid mismatch in telephony
        long token = Binder.clearCallingIdentity();
        try {
            messages = smsManager.getAllMessagesFromIcc();
        } finally {
            Binder.restoreCallingIdentity(token);
        }
        if (messages == null || messages.isEmpty()) {
            Log.e(TAG, "getSingleMessageFromIcc messages is null");
            return null;
        }
        MtkSmsMessage message = messages.get(messageIndex);
        if (message == null) {
            throw new IllegalArgumentException(
                    "Message not retrieved. ID: " + messageIndexString);
        }
        MatrixCursor cursor = new MatrixCursor(ICC_COLUMNS, 1);
        cursor.addRow(convertIccToSms(message, 0, subId));
        return withIccNotificationUri(cursor);
    }

    /**
     * Return a Cursor listing all the messages stored on the ICC.
     */
    /// M: Code analyze 007, fix bug ALPS00042403, should show the sender's number
    /// in manage SIM card. show concatenation sms in one bubble, set incoming sms
    /// on left and sent sms on right, display sender information for every sms. @{
    private Cursor getAllMessagesFromIcc(Uri uri, int subId) {

        ArrayList<MtkSmsMessage> messages;

        // use phone app permissions to avoid UID mismatch in AppOpsManager.noteOp() call
        long token = Binder.clearCallingIdentity();
        try {
            messages = MtkSmsManager.getSmsManagerForSubscriptionId(subId).getAllMessagesFromIcc();
        } finally {
            Binder.restoreCallingIdentity(token);
        }

        /// M: Code analyze 012, unknown, check if "messages" is valid. @{
        if (messages == null || messages.isEmpty()) {
            Log.e(TAG, "getAllMessagesFromIcc messages is null");
            return null;
        }
        /// @}
        final int count = messages.size();
        MatrixCursor cursor = new MatrixCursor(ICC_COLUMNS, count);
        ArrayList<String> concatSmsIndexAndBody = null;
        /// M: Code analyze 009, use a flag "showInOne" indicate show long
        ///sms in one bubble or not.for OP01 @{
        boolean showInOne = "1".equals(uri.getQueryParameter("showInOne"));
        /// @}
        for (int i = 0; i < count; i++) {
            concatSmsIndexAndBody = null;
            MtkSmsMessage message = messages.get(i);
            if (message != null && !message.isStatusReportMessage()) {
                /// M: Code analyze 009, use a flag "showInOne" indicate show long
                ///sms in one bubble or not.for OP01 @{
                if (showInOne) {
                    SmsHeader smsHeader = message.getUserDataHeader();
                    if (null != smsHeader && null != smsHeader.concatRef) {
                        concatSmsIndexAndBody = getConcatSmsIndexAndBody(messages, i);
                    }
                }
                /// @}
                cursor.addRow(convertIccToSms(message, concatSmsIndexAndBody, i, subId));
            }
        }
        return withIccNotificationUri(cursor);
    }

    private Cursor withIccNotificationUri(Cursor cursor) {
        cursor.setNotificationUri(getContext().getContentResolver(), ICC_URI);
        return cursor;
    }

    private void constructQueryForBox(SQLiteQueryBuilder qb, int type, String smsTable) {
        qb.setTables(smsTable);

        if (type != Sms.MESSAGE_TYPE_ALL) {
            qb.appendWhere("type=" + type);
        }
    }

    private void constructQueryForUndelivered(SQLiteQueryBuilder qb, String smsTable) {
        qb.setTables(smsTable);

        qb.appendWhere("(type=" + Sms.MESSAGE_TYPE_OUTBOX +
                       " OR type=" + Sms.MESSAGE_TYPE_FAILED +
                       " OR type=" + Sms.MESSAGE_TYPE_QUEUED + ")");
    }

    @Override
    public String getType(Uri url) {
        switch (url.getPathSegments().size()) {
            case 0:
                return VND_ANDROID_DIR_SMS;
            case 1:
                try {
                    Integer.parseInt(url.getPathSegments().get(0));
                    return VND_ANDROID_SMS;
                } catch (NumberFormatException ex) {
                    return VND_ANDROID_DIR_SMS;
                }
            case 2:
                // TODO: What about "threadID"?
                if (url.getPathSegments().get(0).equals("conversations")) {
                    return VND_ANDROID_SMSCHAT;
                } else {
                    return VND_ANDROID_SMS;
                }
        }
        return null;
    }

    @Override
    public int bulkInsert(@NonNull Uri url, @NonNull ContentValues[] values) {
        final int callerUid = Binder.getCallingUid();
        final String callerPkg = getCallingPackage();
        long token = Binder.clearCallingIdentity();
        try {
            int messagesInserted = 0;
            for (ContentValues initialValues : values) {
                Uri insertUri = insertInner(url, initialValues, callerUid, callerPkg);
                if (insertUri != null) {
                    messagesInserted++;
                }
            }

            // The raw table is used by the telephony layer for storing an sms before
            // sending out a notification that an sms has arrived. We don't want to notify
            // the default sms app of changes to this table.
            final boolean notifyIfNotDefault = sURLMatcher.match(url) != SMS_RAW_MESSAGE;
            notifyChange(notifyIfNotDefault, url, callerPkg, false);
            return messagesInserted;
        } finally {
            Binder.restoreCallingIdentity(token);
        }
    }

    @Override
    public Uri insert(Uri url, ContentValues initialValues) {
        final int callerUid = Binder.getCallingUid();
        final String callerPkg = getCallingPackage();
        long token = Binder.clearCallingIdentity();
        try {
            Uri insertUri = insertInner(url, initialValues, callerUid, callerPkg);

            // The raw table is used by the telephony layer for storing an sms before
            // sending out a notification that an sms has arrived. We don't want to notify
            // the default sms app of changes to this table.
            final boolean notifyIfNotDefault = sURLMatcher.match(url) != SMS_RAW_MESSAGE;
            notifyChange(notifyIfNotDefault, insertUri, callerPkg, false);
            return insertUri;
        } finally {
            Binder.restoreCallingIdentity(token);
        }
    }

    private Uri insertInner(Uri url, ContentValues initialValues, int callerUid, String callerPkg) {
        ContentValues values;
        long rowID = 0;
        int type = Sms.MESSAGE_TYPE_ALL;

        int match = sURLMatcher.match(url);
        String table = TABLE_SMS;
        boolean notifyIfNotDefault = true;

        switch (match) {
            case SMS_ALL:
                Integer typeObj = initialValues.getAsInteger(Sms.TYPE);
                if (typeObj != null) {
                    type = typeObj.intValue();
                } else {
                    // default to inbox
                    type = Sms.MESSAGE_TYPE_INBOX;
                }
                break;

            case SMS_INBOX:
                type = Sms.MESSAGE_TYPE_INBOX;
                break;

            case SMS_FAILED:
                type = Sms.MESSAGE_TYPE_FAILED;
                break;

            case SMS_QUEUED:
                type = Sms.MESSAGE_TYPE_QUEUED;
                break;

            case SMS_SENT:
                type = Sms.MESSAGE_TYPE_SENT;
                break;

            case SMS_DRAFT:
                type = Sms.MESSAGE_TYPE_DRAFT;
                break;

            case SMS_OUTBOX:
                type = Sms.MESSAGE_TYPE_OUTBOX;
                break;

            case SMS_RAW_MESSAGE:
                table = "raw";
                // The raw table is used by the telephony layer for storing an sms before
                // sending out a notification that an sms has arrived. We don't want to notify
                // the default sms app of changes to this table.
                notifyIfNotDefault = false;
                break;

            case SMS_STATUS_PENDING:
                table = "sr_pending";
                break;

            case SMS_ATTACHMENT:
                table = "attachments";
                break;

            case SMS_NEW_THREAD_ID:
                table = "canonical_addresses";
                break;

            default:
                MmsSmsProvider.MmsProviderLog.epi(TAG, "Invalid request: " + url);
                return null;
        }

        SQLiteDatabase db = getWritableDatabase(match);

        if (table.equals(TABLE_SMS)) {
            boolean addDate = false;
            boolean addType = false;

            // Make sure that the date and type are set
            if (initialValues == null) {
                values = new ContentValues(1);
                addDate = true;
                addType = true;
            } else {
                values = new ContentValues(initialValues);

                if (!initialValues.containsKey(Sms.DATE)) {
                    addDate = true;
                }

                if (!initialValues.containsKey(Sms.TYPE)) {
                    addType = true;
                }
            }

            if (addDate) {
                values.put(Sms.DATE, new Long(System.currentTimeMillis()));
                /// M: Code analyze 014, fix bug ALPS00114870, messages' time were abnormal
                /// after restored. set the date as the right value when import.
            } else {
                Long date = values.getAsLong(Sms.DATE);
                values.put(Sms.DATE, date);
                /// @}
            }

            if (addType && (type != Sms.MESSAGE_TYPE_ALL)) {
                values.put(Sms.TYPE, Integer.valueOf(type));
            }

            // thread_id
            Long threadId = values.getAsLong(Sms.THREAD_ID);
            String address = values.getAsString(Sms.ADDRESS);

            if (((threadId == null) || (threadId == 0)) && (!TextUtils.isEmpty(address))) {
                long id = getThreadIdInternal(address, db, false);
                values.put(Sms.THREAD_ID, id);
            }

            // If this message is going in as a draft, it should replace any
            // other draft messages in the thread.  Just delete all draft
            // messages with this thread ID.  We could add an OR REPLACE to
            // the insert below, but we'd have to query to find the old _id
            // to produce a conflict anyway.
            if (values.getAsInteger(Sms.TYPE) == Sms.MESSAGE_TYPE_DRAFT) {
                db.delete(TABLE_SMS, "thread_id=? AND type=?",
                        new String[] { values.getAsString(Sms.THREAD_ID),
                                       Integer.toString(Sms.MESSAGE_TYPE_DRAFT) });
            }

            if (type == Sms.MESSAGE_TYPE_INBOX) {
                // Look up the person if not already filled in.
                if ((values.getAsLong(Sms.PERSON) == null) && (!TextUtils.isEmpty(address))) {
                    Cursor cursor = null;
                    Uri uri = Uri.withAppendedPath(Contacts.Phones.CONTENT_FILTER_URL,
                            Uri.encode(address));
                    try {
                        cursor = getContext().getContentResolver().query(
                                uri,
                                CONTACT_QUERY_PROJECTION,
                                null, null, null);

                        if (cursor.moveToFirst()) {
                            Long id = Long.valueOf(cursor.getLong(PERSON_ID_COLUMN));
                            values.put(Sms.PERSON, id);
                        }
                    } catch (Exception ex) {
                        Log.e(TAG, "insert: query contact uri " + uri + " caught ", ex);
                    } finally {
                        if (cursor != null) {
                            cursor.close();
                        }
                    }
                }
            } else {
                // Mark all non-inbox messages read.
                values.put(Sms.READ, ONE);
            }
            if (ProviderUtil.shouldSetCreator(values, callerUid)) {
                // Only SYSTEM or PHONE can set CREATOR
                // If caller is not SYSTEM or PHONE, or SYSTEM or PHONE does not set CREATOR
                // set CREATOR using the truth on caller.
                // Note: Inferring package name from UID may include unrelated package names
                values.put(Sms.CREATOR, callerPkg);
            }
        } else {
            if (initialValues == null) {
                values = new ContentValues(1);
            } else {
                values = initialValues;
            }
        }

        rowID = db.insert(table, "body", values);

        // Don't use a trigger for updating the words table because of a bug
        // in FTS3.  The bug is such that the call to get the last inserted
        // row is incorrect.
        if (table == TABLE_SMS) {
            // Update the words table with a corresponding row.  The words table
            // allows us to search for words quickly, without scanning the whole
            // table;
            ContentValues cv = new ContentValues();
            cv.put(Telephony.MmsSms.WordsTable.ID, rowID);
            cv.put(Telephony.MmsSms.WordsTable.INDEXED_TEXT, values.getAsString("body"));
            cv.put(Telephony.MmsSms.WordsTable.SOURCE_ROW_ID, rowID);
            cv.put(Telephony.MmsSms.WordsTable.TABLE_ID, 1);
            db.insert(TABLE_WORDS, Telephony.MmsSms.WordsTable.INDEXED_TEXT, cv);
        }
        if (rowID > 0) {
            Uri uri = Uri.withAppendedPath(url, String.valueOf(rowID));
            if (Log.isLoggable(TAG, Log.VERBOSE)) {
                Log.d(TAG, "insertInner " + uri + " succeeded");
            }
            //now notify the launcher to show unread message.
            notifyChange(false, uri, null, true);
            Log.d(TAG, "insertInner succeed" + uri);
            return uri;
        } else {
            Log.e(TAG, "insert: failed!");
        }

        return null;
    }

    @Override
    public int delete(Uri url, String where, String[] whereArgs) {
        MmsSmsProvider.MmsProviderLog.dpi(TAG, "delete begin, uri = " + url + ", selection = " + where);
        int count = 0;
        int match = sURLMatcher.match(url);
        SQLiteDatabase db = getWritableDatabase(match);
        boolean notifyIfNotDefault = true;
        Log.d(TAG, "delete begin, match = " + match);

        switch (match) {
            case SMS_ALL:
                if (where != null && where.equals(FOR_MULTIDELETE)) {
                    Log.d(TAG, "delete FOR_MULTIDELETE, length:" + whereArgs.length);
                    String selectids = getSmsIdsFromArgs(whereArgs);
                    String threadQuery = String.format("SELECT DISTINCT thread_id FROM sms " +
                            "WHERE _id IN %s", selectids);
                    Cursor cursor = db.rawQuery(threadQuery, null);
                    /// M: fix ALPS01263429, consider cursor as view, we should read cursor
                    /// before delete related records.
                    long[] deletedThreads = null;
                    try {
                        deletedThreads = new long[cursor.getCount()];
                        int i = 0;
                        while (cursor.moveToNext()) {
                            deletedThreads[i++] = cursor.getLong(0);
                        }
                    } finally {
                        cursor.close();
                    }
                    String finalSelection = String.format(" _id IN %s", selectids);
                    count = deleteMessages(db, finalSelection, null);
                    if (count != 0) {
                        MmsSmsDatabaseHelper.updateMultiThreads(db, deletedThreads);
                    }
                } else {
                    Cursor cursor = db.query(TABLE_SMS, new String[]{"distinct thread_id"},
                            where, whereArgs, null, null, null);
                    long[] deletedThreads = null;
                    try {
                        deletedThreads = new long[cursor.getCount()];
                        int i = 0;
                        while (cursor.moveToNext()) {
                            deletedThreads[i++] = cursor.getLong(0);
                        }
                    } finally {
                        cursor.close();
                    }

                    count = db.delete(TABLE_SMS, where, whereArgs);
                    if (count != 0) {
                        MmsSmsDatabaseHelper.updateMultiThreads(db, deletedThreads);
                    }
                }
                break;

            case SMS_ALL_ID:
                try {
                    int message_id = Integer.parseInt(url.getPathSegments().get(0));
                    count = MmsSmsDatabaseHelper.deleteOneSms(db, message_id);
                } catch (Exception e) {
                    throw new IllegalArgumentException(
                        "Bad message id: " + url.getPathSegments().get(0));
                }
                break;

            case SMS_CONVERSATIONS_ID:
                int threadID;

                try {
                    threadID = Integer.parseInt(url.getPathSegments().get(1));
                } catch (Exception ex) {
                    throw new IllegalArgumentException(
                            "Bad conversation thread id: "
                            + url.getPathSegments().get(1));
                }

                // delete the messages from the sms table
                where = DatabaseUtils.concatenateWhere("thread_id=" + threadID, where);
                count = db.delete(TABLE_SMS, where, whereArgs);
                MmsSmsDatabaseHelper.updateThread(db, threadID);
                break;

            /// M: Code analyze 011, fix bug ALPS00282321, ANR while delete old messages.
            /// use new process of delete. @{
            case SMS_AUTO_DELETE:
                try {
                    threadID = Integer.parseInt(url.getPathSegments().get(1));
                } catch (Exception ex) {
                    throw new IllegalArgumentException(
                            "Bad conversation thread id: "
                            + url.getPathSegments().get(1));
                }

                where = DatabaseUtils.concatenateWhere("thread_id=" + threadID, where);
                /// M: delete the messages from the sms table
                if (whereArgs != null) {
                    String selectids = getSmsIdsFromArgs(whereArgs);
                    where = "_id in " + selectids;
                    count = deleteMessages(db, where, null);
                } else {
                    count = db.delete(TABLE_SMS, where, null);
                }
                MmsSmsDatabaseHelper.updateThread(db, threadID);
                break;
            /// @}

            case SMS_RAW_MESSAGE:
                ContentValues cv = new ContentValues();
                cv.put("deleted", 1);
                count = db.update(TABLE_RAW, cv, where, whereArgs);
                if (Log.isLoggable(TAG, Log.VERBOSE)) {
                    Log.d(TAG, "delete: num rows marked deleted in raw table: " + count);
                }
                notifyIfNotDefault = false;
                break;

            case SMS_RAW_MESSAGE_PERMANENT_DELETE:
                count = db.delete(TABLE_RAW, where, whereArgs);
                if (Log.isLoggable(TAG, Log.VERBOSE)) {
                    Log.d(TAG, "delete: num rows permanently deleted in raw table: " + count);
                }
                notifyIfNotDefault = false;
                break;

            case SMS_STATUS_PENDING:
                count = db.delete("sr_pending", where, whereArgs);
                break;

            case SMS_ICC:
                String messageIndexString = url.getPathSegments().get(1);

                return deleteMessageFromIcc(messageIndexString, getSubIdFromUri(url));

            case SMS_ALL_ICC:
                int subId = getSubIdFromUri(url);
                Log.i(TAG, "Delete messages in subId = " + subId);
                if (where != null && where.equals(FOR_MULTIDELETE)) {
                    String message_id = "";
                    for (int i = 0; i < whereArgs.length; i++) {
                        if (whereArgs[i] != null) {
                            message_id = whereArgs[i];
                            Log.i(TAG, "Delete Sub" + (subId + 1) + " SMS id: " + message_id);
                            count += deleteMessageFromIcc(message_id, subId);
                        }
                    }
                } else {
                    count = deleteMessageFromIcc("-1", subId);
                }
                break;

            default:
                throw new IllegalArgumentException("Unknown URL");
        }

        if (count > 0) {
            notifyChange(notifyIfNotDefault, url, getCallingPackage(), false);
        }
        Log.d(TAG, "delete end, count = " + count);
        return count;
    }

    protected static String getSmsIdsFromArgs(String[] selectionArgs) {
        StringBuffer content = new StringBuffer("(");
        String res = "";
        if (selectionArgs == null || selectionArgs.length < 1) {
            return "()";
        }
        for (int i = 0; i < selectionArgs.length - 1; i++) {
            if (selectionArgs[i] == null) {
                break;
            }
            content.append(selectionArgs[i]);
            content.append(",");
        }
        if (selectionArgs[selectionArgs.length - 1] != null) {
           content.append(selectionArgs[selectionArgs.length - 1]);
        }
        res = content.toString();
        if (res.endsWith(",")) {
            res = res.substring(0, res.lastIndexOf(","));
        }
        res += ")";
        return res;
    }

    /**
     * Delete the message at index from ICC.  Return true iff
     * successful.
     */
    private int deleteMessageFromIcc(String messageIndexString, int subId) {
        SmsManager smsManager = SmsManager.getSmsManagerForSubscriptionId(subId);
        // Use phone id to avoid AppOps uid mismatch in telephony
        long token = Binder.clearCallingIdentity();
        try {
            return smsManager.deleteMessageFromIcc(
                    Integer.parseInt(messageIndexString))
                    ? 1 : 0;
        } catch (NumberFormatException exception) {
            throw new IllegalArgumentException(
                    "Bad SMS ICC ID: " + messageIndexString);
        } finally {
            ContentResolver cr = getContext().getContentResolver();
            //cr.notifyChange(ICC_URI, null, true, UserHandle.USER_ALL);
            cr.notifyChange(ICC_URI, null);

            Binder.restoreCallingIdentity(token);
        }
    }

    @Override
    public int update(Uri url, ContentValues values, String where, String[] whereArgs) {
        final int callerUid = Binder.getCallingUid();
        final String callerPkg = getCallingPackage();
        MmsSmsProvider.MmsProviderLog.dpi(TAG, "update begin, uri = " + url + ", values = " + values
                + ", selection = " + where);
        int count = 0;
        String table = TABLE_SMS;
        String extraWhere = null;
        boolean notifyIfNotDefault = true;
        int match = sURLMatcher.match(url);
        SQLiteDatabase db = getWritableDatabase(match);
        Log.d(TAG, "update begin, match = " + match);

        switch (match) {
            case SMS_RAW_MESSAGE:
                table = TABLE_RAW;
                notifyIfNotDefault = false;
                break;

            case SMS_STATUS_PENDING:
                table = TABLE_SR_PENDING;
                break;

            case SMS_ALL:
            case SMS_FAILED:
            case SMS_QUEUED:
            case SMS_INBOX:
            case SMS_SENT:
            case SMS_DRAFT:
            case SMS_OUTBOX:
            case SMS_CONVERSATIONS:
                break;

            case SMS_ALL_ID:
                extraWhere = "_id=" + url.getPathSegments().get(0);
                break;

            case SMS_INBOX_ID:
            case SMS_FAILED_ID:
            case SMS_SENT_ID:
            case SMS_DRAFT_ID:
            case SMS_OUTBOX_ID:
                extraWhere = "_id=" + url.getPathSegments().get(1);
                break;

            case SMS_CONVERSATIONS_ID: {
                String threadId = url.getPathSegments().get(1);

                try {
                    Integer.parseInt(threadId);
                } catch (Exception ex) {
                    Log.e(TAG, "Bad conversation thread id: " + threadId);
                    break;
                }

                extraWhere = "thread_id=" + threadId;
                break;
            }

            case SMS_STATUS_ID:
                extraWhere = "_id=" + url.getPathSegments().get(1);
                break;

            default:
                throw new UnsupportedOperationException(
                        "URI " + url + " not supported");
        }

        if (table.equals(TABLE_SMS) && ProviderUtil.shouldRemoveCreator(values, callerUid)) {
            // CREATOR should not be changed by non-SYSTEM/PHONE apps
            Log.w(TAG, callerPkg + " tries to update CREATOR");
            values.remove(Sms.CREATOR);
        }

        where = DatabaseUtils.concatenateWhere(where, extraWhere);
        count = db.update(table, values, where, whereArgs);

        if (count > 0) {
            if (Log.isLoggable(TAG, Log.VERBOSE)) {
                MmsSmsProvider.MmsProviderLog.dpi(TAG, "update " + url + " succeeded");
            }
            Boolean notify = values.containsKey(Sms.READ);
            notifyChange(notifyIfNotDefault, url, callerPkg, notify);
        }
        Log.d(TAG, "update end, affectedRows = " + count);
        return count;
    }

    private void notifyChange(boolean notifyIfNotDefault, Uri uri, final String callingPackage,
            boolean notify) {
        final Context context = getContext();
        ContentResolver cr = context.getContentResolver();
        cr.notifyChange(uri, null, true, UserHandle.USER_ALL);
        cr.notifyChange(MmsSms.CONTENT_URI, null, true, UserHandle.USER_ALL);
        cr.notifyChange(Uri.parse("content://mms-sms/conversations/"), null, true,
                UserHandle.USER_ALL);
        int urlMatcher =  sURLMatcher.match(uri);
        Log.d(TAG, "URLMatcher matches type of message" + urlMatcher);
        if (notify && urlMatcher != SMS_RAW_MESSAGE_ID
            && urlMatcher != SMS_RAW_MESSAGE
            && urlMatcher != -1) {
            Log.d(TAG, "notifyChange, notify unread change");
            MmsSmsProvider.notifyUnreadMessageNumberChanged(getContext());
        }
        if (notifyIfNotDefault) {
            ProviderUtil.notifyIfNotDefaultSmsApp(uri, callingPackage, context);
        }
    }

    // Db open helper for tables stored in CE(Credential Encrypted) storage.
    @VisibleForTesting
    public SQLiteOpenHelper mCeOpenHelper;
    // Db open helper for tables stored in DE(Device Encrypted) storage. It's currently only used
    // to store raw table.
    @VisibleForTesting
    public SQLiteOpenHelper mDeOpenHelper;

    private final static String TAG = "Mms/Provider/Sms";
    private final static String VND_ANDROID_SMS = "vnd.android.cursor.item/sms";
    private final static String VND_ANDROID_SMSCHAT =
            "vnd.android.cursor.item/sms-chat";
    private final static String VND_ANDROID_DIR_SMS =
            "vnd.android.cursor.dir/sms";

    private static final String[] sIDProjection = new String[] { "_id" };

    private static final int SMS_ALL = 0;
    private static final int SMS_ALL_ID = 1;
    private static final int SMS_INBOX = 2;
    private static final int SMS_INBOX_ID = 3;
    private static final int SMS_SENT = 4;
    private static final int SMS_SENT_ID = 5;
    private static final int SMS_DRAFT = 6;
    private static final int SMS_DRAFT_ID = 7;
    private static final int SMS_OUTBOX = 8;
    private static final int SMS_OUTBOX_ID = 9;
    private static final int SMS_CONVERSATIONS = 10;
    private static final int SMS_CONVERSATIONS_ID = 11;
    private static final int SMS_RAW_MESSAGE = 15;
    private static final int SMS_ATTACHMENT = 16;
    private static final int SMS_ATTACHMENT_ID = 17;
    private static final int SMS_NEW_THREAD_ID = 18;
    private static final int SMS_QUERY_THREAD_ID = 19;
    private static final int SMS_STATUS_ID = 20;
    private static final int SMS_STATUS_PENDING = 21;
    private static final int SMS_ALL_ICC = 22;
    private static final int SMS_ICC = 23;
    private static final int SMS_FAILED = 24;
    private static final int SMS_FAILED_ID = 25;
    private static final int SMS_QUEUED = 26;
    private static final int SMS_UNDELIVERED = 27;
    private static final int SMS_RAW_MESSAGE_PERMANENT_DELETE = 28;
    private static final int SMS_RAW_MESSAGE_ID = 29;
    /// M: fix bug ALPS00282321, ANR while delete old messages.
    /// use new process of delete. @{
    private static final int SMS_ALL_THREADID = 30;
    private static final int SMS_AUTO_DELETE  = 31;
    /// @}
    private static final int URI_THREAD_ID = 32;

    private static final UriMatcher sURLMatcher =
            new UriMatcher(UriMatcher.NO_MATCH);

    static {
        sURLMatcher.addURI("sms", null, SMS_ALL);
        sURLMatcher.addURI("sms", "#", SMS_ALL_ID);
        sURLMatcher.addURI("sms", "inbox", SMS_INBOX);
        sURLMatcher.addURI("sms", "inbox/#", SMS_INBOX_ID);
        sURLMatcher.addURI("sms", "sent", SMS_SENT);
        sURLMatcher.addURI("sms", "sent/#", SMS_SENT_ID);
        sURLMatcher.addURI("sms", "draft", SMS_DRAFT);
        sURLMatcher.addURI("sms", "draft/#", SMS_DRAFT_ID);
        sURLMatcher.addURI("sms", "outbox", SMS_OUTBOX);
        sURLMatcher.addURI("sms", "outbox/#", SMS_OUTBOX_ID);
        sURLMatcher.addURI("sms", "undelivered", SMS_UNDELIVERED);
        sURLMatcher.addURI("sms", "failed", SMS_FAILED);
        sURLMatcher.addURI("sms", "failed/#", SMS_FAILED_ID);
        sURLMatcher.addURI("sms", "queued", SMS_QUEUED);
        sURLMatcher.addURI("sms", "conversations", SMS_CONVERSATIONS);
        sURLMatcher.addURI("sms", "conversations/*", SMS_CONVERSATIONS_ID);
        sURLMatcher.addURI("sms", "raw", SMS_RAW_MESSAGE);
        sURLMatcher.addURI("sms", "raw/permanentDelete", SMS_RAW_MESSAGE_PERMANENT_DELETE);
        sURLMatcher.addURI("raw", "#", SMS_RAW_MESSAGE_ID);
        sURLMatcher.addURI("sms", "attachments", SMS_ATTACHMENT);
        sURLMatcher.addURI("sms", "attachments/#", SMS_ATTACHMENT_ID);
        sURLMatcher.addURI("sms", "threadID", SMS_NEW_THREAD_ID);
        sURLMatcher.addURI("sms", "threadID/*", SMS_QUERY_THREAD_ID);
        sURLMatcher.addURI("sms", "status/#", SMS_STATUS_ID);
        sURLMatcher.addURI("sms", "sr_pending", SMS_STATUS_PENDING);
        sURLMatcher.addURI("sms", "icc", SMS_ALL_ICC);
        sURLMatcher.addURI("sms", "icc/#", SMS_ICC);
        //we keep these for not breaking old applications
        sURLMatcher.addURI("sms", "sim", SMS_ALL_ICC);
        sURLMatcher.addURI("sms", "sim/#", SMS_ICC);
        /// M: fix bug ALPS00282321, ANR while delete old messages.
        /// use new process of delete. @{
        sURLMatcher.addURI("sms", "all_threadid", SMS_ALL_THREADID);
        sURLMatcher.addURI("sms", "auto_delete/#", SMS_AUTO_DELETE);
        /// @}
        sURLMatcher.addURI("sms", "thread_id", URI_THREAD_ID);
    }

    /// M: Code analyze 007, fix bug ALPS00042403, should show the sender's number
    /// in manage SIM card. show concatenation sms in one bubble, set incoming sms
    /// on left and sent sms on right, display sender information for every sms. @{
    private Object[] convertIccToSms(MtkSmsMessage message, int id, int subId) {
        return convertIccToSms(message, null, id, subId);
    }
    /// @}

    /// M: Code analyze 007, fix bug ALPS00042403, should show the sender's number
    /// in manage SIM card. show concatenation sms in one bubble, set incoming sms
    /// on left and sent sms on right, display sender information for every sms. for OP01 @{
    private ArrayList<String> getConcatSmsIndexAndBody(ArrayList<MtkSmsMessage> messages, int index) {
        int totalCount = messages.size();
        int refNumber = 0;
        int msgCount = 0;
        ArrayList<String> indexAndBody = new ArrayList<String>();
        StringBuilder smsIndex = new StringBuilder();
        StringBuilder smsBody = new StringBuilder();
        ArrayList<MtkSmsMessage> concatMsg = null;
        MtkSmsMessage message = messages.get(index);
        if (message != null) {
            SmsHeader smsHeader = message.getUserDataHeader();
            if (null != smsHeader && null != smsHeader.concatRef) {
                msgCount = smsHeader.concatRef.msgCount;
                refNumber = smsHeader.concatRef.refNumber;
            }
        }

        concatMsg = new ArrayList<MtkSmsMessage>();
        concatMsg.add(message);

        for (int i = index + 1; i < totalCount; i++) {
            MtkSmsMessage sms = messages.get(i);
            if (sms != null) {
                SmsHeader smsHeader = sms.getUserDataHeader();
                if (null != smsHeader && null != smsHeader.concatRef
                        && refNumber == smsHeader.concatRef.refNumber) {
                    concatMsg.add(sms);
                    messages.set(i, null);
                    if (msgCount == concatMsg.size()) {
                        break;
                    }
                }
            }
        }

        int concatCount = concatMsg.size();
        for (int k = 0; k < msgCount; k++) {
            for (int j = 0; j < concatCount; j++) {
                MtkSmsMessage sms = concatMsg.get(j);
                SmsHeader smsHeader = sms.getUserDataHeader();
                if (k == smsHeader.concatRef.seqNumber - 1) {
                    smsIndex.append(sms.getIndexOnIcc());
                    smsIndex.append(";");
                    smsBody.append(sms.getDisplayMessageBody());
                    break;
                }
            }
        }

        MmsSmsProvider.MmsProviderLog.dpi(TAG, "concatenation sms index:" + smsIndex.toString() +
                "concatenation sms body:" + smsBody.toString());
        indexAndBody.add(smsIndex.toString());
        indexAndBody.add(smsBody.toString());

        return indexAndBody;
    }
    /// @}

    /// M: Code analyze 006, fix bug ALPS00252799, it cost long time to restore messages.
    /// support batch processing while restore messages. @{
    @Override
    public ContentProviderResult[] applyBatch(ArrayList<ContentProviderOperation> operations)
            throws OperationApplicationException {
        int ypCount = 0;
        int opCount = 0;
        SQLiteDatabase db = getDBOpenHelper(0).getWritableDatabase();
        /// M: Fix ALPS00288517, not use transaction again to avoid ANR
        /// because of order of locking db
        db.beginTransaction();
        /// @}
        try {
            final int numOperations = operations.size();
            final ContentProviderResult[] results = new ContentProviderResult[numOperations];
            for (int i = 0; i < numOperations; i++) {
                if (++opCount > MAX_OPERATIONS_PER_PATCH) {
                    throw new OperationApplicationException(
                            "Too many content provider operations between yield points. "
                                    + "The maximum number of operations per yield point is "
                                    + MAX_OPERATIONS_PER_PATCH, ypCount);
                }
                final ContentProviderOperation operation = operations.get(i);
                results[i] = operation.apply(this, results, i);
            }
            /// M: Fix ALPS00288517, not use transaction again to avoid ANR
            /// because of order of locking db
            db.setTransactionSuccessful();
            /// @}
            return results;
        } finally {
            /// M: Fix ALPS00288517, not use transaction again to avoid ANR
            /// because of order of locking db
            db.endTransaction();
            /// @}
        }
    }
    /// @}

    /// M: Code analyze 011, fix bug ALPS00282321, ANR while delete old messages.
    /// use new process of delete. @{
    private Cursor getAllSmsThreadIds(String selection, String[] selectionArgs) {
        SQLiteDatabase db = getDBOpenHelper(0).getReadableDatabase();
        return db.query("sms",  new String[] {"distinct thread_id"},
                selection, selectionArgs, null, null, null);
    }
    /// @}

    /// M: Code analyze 005, fix bug ALPS00245352, it cost long time to restore messages.
    /// remove useless operation and add transaction while import sms. @{
    private long getThreadIdInternal(String recipient, SQLiteDatabase db, boolean importSms) {
        String THREAD_QUERY;
        //if (SystemProperties.get("ro.vendor.mtk_wappush_support").equals("1") == true) {
            THREAD_QUERY = "SELECT _id FROM threads " + "WHERE type<>"
                    + MtkTelephony.MtkThreads.WAPPUSH_THREAD + " AND type<>"
                    + MtkTelephony.MtkThreads.CELL_BROADCAST_THREAD + " AND recipient_ids=?"
                    + " AND status=0";
        /*} else {
            THREAD_QUERY = "SELECT _id FROM threads " + "WHERE type<>"
                    + MtkTelephony.MtkThreads.CELL_BROADCAST_THREAD + " AND recipient_ids=?"
                    + " AND status=0";
        }*/
        long recipientId = 0;
        if (importSms) {
            recipientId = fastGetRecipientId(recipient, db);
        } else {
            recipientId = getRecipientId(recipient, db);
        }
        MmsSmsProvider.MmsProviderLog.dpi(TAG,
                "getThreadIdInternal, recipientId = " + recipientId);
        String[] selectionArgs = new String[] { String.valueOf(recipientId) };
        Cursor cursor = db.rawQuery(THREAD_QUERY, selectionArgs);
        try {
            if (cursor != null && cursor.getCount() == 0) {
                MmsSmsProvider.MmsProviderLog.dpi(TAG,
                        "getThreadId: create new thread_id for recipients " + recipient);
                return insertThread(recipientId, db);
            } else if (cursor.getCount() == 1) {
                if (cursor.moveToFirst()) {
                    return cursor.getLong(0);
                }
            } else {
                Log.e(TAG, "getThreadId: why is cursorCount=" + cursor.getCount());
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        return 0;
    }

    private Cursor getThreadIdWithoutInsert(String recipient, SQLiteDatabase db) {
        String THREAD_QUERY;
        //if (SystemProperties.get("ro.vendor.mtk_wappush_support").equals("1") == true) {
            THREAD_QUERY = "SELECT _id FROM threads " + "WHERE type<>"
                    + MtkTelephony.MtkThreads.WAPPUSH_THREAD + " AND type<>"
                    + MtkTelephony.MtkThreads.CELL_BROADCAST_THREAD + " AND recipient_ids=?";
        /*} else {
            THREAD_QUERY = "SELECT _id FROM threads " + "WHERE type<>"
                    + MtkTelephony.MtkThreads.CELL_BROADCAST_THREAD + " AND recipient_ids=?";
        }*/
        long recipientId = getSingleAddressId(recipient, db, false);
        MmsSmsProvider.MmsProviderLog.dpi(TAG,
                "getThreadIdWithoutInsert, recipientId = " + recipientId);
        if (recipientId != -1L) {
            String[] selectionArgs = new String[] { String.valueOf(recipientId) };
            return db.rawQuery(THREAD_QUERY, selectionArgs);
        }
        return null;
    }

    /**
     * Insert a record for a new thread.
     */
    private long insertThread(long recipientIds, SQLiteDatabase db) {
        ContentValues values = new ContentValues(4);

        long date = System.currentTimeMillis();
        values.put(ThreadsColumns.DATE, date - date % 1000);
        values.put(ThreadsColumns.RECIPIENT_IDS, recipientIds);
        values.put(ThreadsColumns.MESSAGE_COUNT, 0);
        long threadId = db.insert("threads", null, values);

        return threadId;
    }

    private long getRecipientId(String address, SQLiteDatabase db) {
         if (!address.equals(PduHeaders.FROM_INSERT_ADDRESS_TOKEN_STR)) {
             long id = getSingleAddressId(address, db, true);
             if (id != -1L) {
                 return id;
             } else {
                 MmsSmsProvider.MmsProviderLog.dpi(TAG,
                 "getAddressIds: address ID not found for " + address);
             }
         }
         return 0;
    }

    private long fastGetRecipientId(String address, SQLiteDatabase db) {
        if (!address.equals(PduHeaders.FROM_INSERT_ADDRESS_TOKEN_STR)) {
            long id = -1L;
            String escapedAddress = DatabaseUtils.sqlEscapeString(address);
            boolean useStrictPhoneNumberComparation =
                    getContext().getResources().getBoolean(
                            com.android.internal.R.bool.config_use_strict_phone_number_comparation);
            String selection = "(address=" + escapedAddress + " OR PHONE_NUMBERS_EQUAL(address, " +
                    escapedAddress + (useStrictPhoneNumberComparation ? ", 1))" : ", 0))");
            Cursor cursor = db.query(
                    "canonical_addresses", CANONICAL_ADDRESSES_COLUMNS_2,
                    selection, null, null, null, null);
            try {
                if (cursor != null && cursor.moveToFirst()) {
                    id = cursor.getLong(cursor.getColumnIndex("_id"));
                    MmsSmsProvider.MmsProviderLog.dpi(TAG, "fastGetRecipientId, id=" + id);
                }
            } finally {
                if (cursor != null) {
                    cursor.close();
                }
            }
            if (id != -1L) {
                return id;
            } else {
                MmsSmsProvider.MmsProviderLog.dpi(TAG,
                "fastGetRecipientId: address ID not found for " + address);
                return insertCanonicalAddresses(db, address);
            }
        }
        return 0;
    }
    /**
     * Return the canonical address ID for this address.
     */
    private long getSingleAddressId(String address, SQLiteDatabase db, boolean needInsert) {
        long retVal = -1L;
        HashMap<String, Long> addressesMap = new HashMap<String, Long>();
        HashMap<String, ArrayList<String>> addressKeyMap = new HashMap<String, ArrayList<String>>();
        String key = "";
        ArrayList<String> candidates = null;
        Cursor cursor = null;
        try {
            cursor = db.query(
                    "canonical_addresses", CANONICAL_ADDRESSES_COLUMNS_2,
                    null, null, null, null, null);

            if (cursor != null) {
                long id;
                String number = "";
                while (cursor.moveToNext()) {
                    id = cursor.getLong(cursor.getColumnIndexOrThrow(
                            CanonicalAddressesColumns._ID));
                    number = cursor.getString(cursor.getColumnIndexOrThrow(
                            CanonicalAddressesColumns.ADDRESS));
                    CharBuffer keyBuffer = CharBuffer.allocate(
                            MmsSmsProvider.STATIC_KEY_BUFFER_MAXIMUM_LENGTH);
                    key = MmsSmsProvider.key(number, keyBuffer);
                    candidates = addressKeyMap.get(key);
                    if (candidates == null) {
                        candidates = new ArrayList<String>();
                        addressKeyMap.put(key, candidates);
                    }
                    candidates.add(number);
                    addressesMap.put(number, id);
                }
            }

            boolean isEmail = Mms.isEmailAddress(address);
            boolean isPhoneNumber = Mms.isPhoneNumber(address);
            String refinedAddress = isEmail ? address.toLowerCase() : address;
            CharBuffer keyBuffer = CharBuffer.allocate(
                    MmsSmsProvider.STATIC_KEY_BUFFER_MAXIMUM_LENGTH);
            key = MmsSmsProvider.key(refinedAddress, keyBuffer);
            candidates = addressKeyMap.get(key);
            String addressValue = "";
            if (candidates != null) {
                for (int i = 0; i < candidates.size(); i++) {
                    addressValue = candidates.get(i);
                    if (addressValue.equals(refinedAddress)) {
                        retVal = addressesMap.get(addressValue);
                        break;
                    }
                    if (isPhoneNumber && (refinedAddress != null
                            && refinedAddress.length() <= NORMAL_NUMBER_MAX_LENGTH)
                            && (addressValue != null
                                    && addressValue.length() <= NORMAL_NUMBER_MAX_LENGTH)) {
                        boolean useStrictPhoneNumberComparation
                                = getContext().getResources().getBoolean(
                                com.android.internal.R.bool
                                .config_use_strict_phone_number_comparation);

                        if (PhoneNumberUtils.compare(refinedAddress, addressValue,
                                useStrictPhoneNumberComparation)) {
                            retVal = addressesMap.get(addressValue);
                            break;
                        }
                    }
                }
            }
            if (!needInsert) {
                return retVal;
            }
            if (retVal == -1L) {
                retVal = insertCanonicalAddresses(db, refinedAddress);
                MmsSmsProvider.MmsProviderLog.dpi(TAG,
                "getSingleAddressId: insert new canonical_address for " +
                        /*address*/ "xxxxxx" + ", addressess = " + refinedAddress.toString());
            } else {
                MmsSmsProvider.MmsProviderLog.dpi(TAG, "getSingleAddressId: get exist id=" + retVal
                + ", refinedAddress=" + refinedAddress + ", currentNumber=" + addressValue);
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        return retVal;
    }

    private long insertCanonicalAddresses(SQLiteDatabase db, String refinedAddress) {
        MmsSmsProvider.MmsProviderLog.dpi(TAG, "sms insert insertCanonicalAddresses for address = "
        + refinedAddress);
        ContentValues contentValues = new ContentValues(1);
        contentValues.put(CanonicalAddressesColumns.ADDRESS, refinedAddress);
        return db.insert("canonical_addresses", CanonicalAddressesColumns.ADDRESS, contentValues);
    }
    /// @}

    /// M: because of triggers on sms and pdu, delete a large number of sms/pdu through an
    /// atomic operation will cost too much time. To avoid blocking other database operation,
    /// remove trigger sms_update_thread_on_delete, and set a limit to each delete operation. @{
    static int deleteMessages(SQLiteDatabase db,
            String selection, String[] selectionArgs) {
        Log.d(TAG, "deleteMessages, selection: " + selection);
        int count = 0;
        db.beginTransaction();
        try {
            // will trigger sms_words_delete when delete each row of sms, and may cost a long time
            // so drop the trigger first, after delete the sms and words, create the trigger again.
            db.execSQL("drop trigger if exists sms_words_delete");
            count = db.delete(TABLE_SMS, selection, selectionArgs);
            // delete useless sms body in table words
            db.execSQL("delete from words " +
                    " where table_to_use=1 and source_id not in (select _id from sms)");
            db.execSQL("CREATE TRIGGER sms_words_delete AFTER DELETE ON sms " +
                    " BEGIN " +
                    "  DELETE FROM words WHERE source_id = OLD._id AND table_to_use = 1;" +
                    " END;");
            db.setTransactionSuccessful();
        } finally {
            db.endTransaction();
        }
        Log.d(TAG, "deleteMessages, delete end, count = " + count);
        return count;
    }
    /// @}

    public static int getSubIdFromUri(Uri uri) {
        String subIdStr = uri.getQueryParameter(PhoneConstants.SUBSCRIPTION_KEY);
        int subId = SubscriptionManager.getDefaultSubscriptionId();
        try {
            subId = Integer.valueOf(subIdStr);
        } catch (NumberFormatException e) {
            Log.d(TAG, "getSubIdFromUri : " + e);
        }
        return subId;
    }

    /**
     * These methods can be overridden in a subclass for testing SmsProvider using an
     * in-memory database.
     */
    SQLiteDatabase getReadableDatabase(int match) {
        return getDBOpenHelper(match).getReadableDatabase();
    }

    SQLiteDatabase getWritableDatabase(int match) {
        return  getDBOpenHelper(match).getWritableDatabase();
    }
}
