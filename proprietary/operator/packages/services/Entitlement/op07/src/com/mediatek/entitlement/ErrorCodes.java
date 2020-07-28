package com.mediatek.entitlement;

/**
 * Created by MTK33184 on 8/25/16.
 */
public  class ErrorCodes {
    private static int[] sErrorCode;

    // defined by MTK internal, empty or invalid response.
    public static int INVALID_RESPONSE = -1;

    public static int REQUEST_SUCCESSFUL = 1000;
    public static int AKA_CHALLENGE = 1003;
    public static int INVALID_REQUEST = 1004;
    public static int INVALID_IP_AUTH = 1005;
    public static int AKA_AUTH_FAILED = 1006;
    public static int FORBIDDEN_REQUEST = 1007;
    public static int INVALID_CLIENT_ID = 1008;
    public static int MAX_DEVICES_REACHED = 1010;
    public static int UNKNOWN_DEVICE = 1020;
    public static int EPDG_NOT_FOUND = 1021;
    public static int CERT_FAILURE = 1022;
    public static int REMOVE_FAILURE = 1023;
    public static int INVALID_OWNER_ID = 1024;
    public static int INVALID_CSR = 1025;
    public static int DEVICE_LOCKED = 1028;
    public static int INVALID_DEVICE_STATUS = 1029;
    public static int MAX_SERVICES_REACHED = 1040;
    public static int INVALID_SERVICE_FINGERPRINT = 1041;
    public static int INVALID_TARGET_DEVICE = 1042;
    public static int INVALID_TARGET_USER = 1043;
    public static int MAX_SERVICE_INSTANCE_REACHED = 1044;
    public static int FORBIDDEN_COPY = 1045;
    public static int INVALID_SERVICE_NAME = 1046;
    public static int SERVICE_INVALID_IMSI = 1047;
    public static int SERVICE_NOT_ENTITLED = 1048;
    public static int SERVICE_NO_PERMISSION = 1049;
    public static int SERVICE_PARAMS_FAILURE = 1050;
    public static int REMOVE_SERVICE_FAILURE = 1052;
    public static int INVALID_SERVICE_INSTANCE_ID = 1053;
    public static int INVALID_DEVICE_GROUP = 1054;
    public static int NO_MSISDN = 1060;
    public static int ERROR_IN_MSISDN_CREATION = 1061;
    public static int MAX_MSISDN_EXCEEDED = 1062;
    public static int INVALID_MSISDN = 1070;
    public static int INVALID_PUSH_TOKEN = 1080;
    public static int SERVER_ERROR = 1111;
    public static int THREEGPP_AUTH_ONGOING = 1112;
    public static int ONGOING_REQUEST = 1500;
    public static int UNSUPPORTED_OPERATION = 9999;

    private static void init() {

        if (sErrorCode == null) {
            sErrorCode = new int[4];

            for (int i = 0; i < 4; i ++) {
                sErrorCode[i] = 0;
            }
        }

    }

    public static void setErrorCode(int slotId, int errCode){

        init();
        sErrorCode[slotId] = errCode;
    }

    public static int getErrorCode(int slotId){

        init();
        return sErrorCode[slotId];
    }
}
