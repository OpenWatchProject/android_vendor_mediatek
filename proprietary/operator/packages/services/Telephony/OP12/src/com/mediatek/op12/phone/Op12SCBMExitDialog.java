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

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnCancelListener;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.CountDownTimer;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.util.Log;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.TelephonyIntents;
import com.mediatek.internal.telephony.MtkGsmCdmaPhone;
import com.mediatek.internal.telephony.scbm.ISCBMManager;
import com.mediatek.op12.phone.Op12SCBMService;
import com.mediatek.op12.phone.R;
import com.mediatek.phone.ext.ExtensionManager;

/**
 * Displays dialog that enables users to exit Emergency Callback Mode
 *
 * @see Op12SCBMService
 */
public class Op12SCBMExitDialog extends Activity implements OnCancelListener {

    private static final String TAG = "SCBMDialog";

    /** Intent to trigger the Emergency Callback Mode exit dialog */
    static final String ACTION_SHOW_SCM_EXIT_DIALOG =
            "com.mediatek.op12.phone.action.ACTION_SHOW_SCM_EXIT_DIALOG";
    /** Used to get the users choice from the return Intent's extra */
    public static final String EXTRA_EXIT_SCM_RESULT = "exit_scm_result";

    public static final int EXIT_SCM_DIALOG = 1;
    public static final int EXIT_SCM_PROGRESS_DIALOG = 2;
    public static final int EXIT_SCM_IN_EMERGENCY_CALL_DIALOG = 3;

    AlertDialog mAlertDialog = null;
    ProgressDialog mProgressDialog = null;
    CountDownTimer mTimer = null;
    Op12SCBMService mService = null;
    Handler mHandler = null;
    int mDialogType = 0;
    long mScmTimeout = 0;
    private boolean mInEmergencySms = false;
    private MtkGsmCdmaPhone mPhone;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mHandler = new Handler();

        // Start thread that will wait for the connection completion so that it can get
        // timeout value from the service
        Thread waitForConnectionCompleteThread = new Thread(null, mTask,
                "ScmExitDialogWaitThread");
        waitForConnectionCompleteThread.start();

