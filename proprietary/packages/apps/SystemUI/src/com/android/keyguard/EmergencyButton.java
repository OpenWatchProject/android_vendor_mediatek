/*
 * Copyright (C) 2008 The Android Open Source Project
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

package com.android.keyguard;

import android.app.ActivityOptions;
import android.app.ActivityTaskManager;
import android.content.Context;
import android.content.Intent;
import android.content.res.TypedArray;
import android.content.res.Configuration;
import android.os.PowerManager;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemClock;
import android.os.UserHandle;
import android.telephony.ServiceState;
import android.telecom.TelecomManager;
import android.util.AttributeSet;
import android.util.Slog;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewConfiguration;
import android.widget.Button;

import com.android.internal.logging.MetricsLogger;
import com.android.internal.logging.nano.MetricsProto.MetricsEvent;
import com.android.internal.telephony.IccCardConstants.State;
import com.android.internal.util.EmergencyAffordanceManager;
import com.android.internal.widget.LockPatternUtils;
import com.android.systemui.util.EmergencyDialerConstants;

import com.mediatek.keyguard.AntiTheft.AntiTheftManager;
import com.mediatek.keyguard.ext.IEmergencyButtonExt;
import com.mediatek.keyguard.ext.OpKeyguardCustomizationFactoryBase;
import com.mediatek.telephony.MtkTelephonyManagerEx;

/**
 * This class implements a smart emergency button that updates itself based
 * on telephony state.  When the phone is idle, it is an emergency call button.
 * When there's a call in progress, it presents an appropriate message and
 * allows the user to return to the call.
 */
public class EmergencyButton extends Button {

    private static final boolean DEBUG = KeyguardConstants.DEBUG;
    private static final Intent INTENT_EMERGENCY_DIAL = new Intent()
            .setAction(EmergencyDialerConstants.ACTION_DIAL)
            .setPackage("com.android.phone")
            .setFlags(Intent.FLAG_ACTIVITY_NEW_TASK
                    | Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS
                    | Intent.FLAG_ACTIVITY_CLEAR_TOP)
            .putExtra(EmergencyDialerConstants.EXTRA_ENTRY_TYPE,
                    EmergencyDialerConstants.ENTRY_TYPE_LOCKSCREEN_BUTTON);

    private static final String LOG_TAG = "EmergencyButton";
    private final EmergencyAffordanceManager mEmergencyAffordanceManager;

    private int mDownX;
    private int mDownY;
    KeyguardUpdateMonitorCallback mInfoCallback = new KeyguardUpdateMonitorCallback() {

        @Override
        public void onSimStateChanged(int subId, int slotId, State simState) {
            updateEmergencyCallButton();
        }

        @Override
        public void onPhoneStateChanged(int phoneState) {
            updateEmergencyCallButton();
        }

        @Override
        public void onSimStateChangedUsingPhoneId(int phoneId, State simState) {
            if (DEBUG) {
                Slog.d(LOG_TAG, "onSimStateChangedUsingSubId: " + simState + ", phoneId=" + phoneId);
            }
            updateEmergencyCallButton();
        }

        /// M: CTA new feature
        @Override
        public void onRefreshCarrierInfo() {
            updateEmergencyCallButton();
        }
    };
    private boolean mLongPressWasDragged;

    public interface EmergencyButtonCallback {
        public void onEmergencyButtonClickedWhenInCall();
    }

    private LockPatternUtils mLockPatternUtils;
    private PowerManager mPowerManager;

    /// M: For the extra info of the intent to start emergency dialer
    private IEmergencyButtonExt mEmergencyButtonExt;

    private EmergencyButtonCallback mEmergencyButtonCallback;

    private final boolean mIsVoiceCapable;
    private final boolean mEnableEmergencyCallWhileSimLocked;

    /// M : added for phoneId in Ecc intent in none security mode
    private int mEccPhoneIdForNoneSecurityMode = -1;

    /// M : Added for operator feature. This will help to check the position
    // of current ECC button(i.e. Notification Keyguard or Bouncer)
    private boolean mLocateAtNonSecureView = false;

    /// M : Added for got service state when carrier update
    private MtkTelephonyManagerEx mTelephonyManager;

    /// M: Optimization, save lockpatternUtils's isSecure state
    private boolean mIsSecure;

    public EmergencyButton(Context context) {
        this(context, null);
    }

    public EmergencyButton(Context context, AttributeSet attrs) {
        super(context, attrs);
        mIsVoiceCapable = context.getResources().getBoolean(
                com.android.internal.R.bool.config_voice_capable);
        mEnableEmergencyCallWhileSimLocked = mContext.getResources().getBoolean(
                com.android.internal.R.bool.config_enable_emergency_call_while_sim_locked);
        mEmergencyAffordanceManager = new EmergencyAffordanceManager(context);

        /// M: Init keyguard operator plugin @{
        try {
            mEmergencyButtonExt = OpKeyguardCustomizationFactoryBase.getOpFactory(mContext).
                    makeEmergencyButton();
        } catch (Exception e) {
            Slog.d(LOG_TAG, "EmergencyButton() - error in calling getEmergencyButtonExt().") ;
            e.printStackTrace();
        }
        /// @}

        TypedArray localAtNonSecureValue =
                context.obtainStyledAttributes(attrs, R.styleable.ECCButtonAttr);
        mLocateAtNonSecureView = localAtNonSecureValue.getBoolean(
                R.styleable.ECCButtonAttr_locateAtNonSecureView, mLocateAtNonSecureView);
        localAtNonSecureValue = null;

        mTelephonyManager = MtkTelephonyManagerEx.getDefault();
    }

