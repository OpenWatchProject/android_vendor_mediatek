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
package com.mediatek.simprocessor;

import android.content.Context;
import android.content.ContentValues;
import android.content.Intent;
import android.database.Cursor;
import android.os.UserHandle;
import android.provider.ContactsContract.Groups;
import android.provider.ContactsContract.RawContacts;
import android.provider.ContactsContract.Settings;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.text.TextUtils;

import com.android.internal.telephony.PhoneConstants;

import com.mediatek.provider.MtkContactsContract;
import com.mediatek.simprocessor.SimCardUtils;
import com.mediatek.simprocessor.SubInfoUtils;
import com.mediatek.simprocessor.Log;

import java.util.ArrayList;
import java.util.List;

public class SimServiceUtils {
    private static final String TAG = "SimServiceUtils";

    private static SimProcessorState sSimProcessorState;

    public static final String SERVICE_SUBSCRIPTION_KEY = "subscription_key";
    public static final String SERVICE_SLOT_KEY = "which_slot";
    public static final String SERVICE_WORK_TYPE = "work_type";

    public static final int SERVICE_WORK_NONE = 0;
    public static final int SERVICE_WORK_IMPORT = 1;
    public static final int SERVICE_WORK_REMOVE = 2;
    public static final int SERVICE_WORK_EDIT = 3;
    public static final int SERVICE_WORK_DELETE = 4;
    public static final int SERVICE_WORK_UNKNOWN = -1;
    public static final int SERVICE_IDLE = 0;
    public static final int SERVICE_FORCE_REMOVE_SUB_ID = -20;
    public static final int SERVICE_REMOVE_DUP_SUB_ID = -30;

    public static final int SERVICE_DELETE_CONTACTS = 1;
    public static final int SERVICE_QUERY_SIM = 2;
    public static final int SERVICE_IMPORT_CONTACTS = 3;

    public static final int SIM_TYPE_SIM = SimCardUtils.SimType.SIM_TYPE_SIM;
    public static final int SIM_TYPE_USIM = SimCardUtils.SimType.SIM_TYPE_USIM;
    public static final int SIM_TYPE_RUIM = SimCardUtils.SimType.SIM_TYPE_RUIM;
    public static final int SIM_TYPE_CSIM = SimCardUtils.SimType.SIM_TYPE_CSIM;
    public static final int SIM_TYPE_UNKNOWN = SimCardUtils.SimType.SIM_TYPE_UNKNOWN;

    public static final int TYPE_IMPORT = 1;
    public static final int TYPE_REMOVE = 2;

    public static class ServiceWorkData {
        public int mSubId = -1;
        public int mSimType = SIM_TYPE_UNKNOWN;
        public Cursor mSimCursor = null;

        ServiceWorkData() {
        }

        ServiceWorkData(int subId, int simType, Cursor simCursor) {
            mSubId = subId;
            mSimType = simType;
            mSimCursor = simCursor;
        }
    }

    public static void deleteSimContact(Context context, int subId) {
        Log.i(TAG, "[deleteSimContact]subId:" + subId);
        List<SubscriptionInfo> subscriptionInfoList = SubInfoUtils.getActivatedSubInfoList();

        if (subId == SERVICE_REMOVE_DUP_SUB_ID && subscriptionInfoList != null
                && subscriptionInfoList.size() > 0) {
            deleteDupSimContact(context);
            return;
        }

        ArrayList<Integer> deleteSubIds = new ArrayList<Integer>();
        /// fix ALPS02816596.when force remove all sim contacts,deleteSubIds.size will be
        //  zero.it will remove all sim contacts.@{
        if (subId != SERVICE_FORCE_REMOVE_SUB_ID) {
            deleteSubIds.add(subId);
        }
        /// @}

        // Be going to delete the invalid SIM contacts records.
        StringBuilder delSelection = new StringBuilder();
        String filter = null;
        for (int id : deleteSubIds) {
            delSelection.append(id).append(",");
        }
        if (delSelection.length() > 0) {
            delSelection.deleteCharAt(delSelection.length() - 1);
            filter = delSelection.toString();
        }
        filter = TextUtils.isEmpty(filter) ?
                MtkContactsContract.RawContactsColumns.INDICATE_PHONE_SIM + " > 0 "
                : MtkContactsContract.RawContactsColumns.INDICATE_PHONE_SIM + " > 0 "
                        + " AND " + MtkContactsContract.RawContactsColumns.INDICATE_PHONE_SIM
                        + " IN (" + filter + ")";
        Log.d(TAG, "[deleteSimContact] sim contacts filter:" + filter +
                ",deleteSubIds: " + deleteSubIds);
        int count = context.getContentResolver().delete(
                RawContacts.CONTENT_URI.buildUpon().appendQueryParameter("sim", "true").build(),
                filter, null);
        // add for ALPS01964765.
        Log.d(TAG, "[deleteSimContact]the current user is: " + UserHandle.myUserId() +
                ",count = " + count);

        /// M:[ALPS03485388]Delete again to avoid DB insert sametime in the other thread. @{
        try {
            Thread.sleep(500);
        } catch (InterruptedException e) {
        }
        Cursor cursor = context.getContentResolver().query(
                RawContacts.CONTENT_URI.buildUpon().appendQueryParameter("sim", "true").build(),
                null, filter, null,null);
        if (cursor != null) {
            if (cursor.getCount() > 0) {
                int count2 = context.getContentResolver().delete(
                    RawContacts.CONTENT_URI.buildUpon().appendQueryParameter("sim","true").build(),
                    filter, null);
                Log.e(TAG, "[deleteSimContact]before detele again count = " +
                    cursor.getCount() + ", delete again count = " + count2);
            }
            cursor.close();
        }
        /// @}

        /// M: [Sim Contact Flow][ALPS04179873] reset ungroup_visible property for sim 1/2 @{
        /// Ignore delete triggered by boot up, do it in delete trigger by import
        if (SubscriptionManager.isValidSubscriptionId(subId)) {
            resetSimUnGroupVisible(context, subId);
        }
        /// @}

        // Be going to delete the invalid USIM group records.
        delSelection = new StringBuilder();
        filter = null;
        for (int id : deleteSubIds) {
            delSelection.append("'" + "USIM" + id + "'" + ",");
        }

        if (delSelection.length() > 0) {
            delSelection.deleteCharAt(delSelection.length() - 1);
            filter = delSelection.toString();
        }
        filter = TextUtils.isEmpty(filter) ? (Groups.ACCOUNT_TYPE + "='USIM Account'")
                : (Groups.ACCOUNT_NAME + " IN " + "(" + filter + ")" + " AND "
                        + Groups.ACCOUNT_TYPE + "='USIM Account'");
        Log.d(TAG, "[deleteSimContact]usim group filter:" + filter);
        count = context.getContentResolver().delete(Groups.CONTENT_URI, filter, null);
        Log.d(TAG, "[deleteSimContact] group count:" + count);
    }

