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

package com.mediatek.connectivity;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.Binder;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.util.Log;

import java.net.Inet4Address;
import java.net.Inet6Address;
import java.net.InetAddress;

/**
  *
  * Android service for Ping command.
  * Support Ping command in the background
  *
 **/
public class CdsPingService extends Service {
    private static final String TAG = "CDSINFO/Ping_SRV";

    private static Handler sPingHandler;

    private static final int EVENT_PING_CMD = 0x1001;

    private static final int IPV4 = 0;
    private static final int IPV6 = 1;

    private Context mContext;
    private ServiceBinder mBinder = new ServiceBinder();
    private boolean mIsStop;
    private int mPingType;
    private int mPingCount;
    private int mPingInterval;
    private String mPingHost;

    private StringBuilder mOutputSb = new StringBuilder("");

    @Override
    public void onCreate() {
        super.onCreate();

        mContext = this.getApplicationContext();

        HandlerThread pingThread = new HandlerThread(TAG);
        pingThread.start();
        sPingHandler = new PingCmdHandler(pingThread.getLooper());

        setServiceForeground();
        Log.i(TAG, "CdsPingService is started");
    }

    /** Handler to do the network accesses on. */
    private class PingCmdHandler extends Handler {

        public PingCmdHandler(Looper l) {
            super(l);
        }

        @Override
        public void handleMessage(Message msg) {
            Log.d(TAG, "handleMessage:" + msg.what);
            switch (msg.what) {
            case EVENT_PING_CMD:
                boolean isEcho = false;
                String host = mPingHost;
                String msgLine;
                InetAddress inetAddress = null;
                int count = mPingCount;
                int interval = mPingInterval;
                int i = 0;

                if (hasMessages(EVENT_PING_CMD)) {
                    removeMessages(EVENT_PING_CMD);
                }
                try {


                    mIsStop = false;
                    mOutputSb.setLength(0);
                    InetAddress[] inetAddressSet = InetAddress.getAllByName(host);
                    for (i = 0; i < inetAddressSet.length; i ++) {
                        inetAddress = inetAddressSet[i];
                        if (mPingType == IPV4) {
                            if (inetAddress instanceof Inet4Address) {
                                break;
                            }
                        } else if (mPingType == IPV6) {
                            if (inetAddress instanceof Inet6Address) {
                                inetAddress = inetAddressSet[i];
                                break;
                            }
                        }
                    }

                    if (i == inetAddressSet.length || inetAddress == null) {
                        mOutputSb.append("There is no available IPv4/v6 address");
                        return;
                    }

                    if (count > 0) {
                        while (count > 0) {
                            count--;
                            isEcho = inetAddress.isReachable(1000);
                            Log.i(TAG, "Ping result:" + isEcho);
                            if (isEcho) {
                                msgLine = inetAddress + ": reply from host\n";
                            } else {
                                msgLine = inetAddress + ": no reply from host\n";
                            }
                            mOutputSb.append(msgLine);
                            Thread.sleep(interval * 1000);
                            if (mIsStop) {
                                break;
                            }
                        }
                        mOutputSb.append("Ping is done\n");
                    } else {
                        while (true) {
                            isEcho = inetAddress.isReachable(1000);
                            Log.i(TAG, "Ping result:" + isEcho);
                            if (isEcho) {
                                msgLine = inetAddress + ": reply from host\n";
                            } else {
                                msgLine = inetAddress + ": no reply from host\n";
                            }
                            mOutputSb.append(msgLine);
                            Thread.sleep(interval * 1000);
                            if (mIsStop) {
                                break;
                            }
                        }
                        mOutputSb.append("Ping is done\n");
                    }
                } catch (Exception e) {
                    e.printStackTrace();
                    mOutputSb.append("Ping error:" + e + "\n");
                }
                break;
            default:
                break;
            }
        }
    }

    @Override
    public IBinder onBind(Intent intent) {
        return mBinder;
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        return Service.START_STICKY;
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "onDestroy()");
        stopForeground(true);
        super.onDestroy();
    }

    /**
      * This is an IPC communication binder between activity and service.
      * Provide service APIs for activity to use.
      *
    */
    public class ServiceBinder extends Binder {
        /**
         *
         * API to execute ping command.
         * @param cmd indicates ping command to execute.
         * @throws InterruptedException if Interrupted exception is occurred.
         *
         */
        public void execCommand(String host, int count,
            int interval, int type) throws InterruptedException {
            if (sPingHandler.hasMessages(EVENT_PING_CMD)) {
                sPingHandler.removeMessages(EVENT_PING_CMD);
            }
            mPingHost = host;
            mPingCount = count;
            mPingInterval = interval;
            mPingType = type;
            sPingHandler.obtainMessage(EVENT_PING_CMD).sendToTarget();
        }

        /**
         * Provide ping output result.
         * @return the output result.
         */
        public String getOutput() {
            return mOutputSb.toString();
        }

        /**
         * Terminate the ping command.
         */
        public void finish() {
            mIsStop = true;
            mOutputSb.setLength(0);
            Log.d(TAG, "finish is called");
        }
    }

    private Intent getLaunchIntent() {
        Intent intent = new Intent(mContext, CdsUtilityActivity.class);
        return intent;
    }

    private void setServiceForeground() {
        Log.i(TAG, "setServiceForeground");
        String channelId = "cds_id";

        NotificationManager notificationManager = (NotificationManager)  mContext
                .getSystemService(Context.NOTIFICATION_SERVICE);
        NotificationChannel notificationChannel = new NotificationChannel(
            channelId, "CDS APP", NotificationManager.IMPORTANCE_LOW);
        notificationManager.createNotificationChannel(notificationChannel);

        PendingIntent pIntent = PendingIntent.getActivity(this, 0,
                                getLaunchIntent(), 0);
        Notification notification = new Notification.Builder(this)
        .setAutoCancel(true)
        .setContentTitle("CDS Ping Service")
        .setContentText("Service is Running! Tap to launch Activity.")
        .setContentIntent(pIntent)
        .setWhen(System.currentTimeMillis())
        .setChannel(channelId)
        .build();

        Log.i(TAG, String.format("notification = %s", notification));
        startForeground(0x5678, notification);   // notification ID
    }

}