package com.debug.loggerui.taglog.db;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.CursorWindowAllocationException;
import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteDatabase.CursorFactory;
import android.database.sqlite.SQLiteException;
import android.database.sqlite.SQLiteOpenHelper;

import com.debug.loggerui.MyApplication;
import com.debug.loggerui.utils.Utils;

import java.util.ArrayList;
import java.util.List;



/**
 * This class is used to operate issue info in database.
 * @author Bo.Shang
 */
public class MySQLiteHelper {
    private static final String TAG = Utils.TAG + "/DatabaseManagerHelper";
    private static final String DB_NAME = "taglog_v0.db";
    public static final int DATABASE_VERSION = 1;

    public static final String TAGLOG_STATE_INIT = "INIT";
    public static final String TAGLOG_STATE_DOTAG = "DOTAG";
    public static final String TAGLOG_STATE_DONE = "DONE";
    public static final String TAGLOG_STATE_IGNORE = "IGNORE";

    public static final String FILEINFO_STATE_PREPARE = "PREPARE";
    public static final String FILEINFO_STATE_WAITING = "WAITING";
    public static final String FILEINFO_STATE_DOING = "DOING";
    public static final String FILEINFO_STATE_DONE = "DONE";

    /**
     * Taglog table part.
     */
    public static final String TAGLOG_TABLE = "taglog_table";
    public static final String COLUMN_ID = "id";
    public static final String COLUMN_TARGET_FOLDER = "target_folder";
    public static final String COLUMN_STATE = "state";
    public static final String COLUMN_FILE_LIST = "filelist";
    public static final String COLUMN_DB_PATH = "path";
    public static final String COLUMN_DB_FILENAME = "db_filename";
    public static final String COLUMN_ZZ_FILENAME = "zz_filename";

    public static final String COLUMN_IS_NEED_ZIP = "isneedzip";
    public static final String COLUMN_IS_NEED_ALLLOG = "isneedalllog";
    public static final String COLUMN_NEED_LOGTYPE = "needlogtype";
    public static final String COLUMN_REASON = "Reason";
    public static final String COLUMN_FROM_WHERE = "fromWhere";
    public static final String COLUMN_ZZINTERNAL_TIME = "exceptionTime";

    public static final String COLUMN_MORE_0 = "more0";
    public static final String COLUMN_MORE_1 = "more1";
    public static final String COLUMN_MORE_2 = "more2";

    private static final String SQL_CREATE_TAGLOG_TABLE = "create table if not exists "
            + TAGLOG_TABLE
            + "("
            + COLUMN_ID
            + " integer primary key, "
            + COLUMN_TARGET_FOLDER
            + " varchar, "
            + COLUMN_STATE
            + " varchar, "
            + COLUMN_FILE_LIST
            + " varchar, "
            + COLUMN_DB_PATH
            + " varchar, "

            + COLUMN_DB_FILENAME
            + " varchar, "
            + COLUMN_ZZ_FILENAME
            + " varchar, "
            + COLUMN_IS_NEED_ZIP
            + " varchar, "
            + COLUMN_IS_NEED_ALLLOG
            + " varchar, "
            + COLUMN_NEED_LOGTYPE
            + " integer, "
            + COLUMN_REASON
            + " varchar, "
            + COLUMN_FROM_WHERE
            + " varchar, "
            + COLUMN_ZZINTERNAL_TIME
            + " varchar, "

            + COLUMN_MORE_0
            + " varchar, "
            + COLUMN_MORE_1
            + " varchar, "
            + COLUMN_MORE_2
            + " varchar)";

    public static final String FILEINFO_TABLE = "loginfo_table";
//    public static final String COLUMN_FILEINFOR_ID = "id";
    public static final String COLUMN_LOGTYPE = "logtype";
    public static final String COLUMN_ORIGINAL_PATH = "originalpath";
    public static final String COLUMN_SOURCE_PATH = "sourcepath";
//    public static final String COLUMN_TARGET_FOLDER = "targetfolder";
    public static final String COLUMN_TARGET_FILE = "targetfile";
    public static final String COLUMN_TREATMENT = "treatment";
//    public static final String COLUMN_STATE = "state";
    public static final String COLUMN_FILE_COUNT = "filecount";
    public static final String COLUMN_FILE_PROGRESS = "fileprogress";
    public static final String COLUMN_LEVEL = "level";
    public static final String COLUMN_NEED_TAG = "needtag";

