/*
 * Copyright (C) 2015 The Android Open Source Project
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
 * limitations under the License
 */

package com.android.phone.settings;

import android.content.Context;
import android.database.ContentObserver;
import android.media.AudioManager;
import android.os.Bundle;
import android.os.Handler;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.preference.PreferenceScreen;
import android.preference.SwitchPreference;
import android.provider.Settings;
import android.telephony.CarrierConfigManager;
import android.telephony.PhoneStateListener;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.android.ims.ImsManager;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneFactory;
import com.android.phone.PhoneGlobals;
import com.android.phone.R;
import com.mediatek.phone.ext.ExtensionManager;
import com.mediatek.settings.CallSettingUtils;

public class AccessibilitySettingsFragment extends PreferenceFragment {
    private static final String LOG_TAG = AccessibilitySettingsFragment.class.getSimpleName();
    private static final boolean DBG = true; //(PhoneGlobals.DBG_LEVEL >= 2);

    private static final String BUTTON_TTY_KEY = "button_tty_mode_key";
    private static final String BUTTON_HAC_KEY = "button_hac_key";
    private static final String BUTTON_RTT_KEY = "button_rtt_key";
    private static final String RTT_INFO_PREF = "button_rtt_more_information_key";

    private final PhoneStateListener mPhoneStateListener = new PhoneStateListener() {
        /**
         * Disable the TTY setting when in/out of a call (and if carrier doesn't
         * support VoLTE with TTY).
         * @see android.telephony.PhoneStateListener#onCallStateChanged(int,
         * java.lang.String)
         */
        @Override
        public void onCallStateChanged(int state, String incomingNumber) {
            if (DBG) Log.d(LOG_TAG, "PhoneStateListener.onCallStateChanged: state=" + state);
            Preference pref = getPreferenceScreen().findPreference(BUTTON_TTY_KEY);
            if (pref != null) {
                // Use TelephonyManager#getCallState instead of 'state' parameter because
                // needs to check the current state of all phone calls to
                // support multi sim configuration.
                TelephonyManager telephonyManager =
                        (TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE);
                final boolean isVolteTtySupported = getVolteTtySupported();
                final boolean isVolteCurrentlyEnabled =
                        ImsManager.isVolteEnabledByPlatform(mContext);
                pref.setEnabled((isVolteTtySupported && isVolteCurrentlyEnabled &&
                        !isVideoCallOrConferenceInProgress()) ||
                        (telephonyManager.getCallState() == TelephonyManager.CALL_STATE_IDLE));
            }
            ///M: For Plugin to handle call state changed @{
            ExtensionManager.getAccessibilitySettingsExt().handleCallStateChanged(
                    AccessibilitySettingsFragment.this, state,
                    R.array.tty_mode_entries, R.array.tty_mode_values);
            ///@}
        }
    };

    private Context mContext;
    private AudioManager mAudioManager;

    private TtyModeListPreference mButtonTty;
    private SwitchPreference mButtonHac;
    private SwitchPreference mButtonRtt;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mContext = getActivity().getApplicationContext();
        mAudioManager = (AudioManager) mContext.getSystemService(Context.AUDIO_SERVICE);

        addPreferencesFromResource(R.xml.accessibility_settings);

        mButtonTty = (TtyModeListPreference) findPreference(
                getResources().getString(R.string.tty_mode_key));
        mButtonHac = (SwitchPreference) findPreference(BUTTON_HAC_KEY);
        mButtonRtt = (SwitchPreference) findPreference(BUTTON_RTT_KEY);

        if (PhoneGlobals.getInstance().phoneMgr.isTtyModeSupported() && isTtySupportedByCarrier()) {
            mButtonTty.init();
        } else {
            getPreferenceScreen().removePreference(mButtonTty);
            mButtonTty = null;
        }

        /// M: Add for mtk features @{
        // PhoneGlobals.getInstance().phoneMgr.isHearingAidCompatibilitySupported()
        if (CallSettingUtils.isHacSupport()) {
        /// @}
            int hac = Settings.System.getInt(mContext.getContentResolver(),
                    Settings.System.HEARING_AID, SettingsConstants.HAC_DISABLED);
            mButtonHac.setChecked(hac == SettingsConstants.HAC_ENABLED);
        } else {
            getPreferenceScreen().removePreference(mButtonHac);
            mButtonHac = null;
        }

        if (PhoneGlobals.getInstance().phoneMgr
                .isRttSupported(SubscriptionManager.getDefaultVoiceSubscriptionId())) {
            // TODO: this is going to be a on/off switch for now. Ask UX about how to integrate
            // this settings with TTY
            boolean rttOn = Settings.Secure.getInt(
                    mContext.getContentResolver(), Settings.Secure.RTT_CALLING_MODE, 0) != 0;
            mButtonRtt.setChecked(rttOn);
        } else {
            getPreferenceScreen().removePreference(mButtonRtt);
            Preference rttInfoPref = findPreference(RTT_INFO_PREF);
            getPreferenceScreen().removePreference(rttInfoPref);
            mButtonRtt = null;
        }

