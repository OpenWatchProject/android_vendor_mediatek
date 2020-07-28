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
package com.mediatek.contacts.group;

import android.R.integer;
import android.app.Activity;
import android.content.ContentProviderOperation;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.Context;
import android.content.Intent;
import android.content.OperationApplicationException;
import android.database.Cursor;
import android.net.Uri;
import android.os.Handler;
import android.os.Looper;
import android.os.RemoteException;
import android.provider.ContactsContract;
import android.provider.ContactsContract.Contacts;
import android.provider.ContactsContract.Groups;
import android.text.TextUtils;
import android.util.SparseIntArray;
import android.widget.Toast;

import com.android.contacts.activities.ContactEditorActivity.ContactEditor.SaveMode;
import com.android.contacts.model.account.AccountWithDataSet;
import com.android.contacts.ContactSaveService;
import com.android.contacts.editor.ContactEditorFragment;
import com.android.contacts.GroupListLoader;
import com.android.contacts.R;
import com.google.common.base.Objects;
import com.mediatek.contacts.ContactsApplicationEx;
import com.mediatek.contacts.simservice.SimProcessorService;
import com.mediatek.contacts.simservice.SimServiceUtils;
import com.mediatek.contacts.util.ContactsGroupUtils;
import com.mediatek.contacts.util.ContactsGroupUtils.USIMGroupException;
import com.mediatek.contacts.util.Log;
import com.mediatek.contacts.util.MtkToast;
import com.mediatek.provider.MtkContactsContract;

import java.util.ArrayList;
import java.util.HashMap;

public class SimGroupUtils {
    private static final String TAG = "GroupsUtils";
    // add to update group infos intent keys from groupEditorFragment
    public static final String EXTRA_SIM_INDEX_TO_ADD = "simIndexToAdd";
    public static final String EXTRA_SIM_INDEX_TO_REMOVE = "simIndexToRemove";
    public static final String EXTRA_ORIGINAL_GROUP_NAME = "originalGroupName";
    public static final String EXTRA_SIM_INDEX_ARRAY = "simIndexArray";
    public static final String EXTRA_SUB_ID = "subId";
    public static final String KEY_ACCOUNT_CATEGORY = "AccountCategory";
    // add new group name as back item of GroupCreationDialogFragment
    public static final String EXTRA_NEW_GROUP_NAME = "addGroupName";
    // use this to check if need to toast in PeopleActivity
    public static final String EXTRA_DONE_TOAST = "haveToastDone";

    // :[Gemini+] all possible slot error can be safely put in this sparse int
    // array.
    private static SparseIntArray mSubIdError = new SparseIntArray();

    private static Handler mMainHandler = new Handler(Looper.getMainLooper());
    // We need get group numbers for Move to other group feature.
    /**
     * Get group numbers for every account.
     */
    public static void initGroupsByAllAccount(Cursor mCursor, ArrayList<String> mAccountNameList,
            HashMap<String, Integer> mAccountGroupMembers) {
        if (mCursor == null || mCursor.getCount() == 0) {
            Log.e(TAG, "[initGroupsByAllAccount] mCursor = " + mCursor);
            return;
        }

        mCursor.moveToPosition(-1);
        int accountNum = getAllAccoutNums(mCursor, mAccountNameList);
        Log.d(TAG, "[initGroupsByAllAccount]group count:" + Log.anonymize(accountNum));
        int groups = 0;
        for (int index = 0; index < accountNum; index++) {
            groups = getGroupNumsByAccountName(mCursor, mAccountNameList.get(index));
            mAccountGroupMembers.put(mAccountNameList.get(index), groups);
        }
    }
    /**
     * Get groups by specified account name.
     * @param name, account name.
     * @return
     */
    public static int getGroupNumsByAccountName(Cursor mCursor, String name) {
        int count = 0;
        int index = 0;
        while (mCursor.moveToPosition(index)) {
            String accountName = mCursor.getString(GroupListLoader.ACCOUNT_NAME);
            if (accountName.equals(name)) {
                count++;
            }
            index++;
        }
        return count;
    }
    /**
     * Get all account numbers.
     * @return
     */
    public static int getAllAccoutNums(Cursor mCursor, ArrayList<String> mAccountNameList) {
        int pos = 0;
        int count = 0;
        mAccountNameList.clear();
        while (mCursor.moveToPosition(pos)) {
            String accountName = mCursor.getString(GroupListLoader.ACCOUNT_NAME);
            String accountType = mCursor.getString(GroupListLoader.ACCOUNT_TYPE);
            String dataSet = mCursor.getString(GroupListLoader.DATA_SET);
            int previousIndex = pos - 1;
            if (previousIndex >= 0 && mCursor.moveToPosition(previousIndex)) {
                String previousGroupAccountName = mCursor.getString(GroupListLoader.ACCOUNT_NAME);
                String previousGroupAccountType = mCursor.getString(GroupListLoader.ACCOUNT_TYPE);
                String previousGroupDataSet = mCursor.getString(GroupListLoader.DATA_SET);

                if (!(accountName.equals(previousGroupAccountName) &&
                        accountType.equals(previousGroupAccountType) && Objects
                        .equal(dataSet, previousGroupDataSet))) {
                    count++;
                    mAccountNameList.add(accountName);
                }
            }
            else {
                mAccountNameList.add(accountName);
                count++;
            }
            pos++;
        }

        return count;
    }

