package com.mediatek.providers.calendar.extension;

import android.database.sqlite.SQLiteDatabase;
import android.provider.CalendarContract;

public class PCSyncAccountExt implements ITableExt {

    private static final String PC_SYNC_ACCOUNT_COLOR = "-9215145";
    private static final String PC_SYNC_ACCOUNT_NAME = "PC Sync";
    private static final String PC_SYNC_ACCOUNT_DISPLAY_NAME = "PC Sync";
    private static final String PC_SYNC_ACCOUNT_OWNER_ACCOUNT = "PC Sync";
    private static final String PC_SYNC_ACCOUNT_ACCESS_LEVEL = "700";
    private static final String PC_SYNC_ACCOUNT_SYNC_EVENTS = "1";

    private String mTableName;

    public PCSyncAccountExt(String tableName) {
        mTableName = tableName;
    }

    @Override
    public void tableExtension(SQLiteDatabase db) {
        createPCSyncAccount(db);
    }

    private void createPCSyncAccount(SQLiteDatabase db) {
        db.execSQL("INSERT INTO " + mTableName + " (" +
                CalendarContract.Calendars.ACCOUNT_NAME + ", " +
                CalendarContract.Calendars.ACCOUNT_TYPE + ", " +
                CalendarContract.Calendars.CALENDAR_DISPLAY_NAME + ", " +
                CalendarContract.Calendars.CALENDAR_COLOR + ", " +
                CalendarContract.Calendars.CALENDAR_ACCESS_LEVEL + ", " +
                CalendarContract.Calendars.SYNC_EVENTS + ", " +
                CalendarContract.Calendars.OWNER_ACCOUNT + ") VALUES (" +
                "'" + PC_SYNC_ACCOUNT_NAME + "'," +
                "'" + CalendarContract.ACCOUNT_TYPE_LOCAL + "'," +
                "'" + PC_SYNC_ACCOUNT_DISPLAY_NAME + "'," +
                "'" + PC_SYNC_ACCOUNT_COLOR + "'," +
                "'" + PC_SYNC_ACCOUNT_ACCESS_LEVEL + "'," +
                "'" + PC_SYNC_ACCOUNT_SYNC_EVENTS + "'," +
                "'" + PC_SYNC_ACCOUNT_OWNER_ACCOUNT + "');");
    }
}
