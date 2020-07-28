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

import static android.provider.Telephony.RcsColumns.IS_RCS_TABLE_SCHEMA_CODE_COMPLETE;

import android.content.BroadcastReceiver;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.database.Cursor;
import android.database.DatabaseErrorHandler;
import android.database.DefaultDatabaseErrorHandler;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteException;
import android.database.sqlite.SQLiteOpenHelper;
import android.os.storage.StorageManager;
import android.os.SystemProperties;
import android.preference.PreferenceManager;
import android.provider.BaseColumns;
import android.provider.Telephony;
import android.provider.Telephony.Mms;
import android.provider.Telephony.Mms.Addr;
import android.provider.Telephony.Mms.Part;
import android.provider.Telephony.Mms.Rate;
import android.provider.Telephony.MmsSms;
import android.provider.Telephony.MmsSms.PendingMessages;
import android.provider.Telephony.Sms;
import android.provider.Telephony.Sms.Intents;
import android.provider.Telephony.Threads;
import android.telephony.SubscriptionManager;
import android.text.TextUtils;
import android.util.Log;
import android.util.Slog;

import com.android.internal.annotations.VisibleForTesting;
import com.android.internal.telephony.PhoneFactory;
import com.google.android.mms.pdu.EncodedStringValue;
import com.google.android.mms.pdu.PduHeaders;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;
import java.util.concurrent.atomic.AtomicBoolean;

import mediatek.telephony.MtkTelephony;
import mediatek.telephony.MtkTelephony.MtkSms;
import mediatek.telephony.MtkTelephony.MtkMms;
import mediatek.telephony.MtkTelephony.MtkThreadSettings;
import mediatek.telephony.MtkTelephony.WapPush;
import mediatek.telephony.MtkTelephony.MtkMwi;
import mediatek.telephony.MtkTelephony.MtkThreads;
import mediatek.telephony.MtkTelephony.MtkMmsSms;
import mediatek.telephony.MtkTelephony.SmsCb;

/**
 * A {@link SQLiteOpenHelper} that handles DB management of SMS and MMS tables.
 *
 * From N, SMS and MMS tables are split into two groups with different levels of encryption.
 *   - the raw table, which lives inside DE(Device Encrypted) storage.
 *   - all other tables, which lives under CE(Credential Encrypted) storage.
 *
 * All tables are created by this class in the same database that can live either in DE or CE
 * storage. But not all tables in the same database should be used. Only DE tables should be used
 * in the database created in DE and only CE tables should be used in the database created in CE.
 * The only exception is a non-FBE device migrating from M to N, in which case the DE and CE tables
 * will actually live inside the same storage/database.
 *
 * This class provides methods to create instances that manage databases in different storage.
 * It's the responsibility of the clients of this class to make sure the right instance is
 * used to access tables that are supposed to live inside the intended storage.
 */
public class MmsSmsDatabaseHelper extends SQLiteOpenHelper {
    private static final String TAG = "Mms/Provider/MmsSmsDatabaseHelper";

    static final String TABLE_CELLBROADCAST = "cellbroadcast";
    static final String TABLE_WAPPUSH = "wappush";

    private static final boolean MTK_WAPPUSH_SUPPORT = true; //SystemProperties.get(
//            "ro.vendor.mtk_wappush_support").equals("1");


    /// M: ALPS00514953, Update thread date by the latest sms date @{
    private static final String SMS_UPDATE_THREAD_DATE =
            " UPDATE threads" +
            "  SET" +
            "  date =" +
            "    (SELECT date FROM" +
            "        (SELECT date * 1000 AS date, thread_id FROM pdu " +
            "         WHERE (m_type=132 OR m_type=130 OR m_type=128) AND (thread_id = "
            + "new." + Sms.THREAD_ID + ") " +
            "         UNION SELECT date, thread_id FROM sms " +
            "         WHERE thread_id = " + "new." + Sms.THREAD_ID + ") " +
            "     WHERE thread_id = " + "new." + Sms.THREAD_ID + " ORDER BY date DESC LIMIT 1) " +
            "  WHERE threads._id = " + "new." + Sms.THREAD_ID + "; ";

    private static final String SMS_UPDATE_THREAD_DATE_SENT =
            " UPDATE threads" +
            "  SET" +
            "  date_sent =" +
            "    (SELECT date_sent FROM" +
            "        (SELECT date_sent * 1000 AS date_sent, date * 1000 as date, " +
            "thread_id FROM pdu" +
            "         WHERE (m_type=132 OR m_type=130 OR m_type=128) AND (thread_id = "
            + "new." + Sms.THREAD_ID + ") " +
            "         UNION SELECT date_sent, date, thread_id FROM sms " +
            "         WHERE thread_id = " + "new." + Sms.THREAD_ID + ") " +
            "     WHERE thread_id = " + "new." + Sms.THREAD_ID + " ORDER BY date DESC LIMIT 1) " +
            "  WHERE threads._id = " + "new." + Sms.THREAD_ID + "; ";


    private static final String SMS_UPDATE_THREAD_SNIPPET =
            " UPDATE threads" +
            "  SET" +
            "  snippet =" +
            "    (SELECT snippet FROM" +
            "        (SELECT sub AS snippet, thread_id, date * 1000 AS date FROM pdu " +
            "         WHERE (m_type=132 OR m_type=130 OR m_type=128) AND (thread_id = " + "new."
            + Sms.THREAD_ID + ") " +
            "         UNION SELECT body AS snippet, thread_id, date FROM sms " +
            "         WHERE thread_id = " + "new." + Sms.THREAD_ID + " ORDER BY date DESC LIMIT 1 "
            + ") " +
            "     WHERE thread_id = " + "new." + Sms.THREAD_ID + " ) " +
            "  WHERE threads._id = " + "new." + Sms.THREAD_ID + "; ";

    private static final String SMS_UPDATE_THREAD_SNIPPET_CS =
            " UPDATE threads" +
            "  SET" +
            "  snippet_cs =" +
            "    (SELECT snippet_cs FROM" +
            "        (SELECT sub_cs AS snippet_cs, thread_id, date * 1000 AS date FROM pdu " +
            "         WHERE (m_type=132 OR m_type=130 OR m_type=128) AND (thread_id = " + "new."
            + Sms.THREAD_ID + ") " +
            "         UNION SELECT 0 AS snippet_cs, thread_id, date FROM sms " +
            "         WHERE thread_id = " + "new." + Sms.THREAD_ID + " ORDER BY date DESC LIMIT 1 "
            + ") " +
            "     WHERE thread_id = " + "new." + Sms.THREAD_ID + " ) " +
            "  WHERE threads._id = " + "new." + Sms.THREAD_ID + "; ";

    /// M: Add for ip message @{
    // this need be the first because it is used by later final consts.
    private static final String UPDATE_THREAD_READ_COUNT =
            "  UPDATE threads SET readcount = " +
            "  (SELECT count(_id)FROM " +
            "  (SELECT DISTINCT date * 1 AS normalized_date, _id, read FROM sms " +
            "    WHERE ((read=1) AND thread_id = new.thread_id AND (type != 3)) " +
            "  UNION SELECT DISTINCT date * 1000 AS normalized_date, pdu._id, read " +
            "  FROM pdu LEFT JOIN pending_msgs ON pdu._id = pending_msgs.msg_id " +
            "  WHERE ((read=1) AND thread_id = new.thread_id AND msg_box != 3 AND (msg_box != 3 " +
            "        AND (m_type = 128 OR m_type = 132 OR m_type = 130)))" +
            "   UNION SELECT DISTINCT date * 1 AS normalized_date, _id, read FROM cellbroadcast " +
            "   WHERE ((read=1) AND thread_id = new.thread_id) ORDER BY normalized_date ASC))  " +
            "  WHERE threads._id = new.thread_id; ";

    private static final String UPDATE_THREAD_READ_COUNT_OLD =
        "  UPDATE threads SET readcount = " +
        "  (SELECT count(_id) FROM " +
        "  (SELECT DISTINCT date * 1 AS normalized_date, _id, read FROM sms " +
        "    WHERE ((read=1) AND thread_id = old.thread_id AND (type != 3)) " +
        "  UNION SELECT DISTINCT date * 1000 AS normalized_date, pdu._id, read " +
        "  FROM pdu LEFT JOIN pending_msgs ON pdu._id = pending_msgs.msg_id " +
        "  WHERE ((read=1) AND thread_id = old.thread_id AND msg_box != 3 AND (msg_box != 3 " +
        "        AND (m_type = 128 OR m_type = 132 OR m_type = 130)))" +
        "   UNION SELECT DISTINCT date * 1 AS normalized_date, _id, read FROM cellbroadcast " +
        "   WHERE ((read=1) AND thread_id = old.thread_id) ORDER BY normalized_date ASC))  " +
        "  WHERE threads._id = old.thread_id; ";
    /// @}

//    private static final String SMS_UPDATE_THREAD_READ_BODY =
//            "  UPDATE threads SET read = " +
//            "    CASE (SELECT COUNT(*)" +
//            "          FROM sms" +
//            "          WHERE " + Sms.READ + " = 0" +
//            "            AND " + Sms.THREAD_ID + " = threads._id)" +
//            "      WHEN 0 THEN 1" +
//            "      ELSE 0" +
//            "    END" +
//            "  WHERE threads._id = new." + Sms.THREAD_ID + "; ";

    private static final String SMS_UPDATE_THREAD_READ_BODY =
                        "  UPDATE threads SET read = " +
                        "    CASE ((SELECT COUNT(*)" +
                        "          FROM sms" +
                        "          WHERE " + Sms.READ + " = 0" +
                        "            AND " + Sms.THREAD_ID + " = threads._id) + " +
                        "         (SELECT COUNT(*)" +
                        "          FROM " + MmsProvider.TABLE_PDU +
                        "          WHERE " + Mms.READ + " = 0" +
                        "            AND " + Mms.THREAD_ID + " = threads._id " +
                        "            AND (m_type=132 OR m_type=130 OR m_type=128))) " +
                        "      WHEN 0 THEN 1" +
                        "      ELSE 0" +
                        "    END" +
                        "  WHERE threads._id = new." + Sms.THREAD_ID + "; ";

    private static final String UPDATE_THREAD_COUNT_ON_NEW =
                        "  UPDATE threads SET message_count = " +
                        "     (SELECT COUNT(sms._id) FROM sms LEFT JOIN threads " +
                        "      ON threads._id = " + Sms.THREAD_ID +
                        "      WHERE " + Sms.THREAD_ID + " = new.thread_id" +
                        "        AND sms." + Sms.TYPE + " != 3) + " +
                        "     (SELECT COUNT(pdu._id) FROM pdu LEFT JOIN threads " +
                        "      ON threads._id = " + Mms.THREAD_ID +
                        "      WHERE " + Mms.THREAD_ID + " = new.thread_id" +
                        "        AND (m_type=132 OR m_type=130 OR m_type=128)" +
                        "        AND " + Mms.MESSAGE_BOX + " != 3) " +
                        "  WHERE threads._id = new.thread_id; ";

    private static final String UPDATE_THREAD_COUNT_ON_OLD =
                        "  UPDATE threads SET message_count = " +
                        "     (SELECT COUNT(sms._id) FROM sms LEFT JOIN threads " +
                        "      ON threads._id = " + Sms.THREAD_ID +
                        "      WHERE " + Sms.THREAD_ID + " = old.thread_id" +
                        "        AND sms." + Sms.TYPE + " != 3) + " +
                        "     (SELECT COUNT(pdu._id) FROM pdu LEFT JOIN threads " +
                        "      ON threads._id = " + Mms.THREAD_ID +
                        "      WHERE " + Mms.THREAD_ID + " = old.thread_id" +
                        "        AND (m_type=132 OR m_type=130 OR m_type=128)" +
                        "        AND " + Mms.MESSAGE_BOX + " != 3) " +
                        "  WHERE threads._id = old.thread_id; ";

    /// M: fix ALPS01514772, update columns snippet/date to the latest.
    private static final String SMS_UPDATE_THREAD_DATE_SNIPPET_COUNT_ON_UPDATE =
            "BEGIN" +
            /*"  UPDATE threads SET" +
            /// M: Code analyze 001, fix bug ALPS00245345, when restore messages,
            /// thread time should be the latest sms' time, not now.
            "    date = new." + Sms.DATE + ", " +
            "    date_sent = new." + Sms.DATE_SENT + ", " +
            "    snippet = new." + Sms.BODY + ", " +
            "    snippet_cs = 0" +
            "  WHERE threads._id = new." + Sms.THREAD_ID + "; " +*/
            SMS_UPDATE_THREAD_DATE +
            SMS_UPDATE_THREAD_DATE_SENT +
            SMS_UPDATE_THREAD_SNIPPET +
            SMS_UPDATE_THREAD_SNIPPET_CS +
            UPDATE_THREAD_COUNT_ON_NEW +
            SMS_UPDATE_THREAD_READ_BODY +
            UPDATE_THREAD_READ_COUNT +
            "END;";

    private static final String SMS_UPDATE_THREAD_DATE_SNIPPET_COUNT_ON_INSERT =
            "BEGIN" +
            /*"  UPDATE threads SET" +
            "    snippet = new." + Sms.BODY + ", " +
            "    snippet_cs = 0" +
            "  WHERE threads._id = new." + Sms.THREAD_ID + "; " +*/
            SMS_UPDATE_THREAD_DATE +
            SMS_UPDATE_THREAD_DATE_SENT +
            SMS_UPDATE_THREAD_SNIPPET +
            SMS_UPDATE_THREAD_SNIPPET_CS +
            UPDATE_THREAD_COUNT_ON_NEW +
            SMS_UPDATE_THREAD_READ_BODY +
            UPDATE_THREAD_READ_COUNT +
            "END;";
    /// @}

    private static final String PDU_UPDATE_THREAD_CONSTRAINTS =
                        "  WHEN new." + Mms.MESSAGE_TYPE + "=" +
                        PduHeaders.MESSAGE_TYPE_RETRIEVE_CONF +
                        "    OR new." + Mms.MESSAGE_TYPE + "=" +
                        PduHeaders.MESSAGE_TYPE_NOTIFICATION_IND +
                        "    OR new." + Mms.MESSAGE_TYPE + "=" +
                        PduHeaders.MESSAGE_TYPE_SEND_REQ + " ";

    // When looking in the pdu table for unread messages, only count messages that
    // are displayed to the user. The constants are defined in PduHeaders and could be used
    // here, but the string "(m_type=132 OR m_type=130 OR m_type=128)" is used throughout this
    // file and so it is used here to be consistent.
    //     m_type=128   = MESSAGE_TYPE_SEND_REQ
    //     m_type=130   = MESSAGE_TYPE_NOTIFICATION_IND
    //     m_type=132   = MESSAGE_TYPE_RETRIEVE_CONF
//    private static final String PDU_UPDATE_THREAD_READ_BODY =
//            "  UPDATE threads SET read = " +
//            "    CASE (SELECT COUNT(*)" +
//            "          FROM " + MmsProvider.TABLE_PDU +
//            "          WHERE " + Mms.READ + " = 0" +
//            "            AND " + Mms.THREAD_ID + " = threads._id " +
//            "            AND (m_type=132 OR m_type=130 OR m_type=128)) " +
//            "      WHEN 0 THEN 1" +
//            "      ELSE 0" +
//            "    END" +
//            "  WHERE threads._id = new." + Mms.THREAD_ID + "; ";
    private static final String PDU_UPDATE_THREAD_READ_BODY =
        "  UPDATE threads SET read = " +
        "    CASE (SELECT COUNT(*)" +
        "          FROM " + MmsProvider.TABLE_PDU +
        "          WHERE " + Mms.READ + " = 0" +
        "            AND " + Mms.THREAD_ID + " = threads._id " +
        "            AND (m_type=132 OR m_type=130 OR m_type=128)) + " +
        "         (SELECT COUNT(*)" +
        "          FROM sms" +
        "          WHERE " + Sms.READ + " = 0" +
        "            AND " + Sms.THREAD_ID + " = threads._id)" +
        "      WHEN 0 THEN 1" +
        "      ELSE 0" +
        "    END" +
        "  WHERE threads._id = new." + Mms.THREAD_ID + "; ";

    private static final String PDU_UPDATE_THREAD_DATE_SNIPPET_COUNT_ON_UPDATE =
            " BEGIN " +
//            "  UPDATE threads SET" +
//            "    date = (strftime('%s','now') * 1000), " +
//            "    snippet = new." + Mms.SUBJECT + ", " +
//            "    snippet_cs = new." + Mms.SUBJECT_CHARSET +
//            /// M: Code analyze 002, fix bug, only update messages which are
              /// displayed to the user.
//            "  WHERE (new.m_type=132 OR new.m_type=130 OR new.m_type=128)"
//            " AND threads._id = new." + Mms.THREAD_ID + "; " +
            SMS_UPDATE_THREAD_DATE +
            SMS_UPDATE_THREAD_SNIPPET +
            SMS_UPDATE_THREAD_SNIPPET_CS +
            SMS_UPDATE_THREAD_DATE_SENT +
            UPDATE_THREAD_COUNT_ON_NEW +
            PDU_UPDATE_THREAD_READ_BODY +
            UPDATE_THREAD_READ_COUNT +
            " END;";

    /// M: Code analyze 003, fix bug ALPS00301106, when restore MMS, already read MMS status
    /// shouldn't be changed into un-read, and thread time should be the latest pdu's time, not now.
    /// @{
    private static final String PDU_UPDATE_THREAD_DATE =
            "UPDATE threads" +
            "  SET" +
            "  date =" +
            "    (SELECT date FROM" +
            "        (SELECT date * 1000 AS date, thread_id FROM pdu " +
            "         WHERE (new.m_type=132 OR new.m_type=130 OR new.m_type=128) AND (thread_id = "
            + "new." + Mms.THREAD_ID + ") " +
            "         UNION SELECT date, thread_id FROM sms " +
            "         WHERE thread_id = " + "new." + Mms.THREAD_ID + ") " +
            "     WHERE thread_id = " + "new." + Mms.THREAD_ID + " ORDER BY date DESC LIMIT 1) " +
            "  WHERE threads._id = " + "new." + Mms.THREAD_ID + ";";

    // M: use trigger to update thread status {
    private static final String PDU_UPDATE_THREAD_STATUS =
                        "  UPDATE threads SET status = 0 " +
                        "  WHERE (new.m_type=132 OR new.m_type=130 OR new.m_type=128) " +
                        "AND threads._id=new.thread_id; ";
    // @}

    private static final String PDU_UPDATE_THREAD_DATE_SNIPPET_COUNT_ON_INSERT =
            "  BEGIN  " +
//            "  UPDATE threads SET" +
//            "    snippet = new." + Mms.SUBJECT + ", " +
//            "    snippet_cs = new." + Mms.SUBJECT_CHARSET +
//            "  WHERE (new.m_type=132 OR new.m_type=130 OR new.m_type=128) AND threads._id = new."
//            + Mms.THREAD_ID + "; " +
            PDU_UPDATE_THREAD_DATE +
            SMS_UPDATE_THREAD_SNIPPET +
            SMS_UPDATE_THREAD_SNIPPET_CS +
            SMS_UPDATE_THREAD_DATE_SENT +
            UPDATE_THREAD_COUNT_ON_NEW +
            PDU_UPDATE_THREAD_READ_BODY +
            UPDATE_THREAD_READ_COUNT +
            PDU_UPDATE_THREAD_STATUS +
            "  END;";
    /// @}
    private static final String UPDATE_THREAD_SNIPPET_SNIPPET_CS_ON_DELETE =
            "  UPDATE threads SET snippet = " +
            "   (SELECT snippet FROM" +
            /// M: Code analyze 002, fix bug, only update messages which are displayed to the user.
            "     (SELECT date * 1000 AS date, sub AS snippet, thread_id FROM pdu " +
            "WHERE m_type=132 OR m_type=130 OR m_type=128" +
            "      UNION SELECT date, body AS snippet, thread_id FROM sms)" +
            "    WHERE thread_id = OLD.thread_id ORDER BY date DESC LIMIT 1) " +
            "  WHERE threads._id = OLD.thread_id; " +
            "  UPDATE threads SET snippet_cs = " +
            "   (SELECT snippet_cs FROM" +
            /// M: Code analyze 002, fix bug, only update messages which are displayed to the user.
            "     (SELECT date * 1000 AS date, sub_cs AS snippet_cs, thread_id FROM pdu " +
            "WHERE m_type=132 OR m_type=130 OR m_type=128" +
            "      UNION SELECT date, 0 AS snippet_cs, thread_id FROM sms)" +
            "    WHERE thread_id = OLD.thread_id ORDER BY date DESC LIMIT 1) " +
            "  WHERE threads._id = OLD.thread_id; " +
            "  UPDATE threads SET date_sent = " +
            "   (SELECT date_sent FROM" +
            "     (SELECT date_sent * 1000 as date_sent, date * 1000 AS date, thread_id FROM pdu " +
            "WHERE m_type=132 OR m_type=130 OR m_type=128" +
            "      UNION SELECT date_sent, date, thread_id FROM sms)" +
            "    WHERE thread_id = OLD.thread_id ORDER BY date DESC LIMIT 1) " +
            "  WHERE threads._id = OLD.thread_id; "
            ;

    // When a part is inserted, if it is not text/plain or application/smil
    // (which both can exist with text-only MMSes), then there is an attachment.
    // Set has_attachment=1 in the threads table for the thread in question.
    private static final String PART_UPDATE_THREADS_ON_INSERT_TRIGGER =
                        "CREATE TRIGGER update_threads_on_insert_part " +
                        " AFTER INSERT ON part " +
                        " WHEN new.ct != 'text/plain' AND new.ct != 'application/smil' " +
                        " BEGIN " +
                        "  UPDATE threads SET has_attachment=1 WHERE _id IN " +
                        "   (SELECT pdu.thread_id FROM part JOIN pdu ON pdu._id=part.mid " +
                        "     WHERE part._id=new._id LIMIT 1); " +
                        " END";

    // When the 'mid' column in the part table is updated, we need to run the trigger to update
    // the threads table's has_attachment column, if the part is an attachment.
    private static final String PART_UPDATE_THREADS_ON_UPDATE_TRIGGER =
                        "CREATE TRIGGER update_threads_on_update_part " +
                        " AFTER UPDATE of " + Part.MSG_ID + " ON part " +
                        " WHEN new.ct != 'text/plain' AND new.ct != 'application/smil' " +
                        " BEGIN " +
                        "  UPDATE threads SET has_attachment=1 WHERE _id IN " +
                        "   (SELECT pdu.thread_id FROM part JOIN pdu ON pdu._id=part.mid " +
                        "     WHERE part._id=new._id LIMIT 1); " +
                        " END";


    // When a part is deleted (with the same non-text/SMIL constraint as when
    // we set has_attachment), update the threads table for all threads.
    // Unfortunately we cannot update only the thread that the part was
    // attached to, as it is possible that the part has been orphaned and
    // the message it was attached to is already gone.
    /// M: Code analyze 004, fix bug ALPS00295814, update threads consumes too much time
    /// after delete part. @{
    private static final String PART_UPDATE_THREADS_ON_DELETE_TRIGGER =
                        "CREATE TRIGGER update_threads_on_delete_part " +
                        " AFTER DELETE ON part " +
                        " WHEN old.ct != 'text/plain' AND old.ct != 'application/smil' " +
                        " BEGIN " +
                        "  UPDATE threads SET has_attachment = " +
                        "   CASE " +
                        "    (SELECT COUNT(*) FROM part JOIN pdu " +
                        "     ON pdu._id=old.mid AND part.mid=pdu._id " +
                        "     WHERE part.ct != 'text/plain' AND part.ct != 'application/smil') " +
                        "   WHEN 0 THEN 0 " +
                        "   ELSE 1 " +
                        "   END " +
                        "   WHERE threads._id=(SELECT thread_id FROM pdu WHERE _id=old.mid); " +
                        " END";

    // When the 'thread_id' column in the pdu table is updated, we need to run the trigger to update
    // the threads table's has_attachment column, if the message has an attachment in 'part' table
    private static final String PDU_UPDATE_THREADS_ON_UPDATE_TRIGGER =
                        "CREATE TRIGGER update_threads_on_update_pdu " +
                        " AFTER UPDATE of thread_id ON pdu " +
                        " BEGIN " +
                        "  UPDATE threads SET has_attachment=1 WHERE _id IN " +
                        "   (SELECT pdu.thread_id FROM part JOIN pdu " +
                        "     WHERE part.ct != 'text/plain' AND part.ct != 'application/smil' " +
                        "     AND part.mid = pdu._id);" +
                        " END";
    /// M: Code analyze 005, new feature, update records which related to cellbroadcast.
    /// TODO: this code is not easy to read, modifying to google's format is suggested. @{
    /// M: CB:update date && snippet && count of threads table.

