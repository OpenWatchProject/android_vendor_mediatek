package com.mediatek.providers.calendar.extension;

public class ExtensionFactory {

    public static ITableExt getCalendarsTableExt(String tableName) {
        return new PCSyncAccountExt(tableName);
    }

    public static IDatabaseUpgradeExt getDatabaseUpgradeExt() {
        return new MTKDatabaseUpgradeExt();
    }
}
