/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */
package com.mediatek.vcalendar.component;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.os.Build;
import android.provider.CalendarContract.Events;
import android.text.format.DateFormat;
import android.text.format.DateUtils;

import com.mediatek.vcalendar.ComponentPreviewInfo;
import com.mediatek.vcalendar.SingleComponentContentValues;
import com.mediatek.vcalendar.SingleComponentCursorInfo;
import com.mediatek.vcalendar.VCalendarException;

import com.mediatek.vcalendar.parameter.Encoding;
import com.mediatek.vcalendar.parameter.Parameter;
import com.mediatek.vcalendar.parameter.Role;
import com.mediatek.vcalendar.parameter.Value;
import com.mediatek.vcalendar.property.AAlarm;
import com.mediatek.vcalendar.property.Attendee;
import com.mediatek.vcalendar.property.DAlarm;
import com.mediatek.vcalendar.property.Description;
import com.mediatek.vcalendar.property.DtEnd;
import com.mediatek.vcalendar.property.DtStamp;
import com.mediatek.vcalendar.property.DtStart;
import com.mediatek.vcalendar.property.Duration;
import com.mediatek.vcalendar.property.Location;
import com.mediatek.vcalendar.property.Property;
import com.mediatek.vcalendar.property.RRule;
import com.mediatek.vcalendar.property.Status;
import com.mediatek.vcalendar.property.Summary;
import com.mediatek.vcalendar.property.Uid;
import com.mediatek.vcalendar.property.Version;
import com.mediatek.vcalendar.utils.CursorUtil;
import com.mediatek.vcalendar.utils.LogUtil;
import com.mediatek.vcalendar.utils.StringUtil;
import com.mediatek.vcalendar.utils.Utility;
import com.mediatek.vcalendar.valuetype.CalAddress;
import com.mediatek.vcalendar.valuetype.Charset;
import com.mediatek.vcalendar.valuetype.DDuration;
import com.mediatek.vcalendar.valuetype.DateTime;

import java.util.LinkedList;
import java.util.List;

/**
 * "VEVENT" component of iCalendar.
 */
public class VEvent extends Component {
    private static final String TAG = "VEvent";

    public static final String VEVENT_BEGIN = "BEGIN:VEVENT";
    public static final String VEVENT_END = "END:VEVENT";

    // Events table column names for ics or lower.
    public static final String CREATE_TIME_COLUMN_NAME = "createTime";
    public static final String MODIFY_TIME_COLUMN_NAME = "modifyTime";

    public VEvent() {
        super(VEVENT, null);
        LogUtil.d(TAG, "Constructor: VEvent component created!");
    }

    @Override
    public void compose(SingleComponentCursorInfo eventInfo)
            throws VCalendarException {
        if (eventInfo == null) {
            throw new VCalendarException("compose(): SingleComponentCursorInfo is null in component: " + mName);
        }

        // parse the Events table cursor firstly.
        compose(eventInfo.cursor);
        eventInfo.cursor.close();

        // parse the alarms table cursor row by row.
        Cursor alarmsCursor = eventInfo.remindersCursor;
        if (alarmsCursor != null) {
            alarmsCursor.moveToFirst();
            for (int i = 0; i < alarmsCursor.getCount(); i++) {
                VAlarm alarm = new VAlarm(this);
                Cursor cursor = CursorUtil.copyCurrentRow(alarmsCursor);
                alarm.compose(cursor);
                cursor.close();
                addChild(alarm);
                alarmsCursor.moveToNext();
            }
            alarmsCursor.close();
        }

        // parse the attendee table cursor row by row
        Cursor attendeesCursor = eventInfo.attendeesCursor;
        if (attendeesCursor != null) {
            attendeesCursor.moveToFirst();
            for (int i = 0; i < attendeesCursor.getCount(); i++) {
                Attendee attendee = new Attendee(null);
                Cursor cursor = CursorUtil.copyCurrentRow(attendeesCursor);
                attendee.compose(cursor, this);
                cursor.close();
                addProperty(attendee);
                attendeesCursor.moveToNext();
            }
            attendeesCursor.close();
        }
    }

