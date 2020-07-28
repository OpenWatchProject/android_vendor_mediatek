package com.mediatek.internal.telephony.ppl;

import android.os.Bundle;

public interface IPplSmsFilter {
    String KEY_PDUS = "pdus";
    String KEY_FORMAT = "format";
    String KEY_SIM_ID = "simId";
    String KEY_SUB_ID = "subId";
    String KEY_SMS_TYPE = "smsType"; // 0 - MT, 1 - MO
    /*
     * telephony framework filling either pdus or these following 3 items.
     * pdus is preperentially.
     */
    String KEY_MSG_CONTENT = "msgContent";
    String KEY_SRC_ADDR = "srdAddr"; // for MT SMS
    String KEY_DST_ADDR = "dstAddr"; // for MO SMS

    /**
     * Whether need to filter the message out from saving & broadcasting.
     *
     * @param params
     * @return
     */
    boolean pplFilter(Bundle params);
}
