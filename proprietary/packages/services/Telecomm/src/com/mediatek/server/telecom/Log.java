/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.server.telecom;

import android.content.Context;
import android.os.Build;
import android.os.SystemProperties;
import android.telecom.Logging.EventManager;
import android.telecom.Logging.Session;
import android.telecom.Logging.SessionManager;
import android.text.TextUtils;

import com.android.internal.annotations.VisibleForTesting;
import com.android.internal.util.IndentingPrintWriter;

import java.util.IllegalFormatException;
import java.util.Locale;

public class Log {

    private static final long EXTENDED_LOGGING_DURATION_MILLIS = 60000 * 30; // 30 minutes
    private static final String KEY_FORCE_LOGGING_ON = "persist.vendor.log.tel_dbg";

    // Generic tag for all Telecom logging
    @VisibleForTesting
    public static String TAG = "Telecom";
    public static boolean DEBUG = android.telecom.Log.DEBUG;
    public static boolean INFO = android.telecom.Log.INFO;
    public static boolean VERBOSE = android.telecom.Log.VERBOSE;
    public static boolean WARN = android.telecom.Log.WARN;
    public static boolean ERROR = android.telecom.Log.ERROR;

    /**
     * Tracks whether user-activated extended logging is enabled.
     */
    private static boolean sIsUserExtendedLoggingEnabled = false;

    /**
     * The time when user-activated extended logging should be ended.  Used to determine when
     * extended logging should automatically be disabled.
     */
    private static long sUserExtendedLoggingStopTime = 0;

    public static void d(String subTag, String format, Object... args) {
        if (sIsUserExtendedLoggingEnabled) {
            maybeDisableLogging();
            android.util.Slog.i(buildTag(subTag), buildMessage(format, args));
        } else if (DEBUG) {
            android.util.Slog.d(buildTag(subTag), buildMessage(format, args));
        }
    }

    public static void d(Object objectSubTag, String format, Object... args) {
        if (sIsUserExtendedLoggingEnabled) {
            maybeDisableLogging();
            android.util.Slog.i(buildTag(objectSubTag), buildMessage(format, args));
        } else if (DEBUG) {
            android.util.Slog.d(buildTag(objectSubTag), buildMessage(format, args));
        }
    }

    public static void i(String subTag, String format, Object... args) {
        if (INFO) {
            android.util.Slog.i(buildTag(subTag), buildMessage(format, args));
        }
    }

    public static void i(Object objectSubTag, String format, Object... args) {
        if (INFO) {
            android.util.Slog.i(buildTag(objectSubTag), buildMessage(format, args));
        }
    }

    public static void v(String subTag, String format, Object... args) {
        if (sIsUserExtendedLoggingEnabled) {
            maybeDisableLogging();
            android.util.Slog.i(buildTag(subTag), buildMessage(format, args));
        } else if (VERBOSE) {
            android.util.Slog.v(buildTag(subTag), buildMessage(format, args));
        }
    }

    public static void v(Object objectSubTag, String format, Object... args) {
        if (sIsUserExtendedLoggingEnabled) {
            maybeDisableLogging();
            android.util.Slog.i(buildTag(objectSubTag), buildMessage(format, args));
        } else if (VERBOSE) {
            android.util.Slog.v(buildTag(objectSubTag), buildMessage(format, args));
        }
    }

    public static void w(String subTag, String format, Object... args) {
        if (WARN) {
            android.util.Slog.w(buildTag(subTag), buildMessage(format, args));
        }
    }

    public static void w(Object objectSubTag, String format, Object... args) {
        if (WARN) {
            android.util.Slog.w(buildTag(objectSubTag), buildMessage(format, args));
        }
    }

    public static void e(String subTag, Throwable tr, String format, Object... args) {
        if (ERROR) {
            android.util.Slog.e(buildTag(subTag), buildMessage(format, args), tr);
        }
    }

    public static void e(Object objectSubTag, Throwable tr, String format, Object... args) {
        if (ERROR) {
            android.util.Slog.e(buildTag(objectSubTag), buildMessage(format, args),
                    tr);
        }
    }

