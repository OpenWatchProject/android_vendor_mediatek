/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.camera.common.debug;
import android.os.Build;
import android.util.Log;

import com.mediatek.camera.portability.SystemProperties;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;

/**
 * Helpful logging extensions for including more detailed info about
 * the objects and threads involved in the logging.
 * <li> Add prefix(CAMERA_LOGTAG_PREFIX) for all Tag<li>
 * <li> By setting the system property, you can control the log level can be printed.<li>
 */
public class LogUtil {
    public static final int MTKCAM_LOG_LEVEL_DEFAULT = -1;
    public static final int MTKCAM_LOG_LEVEL_ERROR = 0;
    public static final int MTKCAM_LOG_LEVEL_WARNING = 1;
    public static final int MTKCAM_LOG_LEVEL_INFO = 2;
    public static final int MTKCAM_LOG_LEVEL_DEBUG = 3;
    public static final int MTKCAM_LOG_LEVEL_VERBOS = 4;
    private static final String CAMERA_LOGTAG_PREFIX = "CamAp_";
    private static final String mTag = "CamAp_LogUtil";
    private static boolean sUserDebugLogAll = false;
    private static int sPersistLogLevel = MTKCAM_LOG_LEVEL_DEFAULT;

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
        private String mValue;

        /**
         * To check the log tag length. if the tag length is larger than MAX_TAG_LEN,
         * it will be truncated.
         * @param tag user-defined tag.
         */
        public Tag(String tag) {
            mValue = new StringBuilder(CAMERA_LOGTAG_PREFIX).append(tag).toString();
        }

        @Override
        /**
         * format the Tag to String.
         * return the tag string.
         */
        public String toString() {
            return mValue;
        }

