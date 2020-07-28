/*
 * Copyright (C) 2013 The Android Open Source Project
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

package com.mediatek.dialer.database;

import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.database.DatabaseUtils;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteException;
import android.database.sqlite.SQLiteOpenHelper;
import android.database.sqlite.SQLiteStatement;
import android.net.Uri;
import android.os.AsyncTask;
import android.provider.BaseColumns;
import android.provider.ContactsContract;
import android.provider.CallLog.Calls;
import android.provider.ContactsContract.CommonDataKinds.Callable;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.ContactsContract.Contacts;
import android.provider.ContactsContract.Data;
import android.provider.ContactsContract.Directory;
import android.provider.ContactsContract.RawContacts;
import android.text.TextUtils;
import android.util.Log;

import com.android.dialer.common.LogUtil;
//import com.android.dialer.compat.CompatUtils;
import com.android.contacts.common.util.StopWatch;
//import com.android.dialer.database.FilteredNumberContract.FilteredNumberColumns;
//import com.android.dialer.database.VoicemailArchiveContract.VoicemailArchive;
import com.android.dialer.R;
import com.android.dialer.phonenumberutil.PhoneNumberHelper;
import com.android.dialer.smartdial.map.SmartDialMap;
import com.android.dialer.smartdial.util.SmartDialNameMatcher;

import com.android.dialer.util.PermissionsUtil;

import com.google.common.annotations.VisibleForTesting;
import com.google.common.base.Objects;
import com.google.common.base.Preconditions;
import com.google.common.collect.Lists;
import com.mediatek.dialer.compat.ContactsCompat.RawContactsCompat;
import com.mediatek.dialer.search.DialerSearchNameMatcher;
import com.mediatek.dialer.search.SmartDialPrefixEx;
import com.mediatek.dialer.util.DialerFeatureOptions;
import com.mediatek.dialer.util.DialerSearchUtils;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.Map.Entry;
import java.util.concurrent.atomic.AtomicBoolean;
import com.android.dialer.smartdial.map.CompositeSmartDialMap;

import android.os.Build;
import android.os.Build.VERSION;


/**
 * M: Copy from Dialer/src/com/android/dialer/database/DialerDatabaseHelper.java,
 * Only used for mediatek dialer search. So comment out tables FILTERED_NUMBER_TABLE
 * & VOICEMAIL_ARCHIVE_TABLE.
 */
/**
 * Database helper for smart dial. Designed as a singleton to make sure there is
 * only one access point to the database. Provides methods to maintain, update,
 * and query the database.
 */
public class DialerDatabaseHelperEx extends SQLiteOpenHelper {
    private static final String TAG = "DialerDatabaseHelperEx";
    private boolean mIsTestInstance = false;

    private static DialerDatabaseHelperEx sSingleton = null;

    private static final Object mLock = new Object();
    private static final AtomicBoolean sInUpdate = new AtomicBoolean(false);
    private final Context mContext;

    /**
     * SmartDial DB version ranges:
     * <pre>
     *   0-98   KitKat
     * </pre>
     */
    public static final int DATABASE_VERSION = 9;
    /// M: [MTK-Dialer-Search] Using dialer2.db to replace the original dialer.db.
    public static final String DATABASE_NAME = "dialer2.db";

    /**
     * Saves the last update time of smart dial databases to shared preferences.
     */
    private static final String DATABASE_LAST_CREATED_SHARED_PREF = "com.android.dialer";
    private static final String LAST_UPDATED_MILLIS = "last_updated_millis";
    private static final String DATABASE_VERSION_PROPERTY = "database_version";

    /// M: [MTK-Dialer-Search] Increase the max entries to 150.
    public static final int MAX_ENTRIES = 150;

    public interface Tables {
        /** Saves a list of numbers to be blocked.*/
        static final String FILTERED_NUMBER_TABLE = "filtered_numbers_table";
        /** Saves the necessary smart dial information of all contacts. */
        static final String SMARTDIAL_TABLE = "smartdial_table";
        /** Saves all possible prefixes to refer to a contacts.*/
        static final String PREFIX_TABLE = "prefix_table";
        /** Saves all archived voicemail information. */
        static final String VOICEMAIL_ARCHIVE_TABLE = "voicemail_archive_table";
        /** Database properties for internal use */
        static final String PROPERTIES = "properties";

        /// M: [MTK-Dialer-Search] Save the necessary information of callLog
        /// which does not belong to any contact @{
        static final String SMARTDIAL_CALLLOG_TABLE = "smartdial_calllog_table";
        /// @}
    }

    public static final String ACTION_SMART_DIAL_UPDATED =
        "com.android.dialer.database.ACTION_SMART_DIAL_UPDATED";

    public interface SmartDialDbColumns {
        static final String _ID = "id";
        static final String DATA_ID = "data_id";
        static final String NUMBER = "phone_number";
        static final String CONTACT_ID = "contact_id";
        static final String LOOKUP_KEY = "lookup_key";
        static final String DISPLAY_NAME_PRIMARY = "display_name";
        static final String PHOTO_ID = "photo_id";
        static final String LAST_TIME_USED = "last_time_used";
        static final String TIMES_USED = "times_used";
        static final String STARRED = "starred";
        static final String IS_SUPER_PRIMARY = "is_super_primary";
        static final String IN_VISIBLE_GROUP = "in_visible_group";
        static final String IS_PRIMARY = "is_primary";
        static final String CARRIER_PRESENCE = "carrier_presence";
        static final String LAST_SMARTDIAL_UPDATE_TIME = "last_smartdial_update_time";

        /// M: [MTK-Dialer-Search] Add fields for enhancing dialer search @{
        static final String TYPE = "type";
        static final String LABEL = "label";
        static final String INDICATE_PHONE_SIM = "indicate_phone_sim";
        static final String IS_SDN = "is_sdn";
        static final String NORMALIZED_NAME = "normalized_name";
        static final String SEARCH_DATA_OFFSETS = "search_data_offsets";
        static final String SORT_KEY = "sort_key";
        static final String INITIALS = "initials";
        /// @}
    }

    public static interface PrefixColumns extends BaseColumns {
        static final String PREFIX = "prefix";
        static final String CONTACT_ID = "contact_id";
    }

    public interface PropertiesColumns {
        String PROPERTY_KEY = "property_key";
        String PROPERTY_VALUE = "property_value";
    }

    /** Query options for querying the contact database.*/
    public static interface PhoneQuery {
        /// M: Google Code
        //static final Uri URI = Phone.CONTENT_URI.buildUpon().
        ///M: Using callable to support SIP/IMS regular search
        static final Uri URI = Callable.CONTENT_URI.buildUpon().
               appendQueryParameter(ContactsContract.DIRECTORY_PARAM_KEY,
                       String.valueOf(Directory.DEFAULT)).
               appendQueryParameter(ContactsContract.REMOVE_DUPLICATE_ENTRIES, "true").
               build();

       static final String[] PROJECTION = new String[] {
            Phone._ID,                          // 0
            Phone.TYPE,                         // 1
            Phone.LABEL,                        // 2
            Phone.NUMBER,                       // 3
            Phone.CONTACT_ID,                   // 4
            Phone.LOOKUP_KEY,                   // 5
            Phone.DISPLAY_NAME_PRIMARY,         // 6
            Phone.PHOTO_ID,                     // 7
            Data.LAST_TIME_USED,                // 8
            Data.TIMES_USED,                    // 9
            Contacts.STARRED,                   // 10
            Data.IS_SUPER_PRIMARY,              // 11
            Contacts.IN_VISIBLE_GROUP,          // 12
            Data.IS_PRIMARY,                    // 13
            Data.CARRIER_PRESENCE               // 14
        };

        static final int PHONE_ID = 0;
        static final int PHONE_TYPE = 1;
        static final int PHONE_LABEL = 2;
        static final int PHONE_NUMBER = 3;
        static final int PHONE_CONTACT_ID = 4;
        static final int PHONE_LOOKUP_KEY = 5;
        static final int PHONE_DISPLAY_NAME = 6;
        static final int PHONE_PHOTO_ID = 7;
        static final int PHONE_LAST_TIME_USED = 8;
        static final int PHONE_TIMES_USED = 9;
        static final int PHONE_STARRED = 10;
        static final int PHONE_IS_SUPER_PRIMARY = 11;
        static final int PHONE_IN_VISIBLE_GROUP = 12;
        static final int PHONE_IS_PRIMARY = 13;
        static final int PHONE_CARRIER_PRESENCE = 14;
        /// M: [MTK-Dialer-Search] @{
        static final int PHONE_INDICATE_PHONE_SIM = 15;
        static final int PHONE_IS_SDN_CONTACT = 16;
        /// @}

        /** Selects only rows that have been updated after a certain time stamp.*/
        static final String SELECT_UPDATED_CLAUSE =
                Phone.CONTACT_LAST_UPDATED_TIMESTAMP + " > ?";

        /** Ignores contacts that have an unreasonably long lookup key. These are likely to be
         * the result of multiple (> 50) merged raw contacts, and are likely to cause
         * OutOfMemoryExceptions within SQLite, or cause memory allocation problems later on
         * when iterating through the cursor set (see b/13133579)
         */
        static final String SELECT_IGNORE_LOOKUP_KEY_TOO_LONG_CLAUSE =
                "length(" + Phone.LOOKUP_KEY + ") < 1000";

        static final String SELECTION = SELECT_UPDATED_CLAUSE + " AND " +
                SELECT_IGNORE_LOOKUP_KEY_TOO_LONG_CLAUSE;

        /// M: [MTK-Dialer-Search] Add for update a time interval data. @{
        static final String SELECT_UPDATED_CLAUSE_RANGE =
                Phone.CONTACT_LAST_UPDATED_TIMESTAMP + " > ? AND " +
                Phone.CONTACT_LAST_UPDATED_TIMESTAMP + " < ?";
        static final String SELECTION_RANGE = SELECT_UPDATED_CLAUSE_RANGE + " AND " +
                SELECT_IGNORE_LOOKUP_KEY_TOO_LONG_CLAUSE;
        /// @}
    }

    /**
     * Query for all contacts that have been updated since the last time the smart dial database
     * was updated.
     */
    public static interface UpdatedContactQuery {
        static final Uri URI = ContactsContract.Contacts.CONTENT_URI;

        static final String[] PROJECTION = new String[] {
                ContactsContract.Contacts._ID  // 0
        };

        static final int UPDATED_CONTACT_ID = 0;

        static final String SELECT_UPDATED_CLAUSE =
                ContactsContract.Contacts.CONTACT_LAST_UPDATED_TIMESTAMP + " > ?";
    }

    /** Query options for querying the deleted contact database.*/
    public static interface DeleteContactQuery {
       static final Uri URI = ContactsContract.DeletedContacts.CONTENT_URI;

       static final String[] PROJECTION = new String[] {
            ContactsContract.DeletedContacts.CONTACT_ID,                          // 0
            ContactsContract.DeletedContacts.CONTACT_DELETED_TIMESTAMP,           // 1
        };

        static final int DELETED_CONTACT_ID = 0;
        static final int DELECTED_TIMESTAMP = 1;

        /** Selects only rows that have been deleted after a certain time stamp.*/
        public static final String SELECT_UPDATED_CLAUSE =
                ContactsContract.DeletedContacts.CONTACT_DELETED_TIMESTAMP + " > ?";
    }

    /**
     * Gets the sorting order for the smartdial table. This computes a SQL "ORDER BY" argument by
     * composing contact status and recent contact details together.
     */
    private static interface SmartDialSortingOrder {
        /** Current contacts - those contacted within the last 3 days (in milliseconds) */
        static final long LAST_TIME_USED_CURRENT_MS = 3L * 24 * 60 * 60 * 1000;
        /** Recent contacts - those contacted within the last 30 days (in milliseconds) */
        static final long LAST_TIME_USED_RECENT_MS = 30L * 24 * 60 * 60 * 1000;

