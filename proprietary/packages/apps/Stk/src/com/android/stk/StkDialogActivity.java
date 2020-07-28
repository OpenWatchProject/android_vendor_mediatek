/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.stk;

import com.android.internal.telephony.cat.CatLog;
import com.android.internal.telephony.cat.TextMessage;
import com.android.internal.telephony.PhoneConstants;

import android.app.Activity;
import android.app.ActivityManager;
import android.app.AlarmManager;
import android.app.AlertDialog;
import android.app.PendingIntent;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.graphics.BitmapFactory;

import android.os.Build;
import android.os.Bundle;
import android.provider.Settings;
import android.os.SystemClock;
import android.support.v4.content.LocalBroadcastManager;
import android.telephony.SubscriptionManager;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.Gravity;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;

import com.mediatek.internal.telephony.cat.MtkCatLog;

/**
 * AlertDialog used for DISPLAY TEXT commands.
 *
 */
public class StkDialogActivity extends Activity {
    // members
    private static final String className =
            new Object(){}.getClass().getEnclosingClass().getName();
    private static final String LOG_TAG =
            className.substring(className.lastIndexOf('.') + 1);
    TextMessage mTextMsg = null;
    private int mSlotId = -1;
    private StkAppService appService = StkAppService.getInstance();
    // Determines whether Terminal Response (TR) has been sent
    private boolean mIsResponseSent = false;
    private Context mContext;
    // Determines whether this is in the pending state.
    private boolean mIsPending = false;

    // Utilize AlarmManager for real-time countdown
    private static final String DIALOG_ALARM_TAG = LOG_TAG;
    private static final long NO_DIALOG_ALARM = -1;
    private long mAlarmTime = NO_DIALOG_ALARM;

    // Keys for saving the state of the dialog in the bundle
    private static final String TEXT_KEY = "text";
    private static final String ALARM_TIME_KEY = "alarm_time";
    private static final String RESPONSE_SENT_KEY = "response_sent";
    private static final String SLOT_ID_KEY = "slotid";
    private static final String PENDING = "pending";

    private AlertDialog mAlertDialog;

    /// M:  @{
    private boolean mIsRegisterReceiverDone = false;
    /// @}

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        MtkCatLog.d(LOG_TAG, "onCreate, sim id: " + mSlotId);

        // appService can be null if this activity is automatically recreated by the system
        // with the saved instance state right after the phone process is killed.
        if (appService == null) {
            MtkCatLog.d(LOG_TAG, "onCreate - appService is null");
            finish();
            return;
        }

        // New Dialog is created - set to no response sent
        mIsResponseSent = false;

        AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(this);

