package com.mediatek.rcse.settings;


import android.content.Context;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;

import com.mediatek.rcse.api.Logger;

/**
 * Settings utils. It defines setting constant; some get settings common method.
 *
 */
public class SettingUtils {
    private final static String TAG = "SettingUtils";

    /**
     * return the current status is roaming or not.
     *
     * @param context context
     * @return true: roaming false: other conditions.
     */
    public static Boolean isRoaming(Context context) {
        Boolean isRoaming = false;
        ConnectivityManager connectivityManager = (ConnectivityManager) context
                .getSystemService(Context.CONNECTIVITY_SERVICE);
        NetworkInfo networkInfo = connectivityManager.getActiveNetworkInfo();
        isRoaming = (networkInfo != null
                && networkInfo.getType() == ConnectivityManager.TYPE_MOBILE && networkInfo
                .isRoaming());
        Logger.d(TAG, "Phone isRoaming " + isRoaming);
        return isRoaming;
    }

}
