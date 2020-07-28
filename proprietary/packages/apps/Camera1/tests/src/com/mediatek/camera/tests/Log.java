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

package com.mediatek.camera.tests;

/**
 * This utility class is the main entrance to print special camera test log. All camera tests should
 * always use this class to print logs.
 */
public final class Log {
    private static final String LOG_TAG = "CamAT";

    private Log() {
    }

    /**
     * Send a verbose log message.
     *
     * @param tag
     *            Used to identify the source of a log message. It usually identifies the class or
     *            activity where the log call occurs.
     * @param msg
     *            The message you would like logged.
     */
    public static void v(String tag, String msg) {
        android.util.Log.v(formatTag(tag), msg);
    }

    /**
     * Send a verbose log message and log the exception.
     *
     * @param tag
     *            Used to identify the source of a log message. It usually identifies the class or
     *            activity where the log call occurs.
     * @param msg
     *            The message you would like logged.
     * @param tr
     *            An exception to log.
     */
    public static void v(String tag, String msg, Throwable tr) {
        android.util.Log.v(formatTag(tag), msg, tr);
    }

    /**
     * Send a debug log message.
     *
     * @param tag
     *            Used to identify the source of a log message. It usually identifies the class or
     *            activity where the log call occurs.
     * @param msg
     *            The message you would like logged.
     */
    public static void d(String tag, String msg) {
        android.util.Log.d(formatTag(tag), msg);
    }

    /**
     * Send a debug log message and log the exception.
     *
     * @param tag
     *            Used to identify the source of a log message. It usually identifies the class or
     *            activity where the log call occurs.
     * @param msg
     *            The message you would like logged.
     * @param tr
     *            An exception to log
     */
    public static void d(String tag, String msg, Throwable tr) {
        android.util.Log.d(formatTag(tag), msg, tr);
    }

    /**
     * Send an info log message.
     *
     * @param tag
     *            Used to identify the source of a log message. It usually identifies the class or
     *            activity where the log call occurs.
     * @param msg
     *            The message you would like logged.
     */
    public static void i(String tag, String msg) {
        android.util.Log.i(formatTag(tag), msg);
    }

    /**
     * Send a info log message and log the exception.
     *
     * @param tag
     *            Used to identify the source of a log message. It usually identifies the class or
     *            activity where the log call occurs.
     * @param msg
     *            The message you would like logged.
     * @param tr
     *            An exception to log
     */
    public static void i(String tag, String msg, Throwable tr) {
        android.util.Log.i(formatTag(tag), msg, tr);
    }

    /**
     * Send a warning log message.
     *
     * @param tag
     *            Used to identify the source of a log message. It usually identifies the class or
     *            activity where the log call occurs.
     * @param msg
     *            The message you would like logged.
     */
    public static void w(String tag, String msg) {
        android.util.Log.w(formatTag(tag), msg);
    }

    /**
     * Send a warning log message and log the exception.
     *
     * @param tag
     *            Used to identify the source of a log message. It usually identifies the class or
     *            activity where the log call occurs.
     * @param msg
     *            The message you would like logged.
     * @param tr
     *            An exception to log
     */
    public static void w(String tag, String msg, Throwable tr) {
        android.util.Log.w(formatTag(tag), msg, tr);
    }

    /**
     * Send an error log message.
     *
     * @param tag
     *            Used to identify the source of a log message. It usually identifies the class or
     *            activity where the log call occurs.
     * @param msg
     *            The message you would like logged.
     */
    public static void e(String tag, String msg) {
        android.util.Log.e(formatTag(tag), msg);
    }

    /**
     * Send a error log message and log the exception.
     *
     * @param tag
     *            Used to identify the source of a log message. It usually identifies the class or
     *            activity where the log call occurs.
     * @param msg
     *            The message you would like logged.
     * @param tr
     *            An exception to log
     */
    public static void e(String tag, String msg, Throwable tr) {
        android.util.Log.e(formatTag(tag), msg, tr);
    }

    private static String formatTag(String tag) {
        StringBuilder builder = new StringBuilder();
        builder.append(LOG_TAG).append("_").append(tag);
        return builder.toString();
    }
}
