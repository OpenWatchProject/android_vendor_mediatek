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

package com.mediatek.entitlement.fcm;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Message;
import android.util.Log;

import com.google.firebase.iid.FirebaseInstanceId;
import com.google.firebase.messaging.FirebaseMessaging;

import com.google.android.gms.common.ConnectionResult;
import com.google.android.gms.common.GoogleApiAvailability;

import java.io.IOException;

public class FcmService extends Service {
    private static final String TAG = "FcmService";

    private static final String[] TOPICS = {"global"};

    static final protected int CMD_QUERY_FCM_TOKEN = 1;

    static final private int RETRY_COUNT = 60;  // 60 times, total retry 5 minutes
    static final private int RETRY_DELAY = 5000; // 5 seconds
    int mRetryCount;

    Context mContext;
    Handler mHandler;
    HandlerThread mHandlerThread;


    public FcmService() {
    }

    @Override
    public void onCreate() {
        super.onCreate();
        Log.d(TAG, "onCreate()");
        mContext = getApplicationContext();

        // Start handler thread for Fcm Service
        mHandlerThread = new HandlerThread("FcmService");
        mHandlerThread.start();
        initHandler();
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        Log.d(TAG, "onDestory()");
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.d(TAG, "onStartCommand: " + intent + " " + startId);

        startForeground();

        if (!checkPlayServices()) {
            return START_NOT_STICKY;
        }

        // clear message and reset parameter
        if (mHandler.hasMessages(CMD_QUERY_FCM_TOKEN)) {
            mHandler.removeMessages(CMD_QUERY_FCM_TOKEN);
        }
        mRetryCount = 0;

        // Must use non-main thread to query FCM token
        mHandler.sendMessage(mHandler.obtainMessage(CMD_QUERY_FCM_TOKEN));

        return START_NOT_STICKY;
    }

    private void startForeground() {

        String NOTIFICATION_CHANNEL = "FCM Channel";
        String NOTIFICATION_NAME = "FCM Name";

        NotificationChannel channel = new NotificationChannel(NOTIFICATION_CHANNEL, NOTIFICATION_NAME,
                NotificationManager.IMPORTANCE_LOW);

        NotificationManager notificationManager =
                (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);

        notificationManager.createNotificationChannel(channel);
        Notification notification = new Notification.Builder(mContext, NOTIFICATION_CHANNEL)
                .setContentTitle(NOTIFICATION_NAME)
                .setSmallIcon(android.R.drawable.ic_dialog_info)
                .setChannelId(NOTIFICATION_CHANNEL)
                .build();

        // The identifier for this notification must not be 0
        startForeground(1, notification);
        stopForeground(STOP_FOREGROUND_REMOVE);
    }

    private void sendBroadcast(String token) {
        Log.d(TAG, "sendBroadcast(), FCM_NEW_TOKEN_INTENT, token:" + token);

        Intent intent = new Intent(FcmListener.FCM_NEW_TOKEN_INTENT);
        intent.setPackage(FcmListener.ENTITLEMENT_PACKAGE_NAME);

        intent.putExtra(FcmListener.FCM_INTENT_KEY_TOKEN, token);
        getApplicationContext().sendBroadcast(intent, FcmListener.FCM_BROADCAST_PERMISSION);

    }

    private void queryFcmToken() {

        String token = registerFcmToken();

        if (token != null) {

            sendBroadcast(token);

        } else if (++mRetryCount < RETRY_COUNT) {

            Log.d(TAG, "queryFcmToken(), retry " + mRetryCount + " after " + RETRY_DELAY);
            mHandler.sendMessageDelayed(
                    mHandler.obtainMessage(CMD_QUERY_FCM_TOKEN),
                    RETRY_DELAY);
        } else {

            Log.d(TAG, "queryFcmToken(), fail, no retry again");
        }
    }

    private void initHandler() {
        /** Handler to handle internal message command, run on non-main thread */
        mHandler = new Handler(mHandlerThread.getLooper()) {

            @Override
            public void handleMessage(Message msg) {
                Log.d(TAG, "handleMessage(), what: " + msg.what);

                switch (msg.what) {
                    case CMD_QUERY_FCM_TOKEN:

                        queryFcmToken();

                        break;
                    default:
                        break;
                }
            }
        };
    }

    private boolean checkPlayServices() {
        int resultCode = GoogleApiAvailability.getInstance().
                isGooglePlayServicesAvailable(mContext);

        if (resultCode == ConnectionResult.SUCCESS) {
            return true;
        }
        Log.e(TAG, "checkPlayServices(), no gms package !!!");
        return false;
    }


    /*
     * trigger register for fcm, and get the token.
     *
     * @return FCM Token.
     */
    private String registerFcmToken() {
        Log.i(TAG, "registerFcmToken()");
        try {

            //Log.i(TAG, "Call FirebaseApp.initializeApp()");
            //FirebaseApp.initializeApp(context);

            // Initially this call goes out to the network to retrieve the token, subsequent calls
            // are local.
            // R.string.gcm_defaultSenderId (the Sender ID) is typically derived from google-services.json.
            // See https://developers.google.com/cloud-messaging/android/start for details on this file.
            FirebaseInstanceId instanceID = FirebaseInstanceId.getInstance();
            String token = instanceID.getToken("418816648224",
                    FirebaseMessaging.INSTANCE_ID_SCOPE);

            Log.i(TAG, "FCM token: (418816648224)" + token);

            // Subscribe to topic channels
            subscribeTopics();

            return token;
        } catch (Exception e) {
            Log.e(TAG, "Failed to complete token refresh", e);
        }

        return null;
    }

    /**
     * Subscribe to any FCM topics of interest, as defined by the TOPICS constant.
     *
     * @param token FCM token
     * @throws IOException if unable to reach the FCM PubSub service
     */
    private void subscribeTopics() throws IOException {
        FirebaseMessaging fbm = FirebaseMessaging.getInstance();
        for (String topic : TOPICS) {
            FirebaseMessaging.getInstance().subscribeToTopic(topic);
        }

    }

}
