
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

import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.widget.Toast;

import com.mediatek.contacts.ContactsApplicationEx;

public final class MtkToast {
    private static Toast sToast = null;
    private static final String DEFAULT_TOAST_STRING = "MTKToast";

    public static final int LENGTH_LONG = Toast.LENGTH_LONG;
    public static final int LENGTH_SHORT = Toast.LENGTH_SHORT;

    private MtkToast() {
        // do nothing
    }

    /**
     * Toast with default duration,LENGTH_SHORT
     *
     * @param context
     * @param msg
     *            The message to toast
     */
    public static void toast(Context context, String msg) {
        toast(context, msg, LENGTH_SHORT);
    }

    /**
     * Toast with default duration,LENGTH_SHORT
     *
     * @param context
     * @param resId
     *            The string id.
     */
    public static void toast(Context context, int resId) {
        toast(context, resId, LENGTH_SHORT);
    }

    /**
     *
     * @param context
     * @param resId
     *            The string id
     * @param duration
     *            The toast duration
     */
    public static void toast(Context context, int resId, int duration) {
        toast(context, context.getResources().getString(resId), LENGTH_SHORT);
    }

    /**
     *
     * @param context
     * @param msg
     *            The message to toast
     * @param duration
     *            The toast duration
     */
    public static void toast(Context context, String msg, int duration) {
        Toast toast = getToast(context, msg, duration);
        toast.show();
    }

    /**
     * return a toast object for toast
     *
     * @param context
     * @param msg
     * @param duration
     * @return the Toast singleton
     */
    private static Toast getToast(Context context, String msg, int duration) {
        if (sToast == null) {
            sToast = Toast.makeText(context.getApplicationContext(), DEFAULT_TOAST_STRING,
                    duration);
        }
        sToast.setText(msg);
        sToast.setDuration(duration);
        return sToast;
    }

    public static void toastFromNoneUiThread(final int msgResId) {
        new Handler(Looper.getMainLooper()).post(new Runnable() {
            @Override
            public void run() {
                Toast.makeText(ContactsApplicationEx.getContactsApplication(),
                        msgResId, Toast.LENGTH_SHORT).show();
            }
        });
    }
}
