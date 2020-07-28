package com.mediatek.op20.settings;

import android.content.Context;
import android.os.PersistableBundle;
import android.telephony.CarrierConfigManager;
import android.util.Log;

import com.mediatek.settings.UtilsExt;
import com.mediatek.settings.ext.DefaultMobileNetworkSettingsExt;

import mediatek.telephony.MtkCarrierConfigManager;

public class Op20MobileNetworkSettingExt extends
        DefaultMobileNetworkSettingsExt {

    private static final String TAG = "Op20MobileNetworkSettingExt";
    private Context mContext;
    CarrierConfigManager mCarrierConfigManager;

    public Op20MobileNetworkSettingExt(Context context) {
        mContext = context;
        mCarrierConfigManager = new CarrierConfigManager(context);
    }

    @Override
    public boolean customizaSpintRoaming(int mSubId) {
        final PersistableBundle carrierConfig = mCarrierConfigManager.getConfigForSubId(mSubId);
        boolean isAvailable = carrierConfig.getBoolean(
                MtkCarrierConfigManager.MTK_KEY_ROAMING_BAR_GUARD_BOOL);
        Log.i(TAG, "getAvailabilityStatus, subId=" + mSubId
                + ", isAvailable=" + isAvailable);
        return isAvailable;
    }
}