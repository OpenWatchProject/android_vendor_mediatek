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
 * have been modified by MediaTek Inc. All revisions are subject to any receiver\'s
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.internal.telephony.cat;

import android.os.Build;
import android.telephony.Rlog;
import android.text.TextUtils;

public abstract class MtkCatLog {
    static final boolean DEBUG = true;
    static final String TAG = "MTKCAT";
    static final boolean ENGDEBUG = TextUtils.equals(Build.TYPE, "eng");

    public static void d(Object caller, String msg) {
        if (!DEBUG) {
            return;
        }

        String className = caller.getClass().getName();
        Rlog.d(TAG, className.substring(className.lastIndexOf('.') + 1) + ": " + msg);
    }

    public static void d(String caller, String msg) {
        if (!DEBUG) {
            return;
        }

        Rlog.d(TAG, caller + ": " + msg);
    }

    public static void e(Object caller, String msg) {
        String className = caller.getClass().getName();
        Rlog.e(TAG, className.substring(className.lastIndexOf('.') + 1) + ": " + msg);
    }

    public static void e(String caller, String msg) {
        Rlog.e(TAG, caller + ": " + msg);
    }

    public static void w(Object caller, String msg) {
        if (!ENGDEBUG) {
            return;
        }

        String className = caller.getClass().getName();
        Rlog.w(TAG, className.substring(className.lastIndexOf('.') + 1) + ": " + msg);
    }

    public static void w(String caller, String msg) {
        if (!ENGDEBUG) {
            return;
        }

        Rlog.w(TAG, caller + ": " + msg);
    }

    public static void v(Object caller, String msg) {
        if (!ENGDEBUG) {
            return;
        }

        String className = caller.getClass().getName();
        Rlog.v(TAG, className.substring(className.lastIndexOf('.') + 1) + ": " + msg);
    }

    public static void v(String caller, String msg) {
        if (!ENGDEBUG) {
            return;
        }

        Rlog.v(TAG, caller + ": " + msg);
    }

}
