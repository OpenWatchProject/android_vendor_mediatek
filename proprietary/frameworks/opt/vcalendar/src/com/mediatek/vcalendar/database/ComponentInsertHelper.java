package com.mediatek.vcalendar.database;

import android.content.ContentValues;
import android.content.Context;
import android.net.Uri;

import com.mediatek.vcalendar.SingleComponentContentValues;
import com.mediatek.vcalendar.component.Component;
import com.mediatek.vcalendar.utils.LogUtil;

import java.util.List;
import java.util.LinkedList;

/**
 * It is used to insert parsed components from vcs or ics file. Normally
 * iCalendar components are stored in different databases, so every component
 * has its own implementation.
 *
 */
public abstract class ComponentInsertHelper {
    private static final String TAG = "ComponentInsertHelper";

    protected ComponentInsertHelper() {

    }

    /**
     * Create a concrete insert helper for one iCalendar component.
     *
     * @param context
     *            the Context
     * @param sccv
     *            content values will be inserted into database
     * @return a helper if succeed, null otherwise
     */
    static ComponentInsertHelper buildInsertHelper(Context context,
            SingleComponentContentValues sccv) {
        LogUtil.i(TAG, "buildInsertHelper(): component type: "
                + sccv.componentType);

        if (sccv.componentType.equals(Component.VEVENT)) {
            return new VEventInsertHelper(context);
        }

        return null;
    }

    /**
     * Insert one component into database.
     *
     * @param sccv
     *            information of the component
     * @return a URI if succeed, null otherwise
     */
    abstract Uri insertContentValues(SingleComponentContentValues sccv);

    /**
     * Insert more than one content values into database
     * @param mutiContentValues is a List content the information of the component
     * @return a URI if succeed, null otherwise
     */
    abstract Uri insertMultiComponentContentValues(
            List<SingleComponentContentValues> multiContentValues);

    protected void buildMemberCVList(LinkedList<ContentValues> list,
            String eventId, String columnName) {
        for (ContentValues values : list) {
            values.put(columnName, eventId);
        }
    }
}
