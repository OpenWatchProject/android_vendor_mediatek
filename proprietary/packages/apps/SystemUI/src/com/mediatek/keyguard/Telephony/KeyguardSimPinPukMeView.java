/*
 * Copyright (C) 2012 The Android Open Source Project
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

package com.mediatek.keyguard.Telephony;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.app.Dialog;
import android.app.INotificationManager;
import android.app.ITransientNotification;
import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.res.ColorStateList;
import android.content.res.Resources;
import android.graphics.Color;
import android.graphics.PixelFormat;
import android.graphics.drawable.Drawable;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.PowerManager;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.provider.Settings;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.util.AttributeSet;
import android.util.Log;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.WindowManager;
import android.view.WindowManagerImpl;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;

import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.PhoneConstants;

import com.android.keyguard.EmergencyCarrierArea ;
import com.android.keyguard.KeyguardPinBasedInputView ;
import com.android.keyguard.KeyguardSecurityModel;
import com.android.keyguard.KeyguardUpdateMonitor ;
import com.android.keyguard.KeyguardUpdateMonitorCallback ;
import com.android.keyguard.KeyguardUtils ;
import com.android.keyguard.R ;
import com.android.keyguard.SecurityMessageDisplay;

import com.mediatek.internal.telephony.IMtkTelephonyEx;
import com.mediatek.internal.telephony.MtkIccCardConstants;
import com.mediatek.keyguard.ext.IKeyguardUtilExt;
import com.mediatek.keyguard.ext.IOperatorSIMString;
import com.mediatek.keyguard.ext.IOperatorSIMString.SIMChangedTag;
import com.mediatek.keyguard.ext.OpKeyguardCustomizationFactoryBase;

/**
 * M: Displays a PIN/PUK pad for unlocking.
 */
public class KeyguardSimPinPukMeView extends KeyguardPinBasedInputView {
    private static final String TAG = "KeyguardSimPinPukMeView";
    private static final boolean DEBUG = true ;

    private ProgressDialog mSimUnlockProgressDialog = null;
    private volatile boolean mSimCheckInProgress;
    KeyguardUpdateMonitor mUpdateMonitor = null;

    private int mUnlockEnterState;

    private int mPinRetryCount;
    private int mPukRetryCount;
    private int mDeviceRetryCount;

    private String mPukText;
    private String mNewPinText;
    private StringBuffer mSb = null;

    /// M: Save Sim Card dialog, we will close this dialog
    // when phone state change to ringing or offhook
    private AlertDialog mSimCardDialog;

    /// M: wait next SIM ME state reflash flag
    private KeyguardSecurityModel mSecurityModel;
    private int mNextRepollStatePhoneId = KeyguardUtils.INVALID_PHONE_ID;
    private IccCardConstants.State mLastSimState = IccCardConstants.State.UNKNOWN;

    static final int VERIFY_TYPE_PIN = 501;
    static final int VERIFY_TYPE_PUK = 502;

    // size limits for the pin.
    private static final int MIN_PIN_LENGTH = 4;
    private static final int MAX_PIN_LENGTH = 8;

    private static final int GET_SIM_RETRY_EMPTY = -1;

    private static final int STATE_ENTER_PIN = 0;
    private static final int STATE_ENTER_PUK = 1;
    private static final int STATE_ENTER_NEW = 2;
    private static final int STATE_REENTER_NEW = 3;
    private static final int STATE_ENTER_FINISH = 4;
    private static final int STATE_ENTER_ME = 5;
    // Lock category name string Temp use for QA/RD
    private String[] strLockName = {" [NP]", " [NSP]", " [SP]", " [CP]", " [SIMP]", " [NSSP]", " [SIMCP]"};
    //Default device retry count.
    private static final int DEVICE_RETRY_COUNT = 5;

    /// M: for get the proper SIM UIM string according to operator.
    private IOperatorSIMString mIOperatorSIMString;

    private int mPhoneId;
    private int mSubId;
    private KeyguardUtils mKeyguardUtils ;

    private Handler mHandler = new Handler(Looper.myLooper(), null, true /*async*/);

    private ImageView mSimImageView;

    private Button mDismissButton;

    /**
     * Used to dismiss SimPinPuk view after a delay
     */
    private Runnable mDismissSimPinPukRunnable = new Runnable() {
        public void run() {
            mUpdateMonitor.reportSimUnlocked(mPhoneId);
        }
    };

    KeyguardUpdateMonitorCallback mUpdateMonitorCallback = new KeyguardUpdateMonitorCallback() {
        @Override
        public void onSimStateChangedUsingPhoneId(int phoneId, IccCardConstants.State simState) {
            Log.d(TAG, "onSimStateChangedUsingPhoneId: " + simState +
                       ", phoneId = " + phoneId + ", mPhoneId = " + mPhoneId);
            Log.d(TAG, "onSimStateChangedUsingPhoneId: mCallback = " + mCallback) ;

            if (phoneId == mPhoneId) {
                resetState(true);

                if (mSimUnlockProgressDialog != null) {
                    mSimUnlockProgressDialog.hide();
                }
                mHandler.removeCallbacks(mDismissSimPinPukRunnable);

                if (IccCardConstants.State.READY == simState) {
                    simStateReadyProcess();
                } else if (IccCardConstants.State.NOT_READY == simState ||
                        IccCardConstants.State.ABSENT == simState) {
                    // it will try next security screen or finish
                    Log.d(TAG, "onSimStateChangedUsingPhoneId: not ready, phoneId = " + phoneId) ;
                    mCallback.dismiss(true, KeyguardUpdateMonitor.getCurrentUser());
                    // M: add for ALPS02468469
                    mSimCheckInProgress = false;
                    Log.d(TAG, "set mSimCheckInProgress false");
                } else if (IccCardConstants.State.NETWORK_LOCKED == simState) {
                    if (!KeyguardUtils.isMediatekSimMeLockSupport()) {
                        // it will try next security screen or finish
                        mCallback.dismiss(true, KeyguardUpdateMonitor.getCurrentUser());
                    } else if (0 == getRetryMeCount(mPhoneId)) { //permanently locked, exit
                        // do not show permanently locked dialog here, it will show in ViewMediator
                        Log.d(TAG, "onSimStateChanged: ME retrycount is 0, dismiss it");
                        mCallback.dismiss(true, KeyguardUpdateMonitor.getCurrentUser());
                    }
                }
                mLastSimState = simState;
                Log.d(TAG, "assign mLastSimState=" + mLastSimState);
            } else if (phoneId == mNextRepollStatePhoneId) {
                Log.d(TAG, "onSimStateChanged: mNextRepollStatePhoneId = " +
                           mNextRepollStatePhoneId);
                if (mSimUnlockProgressDialog != null) {
                    mSimUnlockProgressDialog.hide();
                }

                if (IccCardConstants.State.READY == simState) {
                    // pretend current sim is still ME lock state
                    mLastSimState = IccCardConstants.State.NETWORK_LOCKED;
                    simStateReadyProcess();
                } else {
                    // exit current SIM unlock to show next SIM unlock
                    mCallback.dismiss(true, KeyguardUpdateMonitor.getCurrentUser());
                    mLastSimState = simState;
                }
            }
        }

        ///M: fix ALPS01794428. SimPinPukMeView should disappear when Flight Mode turns on.
        @Override
        public void onAirPlaneModeChanged(boolean airPlaneModeEnabled) {
            Log.d(TAG, "onAirPlaneModeChanged(airPlaneModeEnabled = " + airPlaneModeEnabled + ")") ;
            if (airPlaneModeEnabled == true) {
                Log.d(TAG, "Flight-Mode turns on & keyguard is showing, dismiss keyguard.") ;

                // 1. dismiss keyguard
                mPasswordEntry.reset(true, true) ;
                mCallback.userActivity();
                mCallback.dismiss(true, KeyguardUpdateMonitor.getCurrentUser());
            }
        }
    };