    private static final String SQL_CREATE_TABLE_LOGINFO = "create table if not exists "
            + FILEINFO_TABLE
            + "("
            + COLUMN_ID
            + " integer primary key, "
            + COLUMN_LOGTYPE
            + " integer, "
            + COLUMN_ORIGINAL_PATH
            + " varchar, "
            + COLUMN_SOURCE_PATH
            + " varchar, "
            + COLUMN_TARGET_FOLDER
            + " varchar, "
            + COLUMN_TARGET_FILE
            + " varchar, "
            + COLUMN_TREATMENT
            + " varchar, "
            + COLUMN_STATE
            + " varchar, "
            + COLUMN_FILE_COUNT
            + " integer, "
            + COLUMN_FILE_PROGRESS
            + " integer, "
            + COLUMN_LEVEL
            + " integer, "
            + COLUMN_NEED_TAG
            + " varchar, "
            + COLUMN_MORE_0
            + " varchar, "
            + COLUMN_MORE_1
            + " varchar, "
            + COLUMN_MORE_2
            + " varchar)";

    private static MySQLiteHelper sInstance = null;
    private static SQLiteOpenHelper sSqliteHelper = null;

    /**
     * Get database handle instance.
     * @return handle.
     */
    public static MySQLiteHelper getInstance() {
        if (sInstance == null) {
            synchronized (MySQLiteHelper.class) {
                if (sInstance == null) {
                    sInstance = new MySQLiteHelper(MyApplication.getInstance()
                            .getApplicationContext());
                }
            }
        }
        return sInstance;
    }

    /**
     * Private construction.
     * @param context to operate db.
     */
    private MySQLiteHelper(Context context) {
        sSqliteHelper = new MySQLiteOpenHelper(context,
                DB_NAME, null, DATABASE_VERSION);
    }

    /**
     * Insert taglogtables into db.
     * @param taglogtables List<TaglogTable>.
     * @return result
     */
    public synchronized long insertTaglogInfo(List<TaglogTable> taglogtables) {
        if (null == taglogtables) {
            Utils.loge(TAG, "-->insertTaglogInfo(), taglogtable is null");
            return -1;
        }

        ContentValues contentValues = new ContentValues();
        long result = 0;
        try {
            SQLiteDatabase db = sSqliteHelper.getWritableDatabase();
            db.beginTransaction();
            for (TaglogTable taglogtable : taglogtables) {
                contentValues.put(COLUMN_TARGET_FOLDER,
                        taglogtable.getTargetFolder());
                contentValues.put(COLUMN_STATE, taglogtable.getState());
                contentValues.put(COLUMN_FILE_LIST, taglogtable.getFileList());
                contentValues.put(COLUMN_DB_PATH, taglogtable.getDBPath());
                contentValues.put(COLUMN_DB_FILENAME,
                        taglogtable.getDBFileName());

                contentValues.put(COLUMN_ZZ_FILENAME,
                        taglogtable.getmDBZZFileName());
                contentValues.put(COLUMN_IS_NEED_ZIP, taglogtable.isNeedZip());
                contentValues.put(COLUMN_IS_NEED_ALLLOG,
                        taglogtable.isNeedAllLogs());
                contentValues.put(COLUMN_NEED_LOGTYPE,
                        taglogtable.getNeedLogType());
                contentValues.put(COLUMN_REASON, taglogtable.getReason());
                contentValues
                        .put(COLUMN_FROM_WHERE, taglogtable.getFromWhere());
                contentValues.put(COLUMN_ZZINTERNAL_TIME,
                        taglogtable.getZzInternalTime());
                result = db.insert(TAGLOG_TABLE, null, contentValues);
            }
            db.setTransactionSuccessful();
            db.endTransaction();
        } catch (SQLiteException exp) {
            Utils.logw(TAG, "insert fail, for SQLiteException happened!");
        } catch (IllegalStateException e) {
            Utils.logw(TAG,
                    "Fail to insert! exception for  IllegalStateException ");
        }
        if (result <= 0) {
            Utils.loge(TAG, "Fail to insertTaglogInfo record");
        }

        Utils.logi(TAG, "<--insertTaglogInfo() done, result = " + result);
        return result;
    }

