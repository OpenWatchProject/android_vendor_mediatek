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

/*
 * Copyright (C) 2007-2008 Esmertec AG.
 * Copyright (C) 2007-2008 The Android Open Source Project
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
package com.android.mms.util;

import android.os.SystemProperties;
import android.util.Log;

public class MmsLog {

    static private boolean  piLoggable =
                       !("user".equals(SystemProperties.get("ro.build.type", "user"))) ;
    static private boolean  hideSensInEng = true;

    public static void v(String tag, String msg) {
        Log.v(tag, msg);
    }

    public static void e(String tag, String msg) {
        Log.e(tag, msg);
    }

    public static void e(String tag, String msg, Throwable tr) {
         Log.e(tag, msg, tr);
    }

    public static void i(String tag, String msg) {
        Log.i(tag, msg);
    }

    public static void d(String tag, String msg) {
        Log.d(tag, msg);
    }

    public static void d(String tag, String msg, Throwable tr) {
        Log.d(tag, msg, tr);
    }

    public static void w(String tag, String msg) {
        Log.w(tag, msg);
    }

    public static void w(String tag, String msg, Throwable tr) {
        Log.w(tag, msg, tr);
    }

    public static void vpi(String tag, String msg) {
        if (piLoggable) {
            if (hideSensInEng == true) {
                msg = partialHideNumber(msg);
            }
            Log.v(tag, msg);
        }
    }

    public static void epi(String tag, String msg) {
        if (piLoggable) {
            if (hideSensInEng == true) {
                msg = partialHideNumber(msg);
            }
            Log.e(tag, msg);
        }
    }

    public static void epi(String tag, String msg, Throwable tr) {
        if (piLoggable) {
            if (hideSensInEng == true) {
                msg = partialHideNumber(msg);
            }
            Log.e(tag, msg, tr);
        }
    }

    public static void ipi(String tag, String msg) {
        if (piLoggable) {
            if (hideSensInEng == true) {
                msg = partialHideNumber(msg);
            }
            Log.i(tag, msg);
        }
    }

    public static void dpi(String tag, String msg) {
        if (piLoggable) {
            if (hideSensInEng == true) {
                msg = partialHideNumber(msg);
            }
            Log.d(tag, msg);
        }
    }

    public static void dpi(String tag, String msg, Throwable tr) {
        if (piLoggable) {
            if (hideSensInEng == true) {
                msg = partialHideNumber(msg);
            }
            Log.d(tag, msg, tr);
        }
    }

    public static void wpi(String tag, String msg) {
        if (piLoggable) {
            if (hideSensInEng == true) {
                msg = partialHideNumber(msg);
            }
            Log.w(tag, msg);
        }
    }

    public static void wpi(String tag, String msg, Throwable tr) {
        if (piLoggable) {
            if (hideSensInEng == true) {
                msg = partialHideNumber(msg);
            }
            Log.w(tag, msg, tr);
        }
    }

    public static void dumpStackTrace() {
        try {
            String tempString = null;
            int len = tempString.length();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private static String partialHideNumber(String logString) {
        if (logString == null) {
            return "";
        }

        final String sLogNumPattern = "(?<=[\\d]{2})\\d(?=[\\d]{2})";
        final String sLogEmailPattern = "(?<=@)\\w+(?=.)";
        String newLogString = logString.replaceAll(sLogNumPattern, "*");
        newLogString = newLogString.replaceAll(sLogEmailPattern, "*****");

        return newLogString;
    }
}