    public static void wtf(String subTag, Throwable tr, String format, Object... args) {
        android.util.Slog.wtf(buildTag(subTag), buildMessage(format, args), tr);
    }

    public static void wtf(Object objectSubTag, Throwable tr, String format, Object... args) {
        android.util.Slog.wtf(buildTag(objectSubTag), buildMessage(format, args),
                tr);
    }

    public static void wtf(String subTag, String format, Object... args) {
        String msg = buildMessage(format, args);
        android.util.Slog.wtf(buildTag(subTag), msg, new IllegalStateException(msg));
    }

    public static void wtf(Object objectSubTag, String format, Object... args) {
        String msg = buildMessage(format, args);
        android.util.Slog.wtf(buildTag(objectSubTag), msg, new IllegalStateException(msg));
    }

        /**
         * The ease of use methods below only act mostly as proxies to the Session and Event
         * Loggers. They also control the lazy loaders of the singleton instances, which will never
         *  be loaded if the proxy methods aren't used.
         *
         * Please see each method's documentation inside of their respective implementations in the
         * loggers.
         */
        public static void setSessionContext(Context context) {
            android.telecom.Log.setSessionContext(context);
        }

        public static void startSession(String shortMethodName) {
            android.telecom.Log.startSession(shortMethodName);
        }

        public static void startSession(Session.Info info, String shortMethodName) {
            android.telecom.Log.startSession(info, shortMethodName);
        }

        public static void startSession(String shortMethodName, String callerIdentification) {
            android.telecom.Log.startSession(shortMethodName, callerIdentification);
        }

        public static void startSession(Session.Info info, String shortMethodName,
                String callerIdentification) {
            android.telecom.Log.startSession(info, shortMethodName, callerIdentification);
        }

        public static Session createSubsession() {
            return android.telecom.Log.createSubsession();
        }

        public static Session.Info getExternalSession() {
            return android.telecom.Log.getExternalSession();
        }

        public static void cancelSubsession(Session subsession) {
            android.telecom.Log.cancelSubsession(subsession);
        }

        public static void continueSession(Session subsession, String shortMethodName) {
            android.telecom.Log.continueSession(subsession, shortMethodName);
        }

        public static void endSession() {
            android.telecom.Log.endSession();
        }

        public static void registerSessionListener(SessionManager.ISessionListener l) {
            android.telecom.Log.registerSessionListener(l);
        }

        public static String getSessionId() {
            return android.telecom.Log.getSessionId();
        }

        public static void addEvent(EventManager.Loggable recordEntry, String event) {
            android.telecom.Log.addEvent(recordEntry, event);
        }

        public static void addEvent(EventManager.Loggable recordEntry, String event, Object data) {
            android.telecom.Log.addEvent(recordEntry, event, data);
        }

        public static void addEvent(EventManager.Loggable recordEntry, String event, String format,
                Object... args) {
            android.telecom.Log.addEvent(recordEntry, event, format, args);
        }

        public static void registerEventListener(EventManager.EventListener e) {
            android.telecom.Log.registerEventListener(e);
        }

        public static void addRequestResponsePair(EventManager.TimedEventPair p) {
            android.telecom.Log.addRequestResponsePair(p);
        }

        public static void dumpEvents(IndentingPrintWriter pw) {
            android.telecom.Log.dumpEvents(pw);
        }

        /**
         * Dumps the events in a timeline format.
         * @param pw The {@link IndentingPrintWriter} to write to.
         * @hide
         */
        public static void dumpEventsTimeline(IndentingPrintWriter pw) {
            android.telecom.Log.dumpEventsTimeline(pw);
        }

