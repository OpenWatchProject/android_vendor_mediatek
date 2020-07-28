package com.mediatek.vcalendar.utils;

import com.mediatek.vcalendar.component.VCalendar;
import com.mediatek.vcalendar.parameter.TzId;
import com.mediatek.vcalendar.property.Version;
import com.mediatek.vcalendar.valuetype.DateTime;

/**
 * Option Utility class.
 */
public class OptionUtil {
    private static final String TAG = "OptionUtil";
    private static final String OP01 = "OP01";
    private static final String RO_OPERATOR_OPTR = "ro.vendor.operator.optr";

    /**
     * check the current optr.
     * @return true if it is op01 optr.
     */
    public static boolean supportOp01() {
        return OP01.equals(android.os.SystemProperties.get(RO_OPERATOR_OPTR));
    }

    /**
     * Get a string representation of local time zone from a TzId object. It
     * only can be used in DtStart/DtEnd property, and DAlarm/AAlarm for
     * vcalendar1.0.
     * @param tzid a TzId object
     * @param time a time string
     * @return the local time zone, never return null
     */
    public static String getLocalTimezone(TzId tzid, String time) {
        if (supportOp01()) {
            return getLocalTimezoneOp01(tzid, time);
        } else {
            return getLocalTimezoneCommon(tzid, time);
        }
    }

    private static String getLocalTimezoneCommon(TzId tzid, String time) {
        // TZID likes TZID=Asia/Shanghai, here we assume it is always a standard
        // tz id.
        String timezone = null;

        if (tzid == null) {
            if (VCalendar.getVCalendarVersion().contains(Version.VERSION10)
                    && VCalendar.getV10TimeZone() != null) {
                // TZ likes TZ:+08:00--sTz would like +08:00
                timezone = DateTime.getPossibleTimezoneV1(VCalendar.getV10TimeZone());
            } else {
                timezone = DateTime.UTC;
            }
        } else {
            String tzidValue = tzid.getValue();
            timezone = DateTime.getPossibleTimezone(time, tzidValue);
        }

        LogUtil.d(TAG, "getLocalTimezoneCommon(): Local time(from tzid or tz) is " + timezone);
        return timezone;
    }

    private static String getLocalTimezoneOp01(TzId tzid, String time) {
        // TZID likes TZID=Asia/Shanghai, here we assume it is always a standard
        // tz id.
        String tzidValue;

        if (tzid == null) {
            if (VCalendar.getVCalendarVersion().contains(Version.VERSION10)
                    && VCalendar.getV10TimeZone() != null) {
                // TZ likes TZ:+08:00--sTz would like +08:00
                tzidValue = DateTime.getPossibleTimezoneV1(VCalendar.getV10TimeZone());
            } else {
                tzidValue = DateTime.UTC;
            }
        } else {
            tzidValue = tzid.getValue();
        }

        LogUtil.d(TAG, "getLocalTimezoneOp01(): Local time(from tzid or tz) is " + tzidValue);
        return tzidValue;
    }
}
