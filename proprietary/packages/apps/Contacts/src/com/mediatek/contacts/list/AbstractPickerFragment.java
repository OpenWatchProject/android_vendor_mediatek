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
import android.content.Loader;
import android.content.res.Configuration;
import android.database.Cursor;
import android.graphics.Color;
import android.os.Bundle;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.ListView;
import android.widget.TextView;
import androidx.appcompat.app.AppCompatActivity;

import com.android.contacts.list.ContactEntryListAdapter;
import com.android.contacts.list.ContactListAdapter.ContactQuery;
import com.android.contacts.list.MultiSelectContactsListFragment;
import com.android.contacts.list.MultiSelectEntryContactListAdapter;
import com.android.contacts.R;

import com.mediatek.contacts.GlobalEnv;
import com.mediatek.contacts.util.Log;
import com.mediatek.contacts.util.MtkToast;

import java.util.Arrays;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;
import java.util.TreeSet;

/**
 * The Fragment Base class to handle the basic functions.
 */
public abstract class AbstractPickerFragment extends
        MultiSelectContactsListFragment<MultiSelectEntryContactListAdapter> implements
        ContactListMultiChoiceListener {
    private static final String TAG = "AbstractPickerFragment";

    private static final String KEY_CHECKEDIDS = "checkedids";

    /**
     * The default limit multi choice max count,default is 3500
     */
    public static final int DEFAULT_MULTI_CHOICE_MAX_COUNT = 3500;

    private String mSlectedItemsFormater = null;

    private String mSearchString = null;

    // Show account filter settings
    protected View mAccountFilterHeader = null;

    private View mToolBar = null;

    private TextView mEmptyView = null;

    // is or is not select all items.
    private boolean mIsSelectedAll = false;
    // is or is not select on item.
    private boolean mIsSelectedNone = true;

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
        Log.i(TAG, "[onActivityCreated]");
        mSlectedItemsFormater = getActivity().getString(R.string.menu_actionbar_selected_items);

        restoreSavedState(savedInstanceState);
        showSelectedCount(getAdapter().getSelectedContactIds().size());
        // here should disable the Ok Button,because if call
        // #onActivityCreated()
        // it says it is first Run this Activity or the previous process has
        // been killed
        // the start this Activity again,then it will load data from DB in
        // #onStart()
        setOkButtonStatus(false);

        // Enable multiple choice mode.
        getListView().setChoiceMode(ListView.CHOICE_MODE_MULTIPLE);
    }

    @Override
    protected View inflateView(LayoutInflater inflater, ViewGroup container) {
        Log.i(TAG, "[inflateView]");
        return inflater.inflate(R.layout.mtk_multichoice_contact_list, null);
    }

    @Override
    protected void onCreateView(LayoutInflater inflater, ViewGroup container) {
        super.onCreateView(inflater, container);
        Log.i(TAG, "[onCreateView]");
        mAccountFilterHeader = getView().findViewById(R.id.account_filter_header_container);

        mEmptyView = (TextView) getView().findViewById(R.id.contact_list_empty);
        if (mEmptyView != null) {
            mEmptyView.setText(R.string.noContacts);
        }
    }

    @Override
    protected void configureAdapter() {
        super.configureAdapter();
        Log.i(TAG, "[configureAdapter]...");
        ContactEntryListAdapter adapter = getAdapter();
        if (adapter == null) {
            Log.w(TAG, "[configureAdapter]adapter is null.");
            return;
        }
        adapter.setEmptyListEnabled(true);
        // Show A-Z section index.
        adapter.setSectionHeaderDisplayEnabled(true);
        adapter.setDisplayPhotos(true);
        adapter.setQuickContactEnabled(false);
        super.setPhotoLoaderEnabled(true);
        adapter.setQueryString(mSearchString);
        adapter.setIncludeFavorites(false);

        // Apply MTK theme manager
        /*
        if (mAccountFilterHeader != null) {
            final TextView headerTextView = (TextView) mAccountFilterHeader
                    .findViewById(R.id.account_filter_header);

            if (headerTextView != null) {
                headerTextView.setText(R.string.mtk_contact_list_loading);
                mAccountFilterHeader.setVisibility(View.VISIBLE);
            }
        }*/
    }

   @Override
    protected void onItemClick(int position, long id) {
       /*M: fixed CR ALPS02313143 click item mark checkBox @{*/
       Cursor cursor = (Cursor)(getAdapter().getItem(position));
       long contactId = cursor.getLong(ContactQuery.CONTACT_ID);
       Log.d(TAG, "[onItemClick] position: " + position+ ",contactId:" + contactId);
       TreeSet<Long> SelectedContactIds = getAdapter().getSelectedContactIds();

       /* fix CR ALPS02350421 intercept more than 3500 selected @{*/
       int multiChoiceLimitCount = getMultiChoiceLimitCount();
       boolean isSelectedLimited = getAdapter().getSelectedContactIds().size() >=
               multiChoiceLimitCount;
       boolean isCurrentItemChecked = SelectedContactIds.contains(contactId);
       if (isSelectedLimited && (!isCurrentItemChecked)) {
           Log.i(TAG, "[onItemClick] Current selected Contact cnt > 3500,cannot select more");
           String msg = getResources().getString(R.string.multichoice_contacts_limit,
                   multiChoiceLimitCount);
           MtkToast.toast(getActivity().getApplicationContext(), msg);
           return;
       }
       //@}

       if (SelectedContactIds.contains(contactId)) {
           SelectedContactIds.remove(contactId);
           Log.d(TAG, "[onItemClick]  SelectedContactIds.remove ");
       } else {
           SelectedContactIds.add(contactId);
           Log.d(TAG, "[onItemClick] SelectedContactIds.add ");
       }
       getAdapter().notifyDataSetChanged();
       updateSelectedItemsView();
       //@}
    }

    ///M:[ALPS03482338]onItemClick instead of onItemLongClick @{
    @Override
    protected boolean onItemLongClick(int position, long id) {
        return false;
    }
    /// @}

    @Override
    public void onClearSelect() {
        Log.i(TAG, "[onClearSelect]");
        updateCheckBoxState(false);
    }

    @Override
    public void onSelectAll() {
        Log.i(TAG, "[onSelectAll]");
        updateCheckBoxState(true);
    }

    @Override
    public void restoreSavedState(Bundle savedState) {
        super.restoreSavedState(savedState);

        if (savedState == null) {
            Log.w(TAG, "[restoreSavedState]saved state is null");
            return;
        }

        getAdapter().getSelectedContactIds().clear();

        long[] ids = savedState.getLongArray(KEY_CHECKEDIDS);
        int checkedItemSize = ids.length;
        Log.i(TAG, "[restoreSavedState]restore " + checkedItemSize + " ids");
        for (int index = 0; index < checkedItemSize; ++index) {
            getAdapter().getSelectedContactIds().add(Long.valueOf(ids[index]));
        }
    }

    @Override
    public void onLoadFinished(Loader<Cursor> loader, Cursor data) {
        Log.i(TAG, "[onLoadFinished]");
        /// M:check whether the fragment still in Activity@{
        if (!isAdded()) {
            Log.w(TAG, "[onLoadFinished],This Fragment is not added to the Activity now.data:"
                    + data);
            if (data != null) {
                data.close();
            }
            return;
        }
        /// @}
        if (data != null) {
            int[] ids = data.getExtras().getIntArray("checked_ids");
            if (ids != null) {
                StringBuilder sb = new StringBuilder();
                sb.append("[onLoadFinished] ids: ");
                for (int id : ids) {
                    sb.append(String.valueOf(id) + ",");
                }

                for (Iterator<Long> it = getAdapter().getSelectedContactIds().iterator();
                        it.hasNext(); ) {
                    Long id = it.next();
                    if (Arrays.binarySearch(ids, id.intValue()) < 0) {
                        it.remove();
                    }
                }
            }
        }

        if (getAdapter().isSearchMode()) {
            Log.i(TAG, "[onLoadFinished]SearchMode");
            getListView().setFastScrollEnabled(false);
            getListView().setFastScrollAlwaysVisible(false);
        }

        if (data == null || (data != null && data.getCount() == 0)) {
            Log.w(TAG, "[onLoadFinished]nothing loaded, data:" + data + ",empty view: "
                    + mEmptyView);
            if (mEmptyView != null) {
                if (getAdapter().isSearchMode()) {
                    mEmptyView.setText(R.string.listFoundAllContactsZero);
                } else {
                    mEmptyView.setText(R.string.noContacts);
                }
                mEmptyView.setVisibility(View.VISIBLE);
            }
            // Disable fast scroll bar
            getListView().setFastScrollEnabled(false);
            getListView().setFastScrollAlwaysVisible(false);
        } else {
            if (mEmptyView != null) {
                if (getAdapter().isSearchMode()) {
                    mEmptyView.setText(R.string.listFoundAllContactsZero);
                } else {
                    mEmptyView.setText(R.string.noContacts);
                }
                mEmptyView.setVisibility(View.GONE);
            }
            // Enable fast scroll bar
            if (!getAdapter().isSearchMode()) {
                getListView().setFastScrollEnabled(true);
                getListView().setFastScrollAlwaysVisible(true);
            }
        }

        // clear list view choices
        getListView().clearChoices();

        Set<Long> newDataSet = new HashSet<Long>();

        long dataId = -1;
        int position = 0;

        if (data != null) {
            Log.d(TAG, "[onLoadFinished]query data count: " + data.getCount());
            data.moveToPosition(-1);
            /// M:[ALPS03453965]catch exception for too much records exception.
            try {
                while (data.moveToNext()) {
                    dataId = -1;
                    dataId = data.getInt(0);
                    newDataSet.add(dataId);

                    if (getAdapter().getSelectedContactIds().contains(dataId)
                            || mPushedIds.contains(dataId)) {
                        getListView().setItemChecked(position, true);
                    }
                    ++position;
                    handleCursorItem(data);
                }
            } catch (IllegalStateException e) {
                Log.e(TAG, "[onLoadFinished]IllegalStateException", e);
                if (getActivity() != null) {
                    getActivity().finish();
                }
                return;
            }
        }
        if (!getAdapter().isSearchMode()) {
            for (Iterator<Long> it = getAdapter().getSelectedContactIds().iterator();
                    it.hasNext(); ) {
                Long id = it.next();
                if (!newDataSet.contains(id)) {
                    it.remove();
                }
            }
        }
        // /fix [ALPS00539605] first onLoadComplete won't load newly pushed
        // id data.
        // / so, the pushed ids should be merged in getAdapter().getSelectedContactIds() after first
        // onLoadFinished finished.
        if (!mPushedIds.isEmpty()) {
            getAdapter().getSelectedContactIds().addAll(mPushedIds);
            mPushedIds.clear();
        }
        updateSelectedItemsView(getAdapter().getSelectedContactIds().size());

        // M:[ALPS00578162]
        // the super class will try to restore the list view state if the
        // process been killed
        // in back ground,and the state has been set to the lasted,so clear last
        // state to prevent restore.
        clearListViewLastState();

        // The super function has to be called here.
        super.onLoadFinished(loader, data);
    }

    @Override
    public void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        final int checkedItemsCount = getAdapter().getSelectedContactIds().size();
        long[] checkedIds = new long[checkedItemsCount];
        int index = 0;
        for (Long id : getAdapter().getSelectedContactIds()) {
            checkedIds[index++] = id;
        }
        Log.i(TAG, "[onSaveInstanceState]save " + checkedIds.length + " ids");
        outState.putLongArray(KEY_CHECKEDIDS, checkedIds);
    }

    protected void updateSelectedItemsView(int checkedItemsCount) {
        Log.d(TAG, "[updateSelectedItemsView]checkedItemsCount: " + checkedItemsCount);
        if (checkedItemsCount == 0) {
            mIsSelectedNone = true;
        } else {
            mIsSelectedNone = false;
        }
        setOkButtonStatus(!mIsSelectedNone);

        if (getAdapter().isSearchMode()) {
            Log.w(TAG, "[updateSelectedItemsView]isSearchMonde,don't showSelectedCount:"
                    + checkedItemsCount);
            return;
        }

        showSelectedCount(checkedItemsCount);
    }

    public boolean updateSelectedItemsView() {
        final int count = getListView().getAdapter().getCount();
        final int checkCount = getAdapter().getSelectedContactIds().size();
        Log.i(TAG, "[updateSelectedItemsView]count: " + count + ",checkCount:" + checkCount);
        updateSelectedItemsView(checkCount);
        /** Add consideration of "0" case @{ */
        if (count != 0 && count == getAdapter().getSelectedContactIds().size()
                /*getListView().getCheckedItemCount()*/
                || checkCount >= getMultiChoiceLimitCount()) {
            mIsSelectedAll = true;
        } else {
            mIsSelectedAll = false;
        }
        /** @} */
        return (count >= checkCount) ? true : false;
    }

    public long[] getCheckedItemIds() {
        return convertSetToPrimitive(getAdapter().getSelectedContactIds());
    }

    public void startSearch(String searchString) {
        // It could not meet the layout Request. So, we should not use the
        // default search function.

        // Normalize the empty query.
        if (TextUtils.isEmpty(searchString)) {
            searchString = null;
        }

        ContactEntryListAdapter adapter = (ContactEntryListAdapter) getAdapter();
        if (searchString == null) {
            if (adapter != null) {
                mSearchString = null;
                adapter.setQueryString(searchString);
                adapter.setSearchMode(false);
                reloadData();
            }
        } else if (!TextUtils.equals(mSearchString, searchString)) {
            mSearchString = searchString;
            if (adapter != null) {
                adapter.setQueryString(searchString);
                adapter.setSearchMode(true);
                reloadData();
            }
        }
    }

    public void markItemsAsSelectedForCheckedGroups(long[] ids) {
        // /M: ensure the pushed ids merged into getAdapter().getSelectedContactIds(),
        // / would not exceed mAllowedMaxItems
        TreeSet<Long> tempCheckedIds = new TreeSet<Long>();
        tempCheckedIds.addAll(getAdapter().getSelectedContactIds());
        int multiChoiceLimitCount = getMultiChoiceLimitCount();
        for (long id : ids) {
            if (tempCheckedIds.size() >= multiChoiceLimitCount) {
                String msg = getResources().getString(R.string.multichoice_contacts_limit,
                        multiChoiceLimitCount);
                MtkToast.toast(getActivity().getApplicationContext(), msg);
                Log.w(TAG, "[markItemsAsSelectedForCheckedGroups]size:"
                        + getAdapter().getSelectedContactIds().size() + " >= limit:"
                        + multiChoiceLimitCount);
                return;
            } else {
                tempCheckedIds.add(id);
                mPushedIds.add(id);
            }
        }
    }

    /**
     * @return mIsSelectedAll
     */
    public boolean isSelectedAll() {
        return mIsSelectedAll;
    }

    /**
     * @return mIsSelectedNone
     */
    public boolean isSelectedNone() {
        return mIsSelectedNone;
    }

    public abstract long getListItemDataId(int position);

    public void handleCursorItem(Cursor cursor) {
        return;
    }

    // //////////////////////////private
    // function///////////////////////////////////////////
    /**
     * M: fix [ALPS00539605] It displays three contacts selectd,but displays
     * four contacts in group view.
     */
    private Set<Long> mPushedIds = new HashSet<Long>();

    /**
     *
     * @param checkedItemsCount
     *            The count of items selected to show on the top view.
     */
    private void showSelectedCount(int checkedItemsCount) {
        Log.d(TAG, "[showSelectedCount]checkedItemsCount = " + checkedItemsCount);
        TextView selectedItemsView = null;
        /// M:ALPS02818340.getActivity may be null,just defend it @{
        AppCompatActivity hostActivity = (AppCompatActivity)getActivity();
        if (hostActivity == null) {
            Log.w(TAG, "[showSelectedCount] host Activity is null,return!");
            return;
        }
        View view = hostActivity.getSupportActionBar().getCustomView();
        /// @}
        if (view != null) {
            selectedItemsView = (TextView) view.findViewById(R.id.select_items);
        }
        if (selectedItemsView == null) {
            Log.e(TAG, "[showSelectedCount]Load view resource error!");
            return;
        }
        if (mSlectedItemsFormater == null) {
            Log.e(TAG, "[showSelectedCount]mSlectedItemsFormater is null!");
            return;
        }
        selectedItemsView.setText(String.format(mSlectedItemsFormater,
                String.valueOf(checkedItemsCount)));
    }

    /**
     *
     * @param enable
     *            True to enable the OK button on the Top view,false to diable.
     */
    private void setOkButtonStatus(boolean enable) {
        Button optionView = null;
        /// M:ALPS02818340.getActivity may be null,just defend it @{
        AppCompatActivity hostActivity = (AppCompatActivity)getActivity();
        if (hostActivity == null) {
            Log.w(TAG, "[setOkButtonStatus] host Activity is null,return!");
            return;
        }
        View view = hostActivity.getSupportActionBar().getCustomView();
        /// @}
        if (view != null) {
            optionView = (Button) view.findViewById(R.id.menu_option);
        }
        if (optionView != null) {
            if (enable) {
                optionView.setEnabled(true);
                optionView.setTextColor(getResources().getColor(R.color.background_primary));
            } else {
                optionView.setEnabled(false);
                optionView.setTextColor(Color.LTGRAY);
            }
        }
    }

    /**
     * Long array converters to primitive long array.</br>
     *
     * @param set
     *            a Long array.
     * @return a long array, or null if array is null or empty
     */
    private static long[] convertSetToPrimitive(Set<Long> set) {
        if (set == null) {
            Log.e(TAG, "[convertSetToPrimitive]set is null,return.");
            return null;
        }

        final int arraySize = set.size();
        long[] result = new long[arraySize];

        int index = 0;
        for (Long id : set) {
            if (index >= arraySize) {
                break;
            }
            result[index++] = id.longValue();
        }

        return result;
    }

    ///M-TODO: need merge with parent
    public void updateCheckBoxState(boolean checked) {
        final int count = getListView().getAdapter().getCount();
        long dataId = -1;
        int multiChoiceLimitCount = getMultiChoiceLimitCount();
        Log.d(TAG, "[updateCheckBoxState]count = " + count + ",checked =" + checked);
        for (int position = 0; position < count; ++position) {
            if (checked) {
                if (getAdapter().getSelectedContactIds().size() >= multiChoiceLimitCount) {
                    String msg = getResources().getString(R.string.multichoice_contacts_limit,
                            multiChoiceLimitCount);
                    MtkToast.toast(getActivity().getApplicationContext(), msg);
                    Log.w(TAG, "[updateCheckBoxState] getAdapter().getSelectedContactIds() size:"
                    + getAdapter().getSelectedContactIds().size() + " >= limit:"
                            + multiChoiceLimitCount);
                    getListView().setSelection(multiChoiceLimitCount - 1);
                    break;
                }
                dataId = getListItemDataId(position);
                if (!getAdapter().getSelectedContactIds().contains(dataId)) {
                    getAdapter().getSelectedContactIds().add(Long.valueOf(dataId));
                } else {
                    Log.d(TAG, "[updateCheckBoxState]has selected,unnecessary to add again.");
                }
            } else if (getAdapter().getSelectedContactIds().size() > 0) {
                Log.d(TAG, "[updateCheckBoxState] clean all contacts done!");
                getAdapter().getSelectedContactIds().clear();
                break;
            }
        }
        // M: ALPS02615046. update selected contacts state.
        getAdapter().notifyDataSetChanged();

        updateSelectedItemsView(getAdapter().getSelectedContactIds().size());
    }

    /**
     *
     * @return The max count of current multi choice
     */
    protected int getMultiChoiceLimitCount() {
        return DEFAULT_MULTI_CHOICE_MAX_COUNT;
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        /** M: Bug Fix for ALPS01852524 @{ */
        if (GlobalEnv.isUsingTwoPanes()) {
            int originPadding = getListView().getPaddingLeft();
            int tmp = (int) getResources().getDimension(R.dimen.tmp_panding_value);
            Log.i(TAG, "[onConfigurationChanged] originPadding : " + originPadding
                    + " | tmp : " + tmp);
            if (tmp > 0) {
                int top = getListView().getPaddingTop();
                int bottom = getListView().getPaddingBottom();
                getListView().setPadding(tmp, top, tmp, bottom);
            }
        }
        /** @} */
    }
}
