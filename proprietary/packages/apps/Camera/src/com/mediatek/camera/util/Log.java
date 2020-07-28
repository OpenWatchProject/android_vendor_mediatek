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

package com.mediatek.camera.util;


/**
 * This utility class is the main entrance to print log with Android Log class. Our
 * application should always use this class to print logs.
 */
public final class Log {
    public static void v(String tag, String msg) {
        com.android.camera.Log.v(tag, msg);
    }

    public static void v(String tag, String msg, Throwable t) {
        com.android.camera.Log.v(tag, msg, t);
    }

    public static void d(String tag, String msg) {
        com.android.camera.Log.d(tag, msg);
    }

    public static void d(String tag, String msg, Throwable t) {
        com.android.camera.Log.d(tag, msg, t);
    }

    public static void i(String tag, String msg) {
        com.android.camera.Log.i(tag, msg);
    }

    public static void i(String tag, String msg, Throwable t) {
        com.android.camera.Log.i(tag, msg, t);
    }

    public static void w(String tag, String msg) {
        com.android.camera.Log.w(tag, msg);
    }

    public static void w(String tag, String msg, Throwable t) {
        com.android.camera.Log.w(tag, msg, t);
    }

    public static void e(String tag, String msg) {
        com.android.camera.Log.e(tag, msg);
    }

    public static void e(String tag, String msg, Throwable t) {
        com.android.camera.Log.e(tag, msg, t);
    }

    /**
     * print log for performance test. this log records the starting time.
     *
     * @param description
     *            the description of this check point.
     */
    public static void performanceStart(String description) {
        com.android.camera.Log.performanceStart(description);
    }

    /**
     * print log for performance test. this log records the ending time of
     * procedure.
     *
     * @param description
     *            the description of this check point.
     */
    public static void performanceEnd(String description) {
        com.android.camera.Log.performanceEnd(description);
    }
}
