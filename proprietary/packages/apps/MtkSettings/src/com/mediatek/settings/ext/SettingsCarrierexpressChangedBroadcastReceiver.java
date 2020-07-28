package com.mediatek.settings.ext;

import com.mediatek.settings.UtilsExt;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

public class SettingsCarrierexpressChangedBroadcastReceiver extends
        BroadcastReceiver {

    private static final String TAG = "SettingsCarrierexpressChangedBroadcastReceiver";
    private String CARRIER_EXPRESS_CHANGED = "com.mediatek.common.carrierexpress.operator_config_changed";
    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        Log.i(TAG, "action = " + action);
        if (null != action && CARRIER_EXPRESS_CHANGED.equals(action)) {
            OpSettingsCustomizationUtils.setOPSettings();
            UtilsExt.resetSettingExt();
        }
    }
}
