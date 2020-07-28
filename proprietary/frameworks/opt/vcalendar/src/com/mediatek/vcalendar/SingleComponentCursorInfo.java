package com.mediatek.vcalendar;

import android.database.Cursor;

/**
 * Information of an iCalendar component, used to query from database and
 * compose an iCalendar component.
 */
public class SingleComponentCursorInfo {
    /**
     * Identifier for an iCalendar component, e.g. VEVENT, VTODO. This field
     * MUST NOT be null or empty.
     */
    public String componentType;

    /**
     * Component information queried form database.
     */
    public Cursor cursor;

    /**
     * Calendar id for the component if exists, may be -1.
     */
    public long calendarId;

    /**
     * Calendar name for the component if exists, may be null.
     */
    public String calendarName;

    /**
     * Reminders information queried form database for the component, may be
     * null.
     */
    public Cursor remindersCursor;

    /**
     * Attendees information queried form database for the component, may be
     * null.
     */
    public Cursor attendeesCursor;
}