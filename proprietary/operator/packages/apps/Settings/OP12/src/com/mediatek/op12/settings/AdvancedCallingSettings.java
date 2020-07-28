package com.mediatek.op12.settings;

import android.app.AlertDialog;
import android.app.Dialog;
import android.content.ActivityNotFoundException;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.SystemProperties;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceClickListener;
import android.preference.PreferenceActivity;
import android.telephony.SubscriptionManager;
import android.util.Log;
import android.view.View;

import com.android.ims.ImsConfig;
import com.android.ims.ImsException;
import com.android.ims.ImsManager;

/**
 * Adding Advanced Calling Settings.
 */
public class AdvancedCallingSettings extends PreferenceActivity {

    private static final String TAG = "Op12AdvancedCallingsSettings";
    static final String USERCONSENT = "user_consent";
    private Preference mActivateAdvancedCallingButton;
    private Preference mAdvancedCallingButton;
    private Preference mActivateWfcCalling;
    private Preference mWfcCalling;
    private IntentFilter mIntentFilter;
    private BroadcastReceiver mWfcReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.d(TAG, "onReceive()... " + action);
            if (action.equals("com.android.intent.action.IMS_CONFIG_CHANGED")) {
                handleProvision();
            } else if (action.equals("com.mediatek.intent.action.ACTIVATE_WFC_RSP")) {
                handleProvision();
            }
        }
    };


    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        addPreferencesFromResource(R.xml.advanced_calling_settings);
        Log.d(TAG, "onCreated entry.");
        mIntentFilter = new IntentFilter();
        mIntentFilter.addAction("com.android.intent.action.IMS_CONFIG_CHANGED");
        mIntentFilter.addAction("com.mediatek.intent.action.ACTIVATE_WFC_RSP");
        registerReceiver(mWfcReceiver, mIntentFilter);

        mActivateAdvancedCallingButton =  findPreference("activate_advanced_calling_key");
        mAdvancedCallingButton =  findPreference("advanced_calling_key");
        mActivateWfcCalling = findPreference("activate_wifi_calling_key");
        mWfcCalling =  findPreference("wifi_calling_key");

        mActivateWfcCalling.setOnPreferenceClickListener(
                             new OnPreferenceClickListener() {
            @Override
            public boolean onPreferenceClick(Preference preference) {
                try {
                    Intent intent = new Intent("com.mediatek.intent.action.ACTIVATE_WFC_REQ");
                    intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                    Log.d(TAG, "Will Open ECC add menu tree");
                    startActivity(intent);
                } catch (ActivityNotFoundException e) {
                        Log.e(TAG, e.toString());
                }
                return true;
            }
        });

        if (!ImsManager.isWfcEnabledByPlatform(this)) {
            getPreferenceScreen().removePreference(mActivateWfcCalling);
            getPreferenceScreen().removePreference(mWfcCalling);
        } else {
            controlWFCOptIn();
        }
        mActivateAdvancedCallingButton.setOnPreferenceClickListener(
                             new OnPreferenceClickListener() {
            @Override
            public boolean onPreferenceClick(Preference preference) {
                try {
                    Intent intent = new Intent("com.vzw.hss.intent.action.PROVISION_VOLTE");
                    intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                    Log.d(TAG, "Will Open Verizon app");
                    startActivity(intent);
                } catch (ActivityNotFoundException e) {
                        Log.e(TAG, e.toString());
                }
                return true;
            }
        });
    }


    @Override
    public void onResume() {
        super.onResume();
        Log.d(TAG, "onResumed");
        handleProvision();
    }

    private void handleProvision() {
        Log.d(TAG, "handleProvision entry, ");
        Preference advancedCallingButton =  findPreference("advanced_calling_key");
        Preference activateWfcCalling = findPreference("activate_wifi_calling_key");
        Preference wfcCalling =  findPreference("wifi_calling_key");
        if (ImsManager.isVolteProvisionedOnDevice(this)) {
            Log.d(TAG, "Volte is provisioned");
            mActivateAdvancedCallingButton.setEnabled(false);
            mActivateAdvancedCallingButton.setSummary(getString(
                                  R.string.activated_advanced_calling_summary));
            if (advancedCallingButton == null) {
                getPreferenceScreen().addPreference(mAdvancedCallingButton);
            }
            if (ImsManager.isWfcEnabledByPlatform(this)) {
                if (isWifiCallingProvisioned()) {
                    Log.d(TAG, "WFC provisioned");
                    if (activateWfcCalling != null) {
                        getPreferenceScreen().removePreference(activateWfcCalling);
                    }
                    if (wfcCalling == null) {
                        getPreferenceScreen().addPreference(mWfcCalling);
                    }
                } else {
                    Log.d(TAG, "WFC de-provisioned");
                    if (activateWfcCalling == null) {
                        getPreferenceScreen().addPreference(mActivateWfcCalling);
                        mActivateWfcCalling.setEnabled(true);
                    } else {
                        activateWfcCalling.setEnabled(true);
                    }
                    if (wfcCalling != null) {
                        getPreferenceScreen().removePreference(mWfcCalling);
                    }
                }
            }
        } else {
            Log.d(TAG, "Volte is de-provisioned");
            mActivateAdvancedCallingButton.setEnabled(true);
            mActivateAdvancedCallingButton.setSummary(getString(
                                  R.string.activate_advanced_calling_summary));
            if (advancedCallingButton != null) {
                getPreferenceScreen().removePreference(advancedCallingButton);
            }
            if (ImsManager.isWfcEnabledByPlatform(this)) {
                if (activateWfcCalling == null) {
                    getPreferenceScreen().addPreference(mActivateWfcCalling);
                    mActivateWfcCalling.setEnabled(false);
                } else {
                    activateWfcCalling.setEnabled(false);
                }
                if (wfcCalling != null) {
                    getPreferenceScreen().removePreference(wfcCalling);
                }
            }
        }
    }

   private boolean isWifiCallingProvisioned() {
       if (controlWFCOptIn()) {
           return true;
       }
       Log.d(TAG, "Wfc isWifiCallingProvisioned entry");
        boolean wifiCallingProvisioned = false;
        ImsConfig imsConfig = null;
        ImsManager imsManager = ImsManager.getInstance(this,
                        SubscriptionManager.getDefaultVoicePhoneId());
        if (imsManager != null) {
            try {
                imsConfig = imsManager.getConfigInterface();
                if (imsConfig != null) {
                    int value = imsConfig.getProvisionedValue(
                            ImsConfig.ConfigConstants.VOICE_OVER_WIFI_SETTING_ENABLED);
                    if (value == 1) {
                        wifiCallingProvisioned = true;
                    }
                }
            } catch (ImsException e) {
                Log.e(TAG, "WFC settings not updated, ImsConfig null");
                e.printStackTrace();
            }
        } else {
            Log.e(TAG, "WFC settings not updated, ImsManager null");
        }
        Log.d(TAG, "Wfc provisioned:" + wifiCallingProvisioned);
        return wifiCallingProvisioned;
    }

    private boolean controlWFCOptIn() {
        int wfcOptIn = 0;
        ImsConfig imsConfig = null;
        ImsManager imsManager = ImsManager.getInstance(this,
                                SubscriptionManager.getDefaultVoicePhoneId());
        String wfcOptInString = SystemProperties.get("persist.vendor.mtk_wfc_opt_in", "0");
        Log.d(TAG, "Control WFC Opt-in wfcOptInString = " + wfcOptInString);
        if (wfcOptInString.equals("0")) {
           wfcOptIn = 1;
        }
        if (imsManager != null) {
            try {
                imsConfig = imsManager.getConfigInterface();
                if (imsConfig != null) {
                    int value = imsConfig.getProvisionedValue(
                            ImsConfig.ConfigConstants.VOICE_OVER_WIFI_SETTING_ENABLED);
                    if (value == 1) {
                        return true;
                    }
                    Log.d(TAG, "[controlWFCOptIn], get WFC provision value: " + value);
                }
            } catch (ImsException e) {
                Log.e(TAG, "[controlWFCOptIn]WFC settings not updated, ImsConfig null");
                e.printStackTrace();
                return false;
            }
        } else {
            Log.e(TAG, "[controlWFCOptIn]WFC settings not updated, ImsManager null");
            return false;
        }
        if (wfcOptIn == 1) {
           return true;
        } else {
           return false;
        }
    }
}
