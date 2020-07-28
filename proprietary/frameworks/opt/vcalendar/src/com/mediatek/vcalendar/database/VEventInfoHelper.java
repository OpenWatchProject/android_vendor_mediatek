package com.mediatek.vcalendar.database;

import android.content.Context;
import android.database.Cursor;
import android.provider.CalendarContract.Attendees;
import android.provider.CalendarContract.Events;
import android.provider.CalendarContract.Reminders;

import com.mediatek.vcalendar.SingleComponentCursorInfo;
import com.mediatek.vcalendar.utils.CursorUtil;
import com.mediatek.vcalendar.utils.LogUtil;

/**
 * Build information used to compose a VEVENT component.
 *
 */
public class VEventInfoHelper extends ComponentInfoHelper {
    private static final String TAG = "VEventInfoHelper";

    protected VEventInfoHelper(Context context, String type) {
        super(context, type);
    }

    @Override
    protected boolean buildComponentInfo(Cursor componentCursor,
            SingleComponentCursorInfo info) {
        LogUtil.i(TAG, "buildComponentInfo()");

        info.cursor = CursorUtil.copyCurrentRow(componentCursor);
        info.componentType = mType;
        info.calendarId = componentCursor.getLong(componentCursor
                .getColumnIndex(Events.CALENDAR_ID));
        info.calendarName = null;

        long eventId = -1;
        try {
            eventId = componentCursor.getLong(componentCursor
                    .getColumnIndex(Events._ID));
        } catch (IllegalArgumentException e) {
            LogUtil.e(TAG, "buildComponentInfo():\t" + e.getMessage());
            return false;
        }
        if (eventId == -1) {
            return false;
        }

        // query reminders info
        Cursor reminders = null;
        reminders = mContentResolver.query(Reminders.CONTENT_URI, null,
                Reminders.EVENT_ID + "=" + eventId, null, null);
        if (reminders == null) {
            LogUtil.e(TAG, "buildComponentInfo(): Get the reminders failed.");
            return false;
        }
        info.remindersCursor = CursorUtil.copyOneCursor(reminders);
        reminders.close();

        // query attendees info
        Cursor attendees = null;
        attendees = mContentResolver.query(Attendees.CONTENT_URI, null,
                Attendees.EVENT_ID + "=" + eventId, null, null);
        if (attendees == null) {
            LogUtil.e(TAG, "buildComponentInfo(): Get the attendees failed.");
            return false;
        }
        info.attendeesCursor = CursorUtil.copyOneCursor(attendees);
        attendees.close();

        return true;
    }
}
