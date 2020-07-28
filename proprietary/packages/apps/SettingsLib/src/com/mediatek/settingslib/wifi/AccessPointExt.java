package com.mediatek.settingslib.wifi;

import android.content.Context;
import android.net.wifi.ScanResult;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiConfiguration.KeyMgmt;
import android.util.Log;

import com.android.settingslib.R;

public class AccessPointExt {
    private static final String TAG = "AccessPointExt";

    /* security type */
    public static final int SECURITY_WAPI_PSK = 9;
    public static final int SECURITY_WAPI_CERT = 10;

    public AccessPointExt(Context context) {
    }

    /**
     * add other security, like as wapi, wep
     * @param config
     * @return
     */
    public static int getSecurity(WifiConfiguration config) {
        /* support wapi psk/cert */
        if (config.allowedKeyManagement.get(KeyMgmt.WAPI_PSK)) {
            return SECURITY_WAPI_PSK;
        }
        if (config.allowedKeyManagement.get(KeyMgmt.WAPI_CERT)) {
            return SECURITY_WAPI_CERT;
        }
        return -1;
    }

    public static int getSecurity(ScanResult result) {
        if (result.capabilities.contains("WAPI-PSK")) {
            /*  WAPI_PSK */
            return SECURITY_WAPI_PSK;
        } else if (result.capabilities.contains("WAPI-CERT")) {
            /* WAPI_CERT */
            return SECURITY_WAPI_CERT;
        }
        return -1;
    }

    public String getSecurityString(int security, Context context) {
        switch(security) {
            case SECURITY_WAPI_PSK:
                /*return WAPI_PSK string */
                return context.getString(R.string.wifi_security_wapi_psk);
            case SECURITY_WAPI_CERT:
                /* return WAPI_CERT string */
                return context.getString(R.string.wifi_security_wapi_certificate);
            default:
        }
        return null;
    }
}
