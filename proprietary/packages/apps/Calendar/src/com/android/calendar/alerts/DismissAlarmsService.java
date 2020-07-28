/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License
 */

package com.android.calendar.alerts;

import android.app.IntentService;
import android.app.TaskStackBuilder;
import android.content.ContentProviderOperation;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Intent;
import android.content.OperationApplicationException;
import android.net.Uri;
import android.os.IBinder;
import android.os.RemoteException;
import android.os.UserHandle;
import android.provider.CalendarContract;
import android.provider.CalendarContract.CalendarAlerts;
import android.util.Log;

import com.android.calendar.EventInfoActivity;

import java.util.ArrayList;

/**
 * Service for asynchronously marking fired alarms as dismissed.
 */
public class DismissAlarmsService extends IntentService {
    private static final String TAG = "DismissAlarmsService";
    public static final String SHOW_ACTION = "com.android.calendar.SHOW";
    public static final String DISMISS_ACTION = "com.android.calendar.DISMISS";

    private static final String[] PROJECTION = new String[] {
            CalendarAlerts.STATE,
    };
    private static final int COLUMN_INDEX_STATE = 0;
    /** M:fix Expression tree is too large, must less than 1000*/
    private static final int MAX_MULTIPLE_EVENTS_NUM = 500;

    public DismissAlarmsService() {
        super("DismissAlarmsService");
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onHandleIntent(Intent intent) {
        if (AlertService.DEBUG) {
            Log.d(TAG, "onReceive: a=" + intent.getAction() + " " + intent.toString());
        }
        try {

        long eventId = intent.getLongExtra(AlertUtils.EVENT_ID_KEY, -1);
        long eventStart = intent.getLongExtra(AlertUtils.EVENT_START_KEY, -1);
        long eventEnd = intent.getLongExtra(AlertUtils.EVENT_END_KEY, -1);
        long[] eventIds = intent.getLongArrayExtra(AlertUtils.EVENT_IDS_KEY);
        int notificationId = intent.getIntExtra(AlertUtils.NOTIFICATION_ID_KEY, -1);
        /**
         * M: whether this event has been opened or will be opened, if true, we should change state
         * to DISMISSED, otherwise, we should determine by whether this event is an overdue one, if
         * it's overdue, dismiss it, or we should make it IGNORED @{
         */
        boolean alreadyShowed = intent.getBooleanExtra(AlertUtils.EVENT_SHOWED, false);
        boolean shouldDismiss = alreadyShowed;
        /** @} */

        Uri uri = CalendarAlerts.CONTENT_URI;
        String selection = "";
        /** M:Expression tree is too large.*/
        ArrayList<String> selectionsArray = new ArrayList<String>();

        /**
         * M: Change the state of a specific fired alarm if id is present, otherwise, change all
         * alarms' states @{
         */
        if (eventId != -1) {
            /// M: update event id to notification id map, which means remove one notification id @{
            AlertService.getEventIdToNotificationIdMap().remove(eventId);
            /// @}
            selection = "(" + CalendarAlerts.STATE + "=" + CalendarAlerts.STATE_FIRED
                    + " OR " + CalendarAlerts.STATE + "=" + AlertUtils.ALERT_EXT_STATE_IGNORED
                    + ")" + " AND " + CalendarAlerts.EVENT_ID + "=" + eventId;
            /** M:*/
            selectionsArray.add(selection);
        } else if (eventIds != null && eventIds.length > 0) {
            /** M: change build selection to build selectionArray.@{*/
            buildMultipleEventsQuery(eventIds, selectionsArray);
            /**@}*/
            /// M: update event id to notification id map, which means remove one notification id @{
            for (long id : eventIds) {
                AlertService.getEventIdToNotificationIdMap().remove(id);
            }
            /// @}
        } else {
            selection = CalendarAlerts.STATE + "=" + CalendarAlerts.STATE_FIRED
                    + " OR " + CalendarAlerts.STATE + "=" + AlertUtils.ALERT_EXT_STATE_IGNORED;
            /** M:*/
            selectionsArray.add(selection);
            /// M: update event id to notification id map, which means remove one notification id @{
            AlertService.getEventIdToNotificationIdMap().clear();
            /// @}
        }

        ContentResolver resolver = getContentResolver();
        ContentValues values = new ContentValues();
        /** M: #update message: dismiss# @{ */
        long currentTime = System.currentTimeMillis();
        String expired = " AND " + CalendarAlerts.END + "<=" + currentTime;
        String unExpired = " AND " + CalendarAlerts.END + ">" + currentTime;

        ArrayList<ContentProviderOperation> ops = new ArrayList<ContentProviderOperation>();
        ContentProviderOperation.Builder b;

        values.put(CalendarAlerts.STATE, CalendarAlerts.STATE_DISMISSED);
        if (shouldDismiss) {
            // update viewed events' alert info to be dismiss status.
            for (String selectionUpdate : selectionsArray) {
                b = ContentProviderOperation.newUpdate(uri).withValues(values)
                        .withSelection(selectionUpdate, null);
                ops.add(b.build());
            }
        } else {
            // update expired events' alert info to be dismiss status.
            for (String updateSelection : selectionsArray) {
                b = ContentProviderOperation.newUpdate(uri).withValues(values)
                        .withSelection(updateSelection + expired, null);
                ops.add(b.build());
            }
            // update unexpired events' alert info to be ignored status.
            values.put(CalendarAlerts.STATE, AlertUtils.ALERT_EXT_STATE_IGNORED);
            for (String updateSelection : selectionsArray) {
                b = ContentProviderOperation.newUpdate(uri).withValues(values)
                        .withSelection(updateSelection + unExpired, null);
                ops.add(b.build());
            }
        }

        if (ops.size() > 0) {
            try {
                resolver.applyBatch(CalendarContract.AUTHORITY, ops);
            } catch (OperationApplicationException e) {
                e.printStackTrace();
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }
        /**@}*/

        /** M: We don't need to cancel the notification here, just refresh it according to database
         * change
        */
//        // Remove from notification bar.
//        if (notificationId != -1) {
//            NotificationManager nm =
//                    (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
//            nm.cancel(notificationId);
//        }
        AlertUtils.scheduleNextNotificationRefresh(this, null, System.currentTimeMillis());

        if (SHOW_ACTION.equals(intent.getAction())) {
            // Show event on Calendar app by building an intent and task stack to start
            // EventInfoActivity with AllInOneActivity as the parent activity rooted to home.
            Intent i = AlertUtils.buildEventViewIntent(this, eventId, eventStart, eventEnd);
/*
            TaskStackBuilder.create(this)
                    .addParentStack(EventInfoActivity.class).addNextIntent(i).startActivities();
*/
            Intent[] intents = TaskStackBuilder.create(this)
                    .addParentStack(EventInfoActivity.class).addNextIntent(i).getIntents();
            intents[0].setFlags(intents[0].getFlags() & ~Intent.FLAG_ACTIVITY_TASK_ON_HOME);
            startActivitiesAsUser(intents, null, new UserHandle(UserHandle.myUserId()));
        }

        /// M: post unread number @{
        AlertUtils.postUnreadNumber(this);
        /// @}
        /** @} */

        // Stop this service
        /** M: we don't need stop the service here, cause: 1, If we have more than one intent arrived
        * to this service, the rest intents would never be processed after the first one had been
        * handled; 2, The handler which process the intents will stop itself when the intent it's
        * responsible for had been processed
        //stopSelf();
        */
    }
      catch (SecurityException e) {
            Log.d(TAG, "Exception SecurityException ");
      }
    }

    /**
     * M: We should also query alerts which is in IGNORED state
     * @param eventIds
     * @return
     */
    private String buildMultipleEventsQuery(long[] eventIds) {
        StringBuilder selection = new StringBuilder();
        selection.append("(");
        selection.append(CalendarAlerts.STATE);
        selection.append("=");
        selection.append(CalendarAlerts.STATE_FIRED);
        selection.append(" OR ");
        selection.append(CalendarAlerts.STATE);
        selection.append("=");
        selection.append(AlertUtils.ALERT_EXT_STATE_IGNORED);
        selection.append(")");
        if (eventIds.length > 0) {
            selection.append(" AND (");
            selection.append(CalendarAlerts.EVENT_ID);
            selection.append("=");
            selection.append(eventIds[0]);
            for (int i = 1; i < eventIds.length; i++) {
                selection.append(" OR ");
                selection.append(CalendarAlerts.EVENT_ID);
                selection.append("=");
                selection.append(eventIds[i]);
            }
            selection.append(")");
        }
        return selection.toString();
    }

    /**
     * M: build multiple events' query selections, and because the parameter
     * should less than 1000 in Expression, so we split the selection every 500
     * events(but the first one has 501 event id info).
     * eventIds' number should >= 2.
     */
    private void buildMultipleEventsQuery(long[] eventIds, ArrayList<String> selectionsArray) {
        StringBuilder selection1 = new StringBuilder();
        selection1.append("(");
        selection1.append(CalendarAlerts.STATE);
        selection1.append("=");
        selection1.append(CalendarAlerts.STATE_FIRED);
        selection1.append(" OR ");
        selection1.append(CalendarAlerts.STATE);
        selection1.append("=");
        selection1.append(AlertUtils.ALERT_EXT_STATE_IGNORED);
        selection1.append(")");

        StringBuilder selection2 = new StringBuilder();
        if (eventIds.length > 0) {
            selection1.append(" AND (");
            int length = eventIds.length;
            for (int i = 0; i < length; i++) {
                selection2.append(CalendarAlerts.EVENT_ID);
                selection2.append("=");
                selection2.append(eventIds[i]);

                if (i != 0 && ((i % MAX_MULTIPLE_EVENTS_NUM) == 0 || i == (length - 1))) {
                    selection2.append(")");
                    selectionsArray.add(selection1.toString() + selection2.toString());
                    selection2.delete(0, selection2.length());
                } else {
                    selection2.append(" OR ");
                }
            }
        }
    }
}