    private static final String CB_UPDATE_THREAD_READ_BODY = "  UPDATE threads SET read = "
            + "    CASE (SELECT COUNT(*)"
            + "          FROM cellbroadcast"
            + "          WHERE "
            + MtkTelephony.SmsCb.READ
            + " = 0"
            + "            AND "
            + MtkTelephony.SmsCb.THREAD_ID
            + " = threads._id)"
            + "      WHEN 0 THEN 1"
            + "      ELSE 0"
            + "    END"
            + "  WHERE threads._id = new."
            + MtkTelephony.SmsCb.THREAD_ID + "; ";

    private static final String CB_UPDATE_THREAD_READ_BODY_DELETE = "  UPDATE threads SET read = "
            + "    CASE (SELECT COUNT(*)"
            + "          FROM cellbroadcast"
            + "          WHERE "
            + MtkTelephony.SmsCb.READ
            + " = 0"
            + "            AND "
            + MtkTelephony.SmsCb.THREAD_ID
            + " = threads._id)"
            + "      WHEN 0 THEN 1"
            + "      ELSE 0"
            + "    END"
            + "  WHERE threads._id = old."
            + MtkTelephony.SmsCb.THREAD_ID + "; ";

    private static final String CB_UPDATE_THREAD_COUNT_ON_NEW = "UPDATE threads SET message_count ="
            + "     (SELECT COUNT(cellbroadcast._id) FROM cellbroadcast LEFT JOIN threads "
            + "      ON threads._id = "
            + MtkTelephony.SmsCb.THREAD_ID
            + "      WHERE "
            + MtkTelephony.SmsCb.THREAD_ID
            + " = new.thread_id )" + "  WHERE threads._id = new.thread_id; ";

    private static final String CB_UPDATE_THREAD_DATE_SNIPPET_COUNT_ON_UPDATE = "BEGIN"
            + "  UPDATE threads SET"
            + "    date = (strftime('%s','now') * 1000), "
            + "    type= "
            + MtkTelephony.MtkThreads.CELL_BROADCAST_THREAD
            + ", "
            + "    snippet = new."
            + MtkTelephony.SmsCb.BODY
            + " "
            + "  WHERE threads._id = new."
            + MtkTelephony.SmsCb.THREAD_ID
            + "; "
            + CB_UPDATE_THREAD_COUNT_ON_NEW
            + CB_UPDATE_THREAD_READ_BODY
            + UPDATE_THREAD_READ_COUNT
            + "END;";

    private static final String CB_UPDATE_THREAD_COUNT_ON_OLD =
            "  UPDATE threads SET message_count = " +
            "     (SELECT COUNT(cellbroadcast._id) FROM cellbroadcast LEFT JOIN threads " +
            "      ON threads._id = " + MtkTelephony.SmsCb.THREAD_ID +
            "      WHERE " + MtkTelephony.SmsCb.THREAD_ID + " = old.thread_id)" +
            "  WHERE threads._id = old.thread_id; ";

    private static final String CB_UPDATE_THREAD_SNIPPET_ON_DELETE =
            "  UPDATE threads SET snippet = " +
            "   (SELECT body FROM" +
            "     (SELECT date, body, thread_id FROM cellbroadcast)" +
            "    WHERE thread_id = OLD.thread_id ORDER BY date DESC LIMIT 1) " +
            "  WHERE threads._id = OLD.thread_id; ";

    private static final String CB_UPDATE_THREAD_DATE_ON_DELETE =
            "  UPDATE threads SET date = " +
            "   (SELECT date FROM" +
            "     (SELECT date, body, thread_id FROM cellbroadcast)" +
            "    WHERE thread_id = OLD.thread_id ORDER BY date DESC LIMIT 1) " +
            "  WHERE threads._id = OLD.thread_id; ";
    /// @}

    // update the old thread status when update sms thread_id.
    private static final String SMS_UPDATE_OLD_THREAD_DATE =
        "UPDATE threads SET date = "
        + "(SELECT max(date) as date FROM "
        + "(SELECT date * 1000 as date FROM pdu WHERE (thread_id = old.thread_id AND " +
        "(m_type=132 OR m_type=130 OR m_type=128)) "
        + "UNION SELECT date FROM sms WHERE (thread_id = old.thread_id))) "
        + "WHERE _id = old.thread_id;"
        + "UPDATE threads SET date_sent = "
        + "(SELECT max(date_sent) as date_sent FROM "
        + "(SELECT date_sent * 1000 as date_sent FROM pdu WHERE (thread_id = old.thread_id " +
        "AND (m_type=132 OR m_type=130 OR m_type=128)) "
        + "UNION SELECT date_sent FROM sms WHERE (thread_id = old.thread_id))) "
        + "WHERE _id = old.thread_id;";

    private static final String SMS_UPDATE_OLD_THREAD_MESSAGE_COUNT
        = "UPDATE threads SET message_count = "
        + "(SELECT count(_id) FROM (SELECT _id FROM sms WHERE thread_id = old.thread_id " +
        "UNION SELECT _id FROM pdu WHERE thread_id = old.thread_id " +
        "AND (m_type=132 OR m_type=130 OR m_type=128))) "
        + "WHERE _id = old.thread_id;"
        + "UPDATE threads SET readcount = "
        + "(SELECT count(_id) FROM (SELECT _id FROM sms WHERE " +
        "(thread_id = old.thread_id AND (read = 1) AND (type != 3)) " +
        "UNION SELECT _id FROM pdu WHERE (thread_id = old.thread_id " +
        "AND (m_type=132 OR m_type=130 OR m_type=128) AND read = 1 AND msg_box != 3))) "
        + "WHERE _id = old.thread_id;";

    private static final String SMS_UPDATE_OLD_THREAD_SNIPPET =
        "UPDATE threads SET snippet = "
        + "(SELECT snippet FROM (SELECT max(date), snippet FROM "
        + "(SELECT max(date) * 1000 as date, sub as snippet FROM pdu WHERE " +
        "(thread_id = old.thread_id AND (m_type=132 OR m_type=130 OR m_type=128)) "
        + "UNION SELECT max(date), body as snippet FROM sms WHERE (thread_id = old.thread_id)) " +
        "WHERE snippet is not null)) "
        + "WHERE _id = old.thread_id;"
        + "UPDATE threads SET snippet_cs = "
        + "(SELECT snippet_cs FROM (SELECT max(date), snippet_cs FROM "
        + "(SELECT max(date) * 1000 as date, sub_cs as snippet_cs FROM pdu WHERE " +
        "(thread_id = old.thread_id AND (m_type=132 OR m_type=130 OR m_type=128)) "
        + "UNION SELECT max(date), 0 as snippet_cs FROM sms WHERE (thread_id = old.thread_id)))) "
        + "WHERE _id = old.thread_id;";

    private static final String SMS_UPDATE_OLD_THREAD_ERROR =
        "UPDATE threads SET error = "
        + "(SELECT count(_id) FROM (SELECT _id FROM sms WHERE thread_id = old.thread_id " +
        "and type = 5 UNION SELECT pdu._id FROM pdu WHERE thread_id = old.thread_id "
        + "AND st = 135 AND (m_type=132 OR m_type=130 OR m_type=128))) WHERE _id = old.thread_id;";
    /// @}

    private static MmsSmsDatabaseHelper sDeInstance = null;
    private static MmsSmsDatabaseHelper sCeInstance = null;
    private static MmsSmsDatabaseErrorHandler sDbErrorHandler = null;

    private static final String[] BIND_ARGS_NONE = new String[0];

    private static boolean sTriedAutoIncrement = false;
    private static boolean sFakeLowStorageTest = false;     // for testing only

    static final String DATABASE_NAME = "mmssms.db";
    /* M: Code analyze 006, unknown, for database upgrade.
     * M branch: 601000 ~ 601xxx.
     * N branch: begin at 631000.
     */
    static final int DATABASE_VERSION = 670020;
    private static final int IDLE_CONNECTION_TIMEOUT_MS = 30000;

    private final Context mContext;
    private LowStorageMonitor mLowStorageMonitor;

    // SharedPref key used to check if initial create has been done (if onCreate has already been
    // called once)
    private static final String INITIAL_CREATE_DONE = "initial_create_done";
    // cache for INITIAL_CREATE_DONE shared pref so access to it can be avoided when possible
    private static AtomicBoolean sInitialCreateDone = new AtomicBoolean(false);

    /**
     * The primary purpose of this DatabaseErrorHandler is to broadcast an intent on corruption and
     * print a Slog.wtf so database corruption can be caught earlier.
     */
    private static class MmsSmsDatabaseErrorHandler implements DatabaseErrorHandler {
        private DefaultDatabaseErrorHandler mDefaultDatabaseErrorHandler
                = new DefaultDatabaseErrorHandler();
        private Context mContext;

        MmsSmsDatabaseErrorHandler(Context context) {
            mContext = context;
        }

        @Override
        public void onCorruption(SQLiteDatabase dbObj) {
            String logMsg = "Corruption reported by sqlite on database: " + dbObj.getPath();
            localLogWtf(logMsg);
            sendDbLostIntent(mContext, true);
            // Let the default error handler take other actions
            mDefaultDatabaseErrorHandler.onCorruption(dbObj);
        }
    }

    @VisibleForTesting
    MmsSmsDatabaseHelper(Context context, MmsSmsDatabaseErrorHandler dbErrorHandler) {
        super(context, DATABASE_NAME, null, DATABASE_VERSION, dbErrorHandler);
        mContext = context;
        // Memory optimization - close idle connections after 30s of inactivity
        setIdleConnectionTimeout(IDLE_CONNECTION_TIMEOUT_MS);
        setWriteAheadLoggingEnabled(false);
        try {
            PhoneFactory.addLocalLog(TAG, 100);
        } catch (IllegalArgumentException e) {
            // ignore
        }
    }

    private static synchronized MmsSmsDatabaseErrorHandler getDbErrorHandler(Context context) {
        if (sDbErrorHandler == null) {
            sDbErrorHandler = new MmsSmsDatabaseErrorHandler(context);
        }
        return sDbErrorHandler;
    }

    private static void sendDbLostIntent(Context context, boolean isCorrupted) {
        // Broadcast ACTION_SMS_MMS_DB_LOST
        Intent intent = new Intent(Sms.Intents.ACTION_SMS_MMS_DB_LOST);
        intent.putExtra(Sms.Intents.EXTRA_IS_CORRUPTED, isCorrupted);
        intent.addFlags(Intent.FLAG_RECEIVER_INCLUDE_BACKGROUND);
        context.sendBroadcast(intent, android.Manifest.permission.READ_PRIVILEGED_PHONE_STATE);
    }
    /**
     * Returns a singleton helper for the combined MMS and SMS database in device encrypted storage.
     */
    /* package */ static synchronized MmsSmsDatabaseHelper getInstanceForDe(Context context) {
        if (sDeInstance == null) {
            Context deContext = ProviderUtil.getDeviceEncryptedContext(context);
            sDeInstance = new MmsSmsDatabaseHelper(deContext, getDbErrorHandler(deContext));
        }
        return sDeInstance;
    }

    /**
     * Returns a singleton helper for the combined MMS and SMS database in credential encrypted
     * storage. If FBE is not available, use the device encrypted storage instead.
     */
    /* package */ static synchronized MmsSmsDatabaseHelper getInstanceForCe(Context context) {
        if (sCeInstance == null) {
            if (StorageManager.isFileEncryptedNativeOrEmulated()) {
                Context ceContext = ProviderUtil.getCredentialEncryptedContext(context);
                sCeInstance = new MmsSmsDatabaseHelper(ceContext, getDbErrorHandler(ceContext));
            } else {
                sCeInstance = getInstanceForDe(context);
            }
        }
        return sCeInstance;
    }

    /**
     * Look through all the recipientIds referenced by the threads and then delete any
     * unreferenced rows from the canonical_addresses table.
     */
    protected static void removeUnferencedCanonicalAddresses(SQLiteDatabase db) {
        Cursor c = db.query(MmsSmsProvider.TABLE_THREADS, new String[] { "recipient_ids" },
                null, null, null, null, null);
        if (c != null) {
            try {
                if (c.getCount() == 0) {
                    // no threads, delete all addresses
                    int rows = db.delete("canonical_addresses", null, null);
                } else {
                    // Find all the referenced recipient_ids from the threads. recipientIds is
                    // a space-separated list of recipient ids: "1 14 21"
                    HashSet<Integer> recipientIds = new HashSet<Integer>();
                    while (c.moveToNext()) {
                        String[] recips = c.getString(0).split(" ");
                        for (String recip : recips) {
                            try {
                                int recipientId = Integer.parseInt(recip);
                                recipientIds.add(recipientId);
                            } catch (Exception e) {
                            }
                        }
                    }
                    // Now build a selection string of all the unique recipient ids
                    StringBuilder sb = new StringBuilder();
                    Iterator<Integer> iter = recipientIds.iterator();
                    sb.append("_id NOT IN (");
                    while (iter.hasNext()) {
                        sb.append(iter.next());
                        if (iter.hasNext()) {
                            sb.append(",");
                        }
                    }
                    sb.append(")");
                    int rows = db.delete("canonical_addresses", sb.toString(), null);
                }
            } finally {
                c.close();
            }
        }
    }

    public static void updateThread(SQLiteDatabase db, long thread_id) {
        if (thread_id < 0) {
            updateAllThreads(db, null, null);
            return;
        }

        db.beginTransaction();
        try {
            /// M: Code analyze 008, new feature, support for wappush. @{
            /// M: if it's a wappush thread, it doesn't need to be updated here;
            if (MTK_WAPPUSH_SUPPORT) {
                Cursor pushCursor = db.rawQuery("select * from threads where type="
                        + MtkTelephony.MtkThreads.WAPPUSH_THREAD + " AND _id=" + thread_id, null);
                if (pushCursor != null) {
                    try {
                        if (pushCursor.getCount() != 0) {
                            return;
                        }
                    } finally {
                        pushCursor.close();
                    }
                }
            }
            /// @}
            // Delete the row for this thread in the threads table if
            // there are no more messages attached to it in either
            // the sms or pdu tables.
            /// M: Code analyze 008, new feature, support for wappush. @{
            int rows = 0;
            if (MTK_WAPPUSH_SUPPORT) {
                rows = db.delete(MmsSmsProvider.TABLE_THREADS,
                      "status = 0 AND " +
                      "_id = ? AND type <> ? AND _id NOT IN" +
                      "          (SELECT thread_id FROM sms where thread_id is not null " +
                      "           UNION SELECT thread_id FROM pdu where thread_id is not null)",
                      new String[] { String.valueOf(thread_id),
                        String.valueOf(MtkThreads.WAPPUSH_THREAD) });
            } else {
                rows = db.delete(MmsSmsProvider.TABLE_THREADS,
                      "status = 0 AND " +
                      "_id = ? AND _id NOT IN" +
                      "          (SELECT thread_id FROM sms where thread_id is not null " +
                      "           UNION SELECT thread_id FROM pdu where thread_id is not null)",
                      new String[] { String.valueOf(thread_id) });
            }
            /// @}
            if (rows > 0) {
                // If this deleted a row, let's remove orphaned canonical_addresses
                /// and get outta here
                removeUnferencedCanonicalAddresses(db);
            } else {
                /// M: Code analyze 007, fix bug ALPS00276375, delete un-read message
                /// in folder mode make conversation
                /// mode display abnormally, then update the read field in threads
                /// after deleting a message.  for OP01 @{
                updateThreadReadAfterDeleteMessage(db, thread_id);
                /// @}
                // Update the message count in the threads table as the sum
                // of all messages in both the sms and pdu tables.
                db.execSQL(
                    "  UPDATE threads SET message_count = " +
                    "     (SELECT COUNT(sms._id) FROM sms LEFT JOIN threads " +
                    "      ON threads._id = " + Sms.THREAD_ID +
                    "      WHERE " + Sms.THREAD_ID + " = " + thread_id +
                    "        AND sms." + Sms.TYPE + " != 3) + " +
                    "     (SELECT COUNT(pdu._id) FROM pdu LEFT JOIN threads " +
                    "      ON threads._id = " + Mms.THREAD_ID +
                    "      WHERE " + Mms.THREAD_ID + " = " + thread_id +
                    "        AND (m_type=132 OR m_type=130 OR m_type=128)" +
                    "        AND " + Mms.MESSAGE_BOX + " != 3) " +
                    "  WHERE threads._id = " + thread_id + ";");

                /// M: update readcount and date_sent column in threads table. @{
                db.execSQL(
                    "  UPDATE threads" +
                    "  SET" +
                    "  readcount =" +
                    "    (SELECT count(_id) FROM " +
                    "      (SELECT DISTINCT date * 1 AS normalized_date, _id, read FROM sms " +
                    "       WHERE ((read=1) AND thread_id = " + thread_id
                    + " AND (type != 3)) " +
                    "       UNION " +
                    "       SELECT DISTINCT date * 1000 AS normalized_date, pdu._id, read " +
                    "       FROM pdu LEFT JOIN pending_msgs ON pdu._id = pending_msgs.msg_id " +
                    "       WHERE ((read=1) AND thread_id = " + thread_id
                    + " AND msg_box != 3 AND (msg_box != 3 " +
                    "       AND (m_type = 128 OR m_type = 132 OR m_type = 130))) " +
                    "       UNION " +
                    "       SELECT DISTINCT date * 1 AS normalized_date, _id, " +
                    "read FROM cellbroadcast " +
                    "       WHERE ((read=1) AND thread_id = " + thread_id + ") " +
                    "ORDER BY normalized_date ASC))" +
                    " , date_sent =" +
                    "    (SELECT date_sent FROM" +
                    "      (SELECT date_sent * 1000 as date_sent, date * 1000 AS date, " +
                    "thread_id FROM pdu WHERE m_type=132 OR m_type=130 OR m_type=128" +
                    "      UNION SELECT date_sent, date, thread_id FROM sms)" +
                    "    WHERE thread_id = " + thread_id + " ORDER BY date DESC LIMIT 1) " +
                    "  WHERE threads._id = " + thread_id +
                    "; ");
                /// @}

                // Update the date and the snippet (and its character set) in
                // the threads table to be that of the most recent message in
                // the thread.
                /// M: Code analyze 010, fix bug ALPS00293687, append has_attachment
                /// field update in the threads, and modify to only update messages
                /// which are displayed to the user. @{
                db.execSQL(
                    "  UPDATE threads" +
                    "  SET" +
                    "  date =" +
                    "    (SELECT date FROM" +
                    "        (SELECT date * 1000 AS date, thread_id FROM pdu" +
                    "         UNION SELECT date, thread_id FROM sms)" +
                    "     WHERE thread_id = " + thread_id + " ORDER BY date DESC LIMIT 1)," +
                    "  snippet =" +
                    "    (SELECT snippet FROM" +
                    "        (SELECT date * 1000 AS date, sub AS snippet, thread_id FROM pdu " +
                    "WHERE m_type=132 OR m_type=130 OR m_type=128" +
                    "         UNION SELECT date, body AS snippet, thread_id FROM sms)" +
                    "     WHERE thread_id = " + thread_id + " ORDER BY date DESC LIMIT 1)," +
                    "  snippet_cs =" +
                    "    (SELECT snippet_cs FROM" +
                    "        (SELECT date * 1000 AS date, sub_cs AS snippet_cs, thread_id " +
                    "FROM pdu WHERE m_type=132 OR m_type=130 OR m_type=128" +
                    "         UNION SELECT date, 0 AS snippet_cs, thread_id FROM sms)" +
                    "     WHERE thread_id = " + thread_id + " ORDER BY date DESC LIMIT 1)," +
                    "  has_attachment = " +
                    "   CASE " +
                    "    (SELECT COUNT(*) FROM part JOIN pdu " +
                    "     WHERE pdu.thread_id = " + thread_id +
                    "     AND part.ct != 'text/plain' AND part.ct != 'application/smil' " +
                    "     AND part.mid = pdu._id)" +
                    "   WHEN 0 THEN 0 " +
                    "   ELSE 1 " +
                    "   END " +
                    "  WHERE threads._id = " + thread_id + ";");
                /// @}
                // Update the error column of the thread to indicate if there
                // are any messages in it that have failed to send.
                // First check to see if there are any messages with errors in this thread.
                String query = "SELECT thread_id FROM sms WHERE type=" +
                        Telephony.TextBasedSmsColumns.MESSAGE_TYPE_FAILED +
                        " AND thread_id = " + thread_id;
//                                            + " LIMIT 1";
                int setError = 0;
                Cursor c = db.rawQuery(query, null);
                ///M: For ALPS01009088, update error by correct number of sending
                /// failed message(mms and sms)
                if (c != null) {
                    setError = c.getCount();
                    c.close();
                }
                String mms_query = "SELECT _id FROM pdu WHERE thread_id = " + thread_id +
                        " AND m_type = " + PduHeaders.MESSAGE_TYPE_SEND_REQ;
                Cursor c_mms = db.rawQuery(mms_query, null);
                String in = " IN (";
                if (c_mms != null) {
                    if (c_mms.moveToFirst()) {
                        in += c_mms.getInt(0);
                    }
                    while (c_mms.moveToNext()) {
                        in = in + "," + c_mms.getInt(0);
                    }
                    c_mms.close();
                }
                in += ")";
                String pending_query = "SELECT _id, err_type FROM pending_msgs " +
                        "WHERE err_type >= " + MmsSms.ERR_TYPE_GENERIC_PERMANENT
                        + " AND msg_id " + in ;
                Cursor c_pending = db.rawQuery(pending_query, null);
                if (c_pending != null) {
                    setError += c_pending.getCount();
                    c_pending.close();
                }
                Log.d(TAG, "updateThread, set error with mms id " + in
                        + "; setError = " + setError);
                /// end

                // What's the current state of the error flag in the threads table?
                String errorQuery = "SELECT error FROM threads WHERE _id = " + thread_id;
                c = db.rawQuery(errorQuery, null);
                if (c != null) {
                    try {
                        if (c.moveToNext()) {
                            int curError = c.getInt(0);
                            if (curError != setError) {
                                // The current thread error column differs, update it.
                                db.execSQL("UPDATE threads SET error=" + setError +
                                        " WHERE _id = " + thread_id);
                            }
                        }
                    } finally {
                        c.close();
                    }
                }
            }
            db.setTransactionSuccessful();
        } catch (Throwable ex) {
            Log.e(TAG, ex.getMessage(), ex);
        } finally {
            db.endTransaction();
        }
    }

    public static void updateAllThreads(SQLiteDatabase db, String where, String[] whereArgs) {
        Log.d(TAG, "updateAllThreads start");
        db.beginTransaction();
        try {
            if (where == null) {
                where = "";
            } else {
                where = "WHERE (" + where + ")";
            }
            /// M: Code analyze 012, fix bug ALPS00283284, query all thread id
            /// before update thread. @{
            String query = "SELECT _id FROM threads WHERE status<>0 OR _id IN " +
                           "(SELECT DISTINCT thread_id FROM sms " + where +
                           " UNION SELECT DISTINCT thread_id FROM pdu " + where + ")";
            Log.d(TAG, "updateAllThreads query " + query);
            /// @}
            /// M: two where params, need two whereArgs params. @{
            String[] whereArgs1 = null;
            if (whereArgs != null && whereArgs.length > 0) {
                whereArgs1 = new String[whereArgs.length * 2];
                for (int i = 0; i < whereArgs.length; i++) {
                    whereArgs1[i] = whereArgs[i];
                    whereArgs1[i + whereArgs.length] = whereArgs[i];
                }
            }
            Cursor c = db.rawQuery(query, whereArgs1);
            if (c != null) {
                try {
                    while (c.moveToNext()) {
                        updateThread(db, c.getInt(0));
                    }
                } finally {
                    c.close();
                }
            }
            // TODO: there are several db operations in this function. Lets wrap them in a
            // transaction to make it faster.
            // remove orphaned threads
            /// M: Code analyze 008, new feature, support for wappush. @{
            if (MTK_WAPPUSH_SUPPORT) {
                db.delete(MmsSmsProvider.TABLE_THREADS,
                    "status = 0 AND " +
                    "_id NOT IN (SELECT DISTINCT thread_id FROM sms where thread_id NOT NULL " +
                    "UNION SELECT DISTINCT thread_id FROM pdu where thread_id NOT NULL)" + " AND "
                    + Threads.TYPE + " <> " + MtkThreads.WAPPUSH_THREAD, null);
            } else {
                db.delete(MmsSmsProvider.TABLE_THREADS,
                    "status = 0 AND " +
                    "_id NOT IN (SELECT DISTINCT thread_id FROM sms where thread_id NOT NULL " +
                    "UNION SELECT DISTINCT thread_id FROM pdu where thread_id NOT NULL )", null);
            }
            /// @}
            // remove orphaned canonical_addresses
            removeUnferencedCanonicalAddresses(db);
            db.setTransactionSuccessful();
        } catch (Throwable ex) {
            Log.e(TAG, ex.getMessage(), ex);
        } finally {
            db.endTransaction();
        }
        Log.d(TAG, "updateAllThreads end");
    }

