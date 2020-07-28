package com.mediatek.digits;

public class DigitsConst {

    /**
     * Service is gone
     */
    public static final int STATE_UNKNOWN = -1;

    /**
     * Service isn't ready (not yet on boarding)
     */
    public static final int STATE_IDLE = 0;

    /**
     * Service is ready (on boarding), not yet login (subscribe)
     */
    public static final int STATE_READY = 1;

    /**
     * Service is ready (on boarding) and in login status (subscribed)
     */
    public static final int STATE_SUBSCRIBED = 2;

    /**
     * Service is in loging out status (during logout)
     */
    public static final int STATE_LOGOUT = 3;

    /**
     * Jansky_service is unknown
     */
    public static final int JANSKY_SERVICE_UNKNOWN = 0;

    /**
     * Jansky_service is false
     */
    public static final int JANSKY_SERVICE_OFF = 1;

    /**
     * Jansky_service is true
     */
    public static final int JANSKY_SERVICE_ON = 2;


    /** callback to notify service is connected
     *
     */
    public static final int EVENT_SERVICE_CONNECTION_RESULT = 0;

    /**
     * callback to notify service state changed
     * @see EXTRA_INT_SERVICE_STATE
     * @see EXTRA_INT_JANSKY_SERVICE
     */
    public static final int EVENT_SERVICE_STATE_CHANGED = 1;

    /**
     * [Push notification] callback to notify call is answered
     *
     * @see EXTRA_STRING_DEVICE_ID
     */
    public static final int EVENT_CALL_ANSWERED = 2;

    /**
     * Callback to notify deviced list is changed
     *
     * @see EXTRA_OBJECT_ARRAY_DIGIT_DEVICE
     */
    public static final int EVENT_REGISTERED_DEVICES_CHANGED = 3;

    /**
     * Callback to notify msisdn list is changed
     *
     * @see EXTRA_OBJECT_ARRAY_DIGIT_LINE
     */
    public static final int EVENT_REGISTERED_MSISDN_CHANGED = 4;


    /**
     * [Push notification] callback to notify device name is changed
     *
     * @see #EXTRA_STRING_DEVICE_NAME
     */
    public static final int EVENT_DEVICE_NAME_CHANGED = 5;

    /**
     * [Push notification] callback to notify line name is changed
     *
     * @see #EXTRA_STRING_MSISDN
     * @see #EXTRA_STRING_LINE_NAME
     */
    public static final int EVENT_LINE_NAME_CHANGED = 6;

    /**
     *
     * @see #EXTRA_STRING_SERVER_URL
     * @see #EXTRA_STRING_SERVER_DATA
     */
    public static final int EVENT_NO_E911_ADDRESS_ON_FILE = 7;

    /**
     * The key to look up for DigitsDevice
     */
    public static final String EXTRA_OBJECT_ARRAY_DIGITS_DEVICE = "digits_device_array";

    /**
     * The key to look up for DegitsLine
     */
    public static final String EXTRA_OBJECT_ARRAY_DIGITS_LINE = "digits_line_array";

    /**
     * The key to look up for DegitsLine
     */
    public static final String EXTRA_OBJECT_DIGITS_PROFILE = "digits_profile";


    /**
     * The key to look up for the Websheet server url.
     */
    public static final String EXTRA_STRING_SERVER_URL = "server_url";

    /**
     * The key to look up for post-data of websheet.
     */
    public static final String EXTRA_STRING_SERVER_DATA = "server_data";

    /**
     * The key to look up for device id
     */
    public static final String EXTRA_STRING_DEVICE_ID = "device_id";

    /**
     * The key to look up for device name
     */
    public static final String EXTRA_STRING_DEVICE_NAME = "device_name";

    /**
     * The key to look up for msisdn
     */
    public static final String EXTRA_STRING_MSISDN = "msisdn";

    /**
     * The key to look up for msisdn array
     */
    public static final String EXTRA_STRING_ARRAY_MSISDN = "msisdn_array";

    /**
     * The key to look up for result array
     */
    public static final String EXTRA_INT_ARRAY_RESULT = "result_array";

    /**
     * The key to look up for line name
     */
    public static final String EXTRA_STRING_LINE_NAME = "line_name";

    /**
     * The key to look up for line color
     */
    public static final String EXTRA_INT_LINE_COLOR = "line_color";

    /**
     * The key to look up for service state
     */
    public static final String EXTRA_INT_SERVICE_STATE = "service_state";

    /**
     * The key to look up jansky_service
     */
    public static final String EXTRA_INT_JANSKY_SERVICE = "jansky_service";

    /**
     * The value for the key: EXTRA_INT_RESULT
     */
    public static final int RESULT_SUCCEED = 0;
    public static final int RESULT_FAIL_UNKNOWN = 1;
    public static final int RESULT_FAIL_BUSY = 2;
    public static final int RESULT_FAIL_SERVICE_NOT_READY = 3;
    public static final int RESULT_FAIL_WRONG_STATE = 4;
    public static final int RESULT_FAIL_ACTIVE_CALL = 5;

    public static String eventToString(int event) {
        switch (event) {
            case EVENT_SERVICE_CONNECTION_RESULT:
                return "EVENT_SERVICE_CONNECTION_RESULT";
            case EVENT_SERVICE_STATE_CHANGED:
                return "EVENT_SERVICE_STATE_CHANGED";
            case EVENT_CALL_ANSWERED:
                return "EVENT_CALL_ANSWERED";
            case EVENT_REGISTERED_DEVICES_CHANGED:
                return "EVENT_REGISTERED_DEVICES_CHANGED";
            case EVENT_REGISTERED_MSISDN_CHANGED:
                return "EVENT_REGISTERED_MSISDN_CHANGED";
            case EVENT_DEVICE_NAME_CHANGED:
                return "EVENT_DEVICE_NAME_CHANGED";
            case EVENT_LINE_NAME_CHANGED:
                return "EVENT_LINE_NAME_CHANGED";
            case EVENT_NO_E911_ADDRESS_ON_FILE:
                return "EVENT_NO_E911_ADDRESS_ON_FILE";

            default:
                return "EVENT_UNKNOWN(" + event + ")";

        }
    }
}