    /**
     * Shows a toast on the UI thread.
     */
    private static void showToast(final int message) {
        mMainHandler.post(new Runnable() {
            @Override
            public void run() {
                Toast.makeText(ContactsApplicationEx.getContactsApplication(), message,
                        Toast.LENGTH_LONG).show();
            }
        });
    }

    /**
     * delete group in icc card,like usim
     *
     * @param intent
     * @param groupId
     * @return true if success,false else.
     */
    public static boolean deleteGroupInIcc(Context context, Intent intent,
            long groupId) {
        String groupLabel = intent.getStringExtra(ContactSaveService.EXTRA_GROUP_LABEL);
        int subId = intent.getIntExtra(EXTRA_SUB_ID, -1);

        if (subId <= 0 || TextUtils.isEmpty(groupLabel)) {
            Log.w(TAG, "[deleteGroupInIcc] subId:" + subId + ",groupLabel:"
                + Log.anonymize(groupLabel) + " have errors");
            return false;
        }

        // check whether group exists
        int ugrpId = -1;
        try {
            ugrpId = ContactsGroupUtils.USIMGroup.hasExistGroup(subId, groupLabel);
            Log.d(TAG, "[deleteGroupInIcc]ugrpId:" + ugrpId);
        } catch (RemoteException e) {
            e.printStackTrace();
            ugrpId = -1;
        }
        if (ugrpId > 0) {
            // fix ALPS01002380. should not use groupLabel for groupuri,because groupname "/"
            // will lead to SQLite exception.
            Uri groupUri = ContentUris.withAppendedId(Contacts.CONTENT_GROUP_URI, groupId)
                    .buildUpon().appendQueryParameter(
                        MtkContactsContract.Groups.QUERY_WITH_GROUP_ID, "true").build();
            Cursor c = context.getContentResolver().query(groupUri,
                    new String[] { Contacts._ID,
                    MtkContactsContract.ContactsColumns.INDEX_IN_SIM },
                    MtkContactsContract.ContactsColumns.INDICATE_PHONE_SIM +
                    " = " + subId, null, null);
            Log.d(TAG, "[deleteGroupInIcc]groupUri:" + groupUri + ". simId:" + subId 
                    + "|member count:" + (c == null ? "null" : c.getCount()));
            try {
                while (c != null && c.moveToNext()) {
                    int indexInSim = c.getInt(1);
                    boolean ret = ContactsGroupUtils.USIMGroup.deleteUSIMGroupMember(subId,
                            indexInSim, ugrpId);
                    Log.d(TAG,
                            "[deleteGroupInIcc]subId:" + subId + "ugrpId:" + ugrpId + "|simIndex:"
                                    + indexInSim + "|Result:" + ret + " | contactid : "
                                    + c.getLong(0));
                }
            } finally {
                if (c != null) {
                    c.close();
                }
            }
            // Delete USIM group
            int error = ContactsGroupUtils.USIMGroup.deleteUSIMGroup(subId, groupLabel);
            Log.d(TAG, "[deleteGroupInIcc]error:" + error);
            if (error != 0) {
                showToast(R.string.delete_group_failure);
                return false;
            }
        }
        return true;
    }

