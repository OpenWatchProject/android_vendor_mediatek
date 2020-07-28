package com.mediatek.calendar.extension;

import android.content.Context;
import android.content.res.Resources;

import com.mediatek.calendar.features.Features;

public class CalendarThemeExt implements ICalendarThemeExt {
    private Context mContext;
    private static final int THEME_COLOR_DEFAULT = 0xe633b5e5;

    public CalendarThemeExt(Context context) {
        mContext = context;
    }
    @Override
    public boolean isThemeManagerEnable() {
        return Features.isThemeManagerEnabled();
    }

    @Override
    public int getThemeColor() {
        Resources res = mContext.getResources();
//        int colorValue = res.getThemeMainColor();
        int colorValue = THEME_COLOR_DEFAULT;
        return colorValue == THEME_COLOR_INVALID ? THEME_COLOR_DEFAULT : colorValue;
    }
}
