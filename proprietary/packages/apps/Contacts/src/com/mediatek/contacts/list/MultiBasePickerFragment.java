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
import android.content.Context;
import android.content.CursorLoader;
import android.content.Intent;
import android.content.Loader;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;

import com.android.contacts.R;
import com.android.contacts.list.ContactListAdapter;
import com.android.contacts.list.ContactListFilter;
import com.android.contacts.list.ContactListFilterController;
import com.android.contacts.list.FavoritesAndContactsLoader;
import com.android.contacts.util.AccountFilterUtil;

import com.mediatek.contacts.list.MultiBasePickerAdapter.SelectedContactsListener;
import com.mediatek.contacts.util.Log;

public class MultiBasePickerFragment extends AbstractPickerFragment
        implements SelectedContactsListener {
    private static final String TAG = "MultiBasePickerFragment";

    public static final String FRAGMENT_ARGS = "intent";

    protected static final String RESULT_INTENT_EXTRA_NAME =
            "com.mediatek.contacts.list.pickcontactsresult";

    private static final String KEY_FILTER = "filter";

    private static final int REQUEST_CODE_ACCOUNT_FILTER = 1;

    // Show account filter settings
    private boolean mShowFilterHeader = true;

    private ContactListFilter mFilter;
    private SharedPreferences mPrefs;

    private class FilterHeaderClickListener implements OnClickListener {
        @Override
        public void onClick(View view) {
            AccountFilterUtil.startAccountFilterActivityForResult(
                    MultiBasePickerFragment.this, REQUEST_CODE_ACCOUNT_FILTER, mFilter);
        }
    }

    private OnClickListener mFilterHeaderClickListener = new FilterHeaderClickListener();

    // SDN contacts should be use ProfileAndContactsLoader.
    @Override
    public CursorLoader createCursorLoader(Context context) {
        Log.i(TAG, "[createCursorLoader]");
        return new FavoritesAndContactsLoader(context);
    }

    @Override
    protected void onCreateView(LayoutInflater inflater, ViewGroup container) {
        super.onCreateView(inflater, container);

        if (isAccountFilterEnable()) {
            mAccountFilterHeader.setOnClickListener(mFilterHeaderClickListener);
        } else {
            mAccountFilterHeader.setClickable(false);
        }
        updateFilterHeaderView();
    }

    @Override
    public void onAttach(Activity activity) {
        super.onAttach(activity);
        mPrefs = PreferenceManager.getDefaultSharedPreferences(activity);
        if (isAccountFilterEnable()) {
            restoreFilter();
        }
    }

    private void restoreFilter() {
        mFilter = ContactListFilter.restoreDefaultPreferences(mPrefs);
    }

    @Override
    protected void configureAdapter() {
        Log.i(TAG, "[configureAdapter]");
        super.configureAdapter();
        final MultiBasePickerAdapter adapter = (MultiBasePickerAdapter) getAdapter();
        adapter.setFilter(mFilter);
        adapter.setSelectedContactsListener(
            (com.mediatek.contacts.list.MultiBasePickerAdapter.SelectedContactsListener)this);

    }

    @Override
    protected ContactListAdapter createListAdapter() {
        Log.i(TAG, "[createListAdapter] adapter=" + ContactListFilter
                .createFilterWithType(ContactListFilter.FILTER_TYPE_ALL_ACCOUNTS));
        MultiBasePickerAdapter adapter = new MultiBasePickerAdapter(getActivity(),
                getListView());
        adapter.setFilter(ContactListFilter
                .createFilterWithType(ContactListFilter.FILTER_TYPE_ALL_ACCOUNTS));
        return adapter;
    }

    protected void setListFilter(ContactListFilter filter) {
        if (isAccountFilterEnable()) {
            Log.e(TAG, "[setListFilter]invalid call.");
            throw new RuntimeException(
                    "The #setListFilter could not be called if #isAccountFilterEnable is true");
        }
        Log.i(TAG, "[configureAdapter]setListFilter:" + filter.toString());
        mFilter = filter;
        getAdapter().setFilter(mFilter);
        updateFilterHeaderView();
    }

    /**
     * Check whether or not to show the account filter
     *
     * @return true: the UI would to follow the user selected, false: the fixed
     *         account to passed and could not changed In the case, the the
     *         filter would to get by the function of
     *         {@link MultiBasePickerFragment#setListFilter(ContactListFilter)}
     */
    public boolean isAccountFilterEnable() {
        return true;
    }

    @Override
    public void restoreSavedState(Bundle savedState) {
        super.restoreSavedState(savedState);

        if (savedState == null) {
            Log.w(TAG, "[restoreSavedState]savedState is null,return.");
            return;
        }
        mFilter = savedState.getParcelable(KEY_FILTER);
    }

    @Override
    public void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        outState.putParcelable(KEY_FILTER, mFilter);
    }

    private void setFilter(ContactListFilter filter) {
        if (mFilter == null && filter == null) {
            Log.w(TAG, "[setFilter]mFilter and filter is null,return.");
            return;
        }

        if (mFilter != null && mFilter.equals(filter)) {
            Log.w(TAG, "[setFilter]mFilter equals filter,return.");
            return;
        }
        Log.d(TAG, "[setFilter]" + filter.toString());
        mFilter = filter;
        saveFilter();
        reloadData();
    }

    private void updateFilterHeaderView() {
        if (!mShowFilterHeader) {
            if (mAccountFilterHeader != null) {
                mAccountFilterHeader.setVisibility(View.GONE);
            }
            return;
        }
        if (mAccountFilterHeader == null) {
            Log.w(TAG, "[updateFilterHeaderView]mAccountFilterHeader is null,return.");
            return; // Before onCreateView -- just ignore it.
        }

        //if (mFilter != null && !isSearchMode()) {
        //    final boolean shouldShowHeader = AccountFilterUtil.updateAccountFilterTitleForPeople(
        //            mAccountFilterHeader, mFilter, true);
        //    mAccountFilterHeader.setVisibility(shouldShowHeader ? View.VISIBLE : View.GONE);
        //} else {
            mAccountFilterHeader.setVisibility(View.GONE);
        //}
    }

    private void saveFilter() {
        ContactListFilter.storeToPreferences(mPrefs, mFilter);
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        Log.i(TAG, "[onActivityResult]requestCode = " + requestCode + ",resultCode = "
                + resultCode);
        if (requestCode == REQUEST_CODE_ACCOUNT_FILTER) {
            if (getActivity() != null) {
                AccountFilterUtil.handleAccountFilterResult(
                        ContactListFilterController.getInstance(getActivity()), resultCode, data);
                if (resultCode == Activity.RESULT_OK) {
                    setFilter(ContactListFilterController.getInstance(getActivity()).getFilter());
                    updateFilterHeaderView();
                }
            } else {
                Log.e(TAG, "[onActivityResult]returns null during onActivityResult()");
            }
        }
    }

    public void onOptionAction() {
        final long[] idArray = getCheckedItemIds();
        if (idArray == null) {
            Log.e(TAG, "[onOptionAction]idArray is null,return.");
            return;
        }

        final Activity activity = getActivity();
        final Intent retIntent = new Intent();
        retIntent.putExtra(RESULT_INTENT_EXTRA_NAME, idArray);
        activity.setResult(Activity.RESULT_OK, retIntent);
        activity.finish();
    }

    protected void setDataSetChangedNotifyEnable(boolean enable) {
        MultiBasePickerAdapter adapter = (MultiBasePickerAdapter) getAdapter();
        if (adapter != null) {
            adapter.setDataSetChangedNotifyEnable(enable);
        }
    }

    @Override
    public void onLoadFinished(Loader<Cursor> loader, Cursor data) {
        Log.i(TAG, "[onLoadFinished].");
        updateFilterHeaderView();
        super.onLoadFinished(loader, data);
    }

    public void showFilterHeader(boolean enable) {
        mShowFilterHeader = enable;
    }

    @Override
    public long getListItemDataId(int position) {
        final MultiBasePickerAdapter adapter =
                (MultiBasePickerAdapter) getAdapter();
        if (adapter != null) {
            return adapter.getContactID(position);
        }
        return -1;
    }

    @Override
    public void handleCursorItem(Cursor cursor) {
        final MultiBasePickerAdapter adapter =
                (MultiBasePickerAdapter) getAdapter();
        adapter.cacheDataItem(cursor);
    }

    /// M:Add for multi check box
    @Override
    public void onSelectedContactsChangedViaCheckBox() {
        final int checkCount = getAdapter().getSelectedContactIds().size();
        Log.i(TAG, "[onSelectedContactsChangedViaCheckBox] checkCount : " + checkCount);
        updateSelectedItemsView(checkCount);
    }
}
