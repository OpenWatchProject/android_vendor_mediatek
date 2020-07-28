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
package com.mediatek.contacts.list;

import android.app.Activity;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.os.Bundle;
import android.provider.ContactsContract.CommonDataKinds.Email;
import android.provider.ContactsContract.CommonDataKinds.GroupMembership;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.ContactsContract.Data;
import android.text.TextUtils;
import android.util.SparseBooleanArray;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.ListView;

import com.android.contacts.R;
import com.android.contacts.util.WeakAsyncTask;
import com.android.contacts.group.GroupListItem;

import com.mediatek.contacts.group.GroupBrowseListAdapter;
import com.mediatek.contacts.group.GroupBrowseListFragment;
import com.mediatek.contacts.util.Log;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class ContactGroupBrowseListFragment extends GroupBrowseListFragment {
    private static final String TAG = "ContactGroupBrowseListFragment";

    private Context mContext;

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        Log.i(TAG, "[onActivityCreated]");
        super.onActivityCreated(savedInstanceState);
        getListView().setChoiceMode(ListView.CHOICE_MODE_MULTIPLE);
    }

    @Override
    public void onAttach(Activity activity) {
        super.onAttach(activity);
        mContext = activity;
    }

    @Override
    public void onDetach() {
        super.onDetach();
        mContext = null;
    }

    @Override
    protected GroupBrowseListAdapter configAdapter() {
        return new ContactGroupListAdapter(mContext);
    }

    @Override
    protected OnItemClickListener configOnItemClickListener() {
        return new OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                CheckBox checkbox = (CheckBox) view.findViewById(android.R.id.checkbox);
                checkbox.setChecked(getListView().isItemChecked(position));
                updateOkButton(getListView().getCheckedItemCount() > 0);
            }
        };
    }

    @Override
    public void onStart() {
        // get check-box states for reseting check-box when switch screen.
        SparseBooleanArray checkstates = getListView().getCheckedItemPositions();
        if (checkstates != null) {
            ContactGroupListAdapter adapter = (ContactGroupListAdapter) getListView().getAdapter();
            adapter.setSparseBooleanArray(checkstates);
            updateOkButton(checkstates.size() > 0);
        }
        super.onStart();
    }

    public void onOkClick() {
        if (getListView().getCheckedItemCount() == 0) {
            Log.w(TAG, "[onOkClick]tap OK when no item selected");
            getActivity().setResult(Activity.RESULT_CANCELED, null);
            getActivity().finish();
            return;
        }
        List<String> selectedGroupIdList = new ArrayList<String>();
        GroupBrowseListAdapter adapter = (GroupBrowseListAdapter) getListView().getAdapter();
        int listSize = getListView().getCount();

        for (int position = 0; position < listSize; ++position) {
            if (getListView().isItemChecked(position)) {
                GroupListItem item = adapter.getItem(position);
                if (item != null) {
                    selectedGroupIdList.add(String.valueOf(item.getGroupId()));
                }
            } else {
                Log.w(TAG, "[onOkClick]position " + position + " item is not checked");
            }
        }

        if (selectedGroupIdList.isEmpty()) {
            Log.w(TAG, "[onOkClick]finally, no group selected");
            getActivity().setResult(Activity.RESULT_CANCELED, null);
            getActivity().finish();
            return;
        }
        new GroupQueryTask(getActivity()).execute(selectedGroupIdList);
    }

    public class GroupQueryTask extends WeakAsyncTask<List<String>, Void, long[], Activity> {
        private WeakReference<ProgressDialog> mProgress;

        public GroupQueryTask(Activity target) {
            super(target);
        }

        @Override
        protected void onPreExecute(final Activity target) {
            Log.d(TAG, "[onPreExecute]");
            ProgressDialog progressDlg = ProgressDialog.show(target, null,
                    target.getText(R.string.please_wait), false, false);
            mProgress = new WeakReference<ProgressDialog>(progressDlg);
            super.onPreExecute(target);
        }

        @Override
        protected long[] doInBackground(Activity target, List<String>... params) {
            // /M:fix ALPS00559786,refactor sqlite query,use a nested query to
            // improve
            List<String> groupIdList = params[0];
            Log.d(TAG, "[doInBackground]groupIdList = " + groupIdList);
            if (groupIdList == null || groupIdList.isEmpty()) {
                Log.e(TAG, "[doInBackground] groupIds is empty");
                return null;
            }

            // ArrayList<String> rawContactIdsList = new ArrayList<String>();
            ArrayList<Long> phoneIdsList = new ArrayList<Long>();

            final StringBuilder whereBuilder = new StringBuilder();

            final String[] groupQuestionMarks = new String[groupIdList.size()];
            Arrays.fill(groupQuestionMarks, "?");

            whereBuilder.append(Data.MIMETYPE + "='" + GroupMembership.CONTENT_ITEM_TYPE + "'");
            whereBuilder.append(" AND ");
            whereBuilder.append(Data.DATA1 + " IN (");
            whereBuilder.append(TextUtils.join(",", groupQuestionMarks));
            whereBuilder.append(")");
            String sql = "select " + Data.RAW_CONTACT_ID + " from view_data where (" + whereBuilder
                    + ")";

            whereBuilder.delete(0, whereBuilder.length());

            whereBuilder.append("(" + Data.MIMETYPE + " ='");
            whereBuilder.append(Phone.CONTENT_ITEM_TYPE + "' OR ");
            whereBuilder.append(Data.MIMETYPE + " ='");
            whereBuilder.append(Email.CONTENT_ITEM_TYPE + "') ");
            whereBuilder.append("AND " + Data.RAW_CONTACT_ID + " IN (" + sql);
            whereBuilder.append(")");

            Cursor cursor = null;
            try {
                cursor = mContext.getContentResolver().query(Data.CONTENT_URI,
                        new String[] { Data._ID }, whereBuilder.toString(),
                        groupIdList.toArray(new String[groupIdList.size()]), null);
                if (cursor != null) {
                    cursor.moveToPosition(-1);
                    while (cursor.moveToNext()) {
                        long id = cursor.getLong(0);
                        phoneIdsList.add(Long.valueOf(id));
                    }
                }
            } finally {
                if (cursor != null) {
                    cursor.close();
                    cursor = null;
                }
            }

            long[] phoneIds = new long[phoneIdsList.size()];
            int index = 0;
            for (Long id : phoneIdsList) {
                phoneIds[index++] = id.longValue();
            }

            return phoneIds;
        }

        @Override
        protected void onPostExecute(final Activity target, long[] ids) {
            Log.d(TAG, "[onPostExecute]");
            final ProgressDialog progress = mProgress.get();
            if (!target.isFinishing() && progress != null && progress.isShowing()) {
                progress.dismiss();
            }
            super.onPostExecute(target, ids);
            if (ids == null || ids.length == 0) {
                getActivity().setResult(Activity.RESULT_CANCELED, new Intent());
            } else {
                getActivity().setResult(
                        ContactListMultiChoiceActivity.CONTACTGROUPLISTACTIVITY_RESULT_CODE,
                        new Intent().putExtra("checkedids", ids));
            }
            target.finish();
        }
    }

    private void updateOkButton(boolean enabled) {
        Button btnOk = (Button) getActivity().findViewById(R.id.btn_ok);
        btnOk.setEnabled(enabled);
    }
}
