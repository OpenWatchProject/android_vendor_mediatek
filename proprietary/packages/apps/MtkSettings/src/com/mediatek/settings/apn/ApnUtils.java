
package com.mediatek.apn;

import android.provider.Telephony;
import mediatek.telephony.MtkTelephony;

public class ApnUtils {

    public static final String TAG = "Settings/ApnUtils";

    public static final int SIM_CARD_SINGLE = 0;
    // M: code review for value to be -2
    public static final int SIM_CARD_UNDEFINED = -2;
    // preferred APN URI
    public static final String PREFERRED_APN_URI = "content://telephony/carriers/preferapn";
    // for omacp receiver service
    public static final String ACTION_START_OMACP_SERVICE =
            "com.mediatek.apn.action.start.omacpservice";

    public static final String TRANSACTION_START = "com.android.mms.transaction.START";
    public static final String TRANSACTION_STOP = "com.android.mms.transaction.STOP";

    // the source type for apn
    public static final int SOURCE_TYPE_DEFAULT = 0; // the default configured
                                                     // by apns-config.xml
    public static final int SOURCE_TYPE_USER_EDIT = 1; // from user editing
    /**
     * Standard projection for the interesting columns of a normal note.
     */
    public static final String[] PROJECTION = new String[] {
            Telephony.Carriers._ID, // 0
            Telephony.Carriers.NAME, // 1
            Telephony.Carriers.APN, // 2
            Telephony.Carriers.PROXY, // 3
            Telephony.Carriers.PORT, // 4
            Telephony.Carriers.USER, // 5
            Telephony.Carriers.SERVER, // 6
            Telephony.Carriers.PASSWORD, // 7
            Telephony.Carriers.MMSC, // 8
            Telephony.Carriers.MCC, // 9
            Telephony.Carriers.MNC, // 10
            Telephony.Carriers.NUMERIC, // 11
            Telephony.Carriers.MMSPROXY, // 12
            Telephony.Carriers.MMSPORT, // 13
            Telephony.Carriers.AUTH_TYPE, // 14
            Telephony.Carriers.TYPE, // 15
            MtkTelephony.Carriers.SOURCE_TYPE, // 16
            MtkTelephony.Carriers.OMACP_ID, // 17
            MtkTelephony.Carriers.NAP_ID, // 18
            MtkTelephony.Carriers.PROXY_ID, // 19
    };

    public static final int ID_INDEX = 0;
    public static final int NAME_INDEX = 1;
    public static final int APN_INDEX = 2;
    public static final int PROXY_INDEX = 3;
    public static final int PORT_INDEX = 4;
    public static final int USER_INDEX = 5;
    public static final int SERVER_INDEX = 6;
    public static final int PASSWORD_INDEX = 7;
    public static final int MMSC_INDEX = 8;
    public static final int MCC_INDEX = 9;
    public static final int MNC_INDEX = 10;
    public static final int NUMERIC_INDEX = 11;
    public static final int MMSPROXY_INDEX = 12;
    public static final int MMSPORT_INDEX = 13;
    public static final int AUTH_TYPE_INDEX = 14;
    public static final int TYPE_INDEX = 15;
    public static final int SOURCE_TYPE_INDEX = 16;
    public static final int APN_ID_INDEX = 17;
    public static final int NAP_ID_INDEX = 18;
    public static final int PROXY_ID_INDEX = 19;

    public static String checkNotSet(String value) {
        if (value == null || value.length() == 0) {
            return "";
        } else {
            return value;
        }
    }

}
