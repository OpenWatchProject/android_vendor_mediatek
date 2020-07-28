package com.mediatek.op18.settings;


import android.app.Activity;
import android.content.ActivityNotFoundException;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.telecom.TelecomManager;
import android.telephony.TelephonyManager;
import android.util.Log;


/**
 * Adding Call Settings.
 */
public class Op18CallSettings extends Activity {

    private static final String TAG = "Op18CallSettings";


    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        Intent callSettingsIntent = null;
        TelephonyManager telephonyManager =
                    (TelephonyManager) getSystemService(Context.TELEPHONY_SERVICE);
        try {
            if (telephonyManager == null || telephonyManager.getPhoneCount() <= 1) {
                callSettingsIntent =
                       new Intent(TelecomManager.ACTION_SHOW_CALL_SETTINGS);
                Log.d(TAG, "Phone account is 1, so show Call Settings");
            } else {
                callSettingsIntent =
                       new Intent(TelecomManager.ACTION_CHANGE_PHONE_ACCOUNTS);
                Log.d(TAG, "Phone account is more then 1, so show PHONE_ACCOUNTS Settings");
            }
            callSettingsIntent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
            Log.d(TAG, "Launching Call Settings");
            startActivity(callSettingsIntent);
            finish();
        } catch (ActivityNotFoundException e) {
             Log.e(TAG, e.toString());
        }
    }
}