    @Override
    protected void compose(Cursor cursor) throws VCalendarException {
        LogUtil.i(TAG, "compose(): compose a VEVENT.");
        super.compose(cursor);

        // use QP encode the text,and char set
        Parameter encodePara = new Parameter(Parameter.ENCODING,
                Encoding.QUOTED_PRINTABLE);
        Parameter charsetPara = new Parameter(Parameter.CHARSET, Charset.UTF8);

        String valueString;
        // add Uid property, equals event id
        valueString = cursor.getString(cursor.getColumnIndex(Events._ID));
        if (!StringUtil.isNullOrEmpty(valueString)) {
            addProperty(new Uid(valueString));
        }

        // add summary property
        valueString = cursor.getString(cursor.getColumnIndex(Events.TITLE));
        if (!StringUtil.isNullOrEmpty(valueString)) {
            Property summary = new Summary(valueString);
            if (Utility.needQpEncode()) {
                summary.addParameter(encodePara);
            }
            summary.addParameter(charsetPara);
            addProperty(summary);
        }

        // add dtstamp property, JB & ICS both named "createTime" or
        // "modifyTime",
        // keep if here to notify the column name are different in future
        if (Build.VERSION.SDK_INT <= 15) {
            // ICS or lower
            LogUtil.i(TAG, "compose(): OS Version is <=15.");
        } else {
            // JB
            LogUtil.i(TAG, "compose(): OS Version is > 15.");
            if (cursor.getColumnIndex(CREATE_TIME_COLUMN_NAME) < 0) {
                throw new VCalendarException("Cannot create DtStamp, the needed \"createTime\"  does not exist in DB.");
            }
        }

        valueString = cursor.getString(cursor
                .getColumnIndex(MODIFY_TIME_COLUMN_NAME));
        if (valueString == null) {
            valueString = cursor.getString(cursor
                    .getColumnIndex(CREATE_TIME_COLUMN_NAME));
        }
        if (!StringUtil.isNullOrEmpty(valueString)) {
            addProperty(new DtStamp(DateTime.getUtcTimeString(Long
                    .parseLong(valueString))));
        }

        // add status property
        int status = cursor.getInt(cursor.getColumnIndex(Events.STATUS));
        valueString = Status.getStatusString(status);
        if (valueString != null) {
            addProperty(new Status(valueString));
        }

        // add organizer property
        valueString = cursor.getString(cursor.getColumnIndex(Events.ORGANIZER));
        if (valueString != null) {
            valueString = CalAddress.getUserCalAddress(valueString);
            if (valueString != null) {
                addProperty(new Property(Property.ORGANIZER, valueString));
            }
        }

        // add location property
        valueString = cursor.getString(cursor.getColumnIndex(Events.EVENT_LOCATION));
        if (!StringUtil.isNullOrEmpty(valueString)) {
            Property location = new Location(valueString);
            if (Utility.needQpEncode()) {
                location.addParameter(encodePara);
            }
            location.addParameter(charsetPara);
            addProperty(location);
        }
        // add Description property
        valueString = cursor.getString(cursor.getColumnIndex(Events.DESCRIPTION));
        if (!StringUtil.isNullOrEmpty(valueString)) {
            Property description = new Description(valueString.replaceAll("\r\n", "\n"));
            if (Utility.needQpEncode()) {
                description.addParameter(encodePara);
            }
            description.addParameter(charsetPara);
            addProperty(description);
        }

        long millis = -1;
        boolean isAllDay = cursor.getInt(cursor.getColumnIndex(Events.ALL_DAY)) 
                == 1 ? true : false;
        // add allday info
        String allday = cursor.getString(cursor.getColumnIndex(Events.ALL_DAY));
        if (!StringUtil.isNullOrEmpty(allday)) {
            Property alldayProperty = new Property(Property.X_ALLDAY, allday);
            addProperty(alldayProperty);
        }

        // Here, keep event's time zone info.
        valueString = cursor.getString(cursor
                .getColumnIndex(Events.EVENT_TIMEZONE));
        String timezone = DateTime.UTC;
        if (!StringUtil.isNullOrEmpty(valueString)) {
            timezone = valueString;
            addProperty(new Property(Property.X_TIMEZONE, valueString));
        }

        // Add DtStart Property
        int columnIndex = cursor.getColumnIndex(Events.DTSTART);
        if (cursor.isNull(columnIndex)) {
            throw new VCalendarException("Cannot create DtStart, the needed \"DtStart\" does not exist in DB.");
        } else {
            millis = cursor.getLong(columnIndex);
            valueString = DateTime.getUtcTimeString(millis);
            DtStart dtStart = new DtStart(valueString);
            if (!isAllDay && Utility.needTzIdParameter()) {
                Parameter tzid = new Parameter(Parameter.TZID, timezone);
                dtStart.addParameter(tzid);
            }
            addProperty(dtStart);
        }

        // Add Duration property
        valueString = cursor.getString(cursor.getColumnIndex(Events.DURATION));
        if (!StringUtil.isNullOrEmpty(valueString)) {
            addProperty(new Duration(valueString));
        }

        // Add DtEnd property, it is conflicted to Duration
        if (!mPropsMap.containsKey(Property.DURATION)) {
            columnIndex = cursor.getColumnIndex(Events.DTEND);
            if (!cursor.isNull(columnIndex)) {
                millis = cursor.getLong(columnIndex);
                valueString = DateTime.getUtcTimeString(millis);
                DtEnd dtEnd = new DtEnd(valueString);
                if (!isAllDay && Utility.needTzIdParameter()) {
                    Parameter tzid = new Parameter(Parameter.TZID, timezone);
                    dtEnd.addParameter(tzid);
                }
                addProperty(dtEnd);
            }
        }

        // add Recurrence rule
        valueString = cursor.getString(cursor.getColumnIndex(Events.RRULE));
        if (!StringUtil.isNullOrEmpty(valueString)) {
            addProperty(new RRule(valueString));
        }
    }

