package com.mediatek.gba;

import android.os.Build;
import android.os.SystemProperties;
import android.text.TextUtils;
import android.util.Log;

/**
 * implementation for GbaConstant.
 *
 * @hide
 */
public class GbaConstant {
    public static final int GBA_NONE  = 0;
    public static final int GBA_ME    = 1;
    public static final int GBA_U     = 2;

    public static final int GBA_CARD_UNKNOWN  = 0;
    public static final int GBA_CARD_SIM    = 1;
    public static final int GBA_CARD_USIM   = 2;
    public static final int GBA_CARD_ISIM   = 3;

    public static final String GBA_KS_NAF      = "naf_ks";
    public static final String GBA_KS_EXT_NAF  = "nas_ks_ext";

    public static final String US_ASCII = "US-ASCII";
    public static final String ISO_8859_1 = "ISO-8859-1";

    private static final String PROP_FORCE_DEBUG_KEY = "persist.vendor.log.tel_dbg";
    public static final boolean SENLOG = TextUtils.equals(Build.TYPE, "user");
    public static final boolean DBGLOG =
            !SENLOG || (SystemProperties.getInt(PROP_FORCE_DEBUG_KEY, 0) == 1);

    public static void SSLog(String tag, String message) {
        if (DBGLOG) {
            Log.d(tag, message);
        }
    }

    public static void PiiLog(String tag, String message) {
        if (!SENLOG) {
            Log.d(tag, message);
        }
    }
}

