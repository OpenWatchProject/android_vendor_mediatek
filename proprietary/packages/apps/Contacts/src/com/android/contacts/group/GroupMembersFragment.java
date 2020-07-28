/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License
 */
package com.android.contacts.group;

import android.app.Activity;
import android.app.LoaderManager.LoaderCallbacks;
import android.content.ContentResolver;
import android.content.Context;
import android.content.CursorLoader;
import android.content.Intent;
import android.content.Loader;
import android.database.Cursor;
import android.database.CursorWrapper;
import android.graphics.PorterDuff;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.provider.ContactsContract;
import android.provider.ContactsContract.Contacts;
import android.text.TextUtils;
//import android.util.Log;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.Toast;
import androidx.core.content.ContextCompat;
import com.android.contacts.ContactSaveService;
import com.android.contacts.ContactSaveService.DeleteEndListener;
import com.android.contacts.ContactsUtils;
import com.android.contacts.GroupMetaDataLoader;
import com.android.contacts.R;
import com.android.contacts.activities.ActionBarAdapter;
import com.android.contacts.activities.PeopleActivity;
import com.android.contacts.group.GroupMembersAdapter.GroupMembersQuery;
import com.android.contacts.interactions.GroupDeletionDialogFragment;
import com.android.contacts.list.ContactsRequest;
import com.android.contacts.list.ContactsSectionIndexer;
import com.android.contacts.list.MultiSelectContactsListFragment;
import com.android.contacts.list.MultiSelectEntryContactListAdapter.DeleteContactListener;
import com.android.contacts.list.UiIntentActions;
import com.android.contacts.logging.ListEvent;
import com.android.contacts.logging.ListEvent.ListType;
import com.android.contacts.logging.Logger;
import com.android.contacts.logging.ScreenEvent;
import com.android.contacts.model.account.AccountWithDataSet;
import com.android.contacts.util.ImplicitIntentsUtil;
import com.android.contactsbind.FeedbackHelper;
import com.google.common.primitives.Longs;

import com.mediatek.contacts.group.SimGroupUtils;
import com.mediatek.contacts.group.UpdateSimGroupMembersAsyncTask;
import com.mediatek.contacts.simcontact.SubInfoUtils;
import com.mediatek.contacts.util.AccountTypeUtils;
import com.mediatek.contacts.util.MtkToast;
import com.mediatek.contacts.util.Log;
import com.mediatek.contacts.util.ProgressHandler;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

