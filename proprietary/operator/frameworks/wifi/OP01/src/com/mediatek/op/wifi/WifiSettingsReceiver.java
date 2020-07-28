package com.mediatek.op.wifi;

import java.util.List;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager.NameNotFoundException;
import android.database.Cursor;
import android.net.NetworkInfo;
import android.net.Uri;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.net.wifi.WifiConfiguration.KeyMgmt;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.provider.Settings;
import android.telephony.TelephonyManager;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.util.Log;

import com.android.internal.telephony.ITelephony;
import com.mediatek.telephony.MtkTelephonyManagerEx;

import com.mediatek.provider.MtkSettingsExt;

/**
 * For CMCC AP afer change SIM.
 */
public class WifiSettingsReceiver extends BroadcastReceiver {

    private final static String TAG = "WifiSettingsReceiver";
    static final String CMCC_SSID = "CMCC";
    static final String CMCC_AUTO_SSID = "CMCC-AUTO";
    private static final String PREF_REMIND_HOTSPOT = "pref_remind_hotspot";
    private static final String PREF_REMIND_CONNECT = "pref_remind_connect";
    private static final String WIFI_DIALOG_CONTENT= "content://com.mediatek.cmcc.wifidiaog";

    // constant for current sim mode
    private static final int INVALID_NETWORK_ID = -1;

    // remind user if connect to access point
    private static final int WIFI_CONNECT_REMINDER_ALWAYS = 0;

    /* to mark if the tcard is insert, set to true only the time SD inserted */
    private WifiManager mWifiManager;
    private TelephonyManager mTm;
    private static boolean sHasDisconnect = true;

