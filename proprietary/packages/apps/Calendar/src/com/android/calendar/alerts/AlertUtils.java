/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2012 The Android Open Source Project
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

import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.net.Uri;
import android.provider.CalendarContract;
import android.provider.CalendarContract.CalendarAlerts;
import android.provider.CalendarContract.Events;
import android.text.TextUtils;
import android.text.format.DateFormat;
import android.text.format.DateUtils;
import android.text.format.Time;
import android.util.Log;

import com.android.calendar.EventInfoActivity;
import com.android.calendar.R;
import com.android.calendar.Utils;
import com.mediatek.calendar.LogUtil;
import com.mediatek.calendar.MTKUtils;

import java.util.Locale;
import java.util.Map;
import java.util.TimeZone;

public class AlertUtils {
    private static final String TAG = "AlertUtils";
    static final boolean DEBUG = true;

    /// M: used for dismiss alert @{
    public static final String DISMISS_ALERT_ACTION = "com.android.calendar.DELETE";
    // @}
    public static final long SNOOZE_DELAY = 5 * 60 * 1000L;

    // We use one notification id for the expired events notification.  All
    // other notifications (the 'active' future/concurrent ones) use a unique ID.
    public static final int EXPIRED_GROUP_NOTIFICATION_ID = 0;

    public static final String EVENT_ID_KEY = "eventid";
    public static final String EVENT_START_KEY = "eventstart";
    public static final String EVENT_END_KEY = "eventend";
    public static final String NOTIFICATION_ID_KEY = "notificationid";
    public static final String EVENT_IDS_KEY = "eventids";
    public static final String EVENT_STARTS_KEY = "starts";
    /**
     * M: This is used to identify the event has been opened through clicking it and whether the
     * event is overdue @{
     */
    public static final String EVENT_SHOWED = "eventshowed";
    // indicate that the alert has been ignored, which means the user swipe the notification away
    public static final int ALERT_EXT_STATE_IGNORED = 100;
    /** @} */

    // A flag for using local storage to save alert state instead of the alerts DB table.
    // This allows the unbundled app to run alongside other calendar apps without eating
    // alerts from other apps.
    static boolean BYPASS_DB = true;

    // SharedPrefs table name for storing fired alerts.  This prevents other installed
    // Calendar apps from eating the alerts.
    private static final String ALERTS_SHARED_PREFS_NAME = "calendar_alerts";

    // Keyname prefix for the alerts data in SharedPrefs.  The key will contain a combo
    // of event ID, begin time, and alarm time.  The value will be the fired time.
    private static final String KEY_FIRED_ALERT_PREFIX = "preference_alert_";

    // The last time the SharedPrefs was scanned and flushed of old alerts data.
    private static final String KEY_LAST_FLUSH_TIME_MS = "preference_flushTimeMs";

    // The # of days to save alert states in the shared prefs table, before flushing.  This
    // can be any value, since AlertService will also check for a recent alertTime before
    // ringing the alert.
    private static final int FLUSH_INTERVAL_DAYS = 1;
    private static final int FLUSH_INTERVAL_MS = FLUSH_INTERVAL_DAYS * 24 * 60 * 60 * 1000;

    /**
     * Creates an AlarmManagerInterface that wraps a real AlarmManager.  The alarm code
     * was abstracted to an interface to make it testable.
     */
    public static AlarmManagerInterface createAlarmManager(Context context) {
        final AlarmManager mgr = (AlarmManager) context.getSystemService(Context.ALARM_SERVICE);
        return new AlarmManagerInterface() {
            @Override
            public void set(int type, long triggerAtMillis, PendingIntent operation) {
                if (Utils.isKeyLimePieOrLater()) {
                    mgr.setExact(type, triggerAtMillis, operation);
                } else {
                    mgr.set(type, triggerAtMillis, operation);
                }
            }
        };
    }

    /**
     * Schedules an alarm intent with the system AlarmManager that will notify
     * listeners when a reminder should be fired. The provider will keep
     * scheduled reminders up to date but apps may use this to implement snooze
     * functionality without modifying the reminders table. Scheduled alarms
     * will generate an intent using AlertReceiver.EVENT_REMINDER_APP_ACTION.
     *
     * @param context A context for referencing system resources
     * @param manager The AlarmManager to use or null
     * @param alarmTime The time to fire the intent in UTC millis since epoch
     */
    public static void scheduleAlarm(Context context, AlarmManagerInterface manager,
            long alarmTime) {
        scheduleAlarmHelper(context, manager, alarmTime, false);
    }

