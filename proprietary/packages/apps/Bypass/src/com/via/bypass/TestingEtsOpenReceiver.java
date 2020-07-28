package com.via.bypass;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.SystemProperties;
import android.util.Log;

/**
 * Receiver to handle secret code.
 */
public class TestingEtsOpenReceiver extends BroadcastReceiver {

    /**
     * Constructor.
     */
    public TestingEtsOpenReceiver() {
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        Log.i("TestingEtsOpenReceiver", "onReceive() Action:" + intent.getAction());
        if (intent.getAction().equals("android.provider.Telephony.SECRET_CODE")
                && "1".equals(SystemProperties.get("ro.boot.opt_c2k_lte_mode"))) {
            Intent stintent = new Intent("com.via.bypass.setting");
            stintent.setClass(context, BypassSettings.class);
            stintent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            context.startActivity(stintent);
        }
    }
}
