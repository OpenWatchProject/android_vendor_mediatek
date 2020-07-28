/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
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
package com.mediatek.hdmilocalservice;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.os.Message;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.os.SystemProperties;
import android.os.UEventObserver;
import android.os.UserHandle;
import android.util.Slog;
import com.android.server.SystemService;

import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;

public class HdmiLocalService extends SystemService {
    private final String TAG = "HdmiLocalService";
    private Context mContext;
    private HdmiObserver mHdmiObserver;
    private static final boolean HDMI_TB_SUPPORT =
            !("".equals(SystemProperties.get("ro.vendor.mtk_tb_hdmi")));

    public HdmiLocalService(Context context) {
        super(context);
        mContext = context;
    }

    @Override
    public void onStart() {
        Slog.d(TAG, "Start HdmiLocalService");
    }

    @Override
    public void onBootPhase(int phase) {
        if (phase == SystemService.PHASE_BOOT_COMPLETED) {
            Slog.d(TAG, "Do something in this phase(" + SystemService.PHASE_BOOT_COMPLETED + ")");
            if (HDMI_TB_SUPPORT){
                if (null == mHdmiObserver) {
                    mHdmiObserver = new HdmiObserver(mContext);
                    mHdmiObserver.startObserve();
                }
            }
        }
    }

    private class HdmiObserver extends UEventObserver {
        private static final String TAG = "HdmiLocalService.HdmiObserver";

        private static final String HDMI_UEVENT_MATCH = "DEVPATH=/devices/virtual/switch/hdmi";
        private static final String HDMI_STATE_PATH = "/sys/class/switch/hdmi/state";
        private static final String HDMI_NAME_PATH = "/sys/class/switch/hdmi/name";
        private static final String HDMI_NOTIFICATION_CHANNEL_ID = "hdmi_notification_channel";
        private static final String HDMI_NOTIFICATION_NAME = "HDMI";

        // Monitor notify HDMI
        private static final int MSG_HDMI_PLUG_IN = 10;
        private static final int MSG_HDMI_PLUG_OUT = 11;

        private int mHdmiState;
        private int mPrevHdmiState;
        private String mHdmiName;

        private final Context mCxt;
        private final WakeLock mWakeLock;

        public HdmiObserver(Context context) {
            mCxt = context;
            PowerManager pm = (PowerManager) context
                    .getSystemService(Context.POWER_SERVICE);
            mWakeLock = pm.newWakeLock(PowerManager.FULL_WAKE_LOCK,
                    "HdmiObserver");
            mWakeLock.setReferenceCounted(false);
            init();
        }

        public void startObserve() {
            startObserving(HDMI_UEVENT_MATCH);
        }

        public void stopObserve() {
            stopObserving();
        }

        @Override
        public void onUEvent(UEventObserver.UEvent event) {
            Slog.d(TAG, "HdmiObserver: onUEvent: " + event.toString());
            String name = event.get("SWITCH_NAME");
            int state = 0;
            try {
                state = Integer.parseInt(event.get("SWITCH_STATE"));
            } catch (NumberFormatException e) {
                Slog.w(TAG,
                        "HdmiObserver: Could not parse switch state from event "
                                + event);
            }
            Slog.d(TAG, "HdmiObserver.onUEvent(), name=" + name + ", state="
                    + state);
            update(name, state);
        }

        private void init() {
            String newName = mHdmiName;
            int newState = mHdmiState;
            mPrevHdmiState = mHdmiState;
            newName = getContentFromFile(HDMI_NAME_PATH);
            try {
                newState = Integer
                        .parseInt(getContentFromFile(HDMI_STATE_PATH));
            } catch (NumberFormatException e) {
                Slog.w(TAG, "HDMI state fail");
                return;
            }
            update(newName, newState);
        }

        private String getContentFromFile(String filePath) {
            char[] buffer = new char[1024];
            FileReader reader = null;
            String content = null;
            try {
                reader = new FileReader(filePath);
                int len = reader.read(buffer, 0, buffer.length);
                content = String.valueOf(buffer, 0, len).trim();
                Slog.d(TAG, filePath + " content is " + content);
            } catch (FileNotFoundException e) {
                Slog.w(TAG, "can't find file " + filePath);
            } catch (IOException e) {
                Slog.w(TAG, "IO exception when read file " + filePath);
            } catch (IndexOutOfBoundsException e) {
                Slog.w(TAG, "index exception: " + e.getMessage());
            } finally {
                if (null != reader) {
                    try {
                        reader.close();
                    } catch (IOException e) {
                        Slog.w(TAG, "close reader fail: " + e.getMessage());
                    }
                }
            }
            return content;
        }

        private void update(String newName, int newState) {
            Slog.d(TAG, "HDMIOberver.update(), oldState=" + mHdmiState
                    + ", newState=" + newState);
            // Retain only relevant bits
            int hdmiState = newState;
            int newOrOld = hdmiState | mHdmiState;
            int delay = 0;
            // reject all suspect transitions: only accept state changes from:
            // - a: 0 HDMI to 1 HDMI
            // - b: 1 HDMI to 0 HDMI

            mHdmiName = newName;
            mPrevHdmiState = mHdmiState;
            mHdmiState = hdmiState;
            if (mHdmiState == 0){
                mWakeLock.release();
                handleNotification(false);
                Slog.d(TAG, "HDMIOberver.update(), release");
            } else {
                mWakeLock.acquire();
                handleNotification(true);
                Slog.d(TAG, "HDMIOberver.update(), acquire");
            }
        }

        private void handleNotification(boolean showNoti) {
            NotificationManager notificationManager = (NotificationManager) mCxt
                    .getSystemService(Context.NOTIFICATION_SERVICE);
            if (notificationManager == null) {
                Slog.w(TAG, "Fail to get NotificationManager");
                return;
            }
            if (showNoti) {
                Slog.d(TAG, "Show notification now");
                //Notification notification = new Notification();
                /// M: Create NotificationChannel
                notificationManager.createNotificationChannel(new NotificationChannel(
                        HDMI_NOTIFICATION_CHANNEL_ID, HDMI_NOTIFICATION_NAME, NotificationManager.IMPORTANCE_LOW));
                Notification notification = new Notification.Builder(mCxt,
                        HDMI_NOTIFICATION_CHANNEL_ID).build();;
                String titleStr = mCxt.getResources().getString(
                        com.mediatek.internal.R.string.hdmi_notification_title);
                String contentStr = mCxt.getResources().getString(
                        com.mediatek.internal.R.string.hdmi_notification_content);
                notification.icon = com.mediatek.internal.R.drawable.ic_hdmi_notification;

                notification.tickerText = titleStr;
                notification.flags = Notification.FLAG_ONGOING_EVENT
                        | Notification.FLAG_NO_CLEAR
                        | Notification.FLAG_SHOW_LIGHTS;
                Intent intent = Intent
                        .makeRestartActivityTask(new ComponentName(
                                "com.android.settings",
                                "com.android.settings.HdmiSettings"));
                PendingIntent pendingIntent = PendingIntent.getActivityAsUser(
                        mCxt, 0, intent, 0, null, UserHandle.CURRENT);
                notification.setLatestEventInfo(mCxt, titleStr, contentStr,
                        pendingIntent);
                notificationManager.notifyAsUser(null,
                        com.mediatek.internal.R.drawable.ic_hdmi_notification, notification,
                        UserHandle.CURRENT);
            } else {
                Slog.d(TAG, "Clear notification now");
                notificationManager.cancelAsUser(null,
                        com.mediatek.internal.R.drawable.ic_hdmi_notification, UserHandle.CURRENT);
            }
        }

    }

}
