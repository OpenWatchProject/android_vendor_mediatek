package com.mediatek.mms.ipmessage;

import android.database.Cursor;

import com.mediatek.mms.callback.IColumnsMapCallback;

public class DefaultIpColumnsMapExt implements IIpColumnsMapExt {

    @Override
    public void onCreate(int maxColumnValue, IColumnsMapCallback callback) {
        // do nothing
    }

    @Override
    public void onCreate(Cursor cursor, IColumnsMapCallback callback) {
        //do nothing
    }
}
