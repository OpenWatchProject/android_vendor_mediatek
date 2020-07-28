package com.mediatek.calendar.ext;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.text.format.Time;



public interface ILunarExt {

    /**
     * Gets lunar date or lunar date range of a event according to the given startMillis
     * and endMillis of this event
     * @param localTimezone the current time zone
     * @param startMillis   the start time of a event
     * @param endMillis     the and time of a event
     * @param allDay        whether the event is all day event
     * @return lunar date like: 农历[闰]xx月xx
     *         or lunar date range like:农历[闰]xx月(初|十|廿|卅)x - [闰]xx月(初|十|廿|卅)x
     * @internal
     */
    public String getLunarDisplayedDate(String localTimezone, long startMillis,
            long endMillis, boolean allDay);

    /**
     * Build lunar date according the given gregorian date or time
     * @param timeZone  Timezone set for calender
     * @param milliTime the time in milis
     * @param date if it is null, timezone and milliTime must be assigned
     * @return lunar date string like  [闰]xx月(初|十|廿|卅)x
     * @internal
     */
    public String buildLunarDate(Time date, String timeZone, long milliTime);


    /** Draw lunar in the box(x ,y) of the month view if needed
		 * @param context application context
     * @param canvas        the canvas to draw
     * @param monthNumPaint the paint of the month number
     * @param x             the x of the number's right-bottom
     * @param y             the y of the number's right-bottom
     * @param weekDay day of the week.
     * @returns Void
     * @internal
     */
    public void drawLunarString(Context context, Canvas canvas, Paint monthNumPaint,
            int x, int y, Time weekDay);

}