    //FILE INFOR
    /**
     * @param fileinfoList List<FileInfoTable>
     * @return String
     */
    public synchronized String insertFileInfo(List<FileInfoTable> fileinfoList) {
        Utils.logi(TAG, "-->insertFileInfo(), fileinfoList length="
                + fileinfoList.size());
        if (fileinfoList == null || fileinfoList.size() < 1) {
            return "";
        }
        String fileIndexList = "";
        ContentValues contentValues = new ContentValues();
        try {
            SQLiteDatabase db = sSqliteHelper.getWritableDatabase();
            db.beginTransaction();
            for (FileInfoTable fileInfo : fileinfoList) {
                contentValues.put(COLUMN_LOGTYPE, fileInfo.getLogType());
                contentValues.put(COLUMN_ORIGINAL_PATH, fileInfo.getOriginalPath());
                contentValues.put(COLUMN_SOURCE_PATH, fileInfo.getSourcePath());
                contentValues.put(COLUMN_TARGET_FOLDER, fileInfo.getTargetFolder());
                contentValues.put(COLUMN_TARGET_FILE, fileInfo.getTargetFile());
                contentValues.put(COLUMN_TREATMENT, fileInfo.getTreatment());
                contentValues.put(COLUMN_STATE, fileInfo.getState());
                contentValues.put(COLUMN_FILE_COUNT, fileInfo.getFileCount());
                contentValues.put(COLUMN_FILE_PROGRESS, fileInfo.getFileProgress());
                contentValues.put(COLUMN_LEVEL, fileInfo.getFileLevel());
                contentValues.put(COLUMN_NEED_TAG, fileInfo.isNeedTag());

                long result = 0;
                result = db.insert(FILEINFO_TABLE, null, contentValues);
                if (result <= 0) {
                    Utils.loge(TAG, "Fail to instert fileinfo record: " + fileInfo.toString());
                } else {
                    fileIndexList += String.valueOf(result) + ",";
                }
            }
            db.setTransactionSuccessful();
            db.endTransaction();
        } catch (SQLiteException exp) {
            Utils.logw(TAG, "insert fail, for SQLiteException happened!");
        } catch (IllegalStateException e) {
            Utils.logw(TAG,
                    "Fail to init! exception for  IllegalStateException ");
        }
        if (fileIndexList.length() >= 1) {
            fileIndexList = fileIndexList.substring(0, fileIndexList.length() - 1);
        }
        Utils.logi(TAG, "<--insertFileInfo() done. fileIndexlist = " + fileIndexList.toString());
        return fileIndexList;
    }