    @Override
    public void fillPreviewInfo(ComponentPreviewInfo info)
            throws VCalendarException {
        LogUtil.i(TAG, "fillPreviewInfo()");

        if (info == null) {
            throw new VCalendarException("fillPreviewInfo(): ComponentPreviewInfo is null.");
        }

        info.eventSummary = getTitle();
        info.eventOrganizer = getOrganizer();
        info.eventStartTime = getDtStart();
        info.eventDuration = getTime(mContext);
    }

    @Override
    public void writeInfoToContentValues(SingleComponentContentValues sccv)
            throws VCalendarException {
        LogUtil.i(TAG, "writeInfoToContentValues()");

        if (sccv == null) {
            throw new VCalendarException("writeInfoToContentValues(): SingleComponentContentValues is null.");
        }

        /*
         * Write the component type info, it will be used to write database.
         */
        sccv.componentType = Component.VEVENT;
        /*
         * If there is an organizer in the event, we should push it into
         * database.
         */
        String organizer = getOrganizer();
        if (organizer != null) {
            sccv.contentValues.put(Events.ORGANIZER, organizer);
        }

        writeInfoToContentValues(sccv.contentValues);
        writeAlarmsContentValues(sccv.alarmValuesList);
        writeAttendeesContentValues(sccv.attendeeValuesList);
    }

