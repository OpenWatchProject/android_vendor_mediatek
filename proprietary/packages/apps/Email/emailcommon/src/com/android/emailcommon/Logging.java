/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2011 The Android Open Source Project
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

package com.android.emailcommon;

import android.text.TextUtils;
import android.util.Log;

import com.android.mail.utils.LogTag;

public class Logging {
    /// M: add mtk xlog build toggle.
    private static final boolean XLOG_ENABLED = true;

    public static final String LOG_TAG = LogTag.getLogTag();

    /**
     * Set this to 'true' to enable as much Email logging as possible.
     */
    public static final boolean LOGD;

    /**
     * If this is enabled then logging that normally hides sensitive information
     * like passwords will show that information.
     */
    public static final boolean DEBUG_SENSITIVE;

    /**
     * If true, logging regarding UI (such as activity/fragment lifecycle) will be enabled.
     *
     * TODO rename it to DEBUG_UI.
     */
    public static final boolean DEBUG_LIFECYCLE;

    static {
        // Declare values here to avoid dead code warnings; it means we have some extra
        // "if" statements in the byte code that always evaluate to "if (false)"
        LOGD = false; // DO NOT CHECK IN WITH TRUE
        DEBUG_SENSITIVE = false; // DO NOT CHECK IN WITH TRUE
        DEBUG_LIFECYCLE = false; // DO NOT CHECK IN WITH TRUE
    }

    /** M: add mtk xlog @ {*/
    public static void v(String tag, String msg) {
        tag = TextUtils.isEmpty(tag) ? LOG_TAG : LOG_TAG + "/" + tag;
        if (XLOG_ENABLED) {
            /** M: MTK Dependence */
            Log.v(tag, msg);
        } else {
            Log.v(tag, msg);
        }
    }

    public static void v(String msg) {
        if (XLOG_ENABLED) {
            /** M: MTK Dependence */
            Log.v(LOG_TAG, msg);
        } else {
            Log.v(LOG_TAG, msg);
        }
    }

    public static void v(String tag, String msg, Throwable t) {
        tag = TextUtils.isEmpty(tag) ? LOG_TAG : LOG_TAG + "/" + tag;
        if (XLOG_ENABLED) {
            /** M: MTK Dependence */
            Log.v(tag, msg, t);
        } else {
            Log.v(tag, msg, t);
        }
    }

    public static void v(String msg, Throwable t) {
        if (XLOG_ENABLED) {
            /** M: MTK Dependence */
            Log.v(LOG_TAG, msg, t);
        } else {
            Log.v(LOG_TAG, msg, t);
        }
    }

    public static void d(String tag, String msg) {
        tag = TextUtils.isEmpty(tag) ? LOG_TAG : LOG_TAG + "/" + tag;
        if (XLOG_ENABLED) {
            /** M: MTK Dependence */
            Log.d(tag, msg);
        } else {
            Log.d(tag, msg);
        }
    }

    public static void d(String msg) {
        if (XLOG_ENABLED) {
            /** M: MTK Dependence */
            Log.d(LOG_TAG, msg);
        } else {
            Log.d(LOG_TAG, msg);
        }
    }

    public static void d(String tag, String msg, Throwable t) {
        tag = TextUtils.isEmpty(tag) ? LOG_TAG : LOG_TAG + "/" + tag;
        if (XLOG_ENABLED) {
            /** M: MTK Dependence */
            Log.d(tag, msg, t);
        } else {
            Log.d(tag, msg, t);
        }
    }

    public static void d(String msg, Throwable t) {
        if (XLOG_ENABLED) {
            /** M: MTK Dependence */
            Log.d(LOG_TAG, msg, t);
        } else {
            Log.d(LOG_TAG, msg, t);
        }
    }

    public static void i(String tag, String msg) {
        tag = TextUtils.isEmpty(tag) ? LOG_TAG : LOG_TAG + "/" + tag;
        if (XLOG_ENABLED) {
            /** M: MTK Dependence */
            Log.i(tag, msg);
        } else {
            Log.i(tag, msg);
        }
    }

    public static void i(String msg) {
        if (XLOG_ENABLED) {
            /** M: MTK Dependence */
            Log.i(LOG_TAG, msg);
        } else {
            Log.i(LOG_TAG, msg);
        }
    }

    public static void i(String tag, String msg, Throwable t) {
        tag = TextUtils.isEmpty(tag) ? LOG_TAG : LOG_TAG + "/" + tag;
        if (XLOG_ENABLED) {
            /** M: MTK Dependence */
            Log.i(tag, msg, t);
        } else {
            Log.i(tag, msg, t);
        }
    }

    public static void i(String msg, Throwable t) {
        if (XLOG_ENABLED) {
            /** M: MTK Dependence */
            Log.i(LOG_TAG, msg, t);
        } else {
            Log.i(LOG_TAG, msg, t);
        }
    }

    public static void w(String tag, String msg) {
        tag = TextUtils.isEmpty(tag) ? LOG_TAG : LOG_TAG + "/" + tag;
        if (XLOG_ENABLED) {
            /** M: MTK Dependence */
            Log.w(tag, msg);
        } else {
            Log.w(tag, msg);
        }
    }

    public static void w(String msg) {
        if (XLOG_ENABLED) {
            /** M: MTK Dependence */
            Log.w(LOG_TAG, msg);
        } else {
            Log.w(LOG_TAG, msg);
        }
    }

    public static void w(String tag, String msg, Throwable t) {
        tag = TextUtils.isEmpty(tag) ? LOG_TAG : LOG_TAG + "/" + tag;
        if (XLOG_ENABLED) {
            /** M: MTK Dependence */
            Log.w(tag, msg, t);
        } else {
            Log.w(tag, msg, t);
        }
    }

    public static void w(String msg, Throwable t) {
        if (XLOG_ENABLED) {
            /** M: MTK Dependence */
            Log.w(LOG_TAG, msg, t);
        } else {
            Log.w(LOG_TAG, msg, t);
        }
    }

    public static void e(String tag, String msg) {
        tag = TextUtils.isEmpty(tag) ? LOG_TAG : LOG_TAG + "/" + tag;
        if (XLOG_ENABLED) {
            /** M: MTK Dependence */
            Log.e(tag, msg);
        } else {
            Log.e(tag, msg);
        }
    }

    public static void e(String msg) {
        if (XLOG_ENABLED) {
            /** M: MTK Dependence */
            Log.e(LOG_TAG, msg);
        } else {
            Log.e(LOG_TAG, msg);
        }
    }

    public static void e(String tag, String msg, Throwable t) {
        tag = TextUtils.isEmpty(tag) ? LOG_TAG : LOG_TAG + "/" + tag;
        if (XLOG_ENABLED) {
            /** M: MTK Dependence */
            Log.e(tag, msg, t);
        } else {
            Log.e(tag, msg, t);
        }
    }

    public static void e(String msg, Throwable t) {
        if (XLOG_ENABLED) {
            /** M: MTK Dependence */
            Log.e(LOG_TAG, msg, t);
        } else {
            Log.e(LOG_TAG, msg, t);
        }
    }
    /**@ }*/
}
