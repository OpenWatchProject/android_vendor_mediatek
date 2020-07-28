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
package com.mediatek.server;

import android.content.Context;
import android.content.Intent;

import android.os.Handler;
import android.os.PowerManager;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemClock;
import android.os.SystemProperties;

import android.system.ErrnoException;
import android.system.Os;

import android.util.Log;

import com.android.server.power.ShutdownThread;
import com.android.server.power.PowerManagerService;

// for Polling Backlight state
import android.hardware.display.DisplayManager;
import android.view.Display;
import android.view.DisplayInfo;

// for shutdown animation
import android.app.Dialog;
import android.view.Surface;
import android.view.WindowManager;
import android.view.IWindowManager;
import android.provider.Settings;
import com.mediatek.provider.MtkSettingsExt;



public class MtkShutdownThread extends ShutdownThread{
    private static String TAG = "MtkShutdownThread";
    private static boolean DEBUG = true;
    private static boolean mSpew = false;

    // Show SysUI or Shutdown Dialog
    private static final int DEFAULT_MODE = 0;
    //Show shutdown animation
    private static final int ANIMATION_MODE = 1;

    private static final int MAX_BLIGHT_OFF_DELAY_TIME = 5 * 1000;
    // for Polling Backlight state
    private static final int MAX_BLIGHT_OFF_POLL_TIME = 1 * 1000;
    private static final int BACKLIGHT_STATE_POLL_SLEEP_MSEC = 50;
    private static boolean mBlightOff = false;

    ///added for Shutdown animation @{
    private static final String OPERATOR_SYSPROP = "persist.vendor.operator.optr";
    private static final int MIN_SHUTDOWN_ANIMATION_PLAY_TIME = 5 * 1000;

    // CU/CMCC operator require 3-5s
    private static long beginAnimationTime = 0;
    private static long endAnimationTime = 0;
    private static boolean bConfirmForAnimation = true;
    private static boolean bPlayaudio = true;
    private static int mShutOffAnimation = -1;
    //@}

    public MtkShutdownThread(){
    }

    @Override
    protected boolean mIsShowShutdownSysui() {
        // for Shutdown animation
        if(isCustBootAnim()== ANIMATION_MODE) {
            //Show Animation ,No Sysui.
            return false;
        }
        // Show Shutdown SysUI
        return true;
    }

    @Override
    protected boolean mIsShowShutdownDialog(Context context) {
        // for Shutdown animation
        if(showShutdownAnimation(context)) {
            return false;
        }
        //Show Shutdown Dialog
        return true;
    }

    @Override
    protected boolean mStartShutdownSeq(Context context) {
        if (getState() != Thread.State.NEW || isAlive()) {
            Log.i(TAG, "Thread state is not normal! froce to shutdown!");
            // for Shutdown animation
            if (isCustBootAnim()== ANIMATION_MODE)
                delayForPlayAnimation();

            // Turn off backlight
            setBacklightOff();
            //unmout data/cache partitions while performing shutdown
            PowerManagerService.lowLevelShutdown(mReason);
            // abnormal case,shutdown directly
            return false;
        } else {
            int screenTurnOffTime = MAX_BLIGHT_OFF_DELAY_TIME;

            // for Shutdown animation
            if (isCustBootAnim()== ANIMATION_MODE) {
                screenTurnOffTime = getScreenTurnOffTime(context);
            }
            mHandler.postDelayed(mDelayDim, screenTurnOffTime);
            return true;
        }
    }

    @Override
    protected void mShutdownSeqFinish(Context context) {
        // for Shutdown animation @{
        shutdownAnimationService();

        //Turn backlight off
        setBacklightOff();
    }

    @Override
    protected void mLowLevelShutdownSeq(Context context) {
        pollBacklightOff(context);
        if (mSpew) {
            // Debug only,for capture shutdown log.
            if (SystemProperties.getInt("vendor.shutdown_delay", 0) == 1) {
                Log.i(TAG, "Delay Shutdown 5s");
                SystemClock.sleep(5000);
            }
        }
    }

    private static void setBacklightOff() {
        if(mBlightOff)
            return;

        if (((MtkShutdownThread)sInstance).mPowerManager == null) {
            Log.e(TAG, "check PowerManager: PowerManager service is null");
            return;
        }
        mBlightOff  = true;
        Log.i(TAG, "setBacklightBrightness: Off");
        ((MtkShutdownThread)sInstance).mPowerManager.goToSleep(SystemClock.uptimeMillis(),
                PowerManager.GO_TO_SLEEP_REASON_FORCE_SUSPEND, 0);
    }

