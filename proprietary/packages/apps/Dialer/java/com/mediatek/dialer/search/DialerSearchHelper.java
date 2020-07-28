/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2011. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */

package com.mediatek.dialer.search;

import android.content.ContentResolver;
import android.content.Context;
import android.database.Cursor;
import android.database.MatrixCursor;
import android.net.Uri;
import android.net.Uri.Builder;
import android.provider.BaseColumns;
import android.provider.ContactsContract;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.ContactsContract.RawContacts;
import android.telephony.PhoneNumberUtils;
import android.text.TextUtils;
import android.util.Log;

import com.android.dialer.database.Database;
import com.google.common.base.Preconditions;
import com.google.common.collect.Lists;
import com.mediatek.dialer.database.DialerDatabaseHelperEx;
import com.mediatek.dialer.database.DialerDatabaseHelperEx.CallLogInfo;
import com.mediatek.dialer.database.DialerDatabaseHelperEx.ContactNumber;
import com.mediatek.dialer.search.DialerSearchNameMatcher;
import com.mediatek.dialer.util.DialerFeatureOptions;

import java.util.ArrayList;
import java.util.List;

/**
 * M: [MTK Dialer Search] Data query helper class
 */
public class DialerSearchHelper {
    private static final String TAG = "DialerSearchHelper";
    private static DialerSearchHelper sSingleton = null;
    private final Context mContext;

    public interface DialerSearchQuery {
        String[] COLUMNS = new String[] {
                DialerSearch.NAME_LOOKUP_ID,
                DialerSearch.CONTACT_ID,
                "data_id",
                DialerSearch.CALL_DATE,
                DialerSearch.CALL_LOG_ID,
                DialerSearch.CALL_TYPE,
                DialerSearch.CALL_GEOCODED_LOCATION,
                "phoneAccoutId",
                "phoneAccountComponentName",
                "presentation",
                DialerSearch.INDICATE_PHONE_SIM,
                DialerSearch.CONTACT_STARRED,
                DialerSearch.PHOTO_ID,
                DialerSearch.SEARCH_PHONE_TYPE,
                "numberLabel",
                DialerSearch.NAME,
                DialerSearch.SEARCH_PHONE_NUMBER,
                DialerSearch.CONTACT_NAME_LOOKUP,
                DialerSearch.IS_SDN_CONTACT,
                DialerSearch.MATCHED_DATA_OFFSET,
                DialerSearch.MATCHED_NAME_OFFSET
        };
    }

    private DialerSearchHelper(Context context) {
        mContext = Preconditions.checkNotNull(context, "Context must not be null");
    }

    /**
     * Access function to get the singleton instance of DialerDatabaseHelper.
     */
    public static synchronized DialerSearchHelper getInstance(Context context) {
        Log.d(TAG, "Getting Instance");

        if (sSingleton == null) {
            // Use application context instead of activity context because this
            // is a singleton,
            // and we don't want to leak the activity if the activity is not
            // running but the
            // dialer database helper is still doing work.
            sSingleton = new DialerSearchHelper(context.getApplicationContext());
        }
        return sSingleton;
    }

    private Object[] buildCursorRecord(long id, long contactId, long dataId, String callDate,
            long callLogId, int callType, String geo, String phoneAccountId,
            String phoneAccountComponentName, int presentation, int simIndicator, int starred,
            long photoId, int numberType, String numberLabel, String name, String number,
            String lookup, int isSdn, String isRegularSearch, String nameOffset) {
        Object[] record = new Object[] { id, contactId, dataId, callDate, callLogId, callType, geo,
                phoneAccountId, phoneAccountComponentName, presentation, simIndicator, starred,
                photoId, numberType, numberLabel, name, number, lookup, isSdn, isRegularSearch,
                nameOffset };
        return record;
    }

    private Cursor buildCursor(Object[][] cursorValues) {
        MatrixCursor c = new MatrixCursor(DialerSearchQuery.COLUMNS);
        if (cursorValues != null) {
            for (Object[] record : cursorValues) {
                if (record == null) {
                    break;
                }
                c.addRow(record);
            }
        }
        return c;
    }

