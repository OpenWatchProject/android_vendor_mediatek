package com.mediatek.mms.ipmessage;

import android.database.Cursor;

import com.mediatek.mms.callback.IColumnsMapCallback;

public interface IIpColumnsMapExt {

    /**
     * Called in ColumnsMap's construction ColumnsMap().
     * @param maxColumnValue the max value in host
     * @internal
     */
    void onCreate(int maxColumnValue, IColumnsMapCallback callback);

    /**
     * Called in ColumnsMap's construction ColumnsMap(Cursor).
     * @param cursor Cursor
     * @internal
     */
    void onCreate(Cursor cursor, IColumnsMapCallback callback);
}