    /**
     * Schedules the next alarm to silently refresh the notifications.  Note that if there
     * is a pending silent refresh alarm, it will be replaced with this one.
     */
    static void scheduleNextNotificationRefresh(Context context, AlarmManagerInterface manager,
            long alarmTime) {
        scheduleAlarmHelper(context, manager, alarmTime, true);
    }

    private static void scheduleAlarmHelper(Context context, AlarmManagerInterface manager,
            long alarmTime, boolean quietUpdate) {
        /**
         * M: make it work as JB even when manager is NULL
         */
        if (manager == null) {
            manager = createAlarmManager(context);
        }
        /** @} */

        int alarmType = AlarmManager.RTC_WAKEUP;
        Intent intent = new Intent(AlertReceiver.EVENT_REMINDER_APP_ACTION);
        intent.setClass(context, AlertReceiver.class);
        if (quietUpdate) {
            alarmType = AlarmManager.RTC;
        } else {
            // Set data field so we get a unique PendingIntent instance per alarm or else alarms
            // may be dropped.
            Uri.Builder builder = CalendarAlerts.CONTENT_URI.buildUpon();
            ContentUris.appendId(builder, alarmTime);
            intent.setData(builder.build());
        }

        intent.putExtra(CalendarContract.CalendarAlerts.ALARM_TIME, alarmTime);
        PendingIntent pi = PendingIntent.getBroadcast(context, 0, intent,
                PendingIntent.FLAG_UPDATE_CURRENT);
        manager.set(alarmType, alarmTime, pi);
    }

    /**
     * Format the second line which shows time and location for single alert or the
     * number of events for multiple alerts
     *     1) Show time only for non-all day events
     *     2) No date for today
     *     3) Show "tomorrow" for tomorrow
     *     4) Show date for days beyond that
     */
    static String formatTimeLocation(Context context, long startMillis, boolean allDay,
            String location) {
        String tz = Utils.getTimeZone(context, null);
        Time time = new Time(tz);
        time.setToNow();
        int today = Time.getJulianDay(time.toMillis(false), time.gmtoff);
        time.set(startMillis);
        int eventDay = Time.getJulianDay(time.toMillis(false), allDay ? 0 : time.gmtoff);

        ///M: FORMAT_ABBREV_ALL flag will limit the month/weekday/time to a brief format
        /// and we don't want TIME be formated like 3pm. we need 3:00pm @{
        int flags = DateUtils.FORMAT_ABBREV_MONTH | DateUtils.FORMAT_ABBREV_WEEKDAY;
        ///@}
        if (!allDay) {
            flags |= DateUtils.FORMAT_SHOW_TIME;
            if (DateFormat.is24HourFormat(context)) {
                flags |= DateUtils.FORMAT_24HOUR;
            }
        } else {
            flags |= DateUtils.FORMAT_UTC;
        }

        if (eventDay < today || eventDay > today + 1) {
            flags |= DateUtils.FORMAT_SHOW_DATE;
        }

        StringBuilder sb = new StringBuilder(Utils.formatDateRange(context, startMillis,
                startMillis, flags));

        if (!allDay && tz != Time.getCurrentTimezone()) {
            // Assumes time was set to the current tz
            time.set(startMillis);
            boolean isDST = time.isDst != 0;
            sb.append(" ").append(TimeZone.getTimeZone(tz).getDisplayName(
                    isDST, TimeZone.SHORT, Locale.getDefault()));
        }

        if (eventDay == today + 1) {
            // Tomorrow
            sb.append(", ");
            sb.append(context.getString(R.string.tomorrow));
        }

        String loc;
        if (location != null && !TextUtils.isEmpty(loc = location.trim())) {
            sb.append(", ");
            sb.append(loc);
        }
        return sb.toString();
    }