    public static int deleteOneSms(SQLiteDatabase db, int message_id) {
        int thread_id = -1;
        // Find the thread ID that the specified SMS belongs to.
        Cursor c = db.query("sms", new String[] { "thread_id" },
                            "_id=" + message_id, null, null, null, null);
        if (c != null) {
            if (c.moveToFirst()) {
                thread_id = c.getInt(0);
            }
            c.close();
        }

        // Delete the specified message.
        int rows = db.delete("sms", "_id=" + message_id, null);
        if (thread_id > 0) {
            // Update its thread.
            updateThread(db, thread_id);
        }
        return rows;
    }

    @Override
    public void onCreate(SQLiteDatabase db) {
        localLog("onCreate: Creating all SMS-MMS tables.");
        // if FBE is not supported, or if this onCreate is for CE partition database
        if (!StorageManager.isFileEncryptedNativeOrEmulated()
                || (mContext != null && mContext.isCredentialProtectedStorage())) {
            localLog("onCreate: broadcasting ACTION_SMS_MMS_DB_CREATED");
            // Broadcast ACTION_SMS_MMS_DB_CREATED
            Intent intent = new Intent(Sms.Intents.ACTION_SMS_MMS_DB_CREATED);
            intent.addFlags(Intent.FLAG_RECEIVER_INCLUDE_BACKGROUND);

            if (isInitialCreateDone()) {
                // this onCreate is called after onCreate was called once initially. The db file
                // disappeared mysteriously?
                localLogWtf("onCreate: was already called once earlier");
                intent.putExtra(Intents.EXTRA_IS_INITIAL_CREATE, false);
                sendDbLostIntent(mContext, false);
            } else {
                setInitialCreateDone();
                intent.putExtra(Intents.EXTRA_IS_INITIAL_CREATE, true);
            }

            mContext.sendBroadcast(intent, android.Manifest.permission.READ_SMS);
        }
        if (MTK_WAPPUSH_SUPPORT) {
            createWapPushTables(db);
        }
        createMmsTables(db);
        createSmsTables(db);
        createCommonTables(db);

        if (IS_RCS_TABLE_SCHEMA_CODE_COMPLETE) {
            RcsProviderThreadHelper.createThreadTables(db);
            RcsProviderParticipantHelper.createParticipantTables(db);
            RcsProviderMessageHelper.createRcsMessageTables(db);
            RcsProviderEventHelper.createRcsEventTables(db);
        }
        createCBTables(db);
        /// M: Add for ip message
        createThreadSettingsTable(db);
        createCommonTriggers(db);
        createMmsTriggers(db);
        createWordsTables(db);
        createIndices(db);
        createQuickText(db);
        createMwiTables(db);
    }

    private static void localLog(String logMsg) {
        Log.d(TAG, logMsg);
        PhoneFactory.localLog(TAG, logMsg);
    }

    private static void localLogWtf(String logMsg) {
        Slog.wtf(TAG, logMsg);
        PhoneFactory.localLog(TAG, logMsg);
    }

    private boolean isInitialCreateDone() {
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(mContext);
        return sp.getBoolean(INITIAL_CREATE_DONE, false);
    }

    private void setInitialCreateDone() {
        if (!sInitialCreateDone.getAndSet(true)) {
            SharedPreferences.Editor editor
                    = PreferenceManager.getDefaultSharedPreferences(mContext).edit();
            editor.putBoolean(INITIAL_CREATE_DONE, true);
            editor.commit();
        }
    }

    // When upgrading the database we need to populate the words
    // table with the rows out of sms and part.
    private void populateWordsTable(SQLiteDatabase db) {
        final String TABLE_WORDS = "words";
        {
            Cursor smsRows = db.query(
                    "sms",
                    new String[] { Sms._ID, Sms.BODY },
                    null,
                    null,
                    null,
                    null,
                    null);
            try {
                if (smsRows != null) {
                    smsRows.moveToPosition(-1);
                    ContentValues cv = new ContentValues();
                    while (smsRows.moveToNext()) {
                        cv.clear();

                        long id = smsRows.getLong(0);        // 0 for Sms._ID
                        String body = smsRows.getString(1);  // 1 for Sms.BODY

                        cv.put(Telephony.MmsSms.WordsTable.ID, id);
                        cv.put(Telephony.MmsSms.WordsTable.INDEXED_TEXT, body);
                        cv.put(Telephony.MmsSms.WordsTable.SOURCE_ROW_ID, id);
                        cv.put(Telephony.MmsSms.WordsTable.TABLE_ID, 1);
                        db.insert(TABLE_WORDS, Telephony.MmsSms.WordsTable.INDEXED_TEXT, cv);
                    }
                }
            } finally {
                if (smsRows != null) {
                    smsRows.close();
                }
            }
        }

        {
            Cursor mmsRows = db.query(
                    "part",
                    new String[] { Part._ID, Part.TEXT },
                    "ct = 'text/plain'",
                    null,
                    null,
                    null,
                    null);
            try {
                if (mmsRows != null) {
                    mmsRows.moveToPosition(-1);
                    ContentValues cv = new ContentValues();
                    while (mmsRows.moveToNext()) {
                        cv.clear();

                        long id = mmsRows.getLong(0);         // 0 for Part._ID
                        String body = mmsRows.getString(1);   // 1 for Part.TEXT

                        cv.put(Telephony.MmsSms.WordsTable.ID, id);
                        cv.put(Telephony.MmsSms.WordsTable.INDEXED_TEXT, body);
                        cv.put(Telephony.MmsSms.WordsTable.SOURCE_ROW_ID, id);
                        cv.put(Telephony.MmsSms.WordsTable.TABLE_ID, 1);
                        db.insert(TABLE_WORDS, Telephony.MmsSms.WordsTable.INDEXED_TEXT, cv);
                    }
                }
            } finally {
                if (mmsRows != null) {
                    mmsRows.close();
                }
            }
        }

        {
            Cursor wpRows = db.query(
                    "wappush",
                    new String[] { "_id", "url", "text" },
                    null,
                    null,
                    null,
                    null,
                    null);
            try {
                if (wpRows != null) {
                    wpRows.moveToPosition(-1);
                    ContentValues cv = new ContentValues();
                    while (wpRows.moveToNext()) {
                        cv.clear();

                        long id = wpRows.getLong(0);         // 0 for WapPush._ID
                        String body = wpRows.getString(1);   // 1 for WapPush.URL
                        String url = wpRows.getString(2);   // 2 for WapPush.TEXT

                        cv.put(Telephony.MmsSms.WordsTable.ID, id);
                        cv.put(Telephony.MmsSms.WordsTable.INDEXED_TEXT, url + " " + body);
                        cv.put(Telephony.MmsSms.WordsTable.SOURCE_ROW_ID, id);
                        cv.put(Telephony.MmsSms.WordsTable.TABLE_ID, 1);
                        db.insert(TABLE_WORDS, Telephony.MmsSms.WordsTable.INDEXED_TEXT, cv);
                    }
                }
            } finally {
                if (wpRows != null) {
                    wpRows.close();
                }
            }
        }
    }

    private void createWordsTables(SQLiteDatabase db) {
        try {
            db.execSQL("CREATE VIRTUAL TABLE words USING FTS3 (_id INTEGER PRIMARY KEY, " +
                    "index_text TEXT, source_id INTEGER, table_to_use INTEGER);");

            // monitor the sms table
            // NOTE don't handle inserts using a trigger because it has an unwanted
            // side effect:  the value returned for the last row ends up being the
            // id of one of the trigger insert not the original row insert.
            // Handle inserts manually in the provider.
            /// M: Code analyze 014, unknown, can find CR ALPS00268010,
            /// trigger the trigger only when the "body" column was update
            db.execSQL("CREATE TRIGGER sms_words_update AFTER UPDATE OF body " +
                    " ON sms BEGIN UPDATE words " +
                    " SET index_text = NEW.body WHERE (source_id=NEW._id AND table_to_use=1); " +
                    " END;");
            db.execSQL("CREATE TRIGGER sms_words_delete AFTER DELETE ON sms BEGIN DELETE FROM " +
                    "  words WHERE source_id = OLD._id AND table_to_use = 1; END;");

            /// M: monitor the wappush table
            db.execSQL("CREATE TRIGGER wp_words_update AFTER UPDATE ON wappush " +
                    "BEGIN UPDATE words " +
                    " SET index_text = coalesce(NEW.text||' '||NEW.url,NEW.text,NEW.url) " +
                    "WHERE (source_id=NEW._id AND table_to_use=3); END;");
            db.execSQL("CREATE TRIGGER wp_words_delete AFTER DELETE ON wappush BEGIN DELETE FROM " +
                    " words WHERE source_id = OLD._id AND table_to_use = 3; END;");

            populateWordsTable(db);
        } catch (Exception ex) {
            Log.e(TAG, "got exception creating words table: " + ex.toString());
        }
    }

    private void createIndices(SQLiteDatabase db) {
        createThreadIdIndex(db);
        createThreadIdDateIndex(db);
        createPartMidIndex(db);
        createAddrMsgIdIndex(db);
    }

    private void createThreadIdIndex(SQLiteDatabase db) {
        try {
            db.execSQL("CREATE INDEX IF NOT EXISTS typeThreadIdIndex ON sms" +
            " (type, thread_id);");
        } catch (Exception ex) {
            Log.e(TAG, "got exception creating indices: " + ex.toString());
        }
    }

    private void createThreadIdDateIndex(SQLiteDatabase db) {
        try {
            db.execSQL("CREATE INDEX IF NOT EXISTS threadIdDateIndex ON sms" +
            " (thread_id, date);");
        } catch (Exception ex) {
            Log.e(TAG, "got exception creating indices: " + ex.toString());
        }
    }

    private void createPartMidIndex(SQLiteDatabase db) {
        try {
            db.execSQL("CREATE INDEX IF NOT EXISTS partMidIndex ON part (mid)");
        } catch (Exception ex) {
            Log.e(TAG, "got exception creating indices: " + ex.toString());
        }
    }

    private void createAddrMsgIdIndex(SQLiteDatabase db) {
        try {
            db.execSQL("CREATE INDEX IF NOT EXISTS addrMsgIdIndex ON addr (msg_id)");
        } catch (Exception ex) {
            Log.e(TAG, "got exception creating indices: " + ex.toString());
        }
    }

    @VisibleForTesting
    void createMmsTables(SQLiteDatabase db) {
        // N.B.: Whenever the columns here are changed, the columns in
        // {@ref MmsSmsProvider} must be changed to match.
        db.execSQL("CREATE TABLE " + MmsProvider.TABLE_PDU + " (" +
                   Mms._ID + " INTEGER PRIMARY KEY AUTOINCREMENT," +
                   Mms.THREAD_ID + " INTEGER," +
                   Mms.DATE + " INTEGER," +
                   Mms.DATE_SENT + " INTEGER DEFAULT 0," +
                   Mms.MESSAGE_BOX + " INTEGER," +
                   Mms.READ + " INTEGER DEFAULT 0," +
                   Mms.MESSAGE_ID + " TEXT," +
                   Mms.SUBJECT + " TEXT," +
                   Mms.SUBJECT_CHARSET + " INTEGER," +
                   Mms.CONTENT_TYPE + " TEXT," +
                   Mms.CONTENT_LOCATION + " TEXT," +
                   Mms.EXPIRY + " INTEGER," +
                   Mms.MESSAGE_CLASS + " TEXT," +
                   Mms.MESSAGE_TYPE + " INTEGER," +
                   Mms.MMS_VERSION + " INTEGER," +
                   Mms.MESSAGE_SIZE + " INTEGER," +
                   Mms.PRIORITY + " INTEGER," +
                   Mms.READ_REPORT + " INTEGER," +
                   Mms.REPORT_ALLOWED + " INTEGER," +
                   Mms.RESPONSE_STATUS + " INTEGER," +
                   Mms.STATUS + " INTEGER," +
                   Mms.TRANSACTION_ID + " TEXT," +
                   Mms.RETRIEVE_STATUS + " INTEGER," +
                   Mms.RETRIEVE_TEXT + " TEXT," +
                   Mms.RETRIEVE_TEXT_CHARSET + " INTEGER," +
                   Mms.READ_STATUS + " INTEGER," +
                   Mms.CONTENT_CLASS + " INTEGER," +
                   Mms.RESPONSE_TEXT + " TEXT," +
                   Mms.DELIVERY_TIME + " INTEGER," +
                   Mms.DELIVERY_REPORT + " INTEGER," +
                   Mms.LOCKED + " INTEGER DEFAULT 0," +
                   Mms.SUBSCRIPTION_ID + " INTEGER DEFAULT "
                           + SubscriptionManager.INVALID_SUBSCRIPTION_ID + ", " +
                   "service_center TEXT," +
                   Mms.SEEN + " INTEGER DEFAULT 0," +
                   Mms.CREATOR + " TEXT," +
                   Mms.TEXT_ONLY + " INTEGER DEFAULT 0" +
                   ");");

        db.execSQL("CREATE TABLE " + MmsProvider.TABLE_ADDR + " (" +
                   Addr._ID + " INTEGER PRIMARY KEY," +
                   Addr.MSG_ID + " INTEGER," +
                   Addr.CONTACT_ID + " INTEGER," +
                   Addr.ADDRESS + " TEXT," +
                   Addr.TYPE + " INTEGER," +
                   Addr.CHARSET + " INTEGER);");

        db.execSQL("CREATE TABLE " + MmsProvider.TABLE_PART + " (" +
                   Part._ID + " INTEGER PRIMARY KEY AUTOINCREMENT," +
                   Part.MSG_ID + " INTEGER," +
                   Part.SEQ + " INTEGER DEFAULT 0," +
                   Part.CONTENT_TYPE + " TEXT," +
                   Part.NAME + " TEXT," +
                   Part.CHARSET + " INTEGER," +
                   Part.CONTENT_DISPOSITION + " TEXT," +
                   Part.FILENAME + " TEXT," +
                   Part.CONTENT_ID + " TEXT," +
                   Part.CONTENT_LOCATION + " TEXT," +
                   Part.CT_START + " INTEGER," +
                   Part.CT_TYPE + " TEXT," +
                   Part._DATA + " TEXT," +
                   Part.TEXT + " TEXT);");

        db.execSQL("CREATE TABLE " + MmsProvider.TABLE_RATE + " (" +
                   Rate.SENT_TIME + " INTEGER);");

        db.execSQL("CREATE TABLE " + MmsProvider.TABLE_DRM + " (" +
                   BaseColumns._ID + " INTEGER PRIMARY KEY," +
                   "_data TEXT);");

        // Restricted view of pdu table, only sent/received messages without wap pushes
        db.execSQL("CREATE VIEW " + MmsProvider.VIEW_PDU_RESTRICTED + " AS " +
                "SELECT * FROM " + MmsProvider.TABLE_PDU + " WHERE " +
                "(" + Mms.MESSAGE_BOX + "=" + Mms.MESSAGE_BOX_INBOX +
                " OR " +
                Mms.MESSAGE_BOX + "=" + Mms.MESSAGE_BOX_SENT + ")" +
                " AND " +
                "(" + Mms.MESSAGE_TYPE + "!=" + PduHeaders.MESSAGE_TYPE_NOTIFICATION_IND + ");");
    }

    // Unlike the other trigger-creating functions, this function can be called multiple times
    // without harm.
    private void createMmsTriggers(SQLiteDatabase db) {
        // Cleans up parts when a MM is deleted.
        db.execSQL("DROP TRIGGER IF EXISTS part_cleanup");
        db.execSQL("CREATE TRIGGER part_cleanup DELETE ON " + MmsProvider.TABLE_PDU + " " +
                "BEGIN " +
                "  DELETE FROM " + MmsProvider.TABLE_PART +
                "  WHERE " + Part.MSG_ID + "=old._id;" +
                "END;");

        // Cleans up address info when a MM is deleted.
        db.execSQL("DROP TRIGGER IF EXISTS addr_cleanup");
        db.execSQL("CREATE TRIGGER addr_cleanup DELETE ON " + MmsProvider.TABLE_PDU + " " +
                "BEGIN " +
                "  DELETE FROM " + MmsProvider.TABLE_ADDR +
                "  WHERE " + Addr.MSG_ID + "=old._id;" +
                "END;");

        // Delete obsolete delivery-report, read-report while deleting their
        // associated Send.req.
        db.execSQL("DROP TRIGGER IF EXISTS cleanup_delivery_and_read_report");
        db.execSQL("CREATE TRIGGER cleanup_delivery_and_read_report " +
                "AFTER DELETE ON " + MmsProvider.TABLE_PDU + " " +
                "WHEN old." + Mms.MESSAGE_TYPE + "=" + PduHeaders.MESSAGE_TYPE_SEND_REQ + " " +
                "BEGIN " +
                "  DELETE FROM " + MmsProvider.TABLE_PDU +
                "  WHERE (" + Mms.MESSAGE_TYPE + "=" + PduHeaders.MESSAGE_TYPE_DELIVERY_IND +
                "    OR " + Mms.MESSAGE_TYPE + "=" + PduHeaders.MESSAGE_TYPE_READ_ORIG_IND +
                ")" +
                "    AND " + Mms.MESSAGE_ID + "=old." + Mms.MESSAGE_ID + "; " +
                "END;");

        db.execSQL("DROP TRIGGER IF EXISTS update_threads_on_insert_part");
        db.execSQL(PART_UPDATE_THREADS_ON_INSERT_TRIGGER);

        db.execSQL("DROP TRIGGER IF EXISTS update_threads_on_update_part");
        db.execSQL(PART_UPDATE_THREADS_ON_UPDATE_TRIGGER);

        db.execSQL("DROP TRIGGER IF EXISTS update_threads_on_delete_part");
        db.execSQL(PART_UPDATE_THREADS_ON_DELETE_TRIGGER);

        db.execSQL("DROP TRIGGER IF EXISTS update_threads_on_update_pdu");
        db.execSQL(PDU_UPDATE_THREADS_ON_UPDATE_TRIGGER);

        // Delete pending status for a message when it is deleted.
        db.execSQL("DROP TRIGGER IF EXISTS delete_mms_pending_on_delete");
        db.execSQL("CREATE TRIGGER delete_mms_pending_on_delete " +
                   "AFTER DELETE ON " + MmsProvider.TABLE_PDU + " " +
                   "BEGIN " +
                   "  DELETE FROM " + MmsSmsProvider.TABLE_PENDING_MSG +
                   "  WHERE " + PendingMessages.MSG_ID + "=old._id; " +
                   "END;");

        // When a message is moved out of Outbox, delete its pending status.
        db.execSQL("DROP TRIGGER IF EXISTS delete_mms_pending_on_update");
        db.execSQL("CREATE TRIGGER delete_mms_pending_on_update " +
                   "AFTER UPDATE ON " + MmsProvider.TABLE_PDU + " " +
                   "WHEN (old." + Mms.MESSAGE_BOX + "=" + Mms.MESSAGE_BOX_OUTBOX +
                   "   OR old." + Mms.MESSAGE_BOX + "=" + Mms.MESSAGE_BOX_FAILED + ")" +
                   "  AND new." + Mms.MESSAGE_BOX + "!=" + Mms.MESSAGE_BOX_OUTBOX +
                   "  AND new." + Mms.MESSAGE_BOX + "!=" + Mms.MESSAGE_BOX_FAILED + " " +
                   "BEGIN " +
                   "  DELETE FROM " + MmsSmsProvider.TABLE_PENDING_MSG +
                   "  WHERE " + PendingMessages.MSG_ID + "=new._id; " +
                   "END;");

        // Insert pending status for M-Notification.ind or M-ReadRec.ind
        // when they are inserted into Inbox/Outbox.
        db.execSQL("DROP TRIGGER IF EXISTS insert_mms_pending_on_insert");
        db.execSQL("CREATE TRIGGER insert_mms_pending_on_insert " +
                   "AFTER INSERT ON pdu " +
                   "WHEN new." + Mms.MESSAGE_TYPE + "=" + PduHeaders.MESSAGE_TYPE_NOTIFICATION_IND +
                   "  OR new." + Mms.MESSAGE_TYPE + "=" + PduHeaders.MESSAGE_TYPE_READ_REC_IND +
                   " " +
                   "BEGIN " +
                   "  INSERT INTO " + MmsSmsProvider.TABLE_PENDING_MSG +
                   "    (" + PendingMessages.PROTO_TYPE + "," +
                   "     " + PendingMessages.MSG_ID + "," +
                   "     " + PendingMessages.MSG_TYPE + "," +
                   "     " + PendingMessages.ERROR_TYPE + "," +
                   "     " + PendingMessages.ERROR_CODE + "," +
                   "     " + PendingMessages.RETRY_INDEX + "," +
                   "     " + PendingMessages.DUE_TIME + ") " +
                   "  VALUES " +
                   "    (" + MmsSms.MMS_PROTO + "," +
                   "      new." + BaseColumns._ID + "," +
                   "      new." + Mms.MESSAGE_TYPE + ",0,0,0,0);" +
                   "END;");


        // Insert pending status for M-Send.req when it is moved into Outbox.
        db.execSQL("DROP TRIGGER IF EXISTS insert_mms_pending_on_update");
        db.execSQL("CREATE TRIGGER insert_mms_pending_on_update " +
                   "AFTER UPDATE ON pdu " +
                   "WHEN new." + Mms.MESSAGE_TYPE + "=" + PduHeaders.MESSAGE_TYPE_SEND_REQ +
                   "  AND new." + Mms.MESSAGE_BOX + "=" + Mms.MESSAGE_BOX_OUTBOX +
                   "  AND old." + Mms.MESSAGE_BOX + "!=" + Mms.MESSAGE_BOX_OUTBOX +
                   "  AND old." + Mms.MESSAGE_BOX + "!=" + Mms.MESSAGE_BOX_FAILED + " " +
                   "BEGIN " +
                   "  INSERT INTO " + MmsSmsProvider.TABLE_PENDING_MSG +
                   "    (" + PendingMessages.PROTO_TYPE + "," +
                   "     " + PendingMessages.MSG_ID + "," +
                   "     " + PendingMessages.MSG_TYPE + "," +
                   "     " + PendingMessages.ERROR_TYPE + "," +
                   "     " + PendingMessages.ERROR_CODE + "," +
                   "     " + PendingMessages.RETRY_INDEX + "," +
                   "     " + PendingMessages.DUE_TIME + ") " +
                   "  VALUES " +
                   "    (" + MmsSms.MMS_PROTO + "," +
                   "      new." + BaseColumns._ID + "," +
                   "      new." + Mms.MESSAGE_TYPE + ",0,0,0,0);" +
                   "END;");

        // monitor the mms table
        db.execSQL("DROP TRIGGER IF EXISTS mms_words_update");
        db.execSQL("CREATE TRIGGER mms_words_update AFTER UPDATE ON part BEGIN UPDATE words " +
                " SET index_text = NEW.text WHERE (source_id=NEW._id AND table_to_use=2); " +
                " END;");

        db.execSQL("DROP TRIGGER IF EXISTS mms_words_delete");
        db.execSQL("CREATE TRIGGER mms_words_delete AFTER DELETE ON part BEGIN DELETE FROM " +
                " words WHERE source_id = OLD._id AND table_to_use = 2; END;");

        // Updates threads table whenever a message in pdu is updated.
        db.execSQL("DROP TRIGGER IF EXISTS pdu_update_thread_date_subject_on_update");
        db.execSQL("CREATE TRIGGER pdu_update_thread_date_subject_on_update AFTER" +
                   "  UPDATE OF " + Mms.DATE + ", " + Mms.SUBJECT + ", " + Mms.MESSAGE_BOX +
                   "  ON " + MmsProvider.TABLE_PDU + " " +
                   PDU_UPDATE_THREAD_CONSTRAINTS +
                   PDU_UPDATE_THREAD_DATE_SNIPPET_COUNT_ON_UPDATE);

        // Update threads table whenever a message in pdu is deleted
        db.execSQL("DROP TRIGGER IF EXISTS pdu_update_thread_on_delete");
        /// M: because of triggers on sms and pdu, delete a large number of sms/pdu through an
        /// atomic operation will cost too much time. To improve deleting performance,
        /// remove triggers pdu_update_thread_on_delete and sms_update_thread_on_delete. @{
        /*db.execSQL("CREATE TRIGGER pdu_update_thread_on_delete " +
                   "AFTER DELETE ON pdu " +
                   "BEGIN " +
                   "  UPDATE threads SET " +
                   "     date = (strftime('%s','now') * 1000)" +
                   "  WHERE threads._id = old." + Mms.THREAD_ID + "; " +
                   UPDATE_THREAD_COUNT_ON_OLD +
                   UPDATE_THREAD_READ_COUNT_OLD +
                   UPDATE_THREAD_SNIPPET_SNIPPET_CS_ON_DELETE +
                   "END;");*/
        /// @}

        // Updates threads table whenever a message is added to pdu.
        db.execSQL("DROP TRIGGER IF EXISTS pdu_update_thread_on_insert");
        db.execSQL("CREATE TRIGGER pdu_update_thread_on_insert AFTER INSERT ON " +
                   MmsProvider.TABLE_PDU + " " +
                   PDU_UPDATE_THREAD_CONSTRAINTS +
                   PDU_UPDATE_THREAD_DATE_SNIPPET_COUNT_ON_INSERT);

        // Updates threads table whenever a message in pdu is updated.
        db.execSQL("DROP TRIGGER IF EXISTS pdu_update_thread_read_on_update");
        db.execSQL("CREATE TRIGGER pdu_update_thread_read_on_update AFTER" +
                   "  UPDATE OF " + Mms.READ +
                   "  ON " + MmsProvider.TABLE_PDU + " " +
                   PDU_UPDATE_THREAD_CONSTRAINTS +
                   "BEGIN " +
                   PDU_UPDATE_THREAD_READ_BODY +
                   /// M: fix bug ALPS00433858, update read_count
                   UPDATE_THREAD_READ_COUNT +
                   "END;");

        // Update the error flag of threads when delete pending message.
        db.execSQL("DROP TRIGGER IF EXISTS update_threads_error_on_delete_mms");
        db.execSQL("CREATE TRIGGER update_threads_error_on_delete_mms " +
                   "  BEFORE DELETE ON pdu" +
                   "  WHEN OLD._id IN (SELECT DISTINCT msg_id" +
                   "                   FROM pending_msgs" +
                   "                   WHERE err_type >= 10) " +
                   "BEGIN " +
                   "  UPDATE threads SET error = error - 1" +
                   "  WHERE _id = OLD.thread_id; " +
                   "END;");

        // Update the error flag of threads while moving an MM out of Outbox,
        // which was failed to be sent permanently.
        db.execSQL("DROP TRIGGER IF EXISTS update_threads_error_on_move_mms");
        db.execSQL("CREATE TRIGGER update_threads_error_on_move_mms " +
                   "  BEFORE UPDATE OF msg_box ON pdu " +
                   "  WHEN ((OLD.msg_box = 4 OR OLD.msg_box = 5) " +
                   "  AND NEW.msg_box != 4 AND NEW.msg_box != 5) " +
                   "  AND (OLD._id IN (SELECT DISTINCT msg_id" +
                   "                   FROM pending_msgs" +
                   "                   WHERE err_type >= 10)) " +
                   "BEGIN " +
                   "  UPDATE threads SET error = error - 1" +
                   "  WHERE _id = OLD.thread_id; " +
                   "END;");
    }

