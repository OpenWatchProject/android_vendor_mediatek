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
package com.android.keyguard;

import android.app.admin.DevicePolicyManager;
import android.content.Context;
import android.telephony.SubscriptionManager;
import android.util.Log;

import com.android.internal.telephony.IccCardConstants;
import com.android.internal.widget.LockPatternUtils;

import com.mediatek.keyguard.AntiTheft.AntiTheftManager;

public class KeyguardSecurityModel {

    /**
     * The different types of security available.
     * @see KeyguardSecurityContainer#showSecurityScreen
     */
    public enum SecurityMode {
        Invalid, // NULL state
        None, // No security enabled
        Pattern, // Unlock by drawing a pattern.
        Password, // Unlock by entering an alphanumeric password
        PIN, // Strictly numeric password
        // SimPin, // Unlock by entering a sim pin.
        // SimPuk, // Unlock by entering a sim puk
        SimPinPukMe1, // Unlock by entering a sim pin/puk/me for sim or gemini sim1.
        SimPinPukMe2, // Unlock by entering a sim pin/puk/me for sim or gemini sim2.
        SimPinPukMe3, // Unlock by entering a sim pin/puk/me for sim or gemini sim3.
        SimPinPukMe4, // Unlock by entering a sim pin/puk/me for sim or gemini sim4.
        AntiTheft, // Antitheft feature
    }

    private final Context mContext;
    private final boolean mIsPukScreenAvailable;

    private LockPatternUtils mLockPatternUtils;
    private static final String TAG = "KeyguardSecurityModel";

    public KeyguardSecurityModel(Context context) {
        mContext = context;
        mLockPatternUtils = new LockPatternUtils(context);
        mIsPukScreenAvailable = mContext.getResources().getBoolean(
                com.android.internal.R.bool.config_enable_puk_unlock_screen);
    }

    void setLockPatternUtils(LockPatternUtils utils) {
        mLockPatternUtils = utils;
    }

    public SecurityMode getSecurityMode(int userId) {
        KeyguardUpdateMonitor monitor = KeyguardUpdateMonitor.getInstance(mContext);
        SecurityMode mode = SecurityMode.None;
        for (int i = 0; i < KeyguardUtils.getNumOfPhone(); i++) {
            if (isPinPukOrMeRequiredOfPhoneId(i)) {
                if (0 == i) {
                    mode = SecurityMode.SimPinPukMe1;
                } else if (1 == i) {
                    mode = SecurityMode.SimPinPukMe2;
                } else if (2 == i) {
                    mode = SecurityMode.SimPinPukMe3;
                } else if (3 == i) {
                    mode = SecurityMode.SimPinPukMe4;
                }
                break;
            }
        }

        if (AntiTheftManager.isAntiTheftPriorToSecMode(mode)) {
            Log.d("KeyguardSecurityModel", "should show AntiTheft!") ;
            mode = SecurityMode.AntiTheft;
        }

        /*if (mIsPukScreenAvailable && SubscriptionManager.isValidSubscriptionId(
                monitor.getNextSubIdForState(IccCardConstants.State.PUK_REQUIRED))) {
            return SecurityMode.SimPuk;
        }

        if (SubscriptionManager.isValidSubscriptionId(
                monitor.getNextSubIdForState(IccCardConstants.State.PIN_REQUIRED))) {
            return SecurityMode.SimPin;
        } */

        if (mode == SecurityMode.None) {
            final int security = mLockPatternUtils.getActivePasswordQuality(userId);
            switch (security) {
                case DevicePolicyManager.PASSWORD_QUALITY_NUMERIC:
                case DevicePolicyManager.PASSWORD_QUALITY_NUMERIC_COMPLEX:
                    return SecurityMode.PIN;

                case DevicePolicyManager.PASSWORD_QUALITY_ALPHABETIC:
                case DevicePolicyManager.PASSWORD_QUALITY_ALPHANUMERIC:
                case DevicePolicyManager.PASSWORD_QUALITY_COMPLEX:
                case DevicePolicyManager.PASSWORD_QUALITY_MANAGED:
                    return SecurityMode.Password;

                case DevicePolicyManager.PASSWORD_QUALITY_SOMETHING:
                    return SecurityMode.Pattern;
                case DevicePolicyManager.PASSWORD_QUALITY_UNSPECIFIED:
                    return SecurityMode.None;

                default:
                    throw new IllegalStateException("Unknown security quality:" + security);
            }
        }

        Log.d(TAG, "getSecurityMode() - mode = " + mode);
        return mode;
    }

    /********************************************************
     ** Mediatek add begin
     ********************************************************/

    /**
     * M:
     * This function checking if we need to show the SimPin lock view for this sim id.
     *
     * @param phoneId phoneId
     * @return subId does requre SIM PIN/PUK/ME unlock
     */
    public boolean isPinPukOrMeRequiredOfPhoneId(int phoneId) {
        KeyguardUpdateMonitor updateMonitor = KeyguardUpdateMonitor.getInstance(mContext);
        if (updateMonitor != null) {
            final IccCardConstants.State simState = updateMonitor.getSimStateOfPhoneId(phoneId);

            Log.d(TAG, "isPinPukOrMeRequiredOfSubId() - phoneId = " + phoneId +
                       ", simState = " + simState) ;
            return (
                // check PIN required
                (simState == IccCardConstants.State.PIN_REQUIRED
                && !updateMonitor.getPinPukMeDismissFlagOfPhoneId(phoneId))
                // check PUK required
                || (simState == IccCardConstants.State.PUK_REQUIRED
                && !updateMonitor.getPinPukMeDismissFlagOfPhoneId(phoneId)
                && updateMonitor.getRetryPukCountOfPhoneId(phoneId) != 0)
                // check ME required
                || (simState == IccCardConstants.State.NETWORK_LOCKED
                && !updateMonitor.getPinPukMeDismissFlagOfPhoneId(phoneId)
                && updateMonitor.getSimMeLeftRetryCountOfPhoneId(phoneId) != 0
                && KeyguardUtils.isMediatekSimMeLockSupport()
                && !updateMonitor.getSimmeDismissFlagOfPhoneId(phoneId)
                && KeyguardUtils.isSimMeLockValid(phoneId))
                );
        } else {
            return false;
        }
    }

    /**
     * M:
     * This function return the phone id of input SimPinPukMe mode.
     * @param mode security mode
     * @return phone id. If not in security mode, return -1.
     */
    int getPhoneIdUsingSecurityMode(SecurityMode mode) {
        int phoneId = -1;

        if (isSimPinPukSecurityMode(mode)) {
            phoneId = mode.ordinal() - SecurityMode.SimPinPukMe1.ordinal();
        }
        return phoneId;
    }

    /**
     * M:
     * This function checking if the input security is SimPinPukMe mode or not.
     */
    boolean isSimPinPukSecurityMode(SecurityMode mode) {
        switch(mode) {
            case SimPinPukMe1:
            case SimPinPukMe2:
            case SimPinPukMe3:
            case SimPinPukMe4:
                return true;
            default:
                return false;
        }
    }
}
