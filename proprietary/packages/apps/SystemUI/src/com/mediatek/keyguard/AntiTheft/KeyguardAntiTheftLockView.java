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

package com.mediatek.keyguard.AntiTheft;

import android.content.Context;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;

import com.android.internal.widget.LockPatternUtils;
import com.android.keyguard.EmergencyCarrierArea;
import com.android.keyguard.KeyguardPinBasedInputView;
import com.android.keyguard.KeyguardMessageArea;
import com.android.keyguard.KeyguardUpdateMonitor;
import com.android.keyguard.SecurityMessageDisplay;
import com.android.keyguard.R;

/**
 * Displays a PIN pad for unlocking.
 */
public class KeyguardAntiTheftLockView extends KeyguardPinBasedInputView {

    private static final String TAG = "KeyguardAntiTheftLockView";

    private Context mContext;
    private ViewGroup mBouncerFrameView;
    private AntiTheftManager mAntiTheftManager;

    private static int mReportUnlockAttemptTimeout = 30000;

    /**
     * Init keyguard antitheft view.
     * @param content the Context
     */
    public KeyguardAntiTheftLockView(Context context) {
        this(context, null);
    }

    /**
     * Init keyguard antitheft view.
     * @param content the Context
     * @param attrs the view attribute
     */
    public KeyguardAntiTheftLockView(Context context, AttributeSet attrs) {
        super(context, attrs);
        mAntiTheftManager = AntiTheftManager.getInstance(context, null, null);
    }

    protected void resetState() {
        super.resetState();

        updateKeypadVisibility();
    }

    @Override
    protected int getPasswordTextViewId() {
        return R.id.antiTheftPinEntry;
    }

    @Override
    protected void verifyPasswordAndUnlock() {
        //String entry = mPasswordEntry.getText();
        byte[] entry = getPasswordText();
        boolean isLockOut = false;
        int userId = KeyguardUpdateMonitor.getCurrentUser();
        Log.d(TAG, "verifyPasswordAndUnlock is called.");

        if (AntiTheftManager.getInstance(null, null, null).checkPassword(entry)) {
            mCallback.reportUnlockAttempt(userId, true, mReportUnlockAttemptTimeout);
            mCallback.dismiss(true, userId);

            /// M: ALPS01370779 Because other security views except AntiTheft mode can
            /// show more info on status bar, we need to call
            /// KeyguardViewMediator.adjustStatusBarLocked() to reset/adjust status bar
            /// info to reshow more info of other security modes.
            AntiTheftManager.getInstance(null, null, null).adjustStatusBarLocked();
       } else if (entry.length > MINIMUM_PASSWORD_LENGTH_BEFORE_REPORT) {
            Log.d(TAG, "verifyPasswordAndUnlock fail");

            // to avoid accidental lockout, only count attempts that are long enough to be a
            // real password. This may require some tweaking.
            mCallback.reportUnlockAttempt(userId, false, mReportUnlockAttemptTimeout);
            if (0 == (mLockPatternUtils.getCurrentFailedPasswordAttempts(userId)
                   % LockPatternUtils.FAILED_ATTEMPTS_BEFORE_WIPE_GRACE)) {
               long deadline = mLockPatternUtils.setLockoutAttemptDeadline(
                           KeyguardUpdateMonitor.getCurrentUser(), mReportUnlockAttemptTimeout);
               handleAttemptLockout(deadline);
               isLockOut = true;
           }
           mSecurityMessageDisplay.setMessage(getWrongPasswordStringId());
       }
       //mPasswordEntry.setText("");
       setPasswordEntryEnabled(true);
       resetPasswordText(true, false);

       ///M: fix ALPS01952796(side effect of ALPS01926268)
       if (isLockOut) {
            setPasswordEntryEnabled(false);
        }
    }


    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();

        Log.d(TAG, "onFinishInflate() is called");

        mBouncerFrameView  = (ViewGroup) findViewById(R.id.keyguard_bouncer_frame);

        //mPasswordEntry.requestFocus();

        if (!AntiTheftManager.isKeypadNeeded()) {
            Log.d(TAG, "onFinishInflate, not need keypad");
            mBouncerFrameView.setVisibility(View.INVISIBLE);
        }

        // Suppose an AntiTheftManager object was already created now.
        // Some kinds of anti-theft locks need to use the functions of related services.
        // We should bind these services as early as possible.
        AntiTheftManager.getInstance(null, null, null).doBindAntiThftLockServices();

        if (mEcaView instanceof EmergencyCarrierArea) {
            ((EmergencyCarrierArea) mEcaView).setCarrierTextVisible(true);
        }
    }

    @Override
    public void showUsabilityHint() {
    }

    @Override
    public int getWrongPasswordStringId() {
        return R.string.kg_wrong_pin;
    }

    @Override
    public void onAttachedToWindow() {
        super.onAttachedToWindow();

        // Suppose an AntiTheftManager object was already created now.
        // Some kinds of anti-theft locks need to use the functions of related services.
        // We should bind these services as early as possible.
        AntiTheftManager.getInstance(null, null, null).doBindAntiThftLockServices() ;
    }

    @Override
    public void onDetachedFromWindow() {
        Log.d(TAG, "onDetachedFromWindow() is called.");
        super.onDetachedFromWindow();
        mAntiTheftManager.setSecurityViewCallback(null);
    }

    @Override
    public void onPause() {
        Log.d(TAG, "onPause");
        //AntiTheftManager.setKeyguardCurrentModeIsAntiTheftMode(false) ;
    }

    @Override
    public void onResume(int reason) {
        super.onResume(reason);
        //final boolean mediaPlaying = AudioSystem.isStreamActive(AudioSystem.STREAM_MUSIC, 0)
        //        || AudioSystem.isStreamActive(AudioSystem.STREAM_FM, 0);

        Log.d(TAG, "onResume");
        if (mSecurityMessageDisplay != null) {
            mSecurityMessageDisplay.setMessage(AntiTheftManager.ANTITHEFT_NONEED_PRINT_TEXT);
        }
        // Suppose an AntiTheftManager object was already created now.
        // Some kinds of anti-theft locks need to use the functions of related services.
        // We should bind these services as early as possible.
        AntiTheftManager.getInstance(null, null, null).doBindAntiThftLockServices();
        mAntiTheftManager.setSecurityViewCallback(mCallback);
        updateKeypadVisibility();
    }

    /*@Override
    public void onWindowFocusChanged(boolean hasWindowFocus) {
        if (hasWindowFocus) {
            KeyguardUtils.requestImeStatusRefresh(mContext) ;
        }
    }*/

    @Override
    public void startAppearAnimation() {
        // noop.
    }

    @Override
    public boolean startDisappearAnimation(Runnable finishRunnable) {
        return false;
    }

    private void updateKeypadVisibility() {
        if (AntiTheftManager.isKeypadNeeded()) {
            //mPasswordEntry.setEnabled(true);
            mBouncerFrameView.setVisibility(View.VISIBLE);
        } else {
            mBouncerFrameView.setVisibility(View.INVISIBLE);
        }
    }
}
