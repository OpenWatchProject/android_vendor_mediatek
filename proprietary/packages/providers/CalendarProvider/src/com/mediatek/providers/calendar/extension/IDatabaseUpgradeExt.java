package com.mediatek.providers.calendar.extension;

import android.database.sqlite.SQLiteDatabase;

public interface IDatabaseUpgradeExt {
    /**
     * M: MTK gb db version
     */
    int MTK_GB_DB_VERSION = 103;
    /**
     * M: The google gb db version which MTK db based on
     */
    int MTK_GB_DB_VERSION_BASE = 102;
    /**
     * M: MTK ics db version
     */
    int MTK_ICS_DB_VERSION = 309;
    /**
     * M: The google ics db version which MTK db based on
     */
    int MTK_ICS_DB_VERSION_BASE = 308;
    /**
     * M: MTK jb db version
     */
    int MTK_JB_DB_VERSION = 404;
    /**
     * M: The google jb db version which MTK db based on
     */
    int MTK_JB_DB_VERSION_BASE = 403;

    /**
     * M: Downgrade MTK version to the google version which MTK db based on.
     * @param oldVersion the current db version to be upgraded
     * @param db SQLiteDatabase for db ops
     * @return the downgraded version if oldVersion is an MTK version
     */
    int downgradeMTKVersionsIfNeeded(int oldVersion, SQLiteDatabase db);
    /**
     * M: When db version already upgraded to the newest Google version,
     * upgrade it to newest MTK version
     * @param db SQLiteDatabase for db ops
     * @return the newest MTK version number
     */
    int upgradeToMTKJBVersion(SQLiteDatabase db);
}
