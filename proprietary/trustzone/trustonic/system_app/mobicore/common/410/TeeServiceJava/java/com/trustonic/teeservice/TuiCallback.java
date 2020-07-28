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

import android.app.Notification.Builder;
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Point;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.Messenger;
import android.os.PowerManager;
import android.os.RemoteException;
import android.os.SystemClock;
import android.telephony.TelephonyManager;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Display;
import android.view.WindowManager;
import com.trustonic.tuiapi.TUI_Event;
import com.trustonic.tuiapi.TUI_EventType;
import com.trustonic.teeservice.TuiActivity.PerformActionInBackground;
import java.util.LinkedList;
import java.util.concurrent.atomic.AtomicBoolean;
import vendor.trustonic.tee.tui.V1_0.ITuiCallback;

public class TuiCallback extends ITuiCallback.Stub {

    private static final String TAG = TuiCallback.class.getSimpleName();

//    public static final int INIT_SESSION    = 1;
    public static final int CLOSE_SESSION   = 2;
    public static final int MAX_WAIT_MS = 5000;

    private static PerformActionInBackground handler    = null;
    private static final Object sessionSignal           = new Object();
    private static final Object startSignal             = new Object();
    private static final Object finishSignal            = new Object();
    private static boolean sessionOpened                = false;
    private static boolean activityCreationAllowed      = false;
    private static AtomicBoolean isActityAlive = new AtomicBoolean();
    private static AtomicBoolean isActivityCreated = new AtomicBoolean();

    private Context mContext;
    private PowerManager pm;
    private PowerManager.WakeLock wl;


    TuiCallback(Context ctx) {
        mContext = ctx;
        pm = (PowerManager) ctx.getSystemService(Context.POWER_SERVICE);
        wl  = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, "TuiService");
    }

    public static boolean isSessionOpened() {
        synchronized (sessionSignal) {
            return sessionOpened;
        }
    }
    public static void setSessionOpened(boolean sessionOpened) {
        synchronized (sessionSignal) {
            TuiCallback.sessionOpened = sessionOpened;
        }
    }

    public static Object getStartSignal() {
        return startSignal;
    }

    public static boolean isActivityCreationAllowed() {
        return activityCreationAllowed;
    }

    public static Object getFinishSignal() {
        return finishSignal;
    }

    public static PerformActionInBackground getHandler() {
        return handler;
    }
    public static void setHandler(PerformActionInBackground handler) {
        TuiCallback.handler = handler;
    }

    public static void setIsActityAlive(boolean status) {
        TuiCallback.isActityAlive.set(status);
    }

    public static void setIsActivityCreated(boolean status) {
        TuiCallback.isActivityCreated.set(status);
    }

    public int startSession() {
        Log.d(TAG, "startSession!");
        try {
            synchronized (startSignal) {
                activityCreationAllowed = true;
                /* create activities */
                Intent myIntent = new Intent(mContext, TuiActivity.class);
                myIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK
                        | Intent.FLAG_DEBUG_LOG_RESOLUTION
                        | Intent.FLAG_ACTIVITY_NO_ANIMATION);
                mContext.startActivity(myIntent);
                
                /* Wait activity created (TBUG-1452)*/
                long startTime = SystemClock.uptimeMillis();
                startSignal.wait(MAX_WAIT_MS);
                long waitTime = SystemClock.uptimeMillis();
                if (waitTime >= startTime + MAX_WAIT_MS) {
                    Log.i(TAG, "Wait too much... finish!!");
                    stopSession();
                    return -42;
                }
                
                activityCreationAllowed = false;
                if( ! TuiCallback.isActityAlive.get()) {
                    Log.d(TAG, "ERROR ACTIVITY timout");
                    stopSession();
                    return -42;
                }
            }
        } catch (InterruptedException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }

        /* Enable cancel events catching */
        synchronized (sessionSignal) {
            TuiCallback.sessionOpened = true;
        }

        return 0;
    }

    public void acquireWakeLock() {
        /* Ensure that CPU is still running */
        try {
            wl.acquire();
        } catch (Exception e1) {
            // TODO Auto-generated catch block
            e1.printStackTrace();
        }
    }

    public int stopSession(){
        Log.d(TAG, "stopSession!");
        if(TuiCallback.isActivityCreated.get()) {
            /* Disable cancel events catching */
            synchronized (sessionSignal) {
                TuiCallback.sessionOpened = false;
            }

            try{
                synchronized (finishSignal) {
                    /* Send a message to the activity UI thread */
                    handler.sendMessage(handler.obtainMessage(CLOSE_SESSION));
                    /* Wait activity closed */
                    long tBefore=System.currentTimeMillis();
                    finishSignal.wait(5000);
                    if ((System.currentTimeMillis() - tBefore) < 5000){
                        TuiCallback.setIsActivityCreated(false);
                    }
                }
            }catch (Exception e) {
                // TODO: handle exception
                e.printStackTrace();
            }
        }
        try {
            if (wl.isHeld()) {
                wl.release();
            }
        } catch (Exception e2) {
            // TODO Auto-generated catch block
            e2.printStackTrace();
        }
        return 0;
    }


    public void getResolution(getResolutionCallback _hidl_cb) throws android.os.RemoteException {
        Point displayMetrics = new Point();
        WindowManager wm = (WindowManager) mContext.getApplicationContext().getSystemService(mContext.WINDOW_SERVICE);
        wm.getDefaultDisplay().getRealSize(displayMetrics);
        int screenWidth = displayMetrics.x;
        int screenHeight = displayMetrics.y;

        // set the output arguments by calling the callback
        _hidl_cb.onValues(0, screenWidth, screenHeight);
    }
}
