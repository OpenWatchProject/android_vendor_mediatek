package com.mediatek.wfo;

public class WifiOffloadManager {
    /**
     * For accessing the WifiOffload related service.
     * Internal use only.
     */
    public static final String WFO_SERVICE = "wfo";

    public static final String ACTION_NOTIFY_CONNECTION_ERROR =
            "android.intent.action.NOTIFY_CONNECTION_ERROR";

    public static final String EXTRA_ERROR_CODE = "error_code";

    /**
     * Alert need to send when user moves out of Wifi area
     * and no LTE is available to camp on
     */
    public static final String ACTION_ROVE_OUT_ALERT = "android.intent.action.ROVE_OUT_ALERT";

    public static final int RAN_TYPE_UNSPEC = 0;
    public static final int RAN_TYPE_MOBILE_3GPP = 1;
    public static final int RAN_TYPE_WIFI = 2;
    public static final int RAN_TYPE_MOBILE_3GPP2 = 3;
    // deny by PS
    public static final int RAN_TYPE_NONE = -2;
    // deny by RDS
    public static final int RAN_TYPE_DENY = -1;

    public static final int HANDOVER_START = 0;
    public static final int HANDOVER_END = 1;
    public static final int HANDOVER_FAILED = -1;

    // Call type.
    public static final int CALL_TYPE_VOICE = 1;
    public static final int CALL_TYPE_VIDEO = 2;

    // Call state.
    public static final int CALL_STATE_END = 0;
    public static final int CALL_STATE_ACTIVE = 1;
    public static final int CALL_STATE_ESTABLISHING = 2;

    /**
     * Listener interface for WifiOffload events.
     */
    public abstract static class Listener extends IWifiOffloadListener.Stub {
        @Override
        public void onHandover(int simIdx, int stage, int ratType) {};
        @Override
        public void onRoveOut(int simIdx, boolean roveOut, int rssi) {};
        @Override
        public void onRequestImsSwitch(int simIdx, boolean isImsOn) {};
        @Override
        public void onWifiPdnOOSStateChanged(int simIdx, int oosState) {};
        @Override
        public void onAllowWifiOff() {};
        @Override
        public void onWfcStateChanged(int simIdx, int state) {};
    }
}