    public static int updateGroupToIcc(Context context, Intent intent, Intent callbackIntent) {
        int subId = intent.getIntExtra(EXTRA_SUB_ID, -1);
        String originalName = intent.getStringExtra(EXTRA_ORIGINAL_GROUP_NAME);
        String groupName = intent.getStringExtra(ContactSaveService.EXTRA_GROUP_LABEL);
        int groupIdInIcc = -1;

        if (subId < 0) {
            Log.w(TAG, "[updateGroupToIcc] subId is error.subId:" + subId);
            return groupIdInIcc;
        }

        Log.d(TAG, "[updateGroupToIcc]groupName:" + Log.anonymize(groupName)
                + "|originalName:" + Log.anonymize(originalName) + " |subId:" + subId);

        try {
            groupIdInIcc = ContactsGroupUtils.USIMGroup.syncUSIMGroupUpdate(subId, originalName,
                    groupName);
        } catch (RemoteException e) {
            Log.e(TAG, "[updateGroupToIcc]e : " + e);
        } catch (USIMGroupException e) {
            Log.e(TAG, "[updateGroupToIcc] catched USIMGroupException." + " ErrorType: "
                    + e.getErrorType());
            mSubIdError.put(e.getErrorSubId(), e.getErrorType());
            checkAllSlotErrors(callbackIntent);
        }
        return groupIdInIcc;
    }

    public static int createGroupToIcc(Context context, Intent intent, Intent callbackIntent) {
        String groupName = intent.getStringExtra(ContactSaveService.EXTRA_GROUP_LABEL);
        int groupIdInIcc = -1;

        int subId = intent.getIntExtra(EXTRA_SUB_ID, -1);
        if (subId <= 0) {
            Log.w(TAG, "[createGroupToIcc] subId error..subId:" + subId);
            return groupIdInIcc;
        }

        try {
            groupIdInIcc = ContactsGroupUtils.USIMGroup.syncUSIMGroupNewIfMissing(subId, groupName);
        } catch (RemoteException e) {
            e.printStackTrace();
        } catch (USIMGroupException e) {
            Log.w(TAG, "[createGroupToIcc] create group fail type:" + e.getErrorType()
                    + ",fail subId:" + e.getErrorSubId());
            mSubIdError.put(e.getErrorSubId(), e.getErrorType());
            checkAllSlotErrors(callbackIntent);
            if (e.getErrorType() == USIMGroupException.GROUP_NAME_OUT_OF_BOUND) {
                intent.putExtra(ContactEditorFragment.SAVE_MODE_EXTRA_KEY, SaveMode.RELOAD);
            }
        }
        return groupIdInIcc;
    }

    public static int createGroupToIcc(String groupName, int subId) {
        int groupIdInIcc = -1;
        if (subId <= 0) {
            Log.w(TAG, "[createGroupToIcc] subId error..subId:" + subId);
            return groupIdInIcc;
        }
        try {
            groupIdInIcc = ContactsGroupUtils.USIMGroup.syncUSIMGroupNewIfMissing(subId, groupName);
        } catch (RemoteException e) {
            e.printStackTrace();
        } catch (USIMGroupException e) {
            Log.w(TAG, "[createGroupToIcc] create group fail type:" + e.getErrorType()
                    + ",fail subId:" + e.getErrorSubId());
            mSubIdError.put(e.getErrorSubId(), e.getErrorType());
            checkAllSlotErrors(null);
        }
        return groupIdInIcc;
    }

