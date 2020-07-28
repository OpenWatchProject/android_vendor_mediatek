package com.mediatek.vcalendar.database;

import java.util.ArrayList;
import java.util.List;

import android.content.ContentProviderOperation;
import android.content.ContentProviderResult;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.OperationApplicationException;
import android.net.Uri;
import android.os.RemoteException;
import android.provider.CalendarContract;
import android.provider.CalendarContract.Attendees;
import android.provider.CalendarContract.Events;
import android.provider.CalendarContract.Reminders;

import com.mediatek.vcalendar.SingleComponentContentValues;
import com.mediatek.vcalendar.utils.LogUtil;

/**
 * Help to insert a VEVENT component to database.
 *
 */
public class VEventInsertHelper extends ComponentInsertHelper {
    private static final String TAG = "VEventInsertHelper";
    //Do insert to DB one time, The variable means the max number of calendar event.
    private static final int MAX_COMPONENT_COUNT = 500;

    private Context mContext;

    protected VEventInsertHelper(Context context) {
        super();

        mContext = context;
    }

    @Override
    Uri insertContentValues(SingleComponentContentValues sccv) {
        LogUtil.i(TAG, "insertContentValues()");

        ContentResolver cr = mContext.getContentResolver();
        Uri result = null;
        int alarmSize = sccv.alarmValuesList.size();
        int attendeesSize = sccv.attendeeValuesList.size();
        LogUtil.d(TAG, "insertContentValues(): Alarms count: " + alarmSize);
        LogUtil.d(TAG, "insertContentValues(): Attendees count:" + attendeesSize);

        ArrayList<ContentProviderOperation> ops = new ArrayList<ContentProviderOperation>();
        int eventIdIndex = ops.size();
        // insert event.
        ContentProviderOperation.Builder b = ContentProviderOperation.newInsert(
                Events.CONTENT_URI).withValues(sccv.contentValues);
        ops.add(b.build());

        // insert reminders.
        for (ContentValues reminder : sccv.alarmValuesList) {
            b = ContentProviderOperation.newInsert(Reminders.CONTENT_URI).withValues(reminder);
            b.withValueBackReference(Reminders.EVENT_ID, eventIdIndex);
            ops.add(b.build());
        }
        // insert attendees.
        for (ContentValues attendee : sccv.attendeeValuesList) {
            b = ContentProviderOperation.newInsert(Attendees.CONTENT_URI).withValues(attendee);
            b.withValueBackReference(Attendees.EVENT_ID, eventIdIndex);
            ops.add(b.build());
        }
        ContentProviderResult[] cpr = null;
        try {
            cpr = cr.applyBatch(CalendarContract.AUTHORITY, ops);
        } catch (OperationApplicationException e) {
            e.printStackTrace();
        } catch (RemoteException e) {
            e.printStackTrace();
        }
        if (cpr != null && cpr[0] != null) {
            result = cpr[0].uri;
            LogUtil.v(TAG, "addNextContentValue: insert event=" + result);
        } else {
            LogUtil.e(TAG, "addNextContentValue: insert event failed.");
        }

        return result;
    }

    @Override
    Uri insertMultiComponentContentValues(
            List<SingleComponentContentValues> multiComponentContentValues) {
        LogUtil.i(TAG, "insertmultiComponentContentValues()");
        Uri result = null;
        int allSize = multiComponentContentValues.size();
        if (allSize <= 0) {
            LogUtil.e(TAG, "insertMultiComponentContentValues the count is null.");
        } else {
            int totalCount = allSize / MAX_COMPONENT_COUNT;
            int i = 0;
            List<SingleComponentContentValues> componentContentValues
                    = new ArrayList<SingleComponentContentValues>(MAX_COMPONENT_COUNT);
            for (int currentCount = 0; currentCount <= totalCount; currentCount ++) {
                for (int index = 0; index < MAX_COMPONENT_COUNT; index ++) {
                    if (i >= allSize) {
                        break;
                    }
                    componentContentValues.add(index, multiComponentContentValues.get(i));
                    i ++;
                }
                result = doInsertMultiComponentValues(componentContentValues);
            }
        }

        return result;
    }

    private Uri doInsertMultiComponentValues(List<SingleComponentContentValues> componentContentValues) {
        ContentResolver cr = mContext.getContentResolver();
        ArrayList<ContentProviderOperation> ops = new ArrayList<ContentProviderOperation>();
        Uri result = null;
        for (SingleComponentContentValues sccv : componentContentValues) {
            int alarmSize = sccv.alarmValuesList.size();
            int attendeesSize = sccv.attendeeValuesList.size();
            LogUtil.d(TAG, "insertContentValues(): Alarms count: " + alarmSize
                    + ", Attendees count:" + attendeesSize);
            int eventIdIndex = ops.size();
            // insert event.
            ContentProviderOperation.Builder b = ContentProviderOperation.newInsert(
                    Events.CONTENT_URI).withValues(sccv.contentValues);
            ops.add(b.build());

            // insert reminders.
            for (ContentValues reminder : sccv.alarmValuesList) {
                b = ContentProviderOperation.newInsert(Reminders.CONTENT_URI).withValues(reminder);
                b.withValueBackReference(Reminders.EVENT_ID, eventIdIndex);
                ops.add(b.build());
            }
            // insert attendees.
            for (ContentValues attendee : sccv.attendeeValuesList) {
                b = ContentProviderOperation.newInsert(Attendees.CONTENT_URI).withValues(attendee);
                b.withValueBackReference(Attendees.EVENT_ID, eventIdIndex);
                ops.add(b.build());
            }
        }
        ContentProviderResult[] cpr = null;
        try {
            cpr = cr.applyBatch(CalendarContract.AUTHORITY, ops);
        } catch (OperationApplicationException e) {
            e.printStackTrace();
        } catch (RemoteException e) {
            e.printStackTrace();
        }
        if (cpr != null && cpr[0] != null) {
            result = cpr[0].uri;
            LogUtil.v(TAG, "addNextContentValue: insert event=" + result);
        } else {
            LogUtil.e(TAG, "addNextContentValue: insert event failed.");
        }

        return result;
    }
}
