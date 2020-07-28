/*
 * Copyright (C) 2008 The Android Open Source Project
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

import android.app.AppOpsManager;
import android.app.SearchManager;
import android.content.ComponentName;
import android.content.ContentProvider;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.UriMatcher;
import android.database.Cursor;
import android.database.DatabaseUtils;
import android.database.MatrixCursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.database.sqlite.SQLiteQueryBuilder;
import android.net.Uri;
import android.net.Uri.Builder;
import android.os.Binder;
import android.os.Bundle;
import android.os.UserHandle;
import android.os.ParcelFileDescriptor;
import android.os.SystemProperties;
import android.provider.BaseColumns;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.Telephony;
import android.provider.Telephony.CanonicalAddressesColumns;
import android.provider.Telephony.Mms;
import android.provider.Telephony.MmsSms;
import android.provider.Telephony.MmsSms.PendingMessages;
import android.provider.Telephony.Sms;
import android.provider.Telephony.Sms.Conversations;
import android.provider.Telephony.Threads;
import android.provider.Telephony.ThreadsColumns;
import android.telephony.PhoneNumberUtils;
import android.text.TextUtils;
import android.util.Log;

import com.google.android.mms.pdu.PduHeaders;

import java.io.File;
import java.io.FileDescriptor;
import java.io.FileNotFoundException;
import java.io.PrintWriter;
import java.io.UnsupportedEncodingException;
import java.nio.CharBuffer;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Locale;
import java.util.Set;

import mediatek.telephony.MtkTelephony;
import mediatek.telephony.MtkTelephony.MtkThreadSettings;
import mediatek.telephony.MtkTelephony.MtkThreads;

/**
 * This class provides the ability to query the MMS and SMS databases
 * at the same time, mixing messages from both in a single thread
 * (A.K.A. conversation).
 *
 * A virtual column, MmsSms.TYPE_DISCRIMINATOR_COLUMN, may be
 * requested in the projection for a query.  Its value is either "mms"
 * or "sms", depending on whether the message represented by the row
 * is an MMS message or an SMS message, respectively.
 *
 * This class also provides the ability to find out what addresses
 * participated in a particular thread.  It doesn't support updates
 * for either of these.
 *
 * This class provides a way to allocate and retrieve thread IDs.
 * This is done atomically through a query.  There is no insert URI
 * for this.
 *
 * Finally, this class provides a way to delete or update all messages
 * in a thread.
 */
public class MmsSmsProvider extends ContentProvider {
    private static final UriMatcher URI_MATCHER =
            new UriMatcher(UriMatcher.NO_MATCH);
    private static final String LOG_TAG = "Mms/Provider/MmsSms";
    private static final String WAPPUSH_TAG = "WapPush/Provider";
    private static final boolean DEBUG = false;

    private static final String NO_DELETES_INSERTS_OR_UPDATES =
            "MmsSmsProvider does not support deletes, inserts, or updates for this URI.";
    /// M: Add for ip message
    private static final String WALLPAPER_PATH
            = "/data/data/com.android.providers.telephony/app_wallpaper";
    private static final String WALLPAPER_JPEG = ".jpeg";
    private static final String WALLPAPER = "wallpaper";
    private static final String GENERAL_WALLPAPER = "general_wallpaper";

    private static final int URI_CONVERSATIONS                     = 0;
    private static final int URI_CONVERSATIONS_MESSAGES            = 1;
    private static final int URI_CONVERSATIONS_RECIPIENTS          = 2;
    private static final int URI_MESSAGES_BY_PHONE                 = 3;
    private static final int URI_THREAD_ID                         = 4;
    private static final int URI_CANONICAL_ADDRESS                 = 5;
    private static final int URI_PENDING_MSG                       = 6;
    private static final int URI_COMPLETE_CONVERSATIONS            = 7;
    private static final int URI_UNDELIVERED_MSG                   = 8;
    private static final int URI_CONVERSATIONS_SUBJECT             = 9;
    private static final int URI_NOTIFICATIONS                     = 10;
    private static final int URI_OBSOLETE_THREADS                  = 11;
    private static final int URI_DRAFT                             = 12;
    private static final int URI_CANONICAL_ADDRESSES               = 13;
    private static final int URI_SEARCH                            = 14;
    private static final int URI_SEARCH_SUGGEST                    = 15;
    private static final int URI_FIRST_LOCKED_MESSAGE_ALL          = 16;
    private static final int URI_FIRST_LOCKED_MESSAGE_BY_THREAD_ID = 17;
    private static final int URI_MESSAGE_ID_TO_THREAD              = 18;
    private static final int URI_QUICK_TEXT                        = 19;
    /// M: Code analyze 004, new feature, support folder mode for OP01 @{
    private static final int URI_RECIPIENTS_NUMBER                 = 24;
    /// @}
    /// M: Code analyze 005, fix bug ALPS00091288, ANR after send mms to self.
    /// add status column in thread table.
    private static final int URI_STATUS                            = 26;
    /// M: Code analyze 003, new feature, support for cellbroadcast.
    private static final int URI_CELLBROADCAST                     = 27;
    /// M: Code analyze 006, new feature, display unread message number in mms launcher.
    private static final int URI_UNREADCOUNT                       = 28;
    /// M: Code analyze 007, fix bug ALPS00255806, when reply a message, use the same
    /// number which receiving message.
    private static final int URI_SIMID_LIST                        = 29;
    /// M: Add for ip message @{
    private static final int URI_CONVERSATION_SETTINGS             = 31;
    private static final int URI_CONVERSATION_SETTINGS_ITEM        = 32;
    private static final int URI_CONVERSATIONS_EXTEND              = 33;
    private static final int URI_SUGGEST_SHORTCUT             = 37;
    /// M: Code analyze 009, fix bug ALPS00229750, failed to add fetion friends.
    /// correct query condition.
    private static final int URI_WIDGET_THREAD                     = 40;

    /// M: fix bug ALPS00473488, delete ObsoleteThread through threadID when discard()
    private static final int URI_OBSOLETE_THREAD_ID                = 41;

    /// M: ALPS01374857, MAP new feature: support auto-select-sim mode
    private static final int URI_MAP_BY_PHONE                      = 43;

    /// M: Get db size, for SELinux enhancement, mms can not get the db size directly @{
    private static final int URI_DATABASE_SIZE                      = 44;

    /// M: when query conversation messages, set max number use SQL args limit and offset
    private static final int URI_CONVERSATION_LIMIT_MESSAGES        = 45;

    private static final String DB_PATH
          = "/data/user_de/0/com.android.providers.telephony/databases/mmssms.db";
    /// @}

    /**
     * the name of the table that is used to store the queue of
     * messages(both MMS and SMS) to be sent/downloaded.
     */
    public static final String TABLE_PENDING_MSG = "pending_msgs";

    /**
     * the name of the table that is used to store the canonical addresses for both SMS and MMS.
     */
    public static final String TABLE_CANONICAL_ADDRESSES = "canonical_addresses";

    private static final String TABLE_QUICK_TEXT = "quicktext";

    private static final String TABLE_CELLBROADCAST = "cellbroadcast";

    /// M: Code analyze 005, fix bug ALPS00091288, ANR after send mms to self.
    /// add status column in thread table.
    //  private static final String TABLE_THREADS = "threads";

    /// M: Add for ip message
    public static final String TABLE_THREAD_SETTINGS = "thread_settings";

    /// M: Code analyze 010, fix bug ALPS00280371, mms can't be found out,
    /// change the search uri. @{
    private static final Uri PICK_PHONE_EMAIL_URI = Uri
            .parse("content://com.android.contacts/data/phone_email");
    public static final Uri PICK_PHONE_EMAIL_FILTER_URI = Uri.withAppendedPath(
            PICK_PHONE_EMAIL_URI, "filter");
    /// @}
    /// M: Add for ip message
    private static final String IP_MESSAGE_GUIDE_NUMBER = "35221601851";

    private static final int NORMAL_NUMBER_MAX_LENGTH              = 15;

    /**
     * the name of the table that is used to store the conversation threads.
     */
    static final String TABLE_THREADS = "threads";

    private static final boolean MTK_WAPPUSH_SUPPORT = true; //SystemProperties.get(
//            "ro.vendor.mtk_wappush_support").equals("1");

    // These constants are used to construct union queries across the
    // MMS and SMS base tables.

    // These are the columns that appear in both the MMS ("pdu") and
    // SMS ("sms") message tables.
    private static final String[] MMS_SMS_COLUMNS =
            { BaseColumns._ID, Mms.DATE, Mms.DATE_SENT, Mms.READ, Mms.THREAD_ID, Mms.LOCKED,
                    Mms.SUBSCRIPTION_ID };

    // These are the columns that appear only in the MMS message
    // table.
    private static final String[] MMS_ONLY_COLUMNS = {
        Mms.CONTENT_CLASS, Mms.CONTENT_LOCATION, Mms.CONTENT_TYPE,
        Mms.DELIVERY_REPORT, Mms.EXPIRY, Mms.MESSAGE_CLASS, Mms.MESSAGE_ID,
        Mms.MESSAGE_SIZE, Mms.MESSAGE_TYPE, Mms.MESSAGE_BOX, Mms.PRIORITY,
        Mms.READ_STATUS, Mms.RESPONSE_STATUS, Mms.RESPONSE_TEXT,
        Mms.RETRIEVE_STATUS, Mms.RETRIEVE_TEXT_CHARSET, Mms.REPORT_ALLOWED,
        Mms.READ_REPORT, Mms.STATUS, Mms.SUBJECT, Mms.SUBJECT_CHARSET,
        Mms.TRANSACTION_ID, Mms.MMS_VERSION, MtkTelephony.MtkMms.SERVICE_CENTER, Mms.TEXT_ONLY };

    /// M: Add for ip message @{
    private static final String[] THREAD_SETTINGS_COLUMNS = {
        MtkThreadSettings._ID, MtkThreadSettings.SPAM, MtkThreadSettings.NOTIFICATION_ENABLE,
        MtkThreadSettings.MUTE, MtkThreadSettings.MUTE_START, MtkThreadSettings.RINGTONE,
        MtkThreadSettings.WALLPAPER, MtkThreadSettings.VIBRATE};
    /// @}

    // These are the columns that appear only in the SMS message
    // table.
    private static final String[] SMS_ONLY_COLUMNS =
            { "address", "body", "person", "reply_path_present",
              "service_center", "status", "subject", "type", "error_code", "ipmsg_id"};

    /// M: Code analyze 003, new feature, support for cellbroadcast. @{
    private static final String[] CB_ONLY_COLUMNS =
            { "channel_id" };
    /// @}

    // These are all the columns that appear in the "threads" table.
    private static final String[] THREADS_COLUMNS = {
        BaseColumns._ID,
        ThreadsColumns.DATE,
        ThreadsColumns.RECIPIENT_IDS,
        ThreadsColumns.MESSAGE_COUNT
    };

    private static final String[] CANONICAL_ADDRESSES_COLUMNS_1 =
            new String[] { CanonicalAddressesColumns.ADDRESS };

    private static final String[] CANONICAL_ADDRESSES_COLUMNS_2 =
            new String[] { CanonicalAddressesColumns._ID,
                    CanonicalAddressesColumns.ADDRESS };

    // These are all the columns that appear in the MMS and SMS
    // message tables.
    private static final String[] UNION_COLUMNS =
            new String[MMS_SMS_COLUMNS.length
                       + MMS_ONLY_COLUMNS.length
                       + SMS_ONLY_COLUMNS.length];

    // These are all the columns that appear in the MMS table.
    public static final Set<String> MMS_COLUMNS = new HashSet<String>();

    // These are all the columns that appear in the SMS table.
    public static final Set<String> SMS_COLUMNS = new HashSet<String>();

    /// M: Code analyze 003, new feature, support for cellbroadcast.
    public static final Set<String> CB_COLUMNS = new HashSet<String>();

    private static final String VND_ANDROID_DIR_MMS_SMS =
            "vnd.android-dir/mms-sms";

    private static final String[] ID_PROJECTION = { BaseColumns._ID };

    /// M: Code analyze 005, fix bug ALPS00091288, ANR after send mms to self.
    /// add status column in thread table.
    private static final String[] STATUS_PROJECTION = { MtkTelephony.MtkThreads.STATUS };

    public static final String[] EMPTY_STRING_ARRAY = new String[0];

    private static final String[] SEARCH_STRING = new String[1];
    private static final String SEARCH_QUERY = "SELECT snippet(words, '', ' ', '', 1, 1) as " +
            "snippet FROM words WHERE index_text MATCH ? ORDER BY snippet LIMIT 50;";

    public static final String SMS_CONVERSATION_CONSTRAINT = "(" +
            Sms.TYPE + " != " + Sms.MESSAGE_TYPE_DRAFT + ")";

    public static final String MMS_CONVERSATION_CONSTRAINT = "(" +
            Mms.MESSAGE_BOX + " != " + Mms.MESSAGE_BOX_DRAFTS + " AND (" +
            Mms.MESSAGE_TYPE + " = " + PduHeaders.MESSAGE_TYPE_SEND_REQ + " OR " +
            Mms.MESSAGE_TYPE + " = " + PduHeaders.MESSAGE_TYPE_RETRIEVE_CONF + " OR " +
            Mms.MESSAGE_TYPE + " = " + PduHeaders.MESSAGE_TYPE_NOTIFICATION_IND + "))";

    /// M: Code analyze 014, fix bug ALPS00231848, search result is wrong sometimes.
    /// match contact number with space.
    private static final String SELF_ITEM_KEY = "Self_Item_Key";

    private static String getTextSearchQuery(String smsTable, String pduTable) {
        // Search on the words table but return the rows from the corresponding sms table
        final String smsQuery = "SELECT "
                + smsTable + "._id AS _id,"
                + "thread_id,"
                + "address,"
                + "body,"
                + "date,"
                + "date_sent,"
                + "index_text,"
                + "words._id "
                + "FROM " + smsTable + ",words "
                + "WHERE (index_text MATCH ? "
                + "AND " + smsTable + "._id=words.source_id "
                + "AND words.table_to_use=1)";

        // Search on the words table but return the rows from the corresponding parts table
        final String mmsQuery = "SELECT "
                + pduTable + "._id,"
                + "thread_id,"
                + "addr.address,"
                + "part.text AS body,"
                + pduTable + ".date,"
                + pduTable + ".date_sent,"
                + "index_text,"
                + "words._id "
                + "FROM " + pduTable + ",part,addr,words "
                + "WHERE ((part.mid=" + pduTable + "._id) "
                + "AND (addr.msg_id=" + pduTable + "._id) "
                + "AND (addr.type=" + PduHeaders.TO + ") "
                + "AND (part.ct='text/plain') "
                + "AND (index_text MATCH ?) "
                + "AND (part._id = words.source_id) "
                + "AND (words.table_to_use=2))";

        // This code queries the sms and mms tables and returns a unified result set
        // of text matches.  We query the sms table which is pretty simple.  We also
        // query the pdu, part and addr table to get the mms result.  Note we're
        // using a UNION so we have to have the same number of result columns from
        // both queries.
        return smsQuery + " UNION " + mmsQuery + " "
                + "GROUP BY thread_id "
                + "ORDER BY thread_id ASC, date DESC";
    }

    private static final String AUTHORITY = "mms-sms";

