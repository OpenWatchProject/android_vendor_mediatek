/*
 * Copyright (C) 2008 The Android Open Source Project
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

package com.android.music;

import android.util.Log;

import android.os.SystemProperties;

public class MusicLogUtils {
    private static final boolean MUSIC_XLOG_ENABLED = false;

    private static final String TAG = "MusicLog";

    private static final boolean LOG_ENABLE =
            SystemProperties.get("ro.build.type").equals("eng") || Log.isLoggable(TAG, Log.DEBUG);

    /**
     * M: If xlog enabled, use xlog, otherwise use default log.
     *
     * @param tag Log tag
     * @param msg Log message
     * @return If log success, return true, otherwise false.
     */
    public static final int v(String tag, String msg) {
       int result = 0;
       if (LOG_ENABLE) {

            result = Log.v(tag, msg);
       }
        return result;

    }

    /**
     * M: If xlog enabled, use xlog, otherwise use default log.
     *
     * @param tag Log tag
     * @param msg Log message
     * @return If log success, return true, otherwise false.
     */
    public static final int v(String tag, String msg, Throwable tr) {
        int result = 0;
        if (LOG_ENABLE) {
            result = Log.v(tag, msg, tr);
        }
        return result;
    }

    /**
     * M: If xlog enabled, use xlog, otherwise use default log.
     *
     * @param tag Log tag
     * @param msg Log message
     * @return If log success, return true, otherwise false.
     */
    public static final int d(String tag, String msg) {
        int result = 0;
        if (LOG_ENABLE) {
            result = Log.d(tag, msg);
        }
        return result;
    }

    /**
     * M: If xlog enabled, use xlog, otherwise use default log.
     *
     * @param tag Log tag
     * @param msg Log message
     * @return If log success, return true, otherwise false.
     */
    public static final int d(String tag, String msg, Throwable tr) {
        int result = 0;
        if (LOG_ENABLE) {
            result = Log.d(tag, msg, tr);
        }
        return result;
    }

    /**
     * M: If xlog enabled, use xlog, otherwise use default log.
     *
     * @param tag Log tag
     * @param msg Log message
     * @return If log success, return true, otherwise false.
     */
    public static final int i(String tag, String msg) {
        int result;

            result = Log.i(tag, msg);

        return result;
    }

    /**
     * M: If xlog enabled, use xlog, otherwise use default log.
     *
     * @param tag Log tag
     * @param msg Log message
     * @return If log success, return true, otherwise false.
     */
    public static final int i(String tag, String msg, Throwable tr) {
        int result;

            result = Log.i(tag, msg, tr);

        return result;
    }

    /**
     * M: If xlog enabled, use xlog, otherwise use default log.
     *
     * @param tag Log tag
     * @param msg Log message
     * @return If log success, return true, otherwise false.
     */
    public static final int w(String tag, String msg) {
        int result;

            result = Log.w(tag, msg);

        return result;
    }

    /**
     * M: If xlog enabled, use xlog, otherwise use default log.
     *
     * @param tag Log tag
     * @param msg Log message
     * @return If log success, return true, otherwise false.
     */
    public static final int w(String tag, String msg, Throwable tr) {
        int result;

            result = Log.w(tag, msg, tr);

        return result;
    }

    /**
     * M: If xlog enabled, use xlog, otherwise use default log.
     *
     * @param tag Log tag
     * @param msg Log message
     * @return If log success, return true, otherwise false.
     */
    public static final int e(String tag, String msg) {
        int result;

            result = Log.e(tag, msg);

        return result;
    }

    /**
     * M: If xlog enabled, use xlog, otherwise use default log.
     *
     * @param tag Log tag
     * @param msg Log message
     * @return If log success, return true, otherwise false.
     */
    public static final int e(String tag, String msg, Throwable tr) {
        int result;

            result = Log.e(tag, msg, tr);

        return result;
    }
}
