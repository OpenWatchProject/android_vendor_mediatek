package com.mediatek.vcalendar;

import android.content.ContentValues;

import java.util.LinkedList;

/**
 * Single component ContentValues Lists. Used to parse iCalendar components and
 * write them into database.
 *
 */
public class SingleComponentContentValues {
    /**
     * Identifier for an iCalendar component, e.g. VEVENT, VTODO. This field
     * MUST NOT be null or empty.
     */
    public String componentType;

    /**
     * Parsed values for an iCalendar component.
     */
    public final ContentValues contentValues = new ContentValues();

    /**
     * Alarm information for the component if exist, may be null.
     */
    public final LinkedList<ContentValues> alarmValuesList = new LinkedList<ContentValues>();

    /**
     * Attendee information for the component if exist, may be null.
     */
    public final LinkedList<ContentValues> attendeeValuesList = new LinkedList<ContentValues>();
}