        /** Time since last contact. */
        static final String TIME_SINCE_LAST_USED_MS = "( ?1 - " +
                Tables.SMARTDIAL_TABLE + "." + SmartDialDbColumns.LAST_TIME_USED + ")";

        /** Contacts that have been used in the past 3 days rank higher than contacts that have
         * been used in the past 30 days, which rank higher than contacts that have not been used
         * in recent 30 days.
         */
        static final String SORT_BY_DATA_USAGE =
                "(CASE WHEN " + TIME_SINCE_LAST_USED_MS + " < " + LAST_TIME_USED_CURRENT_MS +
                " THEN 0 " +
                " WHEN " + TIME_SINCE_LAST_USED_MS + " < " + LAST_TIME_USED_RECENT_MS +
                " THEN 1 " +
                " ELSE 2 END)";

        /** This sort order is similar to that used by the ContactsProvider when returning a list
         * of frequently called contacts.
         */
        static final String SORT_ORDER =
                Tables.SMARTDIAL_TABLE + "." + SmartDialDbColumns.STARRED + " DESC, "
                /// M: [MTK-Dialer-Search] order by alphabet @{
                + Tables.SMARTDIAL_TABLE + "." +SmartDialDbColumns.SORT_KEY + ", "
                /// @}
                + Tables.SMARTDIAL_TABLE + "." + SmartDialDbColumns.IS_SUPER_PRIMARY + " DESC, "
                + SORT_BY_DATA_USAGE + ", "
                + Tables.SMARTDIAL_TABLE + "." + SmartDialDbColumns.TIMES_USED + " DESC, "
                + Tables.SMARTDIAL_TABLE + "." + SmartDialDbColumns.IN_VISIBLE_GROUP + " DESC, "
                + Tables.SMARTDIAL_TABLE + "." + SmartDialDbColumns.DISPLAY_NAME_PRIMARY + ", "
                + Tables.SMARTDIAL_TABLE + "." + SmartDialDbColumns.CONTACT_ID + ", "
                + Tables.SMARTDIAL_TABLE + "." + SmartDialDbColumns.IS_PRIMARY + " DESC"
                /// M: [MTK-Dialer-Search] order by data id @{
                + ", "
                + Tables.SMARTDIAL_TABLE + "." + SmartDialDbColumns.DATA_ID + " ASC";
                /// @}
    }

    /**
     * Simple data format for a contact, containing only information needed for showing up in
     * smart dial interface.
     */
    public static class ContactNumber {
        public final long id;
        public final long dataId;
        public final String displayName;
        public final String phoneNumber;
        public final String lookupKey;
        public final long photoId;
        public final int carrierPresence;

        /// M: [MTK-Dialer-Search] @{
        public final int type;
        public final String label;
        public final int indicatePhoneSim;
        public final int isSdnContact;
        public final String nameOffset;
        public final String dataOffset;
        /// @}

        public ContactNumber(long id, long dataID, String displayName, String phoneNumber,
                String lookupKey, long photoId, int carrierPresence,
                int type, String label, int indicatePhoneSim, int isSdnContact,
                String nameOffset, String dataOffset) {
            this.dataId = dataID;
            this.id = id;
            this.displayName = displayName;
            this.phoneNumber = phoneNumber;
            this.lookupKey = lookupKey;
            this.photoId = photoId;
            this.carrierPresence = carrierPresence;

            /// M: [MTK-Dialer-Search] @{
            this.type = type;
            this.label = label;
            this.indicatePhoneSim = indicatePhoneSim;
            this.isSdnContact = isSdnContact;
            this.nameOffset = nameOffset;
            this.dataOffset = dataOffset;
            /// @}
        }

        @Override
        public int hashCode() {
            return Objects.hashCode(id, dataId, displayName, phoneNumber, lookupKey, photoId,
                    carrierPresence, type, label, indicatePhoneSim, isSdnContact, nameOffset,
                    dataOffset);
        }

        @Override
        public boolean equals(Object object) {
            if (this == object) {
                return true;
            }
            if (object instanceof ContactNumber) {
                final ContactNumber that = (ContactNumber) object;
                return Objects.equal(this.id, that.id)
                        && Objects.equal(this.dataId, that.dataId)
                        && Objects.equal(this.displayName, that.displayName)
                        && Objects.equal(this.phoneNumber, that.phoneNumber)
                        && Objects.equal(this.lookupKey, that.lookupKey)
                        && Objects.equal(this.photoId, that.photoId)
                        && Objects.equal(this.carrierPresence, that.carrierPresence)

                        /// M: [MTK-Dialer-Search] @{
                        && Objects.equal(this.type, that.type)
                        && Objects.equal(this.label, that.label)
                        && Objects.equal(this.indicatePhoneSim, that.indicatePhoneSim)
                        && Objects.equal(this.isSdnContact, that.isSdnContact)
                        && Objects.equal(this.nameOffset, that.nameOffset)
                        && Objects.equal(this.dataOffset, that.dataOffset);
                        /// @}
            }
            return false;
        }
    }

    /**
     * Data format for finding duplicated contacts.
     */
    private class ContactMatch {
        private final String lookupKey;
        private final long id;
        /// M: [MTK-Dialer-Search] @{
        private final String number;
        /// @}

        public ContactMatch(String lookupKey, long id, String number) {
            this.lookupKey = lookupKey;
            this.id = id;
            this.number = number;
        }

        @Override
        public int hashCode() {
            return Objects.hashCode(lookupKey, id, number);
        }

        @Override
        public boolean equals(Object object) {
            if (this == object) {
                return true;
            }
            if (object instanceof ContactMatch) {
                final ContactMatch that = (ContactMatch) object;
                return Objects.equal(this.lookupKey, that.lookupKey)
                        && Objects.equal(this.id, that.id)
                        /// M: [MTK-Dialer-Search] @{
                        && Objects.equal(this.number, that.number);
                        /// @}
            }
            return false;
        }
    }

    /**
     * Access function to get the singleton instance of DialerDatabaseHelper.
     */
    public static synchronized DialerDatabaseHelperEx getInstance(Context context) {
        LogUtil.i(TAG, "Getting Instance");
        if (sSingleton == null) {
            // Use application context instead of activity context because this is a singleton,
            // and we don't want to leak the activity if the activity is not running but the
            // dialer database helper is still doing work.
            sSingleton = new DialerDatabaseHelperEx(context.getApplicationContext(),
                    DATABASE_NAME);
        }
        return sSingleton;
    }

    /**
     * Returns a new instance for unit tests. The database will be created in memory.
     */
    @VisibleForTesting
    static DialerDatabaseHelperEx getNewInstanceForTest(Context context) {
        return new DialerDatabaseHelperEx(context, null, true);
    }

    protected DialerDatabaseHelperEx(Context context, String databaseName, boolean isTestInstance) {
        this(context, databaseName, DATABASE_VERSION);
        mIsTestInstance = isTestInstance;
    }

    protected DialerDatabaseHelperEx(Context context, String databaseName) {
        this(context, databaseName, DATABASE_VERSION);
    }

    public DialerDatabaseHelperEx(Context context, String databaseName, int dbVersion) {
        super(context, databaseName, null, dbVersion);
        mContext = Preconditions.checkNotNull(context, "Context must not be null");
    }

    /**
     * Creates tables in the database when database is created for the first time.
     *
     * @param db The database.
     */
    @Override
    public void onCreate(SQLiteDatabase db) {
        setupTables(db);
    }

    private void setupTables(SQLiteDatabase db) {
        dropTables(db);
        db.execSQL("CREATE TABLE " + Tables.SMARTDIAL_TABLE + " ("
                + SmartDialDbColumns._ID + " INTEGER PRIMARY KEY AUTOINCREMENT,"
                + SmartDialDbColumns.DATA_ID + " INTEGER, "
                + SmartDialDbColumns.NUMBER + " TEXT,"
                + SmartDialDbColumns.CONTACT_ID + " INTEGER,"
                + SmartDialDbColumns.LOOKUP_KEY + " TEXT,"
                + SmartDialDbColumns.DISPLAY_NAME_PRIMARY + " TEXT, "
                + SmartDialDbColumns.PHOTO_ID + " INTEGER, "
                + SmartDialDbColumns.LAST_SMARTDIAL_UPDATE_TIME + " LONG, "
                + SmartDialDbColumns.LAST_TIME_USED + " LONG, "
                + SmartDialDbColumns.TIMES_USED + " INTEGER, "
                + SmartDialDbColumns.STARRED + " INTEGER, "
                + SmartDialDbColumns.IS_SUPER_PRIMARY + " INTEGER, "
                + SmartDialDbColumns.IN_VISIBLE_GROUP + " INTEGER, "
                + SmartDialDbColumns.IS_PRIMARY + " INTEGER, "
                + SmartDialDbColumns.CARRIER_PRESENCE + " INTEGER NOT NULL DEFAULT 0"
                /// M: [MTK-Dialer-Search] Add fields for enhancing dialer search @{
                + ", "
                + SmartDialDbColumns.TYPE + " INTEGER, "
                + SmartDialDbColumns.LABEL + " TEXT, "
                + SmartDialDbColumns.INDICATE_PHONE_SIM + " INTEGER, "
                + SmartDialDbColumns.IS_SDN + " INTEGER, "
                + SmartDialDbColumns.NORMALIZED_NAME + " TEXT, "
                + SmartDialDbColumns.SEARCH_DATA_OFFSETS + " TEXT, "
                + SmartDialDbColumns.SORT_KEY + " TEXT, "
                + SmartDialDbColumns.INITIALS + " TEXT"
                /// @}
                + ");");

        db.execSQL("CREATE TABLE " + Tables.PREFIX_TABLE + " ("
                + PrefixColumns._ID + " INTEGER PRIMARY KEY AUTOINCREMENT,"
                + PrefixColumns.PREFIX + " TEXT COLLATE NOCASE, "
                + PrefixColumns.CONTACT_ID + " INTEGER"
                + ");");

        db.execSQL("CREATE TABLE " + Tables.PROPERTIES + " ("
                + PropertiesColumns.PROPERTY_KEY + " TEXT PRIMARY KEY, "
                + PropertiesColumns.PROPERTY_VALUE + " TEXT "
                + ");");

        // This will need to also be updated in setupTablesForFilteredNumberTest and onUpgrade.
        // Hardcoded so we know on glance what columns are updated in setupTables,
        // and to be able to guarantee the state of the DB at each upgrade step.
        /// M: Google Code, remove unused table
/*
        db.execSQL("CREATE TABLE " + Tables.FILTERED_NUMBER_TABLE + " ("
                + FilteredNumberColumns._ID + " INTEGER PRIMARY KEY AUTOINCREMENT,"
                + FilteredNumberColumns.NORMALIZED_NUMBER + " TEXT UNIQUE,"
                + FilteredNumberColumns.NUMBER + " TEXT,"
                + FilteredNumberColumns.COUNTRY_ISO + " TEXT,"
                + FilteredNumberColumns.TIMES_FILTERED + " INTEGER,"
                + FilteredNumberColumns.LAST_TIME_FILTERED + " LONG,"
                + FilteredNumberColumns.CREATION_TIME + " LONG,"
                + FilteredNumberColumns.TYPE + " INTEGER,"
                + FilteredNumberColumns.SOURCE + " INTEGER"
                + ");");
*/

        /// M: [MTK-Dialer-Search]
        /// Create smartdial_calllog_table to save necessary callLog information @{
        db.execSQL("CREATE TABLE " + Tables.SMARTDIAL_CALLLOG_TABLE + " (" +
                SmartDialCallLogColumns._ID + " INTEGER PRIMARY KEY AUTOINCREMENT," +
                SmartDialCallLogColumns.CALLLOG_ID + " INTEGER," +
                SmartDialCallLogColumns.NUMBER + " TEXT," +
                SmartDialCallLogColumns.NUMBER_PRESENTATION + " INTEGER," +
                SmartDialCallLogColumns.DATE + " INTEGER," +
                SmartDialCallLogColumns.TYPE + " INTEGER," +
                SmartDialCallLogColumns.GEOCODED_LOCATION + " TEXT," +
                SmartDialCallLogColumns.PHONE_ACCOUNT_COMPONENT_NAME + " TEXT," +
                SmartDialCallLogColumns.PHONE_ACCOUNT_ID + " TEXT" +
        ");");
        /// @}

        /// M: Google Code, remove unused table
        //createVoicemailArchiveTable(db);
        setProperty(db, DATABASE_VERSION_PROPERTY, String.valueOf(DATABASE_VERSION));
        if (!mIsTestInstance) {
            resetSmartDialLastUpdatedTime();
        }
    }

