/*
 * Copyright (C) 2018 The Android Open Source Project
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

package com.android.settings.network.telephony;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.os.PersistableBundle;
import android.os.SystemProperties;
import android.provider.Settings;
import android.telephony.CarrierConfigManager;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;

import androidx.preference.ListPreference;
import androidx.preference.Preference;

import com.android.internal.telephony.Phone;
import com.android.settings.R;

import com.mediatek.settings.UtilsExt;

import java.util.List;

/**
 * Preference controller for "Enabled network mode".
 */
public class CustomizeRjioNetworkModePreferenceController extends
        TelephonyBasePreferenceController implements
        ListPreference.OnPreferenceChangeListener {

    private static final String TAG = "CustomizeRjioNetworkModePreferenceController";

    private CarrierConfigManager mCarrierConfigManager;
    private TelephonyManager mTelephonyManager;
    private int mNetworkMode;
    private int mPhoneId;
    private int mSubId;
    private int mPrimaryPhoneSubId;
    private ListPreference mListPreference;
    public CustomizeRjioNetworkModePreferenceController(Context context, String key) {
        super(context, key);
        mCarrierConfigManager = context.getSystemService(CarrierConfigManager.class);
    }

    @Override
    public int getAvailabilityStatus(int subId) {
        boolean visible;
        if (subId == SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
            Log.d(TAG, "getAvailabilityStatus, subId is invalid.");
            visible = false;
        } else {
            visible = UtilsExt.getMobileNetworkSettingsExt(mContext)
                    .customizeRjioNetworkModePreference(subId);
        }
        return visible ? AVAILABLE : CONDITIONALLY_UNAVAILABLE;
    }
    @Override
    public void updateState(Preference preference) {
        super.updateState(preference);
        mListPreference = (ListPreference) preference;
        mNetworkMode = getCurrentNetworkMode();
        customizePreferredNetworkMode(mListPreference, mSubId);
        updatePreferenceValueAndSummary(mListPreference, mNetworkMode);
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object object) {
        final int settingsMode = Integer.parseInt((String) object);
        if (isNetworkUpdateNeeded((ListPreference) preference, settingsMode)) {
            updateNetworkMode((ListPreference) preference, mSubId, settingsMode);
        }
        return true;
    }

    public void init(int subId) {
        mSubId = subId;
        mPhoneId = SubscriptionManager.getSlotIndex(subId);
        mTelephonyManager = TelephonyManager.from(mContext).createForSubscriptionId(mSubId);
        final PersistableBundle carrierConfig = mCarrierConfigManager.getConfigForSubId(mSubId);

    }

    private int getCurrentNetworkMode() {
        int mode = Settings.Global.getInt(mContext.getContentResolver(),
                Settings.Global.PREFERRED_NETWORK_MODE + mSubId,
                Phone.PREFERRED_NT_MODE);
        Log.i(TAG, "getPreferredNetworkMode mode = " + mode);
        return mode;
    }

    /**
     * Update Network Settings UI based on RJIO req.
     * @param buttonEnabledNetworks pref network Setting
     * @param subId Setting sim subid
     */
    public void customizePreferredNetworkMode(ListPreference buttonEnabledNetworks, int subId) {
        if (buttonEnabledNetworks == null) {
            Log.d(TAG, "buttonEnabledNetworks is null");
            return;
        }
        int currentNwMode = getCurrentNetworkMode();
        Log.d(TAG, "Current network mode for subId " + subId + " : " + currentNwMode);
        CharSequence entries[] = mContext.getResources().getTextArray(
                R.array.rjio_network_mode_options);
        CharSequence entryValues[] = mContext.getResources().getTextArray(
                R.array.rjio_network_mode_options_values);
        buttonEnabledNetworks.setEntries(entries);
        buttonEnabledNetworks.setEntryValues(entryValues);
        updatePreferenceValueAndSummary(mListPreference, mNetworkMode);
    }

    private void updatePreferenceValueAndSummary(ListPreference preference, int networkMode) {
        Log.d(TAG, "updatePreferenceValueAndSummary, subId=" + mSubId
                + ", networkMode=" + networkMode);
        mNetworkMode = networkMode;
        if (preference == null) {
            Log.d(TAG, "preference is null");
            return;
        }
        switch (networkMode) {
            case TelephonyManager.NETWORK_MODE_LTE_GSM_WCDMA :
            case TelephonyManager.NETWORK_MODE_LTE_WCDMA:
            case TelephonyManager.NETWORK_MODE_LTE_CDMA_EVDO_GSM_WCDMA:
                preference.setValue(Integer.toString(TelephonyManager.NETWORK_MODE_LTE_GSM_WCDMA));
                preference.setSummary(mContext.getText(R.string.gsm_wcdma_lte_mode));
                break;
            case TelephonyManager.NETWORK_MODE_WCDMA_PREF:
            case TelephonyManager.NETWORK_MODE_GSM_UMTS:
            case TelephonyManager.NETWORK_MODE_CDMA_EVDO:
            case TelephonyManager.NETWORK_MODE_EVDO_NO_CDMA:
            case TelephonyManager.NETWORK_MODE_GLOBAL:
                preference.setValue(Integer.toString(TelephonyManager.NETWORK_MODE_WCDMA_PREF));
                preference.setSummary(mContext.getText(R.string.gsm_wcdma_mode));
                break;
            case TelephonyManager.NETWORK_MODE_WCDMA_ONLY:
                preference.setValue(Integer.toString(TelephonyManager.NETWORK_MODE_WCDMA_ONLY));
                preference.setSummary(mContext.getText(R.string.wcdma_only_mode));
                break;
            case TelephonyManager.NETWORK_MODE_GSM_ONLY :
                preference.setValue(Integer.toString(TelephonyManager.NETWORK_MODE_GSM_ONLY));
                preference.setSummary(mContext.getText(R.string.gsm_only_mode));
                break;
            case TelephonyManager.NETWORK_MODE_LTE_ONLY:
                preference.setValue(Integer.toString(TelephonyManager.NETWORK_MODE_LTE_ONLY));
                preference.setSummary(mContext.getText(R.string.lte_only_mode));
                break;
            default:
                preference.setValue(Integer.toString(TelephonyManager.NETWORK_MODE_LTE_GSM_WCDMA));
                preference.setSummary(mContext.getText(R.string.gsm_wcdma_lte_mode));
        }
    }

    /**
     * Updating network mode and summary.
     * @param buttonEnabledNetworks list preference
     * @param networkMode network mode
     * @return false if updated based on condition
     */
    public boolean isNetworkUpdateNeeded(ListPreference buttonEnabledNetworks, int networkMode) {
        Log.d(TAG, "isNetworkUpdateNeeded:" + networkMode);
        int currentMode = getCurrentNetworkMode();
        List<SubscriptionInfo> simList = SubscriptionManager.from(mContext).
                getActiveSubscriptionInfoList();
        if (buttonEnabledNetworks == null) {
            Log.d(TAG, "buttonEnabledNetworks is null");
            return true;
        }
        String ratProtocol2 = SystemProperties.get("persist.vendor.radio.mtk_ps2_rat");
        if (ratProtocol2.equalsIgnoreCase("L/W/G")) {
            return handleRatSettingsForLL(buttonEnabledNetworks, networkMode, currentMode);
        }
        AlertDialog.Builder builder;
        switch (networkMode) {
            case TelephonyManager.NETWORK_MODE_GSM_ONLY :
            case TelephonyManager.NETWORK_MODE_WCDMA_ONLY:
            case TelephonyManager.NETWORK_MODE_WCDMA_PREF:
                if (currentMode < TelephonyManager.NETWORK_MODE_LTE_CDMA_EVDO) {
                    Log.d(TAG, "Current mode is non LTE " + currentMode);
                    return true;
                }
                builder =
                        new AlertDialog.Builder(buttonEnabledNetworks.getContext())
                        .setTitle(mContext.getText(R.string.rjio_network_mode_update_warning_title))
                        .setMessage(getWarningText(networkMode, mSubId))
                        .setPositiveButton(android.R.string.yes,
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int which) {
                                if (simList != null) {
                                    int simCount = simList.size();
                                    for (int index = 0; index < simCount; index ++) {
                                        int subId = simList.get(index).getSubscriptionId();
                                        if (subId == mSubId) {
                                            //Change the network mode to GSM or 3G
                                            updateNetworkMode(buttonEnabledNetworks, mSubId,
                                                    networkMode);
                                            //setSimIdCapabilityforSubId(mSubId, networkMode);
                                            Log.d(TAG, "For yes change mode to Gsm");
                                        } else {
                                            int currentNwMode = getPreferredNetworkMode(subId);
                                            if (isMode2g3g(currentNwMode)) {
                                                updateNetworkMode(buttonEnabledNetworks, subId,
                                                        TelephonyManager.
                                                        NETWORK_MODE_LTE_GSM_WCDMA);
                                                Log.d(TAG, "yes set mode of other to auto");
                                            }
                                        }
                                    }
                                }
                            }
                         })
                        .setNegativeButton(android.R.string.no,
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int which) {
                                // do nothing
                                updatePreferenceValueAndSummary(buttonEnabledNetworks,
                                        currentMode);
                                Log.d(TAG, "For no don't change mode");
                            }
                         });
                builder.show();
                return false;
            case TelephonyManager.NETWORK_MODE_LTE_ONLY :
            case TelephonyManager.NETWORK_MODE_LTE_GSM_WCDMA :
                if ((currentMode >= TelephonyManager.NETWORK_MODE_LTE_CDMA_EVDO) ||
                        isDeviceSingleSim()) {
                    Log.d(TAG, "Current mode is of LTE " + currentMode);
                    return true;
                }
                int othersMode;
                int smCount = simList.size();
                for (int index = 0; index < smCount; index ++) {
                    int subId = simList.get(index).getSubscriptionId();
                    if (subId != mSubId) {
                        othersMode = getPreferredNetworkMode(subId);
                        if (!(othersMode == TelephonyManager.NETWORK_MODE_LTE_ONLY ||
                                othersMode == TelephonyManager.NETWORK_MODE_LTE_GSM_WCDMA)) {
                            Log.d(TAG, "No mode is currently LTE , so no warning or change" +
                                    "of mode needed: Others" + othersMode + "current: " +
                                    currentMode);
                            return true;
                        }
                    }
                }
                Log.d(TAG, "Show alert dialog for MODE_LTE");
                builder =
                        new AlertDialog.Builder(buttonEnabledNetworks.getContext())
                        .setTitle(mContext.getText(R.string.rjio_network_mode_update_warning_title))
                        .setMessage(getWarningText(networkMode, mSubId))
                        .setPositiveButton(android.R.string.yes,
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int which) {
                                if (simList != null) {
                                    int simCount = simList.size();
                                    for (int index = 0; index < simCount; index ++) {
                                        int subId = simList.get(index).getSubscriptionId();
                                        if (subId == mSubId) {
                                            //Set the network mode to Auto mode
                                            updateNetworkMode(buttonEnabledNetworks, subId,
                                                    networkMode);
                                            //setSimIdCapabilityforSubId(subId, networkMode);
                                            Log.d(TAG, "For yes change mode to LTE Only or Auto");
                                        } else {
                                            int currentNwMode = getPreferredNetworkMode(subId);
                                            if (currentNwMode >=
                                                    TelephonyManager.
                                                    NETWORK_MODE_LTE_CDMA_EVDO) {
                                                //Set the network mode of other to GSM only mode
                                                updateNetworkMode(buttonEnabledNetworks, subId,
                                                        TelephonyManager.NETWORK_MODE_GSM_ONLY);
                                                Log.d(TAG, "For yes set mode of other to gsm");
                                            }
                                        }
                                    }
                                }
                            }
                         })
                        .setNegativeButton(android.R.string.no,
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int which) {
                                // do nothing
                                updatePreferenceValueAndSummary(buttonEnabledNetworks,
                                        currentMode);
                                Log.d(TAG, "For no don't change mode");
                            }
                         });
                builder.show();
                return false;
            default:
                updateNetworkMode(buttonEnabledNetworks, mSubId, networkMode);
                //setSimIdCapabilityforSubId(mSubId, networkMode);
                Log.d(TAG, "For yes change mode by default");
        }
        return false;
    }


    private int getPreferredNetworkMode(int subId) {
        return Settings.Global.getInt(mContext.getContentResolver(),
                Settings.Global.PREFERRED_NETWORK_MODE + subId,
                Phone.PREFERRED_NT_MODE);
    }

    private boolean isMode2g3g(int currentNwMode) {
        if (currentNwMode == TelephonyManager.NETWORK_MODE_GSM_ONLY ||
                currentNwMode == TelephonyManager.NETWORK_MODE_WCDMA_ONLY ||
                currentNwMode == TelephonyManager.NETWORK_MODE_WCDMA_PREF) {
                    return true;
                } else {
                    return false;
                }
    }

    /*@Override
    public boolean isEnhancedLTENeedToAdd(boolean defaultValue, int phoneId) {
        if (SystemProperties.getInt("persist.mtk_multiple_ims_support", 1) != 1) {
            int subId = MtkSubscriptionManager.getSubIdUsingPhoneId(phoneId);
            Phone phone = PhoneFactory.getPhone(SubscriptionManager.getPhoneId(subId));
            int primaryPhoneSubId = android.provider.Settings.Global.getInt(
                                phone.getContext().getContentResolver(),
                                com.mediatek.provider.MtkSettingsExt.Global.PRIMARY_SIM, 0);
            if (subId == primaryPhoneSubId) {
                return true;
            }
            return false;
        } else {
            return defaultValue;
        }
    }*/
    private void updateNetworkMode(ListPreference preference, int subId, int settingsMode) {
        Log.d(TAG, "Updating network mode for subId " + subId + "mode " + settingsMode);
        TelephonyManager telephonyManager =
                TelephonyManager.from(mContext).createForSubscriptionId(subId);
        if (telephonyManager.setPreferredNetworkType(subId, settingsMode)) {
            Settings.Global.putInt(mContext.getContentResolver(),
                    Settings.Global.PREFERRED_NETWORK_MODE + subId,
                    settingsMode);
            updatePreferenceValueAndSummary((ListPreference) preference, settingsMode);
        }
    }

    /*private boolean setSimIdCapabilityforSubId(int phoneSubId, int mode) {
        ProxyController proxyController = ProxyController.getInstance();
        List<SubscriptionInfo> simList = SubscriptionManager.from(mContext).
                getActiveSubscriptionInfoList();
        int simCount = simList.size();
        RadioAccessFamily[] rafs = new RadioAccessFamily[simCount];
        int raf;
        int count = SubscriptionManager.from(mContext).getActiveSubscriptionInfoCount();
        Log.d(TAG, "Active Sub Info Count " + count);
        if (count == 1) {
            return true;
        }
        if (mode == TelephonyManager.NETWORK_MODE_GSM_ONLY) {
            for (int index = 0; index < simCount; index ++) {
                int subId = simList.get(index).getSubscriptionId();
                if (subId == phoneSubId) {
                    raf = proxyController.getMinRafSupported();
                    Log.d(TAG, "Set proxyController for GSM Only to GSM");
                } else {
                    raf = proxyController.getMaxRafSupported();
                    Log.d(TAG, "Set proxyController for GSM Only to LTE");
                }
                rafs[index] = new RadioAccessFamily(index, raf);
            }
        } else if (mode == TelephonyManager.NETWORK_MODE_WCDMA_ONLY ||
                        mode == TelephonyManager.NETWORK_MODE_WCDMA_PREF) {
            for (int index = 0; index < simCount; index ++) {
                int subId = simList.get(index).getSubscriptionId();
                if (subId == phoneSubId) {
                    raf = RadioAccessFamily.RAF_UMTS;
                    Log.d(TAG, "Set proxyController to WCDMA");
                } else {
                    raf = proxyController.getMaxRafSupported();
                    Log.d(TAG, "Set proxyController to LTE");
                }
                rafs[index] = new RadioAccessFamily(index, raf);
            }
        } else {
            for (int index = 0; index < simCount; index ++) {
                int subId = simList.get(index).getSubscriptionId();
                if (subId == phoneSubId) {
                    raf = proxyController.getMaxRafSupported();
                    Log.d(TAG, "Set proxyController for LTE to LTE");
                } else {
                    raf = proxyController.getMinRafSupported();
                    Log.d(TAG, "Set proxyController for LTE to GSM");
                }
                rafs[index] = new RadioAccessFamily(index, raf);
            }
        }
        boolean result = proxyController.setRadioCapability(rafs);
        Log.d(TAG, "Result after setting ProxyController: " + result);
        return result;
    }*/

    private boolean isDeviceSingleSim() {
        int count = SubscriptionManager.from(mContext).getActiveSubscriptionInfoCount();
        Log.d(TAG, "Active Sub Info Count " + count);
        if (count == 1) {
            return true;
        }
        return false;
    }
    private CharSequence getWarningText(int mode, int subId) {
        if (isDeviceSingleSim()) {
            return mContext.getText(R.string.rjio_network_mode_update_warning_text);
        }
        int phoneId = SubscriptionManager.getPhoneId(subId);
        Log.d(TAG, "Show warning when mode change to: " + mode +
                " for phone id: " + phoneId);
        if (mode == TelephonyManager.NETWORK_MODE_LTE_ONLY ||
                mode == TelephonyManager.NETWORK_MODE_LTE_GSM_WCDMA) {
            if (phoneId == 0) {
                return mContext.getText(R.string.rjio_network_mode_update_warning_text_sub2);
            } else {
                return mContext.getText(R.string.rjio_network_mode_update_warning_text_sub1);
            }
        } else {
            if (phoneId == 0) {
                return mContext.getText(R.string.rjio_network_mode_update_warning_text_sub1);
            } else {
                return mContext.getText(R.string.rjio_network_mode_update_warning_text_sub2);
            }
        }
    }


    private boolean handleRatSettingsForLL(ListPreference buttonEnabledNetworks,
            int networkMode, int currentMode) {
        AlertDialog.Builder builder;
        switch (networkMode) {
            case TelephonyManager.NETWORK_MODE_GSM_ONLY :
            case TelephonyManager.NETWORK_MODE_WCDMA_ONLY:
            case TelephonyManager.NETWORK_MODE_WCDMA_PREF:
                if (currentMode < TelephonyManager.NETWORK_MODE_LTE_CDMA_EVDO) {
                    Log.d(TAG, "Current mode is non LTE " + currentMode);
                    return true;
                }
                builder =
                        new AlertDialog.Builder(buttonEnabledNetworks.getContext())
                        .setTitle(mContext.getText(R.string.rjio_network_mode_update_warning_title))
                        .setMessage(getWarningText(networkMode, mSubId))
                        .setPositiveButton(android.R.string.yes,
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int which) {
                                updateNetworkMode(buttonEnabledNetworks, mSubId, networkMode);
                            }
                        })
                        .setNegativeButton(android.R.string.no,
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int which) {
                                // do nothing
                                updatePreferenceValueAndSummary(buttonEnabledNetworks,
                                        currentMode);
                                Log.d(TAG, "For no don't change mode");
                            }
                        });
                builder.show();
                return false;
            default:
                return true;
        }
    }
}
