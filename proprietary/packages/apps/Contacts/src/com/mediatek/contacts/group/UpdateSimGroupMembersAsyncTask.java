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

import java.util.ArrayList;
import java.util.List;

import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.provider.ContactsContract;
import android.provider.ContactsContract.RawContacts;
import android.widget.Toast;

import com.android.contacts.ContactSaveService;
import com.android.contacts.activities.PeopleActivity;
import com.android.contacts.group.GroupUtil;
import com.android.contacts.group.UpdateGroupMembersAsyncTask;
import com.android.contacts.model.account.AccountWithDataSet;
import com.mediatek.contacts.simcontact.SubInfoUtils;
import com.mediatek.contacts.util.Log;
import com.mediatek.provider.MtkContactsContract;

public class UpdateSimGroupMembersAsyncTask extends UpdateGroupMembersAsyncTask {

    private static final String TAG = "UpdateSimGroupMembersAsyncTask";
    private String mGroupName;
    private int mSubId = SubInfoUtils.getInvalidSubId();

    public UpdateSimGroupMembersAsyncTask(int type, Context context, long[] contactIds,
            long groupId, String accountName, String accountType, String dataSet,
            String groupName, int subId) {
        super(type, context, contactIds, groupId, accountName, accountType, dataSet);
        mGroupName = groupName;
        mSubId = subId;
    }

    /**
     * The member that to add to a group or remove from a group.
     */
    private class Member {
        private final long mRawContactId;
        private final int mIndexInSim;

        public Member(long rawContactId, int indexInSim) {
            mRawContactId = rawContactId;
            mIndexInSim = indexInSim;
        }
    }

    @Override
    protected Intent doInBackground(Void... params) {
        final List<Member> members = getMembers();
        Log.d(TAG, "[doInBackground] members count = " + members.size()
                + ", mType = " + getType() + ", mGroupId = " + getGroupId()
                + ", mGroupName = " + Log.anonymize(mGroupName) + ", mSubId = " + mSubId
                + ", mAccountName = " + Log.anonymize(getAccountName())
                + ", mAccountType = " + getAccountType());
        if (members.isEmpty()) {
            return null;
        }
        final long[] rawContactIds = getRawContactIdArray(members);
        final int[] indexInSims = getIndexInSimArray(members);

        final long[] rawContactIdsToAdd;
        final long[] rawContactIdsToRemove;
        final int [] indexInSimsToAdd;
        final int [] indexInSimsToRemove;
        final String action;

        if (isTypeAdd()) {
            rawContactIdsToAdd = rawContactIds;
            rawContactIdsToRemove = null;
            indexInSimsToAdd = indexInSims;
            indexInSimsToRemove = null;
            action = GroupUtil.ACTION_ADD_TO_GROUP;
        } else if (isTypeRemove()) {
            rawContactIdsToAdd = null;
            rawContactIdsToRemove = rawContactIds;
            indexInSimsToAdd = null;
            indexInSimsToRemove = indexInSims;
            action = GroupUtil.ACTION_REMOVE_FROM_GROUP;
        } else {
            throw new IllegalStateException("Unrecognized type " + getType());
        }
        return SimGroupUtils.createGroupUpdateIntentForIcc(
                getContext(), getGroupId(), /* newLabel */ null,
                rawContactIdsToAdd, rawContactIdsToRemove,
                PeopleActivity.class, action, mGroupName,
                mSubId, indexInSimsToAdd, indexInSimsToRemove,
                new AccountWithDataSet(getAccountName(), getAccountType(),
                        getDataSet()));
    }

    private List<Member> getMembers() {
        final Uri.Builder builder = RawContacts.CONTENT_URI.buildUpon();
        if (getAccountName() != null) {
            builder.appendQueryParameter(RawContacts.ACCOUNT_NAME, getAccountName());
            builder.appendQueryParameter(RawContacts.ACCOUNT_TYPE, getAccountType());
        }
        if (getDataSet() != null) {
            builder.appendQueryParameter(RawContacts.DATA_SET, getDataSet());
        }
        final Uri rawContactUri = builder.build();
        final String[] projection = new String[]{
                ContactsContract.RawContacts._ID,          // column index = 0
                MtkContactsContract.RawContactsColumns.INDEX_IN_SIM // column index = 1
                };
        final StringBuilder selection = new StringBuilder();
        final long[] contactIds = getContactIds();
        final String[] selectionArgs = new String[contactIds.length];
        for (int i = 0; i < contactIds.length; i++) {
            if (i > 0) {
                selection.append(" OR ");
            }
            selection.append(ContactsContract.RawContacts.CONTACT_ID).append("=?");
            selectionArgs[i] = Long.toString(contactIds[i]);
        }
        final Cursor cursor = getContext().getContentResolver().query(
                rawContactUri, projection, selection.toString(), selectionArgs, null, null);
        final List<Member> members = new ArrayList<Member>(cursor.getCount());
        try {
            while (cursor.moveToNext()) {
                members.add(new Member(cursor.getLong(0), cursor.getInt(1)));
            }
        } finally {
            cursor.close();
        }
        Log.d(TAG, "[getMembers] members count is " + members.size());
        return members;
    }

    private static long[] getRawContactIdArray(List<Member> listMembers) {
        int size = listMembers.size();
        long[] rawContactIds = new long[size];
        for (int i = 0; i < size; i++) {
            rawContactIds[i] = listMembers.get(i).mRawContactId;
        }
        return rawContactIds;
    }

    private static int[] getIndexInSimArray(List<Member> listMembers) {
        int size = listMembers.size();
        int[] indexInSims = new int[size];
        for (int i = 0; i < size; i++) {
            indexInSims[i] = listMembers.get(i).mIndexInSim;
        }
        return indexInSims;
    }
}