    public void dropTables(SQLiteDatabase db) {
        db.execSQL("DROP TABLE IF EXISTS " + Tables.PREFIX_TABLE);
        db.execSQL("DROP TABLE IF EXISTS " + Tables.SMARTDIAL_TABLE);
        db.execSQL("DROP TABLE IF EXISTS " + Tables.PROPERTIES);
        /// M: Google Code, remove unused table
        //db.execSQL("DROP TABLE IF EXISTS " + Tables.FILTERED_NUMBER_TABLE);
        //db.execSQL("DROP TABLE IF EXISTS " + Tables.VOICEMAIL_ARCHIVE_TABLE);

        /// M: [MTK-Dialer-Search] @{
        db.execSQL("DROP TABLE IF EXISTS " + Tables.SMARTDIAL_CALLLOG_TABLE);
        /// @}
    }

    @Override
    public void onUpgrade(SQLiteDatabase db, int oldNumber, int newNumber) {
        // Disregard the old version and new versions provided by SQLiteOpenHelper, we will read
        // our own from the database.

        int oldVersion;

        oldVersion = getPropertyAsInt(db, DATABASE_VERSION_PROPERTY, 0);

        if (oldVersion == 0) {
            Log.e(TAG, "Malformed database version..recreating database");
        }

        if (oldVersion < 4) {
            setupTables(db);
            return;
        }
        /// M: Google Code, remove unused table
/*
        if (oldVersion < 7) {
            db.execSQL("DROP TABLE IF EXISTS " + Tables.FILTERED_NUMBER_TABLE);
            db.execSQL("CREATE TABLE " + Tables.FILTERED_NUMBER_TABLE + " ("
                    + FilteredNumberColumns._ID + " INTEGER PRIMARY KEY AUTOINCREMENT,"
                    + FilteredNumberColumns.NORMALIZED_NUMBER + " TEXT UNIQUE,"
                    + FilteredNumberColumns.NUMBER + " TEXT,"
                    + FilteredNumberColumns.COUNTRY_ISO + " TEXT,"
                    + FilteredNumberColumns.TIMES_FILTERED + " INTEGER,"
                    + FilteredNumberColumns.LAST_TIME_FILTERED + " LONG,"
                    + FilteredNumberColumns.CREATION_TIME + " LONG,"
                    + FilteredNumberColumns.TYPE + " INTEGER,"
                    + FilteredNumberColumns.SOURCE + " INTEGER"
                    + ");");
            oldVersion = 7;
        }

        if (oldVersion < 8) {
            upgradeToVersion8(db);
            oldVersion = 8;
        }

        if (oldVersion < 9) {
            db.execSQL("DROP TABLE IF EXISTS " + Tables.VOICEMAIL_ARCHIVE_TABLE);
            createVoicemailArchiveTable(db);
            oldVersion = 9;
        }
*/
        if (oldVersion != DATABASE_VERSION) {
            throw new IllegalStateException(
                    "error upgrading the database to version " + DATABASE_VERSION);
        }

        setProperty(db, DATABASE_VERSION_PROPERTY, String.valueOf(DATABASE_VERSION));
    }

    public void upgradeToVersion8(SQLiteDatabase db) {
        db.execSQL("ALTER TABLE smartdial_table ADD carrier_presence INTEGER NOT NULL DEFAULT 0");
    }

    /**
     * Stores a key-value pair in the {@link Tables#PROPERTIES} table.
     */
    public void setProperty(String key, String value) {
        setProperty(getWritableDatabase(), key, value);
    }

    public void setProperty(SQLiteDatabase db, String key, String value) {
        final ContentValues values = new ContentValues();
        values.put(PropertiesColumns.PROPERTY_KEY, key);
        values.put(PropertiesColumns.PROPERTY_VALUE, value);
        db.replace(Tables.PROPERTIES, null, values);
    }

    /**
     * Returns the value from the {@link Tables#PROPERTIES} table.
     */
    public String getProperty(String key, String defaultValue) {
        return getProperty(getReadableDatabase(), key, defaultValue);
    }

    public String getProperty(SQLiteDatabase db, String key, String defaultValue) {
        try {
            String value = null;
            final Cursor cursor = db.query(Tables.PROPERTIES,
                    new String[] {PropertiesColumns.PROPERTY_VALUE},
                            PropertiesColumns.PROPERTY_KEY + "=?",
                    new String[] {key}, null, null, null);
            if (cursor != null) {
                try {
                    if (cursor.moveToFirst()) {
                        value = cursor.getString(0);
                    }
                } finally {
                    cursor.close();
                }
            }
            return value != null ? value : defaultValue;
        } catch (SQLiteException e) {
            return defaultValue;
        }
    }

    public int getPropertyAsInt(SQLiteDatabase db, String key, int defaultValue) {
        final String stored = getProperty(db, key, "");
        try {
            return Integer.parseInt(stored);
        } catch (NumberFormatException e) {
            return defaultValue;
        }
    }

    private void resetSmartDialLastUpdatedTime() {
        final SharedPreferences databaseLastUpdateSharedPref = mContext.getSharedPreferences(
                DATABASE_LAST_CREATED_SHARED_PREF, Context.MODE_PRIVATE);
        final SharedPreferences.Editor editor = databaseLastUpdateSharedPref.edit();
        editor.putLong(LAST_UPDATED_MILLIS, 0);
        editor.commit();
    }

    /**
     * Starts the database upgrade process in the background.
     */
    public void startSmartDialUpdateThread() {
        if (PermissionsUtil.hasContactsReadPermissions(mContext)
                || PermissionsUtil.hasPhonePermissions(mContext)) {
            new SmartDialUpdateAsyncTask().execute();
        } else {
            ///M: print log if permission was not enabled!
            Log.i(TAG, "SmartDial has no contacts and phone permission!");
        }
    }

    private class SmartDialUpdateAsyncTask extends AsyncTask {
        @Override
        protected Object doInBackground(Object[] objects) {
             LogUtil.i(TAG, "Updating database");
            /// M: [MTK-Dialer-Search] @{
            /// Original Google Code
            /// updateSmartDialDatabase();
            long start = System.currentTimeMillis();
            synchronized(mLock) {
                sInUpdate.getAndSet(true);
                try {
                    updateSmartDialDatabase();
                    updateSmartDialCallLog();
                } catch (Exception e) {
                    // catch exception to avoid block launch.
                    Log.e(TAG, "SmartDialUpdateAsyncTask, update failed.", e);
                }
                notifyChange();
                sInUpdate.getAndSet(false);
            }
            long end = System.currentTimeMillis();
            LogUtil.i(TAG, "Updating database duration=" + (end - start));
            /// @}

            return null;
        }

        @Override
        protected void onCancelled() {
            LogUtil.d(TAG, "Updating Cancelled");
            super.onCancelled();
        }

        @Override
        protected void onPostExecute(Object o) {
            LogUtil.d(TAG, "Updating Finished");
            super.onPostExecute(o);
        }
    }
    /**
     * Removes rows in the smartdial database that matches the contacts that have been deleted
     * by other apps since last update.
     *
     * @param db Database to operate on.
     * @param deletedContactCursor Cursor containing rows of deleted contacts
     */
    @VisibleForTesting
    void removeDeletedContacts(SQLiteDatabase db, Cursor deletedContactCursor) {
        if (deletedContactCursor == null) {
            return;
        }

        db.beginTransaction();
        try {
            while (deletedContactCursor.moveToNext()) {
                final Long deleteContactId =
                        deletedContactCursor.getLong(DeleteContactQuery.DELETED_CONTACT_ID);
                db.delete(Tables.SMARTDIAL_TABLE,
                        SmartDialDbColumns.CONTACT_ID + "=" + deleteContactId, null);
                db.delete(Tables.PREFIX_TABLE,
                        PrefixColumns.CONTACT_ID + "=" + deleteContactId, null);
            }

            db.setTransactionSuccessful();
        } finally {
            deletedContactCursor.close();
            db.endTransaction();
        }
    }

    private Cursor getDeletedContactCursor(String lastUpdateMillis) {
        return mContext.getContentResolver().query(
                DeleteContactQuery.URI,
                DeleteContactQuery.PROJECTION,
                DeleteContactQuery.SELECT_UPDATED_CLAUSE,
                new String[] {lastUpdateMillis},
                null);
    }

    /**
     * Removes potentially corrupted entries in the database. These contacts may be added before
     * the previous instance of the dialer was destroyed for some reason. For data integrity, we
     * delete all of them.

     * @param db Database pointer to the dialer database.
     * @param last_update_time Time stamp of last successful update of the dialer database.
     */
    private void removePotentiallyCorruptedContacts(SQLiteDatabase db, String last_update_time) {
        db.delete(Tables.PREFIX_TABLE,
                PrefixColumns.CONTACT_ID + " IN " +
                "(SELECT " + SmartDialDbColumns.CONTACT_ID + " FROM " + Tables.SMARTDIAL_TABLE +
                " WHERE " + SmartDialDbColumns.LAST_SMARTDIAL_UPDATE_TIME + " > " +
                last_update_time + ")",
                null);
        db.delete(Tables.SMARTDIAL_TABLE,
                SmartDialDbColumns.LAST_SMARTDIAL_UPDATE_TIME + " > " + last_update_time, null);
    }

    /**
     * All columns excluding MIME_TYPE, _DATA, ARCHIVED, SERVER_ID, are the same as
     *  the columns in the {@link android.provider.CallLog.Calls} table.
     *
     *  @param db Database pointer to the dialer database.
     */
    /// M: Google Code, remove unused table
    /*private void createVoicemailArchiveTable(SQLiteDatabase db) {
        db.execSQL("CREATE TABLE " + Tables.VOICEMAIL_ARCHIVE_TABLE + " ("
                + VoicemailArchive._ID + " INTEGER PRIMARY KEY AUTOINCREMENT,"
                + VoicemailArchive.NUMBER + " TEXT,"
                + VoicemailArchive.DATE + " LONG,"
                + VoicemailArchive.DURATION + " LONG,"
                + VoicemailArchive.MIME_TYPE + " TEXT,"
                + VoicemailArchive.COUNTRY_ISO + " TEXT,"
                + VoicemailArchive._DATA + " TEXT,"
                + VoicemailArchive.GEOCODED_LOCATION + " TEXT,"
                + VoicemailArchive.CACHED_NAME + " TEXT,"
                + VoicemailArchive.CACHED_NUMBER_TYPE + " INTEGER,"
                + VoicemailArchive.CACHED_NUMBER_LABEL + " TEXT,"
                + VoicemailArchive.CACHED_LOOKUP_URI + " TEXT,"
                + VoicemailArchive.CACHED_MATCHED_NUMBER + " TEXT,"
                + VoicemailArchive.CACHED_NORMALIZED_NUMBER + " TEXT,"
                + VoicemailArchive.CACHED_PHOTO_ID + " LONG,"
                + VoicemailArchive.CACHED_FORMATTED_NUMBER + " TEXT,"
                + VoicemailArchive.ARCHIVED + " INTEGER,"
                + VoicemailArchive.NUMBER_PRESENTATION + " INTEGER,"
                + VoicemailArchive.ACCOUNT_COMPONENT_NAME + " TEXT,"
                + VoicemailArchive.ACCOUNT_ID + " TEXT,"
                + VoicemailArchive.FEATURES + " INTEGER,"
                + VoicemailArchive.SERVER_ID + " INTEGER,"
                + VoicemailArchive.TRANSCRIPTION + " TEXT,"
                + VoicemailArchive.CACHED_PHOTO_URI + " TEXT"
                + ");");
    } */