        /// M: Add for mtk features @{
        initUi(getPreferenceScreen());
        /// @}
    }

    @Override
    public void onResume() {
        super.onResume();
        TelephonyManager tm =
                (TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE);
        tm.listen(mPhoneStateListener, PhoneStateListener.LISTEN_CALL_STATE);

        /// M: Observer TTY mode @{
        mContext.getContentResolver().registerContentObserver(Settings.Secure.getUriFor(
                Settings.Secure.PREFERRED_TTY_MODE), false, mTtyObserver);
        /// @}
    }

    @Override
    public void onPause() {
        super.onPause();
        TelephonyManager tm =
                (TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE);
        tm.listen(mPhoneStateListener, PhoneStateListener.LISTEN_NONE);

        /// M: Observer TTY mode @{
        mContext.getContentResolver().unregisterContentObserver(mTtyObserver);
        /// @}
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference) {
        if (preference == mButtonTty) {
            return true;
        } else if (preference == mButtonHac) {
            int hac = mButtonHac.isChecked()
                    ? SettingsConstants.HAC_ENABLED : SettingsConstants.HAC_DISABLED;
            // Update HAC value in Settings database.
            Settings.System.putInt(mContext.getContentResolver(), Settings.System.HEARING_AID, hac);

            // Update HAC Value in AudioManager.
            mAudioManager.setParameter(SettingsConstants.HAC_KEY,
                    hac == SettingsConstants.HAC_ENABLED
                            ? SettingsConstants.HAC_VAL_ON : SettingsConstants.HAC_VAL_OFF);
            return true;
        } else if (preference == mButtonRtt) {
            Log.i(LOG_TAG, "RTT setting changed -- now " + mButtonRtt.isChecked());
            int rttMode = mButtonRtt.isChecked() ? 1 : 0;
            Settings.Secure.putInt(mContext.getContentResolver(), Settings.Secure.RTT_CALLING_MODE,
                    rttMode);
            // Update RTT config with IMS Manager
            ImsManager imsManager = ImsManager.getInstance(getContext(),
                    SubscriptionManager.getDefaultVoicePhoneId());
            imsManager.setRttEnabled(mButtonRtt.isChecked());
            return true;
        /// M: Add for mtk feature @{
        } else {
            return onPreferenceTreeClick(preference);
        }
        /// @}
    }

    private boolean getVolteTtySupported() {
        CarrierConfigManager configManager =
                (CarrierConfigManager) mContext.getSystemService(Context.CARRIER_CONFIG_SERVICE);
        return configManager.getConfig().getBoolean(
                CarrierConfigManager.KEY_CARRIER_VOLTE_TTY_SUPPORTED_BOOL);
    }

    private boolean isVideoCallOrConferenceInProgress() {
        final Phone[] phones = PhoneFactory.getPhones();
        if (phones == null) {
            if (DBG) Log.d(LOG_TAG, "isVideoCallOrConferenceInProgress: No phones found.");
            return false;
        }

        for (Phone phone : phones) {
            if (phone.isImsVideoCallOrConferencePresent()) {
                return true;
            }
        }
        return false;
    }

    /**
     * Determines if the device supports TTY per carrier config.
     * @return {@code true} if the carrier supports TTY, {@code false} otherwise.
     */
    private boolean isTtySupportedByCarrier() {
        CarrierConfigManager configManager =
                (CarrierConfigManager) mContext.getSystemService(Context.CARRIER_CONFIG_SERVICE);
        return configManager.getConfig().getBoolean(
                CarrierConfigManager.KEY_TTY_SUPPORTED_BOOL);
    }

    /// --------------------------------- MTK -------------------------------------
    private SwitchPreference mButtonDualMic;

    private static final String BUTTON_DUAL_MIC_KEY = "button_dual_mic_key";

    private void initUi(PreferenceScreen prf) {
        mButtonDualMic = (SwitchPreference) prf.findPreference(BUTTON_DUAL_MIC_KEY);
        if (CallSettingUtils.isMtkDualMicSupport()) {
            mButtonDualMic.setChecked(CallSettingUtils.isDualMicModeEnabled());
        } else {
            prf.removePreference(mButtonDualMic);
            mButtonDualMic = null;
        }
    }

    private boolean onPreferenceTreeClick(Preference preference) {
        if (preference == mButtonDualMic) {

            Log.d(LOG_TAG, "mButtonDualmic turn on: " + mButtonDualMic.isChecked());

            CallSettingUtils.setDualMicMode(mButtonDualMic.isChecked() ?
                    SettingsConstants.DUA_VAL_ON : SettingsConstants.DUAL_VAL_OFF);
            return true;
        }
        return false;
    }

    private final ContentObserver mTtyObserver = new ContentObserver(new Handler()) {
        @Override
        public void onChange(boolean selfChange) {

            if (mButtonTty != null) {
                Log.d(LOG_TAG, " --- TTY mode changed ---");
                mButtonTty.init();
            }
        }
    };
}
