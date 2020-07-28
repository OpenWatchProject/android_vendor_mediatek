package com.mediatek.ims.pco;

/**
 *  All the necessary constants are defined.
 */
final public class Constants {

    /*
    * For different screens
    */

    public static final String SCREEN_TYPE = "screen-type";
    public static final String SCREEN_UI_UNKNOWN = "unknown";
    public static final String SCREEN_SHOW_WELCOME = "startup wizard";
    public static final String SCREEN_SHOW_WIFI_DIALOGUE = "show-wifi-dialogue";
    public static final String SCREEN_SHOW_DATA_DIALOGUE = "show-data-dialogue";
    public static final String SCREEN_WIFI_NOT_CONNECTED = "wifi-not-connected";
    public static final String SCREEN_WIFI_CONNECTING = "wifi-connecting";
    public static final String SCREEN_WIFI_CONNECTED = "wifi-connected";
    public static final String SCREEN_SHOW_URL_DIALOGUE = "signup-url-dialogue";
    public static final String SCREEN_SHOW_ACTIVATED = "activated-dialogue";

    public static final String CONNECTED = "connected";
    public static final String NOT_CONNECTED = "not connected";
    public static final String UNKNOWN = "unknown";

    public static final int MSG_ID_REQ_TRUN_ON_WIFI = 11;
    public static final int MSG_ID_RSP_TRUN_ON_WIFI = 12;
    public static final int MSG_ID_REQ_TRUN_OFF_WIFI = 13;
    public static final int MSG_ID_RSP_TRUN_OFF_WIFI = 14;
    public static final int MSG_ID_REQ_TRUN_ON_MOBILE_DATA = 15;
    public static final int MSG_ID_RSP_TRUN_ON_MOBILE_DATA = 16;
    public static final int MSG_ID_REQ_TRUN_OFF_MOBILE_DATA = 17;
    public static final int MSG_ID_RSP_TRUN_OFF_MOBILE_DATA = 18;
    public static final int MSG_ID_REQ_TRUN_ON_RADIO = 19;
    public static final int MSG_ID_REQ_RADIO_TRUNOFF = 20;

    public static final int MSG_ID_REQ_OPEN_END_POINT_URL = 21;
    public static final int MSG_ID_RSP_OPEN_END_POINT_URL = 22;
    public static final int MSG_ID_REQ_OPEN_END_POINT_URL_DISAGREE = 23;
    public static final int MSG_ID_RSP_OPEN_END_POINT_URL_DISAGREE = 24;
    public static final int MSG_ID_REQ_CALL_911 = 25;
    public static final int MSG_ID_RSP_CALL_911 = 26;
    public static final int MSG_ID_REQ_CHECK_IMS_PCO_STATUS = 27;
    public static final int MSG_ID_RSP_CHECK_IMS_PCO_STATUS = 28;

    public static final int MSG_ID_REQ_HTTP_GET = 29;
    public static final int MSG_ID_RSP_HTTP_GET = 30;
    public static final int MSG_ID_REQ_HTTP_POST = 31;
    public static final int MSG_ID_RSP_HTTP_POST = 32;
    public static final int MSG_ID_REQ_HTTP_PUT = 33;
    public static final int MSG_ID_RSP_HTTP_PUT = 34;


    /**
     * Static notification IDs.
     */

    public static final String SIGNUP_USER_NOTIFICATION_ID =
            "com.mediatek.ims.pco.ui.SIGNUP_USER_NOTIFICATION_ID";

    public static final String ACTION_SIGN_UP_EXIT =
           "com.mediatek.ims.pco.ui.ACTION_SIGN_UP_EXIT";

    public static final String ACTION_QUERY_IMS_PCO_STATUS_FRM_NOTIFICATION =
            "com.mediatek.ims.pco.ACTION_QUERY_IMS_PCO_STATUS_FRM_NOTIFICATION";

    public static final String ACTION_QUERY_IMS_PCO_STATUS_FRM_BROWSER =
            "com.mediatek.ims.pco.ACTION_QUERY_IMS_PCO_STATUS_FRM_BROWSER";

    public static final String ACTION_QUERY_IMS_PCO_STATUS_FRM_CALL =
            "com.mediatek.ims.pco.ACTION_QUERY_IMS_STATUS_FRM_CALL";

    public static final String ACTION_SIGN_UP =
                       "com.mediatek.ims.pco.ui.ACTION_SIGN_UP";
    public static final String ACTION_CATEGORY_UI =
            "com.mediatek.ims.pco.ui.ACTION_CATEGORY_UI";

    public static final String EXTRA_ACTIVATION_STATUS =
            "activation-status";
    public static final String EXTRA_NETWORK_TYPE_TO_USE =
            "preferred-network-type-for-activation";
    public static final String EXTRA_SCREEN_TYPE =
            "screen-type";

    public static final String ACTION_CARRIER_SIGNAL_PCO_VALUE_DEBUG =
            "com.mediatek.ims.pco.ui.ACTION_CARRIER_SIGNAL_PCO_VALUE_DEBUG";

    public static final String OPERATOR_APPLICATION_SETTING =
            "com.android.settings.OPERATOR_APPLICATION_SETTING";

    public static final String USE_WIFI_NW_ONLY = "useWifiNWOnly";
    public static final String USE_MOBILE_NW_ONLY = "useMobileNW";
    public static final String ACTIVATED = "Activated";
    public static final String NOT_ACTIVATED = "notActivated";
}
