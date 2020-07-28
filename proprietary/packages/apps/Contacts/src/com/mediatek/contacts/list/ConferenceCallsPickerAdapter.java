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

import android.content.Context;
import android.content.CursorLoader;
import android.database.Cursor;
import android.net.Uri;
import android.net.Uri.Builder;
import android.provider.ContactsContract;
import android.provider.ContactsContract.CommonDataKinds.Callable;
import android.provider.ContactsContract.CommonDataKinds.SipAddress;
import android.provider.ContactsContract.Data;
import android.provider.ContactsContract.Directory;
import android.text.TextUtils;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.CheckBox;
import android.widget.ListView;
import android.widget.TextView;

import com.android.contacts.R;
import com.android.contacts.list.ContactListFilter;
import com.android.contacts.list.ContactListItemView;

import com.mediatek.contacts.util.ContactsIntent;
import com.mediatek.contacts.util.Log;
import com.mediatek.contacts.util.MtkToast;

import java.util.ArrayList;
import java.util.List;

public class ConferenceCallsPickerAdapter extends PhoneNumbersPickerAdapter {
    private static final String TAG = "ConferenceCallsPickerAdapter";

    public static final Uri PICK_CONFERENCE_CALL_URI = Callable.CONTENT_URI;
    public static final Uri PICK_CONFERENCE_CALL_FILTER_URI = Callable.CONTENT_FILTER_URI;
    private int mReferenceCallMaxNumber = ContactsIntent.CONFERENCE_CALL_LIMITES;
    private Context mContext;

    public ConferenceCallsPickerAdapter(Context context, ListView lv) {
        super(context, lv);
        mContext = context;
    }

    public void setRefenceCallMaxNumber(int num) {
        mReferenceCallMaxNumber = num;
    }

    @Override
    protected Uri configLoaderUri(long directoryId) {
        Uri uri;
        boolean isSearchMode = isSearchMode();
        Log.i(TAG, "[configLoaderUri]directoryId = " + directoryId + ",isSearchMode = "
                + isSearchMode);

        if (directoryId != Directory.DEFAULT) {
            Log.w(TAG, "[configLoaderUri] is not ready for non-default directory ID");
        }

        if (isSearchMode) {
            String query = getQueryString();
            Builder builder = PICK_CONFERENCE_CALL_FILTER_URI.buildUpon();
            if (TextUtils.isEmpty(query)) {
                builder.appendPath("");
            } else {
                builder.appendPath(query); // Builder will encode the query
            }

            builder.appendQueryParameter(ContactsContract.DIRECTORY_PARAM_KEY,
                    String.valueOf(directoryId));
            builder.appendQueryParameter("checked_ids_arg", PICK_CONFERENCE_CALL_URI.toString());
            uri = builder.build();
        } else {
            uri = PICK_CONFERENCE_CALL_URI;
            uri = uri
                    .buildUpon()
                    .appendQueryParameter(ContactsContract.DIRECTORY_PARAM_KEY,
                            String.valueOf(directoryId)).build();
            if (isSectionHeaderDisplayEnabled()) {
                uri = buildSectionIndexerUri(uri);
            }
        }

        return uri;
    }

    @Override
    protected void bindData(ContactListItemView view, Cursor cursor) {
        TextView emptyLabel = view.getLabelView();
        emptyLabel.setText("");
        super.bindData(view, cursor);
    }

    @Override
    public void bindView(View itemView, int partition, Cursor cursor, int position) {
        Log.d(TAG, "[bindView]position = " + position + ",partition = " + partition);
        super.bindView(itemView, partition, cursor, position);
        final CheckBox checkBox = ((ContactListItemView) itemView).getCheckBox();
        checkBox.setOnClickListener(mCheckBoxListener);
    }

