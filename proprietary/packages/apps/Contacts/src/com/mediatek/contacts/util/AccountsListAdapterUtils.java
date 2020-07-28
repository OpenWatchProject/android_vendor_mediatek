/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.contacts.util;

import android.content.Context;
import android.graphics.drawable.Drawable;
import android.os.UserHandle;
import android.text.TextUtils;
import android.text.TextUtils.TruncateAt;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import com.android.contacts.model.account.AccountInfo;
import com.android.contacts.model.account.AccountType;
import com.android.contacts.model.account.AccountWithDataSet;
import com.android.contacts.model.account.ExchangeAccountType;

import com.mediatek.contacts.ContactsSystemProperties;
import com.mediatek.contacts.model.account.AccountWithDataSetEx;
import com.mediatek.contacts.simcontact.PhbInfoUtils;
import com.mediatek.contacts.simcontact.SimCardUtils;
import com.mediatek.contacts.simcontact.SubInfoUtils;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

/**
 * List-Adapter for Account selection
 */
public final class AccountsListAdapterUtils {
    private static final String TAG = "AccountsListAdapterUtils";

    public static ArrayList<AccountInfo> getGroupAccount(
            List<AccountInfo> accountInfoList) {
        List<AccountInfo> newAccountList = new ArrayList<AccountInfo>();
        Log.i(TAG, "[getGroupAccount]accountInfoList size:" + accountInfoList.size());
        for (AccountInfo accountInfo : accountInfoList) {
            if (accountInfo.getAccount() instanceof AccountWithDataSetEx) {
                int subId = ((AccountWithDataSetEx) accountInfo.getAccount()).getSubId();
                Log.d(TAG, "[getGroupAccount]subId:" + subId);
                if (SimCardUtils.isUsimType(subId)) {
                    Log.d(TAG, "[getGroupAccount]getUsimGroupMaxNameLength:"
                                    + PhbInfoUtils.getUsimGroupMaxNameLength(subId));
                    if (PhbInfoUtils.getUsimGroupMaxNameLength(subId) > 0) {
                        newAccountList.add(accountInfo);
                    }
                }
            } else {
                newAccountList.add(accountInfo);
            }
        }
        return new ArrayList<AccountInfo>(newAccountList);
    }

    /**
     * @param context
     *            the context for resrouce
     * @param account
     *            the current account data set
     * @param accountType
     *            the current account type
     * @param text2
     *            the sim name
     * @param icon
     *            the sim icon
     */
    public static void getViewForName(Context context, AccountWithDataSet account,
            AccountType accountType, HashMap<Integer, Drawable> subIconCache,
            TextView text2, ImageView icon) {
        int subId = -1;
        if (account instanceof AccountWithDataSetEx) {
            subId = ((AccountWithDataSetEx) account).getSubId();
            String displayName = ((AccountWithDataSetEx) account).getDisplayName();
            Log.d(TAG, "[getViewForName]displayName=" + Log.anonymize(displayName));
            if (TextUtils.isEmpty(displayName)) {
                displayName = account.name;
            }
            text2.setText(displayName);
        } else {
            text2.setText(account.name);
        }
        boolean isLocalPhone = AccountWithDataSetEx.isLocalPhone(accountType.accountType);
        Log.d(TAG, "[getViewForName]isLocalPhone:" + isLocalPhone
                + "accountType.accountType = " + accountType.accountType
                + ",account.name = " + Log.anonymize(account.name));
        if (isLocalPhone) {
            text2.setVisibility(View.GONE);
        } else {
            text2.setVisibility(View.VISIBLE);
        }
        if (ExchangeAccountType.isExchangeType(accountType.accountType)) {
            text2.setVisibility(View.VISIBLE);
        }
        text2.setEllipsize(TruncateAt.MIDDLE);
        if (accountType != null && accountType.isIccCardAccount()) {
            // [ALPS04414051] 3/4
            if (null == subIconCache.get(subId)) {
                subIconCache.put(subId, accountType.getDisplayIconBySubId(context, subId));
            }
            icon.setImageDrawable(subIconCache.get(subId));
        } else {
            icon.setImageDrawable(accountType.getDisplayIcon(context));
        }
    }

}
