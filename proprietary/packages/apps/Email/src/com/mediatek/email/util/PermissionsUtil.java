package com.mediatek.email.util;

/*
 * Copyright (C) 2015 The Android Open Source Project
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

import java.util.ArrayList;

import com.android.mail.utils.LogUtils;

import android.Manifest.permission;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.provider.Settings;
import androidx.core.app.NotificationCompat;
import android.text.TextUtils;
import com.android.email.R;

/**
 * Utility class to help with runtime permissions.
 */
public class PermissionsUtil {
    public static final String READ_PHONE_STATE = permission.READ_PHONE_STATE;
    public static final String READ_CALENDAR = permission.READ_CALENDAR;
    public static final String WRITE_CALENDAR = permission.WRITE_CALENDAR;
    public static final String READ_CONTACTS = permission.READ_CONTACTS;
    public static final String WRITE_CONTACTS = permission.WRITE_CONTACTS;
    public static final String WRITE_STORAGE = permission.WRITE_EXTERNAL_STORAGE;
    public static final String READ_STORAGE = permission.READ_EXTERNAL_STORAGE;
    public static final String ATTACHMENT = "com.android.email.permission.READ_ATTACHMENT";
    public static final String SEND_EMAIL = "com.mediatek.permission.CTA_SEND_EMAIL";
    public static final String ACTION_EMAIL_PERMISSSION = "com.android.mail.permission";

    private static final int NOTIFICATION_ID_PERMISSION_NEEDED = 0x30000000;

    public static final String PERMISSON_LIST[] = new String[] { READ_PHONE_STATE, READ_CALENDAR,
            WRITE_CALENDAR, READ_CONTACTS, WRITE_CONTACTS, READ_STORAGE, WRITE_STORAGE, ATTACHMENT, SEND_EMAIL };

    public static boolean hasPermission(Context context, String permission) {
        return (context.checkSelfPermission(permission) == PackageManager.PERMISSION_GRANTED);
    }

    public static String[] needPermissionList(Context context) {
        ArrayList<String> needPermission = new ArrayList<String>();
        for (String permisssion : PERMISSON_LIST) {
            if (!hasPermission(context, permisssion)) {
                LogUtils.d("PermissionsUtil", "needPermission:" + permisssion);
                needPermission.add(permisssion);
            } else {
                LogUtils.d("PermissionsUtil", "hasPermission:" + permisssion);
            }
        }
        return needPermission.toArray(new String[needPermission.size()]);
    }

    public static void createPermissionNotification(Context context) {
        // Pending Intent
        PendingIntent pending = PendingIntent.getActivity(context, 0, getPermissionIntent(context),
                PendingIntent.FLAG_UPDATE_CURRENT);

        String[] permissions = PermissionsUtil.needPermissionList(context);
        if (permissions.length == 0) {
            return;
        }

        String title = "Permission need updated";
        String contentText = "Need update Emaill app in setting";
        String ticker = "this a ticker";
        // NOTE: the ticker is not shown for notifications in the Holo UX
        final NotificationCompat.Builder builder = new NotificationCompat.Builder(context)
                .setContentTitle(title).setContentText(contentText).setContentIntent(pending)
                .setNumber(0).setAutoCancel(true)
                .setSmallIcon(R.drawable.ic_notification_mail_24dp).setWhen(
                        System.currentTimeMillis()).setTicker(ticker).setOngoing(false);

        String ringtoneUri = Settings.System.DEFAULT_NOTIFICATION_URI.toString();
        int defaults = Notification.DEFAULT_LIGHTS | Notification.DEFAULT_VIBRATE;

        builder.setSound(TextUtils.isEmpty(ringtoneUri) ? null : Uri.parse(ringtoneUri))
                .setDefaults(defaults);
        NotificationManager notificationManager = (NotificationManager) context
                .getSystemService(Context.NOTIFICATION_SERVICE);
        notificationManager.notify(NOTIFICATION_ID_PERMISSION_NEEDED, builder.build());
    }

    public static Intent getPermissionIntent(Context context) {
        Intent intent = new Intent(context, SafePermissionActivity.class);
        intent.setPackage(context.getPackageName());
        return intent;
    }
}
