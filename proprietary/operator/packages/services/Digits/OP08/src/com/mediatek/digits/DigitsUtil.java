package com.mediatek.digits;

import android.content.Context;

import android.telephony.TelephonyManager;
import android.telephony.SubscriptionManager;
import android.util.Log;

import com.mediatek.digits.DigitsSharedPreference;
import com.mediatek.digits.utils.Utils;

public class DigitsUtil {
    static protected final String TAG = "DigitsUtil";

    private static DigitsUtil sDigitsUtil;
    private Context mContext;

    TelephonyManager mTelephonyManager;

    private String mImei;

    private String mDeviceId;
    // Private constructor
    private DigitsUtil(Context context) {
        mContext = context;

        mTelephonyManager = (TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE);
    }

    public static DigitsUtil getInstance(Context context) {

        if (sDigitsUtil != null) {
            return sDigitsUtil;
        }

        DigitsUtil util = new DigitsUtil(context);
        sDigitsUtil = util;

        return util;
    }

    public String getImei() {
        if (mImei != null) {
            return mImei;
        }

        String mImei = mTelephonyManager.getImei();

        return mImei;
    }

    public String getDeviceId() {
        if (mDeviceId != null) {
            return mDeviceId;
        }

        // Use getImei() for all cases. getDeviceId() returns "00000000" if no sim
        String imei = mTelephonyManager.getImei();

        if (imei == null || imei.length() != 15) {
            Log.e(TAG, "getDeviceId failed!, wrong imei: " + imei);
            mDeviceId = null;
        } else {

            // Ex: "urn:gsma:imei:69878965-253645-4"
            String gsmaFormat = "urn:gsma:imei:" +
                imei.substring(0, 8) + "-" +
                imei.substring(8, 14) + "-" +
                imei.substring(14, 15);

            mDeviceId = gsmaFormat;
        }

        Log.d(TAG, "retrieve mDeviceId=" + mDeviceId);

        return mDeviceId;
    }

    public String getDeviceName() {
        String deviceName = DigitsSharedPreference.getInstance(mContext).getDeviceName();
        Log.d(TAG, "getDeviceName deviceName = " + deviceName);
        return deviceName;
    }

    public boolean sameDeviceId(String a, String b) {

        // Skip the last byte
        return (a.substring(0, a.length() - 1).equals(b.substring(0, b.length() - 1)));

    }

}