    @VisibleForTesting
    public static String CREATE_SMS_TABLE_STRING =
            "CREATE TABLE sms (" +
            "_id INTEGER PRIMARY KEY," +
            "thread_id INTEGER," +
            "address TEXT," +
            /// M: Code analyze 016, unknown, new column in sms table.
            "m_size INTEGER," +
            /// M: @}
            "person INTEGER," +
            "date INTEGER," +
            "date_sent INTEGER DEFAULT 0," +
            "protocol INTEGER," +
            "read INTEGER DEFAULT 0," +
            "status INTEGER DEFAULT -1," + // a TP-Status value
            // or -1 if it
            // status hasn't
            // been received
            "type INTEGER," +
            "reply_path_present INTEGER," +
            "subject TEXT," +
            "body TEXT," +
            "service_center TEXT," +
            "locked INTEGER DEFAULT 0," +
            "sub_id INTEGER DEFAULT " + SubscriptionManager.INVALID_SUBSCRIPTION_ID + ", " +
            "error_code INTEGER DEFAULT 0," +
            "creator TEXT," +
            "seen INTEGER DEFAULT 0," +
            "ipmsg_id INTEGER DEFAULT 0" +
            ");";

    @VisibleForTesting
    public static String CREATE_ATTACHMENTS_TABLE_STRING =
            "CREATE TABLE attachments (" +
            "sms_id INTEGER," +
            "content_url TEXT," +
            "offset INTEGER);";

    /**
     * This table is used by the SMS dispatcher to hold
     * incomplete partial messages until all the parts arrive.
     */
    @VisibleForTesting
    public static String CREATE_RAW_TABLE_STRING =
            "CREATE TABLE raw (" +
            "_id INTEGER PRIMARY KEY," +
            "date INTEGER," +
            "reference_number INTEGER," + // one per full message
            "count INTEGER," + // the number of parts
            "sequence INTEGER," + // the part number of this message
            "destination_port INTEGER," +
            "address TEXT," +
            "sub_id INTEGER DEFAULT " + SubscriptionManager.INVALID_SUBSCRIPTION_ID + ", " +
            "pdu TEXT," + // the raw PDU for this part
            "deleted INTEGER DEFAULT 0," + // bool to indicate if row is deleted
            "message_body TEXT," + // message body
            "display_originating_addr TEXT);";
    // email address if from an email gateway, otherwise same as address
    @VisibleForTesting
    void createSmsTables(SQLiteDatabase db) {
        // N.B.: Whenever the columns here are changed, the columns in
        // {@ref MmsSmsProvider} must be changed to match.
        db.execSQL(CREATE_SMS_TABLE_STRING);

        db.execSQL(CREATE_RAW_TABLE_STRING);

        db.execSQL(CREATE_ATTACHMENTS_TABLE_STRING);

        /**
         * This table is used by the SMS dispatcher to hold pending
         * delivery status report intents.
         */
        db.execSQL("CREATE TABLE sr_pending (" +
                   "reference_number INTEGER," +
                   "action TEXT," +
                   "data TEXT);");

        // Restricted view of sms table, only sent/received messages
        db.execSQL("CREATE VIEW " + SmsProvider.VIEW_SMS_RESTRICTED + " AS " +
                   "SELECT * FROM " + SmsProvider.TABLE_SMS + " WHERE " +
                   Sms.TYPE + "=" + Sms.MESSAGE_TYPE_INBOX +
                   " OR " +
                   Sms.TYPE + "=" + Sms.MESSAGE_TYPE_SENT + ";");

        if (mContext.getPackageManager().hasSystemFeature(PackageManager.FEATURE_AUTOMOTIVE)) {
            // Create a table to keep track of changes to SMS table - specifically on update to read
            // and deletion of msgs
            db.execSQL("CREATE TABLE sms_changes (" +
                       "_id INTEGER PRIMARY KEY," +
                       "orig_rowid INTEGER," +
                       "sub_id INTEGER," +
                       "type INTEGER," +
                       "new_read_status INTEGER" +
                       ");");
            db.execSQL("CREATE TRIGGER sms_update_on_read_change_row " +
                        "AFTER UPDATE OF read ON sms WHEN NEW.read != OLD.read " +
                        "BEGIN " +
                        "  INSERT INTO sms_changes VALUES(null, NEW._id, NEW.sub_id, " +
                        "0, NEW.read); " +
                        "END;");
            db.execSQL("CREATE TRIGGER sms_delete_change_row " +
                       "AFTER DELETE ON sms " +
                       "BEGIN " +
                       "  INSERT INTO sms_changes values(null, OLD._id, OLD.sub_id, 1, null); " +
                       "END;");
        }
    }

    @VisibleForTesting
    void createCommonTables(SQLiteDatabase db) {
        // TODO Ensure that each entry is removed when the last use of
        // any address equivalent to its address is removed.

        /**
         * This table maps the first instance seen of any particular
         * MMS/SMS address to an ID, which is then used as its
         * canonical representation.  If the same address or an
         * equivalent address (as determined by our Sqlite
         * PHONE_NUMBERS_EQUAL extension) is seen later, this same ID
         * will be used. The _id is created with AUTOINCREMENT so it
         * will never be reused again if a recipient is deleted.
         */
        db.execSQL("CREATE TABLE canonical_addresses (" +
                   "_id INTEGER PRIMARY KEY AUTOINCREMENT," +
                   "address TEXT);");

        /**
         * This table maps the subject and an ordered set of recipient
         * IDs, separated by spaces, to a unique thread ID.  The IDs
         * come from the canonical_addresses table.  This works
         * because messages are considered to be part of the same
         * thread if they have the same subject (or a null subject)
         * and the same set of recipients.
         */
        db.execSQL("CREATE TABLE threads (" +
                   Threads._ID + " INTEGER PRIMARY KEY AUTOINCREMENT," +
                   Threads.DATE + " INTEGER DEFAULT 0," +
                   MtkThreads.DATE_SENT + " INTEGER DEFAULT 0," +
                   Threads.MESSAGE_COUNT + " INTEGER DEFAULT 0," +
                   /// M: Code analyze 017, unknown, new column in threads table.
                   MtkThreads.READ_COUNT + " INTEGER DEFAULT 0," +
                   Threads.RECIPIENT_IDS + " TEXT," +
                   Threads.SNIPPET + " TEXT," +
                   Threads.SNIPPET_CHARSET + " INTEGER DEFAULT 0," +
                   Threads.READ + " INTEGER DEFAULT 1," +
                   Threads.ARCHIVED + " INTEGER DEFAULT 0," +
                   Threads.TYPE + " INTEGER DEFAULT 0," +
                   Threads.ERROR + " INTEGER DEFAULT 0," +
                   Threads.HAS_ATTACHMENT + " INTEGER DEFAULT 0," +
                   /// M: Code analyze 017, unknown, new column in threads table.
                   MtkThreads.STATUS + " INTEGER DEFAULT 0);");

        /**
         * This table stores the queue of messages to be sent/downloaded.
         */
        db.execSQL("CREATE TABLE " + MmsSmsProvider.TABLE_PENDING_MSG +" (" +
                   PendingMessages._ID + " INTEGER PRIMARY KEY," +
                   PendingMessages.PROTO_TYPE + " INTEGER," +
                   PendingMessages.MSG_ID + " INTEGER," +
                   PendingMessages.MSG_TYPE + " INTEGER," +
                   PendingMessages.ERROR_TYPE + " INTEGER," +
                   PendingMessages.ERROR_CODE + " INTEGER," +
                   PendingMessages.RETRY_INDEX + " INTEGER NOT NULL DEFAULT 0," +
                   PendingMessages.DUE_TIME + " INTEGER," +
                   PendingMessages.SUBSCRIPTION_ID + " INTEGER DEFAULT " +
                           SubscriptionManager.INVALID_SUBSCRIPTION_ID + ", " +
                   PendingMessages.LAST_TRY + " INTEGER);");

    }