    /// M: [Sim Contact Flow][ALPS04179873] reset ungroup_visible property for sim 2/2 @{
    private static void resetSimUnGroupVisible(Context context, int subId) {
        Log.d(TAG, "[resetSimUnGroupVisible] " + subId);
        String accountName = SimProcessorUtils.getAccountNameUsingSubId(subId);
        String accountType = SimProcessorUtils.getAccountTypeUsingSubId(subId);
        String filter = Settings.ACCOUNT_NAME + " = '" + accountName + "' AND "
                + Settings.ACCOUNT_TYPE + " = '" + accountType + "'";

        Log.d(TAG, "[resetSimUnGroupVisible] " + accountName + ", " + accountType);
        ContentValues values = new ContentValues();
        values.put(Settings.UNGROUPED_VISIBLE, 1);
        int count = context.getContentResolver().update(Settings.CONTENT_URI, values, filter, null);
        Log.d(TAG, "[resetSimUnGroupVisible] update " + count + " row");
    }
    /// @}

    private static void deleteDupSimContact(Context context) {
        Log.i(TAG, "[deleteDupSimContact]");
        ArrayList<Integer> validSubIds = new ArrayList<Integer>();
        List<SubscriptionInfo> subscriptionInfoList = SubInfoUtils.getActivatedSubInfoList();
        if (subscriptionInfoList == null) {
            Log.w(TAG, "[deleteDupSimContact]getActivatedSubInfoList is null!");
        } else {
            for (SubscriptionInfo subscriptionInfo : subscriptionInfoList) {
                int subId = subscriptionInfo.getSubscriptionId();
                if (SimCardUtils.isPhoneBookReady(subId)) {
                    //only keep the sim contacts whose subId is active(valid) && phb is ready
                    validSubIds.add(subId);
                }
            }
        }
        // Be going to delete the invalid SIM or phb unready contacts records.
        StringBuilder delSelection = new StringBuilder();
        String filter = null;
        for (int id : validSubIds) {
            delSelection.append(id).append(",");
        }
        if (delSelection.length() > 0) {
            delSelection.deleteCharAt(delSelection.length() - 1);
            filter = delSelection.toString();
        }
        filter = TextUtils.isEmpty(filter) ?
                MtkContactsContract.RawContactsColumns.INDICATE_PHONE_SIM + " > 0 "
                : MtkContactsContract.RawContactsColumns.INDICATE_PHONE_SIM + " > 0 "
                        + " AND " + MtkContactsContract.RawContactsColumns.INDICATE_PHONE_SIM
                        + " NOT IN (" + filter + ")";
        Log.d(TAG, "[deleteDupSimContact]sim contacts filter:" + filter);
        int count = context.getContentResolver().delete(
                RawContacts.CONTENT_URI.buildUpon().appendQueryParameter("sim", "true").build(),
                filter, null);
        // add for ALPS01964765.
        Log.d(TAG, "[deleteDupSimContact]the current user is: " + UserHandle.myUserId() +
                ",count = " + count);

        // Be going to delete the invalid USIM group records.
        delSelection = new StringBuilder();
        filter = null;
        for (int id : validSubIds) {
            delSelection.append("'" + "USIM" + id + "'" + ",");
        }

        if (delSelection.length() > 0) {
            delSelection.deleteCharAt(delSelection.length() - 1);
            filter = delSelection.toString();
        }
        filter = TextUtils.isEmpty(filter) ? (Groups.ACCOUNT_TYPE + "='USIM Account'")
                : (Groups.ACCOUNT_NAME + " NOT IN " + "(" + filter + ")" + " AND "
                        + Groups.ACCOUNT_TYPE + "='USIM Account'");
        Log.d(TAG, "[deleteDupSimContact]usim group filter:" + filter);
        count = context.getContentResolver().delete(Groups.CONTENT_URI, filter, null);
        Log.d(TAG, "[deleteDupSimContact] group count:" + count);
    }

    /**
     * check PhoneBook State is ready if ready, then return true.
     *
     * @param subId
     * @return
     */
    static boolean checkPhoneBookState(final int subId) {
        return SimCardUtils.isPhoneBookReady(subId);
    }

    public static boolean isServiceRunning(int subId) {
        if (sSimProcessorState != null) {
            return sSimProcessorState.isImportRemoveRunning(subId);
        }

        return false;
    }

    public static void setSimProcessorState(SimProcessorState processorState) {
        sSimProcessorState = processorState;
    }

    public interface SimProcessorState {
        public boolean isImportRemoveRunning(int subId);
    }
}
