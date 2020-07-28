/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
package com.mediatek.provider;

import android.annotation.ProductApi;
import android.content.ContentResolver;
import android.content.Context;
import android.os.Bundle;
import android.provider.ContactsContract;
import android.text.TextUtils;

public class MtkContactsContract {

    public interface ContactsColumns {
        /**
         * An opaque value that indicate contact store location.
         * "-1", indicates phone contacts
         * others, indicate sim id of a sim contact
         *
         * @hide
         */

        public static final String INDICATE_PHONE_SIM = "indicate_phone_or_sim_contact";

        /**
         * For a SIM/USIM contact, this value is its index in the relative SIM
         * card.
         *
         * @hide
         */
        public static final String INDEX_IN_SIM = "index_in_sim";

        /**
         * Whether the contact should always be sent to voicemail for VT. If
         * missing, defaults to false.
         * <P>
         * Type: INTEGER (0 for false, 1 for true)
         * </P>
         *
         * @hide
         */
        public static final String SEND_TO_VOICEMAIL_VT = "send_to_voicemail_vt";

        /**
         * Whether the contact should always be sent to voicemail for SIP. If
         * missing, defaults to false.
         * <P>
         * Type: INTEGER (0 for false, 1 for true)
         * </P>
         *
         * @hide
         */
        public static final String SEND_TO_VOICEMAIL_SIP = "send_to_voicemail_sip";

        /**
         * To filter the Contact for Widget.
         *
         * @hide
         */
        public static final String FILTER = "filter";

        /**
         * To filter the Contact for Widget.
         *
         * @hide
         */
        public static final int FILTER_NONE = 0;

        /**
         * To filter the Contact for Widget.
         *
         * @hide
         */
        public static final int FILTER_WIDGET = 1;

        /**
         * For SIM contact's flag, SDN's contacts value is 1, ADN's contacts value is 0
         * card.
         *
         * @hide
         */
        public static final String IS_SDN_CONTACT = "is_sdn_contact";
    }

    public interface RawContactsColumns {
        /**
         * An opaque value that indicate contact store location.
         * "-1", indicates phone contacts
         * others, indicate sim id of a sim contact
         *
         * @hide
         */
        public static final String INDICATE_PHONE_SIM = "indicate_phone_or_sim_contact";

        /**
         * For a SIM/USIM contact, this value is its index in the relative SIM
         * card.
         *
         * @hide
         */
        public static final String INDEX_IN_SIM = "index_in_sim";

        /**
         * Whether the contact should always be sent to voicemail for VT. If
         * missing, defaults to false.
         * <P>
         * Type: INTEGER (0 for false, 1 for true)
         * </P>
         *
         * @hide
         */
        public static final String SEND_TO_VOICEMAIL_VT = "send_to_voicemail_vt";

        /**
         * Whether the contact should always be sent to voicemail for SIP. If
         * missing, defaults to false.
         * <P>
         * Type: INTEGER (0 for false, 1 for true)
         * </P>
         *
         * @hide
         */
        public static final String SEND_TO_VOICEMAIL_SIP = "send_to_voicemail_sip";

        /**
         * M:
         * For SIM contact's flag, SDN's contacts value is 1, ADN's contacts value is 0
         * card.
         *
         * @hide
         */
        public static final String IS_SDN_CONTACT = "is_sdn_contact";
    }

    public static final class RawContacts  {
        /**
         * Indicate flag: Indicate it is a phone contact.
         * @hide
         * @internal
         */
        public static final int INDICATE_PHONE = -1;

        /**
         * time stamp that is updated whenever version changes.
         * <P>
         * Type: INTEGER
         * </P>
         *
         * @hide
         * @internal
         */
        public static final String TIMESTAMP = "timestamp";
    }

    public interface DataColumns {
        /**
         * M: Code add by Mediatek inc.
         * @hide
         */
        public static final String IS_ADDITIONAL_NUMBER = "is_additional_number";
    }

