package com.mediatek.op07.settings;

import android.content.Context;
import android.telephony.SubscriptionManager;
import android.util.Log;
import androidx.preference.SwitchPreference;

import com.android.ims.ImsManager;

import com.mediatek.settings.ext.DefaultMobileNetworkSettingsExt;

public class OP07MobileNetworkSettingsExt extends
        DefaultMobileNetworkSettingsExt {
    private static final String TAG = "OP07MobileNetworkSettingsExt";
    private Context mContext;

    public OP07MobileNetworkSettingsExt(Context context) {
        mContext = context;
    }

    @Override
    public boolean customizeATTNetworkModePreference(int subId) {
        Log.i(TAG, "customizeATTNetworkModePreference return true");
        return true;
    }

    @Override
    public void customizaMobileDataSummary(Object object, int subId) {
        SwitchPreference mobileDataPref = (SwitchPreference) object;
        int phoneId = SubscriptionManager.getPhoneId(subId);
        boolean isVtEnabled = ImsManager.getInstance(mContext, phoneId)
                .isVtEnabledByPlatform();
        Log.i(TAG, "customizaMobileDataSummary set MobileDataSummary"
                + " subId = " + subId
                + " phoneId = " + phoneId
                + " isVtEnabled = " + isVtEnabled);
        String title = mContext.getString(R.string.mobile_data_att);
        String summary;
        if (isVtEnabled) {
            summary = mContext
                    .getString(R.string.mobile_data_summary_when_vilte_on_att);
        } else {
            summary = mContext.getString(R.string.mobile_data_summary_default_att);
        }
        if (null != mobileDataPref) {
            mobileDataPref.setTitle(title);
            mobileDataPref.setSummary(summary);
            Log.d(TAG, "change mobile data title=" + title + ", summary="
                    + summary);
        } else {
            Log.d(TAG, "customizaMobileDataSummary null == mobileDataPref");
        }
    }

    @Override
    public void customizaRoamingPreference(Object object, int subId) {
        SwitchPreference dataRoamingPref = (SwitchPreference)object;
        int phoneId = SubscriptionManager.getPhoneId(subId);
        boolean isVtEnabled = ImsManager.getInstance(mContext, phoneId)
                .isVtEnabledByPlatform();
        Log.i(TAG, "customizaRoamingPreference set RoamingPreference"
                + " subId = " + subId
                + " phoneId = " + phoneId
                + " isVtEnabled = " + isVtEnabled);
        if (dataRoamingPref != null) {
            String title = mContext.getString(R.string.data_roaming_title_att);
            String summary;
            if (isVtEnabled) {
                summary = mContext
                        .getString(R.string.data_roaming_warning_vilte_capable_att);
            } else {
                summary = mContext.getString(R.string.data_roaming_warning_att);
            }
            dataRoamingPref.setTitle(title);
            dataRoamingPref.setSummary(summary);
            dataRoamingPref.setSummaryOn(summary);
            dataRoamingPref.setSummaryOff(summary);
            Log.d(TAG, "change data roaming title=" + title + ", summary="
                    + summary);
        } else {
            Log.d(TAG, "customizaRoamingPreference null == dataRoamingPref");
        }
    }

    @Override
    public void customizaEnhanced4gLTEPreference(Object object, int subId) {
        SwitchPreference enhanced4gLTEPref = (SwitchPreference)object;
        if (enhanced4gLTEPref != null) {
            Log.i(TAG, "customizaEnhanced4gLTEPreference subId = " + subId);
            String title = mContext.getString(R.string.volte_switch_title_att);
            String summary = mContext.getString(R.string.volte_switch_summary_att);
            enhanced4gLTEPref.setTitle(title);
            enhanced4gLTEPref.setSummary(summary);
            Log.d(TAG, "customizeEnhanced4GLteSwitchPreference,"
                    + " subId = " + subId
            		+ " title = " + title
            		+ " summary=" + summary);
        } else {
            Log.d(TAG, "customizeEnhanced4GLteSwitchPreference enhanced4gLTEPref = null");
        }
    }

    @Override
    public String customizaRoamingPreferenceDialogSummary(String summary,
            int subId) {
        int phoneId = SubscriptionManager.getPhoneId(subId);
        boolean isVtEnabled = ImsManager.getInstance(mContext, phoneId)
                .isVtEnabledByPlatform();

        Log.d(TAG, "customizeDataRoamingAlertDialog, subId=" + subId
                + ", phoneId=" + phoneId + ", isVtEnabled=" + isVtEnabled);

        String customizeSummary = summary;
        if (isVtEnabled) {
            customizeSummary = mContext.getString(
                    R.string.data_roaming_warning_vilte_capable_att);
        } else {
            customizeSummary = mContext.getString(R.string.data_roaming_warning_att);
        }
        Log.d(TAG, "dataRoamingAlert customizeSummary = " + customizeSummary);
        return customizeSummary;
    }

    @Override
    public boolean customizeATTDisable2Gvisible(int subId) {
        Log.i(TAG, "customizeATTDisable2Gvisible return true");
        return true;
    }

    @Override
    public boolean customizeATTManualFemtoCellSelectionPreference(int subId) {
        Log.i(TAG, "customizeATTManualFemtoCellSelectionPreference return true");
        return true;
    }
}