    private int mAutoConnect;
    private Context mContext;
    @Override
    public void onReceive(Context context, Intent intent) {
        mContext = context;
        String action = intent.getAction();
        Log.i(TAG, "onReceive() action = " + action);
        mWifiManager = (WifiManager) context
                .getSystemService(Context.WIFI_SERVICE);

        if (WifiManager.NETWORK_STATE_CHANGED_ACTION.equals(action)) {
            NetworkInfo netInfo = (NetworkInfo) intent
                    .getParcelableExtra(WifiManager.EXTRA_NETWORK_INFO);
            NetworkInfo.DetailedState dState = netInfo.getDetailedState();

            if (dState == NetworkInfo.DetailedState.DISCONNECTED) {
                sHasDisconnect = true;
                Log.i(TAG, "wifi disconnected state");
            } else if (sHasDisconnect
                    && dState == NetworkInfo.DetailedState.CONNECTED) {
                sHasDisconnect = false;
                WifiInfo wifiInfo = mWifiManager.getConnectionInfo();
                ;
                Log.i(TAG, "wifi connected state");
                if (wifiInfo != null) {
                    Log.i(TAG, "wifi info");
                    int networkId = wifiInfo.getNetworkId();
                    Log.i(TAG, "networkId = " + networkId);
                    if (networkId == INVALID_NETWORK_ID) {
                        Log.i(TAG, "can't get networkId");
                        return;
                    }

                    String wifissid = wifiInfo.getSSID();
                    Log.i(TAG, "wifissid = " + wifissid);
                    String ssid = removeDoubleQuotes(wifiInfo.getSSID());
                    Log.i(TAG, "ssid = " + ssid);

                    if (CMCC_AUTO_SSID.equals(ssid) || CMCC_SSID.equals(ssid)) {
                        WifiManager wifiManager = (WifiManager) context
                                .getSystemService(Context.WIFI_SERVICE);
                        final List<WifiConfiguration> configs = wifiManager
                                .getConfiguredNetworks();
                        if (configs == null) {
                            Log.i(TAG, "configs =null");

                        }
                        boolean isCMCC = false;
                        if (configs != null) {
                            for (WifiConfiguration config : configs) {
                                int cnetworkid = config.networkId;
                                boolean eap = config.allowedKeyManagement
                                        .get(KeyMgmt.WPA_EAP);
                                boolean ieee = config.allowedKeyManagement
                                        .get(KeyMgmt.IEEE8021X);
                                Log.i(TAG, "cnetworkid = " + cnetworkid
                                        + "  eap = " + eap + "   ieee = "
                                        + ieee);
                                if (config != null
                                        && networkId == config.networkId
                                        && (config.allowedKeyManagement
                                                .get(KeyMgmt.WPA_EAP) || config.allowedKeyManagement
                                                .get(KeyMgmt.IEEE8021X))) {
                                    isCMCC = true;
                                    break;
                                }
                            }
                        }

                        if (!isCMCC) {
                            return;
                        }
                        int value = Settings.System.getInt(
                                context.getContentResolver(),
                                MtkSettingsExt.System.WIFI_CONNECT_REMINDER,
                                WIFI_CONNECT_REMINDER_ALWAYS);
                        Log.i(TAG, "wifi connect remind value = " + value);
                        if (value != WIFI_CONNECT_REMINDER_ALWAYS) {
                            return;
                        }
                        String[] projection = {"remind_connect_value"};
                        String selection = "remind_connect_key=?";
                        String[] selectionArgs = {"wificonnectnotifydialog_remind"};
                        Cursor cursor = context.getContentResolver().query(Uri.parse(WIFI_DIALOG_CONTENT),
                                projection , selection , selectionArgs , null);
                        int dialogNotShow = -2;
                        if (null == cursor) {
                            Log.i(TAG, "cursor == null start dialog");
                            startWifiConnectNotifyDialog(context);
                        } else {
                            if (!cursor.moveToNext()) {
                                Log.i(TAG, "cursor != null but cursor.moveToNext = false start dialog");
                                startWifiConnectNotifyDialog(context);
                                cursor.close();
                            } else {
                              /// dialogshow is 2, means do not show dialog again.
                                dialogNotShow = cursor.getInt(0);
                                Log.i(TAG, "cursor move to 0 next dialogNotShow = " + dialogNotShow);
                                if (2 == dialogNotShow) {
                                    cursor.close();
                                    return;
                                }
                            }
                        }
                    }
                }
            }
            return;
        }

        mTm = (TelephonyManager) context
                .getSystemService(Context.TELEPHONY_SERVICE);

        if (WifiManager.WIFI_AP_STATE_CHANGED_ACTION.equals(action)) {
            int state = intent.getIntExtra(WifiManager.EXTRA_WIFI_AP_STATE,
                    WifiManager.WIFI_AP_STATE_FAILED);
            Log.i(TAG, "state = " + state);
            if (state == WifiManager.WIFI_AP_STATE_ENABLED) {
                boolean isDataOn = isDataCanUsed();
                Log.i(TAG, "isDataOn = " + isDataOn);
                if (!isDataOn) {
                    String[] projection = {"remind_hotspot_value"};
                    String selection = "remind_hotspot_key=?";
                    String[] selectionArgs = {"wifihotspotnotifydialog_remind"};
                    Cursor cursor = context.getContentResolver().query(Uri.parse(WIFI_DIALOG_CONTENT),
                            projection , selection , selectionArgs , null);
                    int dialogNotShow = -2;
                    if (null == cursor) {
                        Log.i(TAG, "cursor == null start WifiHotspotNotifyDialog");
                        startWifiHotspotNotifyDialog(context);
                    } else {
                        if (!cursor.moveToNext()) {
                            Log.i(TAG, "cursor != null but cursor.moveToNext()= false start WifiHotspotNotifyDialog");
                            startWifiHotspotNotifyDialog(context);
                            cursor.close();
                        } else {
                            /// dialogshow is 2, means do not show dialog again.
                            dialogNotShow = cursor.getInt(0);
                            Log.i(TAG, "cursor move to 0 next do not startWifiHotspotNotifyDialog dialogNotShow = " + dialogNotShow);
                            if (2 == dialogNotShow) {
                                cursor.close();
                                return;
                            } 
                        }
                    }
                }
            }
            return;
        }

        mAutoConnect = Settings.System.getInt(context.getContentResolver(),
                MtkSettingsExt.System.WIFI_CONNECT_TYPE,
                MtkSettingsExt.System.WIFI_CONNECT_TYPE_AUTO);
        final int wifiState = mWifiManager.getWifiState();
        Log.i(TAG, "onReceive() wifiState = " + wifiState);
    }