    static {
        URI_MATCHER.addURI(AUTHORITY, "conversations", URI_CONVERSATIONS);
        /// M: Add for ip message
        URI_MATCHER.addURI(AUTHORITY, "conversations/extend", URI_CONVERSATIONS_EXTEND);

        URI_MATCHER.addURI(AUTHORITY, "complete-conversations", URI_COMPLETE_CONVERSATIONS);

        // In these patterns, "#" is the thread ID.
        URI_MATCHER.addURI(
                AUTHORITY, "conversations/#", URI_CONVERSATIONS_MESSAGES);

        URI_MATCHER.addURI(
                AUTHORITY, "conversations/#/limit/#/offset/#", URI_CONVERSATION_LIMIT_MESSAGES);

        URI_MATCHER.addURI(
                AUTHORITY, "conversations/#/recipients",
                URI_CONVERSATIONS_RECIPIENTS);

        URI_MATCHER.addURI(
                AUTHORITY, "conversations/#/subject",
                URI_CONVERSATIONS_SUBJECT);

        // URI for deleting obsolete threads.
        URI_MATCHER.addURI(AUTHORITY, "conversations/obsolete", URI_OBSOLETE_THREADS);
        /// M: fix bug ALPS00473488, delete ObsoleteThread through threadID when discard()
        URI_MATCHER.addURI(AUTHORITY, "conversations/obsolete/#", URI_OBSOLETE_THREAD_ID);

        URI_MATCHER.addURI(
                AUTHORITY, "messages/byphone/*",
                URI_MESSAGES_BY_PHONE);

        // In this pattern, two query parameter names are expected:
        // "subject" and "recipient."  Multiple "recipient" parameters
        // may be present.
        URI_MATCHER.addURI(AUTHORITY, "threadID", URI_THREAD_ID);

        // Use this pattern to query the canonical address by given ID.
        URI_MATCHER.addURI(AUTHORITY, "canonical-address/#", URI_CANONICAL_ADDRESS);

        // Use this pattern to query all canonical addresses.
        URI_MATCHER.addURI(AUTHORITY, "canonical-addresses", URI_CANONICAL_ADDRESSES);

        URI_MATCHER.addURI(AUTHORITY, "search", URI_SEARCH);
        URI_MATCHER.addURI(AUTHORITY, "searchSuggest", URI_SEARCH_SUGGEST);
        URI_MATCHER.addURI(AUTHORITY, SearchManager.SUGGEST_URI_PATH_SHORTCUT + "/#",
                URI_SUGGEST_SHORTCUT);

        // In this pattern, two query parameters may be supplied:
        // "protocol" and "message." For example:
        //   content://mms-sms/pending?
        //       -> Return all pending messages;
        //   content://mms-sms/pending?protocol=sms
        //       -> Only return pending SMs;
        //   content://mms-sms/pending?protocol=mms&message=1
        //       -> Return the the pending MM which ID equals '1'.
        //
        URI_MATCHER.addURI(AUTHORITY, "pending", URI_PENDING_MSG);

        // Use this pattern to get a list of undelivered messages.
        URI_MATCHER.addURI(AUTHORITY, "undelivered", URI_UNDELIVERED_MSG);

        // Use this pattern to see what delivery status reports (for
        // both MMS and SMS) have not been delivered to the user.
        URI_MATCHER.addURI(AUTHORITY, "notifications", URI_NOTIFICATIONS);

        URI_MATCHER.addURI(AUTHORITY, "draft", URI_DRAFT);

        URI_MATCHER.addURI(AUTHORITY, "locked", URI_FIRST_LOCKED_MESSAGE_ALL);

        URI_MATCHER.addURI(AUTHORITY, "locked/#", URI_FIRST_LOCKED_MESSAGE_BY_THREAD_ID);

        URI_MATCHER.addURI(AUTHORITY, "messageIdToThread", URI_MESSAGE_ID_TO_THREAD);

        URI_MATCHER.addURI(AUTHORITY, "quicktext", URI_QUICK_TEXT);

        URI_MATCHER.addURI(AUTHORITY, "cellbroadcast", URI_CELLBROADCAST);
        /// M: Code analyze 005, fix bug ALPS00091288, ANR after send mms to self.
        /// add status column in thread table.
        URI_MATCHER.addURI(AUTHORITY, "conversations/status/#", URI_STATUS);

        /// M: Code analyze 004, new feature, support folder mode for OP01 @{
        URI_MATCHER.addURI(AUTHORITY, "thread_id/#", URI_RECIPIENTS_NUMBER);
        /// @}
        /// M: Code analyze 006, new feature, display unread message number in mms launcher.
        URI_MATCHER.addURI(AUTHORITY, "unread_count", URI_UNREADCOUNT);
        /// M: Code analyze 007, fix bug ALPS00255806, when reply a message, use the same
        /// number which receiving message.
        URI_MATCHER.addURI(AUTHORITY, "simid_list/#", URI_SIMID_LIST);
        /// M: Add for ip message @{
        URI_MATCHER.addURI(AUTHORITY, "thread_settings", URI_CONVERSATION_SETTINGS);

        URI_MATCHER.addURI(AUTHORITY, "thread_settings/#", URI_CONVERSATION_SETTINGS_ITEM);

        ///for wp and cb sim tag.
        URI_MATCHER.addURI(AUTHORITY, "widget/thread/#", URI_WIDGET_THREAD);
        /// M: ALPS01374857, MAP new feature: support auto-select-sim mode
        URI_MATCHER.addURI(AUTHORITY, "conversations/map", URI_MAP_BY_PHONE);

        /// M: Get db size, for SELinux enhancement, mms can not get the db size directly
        URI_MATCHER.addURI(AUTHORITY, "database_size", URI_DATABASE_SIZE);

        initializeColumnSets();
    }

    private SQLiteOpenHelper mOpenHelper;

    private boolean mUseStrictPhoneNumberComparation;

    private static final String METHOD_IS_RESTORING = "is_restoring";
    private static final String IS_RESTORING_KEY = "restoring";

    @Override
    public boolean onCreate() {
        setAppOps(AppOpsManager.OP_READ_SMS, AppOpsManager.OP_WRITE_SMS);
        mOpenHelper = MmsSmsDatabaseHelper.getInstanceForCe(getContext());
        mUseStrictPhoneNumberComparation =
            getContext().getResources().getBoolean(
                    com.android.internal.R.bool.config_use_strict_phone_number_comparation);
        TelephonyBackupAgent.DeferredSmsMmsRestoreService.startIfFilesExist(getContext());
        return true;
    }