    // TODO Check the query plans for these triggers.
    private void createCommonTriggers(SQLiteDatabase db) {
        // Updates threads table whenever a message is added to pdu.
        /// M: Code analyze 003, fix bug ALPS00301106, when restore MMS, already read MMS status
        /// shouldn't be changed into unread, and thread time should be the latest pdu's time,
        /// not now. @{
        db.execSQL("CREATE TRIGGER pdu_update_thread_on_insert AFTER INSERT ON " +
                   MmsProvider.TABLE_PDU + " " +
                   PDU_UPDATE_THREAD_CONSTRAINTS +
                   PDU_UPDATE_THREAD_DATE_SNIPPET_COUNT_ON_INSERT);
        /// @}
        /// M: ALPS00514953, Update thread date by the latest sms date @{
        // Updates threads table whenever a message is added to sms.
        db.execSQL("CREATE TRIGGER sms_update_thread_on_insert AFTER INSERT ON sms " +
                   SMS_UPDATE_THREAD_DATE_SNIPPET_COUNT_ON_INSERT);
        /// @}

        // Updates threads table whenever a message in sms is updated.
        db.execSQL("CREATE TRIGGER sms_update_thread_date_subject_on_update AFTER" +
                   "  UPDATE OF " + Sms.DATE + ", " + Sms.BODY + ", " + Sms.TYPE +
                   "  ON sms " +
                   SMS_UPDATE_THREAD_DATE_SNIPPET_COUNT_ON_UPDATE);

        // Updates threads table whenever a message in sms is updated.
        db.execSQL("CREATE TRIGGER sms_update_thread_read_on_update AFTER" +
                   "  UPDATE OF " + Sms.READ +
                   "  ON sms " +
                   "BEGIN " +
                   SMS_UPDATE_THREAD_READ_BODY +
                   /// M: Add for ip message
                   UPDATE_THREAD_READ_COUNT +
                   "END;");

        // ALPS04649945, when update thread_id to -1 in sms table,
        // the message count of conversation not update.
        db.execSQL("CREATE TRIGGER sms_update_thread_on_split AFTER"
            + "  UPDATE OF " + Sms.THREAD_ID + " ON sms "
            + "BEGIN "
            + SMS_UPDATE_OLD_THREAD_DATE
            + SMS_UPDATE_OLD_THREAD_MESSAGE_COUNT
            + SMS_UPDATE_OLD_THREAD_SNIPPET
            + SMS_UPDATE_OLD_THREAD_ERROR
            + "END;");

      /// M: for update readcount.
        /// M: because of triggers on sms and pdu, delete a large number of sms/pdu through an
        /// atomic operation will cost too much time. To improve deleting performance,
        /// remove triggers pdu_update_thread_on_delete and sms_update_thread_on_delete. @{
        /*db.execSQL("CREATE TRIGGER sms_update_thread_on_delete " +
            "AFTER DELETE ON sms " +
            "BEGIN " +
            UPDATE_THREAD_COUNT_ON_OLD +
            UPDATE_THREAD_READ_COUNT_OLD +
            "  UPDATE threads SET date_sent = " +
            "   (SELECT date_sent FROM" +
            "     (SELECT date_sent * 1000 as date_sent, date * 1000 AS date, thread_id FROM pdu
            WHERE m_type=132 OR m_type=130 OR m_type=128" +
            "      UNION SELECT date_sent, date, thread_id FROM sms)" +
            "    WHERE thread_id = OLD.thread_id ORDER BY date DESC LIMIT 1) " +
            "  WHERE threads._id = OLD.thread_id; " +
            "END;");*/
        /// @}

        /// M: Code analyze 008, new feature, support for wappush. @{
        /// M:When the wap push message is updated, update thread's read status
        if (MTK_WAPPUSH_SUPPORT) {
            db.execSQL("CREATE TRIGGER wappush_update_thread_on_update AFTER" +
                    "  UPDATE OF " + WapPush.READ +
                    "  ON wappush " +
                    "BEGIN " +
                    "  UPDATE threads SET read = " +
                    "    CASE (SELECT COUNT(*)" +
                    "          FROM wappush" +
                    "          WHERE " + WapPush.READ + " = 0" +
                    "            AND " + WapPush.THREAD_ID + " = threads._id)" +
                    "      WHEN 0 THEN 1" +
                    "      ELSE 0" +
                    "    END" +
                    "  WHERE threads._id = new." + WapPush.THREAD_ID + "; " +
                    "END;");
        }
        /// @}
        /// M: Code analyze 005, new feature, add operation about table wappush and cellbroadcast.
        /// TODO: the next comments should be restore, because it's from google, some code should be
        /// removed, because it has been marked in google version. @{
        /** M: When the last message in a thread is deleted, these
        * triggers ensure that the entry for its thread ID is removed
        * from the threads table.@{*/
        if (MTK_WAPPUSH_SUPPORT) {
//        db.execSQL("CREATE TRIGGER delete_obsolete_threads_pdu " +
//                   "AFTER DELETE ON pdu " +
//                   "BEGIN " +
//                   "  DELETE FROM threads " +
//                   "  WHERE " +
//                   "    _id = old.thread_id " +
//                   "    AND _id NOT IN " +
//                   "    (SELECT thread_id FROM sms " +
//                   "    UNION SELECT thread_id FROM wappush " +
//                   "     UNION SELECT thread_id from pdu " +
//                   "     UNION SELECT thread_id from cellbroadcast); " +
//                   "END;");
        // As of DATABASE_VERSION 55, we've removed these triggers that delete empty threads.
        // These triggers interfere with saving drafts on brand new threads. Instead of
        // triggers cleaning up empty threads, the empty threads should be cleaned up by
        // an explicit call to delete with Threads.OBSOLETE_THREADS_URI.

        db.execSQL("CREATE TRIGGER delete_obsolete_threads_when_update_pdu " +
                   "AFTER UPDATE OF " + Mms.THREAD_ID + " ON pdu " +
                   "WHEN old." + Mms.THREAD_ID + " != new." + Mms.THREAD_ID + " " +
                   "BEGIN " +
                   "  DELETE FROM threads " +
                   "  WHERE " +
                   "    _id = old.thread_id " +
                   "    AND _id NOT IN " +
                   "    (SELECT thread_id FROM sms " +
                    "    UNION SELECT thread_id FROM wappush " +
                   "     UNION SELECT thread_id from pdu " +
                   "     UNION SELECT thread_id from cellbroadcast); " +
                   "END;");

        } else {
//        db.execSQL("CREATE TRIGGER delete_obsolete_threads_pdu " +
//                   "AFTER DELETE ON pdu " +
//                   "BEGIN " +
//                   "  DELETE FROM threads " +
//                   "  WHERE " +
//                   "    _id = old.thread_id " +
//                   "    AND _id NOT IN " +
//                   "    (SELECT thread_id FROM sms " +
//                   "     UNION SELECT thread_id from pdu " +
//                   "     UNION SELECT thread_id from cellbroadcast); " +
//                   "END;");

        db.execSQL("CREATE TRIGGER delete_obsolete_threads_when_update_pdu " +
                   "AFTER UPDATE OF " + Mms.THREAD_ID + " ON pdu " +
                   "WHEN old." + Mms.THREAD_ID + " != new." + Mms.THREAD_ID + " " +
                   "BEGIN " +
                   "  DELETE FROM threads " +
                   "  WHERE " +
                   "    _id = old.thread_id " +
                   "    AND _id NOT IN " +
                   "    (SELECT thread_id FROM sms " +
                   "     UNION SELECT thread_id from pdu " +
                   "     UNION SELECT thread_id from cellbroadcast); " +
                   "END;");
        }
        /// @}

        // TODO Add triggers for SMS retry-status management.

        // Update the error flag of threads when the error type of
        // a pending MM is updated.
        db.execSQL("CREATE TRIGGER update_threads_error_on_update_mms " +
                   "  AFTER UPDATE OF err_type ON pending_msgs " +
                   /// M: Code analyze 018, fix bug ALPS00135481, common trigger revise.
                   "  WHEN (OLD.err_type < 10 AND NEW.err_type >= 10 AND NEW.proto_type = "
                   + MmsSms.MMS_PROTO + " AND NEW.msg_type = " + PduHeaders.MESSAGE_TYPE_SEND_REQ
                   + ")" +
                   "    OR (OLD.err_type >= 10 AND NEW.err_type < 10) " +
                   "BEGIN" +
                   "  UPDATE threads SET error = " +
                   "    CASE" +
                   "      WHEN NEW.err_type >= 10 THEN error + 1" +
                   "      ELSE error - 1" +
                   "    END " +
                   "  WHERE _id =" +
                   "   (SELECT DISTINCT thread_id" +
                   "    FROM pdu" +
                   "    WHERE _id = NEW.msg_id); " +
                   "END;");

        // Update the error flag of threads after a text message was
        // failed to send/receive.
        db.execSQL("CREATE TRIGGER update_threads_error_on_update_sms " +
                   "  AFTER UPDATE OF type ON sms" +
                   "  WHEN (OLD.type != 5 AND NEW.type = 5)" +
                   "    OR (OLD.type = 5 AND NEW.type != 5) " +
                   "BEGIN " +
                   "  UPDATE threads SET error = " +
                   "    CASE" +
                   "      WHEN NEW.type = 5 THEN error + 1" +
                   "      ELSE error - 1" +
                   "    END " +
                   "  WHERE _id = NEW.thread_id; " +
                   "END;");

        /// M: Code analyze 005, new feature, create triggers for cellbroadcast. @{
        /// M: Triggers for CB
        db.execSQL("CREATE TRIGGER cb_update_thread_on_insert AFTER INSERT ON cellbroadcast "
                + CB_UPDATE_THREAD_DATE_SNIPPET_COUNT_ON_UPDATE);
        db.execSQL("CREATE TRIGGER cb_update_thread_read_on_update AFTER"
                + "  UPDATE OF " + MtkTelephony.SmsCb.READ + "  ON cellbroadcast "
                + "BEGIN "
                + CB_UPDATE_THREAD_READ_BODY
                /// M: Add for ip message
                + UPDATE_THREAD_READ_COUNT
                + "END;");
        /// M: Update threads table whenever a message in messages is deleted
        db.execSQL("CREATE TRIGGER cb_update_thread_on_delete " +
                   "AFTER DELETE ON cellbroadcast " +
                   "BEGIN " +
                   "  UPDATE threads SET " +
                   "     date = (strftime('%s','now') * 1000)" +
                   "  WHERE threads._id = old." + MtkTelephony.SmsCb.THREAD_ID + "; " +
                   CB_UPDATE_THREAD_COUNT_ON_OLD +
                   CB_UPDATE_THREAD_SNIPPET_ON_DELETE +
                   CB_UPDATE_THREAD_DATE_ON_DELETE +
                   CB_UPDATE_THREAD_READ_BODY_DELETE +
                   "END;");
        /// @}
        /// M: Add for ip message @{
        // Insert item into thread_settings when a item inserted into threads table
        db.execSQL("DROP TRIGGER IF EXISTS insert_thread_settings_when_insert_threads");
        db.execSQL("CREATE TRIGGER insert_thread_settings_when_insert_threads " +
                   "AFTER INSERT ON threads " +
                   "BEGIN " +
                   "  INSERT INTO thread_settings " +
                   "    (" + MtkThreadSettings.THREAD_ID +
                   "    ," + MtkThreadSettings.SPAM +
                   "    ," + MtkThreadSettings.NOTIFICATION_ENABLE +
                   "    ," + MtkThreadSettings.MUTE +
                   "    ," + MtkThreadSettings.MUTE_START +
                   "    ," + MtkThreadSettings.RINGTONE +
                   "    ," + MtkThreadSettings.WALLPAPER +
                   "    ," + MtkThreadSettings.VIBRATE +
                   ") " +
                   "  VALUES " +
                   "    (new." + MtkThreadSettings._ID + "," +
                   "     0,1,0,0,'','',1" +
                   "); " +
                   "END;");

        // Delete item from thread_settings when a item deleted from threads table
        db.execSQL("DROP TRIGGER IF EXISTS delete_thread_settings_when_delete_threads");
        db.execSQL("CREATE TRIGGER delete_thread_settings_when_delete_threads " +
                   "AFTER DELETE ON threads " +
                   "BEGIN " +
                   "  DELETE FROM thread_settings " +
                   "    WHERE " + MtkThreadSettings.THREAD_ID + "=old._id; " +
                   "END;");
        /// @}
    }

    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int currentVersion) {
        Log.w(TAG, "Upgrading database from version " + oldVersion
                + " to " + currentVersion + ".");

        switch (oldVersion) {
        case 40:
            if (currentVersion <= 40) {
                return;
            }

            db.beginTransaction();
            try {
                upgradeDatabaseToVersion41(db);
                db.setTransactionSuccessful();
            } catch (Throwable ex) {
                Log.e(TAG, ex.getMessage(), ex);
                break;
            } finally {
                db.endTransaction();
            }
            // fall through
        case 41:
            if (currentVersion <= 41) {
                return;
            }

            db.beginTransaction();
            try {
                upgradeDatabaseToVersion42(db);
                db.setTransactionSuccessful();
            } catch (Throwable ex) {
                Log.e(TAG, ex.getMessage(), ex);
                break;
            } finally {
                db.endTransaction();
            }
            // fall through
        case 42:
            if (currentVersion <= 42) {
                return;
            }

            db.beginTransaction();
            try {
                upgradeDatabaseToVersion43(db);
                db.setTransactionSuccessful();
            } catch (Throwable ex) {
                Log.e(TAG, ex.getMessage(), ex);
                break;
            } finally {
                db.endTransaction();
            }
            // fall through
        case 43:
            if (currentVersion <= 43) {
                return;
            }

            db.beginTransaction();
            try {
                upgradeDatabaseToVersion44(db);
                db.setTransactionSuccessful();
            } catch (Throwable ex) {
                Log.e(TAG, ex.getMessage(), ex);
                break;
            } finally {
                db.endTransaction();
            }
            // fall through
        case 44:
            if (currentVersion <= 44) {
                return;
            }

            db.beginTransaction();
            try {
                upgradeDatabaseToVersion45(db);
                db.setTransactionSuccessful();
            } catch (Throwable ex) {
                Log.e(TAG, ex.getMessage(), ex);
                break;
            } finally {
                db.endTransaction();
            }
            // fall through
        case 45:
            if (currentVersion <= 45) {
                return;
            }
            db.beginTransaction();
            try {
                upgradeDatabaseToVersion46(db);
                db.setTransactionSuccessful();
            } catch (Throwable ex) {
                Log.e(TAG, ex.getMessage(), ex);
                break;
            } finally {
                db.endTransaction();
            }
            // fall through
        case 46:
            if (currentVersion <= 46) {
                return;
            }

            db.beginTransaction();
            try {
                upgradeDatabaseToVersion47(db);
                db.setTransactionSuccessful();
            } catch (Throwable ex) {
                Log.e(TAG, ex.getMessage(), ex);
                break;
            } finally {
                db.endTransaction();
            }
            // fall through
        case 47:
            if (currentVersion <= 47) {
                return;
            }

            db.beginTransaction();
            try {
                upgradeDatabaseToVersion48(db);
                db.setTransactionSuccessful();
            } catch (Throwable ex) {
                Log.e(TAG, ex.getMessage(), ex);
                break;
            } finally {
                db.endTransaction();
            }
            // fall through
        case 48:
            if (currentVersion <= 48) {
                return;
            }

            db.beginTransaction();
            try {
                createWordsTables(db);
                db.setTransactionSuccessful();
            } catch (Throwable ex) {
                Log.e(TAG, ex.getMessage(), ex);
                break;
            } finally {
                db.endTransaction();
            }
            // fall through
        case 49:
            if (currentVersion <= 49) {
                return;
            }
            db.beginTransaction();
            try {
                createThreadIdIndex(db);
                db.setTransactionSuccessful();
            } catch (Throwable ex) {
                Log.e(TAG, ex.getMessage(), ex);
                break; // force to destroy all old data;
            } finally {
                db.endTransaction();
            }
            // fall through
        case 50:
            if (currentVersion <= 50) {
                return;
            }

            db.beginTransaction();
            try {
                upgradeDatabaseToVersion51(db);
                db.setTransactionSuccessful();
            } catch (Throwable ex) {
                Log.e(TAG, ex.getMessage(), ex);
                break;
            } finally {
                db.endTransaction();
            }
            // fall through
        case 51:
            if (currentVersion <= 51) {
                return;
            }
            // 52 was adding a new meta_data column, but that was removed.
            // fall through
        case 52:
            if (currentVersion <= 52) {
                return;
            }

            db.beginTransaction();
            try {
                upgradeDatabaseToVersion53(db);
                db.setTransactionSuccessful();
            } catch (Throwable ex) {
                Log.e(TAG, ex.getMessage(), ex);
                break;
            } finally {
                db.endTransaction();
            }
            // fall through
        case 53:
            if (currentVersion <= 53) {
                return;
            }

            db.beginTransaction();
            try {
                upgradeDatabaseToVersion530100(db);
                db.setTransactionSuccessful();
            } catch (Throwable ex) {
                Log.e(TAG, ex.getMessage(), ex);
                break;
            } finally {
                db.endTransaction();
            }
           // fall through
        case 530100:
            if (currentVersion <= 530100) {
                return;
            }

            db.beginTransaction();
            try {
                upgradeDatabaseToVersion530200(db);
                db.setTransactionSuccessful();
            } catch (Throwable ex) {
                Log.e(TAG, ex.getMessage(), ex);
                break;
            } finally {
                db.endTransaction();
            }
            // fall through
        case 530200:
            if (currentVersion <= 530200) {
                return;
            }

            db.beginTransaction();
            try {
                upgradeDatabaseToVersion530300(db);
                db.setTransactionSuccessful();
            } catch (Throwable ex) {
                Log.e(TAG, ex.getMessage(), ex);
                break;
            } finally {
                db.endTransaction();
            }
            // fall through
        case 530300:
            if (currentVersion <= 530300) {
                return;
            }

            db.beginTransaction();
            try {
                upgradeDatabaseToVersion540000(db);
                db.setTransactionSuccessful();
            } catch (Throwable ex) {
                Log.e(TAG, ex.getMessage(), ex);
                break;
            } finally {
                db.endTransaction();
            }
            // fall through
        case 540000:
            if (currentVersion <= 540000) {
                return;
            }

            db.beginTransaction();
            try {
                upgradeDatabaseToVersion550000(db);
                db.setTransactionSuccessful();
            } catch (Throwable ex) {
                Log.e(TAG, ex.getMessage(), ex);
                break;
            } finally {
                db.endTransaction();
            }
            // fall through
        case 550000:
            if (currentVersion <= 550000) {
                return;
            }

            db.beginTransaction();
            try {
                upgradeDatabaseToVersion550100(db);
                db.setTransactionSuccessful();
            } catch (Throwable ex) {
                Log.e(TAG, ex.getMessage(), ex);
                break;
            } finally {
                db.endTransaction();
            }
            // fall through
        case 550100:
            if (currentVersion <= 550100) {
                return;
            }

            db.beginTransaction();
            try {
                upgradeDatabaseToVersion560000(db);
                db.setTransactionSuccessful();
            } catch (Throwable ex) {
                Log.e(TAG, ex.getMessage(), ex);
                break;
            } finally {
                db.endTransaction();
            }
            // fall through
        case 560000:
        case 560100:
            if (currentVersion <= 560100) {
                return;
            }

            db.beginTransaction();
            try {
                upgradeDatabaseToVersion560200(db);
                db.setTransactionSuccessful();
            } catch (Throwable ex) {
                Log.e(TAG, ex.getMessage(), ex);
                break;
            } finally {
                db.endTransaction();
            }
            // fall through
            /// M: improve performance about delete pdu. @{
        case 560200:
            if (currentVersion <= 560200) {
                return;
            }
            db.beginTransaction();
            try {
                upgradeDatabaseToVersion560300(db);
                db.setTransactionSuccessful();
            } catch (Throwable ex) {
                Log.e(TAG, ex.getMessage(), ex);
                break;
            } finally {
                db.endTransaction();
            }
            /// @}
            // fall through
        case 560300:
        case 560400:
            if (currentVersion <= 560400) {
                return;
            }
            db.beginTransaction();
            try {
                upgradeDatabaseToVersion560500(db);
                db.setTransactionSuccessful();
            } catch (Throwable ex) {
                Log.e(TAG, ex.getMessage(), ex);
                break;
            } finally {
                db.endTransaction();
            }
            // fall through
        case 560500:
            if (currentVersion <= 560500) {
                return;
            }

            db.beginTransaction();
            try {
                upgradeDatabaseToVersion600000(db);
                db.setTransactionSuccessful();
            } catch (Throwable ex) {
                Log.e(TAG, ex.getMessage(), ex);
                break;
            } finally {
                db.endTransaction();
            }
            // fall through
        case 600000:
            if (currentVersion <= 600000) {
                return;
            }

            db.beginTransaction();
            try {
                upgradeDatabaseToVersion600100(db);
                db.setTransactionSuccessful();
            } catch (Throwable ex) {
                Log.e(TAG, ex.getMessage(), ex);
                break;
            } finally {
                db.endTransaction();
            }
            // fall through
        case 600100:
            if (currentVersion <= 600100) {
                return;
            }

            db.beginTransaction();
            try {
                upgradeDatabaseToVersion600200(db);
                db.setTransactionSuccessful();
            } catch (Throwable ex) {
                Log.e(TAG, ex.getMessage(), ex);
                break;
            } finally {
                db.endTransaction();
            }
            // fall through
        case 600200:
            if (currentVersion <= 600200) {
                return;
            }

            db.beginTransaction();
            try {
                upgradeDatabaseToVersion600300(db);
                db.setTransactionSuccessful();
            } catch (Throwable ex) {
                Log.e(TAG, ex.getMessage(), ex);
                break;
            } finally {
                db.endTransaction();
            }
            // fall through
        case 600300:
        case 600400:
            if (currentVersion <= 600400) {
                return;
            }
            db.beginTransaction();
            try {
                upgradeDatabaseToVersion601000(db);
                db.setTransactionSuccessful();
            } catch (Throwable ex) {
                Log.e(TAG, ex.getMessage(), ex);
                break;
            } finally {
                db.endTransaction();
            }
            // fall through
        case 601000:
        case 601010:
        case 601011:
            if (currentVersion <= 601011){
                return;
            }
            db.beginTransaction();
            try {
                upgradeDatabaseToVersion601021(db);
                db.setTransactionSuccessful();
            } catch (Throwable ex) {
                Log.e(TAG, ex.getMessage(), ex);
                break;
            } finally {
                db.endTransaction();
            }
            // fall through
        case 601020:
        case 601021:
        case 601022:
        case 601023:
        case 601024:
            // Add this case to avoid JE when upgrade from 601023, do nothing
            if (currentVersion <= 601024){
                return;
            }
            db.beginTransaction();
            try {
                upgradeDatabaseToVersion621000(db);
                upgradeDatabaseToVersion631000(db);
                db.setTransactionSuccessful();
            } catch (Throwable ex) {
                Log.e(TAG, ex.getMessage(), ex);
                break;
            } finally {
                db.endTransaction();
            }
            // fall through
        case 631000:
            if (currentVersion <= 631000) {
                return;
            }

            db.beginTransaction();
            try {
                upgradeDatabaseToVersion641000(db);
                db.setTransactionSuccessful();
            } catch (Throwable ex) {
                Log.e(TAG, ex.getMessage(), ex);
                break;
            } finally {
                db.endTransaction();
            }
            // fall through
        case 641000:
        case 641001:
        case 641002:
            if (currentVersion <= 641002){
                return;
            }
            db.beginTransaction();
            try {
                upgradeDatabaseToVersion661000(db);
                db.setTransactionSuccessful();
            } catch (Throwable ex) {
                Log.e(TAG, ex.getMessage(), ex);
                break;
            } finally {
                db.endTransaction();
            }
            // fall through
        case 661000:
            if (currentVersion <= 661000) {
                return;
            }
            db.beginTransaction();
            try {
                createPartMidIndex(db);
                createAddrMsgIdIndex(db);
                db.setTransactionSuccessful();
            } catch (Throwable ex) {
                Log.e(TAG, ex.getMessage(), ex);
                break; // force to destroy all old data;
            } finally {
                db.endTransaction();
            }
            // fall through
        case 670000:
            if (currentVersion <= 670000) {
                return;
            }
            db.beginTransaction();
            try {
                upgradeDatabaseToVersion670010(db);
                db.setTransactionSuccessful();
            } catch (Throwable ex) {
                Log.e(TAG, ex.getMessage(), ex);
                break;
            } finally {
                db.endTransaction();
            }
            // fall through
        case 670010:
            if (currentVersion <= 670010) {
                return;
            }
            db.beginTransaction();
            try {
                upgradeDatabaseToVersion670020(db);
                db.setTransactionSuccessful();

            } catch (Throwable ex) {
                Log.e(TAG, ex.getMessage(), ex);
                break;
            } finally {
                db.endTransaction();
            }
            // fall through
        case 670020:
            if (currentVersion <= 670020 || !IS_RCS_TABLE_SCHEMA_CODE_COMPLETE) {
                return;
            }
            db.beginTransaction();
            try {
                RcsProviderThreadHelper.createThreadTables(db);
                RcsProviderParticipantHelper.createParticipantTables(db);
                RcsProviderMessageHelper.createRcsMessageTables(db);
                RcsProviderEventHelper.createRcsEventTables(db);
            } catch (Throwable ex) {
                Log.e(TAG, ex.getMessage(), ex);
                break;
            } finally {
                db.endTransaction();
            }
            return;
        }

        Log.e(TAG, "Destroying all old data.");
        localLog("onUpgrade: Calling dropAll() and onCreate(). Upgrading database"
                + " from version " + oldVersion + " to " + currentVersion + "failed.");
        dropAll(db);
        onCreate(db);
    }

    @Override
    public void onDowngrade(SQLiteDatabase db, int oldVersion, int currentVersion) {
        dropAll(db);
        onCreate(db);
    }

    private void dropAll(SQLiteDatabase db) {
        // Clean the database out in order to start over from scratch.
        // We don't need to drop our triggers here because SQLite automatically
        // drops a trigger when its attached database is dropped.
        localLog("****DROPPING ALL SMS-MMS TABLES****");

        //db.execSQL("DROP VIEW IF EXISTS rcs_threads;");
        db.execSQL("DROP TABLE IF EXISTS sys_ipmsg;");

        db.execSQL("DROP VIEW IF EXISTS pdu_restricted;");
        db.execSQL("DROP VIEW IF EXISTS sms_restricted;");

        db.execSQL("DROP TABLE IF EXISTS canonical_addresses");
        db.execSQL("DROP TABLE IF EXISTS threads");
        db.execSQL("DROP TABLE IF EXISTS " + MmsSmsProvider.TABLE_PENDING_MSG);
        db.execSQL("DROP TABLE IF EXISTS sms");
        db.execSQL("DROP TABLE IF EXISTS cellbroadcast");
        db.execSQL("DROP TABLE IF EXISTS words");
        db.execSQL("DROP TABLE IF EXISTS quicktext");
        /// M: Code analyze 008, new feature, support for wappush. @{
        if (MTK_WAPPUSH_SUPPORT) {
            db.execSQL("DROP TABLE IF EXISTS wappush");
        }
        /// @}
        db.execSQL("DROP TABLE IF EXISTS raw");
        db.execSQL("DROP TABLE IF EXISTS attachments");
        db.execSQL("DROP TABLE IF EXISTS thread_ids");
        db.execSQL("DROP TABLE IF EXISTS sr_pending");
        /// M: Add for ip message
        db.execSQL("DROP TABLE IF EXISTS thread_settings");
        db.execSQL("DROP TABLE IF EXISTS " + MmsProvider.TABLE_PDU + ";");
        db.execSQL("DROP TABLE IF EXISTS " + MmsProvider.TABLE_ADDR + ";");
        db.execSQL("DROP TABLE IF EXISTS " + MmsProvider.TABLE_PART + ";");
        db.execSQL("DROP TABLE IF EXISTS " + MmsProvider.TABLE_RATE + ";");
        db.execSQL("DROP TABLE IF EXISTS " + MmsProvider.TABLE_DRM + ";");

        db.execSQL("DROP TABLE IF EXISTS mwi");
        /// M: drop view @{
        db.execSQL("DROP VIEW IF EXISTS " + MmsProvider.VIEW_PDU_RESTRICTED + ";");
        db.execSQL("DROP VIEW IF EXISTS " + SmsProvider.VIEW_SMS_RESTRICTED + ";");
        /// @}
    }

    private void upgradeDatabaseToVersion41(SQLiteDatabase db) {
        db.execSQL("DROP TRIGGER IF EXISTS update_threads_error_on_move_mms");
        db.execSQL("CREATE TRIGGER update_threads_error_on_move_mms " +
                   "  BEFORE UPDATE OF msg_box ON pdu " +
                   "  WHEN (OLD.msg_box = 4 AND NEW.msg_box != 4) " +
                   "  AND (OLD._id IN (SELECT DISTINCT msg_id" +
                   "                   FROM pending_msgs" +
                   "                   WHERE err_type >= 10)) " +
                   "BEGIN " +
                   "  UPDATE threads SET error = error - 1" +
                   "  WHERE _id = OLD.thread_id; " +
                   "END;");
    }

    private void upgradeDatabaseToVersion42(SQLiteDatabase db) {
        db.execSQL("DROP TRIGGER IF EXISTS sms_update_thread_on_delete");
        db.execSQL("DROP TRIGGER IF EXISTS delete_obsolete_threads_sms");
        db.execSQL("DROP TRIGGER IF EXISTS update_threads_error_on_delete_sms");
    }

    private void upgradeDatabaseToVersion43(SQLiteDatabase db) {
        // Add 'has_attachment' column to threads table.
        db.execSQL("ALTER TABLE threads ADD COLUMN has_attachment INTEGER DEFAULT 0");

        updateThreadsAttachmentColumn(db);

        // Add insert and delete triggers for keeping it up to date.
        db.execSQL(PART_UPDATE_THREADS_ON_INSERT_TRIGGER);
        db.execSQL(PART_UPDATE_THREADS_ON_DELETE_TRIGGER);
    }

    private void upgradeDatabaseToVersion44(SQLiteDatabase db) {
        updateThreadsAttachmentColumn(db);

        // add the update trigger for keeping the threads up to date.
        db.execSQL(PART_UPDATE_THREADS_ON_UPDATE_TRIGGER);
    }

    private void upgradeDatabaseToVersion45(SQLiteDatabase db) {
        // Add 'locked' column to sms table.
        db.execSQL("ALTER TABLE sms ADD COLUMN " + Sms.LOCKED + " INTEGER DEFAULT 0");

        // Add 'locked' column to pdu table.
        db.execSQL("ALTER TABLE pdu ADD COLUMN " + Mms.LOCKED + " INTEGER DEFAULT 0");
    }

    private void upgradeDatabaseToVersion46(SQLiteDatabase db) {
        // add the "text" column for caching inline text (e.g. strings) instead of
        // putting them in an external file
        db.execSQL("ALTER TABLE part ADD COLUMN " + Part.TEXT + " TEXT");

        Cursor textRows = db.query(
                "part",
                new String[] { Part._ID, Part._DATA, Part.TEXT},
                "ct = 'text/plain' OR ct == 'application/smil'",
                null,
                null,
                null,
                null);
        ArrayList<String> filesToDelete = new ArrayList<String>();
        try {
            db.beginTransaction();
            if (textRows != null) {
                int partDataColumn = textRows.getColumnIndex(Part._DATA);

                // This code is imperfect in that we can't guarantee that all the
                // backing files get deleted.  For example if the system aborts after
                // the database is updated but before we complete the process of
                // deleting files.
                while (textRows.moveToNext()) {
                    String path = textRows.getString(partDataColumn);
                    if (path != null) {
                        try {
                            InputStream is = new FileInputStream(path);
                            byte [] data = new byte[is.available()];
                            is.read(data);
                            EncodedStringValue v = new EncodedStringValue(data);
                            db.execSQL("UPDATE part SET " + Part._DATA + " = NULL, " +
                                    Part.TEXT + " = ?", new String[] { v.getString() });
                            is.close();
                            filesToDelete.add(path);
                        } catch (IOException e) {
                            // TODO Auto-generated catch block
                            e.printStackTrace();
                        }
                    }
                }
            }
            db.setTransactionSuccessful();
        } finally {
            db.endTransaction();
            for (String pathToDelete : filesToDelete) {
                try {
                    (new File(pathToDelete)).delete();
                } catch (SecurityException ex) {
                    Log.e(TAG, "unable to clean up old mms file for " + pathToDelete, ex);
                }
            }
            if (textRows != null) {
                textRows.close();
            }
        }
    }

    private void upgradeDatabaseToVersion47(SQLiteDatabase db) {
        updateThreadsAttachmentColumn(db);

        // add the update trigger for keeping the threads up to date.
        db.execSQL(PDU_UPDATE_THREADS_ON_UPDATE_TRIGGER);
    }

    private void upgradeDatabaseToVersion48(SQLiteDatabase db) {
        // Add 'error_code' column to sms table.
        db.execSQL("ALTER TABLE sms ADD COLUMN error_code INTEGER DEFAULT 0");
    }

    private void upgradeDatabaseToVersion51(SQLiteDatabase db) {
        db.execSQL("ALTER TABLE sms add COLUMN seen INTEGER DEFAULT 0");
        db.execSQL("ALTER TABLE pdu add COLUMN seen INTEGER DEFAULT 0");

        try {
            // update the existing sms and pdu tables so the new "seen" column is the same as
            // the "read" column for each row.
            ContentValues contentValues = new ContentValues();
            contentValues.put("seen", 1);
            int count = db.update("sms", contentValues, "read=1", null);
            Log.d(TAG, "[MmsSmsDb] upgradeDatabaseToVersion51: updated " + count +
                    " rows in sms table to have READ=1");
            count = db.update("pdu", contentValues, "read=1", null);
            Log.d(TAG, "[MmsSmsDb] upgradeDatabaseToVersion51: updated " + count +
                    " rows in pdu table to have READ=1");
        } catch (Exception ex) {
            Log.e(TAG, "[MmsSmsDb] upgradeDatabaseToVersion51 caught ", ex);
        }
    }

    private void upgradeDatabaseToVersion53(SQLiteDatabase db) {
        db.execSQL("DROP TRIGGER IF EXISTS pdu_update_thread_read_on_update");

        // Updates threads table whenever a message in pdu is updated.
        db.execSQL("CREATE TRIGGER pdu_update_thread_read_on_update AFTER" +
                   "  UPDATE OF " + Mms.READ +
                   "  ON " + MmsProvider.TABLE_PDU + " " +
                   PDU_UPDATE_THREAD_CONSTRAINTS +
                   "BEGIN " +
                   PDU_UPDATE_THREAD_READ_BODY +
                   "END;");
    }
     /// M: Code analyze 006, unknown, for database upgrade. @{
     private void upgradeDatabaseToVersion530100(SQLiteDatabase db) {
        db.execSQL("ALTER TABLE " + MmsProvider.TABLE_PDU + " ADD COLUMN "
                + "service_center" + " TEXT");
    }

    private void upgradeDatabaseToVersion530200(SQLiteDatabase db) {
        db.execSQL("ALTER TABLE threads" + " ADD COLUMN " + "readcount" + " INTEGER");

        db.execSQL("DROP TRIGGER IF EXISTS sms_update_thread_on_insert");
        /// M: Updates threads table whenever a message is added to sms.
        db.execSQL("CREATE TRIGGER sms_update_thread_on_insert AFTER INSERT ON sms " +
                   SMS_UPDATE_THREAD_DATE_SNIPPET_COUNT_ON_UPDATE);

        db.execSQL("DROP TRIGGER IF EXISTS sms_update_thread_date_subject_on_update");
         /// M: Updates threads table whenever a message in sms is updated.
        db.execSQL("CREATE TRIGGER sms_update_thread_date_subject_on_update AFTER" +
                   "  UPDATE OF " + Sms.DATE + ", " + Sms.BODY + ", " + Sms.TYPE +
                   "  ON sms " +
                   SMS_UPDATE_THREAD_DATE_SNIPPET_COUNT_ON_UPDATE);

        db.execSQL("DROP TRIGGER IF EXISTS sms_update_thread_read_on_update");
        /// M: Updates threads table whenever a message in sms is updated.
        db.execSQL("CREATE TRIGGER sms_update_thread_read_on_update AFTER" +
                   "  UPDATE OF " + Sms.READ +
                   "  ON sms " +
                   "BEGIN " +
                   SMS_UPDATE_THREAD_READ_BODY +
                   "END;");

        db.execSQL("DROP TRIGGER IF EXISTS pdu_update_thread_on_insert");
        /// M: Updates threads table whenever a message is added to pdu.
        db.execSQL("CREATE TRIGGER pdu_update_thread_on_insert AFTER INSERT ON " +
                   MmsProvider.TABLE_PDU + " " +
                   PDU_UPDATE_THREAD_CONSTRAINTS +
                   PDU_UPDATE_THREAD_DATE_SNIPPET_COUNT_ON_UPDATE);

        db.execSQL("DROP TRIGGER IF EXISTS pdu_update_thread_date_subject_on_update");
        /// M: Updates threads table whenever a message in pdu is updated.
        db.execSQL("CREATE TRIGGER pdu_update_thread_date_subject_on_update AFTER" +
                   "  UPDATE OF " + Mms.DATE + ", " + Mms.SUBJECT + ", " + Mms.MESSAGE_BOX +
                   "  ON " + MmsProvider.TABLE_PDU + " " +
                   PDU_UPDATE_THREAD_CONSTRAINTS +
                   PDU_UPDATE_THREAD_DATE_SNIPPET_COUNT_ON_UPDATE);

        db.execSQL("DROP TRIGGER IF EXISTS pdu_update_thread_read_on_update");
        /// M: Updates threads table whenever a message in pdu is updated.
        db.execSQL("CREATE TRIGGER pdu_update_thread_read_on_update AFTER" +
            "  UPDATE OF " + Mms.READ +
            "  ON " + MmsProvider.TABLE_PDU + " " +
            PDU_UPDATE_THREAD_CONSTRAINTS +
            "BEGIN " +
            PDU_UPDATE_THREAD_READ_BODY +
            "END;");

        db.execSQL("DROP TRIGGER IF EXISTS cb_update_thread_on_insert");
        /// M: Triggers for CB
        db.execSQL("CREATE TRIGGER cb_update_thread_on_insert AFTER INSERT ON cellbroadcast "
                + CB_UPDATE_THREAD_DATE_SNIPPET_COUNT_ON_UPDATE);

        db.execSQL("DROP TRIGGER IF EXISTS cb_update_thread_read_on_update");
        db.execSQL("CREATE TRIGGER cb_update_thread_read_on_update AFTER"
            + "  UPDATE OF " + MtkTelephony.SmsCb.READ + "  ON cellbroadcast "
            + "BEGIN " + CB_UPDATE_THREAD_READ_BODY + "END;");

    }

    private void upgradeDatabaseToVersion530300(SQLiteDatabase db) {
        db.execSQL("ALTER TABLE threads ADD COLUMN status INTEGER DEFAULT 0");
    }

    private void upgradeDatabaseToVersion540000(SQLiteDatabase db) {
        /// M: Add 'date_sent' column to sms table.
        db.execSQL("ALTER TABLE sms ADD COLUMN " + Sms.DATE_SENT + " INTEGER DEFAULT 0");

        /// M: Add 'date_sent' column to pdu table.
        db.execSQL("ALTER TABLE pdu ADD COLUMN " + Mms.DATE_SENT + " INTEGER DEFAULT 0");

        /// M: Add 'date_sent' column to cb table.
        db.execSQL("ALTER TABLE cellbroadcast ADD COLUMN date_sent INTEGER DEFAULT 0");
    }

    private void upgradeDatabaseToVersion550000(SQLiteDatabase db) {
        /// M: Drop removed triggers
        db.execSQL("DROP TRIGGER IF EXISTS delete_obsolete_threads_pdu");
        db.execSQL("DROP TRIGGER IF EXISTS delete_obsolete_threads_when_update_pdu");
    }

    /// M: Add for ip message @{
    private void upgradeDatabaseToVersion550100(SQLiteDatabase db) {
        // Add 'ipmsg_id' column to sms table.
        db.execSQL("ALTER TABLE sms ADD COLUMN ipmsg_id INTEGER DEFAULT 0");

        // Add thread_settings table.
        db.execSQL("CREATE TABLE thread_settings("
                + MtkThreadSettings._ID + " INTEGER PRIMARY KEY,"
                + MtkThreadSettings.THREAD_ID + " INTEGER,"
                + MtkThreadSettings.SPAM + " INTEGER DEFAULT 0,"
                + MtkThreadSettings.NOTIFICATION_ENABLE + " INTEGER DEFAULT 1,"
                + MtkThreadSettings.MUTE + " INTEGER DEFAULT 0,"
                + MtkThreadSettings.MUTE_START + " INTEGER DEFAULT 0,"
                + MtkThreadSettings.RINGTONE + " TEXT,"
                + MtkThreadSettings.WALLPAPER + " TEXT,"
                + MtkThreadSettings.VIBRATE + " INTEGER DEFAULT 1);");

        // Insert item into thread_settings when a item inserted into threads table
        db.execSQL("CREATE TRIGGER insert_thread_settings_when_insert_threads " +
                   "AFTER INSERT ON threads " +
                   "BEGIN " +
                   "  INSERT INTO thread_settings " +
                   "    (" + MtkThreadSettings.THREAD_ID +
                   "    ," + MtkThreadSettings.SPAM +
                   "    ," + MtkThreadSettings.NOTIFICATION_ENABLE +
                   "    ," + MtkThreadSettings.MUTE +
                   "    ," + MtkThreadSettings.MUTE_START +
                   "    ," + MtkThreadSettings.RINGTONE +
                   "    ," + MtkThreadSettings.WALLPAPER +
                   "    ," + MtkThreadSettings.VIBRATE + ") " +
                   "  VALUES " +
                   "    (new." + MtkThreadSettings._ID + "," +
                   "     0,1,0,0,'','',1); " +
                   "END;");

        // Delete item from thread_settings when a item deleted from threads table
        db.execSQL("CREATE TRIGGER delete_thread_settings_when_delete_threads " +
                   "AFTER DELETE ON threads " +
                   "BEGIN " +
                   "  DELETE FROM thread_settings " +
                   "    WHERE " + MtkThreadSettings.THREAD_ID + "=old._id; " +
                   "END;");

        // add triggers to update read count column
        db.execSQL("DROP TRIGGER IF EXISTS sms_update_thread_on_insert");
        // Updates threads table whenever a message is added to sms.
        db.execSQL("CREATE TRIGGER sms_update_thread_on_insert AFTER INSERT ON sms " +
                   SMS_UPDATE_THREAD_DATE_SNIPPET_COUNT_ON_UPDATE);

        db.execSQL("DROP TRIGGER IF EXISTS sms_update_thread_date_subject_on_update");
        // Updates threads table whenever a message in sms is updated.
        db.execSQL("CREATE TRIGGER sms_update_thread_date_subject_on_update AFTER" +
                   "  UPDATE OF " + Sms.DATE + ", " + Sms.BODY + ", " + Sms.TYPE +
                   "  ON sms " +
                   SMS_UPDATE_THREAD_DATE_SNIPPET_COUNT_ON_UPDATE);

        db.execSQL("DROP TRIGGER IF EXISTS sms_update_thread_read_on_update");
        // Updates threads table whenever a message in sms is updated.
        db.execSQL("CREATE TRIGGER sms_update_thread_read_on_update AFTER" +
                   "  UPDATE OF " + Sms.READ +
                   "  ON sms " +
                   "BEGIN " +
                   SMS_UPDATE_THREAD_READ_BODY +
                   UPDATE_THREAD_READ_COUNT +
                   "END;");

        db.execSQL("DROP TRIGGER IF EXISTS pdu_update_thread_on_insert");
        // Updates threads table whenever a message is added to pdu.
        db.execSQL("CREATE TRIGGER pdu_update_thread_on_insert AFTER INSERT ON " +
                   MmsProvider.TABLE_PDU + " " +
                   PDU_UPDATE_THREAD_CONSTRAINTS +
                   PDU_UPDATE_THREAD_DATE_SNIPPET_COUNT_ON_INSERT);

        db.execSQL("DROP TRIGGER IF EXISTS pdu_update_thread_date_subject_on_update");
        // Updates threads table whenever a message in pdu is updated.
        db.execSQL("CREATE TRIGGER pdu_update_thread_date_subject_on_update AFTER" +
                   "  UPDATE OF " + Mms.DATE + ", " + Mms.SUBJECT + ", " + Mms.MESSAGE_BOX +
                   "  ON " + MmsProvider.TABLE_PDU + " " +
                   PDU_UPDATE_THREAD_CONSTRAINTS +
                   PDU_UPDATE_THREAD_DATE_SNIPPET_COUNT_ON_UPDATE);

        db.execSQL("DROP TRIGGER IF EXISTS pdu_update_thread_read_on_update");
        // Updates threads table whenever a message in pdu is updated.
        db.execSQL("CREATE TRIGGER pdu_update_thread_read_on_update AFTER" +
            "  UPDATE OF " + Mms.READ +
            "  ON " + MmsProvider.TABLE_PDU + " " +
            PDU_UPDATE_THREAD_CONSTRAINTS +
            "BEGIN " +
            PDU_UPDATE_THREAD_READ_BODY +
            UPDATE_THREAD_READ_COUNT +
            "END;");

        db.execSQL("DROP TRIGGER IF EXISTS cb_update_thread_on_insert");
        // Triggers for CB
        db.execSQL("CREATE TRIGGER cb_update_thread_on_insert AFTER INSERT ON cellbroadcast "
                + CB_UPDATE_THREAD_DATE_SNIPPET_COUNT_ON_UPDATE);

        db.execSQL("DROP TRIGGER IF EXISTS cb_update_thread_read_on_update");
        db.execSQL("CREATE TRIGGER cb_update_thread_read_on_update AFTER"
            + "  UPDATE OF " + MtkTelephony.SmsCb.READ + "  ON cellbroadcast "
            + "BEGIN " + CB_UPDATE_THREAD_READ_BODY + UPDATE_THREAD_READ_COUNT + "END;");

        Log.v(TAG, "old thread thread_setting update begin");
        Cursor c = null;
        c = db.query("threads", new String[] {"_id"}, null, null, null, null, null);
        if (c != null) {
            try {
                int count = c.getCount();
                if (count > 0) {
                    while (c.moveToNext()) {
                        long thread_id = c.getLong(0);
                        ContentValues cv = new ContentValues();
                        cv.put(MtkThreadSettings.THREAD_ID, thread_id);
                        db.insert("thread_settings", null, cv);
                    }
                }
            } finally {
                if (c != null) {
                    c.close();
                }
            }
        }
        Log.v(TAG, "old thread thread_setting update end");
    }

    private void upgradeDatabaseToVersion560000(SQLiteDatabase db) {
        // Clear out bad rows, those with empty threadIds, from the pdu table.
        db.execSQL("DELETE FROM " + MmsProvider.TABLE_PDU + " WHERE " + Mms.THREAD_ID + " IS NULL");
        // Add 'text_only' column to pdu table.
        db.execSQL("ALTER TABLE " + MmsProvider.TABLE_PDU + " ADD COLUMN " + Mms.TEXT_ONLY +
                " INTEGER DEFAULT 0");
        /// M: for pdu readcount.
        db.execSQL("DROP TRIGGER IF EXISTS pdu_update_thread_on_delete");
        db.execSQL("CREATE TRIGGER pdu_update_thread_on_delete " +
            "AFTER DELETE ON pdu " +
            "BEGIN " +
            "  UPDATE threads SET " +
            "     date = (strftime('%s','now') * 1000)" +
            "  WHERE threads._id = old." + Mms.THREAD_ID + "; " +
            UPDATE_THREAD_COUNT_ON_OLD +
            UPDATE_THREAD_READ_COUNT_OLD +
            UPDATE_THREAD_SNIPPET_SNIPPET_CS_ON_DELETE +
            "END;");
        /// M: when sms is deleted update readcount.
        db.execSQL("DROP TRIGGER IF EXISTS sms_update_thread_on_delete");
        db.execSQL("CREATE TRIGGER sms_update_thread_on_delete " +
            "AFTER DELETE ON sms " +
            "BEGIN " +
            UPDATE_THREAD_COUNT_ON_OLD +
            UPDATE_THREAD_READ_COUNT_OLD +
            "END;");

        /// M: fix bug ALPS00433858, Updates threads table whenever a message in pdu is updated.
        db.execSQL("DROP TRIGGER IF EXISTS pdu_update_thread_read_on_update");
        db.execSQL("CREATE TRIGGER pdu_update_thread_read_on_update AFTER" +
                   "  UPDATE OF " + Mms.READ +
                   "  ON " + MmsProvider.TABLE_PDU + " " +
                   PDU_UPDATE_THREAD_CONSTRAINTS +
                   "BEGIN " +
                   PDU_UPDATE_THREAD_READ_BODY +
                   /// M: fix bug ALPS00433858, update read_count
                   UPDATE_THREAD_READ_COUNT +
                   "END;");
    }

     /// M: for ct new feature,add some columns to sms,pdu,raw table.
     /// change and add trigger for ct new feature. {
    private void upgradeDatabaseToVersion560200(SQLiteDatabase db) {

        db.execSQL("ALTER TABLE threads ADD COLUMN date_sent INTEGER DEFAULT 0");

        db.execSQL("ALTER TABLE raw ADD COLUMN recv_time INTEGER");
        db.execSQL("ALTER TABLE raw ADD COLUMN upload_flag INTEGER");

        db.execSQL("CREATE TRIGGER sms_update_thread_on_delete " +
            "AFTER DELETE ON sms " +
            "BEGIN " +
            UPDATE_THREAD_COUNT_ON_OLD +
            UPDATE_THREAD_READ_COUNT_OLD +
            "  UPDATE threads SET date_sent = " +
            "   (SELECT date_sent FROM" +
            "     (SELECT date_sent * 1000 as date_sent, date * 1000 AS date, thread_id " +
            "FROM pdu WHERE m_type=132 OR m_type=130 OR m_type=128" +
            "      UNION SELECT date_sent, date, thread_id FROM sms)" +
            "    WHERE thread_id = OLD.thread_id ORDER BY date DESC LIMIT 1) " +
            "  WHERE threads._id = OLD.thread_id; " +
            "END;");

    }
    /// M: @}

    /// M: because of triggers on sms and pdu, delete a large number of sms/pdu through an
    /// atomic operation will cost too much time. To improve deleting performance,
    /// remove triggers pdu_update_thread_on_delete and sms_update_thread_on_delete. @{
    private void upgradeDatabaseToVersion560300(SQLiteDatabase db) {
        db.execSQL("DROP TRIGGER IF EXISTS pdu_update_thread_on_delete");
        db.execSQL("DROP TRIGGER IF EXISTS sms_update_thread_on_delete");
    }
    /// @}

    private void upgradeDatabaseToVersion560500(SQLiteDatabase db) {
        db.execSQL("DROP TRIGGER IF EXISTS sms_update_thread_date_subject_on_update");
        db.execSQL("CREATE TRIGGER sms_update_thread_date_subject_on_update AFTER" +
                "  UPDATE OF " + Sms.DATE + ", " + Sms.BODY + ", " + Sms.TYPE +
                "  ON sms " +
                SMS_UPDATE_THREAD_DATE_SNIPPET_COUNT_ON_UPDATE);
    }

    private void upgradeDatabaseToVersion600000(SQLiteDatabase db) {
        // replace column sim_id by sub_id follow google released.
        db.execSQL("CREATE TABLE pdu_temp (" +
                Mms._ID + " INTEGER PRIMARY KEY AUTOINCREMENT," +
                Mms.THREAD_ID + " INTEGER," +
                Mms.DATE + " INTEGER," +
                Mms.DATE_SENT + " INTEGER DEFAULT 0," +
                Mms.MESSAGE_BOX + " INTEGER," +
                Mms.READ + " INTEGER DEFAULT 0," +
                Mms.MESSAGE_ID + " TEXT," +
                Mms.SUBJECT + " TEXT," +
                Mms.SUBJECT_CHARSET + " INTEGER," +
                Mms.CONTENT_TYPE + " TEXT," +
                Mms.CONTENT_LOCATION + " TEXT," +
                Mms.EXPIRY + " INTEGER," +
                Mms.MESSAGE_CLASS + " TEXT," +
                Mms.MESSAGE_TYPE + " INTEGER," +
                Mms.MMS_VERSION + " INTEGER," +
                Mms.MESSAGE_SIZE + " INTEGER," +
                Mms.PRIORITY + " INTEGER," +
                Mms.READ_REPORT + " INTEGER," +
                Mms.REPORT_ALLOWED + " INTEGER," +
                Mms.RESPONSE_STATUS + " INTEGER," +
                Mms.STATUS + " INTEGER," +
                Mms.TRANSACTION_ID + " TEXT," +
                Mms.RETRIEVE_STATUS + " INTEGER," +
                Mms.RETRIEVE_TEXT + " TEXT," +
                Mms.RETRIEVE_TEXT_CHARSET + " INTEGER," +
                Mms.READ_STATUS + " INTEGER," +
                Mms.CONTENT_CLASS + " INTEGER," +
                Mms.RESPONSE_TEXT + " TEXT," +
                Mms.DELIVERY_TIME + " INTEGER," +
                Mms.DELIVERY_REPORT + " INTEGER," +
                Mms.LOCKED + " INTEGER DEFAULT 0," +
                Mms.SUBSCRIPTION_ID + " INTEGER DEFAULT -1, " +
                "service_center TEXT," +
                Mms.SEEN + " INTEGER DEFAULT 0," +
                Mms.TEXT_ONLY + " INTEGER DEFAULT 0" +
                ");");
        db.execSQL("INSERT INTO pdu_temp SELECT " + Mms._ID + ", " + Mms.THREAD_ID + ", "
                + Mms.DATE + ", " + Mms.DATE_SENT + ", " + Mms.MESSAGE_BOX + ", " + Mms.READ + ", "
                + Mms.MESSAGE_ID + ", " + Mms.SUBJECT + ", " + Mms.SUBJECT_CHARSET + ", "
                + Mms.CONTENT_TYPE + ", " + Mms.CONTENT_LOCATION + ", " + Mms.EXPIRY + ", "
                + Mms.MESSAGE_CLASS + ", " + Mms.MESSAGE_TYPE + ", " + Mms.MMS_VERSION + ", "
                + Mms.MESSAGE_SIZE + ", " + Mms.PRIORITY + ", " + Mms.READ_REPORT + ", "
                + Mms.REPORT_ALLOWED + ", " + Mms.RESPONSE_STATUS + ", " + Mms.STATUS + ", "
                + Mms.TRANSACTION_ID + ", " + Mms.RETRIEVE_STATUS + ", "
                + Mms.RETRIEVE_TEXT + ", " + Mms.RETRIEVE_TEXT_CHARSET
                + ", " + Mms.READ_STATUS + ", "
                + Mms.CONTENT_CLASS + ", " + Mms.RESPONSE_TEXT + ", " + Mms.DELIVERY_TIME + ", "
                + Mms.DELIVERY_REPORT + ", " + Mms.LOCKED + ", sim_id, service_center, "
                + Mms.SEEN + ", " + Mms.TEXT_ONLY + " from pdu;");
        db.execSQL("DROP TABLE pdu;");
        db.execSQL("ALTER TABLE pdu_temp RENAME TO pdu;");

        db.execSQL("CREATE TABLE pending_msgs_temp (" +
                PendingMessages._ID + " INTEGER PRIMARY KEY," +
                PendingMessages.PROTO_TYPE + " INTEGER," +
                PendingMessages.MSG_ID + " INTEGER," +
                PendingMessages.MSG_TYPE + " INTEGER," +
                PendingMessages.ERROR_TYPE + " INTEGER," +
                PendingMessages.ERROR_CODE + " INTEGER," +
                PendingMessages.RETRY_INDEX + " INTEGER NOT NULL DEFAULT 0," +
                PendingMessages.DUE_TIME + " INTEGER," +
                PendingMessages.SUBSCRIPTION_ID + " INTEGER DEFAULT 0, " +
                PendingMessages.LAST_TRY + " INTEGER);");
        db.execSQL("INSERT INTO pending_msgs_temp SELECT * from pending_msgs;");
        db.execSQL("DROP TABLE pending_msgs;");
        db.execSQL("ALTER TABLE pending_msgs_temp RENAME TO pending_msgs;");

        db.execSQL("CREATE TABLE sms_temp (" +
                "_id INTEGER PRIMARY KEY," +
                "thread_id INTEGER," +
                "address TEXT," +
                "m_size INTEGER," +
                "person INTEGER," +
                "date INTEGER," +
                "date_sent INTEGER DEFAULT 0," +
                "protocol INTEGER," +
                "read INTEGER DEFAULT 0," +
                "status INTEGER DEFAULT -1," +
                "type INTEGER," +
                "reply_path_present INTEGER," +
                "subject TEXT," +
                "body TEXT," +
                "service_center TEXT," +
                "locked INTEGER DEFAULT 0," +
                "sub_id LONG DEFAULT -1, " +
                "error_code INTEGER DEFAULT 0," +
                "seen INTEGER DEFAULT 0," +
                "ipmsg_id INTEGER DEFAULT 0" +
                ");");
        db.execSQL("INSERT INTO sms_temp SELECT * from sms;");
        db.execSQL("DROP TABLE sms;");
        db.execSQL("ALTER TABLE sms_temp RENAME TO sms;");

        db.execSQL("CREATE TABLE raw_temp (" +
                "_id INTEGER PRIMARY KEY," +
                "date INTEGER," +
                "reference_number INTEGER," +
                "count INTEGER," +
                "sequence INTEGER," +
                "destination_port INTEGER," +
                "address TEXT," +
                "sub_id LONG DEFAULT -1, " +
                "pdu TEXT," +
                "recv_time INTEGER," +
                "upload_flag INTEGER" +
                ");");
        db.execSQL("INSERT INTO raw_temp SELECT * from raw;");
        db.execSQL("DROP TABLE raw;");
        db.execSQL("ALTER TABLE raw_temp RENAME TO raw;");

        if (MTK_WAPPUSH_SUPPORT) {
            db.execSQL("CREATE TABLE wappush_temp ("
                    + "_id INTEGER PRIMARY KEY,"
                    + "thread_id INTEGER,"
                    + "address TEXT NOT NULL,"
                    + "service_center TEXT NOT NULL,"
                    + "seen INTEGER DEFAULT 0,"
                    + "read INTEGER DEFAULT 0,"
                    + "locked INTEGER DEFAULT 0,"
                    + "error INTEGER DEFAULT 0,"
                    + Mms.SUBSCRIPTION_ID + " INTEGER DEFAULT -1, "
                    + "date INTEGER,"
                    + "type INTEGER DEFAULT 0,"
                    + "siid TEXT,"
                    + "url TEXT," + "action INTEGER," + "created INTEGER,"
                    + "expiration INTEGER," + "text TEXT" + ");");
            db.execSQL("INSERT INTO wappush_temp SELECT * from wappush;");
            db.execSQL("DROP TABLE wappush;");
            db.execSQL("ALTER TABLE wappush_temp RENAME TO wappush;");
        } else {
            db.execSQL("DROP TABLE IF EXISTS wappush");
        }

        db.execSQL("CREATE TABLE cellbroadcast_temp "
                + "(_id INTEGER PRIMARY KEY," + Mms.SUBSCRIPTION_ID + " INTEGER DEFAULT -1,"
                + "locked INTEGER DEFAULT 0,"
                + "body TEXT," + "channel_id INTEGER," + "thread_id INTEGER,"
                + "read INTEGER DEFAULT 0," + "seen INTEGER DEFAULT 0,"
                + "date_sent INTEGER DEFAULT 0," + "date INTEGER);");
        db.execSQL("INSERT INTO cellbroadcast_temp SELECT * from cellbroadcast;");
        db.execSQL("DROP TABLE cellbroadcast;");
        db.execSQL("ALTER TABLE cellbroadcast_temp RENAME TO cellbroadcast;");


        db.execSQL("ALTER TABLE " + MmsProvider.TABLE_PDU +" ADD COLUMN "
                + Mms.CREATOR + " TEXT");

        db.execSQL("ALTER TABLE " + SmsProvider.TABLE_SMS +" ADD COLUMN "
                + Sms.CREATOR + " TEXT");
    /*
    }

    private void upgradeDatabaseToVersion60(SQLiteDatabase db) {
    */
        db.execSQL("ALTER TABLE " + MmsSmsProvider.TABLE_THREADS +" ADD COLUMN "
                + Threads.ARCHIVED + " INTEGER DEFAULT 0");

        createCommonTriggers(db);
        createMmsTriggers(db);
        db.execSQL("CREATE TRIGGER sms_words_update AFTER UPDATE OF body " +
                "ON sms BEGIN UPDATE words " +
                " SET index_text = NEW.body WHERE (source_id=NEW._id AND table_to_use=1); " +
                " END;");
        db.execSQL("CREATE TRIGGER sms_words_delete AFTER DELETE ON sms BEGIN DELETE FROM " +
                "  words WHERE source_id = OLD._id AND table_to_use = 1; END;");

        /// M: monitor the wappush table
        db.execSQL("CREATE TRIGGER wp_words_update AFTER UPDATE ON wappush BEGIN UPDATE words " +
                " SET index_text = coalesce(NEW.text||' '||NEW.url,NEW.text,NEW.url) " +
                "WHERE (source_id=NEW._id AND table_to_use=3); END;");
        db.execSQL("CREATE TRIGGER wp_words_delete AFTER DELETE ON wappush BEGIN DELETE FROM " +
                " words WHERE source_id = OLD._id AND table_to_use = 3; END;");

    }

    private void upgradeDatabaseToVersion600100(SQLiteDatabase db) {
        // fix google issue while L migration, if send mms failed, pending_msgs will be delete,
        // and retry send will failed.
        db.execSQL("DROP TRIGGER IF EXISTS delete_mms_pending_on_update");
        db.execSQL("DROP TRIGGER IF EXISTS insert_mms_pending_on_update");

        db.execSQL("CREATE TRIGGER delete_mms_pending_on_update " +
                   "AFTER UPDATE ON " + MmsProvider.TABLE_PDU + " " +
                   "WHEN old." + Mms.MESSAGE_BOX + "=" + Mms.MESSAGE_BOX_OUTBOX +
                   "  AND new." + Mms.MESSAGE_BOX + "!=" + Mms.MESSAGE_BOX_OUTBOX +
                   "  AND new." + Mms.MESSAGE_BOX + "!=" + Mms.MESSAGE_BOX_FAILED + " " +
                   "BEGIN " +
                   "  DELETE FROM " + MmsSmsProvider.TABLE_PENDING_MSG +
                   "  WHERE " + PendingMessages.MSG_ID + "=new._id; " +
                   "END;");

        db.execSQL("CREATE TRIGGER insert_mms_pending_on_update " +
                   "AFTER UPDATE ON pdu " +
                   "WHEN new." + Mms.MESSAGE_TYPE + "=" + PduHeaders.MESSAGE_TYPE_SEND_REQ +
                   "  AND new." + Mms.MESSAGE_BOX + "=" + Mms.MESSAGE_BOX_OUTBOX +
                   "  AND old." + Mms.MESSAGE_BOX + "!=" + Mms.MESSAGE_BOX_OUTBOX +
                   "  AND old." + Mms.MESSAGE_BOX + "!=" + Mms.MESSAGE_BOX_FAILED + " " +
                   "BEGIN " +
                   "  INSERT INTO " + MmsSmsProvider.TABLE_PENDING_MSG +
                   "    (" + PendingMessages.PROTO_TYPE + "," +
                   "     " + PendingMessages.MSG_ID + "," +
                   "     " + PendingMessages.MSG_TYPE + "," +
                   "     " + PendingMessages.ERROR_TYPE + "," +
                   "     " + PendingMessages.ERROR_CODE + "," +
                   "     " + PendingMessages.RETRY_INDEX + "," +
                   "     " + PendingMessages.DUE_TIME + ") " +
                   "  VALUES " +
                   "    (" + MmsSms.MMS_PROTO + "," +
                   "      new." + BaseColumns._ID + "," +
                   "      new." + Mms.MESSAGE_TYPE + ",0,0,0,0);" +
                   "END;");
    }

    private void upgradeDatabaseToVersion600200(SQLiteDatabase db) {
        // fix L google issue ALPS01767523, if make a failed MMS editable and then resend,
        // old pending_msgs can't be deleted, will become into two MMS.
        db.execSQL("DROP TRIGGER IF EXISTS delete_mms_pending_on_update");

        db.execSQL("CREATE TRIGGER delete_mms_pending_on_update " +
                   "AFTER UPDATE ON " + MmsProvider.TABLE_PDU + " " +
                   "WHEN (old." + Mms.MESSAGE_BOX + "=" + Mms.MESSAGE_BOX_OUTBOX +
                   "   OR old." + Mms.MESSAGE_BOX + "=" + Mms.MESSAGE_BOX_FAILED + ")" +
                   "  AND new." + Mms.MESSAGE_BOX + "!=" + Mms.MESSAGE_BOX_OUTBOX +
                   "  AND new." + Mms.MESSAGE_BOX + "!=" + Mms.MESSAGE_BOX_FAILED + " " +
                   "BEGIN " +
                   "  DELETE FROM " + MmsSmsProvider.TABLE_PENDING_MSG +
                   "  WHERE " + PendingMessages.MSG_ID + "=new._id; " +
                   "END;");
    }

    private void upgradeDatabaseToVersion600300(SQLiteDatabase db) {
        // fix ALPS01840084, update the error flag of threads after a text message was
        // failed to send/delivery/receive.
        db.execSQL("DROP TRIGGER IF EXISTS update_threads_error_on_update_sms");
        db.execSQL("CREATE TRIGGER update_threads_error_on_update_sms " +
                   "  AFTER UPDATE OF type, status ON sms" +
                   "  WHEN (OLD.type != 5 AND NEW.type = 5)" +
                   "    OR (OLD.type = 5 AND NEW.type != 5) " +
                   "    OR (OLD.status >= 64 AND NEW.status < 64) " +
                   "    OR (OLD.status < 64 AND NEW.status >= 64) " +
                   "BEGIN " +
                   "  UPDATE threads SET error = " +
                   "    CASE" +
                   "      WHEN NEW.type = 5 or NEW.status >= 64 THEN error + 1" +
                   "      ELSE error - 1" +
                   "    END " +
                   "  WHERE _id = NEW.thread_id; " +
                   "END;");
    }

    private void upgradeDatabaseToVersion601000(SQLiteDatabase db) {
        db.execSQL("CREATE VIEW " + SmsProvider.VIEW_SMS_RESTRICTED + " AS " +
                   "SELECT * FROM " + SmsProvider.TABLE_SMS + " WHERE " +
                   Sms.TYPE + "=" + Sms.MESSAGE_TYPE_INBOX +
                   " OR " +
                   Sms.TYPE + "=" + Sms.MESSAGE_TYPE_SENT + ";");
        db.execSQL("CREATE VIEW " + MmsProvider.VIEW_PDU_RESTRICTED + "  AS " +
                   "SELECT * FROM " + MmsProvider.TABLE_PDU + " WHERE " +
                   "(" + Mms.MESSAGE_BOX + "=" + Mms.MESSAGE_BOX_INBOX +
                   " OR " +
                   Mms.MESSAGE_BOX + "=" + Mms.MESSAGE_BOX_SENT + ")" +
                   " AND " +
                   "(" + Mms.MESSAGE_TYPE + "!=" + PduHeaders.MESSAGE_TYPE_NOTIFICATION_IND + ");");

    }

    private void upgradeDatabaseToVersion601021(SQLiteDatabase db) {
        db.execSQL("DROP TRIGGER IF EXISTS update_threads_error_on_move_mms");
        db.execSQL("CREATE TRIGGER update_threads_error_on_move_mms " +
                   "  BEFORE UPDATE OF msg_box ON pdu " +
                   "  WHEN ((OLD.msg_box = 4 OR OLD.msg_box = 5) " +
                   "  AND NEW.msg_box != 4 AND NEW.msg_box != 5) " +
                   "  AND (OLD._id IN (SELECT DISTINCT msg_id" +
                   "                   FROM pending_msgs" +
                   "                   WHERE err_type >= 10)) " +
                   "BEGIN " +
                   "  UPDATE threads SET error = error - 1" +
                   "  WHERE _id = OLD.thread_id; " +
                   "END;");
    }

    private void upgradeDatabaseToVersion621000(SQLiteDatabase db) {
        // When a non-FBE device is upgraded to N, all MMS attachment files are moved from
        // /data/data to /data/user_de. We need to update the paths stored in the parts table to
        // reflect this change.
        String newPartsDirPath;
        try {
            newPartsDirPath = mContext.getDir(MmsProvider.PARTS_DIR_NAME, 0).getCanonicalPath();
        }
        catch (IOException e){
            Log.e(TAG, "openFile: check file path failed " + e, e);
            return;
        }

        // The old path of the part files will be something like this:
        //   /data/data/0/com.android.providers.telephony/app_parts
        // The new path of the part files will be something like this:
        //   /data/user_de/0/com.android.providers.telephony/app_parts
        int partsDirIndex = newPartsDirPath.lastIndexOf(
            File.separator, newPartsDirPath.lastIndexOf(MmsProvider.PARTS_DIR_NAME));
        String partsDirName = newPartsDirPath.substring(partsDirIndex) + File.separator;
        // The query to update the part path will be:
        //   UPDATE part SET _data = '/data/user_de/0/com.android.providers.telephony' ||
        //                           SUBSTR(_data, INSTR(_data, '/app_parts/'))
        //   WHERE INSTR(_data, '/app_parts/') > 0
        db.execSQL("UPDATE " + MmsProvider.TABLE_PART +
            " SET " + Part._DATA + " = '" + newPartsDirPath.substring(0, partsDirIndex) + "' ||" +
            " SUBSTR(" + Part._DATA + ", INSTR(" + Part._DATA + ", '" + partsDirName + "'))" +
            " WHERE INSTR(" + Part._DATA + ", '" + partsDirName + "') > 0");
    }

    private void upgradeDatabaseToVersion631000(SQLiteDatabase db) {
        db.execSQL("ALTER TABLE " + SmsProvider.TABLE_RAW +" ADD COLUMN deleted INTEGER DEFAULT 0");
    }

    private void upgradeDatabaseToVersion641000(SQLiteDatabase db) {
        db.execSQL("ALTER TABLE " + SmsProvider.TABLE_RAW +" ADD COLUMN message_body TEXT");
    }

    private void upgradeDatabaseToVersion661000(SQLiteDatabase db) {
      try{
         db.execSQL("ALTER TABLE "
                + SmsProvider.TABLE_RAW +" ADD COLUMN display_originating_addr TEXT");
      }catch(Exception e){
         Log.e(TAG, "got exception upgradeDatabaseToVersion661000: " + e.toString());
      }
      createThreadIdDateIndex(db);
    }

    private void upgradeDatabaseToVersion670010(SQLiteDatabase db) {
        db.execSQL("DROP TRIGGER IF EXISTS update_threads_error_on_update_sms");
        db.execSQL("CREATE TRIGGER update_threads_error_on_update_sms " +
                   "  AFTER UPDATE OF type ON sms" +
                   "  WHEN (OLD.type != 5 AND NEW.type = 5)" +
                   "    OR (OLD.type = 5 AND NEW.type != 5) " +
                   "BEGIN " +
                   "  UPDATE threads SET error = " +
                   "    CASE" +
                   "      WHEN NEW.type = 5 THEN error + 1" +
                   "      ELSE error - 1" +
                   "    END " +
                   "  WHERE _id = NEW.thread_id; " +
                   "END;");
    }


    // op01 rcs;Multi-user sms mms cb;op09
    private void upgradeDatabaseToVersion670020(SQLiteDatabase db) {
      //table
        db.execSQL("DROP TABLE IF EXISTS rcs_message;");
        db.execSQL("DROP TABLE IF EXISTS rcs_conversations;");

        //view
        db.execSQL("DROP VIEW IF EXISTS rcs_threads;");
        db.execSQL("DROP VIEW IF EXISTS rcs_threads_dapi;");

        //trigger
        db.execSQL("DROP TRIGGER IF EXISTS delete_conversation_after_thread_delete;");
        db.execSQL("DROP TRIGGER IF EXISTS update_rcs_thread_after_insert_rcs_message;");
        db.execSQL("DROP TRIGGER IF EXISTS update_rcs_thread_after_insert_rcs_message_dapi;");
        db.execSQL("DROP TRIGGER IF EXISTS update_rcs_thread_after_update_rcs_message_status;");
        db.execSQL("DROP TRIGGER IF EXISTS " +
                   "update_rcs_thread_after_update_rcs_message_status_dapi;");
        db.execSQL("DROP TRIGGER IF EXISTS update_rcs_thread_after_update_rcs_blocked_dapi;");
        db.execSQL("DROP TRIGGER IF EXISTS rcs_words_update;");
        db.execSQL("DROP TRIGGER IF EXISTS rcs_words_delete;");
        db.execSQL("DROP TRIGGER IF EXISTS update_rcs_thread_at_me_after_insert_rcs_message;");
        db.execSQL("DROP TRIGGER IF EXISTS update_rcs_thread_at_me_after_update_rcs_message;");
        //Need to decide whether to use trigger to finish update column status
        db.execSQL("DROP TRIGGER IF EXISTS rcs_update_status_on_insert_sms");
        db.execSQL("DROP TRIGGER IF EXISTS rcs_update_status_on_update_sms_read");
        db.execSQL("DROP TRIGGER IF EXISTS rcs_update_status_on_update_sms_type");
        db.execSQL("DROP TRIGGER IF EXISTS rcs_update_status_on_insert_mms");
        db.execSQL("DROP TRIGGER IF EXISTS rcs_update_status_on_update_mms_read");
        db.execSQL("DROP TRIGGER IF EXISTS rcs_update_status_on_update_mms_msg_box");
        db.execSQL("DROP TRIGGER IF EXISTS rcs_update_status_on_insert_rcs");
        db.execSQL("DROP TRIGGER IF EXISTS rcs_update_status_on_update_rcs");

        db.execSQL("DROP TRIGGER IF EXISTS insert_thread_settings_when_insert_threads");

        // remove the columns: li_date, li_snippet, li_snippet_cs in table threads
        db.execSQL("CREATE TABLE threads_temp (" +
                   Threads._ID + " INTEGER PRIMARY KEY AUTOINCREMENT," +
                   Threads.DATE + " INTEGER DEFAULT 0," +
                   MtkThreads.DATE_SENT + " INTEGER DEFAULT 0," +
                   Threads.MESSAGE_COUNT + " INTEGER DEFAULT 0," +
                   MtkThreads.READ_COUNT + " INTEGER DEFAULT 0," +
                   Threads.RECIPIENT_IDS + " TEXT," +
                   Threads.SNIPPET + " TEXT," +
                   Threads.SNIPPET_CHARSET + " INTEGER DEFAULT 0," +
                   Threads.READ + " INTEGER DEFAULT 1," +
                   Threads.ARCHIVED + " INTEGER DEFAULT 0," +
                   Threads.TYPE + " INTEGER DEFAULT 0," +
                   Threads.ERROR + " INTEGER DEFAULT 0," +
                   Threads.HAS_ATTACHMENT + " INTEGER DEFAULT 0," +
                   MtkThreads.STATUS + " INTEGER DEFAULT 0);");
        db.execSQL("INSERT INTO threads_temp SELECT "
                + Threads._ID + ", "
                + Threads.DATE + ", "
                + MtkThreads.DATE_SENT + ", "
                + Threads.MESSAGE_COUNT + ", "
                + MtkThreads.READ_COUNT + ", "
                + Threads.RECIPIENT_IDS + ", "
                + Threads.SNIPPET + ", "
                + Threads.SNIPPET_CHARSET + ", "
                + Threads.READ + ", "
                + Threads.ARCHIVED + ", "
                + Threads.TYPE + ", "
                + Threads.ERROR + ", "
                + Threads.HAS_ATTACHMENT + ", "
                + MtkThreads.STATUS +
                " from threads;");
        db.execSQL("DROP TABLE threads;");
        db.execSQL("ALTER TABLE threads_temp RENAME TO threads;");
        // delete the related triggers
        db.execSQL("DROP TRIGGER IF EXISTS update_threads_lirelated_on_sms_locked");
        db.execSQL("DROP TRIGGER IF EXISTS update_threads_lirelated_on_sms_unlock");
        db.execSQL("DROP TRIGGER IF EXISTS update_threads_lirelated_on_sms_delete");
        db.execSQL("DROP TRIGGER IF EXISTS update_threads_lirelated_on_pdu_locked");
        db.execSQL("DROP TRIGGER IF EXISTS update_threads_lirelated_on_pdu_unlock");
        db.execSQL("DROP TRIGGER IF EXISTS update_threads_lirelated_on_pdu_delete");

        db.execSQL("CREATE TRIGGER insert_thread_settings_when_insert_threads " +
                   "AFTER INSERT ON threads " +
                   "BEGIN " +
                   "  INSERT INTO thread_settings " +
                   "    (" + MtkThreadSettings.THREAD_ID +
                   "    ," + MtkThreadSettings.SPAM +
                   "    ," + MtkThreadSettings.NOTIFICATION_ENABLE +
                   "    ," + MtkThreadSettings.MUTE +
                   "    ," + MtkThreadSettings.MUTE_START +
                   "    ," + MtkThreadSettings.RINGTONE +
                   "    ," + MtkThreadSettings.WALLPAPER +
                   "    ," + MtkThreadSettings.VIBRATE +
                   ") " +
                   "  VALUES " +
                   "    (new." + MtkThreadSettings._ID + "," +
                   "     0,1,0,0,'','',1" +
                   "); " +
                   "END;");

        db.execSQL("CREATE TABLE thread_settings_temp ("
                + MtkThreadSettings._ID + " INTEGER PRIMARY KEY,"
                + MtkThreadSettings.THREAD_ID + " INTEGER,"
                + MtkThreadSettings.SPAM + " INTEGER DEFAULT 0,"
                + MtkThreadSettings.NOTIFICATION_ENABLE + " INTEGER DEFAULT 1,"
                + MtkThreadSettings.MUTE + " INTEGER DEFAULT 0,"
                + MtkThreadSettings.MUTE_START + " INTEGER DEFAULT 0,"
                + MtkThreadSettings.RINGTONE + " TEXT,"
                + MtkThreadSettings.WALLPAPER + " TEXT,"
                + MtkThreadSettings.VIBRATE + " INTEGER DEFAULT 1"
                + ");");

        db.execSQL("INSERT INTO thread_settings_temp SELECT " + MtkThreadSettings._ID + ", " + MtkThreadSettings.THREAD_ID + ", "
                   + MtkThreadSettings.SPAM + ", " + MtkThreadSettings.NOTIFICATION_ENABLE + ", " + MtkThreadSettings.MUTE + ", "
                   + MtkThreadSettings.MUTE_START + ", " + MtkThreadSettings.RINGTONE + ", " + MtkThreadSettings.WALLPAPER + ", "
                   + MtkThreadSettings.VIBRATE + " from thread_settings;");
                   //* from thread_settings;");
        db.execSQL("DROP TABLE thread_settings;");
        db.execSQL("ALTER TABLE thread_settings_temp RENAME TO thread_settings;");

        //table
        db.execSQL("DROP TABLE IF EXISTS usersms");
        db.execSQL("DROP TABLE IF EXISTS usermms;");
        db.execSQL("DROP TABLE IF EXISTS usercb;");
        //trigger
        db.execSQL("DROP TRIGGER IF EXISTS sms_update_thread_on_add");

        //table
        db.execSQL("CREATE TABLE pdu_temp (" +
                Mms._ID + " INTEGER PRIMARY KEY AUTOINCREMENT," +
                Mms.THREAD_ID + " INTEGER," +
                Mms.DATE + " INTEGER," +
                Mms.DATE_SENT + " INTEGER DEFAULT 0," +
                Mms.MESSAGE_BOX + " INTEGER," +
                Mms.READ + " INTEGER DEFAULT 0," +
                Mms.MESSAGE_ID + " TEXT," +
                Mms.SUBJECT + " TEXT," +
                Mms.SUBJECT_CHARSET + " INTEGER," +
                Mms.CONTENT_TYPE + " TEXT," +
                Mms.CONTENT_LOCATION + " TEXT," +
                Mms.EXPIRY + " INTEGER," +
                Mms.MESSAGE_CLASS + " TEXT," +
                Mms.MESSAGE_TYPE + " INTEGER," +
                Mms.MMS_VERSION + " INTEGER," +
                Mms.MESSAGE_SIZE + " INTEGER," +
                Mms.PRIORITY + " INTEGER," +
                Mms.READ_REPORT + " INTEGER," +
                Mms.REPORT_ALLOWED + " INTEGER," +
                Mms.RESPONSE_STATUS + " INTEGER," +
                Mms.STATUS + " INTEGER," +
                Mms.TRANSACTION_ID + " TEXT," +
                Mms.RETRIEVE_STATUS + " INTEGER," +
                Mms.RETRIEVE_TEXT + " TEXT," +
                Mms.RETRIEVE_TEXT_CHARSET + " INTEGER," +
                Mms.READ_STATUS + " INTEGER," +
                Mms.CONTENT_CLASS + " INTEGER," +
                Mms.RESPONSE_TEXT + " TEXT," +
                Mms.DELIVERY_TIME + " INTEGER," +
                Mms.DELIVERY_REPORT + " INTEGER," +
                Mms.LOCKED + " INTEGER DEFAULT 0," +
                Mms.SUBSCRIPTION_ID + " INTEGER DEFAULT -1, " +
                "service_center TEXT," +
                Mms.SEEN + " INTEGER DEFAULT 0," +
                Mms.TEXT_ONLY + " INTEGER DEFAULT 0" +
                ");");
        db.execSQL("INSERT INTO pdu_temp SELECT " + Mms._ID + ", " + Mms.THREAD_ID + ", " + Mms.DATE + ", "
                   + Mms.DATE_SENT + ", " + Mms.MESSAGE_BOX + ", " + Mms.READ + ", " + Mms.MESSAGE_ID + ", "
                   + Mms.SUBJECT + ", " + Mms.SUBJECT_CHARSET + ", " + Mms.CONTENT_TYPE + ", " + Mms.CONTENT_LOCATION + ", "
                   + Mms.EXPIRY + ", " + Mms.MESSAGE_CLASS + ", " + Mms.MESSAGE_TYPE + ", " + Mms.MMS_VERSION + ", "
                   + Mms.MESSAGE_SIZE + ", " + Mms.PRIORITY + ", " + Mms.READ_REPORT + ", " + Mms.REPORT_ALLOWED + ", "
                   + Mms.RESPONSE_STATUS + ", " + Mms.STATUS + ", " + Mms.TRANSACTION_ID + ", " + Mms.RETRIEVE_STATUS + ", "
                   + Mms.RETRIEVE_TEXT + ", " + Mms.RETRIEVE_TEXT_CHARSET + ", " + Mms.READ_STATUS + ", "
                   + Mms.CONTENT_CLASS + ", " + Mms.RESPONSE_TEXT + ", " + Mms.DELIVERY_TIME + ", "
                   + Mms.DELIVERY_REPORT + ", " + Mms.LOCKED + ", " + Mms.SUBSCRIPTION_ID + ", "
                   + Mms.SEEN + ", " + Mms.TEXT_ONLY + " from pdu;");
                //* from pdu;");
        db.execSQL("DROP TABLE pdu;");
        db.execSQL("ALTER TABLE pdu_temp RENAME TO pdu;");

        db.execSQL("CREATE TABLE sms_temp (" +
                "_id INTEGER PRIMARY KEY," +
                "thread_id INTEGER," +
                "address TEXT," +
                "m_size INTEGER," +
                "person INTEGER," +
                "date INTEGER," +
                "date_sent INTEGER DEFAULT 0," +
                "protocol INTEGER," +
                "read INTEGER DEFAULT 0," +
                "status INTEGER DEFAULT -1," +
                "type INTEGER," +
                "reply_path_present INTEGER," +
                "subject TEXT," +
                "body TEXT," +
                "service_center TEXT," +
                "locked INTEGER DEFAULT 0," +
                "sub_id LONG DEFAULT -1, " +
                "error_code INTEGER DEFAULT 0," +
                "seen INTEGER DEFAULT 0," +
                "ipmsg_id INTEGER DEFAULT 0" +
                ");");
        db.execSQL("INSERT INTO sms_temp SELECT " + "_id" + ", " + "thread_id" + ", " + "address" + ", "
                   + "m_size" + ", " + "person" + ", " + "date" + ", " + "date_sent" + ", " + "protocol" + ", "
                   + "read" + ", " + "status" + ", " + "type" + ", " + "reply_path_present" + ", " + "subject" + ", "
                   + "body" + ", " + "service_center" + ", " + "locked" + ", " + "sub_id" + ", " + "error_code" + ", "
                   + "seen" + ", " + "ipmsg_id" + " from sms;");
                //* from sms;");
        db.execSQL("DROP TABLE sms;");
        db.execSQL("ALTER TABLE sms_temp RENAME TO sms;");
    }

    @Override
    public synchronized  SQLiteDatabase getReadableDatabase() {
        SQLiteDatabase db = super.getWritableDatabase();

        // getReadableDatabase gets or creates a database. So we know for sure that a database has
        // already been created at this point.
        if (mContext.isCredentialProtectedStorage()) {
            setInitialCreateDone();
        }

        return db;
    }

    @Override
    public synchronized SQLiteDatabase getWritableDatabase() {
        SQLiteDatabase db = super.getWritableDatabase();

        // getWritableDatabase gets or creates a database. So we know for sure that a database has
        // already been created at this point.
        if (mContext.isCredentialProtectedStorage()) {
            setInitialCreateDone();
        }

        if (!sTriedAutoIncrement) {
            sTriedAutoIncrement = true;
            boolean hasAutoIncrementThreads = hasAutoIncrement(db, MmsSmsProvider.TABLE_THREADS);
            boolean hasAutoIncrementAddresses = hasAutoIncrement(db, "canonical_addresses");
            boolean hasAutoIncrementPart = hasAutoIncrement(db, "part");
            boolean hasAutoIncrementPdu = hasAutoIncrement(db, "pdu");
            String logMsg = "[getWritableDatabase]" +
                    " hasAutoIncrementThreads: " + hasAutoIncrementThreads +
                    " hasAutoIncrementAddresses: " + hasAutoIncrementAddresses +
                    " hasAutoIncrementPart: " + hasAutoIncrementPart +
                    " hasAutoIncrementPdu: " + hasAutoIncrementPdu;
            Log.d(TAG, logMsg);
            localLog(logMsg);
            boolean autoIncrementThreadsSuccess = true;
            boolean autoIncrementAddressesSuccess = true;
            boolean autoIncrementPartSuccess = true;
            boolean autoIncrementPduSuccess = true;
            if (!hasAutoIncrementThreads) {
                db.beginTransaction();
                try {
                    if (false && sFakeLowStorageTest) {
                        Log.d(TAG, "[getWritableDatabase] mFakeLowStorageTest is true " +
                                " - fake exception");
                        throw new Exception("FakeLowStorageTest");
                    }
                    upgradeThreadsTableToAutoIncrement(db);     // a no-op if already upgraded
                    db.setTransactionSuccessful();
                } catch (Throwable ex) {
                    Log.e(TAG, "Failed to add autoIncrement to threads;: " + ex.getMessage(), ex);
                    autoIncrementThreadsSuccess = false;
                } finally {
                    db.endTransaction();
                }
            }
            if (!hasAutoIncrementAddresses) {
                db.beginTransaction();
                try {
                    if (false && sFakeLowStorageTest) {
                        Log.d(TAG, "[getWritableDatabase] mFakeLowStorageTest is true " +
                        " - fake exception");
                        throw new Exception("FakeLowStorageTest");
                    }
                    upgradeAddressTableToAutoIncrement(db);     // a no-op if already upgraded
                    db.setTransactionSuccessful();
                } catch (Throwable ex) {
                    Log.e(TAG, "Failed to add autoIncrement to canonical_addresses: " +
                            ex.getMessage(), ex);
                    autoIncrementAddressesSuccess = false;
                } finally {
                    db.endTransaction();
                }
            }
            if (!hasAutoIncrementPart) {
                db.beginTransaction();
                try {
                    if (false && sFakeLowStorageTest) {
                        Log.d(TAG, "[getWritableDatabase] mFakeLowStorageTest is true " +
                        " - fake exception");
                        throw new Exception("FakeLowStorageTest");
                    }
                    upgradePartTableToAutoIncrement(db);     // a no-op if already upgraded
                    db.setTransactionSuccessful();
                } catch (Throwable ex) {
                    Log.e(TAG, "Failed to add autoIncrement to part: " +
                            ex.getMessage(), ex);
                    autoIncrementPartSuccess = false;
                } finally {
                    db.endTransaction();
                }
            }
            if (!hasAutoIncrementPdu) {
                db.beginTransaction();
                try {
                    if (false && sFakeLowStorageTest) {
                        Log.d(TAG, "[getWritableDatabase] mFakeLowStorageTest is true " +
                        " - fake exception");
                        throw new Exception("FakeLowStorageTest");
                    }
                    upgradePduTableToAutoIncrement(db);     // a no-op if already upgraded
                    db.setTransactionSuccessful();
                } catch (Throwable ex) {
                    Log.e(TAG, "Failed to add autoIncrement to pdu: " +
                            ex.getMessage(), ex);
                    autoIncrementPduSuccess = false;
                } finally {
                    db.endTransaction();
                }
            }
            if (autoIncrementThreadsSuccess &&
                    autoIncrementAddressesSuccess &&
                    autoIncrementPartSuccess &&
                    autoIncrementPduSuccess) {
                if (mLowStorageMonitor != null) {
                    // We've already updated the database. This receiver is no longer necessary.
                    Log.d(TAG, "Unregistering mLowStorageMonitor - we've upgraded");
                    mContext.unregisterReceiver(mLowStorageMonitor);
                    mLowStorageMonitor = null;
                }
            } else {
                if (sFakeLowStorageTest) {
                    sFakeLowStorageTest = false;
                }

                // We failed, perhaps because of low storage. Turn on a receiver to watch for
                // storage space.
                if (mLowStorageMonitor == null) {
                    Log.d(TAG, "[getWritableDatabase] turning on storage monitor");
                    mLowStorageMonitor = new LowStorageMonitor();
                    IntentFilter intentFilter = new IntentFilter();
                    intentFilter.addAction(Intent.ACTION_DEVICE_STORAGE_LOW);
                    intentFilter.addAction(Intent.ACTION_DEVICE_STORAGE_OK);
                    mContext.registerReceiver(mLowStorageMonitor, intentFilter);
                }
            }
        }
        return db;
    }

    // Determine whether a particular table has AUTOINCREMENT in its schema.
    private boolean hasAutoIncrement(SQLiteDatabase db, String tableName) {
        boolean result = false;
        String query = "SELECT sql FROM sqlite_master WHERE type='table' AND name='" +
                        tableName + "'";
        Cursor c = db.rawQuery(query, null);
        if (c != null) {
            try {
                if (c.moveToFirst()) {
                    String schema = c.getString(0);
                    result = schema != null ? schema.contains("AUTOINCREMENT") : false;
                    Log.d(TAG, "[MmsSmsDb] tableName: " + tableName + " hasAutoIncrement: " +
                            schema + " result: " + result);
                }
            } finally {
                c.close();
            }
        }
        return result;
    }

    // upgradeThreadsTableToAutoIncrement() is called to add the AUTOINCREMENT keyword to
    // the threads table. This could fail if the user has a lot of conversations and not enough
    // storage to make a copy of the threads table. That's ok. This upgrade is optional. It'll
    // be called again next time the device is rebooted.
    private void upgradeThreadsTableToAutoIncrement(SQLiteDatabase db) {
        if (hasAutoIncrement(db, MmsSmsProvider.TABLE_THREADS)) {
            Log.d(TAG, "[MmsSmsDb] upgradeThreadsTableToAutoIncrement: already upgraded");
            return;
        }
        Log.d(TAG, "[MmsSmsDb] upgradeThreadsTableToAutoIncrement: upgrading");

        // Make the _id of the threads table autoincrement so we never re-use thread ids
        // Have to create a new temp threads table. Copy all the info from the old table.
        // Drop the old table and rename the new table to that of the old.
        db.execSQL("CREATE TABLE threads_temp (" +
                Threads._ID + " INTEGER PRIMARY KEY AUTOINCREMENT," +
                Threads.DATE + " INTEGER DEFAULT 0," +
                MtkThreads.DATE_SENT + " INTEGER DEFAULT 0," +
                Threads.MESSAGE_COUNT + " INTEGER DEFAULT 0," +
                MtkThreads.READ_COUNT + " INTEGER DEFAULT 0," +
                Threads.RECIPIENT_IDS + " TEXT," +
                Threads.SNIPPET + " TEXT," +
                Threads.SNIPPET_CHARSET + " INTEGER DEFAULT 0," +
                Threads.READ + " INTEGER DEFAULT 1," +
                Threads.TYPE + " INTEGER DEFAULT 0," +
                Threads.ERROR + " INTEGER DEFAULT 0," +
                Threads.HAS_ATTACHMENT + " INTEGER DEFAULT 0," +
                MtkThreads.STATUS + " INTEGER DEFAULT 0);");
        db.execSQL("INSERT INTO threads_temp SELECT * from threads;");
        db.execSQL("DROP TABLE threads;");
        db.execSQL("ALTER TABLE threads_temp RENAME TO threads;");
    }

    // upgradeAddressTableToAutoIncrement() is called to add the AUTOINCREMENT keyword to
    // the canonical_addresses table. This could fail if the user has a lot of people they've
    // messaged with and not enough storage to make a copy of the canonical_addresses table.
    // That's ok. This upgrade is optional. It'll be called again next time the device is rebooted.
    private void upgradeAddressTableToAutoIncrement(SQLiteDatabase db) {
        if (hasAutoIncrement(db, "canonical_addresses")) {
            Log.d(TAG, "[MmsSmsDb] upgradeAddressTableToAutoIncrement: already upgraded");
            return;
        }
        Log.d(TAG, "[MmsSmsDb] upgradeAddressTableToAutoIncrement: upgrading");

        // Make the _id of the canonical_addresses table autoincrement so we never re-use ids
        // Have to create a new temp canonical_addresses table. Copy all the info from the old
        // table. Drop the old table and rename the new table to that of the old.
        db.execSQL("CREATE TABLE canonical_addresses_temp (_id INTEGER PRIMARY KEY AUTOINCREMENT," +
                "address TEXT);");

        db.execSQL("INSERT INTO canonical_addresses_temp SELECT * from canonical_addresses;");
        db.execSQL("DROP TABLE canonical_addresses;");
        db.execSQL("ALTER TABLE canonical_addresses_temp RENAME TO canonical_addresses;");
    }

    private class LowStorageMonitor extends BroadcastReceiver {

        public LowStorageMonitor() {
        }

        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();

            Log.d(TAG, "[LowStorageMonitor] onReceive intent " + action);

            if (Intent.ACTION_DEVICE_STORAGE_OK.equals(action)) {
                sTriedAutoIncrement = false;    // try to upgrade on the next getWriteableDatabase
            }
        }
    }

    private void updateThreadsAttachmentColumn(SQLiteDatabase db) {
        // Set the values of that column correctly based on the current
        // contents of the database.
        db.execSQL("UPDATE threads SET has_attachment=1 WHERE _id IN " +
                   "  (SELECT DISTINCT pdu.thread_id FROM part " +
                   "   JOIN pdu ON pdu._id=part.mid " +
                   "   WHERE part.ct != 'text/plain' AND part.ct != 'application/smil')");
    }

    /// M: Code analyze 007, fix bug ALPS00276375, delete un-read message in folder mode
    /// make conversation mode display abnormally, then update the read field in threads
    /// after deleting a message  for OP01 @{
    public static void updateThreadReadAfterDeleteMessage(SQLiteDatabase db, long thread_id) {
        if (MTK_WAPPUSH_SUPPORT) {
            db.execSQL(
                    " UPDATE threads SET read = " +
                    "    CASE (SELECT COUNT(sms._id) FROM sms " +
                    "              WHERE sms.thread_id = " + thread_id +
                    "              AND sms.read=0) + " +
                    "          (SELECT COUNT(pdu._id) FROM pdu " +
                    "              WHERE pdu.thread_id = " + thread_id +
                    "              AND (pdu.m_type=132 OR pdu.m_type=130 OR pdu.m_type=128) " +
                    "              AND pdu.read=0) + " +
                    "           (SELECT COUNT(wappush._id) FROM wappush " +
                    "               WHERE  wappush.thread_id = " + thread_id +
                    "               AND wappush.read=0) + " +
                    "           (SELECT COUNT(cellbroadcast._id) FROM cellbroadcast " +
                    "               WHERE  cellbroadcast.thread_id = " + thread_id +
                    "               AND cellbroadcast.read=0) " +
                    "    WHEN 0 THEN 1 " +
                    "    ELSE 0 " +
                    "    END " +
                    " WHERE threads._id = " + thread_id + ";");
        } else {
            db.execSQL(
                    " UPDATE threads SET read = " +
                    "    CASE (SELECT COUNT(sms._id) FROM sms " +
                    "              WHERE sms.thread_id = " + thread_id +
                    "              AND sms.read=0) + " +
                    "          (SELECT COUNT(pdu._id) FROM pdu " +
                    "              WHERE pdu.thread_id = " + thread_id +
                    "              AND (pdu.m_type=132 OR pdu.m_type=130 OR pdu.m_type=128) " +
                    "              AND pdu.read=0) + " +
                    "           (SELECT COUNT(cellbroadcast._id) FROM cellbroadcast " +
                    "               WHERE  cellbroadcast.thread_id = " + thread_id +
                    "               AND cellbroadcast.read=0) " +
                    "    WHEN 0 THEN 1 " +
                    "    ELSE 0 " +
                    "    END " +
                    " WHERE threads._id = " + thread_id + ";");
        }
    }

    /// @}

    /// M: Code analyze 008, new feature, create table wappush. @{
    private void createWapPushTables(SQLiteDatabase db) {
        /// M: create wap push tables
        if (MTK_WAPPUSH_SUPPORT) {
            db.execSQL("CREATE TABLE wappush ("
                    + "_id INTEGER PRIMARY KEY,"
                    + "thread_id INTEGER," /// M: thread id
                    + "address TEXT NOT NULL," /// M: sender address
                    + "service_center TEXT NOT NULL," /// M: service center address
                    + "seen INTEGER DEFAULT 0," /// M: seen status 0:unseen,1:seen
                    + "read INTEGER DEFAULT 0," /// M: read status 0:unread,1:read
                    + "locked INTEGER DEFAULT 0," /// M: lock status
                                                    /// M: 0:unlocked,1:locked
                    + "error INTEGER DEFAULT 0," /// M: expire status
                                                    /// M: 0:unexpired,1:expired
                    + Mms.SUBSCRIPTION_ID + " INTEGER DEFAULT -1, "
                    + "date INTEGER," /// M: receive time
                    + "type INTEGER DEFAULT 0," /// M: 0:SI,1:SL
                    + "siid TEXT,"
                    + "url TEXT," + "action INTEGER," + "created INTEGER,"
                    + "expiration INTEGER," + "text TEXT" + ");");
        }
    }
    /// @}

    /// M: Code analyze 013, new feature, create table quicktext. @{
    private void createQuickText(SQLiteDatabase db) {
        db.execSQL("CREATE TABLE quicktext (" +
                "_id INTEGER PRIMARY KEY," +
                "text TEXT);");
    }
    /// @}
    /// M: Code analyze 005, new feature, create table cellbroadcast. @{
    private void createCBTables(SQLiteDatabase db) {
        db.execSQL("CREATE TABLE " + TABLE_CELLBROADCAST
                + "(_id INTEGER PRIMARY KEY," + Mms.SUBSCRIPTION_ID + " INTEGER DEFAULT -1,"
                + "locked INTEGER DEFAULT 0,"
                + "body TEXT," + "channel_id INTEGER," + "thread_id INTEGER,"
                + "read INTEGER DEFAULT 0," + "seen INTEGER DEFAULT 0,"
                + "date_sent INTEGER DEFAULT 0," + "date INTEGER);");
    }
    /// @}
    /// M: Add for ip message @{
    private void createThreadSettingsTable(SQLiteDatabase db) {
        db.execSQL("CREATE TABLE thread_settings("
                + MtkThreadSettings._ID + " INTEGER PRIMARY KEY,"
                + MtkThreadSettings.THREAD_ID + " INTEGER,"
                + MtkThreadSettings.SPAM + " INTEGER DEFAULT 0,"
                + MtkThreadSettings.NOTIFICATION_ENABLE + " INTEGER DEFAULT 1,"
                + MtkThreadSettings.MUTE + " INTEGER DEFAULT 0,"
                + MtkThreadSettings.MUTE_START + " INTEGER DEFAULT 0,"
                + MtkThreadSettings.RINGTONE + " TEXT,"
                + MtkThreadSettings.WALLPAPER + " TEXT,"
                + MtkThreadSettings.VIBRATE + " INTEGER DEFAULT 1"
                + ");");

        db.execSQL("INSERT INTO thread_settings (" + MtkThreadSettings._ID + ","
                + MtkThreadSettings.THREAD_ID + ") VALUES (0,0)");
    }
    /// @}

    /// M: new feature, create table mwi. @{
    private void createMwiTables(SQLiteDatabase db) {
        /// M: create mwi tables
        Log.d(TAG, "Created table mwi");
        db.execSQL("CREATE TABLE IF NOT EXISTS mwi ("
                + "_id INTEGER PRIMARY KEY,"
                + "msg_account TEXT,"
                + "to_account TEXT,"
                + "from_account TEXT,"
                + "subject TEXT,"
                + "msg_date INTEGER,"
                + "priority INTEGER,"
                + "msg_id TEXT,"
                + "msg_context INTEGER,"
                + "seen INTEGER DEFAULT 0,"
                + "read INTEGER DEFAULT 0,"
                + "got_content INTEGER DEFAULT 0"
                + ");");
    }
    /// @}

    // upgradePartTableToAutoIncrement() is called to add the AUTOINCREMENT keyword to
    // the part table. This could fail if the user has a lot of sound/video/picture attachments
    // and not enough storage to make a copy of the part table.
    // That's ok. This upgrade is optional. It'll be called again next time the device is rebooted.
    private void upgradePartTableToAutoIncrement(SQLiteDatabase db) {
        if (hasAutoIncrement(db, "part")) {
            Log.d(TAG, "[MmsSmsDb] upgradePartTableToAutoIncrement: already upgraded");
            return;
        }
        Log.d(TAG, "[MmsSmsDb] upgradePartTableToAutoIncrement: upgrading");

        // Make the _id of the part table autoincrement so we never re-use ids
        // Have to create a new temp part table. Copy all the info from the old
        // table. Drop the old table and rename the new table to that of the old.
        db.execSQL("CREATE TABLE part_temp (" +
                Part._ID + " INTEGER PRIMARY KEY AUTOINCREMENT," +
                Part.MSG_ID + " INTEGER," +
                Part.SEQ + " INTEGER DEFAULT 0," +
                Part.CONTENT_TYPE + " TEXT," +
                Part.NAME + " TEXT," +
                Part.CHARSET + " INTEGER," +
                Part.CONTENT_DISPOSITION + " TEXT," +
                Part.FILENAME + " TEXT," +
                Part.CONTENT_ID + " TEXT," +
                Part.CONTENT_LOCATION + " TEXT," +
                Part.CT_START + " INTEGER," +
                Part.CT_TYPE + " TEXT," +
                Part._DATA + " TEXT," +
                Part.TEXT + " TEXT);");

        db.execSQL("INSERT INTO part_temp SELECT * from part;");
        db.execSQL("DROP TABLE part;");
        db.execSQL("ALTER TABLE part_temp RENAME TO part;");

        // part-related triggers get tossed when the part table is dropped -- rebuild them.
        createMmsTriggers(db);
    }

    // upgradePduTableToAutoIncrement() is called to add the AUTOINCREMENT keyword to
    // the pdu table. This could fail if the user has a lot of mms messages
    // and not enough storage to make a copy of the pdu table.
    // That's ok. This upgrade is optional. It'll be called again next time the device is rebooted.
    private void upgradePduTableToAutoIncrement(SQLiteDatabase db) {
        if (hasAutoIncrement(db, "pdu")) {
            Log.d(TAG, "[MmsSmsDb] upgradePduTableToAutoIncrement: already upgraded");
            return;
        }
        Log.d(TAG, "[MmsSmsDb] upgradePduTableToAutoIncrement: upgrading");

        // Make the _id of the part table autoincrement so we never re-use ids
        // Have to create a new temp part table. Copy all the info from the old
        // table. Drop the old table and rename the new table to that of the old.
        db.execSQL("CREATE TABLE pdu_temp (" +
                Mms._ID + " INTEGER PRIMARY KEY AUTOINCREMENT," +
                Mms.THREAD_ID + " INTEGER," +
                Mms.DATE + " INTEGER," +
                Mms.DATE_SENT + " INTEGER DEFAULT 0," +
                Mms.MESSAGE_BOX + " INTEGER," +
                Mms.READ + " INTEGER DEFAULT 0," +
                Mms.MESSAGE_ID + " TEXT," +
                Mms.SUBJECT + " TEXT," +
                Mms.SUBJECT_CHARSET + " INTEGER," +
                Mms.CONTENT_TYPE + " TEXT," +
                Mms.CONTENT_LOCATION + " TEXT," +
                Mms.EXPIRY + " INTEGER," +
                Mms.MESSAGE_CLASS + " TEXT," +
                Mms.MESSAGE_TYPE + " INTEGER," +
                Mms.MMS_VERSION + " INTEGER," +
                Mms.MESSAGE_SIZE + " INTEGER," +
                Mms.PRIORITY + " INTEGER," +
                Mms.READ_REPORT + " INTEGER," +
                Mms.REPORT_ALLOWED + " INTEGER," +
                Mms.RESPONSE_STATUS + " INTEGER," +
                Mms.STATUS + " INTEGER," +
                Mms.TRANSACTION_ID + " TEXT," +
                Mms.RETRIEVE_STATUS + " INTEGER," +
                Mms.RETRIEVE_TEXT + " TEXT," +
                Mms.RETRIEVE_TEXT_CHARSET + " INTEGER," +
                Mms.READ_STATUS + " INTEGER," +
                Mms.CONTENT_CLASS + " INTEGER," +
                Mms.RESPONSE_TEXT + " TEXT," +
                Mms.DELIVERY_TIME + " INTEGER," +
                Mms.DELIVERY_REPORT + " INTEGER," +
                Mms.LOCKED + " INTEGER DEFAULT 0," +
                Mms.SUBSCRIPTION_ID + " INTEGER DEFAULT "
                        + SubscriptionManager.INVALID_SUBSCRIPTION_ID + ", " +
                "service_center TEXT," +
                Mms.SEEN + " INTEGER DEFAULT 0," +
                Mms.CREATOR + " TEXT," +
                Mms.TEXT_ONLY + " INTEGER DEFAULT 0" +
                ");");

        db.execSQL("INSERT INTO pdu_temp SELECT * from pdu;");
        db.execSQL("DROP TABLE pdu;");
        db.execSQL("ALTER TABLE pdu_temp RENAME TO pdu;");

        // pdu-related triggers get tossed when the part table is dropped -- rebuild them.
        createMmsTriggers(db);
    }

    /// M: Fix bug ALPS00780175, The 1300 threads deleting will cost more than 10 minutes.
    /// for improve multi threads deleting performance, avoid to update the thread after
    /// delete one conversation, just update all to be deleted threads after delete all
    /// conversations one by one. @{
    public static void updateMultiThreads(SQLiteDatabase db, long[] deletedThreads) {
        Log.d(TAG, "updateMultiThreads start, deletedThreads.length = " + deletedThreads.length);
        if (MTK_WAPPUSH_SUPPORT) {
            db.delete(MmsSmsProvider.TABLE_THREADS,
                  "status = 0 AND " +
                  "type <> ? AND _id NOT IN" +
                  "          (SELECT thread_id FROM sms where thread_id is not null " +
                  "           UNION SELECT thread_id FROM pdu where thread_id is not null " +
                  "           UNION SELECT DISTINCT thread_id FROM cellbroadcast " +
                  "where thread_id is not null)",
                  new String[] {String.valueOf(MtkThreads.WAPPUSH_THREAD) });
        } else {
            db.delete(MmsSmsProvider.TABLE_THREADS,
                  "status = 0 AND " +
                  "_id NOT IN" +
                  "          (SELECT thread_id FROM sms where thread_id is not null " +
                  "           UNION SELECT thread_id FROM pdu where thread_id is not null " +
                  "           UNION SELECT DISTINCT thread_id FROM cellbroadcast " +
                  "where thread_id is not null)",
                  null);
        }
        removeUnferencedCanonicalAddresses(db);
        Log.d(TAG, "delete obsolete threads and addresses end");

        String query = "SELECT _id FROM threads WHERE status<>0 OR _id IN " +
                       "(SELECT DISTINCT thread_id FROM sms " +
                       " UNION SELECT DISTINCT thread_id FROM cellbroadcast " +
                       " UNION SELECT DISTINCT thread_id FROM pdu" + ")";
        Cursor c = db.rawQuery(query, null);
        long[] threadIds = null;
        if (c != null) {
            threadIds = new long[c.getCount()];
            int i = 0;
            try {
                while (c.moveToNext()) {
                    threadIds[i++] = c.getLong(0);
                }
            } finally {
                c.close();
            }
        }

        Arrays.sort(threadIds);
        // find out the thread which wasn't deleted and need to be update.
        for (long deletedThread : deletedThreads) {
            if (Arrays.binarySearch(threadIds, deletedThread) >= 0) {
                updateThread(db, deletedThread);
            }
        }
        Log.d(TAG, "updateMultiThreads end");
    }
    /// @}
}
