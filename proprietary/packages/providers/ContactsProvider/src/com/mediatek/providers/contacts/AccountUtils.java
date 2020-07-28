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

import android.provider.ContactsContract.RawContacts;

/**
 * M: Contacts account utils, provide account info API.
 */
public class AccountUtils {

    // Account name for local phone & tablet
    public static final String ACCOUNT_NAME_LOCAL_PHONE = "Phone";
    public static final String ACCOUNT_NAME_LOCAL_TABLET = "Tablet";

    // Account type for local phone
    public static final String ACCOUNT_TYPE_LOCAL_PHONE = "Local Phone Account";

    // Account types for SIM
    private static final String ACCOUNT_TYPE_SIM = "SIM Account";
    private static final String ACCOUNT_TYPE_USIM = "USIM Account";
    private static final String ACCOUNT_TYPE_RUIM = "RUIM Account";
    private static final String ACCOUNT_TYPE_CSIM = "CSIM Account";

    // Account for Mediatek presence
    private static final String ACCOUNT_TYPE_PRESENCE = "com.mediatek.presence";

    // Local account type selection string, use to append SQLITE query statement.
    private static final String LOCAL_ACCOUNT_TYPES = "('" +
            ACCOUNT_TYPE_LOCAL_PHONE + "' , '" +
            ACCOUNT_TYPE_SIM + "' , '" +
            ACCOUNT_TYPE_USIM + "' , '" +
            ACCOUNT_TYPE_RUIM + "' , '" +
            ACCOUNT_TYPE_CSIM + "')";

    // Local account type collection string, use to append SQLITE query statement.
    private static final String LOCAL_SUPPORT_GROUP_ACCOUNT_TYPES = "('" +
            ACCOUNT_TYPE_LOCAL_PHONE + "' , '" +
            ACCOUNT_TYPE_USIM + "')";

    /**
     * return local account selection
     */
    public static String getLocalAccountSelection() {
        return "(" + RawContacts.ACCOUNT_NAME + " IS NULL AND " +
                RawContacts.ACCOUNT_TYPE + " IS NULL OR " +
                RawContacts.ACCOUNT_TYPE + " IN " + LOCAL_ACCOUNT_TYPES + ")";
    }

    /**
     * return sync accounts selection
     */
    public static String getSyncAccountSelection() {
        return "(" + RawContacts.ACCOUNT_TYPE + " NOT IN " + LOCAL_ACCOUNT_TYPES + ")";
    }

    /**
     * return local account type selection
     */
    public static String getLocalSupportGroupAccountSelection() {
        return "(" + RawContacts.ACCOUNT_NAME + " IS NULL AND " +
               RawContacts.ACCOUNT_TYPE + " IS NULL OR " +
               RawContacts.ACCOUNT_TYPE + " IN " + AccountUtils.LOCAL_SUPPORT_GROUP_ACCOUNT_TYPES +
               ")";
    }

    /**
     * Check if the account is local account
     */
    public static boolean isLocalAccount(String accountType, String accountName) {
        return (accountType == null && accountName == null)
                || (accountType != null && LOCAL_ACCOUNT_TYPES.contains(accountType));
    }

    /**
     * Check if the account is SIM account.
     */
    public static boolean isSimAccount(String accountType) {
        return accountType != null && LOCAL_ACCOUNT_TYPES.contains(accountType)
                && !accountType.equals(ACCOUNT_TYPE_LOCAL_PHONE);
    }

    /**
     * Check if the account is Mediatek presence account.
     */
    public static boolean isPresenceAccount(String accountType) {
        return accountType != null && accountType.equals(ACCOUNT_TYPE_PRESENCE);
    }
}
