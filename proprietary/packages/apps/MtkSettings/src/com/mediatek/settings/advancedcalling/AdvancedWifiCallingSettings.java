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
 * limitations under the License.
 */

package com.mediatek.settings.advancedcalling;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.provider.Settings;
import androidx.preference.ListPreference;
import androidx.preference.Preference;
import android.telecom.TelecomManager;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.widget.Switch;
import android.widget.Toast;

import com.android.ims.ImsConfig;
import com.android.ims.ImsException;
import com.android.ims.ImsManager;
import com.android.internal.logging.nano.MetricsProto.MetricsEvent;
import com.android.internal.telephony.Phone;
import com.android.settings.R;
import com.android.settings.SettingsActivity;
import com.android.settings.SettingsPreferenceFragment;
import com.android.settings.widget.SwitchBar;
import com.mediatek.ims.internal.MtkImsManager;
import com.mediatek.ims.internal.MtkImsManagerEx;
import com.mediatek.internal.telephony.MtkPhoneConstants;
import com.mediatek.settings.sim.TelephonyUtils;


/**
 * "Wi-Fi Calling settings" screen.  This preference screen lets you
 * enable/disable Wi-Fi Calling and change Wi-Fi Calling mode.
 */
public class AdvancedWifiCallingSettings extends SettingsPreferenceFragment
        implements SwitchBar.OnSwitchChangeListener,
        Preference.OnPreferenceChangeListener {

    private static final String TAG = "OP12AdvancedWifiCallingSettings";
    //String keys for preference lookup
    private static final String BUTTON_ROAMING_MODE = "roaming_mode";
    private static final int PREFER_OTHER_CELLULAR_NETWORK = 1;
    private static final int PREFER_WIFI = 0;
    private static boolean sSwitchFlag = true;
    //UI objects
    private SwitchBar mSwitchBar;
    private Switch mSwitch;
    private ListPreference mButtonRoaming;
    private ImsManager mImsManager;
    private Context mContext;
    private boolean mValidListener = false;

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);

        final SettingsActivity activity = (SettingsActivity) getActivity();

        mSwitchBar = activity.getSwitchBar();
        mSwitch = mSwitchBar.getSwitch();
        mSwitchBar.show();
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        mSwitchBar.hide();
    }

    private void showAlert(Intent intent) {

        CharSequence title = intent.getCharSequenceExtra(Phone.EXTRA_KEY_ALERT_TITLE);
        CharSequence message = intent.getCharSequenceExtra(Phone.EXTRA_KEY_ALERT_MESSAGE);

        AlertDialog.Builder builder = new AlertDialog.Builder(mContext);
        builder.setMessage(message)
                .setTitle(title)
                .setIcon(android.R.drawable.ic_dialog_alert)
                .setPositiveButton(android.R.string.ok, null);
        AlertDialog dialog = builder.create();
        dialog.show();
    }

    private IntentFilter mIntentFilter;

    private BroadcastReceiver mIntentReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.d(TAG, "onReceive()... " + action);
            if (action.equals(ImsManager.ACTION_IMS_REGISTRATION_ERROR)) {
                Log.d(TAG, "IMS Registration error, disable WFC Switch");;
                setResultCode(Activity.RESULT_CANCELED);
                mSwitch.setChecked(false);
                showAlert(intent);
            } else if (action.equals("com.android.intent.action.IMS_CONFIG_CHANGED")) {
            	int type = intent.getIntExtra(ImsConfig.EXTRA_CHANGED_ITEM, -1);
            	String value = intent.getStringExtra(ImsConfig.EXTRA_NEW_VALUE);
                Log.i(TAG, "IMS_CONFIG_CHANGED type = " + type + " value = " + value);
                //getActivity().finish();

            } else if (action.equals(TelephonyManager.ACTION_PHONE_STATE_CHANGED)) {
                Log.d(TAG, "Phone state changed, so update the screen");
                updateScreen();
            }
        }
    };

    @Override
    public int getMetricsCategory() {
        return MetricsEvent.WIFI_CALLING;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.xml.advanced_wificalling_settings);

        mContext = getActivity();
        mButtonRoaming = (ListPreference) findPreference(BUTTON_ROAMING_MODE);
        mButtonRoaming.setOnPreferenceChangeListener(this);

        mIntentFilter = new IntentFilter();
        mIntentFilter.addAction(ImsManager.ACTION_IMS_REGISTRATION_ERROR);
        mIntentFilter.addAction(TelephonyManager.ACTION_PHONE_STATE_CHANGED);
        mIntentFilter.addAction("com.android.intent.action.IMS_CONFIG_CHANGED");
        int subId =  SubscriptionManager.getDefaultSubscriptionId();
        int phoneId = SubscriptionManager.getPhoneId(subId);
        mImsManager = ImsManager.getInstance(mContext, phoneId);

    }

    @Override
    public void onResume() {
        super.onResume();

        if (ImsManager.isWfcEnabledByPlatform(mContext)) {
            mSwitchBar.addOnSwitchChangeListener(this);
            mValidListener = true;
        }
        boolean wfcEnabled = ImsManager.isWfcEnabledByUser(mContext);
        mSwitch.setChecked(wfcEnabled);
        mButtonRoaming.setEnabled(wfcEnabled);
        int roamingMode = mImsManager.getWfcMode(true);
        Log.d(TAG, "WFC RoamingMode : " + roamingMode);
        mButtonRoaming.setValue(Integer.toString(roamingMode));

        /// M: update screen
        updateScreen();
        /// @}
        mContext.registerReceiver(mIntentReceiver, mIntentFilter);

        Intent intent = getActivity().getIntent();
        if (intent.getBooleanExtra(Phone.EXTRA_KEY_ALERT_SHOW, false)) {
            showAlert(intent);
        }
    }

    @Override
    public void onPause() {
        super.onPause();

        if (mValidListener) {
            mValidListener = false;
            mSwitchBar.removeOnSwitchChangeListener(this);
        }
        mContext.unregisterReceiver(mIntentReceiver);
    }

    /**
     * Listens to the state change of the switch.
     */
    @Override
    public void onSwitchChanged(final Switch switchView, boolean isChecked) {
        int subId =  SubscriptionManager.getDefaultSubscriptionId();
        int phoneId = SubscriptionManager.getPhoneId(subId);
        Log.d(TAG, "OnSwitchChanged, subId :" + subId + " phoneId :" + phoneId);
        if (isInSwitchProcess()) {
            Log.d(TAG, "[onClick] Switching process ongoing");
            Toast.makeText(getActivity(), R.string.Switch_not_in_use_string, Toast.LENGTH_SHORT)
                    .show();
            mSwitch.setChecked(!isChecked);
            return;
        }
        if (isChecked) {
            Log.d(TAG, "Wifi Switch checked");
            mImsManager.setWfcSetting(isChecked);
            mButtonRoaming.setEnabled(isChecked);
            Log.d(TAG, "Wifi Calling ON");
            sSwitchFlag = true;

            int value = Settings.Global.getInt(getContentResolver(),
                    Settings.Global.WIFI_SLEEP_POLICY,
                    Settings.Global.WIFI_SLEEP_POLICY_NEVER);
            if (value != Settings.Global.WIFI_SLEEP_POLICY_NEVER) {
                Settings.Global.putInt(getContentResolver(), Settings.Global.WIFI_SLEEP_POLICY,
                        Settings.Global.WIFI_SLEEP_POLICY_NEVER);
                AlertDialog.Builder builder = new AlertDialog.Builder(mContext);
                builder.setCancelable(false);
                builder.setMessage(mContext.getString(R.string.wifi_sleep_policy_msg));
                builder.setPositiveButton(android.R.string.ok, null);
                AlertDialog dialog = builder.create();
                dialog.show();
            }
        } else {
            Log.d(TAG, "Wifi Switch Unchecked");
            if (sSwitchFlag) {
                switchView.setChecked(true);
                mSwitchBar.setTextViewLabelAndBackground(true);
                AlertDialog.Builder builder = new AlertDialog.Builder(mContext);
                builder.setCancelable(false);
                builder.setMessage(mContext.getString(R.string.advance_wifi_calling_disable_msg));
                builder.setPositiveButton(R.string.turn_off_wifi_calling,
                            new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            sSwitchFlag = false;
                            switchView.setChecked(false);
                            mSwitchBar.setTextViewLabelAndBackground(false);
                            mImsManager.setWfcSetting(false);
                            mButtonRoaming.setEnabled(false);
                            Log.d(TAG, "Wifi Calling OFF");
                        }
                    });
                builder.setNegativeButton(android.R.string.cancel,
                            new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        sSwitchFlag = true;
                    }
                });
                AlertDialog dialog = builder.create();
                dialog.show();
            }
        }
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        if (preference == mButtonRoaming) {
            mButtonRoaming.setValue((String) newValue);
            int buttonMode = Integer.valueOf((String) newValue);
            int currentMode = mImsManager.getWfcMode(true);
            Log.d(TAG, "onPreferenceChange, buttonMode: " + buttonMode +
                  "\ncurrentMode: " + currentMode);
            if (buttonMode != currentMode) {
                mImsManager.setWfcMode(buttonMode, true);
                Log.d(TAG, "set WFC Roaming mode : " + buttonMode);
            }
        }
        return true;
    }

    private boolean isInSwitchProcess() {
        int imsState = MtkPhoneConstants.IMS_STATE_DISABLED;
        try {
         imsState = MtkImsManagerEx.getInstance()
                        .getImsState(TelephonyUtils.getMainCapabilityPhoneId());
        } catch (ImsException e) {
           return false;
        }
        Log.d(TAG, "isInSwitchProcess , imsState = " + imsState);
        return imsState == MtkPhoneConstants.IMS_STATE_DISABLING
                || imsState == MtkPhoneConstants.IMS_STATE_ENABLING;
    }

    private void updateScreen() {
        final SettingsActivity activity = (SettingsActivity) getActivity();
        if (activity == null) {
            return;
        }
        boolean isWfcEnabled = mSwitchBar.getSwitch().isChecked();
        boolean isCallStateIdle = !TelecomManager.from(activity).isInCall();
        Log.d(TAG, "updateScreen: isWfcEnabled: " + isWfcEnabled
                + ", isCallStateIdle: " + isCallStateIdle);
        mSwitchBar.setEnabled(isCallStateIdle);
        mButtonRoaming.setEnabled(isWfcEnabled && isCallStateIdle);
    }
}
