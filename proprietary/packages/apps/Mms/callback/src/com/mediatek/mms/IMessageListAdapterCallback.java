package com.mediatek.mms.callback;

import android.database.Cursor;

public interface IMessageListAdapterCallback {
    Cursor getCursorCallback();
    IColumnsMapCallback getColumnsMap();
}
