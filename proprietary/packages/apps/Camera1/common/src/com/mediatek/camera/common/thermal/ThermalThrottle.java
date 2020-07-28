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
 * MediaTek Inc. (C) 2014. All rights reserved.
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
package com.mediatek.camera.common.thermal;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.res.Resources;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;

import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;

/**
 * Thermal throttle for Camera.
 */
public class ThermalThrottle {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(ThermalThrottle.class.getSimpleName());

    private static final String THERMAL_THROTTLE_PATH = "/proc/driver/cl_cam_status";
    // buffer point
    private static final int THERMAL_BUFFER_VALUE = 1;
    // urgent point
    private static final int THERMAL_URGENT_VALUE = 2;

    private WarningDialog mAlertDialog;
    private Activity mActivity;
    private Resources mRes;

    protected final Handler mHandler;
    private WorkerHandler mWorkerHandler;
    private HandlerThread mHandlerThread;

    private static final int MSG_READ_THERMAL = 0;
    private static final int MSG_UPDATE_TIME = 1;
    private static final int DELAY_5SECONDS = 5000;
    private static final int DELAY_1SECONDS = 1000;
    private static final int UPDATE_TIME_DELAY = 1000;
    private static final int WAITING_TIME = 30;

    private int mWaitingTime;
    private int mThermalStatus = -1;
    private boolean mIsResumed = false;
    private boolean mIsShowing = false;

    /**
     * Constructor of Thermal throttle for Camera.
     * @param  app the app instance.
     */
    public ThermalThrottle(IApp app) {
        mActivity = app.getActivity();
        mAlertDialog = new WarningDialog(app);
        mRes = mActivity.getResources();
        int title = mRes.getIdentifier("pref_thermal_dialog_title", "string",
                mActivity.getPackageName());
        int contentLaunch = mRes.getIdentifier("pref_thermal_dialog_content_launch", "string",
                mActivity.getPackageName());
        if (getTemperStatus() == THERMAL_URGENT_VALUE
                || mThermalStatus == THERMAL_BUFFER_VALUE) {
            mIsShowing = true;
            showThermalDlg(mActivity, title, contentLaunch);
        }
        mHandler = new MainHandler(app.getActivity().getMainLooper());
        mHandlerThread = new HandlerThread("ThermalThrottle-thread");
        mHandlerThread.start();
        mWorkerHandler = new WorkerHandler(mHandlerThread.getLooper());
        mWorkerHandler.sendEmptyMessageDelayed(MSG_READ_THERMAL, DELAY_5SECONDS);
        mWaitingTime = WAITING_TIME;
    }

    /**
     * activity resume event.
     */
    public void resume() {
        LogHelper.d(TAG, "[resume]...");
        mIsResumed = true;
        mWaitingTime = WAITING_TIME;
        if (mWorkerHandler != null) {
            mWorkerHandler.removeMessages(MSG_READ_THERMAL);
            mWorkerHandler.sendEmptyMessageDelayed(MSG_READ_THERMAL, DELAY_5SECONDS);
        }
    }

    /**
     * activity pause event.
     */
    public void pause() {
        LogHelper.d(TAG, "[pause]...");
        mIsResumed = false;
        if (mWorkerHandler != null) {
            mWorkerHandler.removeCallbacksAndMessages(null);
        }
        if (mHandler != null) {
            mHandler.removeCallbacksAndMessages(null);
        }
        if (mAlertDialog.isShowing()) {
            mAlertDialog.hide();
        }
        mAlertDialog.setCountDownTime(String.valueOf(WAITING_TIME));
        mWaitingTime = WAITING_TIME;
    }

    /**
     * activity destroy event.
     */
    public void destroy() {
        LogHelper.d(TAG, "[destroy]...");
        mAlertDialog.uninitView();
        if (mWorkerHandler != null) {
            mWorkerHandler.getLooper().quit();
        }
        if (mHandlerThread != null) {
            mHandlerThread.quit();
        }
    }

