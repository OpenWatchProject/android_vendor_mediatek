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
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.internal.telephony;

import android.content.ComponentName;
import android.content.Context;
import android.telephony.Rlog;
import android.telephony.SmsManager;
import android.telephony.TelephonyManager;
import android.util.Log;

import android.os.Build;
import com.android.internal.telephony.SmsApplication;

/**
 * Class for managing the primary application that we will deliver SMS/MMS messages to
 *
 * {@hide}
 */
public final class MtkSmsApplication {
    private static final String LOG_TAG = "MtkSmsApplication";

    private static final boolean DBG = "eng".equals(Build.TYPE);

    /**
     * Gets the default SMS application
     * @param context context from the calling app
     * @param updateIfNeeded update the default app if there is no valid default app configured.
     * @param userId specific user identifier to query default SMS application
     * @return component name of the app and class to deliver SMS messages to
     */
    public static ComponentName getDefaultSmsApplication(Context context,
            boolean updateIfNeeded, int userId) {
        ComponentName component = null;

        try {
            SmsApplication.SmsApplicationData smsApplicationData =
                    SmsApplication.getApplication(context, updateIfNeeded, userId);
            if (smsApplicationData != null) {
                component = new ComponentName(smsApplicationData.mPackageName,
                        smsApplicationData.mSmsReceiverClass);
            }
            return component;
        } finally {
            if (DBG) {
                Rlog.d(LOG_TAG, "getDefaultSmsApplication for userId " + userId +
                        " default component= " + component);
            }
        }
    }

    /**
     * Get default SMS application's package name
     *
     * @param context context from the calling app
     * @param userId specific user identifier to query default SMS application
     * @return default SMS application's package name or null if we can't find it
     */
    private static String getDefaultSmsApplicationPackageName(Context context,
            int userId) {
        final ComponentName component = getDefaultSmsApplication(context, false, userId);
        if (component != null) {
            return component.getPackageName();
        }
        return null;
    }

    /**
     * Check if a package is default sms app (or equivalent, like bluetooth)
     *
     * @param context context from the calling app
     * @param packageName the name of the package to be checked
     * @param userId specific user identifier to query default SMS application
     * @return true if the package is default sms app or bluetooth
     */
    public static boolean isDefaultSmsApplication(Context context, String packageName,
            int userId) {
        if (packageName == null) {
            return false;
        }
        final String defaultSmsPackage = getDefaultSmsApplicationPackageName(context, userId);
        if ((defaultSmsPackage != null && defaultSmsPackage.equals(packageName))
                || SmsApplication.BLUETOOTH_PACKAGE_NAME.equals(packageName)) {
            return true;
        }

        // MTK-START
        // Get the db special visitor list
        /*if (SmsApplication.mSmsDbVisitorList == null) {
            SmsApplication.loadSmsDbVisitor();
        }
        if (SmsApplication.mSmsDbVisitorList != null) {
            for (int i = 0 ; i < SmsApplication.mSmsDbVisitorList.size() ; i++) {
                if (packageName.equals(SmsApplication.mSmsDbVisitorList.get(i))) {
                    return true;
                }
            }
        }*/
        // MTK-END

        return false;
    }

    /**
     * Returns whether need to write the SMS message to SMS database for this package.
     * <p>
     * Caller must pass in the correct user context amd user identifier if calling
     * from a singleton service.
     * @param packageName the name of the package to be checked
     * @param context context from the calling app
     * @param userId specific user identifier to query default SMS application
     * @return true if the package need help to write the sent short message
     */
    public static boolean shouldWriteMessageForPackage(String packageName,
            Context context, int userId) {
        if (SmsManager.getDefault().getAutoPersisting()) {
            return true;
        }

        boolean result = !isDefaultSmsApplication(context, packageName, userId);
        if (DBG) {
            Rlog.d(LOG_TAG, "shouldWriteMessageForPackage for userId " + userId
                    + ", shouldWrite=" + result);
        }
        return result;
    }
    // MTK-END
}
