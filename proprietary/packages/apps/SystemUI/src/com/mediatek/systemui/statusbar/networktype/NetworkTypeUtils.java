
package com.mediatek.systemui.statusbar.networktype;

import android.telephony.NetworkRegistrationInfo;
import android.telephony.ServiceState;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.android.systemui.R;
import com.android.systemui.statusbar.policy.NetworkControllerImpl.Config;

import java.util.HashMap;
import java.util.Map;

/**
 * An utility class to access network type.
 */
public class NetworkTypeUtils {
    private static final String TAG = "NetworkTypeUtils";

    public static final int VOLTE_ICON = R.drawable.stat_sys_volte;
    public static final int VONR_ICON = R.drawable.stat_sys_vonr;
    public static final int WFC_ICON = R.drawable.stat_sys_wfc;

    //For 4G+W
    public static final int LWA_ICON = R.drawable.stat_sys_data_fully_connected_4gaw;
    public static final int LWA_STATE_CONNCTED = 1;
    public static final int LWA_STATE_DISCONNCTED = 0;
    public static final int LWA_STATE_UNKNOWN = -1;
    // Connection phase
    public static final String LWA_STATE_CHANGE_ACTION =
        "com.mediatek.server.lwa.LWA_STATE_CHANGE_ACTION";
    public static final String EXTRA_PHONE_ID =
        "com.mediatek.server.lwa.EXTRA_PHONE_ID"; // int phone id
    public static final String EXTRA_STATE =
        "com.mediatek.server.lwa.EXTRA_STATE"; //0: disconnected, 1: connected

    static final Map<Integer, Integer> sNetworkTypeIcons = new HashMap<Integer, Integer>() {
        {
            // For CDMA 3G
            put(TelephonyManager.NETWORK_TYPE_EVDO_0, R.drawable.stat_sys_network_type_3g);
            put(TelephonyManager.NETWORK_TYPE_EVDO_A, R.drawable.stat_sys_network_type_3g);
            put(TelephonyManager.NETWORK_TYPE_EVDO_B, R.drawable.stat_sys_network_type_3g);
            put(TelephonyManager.NETWORK_TYPE_EHRPD, R.drawable.stat_sys_network_type_3g);
            // For CDMA 1x
            put(TelephonyManager.NETWORK_TYPE_CDMA, R.drawable.stat_sys_network_type_1x);
            put(TelephonyManager.NETWORK_TYPE_1xRTT, R.drawable.stat_sys_network_type_1x);
            // Edge
            put(TelephonyManager.NETWORK_TYPE_EDGE, R.drawable.stat_sys_network_type_e);
            // 3G
            put(TelephonyManager.NETWORK_TYPE_UMTS, R.drawable.stat_sys_network_type_3g);
            // For 4G
            put(TelephonyManager.NETWORK_TYPE_LTE, R.drawable.stat_sys_network_type_4g);
            // 3G
            put(TelephonyManager.NETWORK_TYPE_HSDPA, R.drawable.stat_sys_network_type_3g);
            put(TelephonyManager.NETWORK_TYPE_HSUPA, R.drawable.stat_sys_network_type_3g);
            put(TelephonyManager.NETWORK_TYPE_HSPA, R.drawable.stat_sys_network_type_3g);
            put(TelephonyManager.NETWORK_TYPE_HSPAP, R.drawable.stat_sys_network_type_3g);
            put(TelephonyManager.NETWORK_TYPE_IWLAN, 0);
            // For 5G
            put(TelephonyManager.NETWORK_TYPE_NR, R.drawable.stat_sys_network_type_5g);
        }
    };

    /**
     * Map the network type into the related icons.
     * @param serviceState ServiceState to get current network type.
     * @param config Config passed in.
     * @param hasService true for in service.
     * @return Network type's icon.
     */
    public static int getNetworkTypeIcon(ServiceState serviceState, Config config,
            boolean hasService) {
        if (!hasService) {
            // Not in service, no network type.
            return 0;
        }
        int tempNetworkType = getNetworkType(serviceState);
        if (getNr5GNetWorkType(serviceState) == TelephonyManager.NETWORK_TYPE_NR) {
            tempNetworkType = TelephonyManager.NETWORK_TYPE_NR;
        }
        Integer iconId = sNetworkTypeIcons.get(tempNetworkType);
        if (iconId == null) {
            iconId = tempNetworkType == TelephonyManager.NETWORK_TYPE_UNKNOWN ? 0 :
                     config.showAtLeast3G ? R.drawable.stat_sys_network_type_3g :
                                            R.drawable.stat_sys_network_type_g;
        }
        return iconId.intValue();
    }

    private static int getNetworkType(ServiceState serviceState) {
        int type = TelephonyManager.NETWORK_TYPE_UNKNOWN;
        if (serviceState != null) {
            type = serviceState.getDataNetworkType() != TelephonyManager.NETWORK_TYPE_UNKNOWN ?
                    serviceState.getDataNetworkType() : serviceState.getVoiceNetworkType();
        }
        return type;
    }

    private static int getNr5GNetWorkType(ServiceState serviceState) {
        if (serviceState != null) {
            int nrState = serviceState.getNrState();
            Log.d(TAG, "nrState = " + nrState);
            if (nrState == NetworkRegistrationInfo.NR_STATE_CONNECTED ||
                        nrState == NetworkRegistrationInfo.NR_STATE_NOT_RESTRICTED) {
                return TelephonyManager.NETWORK_TYPE_NR;
            }
        }
        return TelephonyManager.NETWORK_TYPE_UNKNOWN;
    }
}
