/*
 * Copyright (C) 2014 The Android Open Source Project
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

package com.mediatek.camera.debug;

import android.os.Build;

/**
 * A wrapper class for AOSP Log.java, All other classes logging should use this class.
 * Provides following features:
 * <p>
 * <li> Add prefix(CAMERA_LOGTAG_PREFIX) for all Tag<li>
 * <li> By setting the system property, you can control the log level can be printed.<li>
 */
public class LogHelper {
    /**
     * All Camera logging using this class will use this tag prefix.
     * Additionally, the prefix itself is checked in isLoggable and
     * serves as an override. So, to toggle all logs allowed by the
     * current {@link Configuration}, you can set properties:
     *
     * adb shell setprop log.tag.CamAp_ VERBOSE
     * adb shell setprop log.tag.CamAp_ ""
     */
    public static final String CAMERA_LOGTAG_PREFIX = "CamAp_";
    private static final LogHelper.Tag TAG = new LogHelper.Tag("Log");

    /**
     * This class restricts the length of the log tag to be less than the
     * framework limit and also prepends the common tag prefix defined by
     * {@code CAMERA_LOGTAG_PREFIX}.
     */
    public static final class Tag {
        // The length limit from Android framework is 23.
        private static final int MAX_TAG_LEN_DEFINED_FRAMEWORK = 23;
        private static final int MAX_TAG_LEN =
                MAX_TAG_LEN_DEFINED_FRAMEWORK - CAMERA_LOGTAG_PREFIX.length();
        final String mValue;

        /**
         * @param tag user-defined tag.
         */
        public Tag(String tag) {
            final int lenDiff = tag.length() - MAX_TAG_LEN;
            if (lenDiff > 0) {
                w(TAG, "Tag " + tag + " is " + lenDiff + " chars longer than limit.");
            }
            mValue = CAMERA_LOGTAG_PREFIX + (lenDiff > 0 ? tag.substring(0, MAX_TAG_LEN) : tag);
        }

        @Override
        public String toString() {
            return mValue;
        }
    }

    /**
    * Send a DEBUG log message.
    * @param tag Used to identify the source of a log message.  It usually identifies
    *        the class or activity where the log call occurs.
    * @param msg The message you would like logged.
    */
    public static void d(Tag tag, String msg) {
        if (isLoggable(tag, android.util.Log.DEBUG)) {
            android.util.Log.d(tag.toString(), msg);
        }
    }

    /**
     * Send a DEBUG log message.
     * @param tag tag Used to identify the source of a log message.
     * @param instance Prefixes a message with with a hashcode tag of the object.
     * @param msg The message you would like logged.
     */
    public static void d(Tag tag, Object instance, String msg) {
        if (isLoggable(tag, android.util.Log.DEBUG)) {
            android.util.Log.d(tag.toString(), LogUtil.addTags(instance, msg));
        }
    }

    /***
     * Send a DEBUG log message.
     * @param tag  Used to identify the source of a log message.
     * @param instance instance Prefixes a message with with a hashcode tag of the object.
     * @param msg  The message you would like logged.
     * @param tags Prefixes a message with the bracketed tags specified in the
     *             tag list.
     */
    public static void d(Tag tag, Object instance, String msg, String tags) {
        if (isLoggable(tag, android.util.Log.DEBUG)) {
            android.util.Log.d(tag.toString(), LogUtil.addTags(instance, msg, tags));
        }
    }

    /**
     * Send a DEBUG log message.
     * @param tag tag tag Used to identify the source of a log message.
     * @param msg The message you would like logged.
     * @param tr An exception to log
     */
    public static void d(Tag tag, String msg, Throwable tr) {
        if (isLoggable(tag, android.util.Log.DEBUG)) {
            android.util.Log.d(tag.toString(), msg, tr);
        }
    }

    /**
     * Send an ERROR log message.
     * @param tag Used to identify the source of a log message.
     * @param msg The message you would like logged.
     */
    public static void e(Tag tag, String msg) {
        if (isLoggable(tag, android.util.Log.ERROR)) {
            android.util.Log.e(tag.toString(), msg);
        }
    }

