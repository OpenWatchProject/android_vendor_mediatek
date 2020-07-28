package com.mediatek.vcalendar.database;

import android.content.ContentResolver;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.provider.CalendarContract.Events;

import com.mediatek.vcalendar.SingleComponentCursorInfo;
import com.mediatek.vcalendar.component.Component;
import com.mediatek.vcalendar.utils.CursorUtil;
import com.mediatek.vcalendar.utils.LogUtil;

/**
 * It is used to query components information from database, then composed into
 * components by composer. Because each iCalendar component has its own feature,
 * information should be filled in SingleComponentCursorInfo object differently,
 * do this in each component's buildComponentInfo method.
 *
 */
public abstract class ComponentInfoHelper {
    private static final String TAG = "ComponentInfoHelper";

    protected final Context mContext;
    protected final ContentResolver mContentResolver;

    private Cursor mComponentCursor;
    private int mComponentCount = -1;

    /*
     * Type of concrete component the information will be composed to, same as
     * the component's name, e.g. "VEVENT", "VTODO".
     */
    protected String mType;

    protected ComponentInfoHelper(Context context, String type) {
        mContext = context;
        mContentResolver = mContext.getContentResolver();
        mType = type;
    }

    /**
     * Create a component's information helper used to fill information queried
     * from database into a SingleComponentCursorInfo object.
     *
     * @param context
     *            the Context
     * @param type
     *            URI string used to query component information
     * @return a concrete helper for a iCalendar component
     */
    static ComponentInfoHelper createComponentInfoHelper(Context context,
            String type) {
        LogUtil.d(TAG, "createComponentInfoHelper(): type URI: " + type);

        /*
         * We should get the type of the components, VEVENT, VTODO, or other
         * iCalendar components. Types of components in one query are the same.
         */
        if (type.equals(Events.CONTENT_URI.toString())) {
            return new VEventInfoHelper(context, Component.VEVENT);
        }

        return null;
    }

    /**
     * Build the cursors of the component which used to compose it.
     *
     * @param componentCursor
     *            cursor containing components
     * @param info
     *            holder of all the information of one single component
     * @return true if succeed, otherwise false
     */
    protected abstract boolean buildComponentInfo(Cursor componentCursor,
            SingleComponentCursorInfo info);

    /**
     * Query component information from database.
     *
     * @param uri
     *            query URI
     * @param projection
     *            the projection
     * @param selection
     *            the query selection
     * @param selectionArgs
     *            the selection arguments
     * @param sortOrder
     *            sort order
     * @return true if query succeed and the cursor not empty, false otherwise.
     */
    public boolean query(Uri uri, String[] projection, String selection,
            String[] selectionArgs, String sortOrder) {
        Cursor cursor = mContentResolver.query(uri, projection, selection,
                selectionArgs, sortOrder);
        if (cursor == null) {
            return false;
        }

        mComponentCursor = CursorUtil.copyOneCursor(cursor);
        mComponentCount = mComponentCursor.getCount();
        cursor.close();

        if (!mComponentCursor.moveToFirst()) {
            return false;
        }

        return true;
    }

    /**
     * return components count.
     *
     * @return the count
     */
    public int getComponentCount() {
        return mComponentCount;
    }

    /**
     * See if has next component's information.
     *
     * @return true if has, false otherwise
     */
    public boolean hasNextComponentInfo() {
        if (mComponentCursor == null || mComponentCursor.isAfterLast()) {
            return false;
        }

        return true;
    }

    /**
     * Get next component's information.
     *
     * @return the information object
     */
    public SingleComponentCursorInfo getNextComponentInfo() {
        SingleComponentCursorInfo info;
        info = new SingleComponentCursorInfo();

        if (!buildComponentInfo(mComponentCursor, info)) {
            return null;
        }

        mComponentCursor.moveToNext();

        return info;
    }
}