    /**
     * Polling backlight off and time out in MAX_BLIGHT_OFF_POLL_TIME.
     *
     * @param context Context used to get dispaly info.
     */
    private void pollBacklightOff(Context context) {
        try {
            DisplayManager displayManager =
                (DisplayManager)context.getSystemService(Context.DISPLAY_SERVICE);
            DisplayInfo di = new DisplayInfo();

            final long endTime = SystemClock.elapsedRealtime() + MAX_BLIGHT_OFF_POLL_TIME;
            long TimeOut = endTime - SystemClock.elapsedRealtime();
            while (TimeOut > 0) {
                displayManager.getDisplay(Display.DEFAULT_DISPLAY).getDisplayInfo(di);
                if(di.state == Display.STATE_OFF) {
                    break;
                }
                SystemClock.sleep(BACKLIGHT_STATE_POLL_SLEEP_MSEC);
                TimeOut = endTime - SystemClock.elapsedRealtime();
            }
            Log.i(TAG, "Backlight polling take:" + (MAX_BLIGHT_OFF_POLL_TIME - TimeOut) + " ms");
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private static Runnable mDelayDim = new Runnable() {
        public void run() {
            if (((MtkShutdownThread)sInstance).mScreenWakeLock != null &&
                ((MtkShutdownThread)sInstance).mScreenWakeLock.isHeld()) {
                ((MtkShutdownThread)sInstance).mScreenWakeLock.release();
            }
            if (((MtkShutdownThread)sInstance).mPowerManager == null) {
                ((MtkShutdownThread)sInstance).mPowerManager =
                    (PowerManager)((MtkShutdownThread)sInstance)
                    .mContext.getSystemService(Context.POWER_SERVICE);
            }
            // Turn off backlight
            setBacklightOff();
        }
    };

    // added for Shutdown animation
    private void shutdownAnimationService() {
        if(isCustBootAnim()!= ANIMATION_MODE) {
                return;
        }
        /* M: fix shutdown animation timing issue */
        Log.i(TAG, "set service.shutanim.running to 1");
        SystemProperties.set("service.shutanim.running", "1");
        /*M: play animation*/
        if ((mReboot == true && mReason != null && mReason.equals("recovery")) ||
                (mReboot == false)) {
            delayForPlayAnimation();
        }
    }

    private boolean showShutdownAnimation(Context context) {
        beginAnimationTime = 0;
        if (isCustBootAnim()== ANIMATION_MODE) {
            configShutdownAnimation(context);
            // Show Shutdown Animation
            bootanimCust(context);
            return true;
        }
        return false;
    }

    private static void bootanimCust(Context context) {
        boolean isRotaionEnabled = false;
        // [MTK] fix shutdown animation timing issue
        SystemProperties.set("service.shutanim.running", "0");
        Log.i(TAG, "set service.shutanim.running to 0");
        try {
            isRotaionEnabled = Settings.System.getInt(context.getContentResolver(),
                    Settings.System.ACCELEROMETER_ROTATION, 1) != 0;
            if (isRotaionEnabled) {
                final IWindowManager wm = IWindowManager.Stub.asInterface(
                        ServiceManager.getService(Context.WINDOW_SERVICE));
                if (wm != null) {
                    wm.freezeRotation(Surface.ROTATION_0);
                }
                Settings.System.putInt(context.getContentResolver(),
                        Settings.System.ACCELEROMETER_ROTATION, 0);
                Settings.System.putInt(context.getContentResolver(),
                        MtkSettingsExt.System.ACCELEROMETER_ROTATION_RESTORE, 1);
            }
        } catch (NullPointerException ex) {
            Log.e(TAG, "check Rotation: context object is null when get Rotation");
        } catch (RemoteException e) {
            e.printStackTrace();
        }
        beginAnimationTime = SystemClock.elapsedRealtime() + MIN_SHUTDOWN_ANIMATION_PLAY_TIME;
        //Disable key dispatch
        try {
            final IWindowManager wm = IWindowManager.Stub.asInterface(
                    ServiceManager.getService(Context.WINDOW_SERVICE));
            if (wm != null) {
                wm.setEventDispatching(false);
            }
        } catch (RemoteException e) {
            e.printStackTrace();
        }
        //Disable key dispatch
        startBootAnimation();
    }

    private static void configShutdownAnimation(Context context) {
        PowerManager pm = (PowerManager)
            context.getSystemService(Context.POWER_SERVICE);

        if (!bConfirmForAnimation && !pm.isScreenOn()) {
            bPlayaudio = false;
        } else {
            bPlayaudio = true;
        }
    }

    private static void startBootAnimation() {
        Log.i(TAG, "Set 'service.bootanim.exit' = 0).");
        SystemProperties.set("service.bootanim.exit", "0");

        if (bPlayaudio) {
            SystemProperties.set("ctl.start", "banim_shutmp3");
            Log.i(TAG, "bootanim:shut mp3");
        } else {
            SystemProperties.set("ctl.start", "banim_shutnomp3");
            Log.i(TAG, "bootanim:shut nomp3");
        }
    }

    private static void delayForPlayAnimation() {
        if (beginAnimationTime <= 0) {
            return;
        }
        endAnimationTime = beginAnimationTime - SystemClock.elapsedRealtime();
        if (endAnimationTime > 0) {
            try {
                Thread.currentThread().sleep(endAnimationTime);
            } catch (InterruptedException e) {
                Log.e(TAG, "Shutdown stop bootanimation Thread.currentThread().sleep exception!");
            }
        }
    }

    public static int getScreenTurnOffTime() {
        if(SystemProperties.get(OPERATOR_SYSPROP, "0").equals("OP01"))
        {
            Log.i(TAG, "Inside MtkShutdownThread OP01");
            return 4 * 1000;
        }
        else if(SystemProperties.get(OPERATOR_SYSPROP, "0").equals("OP02"))
        {
            Log.i(TAG, "Inside MtkShutdownThread OP02");
            return 4 * 1000;
        }
        else if(SystemProperties.get(OPERATOR_SYSPROP, "0").equals("OP09"))
        {
            Log.i(TAG, "Inside MtkShutdownThread OP09");
            return 3 * 1000;
        }
        else
            return 4 * 1000;
    }

    private static int getScreenTurnOffTime(Context context) {
        int screenTurnOffTime = 0;
        try {
            screenTurnOffTime = getScreenTurnOffTime();
            Log.i(TAG, "screen turn off time screenTurnOffTime =" + screenTurnOffTime);
        } catch (Exception e) {
            e.printStackTrace();
        }
        return screenTurnOffTime;
    }

    public static int isCustBootAnim() {
            return mShutOffAnimation;
    }
}

