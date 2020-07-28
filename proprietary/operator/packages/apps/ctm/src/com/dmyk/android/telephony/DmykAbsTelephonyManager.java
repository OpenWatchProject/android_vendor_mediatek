package com.dmyk.android.telephony;

import android.content.Context;
import android.net.Uri;

public abstract class DmykAbsTelephonyManager {
    private static DmykAbsTelephonyManager sInstance = null;

    public static final int DATA_UNKNOWN = -1;
    public static final int DATA_DISCONNECTED = 0;
    public static final int DATA_CONNECTING = 1;
    public static final int DATA_CONNECTED = 2;
    public static final int DATA_SUSPENDED = 3; // e.g. in phone call in 2G mode

    public static final int SIM_STATE_UNKNOWN = 0;
    public static final int SIM_STATE_ABSENT = 1;
    public static final int SIM_STATE_PIN_REQUIRED = 2;
    public static final int SIM_STATE_PUK_REQUIRED = 3;
    public static final int SIM_STATE_NETWORK_LOCKED = 4;
    public static final int SIM_STATE_READY = 5;
    public static final int SIM_STATE_NOT_READY = 6;
    public static final int SIM_STATE_PERM_DISABLED = 7;
    public static final int SIM_STATE_CARD_IO_ERROR = 8;
    public static final int SIM_STATE_CARD_RESTRICTED = 9;

    public static final int DEVICE_TYPE_UNKNOWN = 0;
    public static final int DEVICE_TYPE_CELLPHONE = 1;
    public static final int DEVICE_TYPE_PAD = 2;
    public static final int DEVICE_TYPE_STB = 3; // Set Top Box
    public static final int DEVICE_TYPE_WATCH = 4;
    public static final int DEVICE_TYPE_BRACELET = 5;

    public static final int NETWORK_TYPE_UNKNOWN = 0;
    public static final int NETWORK_TYPE_GPRS = 1;
    public static final int NETWORK_TYPE_EDGE = 2;
    public static final int NETWORK_TYPE_UMTS = 3;
    public static final int NETWORK_TYPE_CDMA = 4;
    public static final int NETWORK_TYPE_EVDO_0 = 5;
    public static final int NETWORK_TYPE_EVDO_A = 6;
    public static final int NETWORK_TYPE_1xRTT = 7;
    public static final int NETWORK_TYPE_HSDPA = 8;
    public static final int NETWORK_TYPE_HSUPA = 9;
    public static final int NETWORK_TYPE_HSPA = 10;
    public static final int NETWORK_TYPE_IDEN = 11;
    public static final int NETWORK_TYPE_EVDO_B = 12;
    public static final int NETWORK_TYPE_LTE = 13;
    public static final int NETWORK_TYPE_EHRPD = 14;
    public static final int NETWORK_TYPE_HSPAP = 15;
    public static final int NETWORK_TYPE_GSM = 16;
    public static final int NETWORK_TYPE_TD_SCDMA = 17;
    public static final int NETWORK_TYPE_IWLAN = 18;
    public static final int NETWORK_TYPE_LTE_CA = 19;

    public static final String VOLTE_DMYK_STATE_0 = "volte_dmyk_state_0";
    public static final String VOLTE_DMYK_STATE_1 = "volte_dmyk_state_1";
    public static final int VOLTE_STATE_ON = 1;
    public static final int VOLTE_STATE_OFF = 0;
    public static final int VOLTE_STATE_UNKNOWN = -1;

    public static final int SWITCH_STATE_OFF = 0;
    public static final int SWITCH_STATE_ON = 1;
    public static final int SWITCH_STATE_UNKNOWN = 2;

    public static final int SWITCH_WIFI = 0;
    public static final int SWITCH_GPRS = 1;
    public static final int SWITCH_BLUETOOTH = 2;
    public static final int SWITCH_GPS = 3;
    public static final int SWITCH_SHOCK = 4;
    public static final int SWITCH_SILENT = 5;
    public static final int SWITCH_HOT_SPOT = 6;
    public static final int SWITCH_FLYING = 7;
    public static final int SWITCH_FLASHLIGHT = 8;
    public static final int SWITCH_SCREEN = 9;
    public static final int SWITCH_SCREEN_ROTATE = 10;
    public static final int SWITCH_LTE = 11;
    public static final int SWITCH_AUTO_BRIGHT = 12;

