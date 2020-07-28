/*
* Copyright (C) 2011-2014 MediaTek Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

package com.mediatek.systemui.statusbar.util;

import android.content.Context;
import android.hardware.display.DisplayManager;
import android.hardware.display.WifiDisplayStatus;
import android.net.ConnectivityManager;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.provider.Settings;
import android.telephony.ServiceState;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;

import com.android.internal.telephony.ITelephony;
import com.mediatek.internal.telephony.MtkIccCardConstants;
import com.mediatek.internal.telephony.MtkIccCardConstants.CardType;
import com.mediatek.internal.telephony.MtkSubscriptionManager;
import com.mediatek.telephony.MtkTelephonyManagerEx;

import java.util.Collections;
import java.util.Comparator;
import java.util.List;

/**
 * [SystemUI] Support "dual SIM" and "Notification toolbar".
 */
public class SIMHelper {

    private static final String TAG = "SIMHelper";

    private static List<SubscriptionInfo> sSimInfos;

    public static final int SLOT_INDEX_DEFAULT = 0;
    public static final int SLOT_INDEX_1 = 1;
    public static final int SLOT_INDEX_2 = 2;
    public static final int SLOT_INDEX_3 = 3;
    public static final int INVALID_SLOT_ID = -1000;

    private static final String PROPERTY_3G_SIM = "persist.vendor.radio.simswitch";
    private static final String[] CT_NUMERIC = { "45502", "45507", "46003", "46011", "46012",
            "46013" };

    public static Context sContext;

    private SIMHelper() {
    }

    public static SubscriptionInfo getSubInfoBySlot(Context context, int slotId) {
        if (sSimInfos == null || sSimInfos.size() == 0) {
            Log.d("@M_" + TAG, "getSubInfoBySlot, SubscriptionInfo is null");
            return null;
        }

        for (SubscriptionInfo info : sSimInfos) {
            if (info.getSimSlotIndex() == slotId) {
                return info;
            }
        }
        return null;
    }

    /**
     * Update Active SubscriptionInfos.
     *
     * @param context A Context object
     */
    public static void updateSIMInfos(Context context) {
        sSimInfos = SubscriptionManager.from(context).getActiveSubscriptionInfoList();
    }

    public static int getFirstSubInSlot(int slotId) {
        int[] subIds = SubscriptionManager.getSubId(slotId);
        if (subIds != null && subIds.length > 0) {
            return subIds[0];
        }
        Log.d(TAG, "Cannot get first sub in slot: " + slotId);
        return SubscriptionManager.INVALID_SUBSCRIPTION_ID;
    }

    public static boolean hasService(ServiceState ss) {
        if (ss != null) {
            // Consider the device to be in service if either voice or data service is available.
            // Some SIM cards are marketed as data-only and do not support voice service, and on
            // these SIM cards, we want to show signal bars for data service as well as the "no
            // service" or "emergency calls only" text that indicates that voice is not available.
            switch (ss.getVoiceRegState()) {
                case ServiceState.STATE_POWER_OFF:
                    return false;
                case ServiceState.STATE_OUT_OF_SERVICE:
                case ServiceState.STATE_EMERGENCY_ONLY:
                    return ss.getDataRegState() == ServiceState.STATE_IN_SERVICE;
                default:
                    return true;
            }
        } else {
            return false;
        }
    }

    public static int getSlotCount() {
        //FIXME: the slot count may not always be equal to phone count
        return TelephonyManager.getDefault().getPhoneCount();
    }

    public static boolean isSimInsertedBySlot(Context context, int slotId) {
        if (sSimInfos != null) {
            if (slotId <= getSlotCount() - 1) {
                SubscriptionInfo info = getSubInfoBySlot(context, slotId);
                if (info != null) {
                    return true;
                } else {
                    return false;
                }
            } else {
                return false; // default return false
            }
        } else {
            Log.d("@M_" + TAG, "isSimInsertedBySlot, SubscriptionInfo is null");
            return false;
        }
    }
    /// @}

    public static void setContext(Context context) {
        sContext = context;
    }

    /// M: add DataConnection in quicksetting @{
    public static boolean isWifiOnlyDevice() {
        ConnectivityManager cm =
                (ConnectivityManager) sContext.getSystemService(sContext.CONNECTIVITY_SERVICE);
        return  !(cm.isNetworkSupported(ConnectivityManager.TYPE_MOBILE));
    }
    /// M: add DataConnection in quicksetting @}

    /// M: Remove CastTile when WFD is not support in quicksetting @{
    public static boolean isWifiDisplaySupport() {
        DisplayManager mDisplayManager =
            (DisplayManager) sContext.getSystemService(Context.DISPLAY_SERVICE);
        WifiDisplayStatus status = mDisplayManager.getWifiDisplayStatus();
        if (status == null || status.getFeatureState()
                == WifiDisplayStatus.FEATURE_STATE_UNAVAILABLE) {
            return false;
        }
        return true;
    }
    /// M: Remove CastTile when WFD is not support in quicksetting @}

    /**
     * Get the radio state of default data sim.
     *
     * @param subId default data sim subid
     * @return The radio state of default data sim
     */
    public static boolean isRadioOn(int subId) {
        ITelephony telephony = ITelephony.Stub.asInterface(ServiceManager
                .getService(Context.TELEPHONY_SERVICE));
        try {
            if (telephony != null) {
                return telephony.isRadioOnForSubscriber(subId, sContext.getPackageName());
            }
        } catch (RemoteException e) {
            Log.e(TAG, "mTelephony exception");
        }
        return false;
    }

    public static int getMainPhoneId() {
        int mainPhoneId = SubscriptionManager.INVALID_PHONE_INDEX;
        String curr3GSim = SystemProperties.get(PROPERTY_3G_SIM, "1");
        if (!TextUtils.isEmpty(curr3GSim)) {
            int curr3GPhoneId = Integer.parseInt(curr3GSim);
            mainPhoneId = curr3GPhoneId - 1;
        }
        Log.d("@M_" + TAG, "getMainPhoneId, mainPhoneId = " + mainPhoneId);
        return mainPhoneId;
    }

    public static boolean isCtSim(int subId) {
        boolean ctSim = false;
        String numeric = TelephonyManager.getDefault().getSimOperator(subId);
        for (String ct : CT_NUMERIC) {
            if (ct.equals(numeric)) {
                ctSim = true;
                break;
            }
        }
        if (FeatureOptions.LOG_ENABLE) {
            Log.d("@M_" + TAG, "isCtSim, ctSim = " + ctSim + ", subId = " + subId);
        }
        return ctSim;
    }

    public static boolean isCTCardType(int slotId) {
        boolean ctCard = false;
        MtkIccCardConstants.CardType cardType =
                MtkTelephonyManagerEx.getDefault().getCdmaCardType(slotId);
        if (((MtkIccCardConstants.CardType.CT_4G_UICC_CARD).equals(cardType) ||
                (MtkIccCardConstants.CardType.CT_3G_UIM_CARD).equals(cardType) ||
                (MtkIccCardConstants.CardType.CT_UIM_SIM_CARD).equals(cardType))) {
            ctCard = true;
        }
        if (FeatureOptions.LOG_ENABLE) {
            Log.d("@M_" + TAG, "isCTCardType, ctCard = " + ctCard
                    + ", soltId = " + slotId);
        }
        return ctCard;
    }
}
