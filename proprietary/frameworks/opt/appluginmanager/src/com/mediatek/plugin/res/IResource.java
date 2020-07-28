package com.mediatek.plugin.res;

import android.graphics.drawable.Drawable;

/**
 * Parser lib resource.
 */
public interface IResource {

    /**
     * Parser string value.
     * @param value
     *            the XML value.
     * @return the String resource.
     */
    public abstract String getString(String value);

    /**
     * Do separate String operation.
     * @param value
     *            The XML value.
     * @param separator
     *            Separator of the String.
     * @return The sub String array.
     */
    public abstract String[] getString(String value, String separator);

    /**
     * Get drawable from.
     * @param value
     *            The XML value.
     * @return the drawable.
     */
    public abstract Drawable getDrawable(String value);
}
