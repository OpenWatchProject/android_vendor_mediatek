/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2011. All rights reserved.
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
 */

package com.mediatek.calendar;

import android.os.Build;
import android.util.Log;


/**
 * This utility class is the main entrance to print log with Xlog/Log class.
 * Our application should always use this class to print logs.
 */
public final class LogUtil {

    private static final boolean XLOG_ENABLED = true;
    private static final String MTK_LOG_TAG = "Calendar";
    private static final boolean IS_ENG_BUILD = "eng".equals(Build.TYPE);

    private LogUtil() {
    }

    public static void v(String tag, String msg) {
        if (XLOG_ENABLED) {
            Log.v("@M_" + MTK_LOG_TAG, "<<" + tag + ">>: " + msg);
        } else {
            Log.v(tag, msg);
        }
    }

    public static void v(String tag, String msg, Throwable t) {
        if (XLOG_ENABLED) {
            Log.v("@M_" + MTK_LOG_TAG, "<<" + tag + ">>: " + msg, t);
        } else {
            Log.v(tag, msg, t);
        }
    }

    public static void d(String tag, String msg) {
        if (XLOG_ENABLED) {
            Log.d("@M_" + MTK_LOG_TAG, "<<" + tag + ">>: " + msg);
        } else {
            Log.d(tag, msg);
        }
    }

    public static void d(String tag, String msg, Throwable t) {
        if (XLOG_ENABLED) {
            Log.d("@M_" + MTK_LOG_TAG, "<<" + tag + ">>: " + msg, t);
        } else {
            Log.d(tag, msg, t);
        }
    }

    public static void i(String tag, String msg) {
        if (XLOG_ENABLED) {
            Log.i("@M_" + MTK_LOG_TAG, "<<" + tag + ">>: " + msg);
        } else {
            Log.i(tag, msg);
        }
    }

    public static void i(String tag, String msg, Throwable t) {
        if (XLOG_ENABLED) {
            Log.i("@M_" + MTK_LOG_TAG, "<<" + tag + ">>: " + msg, t);
        } else {
            Log.i(tag, msg, t);
        }
    }

    public static void w(String tag, String msg) {
        if (XLOG_ENABLED) {
            Log.w("@M_" + MTK_LOG_TAG, "<<" + tag + ">>: " + msg);
        } else {
            Log.w(tag, msg);
        }
    }

    public static void w(String tag, String msg, Throwable t) {
        if (XLOG_ENABLED) {
            Log.w("@M_" + MTK_LOG_TAG, "<<" + tag + ">>: " + msg, t);
        } else {
            Log.w(tag, msg, t);
        }
    }

    public static void e(String tag, String msg) {
        if (XLOG_ENABLED) {
            Log.e("@M_" + MTK_LOG_TAG, "<<" + tag + ">>: " + msg);
        } else {
            Log.e(tag, msg);
        }
    }

    public static void e(String tag, String msg, Throwable t) {
        if (XLOG_ENABLED) {
            Log.e("@M_" + MTK_LOG_TAG, "<<" + tag + ">>: " + msg, t);
        } else {
            Log.e(tag, msg, t);
        }
    }

    /**
     * the optional info log, only log info on eng load.
     * @param tag
     * @param msg
     */
    public static void oi(String tag, String msg) {
        if (IS_ENG_BUILD) {
            if (XLOG_ENABLED) {
                Log.i("@M_" + MTK_LOG_TAG, "<<" + tag + ">>: " + msg);
            } else {
                Log.i(tag, msg);
            }
        }
    }

    /**
     * print log for performance test. this log records the starting time.
     * @param description the description of this check point.
     */
    public static void performanceStart(String description) {
        String msg = makePerformanceLogText("start", description);
        if (XLOG_ENABLED) {
            Log.i("@M_" + MTK_LOG_TAG, msg);
        } else {
            Log.i(MTK_LOG_TAG, msg);
        }
    }

    /**
     * print log for performance test. this log records the ending time of procedure.
     * @param description the description of this check point.
     */
    public static void performanceEnd(String description) {
        String msg = makePerformanceLogText("end", description);
        if (XLOG_ENABLED) {
            Log.i("@M_" + MTK_LOG_TAG, msg);
        } else {
            Log.i(MTK_LOG_TAG, msg);
        }
    }

    /**
     * the format of performance test about calendar app part.
     */
    private static final String PERFORMANCE_FORMAT_STRING = "[Performance test][Calendar][app] %s %s[%d]";

    /**
     * A tool to make the text following the {@link PERFORMANCE_FORMAT_STRING} format.
     * @param checkPoint can be "start" or "end" TODO: maybe an Enum is better.
     * @param description the description of the check point
     * @return text
     */
    private static String makePerformanceLogText(String checkPoint,
            String description) {
        String desc = description == null ? "" : description;
        String msg = String.format(PERFORMANCE_FORMAT_STRING,
                new Object[]{desc, checkPoint, System.currentTimeMillis()});
        return msg;
    }
}
