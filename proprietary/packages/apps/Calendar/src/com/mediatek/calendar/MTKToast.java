package com.mediatek.calendar;

import android.content.Context;
import android.widget.Toast;

public class MTKToast {
    private static Toast sToast;
    private static final String DEFAULT_TOAST_STRING = "MTKToast";
    public static final int LENGTH_LONG = Toast.LENGTH_LONG;
    public static final int LENGTH_SHORT = Toast.LENGTH_SHORT;

    private MTKToast(){
        //do nothing
    }

    /**
     * Toast with default duration,LENGTH_SHORT
     * @param context
     * @param msg The message to toast
     */
    public static void toast(Context context, String msg) {
        toast(context, msg, LENGTH_SHORT);
    }

/**
    * Toast with default duration,LENGTH_SHORT
    * @param context
    * @param resId The string id.
    */
    public static void toast(Context context, int resId) {
        toast(context, resId, LENGTH_SHORT);
    }

    /**
     *
     * @param context
     * @param resId The string id
     * @param duration The toast duration
     */
    public static void toast(Context context, int resId, int duration) {
        toast(context, context.getResources().getString(resId), LENGTH_SHORT);
    }

    /**
     *
     * @param context
     * @param msg The message to toast
     * @param duration The toast duration
     */
    public static void toast(Context context, String msg, int duration) {
        Toast toast = getToast(context, msg, duration);
        toast.show();
    }

    /**
     * return a toast object for toast
     * @param context
     * @param msg
     * @param duration
     * @return
     */
    private static Toast getToast(Context context, String msg, int duration) {
        if (sToast == null) {
            sToast = Toast.makeText(context.getApplicationContext(), DEFAULT_TOAST_STRING, duration);
        }
        sToast.setText(msg);
        sToast.setDuration(duration);
        return sToast;
    }
}