    /**
     * Removes all entries in the smartdial contact database.
     */
    @VisibleForTesting
    void removeAllContacts(SQLiteDatabase db) {
        db.delete(Tables.SMARTDIAL_TABLE, null, null);
        db.delete(Tables.PREFIX_TABLE, null, null);
    }

    /**
     * Counts number of rows of the prefix table.
     */
    @VisibleForTesting
    int countPrefixTableRows(SQLiteDatabase db) {
        return (int)DatabaseUtils.longForQuery(db, "SELECT COUNT(1) FROM " + Tables.PREFIX_TABLE,
                null);
    }

    /**
     * Removes rows in the smartdial database that matches updated contacts.
     *
     * @param db Database pointer to the smartdial database
     * @param updatedContactCursor Cursor pointing to the list of recently updated contacts.
     */
    @VisibleForTesting
    void removeUpdatedContacts(SQLiteDatabase db, Cursor updatedContactCursor) {
        if (updatedContactCursor == null || updatedContactCursor.getCount() <= 0) {
            return;
        }
        db.beginTransaction();
        try {
            updatedContactCursor.moveToPosition(-1);
            StringBuilder contactIds = new StringBuilder();
            while (updatedContactCursor.moveToNext() ) {
                contactIds.append(updatedContactCursor.getLong(
                        PhoneQuery.PHONE_CONTACT_ID)).append(',');
            }
            // remove the last ","
            contactIds.deleteCharAt(contactIds.lastIndexOf(","));

            db.delete(Tables.SMARTDIAL_TABLE, SmartDialDbColumns.CONTACT_ID + " IN ("
                    + contactIds.toString() + ")", null);
            db.delete(Tables.PREFIX_TABLE, PrefixColumns.CONTACT_ID + " IN ("
                    + contactIds.toString() + ")", null);
            db.setTransactionSuccessful();
        } finally {
            db.endTransaction();
        }
    }

    /**
     * Inserts updated contacts as rows to the smartdial table.
     *
     * @param db Database pointer to the smartdial database.
     * @param updatedContactCursor Cursor pointing to the list of recently updated contacts.
     * @param currentMillis Current time to be recorded in the smartdial table as update timestamp.
     */
    @VisibleForTesting
    protected void insertUpdatedContactsAndNumberPrefix(SQLiteDatabase db,
            Cursor updatedContactCursor, Long currentMillis) {
        db.beginTransaction();
        try {
            final String sqlInsert = "INSERT INTO " + Tables.SMARTDIAL_TABLE + " (" +
                    SmartDialDbColumns.DATA_ID + ", " +
                    SmartDialDbColumns.NUMBER + ", " +
                    SmartDialDbColumns.CONTACT_ID + ", " +
                    SmartDialDbColumns.LOOKUP_KEY + ", " +
                    SmartDialDbColumns.DISPLAY_NAME_PRIMARY + ", " +
                    SmartDialDbColumns.PHOTO_ID + ", " +
                    SmartDialDbColumns.LAST_TIME_USED + ", " +
                    SmartDialDbColumns.TIMES_USED + ", " +
                    SmartDialDbColumns.STARRED + ", " +
                    SmartDialDbColumns.IS_SUPER_PRIMARY + ", " +
                    SmartDialDbColumns.IN_VISIBLE_GROUP+ ", " +
                    SmartDialDbColumns.IS_PRIMARY + ", " +
                    SmartDialDbColumns.CARRIER_PRESENCE + ", " +
                    SmartDialDbColumns.LAST_SMARTDIAL_UPDATE_TIME + ", " +
                    /// M: [MTK-Dialer-Search] @{
                    SmartDialDbColumns.TYPE + ", " +
                    SmartDialDbColumns.LABEL + ", " +
                    SmartDialDbColumns.INDICATE_PHONE_SIM + ", " +
                    SmartDialDbColumns.IS_SDN + ", " +
                    SmartDialDbColumns.NORMALIZED_NAME + ", " +
                    SmartDialDbColumns.SEARCH_DATA_OFFSETS + ", " +
                    SmartDialDbColumns.SORT_KEY + ", " +
                    SmartDialDbColumns.INITIALS + ") " +
                    /// @}
                    " VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
            final SQLiteStatement insert = db.compileStatement(sqlInsert);

            final String numberSqlInsert = "INSERT INTO " + Tables.PREFIX_TABLE + " (" +
                    PrefixColumns.CONTACT_ID + ", " +
                    PrefixColumns.PREFIX  + ") " +
                    " VALUES (?, ?)";
            final SQLiteStatement numberInsert = db.compileStatement(numberSqlInsert);

            /// M: [MTK-Dialer-Search] @{
            StringBuilder nameOffsets = new StringBuilder();
            StringBuilder initials = new StringBuilder();
            StringBuilder nameSortKey = new StringBuilder();
            int indicatePhoneSimIndex =
                    updatedContactCursor.getColumnIndex(RawContactsCompat.INDICATE_PHONE_SIM);
            int sdnContactIndex =
                    updatedContactCursor.getColumnIndex(RawContactsCompat.IS_SDN_CONTACT);
            /// @}

            updatedContactCursor.moveToPosition(-1);
            while (updatedContactCursor.moveToNext()) {
                insert.clearBindings();

                // Handle string columns which can possibly be null first. In the case of certain
                // null columns (due to malformed rows possibly inserted by third-party apps
                // or sync adapters), skip the phone number row.
                /// M: Google Code
                /// final String number = updatedContactCursor.getString(PhoneQuery.PHONE_NUMBER);

                /// M: [MTK-Dialer-Search]
                /// normalize number before insert to database (phone type number only) @{
                String originNumber = updatedContactCursor.getString(PhoneQuery.PHONE_NUMBER);
                String number;
                if (PhoneNumberHelper.isUriNumber(originNumber)) {
                    number = originNumber;
                } else {
                    number = DialerSearchUtils.normalizeNumber(originNumber);
                }
                /// @}
                if (TextUtils.isEmpty(number)) {
                    continue;
                } else {
                    insert.bindString(2, number);
                }

                final String lookupKey = updatedContactCursor.getString(
                        PhoneQuery.PHONE_LOOKUP_KEY);
                if (TextUtils.isEmpty(lookupKey)) {
                    continue;
                } else {
                    insert.bindString(4, lookupKey);
                }

                final String displayName = updatedContactCursor.getString(
                        PhoneQuery.PHONE_DISPLAY_NAME);
                if (displayName == null) {
                    insert.bindString(5, mContext.getResources().getString(R.string.missing_name));
                } else {
                    insert.bindString(5, displayName);
                }
                insert.bindLong(1, updatedContactCursor.getLong(PhoneQuery.PHONE_ID));
                insert.bindLong(3, updatedContactCursor.getLong(PhoneQuery.PHONE_CONTACT_ID));
                insert.bindLong(6, updatedContactCursor.getLong(PhoneQuery.PHONE_PHOTO_ID));
                insert.bindLong(7, updatedContactCursor.getLong(PhoneQuery.PHONE_LAST_TIME_USED));
                insert.bindLong(8, updatedContactCursor.getInt(PhoneQuery.PHONE_TIMES_USED));
                insert.bindLong(9, updatedContactCursor.getInt(PhoneQuery.PHONE_STARRED));
                insert.bindLong(10, updatedContactCursor.getInt(PhoneQuery.PHONE_IS_SUPER_PRIMARY));
                insert.bindLong(11, updatedContactCursor.getInt(PhoneQuery.PHONE_IN_VISIBLE_GROUP));
                insert.bindLong(12, updatedContactCursor.getInt(PhoneQuery.PHONE_IS_PRIMARY));
                insert.bindLong(13, updatedContactCursor.getInt(PhoneQuery.PHONE_CARRIER_PRESENCE));
                insert.bindLong(14, currentMillis);

                /// M: [MTK-Dialer-Search] @{
                insert.bindLong(15, updatedContactCursor.getInt(PhoneQuery.PHONE_TYPE));
                bindString(insert, 16, updatedContactCursor.getString(PhoneQuery.PHONE_LABEL));
                insert.bindString(17,
                        indicatePhoneSimIndex == -1 ? ""
                              :updatedContactCursor.getString(PhoneQuery.PHONE_INDICATE_PHONE_SIM));
                insert.bindString(18,
                        sdnContactIndex == -1 ? ""
                                :updatedContactCursor.getString(PhoneQuery.PHONE_IS_SDN_CONTACT));

                /** M: Normalize the displayName. It can translate Chinese character to Pinyin
                 *  and calculate the offset of each character.
                 *  eg. 张三丰  -->  5ZHANG3SAN4FENG - 500000311142222
                 *      Justina --> 7JUSTINA = 70123456
                 *      Li Lei --> 2LI3LEI - 2013345@{
                 */
                nameOffsets.delete(0, nameOffsets.length());
                String normalizedName = HanziToPinyin.getInstance()
                        .getTokensForDialerSearch(displayName, nameOffsets);

                initials.delete(0, initials.length());
                nameSortKey.delete(0, nameSortKey.length());

                String nameOffsetsStr = nameOffsets.toString();
                generateNormalizedNameParams(normalizedName, nameOffsetsStr, initials, nameSortKey);

                bindString(insert, 19, normalizedName);
                bindString(insert, 20, nameOffsetsStr);
                bindString(insert, 21, nameSortKey.toString());
                bindString(insert, 22, normalizeToDialpadString(initials.toString()));
                /// @}

                insert.executeInsert();
                /*
                 * M: Google Code
                final String contactPhoneNumber =
                        updatedContactCursor.getString(PhoneQuery.PHONE_NUMBER);
                final ArrayList<String> numberPrefixes =
                        SmartDialPrefix.parseToNumberTokens(contactPhoneNumber);

                for (String numberPrefix : numberPrefixes) {
                    numberInsert.bindLong(1, updatedContactCursor.getLong(
                            PhoneQuery.PHONE_CONTACT_ID));
                    numberInsert.bindString(2, numberPrefix);
                    numberInsert.executeInsert();
                    numberInsert.clearBindings();
                }
                */
            }

            db.setTransactionSuccessful();
        } finally {
            db.endTransaction();
        }
    }

    /**
     * Inserts prefixes of contact names to the prefix table.
     *
     * @param db Database pointer to the smartdial database.
     * @param nameCursor Cursor pointing to the list of distinct updated contacts.
     */
    @VisibleForTesting
    void insertNamePrefixes(SQLiteDatabase db, Cursor nameCursor) {
        // Google Code
        //final int columnIndexName = nameCursor.getColumnIndex(
        //        SmartDialDbColumns.DISPLAY_NAME_PRIMARY);
        /// M: [MTK-Dialer-Search] Use normalizedName instead of displayName
        final int columnIndexName = nameCursor.getColumnIndex(
                SmartDialDbColumns.NORMALIZED_NAME);

        final int columnIndexContactId = nameCursor.getColumnIndex(SmartDialDbColumns.CONTACT_ID);

        db.beginTransaction();
        try {
            final String sqlInsert = "INSERT INTO " + Tables.PREFIX_TABLE + " (" +
                    PrefixColumns.CONTACT_ID + ", " +
                    PrefixColumns.PREFIX  + ") " +
                    " VALUES (?, ?)";
            final SQLiteStatement insert = db.compileStatement(sqlInsert);

            while (nameCursor.moveToNext()) {
                /** Computes a list of prefixes of a given contact name. */
                /// M: [MTK-Dialer-Search] Using enhanced SmartDialPrefixEx
                final ArrayList<String> namePrefixes = SmartDialPrefixEx
                        .generateNamePrefixes(mContext,nameCursor.getString(columnIndexName));

                for (String namePrefix : namePrefixes) {
                    insert.bindLong(1, nameCursor.getLong(columnIndexContactId));
                    insert.bindString(2, namePrefix);
                    insert.executeInsert();
                    insert.clearBindings();
                }
            }

            db.setTransactionSuccessful();
        } finally {
            db.endTransaction();
        }
    }

