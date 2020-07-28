package com.mediatek.op18.settings;

import android.content.Context;
import android.util.Log;


import com.mediatek.op18.settings.R;
import com.mediatek.settings.ext.DefaultSettingsMiscExt;
import com.mediatek.telephony.MtkTelephonyManagerEx;

/**
 * Device status info plugin.
 */
public class Op18SettingsMiscExt extends DefaultSettingsMiscExt {
    private static final String TAG = "Op18SettingsMiscEx";

    private Context mContext;

    /**
     * Init context.
     * @param context The Context
     */
    public Op18SettingsMiscExt(Context context) {
        super(context);
        mContext = context;
    }

    /**
     * @param defaultString The Default String
     * @param subId SubscriptionId
     * @return Custom String
     */
    @Override
    public String getNetworktypeString(String defaultString, int subId) {
        Boolean volteEnabled = MtkTelephonyManagerEx.getDefault().isVolteEnabled(subId);
        Boolean wifiCallingEnabled = MtkTelephonyManagerEx.getDefault().isWifiCallingEnabled(subId);
        if (defaultString != null &&
                (defaultString.equals("LTE") || defaultString.equals("4G")) && volteEnabled) {
            Log.d(TAG, "@M_getNetworktypeString volteEnabled = " + volteEnabled);
            Log.d(TAG, "@M_getNetworktypeString OP18 defaultString = " +
                    defaultString + "return" + mContext.getString(R.string.lte_string));
            return mContext.getString(R.string.lte_string);
        } else if (defaultString != null && (defaultString.equals("LTE") ||
                defaultString.equals("4G")) && wifiCallingEnabled) {
            Log.d(TAG, "@M_getNetworktypeString wifiCallingEnabled =" + wifiCallingEnabled);
            Log.d(TAG, "@M_getNetworktypeString OP18 defaultString = " +
                    defaultString + "return" + mContext.getString(R.string.lte_wifi_string));
            return mContext.getString(R.string.lte_wifi_string);
        } else {
            Log.d(TAG, "@M_getNetworktypeString OP18 defaultString = " +
                    defaultString + "return" + defaultString);
            return defaultString;
        }
    }
}
