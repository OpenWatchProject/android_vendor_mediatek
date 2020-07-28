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
import android.content.Intent;
import android.content.Loader;
import android.database.Cursor;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import com.android.contacts.R;
import com.android.contacts.list.ContactListFilter;
import com.android.contacts.util.AccountFilterUtil;

import com.mediatek.contacts.list.DataKindBasePickerAdapter.SelectedContactsListener;
import com.mediatek.contacts.util.Log;

public abstract class DataKindBasePickerFragment extends AbstractPickerFragment
        implements SelectedContactsListener {
    private static final String TAG = "DataKindBasePickerFragment";

    private static final String RESULTINTENTEXTRANAME = "com.mediatek.contacts.list.pickdataresult";

    // Show account filter settings
    private View mAccountFilterHeader;

    @Override
    protected void onCreateView(LayoutInflater inflater, ViewGroup container) {
        super.onCreateView(inflater, container);
        mAccountFilterHeader = getView().findViewById(R.id.account_filter_header_container);
        mAccountFilterHeader.setClickable(false);
        mAccountFilterHeader.setVisibility(View.GONE);
    }

    @Override
    protected void configureAdapter() {
        super.configureAdapter();
        DataKindBasePickerAdapter adapter = (DataKindBasePickerAdapter) getAdapter();
        if (adapter == null) {
            Log.w(TAG, "[configureAdapter]adapter = null.");
            return;
        }

        // Disable pinned header. It doesn't work with this fragment.
        adapter.setPinnedPartitionHeadersEnabled(false);
        adapter.setSelectedContactsListener(
                (DataKindBasePickerAdapter.SelectedContactsListener)this);
    }

    @Override
    public void onOptionAction() {
        final long[] idArray = getCheckedItemIds();
        if (idArray == null) {
            Log.w(TAG, "[onOptionAction]idArray = null.");
            return;
        }

        final Activity activity = getActivity();
        final Intent retIntent = new Intent();
        retIntent.putExtra(RESULTINTENTEXTRANAME, idArray);
        activity.setResult(Activity.RESULT_OK, retIntent);
        activity.finish();
    }

    /*
    @Override
    public void onLoadFinished(Loader<Cursor> loader, Cursor data) {
        final boolean shouldShowHeader = AccountFilterUtil.updateAccountFilterTitleForPeople(
                mAccountFilterHeader,
               ContactListFilter.createFilterWithType(ContactListFilter.FILTER_TYPE_ALL_ACCOUNTS),
                true);
        super.onLoadFinished(loader, data);
    }*/

    @Override
    public long getListItemDataId(int position) {
        final DataKindBasePickerAdapter adapter = (DataKindBasePickerAdapter) getAdapter();
        if (adapter != null) {
            return adapter.getDataId(position);
        }
        return -1;
    }

 //M:Add for multi check box

    @Override
    public void onSelectedContactsChangedViaCheckBox() {
        final int checkCount = getAdapter().getSelectedContactIds().size();
        Log.i(TAG, "[onSelectedContactsChangedViaCheckBox] checkCount : " + checkCount);
        updateSelectedItemsView(checkCount);
    }
}