    /**
     * Updates the smart dial and prefix database.
     * This method queries the Delta API to get changed contacts since last update, and updates the
     * records in smartdial database and prefix database accordingly.
     * It also queries the deleted contact database to remove newly deleted contacts since last
     * update.
     */
    public void updateSmartDialDatabase() {
        if (!PermissionsUtil.hasContactsReadPermissions(mContext)) {
            return;
        }
        final SQLiteDatabase db = getWritableDatabase();

        //synchronized(mLock) {
            LogUtil.i(TAG, "Starting to update database");

            final StopWatch stopWatch = StopWatch.start("Updating databases");

            /** Gets the last update time on the database. */
            final SharedPreferences databaseLastUpdateSharedPref = mContext.getSharedPreferences(
                    DATABASE_LAST_CREATED_SHARED_PREF, Context.MODE_PRIVATE);
            final String lastUpdateMillis = String.valueOf(
                    databaseLastUpdateSharedPref.getLong(LAST_UPDATED_MILLIS, 0));

            LogUtil.i(TAG, "Last updated at " + lastUpdateMillis);

            /** Sets the time after querying the database as the current update time. */
            final Long currentMillis = System.currentTimeMillis();


            ///M: Add for ALPS04686197
            Long recordTime;


            stopWatch.lap("Queried the Contacts database");

            /** Prevents the app from reading the dialer database when updating. */
            sInUpdate.getAndSet(true);

            /** Removes contacts that have been deleted. */
            /// M: Google Code
            // removeDeletedContacts(db, getDeletedContactCursor(lastUpdateMillis));
            /// M: Handle batch delete contacts. @{
            try {
                removeDeletedContacts(db, lastUpdateMillis);
            } catch (SQLiteException e) {
                // M: add this catch for ALPS03680558.
                // When disk is full, db.endTransaction() will throw a SQLiteException
                // so add this catch to avoid this JE.
                Log.e(TAG, "removeDeletedContacts, " +
                               "SQLiteException has been catched, maybe disk is full");
            }
            /// @}
            removePotentiallyCorruptedContacts(db, lastUpdateMillis);
            stopWatch.lap("Finished deleting deleted entries");

            /** If the database did not exist before, jump through deletion as there is nothing
             * to delete.
             */
            if (!lastUpdateMillis.equals("0")) {
                /** Removes contacts that have been updated. Updated contact information will be
                 * inserted later. Note that this has to use a separate result set from
                 * updatePhoneCursor, since it is possible for a contact to be updated (e.g.
                 * phone number deleted), but have no results show up in updatedPhoneCursor (since
                 * all of its phone numbers have been deleted).
                 */
                /// M: Google Code
    //            final Cursor updatedContactCursor = mContext.getContentResolver().query(
    //                    UpdatedContactQuery.URI,
    //                    UpdatedContactQuery.PROJECTION,
    //                    UpdatedContactQuery.SELECT_UPDATED_CLAUSE,
    //                    new String[] {lastUpdateMillis},
    //                    null
    //                    );

                /// M: [MTK-Dialer-Search]
                /// Handle the time is set to the past. @{
                /** Queries the contact database to get contacts that have been updated
                 * since the last update time.
                 */
                LogUtil.i(TAG, "!lastUpdateMillis.equals(0)");
                final Cursor updatedContactCursor = getUpdatedContactCursor(
                        lastUpdateMillis, currentMillis);
                /// @}
                if (updatedContactCursor == null) {
                    Log.e(TAG, "SmartDial query received null for cursor");
                    return;
                }
                try {
                    removeUpdatedContacts(db, updatedContactCursor);
                } catch (SQLiteException e) {
                    // M: add this catch for ALPS03540066.
                    // When disk is full, db.endTransaction() will throw a SQLiteException
                    // so add this catch to avoid this JE.
                    Log.e(TAG, "SQLiteException has been catched, maybe disk is full");
                } finally {
                    updatedContactCursor.close();
                }
                stopWatch.lap("Finished deleting entries belonging to updated contacts");
              }

            /** Queries the contact database to get all phone numbers that have been updated
             * since the last update time.
             */
            /* M: Google Code
            final Cursor updatedPhoneCursor = mContext.getContentResolver().query(PhoneQuery.URI,
                    PhoneQuery.PROJECTION, PhoneQuery.SELECTION,
                    new String[]{lastUpdateMillis}, null);
            */
            /// M: Handle the time is set to the past.
            final Cursor updatedPhoneCursor = getUpdatedContactCursor(
                    lastUpdateMillis, currentMillis);
            if (updatedPhoneCursor == null) {
                Log.e(TAG, "SmartDial query received null for cursor");
                return;
            }
            ///M: Add for ALPS04686197
            recordTime = getRecordTime(lastUpdateMillis,currentMillis, updatedPhoneCursor);
            try {
                /** Inserts recently updated phone numbers to the smartdial database.*/
                ///M: Add for ALPS04686197, fix parameter of insertUpdatedContactsAndNumberPrefix
                insertUpdatedContactsAndNumberPrefix(db, updatedPhoneCursor, recordTime);
                stopWatch.lap("Finished building the smart dial table");
             } catch (SQLiteException e) {
                // M: add this catch for ALPS03615301.
                // When disk is full, db.endTransaction() will throw a SQLiteException
                // so add this catch to avoid this JE.
                Log.e(TAG, "insertUpdatedContactsAndNumberPrefix exception, " +
                                  "SQLiteException has been catched, maybe disk is full");
            } finally {
                updatedPhoneCursor.close();
            }

            /** Gets a list of distinct contacts which have been updated, and adds the name prefixes
             * of these contacts to the prefix table.
             */
            final Cursor nameCursor = db.rawQuery(
                    "SELECT DISTINCT " +
                    /// M: Google Code
                    //SmartDialDbColumns.DISPLAY_NAME_PRIMARY + ","+ SmartDialDbColumns.CONTACT_ID +
                    /// M: Use normalizedName instead of displayName
                    SmartDialDbColumns.NORMALIZED_NAME + ", " + SmartDialDbColumns.CONTACT_ID +
                    " FROM " + Tables.SMARTDIAL_TABLE +
                    " WHERE " + SmartDialDbColumns.LAST_SMARTDIAL_UPDATE_TIME +
                    " = " + Long.toString(recordTime),
                    new String[] {});
            if (nameCursor != null) {
                try {
                    stopWatch.lap("Queried the smart dial table for contact names");
                    /** Inserts prefixes of names into the prefix table.*/
                    insertNamePrefixes(db, nameCursor);
                    stopWatch.lap("Finished building the name prefix table");
                } finally {
                    nameCursor.close();
                }
            }

            /** Creates index on contact_id for fast JOIN operation. */
            db.execSQL("CREATE INDEX IF NOT EXISTS smartdial_contact_id_index ON " +
                    Tables.SMARTDIAL_TABLE + " (" + SmartDialDbColumns.CONTACT_ID  + ");");
            /** Creates index on last_smartdial_update_time for fast SELECT operation. */
            db.execSQL("CREATE INDEX IF NOT EXISTS smartdial_last_update_index ON " +
                    Tables.SMARTDIAL_TABLE + " (" +
                    SmartDialDbColumns.LAST_SMARTDIAL_UPDATE_TIME + ");");
            /** Creates index on sorting fields for fast sort operation. */
            db.execSQL("CREATE INDEX IF NOT EXISTS smartdial_sort_index ON " +
                    Tables.SMARTDIAL_TABLE + " (" +
                    SmartDialDbColumns.STARRED + ", " +
                    SmartDialDbColumns.IS_SUPER_PRIMARY + ", " +
                    SmartDialDbColumns.LAST_TIME_USED + ", " +
                    SmartDialDbColumns.TIMES_USED + ", " +
                    SmartDialDbColumns.IN_VISIBLE_GROUP +  ", " +
                    SmartDialDbColumns.DISPLAY_NAME_PRIMARY + ", " +
                    SmartDialDbColumns.CONTACT_ID + ", " +
                    SmartDialDbColumns.IS_PRIMARY +
                    ");");
            /** Creates index on prefix for fast SELECT operation. */
            db.execSQL("CREATE INDEX IF NOT EXISTS nameprefix_index ON " +
                    Tables.PREFIX_TABLE + " (" + PrefixColumns.PREFIX + ");");
            /** Creates index on contact_id for fast JOIN operation. */
            db.execSQL("CREATE INDEX IF NOT EXISTS nameprefix_contact_id_index ON " +
                    Tables.PREFIX_TABLE + " (" + PrefixColumns.CONTACT_ID + ");");

            stopWatch.lap(TAG + "Finished recreating index");
            /** Updates the database index statistics.*/
            db.execSQL("ANALYZE " + Tables.SMARTDIAL_TABLE);
            db.execSQL("ANALYZE " + Tables.PREFIX_TABLE);
            db.execSQL("ANALYZE smartdial_contact_id_index");
            db.execSQL("ANALYZE smartdial_last_update_index");
            db.execSQL("ANALYZE nameprefix_index");
            db.execSQL("ANALYZE nameprefix_contact_id_index");
            stopWatch.stopAndLog(TAG + "Finished updating index stats", 0);
            sInUpdate.getAndSet(false);

            final SharedPreferences.Editor editor = databaseLastUpdateSharedPref.edit();
            ///M: Add for ALPS04686197, add recordTime into peference 
            editor.putLong(LAST_UPDATED_MILLIS, recordTime);
            editor.commit();

            /*// Notify content observers that smart dial database has been updated.
            mContext.getContentResolver().notifyChange(SMART_DIAL_UPDATED_URI, null, false);*/
        //}
    }

    ///M: Add for ALPS04686197 @{
    long  getRecordTime(String lastUpdateMillis, long currentMillis, Cursor updatedContactCursor) {
        long lastUpdateTime = Long.valueOf(lastUpdateMillis);
        long recordtime = lastUpdateTime;
        LogUtil.i(TAG, "updatedContactCursor count = " + updatedContactCursor.getCount());
        if (updatedContactCursor.moveToLast()) {
            int index = updatedContactCursor.getColumnIndex(Phone.CONTACT_LAST_UPDATED_TIMESTAMP);
            LogUtil.i(TAG, "updatedContactCursor.getColumnIndex = " + index);
            if (index != -1) {
                LogUtil.i(TAG, "updatedContactCursor.getLong(index) = " + updatedContactCursor.getLong(index));
            }
            if (index != -1 && updatedContactCursor.getLong(index) <= currentMillis) {
                recordtime  = updatedContactCursor.getLong(index);
                LogUtil.i(TAG, "recordtime = " + recordtime + " currentMillis = " + currentMillis);
            }
        }
        LogUtil.i(TAG, "return recordtime = " + recordtime);
        return recordtime;
     }
    ///@}

