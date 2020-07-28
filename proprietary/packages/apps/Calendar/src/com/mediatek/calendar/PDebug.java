package com.mediatek.calendar;


import android.os.Trace;
import android.os.SystemProperties;


public class PDebug {
    private static final String MSG_HEADER = "Calendar.";

    private static Boolean DEBUG = true;

    private static long TRACE_TAG = 0L;

    static {
        DEBUG = !SystemProperties.get("vendor.ap.performance.debug", "0").equals("0");
        if (DEBUG) {
            TRACE_TAG = 1L << Long.parseLong(SystemProperties.get("ap.performance.debug"));
        }
    }

    public static void Start(String msg) {
        if (DEBUG) {
            Trace.traceCounter(TRACE_TAG, "P$" + MSG_HEADER + msg, 1);
        }
    }

    public static void End(String msg) {
        if (DEBUG) {
            Trace.traceCounter(TRACE_TAG, "P$" + MSG_HEADER + msg, 0);
        }
    }

    public static void EndAndStart(String msg1, String msg2) {
        if (DEBUG) {
            Trace.traceCounter(TRACE_TAG, "P$" + MSG_HEADER + msg1, 0);
            Trace.traceCounter(TRACE_TAG, "P$" + MSG_HEADER + msg2, 1);
        }
    }
}
