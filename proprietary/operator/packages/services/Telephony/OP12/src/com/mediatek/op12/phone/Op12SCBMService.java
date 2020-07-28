/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2019. All rights reserved.
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
package com.mediatek.op12.phone;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.AsyncResult;
import android.os.Binder;
import android.os.CountDownTimer;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.SystemProperties;
import android.util.Log;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.TelephonyProperties;
import com.android.internal.telephony.util.NotificationChannelController;

import com.mediatek.internal.telephony.MtkGsmCdmaPhone;
import com.mediatek.op12.phone.R;

import java.text.SimpleDateFormat;

/**
 * Application service that inserts/removes Emergency SMS Callback Mode notification and
 * updates Emergency SMS Callback Mode countdown clock in the notification
 *
 * @see Op12SCBMExitDialog
 */
public class Op12SCBMService extends Service {
    // Default Emergency SMS Callback Mode timeout value
    private static final int DEFAULT_SCM_EXIT_TIMER_VALUE = 300000;
    private static final String LOG_TAG = "Op12SCBMService";
    private static String CHANNEL_ID = "scbm_channel";

    private NotificationManager mNotificationManager = null;
    private CountDownTimer mTimer = null;
    private long mTimeLeft = 0;
    private boolean mInEmergencySMS = false;

    @Override
    public void onCreate() {
        Log.d(LOG_TAG, "Op12SCBMService onCreate");

        // Register receiver for intents
        IntentFilter filter = new IntentFilter();
        filter.addAction(TelephonyIntents.ACTION_SCBM_CHANGED);
        registerReceiver(mScmReceiver, filter);

        mNotificationManager = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);
        startTimerNotification();
    }

    @Override
    public void onDestroy() {
        // Unregister receiver
        unregisterReceiver(mScmReceiver);
        if (mTimer != null) {
            // Cancel the timer
            mTimer.cancel();
        }
        stopForeground(true);
    }

    /**
     * Listens for Emergency SMS Callback Mode intents
     */
    private BroadcastReceiver mScmReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent.getAction().equals(TelephonyIntents.ACTION_SCBM_CHANGED)) {
                // Stop the service when phone exits Emergency SMS Callback Mode
                if (intent.getBooleanExtra("phoneinSCMState", false) == false) {
                    Log.d(LOG_TAG, "ACTION_SCBM_CHANGED false");
                    stopSelf();
                }
            }
        }
    };

    /**
     * Start timer notification for Emergency SMS Callback Mode
     */
    private void startTimerNotification() {
        // Get Emergency SMS Callback Mode timeout value
        long scmTimeout = DEFAULT_SCM_EXIT_TIMER_VALUE;

        // Show the notification
        showNotification(scmTimeout);

        // Start countdown timer for the notification updates
        if (mTimer != null) {
            mTimer.cancel();
        } else {
            mTimer = new CountDownTimer(scmTimeout, 1000) {

                @Override
                public void onTick(long millisUntilFinished) {
                    mTimeLeft = millisUntilFinished;
                }

                @Override
                public void onFinish() {
                    //Do nothing
                }

            };
        }
        mTimer.start();
    }

    /**
     * Shows notification for Emergency SMS Callback Mode
     */
    private void showNotification(long millisUntilFinished) {
        final Notification.Builder builder = new Notification.Builder(getApplicationContext());
        builder.setOngoing(true);
        builder.setPriority(Notification.PRIORITY_HIGH);
        builder.setSmallIcon(R.drawable.ic_scbm);
        builder.setTicker(getText(R.string.phone_entered_scm_text));
        builder.setContentTitle(getText(R.string.phone_in_scm_notification_title));

        // PendingIntent to launch Emergency SMS Callback Mode Exit activity if the user selects
        // this notification
        PendingIntent contentIntent = PendingIntent.getActivity(this, 0,
                new Intent(Op12SCBMExitDialog.ACTION_SHOW_SCM_EXIT_DIALOG), 0);
        builder.setContentIntent(contentIntent);

        // Format notification string
        String text = null;
        if(mInEmergencySMS) {
            text = getText(R.string.phone_in_scm_call_notification_text).toString();
        } else {
            // Calculate the time in ms when the notification will be finished.
            long finishedCountMs = millisUntilFinished + System.currentTimeMillis();
            builder.setShowWhen(true);
            builder.setChronometerCountDown(true);
            builder.setUsesChronometer(true);
            builder.setWhen(finishedCountMs);

            String completeTime = SimpleDateFormat.getTimeInstance(SimpleDateFormat.SHORT).format(
                    finishedCountMs);
            text = getResources().getString(R.string.phone_in_scm_notification_complete_time,
                    completeTime);
        }
        builder.setContentText(text);

        final NotificationChannel channel = new NotificationChannel(CHANNEL_ID,
                getString(R.string.notification_channel_scbm),
                NotificationManager.IMPORTANCE_DEFAULT);
        mNotificationManager.createNotificationChannel(channel);
        builder.setChannelId(CHANNEL_ID);

        // Show notification
        startForeground(R.string.phone_in_scm_notification_title, builder.build());
    }

    @Override
    public IBinder onBind(Intent intent) {
        return mBinder;
    }

    // This is the object that receives interactions from clients.
    private final IBinder mBinder = new LocalBinder();

    /**
     * Class for clients to access
     */
    public class LocalBinder extends Binder {
        public Op12SCBMService getService() {
            return Op12SCBMService.this;
        }
    }

    /**
     * Returns Emergency SMS Callback Mode timeout value
     */
    public long getSCBMTimeout() {
        return mTimeLeft;
    }

    /**
     * Returns Emergency SMS Callback Mode call state
     */
    public boolean getSCBMState() {
        return mInEmergencySMS;
    }
}
