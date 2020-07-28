package com.mediatek.calendar.extension;

/**
 * M: The extension consts defines here
 */
public final class ExtConsts {

    /**
     * M: the lunar calendar related consts defined here
     */
    public final class LunarEvent {
        /**
         * M: the db field name for lunar, is this a lunar event
         */
        public static final String IS_LUNAR = "isLunar";
        /**
         * M: the db field name for lunar, lunar repeat rule
         */
        public static final String LUNAR_RRULE = "lunarRrule";
    }

    /**
     * M: the PC Sync related consts
     */
    public final class PCSync {
        /**
         * M: The field needed by PC Sync Tool
         */
        public static final String CREATE_TIME = "createTime";
        /**
         * M: The field needed by PC Sync Tool
         */
        public static final String MODIFY_TIME = "modifyTime";
        /**
         * M: the  PC Sync account create table sql
         */
        public static final String CREATE_TABLE = "," + CREATE_TIME + " INTEGER,"
                + MODIFY_TIME + " INTEGER";
    }
}
