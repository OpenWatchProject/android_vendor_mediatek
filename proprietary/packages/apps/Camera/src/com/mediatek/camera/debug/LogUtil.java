/*
 * Copyright (C) 2015 The Android Open Source Project
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

import android.os.SystemProperties;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Objects;

/**
 * Helpful logging extensions for including more detailed info about
 * the objects and threads involved in the logging.
 */
class LogUtil {
    private static final int MTKCAM_LOG_LEVEL_DEFAULT = -1;
    private static final int MTKCAM_LOG_LEVEL_ERROR = 0;
    private static final int MTKCAM_LOG_LEVEL_WARNING = 1;
    private static final int MTKCAM_LOG_LEVEL_INFO = 2;
    private static final int MTKCAM_LOG_LEVEL_DEBUG = 3;
    private static final int MTKCAM_LOG_LEVEL_VERBOS = 4;
    private static int sLogLevelFromProperties =
            SystemProperties.getInt("vendor.debug.mtkcam.loglevel", MTKCAM_LOG_LEVEL_DEFAULT);
    private static int sLogPersistLevelFromProperties = SystemProperties.getInt(
            "persist.vendor.mtkcamapp.loglevel", MTKCAM_LOG_LEVEL_DEFAULT);

    /**
     * Prefixes a message with with a hashcode tag of the object,
     * and a [ui] tag if the method was executed on the main thread.
     */
    public static String addTags(Object object, String msg) {
        return hashCodeTag(object) + mainThreadTag() + " " + msg;
    }

    /**
     * Prefixes a message with the bracketed tags specified in the
     * tag list, along with a hashcode tag of the object, and a
     * [ui] tag if the method was executed on the main thread.
     */
    public static String addTags(Object object, String msg, String tagList) {
        return hashCodeTag(object) + mainThreadTag() + formatTags(tagList) + " " + msg;
    }

    /**
     *
     * @return log system property log level
     */
    public static int getOverrideLevelFromProperty() {
        return sLogLevelFromProperties;
    }

    /**
    * get persisit log level for camera app.
    * @return log system property log level
    */
   public static int getPersistLevelFromProperty() {
       return sLogPersistLevelFromProperties;
   }

    /**
     *
     * @param systemLevel defined in Log.java
     * @return self-defined system log level
     */
    public static int getLogLevelFromSystemLevel(int systemLevel) {
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
                result += "[" + trimmed + "]";
            }
        }
        return result;
    }

    private static String hashCodeTag(Object object) {
        final String tag;
        if (object == null) {
            tag = "null";
        } else {
            tag = Integer.toHexString(Objects.hashCode(object));
        }
        return String.format("[%-9s]", "@" + tag);
    }

    private static String mainThreadTag() {
        return null; //MainThread.isMainThread() ? "[ui]" : "";
    }
}