    /**
     * search the dialer db.
     * @param dsnm
     * @param query
     */
    public Cursor getSmartDialerSearchResults(String query, DialerSearchNameMatcher dsnm) {
        /** Loads results from the database helper. */
        final DialerDatabaseHelperEx dialerDatabaseHelper = Database.get(mContext)
                .getDialerSearchDbHelper(mContext);
        final ArrayList<ContactNumber> allMatches = dialerDatabaseHelper.getLooseMatches(query,
                dsnm);
        int callLogMaxEntries = DialerDatabaseHelperEx.MAX_ENTRIES - allMatches.size();
        /// 500 is the max length of callLog provider storage
        final ArrayList<CallLogInfo> callLogMatches = dialerDatabaseHelper.getCallLogMatch(query,
                dsnm, 500);
        /** Constructs a cursor for the returned array of results. */
        int count = allMatches.size() + callLogMatches.size();
        Object[][] objectMap = new Object[count][];
        /// used to filter the call log which has the same number with contact
        ArrayList<String> numberList = Lists.newArrayList();
        int cursorPos = 0;
        for (ContactNumber contact : allMatches) {
            objectMap[cursorPos++] = buildCursorRecord(0, contact.id, contact.dataId, null, 0, 0,
                    null, null, null, 0, contact.indicatePhoneSim/* simIndicate */, 0,
                    contact.photoId, contact.type/* type */, contact.label, contact.displayName,
                    contact.phoneNumber, contact.lookupKey, contact.isSdnContact/* isSdn */,
                    /**data_offset*/contact.dataOffset, /**name_offset*/contact.nameOffset);
            if (!TextUtils.isEmpty(contact.phoneNumber)) {
                numberList.add(contact.phoneNumber);
            }
        }

        for (CallLogInfo callLog : callLogMatches) {
            if (!numberList.contains(callLog.callNumber) && callLogMaxEntries > 0) {
                objectMap[cursorPos++] = buildCursorRecord(0, 0, 0,
                        /*callDate*/callLog.date, callLog.callLogId, callLog.type,
                        callLog.geoLocation, callLog.phoneAccountId,
                        callLog.phoneAccountComponent, 0, 0/* simIndicate */,
                        0, 0, 0/* type */, null, null, callLog.callNumber, null, 0/* isSdn */,
                        /**data_offset*/callLog.dataOffset, /**name_offset*/null);
                callLogMaxEntries--;
            }
        }
        Cursor cursor = buildCursor(objectMap);
        
        return cursor;
    }


    

    /**
     * Columns for dialer search displayed information
     * Copy from ContactsContract
     * @hide
     */
    public static final class DialerSearch implements BaseColumns, ViewDialerSearchColumns {
        /**
         * The index for highlight number.
         * @hide
         * @internal
         */
        public static final String MATCHED_DATA_OFFSET = "matched_data_offset"; //For results

        /**
         * The index for highlight name.
         * @hide
         * @internal
         */
        public static final String MATCHED_NAME_OFFSET = "matched_name_offset";
        private DialerSearch() {
        }
    }
    /**
     * View dialer Search columns
     * @hide
     */
    protected interface ViewDialerSearchColumns {
        public static final String NAME_LOOKUP_ID = "_id";
        public static final String CONTACT_ID = "vds_contact_id";
        public static final String RAW_CONTACT_ID = "vds_raw_contact_id";
        public static final String NAME = "vds_name";
        public static final String NUMBER_COUNT = "vds_number_count";
        public static final String CALL_LOG_ID = "vds_call_log_id";
        public static final String CALL_TYPE = "vds_call_type";
        // To check whether CALL_DATE can be removed
        public static final String CALL_DATE = "vds_call_date";
        public static final String CALL_GEOCODED_LOCATION = "vds_geocoded_location";
        public static final String SIM_ID = "vds_sim_id";
        public static final String VTCALL = "vds_vtcall";
        public static final String SEARCH_PHONE_NUMBER = "vds_phone_number";
        public static final String SEARCH_PHONE_TYPE = "vds_phone_type";
        public static final String CONTACT_NAME_LOOKUP = "vds_lookup";
        public static final String PHOTO_ID = "vds_photo_id";
        public static final String CONTACT_STARRED = "vds_starred";
        public static final String INDICATE_PHONE_SIM = "vds_indicate_phone_sim";
        public static final String IS_SDN_CONTACT = "vds_is_sdn_contact";
        public static final String SORT_KEY_PRIMARY = "vds_sort_key";
        public static final String SORT_KEY_ALTERNATIVE = "vds_sort_key_alternative";
        public static final String SEARCH_DATA_OFFSETS = "search_data_offsets";
        public static final String NAME_ALTERNATIVE = "vds_name_alternative";
        public static final String SEARCH_DATA_OFFSETS_ALTERNATIVE =
            "search_data_offsets_alternative";
        public static final String NAME_ID = "vds_name_id";
        public static final String NUMBER_ID = "vds_number_id";
        public static final String DS_DATA1 = "vds_data1";
        public static final String DS_DATA2 = "vds_data2";
        public static final String DS_DATA3 = "vds_data3";
        // substitute phone_account_id,phone_account_component_name for KK sim_id
        public static final String PHONE_ACCOUNT_ID = "vds_phone_account_id";
        public static final String PHONE_ACCOUNT_COMPONENT_NAME =
            "vds_phone_account_component_name";
        // add presentation field for read number presentation from dialer
        public static final String NUMBER_PRESENTATION = "vds_number_presentation";
        // add search_phone_label field for read number label information from dialer
        public static final String SEARCH_PHONE_LABEL = "vds_search_phone_label";
    }
}