    private boolean isDataCanUsed() {
        List<SubscriptionInfo> si = SubscriptionManager.from(mContext)
                .getActiveSubscriptionInfoList();
        if (si == null) {
            Log.d(TAG, "si == null,return false");
            return false;
        }
        Log.i(TAG, "isSimOnOffEnabled() = " + isSimOnOffEnabled());
        if (isSimOnOffEnabled()) {
            ///sim on off feature
            boolean isAllSimOn = false;
            for (int i = 0; i < si.size(); i++) {
                int subId = si.get(i).getSubscriptionId();
                int phoneId = SubscriptionManager.getPhoneId(subId);
                boolean isSimon = getSimOnOffState(phoneId)
                        == MtkTelephonyManagerEx.SIM_POWER_STATE_SIM_ON;
                isAllSimOn = isAllSimOn || isSimon;
            }
            if (!isAllSimOn) {
                Log.d(TAG, "is All sim Off, return false");
                return false;
            }
        }
        boolean isAllradioOn = false;
        for (int i = 0; i < si.size(); i++) {
           int subId = si.get(i).getSubscriptionId();
           boolean isRadioOn = isRadioOn(subId, mContext);
           isAllradioOn = isAllradioOn || isRadioOn;
        }
        if (!isAllradioOn) {
            Log.d(TAG, "is All radio Off, return false");
            return false;
        }
        for (int i = 0; i < si.size(); i++) {
            SubscriptionInfo subInfo = si.get(i);
            int subId = subInfo.getSubscriptionId();
            int phoneId = SubscriptionManager.getPhoneId(subId);
            int defaultDataSubId = SubscriptionManager.getDefaultDataSubscriptionId();
            Log.i(TAG, "subId = " + subId + " phoneId = " + phoneId
                  + " defaultDataSubId = " + defaultDataSubId);
            if (subId == defaultDataSubId) {
                boolean dataEnable = mTm.getDataEnabled(subId);
                Log.i(TAG, "subId = " + subId + " phoneId = " + phoneId
                        + " defaultDataSubId = " + defaultDataSubId
                        + " dataEnable = " + dataEnable);
                if (dataEnable) {
                    return true;
                }
            }
         }
        return false;
    }

    private boolean isRadioOn(int subId, Context context) {
        Log.i(TAG, "[isRadioOn]subId:" + subId);
        boolean isRadioOn = false;
        final ITelephony iTel = ITelephony.Stub.asInterface(
                ServiceManager.getService(Context.TELEPHONY_SERVICE));
        if (iTel != null) {
            try {
                isRadioOn = iTel.isRadioOnForSubscriber(subId, mContext.getPackageName());
            } catch (RemoteException e) {
                Log.i(TAG, "[isRadioOn] failed to get radio state for sub " + subId);
                isRadioOn = false;
            }
        } else {
            Log.i(TAG, "[isRadioOn]failed to check radio");
        }
        Log.i(TAG, "[isRadioOn]isRadioOn:" + isRadioOn);

        return isRadioOn;
    }

    private void startWifiHotspotNotifyDialog(Context context) {
        Intent start = new Intent("com.mediatek.OP01.WifiHotspotNotifyDialog");
        start.setComponent(new ComponentName("com.mediatek.server.wifi.op01",
        "com.mediatek.op.wifi.WifiHotspotNotifyDialog"));
        start.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        context.startActivity(start);
    }

    private void startWifiConnectNotifyDialog(Context context) {
        Intent start = new Intent("com.mediatek.OP01.WifiConnectNotifyDialog");
        start.setComponent(new ComponentName("com.mediatek.server.wifi.op01",
                "com.mediatek.op.wifi.WifiConnectNotifyDialog"));
        start.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        context.startActivity(start);
    }

    private String removeDoubleQuotes(String string) {
        if (string != null) {
            int length = string.length();
            if ((length > 1) && (string.charAt(0) == '"')
                    && (string.charAt(length - 1) == '"')) {
                return string.substring(1, length - 1);
            }
        }
        return string;
    }

    /// M: Add for SIM On/Off feature. @{
    public static boolean isSimOnOffEnabled() {
        return MtkTelephonyManagerEx.getDefault().isSimOnOffEnabled();
    }

    public static int getSimOnOffState(int slotId) {
        return MtkTelephonyManagerEx.getDefault().getSimOnOffState(slotId);
    }
    /// @}
}