    /**
     * @param selectionStr String
     * @return List<TaglogTable>
     */
    public List<TaglogTable> queryTaglogTable(String selectionStr) {
        List<TaglogTable> result = new ArrayList<TaglogTable>();
        Cursor cursor = null;
        try {
            SQLiteDatabase db = sSqliteHelper.getReadableDatabase();
            cursor = db.query(TAGLOG_TABLE, null, selectionStr, null, null, null, null);
        } catch (SQLiteException e) {
            Utils.logw(TAG,
                     "Fail to query queryFileIds! exception for  SQLiteDiskIOException ");
        } catch (IllegalStateException e) {
            Utils.logw(TAG,
                    "Fail to query queryFileIds! exception for  IllegalStateException ");
        }
        if (cursor == null) {
            Utils.logw(TAG,
                    "Fail to queryTaglogTable! selectionStr = " + selectionStr);
            return result;
        }
        int count = cursor.getCount();
        Utils.logd(TAG, "Query TAGLOG_TABLE selectionStr = " + selectionStr
                + ", number = " + count);
        while (cursor.moveToNext()) {
            int taglogId = cursor.getInt(cursor.getColumnIndex(COLUMN_ID));
            String targetFolder = cursor.getString(cursor
                    .getColumnIndex(COLUMN_TARGET_FOLDER));
            String state = cursor.getString(cursor
                    .getColumnIndex(COLUMN_STATE));
            String filelist = cursor.getString(cursor
                    .getColumnIndex(COLUMN_FILE_LIST));
            String dbpath = cursor.getString(cursor
                    .getColumnIndex(COLUMN_DB_PATH));
            String dbfilename = cursor.getString(cursor
                    .getColumnIndex(COLUMN_DB_FILENAME));
            String zzfilename = cursor.getString(cursor
                    .getColumnIndex(COLUMN_ZZ_FILENAME));

            String isneedzip = cursor.getString(cursor
                    .getColumnIndex(COLUMN_IS_NEED_ZIP));
            String isneedalllog = cursor.getString(cursor
                    .getColumnIndex(COLUMN_IS_NEED_ALLLOG));
            int needlogtype = cursor.getInt(cursor
                    .getColumnIndex(COLUMN_NEED_LOGTYPE));
            String reason = cursor.getString(cursor
                    .getColumnIndex(COLUMN_REASON));
            String fromwhere = cursor.getString(cursor
                    .getColumnIndex(COLUMN_FROM_WHERE));
            String expTime = cursor.getString(cursor
                    .getColumnIndex(COLUMN_ZZINTERNAL_TIME));
            result.add(new TaglogTable(taglogId, targetFolder, state, filelist, dbpath,
                    dbfilename, zzfilename, isneedzip, isneedalllog,
                    needlogtype, reason, fromwhere, expTime));
        }
        cursor.close();
        return result;
    }

    /**
     * @param selectionStr String
     * @return List<FileInfoTable>
     */
    public List<FileInfoTable> queryFileInfoTable(String selectionStr) {
        List<FileInfoTable> result = new ArrayList<FileInfoTable>();
        Cursor cursor = null;
        try {
            SQLiteDatabase db = sSqliteHelper.getReadableDatabase();
            cursor = db.query(FILEINFO_TABLE, null, selectionStr, null,
                              null, null, null);
        } catch (SQLiteException e) {
            Utils.logw(TAG,
                     "Fail to query fileinfoTable! exception for  SQLiteDiskIOException ");
        } catch (IllegalStateException e) {
            Utils.logw(TAG,
                    "Fail to query fileinfoTable! exception for  IllegalStateException ");
        }
        if (cursor == null) {
            Utils.logw(TAG,
                    "Fail to query fileinfoTable! selectionStr = " + selectionStr);
            return result;
        }
        try {
            int count = cursor.getCount();
            Utils.logd(TAG, "Query FILEINFO_TABLE selectionStr = "
                    + selectionStr + ", number = " + count);
        } catch (CursorWindowAllocationException cwae) {
            Utils.logw(TAG,
                    "Fail to query fileinfoTable! selectionStr = " + selectionStr);
            return result;
        }
        while (cursor.moveToNext()) {
            long fileId = cursor.getInt(cursor.getColumnIndex(COLUMN_ID));
            int logtype = cursor.getInt(cursor.getColumnIndex(COLUMN_LOGTYPE));
            String originalPath = cursor.getString(cursor
                    .getColumnIndex(COLUMN_ORIGINAL_PATH));
            String sourcePath = cursor.getString(cursor
                    .getColumnIndex(COLUMN_SOURCE_PATH));
            String targetfolder = cursor.getString(cursor
                    .getColumnIndex(COLUMN_TARGET_FOLDER));
            String targetfile = cursor.getString(cursor
                    .getColumnIndex(COLUMN_TARGET_FILE));
            String treatment = cursor.getString(cursor
                    .getColumnIndex(COLUMN_TREATMENT));
            String state = cursor.getString(cursor
                    .getColumnIndex(COLUMN_STATE));

            int filecount = cursor.getInt(cursor
                    .getColumnIndex(COLUMN_FILE_COUNT));
            int fileprogress = cursor.getInt(cursor
                    .getColumnIndex(COLUMN_FILE_PROGRESS));
            int level = cursor.getInt(cursor
                    .getColumnIndex(COLUMN_LEVEL));
            String tag = cursor.getString(cursor
                    .getColumnIndex(COLUMN_NEED_TAG));
            result.add(new FileInfoTable(fileId, logtype, originalPath, sourcePath, targetfolder,
                    targetfile, treatment, state, filecount, fileprogress, level, tag));
        }
        cursor.close();
        return result;
    }

