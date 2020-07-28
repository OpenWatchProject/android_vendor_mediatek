package com.mediatek.providers.settings.ext;

import android.content.Context;

/**
 * Interface that defines all methods which are implemented
 * in DatabaseHelper about Settings Provider
 */

/** {@hide} */
public interface IDatabaseHelperExt {
    /**
     * Get the name's string value resource.
     * @internal
     */
    public String getResStr(Context context, String name, String defaultValue);

    /**
     * Get the name's boolean value resource.
     * @internal
     */
    public String getResBoolean(Context context, String name, String defaultValue);

    /**
     * Get the name's Integer value resource.
     * @internal
     */
    public String getResInteger(Context context, String name, String defaultValue);
}
