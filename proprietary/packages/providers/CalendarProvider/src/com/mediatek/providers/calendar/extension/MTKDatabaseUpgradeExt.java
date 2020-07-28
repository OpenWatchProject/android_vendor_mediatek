package com.mediatek.providers.calendar.extension;

import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.provider.CalendarContract.Events;

import com.mediatek.providers.calendar.LogUtil;

public class MTKDatabaseUpgradeExt implements IDatabaseUpgradeExt {

    private static final String TAG = "MTKDatabaseUpgradeExt";

    private static final String MTK_BACKUP_TABLE = "mtk_backup_table";
    private static final String EVENTS_TABLE = "Events";

    private boolean mIsUpgradeFromMTKVersion = false;

    @Override
    public int downgradeMTKVersionsIfNeeded(int oldVersion, SQLiteDatabase db) {
        if (oldVersion == IDatabaseUpgradeExt.MTK_GB_DB_VERSION) {
            mIsUpgradeFromMTKVersion = true;
            return downgradeFromMTKGBVersion(db);
        }
        if (oldVersion == IDatabaseUpgradeExt.MTK_ICS_DB_VERSION) {
            mIsUpgradeFromMTKVersion = true;
            return downgradeFromMTKICSVersion(db);
        }
        return oldVersion;
    }

    @Override
    public int upgradeToMTKJBVersion(SQLiteDatabase db) {
        ensureMTKColumns(db);
        restoreMTKColumnsIfNeeded(db);
        return IDatabaseUpgradeExt.MTK_JB_DB_VERSION;
    }

    /**
     * M: downgrade from mtk gb version to the gb version it based on.
     * @param db SQLiteDatabase
     * @return based version number
     */
    private int downgradeFromMTKGBVersion(SQLiteDatabase db) {
        LogUtil.v(TAG, "downgradeFromMTKGBVersion");
        backupMTKColumns(db);
        return IDatabaseUpgradeExt.MTK_GB_DB_VERSION_BASE;
    }

    /**
     * M: downgrade from mtk ics version to the ics version it based on.
     * @param db SQLiteDatabase
     * @return based version number
     */
    private int downgradeFromMTKICSVersion(SQLiteDatabase db) {
        LogUtil.v(TAG, "downgradeFromMTKICSVersion");
        backupMTKColumns(db);
        return IDatabaseUpgradeExt.MTK_ICS_DB_VERSION_BASE;
    }

    /**
     * M: backup MTK Columns to temp table mtk_backup_table
     * @param db SQLiteDatabase
     */
    private void backupMTKColumns(SQLiteDatabase db) {
        /**
         * M: CREATE TABLE mtk_backup_table AS SELECT
         * _id, create_time, modify_time, isLunar, lunarRrule
         * FROM Events;
         */
        final String backupMTKColumnsSql =
                "CREATE TABLE " + MTK_BACKUP_TABLE + " AS SELECT "
                + Events._ID + ","
                + ExtConsts.PCSync.CREATE_TIME + ","
                + ExtConsts.PCSync.MODIFY_TIME + ","
                + ExtConsts.LunarEvent.IS_LUNAR + ","
                + ExtConsts.LunarEvent.LUNAR_RRULE
                + " FROM " + EVENTS_TABLE + ";";
        LogUtil.v(TAG, "backupMTKColumns, sql = " + backupMTKColumnsSql);
        db.execSQL(backupMTKColumnsSql);
    }

