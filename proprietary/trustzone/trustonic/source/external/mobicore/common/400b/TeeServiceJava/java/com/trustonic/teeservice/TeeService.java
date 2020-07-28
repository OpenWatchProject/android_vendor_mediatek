/*
 * Copyright (c) 2013-2018 TRUSTONIC LIMITED
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the TRUSTONIC LIMITED nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

package com.trustonic.teeservice;

import android.util.Log;
import com.trustonic.tuiapi.TUI_Event;
import com.trustonic.tuiapi.TUI_EventType;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;

import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Build;
import android.os.Bundle;
import android.os.IBinder;
import android.os.Handler;
import android.os.Messenger;
import android.os.Message;
import android.app.Notification.Builder;
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;

import android.telephony.TelephonyManager;

import vendor.trustonic.tee.V1_0.ITee;

public class TeeService extends Service {

    private static final String TAG = TeeService.class.getSimpleName();

    TuiCallback mTuiCallback;
    ITee mTeeService;

    // First executed callback.  The callback on create is executed when the
    // service is created, either because of a call to bindService() (from a
    // client) or because of a startService (from the BOOT_COMPLETED
    // broadcastReceiver)
    @Override
    public void onCreate()
    {
        Log.d(TAG, "Entering onCreate()");

        // Start the native Tee Server
        TeeServiceNative.startTeeServiceServer();

        /* Register the tui callbacks on the vendor server
         * The callbacks gets called when the vendor partiotion request starting
         * or stoping a TUI session */
        try {
            mTeeService = ITee.getService(true);
            mTuiCallback = new TuiCallback(getApplicationContext());
            mTeeService.registerTuiCallback(mTuiCallback);
        } catch (android.os.RemoteException e) {
            Log.e(TAG, String.format("Exception %s", e.toString()));
            Log.e(TAG, "Cannot get TeeService.  Ignoring failure, but tui feature won't be available");
        }

        // Register to intent for the TUI
        IntentFilter filter = new IntentFilter();
        Intent screenIntent = new Intent(Intent.ACTION_SCREEN_OFF);
        Intent batteryIntent = new Intent(Intent.ACTION_BATTERY_LOW);
        filter.addAction(screenIntent.getAction());
        filter.addAction(batteryIntent.getAction());
        filter.addAction("android.intent.action.PHONE_STATE");
        registerReceiver(mReceiver, filter);
    }

    // Executed each time a client calls Context.StartService().  This is in
    // particular executed when the broadcastReceiver of the TeeService receives
    // the BOOT_COMPLETED intent
    @Override
    public int onStartCommand(Intent  intent, int flags, int startId)
    {
        Log.d(TAG, "onStartCommand()");

        /* make the service a foreground service */
        String channelId = "";

        return Service.START_STICKY;
    }

    private BroadcastReceiver mReceiver= new BroadcastReceiver() {

        @Override
        public void onReceive(Context context, Intent intent) {

            Runnable notifyEvent = new Runnable() {
                public void run() {
                    final TUI_Event cancel = new TUI_Event(TUI_EventType.TUI_CANCEL_EVENT);
                    try {
                        if(0 != mTeeService.notifyReeEvent(cancel.getType())) {
                            Log.e(TAG, "notifyEvent failed!");
                        }
                    } catch (android.os.RemoteException e) {
                        Log.e(TAG, String.format("Exception %s", e.toString()));
                        Log.e(TAG, "Cannot notify vendor server.  Ignoring failure, but TUI session won't be cancel");
                    }
                }
            };

            if (mTuiCallback.isSessionOpened()) {

                if((intent.getAction() == Intent.ACTION_SCREEN_OFF)){
                    Log.d(TAG,"event screen off!");
                    mTuiCallback.acquireWakeLock();
                    notifyEvent.run();
                }
                if(intent.getAction() == "android.intent.action.PHONE_STATE"){
                    Bundle bundle = intent.getExtras();
                    if(bundle != null){
                        if(bundle.getString(TelephonyManager.EXTRA_STATE).
                                equalsIgnoreCase(TelephonyManager.EXTRA_STATE_RINGING)){
                            Log.d(TAG,"event incoming call!");
                            notifyEvent.run();
                                }
                    }
                }
                if((intent.getAction() == Intent.ACTION_BATTERY_LOW)){
                    Log.d(TAG,"event battery low!");
                    /* TODO: get the battery level and only send a cancel event
                     * if this level is below a threshold that will be defined */
                    notifyEvent.run();
                }
            }
        }
    };

    /**
     * Handler of incoming messages from clients.
     */
    static class IncomingHandler extends Handler {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                default:
                    super.handleMessage(msg);
            }
        }
    }

    /**
     * Target we publish for clients to send messages to IncomingHandler.
     */
    final Messenger mMessenger = new Messenger(new IncomingHandler());

    /**
     * When binding to the service, we return an interface to our messenger
     * for sending messages to the service.
     */
    @Override
    public IBinder onBind(Intent intent) {
        Log.d(TAG, "onBind");
        return mMessenger.getBinder();
    }

    /** Called when The service is no longer used and is being destroyed */
    @Override
    public void onDestroy()
    {
    }
}
