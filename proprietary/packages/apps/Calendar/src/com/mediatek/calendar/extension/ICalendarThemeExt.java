package com.mediatek.calendar.extension;

/**
 * M:This interface describes the way to change the Calendar Theme's attribute.
 *
 */
public interface ICalendarThemeExt {
    /**
     * It indicate the color's value is invalid.
     */
    int THEME_COLOR_INVALID = 0;

    /**
     * check the Theme Manager is ebable.if return false,
     * then will ignore the corlor that return by onPrepareColor().
     * @return true if Theme Manager is enable,false else.
     */
    boolean isThemeManagerEnable();

    /**
     * change the Calendar components theme color in this function.
     * @return the color If return DEFAULT_THEME_COLOR,it will not change the color.
     */
    int getThemeColor();

}