    public KeyguardSimPinPukMeView(Context context) {
        this(context, null);
    }

    public KeyguardSimPinPukMeView(Context context, AttributeSet attrs) {
        super(context, attrs);

        mKeyguardUtils = new KeyguardUtils(context);
        mSb = new StringBuffer();
        mUpdateMonitor = KeyguardUpdateMonitor.getInstance(getContext());
        mSecurityModel = new KeyguardSecurityModel(getContext());
        mDeviceRetryCount = SystemProperties.getInt("vendor.gsm.sim.slot.lock.device.lock.remain.count"
                , DEVICE_RETRY_COUNT);

        /// M: Init keyguard operator plugins @{
        try {
            mKeyguardUtilExt = OpKeyguardCustomizationFactoryBase.getOpFactory(context).
                    makeKeyguardUtil();
            mIOperatorSIMString = OpKeyguardCustomizationFactoryBase.getOpFactory(context).
                    makeOperatorSIMString();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    /**
     * set phone id.
     * @param phoneId The phone id
     */
    public void setPhoneId(int phoneId) {

        mPhoneId = phoneId;
        Log.i(TAG, "setPhoneId=" + phoneId);

        resetState();

        /// M: A dialog set view to another one, it did not refresh displaying along with it,
        /// so dismiss it and set it to null.
        if (mSimCardDialog != null) {
            if (mSimCardDialog.isShowing()) {
                mSimCardDialog.dismiss();
            }
            mSimCardDialog = null;
        }
    }

    public void resetState() {
        resetState(false) ;
    }

    /**
     * M: reset UI elements.
     * @param forceReload force to reload active sub list or not.
     */
    public void resetState(boolean forceReload) {
        Log.v(TAG, "Resetting state");
        super.resetState();

        TextView forText = (TextView) findViewById(R.id.slot_num_text);
        forText.setText(mContext.getString(R.string.kg_slot_id, mPhoneId + 1) + " ");

        //Update Sim Icon
        Resources rez = getResources();
        String msg = "";
        int count = KeyguardUtils.getNumOfPhone() ;
        int color = Color.WHITE;

        IccCardConstants.State simState = mUpdateMonitor.getSimStateOfPhoneId(mPhoneId) ;
        showDismissIfNeed(simState);
        if (count < 2) {
            if (simState == IccCardConstants.State.PIN_REQUIRED) {
                msg = rez.getString(R.string.kg_sim_pin_instructions);
                mUnlockEnterState = STATE_ENTER_PIN;
            } else if (simState == IccCardConstants.State.PUK_REQUIRED) {
                msg = rez.getString(R.string.kg_puk_enter_puk_hint);
                mUnlockEnterState = STATE_ENTER_PUK;
            } else if ((IccCardConstants.State.NETWORK_LOCKED == simState) &&
                KeyguardUtils.isMediatekSimMeLockSupport()) {
                int category = mUpdateMonitor.getSimMeCategoryOfPhoneId(mPhoneId);
                msg = rez.getString(R.string.simlock_entersimmelock)
                    + strLockName[category]
                    + getRetryMeString(mPhoneId);
                mUnlockEnterState = STATE_ENTER_ME;
            }
        } else {
            int subId = KeyguardUtils.getSubIdUsingPhoneId(mPhoneId) ;
            ///M: fix ALPS01963966, we should force reload sub list for hot-plug sim device.
            ///   since we may insert the sim card later and the sub list is not null and cannot
            ///   fetch the latest/updated active sub list.
            SubscriptionInfo info = mUpdateMonitor.getSubscriptionInfoForSubId(subId, forceReload);
            CharSequence displayName = info != null ? info.getDisplayName() : ""; // don't crash
            // M: add for ALPS02475558
            if (info == null) {
                displayName = "CARD " + Integer.toString(mPhoneId + 1);
                Log.d(TAG, "we set a displayname");
            }
            Log.d(TAG, "resetState() - subId = " + subId + ", displayName = " + displayName) ;

            if (simState == IccCardConstants.State.PIN_REQUIRED) {
                msg = rez.getString(R.string.kg_sim_pin_instructions_multi, displayName);
                mUnlockEnterState = STATE_ENTER_PIN;
            } else if (simState == IccCardConstants.State.PUK_REQUIRED) {
                msg = rez.getString(R.string.kg_puk_enter_puk_hint_multi, displayName);
                mUnlockEnterState = STATE_ENTER_PUK;
            } else if ((IccCardConstants.State.NETWORK_LOCKED == simState) &&
                KeyguardUtils.isMediatekSimMeLockSupport()) {
                int category = mUpdateMonitor.getSimMeCategoryOfPhoneId(mPhoneId);
                msg = rez.getString(R.string.simlock_entersimmelock)
                    + strLockName[category]
                    + getRetryMeString(mPhoneId);
                mUnlockEnterState = STATE_ENTER_ME;
            }
            if (info != null) {
                color = info.getIconTint();
            }
        }

        mKeyguardUtilExt.customizePinPukLockView(mPhoneId, mSimImageView, forText);

        mSimImageView.setImageTintList(ColorStateList.valueOf(color));

        msg = mIOperatorSIMString.getOperatorSIMString(msg, mPhoneId, SIMChangedTag.DELSIM,
                    mContext);
        Log.d(TAG, "resetState() - mSecurityMessageDisplay.setMessage = " + msg) ;
        if (mSecurityMessageDisplay != null) {
            mSecurityMessageDisplay.setMessage(msg);
        }
        //mPasswordEntry.reset(true);
    }

    private String getPinPasswordErrorMessage(int attemptsRemaining) {
        String displayMessage;

        if (attemptsRemaining == 0) {
            displayMessage = getContext().getString(R.string.kg_password_wrong_pin_code_pukked);
        } else if (attemptsRemaining > 0) {
            displayMessage = getContext().getResources()
                    .getQuantityString(R.plurals.kg_password_wrong_pin_code, attemptsRemaining,
                            attemptsRemaining);
        } else {
            displayMessage = getContext().getString(R.string.kg_password_pin_failed);
        }

        Log.d(TAG, "getPinPasswordErrorMessage:"
            + " attemptsRemaining=" + attemptsRemaining + " displayMessage=" + displayMessage);

        return displayMessage;
    }

    private String getPukPasswordErrorMessage(int attemptsRemaining) {
        String displayMessage;

        if (attemptsRemaining == 0) {
            displayMessage = getContext().getString(R.string.kg_password_wrong_puk_code_dead);
        } else if (attemptsRemaining > 0) {
            displayMessage = getContext().getResources()
                    .getQuantityString(R.plurals.kg_password_wrong_puk_code, attemptsRemaining,
                            attemptsRemaining);
        } else {
            displayMessage = getContext().getString(R.string.kg_password_puk_failed);
        }
        if (DEBUG) {
            Log.d(TAG, "getPukPasswordErrorMessage:"
                + " attemptsRemaining=" + attemptsRemaining + " displayMessage=" + displayMessage);
        }
        return displayMessage;
    }

    @Override
    protected boolean shouldLockout(long deadline) {
        // SIM PIN doesn't have a timed lockout
        return false;
    }

    @Override
    protected int getPasswordTextViewId() {
        return R.id.simPinPukMeEntry;
    }

    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();
        mPhoneId = KeyguardUtils.INVALID_PHONE_ID ;

        if (mEcaView instanceof EmergencyCarrierArea) {
            ((EmergencyCarrierArea) mEcaView).setCarrierTextVisible(true);
            /// M: Set the carrier text gravity as center for OP project.
            View ctView =
                    mEcaView.findViewById(R.id.carrier_text);
            mKeyguardUtilExt.customizeCarrierTextGravity((TextView) ctView);
        }

        mSimImageView = (ImageView) findViewById(R.id.keyguard_sim);
        /// M: Add dismiss button for OP request.
        mDismissButton = (Button) findViewById(R.id.key_dismiss);
    }

    @Override
    public void onAttachedToWindow() {
        super.onAttachedToWindow() ;
        Log.d(TAG, "onAttachedToWindow") ;
        mUpdateMonitor.registerCallback(mUpdateMonitorCallback);
    }

    @Override
    public void onDetachedFromWindow() {
        super.onDetachedFromWindow();
        Log.d(TAG, "onDetachedFromWindow") ;
        mHandler.removeCallbacks(mDismissSimPinPukRunnable);
        mUpdateMonitor.removeCallback(mUpdateMonitorCallback);
    }

    @Override
    public void showUsabilityHint() {
    }

    @Override
    public void onResume(int reason) {
        if (mSimUnlockProgressDialog != null) {
            mSimUnlockProgressDialog.dismiss();
            mSimUnlockProgressDialog = null;
        }

        /// M: if has IME, then hide it @{
        InputMethodManager imm = ((InputMethodManager) mContext.
                getSystemService(Context.INPUT_METHOD_SERVICE));
        if (imm.isActive()) {
            Log.i(TAG, "IME is showing, we should hide it");
            imm.hideSoftInputFromWindow(this.getWindowToken(), InputMethodManager.HIDE_NOT_ALWAYS);
        }
        /// @}
    }

    @Override
    public void onPause() {
        if (mSimUnlockProgressDialog != null) {
            mSimUnlockProgressDialog.dismiss();
            mSimUnlockProgressDialog = null;
        }
    }

    private void setInputInvalidAlertDialog(CharSequence message, boolean shouldDisplay) {
        StringBuilder sb = new StringBuilder(message);

        if (shouldDisplay) {
            AlertDialog newDialog = new AlertDialog.Builder(mContext)
            .setMessage(sb)
            .setPositiveButton(com.android.internal.R.string.ok, null)
            .setCancelable(true)
            .create();

            newDialog.getWindow().setType(
                    WindowManager.LayoutParams.TYPE_KEYGUARD_DIALOG);
            newDialog.getWindow().addFlags(
                    WindowManager.LayoutParams.FLAG_DIM_BEHIND);
            newDialog.show();
        } else {
             Toast.makeText(mContext, sb).show();
        }
    }

    private int getRetryDeviceCount() {
        return mDeviceRetryCount;
    }

    private String getRetryMeString(final int phoneId) {
        int meRetryCount = getRetryMeCount(phoneId);
        return "(" + mContext.getString(R.string.retries_left, meRetryCount) + ")";
    }

    private int getRetryMeCount(final int phoneId) {
        if(KeyguardUtils.isUnlockSimMeWithDeviceSupport()) {
            return getRetryDeviceCount();
        }
        return mUpdateMonitor.getSimMeLeftRetryCountOfPhoneId(phoneId);
    }

    private void minusRetryMeCount(final int phoneId) {
        if (!KeyguardUtils.isUnlockSimMeWithDeviceSupport()) {
            mUpdateMonitor.minusSimMeLeftRetryCountOfPhoneId(phoneId);
        }
    }
    private String getRetryPuk(final int phoneId) {
        mPukRetryCount = mUpdateMonitor.getRetryPukCountOfPhoneId(phoneId);
        switch (mPukRetryCount) {
        case GET_SIM_RETRY_EMPTY:
            return " ";
        default:
            return "(" + mContext.getString(R.string.retries_left, mPukRetryCount) + ")";
        }
    }
    private String getRetryPinString(final int phoneId) {
        mPinRetryCount = getRetryPinCount(phoneId);
        switch (mPinRetryCount) {
            case GET_SIM_RETRY_EMPTY:
                return " ";
            default:
                return "(" + mContext.getString(R.string.retries_left, mPinRetryCount) + ")";
        }
    }

    private int getRetryPinCount(final int phoneId) {
        if (phoneId == 3) {
            return SystemProperties.getInt("vendor.gsm.sim.retry.pin1.4", GET_SIM_RETRY_EMPTY);
        } else if (phoneId == 2) {
            return SystemProperties.getInt("vendor.gsm.sim.retry.pin1.3", GET_SIM_RETRY_EMPTY);
        } else if (phoneId == 1) {
            return SystemProperties.getInt("vendor.gsm.sim.retry.pin1.2", GET_SIM_RETRY_EMPTY);
        } else {
            return SystemProperties.getInt("vendor.gsm.sim.retry.pin1", GET_SIM_RETRY_EMPTY);
        }
    }

    private boolean validatePin(String pin, boolean isPUK) {
        // for pin, we have 4-8 numbers, or puk, we use only 8.
        int pinMinimum = isPUK ? MAX_PIN_LENGTH : MIN_PIN_LENGTH;
        // check validity
        if (pin == null || pin.length() < pinMinimum
                || pin.length() > MAX_PIN_LENGTH) {
            return false;
        } else {
            return true;
        }
    }

    /**
     * Enter puk to unlock, and let user to supply new SIM pin code.
     */
    private void updatePinEnterScreen() {

        switch (mUnlockEnterState) {
            case STATE_ENTER_PUK:
               mPukText = mPasswordEntry.getText().toString();
               if (validatePin(mPukText, true)) {
                  mUnlockEnterState = STATE_ENTER_NEW;
                  mSb.delete(0, mSb.length());
                  mSb.append(mContext.getText(R.string.keyguard_password_enter_new_pin_code));
                  Log.d(TAG, "updatePinEnterScreen() - STATE_ENTER_UK, validatePin = true ,"
                    + "mSecurityMessageDisplay.setMessage = " + mSb.toString()) ;
                  mSecurityMessageDisplay.setMessage(mSb.toString());
               } else {
                  Log.d(TAG, "updatePinEnterScreen() - STATE_ENTER_UK, validatePin = false ,"
                    + "mSecurityMessageDisplay.setMessage = R.string.invalidPuk") ;
                  mSecurityMessageDisplay.setMessage(R.string.invalidPuk);
               }
               break;

             case STATE_ENTER_NEW:
                 mNewPinText = mPasswordEntry.getText().toString();
                 if (validatePin(mNewPinText, false)) {
                    mUnlockEnterState = STATE_REENTER_NEW;
                    mSb.delete(0, mSb.length());
                    mSb.append(mContext.getText(R.string.keyguard_password_Confirm_pin_code));
                    Log.d(TAG, "updatePinEnterScreen() - STATE_ENTER_NEW, validatePin = true ,"
                        + "mSecurityMessageDisplay.setMessage = " + mSb.toString()) ;
                    mSecurityMessageDisplay.setMessage(mSb.toString());
                 } else {
                    Log.d(TAG, "updatePinEnterScreen() - STATE_ENTER_NEW, validatePin = false ,"
                    + "mSecurityMessageDisplay.setMessage = R.string.keyguard_code_length_prompt") ;
                    mSecurityMessageDisplay.setMessage(R.string.keyguard_code_length_prompt);
                 }
                 break;

             case STATE_REENTER_NEW:
                if (!mNewPinText.equals(mPasswordEntry.getText().toString())) {
                    mUnlockEnterState = STATE_ENTER_NEW;
                    mSb.delete(0, mSb.length());
                    mSb.append(mContext.getText(R.string.keyguard_code_donnot_mismatch));
                    mSb.append(mContext.getText(R.string.keyguard_password_enter_new_pin_code));
                    Log.d(TAG, "updatePinEnterScreen() - STATE_REENTER_NEW, true ,"
                        + "mSecurityMessageDisplay.setMessage = " + mSb.toString()) ;
                    mSecurityMessageDisplay.setMessage(mSb.toString());
                } else {
                   Log.d(TAG, "updatePinEnterScreen() - STATE_REENTER_NEW, false ,"
                       + "mSecurityMessageDisplay.setMessage = empty string.") ;
                   mUnlockEnterState = STATE_ENTER_FINISH;
                   mSecurityMessageDisplay.setMessage("");
                }
                break;

                default:
                    break;
        }
        mPasswordEntry.reset(true, true);
        mCallback.userActivity();
    }

    /**
     * Since the IPC can block, we want to run the request in a separate thread
     * with a callback.
     */
    private abstract class CheckSimPinPuk extends Thread {
        private final String mPin;
        private final String mPuk;
        private int[] mResult;

        protected CheckSimPinPuk(String pin) {
            mPin = pin;
            mPuk = null;
        }
        protected CheckSimPinPuk(String pin, int phoneId) {
            mPin = pin;
            mPuk = null;
        }

        protected CheckSimPinPuk(String puk, String pin, int phoneId) {
            mPin = pin;
            mPuk = puk;
        }

        abstract void onSimCheckResponse(final int result, final int attemptsRemaining);

        @Override
        public void run() {
            try {
                Log.d(TAG, "CheckSimPinPuk, " + "mPhoneId =" + mPhoneId);

                if (mUpdateMonitor.getSimStateOfPhoneId(mPhoneId) ==
                    IccCardConstants.State.PIN_REQUIRED) {

                    ///M: fix ALPS01806988, avoid call function when phone service died.
                    ITelephony phoneService =
                        ITelephony.Stub.asInterface(ServiceManager.checkService("phone")) ;
                    if (phoneService != null) {
                        int subId = KeyguardUtils.getSubIdUsingPhoneId(mPhoneId);
                        mResult = phoneService.supplyPinReportResultForSubscriber(subId, mPin);
                    } else {
                        Log.d(TAG, "phoneService is gone, skip supplyPinForSubscriber().") ;
                    }
                } else if (mUpdateMonitor.getSimStateOfPhoneId(mPhoneId) ==
                           IccCardConstants.State.PUK_REQUIRED) {

                    ///M: fix ALPS01806988, avoid call function when phone service died.
                    ITelephony phoneService =
                        ITelephony.Stub.asInterface(ServiceManager.checkService("phone")) ;
                    if (phoneService != null) {
                        int subId = KeyguardUtils.getSubIdUsingPhoneId(mPhoneId) ;
                        mResult = phoneService
                                .supplyPukReportResultForSubscriber(subId, mPuk, mPin);
                    } else {
                        Log.d(TAG, "phoneService is gone, skip supplyPukForSubscriber().") ;
                    }
                }
                if (mResult != null) {
                    Log.v(TAG, "supplyPinReportResultForSubscriber returned: "
                            + mResult[0] + " " + mResult[1]);

                    // Create timer then wait for SIM_STATE_CHANGE for ready or network_lock
                    Log.d(TAG, "CheckSimPinPuk.run()," +
                        "mResult is true(success), so we postDelayed a timeout runnable object");

                    mHandler.post(new Runnable() {
                        public void run() {
                            onSimCheckResponse(mResult[0], mResult[1]);
                        }
                    });
                } else {
                    mHandler.post(new Runnable() {
                        public void run() {
                            onSimCheckResponse(PhoneConstants.PIN_GENERAL_FAILURE, -1);
                        }
                    });
                    Log.d(TAG, "there is an error with sim fw");
                }
           } catch (RemoteException e) {
                mHandler.post(new Runnable() {
                    public void run() {
                        onSimCheckResponse(PhoneConstants.PIN_GENERAL_FAILURE, -1);
                    }
                });
            }
        }
    }

    /**
     * Since the IPC can block, we want to run the request in a separate thread
     * with a callback.
     */
    private static final int VERIFY_RESULT_PASS = 0;
    private static final int VERIFY_INCORRECT_PASSWORD = 1;
    private static final int VERIFY_RESULT_EXCEPTION = 2;
    private static final int SIMME_WITH_DEVICE_REQUEST = 8;
    private static final int VERIFY_RESULT_PASS_WITH_DEVICE = 3;

    private abstract class CheckSimMe extends Thread {
        private final String mPasswd;
        private int mResult;

        protected CheckSimMe(String passwd, int phoneId) {
            mPasswd = passwd;
        }

        /**
         * Different request, map different result.
         */
        private int mapSimStatus(int result, int requestNum) {
            if (requestNum == SIMME_WITH_DEVICE_REQUEST) {
                if (result == VERIFY_RESULT_PASS_WITH_DEVICE) {
                    return VERIFY_RESULT_PASS;
                }
            }
            return result;
        }

        abstract void onSimMeCheckResponse(final int ret);

        @Override
        public void run() {
            try {
                Log.d(TAG, "CheckMe, " + "mPhoneId =" + mPhoneId);
                int subId = KeyguardUtils.getSubIdUsingPhoneId(mPhoneId) ;
                if (KeyguardUtils.isUnlockSimMeWithDeviceSupport()) {
                    int[] unlockWithDeviceNetwork = IMtkTelephonyEx.Stub.asInterface
                    (ServiceManager.getService("phoneEx")).supplyDeviceNetworkDepersonalization(mPasswd);
                    if (unlockWithDeviceNetwork != null) {
                        mResult = unlockWithDeviceNetwork[0];
                        int retryCount = unlockWithDeviceNetwork[1];
                        mDeviceRetryCount = retryCount > -1 ? retryCount : mDeviceRetryCount;
                    } else {
                        Log.d(TAG, "CheckMe unlockWithDeviceNetwork is null");
                        mResult = VERIFY_RESULT_EXCEPTION;
                    }
                } else {
                    mResult = IMtkTelephonyEx.Stub.asInterface(ServiceManager.getService("phoneEx"))
                    .supplyNetworkDepersonalization(subId, mPasswd);
                }
                Log.d(TAG, "CheckMe, " + "mPhoneId =" + mPhoneId + " mResult=" + mResult);
                if (KeyguardUtils.isUnlockSimMeWithDeviceSupport()) {
                    mResult = mapSimStatus(mResult, SIMME_WITH_DEVICE_REQUEST);
                }
                if (VERIFY_RESULT_PASS == mResult) {
                    // Create timer then wait for SIM_STATE_CHANGE for ready or network_lock
                    Log.d(TAG, "CheckSimMe.run(), VERIFY_RESULT_PASS == ret,"
                            + " so we postDelayed a timeout runnable object");
                }

                mHandler.post(new Runnable() {
                    public void run() {
                        onSimMeCheckResponse(mResult);
                    }
                });
            } catch (RemoteException e) {
                mHandler.post(new Runnable() {
                    public void run() {
                        onSimMeCheckResponse(VERIFY_RESULT_EXCEPTION);
                    }
                });
            }
        }
    }

    private Dialog getSimUnlockProgressDialog() {
        if (mSimUnlockProgressDialog == null) {
            mSimUnlockProgressDialog = new ProgressDialog(mContext);
            /// M: Change the String with SIM according to Operator. @{
            String msg = mContext.getString(R.string.kg_sim_unlock_progress_dialog_message);
            msg = mIOperatorSIMString.getOperatorSIMString(msg, mPhoneId, SIMChangedTag.DELSIM,
                    mContext);
            mSimUnlockProgressDialog.setMessage(msg);
            /// @}
            mSimUnlockProgressDialog.setIndeterminate(true);
            mSimUnlockProgressDialog.setCancelable(false);
            if (!(mContext instanceof Activity)) {
                mSimUnlockProgressDialog.getWindow().setType(
                        WindowManager.LayoutParams.TYPE_KEYGUARD_DIALOG);
            }
        }
        return mSimUnlockProgressDialog;
    }

    private AlertDialog mRemainingAttemptsDialog;
    private AlertDialog mReminderRebootDialog;

    @Override
    protected void verifyPasswordAndUnlock() {
        String entry = mPasswordEntry.getText().toString();

        ///M: here only for PIN code
        if ((false == validatePin(entry, false)) &&
            (mUpdateMonitor.getSimStateOfPhoneId(mPhoneId) == IccCardConstants.State.PIN_REQUIRED ||
            (mUpdateMonitor.getSimStateOfPhoneId(mPhoneId) == IccCardConstants.State.NETWORK_LOCKED)
             && KeyguardUtils.isMediatekSimMeLockSupport())) {

            // otherwise, display a message to the user, and don't submit.
            if (mUpdateMonitor.getSimStateOfPhoneId(mPhoneId) ==
                IccCardConstants.State.PIN_REQUIRED) {
                mSecurityMessageDisplay.setMessage(R.string.kg_invalid_sim_pin_hint);
            } else {
                // hint to enter 4-8 digits for network_lock mode
                mSecurityMessageDisplay.setMessage(R.string.keyguard_code_length_prompt);
            }

            mPasswordEntry.reset(true, true);
            mPasswordEntry.setEnabled(true);
            mCallback.userActivity();

            return;
        }
        mPasswordEntry.setEnabled(true);
        dealWithPinOrPukUnlock();
    }

    private void dealWithPinOrPukUnlock() {
        if (mUpdateMonitor.getSimStateOfPhoneId(mPhoneId) == IccCardConstants.State.PIN_REQUIRED) {
            Log.d(TAG, "onClick, checkPin, mPhoneId=" + mPhoneId);
            checkPin(mPhoneId);
        } else if (mUpdateMonitor.getSimStateOfPhoneId(mPhoneId) ==
                 IccCardConstants.State.PUK_REQUIRED) {
            Log.d(TAG, "onClick, checkPuk, mPhoneId=" + mPhoneId);
            checkPuk(mPhoneId);
        } else if ((mUpdateMonitor.getSimStateOfPhoneId(mPhoneId) ==
                  IccCardConstants.State.NETWORK_LOCKED)
            && KeyguardUtils.isMediatekSimMeLockSupport()) {
            Log.d(TAG, "onClick, checkMe, mPhoneId=" + mPhoneId);
            checkMe(mPhoneId);
        } else {
            Log.d(TAG, "wrong status, mPhoneId=" + mPhoneId);
        }
    }

    private void checkPin() {
        checkPin(mPhoneId);
    }

    private void checkPin(int phoneId) {
        getSimUnlockProgressDialog().show();
        Log.d(TAG, "mSimCheckInProgress: " + mSimCheckInProgress);
        if (!mSimCheckInProgress) {
            mSimCheckInProgress = true; // there should be only one
            new CheckSimPinPuk(mPasswordEntry.getText().toString(), phoneId) {
                void onSimCheckResponse(final int result, final int attemptsRemaining) {
                    resetPasswordText(true /* animate */,
                            result != PhoneConstants.PIN_RESULT_SUCCESS /* announce */);
                    if (result == PhoneConstants.PIN_RESULT_SUCCESS) {
                        int verify_type = VERIFY_TYPE_PIN ;
                        mKeyguardUtilExt.showToastWhenUnlockPinPuk(mContext, VERIFY_TYPE_PIN);
                        Log.d(TAG, "checkPin() success") ;
                        mUpdateMonitor.reportSimUnlocked(mPhoneId);
                        mCallback.dismiss(true, KeyguardUpdateMonitor.getCurrentUser());
                    } else {
                        if (mSimUnlockProgressDialog != null) {
                            mSimUnlockProgressDialog.hide();
                        }
                        if (result == PhoneConstants.PIN_PASSWORD_INCORRECT) {
                            if (attemptsRemaining <= 2) {
                                // this is getting critical - show dialog
                                getSimRemainingAttemptsDialog(attemptsRemaining).show();
                                // show message
                                mSecurityMessageDisplay.setMessage(
                                        getPinPasswordErrorMessage(attemptsRemaining));
                            } else {
                                // show message
                                mSecurityMessageDisplay.setMessage(
                                        getPinPasswordErrorMessage(attemptsRemaining));
                            }
                        } else {
                            // "PIN operation failed!" - no idea what this was and no way to
                            // find out. :/
                            mSecurityMessageDisplay.setMessage(getContext().getString(
                                    R.string.kg_password_pin_failed));
                        }
                        if (DEBUG) Log.d(TAG, "verifyPasswordAndUnlock "
                                + " CheckSimPin.onSimCheckResponse: " + result
                                + " attemptsRemaining=" + attemptsRemaining);
                            resetPasswordText(true /* animate */,
                                    result != PhoneConstants.PIN_RESULT_SUCCESS /* announce */);
                    }

                    mCallback.userActivity();
                    mSimCheckInProgress = false;
                }
            } .start();
        }
    }

    private void checkPuk() {
        checkPuk(mPhoneId);
    }

    private void checkPuk(int phoneId) {
        updatePinEnterScreen();
        if (mUnlockEnterState != STATE_ENTER_FINISH) {
            return;
        }
        getSimUnlockProgressDialog().show();
        Log.d(TAG, "mSimCheckInProgress: " + mSimCheckInProgress);
        if (!mSimCheckInProgress) {
            mSimCheckInProgress = true; // there should be only one
            new CheckSimPinPuk(mPukText, mNewPinText, phoneId) {
                void onSimCheckResponse(final int result, final int attemptsRemaining) {
                    resetPasswordText(true /* animate */,
                            result != PhoneConstants.PIN_RESULT_SUCCESS /* announce */);
                    if (result == PhoneConstants.PIN_RESULT_SUCCESS) {
                        Log.d(TAG, "checkPuk onSimCheckResponse, success!");
                        int verify_type = VERIFY_TYPE_PUK ;
                        mKeyguardUtilExt.showToastWhenUnlockPinPuk(mContext, VERIFY_TYPE_PUK);
                        mUpdateMonitor.reportSimUnlocked(mPhoneId);
                        mCallback.dismiss(true, KeyguardUpdateMonitor.getCurrentUser());
                    } else {
                        if (mSimUnlockProgressDialog != null) {
                            mSimUnlockProgressDialog.hide();
                        }
                        mUnlockEnterState = STATE_ENTER_PUK;

                        if (result == PhoneConstants.PIN_PASSWORD_INCORRECT) {
                            if (attemptsRemaining <= 2) {
                                // this is getting critical - show dialog
                                getPukRemainingAttemptsDialog(attemptsRemaining).show();
                                // show message
                                mSecurityMessageDisplay.setMessage(
                                    getPukPasswordErrorMessage(attemptsRemaining));
                            } else {
                                // show message
                                mSecurityMessageDisplay.setMessage(
                                    getPukPasswordErrorMessage(attemptsRemaining));
                            }
                        } else {
                            mSecurityMessageDisplay.setMessage(getContext().getString(
                                    R.string.kg_password_puk_failed));
                        }
                        if (DEBUG) Log.d(TAG, "verifyPasswordAndUnlock "
                                    + " UpdateSim.onSimCheckResponse: "
                                    + " attemptsRemaining=" + attemptsRemaining);
                            //mStateMachine.reset();
                    }
                    mCallback.userActivity();
                    mSimCheckInProgress = false;
                }
            } .start();
        }
    }

    private void checkMe() {
        checkMe(mPhoneId);
    }

    private void checkMe(int phoneId) {
        getSimUnlockProgressDialog().show();
        if (!mSimCheckInProgress) {
            mSimCheckInProgress = true; // there should be only one
            new CheckSimMe(mPasswordEntry.getText().toString(), phoneId) {
                void onSimMeCheckResponse(final int ret) {
                    Log.d(TAG, "checkMe onSimChangedResponse, ret = " + ret);
                    if (VERIFY_RESULT_PASS == ret) {
                        Log.d(TAG, "checkMe VERIFY_RESULT_PASS == ret(we had sent runnable before");
                        mUpdateMonitor.reportSimUnlocked(mPhoneId);
                        if (KeyguardUtils.isUnlockSimMeWithDeviceSupport()) {
                            getReminderRebootDialog().show();
                        } else {
                            mCallback.dismiss(true, KeyguardUpdateMonitor.getCurrentUser());
                        }
                    } else if (VERIFY_INCORRECT_PASSWORD == ret) {
                        mSb.delete(0, mSb.length());
                        minusRetryMeCount(mPhoneId);

                        if (mSimUnlockProgressDialog != null) {
                            mSimUnlockProgressDialog.hide();
                        }
                        if (mUnlockEnterState == STATE_ENTER_ME) {
                            if (0 == getRetryMeCount(mPhoneId)) { //permanently locked
                                setInputInvalidAlertDialog(mContext.getText(
                                        R.string.simlock_slot_locked_message), true);
                                mCallback.dismiss(true, KeyguardUpdateMonitor.getCurrentUser());
                            } else {
                                int category = mUpdateMonitor.getSimMeCategoryOfPhoneId(mPhoneId);
                                mSb.append(mContext.getText(R.string.keyguard_wrong_code_input));
                                mSb.append(mContext.getText(R.string.simlock_entersimmelock));
                                mSb.append(strLockName[category] + getRetryMeString(mPhoneId));
                            }
                            Log.d(TAG, "checkMe() - VERIFY_INCORRECT_PASSWORD == ret, "
                                + "mSecurityMessageDisplay.setMessage = " + mSb.toString()) ;
                            mSecurityMessageDisplay.setMessage(mSb.toString());
                            mPasswordEntry.reset(true, true);
                        }
                    } else if (VERIFY_RESULT_EXCEPTION == ret) {
                        if (mSimUnlockProgressDialog != null) {
                            mSimUnlockProgressDialog.hide();
                        }
                        setInputInvalidAlertDialog("Exception happen, fail to unlock", true);
                        mCallback.dismiss(true, KeyguardUpdateMonitor.getCurrentUser());
                    }
                    mCallback.userActivity();
                    mSimCheckInProgress = false;
                }
            } .start();
        }
    }

    private Dialog getSimRemainingAttemptsDialog(int remaining) {
        String msg = getPinPasswordErrorMessage(remaining);
        if (mRemainingAttemptsDialog == null) {
            Builder builder = new AlertDialog.Builder(mContext);
            builder.setMessage(msg);
            builder.setCancelable(false);
            builder.setNeutralButton(R.string.ok, null);
            mRemainingAttemptsDialog = builder.create();
            mRemainingAttemptsDialog.getWindow().setType(
                    WindowManager.LayoutParams.TYPE_KEYGUARD_DIALOG);
        } else {
            mRemainingAttemptsDialog.setMessage(msg);
        }
        return mRemainingAttemptsDialog;
    }

    /**
     * M: SIMME Lock For Require 8, add reboot dialog when unlock me.
     */
    private Dialog getReminderRebootDialog() {
        if (mReminderRebootDialog == null) {
            Builder builder = new AlertDialog.Builder(mContext);
            builder.setMessage(mContext.getString(R.string.remind_restart_phone));
            builder.setNeutralButton(R.string.restart, new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int which) {
                    PowerManager pm = (PowerManager) mContext
                            .getSystemService(Context.POWER_SERVICE);
                    pm.reboot("getReminderRebootDialog, restart");
                }
            });
            mReminderRebootDialog = builder.create();
            mReminderRebootDialog.getWindow().setType(WindowManager.LayoutParams.TYPE_KEYGUARD_DIALOG);
            mReminderRebootDialog.setCanceledOnTouchOutside(false);
            mReminderRebootDialog.setCancelable(false);
        } else {
            mReminderRebootDialog.setMessage(mContext.getString(R.string.remind_restart_phone));
        }
        return mReminderRebootDialog;
    }

    private Dialog getPukRemainingAttemptsDialog(int remaining) {
        String msg = getPukPasswordErrorMessage(remaining);
        if (mRemainingAttemptsDialog == null) {
            AlertDialog.Builder builder = new AlertDialog.Builder(mContext);
            builder.setMessage(msg);
            builder.setCancelable(false);
            builder.setNeutralButton(R.string.ok, null);
            mRemainingAttemptsDialog = builder.create();
            mRemainingAttemptsDialog.getWindow().setType(
                    WindowManager.LayoutParams.TYPE_KEYGUARD_DIALOG);
        } else {
            mRemainingAttemptsDialog.setMessage(msg);
        }
        return mRemainingAttemptsDialog;
    }



    /**
     * M: set the content of ForText of SIMInfoView.
     */
    private void setForTextDetectingCard(int phoneId, TextView forText) {
        StringBuffer forSb = new StringBuffer();

        forSb.append(mContext.getString(R.string.kg_slot_id, phoneId + 1));
        forSb.append(" ");
        forSb.append(mContext.getString(R.string.kg_detecting_simcard));
        forText.setText(forSb.toString());
    }

    private void setSIMCardName() {
        String operName = null;

        try {
            operName = mKeyguardUtils.getOptrNameUsingPhoneId(mPhoneId, mContext);
        } catch (IndexOutOfBoundsException e) {
            Log.w(TAG, "getOptrNameBySlot exception, mPhoneId=" + mPhoneId);
        }
        if (DEBUG) {
            Log.i(TAG, "setSIMCardName, mPhoneId=" + mPhoneId + ", operName=" + operName);
        }
    }

    ///M: the process after receive SIM_STATE READY event
    /// call repollIccStateForNetworkLock if next locked SIM card is ME lock
    private void simStateReadyProcess() {
        mNextRepollStatePhoneId = getNextRepollStatePhoneId();
        Log.d(TAG, "simStateReadyProcess mNextRepollStatePhoneId =" + mNextRepollStatePhoneId);
        if (mNextRepollStatePhoneId != KeyguardUtils.INVALID_PHONE_ID) {
            try {
                getSimUnlockProgressDialog().show();
                Log.d(TAG, "repollIccStateForNetworkLock " + "phoneId =" + mNextRepollStatePhoneId);

                ///M: call repollIccStateForNetworkLock will trigger telephony to resend
                /// sim_ state_change event of specified sim id
                int subId = KeyguardUtils.getSubIdUsingPhoneId(mNextRepollStatePhoneId) ;
                IMtkTelephonyEx.Stub.asInterface(ServiceManager.getService("phoneEx"))
                    .repollIccStateForNetworkLock(subId, true);
            } catch (RemoteException e) {
                Log.d(TAG, "repollIccStateForNetworkLock exception caught");
            }
        } else {
            // it will try next security screen or finish
            if (!KeyguardUtils.isUnlockSimMeWithDeviceSupport()) {
                mCallback.dismiss(true, KeyguardUpdateMonitor.getCurrentUser());
            }
        }
    }

    /// M: check next subscription lock state is ME lock or not
    /// return subId if we found otherwise return 0
    private int getNextRepollStatePhoneId() {
        if ((IccCardConstants.State.NETWORK_LOCKED == mLastSimState) &&
             KeyguardUtils.isMediatekSimMeLockSupport()) {
            for (int i = 0; i < KeyguardUtils.getNumOfPhone(); i++) {
                if (!mSecurityModel.isPinPukOrMeRequiredOfPhoneId(i)) {
                    continue;
                }

                final IccCardConstants.State simState = mUpdateMonitor.getSimStateOfPhoneId(i);
                if (simState == IccCardConstants.State.NETWORK_LOCKED) {
                    return i;
                } else {
                    break;  // for PIN or PUK lock, return INVALID_PHONE_ID
                }
            }
        }
        return KeyguardUtils.INVALID_PHONE_ID;
    }

    public static class Toast {
        static final String LOCAL_TAG = "Toast";
        static final boolean LOCAL_LOGV = false;

        final Handler mHandler = new Handler();
        final Context mContext;
        final TN mTN;
        int mGravity = Gravity.CENTER_HORIZONTAL | Gravity.BOTTOM;
        int mY;
        View mView;

        public Toast(Context context) {
            mContext = context;
            mTN = new TN();
            mY = context.getResources()
                .getDimensionPixelSize(com.android.internal.R.dimen.toast_y_offset);
        }

        public static Toast makeText(Context context, CharSequence text) {
            Toast result = new Toast(context);

            LayoutInflater inflate = (LayoutInflater) context
                    .getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            View v = inflate.inflate(com.android.internal.R.layout.transient_notification, null);
            TextView tv = (TextView) v.findViewById(com.android.internal.R.id.message);
            tv.setText(text);

            result.mView = v;

            return result;
        }

        /**
         * Show the view for the specified duration.
         */
        public void show() {
            if (mView == null) {
                throw new RuntimeException("setView must have been called");
            }
            INotificationManager service = getService();
            String pkg = mContext.getPackageName();
            TN tn = mTN;
            try {
                service.enqueueToast(pkg, tn, 0, 0);
            } catch (RemoteException e) {
                // Empty
            }
        }

        /**
         * Close the view if it's showing, or don't show it if it isn't showing yet.
         * You do not normally have to call this.
         * Normally view will disappear on its own after the appropriate duration.
         */
        public void cancel() {
            mTN.hide();
        }

        private INotificationManager mService;

        private INotificationManager getService() {
            if (mService != null) {
                return mService;
            }
            mService = INotificationManager.Stub
                    .asInterface(ServiceManager.getService("notification"));
            return mService;
        }

        private class TN extends ITransientNotification.Stub {
            final Runnable mShow = new Runnable() {
                public void run() {
                    handleShow();
                }
            };

            final Runnable mHide = new Runnable() {
                public void run() {
                    handleHide();
                }
            };

            private final WindowManager.LayoutParams mParams = new WindowManager.LayoutParams();

            WindowManagerImpl mWM;

            TN() {
                final WindowManager.LayoutParams params = mParams;
                params.height = WindowManager.LayoutParams.WRAP_CONTENT;
                params.width = WindowManager.LayoutParams.WRAP_CONTENT;
                params.flags = WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE
                        | WindowManager.LayoutParams.FLAG_NOT_TOUCHABLE
                        | WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON;
                params.format = PixelFormat.TRANSLUCENT;
                params.windowAnimations = com.android.internal.R.style.Animation_Toast;
                params.type = WindowManager.LayoutParams.TYPE_KEYGUARD_DIALOG;
                params.setTitle("Toast");
            }

            /**
             * schedule handleShow into the right thread
             */
            public void show(IBinder windowToken) {
                if (LOCAL_LOGV) {
                    Log.d(LOCAL_TAG, "SHOW: " + this);
                }
                mHandler.post(mShow);
            }

            /**
             * schedule handleHide into the right thread
             */
            public void hide() {
                if (LOCAL_LOGV) {
                    Log.d(LOCAL_TAG, "HIDE: " + this);
                }
                mHandler.post(mHide);
            }

            public void handleShow() {
                if (LOCAL_LOGV) {
                    Log.d(LOCAL_TAG, "HANDLE SHOW: " + this + " mView=" + mView);
                }

                mWM = (WindowManagerImpl) mContext.getSystemService(Context.WINDOW_SERVICE);
                final int gravity = mGravity;
                mParams.gravity = gravity;
                if ((gravity & Gravity.HORIZONTAL_GRAVITY_MASK) == Gravity.FILL_HORIZONTAL) {
                    mParams.horizontalWeight = 1.0f;
                }
                if ((gravity & Gravity.VERTICAL_GRAVITY_MASK) == Gravity.FILL_VERTICAL) {
                    mParams.verticalWeight = 1.0f;
                }
                mParams.y = mY;
                if (mView != null) {
                    if (mView.getParent() != null) {
                        if (LOCAL_LOGV) {
                            Log.d(LOCAL_TAG, "REMOVE! " + mView + " in " + this);
                        }
                        mWM.removeView(mView);
                    }
                    if (LOCAL_LOGV) {
                        Log.d(LOCAL_TAG, "ADD! " + mView + " in " + this);
                    }
                    mWM.addView(mView, mParams);
                }
            }

            public void handleHide() {
                if (LOCAL_LOGV) {
                    Log.d(LOCAL_TAG, "HANDLE HIDE: " + this + " mView=" + mView);
                }
                if (mView != null) {
                    // note: checking parent() just to make sure the view has
                    // been added... i have seen cases where we get here when
                    // the view isn't yet added, so let's try not to crash.
                    if (mView.getParent() != null) {
                        if (LOCAL_LOGV) {
                            Log.d(LOCAL_TAG, "REMOVE! " + mView + " in " + this);
                        }
                        mWM.removeView(mView);
                    }

                    mView = null;
                }
            }
        }
    }
    /// M: Mediatek added variable for Operation plugin feature
    private IKeyguardUtilExt mKeyguardUtilExt;

    /// M: [ALPS00830104] Refresh the information while the window focus is changed.
    @Override
    public void onWindowFocusChanged(boolean hasWindowFocus) {
        Log.d(TAG, "onWindowFocusChanged(hasWindowFocus = " + hasWindowFocus + ")") ;
        if (hasWindowFocus) {
            ///M: fix ALPS01973175
            resetPasswordText(true /* animate */, false /* announce */);
            KeyguardUtils.requestImeStatusRefresh(mContext) ;
        }
    }

    //Newly added function in Android L
    @Override
    public void startAppearAnimation() {
        // noop.
    }

   @Override
    public boolean startDisappearAnimation(Runnable finishRunnable) {
        return false;
    }

    private void showDismissIfNeed(IccCardConstants.State simState) {
        if (simState == IccCardConstants.State.NETWORK_LOCKED
                && KeyguardUtils.isMediatekSimMeLockSupport()
                && KeyguardUtils.isDismissSimMeLockSupport()) {
            mDismissButton.setVisibility(View.VISIBLE);
            mDismissButton.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    mUpdateMonitor.setSimmeDismissFlagOfPhoneId(mPhoneId, true);
                    resetPasswordText(true /* animate */, false /* announce */);
                    mCallback.dismiss(true, KeyguardUpdateMonitor.getCurrentUser());
                }
            });
        } else {
            mDismissButton.setVisibility(View.INVISIBLE);
            mDismissButton.setOnClickListener(null);
        }
    }
}