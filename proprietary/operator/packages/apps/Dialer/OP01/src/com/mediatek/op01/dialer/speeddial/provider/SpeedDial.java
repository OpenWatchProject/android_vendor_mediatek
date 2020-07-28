package com.mediatek.op01.dialer.speeddial.provider;

import android.net.Uri;

import android.provider.BaseColumns;

class SpeedDial {

    public static final String AUTHORITY = "speed_dial";
    public static final Uri CONTENT_URI = Uri.parse("content://" + AUTHORITY);


    public static class Numbers implements BaseColumns {
        public static final Uri CONTENT_URI = Uri.parse("content://speed_dial/numbers");
        public static final String CONTENT_TYPE = "vnd.android.cursor.dir/numbers";
        public static final String NUMBER = "number";
        public static final String TYPE = "type";
    }
}
