package com.mediatek.calendar.ext;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.text.format.Time;



public class DefaultLunarExtension implements ILunarExt {

    @Override
    public String getLunarDisplayedDate(String localTimezone, long startMillis,
            long endMillis, boolean allDay) {
        return "";
    }

    @Override
    public String buildLunarDate(Time date, String timeZone, long milliTime) {
        return "";
    }

    @Override
    public void drawLunarString(Context context, Canvas canvas, Paint monthNumPaint,
            int x, int y, Time weekDay) {

    }

}