    @Override
    protected void writeInfoToContentValues(final ContentValues cv)
            throws VCalendarException {
        LogUtil.d(TAG, "writeInfoToContentValues()");
        super.writeInfoToContentValues(cv);

        /*
         * Judge if the VEVENT has the Required Properties. DTSTAMP UID DTSTART
         * are required , but we do not store the DTSTAMP into DB. Uid is useful
         * for future duplicated event judgment.
         */
        if (!mPropsMap.containsKey(Property.UID)) {
            LogUtil.w(TAG, "VEVENT did not contains the required UID!!");
        }
        if (!mPropsMap.containsKey(Property.DTSTART)) {
            throw new VCalendarException("VEVENT did not contains the required DTSTART");
        }

        // iCalendar1.0 allowed DTEND & DURATION existing in a same vCalendar.
        if (!VCalendar.getVCalendarVersion().contains(Version.VERSION10)) {
            if (mPropsMap.containsKey(Property.DTEND)
                    && mPropsMap.containsKey(Property.DURATION)) {
                LogUtil.e(TAG, "writeInfoToContentValues(): DTEND DURATION cannot exist at the same VEvent");
                throw new VCalendarException("DTEND, DURATION cannot exist at the same VEvent");
            }
        }

        for (String propertyName : getPropertyNames()) {
            LogUtil.d(TAG, "writeInfoToContentValues(): propertyName = "
                    + propertyName);
            // prevent from multiple calls on same function
            List<Property> pros = getProperties(propertyName);
            LogUtil.d(TAG, "writeInfoToContentValues(): " + propertyName
                    + "'s count = " + pros.size());
            for (Property property : pros) {
                property.writeInfoToContentValues(cv);
            }
        }

        if (isAllDayEvent()) {
            cv.put(Events.ALL_DAY, 1);
        }

        if (mPropsMap.containsKey(Property.DTSTART)
                && !mPropsMap.containsKey(Property.DTEND)
                && !mPropsMap.containsKey(Property.DURATION)) {
            // without DTEND, parse depend on the DTSTART value type
            // DATE_TIME : DTEND = NEXT DAY 00:00:00
            // DATE : ALL DAY , DTEND = DTSTART + ONEDAY, and also the next day
            // 00:00:00 UTC
            DtStart dtStart = (DtStart) getFirstProperty(Property.DTSTART);
            if (dtStart == null) {
                throw new VCalendarException("DTSTART time is needed!");
            }
            long millis = DateTime.getUtcDateMillis(dtStart.getValue());
            cv.put(Events.DTEND, millis + DDuration.MINUTES_IN_DAY
                    * DDuration.MILLIS_IN_MIN);

            LogUtil.d(TAG, "writeInfoToContentValues(): DTSTART value: "
                    + dtStart);
        }

        /*
         * Let sub-components write their necessary information, only VALARM
         * sub-components supported at present.
         */
        for (Component component : getComponents()) {
            component.writeInfoToContentValues(cv);
        }

        // make sure the event's content values has time zone info.
        if (mPropsMap.containsKey(Property.X_TIMEZONE)) {
            Property tz = getFirstProperty(Property.X_TIMEZONE);
            // if tz==null or tz.values is null push UTC
            if (tz == null || StringUtil.isNullOrEmpty(tz.getValue())) {
                cv.put(Events.EVENT_TIMEZONE, DateTime.UTC);
            } else {
                cv.put(Events.EVENT_TIMEZONE, tz.getValue());
            }
        } else {
            if (!cv.containsKey(Events.EVENT_TIMEZONE)) {
                // To check, use UTC time zone or system default time zone?
                cv.put(Events.EVENT_TIMEZONE, DateTime.UTC);
            }
        }
        LogUtil.d(TAG, "writeInfoToContentValues(): event's EVENT_TIMEZONE:"
                        + cv.getAsString(Events.EVENT_TIMEZONE));

        // make sure: not have both DTEND and DURATION in an event
        if (cv.containsKey(Events.DURATION)) {
            LogUtil.d(TAG, "writeInfoToContentValues(): Remove DTEND when event has DURATION:"
                            + cv.getAsString(Events.DURATION));
            cv.remove(Events.DTEND);
        }

        // make sure : not have DTEND but DURATION for recur events
        if (cv.containsKey(Events.RRULE) && cv.containsKey(Events.DTEND)) {
            cv.remove(Events.DTEND);
            if (!cv.containsKey(Events.DURATION)) {
                String duration = DDuration.getDurationString(
                        (getDtEnd() - getDtStart()) / DDuration.MILLIS_IN_MIN);
                cv.put(Events.DURATION, duration);
            }
        }
    }

    private void writeAlarmsContentValues(final LinkedList<ContentValues> cvList)
            throws VCalendarException {
        if (cvList == null) {
            throw new VCalendarException("writeAlarmsContentValues(): ContentValues list is null.");
        }

        // for version 1.0
        if (VCalendar.getVCalendarVersion().contains(Version.VERSION10)) {
            long eventStartMillis = getDtStart();
            LogUtil.d(TAG, "writeAlarmsContentValues(): version 1.0 ");
            // prevent from multiple calls on same function
            List<Property> aPros = getProperties(Property.AALARM);
            for (Property aAlarm : aPros) {
                ((AAlarm) aAlarm).writeInfoToContentValues(cvList, eventStartMillis);
            }

            String dAlarmValue;
            // prevent from multiple calls on same function
            List<Property> dPros = getProperties(Property.DALARM);
            for (Property dAlarm : dPros) {
                dAlarmValue = dAlarm.getValue();
                for (Property aAlarm : aPros) {
                    if (aAlarm.getValue().equalsIgnoreCase(dAlarmValue)) {
                        break;
                    }
                    ((DAlarm) dAlarm).writeInfoToContentValues(cvList, eventStartMillis);
                }
            }
            return;
        }

        for (Component component : getComponents()) {
            component.writeInfoToContentValues(cvList);
        }
    }

    private void writeAttendeesContentValues(
            final LinkedList<ContentValues> cvList) throws VCalendarException {
        if (cvList == null) {
            throw new VCalendarException("writeAttendeesContentValues(): ContentValues list is null.");
        }

        List<Property> attendees = getProperties(Property.ATTENDEE);
        for (Property property : attendees) {
            ((Attendee) property).writeInfoToContentValues(cvList);
        }
    }

    /**
     * Return Event Summary for preview.
     * 
     * @return the summary
     */
    public String getTitle() {
        // Return the Summary property for preview
        Summary summary = (Summary) getFirstProperty(Property.SUMMARY);
        return summary == null ? null : summary.getValue();
    }