        // Register receiver for intent closing the dialog
        IntentFilter filter = new IntentFilter();
        filter.addAction(TelephonyIntents.ACTION_SCBM_CHANGED);
        registerReceiver(mScmExitReceiver, filter);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        try {
            unregisterReceiver(mScmExitReceiver);
        } catch (IllegalArgumentException e) {
            // Receiver was never registered - silently ignore.
        }
    }

    @Override
    protected void onRestoreInstanceState(Bundle savedInstanceState) {
        super.onRestoreInstanceState(savedInstanceState);
        mDialogType = savedInstanceState.getInt("DIALOG_TYPE");
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        outState.putInt("DIALOG_TYPE", mDialogType);
    }

    /**
     * Waits until bind to the service completes
     */
    private Runnable mTask = new Runnable() {
        public void run() {
            Looper.prepare();

            // Bind to the remote service
            bindService(new Intent(Op12SCBMExitDialog.this,
                    Op12SCBMService.class), mConnection, Context.BIND_AUTO_CREATE);

            // Wait for bind to finish
            synchronized (Op12SCBMExitDialog.this) {
                try {
                    if (mService == null) {
                        Op12SCBMExitDialog.this.wait();
                    }
                } catch (InterruptedException e) {
                    Log.d("SCM", "Op12SCBMExitDialog InterruptedException: "
                            + e.getMessage());
                    e.printStackTrace();
                }
            }

            // Get timeout value and call state from the service
            if (mService != null) {
                mScmTimeout = mService.getSCBMTimeout();
                mInEmergencySms = mService.getSCBMState();
                try {
                    // Unbind from remote service
                    unbindService(mConnection);
                } catch (IllegalArgumentException e) {
                    // Failed to unbind from service. Don't crash as this brings down the entire
                    // radio.
                    Log.w(TAG, "Failed to unbind from Op12SCBMService");
                }
            }

            // Show dialog
            mHandler.post(new Runnable() {
                public void run() {
                    showSCBMExitDialog();
                }
            });
        }
    };

    /**
     * Shows Emergency Callback Mode dialog and starts countdown timer
     */
    private void showSCBMExitDialog() {
        if (!this.isResumed()) {
            Log.w(TAG, "Tried to show dialog, but activity was already finished");
            setResult(RESULT_OK, (new Intent()).putExtra(EXTRA_EXIT_SCM_RESULT, false));
            finish();
            return;
        }
        if(mInEmergencySms) {
            mDialogType = EXIT_SCM_IN_EMERGENCY_CALL_DIALOG;
            showDialog(EXIT_SCM_IN_EMERGENCY_CALL_DIALOG);
        } else {
            if (getIntent().getAction().equals(ACTION_SHOW_SCM_EXIT_DIALOG)) {
                mDialogType = EXIT_SCM_DIALOG;
                showDialog(EXIT_SCM_DIALOG);
            }

            mTimer = new CountDownTimer(mScmTimeout, 1000) {
                @Override
                public void onTick(long millisUntilFinished) {
                    CharSequence text = getDialogText(millisUntilFinished);
                    mAlertDialog.setMessage(text);
                }

                @Override
                public void onFinish() {
                    //Do nothing
                }
            }.start();
        }
    }

    /**
     * Creates dialog that enables users to exit Emergency Callback Mode
     */
    @Override
    protected Dialog onCreateDialog(int id) {
        switch (id) {
        case EXIT_SCM_DIALOG:
            CharSequence text = getDialogText(mScmTimeout);
            mAlertDialog = new AlertDialog.Builder(Op12SCBMExitDialog.this,
                    android.R.style.Theme_DeviceDefault_Dialog_Alert)
                    .setTitle(R.string.phone_in_scm_notification_title)
                    .setMessage(text)
                    .setPositiveButton(R.string.alert_dialog_yes,
                            new DialogInterface.OnClickListener() {
                                public void onClick(DialogInterface dialog, int whichButton) {
                                    // User clicked Yes. Exit SCBM
                                    ISCBMManager scbmManager = mPhone.getScbmManagerInstance();
                                    scbmManager.exitSCBM();

                                    // Show Process dialog
                                    showDialog(EXIT_SCM_PROGRESS_DIALOG);
                                    mTimer.cancel();
                                }
                            })
                    .setNegativeButton(R.string.alert_dialog_no,
                            new DialogInterface.OnClickListener() {
                                public void onClick(DialogInterface dialog, int whichButton) {
                                    // User clicked No
                                    setResult(RESULT_OK, (new Intent()).putExtra(
                                            EXTRA_EXIT_SCM_RESULT, false));
                                    finish();
                                }
                            }).create();
            mAlertDialog.setOnCancelListener(this);
            return mAlertDialog;

        case EXIT_SCM_IN_EMERGENCY_CALL_DIALOG:
            mAlertDialog = new AlertDialog.Builder(Op12SCBMExitDialog.this,
                    android.R.style.Theme_DeviceDefault_Dialog_Alert)
                    .setTitle(R.string.phone_in_scm_notification_title)
                    .setMessage(R.string.alert_dialog_in_scm_call)
                    .setNeutralButton(R.string.alert_dialog_dismiss,
                            new DialogInterface.OnClickListener() {
                                public void onClick(DialogInterface dialog, int whichButton) {
                                    // User clicked Dismiss
                                    setResult(RESULT_OK, (new Intent()).putExtra(
                                            EXTRA_EXIT_SCM_RESULT, false));
                                    finish();
                                }
                            }).create();
            mAlertDialog.setOnCancelListener(this);
            return mAlertDialog;

        case EXIT_SCM_PROGRESS_DIALOG:
            mProgressDialog = new ProgressDialog(Op12SCBMExitDialog.this);
            mProgressDialog.setMessage(getText(R.string.progress_dialog_exiting_scm));
            mProgressDialog.setIndeterminate(true);
            mProgressDialog.setCancelable(false);
            return mProgressDialog;

        default:
            return null;
        }
    }

    /**
     * Returns dialog box text with updated timeout value
     */
    private CharSequence getDialogText(long millisUntilFinished) {
        // Format time
        int minutes = (int)(millisUntilFinished / 60000);
        String time = String.format("%d:%02d", minutes,
                (millisUntilFinished % 60000) / 1000);

        switch (mDialogType) {
        case EXIT_SCM_DIALOG:
            return String.format(getResources().getQuantityText(R.plurals.alert_dialog_exit_scm,
                    minutes).toString(), time);
        }
        return null;
    }

    /**
     * Closes activity when dialog is canceled
     */
    @Override
    public void onCancel(DialogInterface dialog) {
        Op12SCBMExitDialog.this.setResult(RESULT_OK, (new Intent())
                .putExtra(EXTRA_EXIT_SCM_RESULT, false));
        finish();
    }

    /**
     * Listens for Emergency SMS Callback Mode state change intents
     */
    private BroadcastReceiver mScmExitReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            // Received exit Emergency Callback Mode notification close all dialogs
            if (intent.getAction().equals(
                    TelephonyIntents.ACTION_SCBM_CHANGED)) {
                if (intent.getBooleanExtra("phoneinSCMState", false) == false) {
                    if (mAlertDialog != null)
                        mAlertDialog.dismiss();
                    if (mProgressDialog != null)
                        mProgressDialog.dismiss();
                    Op12SCBMExitDialog.this.setResult(RESULT_OK, (new Intent())
                            .putExtra(EXTRA_EXIT_SCM_RESULT, true));
                    finish();
                } else {
                    int phoneId = intent.getIntExtra(PhoneConstants.PHONE_KEY, 0);
                    Log.d(TAG, "ACTION_SCBM_CHANGED true. phoneId =" + phoneId);
                    mPhone = (MtkGsmCdmaPhone) PhoneFactory.getPhone(phoneId);
                }
            }
        }
    };

    /**
     * Class for interacting with the interface of the service
     */
    private ServiceConnection mConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName className, IBinder service) {
            mService = ((Op12SCBMService.LocalBinder)service).getService();
            // Notify thread that connection is ready
            synchronized (Op12SCBMExitDialog.this) {
                Op12SCBMExitDialog.this.notify();
            }
        }

        public void onServiceDisconnected(ComponentName className) {
            mService = null;
        }
    };
}
