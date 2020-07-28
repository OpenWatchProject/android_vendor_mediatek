package com.mediatek.mail.utils;

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.NetworkInfo.DetailedState;

public class Utility {

    /**
     * M:Check if device has a network connection (wifi or data)
     * @param context
     * @return true if network connected
     */
    public static boolean hasConnectivity(Context context) {
        ConnectivityManager cm = (ConnectivityManager) context
                .getSystemService(Context.CONNECTIVITY_SERVICE);

        NetworkInfo info = cm.getActiveNetworkInfo();
        if (info != null && info.isConnected()) {
            DetailedState state = info.getDetailedState();
            if (state == DetailedState.CONNECTED) {
                return true;
            }
        }
        return false;
    }
}
