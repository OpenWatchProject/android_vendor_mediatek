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
 * MediaTek Inc. (C) 2010. All rights reserved.
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

/*
 * Copyright (C) 2010 The Android Open Source Project
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

package com.mediatek.engineermode;

import android.app.Application;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.content.Context;
import android.media.AudioAttributes;
import android.media.RingtoneManager;

/**
 * EngineerMode application class.
 *
 */
public class EmApplication extends Application {
    private static Context sContext;
    private static final String DEFAULT_NOTIFICATION_CHANNEL_ID = "mtk_em_default_channel_id";
    private static final String LOW_NOTIFICATION_CHANNEL_ID = "mtk_em_low_channel_id";
    private static final String TAG = "EmApplication";

    @Override
    public void onCreate() {
        sContext = getApplicationContext();
    }

    public static Context getContext() {
        return sContext;
    }


    /**
     * Get channel ID for notification with sound.
     * @return channel ID
     */
    public static String getSoundNotificationChannelID() {
        NotificationManager notificationManager = (NotificationManager)
                sContext.getSystemService(Context.NOTIFICATION_SERVICE);

        if (notificationManager.getNotificationChannel(DEFAULT_NOTIFICATION_CHANNEL_ID) == null) {
            NotificationChannel channel = new NotificationChannel(DEFAULT_NOTIFICATION_CHANNEL_ID,
                    sContext.getString(R.string.app_name), NotificationManager.IMPORTANCE_DEFAULT);
            channel.setSound(RingtoneManager.getDefaultUri(RingtoneManager.TYPE_NOTIFICATION),
                    new AudioAttributes.Builder().setUsage(
                      AudioAttributes.USAGE_NOTIFICATION).build());
            Elog.i(TAG, "get sound " + channel.getSound());
            notificationManager.createNotificationChannel(channel);
            Elog.i(TAG, "create sound notification channel");
        }
        return DEFAULT_NOTIFICATION_CHANNEL_ID;
    }


    /**
     * Get channel ID for notification without sound.
     * @return channel ID
     */
    public static String getSilentNotificationChannelID() {
        NotificationManager notificationManager = (NotificationManager)
                sContext.getSystemService(Context.NOTIFICATION_SERVICE);

        if (notificationManager.getNotificationChannel(LOW_NOTIFICATION_CHANNEL_ID) == null) {
            NotificationChannel channel = new NotificationChannel(LOW_NOTIFICATION_CHANNEL_ID,
                    sContext.getString(R.string.app_name), NotificationManager.IMPORTANCE_LOW);
            notificationManager.createNotificationChannel(channel);
            Elog.i(TAG, "create silent notification channel");
        }
        return LOW_NOTIFICATION_CHANNEL_ID;
    }
}