    @Override
    public Cursor query(Uri uri, String[] projection,
            String selection, String[] selectionArgs, String sortOrder) {
        // First check if restricted views of the "sms" and "pdu" tables should be used based on the
        // caller's identity. Only system, phone or the default sms app can have full access
        // of sms/mms data. For other apps, we present a restricted view which only contains sent
        // or received messages, without wap pushes.
        final boolean accessRestricted = ProviderUtil.isAccessRestricted(
                getContext(), getCallingPackage(), Binder.getCallingUid());
        final String pduTable = MmsProvider.getPduTable(accessRestricted);
        final String smsTable = SmsProvider.getSmsTable(accessRestricted);

        SQLiteDatabase db = mOpenHelper.getReadableDatabase();
        Cursor cursor = null;
        final int match = URI_MATCHER.match(uri);
        MmsProviderLog.dpi(LOG_TAG, "query begin, uri = " + uri + ", selection = " + selection);
        Log.d(LOG_TAG, "query begin, match = " + match);

        switch (match) {
            case URI_COMPLETE_CONVERSATIONS:
                cursor = getCompleteConversations(projection, selection, sortOrder, smsTable,
                        pduTable);
                break;
            case URI_CONVERSATIONS:
                String simple = uri.getQueryParameter("simple");
                if ((simple != null) && simple.equals("true")) {
                    String threadType = uri.getQueryParameter("thread_type");
                    if (!TextUtils.isEmpty(threadType)) {
                        selection = concatSelections(
                                selection, Threads.TYPE + "=" + threadType);
                    } else if (MTK_WAPPUSH_SUPPORT) {
                        selection = concatSelections(selection, Threads.TYPE
                                + "<>" + MtkThreads.WAPPUSH_THREAD);
                    }
                    cursor = getSimpleConversations(
                            projection, selection, selectionArgs, sortOrder);
                    /// M: Code analyze 012, fix bug ALPS00262044, not show out unread message
                    /// icon after restore messages. notify mms application about unread messages
                    /// number after insert operation.
                    notifyUnreadMessageNumberChanged(getContext());
                } else {
                    cursor = getConversations(
                            projection, selection, sortOrder, smsTable, pduTable);
                }
                break;
            /// M: Add for ip message @{
            /* add this new case to support query thread_settings in one query.
             * this is only tested and used by ConversationList of Mms.
             */
            case URI_CONVERSATIONS_EXTEND:
                String simple2 = uri.getQueryParameter("simple");
                if ((simple2 != null) && simple2.equals("true")) {
                    String threadType = uri.getQueryParameter("thread_type");
                    if (!TextUtils.isEmpty(threadType)) {
                        selection = concatSelections(
                                selection, Threads.TYPE + "=" + threadType);
                    } else if (MTK_WAPPUSH_SUPPORT) {
                        selection = concatSelections(selection,
                                Threads.TYPE + "<>" + MtkThreads.WAPPUSH_THREAD);
                    }
                    cursor = getSimpleConversationsExtend(
                            projection, selection, selectionArgs, sortOrder);
                    notifyUnreadMessageNumberChanged(getContext());
                } else {
                    cursor = getConversations(
                            projection, selection, sortOrder, smsTable, pduTable);
                }
                break;
            /// @}
            case URI_CONVERSATIONS_MESSAGES:
                String isRcse = uri.getQueryParameter("isRcse");
                if ((isRcse != null) && isRcse.equals("true")) {
                    String finalSelection = concatSelections(selection,
                            "_id=" + uri.getPathSegments().get(1));
                    cursor = db.query(TABLE_THREADS, projection,
                            finalSelection, selectionArgs, null, null, sortOrder);
                } else {
                    cursor = getConversationMessages(uri.getPathSegments().get(1), projection,
                        selection, sortOrder, smsTable, pduTable);
                }
                break;
            case URI_CONVERSATION_LIMIT_MESSAGES:
                cursor = getConversationSubMessages(
                        uri.getPathSegments().get(1),
                        projection,
                        selection,
                        sortOrder,
                        smsTable,
                        pduTable,
                        uri.getPathSegments().get(3),
                        uri.getPathSegments().get(5));
                break;
            case URI_CONVERSATIONS_RECIPIENTS:
                cursor = getConversationById(
                        uri.getPathSegments().get(1), projection, selection,
                        selectionArgs, sortOrder);
                break;
            case URI_CONVERSATIONS_SUBJECT:
                cursor = getConversationById(
                        uri.getPathSegments().get(1), projection, selection,
                        selectionArgs, sortOrder);
                break;
            case URI_MESSAGES_BY_PHONE:
                cursor = getMessagesByPhoneNumber(
                        uri.getPathSegments().get(2), projection, selection, sortOrder, smsTable,
                        pduTable);
                break;
            case URI_THREAD_ID:
                List<String> recipients = uri.getQueryParameters("recipient");

                    /// M: Code analyze 013, new feature, support for wappush. @{
                    /// M: if WAP Push is supported, SMS and WAP Push from same sender
                    /// will be put in different threads
                    if (MTK_WAPPUSH_SUPPORT) {
                        if (!uri.getQueryParameters("wappush").isEmpty()) {
                            cursor = getWapPushThreadId(recipients);
                        } else if (!uri.getQueryParameters("cellbroadcast").isEmpty()) {
                            cursor = getCBThreadId(recipients);
                        } else {
                            cursor = getThreadId(recipients);
                        }
                        break;
                    }
                    /// @}
                    if (!uri.getQueryParameters("cellbroadcast").isEmpty()) {
                        cursor = getCBThreadId(recipients);
                    } else {
                        cursor = getThreadId(recipients);
                    }
                break;
            case URI_CANONICAL_ADDRESS: {
                String extraSelection = "_id=" + uri.getPathSegments().get(1);
                String finalSelection = TextUtils.isEmpty(selection)
                        ? extraSelection : extraSelection + " AND " + selection;
                cursor = db.query(TABLE_CANONICAL_ADDRESSES,
                        CANONICAL_ADDRESSES_COLUMNS_1,
                        finalSelection,
                        selectionArgs,
                        null, null,
                        sortOrder);
                break;
            }
            case URI_CANONICAL_ADDRESSES:
                cursor = db.query(TABLE_CANONICAL_ADDRESSES,
                        CANONICAL_ADDRESSES_COLUMNS_2,
                        selection,
                        selectionArgs,
                        null, null,
                        sortOrder);
                break;
            case URI_SEARCH_SUGGEST: {
                /// M: Code analyze 018, fix bug ALPS00344334, support recipients search
                /// suggestion.
                //SEARCH_STRING[0] = uri.getQueryParameter("pattern") + '*' ;

                // find the words which match the pattern using the snippet function.  The
                // snippet function parameters mainly describe how to format the result.
                // See http://www.sqlite.org/fts3.html#section_4_2 for details.
                if (       sortOrder != null
                        || selection != null
                        || selectionArgs != null
                        || projection != null) {
                    throw new IllegalArgumentException(
                            "do not specify sortOrder, selection, selectionArgs, or projection" +
                            "with this query");
                }
                /// M: Code analyze 018, fix bug ALPS00344334, support recipients search
                /// suggestion. @{
                MmsProviderLog.dpi(LOG_TAG, "query().URI_SEARCH_SUGGEST: uriStr = " + uri);
                String uriStr = uri.toString();
                String parameterStr = "pattern=";
                String searchString = uriStr.substring(uriStr.lastIndexOf(parameterStr)
                        + parameterStr.length()).trim();
                MmsProviderLog.dpi(LOG_TAG, "query().URI_SEARCH_SUGGEST: searchString = \""
                        + searchString + "\"");
                String pattern = "%" + searchString + "%";
                if (searchString.trim().equals("") || searchString == null) {
                    cursor = null;
                } else {
                    HashMap<String, String> contactRes = getContactsByNumber(searchString);
                    String searchContacts = searchContacts(searchString, contactRes);
                    String smsIdQuery = String.format("SELECT _id FROM sms WHERE thread_id "
                            + searchContacts);
                    String smsIn = queryIdAndFormatIn(db, smsIdQuery);
                    String mmsIdQuery = String.format("SELECT part._id FROM part JOIN pdu " +
                            " ON part.mid=pdu._id " +
                            " WHERE part.ct='text/plain' AND pdu.thread_id " + searchContacts);
                    String mmsIn = queryIdAndFormatIn(db, mmsIdQuery);
                    String mmsPduIdQuery = String.format("SELECT _id FROM pdu" +
                            " WHERE thread_id " + searchContacts);
                    String mmsPduIn = queryIdAndFormatIn(db, mmsPduIdQuery);
                    String query = "SELECT DISTINCT _id, index_text AS " +
                            SearchManager.SUGGEST_COLUMN_TEXT_1 + ", _id AS " +
                            SearchManager.SUGGEST_COLUMN_SHORTCUT_ID + ", index_text AS snippet" +
                            " FROM words WHERE index_text IS NOT NULL AND length(index_text)>0 " +
                            " AND ((index_text LIKE ? AND table_to_use!=3) " +
                            " OR (source_id " + smsIn + " AND table_to_use=1) " +
                            " OR (source_id " + mmsIn + " AND table_to_use=2) "/*+
                            " OR (source_id " + wpIn + " AND table_to_use=3) "*/ +
                            " OR (source_id " + mmsPduIn + " AND table_to_use="
                            + MmsProvider.TABLE_TO_USE_SUBJECT + ")) " +
                            " ORDER BY snippet LIMIT 50";
                    cursor = db.rawQuery(query, new String[]{pattern});
                    Log.d(LOG_TAG, "search suggestion cursor count is : " + cursor.getCount());
                }
                /// @}
                break;
            }
            case URI_SUGGEST_SHORTCUT: {
                Long id = Long.decode(uri.getLastPathSegment());
                String sugguestQuery = "SELECT _id, index_text AS "
                        + SearchManager.SUGGEST_COLUMN_TEXT_1 + ", _id AS "
                        + SearchManager.SUGGEST_COLUMN_SHORTCUT_ID + ", index_text AS snippet" +
                         " FROM words WHERE (_id = " + id + ")";
                cursor = db.rawQuery(sugguestQuery, null);
                break;
            }
            case URI_MESSAGE_ID_TO_THREAD: {
                // Given a message ID and an indicator for SMS vs. MMS return
                // the thread id of the corresponding thread.
                try {
                    long id = Long.parseLong(uri.getQueryParameter("row_id"));
                    switch (Integer.parseInt(uri.getQueryParameter("table_to_use"))) {
                        case 1:  // sms
                            cursor = db.query(
                                smsTable,
                                new String[] { "thread_id" },
                                "_id=?",
                                new String[] { String.valueOf(id) },
                                null,
                                null,
                                null);
                            break;
                        case 2:  // mms
                            String mmsQuery = "SELECT thread_id "
                                    + "FROM " + pduTable + ",part "
                                    + "WHERE ((part.mid=" + pduTable + "._id) "
                                    + "AND " + "(part._id=?))";
                            cursor = db.rawQuery(mmsQuery, new String[] { String.valueOf(id) });
                            break;
                    }
                } catch (NumberFormatException ex) {
                    // ignore... return empty cursor
                }
                break;
            }
            /// M: Code analyze 019, unknown, include new feature and fix bugs. support
            /// simple messages, multimedia messages, wapppush messages search. @{
            case URI_SEARCH: {
                if (       sortOrder != null
                        || selection != null
                        || selectionArgs != null
                        || projection != null) {
                    throw new IllegalArgumentException(
                            "do not specify sortOrder, selection, selectionArgs, or projection" +
                            "with this query");
                }

                /** M: This code queries the sms and mms tables and returns a unified result set
                * of text matches.  We query the sms table which is pretty simple.  We also
                * query the pdu, part and addr table to get the mms result.  Note that we're
                * using a UNION so we have to have the same number of result columns from
                * both queries. @{*/

                String pattern = uri.getQueryParameter("pattern");
                if (pattern != null) {
                   Log.d(LOG_TAG, "URI_SEARCH pattern = " + pattern.length());
                }
                /// M: Add for Chinese subject search @{
                String pduPattern = toIsoString(pattern.getBytes());
                pduPattern = "%" + pduPattern + "%";
                /// @}
                HashMap<String, String> contactRes = getContactsByNumber(pattern);
                String searchContacts = searchContacts(pattern, contactRes);
                String searchString = "%" + pattern + "%";

                String smsProjection = "sms._id as _id,thread_id,address,body,date," +
                "0 as index_text,words._id,0 as charset,0 as m_type,sms.type as msg_box," +
                "1 as msg_type";
                String mmsProjection = "pdu._id,thread_id,addr.address,pdu.sub as " + "" +
                        "body,pdu.date,0 as index_text,0,addr.charset as charset,"
                        + "pdu.m_type as m_type,pdu.msg_box as msg_box,2 as msg_type";

                /// M: search on the words table but return the rows
                /// from the corresponding sms table
                String smsQuery = String.format(
                        "SELECT %s FROM sms,words WHERE ((sms.body LIKE ? OR thread_id %s)" +
                        " AND sms._id=words.source_id AND words.table_to_use=1 AND ("
                        + "sms.thread_id IN (SELECT _id FROM threads)))",
                        smsProjection,
                        searchContacts);

                /// M: search on the words table but return the rows from
                /// the corresponding parts table
                ///M: change for fix alps01202357. rise query mms performance. @{
                String mmsQuery = String.format(Locale.ENGLISH,
                        "SELECT %s FROM pdu left join part ON pdu._id=part.mid" +
                        " AND part.ct='text/plain' left join addr on addr.msg_id=pdu._id " +
                        " WHERE ((((addr.type=%d) AND (pdu.msg_box == %d)) OR ((addr.type=%d)" +
                        " AND (pdu.msg_box != %d))) " +
                        "AND (part.text LIKE ? OR pdu.sub LIKE ? OR thread_id %s) " +
                        "AND (pdu.thread_id IN (SELECT _id FROM threads)))",
                        mmsProjection,
                        PduHeaders.FROM,
                        Mms.MESSAGE_BOX_INBOX,
                        PduHeaders.TO,
                        Mms.MESSAGE_BOX_INBOX,
                        searchContacts);
                ///@}
                /// M: Code analyze 013, new feature, support for wappush. @{
                /*
                 * search wap push
                 * table words is not used
                 * field index_text and _id are just used for union operation.
                 */

                //// M: join the results from sms and part (mms)
                String rawQuery = null;
                rawQuery = String.format(
                        "SELECT * FROM (%s UNION %s) GROUP BY %s ORDER BY %s",
                        smsQuery,
                        mmsQuery,
                        "thread_id",
                        "date DESC");

                try {
                    cursor = db.rawQuery(rawQuery, new String[] { searchString,
                        searchString, pduPattern });
                    MmsProviderLog.epi(LOG_TAG, "rawQuery = " + rawQuery);
                } catch (Exception ex) {
                    Log.e(LOG_TAG, "got exception: " + ex.toString());
                    return null;
                }
                break;
            }
            case URI_PENDING_MSG: {
                String protoName = uri.getQueryParameter("protocol");
                String msgId = uri.getQueryParameter("message");
                int proto = TextUtils.isEmpty(protoName) ? -1
                        : (protoName.equals("sms") ? MmsSms.SMS_PROTO : MmsSms.MMS_PROTO);

                String extraSelection = (proto != -1) ?
                        (PendingMessages.PROTO_TYPE + "=" + proto) : " 0=0 ";
                if (!TextUtils.isEmpty(msgId)) {
                    extraSelection += " AND " + PendingMessages.MSG_ID + "=" + msgId;
                }

                String finalSelection = TextUtils.isEmpty(selection)
                        ? extraSelection : ("(" + extraSelection + ") AND " + selection);
                String finalOrder = TextUtils.isEmpty(sortOrder)
                        ? PendingMessages.DUE_TIME : sortOrder;
                cursor = db.query(TABLE_PENDING_MSG, null,
                        finalSelection, selectionArgs, null, null, finalOrder);
                break;
            }
            case URI_UNDELIVERED_MSG: {
                /// M: ALPS00837193, query undelivered mms with non-permanent
                ///fail ones or not for OP01 @{
                /* If false, it is to filter non-permanent out. If true or no this param,
                 * means common behaviour*/
                Boolean includeNonPermanent = uri.getBooleanQueryParameter(
                    "includeNonPermanent", true);
                /// @}
                cursor = getUndeliveredMessages(projection, selection,
                        selectionArgs, sortOrder, smsTable, pduTable
                        /// M: for OP01 @{
                        , includeNonPermanent
                        /// @}
                        );
                break;
            }
            case URI_DRAFT: {
                cursor = getDraftThread(projection, selection, sortOrder, smsTable, pduTable);
                break;
            }
            case URI_FIRST_LOCKED_MESSAGE_BY_THREAD_ID: {
                long threadId;
                try {
                    threadId = Long.parseLong(uri.getLastPathSegment());
                } catch (NumberFormatException e) {
                    Log.e(LOG_TAG, "Thread ID must be a long.");
                    break;
                }
                cursor = getFirstLockedMessage(projection, "thread_id=" + Long.toString(threadId),
                        sortOrder, smsTable, pduTable);
                break;
            }
            case URI_FIRST_LOCKED_MESSAGE_ALL: {
                cursor = getFirstLockedMessage(
                        projection, selection, sortOrder, smsTable, pduTable);
                break;
            }
            case URI_QUICK_TEXT: {
                cursor = db.query(TABLE_QUICK_TEXT, projection,
                        selection, selectionArgs, null, null, sortOrder);
                break;
            }
            /// M: Code analyze 005, fix bug ALPS00091288, ANR after send mms to self.
            /// add status column in thread table. @{
            case URI_STATUS: {
                long threadId;
                try {
                    threadId = Long.parseLong(uri.getLastPathSegment());
                    MmsProviderLog.dpi(LOG_TAG, "query URI_STATUS Thread ID is " + threadId);
                } catch (NumberFormatException e) {
                    Log.e(LOG_TAG, "Thread ID must be a long.");
                    break;
                }
                cursor = db.query(TABLE_THREADS, STATUS_PROJECTION,
                        "_id=" + Long.toString(threadId), null, null, null, sortOrder);
                Log.d(LOG_TAG, "query URI_STATUS ok");
                break;
            }
            /// @}
            /// M: Code analyze 004, new feature, support folder mode for OP01 @{
            case URI_RECIPIENTS_NUMBER: {
                cursor = getRecipientsNumber(uri.getPathSegments().get(1));
                break;
            }
            /// @}
            /// M: display unread message number in mms launcher. @{
            case URI_UNREADCOUNT: {
                cursor = getAllUnreadCount(db);
                break;
            }
            /// @}
            /// M: Code analyze 007, fix bug ALPS00255806, when reply a message, use the same
            /// number which receiving message. @{
            case URI_SIMID_LIST: {
                  long threadId;
                  try {
                      threadId = Long.parseLong(uri.getLastPathSegment());
                      Log.d(LOG_TAG, "query URI_SIMID_LIST Thread ID is " + threadId);
                  } catch (NumberFormatException e) {
                      Log.e(LOG_TAG, "URI_SIMID_LIST Thread ID must be a long.");
                      break;
                  }
                cursor = getSimidListByThread(db, threadId);
                break;
            }
            /// @}
            /// M: Add for ip message @{
            case URI_CONVERSATION_SETTINGS_ITEM: {
                cursor = getConversationSettingsById(uri.getPathSegments().get(1),
                        projection, selection, selectionArgs, sortOrder);
                break;
            }

            case URI_CONVERSATION_SETTINGS: {
                cursor = db.query(TABLE_THREAD_SETTINGS,
                        projection,
                        selection,
                        selectionArgs,
                        null, null,
                        sortOrder);
                break;
            }
            /// @}

            /// M: Add for widget @{
            case URI_WIDGET_THREAD:
                long threadId;
                try {
                    threadId = Long.parseLong(uri.getLastPathSegment());
                    Log.d(LOG_TAG, "query URI_WIDGET_THREAD Thread ID is " + threadId);
                } catch (NumberFormatException e) {
                    Log.e(LOG_TAG, "URI_WIDGET_THREAD Thread ID must be a long.");
                    break;
                }
                cursor = getMsgInfo(db, threadId, selection);
                break;
            /// @}
            /// M: ALPS01374857, MAP new feature: support auto-select-sim mode for CE4A,
            /// only single-recipient thread
            case URI_MAP_BY_PHONE:
                String rawQuery = String.format("SELECT threads._id FROM threads," +
                        "canonical_addresses WHERE threads.recipient_ids = canonical_addresses._id "
                        + "AND canonical_addresses.address = %s", selection);
                cursor = db.rawQuery(rawQuery, null);
                break;

            default:
                throw new IllegalStateException("Unrecognized URI:" + uri);
        }

        if (cursor != null) {
            Log.d(LOG_TAG, "query end, count = " + cursor.getCount());
            cursor.setNotificationUri(getContext().getContentResolver(), MmsSms.CONTENT_URI);
        }
        return cursor;
    }

    /**
     * Return the canonical address IDs for these addresses.
     */
    /// M: improve get thread id performance, and replace PHONE_NUMBERS_EQAUL() with compare().
    /// keep a mapping of the key to a list of all numbers which have the same key.@{
    private Set<Long> getAddressIds(List<String> addresses) {
        Set<Long> result = new HashSet<Long>(addresses.size());
        HashMap<String, Long> addressesMap = new HashMap<String, Long>();
        HashMap<String, ArrayList<String>> addressKeyMap = new HashMap<String, ArrayList<String>>();
        String key = "";
        ArrayList<String> candidates = null;
        Cursor cursor = null;
        Log.d(LOG_TAG, "getAddressIds begin");
        try {
            SQLiteDatabase db = mOpenHelper.getReadableDatabase();
            db.beginTransaction();
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
                    CharBuffer keyBuffer = CharBuffer.allocate(STATIC_KEY_BUFFER_MAXIMUM_LENGTH);
                    /// M: for ALPS01840116, ignore char case if it is email address. @{
                    if (Mms.isEmailAddress(number)) {
                        number = number.toLowerCase();
                    }
                    /// @}
                    key = key(number, keyBuffer);
                    candidates = addressKeyMap.get(key);
                    if (candidates == null) {
                        candidates = new ArrayList<String>();
                        addressKeyMap.put(key, candidates);
                    }
                    candidates.add(number);
                    addressesMap.put(number, id);
                }
            }