        /**
         * truncate tag when it is lagger than 23 words.
         */
        public void truncateTag() {
            final int lenDiff = mValue.length() - MAX_TAG_LEN;
            mValue = lenDiff > 0 ? mValue.substring(0, MAX_TAG_LEN_DEFINED_FRAMEWORK - 1) : mValue;
        }
    }

    static {
        initCameraLogLevel();
    }

    /**
     * init the log level.
     */
    public static void initCameraLogLevel() {
        sPersistLogLevel = getPersistLevelFromProperty();
    }

    /**
     * get the android sdk version.
     * @return sdk spi version.
     */
    public static int getAndroidSDKVersion() {
        int version = 0;
        try {
            version = Integer.valueOf(Build.VERSION.SDK_INT);
        } catch (NumberFormatException e) {
            Log.e(mTag, e.getMessage());
        }
        return version;
    }

    /**
     * Prefixes a message with with a hashcode tag of the object.
     * @param object Prefixes a message with with a hashcode tag of the object.
     * @param msg The message you would like logged.
     * @return the message with the object info.
     */
    public static String addTags(Object object, String msg) {
        String fs = new StringBuilder(hashCodeTag(object))
                        .append(", ").append(msg).toString();
        return fs;
    }

    /**
     * Prefixes a message with the bracketed tags specified in the
     * tag list, along with a hashcode tag of the object.
     * @param object Prefixes a message with with a hashcode tag of the object.
     * @param msg The message you would like logged.
     * @param tagList Prefixes a message with the bracketed tags specified in the
     *             tag list.
     * @return the message with the object info.
     */
    public static String addTags(Object object, String msg, String tagList) {
        String fs = new StringBuilder(hashCodeTag(object))
                        .append(formatTags(tagList))
                        .append(", ")
                        .append(msg)
                        .toString();
        return fs;
    }

    /**
     * log tag check and log level check.
     * @param tag the log tag.
     * @param level log level.
     * @return true if log tag and log level is OK.
     */
    public static boolean isLoggable(Tag tag, int level) {
        boolean checkLevelResult = false;
        boolean checkResult = true;
        int overrideLogLevel = getOverrideLevelFromProperty();
        if (overrideLogLevel > -1
                || sPersistLogLevel > -1) {
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
            checkLevelResult = getLogLevelFromSystemLevel(level) <= overrideLogLevel;
            checkLevelResult = checkLevelResult ||
                    (getLogLevelFromSystemLevel(level) <= sPersistLogLevel);
        }
        shouldLog(tag, level);
        checkResult = checkLevelResult || shouldLog(tag, level) || isDebugOsBuild();
        return checkResult;
    }

    /**
     * get mtkcam log level.
     * @return log system property log level.
     */
    private static int getOverrideLevelFromProperty() {
        int logLevelFromProperties = MTKCAM_LOG_LEVEL_DEFAULT;
        try {
            logLevelFromProperties = SystemProperties.getInt("vendor.debug.mtkcam.loglevel",
                    MTKCAM_LOG_LEVEL_DEFAULT);
        } catch (IllegalArgumentException e) {
            Log.e(mTag, e.getMessage());
        }
        return logLevelFromProperties;
    }

    private static int getPersistLevelFromProperty() {
        int logLevelFromProperties = MTKCAM_LOG_LEVEL_DEFAULT;
        try {
            logLevelFromProperties = SystemProperties.getInt("persist.vendor.mtkcamapp.loglevel",
                    MTKCAM_LOG_LEVEL_DEFAULT);
        } catch (IllegalArgumentException e) {
            Log.e(mTag, e.getMessage());
        }
        Log.i(mTag, "getPersistLevelFromProperty: " + logLevelFromProperties);
        return logLevelFromProperties;
    }

    /**
     * mapping android log level to mtk log level.
     * @param systemLevel defined in Log.java.
     * @return self-defined system log level.
     */
    private static int getLogLevelFromSystemLevel(int systemLevel) {
        switch (systemLevel) {
        case android.util.Log.ERROR:
            return MTKCAM_LOG_LEVEL_ERROR;
        case android.util.Log.WARN:
            return MTKCAM_LOG_LEVEL_WARNING;
        case android.util.Log.INFO:
            return MTKCAM_LOG_LEVEL_INFO;
        case android.util.Log.DEBUG:
            return MTKCAM_LOG_LEVEL_DEBUG;
        case android.util.Log.VERBOSE:
            return MTKCAM_LOG_LEVEL_VERBOS;
        default:
            return MTKCAM_LOG_LEVEL_DEFAULT;
        }
    }

    private static boolean shouldLog(Tag tag, int level) {
        boolean result = false;
        try {
            result = android.util.Log.isLoggable(tag.toString(), level);
        } catch (IllegalArgumentException ex) {
            tag.truncateTag();
        }
        return result;
    }

    private static boolean isDebugOsBuild() {
        boolean result = false;
        if (sUserDebugLogAll) {
            result = "userdebug".equals(Build.TYPE);
        }
        return result || "eng".equals(Build.TYPE);
    }

    private static String formatTags(String tagList) {
        // Split on common "divider" characters:
        // * All whitespace, except spaces: \x00-\x1F
        // * () Characters: \x28-\x29
        // * , Character: \x2C
        // * / Character: \x2F
        // * ;<=>? Characters: \x3B-\x3F
        // * [\] Characters: \x5B-\x5D
        // * {|} Characters: \x7B-\x7D
        List<String> tags = Arrays.asList(tagList.split("[\\x00-\\x1F\\x28-\\x29\\x2C\\x2F"
              + "\\x3B-\\x3F\\x5B-\\x5D\\x7B-\\x7D]"));
        Collections.sort(tags);
        String result = "";
        // Convert all non-empty entries to tags.
        for (String tag : tags) {
            String trimmed = tag.trim();
            if (trimmed.length() > 0) {
                String ts = new StringBuilder("[").append(trimmed).append("]").toString();
                result = new StringBuilder(result).append(ts).toString();
            }
        }
        return result;
    }

    private static String hashCodeTag(Object object) {
        final String tag;
        if (object == null) {
            tag = "null";
        } else {
            tag = Integer.toHexString(object.hashCode());
        }
        return String.format("[%-9s]", new StringBuilder("@").append(tag));
    }
}