    public static final String ACTION_VOLTE_STATE_CHANGE =
                                   "com.dmyk.android.telephony.action.VOLTE_STATE_CHANGE";
    public static final String ACTION_VOLTE_STATE_SETTING =
                                   "com.dmyk.android.telephony.action.VOLTE_STATE_SETTING";
    public static final String ACTION_APN_STATE_CHANGE =
                                   "com.dmyk.android.telephony.action.APN_STATE_CHANGE";
    public static final String ACTION_SIM_STATE_CHANGED =
                                   "com.dmyk.android.telephony.action.SIM_STATE_CHANGED";
    public static final String ACTION_CONNECTIVITY_CHANGE =
                                   "android.dmyk.net.conn.CONNECTIVITY_CHANGE";

    public static final String EXTRA_SIM_PHONEID = "com.dmyk.android.telephony.extra.SIM_PHONEID";
    public static final String EXTRA_SIM_STATE = "com.dmyk.android.telephony.extra.SIM_STATE";
    //public static final String EXTRA_VOLTE_STATE = "com.dmyk.android.telephony.extra.VOLTE_STATE"; dm 2.0 remove

    public class LocationMsg {
        public double longitude = 0x1.fffffffffffffP+1023;
        public double latitude = 0x1.fffffffffffffP+1023;
        public int addrFrom = -1;
    }
    /**
     * Get the number of SIM slots on the phone.
     *
     * @return number of SIM slots
     */
    public abstract int getPhoneCount();

    /**
     * Get the IMEI of the phone.
     *
     * @param phoneId SIM slot ID
     * @return IMEI
     */
    public abstract String getGsmDeviceId(int phoneId);

    /**
     * Get the MEID of the phone.
     * @return MEID
     */
    public abstract String getCdmaDeviceId();

    /**
     * Get IMSI of SIM in specified slot.
     * @param phoneId SIM slot ID
     * @return IMSI
     */
    public abstract String getSubscriberId(int phoneId);

    /**
     * Get ICCID of SIM in specified slot.
     * @param phoneId SIM slot ID
     * @return ICCID
     */
    public abstract String getIccId(int phoneId);
    public abstract int getDataState(int phoneId);
    public abstract int getSimState(int phoneId);
    public abstract int getNetworkType(int phoneId);
    public abstract String getDeviceSoftwareVersion();
    public abstract int getDeviceType();
    public abstract int getMasterPhoneId();
    public abstract boolean isInternationalNetworkRoaming(int phoneId);
    public abstract int getVoLTEState(int phoneId);
    public abstract Uri getAPNContentUri(int phoneId);
    public abstract int getSlotId(int phoneId);
    public abstract int getCellId(int phoneId);
    public abstract int getLac(int phoneId);
    public abstract String getCTAModel();
    public abstract String getRomStorageSize();
    public abstract String getRamStorageSize();
    public abstract String getMacAddress();
    public abstract String getCPUModel();
    public abstract String getOSVersion();
    public abstract long getWiFiTotalBytes(long startTime, long endTime);
    public abstract long getMobileTotalBytes(int phoneId, long startTime, long endTime);
    public abstract int getPriorNetworkType();
    public abstract String getPhoneNumber(int phoneId);
    public abstract int getSwitchState(int switchId);
    public abstract LocationMsg getLocationMsg();
    public abstract String getModel();

    /**
     * Get an instance of DmykAbsTelephonyManager.
     * NOTE: only the first non-null context specified is effective.
     *
     * @param context Android context used by the instance
     * @return the instance
     */

    public synchronized static DmykAbsTelephonyManager getDefault(Context context) {
        if (sInstance == null) {
            sInstance = new DmykTelephonyManager(context);
        }

        if (((DmykTelephonyManager)sInstance).getContext() == null && context != null) {
            ((DmykTelephonyManager)sInstance).setContext(context);
        }
        return sInstance;
    }

}
