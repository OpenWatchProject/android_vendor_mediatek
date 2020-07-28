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

package com.mediatek.op07.wfo;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;

import android.net.ConnectivityManager;
import android.service.notification.StatusBarNotification;
import android.telephony.PhoneStateListener;
import android.telephony.TelephonyManager;
import android.telephony.SignalStrength;
import android.util.Log;

import com.android.ims.ImsManager;

import com.mediatek.wfo.op.DefaultWosExt;

public class Op07WosExt extends DefaultWosExt {
    static final private String TAG = "Op07WosExt";

    static final private String NOTIFICATION_TAG = "Op07WosExt";

    static final private String CHANNEL_ID = "Op07WosExt_CHANNEL_ID";

    static private final int POOR_RSRP = -114; // dBm

    static private final int MAX_NOTIFICATION_COUNT = 3;

    private Context mContext;
    private ConnectivityManager mCM;
    private TelephonyManager mTelephonyManager;
    private NotificationManager mNotificationManager;

    // Prevous call state
    private int mPreCallState = TelephonyManager.CALL_STATE_IDLE;
    private int mLteSignal;

    private int mNotificationTimes;

    private boolean mInGoodRsrpArea = false;
    private static final String ENTITLEMENT_APP_PATH
        = "com.mediatek.op07.settings/com.mediatek.op07.settings.WfcSettingsActivity";

    private static final String SHOULD_NOT_RESOLVE = "SHOULDN'T RESOLVE!";

    PhoneStateListener mPhoneStateListener = new PhoneStateListener() {
        @Override
        public void onSignalStrengthsChanged(SignalStrength signal) {

            mLteSignal = signal.getLteRsrp();
            Log.d(TAG, "onSignalStrengthsChanged(), mLteSignal: " + mLteSignal);

            tryToShowNotification(false);
        }

        @Override
        public void onCallStateChanged(int state, String incomingNumber) {
            Log.d(TAG, "onCallStateChanged(), state: " + state);

            //call drop
            if (mPreCallState == TelephonyManager.CALL_STATE_OFFHOOK &&
                state == TelephonyManager.CALL_STATE_IDLE) {

                tryToShowNotification(true);
            }

            mPreCallState = state;
        }
    };

    public Op07WosExt(Context context) {
        super(context);
        mContext = context;

        mNotificationTimes = 0;

        mCM = (ConnectivityManager) mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
        mTelephonyManager = (TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE);
        mNotificationManager =
                (NotificationManager) mContext.getSystemService(
                        Context.NOTIFICATION_SERVICE);

        mTelephonyManager.listen(mPhoneStateListener, PhoneStateListener.LISTEN_SIGNAL_STRENGTHS |
                                                      PhoneStateListener.LISTEN_CALL_STATE);
        Log.d(TAG, "Op07WosExt() constructor end");
    }

    @Override
    public void factoryReset() {
       mNotificationTimes = 0;
    }

    private void tryToShowNotification(boolean callStateChange) {
        Log.d(TAG, "tryToShowNotification(): callStateChange: "+callStateChange);
        if (mLteSignal >= POOR_RSRP) {
            mInGoodRsrpArea = true;
            return;
        }
        if (ImsManager.isWfcEnabledByPlatform(mContext) &&
            !ImsManager.isWfcEnabledByUser(mContext) &&
            !isNotificationExisted() &&
            mLteSignal < POOR_RSRP &&
            (callStateChange || (!callStateChange && mInGoodRsrpArea)) &&
            isWifiConnected() &&
            mNotificationTimes < MAX_NOTIFICATION_COUNT) {

            mInGoodRsrpArea = false;

            Log.d(TAG, "onSignalStrengthsChanged"
                + ", lteSignal: " + mLteSignal
                + ", isWifiConnected: " + isWifiConnected()
                + ", isEnabledByPlatform: " +  ImsManager.isWfcEnabledByPlatform(mContext)
                + ", isEnabledByUser: " + ImsManager.isWfcEnabledByUser(mContext)
                + ", isNotificationExisted: " + isNotificationExisted()
                + ", mNotificationTimes: " + mNotificationTimes);


            mNotificationTimes ++;
            showPoorCellularSignalNotification();
        }
    }

    private boolean isNotificationExisted() {
        StatusBarNotification[] notifications = mNotificationManager.getActiveNotifications();

        for (StatusBarNotification notification : notifications) {

            if (notification.getTag().equals(NOTIFICATION_TAG)) {

                return true;
            }
        }

        return false;
    }

    private void showPoorCellularSignalNotification() {
        Log.d(TAG, "showPoorCellularSignalNotification");

        String title = mContext.getResources().getString(R.string.wifi_calling);
        String subText = mContext.getResources().getString(R.string.turn_it_on_now);

        ComponentName componentName = ComponentName.unflattenFromString(ENTITLEMENT_APP_PATH);
        // Build and return intent
        Intent resultIntent = new Intent();


        if (isPackageExist(componentName)) {
            resultIntent.setComponent(componentName);
            resultIntent.putExtra("enable_wfc", true);
        } else {
            resultIntent.setComponent(new ComponentName(SHOULD_NOT_RESOLVE, SHOULD_NOT_RESOLVE));
        }

        PendingIntent pendingIntent = PendingIntent.getActivity(
                mContext, 0, resultIntent, PendingIntent.FLAG_UPDATE_CURRENT);

        createNotificationChannel();

        Notification notification =
                new Notification.Builder(mContext)
                        .setSmallIcon(android.R.drawable.stat_sys_warning)
                        .setContentTitle(title)
                        .setContentText(subText)
                        .setAutoCancel(true)
                        .setContentIntent(pendingIntent)
                        .setChannel(CHANNEL_ID)
                        .build();

        mNotificationManager.notify(NOTIFICATION_TAG, 0,
                notification);
    }

    private boolean isWifiConnected() {
        boolean isConnected = false;
        if (mCM != null) {
            isConnected = mCM.getNetworkInfo(ConnectivityManager.TYPE_WIFI).isConnected();
        }
        return isConnected;
    }


    private boolean isPackageExist(ComponentName componentName) {
        try {
            mContext.getPackageManager().getActivityInfo(componentName, 0);
        } catch (PackageManager.NameNotFoundException e) {
            Log.d(TAG, "package exist: false");
            return false;
        }
        Log.d(TAG, "package exist: true");
        return true;
    }

    private void createNotificationChannel() {
        CharSequence name = mContext.getResources().getString(R.string.channel_name);
        String description = mContext.getResources().getString(R.string.channel_description);
        int importance = NotificationManager.IMPORTANCE_DEFAULT;

        NotificationChannel channel = new NotificationChannel(CHANNEL_ID, name, importance);
        channel.setDescription(description);

        mNotificationManager.createNotificationChannel(channel);
    }




}
