/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */
package com.mediatek.vcalendar.valuetype;

import android.text.format.Time;
import android.util.TimeFormatException;

import com.mediatek.vcalendar.VCalendarException;
import com.mediatek.vcalendar.component.Component;
import com.mediatek.vcalendar.component.VCalendar;
import com.mediatek.vcalendar.component.VTimezone;
import com.mediatek.vcalendar.property.Property;
import com.mediatek.vcalendar.utils.LogUtil;

import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.GregorianCalendar;
import java.util.List;
import java.util.Locale;
import java.util.TimeZone;

/**
 * Create RFC5545 DATE and DATETIME values
 */
public final class DateTime {
    private static final String TAG = "DateTime";

    public static final long DT_MIN = 946656000000L;// 2000-01-01 00:00:00
    public static final String UTC = "UTC";
    public static final int THOUSAND = 1000;

    // for date string, such as 20070809T010000Z
    private static final int YEAR_START_INDEX = 0;
    private static final int YEAR_END_INDEX = 3;
    private static final int MONTH_START_INDEX = 4;
    private static final int MONTH_END_INDEX = 5;
    private static final int DAY_START_INDEX = 6;
    private static final int DAY_END_INDEX = 7;

    // for utc offset, such as +050000
    private static final int HOUR_START_INDEX = 1;
    private static final int HOUR_END_INDEX = 3;
    private static final int MINUTE_START_INDEX = 3;
    private static final int MINUTE_END_INDEX = 5;
    private static final int SECOND_START_INDEX = 5;
    private static final int SECOND_END_INDEX = 7;

    private DateTime() {
    }

    /**
     * get the possible time zone id with the TZID string and start date info.
     * 
     * @param dateTimeString
     *            the start date/time format string
     * @param tzId
     *            the srcTimezone TZ id
     */
    public static String getPossibleTimezone(String dateTimeString, String tzId) {
        LogUtil.i(TAG, "getPossibleTimezone(): dateTimeString=" + dateTimeString
                + ";tzId=" + tzId);

        int offset = Integer.MIN_VALUE;
        if (tzId != null && dateTimeString != null) {
            offset = getOffsetMillis(dateTimeString, tzId);
        }
        if(offset==Integer.MIN_VALUE) {
            //return a default one.
            return UTC;
        }

        Time time = new Time();
        String[] standardTzIds = TimeZone.getAvailableIDs(offset);
        String targetTzid = null;
        targetTzid = standardTzIds.length > 0 ? standardTzIds[0] : UTC;
        LogUtil.d(TAG, "getPossibleTimezone(): offset =" + offset + "; TZID = " + targetTzid);

        //do adjust
        for(String id: standardTzIds) {
            try {
                time.switchTimezone(id);
                time.parse(dateTimeString);
                time.normalize(false);
            } catch (TimeFormatException tfe) {
                LogUtil.e(TAG, "getPossibleTimezone(): parse time error, just stop it. time: " + time);
                break;
            }
            int offset2 = TimeZone.getTimeZone(id).getOffset(time.toMillis(false));
            LogUtil.d(TAG, "getPossibleTimezone(): offset2 = " + offset2);
            if (offset2 == offset) {
                targetTzid = id;
                LogUtil.d(TAG, "getPossibleTimezone(): after adjust TZID = " + targetTzid);
                break;
            }
        }

        return targetTzid;
    }

    /**
     * Transfer a absolute milliseconds to a UTC date time string
     * 
     * @param dateTimeMillis
     *            the absolute millisecond
     * @return the UTC String
     */
    public static String getUtcTimeString(long dateTimeMillis) {
        TimeZone tz = TimeZone.getTimeZone(UTC);
        Calendar gc = GregorianCalendar.getInstance(tz);
        gc.setTimeInMillis(dateTimeMillis);

        return String.format(Locale.US, "%04d", gc.get(Calendar.YEAR)) 
                + String.format(Locale.US, "%02d", gc.get(Calendar.MONTH) + 1)
                + String.format(Locale.US, "%02dT", gc.get(Calendar.DAY_OF_MONTH))
                + String.format(Locale.US, "%02d", gc.get(Calendar.HOUR_OF_DAY)) 
                + String.format(Locale.US, "%02d", gc.get(Calendar.MINUTE))
                + String.format(Locale.US, "%02dZ", gc.get(Calendar.SECOND));
    }

    /**
     * Return the millisecond of the 00:00:00 of the given day.
     * 
     * @param date
     *            the date string
     * @return the millisecond
     */
    public static long getUtcDateMillis(String date) {
        int year = Integer.valueOf(
                date.substring(YEAR_START_INDEX, YEAR_END_INDEX + 1))
                .intValue();
        int month = Integer.valueOf(
                date.substring(MONTH_START_INDEX, MONTH_END_INDEX + 1))
                .intValue();
        int day = Integer.valueOf(
                date.substring(DAY_START_INDEX, DAY_END_INDEX + 1)).intValue();

        TimeZone tz = TimeZone.getTimeZone(UTC);
        Calendar gc = GregorianCalendar.getInstance(tz);
        gc.set(year, month - 1, day, 0, 0, 0);

        return gc.getTimeInMillis() / THOUSAND * THOUSAND;
    }

    /**
     * return the date UTC String of the given milliseconds , ignored the time
     * 
     * @param dateTimemillis
     *            the dateTimle milliseconds
     * @return UTC date String
     */
    public static String getUtcDateString(long dateTimemillis) {
        SimpleDateFormat sdf = new SimpleDateFormat("yyyyMMdd", Locale.US);
        return sdf.format(new Date(dateTimemillis));
    }