    /// M: Comment out this method and using the enhanced getLooseMatch method.
//    /**
//     * Returns a list of candidate contacts where the query is a prefix of the dialpad index of
//     * the contact's name or phone number.
//     *
//     * @param query The prefix of a contact's dialpad index.
//     * @return
//     * A list of top candidate contacts that will be suggested to user to match their input.
//     */
//    public ArrayList<ContactNumber>  getLooseMatches(String query,
//            SmartDialNameMatcher nameMatcher) {
//        final boolean inUpdate = sInUpdate.get();
//        if (inUpdate) {
//            return Lists.newArrayList();
//        }
//
//        final SQLiteDatabase db = getReadableDatabase();
//
//        /** Uses SQL query wildcard '%' to represent prefix matching.*/
//        final String looseQuery = query + "%";
//
//        final ArrayList<ContactNumber> result = Lists.newArrayList();
//
//        final StopWatch stopWatch = DEBUG ? StopWatch.start(":Name Prefix query") : null;
//
//        final String currentTimeStamp = Long.toString(System.currentTimeMillis());
//
//        /** Queries the database to find contacts that have an index matching the query prefix. */
//        final Cursor cursor = db.rawQuery("SELECT " +
//                SmartDialDbColumns.DATA_ID + ", " +
//                SmartDialDbColumns.DISPLAY_NAME_PRIMARY + ", " +
//                SmartDialDbColumns.PHOTO_ID + ", " +
//                SmartDialDbColumns.NUMBER + ", " +
//                SmartDialDbColumns.CONTACT_ID + ", " +
//                SmartDialDbColumns.LOOKUP_KEY + ", " +
//                SmartDialDbColumns.CARRIER_PRESENCE +
//                " FROM " + Tables.SMARTDIAL_TABLE + " WHERE " +
//                SmartDialDbColumns.CONTACT_ID + " IN " +
//                    " (SELECT " + PrefixColumns.CONTACT_ID +
//                    " FROM " + Tables.PREFIX_TABLE +
//                    " WHERE " + Tables.PREFIX_TABLE + "." + PrefixColumns.PREFIX +
//                    " LIKE '" + looseQuery + "')" +
//                " ORDER BY " + SmartDialSortingOrder.SORT_ORDER,
//                new String[] {currentTimeStamp});
//        if (cursor == null) {
//            return result;
//        }
//        try {
//            if (DEBUG) {
//                stopWatch.lap("Prefix query completed");
//            }
//
//            /** Gets the column ID from the cursor.*/
//            final int columnDataId = 0;
//            final int columnDisplayNamePrimary = 1;
//            final int columnPhotoId = 2;
//            final int columnNumber = 3;
//            final int columnId = 4;
//            final int columnLookupKey = 5;
//            final int columnCarrierPresence = 6;
//            if (DEBUG) {
//                stopWatch.lap("Found column IDs");
//            }
//
//            final Set<ContactMatch> duplicates = new HashSet<ContactMatch>();
//            int counter = 0;
//            if (DEBUG) {
//                stopWatch.lap("Moved cursor to start");
//            }
//            /** Iterates the cursor to find top contact suggestions without duplication.*/
//            while ((cursor.moveToNext()) && (counter < MAX_ENTRIES)) {
//                final long dataID = cursor.getLong(columnDataId);
//                final String displayName = cursor.getString(columnDisplayNamePrimary);
//                final String phoneNumber = cursor.getString(columnNumber);
//                final long id = cursor.getLong(columnId);
//                final long photoId = cursor.getLong(columnPhotoId);
//                final String lookupKey = cursor.getString(columnLookupKey);
//                final int carrierPresence = cursor.getInt(columnCarrierPresence);
//
//                /** If a contact already exists and another phone number of the contact is being
//                 * processed, skip the second instance.
//                 */
//                final ContactMatch contactMatch = new ContactMatch(lookupKey, id);
//                if (duplicates.contains(contactMatch)) {
//                    continue;
//                }
//
//                /**
//                 * If the contact has either the name or number that matches the query, add to the
//                 * result.
//                 */
//                final boolean nameMatches = nameMatcher.matches(displayName);
//                final boolean numberMatches =
//                        (nameMatcher.matchesNumber(phoneNumber, query) != null);
//                if (nameMatches || numberMatches) {
//                    /** If a contact has not been added, add it to the result and the hash set.*/
//                    duplicates.add(contactMatch);
//                    result.add(new ContactNumber(id, dataID, displayName, phoneNumber, lookupKey,
//                            photoId, carrierPresence));
//                    counter++;
//                    if (DEBUG) {
//                        stopWatch.lap("Added one result: Name: " + displayName);
//                    }
//                }
//            }
//
//            if (DEBUG) {
//                stopWatch.stopAndLog(TAG + "Finished loading cursor", 0);
//            }
//        } finally {
//            cursor.close();
//        }
//        return result;
//    }

    //////////////// MTK Enhanced Dialer Search ///////////////////////////////////////

    private static final String[] CALLLOG_PROJECTION;
    private static final String[] PROJECTION = new String[] {
            Calls._ID,
            Calls.NUMBER,
            Calls.NUMBER_PRESENTATION,
            Calls.DATE,
            Calls.TYPE,
            Calls.GEOCODED_LOCATION,
            Calls.PHONE_ACCOUNT_COMPONENT_NAME,
            Calls.PHONE_ACCOUNT_ID
    };
    static {
        List<String> projectionList = Lists.newArrayList(PROJECTION);
        if (isNCompatible()) {
            projectionList.add(Calls.POST_DIAL_DIGITS);
        }
        CALLLOG_PROJECTION = projectionList.toArray(new String[projectionList.size()]);
    }

    int countSmartdialTableRows(SQLiteDatabase db) {
        return (int)DatabaseUtils.longForQuery(db, "SELECT COUNT(1) FROM " + Tables.SMARTDIAL_TABLE,
                null);
    }

    /**
     * get the parameters about contact by normalized name
     * @param normalizedName e.g. 5ZHANG3SAN4FENG
     * @param nameOffsets    e.g. 500000311142222
     * @param initials e.g. ZSF
     * @param nameSortKey e.g. ZHANG SAN FENG
     */
    private void generateNormalizedNameParams(String normalizedName, String nameOffsets,
            StringBuilder initials, StringBuilder nameSortKey) {
        if (null == normalizedName) {
            Log.w(TAG, "generateNormalizedNameParams null name.");
            return;
        }
        int len;
        final int length = normalizedName.length();
        for (int i = 0; i < length; i = i + len + 1) {
            len = (int) normalizedName.charAt(i);
            nameSortKey.append(normalizedName.substring(i + 1, i + len + 1) + " ");
            initials.append(normalizedName.charAt(i + 1));
        }
    }

    private void bindString(SQLiteStatement stmt, int index, String value) {
        if (value == null) {
            stmt.bindNull(index);
        } else {
            stmt.bindString(index, value);
        }
    }

    /**
     * Normalize regular query string to dialpad string which consists of numeric characters
     */
    private String normalizeToDialpadString(String query) {
        SmartDialMap map = SmartDialPrefixEx.getMap();
        StringBuilder normaizeQuery = new StringBuilder();
        for (int i = 0; i < query.length(); i++) {
            //char ch = map.normalizeCharacter(query.charAt(i));
            char ch = CompositeSmartDialMap.normalizeCharacter(mContext,query.charAt(i));
            byte nc = CompositeSmartDialMap.getDialpadIndex(mContext,ch);
            //Log.i(TAG, "normalizeToDialpadString, nc = " + nc + " ch = " + ch);
            if (nc == -1) {
                normaizeQuery.append(ch);
            } else {
                normaizeQuery.append(nc);
            }
        }
        return normaizeQuery.toString();
    }

    /**
     * Removes rows in the smartdial database that matches the contacts that have been deleted
     * by other apps since last update.
     *
     * @param db Database pointer to the dialer database.
     * @param lastUpdateMillis Time stamp of last update on the smartdial database
     */
    private void removeDeletedContacts(SQLiteDatabase db, String lastUpdateMillis) {
        LogUtil.d(TAG, "removeDeletedContacts start.");
        //TODO: here is temporary solution to handle deleted contacts.
        int rows = countSmartdialTableRows(db);
        if (rows == 0) {
            // Nothing to do for empty table.
            return;
        }
        final Cursor existedContactCursor = mContext.getContentResolver().query(PhoneQuery.URI,
                new String[] {Phone.CONTACT_ID}, null, null, null);
        if (existedContactCursor == null || existedContactCursor.getCount() == 0) {
            db.delete(Tables.SMARTDIAL_TABLE, null, null);
            db.delete(Tables.PREFIX_TABLE, null, null);
            if (null != existedContactCursor) {
                existedContactCursor.close();
            }
            return;
        }
        StringBuilder contactIds = new StringBuilder();
        while (existedContactCursor.moveToNext()) {
            contactIds.append(existedContactCursor.getLong(0)).append(',');
        }
        existedContactCursor.close();
        // remove the last ","
        contactIds.deleteCharAt(contactIds.lastIndexOf(","));
        db.beginTransaction();
        try {
            db.delete(Tables.SMARTDIAL_TABLE, SmartDialDbColumns.CONTACT_ID + " NOT IN ("
                    + contactIds.toString() + ")", null);
            db.delete(Tables.PREFIX_TABLE,
                    PrefixColumns.CONTACT_ID + " NOT IN (" + contactIds.toString() + ")", null);
            db.setTransactionSuccessful();
        } finally {
            db.endTransaction();
            LogUtil.d(TAG, "removeDeletedContacts end.");
        }
    }

    /**
     * Get the updated contacts since last time update.
     * @param lastUpdateMillis
     * @param currentMillis
     * @return
     */
    private Cursor getUpdatedContactCursor(String lastUpdateMillis, long currentMillis) {
         LogUtil.i(TAG, "YF: Last updated at " + lastUpdateMillis
                    + " ,currentMillis: " + currentMillis);
        /** Queries the contact database to get contacts that have been updated since the last
         * update time.
         */
        long lastUpdateTime = Long.valueOf(lastUpdateMillis);
        final Cursor updatedContactCursor;
        /// M:[portable][MTK SIM Contacts feature] @{
        List<String> projectionList = Lists.newArrayList(PhoneQuery.PROJECTION);
        if (DialerFeatureOptions.isSimContactsSupport()) {
            projectionList.add(RawContactsCompat.INDICATE_PHONE_SIM);
            projectionList.add(RawContactsCompat.IS_SDN_CONTACT);
        }

       ///M: Add for ALPS04686197  @{
       projectionList.add(Phone.CONTACT_LAST_UPDATED_TIMESTAMP);
       /// @}

        String[] projection = projectionList.toArray(new String[projectionList.size()]);
        /// @}
        if (0 != lastUpdateTime && lastUpdateTime < currentMillis) {
            // Update the range.
            updatedContactCursor = mContext.getContentResolver().query(PhoneQuery.URI,
                    /*PhoneQuery.PROJECTION*/projection, PhoneQuery.SELECTION_RANGE,
                    new String[]{lastUpdateMillis, String.valueOf(currentMillis)}, null);
        } else {
            // Update all.
            updatedContactCursor = mContext.getContentResolver().query(PhoneQuery.URI,
                    /*PhoneQuery.PROJECTION*/projection, PhoneQuery.SELECTION,
                    new String[]{lastUpdateMillis}, null);
        }
        LogUtil.i(TAG, "YF:updatedContactCursor: " + updatedContactCursor);

        return updatedContactCursor;
    }

    /**
     * Simple data format for CallLog.
     */
    public static class CallLogInfo {
        public final long callLogId;
        public final String callNumber;
        public final int presentation;
        public final String  date;
        public final int  type;
        public final String geoLocation;
        public final String phoneAccountComponent;
        public final String phoneAccountId;
        public final String dataOffset;

        public CallLogInfo(long callLogId, String callNumber, int presentation, String date,
                int type, String geoLocation, String phoneAccountComponent, String phoneAccountId,
                String dataOffset) {
            this.callLogId = callLogId;
            this.callNumber = callNumber;
            this.presentation = presentation;
            this.date = date;
            this.type = type;
            this.geoLocation = geoLocation;
            this.phoneAccountComponent = phoneAccountComponent;
            this.phoneAccountId = phoneAccountId;
            this.dataOffset = dataOffset;
        }

        @Override
        public int hashCode() {
            return Objects.hashCode(callLogId, callNumber, presentation, date, type,
                    geoLocation, phoneAccountComponent, phoneAccountId);
        }

        @Override
        public boolean equals(Object object) {
            if (this == object) {
                return true;
            }
            if (object instanceof CallLogInfo) {
                final CallLogInfo that = (CallLogInfo)object;
                return Objects.equal(this.callLogId , that.callLogId)
                        && Objects.equal(this.callNumber, that.callNumber)
                        && Objects.equal(this.presentation, that.presentation)
                        && Objects.equal(this.date, that.date)
                        && Objects.equal(this.type, that.type)
                        && Objects.equal(this.geoLocation, that.geoLocation)
                        && Objects.equal(this.phoneAccountComponent, that.phoneAccountComponent)
                        && Objects.equal(this.phoneAccountId, that.phoneAccountId)
                        && Objects.equal(this.dataOffset, that.dataOffset);
            }
            return false;
        }
    }

