package com.mediatek.calendar;

import android.content.ComponentName;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.SystemProperties;
import android.util.Log;
import android.widget.Toast;

import com.android.calendar.R;

public final class MTKUtils {

    ///M: add for iCalendar sharing. @{
    public static final String VCALENDAR_TYPE = "text/x-vcalendar";
    public static final String VCALENDAR_URI = "content://com.mediatek.calendarimporter/";
    private static final String TAG = "MTKUtils";
    ///@}

    //M:#unread message# unread message the key in unread_support_shortcuts.xml defined in Launcher{
    //private static final String CALENDAR_MTK_UNREAD_KEY = "com_android_calendar_mtk_unread";
    //private static final String MTK_ACTION_UNREAD_CHANGED = Intent.ACTION_UNREAD_CHANGED;
    //private static final String MTK_EXTRA_UNREAD_NUMBER = Intent.EXTRA_UNREAD_NUMBER;
    //private static final String MTK_EXTRA_UNREAD_COMPONENT = Intent.EXTRA_UNREAD_COMPONENT;
    ///@}

    public static boolean isEventShareAvailable(Context context) {
        String type = context.getContentResolver().getType(Uri.parse(VCALENDAR_URI));
        return VCALENDAR_TYPE.equalsIgnoreCase(type);
    }

    /**
     * M: Share event by event id.
     * @param context
     * @param eventId event id
     */
    public static void sendShareEvent(Context context, long eventId) {
        Log.i(TAG, "Utils.sendShareEvent() eventId=" + eventId);
        Intent intent = new Intent(Intent.ACTION_SEND);
        intent.putExtra(Intent.EXTRA_STREAM, Uri.parse(VCALENDAR_URI + eventId));
        intent.setType(VCALENDAR_TYPE);
        // Creating chooser to ensure correct information shows in sharing UI.
        try {
            context.startActivity(Intent.createChooser(intent, null));
            } catch (android.content.ActivityNotFoundException ex) {
                // TODO add a toast to notify user
                Log.i(TAG, "No way to share.");
            }
    }

    /**
     * M:#unread message# update the unread message number @{
     * @param context
     * @param numReminders
     */
    public static void writeUnreadReminders(Context context, int numReminders) {
        LogUtil.d(TAG, "Write and broadcast Unread Reminders. num=" + numReminders);
        //sendUnreadBroadcast(context, numReminders);
        //ContentResolver resolver = context.getContentResolver();
        //boolean result = android.provider.Settings.System.putInt
        //        (resolver, CALENDAR_MTK_UNREAD_KEY, numReminders);
        //LogUtil.i(TAG, "Write Unread Reminders to Setting, success:" + result);
    }

    /**
     * M: #unread message# send broadcast for unread message
     * @param context
     * @param numReminders
     */
     /*
    private static void sendUnreadBroadcast(Context context, int numReminders) {
        Intent intent = new Intent();
        intent.setAction(MTK_ACTION_UNREAD_CHANGED);
        intent.putExtra(MTK_EXTRA_UNREAD_NUMBER, Integer.valueOf(numReminders));
        intent.putExtra(MTK_EXTRA_UNREAD_COMPONENT,
                new ComponentName(context, "com.android.calendar.AllInOneActivity"));
        context.sendBroadcast(intent);
        LogUtil.d(TAG, "Send unread broadcast.component = "
                + intent.getParcelableExtra(MTK_EXTRA_UNREAD_COMPONENT)
                       + "; unread number=" + intent.getIntExtra(MTK_EXTRA_UNREAD_NUMBER, -1));
    }
    */

    /**
     * check the storage state.
     * @param context the context
     * @return if it is in low storage state, return true
     */
    public static boolean isLowStorage(Context context) {
        return false;
    }

    /**
     * toast insufficient space when save event under low storage state.
     * @param context the context
     */
    public static void toastLowStorage(Context context) {
        Toast.makeText(context, R.string.low_storage_save_failed, Toast.LENGTH_LONG).show();
    }
}
