package com.mediatek.batterywarning;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.util.Log;

public class BatteryWarningReceiver extends BroadcastReceiver {
     // private static final String ACTION_IPO_BOOT = "android.intent.action.ACTION_BOOT_IPO";
    private static final String ACTION_BATTERY_WARNING = "mediatek.intent.action.BATTERY_WARNING";
    private static final String ACTION_BATTERY_NORMAL = "mediatek.intent.action.BATTERY_NORMAL";
    private static final String TAG = "BatteryWarningReceiver";
    private static final String SHARED_PREFERENCES_NAME = "battery_warning_settings";
    private Context mContext;

    // Thermal over temperature
    private static final String ACTION_THERMAL_WARNING = "mediatek.intent.action.THERMAL_DIAG";

    @Override
    public void onReceive(Context context, Intent intent) {
        mContext = context;
        String action = intent.getAction();
        Log.d(TAG, "action = " + action);
        if (Intent.ACTION_BOOT_COMPLETED.equals(action)) {
            Log.d(TAG, action + " clear battery_warning_settings shared preference");
            SharedPreferences.Editor editor = getSharedPreferences().edit();
            editor.clear();
            editor.apply();
        } else if (ACTION_BATTERY_WARNING.equals(action)) {
            Log.d(TAG, action + " start activity according to shared preference");
            int type = intent.getIntExtra("type", -1);
            Log.d(TAG, "type = " + type);
            type = (int) (Math.log(type) / Math.log(2));
            if (type < 0 || type >= BatteryWarningActivity.sWarningTitle.length) {
                return;
            }
            boolean showDialogFlag = getSharedPreferences().getBoolean(
                    Integer.toString(type), true);
            Log.d(TAG, "type = " + type + "showDialogFlag = " + showDialogFlag);
            if (showDialogFlag) {
                Intent activityIntent = new Intent();
                activityIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK
                        | Intent.FLAG_ACTIVITY_CLEAR_TOP
                        | Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS);
                activityIntent.setClass(mContext, BatteryWarningActivity.class);
                activityIntent.putExtra(BatteryWarningActivity.KEY_TYPE, type);
                mContext.startActivity(activityIntent);
            }
        } else if (ACTION_THERMAL_WARNING.equals(action)) {
            int typeValue = intent.getIntExtra(ThermalWarningActivity.KEY_TYPE, -1);
            Log.d(TAG, "typeValue = " + typeValue);
            if (typeValue == 0 || typeValue == 1) {
                Intent thermalIntent = new Intent();
                thermalIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK
                        | Intent.FLAG_ACTIVITY_CLEAR_TOP
                        | Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS);
                thermalIntent.setClass(mContext, ThermalWarningActivity.class);
                thermalIntent.putExtra(ThermalWarningActivity.KEY_TYPE, typeValue);
                mContext.startActivity(thermalIntent);
            }
        } else if (ACTION_BATTERY_NORMAL.equals(action)) {
               int type = intent.getIntExtra("type", -1);
               int type1;
                if(type==0)
                {
                    type1=-1;
                    Intent activityIntent = new Intent();
                    activityIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK
                          | Intent.FLAG_ACTIVITY_CLEAR_TOP
                          | Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS);
                    activityIntent.setClass(mContext, BatteryWarningActivity.class);
                    activityIntent.putExtra(BatteryWarningActivity.KEY_TYPE, type1);
                    mContext.startActivity(activityIntent);
                }
        }
    }

    private SharedPreferences getSharedPreferences() {
        return mContext.getSharedPreferences(SHARED_PREFERENCES_NAME, Context.MODE_PRIVATE);
    }
}
