package com.mediatek.carrierexpress;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.SystemProperties;
import android.telephony.TelephonyManager;
import android.util.Log;
import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.PhoneConstants;

public class CarrierExpressReceiver extends BroadcastReceiver {
    private final String TAG = "CarrierExpressReceiver";

    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        Log.d(TAG, "onReceive(), action=" + action);

        if ("no".equals(SystemProperties.get("ro.vendor.mtk_carrierexpress_pack", "no"))) {
            Log.i(TAG, "Carrier Express not supported !");
            return;
        }

        // Try to start CXP service process when:
        // - LOCKED_BOOT_COMPLETED, BOOT_COMPLETED
        // - SIM_STATE_CHANGED, SIM_APPLICATION_STATE_CHANGED
        if (Intent.ACTION_LOCKED_BOOT_COMPLETED.equals(action)||
            Intent.ACTION_BOOT_COMPLETED.equals(action)||
            Intent.ACTION_SIM_STATE_CHANGED.equals(action) ||
            TelephonyManager.ACTION_SIM_APPLICATION_STATE_CHANGED.equals(action)) {
            intent.setClass(context, CarrierExpressApp.class);
            context.startService(intent);
        }
    }
}

