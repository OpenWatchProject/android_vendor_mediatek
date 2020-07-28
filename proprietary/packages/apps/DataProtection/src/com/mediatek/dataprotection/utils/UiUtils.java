package com.mediatek.dataprotection.utils;

import android.content.Context;
import android.text.format.DateUtils;
import android.text.format.Time;
import android.widget.Toast;

import com.mediatek.dataprotection.utils.Log;

public class UiUtils {
    private static Toast mToast = null;
    private static final String TAG = "UiUtils";

    private static Toast getToast(Context context, String text) {
        if (mToast == null) {
            mToast = Toast.makeText(context.getApplicationContext(), text,
                    Toast.LENGTH_SHORT);
        } else {
            mToast.setText(text);
        }
        return mToast;
    }

    public static void showToast(Context context, String text) {
        Log.d(TAG, "showToast: " + context + " " + text);
        if (context != null && text != null) {
            getToast(context, text).show();
        }
    }

    public static String formatTimeStampString(Context context, long when) {
        return formatTimeStampString(context, when, false);
    }

    public static String formatTimeStampString(Context context, long when,
            boolean fullFormat) {
        Time then = new Time();
        then.set(when);
        Time now = new Time();
        now.setToNow();

        // Basic settings for formatDateTime() we want for all cases.
        int format_flags = DateUtils.FORMAT_NO_NOON_MIDNIGHT |
                DateUtils.FORMAT_CAP_AMPM;

        // If the message is from a different year, show the date and year.
        if (then.year != now.year) {
            format_flags |= DateUtils.FORMAT_SHOW_YEAR
                    | DateUtils.FORMAT_SHOW_DATE;
        } else if (then.yearDay != now.yearDay) {
            // If it is from a different day than today, show only the date.
            format_flags |= DateUtils.FORMAT_SHOW_DATE;
        } else {
            // Otherwise, if the message is from today, show the time.
            format_flags |= DateUtils.FORMAT_SHOW_TIME;
        }

        // If the caller has asked for full details, make sure to show the date
        // and time no matter what we've determined above (but still make
        // showing
        // the year only happen if it is a different year from today).
        if (fullFormat) {
            format_flags |= (DateUtils.FORMAT_SHOW_DATE | DateUtils.FORMAT_SHOW_TIME);
        }

        return DateUtils.formatDateTime(context, when, format_flags);
    }

}
