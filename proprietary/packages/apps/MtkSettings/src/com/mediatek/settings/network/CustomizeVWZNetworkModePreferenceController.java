package com.mediatek.settings.network;

import android.content.Context;
import android.os.PersistableBundle;
import android.os.SystemProperties;
import android.provider.Settings;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;

import androidx.preference.ListPreference;
import androidx.preference.Preference;

import com.android.internal.telephony.Phone;
import com.android.settings.network.telephony.TelephonyBasePreferenceController;
import com.android.settings.R;

import com.mediatek.internal.telephony.ratconfiguration.RatConfiguration;
import com.mediatek.settings.UtilsExt;
import com.mediatek.telephony.MtkTelephonyManagerEx;

public class CustomizeVWZNetworkModePreferenceController
        extends TelephonyBasePreferenceController
        implements ListPreference.OnPreferenceChangeListener {
    private static final String TAG = "CustomizeVWZNetworkModePreferenceController";
    private TelephonyManager mTelephonyManager;
    private int mNetworkMode;
    private int mPhoneId;
    private static final String PROP_VZW_DEVICE_TYPE = "persist.vendor.vzw_device_type";

    private ListPreference mListPreference;

    public CustomizeVWZNetworkModePreferenceController(Context context, String key) {
        super(context, key);
    }

    @Override
    public int getAvailabilityStatus(int subId) {
        boolean visible;
        if (subId == SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
            Log.d(TAG, "getAvailabilityStatus, subId is invalid.");
            visible = false;
        } else if ("3".equals(SystemProperties.get(PROP_VZW_DEVICE_TYPE, "0"))) {
            Log.d(TAG, "getAvailabilityStatus, PROP_VZW_DEVICE_TYPE is 3");
            visible = false;
        } else {
            visible = UtilsExt.getMobileNetworkSettingsExt(mContext)
                    .customizeVWZNetworkModePreference(subId);
        }
        return visible ? AVAILABLE : CONDITIONALLY_UNAVAILABLE;
    }

    public void init(int subId) {
        mSubId = subId;
        mPhoneId = SubscriptionManager.getSlotIndex(subId);
        mTelephonyManager = TelephonyManager.from(mContext).createForSubscriptionId(mSubId);
    }

    @Override
    public void updateState(Preference preference) {
        super.updateState(preference);
        mListPreference = (ListPreference) preference;
        if (isMtkLCSupport()) {
            CharSequence entries[] = mContext.getResources().getTextArray(
                    R.array.network_mode_options);
            CharSequence entryValues[] = mContext.getResources().getTextArray(
                    R.array.network_mode_options_values);
            mListPreference.setEntries(entries);
            mListPreference.setEntryValues(entryValues);
          //  mListPreference.setValueIndex(0);
        } else if (isCDMALessSupport()) {
            if ("4".equals(SystemProperties.get(PROP_VZW_DEVICE_TYPE, "0"))) {
                CharSequence entries[] = mContext.getResources().getTextArray(
                    R.array.cdma_less_network_mode_options);
                CharSequence entryValues[] = mContext.getResources().getTextArray(
                        R.array.cdma_less_network_mode_options_values);
                mListPreference.setEntries(entries);
                mListPreference.setEntryValues(entryValues);
             //   mListPreference.setValueIndex(0);
            } 
        }
        mNetworkMode = getPreferredNetworkMode();
        updatePreferenceValueAndSummary(mListPreference, mNetworkMode);
    }

    private void updatePreferenceValueAndSummary(ListPreference preference, int networkMode) {
        Log.d(TAG, "updatePreferenceValueAndSummary, subId=" + mSubId
                + ", networkMode=" + networkMode);
        mNetworkMode = networkMode;
        switch (networkMode) {
            case TelephonyManager.NETWORK_MODE_LTE_CDMA_EVDO:
                preference.setValue(Integer.toString(TelephonyManager.NETWORK_MODE_LTE_CDMA_EVDO));
                preference.setSummary(mContext.getText(R.string.global_mode));
                break;
            case TelephonyManager.NETWORK_MODE_LTE_ONLY:
                preference.setValue(Integer.toString(TelephonyManager.NETWORK_MODE_LTE_ONLY));
                preference.setSummary(mContext.getText(R.string.lte_only_mode));
                break;
            case TelephonyManager.NETWORK_MODE_WCDMA_PREF :
                preference.setValue(Integer.toString(TelephonyManager.NETWORK_MODE_WCDMA_PREF));
                preference.setSummary(mContext.getText(R.string.gsm_umts_mode));
                break;
            case TelephonyManager.NETWORK_MODE_LTE_GSM_WCDMA :
                preference.setValue(Integer.toString(TelephonyManager.NETWORK_MODE_LTE_GSM_WCDMA));
                preference.setSummary(mContext.getText(
                        R.string.global_lte_gsm_umts_mode));
                break;
            default:
                preference.setSummary(
                    mContext.getString(R.string.mobile_network_mode_error, networkMode));
            }
    }

    private int getPreferredNetworkMode() {
        int mode = Settings.Global.getInt(mContext.getContentResolver(),
                Settings.Global.PREFERRED_NETWORK_MODE + mSubId,
                Phone.PREFERRED_NT_MODE);
        Log.i(TAG, "getPreferredNetworkMode mode = " + mode);
        return mode;
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object object) {
        final int settingsMode = Integer.parseInt((String) object);
        Settings.Global.putInt(mContext.getContentResolver(),
                Settings.Global.PREFERRED_NETWORK_MODE + mSubId,
                settingsMode);
        updatePreferenceValueAndSummary((ListPreference) preference, settingsMode);
        return true;
    }
    /**
     * C2k LCSupport (C/Lf).
     * @return true if c2k LC supported.
     */
    public boolean isMtkLCSupport() {
        boolean isSupport = RatConfiguration.isC2kSupported() &&
                RatConfiguration.isLteFddSupported() &&
                !RatConfiguration.isGsmSupported() &&
                !RatConfiguration.isWcdmaSupported() &&
                !RatConfiguration.isTdscdmaSupported();
        boolean lcSupport = false;
        if ((SystemProperties.get("ro.vendor.mtk_protocol1_rat_config")).equals("C/Lf")) {
            lcSupport = true;
        }
        Log.d(TAG, "isMtkLCSupport(): " + isSupport);
        return (isSupport && lcSupport);
    }

    /**
     * CDMA less support Device(Lf/W/G).
     * @return true if CDMA less supported.
     */
    public boolean isCDMALessSupport() {
        boolean isCDMALessSupport = false;
        if ("3".equals(SystemProperties.get(PROP_VZW_DEVICE_TYPE, "0")) ||
            "4".equals(SystemProperties.get(PROP_VZW_DEVICE_TYPE, "0"))) {
            isCDMALessSupport = true;
        }
        return isCDMALessSupport;
    }
}
