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
 * MediaTek Inc. (C) 2014. All rights reserved.
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

package com.mediatek.voicecommand.vis;

import android.os.Build;
import android.os.SystemProperties;

/**
 * This utility class is the main entrance to print log with Android Log class. Our
 * application should always use this class to print logs.
 * By default, all logs except Log.d and Log.w will remain disabled in user load.
 * To enable a desired logging level, we can use adb command
 * adb shell setprop vendor.mediatek.voicecommand.forcelog 1
 */
public final class Log {
    private static final String LOG_TAG = "VoiceCommand/VIS/";

    private static final boolean IS_DEBUG_BUILD =
          Build.TYPE.equals("eng") || Build.TYPE.equals("userdebug");

    private static final boolean IS_SUPPORT_FORCE_DEBUG = false;
    private static final String PROP_FORCE_DEBUG_KEY = "vendor.mediatek.voicecommand.vis.forcelog";
    private static final boolean FORCE_DEBUG =
          (SystemProperties.getInt(PROP_FORCE_DEBUG_KEY, 0) == 1) && IS_SUPPORT_FORCE_DEBUG;

    private Log() {
    }

    public static void v(String tag, String msg) {
        if (IS_DEBUG_BUILD || FORCE_DEBUG) {
            android.util.Log.v(LOG_TAG + tag, msg);
        }
    }

    public static void v(String tag, String msg, Throwable t) {
        if (IS_DEBUG_BUILD || FORCE_DEBUG) {
            android.util.Log.v(LOG_TAG + tag, msg, t);
        }
    }

    public static void d(String tag, String msg) {
        if (IS_DEBUG_BUILD || FORCE_DEBUG) {
            android.util.Log.d(LOG_TAG + tag, msg);
        }
    }

    public static void d(String tag, String msg, Throwable t) {
        if (IS_DEBUG_BUILD || FORCE_DEBUG) {
            android.util.Log.d(LOG_TAG + tag, msg, t);
        }
    }

    public static void i(String tag, String msg) {
        if (IS_DEBUG_BUILD || FORCE_DEBUG) {
            android.util.Log.i(LOG_TAG + tag, msg);
        }
    }

    public static void i(String tag, String msg, Throwable t) {
        if (IS_DEBUG_BUILD || FORCE_DEBUG) {
            android.util.Log.i(LOG_TAG + tag, msg, t);
        }
    }

    public static void w(String tag, String msg) {
        android.util.Log.w(LOG_TAG + tag, msg);
    }

    public static void w(String tag, String msg, Throwable t) {
        android.util.Log.w(LOG_TAG + tag, msg, t);
    }

    public static void e(String tag, String msg) {
        android.util.Log.e(LOG_TAG + tag, msg);
    }

    public static void e(String tag, String msg, Throwable t) {
        android.util.Log.e(LOG_TAG + tag, msg, t);
    }
}