    /**
     * Send an ERROR log message.
     * @param tag Used to identify the source of a log message.
     * @param instance Prefixes a message with with a hashcode tag of the object.
     * @param msg The message you would like logged.
     */
    public static void e(Tag tag, Object instance, String msg) {
        if (isLoggable(tag, android.util.Log.ERROR)) {
            android.util.Log.e(tag.toString(), LogUtil.addTags(instance, msg));
        }
    }

    /**
     * Send an ERROR log message.
     * @param tag Used to identify the source of a log message.
     * @param instance Prefixes a message with with a hashcode tag of the object.
     * @param msg The message you would like logged.
     * @param tags Prefixes a message with the bracketed tags specified in the
     *             tag list.
     */
    public static void e(Tag tag, Object instance, String msg, String tags) {
        if (isLoggable(tag, android.util.Log.DEBUG)) {
            android.util.Log.e(tag.toString(), LogUtil.addTags(instance, msg, tags));
        }
    }

    /**
     * Send an ERROR log message.
     * @param tag Used to identify the source of a log message.
     * @param msg The message you would like logged.
     * @param tr An exception to log
     */
    public static void e(Tag tag, String msg, Throwable tr) {
        if (isLoggable(tag, android.util.Log.ERROR)) {
            android.util.Log.e(tag.toString(), msg, tr);
        }
    }

    /**
     * Send an INFO log message.
     * @param tag Used to identify the source of a log message.
     * @param msg The message you would like logged.
     */
    public static void i(Tag tag, String msg) {
        if (isLoggable(tag, android.util.Log.INFO)) {
            android.util.Log.i(tag.toString(), msg);
        }
    }

    /**
     * Send an INFO log message.
     * @param tag Used to identify the source of a log message.
     * @param instance Prefixes a message with with a hashcode tag of the object.
     * @param msg The message you would like logged.
     */
    public static void i(Tag tag, Object instance, String msg) {
        if (isLoggable(tag, android.util.Log.INFO)) {
            android.util.Log.i(tag.toString(), LogUtil.addTags(instance, msg));
        }
    }

    /**
     * Send an INFO log message.
     * @param tag Used to identify the source of a log message.
     * @param instance Prefixes a message with with a hashcode tag of the object.
     * @param msg The message you would like logged.
     * @param tags Prefixes a message with the bracketed tags specified in the
     *             tag list.
     */
    public static void i(Tag tag, Object instance, String msg, String tags) {
        if (isLoggable(tag, android.util.Log.DEBUG)) {
            android.util.Log.i(tag.toString(), LogUtil.addTags(instance, msg, tags));
        }
    }

    /**
     * Send an INFO log message.
     * @param tag Used to identify the source of a log message.
     * @param msg The message you would like logged.
     * @param tr An exception to log
     */
    public static void i(Tag tag, String msg, Throwable tr) {
        if (isLoggable(tag, android.util.Log.INFO)) {
            android.util.Log.i(tag.toString(), msg, tr);
        }
    }

    /**
     * Send an VERBOSE log message.
     * @param tag Used to identify the source of a log message.
     * @param msg The message you would like logged.
     */
    public static void v(Tag tag, String msg) {
        if (isLoggable(tag, android.util.Log.VERBOSE)) {
            android.util.Log.v(tag.toString(), msg);
        }
    }

    /**
     * Send an VERBOSE log message.
     * @param tag Used to identify the source of a log message.
     * @param instance Prefixes a message with with a hashcode tag of the object.
     * @param msg The message you would like logged.
     */
    public static void v(Tag tag, Object instance, String msg) {
        if (isLoggable(tag, android.util.Log.VERBOSE)) {
            android.util.Log.v(tag.toString(), LogUtil.addTags(instance, msg));
        }
    }

    /**
     * Send an VERBOSE log message.
     * @param tag Used to identify the source of a log message.
     * @param instance Prefixes a message with with a hashcode tag of the object.
     * @param msg The message you would like logged.
     * @param tags Prefixes a message with the bracketed tags specified in the
     *             tag list.
     */
    public static void v(Tag tag, Object instance, String msg, String tags) {
        if (isLoggable(tag, android.util.Log.DEBUG)) {
            android.util.Log.v(tag.toString(), LogUtil.addTags(instance, msg, tags));
        }
    }

