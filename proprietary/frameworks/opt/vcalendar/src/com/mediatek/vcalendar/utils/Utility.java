package com.mediatek.vcalendar.utils;

import android.text.format.Time;
import android.util.TimeFormatException;

import com.mediatek.vcalendar.parameter.TzId;

/**
 * Utility class, mainly about option function, such as time/time zone and encode related logic.
 */
public class Utility {
    private static final String TAG = "Utility";

    /**
     * Get time in milliseconds under time zone identified by tzid. It only can
     * be used in DtStart/DtEnd property.
     * @param tzid time zone id
     * @param time a time string
     * @return time in milliseconds
     */
    public static long getTimeInMillis(TzId tzid, String time) {
        String localTimezone = getLocalTimezone(tzid, time);
        Time t = new Time(localTimezone);
        try {
            t.parse(time);
        } catch (TimeFormatException tfe) {
            LogUtil.e(TAG, "getTimeInMillis(): wrong time format, time: "
                    + time);
        }

        LogUtil.d(TAG, "getTimeInMillis(): time=" + t);
        return t.toMillis(false);
    }

    /**
     * Get a string representation of local time zone from a TzId object. It
     * only can be used in DtStart/DtEnd property, and DAlarm/AAlarm for
     * vcalendar1.0.
     * @param tzid a TzId object
     * @return the local time zone, never return null
     */
    public static String getLocalTimezone(TzId tzid, String time) {
        return OptionUtil.getLocalTimezone(tzid, time);
    }

    /**
     * should support Quoted-Printable encode when compose the vcs content.
     * @return false if common load, should not do QP encode;
     * true if op01 load, should do QP encode.
     */
    public static boolean needQpEncode() {
        return OptionUtil.supportOp01();
    }

    /**
     * should support TZID parameter in DTSTART/DTEND/... when compose the vcs content.
     * @return false if common load, no need set TZID in DTSTART/DTEND/....;
     * true if op01 load, need set it.
     */
    public static boolean needTzIdParameter() {
        return OptionUtil.supportOp01();
    }
}
