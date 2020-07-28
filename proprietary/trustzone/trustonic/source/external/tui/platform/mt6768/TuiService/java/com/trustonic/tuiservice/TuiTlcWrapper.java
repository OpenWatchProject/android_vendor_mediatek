/*
 * Copyright (c) 2013-2016 TRUSTONIC LIMITED
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

package com.trustonic.tuiservice;

import java.util.LinkedList;
import java.util.concurrent.atomic.AtomicBoolean;

import com.trustonic.tuiservice.TuiActivity.PerformActionInBackground;
import com.trustonic.util.tLog;

import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.PowerManager;
import android.graphics.Point;
import android.view.Display;
import android.util.DisplayMetrics;
import android.view.WindowManager;
import android.os.RemoteException;

public class TuiTlcWrapper {
    private static final String TAG = TuiTlcWrapper.class.getSimpleName();

//    public static final int INIT_SESSION    = 1;
    public static final int CLOSE_SESSION   = 2;

    private static Context context                      = null;
    private static PerformActionInBackground handler    = null;
    private static final Object sessionSignal           = new Object();
    private static final Object startSignal             = new Object();
    private static final Object finishSignal            = new Object();
    private static boolean sessionOpened                = false;
    private static boolean activityCreationAllowed      = false;
    private static AtomicBoolean isActityAlive = new AtomicBoolean();
    private static AtomicBoolean isActivityCreated = new AtomicBoolean();

    private static PowerManager pm;
    private static PowerManager.WakeLock wl;


    public static boolean isSessionOpened() {
        synchronized (sessionSignal) {
            return sessionOpened;
        }
    }
    public static void setSessionOpened(boolean sessionOpened) {
        synchronized (sessionSignal) {
            TuiTlcWrapper.sessionOpened = sessionOpened;
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
        TuiTlcWrapper.handler = handler;
    }

    public static void setIsActityAlive(boolean status) {
        TuiTlcWrapper.isActityAlive.set(status);
    }

    public static void setIsActivityCreated(boolean status) {
        TuiTlcWrapper.isActivityCreated.set(status);
    }

    public static boolean startTuiSession(){

	    try {
            synchronized (startSignal) {
                activityCreationAllowed = true;
                /* create activities */
                Intent myIntent = new Intent(context, TuiActivity.class);
                myIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK
                        | Intent.FLAG_DEBUG_LOG_RESOLUTION
                        | Intent.FLAG_ACTIVITY_NO_ANIMATION);
		        context.startActivity(myIntent);
                /* Wait activity created*/
                startSignal.wait(5000);
                activityCreationAllowed = false;
                if( ! TuiTlcWrapper.isActityAlive.get()) {
                    tLog.d(TAG, "ERROR ACTIVITY timout");
                    finishTuiSession();
                    return false;
                }
	        }
	    } catch (InterruptedException e) {
	        // TODO Auto-generated catch block
	        e.printStackTrace();
	    }

        /* Enable cancel events catching */
        synchronized (sessionSignal) {
            TuiTlcWrapper.sessionOpened = true;
        }

        return true;
    }

    public static void acquireWakeLock() {
	    /* Ensure that CPU is still running */
	    try {
	        wl.acquire();
	    } catch (Exception e1) {
	        // TODO Auto-generated catch block
	        e1.printStackTrace();
	    }
    }

    public static void finishTuiSession(){
        tLog.d(TAG, "finishTuiSession!");
        if(TuiTlcWrapper.isActivityCreated.get()) {
            /* Disable cancel events catching */
            synchronized (sessionSignal) {
                TuiTlcWrapper.sessionOpened = false;
            }

            try{
                synchronized (finishSignal) {
                    /* Send a message to the activity UI thread */
                    handler.sendMessage(handler.obtainMessage(CLOSE_SESSION));
                    /* Wait activity closed */
                    long tBefore=System.currentTimeMillis();
                    finishSignal.wait(5000);
                    if ((System.currentTimeMillis() - tBefore) < 5000){
                        TuiTlcWrapper.setIsActivityCreated(false);
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
    }


    /* Initialize static members */
    public static void init(Context ctxt) {
        /* Save context */
        context = ctxt;

        pm = (PowerManager) ctxt.getSystemService(Context.POWER_SERVICE);
        wl  = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, "TuiService");

        /* Start the TlcTui native thread */
        startTlcTui();
    }

    public static int[] getCurrentResolution() {
        int ret[]={0,0};

        Point displayMetrics = new Point();
        WindowManager wm = (WindowManager) context.getApplicationContext().getSystemService(Context.WINDOW_SERVICE);
        wm.getDefaultDisplay().getRealSize(displayMetrics);
        int screenWidth = displayMetrics.x;
        int screenHeight = displayMetrics.y;

        ret[0] = screenWidth;
        ret[1] = screenHeight;

        return ret;
    }

    /* Native functions */
    public static native int startTlcTui();
    public static native boolean notifyEvent(int eventType);

    /**
     * this is used to load the library on application startup. The
     * library has already been unpacked to the app specific folder
     * at installation time by the package manager.
     */
    static {
        System.loadLibrary("Tui");
    }
}
