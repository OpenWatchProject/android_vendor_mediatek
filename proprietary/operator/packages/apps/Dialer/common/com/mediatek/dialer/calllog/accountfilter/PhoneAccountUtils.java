/*
 * Copyright (C) 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License
 */

package com.mediatek.dialer.calllog.accountfilter;

import android.content.Context;
import android.telecom.PhoneAccount;
import android.telecom.PhoneAccountHandle;
import android.telecom.TelecomManager;
import android.text.TextUtils;

import java.util.List;

/**
 * Methods to help extract {@code PhoneAccount} information from database and Telecomm sources.
 */
public class PhoneAccountUtils {

    /**M: [Call Log Account Filter].
     * Extract account label from PhoneAccount object.
     * @param context context
     * @param accountHandle accountHandle
     * @return account label
     */
    public static String getAccountLabel(Context context, PhoneAccountHandle accountHandle) {
        PhoneAccount account = getAccountOrNull(context, accountHandle);
        if (account != null && account.getLabel() != null) {
            return account.getLabel().toString();
        }
        return null;
    }

    public static int getAccountColor(Context context, PhoneAccountHandle accountHandle) {
        PhoneAccount account = getAccountOrNull(context, accountHandle);
        if (account != null) {
            return account.getHighlightColor();
        }
        return -1;
    }

    /**M: [Call Log Account Filter].
     * Retrieve the account metadata, but if the account does not exist or the device has only a
     * single registered and enabled account, return null.
     * @param context context
     * @param acctHandle acctHandle
     * @return PhoneAccount
     */
     static PhoneAccount getAccountOrNull(Context context,
            PhoneAccountHandle accountHandle) {
        TelecomManager telecomManager =
                (TelecomManager) context.getSystemService(Context.TELECOM_SERVICE);
        final PhoneAccount account = telecomManager.getPhoneAccount(accountHandle);
        if (telecomManager.getCallCapablePhoneAccounts().size() <= 1) {
            return null;
        }
        return account;
    }

    /** M: [Call Log Account Filter].
     * Get PhoneAccount
     * @param context context
     * @param acctHandle acctHandle
     * @return PhoneAccount
     */
    public static PhoneAccount getPhoneAccount(Context context,
            PhoneAccountHandle acctHandle) {
        TelecomManager telecomManager = (TelecomManager) context
                .getSystemService(Context.TELECOM_SERVICE);
        return telecomManager.getPhoneAccount(acctHandle);
    }

    /** M: [Call Log Account Filter].
     * Get AccountNumber
     * @param context context
     * @param phoneAccount phoneAccount
     * @return phoneAccountNumber
     */
    public static String getAccountNumber(Context context,
            PhoneAccountHandle phoneAccount) {
        final PhoneAccount account = getPhoneAccount(context, phoneAccount);
        if (account == null) {
            return null;
        }
        return account.getAddress().getSchemeSpecificPart();
    }

    /** M: [Call Log Account Filter].
     * Verify whether has multiple accounts
     * @param context context
     * @return multiple of not
     */
    public static boolean hasMultipleCallCapableAccounts(Context context) {
        TelecomManager telecomManager = (TelecomManager) context
                .getSystemService(Context.TELECOM_SERVICE);
        return telecomManager.getCallCapablePhoneAccounts().size() > 1;
    }

    /** M: [Call Log Account Filter].
     * Retrieve the account by given id, return null if not find
     * @param context context
     * @param id id
     * @return phoneAccountHandle
     */
    public static PhoneAccountHandle getPhoneAccountById(Context context, String id) {
        if (!TextUtils.isEmpty(id)) {
            final TelecomManager telecomManager = (TelecomManager) context
                    .getSystemService(Context.TELECOM_SERVICE);
            final List<PhoneAccountHandle> accounts = telecomManager.getCallCapablePhoneAccounts();
            for (PhoneAccountHandle account : accounts) {
                if (id.equals(account.getId())) {
                    return account;
                }
            }
        }
        return null;
    }
}