    public static ContentValues makeContentValues(long eventId, long begin, long end,
            long alarmTime, int minutes) {
        ContentValues values = new ContentValues();
        values.put(CalendarAlerts.EVENT_ID, eventId);
        values.put(CalendarAlerts.BEGIN, begin);
        values.put(CalendarAlerts.END, end);
        values.put(CalendarAlerts.ALARM_TIME, alarmTime);
        long currentTime = System.currentTimeMillis();
        values.put(CalendarAlerts.CREATION_TIME, currentTime);
        values.put(CalendarAlerts.RECEIVED_TIME, 0);
        values.put(CalendarAlerts.NOTIFY_TIME, 0);
        values.put(CalendarAlerts.STATE, CalendarAlerts.STATE_SCHEDULED);
        values.put(CalendarAlerts.MINUTES, minutes);
        return values;
    }

    public static Intent buildEventViewIntent(Context c, long eventId, long begin, long end) {
        Intent i = new Intent(Intent.ACTION_VIEW);
        Uri.Builder builder = CalendarContract.CONTENT_URI.buildUpon();
        builder.appendEncodedPath("events/" + eventId);
        i.setData(builder.build());
        i.setClass(c, EventInfoActivity.class);
        i.putExtra(CalendarContract.EXTRA_EVENT_BEGIN_TIME, begin);
        i.putExtra(CalendarContract.EXTRA_EVENT_END_TIME, end);
        return i;
    }

    public static SharedPreferences getFiredAlertsTable(Context context) {
        return context.getSharedPreferences(ALERTS_SHARED_PREFS_NAME, Context.MODE_PRIVATE);
    }

    private static String getFiredAlertsKey(long eventId, long beginTime,
            long alarmTime) {
        StringBuilder sb = new StringBuilder(KEY_FIRED_ALERT_PREFIX);
        sb.append(eventId);
        sb.append("_");
        sb.append(beginTime);
        sb.append("_");
        sb.append(alarmTime);
        return sb.toString();
    }

    /**
     * Returns whether the SharedPrefs storage indicates we have fired the alert before.
     */
    static boolean hasAlertFiredInSharedPrefs(Context context, long eventId, long beginTime,
            long alarmTime) {
        SharedPreferences prefs = getFiredAlertsTable(context);
        return prefs.contains(getFiredAlertsKey(eventId, beginTime, alarmTime));
    }

    /**
     * Store fired alert info in the SharedPrefs.
     */
    static void setAlertFiredInSharedPrefs(Context context, long eventId, long beginTime,
            long alarmTime) {
        // Store alarm time as the value too so we don't have to parse all the keys to flush
        // old alarms out of the table later.
        SharedPreferences prefs = getFiredAlertsTable(context);
        SharedPreferences.Editor editor = prefs.edit();
        editor.putLong(getFiredAlertsKey(eventId, beginTime, alarmTime), alarmTime);
        editor.apply();
    }

    /**
     * Scans and flushes the internal storage of old alerts.  Looks up the previous flush
     * time in SharedPrefs, and performs the flush if overdue.  Otherwise, no-op.
     */
    static void flushOldAlertsFromInternalStorage(Context context) {
        if (BYPASS_DB) {
            SharedPreferences prefs = getFiredAlertsTable(context);

            // Only flush if it hasn't been done in a while.
            long nowTime = System.currentTimeMillis();
            long lastFlushTimeMs = prefs.getLong(KEY_LAST_FLUSH_TIME_MS, 0);
            if (nowTime - lastFlushTimeMs > FLUSH_INTERVAL_MS) {
                if (DEBUG) {
                    Log.d(TAG, "Flushing old alerts from shared prefs table");
                }

                // Scan through all fired alert entries, removing old ones.
                SharedPreferences.Editor editor = prefs.edit();
                Time timeObj = new Time();
                for (Map.Entry<String, ?> entry : prefs.getAll().entrySet()) {
                    String key = entry.getKey();
                    Object value = entry.getValue();
                    if (key.startsWith(KEY_FIRED_ALERT_PREFIX)) {
                        long alertTime;
                        if (value instanceof Long) {
                            alertTime = (Long) value;
                        } else {
                            // Should never occur.
                            Log.e(TAG,"SharedPrefs key " + key + " did not have Long value: " +
                                    value);
                            continue;
                        }

                        if (nowTime - alertTime >= FLUSH_INTERVAL_MS) {
                            editor.remove(key);
                            if (DEBUG) {
                                int ageInDays = getIntervalInDays(alertTime, nowTime, timeObj);
                                Log.d(TAG, "SharedPrefs key " + key + ": removed (" + ageInDays +
                                        " days old)");
                            }
                        } else {
                            if (DEBUG) {
                                int ageInDays = getIntervalInDays(alertTime, nowTime, timeObj);
                                Log.d(TAG, "SharedPrefs key " + key + ": keep (" + ageInDays +
                                        " days old)");
                            }
                        }
                    }
                }
                editor.putLong(KEY_LAST_FLUSH_TIME_MS, nowTime);
                editor.apply();
            }
        }
    }

