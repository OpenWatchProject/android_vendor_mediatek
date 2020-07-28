package com.mediatek.calendar.extension;

import android.database.Cursor;

public interface IAccountExt {

    Cursor accountQuery(String[] projection);
}