    // Define columns of smart dial callLog table
    public interface SmartDialCallLogColumns extends BaseColumns {
        static final String CALLLOG_ID = "callLog_id";
        static final String NUMBER = Calls.NUMBER;
        static final String NUMBER_PRESENTATION = Calls.NUMBER_PRESENTATION;
        static final String DATE = Calls.DATE;
        static final String TYPE = Calls.TYPE;  // incoming, outgoing , missed or voicemail
        static final String GEOCODED_LOCATION = Calls.GEOCODED_LOCATION;
        static final String PHONE_ACCOUNT_COMPONENT_NAME = Calls.PHONE_ACCOUNT_COMPONENT_NAME;
        static final String PHONE_ACCOUNT_ID = Calls.PHONE_ACCOUNT_ID;
    }

    /**
     * update call log information.
     */
    public void startCallLogUpdateThread() {
        if (PermissionsUtil.hasPhonePermissions(mContext)) {
            new CallLogUpdateAsyncTask().execute();
        } else {
            ///M: print log if permission was not enabled!
            Log.d(TAG, "CallLogUpdate has no phone permissions!");
        }
    }

    private class CallLogUpdateAsyncTask extends AsyncTask {

        @Override
        protected Object doInBackground(Object... arg0) {
            LogUtil.d(TAG, "Updating callLog in CallLogUpdateAsyncTask");
            synchronized(mLock) {
                try {
                    sInUpdate.getAndSet(true);
                    updateSmartDialCallLog();
                    notifyChange();
                    sInUpdate.getAndSet(false);
                } catch (SQLiteException e) {
                    //M: add this catch for ALPS03959520
                    Log.e(TAG, "updateSmartDialCallLog exception, " +
                    "SQLiteException has been catched, maybe disk is full");
                }
            }

            return null;
        }

    }

    /**
     * Starts the contacts information update process in the background.
     */
    public void startContactUpdateThread() {
        if (PermissionsUtil.hasContactsReadPermissions(mContext)) {
            new ContactUpdateAsyncTask().execute();
        } else {
            ///M: print log if permission was not enabled!
            LogUtil.i(TAG, "ContactUpdate has no contact permission!");
        }
    }

    private class ContactUpdateAsyncTask extends AsyncTask {

        @Override
        protected Object doInBackground(Object... arg0) {
            LogUtil.i(TAG, "YF: Updating contact in ContactUpdateAsyncTask");
            synchronized(mLock) {
                sInUpdate.getAndSet(true);
                try {
                    updateSmartDialDatabase();
                } catch (SQLiteException e) {
                    ///M: add this catch for ALPS04670162
                    Log.e(TAG, "ContactUpdateAsyncTask, update failed.", e);
                }
                notifyChange();
                sInUpdate.getAndSet(false);
            }

            return null;
        }

    }

    private void updateSmartDialCallLog() {
        if (!PermissionsUtil.hasPhonePermissions(mContext)) {
            return;
        }
        SQLiteDatabase db = getWritableDatabase();
        // Delete all the smartDial callLog item
        db.delete(Tables.SMARTDIAL_CALLLOG_TABLE, null, null);

        // Copy all call logs from callLog provider
        final Cursor cursor = mContext.getContentResolver().query(Calls.CONTENT_URI,
                CALLLOG_PROJECTION, null, null, null);
        if (cursor == null) {
            Log.e(TAG, "updateSmartDialCallLog got null cursor, return");
            return;
        }

        LogUtil.d(TAG, "updateSmartDialCallLog callLog count : " + cursor.getCount());
        db.beginTransaction();
        try {
            final String sqlInsert = "INSERT INTO " + Tables.SMARTDIAL_CALLLOG_TABLE + " (" +
                    SmartDialCallLogColumns.CALLLOG_ID + ", " +
                    SmartDialCallLogColumns.NUMBER+ ", " +
                    SmartDialCallLogColumns.NUMBER_PRESENTATION + ", " +
                    SmartDialCallLogColumns.DATE + ", " +
                    SmartDialCallLogColumns.TYPE + ", " +
                    SmartDialCallLogColumns.GEOCODED_LOCATION + ", " +
                    SmartDialCallLogColumns.PHONE_ACCOUNT_COMPONENT_NAME + ", " +
                    SmartDialCallLogColumns.PHONE_ACCOUNT_ID + ") " +
                    " VALUES (?, ?, ?, ?, ?, ?, ?, ?)";
            final SQLiteStatement insert = db.compileStatement(sqlInsert);

            int callsIdIndex = cursor.getColumnIndex(Calls._ID);
            int callsNumberIndex = cursor.getColumnIndex(Calls.NUMBER);
            int callsPresentationIndex = cursor.getColumnIndex(Calls.NUMBER_PRESENTATION);
            int callsDateIndex = cursor.getColumnIndex(Calls.DATE);
            int callsTypeIndex = cursor.getColumnIndex(Calls.TYPE);
            int callsGeoIndex = cursor.getColumnIndex(Calls.GEOCODED_LOCATION);
            int callsAccountNameIndex = cursor.getColumnIndex(Calls.PHONE_ACCOUNT_COMPONENT_NAME);
            int callsAccountIdIndex = cursor.getColumnIndex( Calls.PHONE_ACCOUNT_ID);
            int callsPostDialDigitsIndex = isNCompatible() ? cursor
                    .getColumnIndex(Calls.POST_DIAL_DIGITS) : -1;

            cursor.moveToPosition(-1);
            String number;
            while (cursor.moveToNext()) {
                insert.clearBindings();
                insert.bindLong(1, cursor.getInt(callsIdIndex));
                number = cursor.getString(callsNumberIndex);
                if (isNCompatible()) {
                    String digitalNumber = cursor.getString(callsPostDialDigitsIndex);
                    if (!TextUtils.isEmpty(digitalNumber)) {
                        number += digitalNumber;
                    }
                }
                /// normalize number before insert to database (phone type number only) @{
                if (!PhoneNumberHelper.isUriNumber(number)) {
                    number = DialerSearchUtils.normalizeNumber(number);
                }
                /// @}
                if (TextUtils.isEmpty(number)) {
                    continue;
                }
                bindString(insert, 2, number);
                bindString(insert, 3, cursor.getString(callsPresentationIndex));
                bindString(insert, 4, cursor.getString(callsDateIndex));
                bindString(insert, 5, cursor.getString(callsTypeIndex));
                bindString(insert, 6, cursor.getString(callsGeoIndex));
                bindString(insert, 7, cursor.getString(callsAccountNameIndex));
                bindString(insert, 8, cursor.getString(callsAccountIdIndex));

                insert.executeInsert();
            }
            db.setTransactionSuccessful();
        } finally {
            cursor.close();
            db.endTransaction();
        }
    }

    /**
     * Return a list of callLog where the callNumber matches query partially.
     */
    public ArrayList<CallLogInfo> getCallLogMatch(String query,
            DialerSearchNameMatcher nameMatcher, int callLogMaxEntries) {
        //check phone permission here-should not show call log if not has phone permission.
        if (TextUtils.isEmpty(query) || !PermissionsUtil.hasPhonePermissions(mContext)) {
            return Lists.newArrayList();
        }

        final ArrayList<CallLogInfo> result = Lists.newArrayList();
        final SQLiteDatabase db = getReadableDatabase();
        final String numLooseQuery = "%" + query + "%";
        Cursor cursor = db.rawQuery("SELECT * FROM " + Tables.SMARTDIAL_CALLLOG_TABLE
                + " WHERE " + SmartDialCallLogColumns.NUMBER + " LIKE '" + numLooseQuery + "'"
                + " ORDER BY " + SmartDialCallLogColumns.DATE + " DESC", null);
        if (cursor == null) {
            return result;
        }

        final Set<String> duplicates = new HashSet<String>();
        int columnCallLogId = 1;
        int columnNumber = 2;
        int columnPresentation = 3;
        int columnDate = 4;
        int columnType = 5;
        int columnGeoLocation = 6;
        int columnPhoneAccountComponent =7;
        int columnPhoneAccountId = 8;

        String number;

        cursor.moveToPosition(-1);
        try {
            int counter = 0;
            while (cursor.moveToNext() && counter < callLogMaxEntries) {
                number = cursor.getString(columnNumber);
                if (duplicates.contains(number)) {
                    continue;
                }

                if (nameMatcher.doNumberMatch(query, number)) {
                    duplicates.add(number);
                    result.add(new CallLogInfo(
                            cursor.getLong(columnCallLogId),
                            number,
                            cursor.getInt(columnPresentation),
                            cursor.getString(columnDate),
                            cursor.getInt(columnType),
                            cursor.getString(columnGeoLocation),
                            cursor.getString(columnPhoneAccountComponent),
                            cursor.getString(columnPhoneAccountId),
                            nameMatcher.getNumberMatchOffset()));
                    counter++;
                }
            }
        } finally {
            cursor.close();
        }
        LogUtil.d(TAG, "getCallLogMatch resultSize : " + result.size());
        return result;
    }

    /**
     * Returns a list of candidate contacts where the query is a prefix of the dialpad index of
     * the contact's name or phone number.
     *
     * A two step query to get the final result:
     *  Step 1. Just query the needed data to do a match and find out these who are matched.
     *  Step 2. Using the matched item's id to get the completed data.
     *
     * @param query The prefix of a contact's dialpad index.
     * @return A list of top candidate contacts that will be suggested to user to match their input.
     */
    public ArrayList<ContactNumber>  getLooseMatches(String query, DialerSearchNameMatcher dsnm) {
        //check contacts permission here-should not show contacts if not has contacts permission.
        if (TextUtils.isEmpty(query) || !PermissionsUtil.hasContactsReadPermissions(mContext)) {
            return Lists.newArrayList();
        }

        final SQLiteDatabase db = getReadableDatabase();
        final String currentTimeStamp = Long.toString(System.currentTimeMillis());

        LinkedHashMap<Long, MatchResult> quickMatchResult = null;
        String ids = null;

        // Step 1. Just query the needed data to do a match and find out these who are matched.
        // Speed up the match process.
        if (query.length() == 1) {
            quickMatchResult = getQuickMatchedResultOfOneChar(
                    db, query, currentTimeStamp, dsnm, MAX_ENTRIES);
        } else {
            quickMatchResult = getQuickMatchedResult(
                    db, query, currentTimeStamp, dsnm, MAX_ENTRIES);
        }

        if (null != quickMatchResult && quickMatchResult.size() > 0) {
            ids = quickMatchResult.keySet().toString().replace("[", "(").replace("]", ")");
        }

        if (TextUtils.isEmpty(ids)) {
            return Lists.newArrayList();
        }

        // Step 2. Using the matched item's id to get the completed data.
        /** Queries the database to find contacts that have an index matching the query prefix. */
        String smartdial_select_columns =
                SmartDialDbColumns.DATA_ID + ", " +
                SmartDialDbColumns.DISPLAY_NAME_PRIMARY + ", " +
                SmartDialDbColumns.PHOTO_ID + ", " +
                SmartDialDbColumns.NUMBER + ", " +
                SmartDialDbColumns.CONTACT_ID + ", " +
                SmartDialDbColumns.LOOKUP_KEY + ", " +
                SmartDialDbColumns.CARRIER_PRESENCE + ", " +
                SmartDialDbColumns.TYPE + ", " +
                SmartDialDbColumns.LABEL + ", " +
                SmartDialDbColumns.INDICATE_PHONE_SIM + ", " +
                SmartDialDbColumns.IS_SDN + ", " +
                SmartDialDbColumns._ID;
        /** Gets the column ID from the cursor.*/
        final int columnDataId = 0;
        final int columnDisplayNamePrimary = 1;
        final int columnPhotoId = 2;
        final int columnNumber = 3;
        final int columnContactId = 4;
        final int columnLookupKey = 5;
        final int columnCarrierPresence = 6;
        final int columnType = 7;
        final int columnLabel = 8;
        final int columnIndicatePhoneSim = 9;
        final int columnIsSdn = 10;
        final int columnId = 11;

        final Cursor cursor = db.rawQuery(
                "SELECT " + smartdial_select_columns
                + " FROM " + Tables.SMARTDIAL_TABLE
                + " WHERE " + SmartDialDbColumns._ID + " IN " + ids
                + " ORDER BY " + SmartDialSortingOrder.SORT_ORDER,
                new String[] { currentTimeStamp });

        if (null == cursor) {
            return Lists.newArrayList();
        }

        int[] matchTypeOrder = dsnm.getMatchTypeOrder();
        // HashMap<MatchType, LinkedHashMap<ID, MatchResult>>
        Map<Integer, ArrayList<ContactNumber>> resultMap =
                new HashMap<Integer, ArrayList<ContactNumber>>(matchTypeOrder.length);
        for (Integer key : matchTypeOrder) {
            resultMap.put(key, new ArrayList<ContactNumber>());
        }

        MatchResult mr = null;
        try {
            while (cursor.moveToNext()) {
                mr = quickMatchResult.get(cursor.getLong(columnId));
                if (mr != null) {
                    resultMap.get(mr.matchType).add(new ContactNumber(
                            cursor.getLong(columnContactId),
                            cursor.getLong(columnDataId),
                            cursor.getString(columnDisplayNamePrimary),
                            cursor.getString(columnNumber),
                            cursor.getString(columnLookupKey),
                            cursor.getLong(columnPhotoId),
                            cursor.getInt(columnCarrierPresence),
                            cursor.getInt(columnType),
                            cursor.getString(columnLabel),
                            cursor.getInt(columnIndicatePhoneSim),
                            cursor.getInt(columnIsSdn),
                            mr.nameMatchOffset,
                            mr.numberMatchOffset));
                }
            }
        } finally {
            cursor.close();
        }

        final ArrayList<ContactNumber> result = Lists.newArrayList();
        // Sort the result into match type order.
        if (resultMap.size() > 0) {
            for (int key : matchTypeOrder) {
                result.addAll(resultMap.get(key));
            }
        }

        return result;
    }

