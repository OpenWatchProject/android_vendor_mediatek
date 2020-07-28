/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.providers.contacts;

import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.os.SystemProperties;
import android.telephony.TelephonyManager;

import com.android.providers.contacts.ContactsDatabaseHelper.AccountsColumns;
import com.android.providers.contacts.ContactsDatabaseHelper.RawContactsColumns;
import com.android.providers.contacts.ContactsDatabaseHelper.Tables;

/**
 * M: add for Contacts Provider.
 */
public class ContactsProviderUtils {

    private static final String PROPERTY_NAME_VOLTE_SUPPORT = "persist.vendor.volte_support";

    private static final String PROPERTY_NAME_IMS_SUPPORT = "persist.vendor.ims_support";

    private static final String PROPERTY_VALUE_ENABLE = "1";

    /**
     * [VOLTE/IMS] whether VOLTE feature enabled on this device.
     * @return ture if allowed to enable
     */
    public static boolean isVolteEnabled() {
        return isFeatureEnabled(PROPERTY_NAME_VOLTE_SUPPORT);
    }

    /**
     * [VOLTE/IMS] whether ImsCall feature enabled on this device.
     * @return ture if allowed to enable
     */
    public static boolean isImsCallEnabled() {
        return isFeatureEnabled(PROPERTY_NAME_IMS_SUPPORT);
    }

    private static boolean isFeatureEnabled(String property) {
        return PROPERTY_VALUE_ENABLE.equals(SystemProperties.get(property));
    }

    public static boolean hasPresenceRawContact(SQLiteDatabase db, long rawId1, long rawId2) {
       return isRawContactPresence(db, rawId1) || isRawContactPresence(db, rawId2);
    }

    private static boolean isRawContactPresence(SQLiteDatabase db, long rawContactId) {
        String columnName = AccountsColumns.ACCOUNT_TYPE;
        Cursor c = db.query(
                Tables.RAW_CONTACTS_JOIN_ACCOUNTS,
                new String[] { columnName },
                RawContactsColumns.CONCRETE_ID + "=?",
                new String[]{ Long.toString(rawContactId) },
                null,
                null,
                null);
        try {
            if (c.moveToFirst()) {
                String accountType = c.getString(c.getColumnIndexOrThrow(columnName));
                if (AccountUtils.isPresenceAccount(accountType)) {
                    return true;
                }
            }
        } finally {
            c.close();
        }
        return false;
    }

}