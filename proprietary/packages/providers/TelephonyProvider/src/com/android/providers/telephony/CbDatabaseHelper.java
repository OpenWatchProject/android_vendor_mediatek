/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

package com.android.providers.telephony;

import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.database.sqlite.SQLiteDatabase.CursorFactory;
//import android.provider.Telephony;

import mediatek.telephony.MtkTelephony;
import mediatek.telephony.MtkTelephony.SmsCb;

class CbDatabaseHelper extends SQLiteOpenHelper {

    // Context to access resources with
    private Context mContext;
    static final String DATABASE_NAME = "cb.db";
    static final String CHANNEL_TABLE = "channel";
    static final String CBMESSAGE_TABLE = "messages";
    static final String CONVERSATION_TABLE = "threads";
    // TODO need remove same as channel table
    static final String ADDRESS_TABLE = "address"; // need remove

    private static final String UPDATE_THREAD_COUNT_ON_NEW = "  UPDATE threads SET msg_count = "
            + "     (SELECT COUNT(messages._id) FROM messages LEFT JOIN threads "
            + "      ON threads._id = "
            + MtkTelephony.SmsCb.THREAD_ID
            + "      WHERE "
            + MtkTelephony.SmsCb.THREAD_ID
            + " = new.thread_id )" + "  WHERE threads._id = new.thread_id; ";

    private static final String SMS_UPDATE_THREAD_READ_BODY = "  UPDATE threads SET read = "
            + "    CASE (SELECT COUNT(*)"
            + "          FROM messages"
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

    private static final String CB_UPDATE_THREAD_READ_BODY = "  UPDATE threads SET read = "
            + "    CASE (SELECT COUNT(*)"
            + "          FROM messages"
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

    // update date && snippet && count of threads table.
    private static final String CB_UPDATE_THREAD_DATE_SNIPPET_COUNT_ON_UPDATE = "BEGIN"
            + "  UPDATE threads SET"
            + "    date = (strftime('%s','now') * 1000), "
            + "    snippet = new."
            + MtkTelephony.SmsCb.BODY
            + " "
            + "  WHERE threads._id = new."
            + MtkTelephony.SmsCb.THREAD_ID
            + "; "
            + UPDATE_THREAD_COUNT_ON_NEW + CB_UPDATE_THREAD_READ_BODY + "END;";

    private static final String UPDATE_THREAD_COUNT_ON_OLD =
        "  UPDATE threads SET msg_count = " +
        "     (SELECT COUNT(messages._id) FROM messages LEFT JOIN threads " +
        "      ON threads._id = " + MtkTelephony.SmsCb.THREAD_ID +
        "      WHERE " + MtkTelephony.SmsCb.THREAD_ID + " = old.thread_id)" +
        "  WHERE threads._id = old.thread_id; ";

    private static final String UPDATE_THREAD_SNIPPET_ON_DELETE =
        "  UPDATE threads SET snippet = " +
        "   (SELECT body FROM" +
        "     (SELECT date, body, thread_id FROM messages)" +
        "    WHERE thread_id = OLD.thread_id ORDER BY date DESC LIMIT 1) " +
        "  WHERE threads._id = OLD.thread_id; ";

    private static final String UPDATE_THREAD_DATE_ON_DELETE =
        "  UPDATE threads SET date = " +
        "   (SELECT date FROM" +
        "     (SELECT date, body, thread_id FROM messages)" +
        "    WHERE thread_id = OLD.thread_id ORDER BY date DESC LIMIT 1) " +
        "  WHERE threads._id = OLD.thread_id; ";

    public CbDatabaseHelper(Context context) {
        super(context, DATABASE_NAME, null, 1);
        mContext = context;
    }

    public CbDatabaseHelper(Context context, String name,
            CursorFactory factory, int version) {
        super(context, name, factory, version);
        mContext = context;
    }