        /**
         * Enable or disable extended telecom logging.
         *
         * @param isExtendedLoggingEnabled {@code true} if extended logging should be enabled,
         *          {@code false} if it should be disabled.
         */
        public static void setIsExtendedLoggingEnabled(boolean isExtendedLoggingEnabled) {
            android.telecom.Log.setIsExtendedLoggingEnabled(isExtendedLoggingEnabled);

            // If the state hasn't changed, bail early.
            if (sIsUserExtendedLoggingEnabled == isExtendedLoggingEnabled) {
                return;
            }

            sIsUserExtendedLoggingEnabled = isExtendedLoggingEnabled;
            if (sIsUserExtendedLoggingEnabled) {
                sUserExtendedLoggingStopTime = System.currentTimeMillis()
                        + EXTENDED_LOGGING_DURATION_MILLIS;
            } else {
                sUserExtendedLoggingStopTime = 0;
            }
        }

        @VisibleForTesting
        public static SessionManager getSessionManager() {
            return android.telecom.Log.getSessionManager();
        }

        public static void setTag(String tag) {
            android.telecom.Log.setTag(tag);

            TAG = android.telecom.Log.TAG;
            DEBUG = android.telecom.Log.DEBUG;
            INFO = android.telecom.Log.INFO;
            VERBOSE = android.telecom.Log.VERBOSE;
            WARN = android.telecom.Log.WARN;
            ERROR = android.telecom.Log.ERROR;
        }

        /**
         *  Change Log level to print more logs.
         */
        public static void setupLoggableFlags() {
            android.telecom.Log.ERROR = true;
            android.telecom.Log.WARN = true;
            android.telecom.Log.INFO = true;
            if (Build.IS_ENG || SystemProperties.getInt(KEY_FORCE_LOGGING_ON, 0) > 0) {
                android.telecom.Log.DEBUG = true;
            }
            // Force set to false to avoid sensitive information leak. In android.telecom.Log,
            // pii and piiHandle methods will not encrypt string if VERBOSE equals true.
            android.telecom.Log.VERBOSE = false;

            Log.ERROR = true;
            Log.WARN = true;
            Log.INFO = true;
            if (Build.IS_ENG || SystemProperties.getInt(KEY_FORCE_LOGGING_ON, 0) > 0) {
                Log.DEBUG = true;
                Log.VERBOSE = true;
            }
        }

        public static boolean isLoggable(int level) {
            return android.telecom.Log.isLoggable(level);
        }

        public static String piiHandle(Object pii) {
            return android.telecom.Log.piiHandle(pii);
        }

        /**
         * Redact personally identifiable information for production users.
         * If we are running in verbose mode, return the original string,
         * and return "***" otherwise.
         */
        public static String pii(Object pii) {
            return android.telecom.Log.pii(pii);
        }

        /**
         * If user enabled extended logging is enabled and the time limit has passed, disables the
         * extended logging.
         */
        private static void maybeDisableLogging() {
            if (!sIsUserExtendedLoggingEnabled) {
                return;
            }

            if (sUserExtendedLoggingStopTime < System.currentTimeMillis()) {
                sUserExtendedLoggingStopTime = 0;
                sIsUserExtendedLoggingEnabled = false;
            }
        }

    private static String buildMessage(String format, Object... args) {
        // Incorporate thread ID and calling method into prefix
        String sessionName = getSessionId();
        String sessionPostfix = TextUtils.isEmpty(sessionName) ? "" : ": " + sessionName;

        String msg;
        try {
            msg = (args == null || args.length == 0) ? format
                    : String.format(Locale.US, format, args);
        } catch (IllegalFormatException ife) {
            e("", ife, "Log: IllegalFormatException: formatString='%s' numArgs=%d", format,
                    args.length);
            msg = format + " (An error occurred while formatting the message.)";
        }
        return String.format(Locale.US, "%s%s", msg, sessionPostfix);
    }

    private static String buildTag(String subTag) {
        StringBuilder sb = new StringBuilder();
        sb.append(TAG).append("-")
                .append((subTag == null || subTag.equals("")) ? "Common" : subTag);
        return sb.toString();
    }

    private static String buildTag(Object objectSubTag) {
        String subTag =
                (objectSubTag == null ? "Common" : objectSubTag.getClass().getSimpleName());
        return buildTag(subTag);
    }
}