/** Displays the members of a group. */
public class GroupMembersFragment extends MultiSelectContactsListFragment<GroupMembersAdapter>
        /* M: [Sim Group][Google Issue]ALPS00463033 @{ */
        implements DeleteEndListener /* @} */ {

    private static final String TAG = "GroupMembers";

    private static final String KEY_IS_EDIT_MODE = "editMode";
    private static final String KEY_GROUP_URI = "groupUri";
    private static final String KEY_GROUP_METADATA = "groupMetadata";
    /// M: [Sim Group][ALPS03477180] 1/3 @{
    private static final String KEY_SUB_ID = "subId";
    /// @}

    public static final String TAG_GROUP_NAME_EDIT_DIALOG = "groupNameEditDialog";

    private static final String ARG_GROUP_URI = "groupUri";

    private static final int LOADER_GROUP_METADATA = 100;
    private static final int MSG_FAIL_TO_LOAD = 1;
    private static final int RESULT_GROUP_ADD_MEMBER = 100;

    /// M: [Sim Group] sub id for sim group @{
    private int mSubId = SubInfoUtils.getInvalidSubId();
    /// @}

    /** Filters out duplicate contacts. */
    private class FilterCursorWrapper extends CursorWrapper {

        private int[] mIndex;
        private int mCount = 0;
        private int mPos = 0;

        public FilterCursorWrapper(Cursor cursor) {
            super(cursor);

            mCount = super.getCount();
            mIndex = new int[mCount];

            final List<Integer> indicesToFilter = new ArrayList<>();

            if (Log.isLoggable(TAG, Log.VERBOSE)) {
                Log.v(TAG, "Group members CursorWrapper start: " + mCount);
            }

            final Bundle bundle = cursor.getExtras();
            final String sections[] = bundle.getStringArray(Contacts
                    .EXTRA_ADDRESS_BOOK_INDEX_TITLES);
            final int counts[] = bundle.getIntArray(Contacts.EXTRA_ADDRESS_BOOK_INDEX_COUNTS);
            final ContactsSectionIndexer indexer = (sections == null || counts == null)
                    ? null : new ContactsSectionIndexer(sections, counts);

            mGroupMemberContactIds.clear();
            for (int i = 0; i < mCount; i++) {
                super.moveToPosition(i);
                final String contactId = getString(GroupMembersQuery.CONTACT_ID);
                if (!mGroupMemberContactIds.contains(contactId)) {
                    mIndex[mPos++] = i;
                    mGroupMemberContactIds.add(contactId);
                } else {
                    indicesToFilter.add(i);
                }
            }

            if (indexer != null && GroupUtil.needTrimming(mCount, counts, indexer.getPositions())) {
                GroupUtil.updateBundle(bundle, indexer, indicesToFilter, sections, counts);
            }

            mCount = mPos;
            mPos = 0;
            super.moveToFirst();

            if (Log.isLoggable(TAG, Log.VERBOSE)) {
                Log.v(TAG, "Group members CursorWrapper end: " + mCount);
            }
        }

        @Override
        public boolean move(int offset) {
            return moveToPosition(mPos + offset);
        }

        @Override
        public boolean moveToNext() {
            return moveToPosition(mPos + 1);
        }

        @Override
        public boolean moveToPrevious() {
            return moveToPosition(mPos - 1);
        }

        @Override
        public boolean moveToFirst() {
            return moveToPosition(0);
        }

        @Override
        public boolean moveToLast() {
            return moveToPosition(mCount - 1);
        }

        @Override
        public boolean moveToPosition(int position) {
            if (position >= mCount) {
                mPos = mCount;
                return false;
            } else if (position < 0) {
                mPos = -1;
                return false;
            }
            mPos = mIndex[position];
            return super.moveToPosition(mPos);
        }

        @Override
        public int getCount() {
            return mCount;
        }

        @Override
        public int getPosition() {
            return mPos;
        }
    }

    private final LoaderCallbacks<Cursor> mGroupMetaDataCallbacks = new LoaderCallbacks<Cursor>() {

        @Override
        public CursorLoader onCreateLoader(int id, Bundle args) {
            return new GroupMetaDataLoader(mActivity, mGroupUri);
        }

        @Override
        public void onLoadFinished(Loader<Cursor> loader, Cursor cursor) {
            if (cursor == null || cursor.isClosed() || !cursor.moveToNext()) {
                Log.e(TAG, "Failed to load group metadata for " + mGroupUri);
                Toast.makeText(getContext(), R.string.groupLoadErrorToast, Toast.LENGTH_SHORT)
                        .show();
                mHandler.sendEmptyMessage(MSG_FAIL_TO_LOAD);
                return;
            }
            mGroupMetaData = new GroupMetaData(getActivity(), cursor);
            /// M: [Sim Group] @{
            mSubId = AccountTypeUtils.getSubIdBySimAccountName(getActivity(),
                    mGroupMetaData.accountName);
            Log.d(TAG, "[onLoadFinished] mSubId is " + mSubId);
            /// @}
            onGroupMetadataLoaded();
        }

        @Override
        public void onLoaderReset(Loader<Cursor> loader) {}
    };

    /**
     * M:[Google issue][ALPS03411014]This is to disable {@link #onOptionsItemSelected}
     * when we trying to stop the activity/fragment. 1/4 @{
     */
    private boolean mDisableOptionItemSelected;
    /** @} */

    private ActionBarAdapter mActionBarAdapter;

    private PeopleActivity mActivity;

    private Uri mGroupUri;

    private boolean mIsEditMode;

    private GroupMetaData mGroupMetaData;

    private Set<String> mGroupMemberContactIds = new HashSet();

    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            if(msg.what == MSG_FAIL_TO_LOAD) {
                /// M:[ALPS03963769] only handle this group message in group view, @{
                /// otherwise PeopleActivity will exit unexpectedly. @{
                if (!mActivity.isGroupView()) {
                    Log.d(TAG, "[handleMessage] not group view, igore msg.");
                    return;
                }
                /// @}
                mActivity.onBackPressed();
            }
        }
    };

    public static GroupMembersFragment newInstance(Uri groupUri) {
        final Bundle args = new Bundle();
        args.putParcelable(ARG_GROUP_URI, groupUri);

        final GroupMembersFragment fragment = new GroupMembersFragment();
        fragment.setArguments(args);
        return fragment;
    }

    public GroupMembersFragment() {
        setPhotoLoaderEnabled(true);
        setSectionHeaderDisplayEnabled(true);
        setHasOptionsMenu(true);
        setListType(ListType.GROUP);
    }

    @Override
    public void onCreateOptionsMenu(Menu menu, MenuInflater inflater) {
        if (mGroupMetaData == null) {
            // Hide menu options until metadata is fully loaded
            Log.e(TAG, "[onCreateOptionsMenu]mGroupMetaData == null !!!");
            return;
        }
        inflater.inflate(R.menu.view_group, menu);
    }

    @Override
    public void onPrepareOptionsMenu(Menu menu) {
        ///M: cache menu for flightmode swith @{
        mOptionMenu = menu;
        /// @}
        final boolean isSelectionMode = mActionBarAdapter.isSelectionMode();
        final boolean isGroupEditable = mGroupMetaData != null && mGroupMetaData.editable;
        final boolean isGroupReadOnly = mGroupMetaData != null && mGroupMetaData.readOnly;

        setVisible(getContext(), menu, R.id.menu_multi_send_email, !mIsEditMode && !isGroupEmpty());
        setVisible(getContext(), menu, R.id.menu_multi_send_message,
                !mIsEditMode && !isGroupEmpty());
        setVisible(getContext(), menu, R.id.menu_add, isGroupEditable && !isSelectionMode);
        setVisible(getContext(), menu, R.id.menu_rename_group,
                !isGroupReadOnly && !isSelectionMode);
        setVisible(getContext(), menu, R.id.menu_delete_group,
                !isGroupReadOnly && !isSelectionMode);
        setVisible(getContext(), menu, R.id.menu_edit_group,
                isGroupEditable && !mIsEditMode && !isSelectionMode && !isGroupEmpty());
        setVisible(getContext(), menu, R.id.menu_remove_from_group,
                isGroupEditable && isSelectionMode && !mIsEditMode);
    }

    private boolean isGroupEmpty() {
        return getAdapter() != null && getAdapter().isEmpty();
    }

    private static void setVisible(Context context, Menu menu, int id, boolean visible) {
        final MenuItem menuItem = menu.findItem(id);
        if (menuItem != null) {
            menuItem.setVisible(visible);
            final Drawable icon = menuItem.getIcon();
            if (icon != null) {
                icon.mutate().setColorFilter(ContextCompat.getColor(context,
                        R.color.actionbar_icon_color), PorterDuff.Mode.SRC_ATOP);
            }
        }
    }

    /**
     * Helper class for cp2 query used to look up all contact's emails and phone numbers.
     */
    public static abstract class Query {
        public static final String EMAIL_SELECTION =
                ContactsContract.Data.MIMETYPE + "='"
                        + ContactsContract.CommonDataKinds.Email.CONTENT_ITEM_TYPE + "'";

        public static final String PHONE_SELECTION =
                ContactsContract.Data.MIMETYPE + "='"
                        + ContactsContract.CommonDataKinds.Phone.CONTENT_ITEM_TYPE + "'";

        public static final String[] EMAIL_PROJECTION = {
                ContactsContract.Data.CONTACT_ID,
                ContactsContract.CommonDataKinds.Email._ID,
                ContactsContract.Data.IS_SUPER_PRIMARY,
                ContactsContract.Data.DATA1
        };

        public static final String[] PHONE_PROJECTION = {
                ContactsContract.Data.CONTACT_ID,
                ContactsContract.CommonDataKinds.Phone._ID,
                ContactsContract.Data.IS_SUPER_PRIMARY,
                ContactsContract.Data.DATA1
        };

        public static final int CONTACT_ID = 0;
        public static final int ITEM_ID = 1;
        public static final int PRIMARY = 2;
        public static final int DATA1 = 3;
    }

    /**
     * Helper class for managing data related to contacts and emails/phone numbers.
     */
    private class ContactDataHelperClass {

        private List<String> items = new ArrayList<>();
        private String firstItemId = null;
        private String primaryItemId = null;

        public void addItem(String item, boolean primaryFlag) {
            if (firstItemId == null) {
                firstItemId = item;
            }
            if (primaryFlag) {
                primaryItemId = item;
            }
            items.add(item);
        }

        public boolean hasDefaultItem() {
            return primaryItemId != null || items.size() == 1;
        }

        public String getDefaultSelectionItemId() {
            return primaryItemId != null
                    ? primaryItemId
                    : firstItemId;
        }
    }

    private void sendToGroup(long[] ids, String sendScheme, String title) {
        if (ids == null || ids.length == 0) return;

        // Get emails or phone numbers
        // contactMap <contact_id, contact_data>
        final Map<String, ContactDataHelperClass> contactMap = new HashMap<>();
        // itemList <item_data>
        final List<String> itemList = new ArrayList<>();
        final String sIds = GroupUtil.convertArrayToString(ids);
        final String select = (ContactsUtils.SCHEME_MAILTO.equals(sendScheme)
                ? Query.EMAIL_SELECTION
                : Query.PHONE_SELECTION)
                + " AND " + ContactsContract.Data.CONTACT_ID + " IN (" + sIds + ")";
        final ContentResolver contentResolver = getContext().getContentResolver();
        final Cursor cursor = contentResolver.query(ContactsContract.Data.CONTENT_URI,
                ContactsUtils.SCHEME_MAILTO.equals(sendScheme)
                        ? Query.EMAIL_PROJECTION
                        : Query.PHONE_PROJECTION,
                select, null, null);

        if (cursor == null) {
            return;
        }

        try {
            cursor.moveToPosition(-1);
            while (cursor.moveToNext()) {
                final String contactId = cursor.getString(Query.CONTACT_ID);
                final String itemId = cursor.getString(Query.ITEM_ID);
                final boolean isPrimary = cursor.getInt(Query.PRIMARY) != 0;
                final String data = cursor.getString(Query.DATA1);

                if (!TextUtils.isEmpty(data)) {
                    final ContactDataHelperClass contact;
                    if (!contactMap.containsKey(contactId)) {
                        contact = new ContactDataHelperClass();
                        contactMap.put(contactId, contact);
                    } else {
                        contact = contactMap.get(contactId);
                    }
                    contact.addItem(itemId, isPrimary);
                    itemList.add(data);
                }
            }
        } finally {
            cursor.close();
        }

        // Start picker if a contact does not have a default
        for (ContactDataHelperClass i : contactMap.values()) {
            if (!i.hasDefaultItem()) {
                // Build list of default selected item ids
                final List<Long> defaultSelection = new ArrayList<>();
                for (ContactDataHelperClass j : contactMap.values()) {
                    final String selectionItemId = j.getDefaultSelectionItemId();
                    if (selectionItemId != null) {
                        defaultSelection.add(Long.parseLong(selectionItemId));
                    }
                }
                final long[] defaultSelectionArray = Longs.toArray(defaultSelection);
                startSendToSelectionPickerActivity(ids, defaultSelectionArray, sendScheme, title);
                return;
            }
        }

        if (itemList.size() == 0 || contactMap.size() < ids.length) {
            Toast.makeText(getContext(), ContactsUtils.SCHEME_MAILTO.equals(sendScheme)
                            ? getString(R.string.groupSomeContactsNoEmailsToast)
                            : getString(R.string.groupSomeContactsNoPhonesToast),
                    Toast.LENGTH_LONG).show();
        }

        if (itemList.size() == 0) {
            return;
        }

        final String itemsString = TextUtils.join(",", itemList);
        GroupUtil.startSendToSelectionActivity(this, itemsString, sendScheme, title);
    }

    private void startSendToSelectionPickerActivity(long[] ids, long[] defaultSelection,
            String sendScheme, String title) {
        /// M: [Sim Group][ALPS04077821] add mSubId. 1/3 @{
        startActivity(GroupUtil.createSendToSelectionPickerIntent(getContext(), ids,
                defaultSelection, sendScheme, title, mSubId));
        /// @}
    }

    private void startGroupAddMemberActivity() {
        ///M: [Sim Group] add subId @{
        startActivityForResult(GroupUtil.createPickMemberIntent(getContext(), mGroupMetaData,
                getMemberContactIds(), mSubId), RESULT_GROUP_ADD_MEMBER);
        /// @}
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        Log.d(TAG, "[onOptionsItemSelected] item = " + item.getTitle() +
                ", mDisableOptionItemSelected = " + mDisableOptionItemSelected);
        /// M:[Google issue][ALPS03411014] 2/4 @{
        if (mDisableOptionItemSelected) {
            return false;
        }
        /// @}
        final int id = item.getItemId();
        /// M:[Google issue][ALPS03476240] check mGroupMetaData not null @{
        if (mGroupMetaData == null) {
            Log.e(TAG, "[onOptionsItemSelected]mGroupMetaData invalid=" + mGroupMetaData);
            if (id == R.id.menu_add ||
                id == R.id.menu_rename_group ||
                id == R.id.menu_remove_from_group) {
                return false;
            }
        }
        /// @}
        if (id == android.R.id.home) {
            mActivity.onBackPressed();
        } else if (id == R.id.menu_add) {
            startGroupAddMemberActivity();
        } else if (id == R.id.menu_multi_send_email) {
            final long[] ids = mActionBarAdapter.isSelectionMode()
                    ? getAdapter().getSelectedContactIdsArray()
                    : GroupUtil.convertStringSetToLongArray(mGroupMemberContactIds);
            sendToGroup(ids, ContactsUtils.SCHEME_MAILTO,
                    getString(R.string.menu_sendEmailOption));
        } else if (id == R.id.menu_multi_send_message) {
            final long[] ids = mActionBarAdapter.isSelectionMode()
                    ? getAdapter().getSelectedContactIdsArray()
                    : GroupUtil.convertStringSetToLongArray(mGroupMemberContactIds);
            sendToGroup(ids, ContactsUtils.SCHEME_SMSTO,
                    getString(R.string.menu_sendMessageOption));
        } else if (id == R.id.menu_rename_group) {
            GroupNameEditDialogFragment.newInstanceForUpdate(
                    new AccountWithDataSet(mGroupMetaData.accountName,
                            mGroupMetaData.accountType, mGroupMetaData.dataSet),
                    GroupUtil.ACTION_UPDATE_GROUP, mGroupMetaData.groupId,
                    mGroupMetaData.groupName).show(getFragmentManager(),
                    TAG_GROUP_NAME_EDIT_DIALOG);
        } else if (id == R.id.menu_delete_group) {
            deleteGroup();
        } else if (id == R.id.menu_edit_group) {
            mIsEditMode = true;
            mActionBarAdapter.setSelectionMode(true);
            displayDeleteButtons(true);
        } else if (id == R.id.menu_remove_from_group) {
            logListEvent();
            removeSelectedContacts();
        } else {
            return super.onOptionsItemSelected(item);
        }
        return true;
    }

    private void removeSelectedContacts() {
        /// M: [Sim Group] [Google Issue]ALPS00542175 @{
        if (!SimGroupUtils.checkServiceState(true, mSubId, getActivity())) {
            return;
        }
        /// @}
        final long[] contactIds = getAdapter().getSelectedContactIdsArray();
        /// M: [Sim Group] @{
        if (mSubId > 0) {
            new UpdateSimGroupMembersAsyncTask(UpdateGroupMembersAsyncTask.TYPE_REMOVE,
                    getContext(), contactIds, mGroupMetaData.groupId, mGroupMetaData.accountName,
                    mGroupMetaData.accountType, mGroupMetaData.dataSet,
                    mGroupMetaData.groupName, mSubId).execute();
        } else {
        /// @}
            new UpdateGroupMembersAsyncTask(UpdateGroupMembersAsyncTask.TYPE_REMOVE,
                    getContext(), contactIds, mGroupMetaData.groupId, mGroupMetaData.accountName,
                    mGroupMetaData.accountType, mGroupMetaData.dataSet).execute();
        }
        mActionBarAdapter.setSelectionMode(false);
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (resultCode != Activity.RESULT_OK || data == null
                || requestCode != RESULT_GROUP_ADD_MEMBER) {
            return;
        }
        /// M: [Sim Group] [Google Issue]ALPS00542175 @{
        if (!SimGroupUtils.checkServiceState(true, mSubId, getActivity())) {
            return;
        }
        /// @}
        long[] contactIds = data.getLongArrayExtra(
                UiIntentActions.TARGET_CONTACT_IDS_EXTRA_KEY);
        if (contactIds == null) {
            final long contactId = data.getLongExtra(
                    UiIntentActions.TARGET_CONTACT_ID_EXTRA_KEY, -1);
            if (contactId > -1) {
                contactIds = new long[1];
                contactIds[0] = contactId;
            }
        }
        /// M: [Sim Group] @{
        if (mSubId > 0) {
            new UpdateSimGroupMembersAsyncTask(
                    UpdateGroupMembersAsyncTask.TYPE_ADD,
                    getContext(), contactIds, mGroupMetaData.groupId, mGroupMetaData.accountName,
                    mGroupMetaData.accountType, mGroupMetaData.dataSet,
                    mGroupMetaData.groupName, mSubId).execute();
        } else {
        /// @}
            new UpdateGroupMembersAsyncTask(
                    UpdateGroupMembersAsyncTask.TYPE_ADD,
                    getContext(), contactIds, mGroupMetaData.groupId, mGroupMetaData.accountName,
                    mGroupMetaData.accountType, mGroupMetaData.dataSet).execute();
        }
    }

    private final ActionBarAdapter.Listener mActionBarListener = new ActionBarAdapter.Listener() {
        @Override
        public void onAction(int action) {
            switch (action) {
                case ActionBarAdapter.Listener.Action.START_SELECTION_MODE:
                    if (mIsEditMode) {
                        displayDeleteButtons(true);
                        mActionBarAdapter.setActionBarTitle(getString(R.string.title_edit_group));
                    } else {
                        displayCheckBoxes(true);
                    }
                    mActivity.invalidateOptionsMenu();
                    break;
                case ActionBarAdapter.Listener.Action.STOP_SEARCH_AND_SELECTION_MODE:
                    mActionBarAdapter.setSearchMode(false);
                    if (mIsEditMode) {
                        ///M: [Google Issue][ALPS03741254] mIsEditMode should be reset here. @{
                        mIsEditMode = false;
                        /// @}
                        displayDeleteButtons(false);
                    } else {
                        displayCheckBoxes(false);
                    }
                    mActivity.invalidateOptionsMenu();
                    break;
                case ActionBarAdapter.Listener.Action.BEGIN_STOPPING_SEARCH_AND_SELECTION_MODE:
                    break;
            }
        }

        @Override
        public void onUpButtonPressed() {
            mActivity.onBackPressed();
        }
    };

    private final OnCheckBoxListActionListener mCheckBoxListener =
            new OnCheckBoxListActionListener() {
                @Override
                public void onStartDisplayingCheckBoxes() {
                    mActionBarAdapter.setSelectionMode(true);
                }

                @Override
                public void onSelectedContactIdsChanged() {
                    if (mActionBarAdapter == null) {
                        return;
                    }
                    if (mIsEditMode) {
                        mActionBarAdapter.setActionBarTitle(getString(R.string.title_edit_group));
                    } else {
                        mActionBarAdapter.setSelectionCount(getSelectedContactIds().size());
                    }
                }

                @Override
                public void onStopDisplayingCheckBoxes() {
                    mActionBarAdapter.setSelectionMode(false);
                }
            };

    private void logListEvent() {
        Logger.logListEvent(
                ListEvent.ActionType.REMOVE_LABEL,
                getListType(),
                getAdapter().getCount(),
                /* clickedIndex */ -1,
                getAdapter().getSelectedContactIdsArray().length);
    }

    private void deleteGroup() {
        /* M:[Sim Group] ALPS03523813: ensure update done before delete-undo,
         * or will cause the members in db and sim card mismatch. 1/2 @{*/
        if (!SimGroupUtils.checkServiceState(true, mSubId, getActivity())) {
            return;
        }
        /* @} */
        if (getMemberCount() == 0) {
            /// M: [Sim Group] @{
            if (mSubId > 0) {
                final Intent intent = SimGroupUtils.createGroupDeletionIntentForIcc(
                        getContext(), mGroupMetaData.groupId,
                        mSubId, mGroupMetaData.groupName);
                getContext().startService(intent);
            } else {
            /// @}
                final Intent intent = ContactSaveService.createGroupDeletionIntent(
                        getContext(), mGroupMetaData.groupId);
                getContext().startService(intent);
            }
            /* M: [Sim group][Google Issue]ALPS00463033
             * can not switch to all contacts until deleting group done.
             * mark AOSP code @{
            mActivity.switchToAllContacts();
             * @} */
        } else {
            GroupDeletionDialogFragment.show(getFragmentManager(), mGroupMetaData.groupId,
                    mGroupMetaData.groupName,
                    /* M: [Sim Group] add sub id @{*/
                    mSubId
                    /* @} */);
        }
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
        mActivity = (PeopleActivity) getActivity();
        /// M:[SelectAll/DeSelectAll] use MTK ActionBarAdapter constructure @{
        mActionBarAdapter = new ActionBarAdapter(mActivity, mActionBarListener,
                mActivity.getSupportActionBar(), mActivity.getToolbar(),
                        R.string.enter_contact_name, this);
        /// @}
        mActionBarAdapter.setShowHomeIcon(true);
        final ContactsRequest contactsRequest = new ContactsRequest();
        contactsRequest.setActionCode(ContactsRequest.ACTION_GROUP);
        mActionBarAdapter.initialize(savedInstanceState, contactsRequest);
        if (mGroupMetaData != null) {
            mActivity.setTitle(mGroupMetaData.groupName);
            if (mGroupMetaData.editable) {
                setCheckBoxListListener(mCheckBoxListener);
            }
        }
    }

    @Override
    public ActionBarAdapter getActionBarAdapter() {
        return mActionBarAdapter;
    }

    public void displayDeleteButtons(boolean displayDeleteButtons) {
        getAdapter().setDisplayDeleteButtons(displayDeleteButtons);
    }

    public ArrayList<String> getMemberContactIds() {
        return new ArrayList<>(mGroupMemberContactIds);
    }

    public int getMemberCount() {
        return mGroupMemberContactIds.size();
    }

    public boolean isEditMode() {
        return mIsEditMode;
    }

    @Override
    public void onCreate(Bundle savedState) {
        super.onCreate(savedState);
        if (savedState == null) {
            mGroupUri = getArguments().getParcelable(ARG_GROUP_URI);
        } else {
            mIsEditMode = savedState.getBoolean(KEY_IS_EDIT_MODE);
            mGroupUri = savedState.getParcelable(KEY_GROUP_URI);
            mGroupMetaData = savedState.getParcelable(KEY_GROUP_METADATA);
            /// M: [Sim Group][ALPS03477180] 3/3 @{
            mSubId = savedState.getInt(KEY_SUB_ID);
            /// @}
        }
        Log.d(TAG, "[onCreate] mGroupUri is " + mGroupUri + ", mSubId is " + mSubId);
        maybeAttachCheckBoxListener();
        /// M: [Sim Group][Google Issue]ALPS00463033 @{
        ContactSaveService.setDeleteEndListener(this);
        /// @}
    }

    @Override
    public void onResume() {
        super.onResume();
        // Re-register the listener, which may have been cleared when onSaveInstanceState was
        // called. See also: onSaveInstanceState
        mActionBarAdapter.setListener(mActionBarListener);
        /// M:[Google issue][ALPS03411014] 3/4 @{
        mDisableOptionItemSelected = false;
        /// @}
    }

    @Override
    protected void startLoading() {
        Log.d(TAG, "[startLoading] mGroupMetaData = " + mGroupMetaData);
        if (mGroupMetaData == null || !mGroupMetaData.isValid()) {
            getLoaderManager().restartLoader(LOADER_GROUP_METADATA, null, mGroupMetaDataCallbacks);
        } else {
            onGroupMetadataLoaded();
        }
    }

    @Override
    public void onLoadFinished(Loader<Cursor> loader, Cursor data) {
        if (data != null) {
            // Wait until contacts are loaded before showing the scrollbar
            setVisibleScrollbarEnabled(true);

            final FilterCursorWrapper cursorWrapper = new FilterCursorWrapper(data);
            bindMembersCount(cursorWrapper.getCount());
            super.onLoadFinished(loader, cursorWrapper);
            // Update state of menu items (e.g. "Remove contacts") based on number of group members.
            mActivity.invalidateOptionsMenu();
            mActionBarAdapter.updateOverflowButtonColor();
            ///M:[ALPS03608123] exit edit mode here @{
            if (!isInactive() && isEditMode() && getGroupCount() == 0) {
                //If we're deleting the last group member, exit edit mode
                exitEditMode();
            }
            ///@}
        }
    }

    private void bindMembersCount(int memberCount) {
        final View accountFilterContainer = getView().findViewById(
                R.id.account_filter_header_container);
        final View emptyGroupView = getView().findViewById(R.id.empty_group);
        if (memberCount > 0) {
            /// M:[Google Issue][ALPS03932023] mGroupMetaData maybe cleared by
            /// updateExistingGroupFragment(), so check mGroupMetaData validation firstly to
            /// avoid crash in bindMembersCount() @{
            if (mGroupMetaData != null) {
                final AccountWithDataSet accountWithDataSet = new AccountWithDataSet(
                        mGroupMetaData.accountName, mGroupMetaData.accountType,
                                mGroupMetaData.dataSet);
                bindListHeader(getContext(), getListView(), accountFilterContainer,
                        accountWithDataSet, memberCount);
            } else {
                Log.e(TAG, "[bindMembersCount] mGroupMetaData is null and ignore !!!");
            }
            /// @}
            emptyGroupView.setVisibility(View.GONE);
        } else {
            hideHeaderAndAddPadding(getContext(), getListView(), accountFilterContainer);
            emptyGroupView.setVisibility(View.VISIBLE);
        }
    }

    @Override
    public void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        Log.d(TAG, "[onSaveInstanceState] mSubId = " +  mSubId);
        if (mActionBarAdapter != null) {
            mActionBarAdapter.setListener(null);
            mActionBarAdapter.onSaveInstanceState(outState);
        }
        /// M:[Google issue][ALPS03411014] 4/4 @{
        mDisableOptionItemSelected = true;
        /// @}
        outState.putBoolean(KEY_IS_EDIT_MODE, mIsEditMode);
        outState.putParcelable(KEY_GROUP_URI, mGroupUri);
        outState.putParcelable(KEY_GROUP_METADATA, mGroupMetaData);
        /// M: [Sim Group][ALPS03477180] 2/3 @{
        outState.putInt(KEY_SUB_ID, mSubId);
        /// @}
    }

    private void onGroupMetadataLoaded() {
        if (Log.isLoggable(TAG, Log.VERBOSE)) Log.v(TAG, "Loaded " + mGroupMetaData);

        maybeAttachCheckBoxListener();

        mActivity.setTitle(mGroupMetaData.groupName);
        mActivity.invalidateOptionsMenu();
        mActivity.updateDrawerGroupMenu(mGroupMetaData.groupId);

        // Start loading the group members
        super.startLoading();
    }

    private void maybeAttachCheckBoxListener() {
        // Don't attach the multi select check box listener if we can't edit the group
        if (mGroupMetaData != null && mGroupMetaData.editable) {
            setCheckBoxListListener(mCheckBoxListener);
        }
    }

    @Override
    protected GroupMembersAdapter createListAdapter() {
        final GroupMembersAdapter adapter = new GroupMembersAdapter(getContext());
        adapter.setSectionHeaderDisplayEnabled(true);
        adapter.setDisplayPhotos(true);
        adapter.setDeleteContactListener(new DeletionListener());
        return adapter;
    }

    @Override
    protected void configureAdapter() {
        super.configureAdapter();
        if (mGroupMetaData != null) {
            /// M:[ALPS03959978] 1/2. @{
            if (getAdapter().getGroupId() != mGroupMetaData.groupId) {
               /* Mark all partitions as STATUS_NOT_LOADED again here due to the partitions status may
                * changed as STATUS_LOADED in other group members' onLoadFinished() which is triggered
                * as database is modified at the same time. So that ContactEntryListFragment can surely
                * call startLoadingDirectoryPartition() in startLoading() to restartLoader (create a
                * new loader) for loading current new mGroupMetaData's group members. */
                getAdapter().onDataReload();
            }
            /// @}
            getAdapter().setGroupId(mGroupMetaData.groupId);
        }
    }

    @Override
    protected View inflateView(LayoutInflater inflater, ViewGroup container) {
        final View view = inflater.inflate(R.layout.contact_list_content, /* root */ null);
        final View emptyGroupView = inflater.inflate(R.layout.empty_group_view, null);

        final ImageView image = (ImageView) emptyGroupView.findViewById(R.id.empty_group_image);
        final LinearLayout.LayoutParams params =
                (LinearLayout.LayoutParams) image.getLayoutParams();
        final int screenHeight = getResources().getDisplayMetrics().heightPixels;
        params.setMargins(0, screenHeight /
                getResources().getInteger(R.integer.empty_group_view_image_margin_divisor), 0, 0);
        params.gravity = Gravity.CENTER_HORIZONTAL;
        image.setLayoutParams(params);

        final FrameLayout contactListLayout = (FrameLayout) view.findViewById(R.id.contact_list);
        contactListLayout.addView(emptyGroupView);

        final Button addContactsButton =
                (Button) emptyGroupView.findViewById(R.id.add_member_button);
        addContactsButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                ///M:[Sim Group] add subId @{
                startActivityForResult(GroupUtil.createPickMemberIntent(getContext(),
                        mGroupMetaData, getMemberContactIds(), mSubId), RESULT_GROUP_ADD_MEMBER);
                /// @}
            }
        });
        return view;
    }

    @Override
    protected void onItemClick(int position, long id) {
        final Uri uri = getAdapter().getContactUri(position);
        if (uri == null) {
            return;
        }
        if (getAdapter().isDisplayingCheckBoxes()) {
            super.onItemClick(position, id);
            return;
        }
        final int count = getAdapter().getCount();
        Logger.logListEvent(ListEvent.ActionType.CLICK, ListEvent.ListType.GROUP, count,
                /* clickedIndex */ position, /* numSelected */ 0);
        ImplicitIntentsUtil.startQuickContact(
                getActivity(), uri, ScreenEvent.ScreenType.LIST_GROUP);
    }

    @Override
    protected boolean onItemLongClick(int position, long id) {
        if (mActivity != null && mIsEditMode) {
            return true;
        }
        return super.onItemLongClick(position, id);
    }

    private final class DeletionListener implements DeleteContactListener {
        @Override
        public void onContactDeleteClicked(int position) {
            /// M: [Sim Group] [Google Issue]ALPS00542175 @{
            if (!SimGroupUtils.checkServiceState(true, mSubId, getActivity())) {
                return;
            }
            /// @}
            /**
             * M:[Google Issue][ALPS03485650] catch null pointer exception for timing issue @[
             * ori: final long contactId = getAdapter().getContactId(position);
             * new: @{ */
            long contactId;
            try {
                contactId = getAdapter().getContactId(position);
            } catch (Exception e) {
                Log.e(TAG, "[onContactDeleteClicked]exception:" + e.toString());
                return;
            }
            /** @} */
            final long[] contactIds = new long[1];
            contactIds[0] = contactId;
            /// M: [Sim Group] @{
            if (mSubId > 0) {
                new UpdateSimGroupMembersAsyncTask(UpdateGroupMembersAsyncTask.TYPE_REMOVE,
                        getContext(), contactIds, mGroupMetaData.groupId,
                        mGroupMetaData.accountName, mGroupMetaData.accountType,
                        mGroupMetaData.dataSet, mGroupMetaData.groupName, mSubId).execute();
            } else {
            /// @}
                new UpdateGroupMembersAsyncTask(UpdateGroupMembersAsyncTask.TYPE_REMOVE,
                        getContext(), contactIds, mGroupMetaData.groupId,
                        mGroupMetaData.accountName, mGroupMetaData.accountType,
                        mGroupMetaData.dataSet).execute();
            }
        }
    }

    public GroupMetaData getGroupMetaData() {
        return mGroupMetaData;
    }

    public boolean isCurrentGroup(long groupId) {
        return mGroupMetaData != null && mGroupMetaData.groupId == groupId;
    }

    /**
     * Return true if the fragment is not yet added, being removed, or detached.
     */
    public boolean isInactive() {
        return !isAdded() || isRemoving() || isDetached();
    }

    @Override
    public void onDestroy() {
        if (mActionBarAdapter != null) {
            mActionBarAdapter.setListener(null);
        }
        /// M: [Sim Group][Google Issue]ALPS00463033 @{
        ContactSaveService.removeDeleteEndListener(this);
        /// @}
        /// M: [Sim Group]ALPS03607814, ensure to dismiss dialog @{
        mProgressHandler.dismissDialog(getFragmentManager());
        /// @}
        super.onDestroy();
    }

    public void updateExistingGroupFragment(Uri newGroupUri, String action) {
        toastForSaveAction(action);

        Log.e(TAG, "[updateExistingGroupFragment] action:" + action +
                ", oldUri:" + mGroupUri.toString() + ", newUri:" + newGroupUri.toString());
        if (isEditMode() && getGroupCount() == 1) {
            // If we're deleting the last group member, exit edit mode
            /// M:[ALPS03608123] don't update here, onLoadFinished() maybe called earlier,
            /// in that case the group count is 0 and editor mode will show wrong.@{
            //exitEditMode();
            /// @}
        } else if (!GroupUtil.ACTION_REMOVE_FROM_GROUP.equals(action)) {
            mGroupUri = newGroupUri;
            mGroupMetaData = null; // Clear mGroupMetaData to trigger a new load.
            reloadData();
            mActivity.invalidateOptionsMenu();
        }
    }

    public void toastForSaveAction(String action) {
        int id = -1;
        switch(action) {
            case GroupUtil.ACTION_UPDATE_GROUP:
                id = R.string.groupUpdatedToast;
                break;
            case GroupUtil.ACTION_REMOVE_FROM_GROUP:
                id = R.string.groupMembersRemovedToast;
                break;
            case GroupUtil.ACTION_CREATE_GROUP:
                id = R.string.groupCreatedToast;
                break;
            case GroupUtil.ACTION_ADD_TO_GROUP:
                id = R.string.groupMembersAddedToast;
                break;
            case GroupUtil.ACTION_SWITCH_GROUP:
                // No toast associated with this action.
                break;
            default:
                FeedbackHelper.sendFeedback(getContext(), TAG,
                        "toastForSaveAction passed unknown action: " + action,
                        new IllegalArgumentException("Unhandled contact save action " + action));
        }
        toast(id);
    }

    private void toast(int resId) {
        ///M:[Google Issue]While toasting ,the activity maybe already leave and context is null @{
        if (resId == R.string.groupMembersRemovedToast && getContext() != null) {
            ///M:[Google Issue][ALPS03587932] toast too many and lead FD exhaused. @{
            MtkToast.toast(getContext(), resId);
            /// @}
        } else if (resId >= 0 && getContext() != null) {
            Toast.makeText(getContext(), resId, Toast.LENGTH_SHORT).show();
        } else if (resId >= 0){
            Log.e(TAG, "[toast] igore. resId=" + resId + ", getContext()=" + getContext());
        }
        /// @}
    }

    private int getGroupCount() {
        return getAdapter() != null ? getAdapter().getCount() : -1;
    }

    public void exitEditMode() {
        mIsEditMode = false;
        mActionBarAdapter.setSelectionMode(false);
        displayDeleteButtons(false);
    }

    /* M: [Sim Group][Google Issue]ALPS00463033 @{ */
    private ProgressHandler mProgressHandler = new ProgressHandler();

    @Override
    public void onDeleteEnd() {
        mProgressHandler.dismissDialog(getFragmentManager());
        //delete group done, then safely switch to AllContacts on UI thread
        new Handler(Looper.getMainLooper()).post(new Runnable() {
            @Override
            public void run() {
                mActivity.switchToAllContacts();
            }
        });
    }

    @Override
    public void onDeleteStart() {
        mProgressHandler.showDialog(getFragmentManager());
    }
    /* @} */
}