    private final class MatchResult {
        public final String nameMatchOffset;
        public final String numberMatchOffset;
        public final int matchType;
        public final long id;

        public MatchResult(String nameMatchOffset, String numberMatchOffset,
                int matchType, long id) {
            this.nameMatchOffset = nameMatchOffset;
            this.numberMatchOffset = numberMatchOffset;
            this.matchType = matchType;
            this.id = id;
        }
    }

    private LinkedHashMap<Long, MatchResult> getQuickMatchedResult(SQLiteDatabase db,
            String query, String currentTimeStamp, DialerSearchNameMatcher dsnm, int maxEntries) {
        LinkedHashMap<Long, MatchResult> emptyMap = new LinkedHashMap<Long, MatchResult>(0);
        // Uses SQL query wildcard '%' to represent prefix matching.
        final String normalizedQuery = normalizeToDialpadString(query);
        // Number loose query.
        final String numLooseQuery = "%" + query + "%";
        // Prefix need a white space.
        // Prefix query should use normalized query string.
        final String prefixQuery = "% " + normalizedQuery + "%";

        // Query all the contacts with less columns.
        final String quick_match_columns =
                SmartDialDbColumns._ID + ", " +
                SmartDialDbColumns.NUMBER + ", " +
                SmartDialDbColumns.NORMALIZED_NAME + ", " +
                SmartDialDbColumns.SEARCH_DATA_OFFSETS;

        String selectContactsIdSql =
                " (SELECT DISTINCT " + PrefixColumns.CONTACT_ID +
                " FROM " + Tables.PREFIX_TABLE +
                " WHERE " + Tables.PREFIX_TABLE + "." + PrefixColumns.PREFIX +
                " LIKE '" + prefixQuery + "' OR " +
                Tables.PREFIX_TABLE + "." + PrefixColumns.PREFIX +
                "='" + SmartDialPrefixEx.PREFIX_NA + "') ";

        String quickMatchSql = "SELECT " + quick_match_columns
                + " FROM " + Tables.SMARTDIAL_TABLE
                + " WHERE " + SmartDialDbColumns.CONTACT_ID + " IN " + selectContactsIdSql
                /** M: filter number here. */
                + " OR " + SmartDialDbColumns.NUMBER + " LIKE '" + numLooseQuery + "'"
                + " ORDER BY " + SmartDialSortingOrder.SORT_ORDER;

        final Cursor cursor = db.rawQuery(quickMatchSql, new String[] {currentTimeStamp});

        if (null == cursor) {
            return emptyMap;
        }

        int[] matchTypeOrder = dsnm.getMatchTypeOrder();
        // HashMap<MatchType, LinkedHashMap<ID, MatchResult>>
        Map<Integer, LinkedHashMap<Long, MatchResult>> resultMap =
                new HashMap<Integer, LinkedHashMap<Long, MatchResult>>(matchTypeOrder.length);
        for (Integer key : matchTypeOrder) {
            resultMap.put(key, new LinkedHashMap<Long, MatchResult>());
        }

        final int columnId = 0;
        final int columnNumber = 1;
        final int columnNormalizedName = 2;
        final int columnSearchOffset = 3;

        long id;

        final int highestPriorityType = matchTypeOrder[0];
        int highestPriorityTypeCount = 0;
        int matchType = DialerSearchNameMatcher.DS_NOT_MATCH;

        try {
            while (cursor.moveToNext() && highestPriorityTypeCount <= maxEntries) {
                id = cursor.getLong(columnId);

                if (dsnm.doNameAndNumberMatch(
                        query,
                        cursor.getString(columnNormalizedName),
                        cursor.getString(columnSearchOffset),
                        cursor.getString(columnNumber))) {

                    matchType = dsnm.getMatchType();
                    resultMap.get(matchType)
                        .put(id, new MatchResult(
                                    dsnm.getNameMatchOffset(),
                                    dsnm.getNumberMatchOffset(),
                                    matchType,
                                    id));

                    if (matchType == highestPriorityType) {
                        highestPriorityTypeCount++;
                    }
                }
            }
        } finally {
            cursor.close();
        }

        return getLimitedMatchResult(resultMap, matchTypeOrder, maxEntries);
    }

    private LinkedHashMap<Long, MatchResult> getQuickMatchedResultOfOneChar(SQLiteDatabase db,
            String query, String currentTimeStamp, DialerSearchNameMatcher dsnm, int maxEntries) {
        // Uses SQL query wildcard '%' to represent prefix matching.
        String normalizedQuery = normalizeToDialpadString(query);
        String numberlooseQuery = "%" + query + "%";
        String initialLooseQuery = "%" + normalizedQuery + "%";

        // Query all the contacts with less columns.
        final String quick_match_select_column =
                SmartDialDbColumns._ID + ", " +
                SmartDialDbColumns.NUMBER + ", " +
                SmartDialDbColumns.NORMALIZED_NAME + ", " +
                SmartDialDbColumns.SEARCH_DATA_OFFSETS;

        String nameSql = "SELECT " + quick_match_select_column
                + " FROM " + Tables.SMARTDIAL_TABLE
                + " WHERE "
                + (SmartDialDbColumns.INITIALS + " LIKE '" + initialLooseQuery + "'")
                + " ORDER BY " + SmartDialSortingOrder.SORT_ORDER
                + " LIMIT " + maxEntries;
        String nameSql_exclude_limit = " (SELECT " + SmartDialDbColumns._ID
                + " FROM " + Tables.SMARTDIAL_TABLE
                + " WHERE "
                + (SmartDialDbColumns.INITIALS + " LIKE '" + initialLooseQuery + "') ");

        Cursor nameCursor = db.rawQuery(nameSql, new String[] {currentTimeStamp});

        int count = maxEntries;
        if (null != nameCursor) {
            count = count - nameCursor.getCount();
        }
        Cursor numberCursor = null;
        if (count > 0) {
            String numberSql = "SELECT " + SmartDialDbColumns._ID + ", " +
                    SmartDialDbColumns.NUMBER
                    + " FROM " + Tables.SMARTDIAL_TABLE
                    + " WHERE " + SmartDialDbColumns.NUMBER + " LIKE '" + numberlooseQuery + "'"
                    + " AND " + SmartDialDbColumns._ID + " NOT IN " + nameSql_exclude_limit
                    + " ORDER BY " + SmartDialSortingOrder.SORT_ORDER
                    + " LIMIT " + count;
            numberCursor = db.rawQuery(numberSql, new String[] {currentTimeStamp});
        }

        if (null == nameCursor && null == numberCursor) {
            return new LinkedHashMap<Long, MatchResult>(0);
        }

        LinkedHashMap<Long, MatchResult> result = new LinkedHashMap<Long, MatchResult>();

        final int columnId = 0;
        final int columnNumber = 1;
        final int columnNormalizedName = 2;
        final int columnSearchOffset = 3;

        long id;

        try {
            if (null != nameCursor) {
                while (nameCursor.moveToNext()) {
                    id = nameCursor.getLong(columnId);

                    if (dsnm.doNameAndNumberMatch(
                            query,
                            nameCursor.getString(columnNormalizedName),
                            nameCursor.getString(columnSearchOffset),
                            nameCursor.getString(columnNumber))) {

                        result.put(id, new MatchResult(
                                        dsnm.getNameMatchOffset(),
                                        dsnm.getNumberMatchOffset(),
                                        dsnm.getMatchType(),
                                        id));
                    }
                }
            }

            if (numberCursor != null) {
                while (numberCursor.moveToNext()) {
                    id = numberCursor.getLong(columnId);

                    if (dsnm.doNumberMatch(query, numberCursor.getString(columnNumber))) {
                        result.put(id, new MatchResult(
                                        null,
                                        dsnm.getNumberMatchOffset(),
                                        dsnm.getMatchType(),
                                        id));
                    }
                }
            }
        } finally {
            if (null != nameCursor) {
                nameCursor.close();
            }
            if (numberCursor != null) {
                numberCursor.close();
            }
        }

        return result;
    }

    private LinkedHashMap<Long, MatchResult> getLimitedMatchResult(
            Map<Integer, LinkedHashMap<Long, MatchResult>> resultMap,
            int[] matchTypeOrder, int maxEntries) {
        LinkedHashMap<Long, MatchResult> matchResultMap =
                new LinkedHashMap<Long, MatchResult>(maxEntries);
        // Just return maxEntries count items.
        if (resultMap.size() > 0) {
            int needItemCount = maxEntries;
            LinkedHashMap<Long, MatchResult> temp = null;
            for (int key : matchTypeOrder) {
                if (needItemCount <= 0) {
                    break;
                }

                needItemCount = maxEntries - matchResultMap.size();
                temp = resultMap.get(key);

                if (temp.size() > 0) {
                    if (temp.size() <= needItemCount) {
                        matchResultMap.putAll(temp);
                    } else {
                        Set<Entry<Long, MatchResult>> entries = temp.entrySet();
                        for (Entry<Long, MatchResult> entry : entries) {
                            matchResultMap.put(entry.getKey(), entry.getValue());
                            if (--needItemCount <= 0) {
                                break;
                            }
                        }
                    }
                }
            }
        }
        return matchResultMap;
    }

    private void notifyChange() {
        LogUtil.i(TAG, "notifyChange SMART_DIAL_UPDATED_URI...");
        // Notify content observers that smart dial database has been updated.
        Intent intent = new Intent(ACTION_SMART_DIAL_UPDATED);
        intent.setPackage(mContext.getPackageName());
        mContext.sendBroadcast(intent);
    }
    /// @}

    
    private static boolean isNCompatible() {
        return Build.VERSION.SDK_INT >= Build.VERSION_CODES.N;
    }

}
