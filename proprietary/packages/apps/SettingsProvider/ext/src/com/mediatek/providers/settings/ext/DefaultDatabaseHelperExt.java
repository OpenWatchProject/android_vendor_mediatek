package com.mediatek.providers.settings.ext;

import android.content.Context;
import android.content.ContextWrapper;

/**
 * Interface that defines all methos which are implemented in
 * in DatabaseHelper about Settings Provider
 */

/** {@hide} */
public class DefaultDatabaseHelperExt extends ContextWrapper implements
        IDatabaseHelperExt {
    private static final String TAG = "DefaultDatabaseHelperExt";

    public DefaultDatabaseHelperExt(Context context) {
        super(context);
    }

    /**
     * @param context
     *            Context
     * @param name
     *            String
     * @param defaultValue
     *            String
     * @return the defaultValue get the string type
     */
    public String getResStr(Context context, String name, String defaultValue) {
        return defaultValue;
    }

    /**
     * @param context
     *            Context
     * @param name
     *            String
     * @param defaultValue
     *            String
     * @return the defaultValue get the boolean type
     */
    public String getResBoolean(Context context, String name, String defaultValue) {
        return defaultValue;
    }

    /**
     * @param context
     *            Context
     * @param name
     *            String
     * @param defaultValue
     *            String
     * @return the defaultValue get the integer type
     */
    public String getResInteger(Context context, String name, String defaultValue) {
        return defaultValue;
    }
}