    /**
     * Send an VERBOSE log message.
     * @param tag Used to identify the source of a log message.
     * @param msg The message you would like logged.
     * @param tr An exception to log
     */
    public static void v(Tag tag, String msg, Throwable tr) {
        if (isLoggable(tag, android.util.Log.VERBOSE)) {
            android.util.Log.v(tag.toString(), msg, tr);
        }
    }

    /**
     * Send an WARNNING log message.
     * @param tag Used to identify the source of a log message.
     * @param msg The message you would like logged.
     */
    public static void w(Tag tag, String msg) {
        if (isLoggable(tag, android.util.Log.WARN)) {
            android.util.Log.w(tag.toString(), msg);
        }
    }

    /**
     * Send an WARNNING log message.
     * @param tag Used to identify the source of a log message.
     * @param instance Prefixes a message with with a hashcode tag of the object.
     * @param msg The message you would like logged.
     */
    public static void w(Tag tag, Object instance, String msg) {
        if (isLoggable(tag, android.util.Log.WARN)) {
            android.util.Log.w(tag.toString(), LogUtil.addTags(instance, msg));
        }
    }

    /**
     * Send an WARNNING log message.
     * @param tag Used to identify the source of a log message.
     * @param instance Prefixes a message with with a hashcode tag of the object.
     * @param msg The message you would like logged.
     * @param tags Prefixes a message with the bracketed tags specified in the
     *             tag list.
     */
    public static void w(Tag tag, Object instance, String msg, String tags) {
        if (isLoggable(tag, android.util.Log.DEBUG)) {
            android.util.Log.w(tag.toString(), LogUtil.addTags(instance, msg, tags));
        }
    }

    /**
     * Send an WARNNING log message.
     * @param tag Used to identify the source of a log message.
     * @param msg The message you would like logged.
     * @param tr An exception to log
     */
    public static void w(Tag tag, String msg, Throwable tr) {
        if (isLoggable(tag, android.util.Log.WARN)) {
            android.util.Log.w(tag.toString(), msg, tr);
        }
    }

    private static boolean isLoggable(Tag tag, int level) {
        try {
            if (LogUtil.getOverrideLevelFromProperty() > -1
                    || LogUtil.getPersistLevelFromProperty() > -1) {
                // Self-defined log level are:
                // MTKCAM_LOG_LEVEL_ERROR   = 0;
                // MTKCAM_LOG_LEVEL_WARNING = 1;
                // MTKCAM_LOG_LEVEL_INFO    = 2;
                // MTKCAM_LOG_LEVEL_DEBUG   = 3;
                // MTKCAM_LOG_LEVEL_VERBOS  = 4;
                // Only loggable when current log level <= property's level.
                // For example: when property's level is set to 2 by
                // "adb shell setprop vendor.debug.mtkcam.loglevel 2",
                // only ERROR, WARNING, INFO  can loggable.
                boolean logLevelOpened = LogUtil.getLogLevelFromSystemLevel(level)
                        <= LogUtil.getOverrideLevelFromProperty();
                boolean logLevelPerisitOpened = LogUtil.getLogLevelFromSystemLevel(level)
                        <= LogUtil.getPersistLevelFromProperty();
                return logLevelOpened || logLevelPerisitOpened;
            }
            return isDebugOsBuild() || shouldLog(tag, level);
        } catch (IllegalArgumentException ex) {
            e(TAG, "Tag too long:" + tag);
            return false;
        }
    }

    private static boolean shouldLog(Tag tag, int level) {
        // The prefix can be used as an override tag to see all camera logs
        return android.util.Log.isLoggable(CAMERA_LOGTAG_PREFIX, level)
                || android.util.Log.isLoggable(tag.toString(), level);
    }

    private static boolean isDebugOsBuild() {
        return "userdebug".equals(Build.TYPE) || "eng".equals(Build.TYPE);
    }
}