    private static int getOffsetMillis(String dtStart, String tzId) {
        Property tzIdProperty;
        for (VTimezone tz : VCalendar.TIMEZONE_LIST) {
            tzIdProperty = tz.getFirstProperty(Property.TZID);
            // tzIdProperty may be null
            LogUtil.d(TAG, "getOffsetMillis(): the current vtimezone: "
            + ((tzIdProperty != null) ? tzIdProperty.getValue() : "null"));

            if ((tzIdProperty != null)
                    && (tzId.equalsIgnoreCase(tzIdProperty.getValue()))) {
                LogUtil.i(TAG, "getOffsetMillis(): has found the vtimezone: "
                    + tzId);
                String offsetString = getOffsetString(tz, dtStart);
                return getUtcOffsetMillis(offsetString);
            }
        }
        return Integer.MIN_VALUE;
    }

    private static String getOffsetString(VTimezone tz, String dtStart) {
        if (tz == null) {
            return null;
        }
        List<Component> componentList = tz.getComponents();
        Property tzStart;
        String tempDtStart = dtStart;
        Component targetComponent = null;
        Component parentComponent = null;
        for (Component comp : componentList) {
            parentComponent = comp;
            tzStart = comp.getFirstProperty(Property.DTSTART);
            if (tzStart == null) {
                LogUtil.e(TAG, "getOffsetString(): The given tz's sub-component do not contains dtstart property");
                continue;
            }
            // TODO: How to find which one is the most near one, how to consider
            // repeat TZ
            if (tempDtStart.compareToIgnoreCase(tzStart.getValue()) >= 0
                    && tempDtStart.compareToIgnoreCase(dtStart) <= 0) {
                tempDtStart = tzStart.getValue();
                targetComponent = comp;
            }
        }
        LogUtil.d(TAG, "getOffsetString(): dtstart=" + dtStart
                + "; tempDtStart = " + tempDtStart);

        if (targetComponent == null) {
            LogUtil.e(TAG, "getOffsetString(): The given dtStart are not contained in any Daylight or Standard Component.");
            targetComponent = parentComponent;
            //return null;
        }

        if(targetComponent != null) {
        Property offsetTo = targetComponent
                .getFirstProperty(Property.TZOFFSETTO);
        return offsetTo == null ? null : offsetTo.getValue();
    }
      return null;
    }

    private static int getUtcOffsetMillis(String utcOffsetString) {
        if (utcOffsetString == null
                || utcOffsetString.length() < MINUTE_END_INDEX) {
          LogUtil.w(TAG, "Invalid UTC offset ["
                    + utcOffsetString + "] - must be of the form: (+/-)HHMM[SS]");
            return Integer.MIN_VALUE;
        }

        final boolean negative = utcOffsetString.charAt(0) == '-';

        if (!negative && utcOffsetString.charAt(0) != '+') {
            throw new IllegalArgumentException("UTC offset value must be signed");
        }

        int offset = 0;
        offset += Integer.parseInt(utcOffsetString
                .substring(HOUR_START_INDEX, HOUR_END_INDEX))
                * DDuration.MINUTES_IN_HOUR * DDuration.MILLIS_IN_MIN;
        offset += Integer.parseInt(utcOffsetString
                .substring(MINUTE_START_INDEX, MINUTE_END_INDEX))
                * DDuration.MILLIS_IN_MIN;
        try {
            offset += Integer.parseInt(utcOffsetString
                    .substring(SECOND_START_INDEX, SECOND_END_INDEX))
                    * DDuration.MILLIS_IN_SECOND;
        } catch (IndexOutOfBoundsException e) {
            // seconds not supplied..
            LogUtil.i(TAG, "getUtcOffsetMillis(): Seconds not specified: "
                    + e.getMessage());
        }
        if (negative) {
            offset = -offset;
        }
        return offset;
    }

    /**
     * Get first possible time zone according offset. It is only can appeared in
     * vcalendar1.0's vcs file. If can not find one, return "UTC".
     * 
     * @param tzOffset
     *            time zone offset, like +08:00 in TZ property(e.g. "TZ:+08:00")
     * @return first possible time zone according offset, like "Asia/Shanghai".
     */
    public static String getPossibleTimezoneV1(String tzOffset) {
        int offset = 0;
        offset = getTzOffsetMillis(tzOffset);
        String[] availableTzIDs = TimeZone.getAvailableIDs(offset);
        String targetTzId = availableTzIDs.length > 0 ? availableTzIDs[0] : UTC;

        return targetTzId;
    }

    /**
     * @param tzOffset time zone offset, like +08:00
     * @return 
     */
    private static int getTzOffsetMillis(String tzOffset) {
        String offsetString = tzOffset;
        final boolean negative = offsetString.charAt(0) == '-';
        int offset = 0;
        // remove the sign.
        offsetString = offsetString.replace("+", "");
        offsetString = offsetString.replace("-", "");

        // split field use ":".
        String[] offsetArray = offsetString.split(":");
        try {
            if (offsetArray.length >= 1) {
                offset += Integer.parseInt(offsetArray[0]) * DDuration.MINUTES_IN_HOUR
                        * DDuration.MILLIS_IN_MIN;
            }
            if (offsetArray.length >= 2) {
                offset += Integer.parseInt(offsetArray[1]) * DDuration.MILLIS_IN_MIN;
            }
        } catch (NumberFormatException e) {
            LogUtil.w(TAG, "NumberFormatException in getTzOffsetMillis(): "
                    + e.getMessage());
        }

        if (negative) {
            offset = -offset;
        }

        return offset;
    }
}