    private static int getIntervalInDays(long startMillis, long endMillis, Time timeObj) {
        timeObj.set(startMillis);
        int startDay = Time.getJulianDay(startMillis, timeObj.gmtoff);
        timeObj.set(endMillis);
        return Time.getJulianDay(endMillis, timeObj.gmtoff) - startDay;
    }

    /**
     * M: Remove the event notification for the given event, dismiss the alerts correspondingly
     * @Note this is triggered in the Context context, and the intent data must be unique for the
     * click event across all reminders (so using event ID + startTime should be unique). You'd
     * better not call this function in the EditEventActivity's onCreate() if needed.
     * @Note if eventId is -1, all alarms will be dismissed
     * @param context The context the triggered service run in
     * @param eventId The event id of the given event
     * @param startMillis The start time in milliseconds of start time
     * @param endMillis
     */
    public static void removeEventNotification(Context context, long eventId, long startMillis,
            long endMillis) {
        // If no notification id was found in the map, which means this is a low priority event
        Integer notiId = AlertService.getEventIdToNotificationIdMap().get(eventId);
        Intent deleteIntent = new Intent();
        deleteIntent.setClass(context, DismissAlarmsService.class);
        deleteIntent.putExtra(AlertUtils.EVENT_ID_KEY, eventId);
        deleteIntent.putExtra(AlertUtils.EVENT_START_KEY, startMillis);
        deleteIntent.putExtra(AlertUtils.EVENT_END_KEY, endMillis);
        // This function is always called after we have already opened the event, so
        deleteIntent.putExtra(AlertUtils.EVENT_SHOWED, true);
        // if it's a low priority event, push -1 as the notification id
        if (notiId != null) {
            deleteIntent.putExtra(AlertUtils.NOTIFICATION_ID_KEY, notiId.intValue());
        } else {
            deleteIntent.putExtra(AlertUtils.NOTIFICATION_ID_KEY, -1);
        }

        // Must set a field that affects Intent.filterEquals so that the resulting
        // intent will be a unique instance (the 'extras' don't achieve this).
        // This must be unique for the click event across all reminders (so using
        // event ID + startTime should be unique).  This also must be unique from
        // the delete event (which also uses DismissAlarmsService).
        Uri.Builder builder = Events.CONTENT_URI.buildUpon();
        ContentUris.appendId(builder, eventId);
        ContentUris.appendId(builder, startMillis);
        deleteIntent.setData(builder.build());
        deleteIntent.setAction(DISMISS_ALERT_ACTION);
        context.startService(deleteIntent);
    }

    /**
     * M: Query the FIRED and IGNORED alerts, and post unread number
     * @param context
     */
    public static void postUnreadNumber(Context context) {
        ContentResolver resolver = context.getContentResolver();
        String selection = CalendarAlerts.STATE + "=" + CalendarAlerts.STATE_FIRED
                + " OR " + CalendarAlerts.STATE + "=" + AlertUtils.ALERT_EXT_STATE_IGNORED;
        Cursor cursor = resolver.query(CalendarAlerts.CONTENT_URI, null, selection, null, null);
        int unReadMsgNumber = 0;
        if (cursor != null) {
            try {
                unReadMsgNumber = cursor.getCount();
            } finally {
                cursor.close();
            }
        }
        LogUtil.d(TAG, "WriteUnreadReminders(unReadMsgNumber).");
        MTKUtils.writeUnreadReminders(context, unReadMsgNumber);
    }
}
