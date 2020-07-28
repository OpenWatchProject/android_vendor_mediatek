/*
 * Copyright (C) 2014 The Android Open Source Project
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

package com.android.services.telephony;

import android.content.Context;
import android.content.Intent;
import android.os.UserHandle;
import android.provider.Settings;
import android.telephony.TelephonyManager;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneFactory;

import java.util.ArrayList;
import java.util.List;

/// M: CC: Emergency mode for Fusion RIL @{
import com.mediatek.internal.telephony.IRadioPower;
import com.mediatek.internal.telephony.RadioCapabilitySwitchUtil;
import com.mediatek.internal.telephony.RadioManager;
import com.mediatek.services.telephony.MtkTelephonyConnectionServiceUtil;
/// @}

/**
 * Helper class that implements special behavior related to emergency calls or making phone calls
 * when the radio is in the POWER_OFF STATE. Specifically, this class handles the case of the user
 * trying to dial an emergency number while the radio is off (i.e. the device is in airplane mode)
 * or a normal number while the radio is off (because of the device is on Bluetooth), by turning the
 * radio back on, waiting for it to come up, and then retrying the call.
 */
public class RadioOnHelper implements RadioOnStateListener.Callback {

    private final Context mContext;
    private RadioOnStateListener.Callback mCallback;
    private List<RadioOnStateListener> mListeners;
    private List<RadioOnStateListener> mInProgressListeners;
    private boolean mIsRadioOnCallingEnabled;

    /// M: CC: Emergency mode for Fusion RIL @{
    private RadioPowerInterface mRadioPowerIf = null;
    class RadioPowerInterface implements IRadioPower {
        public void notifyRadioPowerChange(boolean power, int phoneId) {
            Log.d(this, "notifyRadioPowerChange, power:" + power + " phoneId:" + phoneId);
            // Timing to set ECM:
            // - Under flight mode (set in TeleService, use EFUN channel):
            //    - before Radio on and to main capability slot only
            if (power == true) {
                int mainPhoneId = RadioCapabilitySwitchUtil.getMainCapabilityPhoneId();
                Phone mainPhone = PhoneFactory.getPhone(mainPhoneId);
                MtkTelephonyConnectionServiceUtil.getInstance()
                        .enterEmergencyMode(mainPhone, 1/*airplane*/);
                /// M: CC: Emergency mode for Fusion RIL @{
                RadioManager.unregisterForRadioPowerChange(mRadioPowerIf);
                mRadioPowerIf = null;
                /// @}
            }
        }
    }
    /// @}

    public RadioOnHelper(Context context) {
        mContext = context;
        /// M: CC: support MSIM
        //mInProgressListeners = new ArrayList<>(2);
        mInProgressListeners = new ArrayList<>(TelephonyManager.getDefault().getPhoneCount());
    }

    private void setupListeners() {
        if (mListeners != null) {
            return;
        }
        /// M: CC: support MSIM
        //mListeners = new ArrayList<>(2);
        mListeners = new ArrayList<>(TelephonyManager.getDefault().getPhoneCount());
        for (int i = 0; i < TelephonyManager.getDefault().getPhoneCount(); i++) {
            mListeners.add(new RadioOnStateListener());
        }
    }
    /**
     * Starts the "turn on radio" sequence. This is the (single) external API of the
     * RadioOnHelper class.
     *
     * This method kicks off the following sequence:
     * - Power on the radio for each Phone
     * - Listen for radio events telling us the radio has come up.
     * - Retry if we've gone a significant amount of time without any response from the radio.
     * - Finally, clean up any leftover state.
     *
     * This method is safe to call from any thread, since it simply posts a message to the
     * RadioOnHelper's handler (thus ensuring that the rest of the sequence is entirely
     * serialized, and runs on the main looper.)
     */
    public void triggerRadioOnAndListen(RadioOnStateListener.Callback callback) {
        setupListeners();
        mCallback = callback;
        mInProgressListeners.clear();
        mIsRadioOnCallingEnabled = false;

        /// M: CC: Emergency mode for Fusion RIL @{
        mRadioPowerIf = new RadioPowerInterface();
        RadioManager.registerForRadioPowerChange("RadioOnHelper", mRadioPowerIf);
        /// @}

        for (int i = 0; i < TelephonyManager.getDefault().getPhoneCount(); i++) {
            Phone phone = PhoneFactory.getPhone(i);
            if (phone == null) {
                continue;
            }

            mInProgressListeners.add(mListeners.get(i));

            /// M: CC: [ALPS04079763] Fix timing issue. If it's entering flight mode but
            // radio is not off yet, wait for radio off first. Or ECC will be dialed immediately
            // and fail because radio is off. @{
            boolean waitForRadioOffFirst = (Settings.Global.getInt(mContext.getContentResolver(),
                    Settings.Global.AIRPLANE_MODE_ON, 0) > 0) && phone.isRadioOn();
            mListeners.get(i).setWaitForRadioOffFirst(waitForRadioOffFirst);
            /// @}

            mListeners.get(i).waitForRadioOn(phone, this);
        }

        powerOnRadio();
    }
    /**
     * Attempt to power on the radio (i.e. take the device out of airplane mode). We'll eventually
     * get an onServiceStateChanged() callback when the radio successfully comes up.
     */
    private void powerOnRadio() {

        // If airplane mode is on, we turn it off the same way that the Settings activity turns it
        // off.
        if (Settings.Global.getInt(mContext.getContentResolver(),
                Settings.Global.AIRPLANE_MODE_ON, 0) > 0) {
            Log.d(this, "==> Turning off airplane mode for emergency call.");

            // Change the system setting
            Settings.Global.putInt(mContext.getContentResolver(),
                    Settings.Global.AIRPLANE_MODE_ON, 0);

            // Post the broadcast intend for change in airplane mode
            // TODO: We really should not be in charge of sending this broadcast.
            // If changing the setting is sufficient to trigger all of the rest of the logic,
            // then that should also trigger the broadcast intent.
            Intent intent = new Intent(Intent.ACTION_AIRPLANE_MODE_CHANGED);
            intent.putExtra("state", false);
            mContext.sendBroadcastAsUser(intent, UserHandle.ALL);
        }
    }

    /**
     * This method is called from multiple Listeners on the Main Looper.
     * Synchronization is not necessary.
     */
    @Override
    public void onComplete(RadioOnStateListener listener, boolean isRadioReady) {
        mIsRadioOnCallingEnabled |= isRadioReady;
        mInProgressListeners.remove(listener);
        if (mCallback != null && mInProgressListeners.isEmpty()) {
            mCallback.onComplete(null, mIsRadioOnCallingEnabled);
            /// M: CC: Emergency mode for Fusion RIL @{
            if (mRadioPowerIf != null) {
                RadioManager.unregisterForRadioPowerChange(mRadioPowerIf);
            }
            /// @}
        }
    }

    @Override
    public boolean isOkToCall(Phone phone, int serviceState) {
        return (mCallback == null) ? false : mCallback.isOkToCall(phone, serviceState);
    }

    /// M: CC: Add interface to cleanup all listeners. @{
    /**
     * Cleanup all in-progress EmergencyCallStateListener.
     */
    public void cleanup() {
        Log.d(this, "cleanup()");
        List<RadioOnStateListener> listeners
                = new ArrayList<RadioOnStateListener>(mInProgressListeners);
        for (RadioOnStateListener l : listeners) {
            l.cleanup();
        }
    }
    /// @}
}