    public static boolean checkGroupNameExist(Context context, String groupName,
            String accountName, String accountType) {
        boolean nameExists = false;

        if (TextUtils.isEmpty(groupName)) {
            showToastInt(R.string.name_needed);
            return false;
        }
        Cursor cursor = context.getContentResolver().query(
                Groups.CONTENT_SUMMARY_URI,
                new String[] { Groups._ID },
                Groups.TITLE + "=? AND " + Groups.ACCOUNT_NAME + " =? AND " + Groups.ACCOUNT_TYPE
                        + "=? AND " + Groups.DELETED + "=0",
                new String[] { groupName, accountName, accountType }, null);
        if (cursor != null) {
            if (cursor.getCount() > 0) {
                nameExists = true;
            }
            cursor.close();
        }
        // If group name exists, make a toast and return false.
        if (nameExists) {
            showToastInt(R.string.group_name_exists);
            return false;
        } else {
            return true;
        }
    }

    /**
     * [Gemini+] check all slot to find whether is there any error happened
     */
    public static void checkAllSlotErrors(Intent callbackIntent) {
        for (int i = 0; i < mSubIdError.size(); i++) {
            int subId = mSubIdError.keyAt(i);
            int errorCode = mSubIdError.valueAt(i);
            if (showMoveUSIMGroupErrorToast(errorCode, subId)
                    && callbackIntent != null) {
                callbackIntent.putExtra(SimGroupUtils.EXTRA_DONE_TOAST, true);
            }
        }
        mSubIdError.clear();
    }

    public static boolean showMoveUSIMGroupErrorToast(int errCode, int subId) {
        Log.d(TAG, "[showMoveUSIMGroupErrorToast]errCode:" + errCode + "|subId:" + subId);
        /** Bug Fix for CR ALPS00451441 */
        String toastMsg = null;
        if (errCode == USIMGroupException.GROUP_GENERIC_ERROR) {
            toastMsg = ContactsApplicationEx.getContactsApplication().getString(
                    R.string.save_group_fail);
        } else {
            toastMsg = ContactsApplicationEx.getContactsApplication().getString(
                    ContactsGroupUtils.USIMGroupException.getErrorToastId(errCode));
        }

        if (toastMsg != null) {
            Log.d(TAG, "[showMoveUSIMGroupErrorToast]toastMsg:" + toastMsg);
            showToastString(toastMsg);
            return true;
        } else {
            return false;
        }
    }

    /**
     * Shows a toast on the UI thread.
     */
    private static void showToastString(final String message) {
        mMainHandler.post(new Runnable() {
            @Override
            public void run() {
                Toast.makeText(ContactsApplicationEx.getContactsApplication(), message,
                        Toast.LENGTH_LONG).show();
            }
        });
    }
    /**
     * Shows a toast on the UI thread.
     */
    private static void showToastInt(final int message) {
        mMainHandler.post(new Runnable() {
            @Override
            public void run() {
                Toast.makeText(ContactsApplicationEx.getContactsApplication(), message,
                        Toast.LENGTH_LONG).show();
            }
        });
    }

    /**
     * fix ALPS00272729
     *
     * @param operations
     * @param resolver
     */
    public static void bufferOperations(ArrayList<ContentProviderOperation> operations,
            ContentResolver resolver) {
        try {
            Log.d(TAG, "[bufferOperatation] begin applyBatch ");
            resolver.applyBatch(ContactsContract.AUTHORITY, operations);
            Log.d(TAG, "[bufferOperatation] end applyBatch");
            operations.clear();
        } catch (RemoteException e) {
            Log.e(TAG, "[bufferOperatation]RemoteException:", e);
            showToast(R.string.contactSavedErrorToast);
        } catch (OperationApplicationException e) {
            Log.e(TAG, "[bufferOperatation]OperationApplicationException:", e);
            showToast(R.string.contactSavedErrorToast);
        }
    }

