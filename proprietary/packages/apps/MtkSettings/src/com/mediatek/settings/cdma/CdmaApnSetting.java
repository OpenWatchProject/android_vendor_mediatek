package com.mediatek.settings.cdma;

import android.content.Context;
import android.os.ServiceManager;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;

//import com.mediatek.internal.telephony.IMtkTelephonyEx;

public class CdmaApnSetting {

    private static final String TAG = "CdmaApnSetting";
    private static final String CT_CHINA_NW_MCC = "460";
    private static final String CT_MACOO_NW_MCC = "455";
    public static final String CT_NUMERIC_LTE = "46011";
    public static final String CT_NUMERIC_CDMA = "46003";

    public static String customizeQuerySelectionforCdma(String where, String numeric, int subId) {
        String result = where;
        Log.d(TAG, "customizeQuerySelectionforCdma, subId = " + subId);
        /*if (!CdmaUtils.isSupportCdma(subId)) {
            Log.d(TAG, "not CDMA, just return");
            return result;
        }*/
        String sqlStr = "";
        String apn = "";
        String sourceType = "";
        /*try*/ {
            //IMtkTelephonyEx telephonyEx = IMtkTelephonyEx.Stub.asInterface(
            //        ServiceManager.getService("phoneEx"));
            String mvnoType = null; //telephonyEx.getMvnoMatchType(subId);
            String mvnoPattern = null; //telephonyEx.getMvnoPattern(subId, mvnoType);
            // If mvnoType or mvnoPattern is null, should replace with ''
            sqlStr = " mvno_type=\'" + replaceNull(mvnoType)
                + "\'" + " and mvno_match_data=\'" + replaceNull(mvnoPattern) + "\'";
        }  /*catch (android.os.RemoteException e) {
            Log.d(TAG, "RemoteException " + e);
        }*/

        // for [APN CT Roaming], special APN selection rule for CT roaming @{
        String networkNumeric = TelephonyManager.getDefault().getNetworkOperator(subId);
        Log.d(TAG, " numeric = " + numeric + ", networkNumeric = " +networkNumeric);
        if (isCtNumeric(numeric)) {
            Log.d(TAG, "networkNumeric = " + networkNumeric);
            if (isCtInRoaming(numeric, subId)) {
                Log.d(TAG, "ROAMING");
                apn += " and apn <> \'ctwap\'";
                result = "numeric=\'" + networkNumeric + "\' and "
                        + "((" + sqlStr + apn + ")" + " or (sourceType = \'1\'))";
                Log.d(TAG, "customizeQuerySelectionforCdma, roaming result = " + result);
                return result;
            } else {
                sqlStr = "((" + sqlStr + ")" + " or (sourceType = \'1\'))";
                result = "numeric=\'" + numeric + "\' and " + sqlStr ;
                Log.d(TAG, "customizeQuerySelectionforCdma, result = " + result);
                return result;
            }
        }
        // @}
        return result;
    }

    private static boolean isCtNumeric(String numeric) {
        return (numeric != null && (numeric.contains(CT_NUMERIC_LTE) || numeric
                .contains(CT_NUMERIC_CDMA)));
    }

    private static boolean isCtInRoaming(String numeric, int subId) {
        if (isCtNumeric(numeric)) {
            String networkNumeric = TelephonyManager.getDefault().getNetworkOperator(subId);
            if (networkNumeric != null && networkNumeric.length() >= 3
                    && !networkNumeric.startsWith(CT_CHINA_NW_MCC)
                    && !networkNumeric.startsWith(CT_MACOO_NW_MCC)) {
                return true;
            }
        }
        return false;
    }

    public static String updateMccMncForCdma(String numeric, int subId) {
        String networkNumeric = TelephonyManager.getDefault().getNetworkOperator(subId);
        Log.d(TAG, "updateMccMncForCdma, subId = " + numeric + ", numeric = " + subId
                + ", networkNumeric = " + networkNumeric);
        /*if (CdmaUtils.isSupportCdma(subId) && isCtInRoaming(networkNumeric, subId)) {
            Log.d(TAG, "ROAMING, return " + networkNumeric);
            return networkNumeric;
        }*/
        return numeric;
    }

    private static String replaceNull(String origString) {
        if (origString == null) {
            return "";
        } else {
            return origString;
        }
    }
}
