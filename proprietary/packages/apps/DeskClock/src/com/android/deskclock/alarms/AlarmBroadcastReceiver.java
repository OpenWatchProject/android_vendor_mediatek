/*
 * Copyright Statement:
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
 */

/*
 * Copyright (C) 2007 The Android Open Source Project
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

package com.android.deskclock.alarms;

import android.app.AlarmManager;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.storage.StorageManager;

import com.android.deskclock.LogUtils;
import com.android.deskclock.provider.AlarmInstance;

/**
 * This receiver is used to some action, eg:1.clear status bar icon when the
 * application data is cleared by settings; 2. ssytem encryption typr change; 3.
 * system language changed
 */
public class AlarmBroadcastReceiver extends BroadcastReceiver {
    private static final String ACTION_PACKAGE_DATA_CLEARED =
            "com.mediatek.intent.action.SETTINGS_PACKAGE_DATA_CLEARED";
    private static final String ACTION_ENCRPTION_TYPE_CHANGED =
            "com.mediatek.intent.extra.ACTION_ENCRYPTION_TYPE_CHANGED";
//            Intent.ACTION_SETTINGS_PACKAGE_DATA_CLEARED;

    @Override
    public void onReceive(Context context, Intent intent) {
        // Add for avoiding null point exception
        if (intent == null) {
            return;
        }
        LogUtils.d("AlarmBroadcastReceiver action:", intent.getAction());
        /** M: Receive system language changed action @{ */
        if (Intent.ACTION_LOCALE_CHANGED.equals(intent.getAction())) {
            // TODO: needs to reset weekdays Utils.resetShortWeekdays();
            return;
        }
        /** @} */

        /** M: When encryption type change, to clear or set power off alarm. @{ */
        if (ACTION_ENCRPTION_TYPE_CHANGED.equals(intent.getAction())) {
            if (StorageManager.CRYPT_TYPE_DEFAULT == PowerOffAlarm.getPasswordType()) {
                AlarmInstance nextAlarm = AlarmStateManager.getNextFiringAlarm(context);
                AlarmStateManager.setPoweroffAlarm(context, nextAlarm);
            } else {
                AlarmManager am = (AlarmManager) context.getSystemService(Context.ALARM_SERVICE);
                am.cancelPoweroffAlarm(context.getPackageName());
            }
            return;
        }
        /** @} */

        if (!ACTION_PACKAGE_DATA_CLEARED.equals(intent.getAction())) {
            return;
        }

        String pkgName = intent.getStringExtra("packageName");
        LogUtils.v("AlarmBroadcastReceiver recevied pkgName = " + pkgName);
        if (pkgName == null || !pkgName.equals(context.getPackageName())) {
            return;
        }

        /// M: When clear date, also cancel power off alarm from alarm manager
        AlarmManager am = (AlarmManager) context.getSystemService(Context.ALARM_SERVICE);
        am.cancelPoweroffAlarm(context.getPackageName());
        AlarmNotifications.registerNextAlarmWithAlarmManager(context, null);
    }
}