    /**
     * Return Event Organizer for preview.
     * 
     * @return the organizer
     */
    public String getOrganizer() {
        LogUtil.d(TAG, "getOrganizer(): sVersion = " + VCalendar.getVCalendarVersion());
        Property organizer = getFirstProperty(Property.ORGANIZER);
        if (organizer != null) {
            return CalAddress.getUserMail(organizer.getValue());
        }

        List<Property> attendeesList = getProperties(Property.ATTENDEE);
        if (attendeesList.isEmpty()) {
            LogUtil.d(TAG, "getOrganizer(): no attendee property.");
            return null;
        }

        for (Property property : attendeesList) {
            Parameter role = property.getFirstParameter(Parameter.ROLE);
            // "ORGANIZER" is a illegal Role value, but supported by old version
            if (role != null && (Role.CHAIR.equalsIgnoreCase(role.getValue())
                    || "ORGANIZER".equals(role.getValue()))) {
                return CalAddress.getUserMail(property.getValue());
            }
        }

        return null;
    }

    /**
     * Return the Event begin time and end time.
     * 
     * @return the startTime ~ endTime String
     * @throws VCalendarException
     */
    public String getTime(Context context) throws VCalendarException {
        int flags;
        if (isAllDayEvent()) {
            flags = DateUtils.FORMAT_UTC | DateUtils.FORMAT_SHOW_WEEKDAY
                    | DateUtils.FORMAT_SHOW_DATE;
        } else {
            flags = DateUtils.FORMAT_SHOW_TIME | DateUtils.FORMAT_SHOW_DATE;
            if (DateFormat.is24HourFormat(context)) {
                flags |= DateUtils.FORMAT_24HOUR;
            }
        }

        return DateUtils.formatDateRange(context, getDtStart(), getDtEnd(),
                flags);
    }

    public long getDtEnd() {
        long time = -1;

        DtEnd dtend = (DtEnd) getFirstProperty(Property.DTEND);
        if (dtend != null) {
            try {
                time = dtend.getValueMillis();
            } catch (VCalendarException e) {
                LogUtil.e(TAG, "getDtEnd(): get end time failed", e);
            }

            return time;
        }

        DtStart dtStart = (DtStart) getFirstProperty(Property.DTSTART);
        if (dtStart == null) {
            return time;
        }
        Duration duration = (Duration) getFirstProperty(Property.DURATION);
        if (duration != null) {
            LogUtil.i(TAG, "getDtEnd(): Can not get DtEnd, return value based on the duration.");
            try {
                time = dtStart.getValueMillis();
            } catch (VCalendarException e) {
                LogUtil.e(TAG, "getDtEnd(): get duration failed", e);
            }

            return time + DDuration.getDurationMillis(duration.getValue());
        }

        // without DTEND & DURATION, parse as a all day event
        // DATE : ALL DAY , DTEND = DTSTART + one day
        LogUtil.i(TAG, "getDtEnd(): Can not get DtEnd & Duration, return value based on the dtstart.");
        long millis = DateTime.getUtcDateMillis(dtStart.getValue());
        return millis + DDuration.MINUTES_IN_DAY * DDuration.MILLIS_IN_MIN;
    }

    public long getDtStart() {
        long time = -1;
        // getFirstProperty() may return null
        DtStart dtStart = (DtStart) getFirstProperty(Property.DTSTART);
        if (dtStart != null) {
            try {
                time = dtStart.getValueMillis();
            } catch (VCalendarException e) {
                LogUtil.e(TAG, "getDtStart(): get duration failed", e);
            }
        }

        return time;
    }

    private boolean isAllDayEvent() {
        LogUtil.d(TAG, "isAllDayEvent(): sVersion = " + VCalendar.getVCalendarVersion());

        DtStart dtStart = (DtStart) getFirstProperty(Property.DTSTART);
        if (dtStart != null) {
            Value value = (Value) dtStart.getFirstParameter(Parameter.VALUE);
            if (value != null && Value.DATE.equals(value.getValue())) {
                LogUtil.d(TAG, "isAllDayEvent(): TRUE.");
                return true;
            }
        }

        Property xallday = getFirstProperty(Property.X_ALLDAY);
        if (xallday != null && ("1".equals(xallday.getValue()))) {
            LogUtil.d(TAG, "isAllDayEvent(): TRUE.");
            return true;
        }

        return false;
    }
}
