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
package com.mediatek.contacts.quickcontact;

import android.content.Context;
import android.text.TextUtils;

import com.android.contacts.group.GroupMetaData;
import com.android.contacts.model.Contact;
import com.mediatek.contacts.simcontact.PhbInfoUtils;
import com.mediatek.contacts.util.AccountTypeUtils;
import com.mediatek.contacts.util.Log;

import java.util.List;

/**
 * Extract some util method in QuickContactActivity to this class.
 */
public class QuickContactUtils {
    private static String TAG = "QuickContactUtils";

    /**
     * Get group title based on the groupId.
     *
     * @param groupMetaData
     *            List<GroupMetaData>
     * @param groupId
     *            long
     * @return group title
     */
    public static String getGroupTitle(List<GroupMetaData> groupMetaData, long groupId) {
        if (groupMetaData == null) {
            Log.w(TAG, "[getGroupTitle]groupMetaData is null,return. ");
            return null;
        }

        for (GroupMetaData group : groupMetaData) {
            if (group.groupId == groupId) {
                if (!group.defaultGroup && !group.favorites) {
                    String title = group.groupName;
                    if (!TextUtils.isEmpty(title)) {
                        return title;
                    }
                }
                break;
            }
        }

        return null;
    }

    /// M: [Sim Contact Flow]ALPS02782438. not support to show email for sim and Ruim type. @{
    public static boolean isSupportShowEmailData(Contact contactsData, Context context) {
        Log.sensitive(TAG, "[isSupportShowEmailData] data : " + contactsData);
        if (contactsData == null) {
            return false;
        }
        String accoutType = contactsData.getRawContacts().get(0).getAccountTypeString();
        Log.sensitive(TAG, "[isSupportShowEmailData] accoutType : " + accoutType);
        if (AccountTypeUtils.ACCOUNT_TYPE_SIM.equals(accoutType) ||
                AccountTypeUtils.ACCOUNT_TYPE_RUIM.equals(accoutType)) {
            Log.i(TAG, "[isSupportShowEmailData] Ruim or sim not support email! ");
            return false;
        }
        if (AccountTypeUtils.ACCOUNT_TYPE_USIM.equals(accoutType)) {
            String accountName = contactsData.getRawContacts().get(0).getAccountName();
            int subId = AccountTypeUtils.getSubIdBySimAccountName(context,
                    accountName);
            int emailCount = PhbInfoUtils.getUsimEmailCount(subId);
            Log.sensitive(TAG, "[isSupportShowEmailData] Usim type, accountName: " + accountName +
                    ",subId: " + subId + ",emailCount: " + emailCount);
            if (emailCount <= 0) {
                Log.i(TAG, "[isSupportShowEmailData] Usim not support email field,remove it!!");
                return false;
            }
        }
        return true;
    }
    /// @}
}