    /**
     * @param selectionStr String
     * @return result List<String>
     */
    public List<String> queryFileIds(String selectionStr) {
        List<String> result = new ArrayList<String>();
        Cursor cursor = null;
        try {
            SQLiteDatabase db = sSqliteHelper.getReadableDatabase();
            cursor = db.query(FILEINFO_TABLE, null, selectionStr, null,
                    null, null, null);
        } catch (SQLiteException e) {
            Utils.logw(TAG,
                 "Fail to query queryFileIds! exception for  SQLiteDiskIOException ");
        } catch (IllegalStateException e) {
            Utils.logw(TAG,
                    "Fail to query queryFileIds! exception for  IllegalStateException ");
        }

        if (cursor == null) {
            Utils.logw(TAG,
                    "Fail to query queryFileIds! selectionStr = " + selectionStr);
            return result;
        }
        int count = cursor.getCount();
        Utils.logd(TAG, "Query FILEINFO_TABLE selectionStr = "
                   + selectionStr + ", number = " + count);
        while (cursor.moveToNext()) {
            long fileId = cursor.getInt(cursor.getColumnIndex(COLUMN_ID));
            result.add(String.valueOf(fileId));
        }
        cursor.close();
        return result;
    }
    /**
     * @param table String
     * @param keyClumn String
     * @param keyId long
     * @param contentValues ContentValues
     * @return boolean
     */
    public synchronized boolean updateRecordsByTable(String table, String keyClumn,
                                long keyId, ContentValues contentValues) {
        if (contentValues == null) {
            Utils.loge(TAG, "-->update db table: "
                        + table + " fail, Id=" + keyId + "for contentvalue = null");
            return false;
        }
        Utils.loge(TAG, "updateRecordsByTable: " + table + " Id=" + keyId);
        int result = 0;
        try {
            SQLiteDatabase db = sSqliteHelper.getWritableDatabase();
            result = db.update(table, contentValues,
                    keyClumn + "=" + keyId, null);
        } catch (SQLiteException exp) {
            Utils.logw(TAG, "update fail, for database or disk is full");
        } catch (IllegalStateException e) {
            Utils.logw(TAG,
                    "Fail to update! exception for  IllegalStateException ");
        }
        if (result != 1) {
            Utils.loge(TAG, "-->update db table: " + table + " fail, Id=" + keyId);
        }
        return result == 1;
    }

