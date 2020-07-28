package com.mediatek.presence.utils;

import android.telephony.SubscriptionManager;
import com.mediatek.telephony.MtkTelephonyManagerEx;

public class SimUtils {
    private static final String[] MCC_MNC_TMO = {
        "310031", "310160", "310200", "310210",
        "310220", "310230", "310240", "310250",
        "310260", "310270", "310280", "310300",
        "310310", "310330", "310660", "310800"
    };

    private static final String[] MCC_MNC_VZW = {
        "310004", "310010", "310012", "310013",
        "310890", "310910", "311110", "311270",
        "311271", "311272", "311273", "311274",
        "311275", "311276", "311277", "311278",
        "311279", "311280", "311281", "311282",
        "311283", "311284", "311285", "311286",
        "311287", "311288", "311289", "311390",
        "311480", "311481", "311482", "311483",
        "311484", "311485", "311486", "311487",
        "311488", "311489"
    };

    private static final String[] MCC_MNC_ATT = {
        "310070", "310150", "310170", "310380",
        "310410", "310560", "310680", "310980",
    };

    private static final String[] MCC_MNC_CMCC = {
        "46000", "46002", "46007"
    };

    public static boolean isTmoSimCard(int slotId) {
        String[] mccMncList = getMccMncList(slotId);
        for (String tmoMccMnc : MCC_MNC_TMO) {
            for (String simMccMnc : mccMncList) {
                if (tmoMccMnc.equals(simMccMnc)) {
                    return true;
                }
            }
        }
        return false;
    }

    public static boolean isVzwSimCard(int slotId) {
        String[] mccMncList = getMccMncList(slotId);
        for (String vzwMccMnc : MCC_MNC_VZW) {
            for (String simMccMnc : mccMncList) {
                if (vzwMccMnc.equals(simMccMnc)) {
                    return true;
                }
            }
        }
        return false;
    }

    public static boolean isAttSimCard(int slotId) {
        String[] mccMncList = getMccMncList(slotId);
        for (String attMccMnc : MCC_MNC_ATT) {
            for (String simMccMnc : mccMncList) {
                if (attMccMnc.equals(simMccMnc)) {
                    return true;
                }
            }
        }
        return false;
    }

    public static boolean isCmccSimCard(int slotId) {
        String[] mccMncList = getMccMncList(slotId);
        for (String cmccMccMnc : MCC_MNC_CMCC) {
            for (String simMccMnc : mccMncList) {
                if (cmccMccMnc.equals(simMccMnc)) {
                    return true;
                }
            }
        }
        return false;
    }

    private static String[] getMccMncList(int slotId) {
        int[] subIds = SubscriptionManager.getSubId(slotId);
        if (subIds == null) {
            return new String[] {"empty"};
        }
        int phoneId = SubscriptionManager.getPhoneId(subIds[0]);
        return MtkTelephonyManagerEx.getDefault()
                .getSimOperatorNumericForPhoneEx(phoneId);
    }
}
