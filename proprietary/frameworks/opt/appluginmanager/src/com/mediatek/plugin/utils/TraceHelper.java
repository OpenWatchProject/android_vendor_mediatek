package com.mediatek.plugin.utils;

import android.os.Build;
import android.os.Trace;

import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

public class TraceHelper {
    private static final String TAG = "PluginManager/TraceHelper";

    private static boolean sHasCheck = false;
    private static boolean sSupportTrace =
            Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR2;
    private static Method sTraceBeginMethod;
    private static Method sTraceEndMethod;
    private static long sViewTag;

    /**
     * Writes a trace message to indicate that a given section of code has begun.
     *
     * @param sectionName The name of the code section to appear in the trace.
     */
    public static void beginSection(String sectionName) {
        checkWhetherSupport();
        if (sTraceBeginMethod != null && sTraceEndMethod != null) {
            try {
                sTraceBeginMethod.invoke(null, sViewTag, sectionName);
            } catch (IllegalAccessException e) {
                Log.d(TAG, "<beginSection> IllegalAccessException", e);
            } catch (InvocationTargetException e) {
                Log.d(TAG, "<beginSection> InvocationTargetException", e);
            }
        } else if (sSupportTrace) {
            Trace.beginSection(sectionName);
        }
    }

    /**
     * Writes a trace message to indicate that a given section of code has ended.
     */
    public static void endSection() {
        checkWhetherSupport();
        if (sTraceBeginMethod != null && sTraceEndMethod != null) {
            try {
                sTraceEndMethod.invoke(null, sViewTag);
            } catch (IllegalAccessException e) {
                Log.d(TAG, "<endSection> IllegalAccessException", e);
            } catch (InvocationTargetException e) {
                Log.d(TAG, "<endSection> InvocationTargetException", e);
            }
        } else if (sSupportTrace) {
            Trace.endSection();
        }
    }

    private static void checkWhetherSupport() {
        if (sHasCheck) {
            return;
        }
        if (!sSupportTrace) {
            sHasCheck = true;
            return;
        }
        try {
            sTraceBeginMethod = Trace.class.getDeclaredMethod("traceBegin", long.class,
                    String.class);
            sTraceBeginMethod.setAccessible(true);

            sTraceEndMethod = Trace.class.getDeclaredMethod("traceEnd", long.class);
            sTraceEndMethod.setAccessible(true);

            Field viewTagFiled = Trace.class.getDeclaredField("TRACE_TAG_VIEW");
            viewTagFiled.setAccessible(true);
            sViewTag = viewTagFiled.getLong(null);

        } catch (NoSuchMethodException e) {
            Log.d(TAG, "<checkWhetherSupport> NoSuchMethodException", e);
        } catch (NoSuchFieldException e) {
            Log.d(TAG, "<checkWhetherSupport> NoSuchFieldException", e);
        } catch (IllegalAccessException e) {
            Log.d(TAG, "<checkWhetherSupport> IllegalAccessException", e);
        }
        sHasCheck = true;
        return;
    }
}