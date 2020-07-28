package com.mediatek.galleryportable;

public class Log {

    private static final int LOG_LEVEL = android.util.Log.DEBUG;
    private static final int INVALID_RTN = -1;
    private static final boolean IS_ENG;
    private static final boolean FORCE_ENABLE;

    static {
        String buildType = SystemPropertyUtils.get("ro.build.type");
        IS_ENG = buildType != null ? buildType.equals("eng") : false;
        FORCE_ENABLE = false || "1".equals(SystemPropertyUtils.get("vendor.gallery.log.enable"));
    }

    private static boolean isLogEnable(String tag) {
        return FORCE_ENABLE || IS_ENG || android.util.Log.isLoggable(tag, LOG_LEVEL);
    }

    public static int v(String tag, String msg) {
        if (isLogEnable(tag)) {
            return android.util.Log.v(tag, msg);
        }
        return INVALID_RTN;
    }

    public static int v(String tag, String msg, Throwable tr) {
        if (isLogEnable(tag)) {
            return android.util.Log.v(tag, msg, tr);
        }
        return INVALID_RTN;
    }

    public static int d(String tag, String msg) {
        if (isLogEnable(tag)) {
            return android.util.Log.d(tag, msg);
        }
        return INVALID_RTN;
    }

    public static int d(String tag, String msg, Throwable tr) {
        if (isLogEnable(tag)) {
            return android.util.Log.d(tag, msg, tr);
        }
        return INVALID_RTN;
    }

    public static int i(String tag, String msg) {
        if (isLogEnable(tag)) {
            return android.util.Log.i(tag, msg);
        }
        return INVALID_RTN;
    }

    public static int i(String tag, String msg, Throwable tr) {
        if (isLogEnable(tag)) {
            return android.util.Log.i(tag, msg, tr);
        }
        return INVALID_RTN;
    }

    public static int w(String tag, String msg) {
        if (isLogEnable(tag)) {
            return android.util.Log.w(tag, msg);
        }
        return INVALID_RTN;
    }

    public static int w(String tag, String msg, Throwable tr) {
        if (isLogEnable(tag)) {
            return android.util.Log.w(tag, msg, tr);
        }
        return INVALID_RTN;
    }

    public static int w(String tag, Throwable tr) {
        if (isLogEnable(tag)) {
            return android.util.Log.w(tag, tr);
        }
        return INVALID_RTN;
    }

    public static int e(String tag, String msg) {
        if (isLogEnable(tag)) {
            return android.util.Log.e(tag, msg);
        }
        return INVALID_RTN;
    }

    public static int e(String tag, String msg, Throwable tr) {
        if (isLogEnable(tag)) {
            return android.util.Log.e(tag, msg, tr);
        }
        return INVALID_RTN;
    }
}
