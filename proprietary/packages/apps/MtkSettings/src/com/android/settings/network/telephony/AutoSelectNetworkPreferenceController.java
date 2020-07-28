package com.android.settings.network.telephony;

import android.content.Context;
import android.content.IntentFilter;
import android.os.SystemProperties;
import android.util.Log;

import androidx.preference.Preference;

import com.mediatek.settings.UtilsExt;

public class AutoSelectNetworkPreferenceController extends
        TelephonyBasePreferenceController {

    private static final String TAG = "AutoSelectNetworkPreferenceController";

    public AutoSelectNetworkPreferenceController(Context context, String key) {
        super(context, key);
    }

    /**
     *  M: for China mobile feature,user can not select network type
     *  show 4/3/2g auto grey item
     */
    @Override
    public int getAvailabilityStatus(int subId) {
        boolean visible = UtilsExt.getMobileNetworkSettingsExt(mContext)
                    .customizeAutoSelectNetworkTypePreference();
        boolean cmccOpenMarketvisible
            = SystemProperties.get("ro.vendor.cmcc_light_cust_support").equals("1");
        Log.i(TAG, "getAvailabilityStatus visible = " + visible
                + " cmccOpenMarketvisible = " + cmccOpenMarketvisible);
        visible = visible || cmccOpenMarketvisible;
        return visible ? AVAILABLE : CONDITIONALLY_UNAVAILABLE;
    }

    public void init(int subId) {
        mSubId = subId;
    }

    @Override
    public void updateState(Preference preference) {
        preference.setEnabled(false);
    }
}
