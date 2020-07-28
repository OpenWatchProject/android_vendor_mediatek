package com.mediatek.calendar.extension;

import android.app.Activity;
import android.database.Cursor;
import android.provider.CalendarContract;
import android.provider.CalendarContract.Calendars;

public class PCSyncExtension implements IAccountExt {

    private static final String ACCOUNT_UNIQUE_KEY = "ACCOUNT_KEY";
    private final Activity mActivity;

    public PCSyncExtension(Activity activity) {
        mActivity = activity;
    }

    @Override
    public Cursor accountQuery(String[] projection) {
        return queryAccountsExceptionPCSync(projection);
    }

    /**
     * in many cases, the #PC Sync# account should be filtered.
     * so the query result should not contain anything about pc sync
     * @param projection
     * @return query result
     */
    private Cursor queryAccountsExceptionPCSync(String[] projection) {
        String[] selectArgs = { CalendarContract.ACCOUNT_TYPE_LOCAL };
        Cursor cursor = mActivity.managedQuery(Calendars.CONTENT_URI, projection,
                //Cheap hack to make WHERE a GROUP BY query
                Calendars.ACCOUNT_TYPE + "!=?" + "1) GROUP BY (" + ACCOUNT_UNIQUE_KEY ,
                selectArgs /* selectionArgs */,
                Calendars.ACCOUNT_NAME /*sort order*/);
        return cursor;
    }

}
