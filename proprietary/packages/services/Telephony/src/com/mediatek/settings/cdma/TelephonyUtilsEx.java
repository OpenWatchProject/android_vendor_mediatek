package com.mediatek.settings.cdma;

import android.os.Build;
import android.os.SystemProperties;
import android.telephony.ServiceState;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;

import com.android.internal.telephony.Phone;

import com.mediatek.internal.telephony.MtkIccCardConstants.CardType;
import com.mediatek.telephony.MtkTelephonyManagerEx;

import java.util.List;

/**
 * Some util functions for C2K features.
 */
public class TelephonyUtilsEx {

    private static final String TAG = "TelephonyUtilsEx";
    private static final boolean DBG = "eng".equals(Build.TYPE);
    private static final String PROPERTY_3G_SIM = "persist.vendor.radio.simswitch";

    private static final String[] CT_NUMERIC = { "45502", "45507", "46003", "46011", "46012",
            "46013", "20404", "45431" };

    /**
     * Check whether Roaming or not.
     * @param phone The phone object.
     * @return true if Roaming
     */
    public static boolean isRoaming(Phone phone) {
        boolean result = false;
        int sub = -1;
        if (phone != null) {
            sub = phone.getSubId();
            ServiceState state = phone.getServiceState();
            if (state.getRoaming()) {
                result = true;
            }
        }
        log("isRoaming[" + sub + "] " + result);
        return result;
    }

    /**
     * Get the main phone id.
     * @return the main phone id
     */
    public static int getMainPhoneId() {
        int mainPhoneId = SubscriptionManager.INVALID_PHONE_INDEX;

        String curr3GSim = SystemProperties.get(PROPERTY_3G_SIM, "1");
        log("current 3G Sim = " + curr3GSim);

        if (!TextUtils.isEmpty(curr3GSim)) {
            int curr3GPhoneId = Integer.parseInt(curr3GSim);
            mainPhoneId = curr3GPhoneId - 1;
        }
        log("getMainPhoneId: " + mainPhoneId);

        return mainPhoneId;
    }

    /**
     * Check if phone has 4G capability.
     * @param phone The phone object.
     * @return the main capability phone id
     */
    public static boolean isCapabilityPhone(Phone phone) {
        boolean result = TelephonyUtilsEx.getMainPhoneId() == phone.getPhoneId();
        log("isCapabilityPhone result = " + result
                + " phoneId = " + phone.getPhoneId());

        return result;
    }

    /**
     * Check if sim is cdma 4g card.
     * @param subId The subscription id.
     * @return true if sim is cdma 4g card
     */
    public static boolean isCdma4gCard(int subId) {
        boolean result = false;
        CardType cardType = MtkTelephonyManagerEx.getDefault().getCdmaCardType(
                SubscriptionManager.getSlotIndex(subId));
        if (cardType != null) {
            result = cardType.is4GCard();
        } else {
            log("isCdma4gCard: cardType == null ");
        }
        log("isCdma4gCard result = " + result + "; subId = " + subId);
        return result;
    }

    private static void log(String msg) {
        if (DBG) {
            Log.d(TAG, msg);
        }
    }

    /**
     * Return if the sim card is ct operator.
     * @param subId sub id identify the sim card
     * @return true if the sim card is ct operator
     */
    public static boolean isCtSim(int subId) {
        boolean ctSim = false;
        String numeric = TelephonyManager.getDefault().getSimOperator(subId);
        for (String ct : CT_NUMERIC) {
            if (ct.equals(numeric)) {
                ctSim = true;
                break;
            }
        }
        log("getSimOperator:" + numeric + ", sub id :" + subId + ", isCtSim " + ctSim);
        return ctSim;
    }

    /**
      * Check whether ct volte is enable and is ct 4g sim.
      * @param subId the given subId
      * @return true if all the conditions are satisfied
      */
    public static boolean isCt4gSim(int subId) {
        return isCtSim(subId) && isCdma4gCard(subId);
    }

    /**
     * Check whether ct volte feature is enable.
     * @return true if all the conditions are satisfied
     */
    public static boolean isCtVolteEnabled() {
        String volteValue = SystemProperties.get("persist.vendor.mtk_ct_volte_support");
        boolean result = (volteValue.equals("1") || volteValue.equals("2"));
        log("volteValue = " + volteValue + ", isCtVolteEnabled " + result);
        return result;
    }

    /**
     * Check whether ct auto volte feature is enable.
     * @return true if all the conditions are satisfied
     */
    public static boolean isCtAutoVolteEnabled() {
        boolean result = SystemProperties.get("persist.vendor.mtk_ct_volte_support").equals("2");
        log("autoVolte = " + result);
        return result;
    }

    /**
     * Check if phone has both slot CT4G.
     * @param phoneSubscriptionManager provide SubScription Manager.
     * @return true if phone has both slot CT4G.
     */
    public static boolean isBothslotCt4gSim(SubscriptionManager phoneSubscriptionManager) {
        List<SubscriptionInfo> infos = phoneSubscriptionManager.getActiveSubscriptionInfoList();
        if (infos == null) {
            return false;
        }
        if (infos.size() <= 1) {
            return false;
        }
        boolean result = false;
        for (SubscriptionInfo info : infos) {
            int subId = info.getSubscriptionId();
            result = isCt4gSim(subId);
            if (result == false) {
                break;
            }
        }
        return result;
    }

    /**
     * Check if phone has both slot CT4G.
     * @param phoneSubscriptionManager provide SubScription Manager.
     * @return true if phone has both slot CT4G.
     */
    public static boolean isBothslotCtSim(SubscriptionManager phoneSubscriptionManager) {
        List<SubscriptionInfo> infos = phoneSubscriptionManager.getActiveSubscriptionInfoList();
        if (infos == null) {
            return false;
        }
        if (infos.size() <= 1) {
            return false;
        }
        boolean result = false;
        for (SubscriptionInfo info : infos) {
            int subId = info.getSubscriptionId();
            result = isCtSim(subId);
            if (result == false) {
                break;
            }
        }
        return result;
    }
}
