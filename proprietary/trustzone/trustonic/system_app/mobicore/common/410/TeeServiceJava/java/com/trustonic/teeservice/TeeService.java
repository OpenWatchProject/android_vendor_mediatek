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

import java.security.MessageDigest;
import java.util.Arrays;

import android.app.Service;
import android.app.Notification.Builder;
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.content.Intent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.content.pm.PackageInfo;
import android.content.pm.Signature;
import android.content.res.Configuration;
import android.graphics.Point;
import android.telephony.TelephonyManager;
import android.os.IBinder;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Messenger;
import android.os.Message;
import android.util.Log;
import android.view.WindowManager;

import com.trustonic.tuiapi.TUI_Event;
import com.trustonic.tuiapi.TUI_EventType;

import vendor.trustonic.tee.tui.V1_0.ITui;

public class TeeService extends Service {

    private static final String TAG = TeeService.class.getSimpleName();
    private static Context mContext;
    private static int mDisplayWidth = 0;
    private static int mDisplayHeight = 0;
    private static int mDisplayDpi = 0;

    TuiCallback mTuiCallback;
    ITui mTuiService;

    // First executed callback.  The callback on create is executed when the
    // service is created, either because of a call to bindService() (from a
    // client) or because of a startService (from the BOOT_COMPLETED
    // broadcastReceiver)
    @Override
    public void onCreate()
    {
        Log.d(TAG, "Entering onCreate()");
        mContext = this;

        // Start the native Tee Server
        startTeeServiceServer(this);
        registerTeeCallback();

        /* Register the tui callbacks on the vendor server
         * The callbacks gets called when the vendor partiotion request starting
         * or stoping a TUI session */
        try {
            mTuiService = ITui.getService(true);
            Log.e(TAG, "registerTuiCallback");
            mTuiCallback = new TuiCallback(getApplicationContext());
            mTuiService.registerTuiCallback(mTuiCallback);
        } catch (android.os.RemoteException e) {
            Log.e(TAG, String.format("Exception %s", e.toString()));
            Log.e(TAG, "Cannot get TeeService.  Ignoring failure, but tui feature won't be available");
        } catch (NullPointerException e) {
            // `mTeeService` may be null.  In this case, it is not possible to
            // notify it of a Ree Event. Just ignore the error.
        }

        // Register to intent for the TUI
        IntentFilter filter = new IntentFilter();
        Intent screenIntent = new Intent(Intent.ACTION_SCREEN_OFF);
        Intent batteryIntent = new Intent(Intent.ACTION_BATTERY_LOW);
        filter.addAction(screenIntent.getAction());
        filter.addAction(batteryIntent.getAction());
        filter.addAction("android.intent.action.PHONE_STATE");
        registerReceiver(mReceiver, filter);
        /* setScreenInfo is for specific platforms
         * that rely on onConfigurationChanged to set resolution
         * it has no effect on Trustonic reference implementaton. 
         */
        setScreenInfo(mContext.getResources().getConfiguration());
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

    /* onConfigurationChanged is for specific platforms
     * that rely on onConfigurationChanged to set resolution
     * it has no effect on Trustonic reference implementaton. 
     */
    @Override
    public void onConfigurationChanged(Configuration newConfig)
    {
        Log.d(TAG, "onConfigurationChanged, old dpi=" + mDisplayDpi + ", new dpi=" + newConfig.densityDpi);
        if (mDisplayDpi != newConfig.densityDpi) {
                setScreenInfo(newConfig);
        }
    }

    private BroadcastReceiver mReceiver= new BroadcastReceiver() {

        @Override
        public void onReceive(Context context, Intent intent) {

            Runnable notifyEvent = new Runnable() {
                public void run() {
                    final TUI_Event cancel = new TUI_Event(TUI_EventType.TUI_CANCEL_EVENT);
                    try {
                        if(0 != mTuiService.notifyReeEvent(cancel.getType())) {
                            Log.e(TAG, "notifyEvent failed!");
                        }
                    } catch (android.os.RemoteException e) {
                        Log.e(TAG, String.format("Exception %s", e.toString()));
                        Log.e(TAG, "Cannot notify vendor server.  Ignoring failure, but TUI session won't be cancel");
                    } catch (NullPointerException e) {
                        // `mTeeService` may be null, if the activity failed to
                        // retrieve it in the onCreate method. In this case, it
                        // is not possible to notify it of a Ree Event. Just
                        // ignore the error
                    }
                }
            };

            if (mTuiCallback.isSessionOpened()) {

                if((intent.getAction().equals(Intent.ACTION_SCREEN_OFF))){
                    Log.d(TAG,"event screen off!");
                    mTuiCallback.acquireWakeLock();
                    notifyEvent.run();
                }
                if(intent.getAction().equals("android.intent.action.PHONE_STATE")){
                    Bundle bundle = intent.getExtras();
                    if(bundle != null){
                        if(bundle.getString(TelephonyManager.EXTRA_STATE).
                                equalsIgnoreCase(TelephonyManager.EXTRA_STATE_RINGING)){
                            Log.d(TAG,"event incoming call!");
                            notifyEvent.run();
                                }
                    }
                }
                if((intent.getAction().equals(Intent.ACTION_BATTERY_LOW))){
                    Log.d(TAG,"event battery low!");
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
    public void onDestroy() {
        Log.d(TAG, "onDestroy");
    }

    private byte[] getApkIdentityHash(int uid) {
        byte[] truncatedHash= null;
        final PackageManager pm = getPackageManager();
        if (pm == null) {
            Log.d(TAG, "Cannot get PackageManager");
        } else {
            String packageName = pm.getNameForUid(uid);
            Log.d(TAG, "packageName=" + packageName);
            PackageInfo packageInfo = null;
            try {
                packageInfo = pm.getPackageInfo(packageName,
                        PackageManager.GET_SIGNATURES);
            } catch (PackageManager.NameNotFoundException e) {
                e.printStackTrace();
                return truncatedHash;
            }

            // XXX: Use deprectaed API because when using the new API, the
            // getPackageInfo method used with flag
            // PackageManager.GET_SIGNING_CERTIFICATES is randomly crashing
            // causing a null pointer dereference fault.
            Signature[] signatures = packageInfo.signatures;
            byte[] cert = signatures[0].toByteArray();
            // infos = (PACKAGE_NAME | SIGNATURE)
            byte[] infos = null;
            try {
                infos = new byte[packageName.getBytes().length + cert.length];
                System.arraycopy(packageName.getBytes(), 0, infos, 0, packageName.getBytes().length);
                System.arraycopy(cert, 0, infos, packageName.getBytes().length, cert.length);
            } catch (Exception e) {
                Log.e(TAG, String.format("Exception %s", e.toString()));
                return truncatedHash;
            }
            try {
                MessageDigest digest = MessageDigest.getInstance("SHA1");

                digest.update(infos);
                truncatedHash = Arrays.copyOfRange(digest.digest(), 0, 16);
            } catch (Exception e) {
                Log.e(TAG, String.format("Exception %s", e.toString()));
            }
        }
        return truncatedHash;
    }

    /* setScreenInfo is for specific platforms
     * that rely on onConfigurationChanged to set resolution
     * it has no effect on Trustonic reference implementaton. 
     */
    private synchronized void setScreenInfo(Configuration config) {
        int density = mContext.getResources().getDisplayMetrics().densityDpi;

        Point realSize = new Point();
        WindowManager wm = (WindowManager)mContext.getSystemService(WINDOW_SERVICE);
        wm.getDefaultDisplay().getRealSize(realSize);

        if (config.orientation == Configuration.ORIENTATION_LANDSCAPE) {
            mDisplayWidth = realSize.y;
            mDisplayHeight = realSize.x;
        } else if (config.orientation == Configuration.ORIENTATION_PORTRAIT) {
            mDisplayWidth = realSize.x;
            mDisplayHeight = realSize.y;
        }

        Log.d(TAG, "WxH: " + mDisplayWidth + "x" + mDisplayHeight + " - dpi: " + density);
        try {
            if(0 != mTuiService.notifyScreenSizeUpdate(mDisplayWidth, mDisplayHeight)) {
                Log.e(TAG, "notifyEvent failed!");
            }
        } catch (android.os.RemoteException e) {
            Log.e(TAG, String.format("Exception %s", e.toString()));
            Log.e(TAG, "Cannot notify vendor server.  Ignoring failure, but display settings may be wrong");
        } catch (NullPointerException e) {
            // `mTeeService` may be null, if the activity failed to
            // retrieve it in the onCreate method. In this case, it
            // is not possible to notify it of a Ree Event. Just
            // ignore the error
        }
        mDisplayDpi = density;
    }

    /* Native functions */
    public static native int startTeeServiceServer(Context ctxt);
    public static native int registerTeeCallback();

    /**
     * this is used to load the library on application startup. The
     * library has already been unpacked to the app specific folder
     * at installation time by the package manager.
     */
    static {
        System.loadLibrary("TeeServiceJni");
    }

}
