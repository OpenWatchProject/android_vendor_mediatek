/*
 * Copyright (C) 2014 MediaTek Inc.
 * Modification based on code covered by the mentioned copyright
 * and/or permission notice(s).
 */

package com.mediatek.provider;

public class MtkCallLog {

    public static class Calls extends android.provider.CallLog.Calls {
        /**
         * An opaque value that indicate contact store location.
         * "-1", indicates phone contacts, others, indicate sim id of a sim contact
         * @hide
         */
        public static final String CACHED_INDICATE_PHONE_SIM = "indicate_phone_or_sim_contact";

        /**
         * For SIM contact's flag, SDN's contacts value is 1,
         * ADN's contacts value is 0 card.
         * @hide
         */
        public static final String CACHED_IS_SDN_CONTACT = "is_sdn_contact";
    }

}