    public static final class CommonDataKinds {
        public static final class Phone {
            /**
             * M: Add for AAS, call this API to get common or AAS label.
             * @param context context
             * @param type phone type
             * @param label label
             * @return AAS label if type is PHONE_TYPE_AAS.
             *
             * @hide
             */
            @ProductApi
            public static final CharSequence getTypeLabel(Context context, int type,
                    CharSequence label) {
                if (type == Aas.PHONE_TYPE_EMPTY) {
                    return Aas.LABEL_EMPTY;
                } else if (type == Aas.PHONE_TYPE_AAS) {
                    if (!TextUtils.isEmpty(label)) {
                        return Aas.getLabel(context.getContentResolver(), label);
                    } else {
                        return Aas.LABEL_EMPTY;
                    }
                } else {
                    return ContactsContract.CommonDataKinds.Phone.getTypeLabel(context.getResources(), type, label);
                }
            }
        }

        /**
         * M: used to support VOLTE IMS Call feature.
         *
         * @hide
         */
        public static final class ImsCall {
            /**
             * This utility class cannot be instantiated.
             */
            private ImsCall() {}

            /**
             * MIME type used when storing this in data table.
             * @internal
             */
            public static final String CONTENT_ITEM_TYPE = "vnd.android.cursor.item/ims";

            /**
             * The data for the contact method.
             * <P>Type: TEXT</P>
             */
            public static final String DATA = "data1";

            /**
             * The type of data, for example Home or Work.
             * <P>Type: INTEGER</P>
             */
            public static final String TYPE = "data2";

            /**
             * The user defined label for the contact method.
             * <P>Type: TEXT</P>
             */
            public static final String LABEL = "data3";

            /**
             * The ImsCall URL string.
             * <P>Type: TEXT</P>
             * @internal
             */
            public static final String URL = DATA;
        }
    }

    public static final class Groups {
        /**
         * M:
         * Used with ContactsContract.Contacts.CONTENT_GROUP_URI to query by group ID.
         *
         * @hide
         */
        public static final String QUERY_WITH_GROUP_ID = "query_with_group_id";
    }

    /**
     * Used by ContactsProvider for PhoneLookupProjectionMap
     */
    public static final class PhoneLookup implements ContactsColumns {
    }

    /**
     * Class defined for AAS (Additional number Alpha String).
     */
    public static final class Aas {

        /**
         * @internal
         */
        public static final int PHONE_TYPE_AAS = 101;

        /**
         * Type for primary number which has no phone type.
         * @internal
         */
        public static final int PHONE_TYPE_EMPTY = 102;

        /**
         * Label for empty type.
         * @internal
         */
        public static final String LABEL_EMPTY = "";

        /**
         * The method to get AAS.
         * @internal
         */
        public static final String AAS_METHOD = "get_aas";

        /**
         * The key to retrieve from the returned Bundle to obtain the AAS label.
         * @internal
         */
        public static final String KEY_AAS = "aas";

        /**
         * The symbol to separate sub id and index when build AAS indicator.
         * @internal
         */
        public static final String ENCODE_SYMBOL = "-";

        /**
         * The function to build AAS indicator.
         * ex. subId is 1 and index is 2 then the indicator will be "1-2".
         * @param subId sub ID
         * @param indexInSim index in SIM
         * @return indicator
         * @internal
         */
        @ProductApi
        public static final String buildIndicator(int subId, int indexInSim) {
            return new StringBuffer().append(subId).append(ENCODE_SYMBOL).append(indexInSim)
                    .toString();
        }

        /**
         * The function to get AAS, return empty string if not exsit.
         * @param resolver content resolver
         * @param indicator it contains sub id and aas index info.
         * @return return aas label
         * @internal
         */
        @ProductApi
        public static CharSequence getLabel(ContentResolver resolver, CharSequence indicator) {
            Bundle response = resolver.call(ContactsContract.AUTHORITY_URI, AAS_METHOD,
                    indicator.toString(), null);
            if (response != null) {
                return response.getCharSequence(KEY_AAS, "");
            }
            return "";
        }
    }
}
