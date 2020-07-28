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

package com.mediatek.contacts.simservice;

import android.app.Activity;
import android.content.ContentProviderOperation;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.OperationApplicationException;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.RemoteException;
import android.provider.ContactsContract;
import android.provider.ContactsContract.CommonDataKinds.GroupMembership;
import android.provider.ContactsContract.Data;
import android.provider.ContactsContract.Groups;
import android.provider.ContactsContract.RawContacts;
import android.text.TextUtils;
import androidx.localbroadcastmanager.content.LocalBroadcastManager;

import com.android.contacts.ContactSaveService;
import com.android.contacts.ContactSaveService.GroupsDao;
import com.android.contacts.ContactSaveService.GroupsDaoImpl;
import com.android.contacts.activities.ContactEditorActivity.ContactEditor.SaveMode;
import com.android.contacts.editor.ContactEditorFragment;
import com.google.android.collect.Lists;
import com.mediatek.contacts.group.SimGroupUtils;
import com.mediatek.contacts.simcontact.SubInfoUtils;
import com.mediatek.contacts.simservice.SimProcessorManager.ProcessorCompleteListener;
import com.mediatek.contacts.util.ContactsGroupUtils;
import com.mediatek.contacts.util.Log;
import com.mediatek.provider.MtkContactsContract;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class SimGroupProcessor extends SimProcessorBase {
    private static final String TAG = "SimGroupProcessor";

    private Context mContext;
    private Intent mIntent = null;
    private Uri mLookupUri = null;
    private int mSubId = SubInfoUtils.getInvalidSubId();
    private static final int GROUP_SIM_ABSENT = 4;
    private static final int MAX_OPERATIONS_SIZE = 400;
    private Handler mMainHandler;
    private GroupsDao mGroupsDao;

    private static List<Listener> sListeners = new ArrayList<Listener>();

    public interface Listener {
        public void onSimGroupCompleted(Intent callbackIntent);
    }

    public static void registerListener(Listener listener) {
        if (!(listener instanceof Activity)) {
            throw new ClassCastException("Only activities can be registered to"
                    + " receive callback from " + SimProcessorService.class.getName());
        }
        Log.d(TAG, "[registerListener]listener added to SIMGroupProcessor: " + listener);
        sListeners.add(listener);
    }

    public static void unregisterListener(Listener listener) {
        Log.d(TAG, "[unregisterListener]listener removed from SIMGroupProcessor: " + listener);
        sListeners.remove(listener);
    }

    public SimGroupProcessor(Context context, int subId, Intent intent,
            ProcessorCompleteListener listener) {
        super(intent, listener);
        mContext = context;
        mSubId = subId;
        mIntent = intent;
        mMainHandler = new Handler(Looper.getMainLooper());
        mGroupsDao = new SimGroupsDaoImpl(context, mSubId);
        Log.i(TAG, "[SIMGroupProcessor]new mSubId = " + mSubId);
    }

    @Override
    public int getType() {
        return SimServiceUtils.SERVICE_WORK_GROUP;
    }

    @Override
    public void doWork() {
        if (mIntent == null) {
            Log.e(TAG, "[doWork]onHandleIntent: could not handle null intent");
            return;
        }

        // Call an appropriate method. If we're sure it affects how incoming phone calls are
        // handled, then notify the fact to in-call screen.
        String action = mIntent.getAction();
        Log.d(TAG, "[doWork]action = " + action);
        if (ContactSaveService.ACTION_CREATE_GROUP.equals(action)) {
            createGroup(mIntent);
        } else if (ContactSaveService.ACTION_RENAME_GROUP.equals(action)) {
            renameGroup(mIntent);
        } else if (ContactSaveService.ACTION_DELETE_GROUP.equals(action)) {
            deleteGroup(mIntent);
        } else if (ContactSaveService.ACTION_UPDATE_GROUP.equals(action)) {
            /* M:[Sim Group] ALPS03523813 2/2 */
            ContactSaveService.setGroupTransactionProcessing(true);
            updateGroup(mIntent);
            ContactSaveService.setGroupTransactionProcessing(false);
        } else if (ContactSaveService.ACTION_UNDO.equals(action)) {
            /* M:[Google Issue] ALPS03478402: ensure undo done to avoid
             * creating a group with same name as the undo group. 2/2 */
            ContactSaveService.setGroupTransactionProcessing(true);
            undo(mIntent);
            ContactSaveService.setGroupTransactionProcessing(false);
        }
    }

    private void createGroup(Intent intent) {
        String accountType = intent.getStringExtra(ContactSaveService.EXTRA_ACCOUNT_TYPE);
        String accountName = intent.getStringExtra(ContactSaveService.EXTRA_ACCOUNT_NAME);
        String dataSet = intent.getStringExtra(ContactSaveService.EXTRA_DATA_SET);
        String label = intent.getStringExtra(ContactSaveService.EXTRA_GROUP_LABEL);
        final long[] rawContactsToAdd = intent.getLongArrayExtra(
                ContactSaveService.EXTRA_RAW_CONTACTS_TO_ADD);

        Log.d(TAG, "[createGroup] groupName:" + Log.anonymize(label)
                + ", accountName:" + Log.anonymize(accountName)
                + ", accountType:" + Log.anonymize(accountType));
        Intent callbackIntent = intent
                .getParcelableExtra(ContactSaveService.EXTRA_CALLBACK_INTENT);
        if (TextUtils.isEmpty(label)) {
            Log.w(TAG, "[createGroup]Group name can't be empty!");
            callbackIntent.putExtra(ContactEditorFragment.SAVE_MODE_EXTRA_KEY, SaveMode.RELOAD);
            deliverCallback(callbackIntent);
            return ;
        }
        if (!SimGroupUtils.checkGroupNameExist(mContext, label, accountName, accountType)) {
            Log.w(TAG, "[createGroup]Group Name exist!");
            callbackIntent.putExtra(ContactEditorFragment.SAVE_MODE_EXTRA_KEY, SaveMode.RELOAD);
            deliverCallback(callbackIntent);
            return;
        }

        int[] rawContactsIndexInIcc = intent
                .getIntArrayExtra(SimGroupUtils.EXTRA_SIM_INDEX_ARRAY);
        int subId = intent.getIntExtra(SimGroupUtils.EXTRA_SUB_ID, -1);
        int groupIdInIcc = -1;
        if (subId > 0) {
            groupIdInIcc = SimGroupUtils.createGroupToIcc(mContext, intent, callbackIntent);
            if (groupIdInIcc < 0) {
                Log.w(TAG, "[createGroup]createGroupToIcc fail!");
                callbackIntent.putExtra(ContactEditorFragment.SAVE_MODE_EXTRA_KEY, SaveMode.RELOAD);
                deliverCallback(callbackIntent);
                return;
            }
        }

        ContentValues values = new ContentValues();
        values.put(Groups.ACCOUNT_TYPE, accountType);
        values.put(Groups.ACCOUNT_NAME, accountName);
        values.put(Groups.DATA_SET, dataSet);
        values.put(Groups.TITLE, label);

        final ContentResolver resolver = mContext.getContentResolver();

        // Create the new group
        final Uri groupUri = resolver.insert(Groups.CONTENT_URI, values);

        // If there's no URI, then the insertion failed. Abort early because group members can't be
        // added if the group doesn't exist
        if (groupUri == null) {
            Log.e(TAG, "[createGroup]Couldn't create group with label "
                    + Log.anonymize(label));
            return;
        }

        boolean isSuccess = addMembersToGroup(resolver, rawContactsToAdd, ContentUris
                .parseId(groupUri), rawContactsIndexInIcc, intent, groupIdInIcc);
        // fix ALPS921231,check if usim have been removed after save
        if (subId > 0 && !SubInfoUtils.isActiveForSubscriber(subId)) {
            Log.w(TAG, "[createGroup] Sim card is not ready");
            if (SimGroupUtils.showMoveUSIMGroupErrorToast(GROUP_SIM_ABSENT, subId)
                    && callbackIntent != null) {
                callbackIntent.putExtra(SimGroupUtils.EXTRA_DONE_TOAST, true);
            }
            deliverCallback(callbackIntent);
            return;
        }

        // TODO: Move this into the contact editor where it belongs. This needs to be integrated
        // with the way other intent extras that are passed to the {@link ContactEditorActivity}.
        values.clear();
        values.put(Data.MIMETYPE, GroupMembership.CONTENT_ITEM_TYPE);
        values.put(GroupMembership.GROUP_ROW_ID, ContentUris.parseId(groupUri));

        Uri groupUriReture = isSuccess ? groupUri : null;
        callbackIntent.setData(groupUriReture);
        callbackIntent.putExtra(ContactsContract.Intents.Insert.DATA, Lists.newArrayList(values));
        deliverCallback(callbackIntent);
    }

    private void renameGroup(Intent intent) {
        long groupId = intent.getLongExtra(ContactSaveService.EXTRA_GROUP_ID, -1);
        if (groupId == -1) {
            Log.e(TAG, "[renameGroup]Invalid arguments for renameGroup request");
            return;
        }
        String label = intent.getStringExtra(ContactSaveService.EXTRA_GROUP_LABEL);
        Intent callbackIntent = intent.getParcelableExtra(
                ContactSaveService.EXTRA_CALLBACK_INTENT);
        if (TextUtils.isEmpty(label)) {
            Log.w(TAG, "[renameGroup]Group name can't be empty!");
            deliverCallback(callbackIntent);
            return ;
        }

        // update Icc card
        int subId = intent.getIntExtra(SimGroupUtils.EXTRA_SUB_ID, -1);
        int groupIdInIcc = -1;
        if (subId > 0) {
            groupIdInIcc = SimGroupUtils.updateGroupToIcc(mContext, intent, callbackIntent);
            if (groupIdInIcc < 0) {
                Log.w(TAG, "[renameGroup] update to Icc fail!");
                deliverCallback(callbackIntent);
                return;
            }
        }

        // update db
        ContentValues values = new ContentValues();
        values.put(Groups.TITLE, label);
        final Uri groupUri = ContentUris.withAppendedId(Groups.CONTENT_URI, groupId);
        Log.d(TAG, "[renameGroup]update to db, group uri = " + Log.anonymize(groupUri));
        mContext.getContentResolver().update(groupUri, values, null, null);

        callbackIntent.setData(groupUri);
        deliverCallback(callbackIntent);
    }

    private void deleteGroup(Intent intent) {
        /// M: [Sim Group][Google Issue]ALPS00463033 @{
        if (ContactSaveService.sDeleteEndListener != null) {
            ContactSaveService.sDeleteEndListener.onDeleteStart();
        }
        /// @}

        do {
            long groupId = intent.getLongExtra(ContactSaveService.EXTRA_GROUP_ID, -1);
            if (groupId == -1) {
                Log.e(TAG, "[deleteGroup]Invalid arguments for deleteGroup request");
                break;
            }
            final Uri groupUri = ContentUris.withAppendedId(Groups.CONTENT_URI, groupId);
            int subId = intent.getIntExtra(SimGroupUtils.EXTRA_SUB_ID, -1);

            //get group undo data for undo function
            Log.i(TAG, "[deleteGroup]get group undo data for undo function");
            final Intent callbackIntent = new Intent(ContactSaveService.BROADCAST_GROUP_DELETED);
            final Bundle undoData = mGroupsDao.captureDeletionUndoData(groupUri);
            callbackIntent.putExtra(ContactSaveService.EXTRA_UNDO_ACTION,
                    ContactSaveService.ACTION_DELETE_GROUP);
            callbackIntent.putExtra(ContactSaveService.EXTRA_UNDO_DATA, undoData);
            callbackIntent.putExtra(SimGroupUtils.EXTRA_SUB_ID, subId);

            //delete group in usim
            String groupLabel = intent.getStringExtra(ContactSaveService.EXTRA_GROUP_LABEL);
            Log.i(TAG, "[deleteGroup]groupLabel:" + groupLabel + ",subId:" + subId);
            if (subId > 0 && !TextUtils.isEmpty(groupLabel)) {
                boolean success = SimGroupUtils.deleteGroupInIcc(mContext, intent, groupId);
                if (!success) {
                    Log.w(TAG, "[deleteGroup] delete gorup in Icc is fail, return");
                    break;
                }
            }

            //delete group in db
            mGroupsDao.delete(groupUri);

            //send group undo data
            LocalBroadcastManager.getInstance(mContext).sendBroadcast(callbackIntent);
        } while (false);

        /// M: [Sim Group][Google Issue]ALPS00463033 @{
        if (ContactSaveService.sDeleteEndListener != null) {
            Log.d(TAG, "[deleteGroup]onDeleteEnd");
            ContactSaveService.sDeleteEndListener.onDeleteEnd();
        }
        /// @}
    }

    private void undo(Intent intent) {
        final String actionToUndo = intent.getStringExtra(ContactSaveService.EXTRA_UNDO_ACTION);
        if (ContactSaveService.ACTION_DELETE_GROUP.equals(actionToUndo)) {
            mGroupsDao.undoDeletion(intent.getBundleExtra(ContactSaveService.EXTRA_UNDO_DATA));
        }
    }

    private void updateGroup(Intent intent) {
        long groupId = intent.getLongExtra(ContactSaveService.EXTRA_GROUP_ID, -1);
        if (groupId == -1) {
            Log.e(TAG, "[updateGroup]Invalid arguments for updateGroup request");
            return;
        }
        Intent callbackIntent = intent.getParcelableExtra(ContactSaveService.EXTRA_CALLBACK_INTENT);

        //check group exist
        String label = intent.getStringExtra(ContactSaveService.EXTRA_GROUP_LABEL);
        String accountType = intent.getStringExtra(ContactSaveService.EXTRA_ACCOUNT_TYPE);
        String accountName = intent.getStringExtra(ContactSaveService.EXTRA_ACCOUNT_NAME);
        if (groupId > 0 && label != null && !SimGroupUtils.checkGroupNameExist(
                    mContext, label, accountName, accountType)) {
            Log.w(TAG, "[updateGroup] Group Name exist!");
            callbackIntent.putExtra(ContactEditorFragment.SAVE_MODE_EXTRA_KEY, SaveMode.RELOAD);
            deliverCallback(callbackIntent);
            return;
        }

        // update group name to usim
        int subId = intent.getIntExtra(SimGroupUtils.EXTRA_SUB_ID, -1);
        int groupIdInIcc = -1;
        if (subId > 0) {
            groupIdInIcc = SimGroupUtils.updateGroupToIcc(mContext, intent, callbackIntent);
            if (groupIdInIcc < 0) {
                Log.w(TAG, "[updateGroup] groupIdInIcc fail!");
                callbackIntent.putExtra(ContactEditorFragment.SAVE_MODE_EXTRA_KEY, SaveMode.RELOAD);
                deliverCallback(callbackIntent);
                return;
            }
        }

        // Update group name to db if necessary
        final ContentResolver resolver = mContext.getContentResolver();
        final Uri groupUri = ContentUris.withAppendedId(Groups.CONTENT_URI, groupId);
        if (label != null) {
            ContentValues values = new ContentValues();
            values.put(Groups.TITLE, label);
            resolver.update(groupUri, values, null, null);
        }

        // Add and remove members if necessary
        long[] rawContactsToAdd = intent.getLongArrayExtra(
                ContactSaveService.EXTRA_RAW_CONTACTS_TO_ADD);
        long[] rawContactsToRemove = intent.getLongArrayExtra(
                ContactSaveService.EXTRA_RAW_CONTACTS_TO_REMOVE);
        int[] simIndexToAddArray = intent
                .getIntArrayExtra(SimGroupUtils.EXTRA_SIM_INDEX_TO_ADD);
        int[] simIndexToRemoveArray = intent
                .getIntArrayExtra(SimGroupUtils.EXTRA_SIM_INDEX_TO_REMOVE);
        boolean isRemoveSuccess = removeMembersFromGroup(resolver, rawContactsToRemove, groupId,
                simIndexToRemoveArray, subId, groupIdInIcc);
        boolean isAddSuccess = addMembersToGroup(resolver, rawContactsToAdd, groupId,
                simIndexToAddArray, intent, groupIdInIcc);
        // fix ALPS921231 check if sim removed after save
        if (subId > 0 && !SubInfoUtils.isActiveForSubscriber(subId)) {
            Log.w(TAG, "[updateGroup] Find sim not ready");
            if (SimGroupUtils.showMoveUSIMGroupErrorToast(GROUP_SIM_ABSENT, subId)
                    && callbackIntent != null) {
                callbackIntent.putExtra(SimGroupUtils.EXTRA_DONE_TOAST, true);
            }
            deliverCallback(callbackIntent);
            return;
        }

        // make sure both remove and add are successful
        Log.i(TAG, "[updateGroup]isAddSuccess:" + isAddSuccess +
                ", isRemoveSuccess:" + isRemoveSuccess + ", groupUri:" + Log.anonymize(groupUri));
        callbackIntent.setData(groupUri);
        deliverCallback(callbackIntent);
    }

    /**
     * true if all are ok,false happened some errors.
     */
    private static boolean addMembersToGroup(ContentResolver resolver, long[] rawContactsToAdd,
            long groupId, int[] rawContactsIndexInIcc, Intent intent, int groupIdInIcc) {
        boolean isAllOk = true;
        if (rawContactsToAdd == null) {
            Log.e(TAG, "[addMembersToGroup] no members to add");
            return true;
        }
        // add members to usim
        int subId = intent.getIntExtra(SimGroupUtils.EXTRA_SUB_ID, -1);
        int i = -1;
        for (long rawContactId : rawContactsToAdd) {
            try {
                // add members to usim first
                i++;
                if (subId > 0 && groupIdInIcc >= 0 && rawContactsIndexInIcc[i] >= 0) {
                    int simIndex = rawContactsIndexInIcc[i];
                    boolean success = ContactsGroupUtils.USIMGroup.addUSIMGroupMember(subId,
                            simIndex, groupIdInIcc);
                    if (!success) {
                        isAllOk = false;
                        Log.w(TAG, "[addMembersToGroup] fail simIndex:" + simIndex
                                + ",groupId:" + groupId);
                        continue;
                    }
                }

                final ArrayList<ContentProviderOperation> rawContactOperations =
                        new ArrayList<ContentProviderOperation>();

                // Build an assert operation to ensure the contact is not already in the group
                final ContentProviderOperation.Builder assertBuilder = ContentProviderOperation
                        .newAssertQuery(Data.CONTENT_URI);
                assertBuilder.withSelection(Data.RAW_CONTACT_ID + "=? AND " +
                        Data.MIMETYPE + "=? AND " + GroupMembership.GROUP_ROW_ID + "=?",
                        new String[] { String.valueOf(rawContactId),
                        GroupMembership.CONTENT_ITEM_TYPE, String.valueOf(groupId)});
                assertBuilder.withExpectedCount(0);
                rawContactOperations.add(assertBuilder.build());

                // Build an insert operation to add the contact to the group
                final ContentProviderOperation.Builder insertBuilder = ContentProviderOperation
                        .newInsert(Data.CONTENT_URI);
                insertBuilder.withValue(Data.RAW_CONTACT_ID, rawContactId);
                insertBuilder.withValue(Data.MIMETYPE, GroupMembership.CONTENT_ITEM_TYPE);
                insertBuilder.withValue(GroupMembership.GROUP_ROW_ID, groupId);
                rawContactOperations.add(insertBuilder.build());

                // Apply batch
                if (!rawContactOperations.isEmpty()) {
                    resolver.applyBatch(ContactsContract.AUTHORITY, rawContactOperations);
                }
            } catch (RemoteException e) {
                // Something went wrong, bail without success
                Log.e(TAG, "[addMembersToGroup]Problem persisting user edits for raw contact ID " +
                        String.valueOf(rawContactId), e);
                isAllOk = false;
            } catch (OperationApplicationException e) {
                // The assert could have failed because the contact is already in the group,
                // just continue to the next contact
                Log.w(TAG, "[addMembersToGroup] Assert failed in adding raw contact ID " +
                        String.valueOf(rawContactId) + ". Already exists in group " +
                        String.valueOf(groupId), e);
                isAllOk = false;
            }
        }
        return isAllOk;
    }

    // To remove USIM group members and contactsProvider if necessary.
    private boolean removeMembersFromGroup(ContentResolver resolver, long[] rawContactsToRemove,
            long groupId, int[] simIndexArray, int subId, int ugrpId) {
        boolean isRemoveSuccess = true;
        if (rawContactsToRemove == null) {
            Log.w(TAG, "[removeMembersFromGroup]RawContacts to be removed is empty!");
            return isRemoveSuccess;
        }

        int simIndex;
        int i = -1;
        for (long rawContactId : rawContactsToRemove) {
            // remove group member from icc card
            i++;
            simIndex = simIndexArray[i];
            boolean ret = false;
            if (subId > 0 && simIndex >= 0 && ugrpId >= 0) {
                ret = ContactsGroupUtils.USIMGroup.deleteUSIMGroupMember(subId, simIndex, ugrpId);
                if (!ret) {
                    isRemoveSuccess = false;
                    Log.i(TAG, "[removeMembersFromGroup]Remove failed RawContactid: "
                            + rawContactId);
                    continue;
                }
            }

            // Apply the delete operation on the data row for the given raw contact's
            // membership in the given group. If no contact matches the provided selection, then
            // nothing will be done. Just continue to the next contact.
            resolver.delete(Data.CONTENT_URI, Data.RAW_CONTACT_ID + "=? AND " +
                    Data.MIMETYPE + "=? AND " + GroupMembership.GROUP_ROW_ID + "=?",
                    new String[] { String.valueOf(rawContactId),
                    GroupMembership.CONTENT_ITEM_TYPE, String.valueOf(groupId)});
        }
        return isRemoveSuccess;
    }

    private void deliverCallbackOnUiThread(final Intent intent) {
        Log.d(TAG, "[deliverCallbackOnUiThread] callbackIntent call onSimGroupCompleted");
        if (mMainHandler == null) {
            Log.d(TAG, "[deliverCallbackOnUiThread] mMainHandler is null, can not callback");
            return;
        }
        for (final Listener listener : sListeners) {
            mMainHandler.post(new Runnable() {
                public void run() {
                    listener.onSimGroupCompleted(intent);
                }
            });
        }
    }

    private void deliverCallback(Intent callbackIntent) {
        deliverCallbackOnUiThread(callbackIntent);
    }

    public static class SimGroupsDaoImpl extends GroupsDaoImpl {

        private static final String TAG = "SimGroupsDaoImpl";
        public static final String KEY_GROUP_MEMBERS_SIM = "groupMemberIndexInSims";
        private int mSubId = SubInfoUtils.getInvalidSubId();

        public SimGroupsDaoImpl(Context context, int subId) {
            super(context);
            mSubId = subId;
            Log.d(TAG, "mSubId = " + mSubId);
        }

        @Override
        public Bundle captureDeletionUndoData(Uri groupUri) {
             return captureSimDeletionUndoData(
                     super.captureDeletionUndoData(groupUri));
        }

        @Override
        public Uri undoDeletion(Bundle deletedGroupData) {
            // undo deletion in sim
            if (!undoDeletionInSim(deletedGroupData)) {
                Log.d(TAG, "undoDeletionInSim fail, no need undoDeletion in db, return");
                return null;
            }
            // undo deletion in db
            return super.undoDeletion(deletedGroupData);
        }

        private Bundle captureSimDeletionUndoData(Bundle data) {
            data.putIntArray(KEY_GROUP_MEMBERS_SIM, getIndexInSims(data));
            return data;
        }

        private int[] getIndexInSims(Bundle data) {
            final ContentValues groupData = data.getParcelable(KEY_GROUP_DATA);
            final long[] rawContactIds = data.getLongArray(KEY_GROUP_MEMBERS);
            if (rawContactIds == null || rawContactIds.length == 0) {
                Log.d(TAG, "[getIndexInSims] no raw contact, return null");
                return null;
            }

            final Uri.Builder builder = RawContacts.CONTENT_URI.buildUpon();
            String accountName = groupData.getAsString(Groups.ACCOUNT_NAME);
            String accountType = groupData.getAsString(Groups.ACCOUNT_TYPE);
            if (accountName != null && accountType != null) {
                builder.appendQueryParameter(RawContacts.ACCOUNT_NAME, accountName);
                builder.appendQueryParameter(RawContacts.ACCOUNT_TYPE, accountType);
            }
            String dataSet = groupData.getAsString(Groups.DATA_SET);
            if (dataSet != null) {
                builder.appendQueryParameter(RawContacts.DATA_SET, dataSet);
            }
            final Uri rawContactUri = builder.build();
            final String[] projection = new String[]{
                    MtkContactsContract.RawContactsColumns.INDEX_IN_SIM // column index = 0
                    };
            final StringBuilder selection = new StringBuilder();
            final String[] selectionArgs = new String[rawContactIds.length];
            for (int i = 0; i < rawContactIds.length; i++) {
                if (i > 0) {
                    selection.append(" OR ");
                }
                selection.append(ContactsContract.RawContacts._ID).append("=?");
                selectionArgs[i] = Long.toString(rawContactIds[i]);
            }
            final Cursor cursor = getContext().getContentResolver().query(
                    rawContactUri, projection, selection.toString(), selectionArgs, null, null);
            if (cursor == null || cursor.getCount() < 1) {
                Log.d(TAG, "[getIndexInSims] cursor is empty, return null");
                return null;
            }
            final int[] indexInSims = new int[cursor.getCount()];
            try {
                int i = 0;
                cursor.moveToPosition(-1);
                while (cursor.moveToNext()) {
                    indexInSims[i] = cursor.getInt(0);
                    i++;
                }
            } finally {
                cursor.close();
            }
            Log.d(TAG, "[getIndexInSims] rawContactIds = " + Arrays.toString(rawContactIds)
                    + ", indexInSims = " + Arrays.toString(indexInSims));
            return indexInSims;
        }

        private boolean undoDeletionInSim(Bundle deletedGroupData) {
            //re-create group in sim
            final ContentValues groupData = deletedGroupData.getParcelable(KEY_GROUP_DATA);
            if (groupData == null) {
                Log.d(TAG, "[undoSimDeletion]groupData is null");
                return false;
            }
            String label = groupData.getAsString(Groups.TITLE);
            if (TextUtils.isEmpty(label)) {
                Log.d(TAG, "[undoSimDeletion]group name is empty");
                return false;
            }
            String accountType = groupData.getAsString(Groups.ACCOUNT_TYPE);
            String accountName = groupData.getAsString(Groups.ACCOUNT_NAME);
            String dataSet = groupData.getAsString(Groups.DATA_SET);
            if (!SimGroupUtils.checkGroupNameExist(getContext(), label, accountName,
                    accountType)) {
                Log.w(TAG, "[undoSimDeletion]Group Name exist!");
                return false;
            }
            int groupIdInIcc = SimGroupUtils.createGroupToIcc(label, mSubId);
            if (groupIdInIcc < 0) {
                Log.w(TAG, "[createGroup]createGroupToIcc fail!");
                return false;
            }

            //re-add members to group in sim
            int[] membersIndexInSim = deletedGroupData.getIntArray(KEY_GROUP_MEMBERS_SIM);
            return addMembersToGroupInSim(membersIndexInSim, groupIdInIcc);
        }

        private boolean addMembersToGroupInSim(int[] indexInSimsToAdd, int groupIdInIcc) {
            if (indexInSimsToAdd == null || indexInSimsToAdd.length == 0) {
                Log.e(TAG, "[addMembersToGroupInSim] no members to add");
                return true;
            }
            if (mSubId <= 0 || groupIdInIcc < 0) {
                Log.e(TAG, "[addMembersToGroupInSim] parameter invalid, mSubId=" + mSubId
                        + ", groupIdInIcc=" + groupIdInIcc);
                return false;
            }
            boolean isAllOk = true;
            for (int indexInSim : indexInSimsToAdd) {
                boolean success = ContactsGroupUtils.USIMGroup.addUSIMGroupMember(mSubId,
                        indexInSim, groupIdInIcc);
                if (!success) {
                    isAllOk = false;
                    Log.w(TAG, "[addMembersToGroupInSim] add fail, simIndex:" + indexInSim
                            + ",groupIdInIcc:" + groupIdInIcc);
                }
            }
            return isAllOk;
        }
    }
}
