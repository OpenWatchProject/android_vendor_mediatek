package com.mediatek.vcalendar;

/**
 * The preview brief information.
 *
 */
public class ComponentPreviewInfo {
    /**
     * Identifier for an iCalendar component, e.g. VEVENT, VTODO.
     */
    public String componentType;
    /**
     * Components count in current parse.
     */
    public int componentCount;

    ////////////////////////////////////////////////////////////////////////////
    // VEVENT only
    /**
     * VEVENT only: event summary.
     */
    public String eventSummary;
    /**
     * VEVENT only: event organizer.
     */
    public String eventOrganizer;
    /**
     * VEVENT only: start time of the event.
     */
    public long eventStartTime;
    /**
     * VEVENT only: duration of the event.
     */
    public String eventDuration;
    ////////////////////////////////////////////////////////////////////////////
}