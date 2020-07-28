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

package com.mediatek.op08.wfo;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import java.lang.CharSequence;

import com.mediatek.wfo.op.DefaultWosExt;

public class Op08WosExt extends DefaultWosExt {
    static final String TAG = "Op08WosExt";
    Context mPluginContext;
    private Context mHostContext;

    final String NOTIFICATION_TAG = "wifi_calling";
    final int NOTIFICATION_ID = 1;
    String CHANNEL_ID = "wos_channel_1";

    public Op08WosExt(Context context) {
        super(context);
        mPluginContext = context;
    }

    public void initialize(Context context) {
        Log.d(TAG, "Op08WosExt initialize.");
        mHostContext = context;
    }

    @Override
    public void showPDNErrorMessages(int errorCode){
        Log.d(TAG, "showPDNErrorMessages errorCode: " + errorCode);

        if (errorCode == 0) {
            Log.d(TAG, "error message is 0, ignore it");
            return;
        } else if (errorCode == 1999) { /* Succeed, but IP discontinuity */
            Log.d(TAG, "error message is 1999, ignore it");
            return;
        }

        String errorCodeString = String.valueOf(errorCode);

        final String[] wfcOperatorErrorCodes =
                mPluginContext.getResources().getStringArray(R.array.wfcOperatorErrorCodes);
        final String[] wfcOperatorErrorAlertMessages =
                mPluginContext.getResources().getStringArray(R.array.wfcOperatorErrorAlertMessages);
        final String[] wfcOperatorErrorNotificationMessages =
                mPluginContext.getResources().getStringArray(
                        R.array.wfcOperatorErrorNotificationMessages);

        final CharSequence title = mPluginContext.getText(
                com.android.internal.R.string.wfcRegErrorTitle);
        CharSequence messageAlert = mPluginContext.getText(R.string.wfcDefaultErrorMessage);
        CharSequence messageNotification = mPluginContext.getText(R.string.wfcDefaultErrorMessage);

        for (int i = 0; i < wfcOperatorErrorCodes.length; i++) {
            // Match error code.
            if (!errorCodeString.startsWith(wfcOperatorErrorCodes[i])) {
                continue;
            }

            messageAlert = wfcOperatorErrorAlertMessages[i];
            messageNotification = wfcOperatorErrorNotificationMessages[i];

            // it can only match a single error code
            // so should break the loop after a successful match.
            break;
        }

        Intent resultIntent = new Intent(Intent.ACTION_MAIN);
        resultIntent.setClassName("com.android.settings",
                "com.android.settings.Settings$WifiCallingSettingsActivity");
        resultIntent.putExtra("alertShow", true);
        resultIntent.putExtra("alertTitle", title);
        resultIntent.putExtra("alertMessage", messageAlert);
        PendingIntent resultPendingIntent =
                PendingIntent.getActivity(
                        mHostContext,
                        0,
                        resultIntent,
                        PendingIntent.FLAG_UPDATE_CURRENT
                );
        createNotificationChannel();

        final Notification notification =
                new Notification.Builder(mHostContext)
                        .setSmallIcon(android.R.drawable.stat_sys_warning)
                        .setContentTitle(title)
                        .setContentText(messageNotification)
                        .setAutoCancel(true)
                        .setContentIntent(resultPendingIntent)
                        .setStyle(new Notification.BigTextStyle().bigText(messageNotification))
                        .setChannel(CHANNEL_ID)
                        .build();

        NotificationManager notificationManager =
                (NotificationManager) mHostContext.getSystemService(
                        Context.NOTIFICATION_SERVICE);
        notificationManager.notify(NOTIFICATION_TAG, NOTIFICATION_ID,
                notification);
    }

    @Override
    public void clearPDNErrorMessages(){
        Log.d(TAG, "clearPDNErrorMessages");
        // Clear all error notifications since PDN is actived now.
        NotificationManager notificationManager =
                (NotificationManager) mHostContext.getSystemService(Context.NOTIFICATION_SERVICE);
        notificationManager.cancel(NOTIFICATION_TAG, NOTIFICATION_ID);
    }

    private void createNotificationChannel() {
        Log.d(TAG, "createNotificationChannel");
        CharSequence name = mPluginContext.getResources().getString(R.string.channel_name);
        String description = mPluginContext.getResources().getString(R.string.channel_description);
        int importance = NotificationManager.IMPORTANCE_DEFAULT;
        NotificationChannel channel = new NotificationChannel(CHANNEL_ID, name, importance);
        channel.setDescription(description);

        NotificationManager notificationManager =
                (NotificationManager) mHostContext.getSystemService(
                        Context.NOTIFICATION_SERVICE);
        notificationManager.createNotificationChannel(channel);
    }
}
