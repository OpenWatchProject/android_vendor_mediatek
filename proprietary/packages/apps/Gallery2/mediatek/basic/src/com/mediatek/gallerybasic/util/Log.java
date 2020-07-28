package com.mediatek.gallerybasic.util;

import com.mediatek.galleryportable.SystemPropertyUtils;

/**
* Log wrapper.
*/
public class Log {
    private static final String TAG = "MtkGallery2/Log";
    private static final int CUST_LOG_LEVEL_V = 0;
    private static final int CUST_LOG_LEVEL_D = 1;
    private static final int CUST_LOG_LEVEL_I = 2;
    private static final int CUST_LOG_LEVEL_W = 3;
    private static final int CUST_LOG_LEVEL_E = 4;

    private static final String BUILD_TYPE = SystemPropertyUtils.get("ro.build.type");
    private static final boolean IS_ENG = "eng".equalsIgnoreCase(BUILD_TYPE);
    private static final int LOG_LEVEL_IN_PROPERTY =
            SystemPropertyUtils.getInt("vendor.debug.gallery.loglevel", CUST_LOG_LEVEL_I);
    private static final int CUST_LOG_LEVEL =
            LOG_LEVEL_IN_PROPERTY >= CUST_LOG_LEVEL_V && LOG_LEVEL_IN_PROPERTY <= CUST_LOG_LEVEL_E
                    ? LOG_LEVEL_IN_PROPERTY : CUST_LOG_LEVEL_I;

    static {
        android.util.Log.d(TAG, "BUILD_TYPE: " + BUILD_TYPE + ", IS_ENG: " + IS_ENG
                + ", CUST_LOG_LEVEL: " + CUST_LOG_LEVEL);
    }

    /**
    * Log.v.
    * @param tag log tag
    * @param msg log message
    */
    public static void v(String tag, String msg) {
        if (tag == null) {
            return;
        }
        if (IS_ENG || enableCustLog(CUST_LOG_LEVEL_V)) {
            android.util.Log.v(tag, msg);
        }
    }

    /**
    * Log.v.
    * @param tag log tag
    * @param msg log message
    * @param tr throwable
    */
    public static void v(String tag, String msg, Throwable tr) {
        if (tag == null) {
            return;
        }
        if (IS_ENG || enableCustLog(CUST_LOG_LEVEL_V)) {
            android.util.Log.v(tag, msg, tr);
        }
    }

    /**
    * Log.d.
    * @param tag log tag
    * @param msg log message
    */
    public static void d(String tag, String msg) {
        if (tag == null) {
            return;
        }
        if (IS_ENG || enableCustLog(CUST_LOG_LEVEL_D)) {
            android.util.Log.d(tag, msg);
        }
    }

    /**
    * Log.d.
    * @param tag log tag
    * @param msg log message
    * @param tr throwable
    */
    public static void d(String tag, String msg, Throwable tr) {
        if (tag == null) {
            return;
        }
        if (IS_ENG || enableCustLog(CUST_LOG_LEVEL_D)) {
            android.util.Log.d(tag, msg, tr);
        }
    }

    /**
    * Log.i.
    * @param tag log tag
    * @param msg log message
    */
    public static void i(String tag, String msg) {
        if (tag == null) {
            return;
        }
        if (IS_ENG || enableCustLog(CUST_LOG_LEVEL_I)) {
            android.util.Log.i(tag, msg);
        }
    }

    /**
    * Log.i.
    * @param tag log tag
    * @param msg log message
    * @param tr throwable
    */
    public static void i(String tag, String msg, Throwable tr) {
        if (tag == null) {
            return;
        }
        if (IS_ENG || enableCustLog(CUST_LOG_LEVEL_I)) {
            android.util.Log.i(tag, msg, tr);
        }
    }

    /**
    * Log.w.
    * @param tag log tag
    * @param msg log message
    */
    public static void w(String tag, String msg) {
        if (tag == null) {
            return;
        }
        if (IS_ENG || enableCustLog(CUST_LOG_LEVEL_W)) {
            android.util.Log.w(tag, msg);
        }
    }

    /**
    * Log.w.
    * @param tag log tag
    * @param msg log message
    * @param tr throwable
    */
    public static void w(String tag, String msg, Throwable tr) {
        if (tag == null) {
            return;
        }
        if (IS_ENG || enableCustLog(CUST_LOG_LEVEL_W)) {
            android.util.Log.w(tag, msg, tr);
        }
    }

    /**
    * Log.e.
    * @param tag log tag
    * @param msg log message
    */
    public static void e(String tag, String msg) {
        if (tag == null) {
            return;
        }
        if (IS_ENG || enableCustLog(CUST_LOG_LEVEL_E)) {
            android.util.Log.e(tag, msg);
        }
    }

    /**
    * Log.e.
    * @param tag log tag
    * @param msg log message
    * @param tr throwable
    */
    public static void e(String tag, String msg, Throwable tr) {
        if (tag == null) {
            return;
        }
        if (IS_ENG || enableCustLog(CUST_LOG_LEVEL_E)) {
            android.util.Log.e(tag, msg, tr);
        }
    }

    private static boolean enableCustLog(int custLogLevel) {
        if (CUST_LOG_LEVEL >= 0 && CUST_LOG_LEVEL <= 4) {
            return custLogLevel >= CUST_LOG_LEVEL;
        }
        return false;
    }
}
