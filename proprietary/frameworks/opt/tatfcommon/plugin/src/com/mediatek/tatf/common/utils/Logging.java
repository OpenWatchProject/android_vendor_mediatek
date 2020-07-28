/*
 * Copyright (C) 2006-2008 The Android Open Source Project
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
 * limitations under the License.
 */
package com.mediatek.tatf.common.utils;

import android.util.Log;

/**
 *
 *Log for TatfServer, use 'TatfServer' to filter Tatf case log.
 */
public class Logging {

    public static final String TAG = "TatfServer";

    /**
     * Priority constant for the println method; use LogUtils.v.
     */
    public static final int VERBOSE = Log.VERBOSE;

    /**
     * Priority constant for the println method; use LogUtils.d.
     */
    public static final int DEBUG = Log.DEBUG;

    /**
     * Priority constant for the println method; use LogUtils.i.
     */
    public static final int INFO = Log.INFO;

    /**
     * Priority constant for the println method; use LogUtils.w.
     */
    public static final int WARN = Log.WARN;

    /**
     * Priority constant for the println method; use LogUtils.e.
     */
    public static final int ERROR = Log.ERROR;

    /**
     * Send a {@link #VERBOSE} log message.
     *
     * @param tag
     *            Used to identify the source of a log message. It usually
     *            identifies the class or activity where the log call occurs.
     * @param format
     *            the format string (see {@link java.util.Formatter#format})
     * @param args
     *            the list of arguments passed to the formatter. If there are
     *            more arguments than required by {@code format}, additional
     *            arguments are ignored.
     */
    public static int v(String format, Object... args) {
        try {
            return Log.v(TAG, String.format(format, args));
        } catch (Exception e) {
            return Log.v(TAG, format + ": !!!! Log format exception: ", e);
        }
    }

    /**
     * Send a {@link #DEBUG} log message.
     *
     * @param tag
     *            Used to identify the source of a log message. It usually
     *            identifies the class or activity where the log call occurs.
     * @param format
     *            the format string (see {@link java.util.Formatter#format})
     * @param args
     *            the list of arguments passed to the formatter. If there are
     *            more arguments than required by {@code format}, additional
     *            arguments are ignored.
     */
    public static int d(String format, Object... args) {
        try {
            return Log.d(TAG, String.format(format, args));
        } catch (Exception e) {
            return Log.d(TAG, format + ": !!!! Log format exception: ", e);
        }
    }

    /**
     * Send a {@link #INFO} log message.
     *
     * @param tag
     *            Used to identify the source of a log message. It usually
     *            identifies the class or activity where the log call occurs.
     * @param format
     *            the format string (see {@link java.util.Formatter#format})
     * @param args
     *            the list of arguments passed to the formatter. If there are
     *            more arguments than required by {@code format}, additional
     *            arguments are ignored.
     */
    public static int i(String format, Object... args) {
        try {
            return Log.i(TAG, String.format(format, args));
        } catch (Exception e) {
            return Log.i(TAG, format + ": !!!! Log format exception: ", e);
        }
    }

    /**
     * Send a {@link #WARN} log message.
     *
     * @param tag
     *            Used to identify the source of a log message. It usually
     *            identifies the class or activity where the log call occurs.
     * @param format
     *            the format string (see {@link java.util.Formatter#format})
     * @param args
     *            the list of arguments passed to the formatter. If there are
     *            more arguments than required by {@code format}, additional
     *            arguments are ignored.
     */
    public static int w(String format, Object... args) {
        try {
            return Log.w(TAG, String.format(format, args));
        } catch (Exception e) {
            return Log.w(TAG, format + ": !!!! Log format exception: ", e);
        }
    }

    /**
     * Send a {@link #ERROR} log message.
     *
     * @param tag
     *            Used to identify the source of a log message. It usually
     *            identifies the class or activity where the log call occurs.
     * @param format
     *            the format string (see {@link java.util.Formatter#format})
     * @param args
     *            the list of arguments passed to the formatter. If there are
     *            more arguments than required by {@code format}, additional
     *            arguments are ignored.
     */
    public static int e(String format, Object... args) {
        try {
            return Log.e(TAG, String.format(format, args));
        } catch (Exception e) {
            return Log.e(TAG, format + ": !!!! Log format exception: ", e);
        }
    }

    /**
     * TatfServer internal log.
     * @param format
     * @param args
     */
    public static void serverProxy(String format, Object... args) {
        try {
            Log.i(TAG, "[SERVER-IPC] " + String.format(format, args));
        } catch (Exception e) {
            Log.i(TAG, "[SERVER-IPC] " + format + ": !!!! Log format exception: ", e);
        }
    }

    /**
     * TatfServer client log, e.g. Service in P or Plugin.
     * @param format
     * @param args
     */
    public static void clientProxy(String format, Object... args) {
        try {
            Log.i(TAG, "[CLIENT-IPC] " + String.format(format, args));
        } catch (Exception e) {
            Log.i(TAG, "[CLIENT-IPC] " + format + ": !!!! Log format exception: ", e);
        }
    }

}