    private void updateCountDownTime(final Activity activity) {
        LogHelper.d(TAG, "[updateCountDownTime]mCountDown = " + mWaitingTime + ",mIsResumed = "
                + mIsResumed);
        if (mThermalStatus == THERMAL_BUFFER_VALUE) {
            if (mWaitingTime > 0) {
                mWaitingTime--;
                mAlertDialog.setCountDownTime(String.valueOf(mWaitingTime));
                if (mIsResumed) {
                    mHandler.sendEmptyMessageDelayed(MSG_UPDATE_TIME, UPDATE_TIME_DELAY);
                }
            } else if (mWaitingTime == 0) {
                activity.runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        if (activity.isFinishing() || activity.isDestroyed()) {
                            LogHelper.d(TAG, "[updateCountDownTime] don't need finish activity");
                        } else {
                            activity.finish();
                        }
                    }
                });
            }
        } else {
            if (mAlertDialog.isShowing()) {
                mAlertDialog.hide();
            }
            mAlertDialog.setCountDownTime(String.valueOf(WAITING_TIME));
            mWaitingTime = WAITING_TIME;
        }
    }

    /**
     * The main thread handler.
     */
    class MainHandler extends Handler {

        public MainHandler(Looper looper) {
            super(looper);
        }
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_UPDATE_TIME:
                    updateCountDownTime(mActivity);
                    break;

                default:
                    break;
            }
        }
    }

    /**
     * The work handler thread for count down.
     */
    private class WorkerHandler extends Handler {
        public WorkerHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_READ_THERMAL:
                    if (getTemperStatus() == THERMAL_URGENT_VALUE) {
                        if (!mActivity.isFinishing()) {
                            mActivity.finish();
                        }
                        return;
                    }
                    if (mThermalStatus == THERMAL_BUFFER_VALUE
                            && !mAlertDialog.isShowing()
                            && !mIsShowing) {
                        LogHelper.d(TAG, "[handleMessage]WorkerHandler, mCountDown = "
                                + mWaitingTime);
                        if (mWaitingTime == WAITING_TIME) {
                            mAlertDialog.show();
                            mHandler.removeMessages(MSG_UPDATE_TIME);
                            mHandler.sendEmptyMessageDelayed(MSG_UPDATE_TIME, UPDATE_TIME_DELAY);
                        }
                    }
                    if (mThermalStatus == THERMAL_BUFFER_VALUE) {
                        mWorkerHandler.sendEmptyMessageDelayed(MSG_READ_THERMAL, DELAY_1SECONDS);
                    } else {
                        mWorkerHandler.sendEmptyMessageDelayed(MSG_READ_THERMAL, DELAY_5SECONDS);
                    }
                    break;

                default:
                    break;
            }
        }
    }

    private void showThermalDlg(final Activity activity, int titleId, int msgId) {
        DialogInterface.OnClickListener buttonListener = new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                activity.finish();
            }
        };

        new AlertDialog.Builder(activity).setCancelable(false)
                .setIconAttribute(android.R.attr.alertDialogIcon).setTitle(titleId)
                .setMessage(msgId).setNeutralButton(android.R.string.ok, buttonListener).show();
    }

    private int getTemperStatus() {
        String temper = null;
        int temperInt = 0;
        try {
            FileReader fls = new FileReader(THERMAL_THROTTLE_PATH);
            BufferedReader br = new BufferedReader(fls);
            temper = br.readLine();
            temperInt = Integer.valueOf(temper);
            br.close();
            fls.close();
        } catch (IOException err) {
            System.out.println(err.toString());
        } finally {
            if (mThermalStatus != temperInt) {
                LogHelper.i(TAG, "Camera Thermal status :" + temperInt);
            }
        }
        mThermalStatus = temperInt;
        return mThermalStatus;
    }
}