    /**
     * Extension createNewGroupIntent() for handle with Icc Card
     */
     public static Intent createNewGroupIntentForIcc(Context context, AccountWithDataSet account,
             String label, final long[] rawContactsToAdd,
             Class<? extends Activity> callbackActivity, String callbackAction,
             final int[] simIndexArray, int subId) {
         Log.d(TAG, "[CreateNewGroupIntentForIcc]");
         Intent serviceIntent = new Intent(context, SimProcessorService.class);
         serviceIntent.putExtra(SimServiceUtils.SERVICE_WORK_TYPE,
                 SimServiceUtils.SERVICE_WORK_GROUP);
         serviceIntent.putExtra(SimServiceUtils.SERVICE_SUBSCRIPTION_KEY, subId);

         serviceIntent.setAction(ContactSaveService.ACTION_CREATE_GROUP);
         serviceIntent.putExtra(ContactSaveService.EXTRA_ACCOUNT_TYPE, account.type);
         serviceIntent.putExtra(ContactSaveService.EXTRA_ACCOUNT_NAME, account.name);
         serviceIntent.putExtra(ContactSaveService.EXTRA_DATA_SET, account.dataSet);
         serviceIntent.putExtra(ContactSaveService.EXTRA_GROUP_LABEL, label);
         serviceIntent.putExtra(ContactSaveService.EXTRA_RAW_CONTACTS_TO_ADD, rawContactsToAdd);

         serviceIntent.putExtra(EXTRA_SIM_INDEX_ARRAY, simIndexArray);
         serviceIntent.putExtra(EXTRA_SUB_ID, subId);

         // Callback intent will be invoked by the service once the new group is created.
         Intent callbackIntent = new Intent(context, callbackActivity);
         callbackIntent.setAction(callbackAction);
         callbackIntent.putExtra(EXTRA_SUB_ID, subId);
         callbackIntent.putExtra(EXTRA_NEW_GROUP_NAME, label);

         serviceIntent.putExtra(ContactSaveService.EXTRA_CALLBACK_INTENT, callbackIntent);
         return serviceIntent;
     }

    /**
     * Extension createGroupUpdateIntent() for handle with Icc Card
     */
     public static Intent createGroupUpdateIntentForIcc(Context context, long groupId,
             String newLabel, long[] rawContactsToAdd, long[] rawContactsToRemove,
             Class<? extends Activity> callbackActivity, String callbackAction,
             String OriginalGroupName, int subId, int[] simIndexToAddArray,
             int[] simIndexToRemoveArray, AccountWithDataSet account) {
         Log.d(TAG, "[createGroupUpdateIntentForIcc]");
         Intent serviceIntent = new Intent(context, SimProcessorService.class);
         serviceIntent.putExtra(SimServiceUtils.SERVICE_WORK_TYPE,
                 SimServiceUtils.SERVICE_WORK_GROUP);
         serviceIntent.putExtra(SimServiceUtils.SERVICE_SUBSCRIPTION_KEY, subId);
         serviceIntent.setAction(ContactSaveService.ACTION_UPDATE_GROUP);
         serviceIntent.putExtra(ContactSaveService.EXTRA_GROUP_ID, groupId);
         serviceIntent.putExtra(ContactSaveService.EXTRA_GROUP_LABEL, newLabel);
         serviceIntent.putExtra(ContactSaveService.EXTRA_RAW_CONTACTS_TO_ADD, rawContactsToAdd);
         serviceIntent.putExtra(ContactSaveService.EXTRA_RAW_CONTACTS_TO_REMOVE,
                 rawContactsToRemove);

         serviceIntent.putExtra(EXTRA_SUB_ID, subId);
         serviceIntent.putExtra(EXTRA_SIM_INDEX_TO_ADD, simIndexToAddArray);
         serviceIntent.putExtra(EXTRA_SIM_INDEX_TO_REMOVE, simIndexToRemoveArray);
         serviceIntent.putExtra(EXTRA_ORIGINAL_GROUP_NAME, OriginalGroupName);
         serviceIntent.putExtra(ContactSaveService.EXTRA_ACCOUNT_TYPE, account.type);
         serviceIntent.putExtra(ContactSaveService.EXTRA_ACCOUNT_NAME, account.name);
         serviceIntent.putExtra(ContactSaveService.EXTRA_DATA_SET, account.dataSet);

         // Callback intent will be invoked by the service once the group is updated
         Intent callbackIntent = new Intent(context, callbackActivity);
         callbackIntent.setAction(callbackAction);
         callbackIntent.putExtra(EXTRA_SUB_ID, subId);

         serviceIntent.putExtra(ContactSaveService.EXTRA_CALLBACK_INTENT, callbackIntent);
         return serviceIntent;
     }

