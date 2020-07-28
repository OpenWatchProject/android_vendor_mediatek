/*
 * Copyright (c) 2013 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */
package com.trustonic.util;

import android.util.Log;

public class tLog {
    /*
     * Enable log with the command: adb shell setprop log.tag.YOUR_APP_TAG LEVEL
     * e.g adb shell setprop log.tag.PinpadHomeActivity DEBUG
     */
    public static void d(String tag, String msg) {
        if (Log.isLoggable(tag, Log.DEBUG)) {
            Log.d(tag, msg);
        }
    }

    public static void i(String tag, String msg) {
        if (Log.isLoggable(tag, Log.INFO)) {
            Log.i(tag, msg);
        }
    }

    public static void e(String tag, String msg) {
        if (Log.isLoggable(tag, Log.ERROR)) {
            Log.e(tag, msg);
        }
    }

    public static void v(String tag, String msg) {
        if (Log.isLoggable(tag, Log.VERBOSE)) {
            Log.v(tag, msg);
        }
    }

    public static void w(String tag, String msg) {
        if (Log.isLoggable(tag, Log.WARN)) {
            Log.w(tag, msg);
        }
    }
}