    /**
     * M: restore MTK Columns from temp table if need, then drop the temp table
     * @param db SQLiteDatabase
     */
    private void restoreMTKColumnsIfNeeded(SQLiteDatabase db) {
        if (mIsUpgradeFromMTKVersion) {
            final String field01 = ExtConsts.PCSync.CREATE_TIME;
            final String field02 = ExtConsts.PCSync.MODIFY_TIME;
            final String field03 = ExtConsts.LunarEvent.IS_LUNAR;
            final String field04 = ExtConsts.LunarEvent.LUNAR_RRULE;
            /**
             * M: UPDATE Events SET
             * create_time=(SELECT create_time FROM mtk_backup_table WHERE _id=Events._id),
             * modify_time=(SELECT modify_time FROM mtk_backup_table WHERE _id=Events._id),
             * isLunar=(SELECT isLunar FROM mtk_backup_table WHERE _id=Events._id),
             * lunarRrule=(SELECT lunarRrule FROM mtk_backup_table WHERE _id=Events._id);
             */
            final String restoreMTKColumnsSql = "UPDATE " + EVENTS_TABLE + " SET "
                    + field01 + "=(SELECT " + field01 + " FROM " + MTK_BACKUP_TABLE
                    + " WHERE " + field01 + "=" + EVENTS_TABLE + "." + field01 + "),"
                    + field02 + "=(SELECT " + field02 + " FROM " + MTK_BACKUP_TABLE
                    + " WHERE " + field02 + "=" + EVENTS_TABLE + "." + field02 + "),"
                    + field03 + "=(SELECT " + field03 + " FROM " + MTK_BACKUP_TABLE
                    + " WHERE " + field03 + "=" + EVENTS_TABLE + "." + field03 + "),"
                    + field04 + "=(SELECT " + field04 + " FROM " + MTK_BACKUP_TABLE
                    + " WHERE " + field04 + "=" + EVENTS_TABLE + "." + field04 + ");";
            LogUtil.v(TAG, "restoreMTKColumns, sql = " + restoreMTKColumnsSql);
            db.execSQL(restoreMTKColumnsSql);

            final String dropBackupTableSql = "DROP TABLE " + MTK_BACKUP_TABLE + ";";
            LogUtil.v(TAG, "drop backup table, sql = " + dropBackupTableSql);
            db.execSQL(dropBackupTableSql);
        } else {
            LogUtil.d(TAG, "not upgrade from MTK versions, no need to restore");
        }
    }

    /**
     * M: add MTK columns if needed, in case not upgrade from MTK version, it's useful.
     * @param db SQLiteDatabase
     */
    private void ensureMTKColumns(SQLiteDatabase db) {
        LogUtil.v(TAG, "ensure MTK Columns exists");
        Cursor cursor = db.rawQuery("select * from Events where _id=0", null);
        if (null == cursor) {
            LogUtil.e(TAG, "the cursor shouldn't be null");
            return;
        }

        if (cursor.getColumnIndex(ExtConsts.PCSync.CREATE_TIME) < 0) {
            LogUtil.v(TAG, "add column: " + ExtConsts.PCSync.CREATE_TIME);
            /**
             * M: ALTER TABLE Evens ADD COLUMN create_time INTEGER;
             */
            db.execSQL("ALTER TABLE " + EVENTS_TABLE + " ADD COLUMN " + ExtConsts.PCSync.CREATE_TIME + " INTEGER;");
        }
        if (cursor.getColumnIndex(ExtConsts.PCSync.MODIFY_TIME) < 0) {
            LogUtil.v(TAG, "add column: " + ExtConsts.PCSync.MODIFY_TIME);
            /**
             * M: ALTER TABLE Events ADD COLUMN modify_time INTEGER;
             */
            db.execSQL("ALTER TABLE " + EVENTS_TABLE + " ADD COLUMN " + ExtConsts.PCSync.MODIFY_TIME + " INTEGER;");
        }
        if (cursor.getColumnIndex(ExtConsts.LunarEvent.IS_LUNAR) < 0) {
            LogUtil.v(TAG, "add column: " + ExtConsts.LunarEvent.IS_LUNAR);
            /**
             * M: ALTER TABLE Events ADD COLUMN isLunar INTEGER NOT NULL DEFAULT 0;
             */
            db.execSQL("ALTER TABLE " + EVENTS_TABLE + " ADD COLUMN " + ExtConsts.LunarEvent.IS_LUNAR
                    + " INTEGER NOT NULL DEFAULT 0;");
        }
        if (cursor.getColumnIndex(ExtConsts.LunarEvent.LUNAR_RRULE) < 0) {
            LogUtil.v(TAG, "add column: " + ExtConsts.LunarEvent.LUNAR_RRULE);
            /**
             * M: ALTER TABLE Events ADD COLUMN lunarRrule TEXT;
             */
            db.execSQL("ALTER TABLE " + EVENTS_TABLE + " ADD COLUMN " + ExtConsts.LunarEvent.LUNAR_RRULE + " TEXT;");
        }

        cursor.close();
    }
}
