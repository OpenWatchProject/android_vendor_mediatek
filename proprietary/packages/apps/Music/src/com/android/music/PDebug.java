package com.android.music;


import android.os.Trace;
import android.os.SystemProperties;


public class PDebug {
    private static Boolean DEBUG = false;

    private static long TRACE_TAG = 0L;

    public static void Start(String msg) {
        if (DEBUG) {
            Trace.traceCounter(TRACE_TAG, "P$" + msg, 1);
        }
    }

    public static void End(String msg) {
        if (DEBUG) {
            Trace.traceCounter(TRACE_TAG, "P$" + msg, 0);
        }
    }

    public static void EndAndStart(String msg1, String msg2) {
        if (DEBUG) {
            Trace.traceCounter(TRACE_TAG, "P$" + msg1, 0);
            Trace.traceCounter(TRACE_TAG, "P$" + msg2, 1);
        }

    }
}
