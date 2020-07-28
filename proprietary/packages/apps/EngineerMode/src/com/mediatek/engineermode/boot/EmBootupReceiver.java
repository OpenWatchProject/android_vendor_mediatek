package com.mediatek.engineermode.boot;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.bluetooth.BtTestTool;
import com.mediatek.engineermode.bluetooth.BtWatchService;
import com.mediatek.engineermode.bypass.BypassService;
import com.mediatek.engineermode.bypass.BypassSettings;
import com.mediatek.engineermode.wifi.MtkCTIATestDialog;
import com.mediatek.engineermode.wifi.WifiLogSwitchActivity;
import com.mediatek.engineermode.wifi.WifiWatchService;

/**
 * The broadcast receiver of device boot complete.
 */
public class EmBootupReceiver extends BroadcastReceiver {

    private static final String TAG = "BootupReceiver";
    private static final String MODEM_FILTER_SHAREPRE = "telephony_modem_filter_settings";


    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        if (Intent.ACTION_BOOT_COMPLETED.equals(action)) {
            onBootupCompleted(context, intent);
        }
    }

    private void onBootupCompleted(Context context, Intent intent) {
        Elog.d(TAG, "Start onBootupCompleted");

        //MD EM filter
        writeSharedPreference(context, false);

        //ShortVolteInterrupt EM disabled
        writeShortVolteInterruptPreference(context);

        tryInvokeBypassService(context);

        if (MtkCTIATestDialog.isWifiCtiaEnabled(context)) {
            WifiWatchService.enableService(context, true);
        }

        WifiLogSwitchActivity.onWifiStateChanged(context);

        if (BtTestTool.getAlwaysVisible(context)) {
            BtTestTool.setAlwaysVisible(true);
            BtWatchService.enableService(context, true);
        }
        Elog.d(TAG, "End onBootupCompleted");
    }

    private void tryInvokeBypassService(Context context) {
        SharedPreferences settings = context
            .getSharedPreferences(BypassSettings.PREF_SERV_ENABLE, 0);
        boolean prefServEnable = settings.getBoolean(BypassSettings.PREF_SERV_ENABLE, false);
        Elog.w(TAG, "prefServEnable : " + prefServEnable);

        if (prefServEnable) {
            Intent servIntent = new Intent(context, BypassService.class);
            Elog.w(TAG, "ready to start BypassService");
            context.startService(servIntent);
        }
    }

    private void writeSharedPreference(Context context, boolean flag) {
        final SharedPreferences modemFilterSh = context.getSharedPreferences(
                         MODEM_FILTER_SHAREPRE, android.content.Context.MODE_PRIVATE);
        SharedPreferences.Editor editor = modemFilterSh.edit();
        editor.putBoolean(context.getString(R.string.enable_md_filter_sim1), flag);
        editor.putBoolean(context.getString(R.string.enable_md_filter_sim2), flag);
        editor.commit();
    }

    private void writeShortVolteInterruptPreference(Context context) {
        Elog.d(TAG, "writeShortVolteInterruptPreference false");
        SharedPreferences mPref = context.getSharedPreferences("GwsdConfigure",
                Context.MODE_PRIVATE);
        SharedPreferences.Editor mPrefEditor = mPref.edit();
        mPrefEditor.putBoolean("gwsd_dual_capability", false);
        mPrefEditor.commit();
    }

}