    protected void configureSelection(CursorLoader loader, long directoryId,
            ContactListFilter filter) {
        if (filter == null || directoryId != Directory.DEFAULT) {
            Log.w(TAG, "[configureSelection]return,filter = " + filter + ",directoryId = "
                    + directoryId);
            return;
        }
        final StringBuilder selection = new StringBuilder();
        final List<String> selectionArgs = new ArrayList<String>();

        // filter sip address as contacts not support such type
        selection.append(Data.MIMETYPE + "!=\"" + SipAddress.CONTENT_ITEM_TYPE + "\"");
        Log.i(TAG, "selection is " + selection);

        Log.i(TAG, "[configureSelection]filter.filterType = " + filter.filterType);
        switch (filter.filterType) {
        case ContactListFilter.FILTER_TYPE_ALL_ACCOUNTS:
        case ContactListFilter.FILTER_TYPE_DEFAULT:
            Log.d(TAG, "[configureSelection] filterType" + filter.filterType);
            break;
        default:
            Log.w(TAG, "[configureSelection]Unsupported filter type came " + "(type: "
                    + filter.filterType + ", toString: " + filter + ")" + " showing all contacts.");
            // No selection.
            break;
        }
        loader.setSelection(selection.toString());
        loader.setSelectionArgs(selectionArgs.toArray(new String[0]));
    }

    private SelectedContactsListener mConferenceCallSelectedListener;

    private final OnClickListener mCheckBoxListener = new OnClickListener() {
        @Override
        public void onClick(View v) {
            final CheckBox checkBox = (CheckBox) v;
            final Long contactId = (Long) checkBox.getTag();

            boolean isSelectedLimited = getSelectedContactIds().size() >= mReferenceCallMaxNumber;
            boolean isChecked = checkBox.isChecked();
            if (isSelectedLimited && isChecked) {
                Log.i(TAG, "[mCheckBoxClickListener] Current selected Contact cnt > 5,cannot " +
                        "select more");
                checkBox.setChecked(false);
                String msg = (mContext.getResources()).getString(
                        R.string.multichoice_contacts_limit, mReferenceCallMaxNumber);
                MtkToast.toast(mContext.getApplicationContext(), msg);
                return;
            }

            TextView dataView = ((ContactListItemView)(((View)checkBox).getParent())).getDataView();
            String data = dataView.getText().toString();
            if (checkBox.isChecked()) {
                getSelectedContactIds().add(contactId);
            } else {
                getSelectedContactIds().remove(contactId);
            }
            if (mConferenceCallSelectedListener != null) {
                mConferenceCallSelectedListener.onSelectedContactsChangedViaCheckBox();
            }
        }
    };

    @Override
    public void setSelectedContactsListener(SelectedContactsListener listener) {
        Log.d(TAG,"[setSelectedContactsListener]");
        mConferenceCallSelectedListener = listener;
    }

    /**
     * Get phone numbers by data ids.
     * @param dataIds select data ids
     * @return phone numbers
     */
    public ArrayList<String> getPhoneNumberByDataIds(long[] dataIds) {
        ArrayList<String> phoneNumbers = new ArrayList<String>();
        StringBuilder selection = new StringBuilder();
        selection.append(Data._ID);
        selection.append(" IN (");
        selection.append(dataIds[0]);
        for (int i = 1; i < dataIds.length; i++) {
            selection.append(",");
            selection.append(dataIds[i]);
        }
        selection.append(")");
        Log.d(TAG, "[getPhoneNumberByDataIds] dataIds " + selection.toString());
        Cursor c = null;
        try {
            c = mContext.getContentResolver().query(Data.CONTENT_URI,
                    new String[]{Data._ID, Data.DATA1},
                    selection.toString(), null, null);
            if (c == null) {
                return phoneNumbers;
            }
            while (c.moveToNext()) {
                Log.sensitive(TAG, "[getPhoneNumberByDataIds] got"
                        + " _ID=" + c.getInt(0)
                        + ", NUMBER=" + c.getString(1));
                phoneNumbers.add(c.getString(1));
            }
        } finally {
            if (c != null) {
                c.close();
            }
        }
        return phoneNumbers;
    }
}