            Set<String> refinedAddresses = new HashSet<String>();
            for (String address : addresses) {
                if (!address.equals(PduHeaders.FROM_INSERT_ADDRESS_TOKEN_STR)) {
                    boolean isEmail = Mms.isEmailAddress(address);
                    boolean isPhoneNumber = Mms.isPhoneNumber(address);
                    String refinedAddress = isEmail ? address.toLowerCase() : address;
                    boolean isEqual = false;
                    CharBuffer keyBuffer = CharBuffer.allocate(STATIC_KEY_BUFFER_MAXIMUM_LENGTH);
                    key = key(refinedAddress, keyBuffer);
                    candidates = addressKeyMap.get(key);
                    Long addressId = -1L;
                    String addressValue = "";
                    if (candidates == null) {
                        candidates = new ArrayList<String>();
                        addressKeyMap.put(key, candidates);
                    } else {
                        for (int i = 0; i < candidates.size(); i++) {
                            addressValue = candidates.get(i);
                            addressId = addressesMap.get(addressValue);
                            if (addressValue.equals(refinedAddress)) {
                                isEqual = true;
                                break;
                            }
                            if (isPhoneNumber && (refinedAddress != null
                                    && refinedAddress.length() <= NORMAL_NUMBER_MAX_LENGTH)
                                    && (addressValue != null
                                            && addressValue.length() <= NORMAL_NUMBER_MAX_LENGTH)) {
                                if (PhoneNumberUtils.compare(refinedAddress,
                                        addressValue, mUseStrictPhoneNumberComparation)) {
                                    isEqual = true;
                                    break;
                                }
                            }
                        }
                    }

                    if ((isEqual == true) && (addressId != null) && (addressId != -1L)) {
                        MmsProviderLog.dpi(LOG_TAG, "getAddressIds: get exist id=" + addressId
                                + ", refinedAddress=" + refinedAddress + ", currentNumber="
                                + addressValue);
                        result.add(addressId);
                    } else if ((isEqual == true) && (addressId == null)) {
                        /// M: fix ALPS01184577.
                        continue;
                    } else {
                        candidates.add(refinedAddress);
                        refinedAddresses.add(refinedAddress);
                    }
                }
            }
            long id = -1L;
            MmsProviderLog.dpi(LOG_TAG, "getAddressIds: insert new canonical_address for " +
                    /*address*/ "xxxxxx" + ", addressess = " + refinedAddresses.toString());
            for (String addr : refinedAddresses) {
                id = insertCanonicalAddresses(mOpenHelper, addr);
                if (id != -1L) {
                    result.add(id);
                } else {
                    MmsProviderLog.dpi(LOG_TAG, "getAddressIds: address ID not found for " + addr);
                }
            }
            db.setTransactionSuccessful();
            db.endTransaction();
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        return result;
    }
    /// @}

    /**
     * Return a sorted array of the given Set of Longs.
     */
    private long[] getSortedSet(Set<Long> numbers) {
        int size = numbers.size();
        long[] result = new long[size];
        int i = 0;

        for (Long number : numbers) {
            result[i++] = number;
        }

        if (size > 1) {
            Arrays.sort(result);
        }

        return result;
    }

    /**
     * Return a String of the numbers in the given array, in order,
     * separated by spaces.
     */
    private String getSpaceSeparatedNumbers(long[] numbers) {
        int size = numbers.length;
        StringBuilder buffer = new StringBuilder();

        for (int i = 0; i < size; i++) {
            if (i != 0) {
                buffer.append(' ');
            }
            buffer.append(numbers[i]);
        }
        return buffer.toString();
    }

    /**
     * Insert a record for a new thread.
     */
    private void insertThread(String recipientIds, List<String> recipients) {
        ContentValues values = new ContentValues(4);

        long date = System.currentTimeMillis();
        values.put(ThreadsColumns.DATE, date - date % 1000);
        values.put(ThreadsColumns.RECIPIENT_IDS, recipientIds);
        if (recipients.size() > 1) {
            values.put(Threads.TYPE, Threads.BROADCAST_THREAD);
        } else if (null != recipients && recipients.size() == 1
                && IP_MESSAGE_GUIDE_NUMBER.equals(recipients.get(0))) {
            values.put(Threads.TYPE, MtkThreads.IP_MESSAGE_GUIDE_THREAD);
        }
        values.put(ThreadsColumns.MESSAGE_COUNT, 0);

        long result = mOpenHelper.getWritableDatabase().insert(TABLE_THREADS, null, values);
        MmsProviderLog.dpi(LOG_TAG, "insertThread: created new thread_id " + result +
                " for recipientIds " + /*recipientIds*/ "xxxxxxx");

        getContext().getContentResolver().notifyChange(MmsSms.CONTENT_URI, null, true,
                UserHandle.USER_ALL);
    }

    /// M: Code analyze 013, new feature, support for wappush. @{
    private static final String THREAD_QUERY;
    /// M: Add query parameter "type" so that SMS & WAP Push Message from same sender
    /// will be put in different threads.
    static {
        if (MTK_WAPPUSH_SUPPORT) {
            THREAD_QUERY = "SELECT _id FROM threads " + "WHERE type<>"
                    + MtkThreads.WAPPUSH_THREAD + " AND type<>"
                    + MtkThreads.CELL_BROADCAST_THREAD + " AND recipient_ids=?"
                    + " AND status=0";
        } else {
            THREAD_QUERY = "SELECT _id FROM threads " + "WHERE type<>"
                    + MtkThreads.CELL_BROADCAST_THREAD + " AND recipient_ids=?"
                    + " AND status=0";
        }
    }

    /**
     * Return the thread ID for this list of
     * recipients IDs.  If no thread exists with this ID, create
     * one and return it.  Callers should always use
     * Threads.getThreadId to access this information.
     */
    private synchronized Cursor getThreadId(List<String> recipients) {
        Set<Long> addressIds = getAddressIds(recipients);
        String recipientIds = "";

        if (addressIds.size() == 0) {
            Log.e(LOG_TAG, "getThreadId: NO receipients specified -- NOT creating thread",
                    new Exception());
            return null;
        } else if (addressIds.size() == 1) {
            // optimize for size==1, which should be most of the cases
            for (Long addressId : addressIds) {
                recipientIds = Long.toString(addressId);
            }
        } else {
            recipientIds = getSpaceSeparatedNumbers(getSortedSet(addressIds));
        }

        if (Log.isLoggable(LOG_TAG, Log.VERBOSE)) {
            MmsProviderLog.dpi(LOG_TAG, "getThreadId: recipientIds (selectionArgs) =" +
                    /*recipientIds*/ "xxxxxxx");
        }

        String[] selectionArgs = new String[] { recipientIds };

        SQLiteDatabase db = mOpenHelper.getReadableDatabase();
        db.beginTransaction();
        Cursor cursor = null;
        try {
            // Find the thread with the given recipients
            cursor = db.rawQuery(THREAD_QUERY, selectionArgs);

            if (cursor.getCount() == 0) {
                // No thread with those recipients exists, so create the thread.
                cursor.close();

                MmsProviderLog.dpi(LOG_TAG, "getThreadId: create new thread_id for recipients " +
                        /*recipients*/ "xxxxxxxx");
                insertThread(recipientIds, recipients);

                // The thread was just created, now find it and return it.
                cursor = db.rawQuery(THREAD_QUERY, selectionArgs);
            }
            db.setTransactionSuccessful();
        } catch (Throwable ex) {
            Log.e(LOG_TAG, ex.getMessage(), ex);
        } finally {
            db.endTransaction();
        }

        if (cursor != null && cursor.getCount() > 1) {
            Log.w(LOG_TAG, "getThreadId: why is cursorCount=" + cursor.getCount());
        }
        return cursor;
    }

    public static String concatSelections(String selection1, String selection2) {
        if (TextUtils.isEmpty(selection1)) {
            return selection2;
        } else if (TextUtils.isEmpty(selection2)) {
            return selection1;
        } else {
            return selection1 + " AND " + selection2;
        }
    }

    /**
     * If a null projection is given, return the union of all columns
     * in both the MMS and SMS messages tables.  Otherwise, return the
     * given projection.
     */
    public static String[] handleNullMessageProjection(
            String[] projection) {
        return projection == null ? UNION_COLUMNS : projection;
    }

    /**
     * If a null projection is given, return the set of all columns in
     * the threads table.  Otherwise, return the given projection.
     */
    private static String[] handleNullThreadsProjection(
            String[] projection) {
        return projection == null ? THREADS_COLUMNS : projection;
    }

    /**
     * If a null sort order is given, return "normalized_date ASC".
     * Otherwise, return the given sort order.
     */
    public static String handleNullSortOrder (String sortOrder) {
        return sortOrder == null ? "normalized_date ASC" : sortOrder;
    }

    /**
     * Return existing threads in the database.
     */
    private Cursor getSimpleConversations(String[] projection, String selection,
            String[] selectionArgs, String sortOrder) {
        return mOpenHelper.getReadableDatabase().query(TABLE_THREADS, projection,
                selection, selectionArgs, null, null, " date DESC");
    }

    /**
     * Return the thread which has draft in both MMS and SMS.
     *
     * Use this query:
     *
     *   SELECT ...
     *     FROM (SELECT _id, thread_id, ...
     *             FROM pdu
     *             WHERE msg_box = 3 AND ...
     *           UNION
     *           SELECT _id, thread_id, ...
     *             FROM sms
     *             WHERE type = 3 AND ...
     *          )
     *   ;
     */
    private Cursor getDraftThread(String[] projection, String selection,
            String sortOrder, String smsTable, String pduTable) {
        String[] innerProjection = new String[] {BaseColumns._ID, Conversations.THREAD_ID};
        SQLiteQueryBuilder mmsQueryBuilder = new SQLiteQueryBuilder();
        SQLiteQueryBuilder smsQueryBuilder = new SQLiteQueryBuilder();

        mmsQueryBuilder.setTables(pduTable);
        smsQueryBuilder.setTables(smsTable);

        String mmsSubQuery = mmsQueryBuilder.buildUnionSubQuery(
                MmsSms.TYPE_DISCRIMINATOR_COLUMN, innerProjection,
                MMS_COLUMNS, 1, "mms",
                concatSelections(selection, Mms.MESSAGE_BOX + "=" + Mms.MESSAGE_BOX_DRAFTS),
                null, null);
        String smsSubQuery = smsQueryBuilder.buildUnionSubQuery(
                MmsSms.TYPE_DISCRIMINATOR_COLUMN, innerProjection,
                SMS_COLUMNS, 1, "sms",
                concatSelections(selection, Sms.TYPE + "=" + Sms.MESSAGE_TYPE_DRAFT),
                null, null);
        SQLiteQueryBuilder unionQueryBuilder = new SQLiteQueryBuilder();

        unionQueryBuilder.setDistinct(true);

        String unionQuery = unionQueryBuilder.buildUnionQuery(
                new String[] { mmsSubQuery, smsSubQuery }, null, null);

        SQLiteQueryBuilder outerQueryBuilder = new SQLiteQueryBuilder();

        outerQueryBuilder.setTables("(" + unionQuery + ")");

        String outerQuery = outerQueryBuilder.buildQuery(
                projection, null, null, null, sortOrder, null);

        return mOpenHelper.getReadableDatabase().rawQuery(outerQuery, EMPTY_STRING_ARRAY);
    }

    /**
     * Return the most recent message in each conversation in both MMS
     * and SMS.
     *
     * Use this query:
     *
     *   SELECT ...
     *     FROM (SELECT thread_id AS tid, date * 1000 AS normalized_date, ...
     *             FROM pdu
     *             WHERE msg_box != 3 AND ...
     *             GROUP BY thread_id
     *             HAVING date = MAX(date)
     *           UNION
     *           SELECT thread_id AS tid, date AS normalized_date, ...
     *             FROM sms
     *             WHERE ...
     *             GROUP BY thread_id
     *             HAVING date = MAX(date))
     *     GROUP BY tid
     *     HAVING normalized_date = MAX(normalized_date);
     *
     * The msg_box != 3 comparisons ensure that we don't include draft
     * messages.
     */
    private Cursor getConversations(String[] projection, String selection,
            String sortOrder, String smsTable, String pduTable) {
        SQLiteQueryBuilder mmsQueryBuilder = new SQLiteQueryBuilder();
        SQLiteQueryBuilder smsQueryBuilder = new SQLiteQueryBuilder();

        mmsQueryBuilder.setTables(pduTable);
        smsQueryBuilder.setTables(smsTable);

        String[] columns = handleNullMessageProjection(projection);
        String[] innerMmsProjection = makeProjectionWithDateAndThreadId(
                UNION_COLUMNS, 1000);
        String[] innerSmsProjection = makeProjectionWithDateAndThreadId(
                UNION_COLUMNS, 1);
        String mmsSubQuery = mmsQueryBuilder.buildUnionSubQuery(
                MmsSms.TYPE_DISCRIMINATOR_COLUMN, innerMmsProjection,
                MMS_COLUMNS, 1, "mms",
                concatSelections(selection, MMS_CONVERSATION_CONSTRAINT),
                "thread_id", "date = MAX(date)");
        String smsSubQuery = smsQueryBuilder.buildUnionSubQuery(
                MmsSms.TYPE_DISCRIMINATOR_COLUMN, innerSmsProjection,
                SMS_COLUMNS, 1, "sms",
                concatSelections(selection, SMS_CONVERSATION_CONSTRAINT),
                "thread_id", "date = MAX(date)");
        SQLiteQueryBuilder unionQueryBuilder = new SQLiteQueryBuilder();

        unionQueryBuilder.setDistinct(true);

        String unionQuery = unionQueryBuilder.buildUnionQuery(
                new String[] { mmsSubQuery, smsSubQuery }, null, null);

        SQLiteQueryBuilder outerQueryBuilder = new SQLiteQueryBuilder();

        outerQueryBuilder.setTables("(" + unionQuery + ")");

        String outerQuery = outerQueryBuilder.buildQuery(
                columns, null, "tid",
                "normalized_date = MAX(normalized_date)", sortOrder, null);

        return mOpenHelper.getReadableDatabase().rawQuery(outerQuery, EMPTY_STRING_ARRAY);
    }

    /**
     * Return the first locked message found in the union of MMS
     * and SMS messages.
     *
     * Use this query:
     *
     *  SELECT _id FROM pdu GROUP BY _id HAVING locked=1 UNION SELECT _id FROM sms GROUP
     *      BY _id HAVING locked=1 LIMIT 1
     *
     * We limit by 1 because we're only interested in knowing if
     * there is *any* locked message, not the actual messages themselves.
     */
    private Cursor getFirstLockedMessage(String[] projection, String selection,
            String sortOrder, String smsTable, String pduTable) {
        SQLiteQueryBuilder mmsQueryBuilder = new SQLiteQueryBuilder();
        SQLiteQueryBuilder smsQueryBuilder = new SQLiteQueryBuilder();

        mmsQueryBuilder.setTables(pduTable);
        smsQueryBuilder.setTables(smsTable);

        String[] idColumn = new String[] { BaseColumns._ID };

        // NOTE: buildUnionSubQuery *ignores* selectionArgs
        String mmsSubQuery = mmsQueryBuilder.buildUnionSubQuery(
                MmsSms.TYPE_DISCRIMINATOR_COLUMN, idColumn,
                null, 1, "mms",
                selection,
                BaseColumns._ID, "locked=1");

        String smsSubQuery = smsQueryBuilder.buildUnionSubQuery(
                MmsSms.TYPE_DISCRIMINATOR_COLUMN, idColumn,
                null, 1, "sms",
                selection,
                BaseColumns._ID, "locked=1");

        SQLiteQueryBuilder unionQueryBuilder = new SQLiteQueryBuilder();

        unionQueryBuilder.setDistinct(true);

        String unionQuery = unionQueryBuilder.buildUnionQuery(
                new String[] { mmsSubQuery, smsSubQuery }, null, "1");

        Cursor cursor = mOpenHelper.getReadableDatabase().rawQuery(unionQuery, EMPTY_STRING_ARRAY);

        if (DEBUG) {
            Log.v(LOG_TAG, "getFirstLockedMessage query: " + unionQuery);
            Log.v(LOG_TAG, "cursor count: " + cursor.getCount());
        }
        return cursor;
    }

    /**
     * Return every message in each conversation in both MMS
     * and SMS.
     */
    private Cursor getCompleteConversations(String[] projection,
            String selection, String sortOrder, String smsTable, String pduTable) {
        String unionQuery = buildConversationQuery(projection, selection, sortOrder, smsTable,
                pduTable);

        return mOpenHelper.getReadableDatabase().rawQuery(unionQuery, EMPTY_STRING_ARRAY);
    }

    /**
     * Add normalized date and thread_id to the list of columns for an
     * inner projection.  This is necessary so that the outer query
     * can have access to these columns even if the caller hasn't
     * requested them in the result.
     */
    private String[] makeProjectionWithDateAndThreadId(
            String[] projection, int dateMultiple) {
        int projectionSize = projection.length;
        String[] result = new String[projectionSize + 2];

        result[0] = "thread_id AS tid";
        result[1] = "date * " + dateMultiple + " AS normalized_date";
        for (int i = 0; i < projectionSize; i++) {
            result[i + 2] = projection[i];
        }
        return result;
    }

    /**
     * Return the union of MMS and SMS messages for this thread ID.
     */
    private Cursor getConversationMessages(
            String threadIdString, String[] projection, String selection,
            String sortOrder, String smsTable, String pduTable) {
        try {
            Long.parseLong(threadIdString);
        } catch (NumberFormatException exception) {
            Log.e(LOG_TAG, "Thread ID must be a Long.");
            return null;
        }

        String finalSelection = concatSelections(
                selection, "thread_id = " + threadIdString);
        String unionQuery = buildConversationQuery(projection, finalSelection, sortOrder, smsTable,
                pduTable);

        return mOpenHelper.getReadableDatabase().rawQuery(unionQuery, EMPTY_STRING_ARRAY);
    }

    private String buildSubProjection(String typeDiscriminatorColumn,
            String typeDiscriminatorValue,
            String[] unionColumns,
            Set<String> columnsPresentInTable) {
        int columnsCount = unionColumns.length;
        String columnStr = null;
        String projectString = " ";
        for (int i = 0; i < columnsCount; i++) {
            columnStr = unionColumns[i];
            if (columnStr.equals(typeDiscriminatorColumn)) {
                columnStr = "'" + typeDiscriminatorValue + "' AS "
                        + typeDiscriminatorColumn;
            } else if (columnsPresentInTable.contains(columnStr)) {
                if (columnStr.equals("date") && typeDiscriminatorValue.equals("mms")) {
                    columnStr = "date * 1000 AS date";
                } else {
                    columnStr = columnStr;
                }
            } else {
                columnStr = "NULL AS " + columnStr;
            }
            projectString = projectString + columnStr + ", ";
        }
        int lastIndex = projectString.lastIndexOf(",");
        projectString = projectString.substring(0, lastIndex);
        return projectString;
    }

    /**
     * Return the union of MMS and SMS messages for a conversation,
     * different with the above "getConversationMessages",
     * the cursor return has a max limit count.
     * limitString: max number of cursor count
     * offsetStr: offset from the original cursor begin
     */
    private Cursor getConversationSubMessages(
            String threadIdString, String[] projection, String selection,
            String sortOrder, String smsTable, String pduTable,
            String limitString, String offsetStr) {
        long threadId = 0;
        int limit = 0;
        int offset = 0;
        try {
            threadId = Long.parseLong(threadIdString);
            limit = Integer.parseInt(limitString);
            offset = Integer.parseInt(offsetStr);
        } catch (NumberFormatException exception) {
            Log.e(LOG_TAG, "getConversationSubMessages, wrong args!");
            return null;
        }

        // query for the first time
        if (offset == 0) {
            Cursor fullCursor = getConversationMessages(threadIdString, projection,
                        selection, sortOrder, smsTable, pduTable);
            Log.d(LOG_TAG, "fullCursor.getCount()" + fullCursor.getCount());
            if (fullCursor.getCount() <= limit) {
                return fullCursor;
            } else {
                fullCursor.close();
            }
        }

        String smsProject = buildSubProjection(
                MmsSms.TYPE_DISCRIMINATOR_COLUMN, "sms",
                projection, SMS_COLUMNS);
        String mmsProject = buildSubProjection(
                MmsSms.TYPE_DISCRIMINATOR_COLUMN, "mms",
                projection, MMS_COLUMNS);
        String cbProject = buildSubProjection(
                MmsSms.TYPE_DISCRIMINATOR_COLUMN, "cellbroadcast",
                projection, CB_COLUMNS);

        String rawSelection = concatSelections(
                selection, "thread_id = " + threadIdString);
        String mmsSelection = concatSelections(
                rawSelection, "(m_type = 128 OR m_type = 130 OR m_type = 132)");

        String rawQuery = String.format(
                    " SELECT " + smsProject + " FROM sms WHERE " + rawSelection +
                    " UNION " +
                    " SELECT " + mmsProject + " FROM pdu WHERE " + mmsSelection +
                    " UNION " +
                    " SELECT " + cbProject + " FROM cellbroadcast WHERE " + rawSelection +
                    " ORDER BY date ASC limit " + limit + " offset " + offset);

        SQLiteDatabase db = mOpenHelper.getReadableDatabase();
        return db.rawQuery(rawQuery, null);
    }

    /**
     * Return the union of MMS and SMS messages whose recipients
     * included this phone number.
     *
     * Use this query:
     *
     * SELECT ...
     *   FROM pdu, (SELECT msg_id AS address_msg_id
     *              FROM addr
     *              WHERE (address='<phoneNumber>' OR
     *              PHONE_NUMBERS_EQUAL(addr.address, '<phoneNumber>', 1/0)))
     *             AS matching_addresses
     *   WHERE pdu._id = matching_addresses.address_msg_id
     * UNION
     * SELECT ...
     *   FROM sms
     *   WHERE (address='<phoneNumber>' OR PHONE_NUMBERS_EQUAL(sms.address, '<phoneNumber>', 1/0));
     */
    private Cursor getMessagesByPhoneNumber(
            String phoneNumber, String[] projection, String selection,
            String sortOrder, String smsTable, String pduTable) {
        String escapedPhoneNumber = DatabaseUtils.sqlEscapeString(phoneNumber);
        String finalMmsSelection =
                concatSelections(
                        selection,
                        pduTable + "._id = matching_addresses.address_msg_id");
        String finalSmsSelection =
                concatSelections(
                        selection,
                        "(address=" + escapedPhoneNumber + " OR PHONE_NUMBERS_EQUAL(address, " +
                        escapedPhoneNumber +
                        (mUseStrictPhoneNumberComparation ? ", 1))" : ", 0))"));
        SQLiteQueryBuilder mmsQueryBuilder = new SQLiteQueryBuilder();
        SQLiteQueryBuilder smsQueryBuilder = new SQLiteQueryBuilder();

        mmsQueryBuilder.setDistinct(true);
        smsQueryBuilder.setDistinct(true);
        mmsQueryBuilder.setTables(
                pduTable +
                ", (SELECT msg_id AS address_msg_id " +
                "FROM addr WHERE (address=" + escapedPhoneNumber +
                " OR PHONE_NUMBERS_EQUAL(addr.address, " +
                escapedPhoneNumber +
                (mUseStrictPhoneNumberComparation ? ", 1))) " : ", 0))) ") +
                "AS matching_addresses");
        smsQueryBuilder.setTables(smsTable);

        String[] columns = handleNullMessageProjection(projection);
        String mmsSubQuery = mmsQueryBuilder.buildUnionSubQuery(
                MmsSms.TYPE_DISCRIMINATOR_COLUMN, columns, MMS_COLUMNS,
                0, "mms", finalMmsSelection, null, null);
        String smsSubQuery = smsQueryBuilder.buildUnionSubQuery(
                MmsSms.TYPE_DISCRIMINATOR_COLUMN, columns, SMS_COLUMNS,
                0, "sms", finalSmsSelection, null, null);
        SQLiteQueryBuilder unionQueryBuilder = new SQLiteQueryBuilder();

        unionQueryBuilder.setDistinct(true);

        String unionQuery = unionQueryBuilder.buildUnionQuery(
                new String[] { mmsSubQuery, smsSubQuery }, sortOrder, null);

        return mOpenHelper.getReadableDatabase().rawQuery(unionQuery, EMPTY_STRING_ARRAY);
    }

    /**
     * Return the conversation of certain thread ID.
     */
    private Cursor getConversationById(
            String threadIdString, String[] projection, String selection,
            String[] selectionArgs, String sortOrder) {
        try {
            Long.parseLong(threadIdString);
        } catch (NumberFormatException exception) {
            Log.e(LOG_TAG, "Thread ID must be a Long.");
            return null;
        }

        String extraSelection = "_id=" + threadIdString;
        String finalSelection = concatSelections(selection, extraSelection);
        SQLiteQueryBuilder queryBuilder = new SQLiteQueryBuilder();
        String[] columns = handleNullThreadsProjection(projection);

        queryBuilder.setDistinct(true);
        queryBuilder.setTables(TABLE_THREADS);
        return queryBuilder.query(
                mOpenHelper.getReadableDatabase(), columns, finalSelection,
                selectionArgs, sortOrder, null, null);
    }

    public static String joinPduAndPendingMsgTables(String pduTable) {
        return pduTable + " LEFT JOIN " + TABLE_PENDING_MSG
                + " ON " + pduTable + "._id = pending_msgs.msg_id";
    }

    public static String[] createMmsProjection(String[] old, String pduTable) {
        String[] newProjection = new String[old.length];
        for (int i = 0; i < old.length; i++) {
            if (old[i].equals(BaseColumns._ID)) {
                newProjection[i] = pduTable + "._id";
            } else {
                newProjection[i] = old[i];
            }
        }
        return newProjection;
    }

    private Cursor getUndeliveredMessages(
            String[] projection, String selection, String[] selectionArgs,
            String sortOrder, String smsTable, String pduTable
            /// M: ALPS00837193, query undelivered mms with non-permanent
            ///fail ones or not for OP01 @{
            ,Boolean includeNonPermanentFail
            /// @}
            ) {
        String[] mmsProjection = createMmsProjection(projection, pduTable);

        SQLiteQueryBuilder mmsQueryBuilder = new SQLiteQueryBuilder();
        SQLiteQueryBuilder smsQueryBuilder = new SQLiteQueryBuilder();

        mmsQueryBuilder.setTables(joinPduAndPendingMsgTables(pduTable));
        smsQueryBuilder.setTables(smsTable);

        String finalMmsSelection;
        /// M: ALPS00837193, query undelivered mms with non-permanent fail ones or not for OP01 @{
        if (includeNonPermanentFail) {
            Log.d(LOG_TAG, "getUndeliveredMessages true");
        /// @}
            finalMmsSelection = concatSelections(
                    selection, "(" + Mms.MESSAGE_BOX + " = " + Mms.MESSAGE_BOX_OUTBOX
                    + " OR " + Mms.MESSAGE_BOX + " = " + Mms.MESSAGE_BOX_FAILED + ")");
        /// M for OP01 @{
        } else {
            Log.d(LOG_TAG, "getUndeliveredMessages false");
            /// M: ALPS00597710, to notify send fail, only need query permanent failed mms
            /// in pending_msgs @{
            finalMmsSelection = concatSelections(
                    selection, Mms.MESSAGE_BOX + " = " + Mms.MESSAGE_BOX_OUTBOX
                    + " AND " + PendingMessages.ERROR_TYPE + " = "
                    + MmsSms.ERR_TYPE_GENERIC_PERMANENT);
            /// @}
        }

        String finalSmsSelection = concatSelections(
                selection, "(" + Sms.TYPE + " = " + Sms.MESSAGE_TYPE_OUTBOX
                + " OR " + Sms.TYPE + " = " + Sms.MESSAGE_TYPE_FAILED
                + " OR " + Sms.TYPE + " = " + Sms.MESSAGE_TYPE_QUEUED + ")");

        String[] smsColumns = handleNullMessageProjection(projection);
        String[] mmsColumns = handleNullMessageProjection(mmsProjection);
        String[] innerMmsProjection = makeProjectionWithDateAndThreadId(
                mmsColumns, 1000);
        String[] innerSmsProjection = makeProjectionWithDateAndThreadId(
                smsColumns, 1);

        Set<String> columnsPresentInTable = new HashSet<String>(MMS_COLUMNS);
        columnsPresentInTable.add(pduTable + "._id");
        columnsPresentInTable.add(PendingMessages.ERROR_TYPE);
        String mmsSubQuery = mmsQueryBuilder.buildUnionSubQuery(
                MmsSms.TYPE_DISCRIMINATOR_COLUMN, innerMmsProjection,
                columnsPresentInTable, 1, "mms", finalMmsSelection,
                null, null);
        String smsSubQuery = smsQueryBuilder.buildUnionSubQuery(
                MmsSms.TYPE_DISCRIMINATOR_COLUMN, innerSmsProjection,
                SMS_COLUMNS, 1, "sms", finalSmsSelection,
                null, null);
        SQLiteQueryBuilder unionQueryBuilder = new SQLiteQueryBuilder();

        unionQueryBuilder.setDistinct(true);

        String unionQuery = unionQueryBuilder.buildUnionQuery(
                new String[] { smsSubQuery, mmsSubQuery }, null, null);

        SQLiteQueryBuilder outerQueryBuilder = new SQLiteQueryBuilder();

        outerQueryBuilder.setTables("(" + unionQuery + ")");

        String outerQuery = outerQueryBuilder.buildQuery(
                smsColumns, null, null, null, sortOrder, null);

        return mOpenHelper.getReadableDatabase().rawQuery(outerQuery, EMPTY_STRING_ARRAY);
    }

    /**
     * Add normalized date to the list of columns for an inner
     * projection.
     */
    public static String[] makeProjectionWithNormalizedDate(
            String[] projection, int dateMultiple) {
        int projectionSize = projection.length;
        String[] result = new String[projectionSize + 1];

        result[0] = "date * " + dateMultiple + " AS normalized_date";
        System.arraycopy(projection, 0, result, 1, projectionSize);
        return result;
    }

    private static String buildConversationQuery(String[] projection,
            String selection, String sortOrder, String smsTable, String pduTable) {
        String[] mmsProjection = createMmsProjection(projection, pduTable);

        SQLiteQueryBuilder mmsQueryBuilder = new SQLiteQueryBuilder();
        SQLiteQueryBuilder smsQueryBuilder = new SQLiteQueryBuilder();

        mmsQueryBuilder.setDistinct(true);
        smsQueryBuilder.setDistinct(true);
        mmsQueryBuilder.setTables(joinPduAndPendingMsgTables(pduTable));
        smsQueryBuilder.setTables(smsTable);

        String[] smsColumns = handleNullMessageProjection(projection);
        String[] mmsColumns = handleNullMessageProjection(mmsProjection);
        String[] innerMmsProjection = makeProjectionWithNormalizedDate(mmsColumns, 1000);
        String[] innerSmsProjection = makeProjectionWithNormalizedDate(smsColumns, 1);

        Set<String> columnsPresentInTable = new HashSet<String>(MMS_COLUMNS);
        columnsPresentInTable.add(pduTable + "._id");
        columnsPresentInTable.add(PendingMessages.ERROR_TYPE);

        String mmsSelection = concatSelections(selection,
                                Mms.MESSAGE_BOX + " != " + Mms.MESSAGE_BOX_DRAFTS);
        String mmsSubQuery = mmsQueryBuilder.buildUnionSubQuery(
                MmsSms.TYPE_DISCRIMINATOR_COLUMN, innerMmsProjection,
                columnsPresentInTable, 0, "mms",
                concatSelections(mmsSelection, MMS_CONVERSATION_CONSTRAINT),
                null, null);
        String smsSubQuery = smsQueryBuilder.buildUnionSubQuery(
                MmsSms.TYPE_DISCRIMINATOR_COLUMN, innerSmsProjection, SMS_COLUMNS,
                0, "sms", concatSelections(selection, SMS_CONVERSATION_CONSTRAINT),
                null, null);

        /// M: Code analyze 003, new feature, support for cellbroadcast. @{
        SQLiteQueryBuilder cbQueryBuilder = new SQLiteQueryBuilder();
        cbQueryBuilder.setDistinct(true);
        cbQueryBuilder.setTables("cellbroadcast");
        String[] cbColumns = handleNullMessageProjection(projection);
        String[] innerCbProjection = makeProjectionWithNormalizedDate(cbColumns, 1);
        String cbSubQuery = cbQueryBuilder.buildUnionSubQuery(
                MmsSms.TYPE_DISCRIMINATOR_COLUMN, innerCbProjection, CB_COLUMNS,
                0, "cellbroadcast", selection, null, null);
        /// @}
        SQLiteQueryBuilder unionQueryBuilder = new SQLiteQueryBuilder();

        unionQueryBuilder.setDistinct(true);
        /// M: Code analyze 003, new feature, support for cellbroadcast. @{
        String unionQuery = unionQueryBuilder.buildUnionQuery(
                new String[] { smsSubQuery, mmsSubQuery, cbSubQuery },
                handleNullSortOrder(sortOrder), null);

        SQLiteQueryBuilder outerQueryBuilder = new SQLiteQueryBuilder();

        outerQueryBuilder.setTables("(" + unionQuery + ")");

        return outerQueryBuilder.buildQuery(
                smsColumns, null, null, null, sortOrder, null);
    }

    @Override
    public String getType(Uri uri) {
        return VND_ANDROID_DIR_MMS_SMS;
    }

    @Override
    public int delete(Uri uri, String selection,
            String[] selectionArgs) {
        MmsProviderLog.dpi(LOG_TAG, "delete begin, uri = " + uri + ", selection = " + selection);
        SQLiteDatabase db = mOpenHelper.getWritableDatabase();
        Context context = getContext();
        int affectedRows = 0;

        int match = URI_MATCHER.match(uri);
        Log.d(LOG_TAG, "delete begin, match = " + match);

        switch(match) {
            case URI_CONVERSATIONS_MESSAGES:
                long threadId;
                try {
                    threadId = Long.parseLong(uri.getLastPathSegment());
                } catch (NumberFormatException e) {
                    Log.e(LOG_TAG, "Thread ID must be a long.");
                    break;
                }

                /// M: Code analyze 017, fix bug ALPS00268161, new received message will be delete
                /// while deleting older messages.
                affectedRows = deleteConversation(uri, selection, selectionArgs);
                MmsSmsDatabaseHelper.updateThread(db, threadId);
                break;
            case URI_CONVERSATIONS:
                /// M: Fix bug ALPS00780175, 1300 threads deleting will cost more than 10 minutes.
                /// for improve multi threads deleting performance, avoid to update the thread after
                /// delete one conversation, just update all to be deleted threads after delete all
                /// conversations one by one. @{

                String multidelete = uri.getQueryParameter("multidelete");
                Log.d(LOG_TAG, "delete URI_CONVERSATIONS begin, multidelete = " + multidelete);
                if ((multidelete != null) && multidelete.equals("true")) {
                    String[] deleteThreadIds = selectionArgs;
                    selectionArgs = null;
                    long id = -1L;
                    int i = 0;
                    long[] deleteThreads = new long[deleteThreadIds.length];
                    for (String deleteThreadId : deleteThreadIds) {
                        try {
                            id = Long.parseLong(deleteThreadId);
                        } catch (NumberFormatException e) {
                            Log.e(LOG_TAG, "Thread ID must be a long.");
                            break;
                        }

                        Uri deleteUri = ContentUris.withAppendedId(Threads.CONTENT_URI, id);
                        String smsId = uri.getQueryParameter("smsId");
                        String mmsId = uri.getQueryParameter("mmsId");
                        if (!TextUtils.isEmpty(smsId)) {
                            deleteUri = deleteUri.buildUpon()
                                    .appendQueryParameter("smsId", smsId).build();
                        }
                        if (!TextUtils.isEmpty(mmsId)) {
                            deleteUri = deleteUri.buildUpon()
                                    .appendQueryParameter("mmsId", mmsId).build();
                        }

                        affectedRows += deleteConversation(deleteUri, selection, selectionArgs);
                        deleteThreads[i++] = id;
                    }
                    Log.d(LOG_TAG, "multi delete URI_CONVERSATIONS end");
                    if (affectedRows > 0) {
                        MmsSmsDatabaseHelper.updateMultiThreads(db, deleteThreads);
                    }
                } else {
                    /// M: Code analyze 017, fix bug ALPS00268161, new received message
                    /// will be delete while deleting older messages.
                    affectedRows = deleteAllConversation(db, uri, selection, selectionArgs);
                    Log.d(LOG_TAG, "delete URI_CONVERSATIONS end");
                    MmsSmsDatabaseHelper.updateAllThreads(db, null, null);
                }
                break;
            case URI_OBSOLETE_THREADS:
                /// M: Code analyze 003, new feature, support for cellbroadcast, wappush.
                /// TODO: other fix. @{
                String delSelectionString = "status=0 AND _id NOT IN (SELECT DISTINCT thread_id " +
                        "FROM sms where thread_id NOT NULL "
                    + "UNION SELECT DISTINCT thread_id FROM cellbroadcast where thread_id NOT NULL "
                    + "UNION SELECT DISTINCT thread_id FROM pdu where thread_id NOT NULL)";

                if (MTK_WAPPUSH_SUPPORT) {
                    delSelectionString = "status=0 AND _id NOT IN (SELECT DISTINCT thread_id " +
                            "FROM sms where thread_id NOT NULL "
                    + "UNION SELECT DISTINCT thread_id FROM cellbroadcast where thread_id NOT NULL "
                    + "UNION SELECT DISTINCT thread_id FROM pdu where thread_id NOT NULL "
                    + "UNION SELECT DISTINCT thread_id FROM wappush where thread_id NOT NULL)";
                }
                affectedRows = db.delete(TABLE_THREADS, delSelectionString, null);
                affectedRows += db.delete(TABLE_THREADS,
                                        "recipient_ids = \"\"" + " AND status=0", null);
                if (affectedRows > 0) {
                    Log.d(LOG_TAG, "delete,  delete obsolete threads end," +
                            " removeUnferencedCanonicalAddresses start");
                    MmsSmsDatabaseHelper.removeUnferencedCanonicalAddresses(db);
                }
                break;

            /// M: fix bug ALPS00473488, delete ObsoleteThread through threadID when discard()
            case URI_OBSOLETE_THREAD_ID:
                long thread_id;
                try {
                    thread_id = Long.parseLong(uri.getLastPathSegment());
                } catch (NumberFormatException e) {
                    Log.e(LOG_TAG, "Thread ID must be a long.");
                    break;
                }
                String delSelectionStringID = "status=0 AND _id = ? AND _id NOT IN " +
                        "(SELECT DISTINCT thread_id FROM sms where thread_id NOT NULL "
                        + "UNION SELECT DISTINCT thread_id FROM cellbroadcast" +
                        " where thread_id NOT NULL "
                        + "UNION SELECT DISTINCT thread_id FROM pdu where thread_id NOT NULL)";

                if (MTK_WAPPUSH_SUPPORT) {
                    delSelectionStringID = "status=0 AND _id = ? AND _id NOT IN (" +
                            "SELECT DISTINCT thread_id FROM sms where thread_id NOT NULL "
                    + "UNION SELECT DISTINCT thread_id FROM cellbroadcast" +
                    " where thread_id NOT NULL "
                    + "UNION SELECT DISTINCT thread_id FROM pdu where thread_id NOT NULL "
                    + "UNION SELECT DISTINCT thread_id FROM wappush where thread_id NOT NULL)";

                }
                affectedRows = db.delete(TABLE_THREADS, delSelectionStringID,
                        new String[] {String.valueOf(thread_id)});
                affectedRows += db.delete(TABLE_THREADS,
                                            "recipient_ids = \"\"" + " AND status=0", null);
                if (affectedRows > 0) {
                    Log.d(LOG_TAG, "delete,  delete obsolete thread end," +
                            " removeUnferencedCanonicalAddresses start");
                    MmsSmsDatabaseHelper.removeUnferencedCanonicalAddresses(db);
                }
                break;
                /// @}
            /// M: Code analyze 002, new feature, support for quicktext. @{
            case URI_QUICK_TEXT:
                affectedRows = db.delete(TABLE_QUICK_TEXT, selection, selectionArgs);
                break;
            /// @}
            /// M: Code analyze 003, new feature, support for cellbroadcast.
            case URI_CELLBROADCAST:
                affectedRows = db.delete(TABLE_CELLBROADCAST, selection, selectionArgs);
                break;
            /// @}
            default:
                throw new UnsupportedOperationException(NO_DELETES_INSERTS_OR_UPDATES);
        }

        if (affectedRows > 0) {
            context.getContentResolver().notifyChange(MmsSms.CONTENT_URI, null, true,
                    UserHandle.USER_ALL);
            /// M: Code analyze 012, fix bug ALPS00262044, not show out unread message
            /// icon after restore messages. notify mms application about unread messages
            /// number after insert operation.
            notifyChange();
        }
        Log.d(LOG_TAG, "delete end, affectedRows = " + affectedRows);
        return affectedRows;
    }

    /**
     * Delete the conversation with the given thread ID.
     */
    /// M: Code analyze 017, fix bug ALPS00268161, new received message will be delete
    /// while deleting older messages. @{
    private int deleteConversation(Uri uri, String selection, String[] selectionArgs) {
        String threadId = uri.getLastPathSegment();

        SQLiteDatabase db = mOpenHelper.getWritableDatabase();
        String finalSelection = concatSelections(selection, "thread_id = " + threadId);
        String smsId = uri.getQueryParameter("smsId");
        String mmsId = uri.getQueryParameter("mmsId");
        Log.d(LOG_TAG, "deleteConversation get max message smsId = " + smsId + " mmsId =" + mmsId);
        String finalSmsSelection;
        String finalMmsSelection;
        if (smsId != null) {
            finalSmsSelection = concatSelections(finalSelection, "_id<=" + smsId);
        } else {
            finalSmsSelection = finalSelection;
        }
        if (mmsId != null) {
            finalMmsSelection = concatSelections(finalSelection, "_id<=" + mmsId);
        } else {
            finalMmsSelection = finalSelection;
        }

        return MmsProvider.deleteMessages(getContext(), db, finalMmsSelection,
                                          selectionArgs, uri, false)
                + SmsProvider.deleteMessages(db, finalSmsSelection, selectionArgs)
                + db.delete("cellbroadcast", finalSelection, selectionArgs);
    }

    @Override
    public Uri insert(Uri uri, ContentValues values) {
        MmsProviderLog.dpi(LOG_TAG, "insert begin, uri = " + uri + ", values = " + values);
        SQLiteDatabase db = mOpenHelper.getWritableDatabase();
        int matchIndex = URI_MATCHER.match(uri);
        Log.d(LOG_TAG, "insert begin, matchIndex = " + matchIndex);
        switch (matchIndex) {
            case URI_QUICK_TEXT:
                db.insertOrThrow("quicktext", null, values);
                return uri;
            case URI_PENDING_MSG:
                long rowId = db.insert(TABLE_PENDING_MSG, null, values);
                return uri.buildUpon().appendPath(Long.toString(rowId)).build();
            case URI_CANONICAL_ADDRESS:
                long rowid = db.insert(TABLE_CANONICAL_ADDRESSES, null, values);
                return uri.buildUpon().appendPath(Long.toString(rowid)).build();
            /// M: Get db size, for SELinux enhancement, mms can not get the db size directly @{
            case URI_DATABASE_SIZE:
                return getDatabaseSize(uri);
            /// @}
            default:
                throw new UnsupportedOperationException(NO_DELETES_INSERTS_OR_UPDATES + uri);
        }
    }

    @Override
    public int update(Uri uri, ContentValues values,
            String selection, String[] selectionArgs) {
        final int callerUid = Binder.getCallingUid();
        final String callerPkg = getCallingPackage();
        SQLiteDatabase db = mOpenHelper.getWritableDatabase();
        int affectedRows = 0;
        int match = URI_MATCHER.match(uri);
        MmsProviderLog.dpi(LOG_TAG, "update begin, uri = " + uri + ", values = " + values
                + ", selection = " + selection);
        Log.d(LOG_TAG, "update begin, match = " + match);

        switch(match) {
            case URI_CONVERSATIONS_MESSAGES:
                String threadIdString = uri.getPathSegments().get(1);
                String isRcse = uri.getQueryParameter("isRcse");
                if ((isRcse != null) && isRcse.equals("true")) {
                    String finalSelection = concatSelections(selection, "_id=" + threadIdString);
                    affectedRows = db.update(TABLE_THREADS, values, finalSelection, selectionArgs);
                } else {
                    affectedRows = updateConversation(threadIdString, values,
                        selection, selectionArgs, callerUid, callerPkg);
                }
                break;

            case URI_PENDING_MSG:
                affectedRows = db.update(TABLE_PENDING_MSG, values, selection, null);
                break;

            case URI_CANONICAL_ADDRESS: {
                String extraSelection = "_id=" + uri.getPathSegments().get(1);
                String finalSelection = TextUtils.isEmpty(selection)
                        ? extraSelection : extraSelection + " AND " + selection;

                affectedRows = db.update(TABLE_CANONICAL_ADDRESSES, values, finalSelection, null);
                break;
            }

            case URI_CONVERSATIONS: {
                final ContentValues finalValues = new ContentValues(1);
                if (values.containsKey(Threads.ARCHIVED)) {
                    // Only allow update archived
                    finalValues.put(Threads.ARCHIVED, values.getAsBoolean(Threads.ARCHIVED));
                }
                affectedRows = db.update(TABLE_THREADS, finalValues, selection, selectionArgs);
                break;
            }

            case URI_QUICK_TEXT:
                affectedRows = db.update(TABLE_QUICK_TEXT, values,
                        selection, selectionArgs);
                break;

            /// M: Code analyze 005, fix bug ALPS00091288, ANR after send mms to self.
            /// add status column in thread table. @{
            case URI_STATUS: {
                long threadId;
                try {
                    threadId = Long.parseLong(uri.getLastPathSegment());
                    Log.d(LOG_TAG, "update URI_STATUS Thread ID is " + threadId);
                } catch (NumberFormatException e) {
                    Log.e(LOG_TAG, "Thread ID must be a long.");
                    break;
                }

                affectedRows = db.update(TABLE_THREADS,
                        values, "_id = " + Long.toString(threadId), null);
                Log.d(LOG_TAG, "update URI_STATUS ok");
                break;
            }
            /// @}

            /// M: Add for ip message @{
            case URI_CONVERSATION_SETTINGS_ITEM: {
                String extraSelection = MtkThreadSettings.THREAD_ID + "="
                        + uri.getPathSegments().get(1);
                String finalSelection = TextUtils.isEmpty(selection)
                        ? extraSelection : extraSelection + " AND " + selection;
                affectedRows = db.update(TABLE_THREAD_SETTINGS,
                        values, finalSelection, selectionArgs);
                break;
            }

            case URI_CONVERSATION_SETTINGS: {
                affectedRows = db.update(TABLE_THREAD_SETTINGS, values, selection, selectionArgs);
                break;
            }
            /// @}
            default:
                throw new UnsupportedOperationException(
                        NO_DELETES_INSERTS_OR_UPDATES + uri);
        }

        if (affectedRows > 0) {
            notifyChange();

        }
        Log.d(LOG_TAG, "update end, affectedRows = " + affectedRows);
        return affectedRows;
    }

    private int updateConversation(String threadIdString, ContentValues values, String selection,
            String[] selectionArgs, int callerUid, String callerPkg) {
        try {
            Long.parseLong(threadIdString);
        } catch (NumberFormatException exception) {
            Log.e(LOG_TAG, "Thread ID must be a Long.");
            return 0;

        }
        if (ProviderUtil.shouldRemoveCreator(values, callerUid)) {
            // CREATOR should not be changed by non-SYSTEM/PHONE apps
            Log.w(LOG_TAG, callerPkg + " tries to update CREATOR");
            // Sms.CREATOR and Mms.CREATOR are same. But let's do this
            // twice in case the names may differ in the future
            values.remove(Sms.CREATOR);
            values.remove(Mms.CREATOR);
        }

        SQLiteDatabase db = mOpenHelper.getWritableDatabase();
        String finalSelection = concatSelections(selection, "thread_id=" + threadIdString);
        return db.update(MmsProvider.TABLE_PDU, values, finalSelection, selectionArgs)
                + db.update("sms", values, finalSelection, selectionArgs)
                + db.update("cellbroadcast", values, finalSelection, selectionArgs);
    }

    /**
     * Construct Sets of Strings containing exactly the columns
     * present in each table.  We will use this when constructing
     * UNION queries across the MMS and SMS tables.
     */
    private static void initializeColumnSets() {
        int commonColumnCount = MMS_SMS_COLUMNS.length;
        int mmsOnlyColumnCount = MMS_ONLY_COLUMNS.length;
        int smsOnlyColumnCount = SMS_ONLY_COLUMNS.length;
        int cbOnlyColumnCount = CB_ONLY_COLUMNS.length;
        Set<String> unionColumns = new HashSet<String>();

        for (int i = 0; i < commonColumnCount; i++) {
            MMS_COLUMNS.add(MMS_SMS_COLUMNS[i]);
            SMS_COLUMNS.add(MMS_SMS_COLUMNS[i]);
            CB_COLUMNS.add(MMS_SMS_COLUMNS[i]);
            unionColumns.add(MMS_SMS_COLUMNS[i]);
        }
        for (int i = 0; i < mmsOnlyColumnCount; i++) {
            MMS_COLUMNS.add(MMS_ONLY_COLUMNS[i]);
            unionColumns.add(MMS_ONLY_COLUMNS[i]);
        }
        for (int i = 0; i < smsOnlyColumnCount; i++) {
            SMS_COLUMNS.add(SMS_ONLY_COLUMNS[i]);
            unionColumns.add(SMS_ONLY_COLUMNS[i]);
        }

        for (int i = 0; i < cbOnlyColumnCount; i++) {
            CB_COLUMNS.add(CB_ONLY_COLUMNS[i]);
            //unionColumns.add(CB_ONLY_COLUMNS[i]);
        }

        int i = 0;
        for (String columnName : unionColumns) {
            UNION_COLUMNS[i++] = columnName;
        }
    }

    @Override
    public void dump(FileDescriptor fd, PrintWriter writer, String[] args) {
        // Dump default SMS app
        String defaultSmsApp = Telephony.Sms.getDefaultSmsPackage(getContext());
        if (TextUtils.isEmpty(defaultSmsApp)) {
            defaultSmsApp = "None";
        }
        writer.println("Default SMS app: " + defaultSmsApp);
    }

    /// M: Add for widget @{
    private Cursor getMsgInfo(SQLiteDatabase db, long threadId, String selection) {
        String rawQuery = null;
        String smsSelection = " thread_id=" + threadId;
        String mmsSelection = " thread_id=" + threadId
                + " AND (m_type=128 OR m_type=130 OR m_type=132)";
        if (selection != null) {
            smsSelection = concatSelections(selection, smsSelection);
            mmsSelection = concatSelections(selection, mmsSelection);
            rawQuery = String.format(
                    " SELECT _id, type AS msg_box, date FROM sms WHERE " + smsSelection +
                    " UNION " +
                    " SELECT _id, msg_box, date*1000 AS date FROM pdu WHERE " + mmsSelection);
        } else {
            rawQuery = String.format(
                    " SELECT _id, type AS msg_box, date FROM sms WHERE " + smsSelection +
                    " UNION " +
                    " SELECT _id, msg_box, date*1000 AS date FROM pdu WHERE " + mmsSelection +
                    " ORDER BY date DESC LIMIT 1");
        }
        MmsProviderLog.dpi(LOG_TAG, "getMsgBox begin rawQuery = " + rawQuery);
        return db.rawQuery(rawQuery, null);
    }
    /// @}

    /// M: Add for ip message @{
    /**
     * Return the thread settings of certain thread ID.
     */
    private Cursor getConversationSettingsById(
            String threadIdString, String[] projection, String selection,
            String[] selectionArgs, String sortOrder) {
        try {
            Long.parseLong(threadIdString);
        } catch (NumberFormatException exception) {
            Log.e(LOG_TAG, "Thread ID must be a Long.");
            return null;
        }

        String finalSelection = concatSelections(selection,
                MtkThreadSettings.THREAD_ID + "=" + threadIdString);
        SQLiteQueryBuilder queryBuilder = new SQLiteQueryBuilder();
        String[] columns = projection == null ? THREAD_SETTINGS_COLUMNS : projection;

        queryBuilder.setDistinct(true);
        queryBuilder.setTables(TABLE_THREAD_SETTINGS);
        return queryBuilder.query(
                mOpenHelper.getReadableDatabase(), columns, finalSelection,
                selectionArgs, sortOrder, null, null);
    }
    // @}

    /// M: Code analyze 018, fix bug ALPS00344334, support recipients search
    /// suggestion. @{
    public static String queryIdAndFormatIn(SQLiteDatabase db, String sql) {
        Cursor cursor = null;
        MmsProviderLog.dpi(LOG_TAG, "queryIdAndFormatIn sql is: " + sql);
        if (sql != null && sql.trim() != "") {
            cursor = db.rawQuery(sql, null);
        }
        if (cursor == null) {
            return " IN () ";
        }
        try {
            Log.d(LOG_TAG, "queryIdAndFormatIn Cursor count is: " + cursor.getCount());
            Set<Long> ids = new HashSet<Long>();
            while (cursor.moveToNext()) {
                Long id = cursor.getLong(0);
                ids.add(id);
            }
            /* to IN sql */
            String in = " IN ";
            in += ids.toString();
            in = in.replace('[', '(');
            in = in.replace(']', ')');
            MmsProviderLog.dpi(LOG_TAG, "queryIdAndFormatIn, In = " + in);
            return in;
        } finally {
            cursor.close();
        }
    }
    /// @}
    /// M: Code analyze 006, new feature, display unread message number in mms launcher.
    /// TODO: other fix. @{
    private static Cursor getAllUnreadCount(SQLiteDatabase db) {
        Log.d(LOG_TAG, "getAllUnreadCount begin");
        String rawQuery = "select sum(message_count - readcount) as unreadcount" +
                " from threads where read = 0 and " + Threads.TYPE + "<>" + MtkThreads.WAPPUSH_THREAD;
        /// M: Log.d(LOG_TAG, "getAllUnreadCount rawQuery = " +rawQuery);
        return db.rawQuery(rawQuery, null);
    }
    /// @}
    /// M: Code analyze 007, fix bug ALPS00255806, when reply a message, use the same
    /// number which receiving message. @{
    private Cursor getSimidListByThread(SQLiteDatabase db, long threadId) {
        String rawQuery = String.format("SELECT DISTINCT sub_id FROM" +
                "(SELECT DISTINCT sub_id FROM sms WHERE thread_id=" + threadId + " AND type=1" +
                " UNION SELECT DISTINCT sub_id FROM pdu WHERE thread_id=" + threadId
                + " AND msg_box=1" + ")");
        MmsProviderLog.dpi(LOG_TAG, "getSimidListByThread begin rawQuery = " + rawQuery);
        return db.rawQuery(rawQuery, null);
    }
    /// @}

    /// M: through threadid to get the recipient number.
    private Cursor getRecipientsNumber(String threadId) {
        String outerQuery = String.format("SELECT recipient_ids FROM threads WHERE _id = "
                + threadId);
        MmsProviderLog.dpi(LOG_TAG, "getRecipientsNumber " + outerQuery);
        return mOpenHelper.getReadableDatabase().rawQuery(outerQuery, EMPTY_STRING_ARRAY);
    }
    /// @}
    /// M: Code analyze 010, fix bug ALPS00280371, mms can't be found out,
    /// change the search uri. TODO: other fix. @{
    private HashMap<String, String> getContactsByNumber(String pattern) {
        Builder builder = PICK_PHONE_EMAIL_FILTER_URI.buildUpon();
        builder.appendPath(pattern);      /// M:  Builder will encode the query
        MmsProviderLog.dpi(LOG_TAG, "getContactsByNumber uri = " + builder.build().toString());
        Cursor cursor = null;

        /// M: query the related contact numbers and name
        HashMap<String, String> contacts = new HashMap<String, String>();
        long token = Binder.clearCallingIdentity();
        try {
            cursor = getContext().getContentResolver().query(builder.build(),
                new String[] {Phone.DISPLAY_NAME_PRIMARY, Phone.NUMBER}, null, null, "sort_key");
            Log.d(LOG_TAG, "getContactsByNumber getContentResolver query contact 1 cursor "
                    + cursor.getCount());
            while (cursor.moveToNext()) {
                String name = cursor.getString(0);
                String number = getValidNumber(cursor.getString(1));
                MmsProviderLog.dpi(LOG_TAG, "getContactsByNumber number = " + number
                + " name = " + name);
                contacts.put(number, name);
            }
        } catch (IllegalArgumentException ex) {
            Log.d(LOG_TAG, ex.toString());
        } finally {
            if (cursor != null) {
                cursor.close();
            }
            Binder.restoreCallingIdentity(token);
        }
         return contacts;
    }
    /// @}
    /// M: Code analyze 015, fix bug ALPS00280371, ALPS00060027, ALPS00070354,
    /// support for search Chinese characters of recipients. @{
    private String searchContacts(String pattern, HashMap<String, String> contactRes) {
        String in = " IN ";
        String name = null;
        /* query the related thread ids */
        Set<Long> threadIds = new HashSet<Long>();
        Cursor cursor = mOpenHelper.getReadableDatabase().rawQuery(
                "SELECT " + Threads._ID + "," + Threads.RECIPIENT_IDS + " FROM threads", null);
        /// M: Query all recipients in table canonical_addresses, use result cursor
        /// instead of querying each time
        Cursor addrCursor = mOpenHelper.getReadableDatabase().rawQuery(
                "SELECT _id, address FROM canonical_addresses ", null);
        try {
            HashMap<Integer, String> contacts = new HashMap<Integer, String>();
            while (addrCursor.moveToNext()) {
                contacts.put(addrCursor.getInt(0), addrCursor.getString(1));
            }
            while (cursor.moveToNext()) {
                if (TextUtils.isEmpty(cursor.getString(1))) {
                    continue;
                }
                Long threadId = cursor.getLong(0);
//                Set<String> recipients = searchRecipients(cursor.getString(1));
                Set<String> recipients = getRecipientNumbers(cursor.getString(1), contacts);
                for (String recipient : recipients) {
                    if (recipient.toLowerCase().contains(pattern.toLowerCase())) {
                        threadIds.add(threadId);
                        break;
                    }
                   // Log.d(LOG_TAG, "searchContacts cursor recipient " + recipient);
                    name = (String) contactRes.get(recipient);
                    /// M: fix ALPS00446245, some time coming address is +86xxxx,
                    /// but phone book saved number is xxx. So make an enhancement. @{
                    if (name == null) {
                        Set<String> addresses = contactRes.keySet();
                        for (String addr : addresses) {
                            if (PhoneNumberUtils.compare(addr, recipient)) {
                                name = (String) contactRes.get(addr);
                                break;
                            }
                        }
                    }
                    /// @}
                    /// M: fix bug ALPS00498271, Ignore case sensitive : "Test1" contain "test".
                    if (name != null && name.toLowerCase().contains(pattern.toLowerCase())) {
                        threadIds.add(threadId);
                        break;
                    }
                }
            }
        } finally {
            cursor.close();
            addrCursor.close();
        }
        Log.d(LOG_TAG, "searchContacts getContentResolver query recipient");
        /* to IN sql */
        in += threadIds.toString();
        in = in.replace('[', '(');
        in = in.replace(']', ')');
        MmsProviderLog.dpi(LOG_TAG, "searchContacts in = " + in);
        return in;
    }
    /// @}
    /// M: Code analyze 014, fix bug ALPS00231848, search result is wrong sometimes.
    /// match contact number with space. @{
    public static String getValidNumber(String numberOrEmail) {
        if (numberOrEmail == null) {
            return null;
        }
       //Xlog.d(LOG_TAG, "Contact.getValidNumber(): numberOrEmail=" + numberOrEmail);
        String workingNumberOrEmail = new String(numberOrEmail);
        workingNumberOrEmail = workingNumberOrEmail.replaceAll(" ", "").replaceAll("-", "");
        if (numberOrEmail.equals(SELF_ITEM_KEY) || Mms.isEmailAddress(numberOrEmail)) {
            // Log.d(LOG_TAG, "Contact.getValidNumber(): The number is me or Email.");
            return numberOrEmail;
        } else if (PhoneNumberUtils.isWellFormedSmsAddress(workingNumberOrEmail)) {
          //  Log.d(LOG_TAG, "Contact.getValidNumber(): Number without space and '-'
            // is a well-formed number for sending sms.");
            return workingNumberOrEmail;
        } else {
           // Log.d(LOG_TAG, "Contact.getValidNumber(): Unknown formed number");
            workingNumberOrEmail = PhoneNumberUtils.stripSeparators(workingNumberOrEmail);
            workingNumberOrEmail = PhoneNumberUtils.formatNumber(workingNumberOrEmail);
            if (numberOrEmail.equals(workingNumberOrEmail)) {
           //    Log.d(LOG_TAG, "Contact.getValidNumber(): Unknown formed number,
                // but the number without local number formatting is a well-formed number.");
                return PhoneNumberUtils.stripSeparators(workingNumberOrEmail);
            } else {
                return numberOrEmail;
            }
        }
    }
    /// @}

    /// M: improve search performance, just use given cursor to get related recipients
    public static Set<String> getRecipientNumbers(
            String recipientIds, HashMap<Integer, String> contacts) {
        Set<String> recipients = new HashSet<String>();
        if (contacts == null || contacts.size() <= 0) {
            Log.d(LOG_TAG, "getRecipientNumbers contacts is null");
            return recipients;
        }
        String[] recipientIdArray = recipientIds.split(" ");
        for (int i = 0; i < recipientIdArray.length; i++) {
            if (contacts.containsKey(Integer.parseInt(recipientIdArray[i]))) {
                recipients.add(contacts.get(Integer.parseInt(recipientIdArray[i])));
            }
        }
        return recipients;
    }
    /// @}

    private Set<String> searchRecipients(String recipientIds) {
        /* link the recipient ids to the addresses */
        Set<String> recipients = new HashSet<String>();
        if (TextUtils.isEmpty(recipientIds)) {
            return recipients;
        }
        String[] recipientIdArray = recipientIds.split(" ");
        if (recipientIdArray.length > 0) {
            String inString = " IN (" + TextUtils.join(",", recipientIdArray) + ") ";
            /* search the canonical address */
            Cursor cursor = mOpenHelper.getReadableDatabase().rawQuery(
                    "SELECT address FROM canonical_addresses WHERE _id" + inString, null);
            try {
                if (cursor == null || cursor.getCount() == 0) {
                    Log.d(LOG_TAG, "searchRecipients cursor is null");
                    return recipients;
                }
                while (cursor.moveToNext()) {
                    String address = cursor.getString(0);
                    if (!TextUtils.isEmpty(address) && !address.trim().isEmpty()) {
                        recipients.add(address);
                    }
                }
            } finally {
                cursor.close();
            }
        }
        return recipients;
    }
    /// @}

    private long insertCanonicalAddresses(SQLiteOpenHelper openHelper, String refinedAddress) {
        ContentValues contentValues = new ContentValues(1);
        contentValues.put(CanonicalAddressesColumns.ADDRESS, refinedAddress);
        SQLiteDatabase db = mOpenHelper.getWritableDatabase();
        return db.insert("canonical_addresses", CanonicalAddressesColumns.ADDRESS, contentValues);
    }

    /// M: Code analyze 013, new feature, support for wappush. @{
    /**
     * Insert a record for a new wap push thread.
     */
    private void insertWapPushThread(String recipientIds, int numberOfRecipients) {
        ContentValues values = new ContentValues(4);

        long date = System.currentTimeMillis();
        values.put(ThreadsColumns.DATE, date - date % 1000);
        values.put(ThreadsColumns.RECIPIENT_IDS, recipientIds);
        values.put(ThreadsColumns.TYPE, MtkTelephony.MtkThreads.WAPPUSH_THREAD);

        long result = mOpenHelper.getWritableDatabase().insert("threads", null, values);
        MmsProviderLog.dpi(WAPPUSH_TAG, "insertThread: created new thread_id " + result +
                " for recipientIds " + recipientIds);
        Log.w(WAPPUSH_TAG, "insertWapPushThread!");
        /// M: Code analyze 012, fix bug ALPS00262044, not show out unread message
        /// icon after restore messages. notify mms application about unread messages
        /// number after insert operation.
        notifyChange();
    }

    /** M:
     * Return the wappush thread ID for this list of
     * recipients IDs.  If no thread exists with this ID, create
     * one and return it. It should only be called for wappush @{*/
    private synchronized Cursor getWapPushThreadId(List<String> recipients) {
        Set<Long> addressIds = getAddressIds(recipients);
        String recipientIds = "";

        /// M: optimize for size==1, which should be most of the cases
        if (addressIds.size() == 1) {
            for (Long addressId : addressIds) {
                recipientIds = Long.toString(addressId);
            }
        } else {
            recipientIds = getSpaceSeparatedNumbers(getSortedSet(addressIds));
        }

        if (Log.isLoggable(LOG_TAG, Log.VERBOSE)) {
            MmsProviderLog.dpi(LOG_TAG, "getWapPushThreadId: recipientIds (selectionArgs) ="
            + recipientIds);
        }

        String queryString = "SELECT _id FROM threads " + "WHERE type="
                + MtkThreads.WAPPUSH_THREAD + " AND recipient_ids=?";
        String[] selectionArgs = new String[] { recipientIds };
        SQLiteDatabase db = mOpenHelper.getReadableDatabase();
        Cursor cursor = db.rawQuery(queryString, selectionArgs);

        if (cursor.getCount() == 0) {
            cursor.close();

            MmsProviderLog.dpi(LOG_TAG,
            "getWapPushThreadId: create new thread_id for recipients " + recipients);
            insertWapPushThread(recipientIds, recipients.size());

            db = mOpenHelper.getReadableDatabase();  /// M: In case insertThread closed it
            cursor = db.rawQuery(queryString, selectionArgs);
        }

        if (cursor.getCount() > 1) {
            Log.w(LOG_TAG, "getWapPushThreadId: why is cursorCount=" + cursor.getCount());
        }

        return cursor;
    }
    /// @}

    /// M: Return the cellbroadcast thread ID. It should only be called for cellbroadcast. @{
    private void insertCBThread(String recipientIds, int numberOfRecipients) {
        ContentValues values = new ContentValues(4);

        long date = System.currentTimeMillis();
        values.put(ThreadsColumns.DATE, date - date % 1000);
        values.put(ThreadsColumns.RECIPIENT_IDS, recipientIds);
        values.put(ThreadsColumns.TYPE, MtkThreads.CELL_BROADCAST_THREAD);

        long result = mOpenHelper.getWritableDatabase().insert("threads", null, values);
        MmsProviderLog.dpi(LOG_TAG, "insertThread: created new thread_id " + result +
                " for recipientIds " + recipientIds);
        Log.w(LOG_TAG, "insertCBThread!");
        /// M: Code analyze 012, fix bug ALPS00262044, not show out unread message
        /// icon after restore messages. notify mms application about unread messages
        /// number after insert operation.
        notifyChange();
    }

    private synchronized Cursor getCBThreadId(List<String> recipients) {
        Set<Long> addressIds = getAddressIds(recipients);
        String recipientIds = "";

        /// M: optimize for size==1, which should be most of the cases
        if (addressIds.size() == 1) {
            for (Long addressId : addressIds) {
                recipientIds = Long.toString(addressId);
            }
        } else {
            recipientIds = getSpaceSeparatedNumbers(getSortedSet(addressIds));
        }

        if (Log.isLoggable(LOG_TAG, Log.VERBOSE)) {
            MmsProviderLog.dpi(LOG_TAG,
            "getCBThreadId: recipientIds (selectionArgs) =" + recipientIds);
        }

        String queryString = "SELECT _id FROM threads " + "WHERE type="
                + MtkThreads.CELL_BROADCAST_THREAD + " AND recipient_ids=?";
        String[] selectionArgs = new String[] { recipientIds };
        SQLiteDatabase db = mOpenHelper.getReadableDatabase();
        Cursor cursor = db.rawQuery(queryString, selectionArgs);

        if (cursor.getCount() == 0) {
            cursor.close();

            MmsProviderLog.dpi(LOG_TAG,
            "getCBThreadId: create new thread_id for recipients " + recipients);
            insertCBThread(recipientIds, recipients.size());

            db = mOpenHelper.getReadableDatabase();  /// M: In case insertThread closed it
            cursor = db.rawQuery(queryString, selectionArgs);
        }

        if (cursor.getCount() > 1) {
            Log.w(LOG_TAG, "getCBThreadId: why is cursorCount=" + cursor.getCount());
        }

        return cursor;
    }
    /// @}

    /// M: Code analyze 012, fix bug ALPS00262044, not show out unread message
    /// icon after restore messages. notify mms application about unread messages
    /// number after insert operation. @{
    private void notifyChange() {
        Log.i(LOG_TAG, "notifyChange");
        getContext().getContentResolver().notifyChange(MmsSms.CONTENT_URI, null);
        notifyUnreadMessageNumberChanged(getContext());
    }
    private static int getUnreadMessageNumber(Context context) {
        int threadsUnreadCount = 0;
        String threadsQuery = "select sum(message_count - readcount) as unreadcount " +
                "from threads where read = 0 and " + Threads.TYPE + "<>" + MtkThreads.WAPPUSH_THREAD;
        Cursor c = MmsSmsDatabaseHelper.getInstanceForCe(context)
                .getReadableDatabase().rawQuery(threadsQuery, null);
        if (c != null) {
            try {
                if (c.moveToFirst()) {
                    threadsUnreadCount = c.getInt(0);
                    Log.d(LOG_TAG, "get threads unread message count = " + threadsUnreadCount);
                }
            } finally {
                c.close();
            }
        } else {
            Log.d(LOG_TAG, "can not get unread message count.");
        }
        return threadsUnreadCount;
    }

    public static void broadcastUnreadMessageNumber(Context context, int unreadMsgNumber) {
//        Intent intent = new Intent();
//        intent.setAction(Intent.ACTION_UNREAD_CHANGED);
//        intent.putExtra(Intent.EXTRA_UNREAD_NUMBER, unreadMsgNumber);
//        intent.putExtra(Intent.EXTRA_UNREAD_COMPONENT,
//                new ComponentName("com.android.mms", "com.android.mms.ui.BootActivity"));
        // unread changed feature is for launcher. It is phased out. So mark first.
        //context.sendBroadcast(intent);
    }

    public static void recordUnreadMessageNumberToSys(Context context, int unreadMsgNumber) {
        try {
            android.provider.Settings.System.putInt(context.getContentResolver(),
                "com_android_mms_mtk_unread", unreadMsgNumber);
        } catch (SecurityException e) {
            Log.e(LOG_TAG, "SecurityException: ", e);
        }
    }

    public static void notifyUnreadMessageNumberChanged(final Context context) {
        int unreadNumber = getUnreadMessageNumber(context);
        recordUnreadMessageNumberToSys(context, unreadNumber);
        broadcastUnreadMessageNumber(context, unreadNumber);

    }

    /// M: Code analyze 017, fix bug ALPS00268161, new received message will be delete
    /// while deleting older messages. @{
    /// M: Delete all the conversation
    private int deleteAllConversation(
            SQLiteDatabase db, Uri uri, String selection, String[] selectionArgs) {
        String threadId = uri.getLastPathSegment();
        String smsId = uri.getQueryParameter("smsId");
        String mmsId = uri.getQueryParameter("mmsId");
        Log.d(LOG_TAG, "deleteAllConversation get max message smsId = "
                + smsId + " mmsId =" + mmsId);
        String finalSmsSelection;
        String finalMmsSelection;
        /// M: Locked words do not delete. @{
        String exceptLockSmsWords = "";
        String exceptFromPart = "";
        String exceptFromPdu = "";
        if (selection != null && selection.contains("locked=0")) {
            exceptLockSmsWords = " AND source_id not in (select _id from sms where locked=1)";
            exceptFromPart = " AND source_id not in (select _id from part where mid in "
                    + "(select _id from pdu where locked=1))";
            exceptFromPdu = " AND source_id not in (select _id from pdu where locked=1)";
        }
        /// @}
        if (smsId != null) {
            finalSmsSelection = concatSelections(selection, "_id<=" + smsId);
            db.execSQL("DELETE FROM words WHERE table_to_use=1 AND source_id<=" + smsId
                    + exceptLockSmsWords + ";");
        } else {
            finalSmsSelection = selection;
            db.execSQL("DELETE FROM words WHERE table_to_use=1" + exceptLockSmsWords + ";");
        }
        if (mmsId != null) {
            /// Delete words which id from part or pdu. @{
            String deleteMmsWords = "DELETE FROM words WHERE (table_to_use=2 AND source_id<="
                    + " (SELECT max(_id) FROM part WHERE mid<=" + mmsId + ")" + exceptFromPart
                    + ") OR (table_to_use=" + MmsProvider.TABLE_TO_USE_SUBJECT
                    + " AND source_id<=" + mmsId + exceptFromPdu + ")";
            /// @}
            finalMmsSelection = concatSelections(selection, "_id<=" + mmsId);
            db.execSQL(deleteMmsWords);
        } else {
            finalMmsSelection = selection;
            db.execSQL("DELETE FROM words WHERE (table_to_use=2" + exceptFromPart
                    + ") OR (table_to_use=" + MmsProvider.TABLE_TO_USE_SUBJECT
                    + exceptFromPdu + ");");
        }

        return MmsProvider.deleteMessages(getContext(), db,
                finalMmsSelection, selectionArgs, uri, false)
                + SmsProvider.deleteMessages(db, finalSmsSelection, selectionArgs)
                + db.delete("cellbroadcast", selection, selectionArgs);
    }
    /// @}

    /// M: Add a new query method to query more columns from table thread_settings @}
    /**
     * Return existing threads in the database.
     * this method will query thread_settings data too.
     */
    private Cursor getSimpleConversationsExtend(String[] projection, String selection,
            String[] selectionArgs, String sortOrder) {
        // change the _id to threads._id
        for (int i = 0; i < projection.length; i++) {
            if (projection[i].equals("_id")) {
                projection[i] = "threads._id";
            }
        }
        selection = concatSelections(selection, "threads._id=thread_settings.thread_id");
        MmsProviderLog.dpi(LOG_TAG, "extend query selection:" + selection);
        return mOpenHelper.getReadableDatabase().query("threads,thread_settings", projection,
                selection, selectionArgs, null, null, " date DESC");
    }

    @Override
    public ParcelFileDescriptor openFile(Uri uri, String mode) throws FileNotFoundException {
        return openFileHelper(uri, mode);
    }
    /// @}

    /// M: Add for Chinese subject search @{
    public static String toIsoString(byte[] bytes) {
        try {
            return new String(bytes, "iso-8859-1");
        } catch (UnsupportedEncodingException e) {
            // Impossible to reach here!
            Log.e(LOG_TAG, "ISO_8859_1 must be supported!", e);
            return "";
        /// M:Code analyze 007,add for ALPS00100954, MO MMS can not send out more than 5min @{
        } catch (NullPointerException e) {
            return "";
        }
        /// @}
    }
    /// @}

    protected static final int STATIC_KEY_BUFFER_MAXIMUM_LENGTH = 7;
    // Invert and truncate to seven characters the phoneNumber so that we
    // can use it as the key in a hashtable.  We keep a mapping of this
    // key to a list of all numbers which have the same key.
    protected static String key(String phoneNumber, CharBuffer keyBuffer) {
        keyBuffer.clear();
        keyBuffer.mark();
        int position = phoneNumber.length();
        int resultCount = 0;
        while (--position >= 0) {
            char c = phoneNumber.charAt(position);
            if (PhoneNumberUtils.isDialable(c)) {
                keyBuffer.put(c);
                if (++resultCount == STATIC_KEY_BUFFER_MAXIMUM_LENGTH) {
                    break;
                }
            }
        }
        keyBuffer.reset();
        if (resultCount > 0) {
            return keyBuffer.toString();
        } else {
            // there were no usable digits in the input phoneNumber
            return phoneNumber;
        }
    }

    /// M: Get db size, for SELinux enhancement, mms can not get the db size directly @{
    private Uri getDatabaseSize(Uri uri) {
         File db = new File(DB_PATH);
        long size = db.length();
        return uri.buildUpon().appendQueryParameter("size", String.valueOf(size)).build();
    }
    /// @}

    @Override
    public Bundle call(String method, String arg, Bundle extras) {
        if (METHOD_IS_RESTORING.equals(method)) {
            Bundle result = new Bundle();
            result.putBoolean(IS_RESTORING_KEY, TelephonyBackupAgent.getIsRestoring());
            return result;
        }
        Log.w(LOG_TAG, "Ignored unsupported " + method + " call");
        return null;
    }

    public static final class MmsProviderLog {
        static private boolean  piLoggable =
                       !("user".equals(SystemProperties.get("ro.build.type", "user"))) ;
        static private boolean  hideSensInEng = true;

        public static void vpi(String tag, String msg) {
            if (piLoggable) {
                if (hideSensInEng == true) {
                    msg = partialHideNumber(msg);
                }
                Log.v(tag, msg);
            }
        }

        public static void epi(String tag, String msg) {
            if (piLoggable) {
                if (hideSensInEng == true) {
                    msg = partialHideNumber(msg);
                }
                Log.e(tag, msg);
            }
        }

        public static void epi(String tag, String msg, Throwable tr) {
            if (piLoggable) {
                if (hideSensInEng == true) {
                    msg = partialHideNumber(msg);
                }
                Log.e(tag, msg, tr);
            }
        }

        public static void ipi(String tag, String msg) {
            if (piLoggable) {
                if (hideSensInEng == true) {
                    msg = partialHideNumber(msg);
                }
                Log.i(tag, msg);
            }
        }

        public static void dpi(String tag, String msg) {
            if (piLoggable) {
                if (hideSensInEng == true) {
                    msg = partialHideNumber(msg);
                }
                Log.d(tag, msg);
            }
        }

        public static void dpi(String tag, String msg, Throwable tr) {
            if (piLoggable) {
                if (hideSensInEng == true) {
                    msg = partialHideNumber(msg);
                }
                Log.d(tag, msg, tr);
            }
        }

        public static void wpi(String tag, String msg) {
            if (piLoggable) {
                if (hideSensInEng == true) {
                    msg = partialHideNumber(msg);
                }
                Log.w(tag, msg);
            }
        }

        public static void wpi(String tag, String msg, Throwable tr) {
            if (piLoggable) {
                if (hideSensInEng == true) {
                    msg = partialHideNumber(msg);
                }
                Log.w(tag, msg, tr);
            }
        }

        private static String partialHideNumber(String logString) {
        if (logString == null) {
            return "";
        }

        final String sLogNumPattern = "(?<=[\\d]{2})\\d(?=[\\d]{2})";
        final String sLogEmailPattern = "(?<=@)\\w+(?=.)";
        String newLogString = logString.replaceAll(sLogNumPattern, "*");
        newLogString = newLogString.replaceAll(sLogEmailPattern, "*****");

        return newLogString;
    }
   }
}
