package com.mediatek.mms.plugin;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.SystemProperties;
import android.preference.CheckBoxPreference;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.widget.CheckBox;

import com.mediatek.mms.ext.DefaultOpSubSelectActivityExt;
import com.mediatek.telephony.MtkTelephonyManagerEx;

public class Op09SubSelectActivityExt extends DefaultOpSubSelectActivityExt {
    private static final String TAG = "Op09SubSelectActivityExt";
    /* P0 migration*/
    public static final boolean MTK_C2K_SUPPORT = SystemProperties.get("ro.vendor.mtk_ps1_rat")
            .contains("C");//.equals("1");
    private static final String SMS_DELIVERY_REPORT_MODE = "pref_key_sms_delivery_reports";
    private static final String PREFERENCE_KEY = "PREFERENCE_KEY";

    @Override
    public boolean getView(Context context, int position, CheckBox subCheckBox,
        String preferenceKey) {
        if (SMS_DELIVERY_REPORT_MODE.equals(preferenceKey)) {
            int[] subIds = SubscriptionManager.getSubId(position);
            /* q0 migration*/
            if (subIds == null) {
                return false;
            }
            int subId = subIds[0];
            if (isUSimType(subId) && isInternationalRoamingStatus(context, subId)) {
                Log.d(TAG, "[getView]:CDMA SIM in international roaming, subId=" + subId);
                if (subCheckBox.isChecked()) {
                    subCheckBox.setChecked(false);
                }
                subCheckBox.setEnabled(false);
                return false;
            }
        }
        return true;
    }

    @Override
    public boolean isSimSupported(int subId) {
        Log.d(TAG, "[isSimSupported]: subId=" + subId);
        if (MTK_C2K_SUPPORT && isUSimType(subId)
            && !isCSIMInGsmMode(subId)) {
            Log.d(TAG, "[isSimSupported]: false");
            return false;
        }
        Log.d(TAG, "[isSimSupported]: true");
        return true;
    }

    private boolean isUSimType(int subId) {
        String phoneType = MtkTelephonyManagerEx.getDefault().getIccCardType(subId);
        if (phoneType == null) {
            Log.d(TAG, "[isUIMType]: phoneType = null");
            return false;
        }
        Log.d(TAG, "[isUIMType]: phoneType = " + phoneType);
        return phoneType.equalsIgnoreCase("CSIM") || phoneType.equalsIgnoreCase("UIM")
            || phoneType.equalsIgnoreCase("RUIM");
    }

    private boolean isCSIMInGsmMode(int subId) {
        if (isUSimType(subId)) {
            MtkTelephonyManagerEx tmEx = MtkTelephonyManagerEx.getDefault();
            int vnt = tmEx.getPhoneType(SubscriptionManager.getSlotIndex(subId));
            Log.d(TAG,
                "[isCSIMInGsmMode]:[NO_PHONE = 0;" +
                "GSM_PHONE = 1; CDMA_PHONE = 2;]; phoneType:"
                    + vnt);
            if (vnt == TelephonyManager.PHONE_TYPE_GSM) {
                return true;
            }
        }
        return false;
    }

    private boolean isInternationalRoamingStatus(Context context, long subId) {
        boolean isRoaming = false;
        int simCount = SubscriptionManager.from(context.getApplicationContext())
                        .getActiveSubscriptionInfoCount();
        if (simCount <= 0) {
            Log.e(TAG, "isInternationalRoamingStatus(): Wrong subId!");
            return false;
        }
        /* P0 migration*/
        TelephonyManager telephonyManager = context.getSystemService(TelephonyManager.class);
        TelephonyManager telephonyManagerEx = telephonyManager.createForSubscriptionId((int)subId);
        isRoaming = telephonyManagerEx.isNetworkRoaming(/*(int) subId*/);
        Log.d(TAG, "isInternationalRoamingStatus() isRoaming: " + isRoaming);
        return isRoaming;
    }

    @Override
    public boolean onListItemClick(final Activity hostActivity, final int subId) {
        Intent intent = hostActivity.getIntent();
        String intentKey = intent.getStringExtra(PREFERENCE_KEY);
        Log.d(TAG, "op09C onListItemClick() subId: " + subId + ", intentKey = " + intentKey);
        if (intentKey != null && intentKey.equals(SMS_DELIVERY_REPORT_MODE)) {
            // CT6M, Disable Usim card delivery report setting if is international roaming
            if (isUSimType(subId) && isInternationalRoamingStatus(hostActivity, subId)) {
                Log.d(TAG, "op09C onListItemClick(), CDMA SIM in international roaming");
                return true;
            }
        }
        return false;
    }
}