     /**
      * Extension createGroupDeletionIntent() for handle with Icc Card
      */
     public static Intent createGroupDeletionIntentForIcc(Context context, long groupId, int subId,
             String groupLabel) {
         Log.d(TAG, "createGroupDeletionIntentForIcc");
         Intent serviceIntent = new Intent(context, SimProcessorService.class);
         serviceIntent.putExtra(SimServiceUtils.SERVICE_WORK_TYPE,
                 SimServiceUtils.SERVICE_WORK_GROUP);
         serviceIntent.putExtra(SimServiceUtils.SERVICE_SUBSCRIPTION_KEY, subId);
         serviceIntent.setAction(ContactSaveService.ACTION_DELETE_GROUP);
         serviceIntent.putExtra(ContactSaveService.EXTRA_GROUP_ID, groupId);

         serviceIntent.putExtra(EXTRA_SUB_ID, subId);
         serviceIntent.putExtra(ContactSaveService.EXTRA_GROUP_LABEL, groupLabel);

         return serviceIntent;
     }

    public static Intent createUndoIntentForIcc(Context context, Intent resultIntent, int subId) {
        Log.d(TAG, "createUndoIntentForIcc");
        Intent serviceIntent = new Intent(context, SimProcessorService.class);
        serviceIntent.putExtra(SimServiceUtils.SERVICE_WORK_TYPE,
                SimServiceUtils.SERVICE_WORK_GROUP);
        serviceIntent.setAction(ContactSaveService.ACTION_UNDO);
        serviceIntent.putExtra(SimServiceUtils.SERVICE_SUBSCRIPTION_KEY, subId);
        serviceIntent.putExtras(resultIntent);
        return serviceIntent;
    }

     public static Intent createGroupRenameIntentForIcc(Context context, long groupId,
             String newLabel, Class<? extends Activity> callbackActivity,
             String callbackAction, String OriginalGroupName, int subId) {
         Log.d(TAG, "createGroupRenameIntentForIcc");
         Intent serviceIntent = new Intent(context, SimProcessorService.class);
         serviceIntent.putExtra(SimServiceUtils.SERVICE_WORK_TYPE,
                 SimServiceUtils.SERVICE_WORK_GROUP);
         serviceIntent.setAction(ContactSaveService.ACTION_RENAME_GROUP);

         serviceIntent.putExtra(EXTRA_SUB_ID, subId);
         serviceIntent.putExtra(ContactSaveService.EXTRA_GROUP_ID, groupId);
         serviceIntent.putExtra(EXTRA_ORIGINAL_GROUP_NAME, OriginalGroupName);
         serviceIntent.putExtra(ContactSaveService.EXTRA_GROUP_LABEL, newLabel);

         Intent callbackIntent = new Intent(context, callbackActivity);
         callbackIntent.setAction(callbackAction);
         serviceIntent.putExtra(ContactSaveService.EXTRA_CALLBACK_INTENT, callbackIntent);

         return serviceIntent;
     }

     /// ALPS00542175 Check background service state if idle @{
     public static boolean checkServiceState(boolean showTips, int subId, Context context) {
        if (context == null) {
            Log.e(TAG, "[checkServiceState]ignore due to context is null");
            return false;
        }
         // change for SIM Service Refactoring
         if (subId > 0 && SimServiceUtils.isServiceRunning(context, subId)) {
             if (showTips) {
                 Toast.makeText(context,
                         R.string.msg_loading_sim_contacts_toast,
                         Toast.LENGTH_SHORT).show();
             }
             return false;
         }
         // ALPS00542175
         if (ContactSaveService.isGroupTransactionProcessing()) {
             if (showTips) {
                 ///M:[ALPS03587932] toast too many and lead FD exhaused. @{
                 MtkToast.toast(context, R.string.phone_book_busy);
                 /// @}
             }
             return false;
         }
         Log.d(TAG, "[checkServiceState] service is idle now, subId=" + subId);
         return true;
     }
}
