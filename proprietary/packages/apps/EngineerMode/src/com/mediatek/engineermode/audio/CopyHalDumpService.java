/*
 *  Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly
 * prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY
 * ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY
 * THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK
 * SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO
 * RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN
 * FORUM.
 * RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
 * LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation
 * ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */
package com.mediatek.engineermode.audio;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.NotificationChannel;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.Binder;
import android.os.IBinder;
import android.os.Handler;
import android.os.Message;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;

public class CopyHalDumpService extends Service implements OnCopyProgressChangeListener {
    public static final String TAG = "Audio/Service";

    enum DUMP_STATUS {
        DEFAULT,
        COPY_HAL_DUMP,
        DELETE_HAL_DUMP,
        COPY_DUMP_DONE,
        DELETE_DUMP_DONE,
    }

    public static final int MSG_COPY_PROGRESS_UPDATE = 10;
    public static final int MSG_DELETE_PROGRESS_UPDATE = 11;
    public static final int MSG_COPY_DONE = 12;
    public static final int MSG_DELETE_DONE = 13;

    private static final String NOTIFICATION_CHANNEL = "CopyAudioHalDumpService";
    private static final String EMAUDIO_NOTIFICATION_NAME =
            "CopyAudioHalDumpService is running";
    private static final int ID_EMAUDIO_SERVICE = 197459;

    private final IBinder mBinder = new LocalBinder();
    private Handler mActivityHandler;
    private DUMP_STATUS mStatus = DUMP_STATUS.DEFAULT;

    @Override
    public void onCreate() {
        super.onCreate();
        Elog.d(TAG, "onCreate");
    }

    @Override
    public void onStart(Intent intent, int startId) {
        super.onStart(intent, startId);
    }

    @Override
    public int onStartCommand(Intent intent, int flag, int startId) {
        NotificationManager mNotificationManager =
                (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);

        NotificationChannel channel = new NotificationChannel(NOTIFICATION_CHANNEL,
                EMAUDIO_NOTIFICATION_NAME,
                NotificationManager.IMPORTANCE_LOW);
        mNotificationManager.createNotificationChannel(channel);
        Notification notification = new Notification.Builder(getApplicationContext())
                .setContentTitle(EMAUDIO_NOTIFICATION_NAME)
                .setSmallIcon(android.R.drawable.ic_dialog_info)
                .setChannelId(NOTIFICATION_CHANNEL)
                .build();
        startForeground(ID_EMAUDIO_SERVICE, notification);
        return super.onStartCommand(intent, flag, startId);
    }

    @Override
    public void onDestroy() {
        Elog.d(TAG, "onDestroy");
        super.onDestroy();
    }

    @Override
    public IBinder onBind(Intent intent) {
        return mBinder;
    }

    /**
     * Class used for the client Binder. Because we know this service always
     * runs in the same process as its clients, we don't need to deal with IPC.
     */
    public class LocalBinder extends Binder {
        public CopyHalDumpService getService(Handler handler) {
            mActivityHandler = handler;
            return CopyHalDumpService.this;
        }
    }

    public void removeUiHandler() {
        mActivityHandler = null;
        if(mStatus == DUMP_STATUS.COPY_DUMP_DONE || mStatus == DUMP_STATUS.DELETE_DUMP_DONE) {
            Elog.v(TAG, "reset dump status");
            mStatus = DUMP_STATUS.DEFAULT;
        }
    }

    public DUMP_STATUS getCopyDumpStatus() {
        return mStatus;
    }

    @Override
    public void onCopyProgressChanged (String fileName, float readSize, float tSize) {
        if(mActivityHandler == null) {
            Elog.v(TAG, "mActivityHandler is null, don't need update COPY UI");
            return;
        }
        Elog.v(TAG, fileName + ":" + readSize + "/" + tSize);
        if("SUCCESS".equals(fileName)) {
            Elog.v(TAG, "[onCopyProgressChanged] done");
            mActivityHandler.sendEmptyMessage(MSG_COPY_DONE);
            mStatus = DUMP_STATUS.COPY_DUMP_DONE;
        } else {
            String text = getString(R.string.Audio_copy_progress, fileName, readSize, tSize);
            Message msg = mActivityHandler.obtainMessage(MSG_COPY_PROGRESS_UPDATE, text);
            mActivityHandler.sendMessage(msg);
            mStatus = DUMP_STATUS.COPY_HAL_DUMP;
        }
    }

    @Override
    public void onDeleteProgressChanged (String fileName) {
        if(mActivityHandler == null) {
            Elog.v(TAG, "mActivityHandler is null, don't need update DELETE UI");
            return;
        }
        Elog.v(TAG, fileName);
        if("SUCCESS".equals(fileName)) {
            Elog.v(TAG, "[onDeleteProgressChanged] done");
            mActivityHandler.sendEmptyMessage(MSG_DELETE_DONE);
            mStatus = DUMP_STATUS.DELETE_DUMP_DONE;
        } else {
            Elog.v(TAG, "[onDeleteProgressChanged] delete" + fileName);
            String text = getString(R.string.Audio_delete_progress, fileName);
            Message msg = mActivityHandler.obtainMessage(MSG_COPY_PROGRESS_UPDATE, text);
            mActivityHandler.sendMessage(msg);
            mStatus = DUMP_STATUS.DELETE_HAL_DUMP;
        }
    }
}