    private void createTables(SQLiteDatabase db) {
        db.execSQL("CREATE TABLE " + CHANNEL_TABLE
                + "(_id INTEGER PRIMARY KEY," + "name TEXT," + "number TEXT,"
                + "enable BOOLEAN," + "sub_id LONG DEFAULT -1);");

        db.execSQL("CREATE TABLE " + CBMESSAGE_TABLE
                + "(_id INTEGER PRIMARY KEY," + "sub_id LONG DEFAULT -1,"
                + "body TEXT," + "channel_id INTEGER," + "thread_id INTEGER,"
                + "read INTEGER DEFAULT 0," + "date_sent INTEGER DEFAULT 0," + "date INTEGER);");

        db.execSQL("CREATE TABLE " + CONVERSATION_TABLE
                + "(_id INTEGER PRIMARY KEY," + "date INTEGER,"
                + "msg_count INTEGER," + "address_id INTEGER,"
                + "read INTEGER DEFAULT 0," + "snippet TEXT);");
        db.execSQL("CREATE TABLE " + ADDRESS_TABLE
                + "(_id INTEGER PRIMARY KEY," + "address TEXT);");

         /**
         * This table is used by the SMSCB dispatcher to hold
         * incomplete partial messages until all the parts arrive.
         */
        db.execSQL("CREATE TABLE cbraw (" +
                   "_id INTEGER PRIMARY KEY," +
                   "msgID INTEGER," +
                   "serialNum INTEGER," + // one per full message
                   "sequence INTEGER," + // the part number of this message
                   "count INTEGER," + // the number of parts
                   "pdu TEXT," +  // the raw PDU for this part
                   "sub_id LONG DEFAULT -1);"); // sub_id

    }

    private void createCommonTriggers(SQLiteDatabase db) {
        db.execSQL("CREATE TRIGGER cb_update_thread_on_insert AFTER INSERT ON messages "
                        + CB_UPDATE_THREAD_DATE_SNIPPET_COUNT_ON_UPDATE);
        db.execSQL("CREATE TRIGGER cb_update_thread_read_on_update AFTER"
                + "  UPDATE OF " + MtkTelephony.SmsCb.READ + "  ON messages "
                + "BEGIN " + CB_UPDATE_THREAD_READ_BODY + "END;");
        // Update threads table whenever a message in messages is deleted
        db.execSQL("CREATE TRIGGER cb_update_thread_on_delete " +
                   "AFTER DELETE ON messages " +
                   "BEGIN " +
                   "  UPDATE threads SET " +
                   "     date = (strftime('%s','now') * 1000)" +
                   "  WHERE threads._id = old." + MtkTelephony.SmsCb.THREAD_ID + "; " +
                   UPDATE_THREAD_COUNT_ON_OLD +
                   UPDATE_THREAD_SNIPPET_ON_DELETE +
                   UPDATE_THREAD_DATE_ON_DELETE +
                   "END;");
    }

    @Override
    public void onCreate(SQLiteDatabase db) {
        createTables(db);
        createCommonTriggers(db);
    }

    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
        // TODO
    }

    public static void updateThread(SQLiteDatabase db, long thread_id) {
        if (thread_id < 0) {
            updateAllThreads(db, null, null);
            return;
        }

        int rows = db.delete(CONVERSATION_TABLE, "_id = ? AND _id NOT IN"
                + "          (SELECT thread_id FROM messages)",
                new String[] { String.valueOf(thread_id) });
    }

    public static void updateAllThreads(SQLiteDatabase db, String where,
            String[] whereArgs) {
        if (where == null) {
            where = "";
        } else {
            where = "WHERE (" + where + ")";
        }

        String query = "SELECT _id FROM threads WHERE _id NOT IN "
                + "(SELECT DISTINCT thread_id FROM messages " + where + ")";
        Cursor c = db.rawQuery(query, whereArgs);
        if (c != null) {
            while (c.moveToNext()) {
                updateThread(db, c.getInt(0));
            }
            c.close();
        }
    }
}
