/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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
package com.mediatek.contacts.util;

import android.os.Build;
import android.os.SystemProperties;
import android.text.TextUtils;

/**
 * This utility class is the main entrance to print log with Android Log class.
 * Our application should always use this class to print MTK extension logs.
 */
public final class Log {
    private static final String LOG_TAG = "ContactsApp/";
    private static final String PROP_FORCE_DEBUG_KEY = "persist.vendor.log.tel_dbg";
    private static final boolean _DEBUG = true;
    private static final boolean NEED_ANONYMIZE = true;
    public static final boolean ENG_DEBUG = TextUtils.equals(Build.TYPE, "eng");
    public static final boolean FORCE_DEBUG =
            (SystemProperties.getInt(PROP_FORCE_DEBUG_KEY, 0) == 1);

    public static final int VERBOSE = android.util.Log.VERBOSE;
    public static final int DEBUG = android.util.Log.DEBUG;
    public static final int INFO = android.util.Log.INFO;
    public static final int WARN = android.util.Log.WARN;
    public static final int ERROR = android.util.Log.ERROR;
    public static final int ASSERT = android.util.Log.ASSERT;

    public static boolean isLoggable(String TAG, int level) {
        return android.util.Log.isLoggable(TAG, level);
    }

    public static void v(String tag, String msg) {
        if (ENG_DEBUG || FORCE_DEBUG) {
            android.util.Log.v(LOG_TAG + tag, msg);
        }
    }

    public static void v(String tag, String msg, Throwable t) {
        if (_DEBUG) {
            android.util.Log.v(LOG_TAG + tag, msg, t);
        }
    }

    public static void d(String tag, String msg) {
        if (ENG_DEBUG || FORCE_DEBUG) {
            android.util.Log.d(LOG_TAG + tag, msg);
        }
    }

    public static void d(String tag, String msg, Throwable t) {
        if (_DEBUG) {
            android.util.Log.d(LOG_TAG + tag, msg, t);
        }
    }

    public static void i(String tag, String msg) {
        if (ENG_DEBUG || FORCE_DEBUG) {
            android.util.Log.i(LOG_TAG + tag, msg);
        }
    }

    public static void i(String tag, String msg, Throwable t) {
        if (_DEBUG) {
            android.util.Log.i(LOG_TAG + tag, msg, t);
        }
    }

    public static void w(String tag, String msg) {
        if (_DEBUG) {
            android.util.Log.w(LOG_TAG + tag, msg);
        }
    }

    public static void w(String tag, String msg, Throwable t) {
        if (_DEBUG) {
            android.util.Log.w(LOG_TAG + tag, msg, t);
        }
    }

    public static void e(String tag, String msg) {
        if (_DEBUG) {
            android.util.Log.e(LOG_TAG + tag, msg);
        }
    }

    public static void e(String tag, String msg, Throwable t) {
        if (_DEBUG) {
            android.util.Log.e(LOG_TAG + tag, msg, t);
        }
    }

    public static void wtf(String tag, String msg) {
        if (_DEBUG) {
            android.util.Log.wtf(LOG_TAG + tag, msg);
        }
    }

    public static void wtf(String tag, String msg, Throwable t) {
        if (_DEBUG) {
            android.util.Log.wtf(LOG_TAG + tag, msg, t);
        }
    }

    /**
     * For sensitive level log, print nothing if NEED_ANONYMIZE is true,
     * or else print out for debugging purpose.
     */
    public static void sensitive(String tag, String msg) {
        if (!NEED_ANONYMIZE) {
            d(tag, msg);
        }
    }

    /**
     * If sensitive message, make it anonymous.
     * @return The message string after anonymization.
     */
    public static String anonymize(Object msg) {
        String msgStr = String.valueOf(msg);
        if (!NEED_ANONYMIZE || msgStr.isEmpty() || msgStr.equals("null")) {
            return msgStr;
        }
        if (msgStr.length() == 1) {
            return "*";
        }
        StringBuilder builder = new StringBuilder(msgStr);
        for (int i=1; i < msgStr.length(); i=i+2) {
            builder.replace(i, i+1, "*");
        }
        return builder.toString();
    }
}