    /**
     * @param taglogtable TaglogTable
     * @return boolean
     */
    public boolean updateTagLogTable(TaglogTable taglogtable) {
        ContentValues contentValues = new ContentValues();
        contentValues.put(COLUMN_TARGET_FOLDER, taglogtable.getTargetFolder());
        contentValues.put(COLUMN_STATE, taglogtable.getState());
        contentValues.put(COLUMN_FILE_LIST, taglogtable.getFileList());
        contentValues.put(COLUMN_DB_PATH, taglogtable.getDBPath());
        contentValues.put(COLUMN_DB_FILENAME, taglogtable.getDBFileName());

        contentValues.put(COLUMN_ZZ_FILENAME, taglogtable.getmDBZZFileName());
        contentValues.put(COLUMN_IS_NEED_ZIP, taglogtable.isNeedZip());
        contentValues.put(COLUMN_IS_NEED_ALLLOG, taglogtable.isNeedAllLogs());
        contentValues.put(COLUMN_NEED_LOGTYPE, taglogtable.getNeedLogType());
        contentValues.put(COLUMN_REASON, taglogtable.getReason());
        contentValues.put(COLUMN_FROM_WHERE, taglogtable.getFromWhere());
        contentValues.put(COLUMN_ZZINTERNAL_TIME, taglogtable.getZzInternalTime());

        return updateRecordsByTable(MySQLiteHelper.TAGLOG_TABLE,
                MySQLiteHelper.COLUMN_ID, taglogtable.getTagLogId(), contentValues);
    }
    /**
     * @param table String
     * @param keyClumn String
     * @param keyIds String[]
     * @param contentValues ContentValues
     * @return boolean
     */
    public synchronized boolean updateRecordsByTable(String table, String keyClumn,
            String[] keyIds, ContentValues contentValues) {
        if (contentValues == null) {
            Utils.loge(TAG, "-->update db table: " + table + " fail, Id=" + keyIds.toString()
                    + "for contentvalue =null");
            return false;
        }
        Utils.loge(TAG, "updateRecordsByTable: " + table + " Id=" + keyIds.toString());
        int result = 0;
        try {
            SQLiteDatabase db = sSqliteHelper.getWritableDatabase();
            result = db.update(table, contentValues, keyClumn + "=?", keyIds);
        } catch (SQLiteException exp) {
            Utils.logw(TAG, "update fail, for database or disk is full");
        } catch (IllegalStateException e) {
            Utils.logw(TAG,
                    "Fail to update! exception for  IllegalStateException ");
        }
        if (result != 1) {
            Utils.logw(TAG, "-->update db table: " + table + " fail, Id=" + keyIds.toString());
        }
        return result == 1;
    }

    /**
     * @param table String
     * @param issueIds long[]
     * @return boolean
     */
    public boolean deleteRecordsByTable(String table, long[] issueIds) {
        StringBuilder idStrBuilder = new StringBuilder();
        for (long id : issueIds) {
            if (idStrBuilder.length() != 0) {
                idStrBuilder.append(", ");
            }
            idStrBuilder.append(id);
        }
        String idString = idStrBuilder.toString();
        int result = 0;
        try {
            SQLiteDatabase db = sSqliteHelper.getWritableDatabase();
            result = db.delete(table, COLUMN_ID + " in ("
                    + idString + ")", null);
        } catch (SQLiteException exp) {
            Utils.logw(TAG, "delete fail, for database or disk is full");
        } catch (IllegalStateException e) {
            Utils.logw(TAG,
                    "Fail to delete! exception for  IllegalStateException ");
        }
        Utils.logd(TAG, "DB table: " + table + " delete issue id string=" + idString
                   + ", result = " + result);
        return result == issueIds.length;
    }

    /**
     * Class to create taglog db.
     */
    class MySQLiteOpenHelper extends SQLiteOpenHelper {
        public MySQLiteOpenHelper(Context context, String name,
                CursorFactory factory, int version) {
            super(context, name, factory, version);
        }

        @Override
        public void onCreate(SQLiteDatabase db) {
            try {
                db.execSQL(SQL_CREATE_TAGLOG_TABLE);
                db.execSQL(SQL_CREATE_TABLE_LOGINFO);
            } catch (SQLException e) {
                Utils.logw(TAG, "db.execSQL error!");
            }
        }

        @Override
        public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
            Utils.logi(TAG, "onUpgrade db table");
                        String dropTaglogTableSqlStr = "drop table if exists "
                    + TAGLOG_TABLE;
            String dropFileInfoTableSqlStr = "drop table if exists "
                    + FILEINFO_TABLE;
            try {
                db.execSQL(dropTaglogTableSqlStr);
                db.execSQL(dropFileInfoTableSqlStr);
            } catch (SQLException e) {
                Utils.logw(TAG, "db.execSQL error!");
            }
            onCreate(db);
        }
    }
}