    @Override
    protected void onAttachedToWindow() {
        super.onAttachedToWindow();
        KeyguardUpdateMonitor.getInstance(mContext).registerCallback(mInfoCallback);
    }

    @Override
    protected void onDetachedFromWindow() {
        super.onDetachedFromWindow();
        KeyguardUpdateMonitor.getInstance(mContext).removeCallback(mInfoCallback);
    }

    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();
        mLockPatternUtils = new LockPatternUtils(mContext);
        mPowerManager = (PowerManager) mContext.getSystemService(Context.POWER_SERVICE);
        setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                takeEmergencyCallAction();
            }
        });

        /// M: Save secure query result here, when lockscreen is created, secure result should
        /// stay unchanged @{
        mIsSecure = mLockPatternUtils.isSecure(KeyguardUpdateMonitor.getCurrentUser());
        /// @}

        setOnLongClickListener(new OnLongClickListener() {
            @Override
            public boolean onLongClick(View v) {
                if (!mLongPressWasDragged
                        && mEmergencyAffordanceManager.needsEmergencyAffordance()) {
                    mEmergencyAffordanceManager.performEmergencyCall();
                    return true;
                }
                return false;
            }
        });
        updateEmergencyCallButton();
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        final int x = (int) event.getX();
        final int y = (int) event.getY();
        if (event.getActionMasked() == MotionEvent.ACTION_DOWN) {
            mDownX = x;
            mDownY = y;
            mLongPressWasDragged = false;
        } else {
            final int xDiff = Math.abs(x - mDownX);
            final int yDiff = Math.abs(y - mDownY);
            int touchSlop = ViewConfiguration.get(mContext).getScaledTouchSlop();
            if (Math.abs(yDiff) > touchSlop || Math.abs(xDiff) > touchSlop) {
                mLongPressWasDragged = true;
            }
        }
        return super.onTouchEvent(event);
    }

    @Override
    public boolean performLongClick() {
        return super.performLongClick();
    }

    @Override
    protected void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        ///M: fix ALPS01969662, force to reload string when config(locale) changed.
        setText(R.string.kg_emergency_call_label);
        updateEmergencyCallButton();
    }

    /**
     * Shows the emergency dialer or returns the user to the existing call.
     */
    public void takeEmergencyCallAction() {
        MetricsLogger.action(mContext, MetricsEvent.ACTION_EMERGENCY_CALL);
        // TODO: implement a shorter timeout once new PowerManager API is ready.
        // should be the equivalent to the old userActivity(EMERGENCY_CALL_TIMEOUT)
        mPowerManager.userActivity(SystemClock.uptimeMillis(), true);
        try {
            ActivityTaskManager.getService().stopSystemLockTaskMode();
        } catch (RemoteException e) {
            Slog.w(LOG_TAG, "Failed to stop app pinning");
        }
        if (isInCall()) {
            resumeCall();
            if (mEmergencyButtonCallback != null) {
                mEmergencyButtonCallback.onEmergencyButtonClickedWhenInCall();
            }
        } else {
            KeyguardUpdateMonitor.getInstance(mContext).reportEmergencyCallAction(
                    true /* bypassHandler */);

            /// M: Fill the extra info the intent to start emergency dialer.
            /// M: add for Ecc intent in none security mode
            int phoneId = getCurPhoneId();
            if (phoneId == -1) {
                phoneId = mEccPhoneIdForNoneSecurityMode;
            }
            mEmergencyButtonExt.customizeEmergencyIntent(INTENT_EMERGENCY_DIAL, phoneId);

            getContext().startActivityAsUser(INTENT_EMERGENCY_DIAL,
                    ActivityOptions.makeCustomAnimation(getContext(), 0, 0).toBundle(),
                    new UserHandle(KeyguardUpdateMonitor.getCurrentUser()));
        }
    }

    private void updateEmergencyCallButton() {
        boolean visible = false;
        /// M: remove mIsVoiceCapable qualification @{
        //if (mIsVoiceCapable) {
            // Emergency calling requires voice capability.
        if (isInCall()) {
            visible = true; // always show "return to call" if phone is off-hook
        } else if (mIsVoiceCapable){
            final boolean simLocked = KeyguardUpdateMonitor.getInstance(mContext)
                    .isSimPinVoiceSecure();
            if (simLocked) {
                // Some countries can't handle emergency calls while SIM is locked.
                visible = mEnableEmergencyCallWhileSimLocked;
            } else {
                // Only show if there is a secure screen (pin/pattern/SIM pin/SIM puk);
                visible = mLockPatternUtils.isSecure(KeyguardUpdateMonitor.getCurrentUser());
            }
        }

        /// M: If antitheft lock is on, we should also show ECC button
        boolean antiTheftLocked = AntiTheftManager.isAntiTheftLocked();
        boolean show = false;

        /// M:CTA new feature
        boolean eccShouldShow = eccButtonShouldShow();

        if (DEBUG) {
            Slog.i(LOG_TAG, "mLocateAtNonSecureView = " + mLocateAtNonSecureView) ;
        }
        /// @}

        if (mLocateAtNonSecureView && !mEmergencyButtonExt.showEccInNonSecureUnlock()) {
            if (DEBUG) {
                Slog.i(LOG_TAG, "ECC Button is located on Notification Keygaurd and OP do not ask"
                        + " to show. So this is a normal case ,we never show it.") ;
            }
            show = false ;
        } else {
            show = (visible || antiTheftLocked
                    || mEmergencyButtonExt.showEccInNonSecureUnlock())
                    && eccShouldShow;
            Slog.i(LOG_TAG, "show = " + show );
            if (DEBUG) {
                Slog.i(LOG_TAG, "visible= " + visible + ", antiTheftLocked="
                        + antiTheftLocked + ", mEmergencyButtonExt.showEccInNonSecureUnlock() ="
                        + mEmergencyButtonExt.showEccInNonSecureUnlock() + ", eccShouldShow="
                        + eccShouldShow);
            }
        }

        if (mLocateAtNonSecureView && !show) {
            this.setVisibility(View.GONE);
        } else {
            updateEmergencyCallButtonState(show, false);
        }
    }

    public void setCallback(EmergencyButtonCallback callback) {
        mEmergencyButtonCallback = callback;
    }

    /**
     * Resumes a call in progress.
     */
    private void resumeCall() {
        getTelecommManager().showInCallScreen(false);
    }

    /**
     * @return {@code true} if there is a call currently in progress.
     */
    private boolean isInCall() {
        return getTelecommManager().isInCall();
    }

    private TelecomManager getTelecommManager() {
        return (TelecomManager) mContext.getSystemService(Context.TELECOM_SERVICE);
    }

    /// M: CTA new feature
    private boolean eccButtonShouldShow() {
        int phoneCount = KeyguardUtils.getNumOfPhone();
        boolean[] isServiceSupportEcc = new boolean[phoneCount];
        /// M: add for Ecc intent in none security mode
        mEccPhoneIdForNoneSecurityMode = -1;

        if (mTelephonyManager != null) {
            for (int i = 0; i < phoneCount; i++) {
                ServiceState ss = mTelephonyManager.getServiceStateByPhoneId(i);
                if (ss != null) {
                    Slog.i(LOG_TAG, "ss.getState()=" + ss.getState() + " ss.isEmergencyOnly()="
                            + ss.isEmergencyOnly() + " for simId=" + i);
                    if (ServiceState.STATE_IN_SERVICE == ss.getState()
                            || ss.isEmergencyOnly()) {  //Full service or Limited service
                        isServiceSupportEcc[i] = true;
                        /// M: add for Ecc intent in none security mode
                        if (mEccPhoneIdForNoneSecurityMode == -1) {
                            mEccPhoneIdForNoneSecurityMode = i;
                        }
                    } else {
                        isServiceSupportEcc[i] = false;
                    }
                } else {
                    Slog.e(LOG_TAG, "Service state is null");
                }
            }
        }

        return mEmergencyButtonExt.showEccByServiceState(isServiceSupportEcc, getCurPhoneId());
    }

    /**
     * M: Add for operator customization.
     * Get current sim slot id of PIN/PUK lock via security mode.
     *
     * @return Current sim phone id,
     *      return 0-3, current lockscreen is PIN/PUK,
     *      return -1, current lockscreen is not PIN/PUK.
     */
    private int getCurPhoneId() {
        KeyguardSecurityModel securityModel = new KeyguardSecurityModel(mContext);
        return securityModel.getPhoneIdUsingSecurityMode(securityModel
                .getSecurityMode(KeyguardUpdateMonitor.getCurrentUser()));
    }

    public void updateEmergencyCallButtonState(boolean shown, boolean showIcon) {
        if (mIsVoiceCapable && shown) {
            this.setVisibility(View.VISIBLE);
        } else {
            this.setVisibility(View.INVISIBLE);
            return;
        }

        int textId;
        if (isInCall()) {
            // show "return to call" text and show phone icon
            textId = com.android.internal.R.string.lockscreen_return_to_call;
            int phoneCallIcon = showIcon ? com.android.internal.R.drawable.stat_sys_phone_call : 0;
            this.setCompoundDrawablesWithIntrinsicBounds(phoneCallIcon, 0, 0, 0);
        } else {
            textId = com.android.internal.R.string.lockscreen_emergency_call;
            int emergencyIcon = showIcon ? com.android.internal.R.drawable.ic_emergency : 0;
            this.setCompoundDrawablesWithIntrinsicBounds(emergencyIcon, 0, 0, 0);
        }
        this.setText(textId);
    }
}