        alertDialogBuilder.setPositiveButton(R.string.button_ok, new
                DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int id) {
                        MtkCatLog.d(LOG_TAG, "OK Clicked!, mSlotId: " + mSlotId);
                        if (mAlertDialog != null) {
                            MtkCatLog.d(LOG_TAG, "dismiss mAlertDialog");
                            mAlertDialog.dismiss();
                        }
                        sendResponse(StkAppService.RES_ID_CONFIRM, true);
                    }
                });

        alertDialogBuilder.setNegativeButton(R.string.button_cancel, new
                DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog,int id) {
                        MtkCatLog.d(LOG_TAG, "Cancel Clicked!, mSlotId: " + mSlotId);
                        if (mAlertDialog != null) {
                            MtkCatLog.d(LOG_TAG, "dismiss mAlertDialog");
                            mAlertDialog.dismiss();
                        }
                        sendResponse(StkAppService.RES_ID_CONFIRM, false);
                    }
                });

        alertDialogBuilder.setOnCancelListener(new DialogInterface.OnCancelListener() {
                    @Override
                    public void onCancel(DialogInterface dialog) {
                        MtkCatLog.d(LOG_TAG, "Moving backward!, mSlotId: " + mSlotId);
                        if (mAlertDialog != null) {
                            MtkCatLog.d(LOG_TAG, "dismiss mAlertDialog");
                            mAlertDialog.dismiss();
                        }
                        sendResponse(StkAppService.RES_ID_BACKWARD);
                    }
                });

        alertDialogBuilder.create();

        initFromIntent(getIntent());
        if (mTextMsg == null) {
            mIsRegisterReceiverDone = false;
            finish();
            return;
        }

        if (!mTextMsg.responseNeeded) {
            alertDialogBuilder.setNegativeButton(null, null);
            // Register the instance of this activity because the dialog displayed for DISPLAY TEXT
            // command with an immediate response object should disappear when the terminal receives
            // a subsequent proactive command containing display data.
            appService.getStkContext(mSlotId).setImmediateDialogInstance(this);
        }

        alertDialogBuilder.setTitle(mTextMsg.title);

        LayoutInflater inflater = this.getLayoutInflater();
        View dialogView = inflater.inflate(R.layout.stk_msg_dialog, null);
        alertDialogBuilder.setView(dialogView);
        TextView tv = (TextView) dialogView.findViewById(R.id.message);
        ImageView iv = (ImageView) dialogView.findViewById(R.id.icon);

        if (mTextMsg.icon != null) {
            iv.setImageBitmap(mTextMsg.icon);
        } else {
            iv.setVisibility(View.GONE);
        }

        // Per spec, only set text if the icon is not provided or not self-explanatory
        if ((mTextMsg.icon == null || !mTextMsg.iconSelfExplanatory)
                && !TextUtils.isEmpty(mTextMsg.text)) {
            tv.setText(mTextMsg.text);
        } else {
            tv.setVisibility(View.GONE);
        }

        mAlertDialog = alertDialogBuilder.create();
        mAlertDialog.setCanceledOnTouchOutside(false);
        mAlertDialog.show();

        mContext = getBaseContext();
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        mContext.registerReceiver(mBroadcastReceiver, intentFilter);

        //MTK add begin
        IntentFilter LocalIntentFilter = new IntentFilter(StkAppService.ACTION_SIM_ABSENT);
        LocalIntentFilter.addAction(StkAppService.ACTION_ALL_SIM_ABSENT);
        LocalBroadcastManager.getInstance(this).registerReceiver(mLocalBroadcastReceiver,
                LocalIntentFilter);

        mIsRegisterReceiverDone = true;
        setFinishOnTouchOutside(false);
        // Set a new task description to change icon
        if (StkAppService.isOP02 && PhoneConstants.SIM_ID_1 < mSlotId) {
            setTaskDescription(new ActivityManager.TaskDescription(null,
            BitmapFactory.decodeResource(getResources(),
            R.drawable.ic_launcher_sim2_toolkit)));
        }
    }

    @Override
    public void onResume() {
        super.onResume();
        MtkCatLog.d(LOG_TAG, "onResume - mIsResponseSent[" + mIsResponseSent +
                "], sim id: " + mSlotId + " mTextMsg.responseNeeded:" +
                mTextMsg.responseNeeded);
        // The pending dialog is unregistered if this instance was registered as it before.
        setPendingState(false);

        /*
         * If the userClear flag is set and dialogduration is set to 0, the display Text
         * should be displayed to user forever until some high priority event occurs
         * (incoming call, MMI code execution etc as mentioned under section
         * ETSI 102.223, 6.4.1)
         */
        if (StkApp.calculateDurationInMilis(mTextMsg.duration) == 0 &&
                !mTextMsg.responseNeeded && mTextMsg.userClear) {
            MtkCatLog.d(LOG_TAG, "User should clear text..showing message forever");
            return;
        }

        //MTK add  begin
        if (appService != null) {
            appService.setDisplayTextDlgVisibility(true, mSlotId);
        } else {
            MtkCatLog.v(LOG_TAG, "onPause, appService is null.");
            mIsResponseSent = true;//Skip TR since this is not a real activity triggered from sim.
            Toast toast = Toast.makeText(mContext,
                    getString(R.string.lable_not_available), Toast.LENGTH_LONG);
            toast.setGravity(Gravity.BOTTOM, 0, 0);
            toast.show();
            finish();
           return;
        }
        //MTK add  end

        /*
         * When another activity takes the foreground, we do not want the Terminal
         * Response timer to be restarted when our activity resumes. Hence we will
         * check if there is an existing timer, and resume it. In this way we will
         * inform the SIM in correct time when there is no response from the User
         * to a dialog.
         */
        if (mAlarmTime == NO_DIALOG_ALARM) {
            startTimeOut();
        }
    }

    @Override
    public void onPause() {
        super.onPause();
        MtkCatLog.d(LOG_TAG, "onPause, sim id: " + mSlotId);
        appService.setDisplayTextDlgVisibility(false, mSlotId);

        /*
         * do not cancel the timer here cancelTimeOut(). If any higher/lower
         * priority events such as incoming call, new sms, screen off intent,
         * notification alerts, user actions such as 'User moving to another activtiy'
         * etc.. occur during Display Text ongoing session,
         * this activity would receive 'onPause()' event resulting in
         * cancellation of the timer. As a result no terminal response is
         * sent to the card.
         */
    }

    @Override
    protected void onStart() {
        MtkCatLog.d(LOG_TAG, "onStart, sim id: " + mSlotId);
        super.onStart();
    }

    @Override
    public void onStop() {
        super.onStop();
        MtkCatLog.d(LOG_TAG, "onStop - before Send CONFIRM false mIsResponseSent[" +
                mIsResponseSent + "], sim id: " + mSlotId);

        // Nothing should be done here if this activity is being finished or restarted now.
        if (isFinishing() || isChangingConfigurations()) {
            return;
        }

        // This is registered as the pending dialog as this was sent to the background.
        setPendingState(true);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        MtkCatLog.d(LOG_TAG, "onDestroy - mIsResponseSent[" + mIsResponseSent +
                "], sim id: " + mSlotId);

        if (mAlertDialog != null && mAlertDialog.isShowing()) {
            mAlertDialog.dismiss();
            mAlertDialog = null;
        }

        if (appService == null) {
            return;
        }
        // if dialog activity is finished by stkappservice
        // when receiving OP_LAUNCH_APP from the other SIM, we can not send TR here
        // , since the dialog cmd is waiting user to process.
        if (!isChangingConfigurations()) {
            if (!mIsResponseSent && appService != null &&
                    !appService.isDialogPending(mSlotId)) {
                sendResponse(StkAppService.RES_ID_CONFIRM, false);
            }
        }
        cancelTimeOut();
        // Cleanup broadcast receivers to avoid leaks
        if (mBroadcastReceiver != null) {
            unregisterReceiver(mBroadcastReceiver);
        }
        //MTK add begin
        if (appService != null) {
            appService.setDisplayTextDlgVisibility(false, mSlotId);
        }
        if (mIsRegisterReceiverDone) {
            LocalBroadcastManager.getInstance(this)
                    .unregisterReceiver(mLocalBroadcastReceiver);
        }
        //MTK add end
    }

    @Override
    public void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);

        MtkCatLog.d(LOG_TAG, "onSaveInstanceState");

        outState.putParcelable(TEXT_KEY, mTextMsg);
        outState.putBoolean(RESPONSE_SENT_KEY, mIsResponseSent);
        outState.putLong(ALARM_TIME_KEY, mAlarmTime);
        outState.putInt(SLOT_ID_KEY, mSlotId);
        outState.putBoolean(PENDING, mIsPending);
    }

    @Override
    public void onRestoreInstanceState(Bundle savedInstanceState) {
        super.onRestoreInstanceState(savedInstanceState);

        MtkCatLog.d(LOG_TAG, "onRestoreInstanceState");

        mTextMsg = savedInstanceState.getParcelable(TEXT_KEY);
        mIsResponseSent = savedInstanceState.getBoolean(RESPONSE_SENT_KEY);
        mAlarmTime = savedInstanceState.getLong(ALARM_TIME_KEY, NO_DIALOG_ALARM);
        mSlotId = savedInstanceState.getInt(SLOT_ID_KEY);

        // The pending dialog must be replaced if the previous instance was in the pending state.
        if (savedInstanceState.getBoolean(PENDING)) {
            setPendingState(true);
        }

        if (mAlarmTime != NO_DIALOG_ALARM) {
            startTimeOut();
        }
    }

    @Override
    protected void onNewIntent(Intent intent) {
        super.onNewIntent(intent);
        MtkCatLog.v(LOG_TAG, "onNewIntent - mIsResponseSent[" + mIsResponseSent + "]"
                + ", mSlotId: " + mSlotId);
        initFromIntent(intent);
        if (mTextMsg == null) {
            finish();
            return;
        }
    }

    @Override
    public void finish() {
        super.finish();
        // Unregister the instance for DISPLAY TEXT command with an immediate response object
        // as it is unnecessary to ask the service to finish this anymore.
        if ((appService != null) && (mTextMsg != null) && !mTextMsg.responseNeeded) {
            if (SubscriptionManager.isValidSlotIndex(mSlotId)) {
                appService.getStkContext(mSlotId).setImmediateDialogInstance(null);
            }
        }
    }

    private void setPendingState(boolean on) {
        if (mTextMsg.responseNeeded) {
            if (mIsPending != on) {
                appService.getStkContext(mSlotId).setPendingDialogInstance(on ? this : null);
                mIsPending = on;
            }
        }
    }

    private void sendResponse(int resId, boolean confirmed) {
        cancelTimeOut();

        if (mSlotId == -1) {
            MtkCatLog.d(LOG_TAG, "sim id is invalid");
            return;
        }

        if (StkAppService.getInstance() == null) {
            MtkCatLog.d(LOG_TAG, "Ignore response: id is " + resId);
            return;
        }

        MtkCatLog.d(LOG_TAG, "sendResponse resID[" + resId +
                "] confirmed[" + confirmed + "]");

        if (mTextMsg.responseNeeded) {
            Bundle args = new Bundle();
            args.putInt(StkAppService.OPCODE, StkAppService.OP_RESPONSE);
            args.putInt(StkAppService.SLOT_ID, mSlotId);
            args.putInt(StkAppService.RES_ID, resId);
            args.putBoolean(StkAppService.CONFIRMATION, confirmed);
            startService(new Intent(this, StkAppService.class).putExtras(args));
            mIsResponseSent = true;
        }
        if (!isFinishing()) {
            finish();
        }
    }

    private void sendResponse(int resId) {
        sendResponse(resId, true);
    }

    private void initFromIntent(Intent intent) {

        if (intent != null) {
            mTextMsg = intent.getParcelableExtra("TEXT");
            mSlotId = intent.getIntExtra(StkAppService.SLOT_ID, -1);
            //MTK add begin
            if (appService == null) {
                MtkCatLog.v(LOG_TAG, "appService is null!");
                mIsResponseSent = true;
                finish();
                return;
            }
            //MTK add end
        } else {
            finish();
        }

        MtkCatLog.d(LOG_TAG, "initFromIntent - [" +
                (Build.IS_DEBUGGABLE ? mTextMsg : "********")
                + "], slot id: " + mSlotId);
    }

    private void cancelTimeOut() {
        if (mAlarmTime != NO_DIALOG_ALARM) {
            MtkCatLog.d(LOG_TAG, "cancelTimeOut - slot id: " + mSlotId);
            AlarmManager am = (AlarmManager) getSystemService(Context.ALARM_SERVICE);
            am.cancel(mAlarmListener);
            mAlarmTime = NO_DIALOG_ALARM;
        }
    }

    private void startTimeOut() {
        // No need to set alarm if device sent TERMINAL RESPONSE already
        // and it is required to wait for user to clear the message.
        if (mIsResponseSent || (mTextMsg.userClear && !mTextMsg.responseNeeded)) {
            return;
        }

        if (mAlarmTime == NO_DIALOG_ALARM) {
            int duration = StkApp.calculateDurationInMilis(mTextMsg.duration);
            // If no duration is specified, the timeout set by the terminal manufacturer is applied.
            if (duration == 0) {
                if (mTextMsg.userClear) {
                    duration = StkApp.DISP_TEXT_WAIT_FOR_USER_TIMEOUT;
                } else {
                    duration = StkApp.DISP_TEXT_CLEAR_AFTER_DELAY_TIMEOUT;
                }
            }
            mAlarmTime = SystemClock.elapsedRealtime() + duration;
        }

        MtkCatLog.d(LOG_TAG, "startTimeOut: " + mAlarmTime + "ms, slot id: " + mSlotId);
        AlarmManager am = (AlarmManager) getSystemService(Context.ALARM_SERVICE);
        am.setExact(AlarmManager.ELAPSED_REALTIME_WAKEUP, mAlarmTime, DIALOG_ALARM_TAG,
                mAlarmListener, null);
    }

    private final BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        @Override public void onReceive(Context context, Intent intent) {
            boolean airplaneModeEnabled = isAirplaneModeOn(mContext);
            StkAppInstaller appInstaller = StkAppInstaller.getInstance();
            MtkCatLog.v(LOG_TAG, "mAirplaneModeReceiver AIRPLANE_MODE_CHANGED: " +
                    airplaneModeEnabled);
            if (airplaneModeEnabled) {
                mIsResponseSent = true;
                cancelTimeOut();
                finish();
            }
        }
    };

    private final AlarmManager.OnAlarmListener mAlarmListener =
            new AlarmManager.OnAlarmListener() {
                @Override
                public void onAlarm() {
                    CatLog.d(LOG_TAG, "The alarm time is reached");
                    mAlarmTime = NO_DIALOG_ALARM;
                    sendResponse(StkAppService.RES_ID_TIMEOUT);
                }
            };

    /// M:  @{
    private BroadcastReceiver mLocalBroadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (StkAppService.ACTION_SIM_ABSENT.equals(intent.getAction())) {
                int slotId = intent.getIntExtra(StkAppService.SLOT_ID,0);
                MtkCatLog.d(this, "ACTION_SIM_ABSENT, slotId: " + slotId);
                if (slotId == mSlotId) {
                    cancelTimeOut();
                    mIsResponseSent = true;
                    finish();
                }
            } else if (StkAppService.ACTION_ALL_SIM_ABSENT.equals(intent.getAction())) {
                MtkCatLog.d(this, "ACTION_ALL_SIM_ABSENT");
                cancelTimeOut();
                mIsResponseSent = true;
                finish();
            }
        }
    };
    private boolean isAirplaneModeOn(Context context) {
        return Settings.Global.getInt(context.getContentResolver(),
                Settings.Global.AIRPLANE_MODE_ON, 0) != 0;
    }
    /// @}
}
