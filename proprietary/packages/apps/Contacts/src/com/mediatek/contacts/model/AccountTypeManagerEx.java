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
package com.mediatek.contacts.model;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.IntentFilter;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.UserHandle;
import android.telephony.SubscriptionInfo;
import android.text.TextUtils;

import com.android.contacts.model.account.AccountWithDataSet;
import com.android.internal.telephony.TelephonyIntents;

import com.mediatek.contacts.GlobalEnv;
import com.mediatek.contacts.model.account.AccountWithDataSetEx;
import com.mediatek.contacts.simcontact.SimCardUtils;
import com.mediatek.contacts.simcontact.SubInfoUtils;
import com.mediatek.contacts.util.AccountTypeUtils;
import com.mediatek.contacts.util.Log;
import com.mediatek.internal.telephony.IMtkTelephonyEx;

import java.util.List;
import java.util.Set;

public class AccountTypeManagerEx {
    private static final String TAG = "AccountTypeManagerEx";

    public static void registerReceiverOnSimStateAndInfoChanged(Context context,
            BroadcastReceiver broadcastReceiver) {
        Log.i(TAG, "[registerReceiverOnSimStateAndInfoChanged]...");
        IntentFilter simFilter = new IntentFilter();
        // For SIM Info Changed
        simFilter.addAction(TelephonyIntents.ACTION_PHB_STATE_CHANGED);
        context.registerReceiver(broadcastReceiver, simFilter);
    }

    public static void loadSimAndLocalAccounts(final Set<AccountWithDataSet> allAccounts) {
        addLocalAccount(allAccounts);
        if (UserHandle.myUserId() == UserHandle.USER_OWNER) {
            loadIccAccount(allAccounts);
        }
    }

    /**
     * fix ALPS00258229. Add Phone Local Type. if it is tablet let accountName is tablet
     */
    private static void addLocalAccount(Set<AccountWithDataSet> localAccounts) {
        final boolean isAddTabletAccount = GlobalEnv.isUsingTwoPanes();
        if (isAddTabletAccount) {
            localAccounts.add(new AccountWithDataSet("Tablet",
                    AccountTypeUtils.ACCOUNT_TYPE_LOCAL_PHONE, null));
        } else {
            localAccounts.add(new AccountWithDataSet("Phone",
                    AccountTypeUtils.ACCOUNT_TYPE_LOCAL_PHONE, null));
        }
    }

    /**
     * load Icc Account.when received broadcast which had registed in AccountTypeManager,
     * such as phb change,it will load all account. for Icc account, we use addIccAccount
     * method to add just in phb ready state by using subId.
     */
    private static void loadIccAccount(Set<AccountWithDataSet> localAccounts) {
        List<SubscriptionInfo> subscriptionInfoList = SubInfoUtils.getActivatedSubInfoList();
        if (subscriptionInfoList != null && subscriptionInfoList.size() > 0) {
            for (SubscriptionInfo subscriptionInfo : subscriptionInfoList) {
                int subId = subscriptionInfo.getSubscriptionId();
                boolean isPhbReady = SimCardUtils.isPhoneBookReady(subId);
                Log.i(TAG, "[loadIccAccount]subId: " + subId + ",phbReady:" + isPhbReady);
                if (isPhbReady && hasValidSimStorage(subId)) {
                    addIccAccount(localAccounts, subId);
                }
            }
        } else {
            Log.w(TAG, "[loadIccAccount]No valid subscriptionInfoList: " + subscriptionInfoList);
        }
    }

    /**
     * add Icc Account. the Icc card info is enable when phb is ready for related subId
     * of Icc card,.so we should add a Icc account just in phb ready state by using subId.
     */
    private static void addIccAccount(Set<AccountWithDataSet> localAccounts, int subId) {
        String accountName = AccountTypeUtils.getAccountNameUsingSubId(subId);
        String accountType = AccountTypeUtils.getAccountTypeBySub(subId);
        if (!TextUtils.isEmpty(accountName) && !TextUtils.isEmpty(accountType)) {
            localAccounts.add(new AccountWithDataSetEx(accountName, accountType, subId));
            Log.d(TAG, "[addIccAccount] new AccountWithDataSetEx, subId:" + subId
                    +", AccountName:" + Log.anonymize(accountName)
                    + ", AccountType: " + accountType);
        }
    }

    private static boolean hasValidSimStorage(int subId) {
        int[] storageInfos = null;
        try {
            IMtkTelephonyEx phoneEx = IMtkTelephonyEx.Stub.asInterface(ServiceManager
                    .checkService("phoneEx"));
            if (phoneEx != null) {
                storageInfos = phoneEx.getAdnStorageInfo(subId);
                if (storageInfos == null) {
                    Log.e(TAG, "[hasValidSimStorage]storageInfos = null.");
                    return false;
                }
                //storageInfos[1] is total enties in this sim card.
                if (storageInfos[1] == 0) {
                    Log.d(TAG, "[hasValidSimStorage] not support storage:" + storageInfos[1]);
                    return false;
                } else if (storageInfos[1] < 0) {
                    Log.d(TAG, "[hasValidSimStorage] invalid:" + storageInfos[1]);
                }
            } else {
                Log.e(TAG, "[hasValidSimStorage] phoneEx is null");
                return false;
            }
        } catch (RemoteException ex) {
            Log.e(TAG, "[hasValidSimStorage] exception: ", ex);
            return false;
        }
        return true;
    }
}
