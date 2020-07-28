/*
 * Copyright (C) 2010 The Android Open Source Project
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
 * limitations under the License.
 */
package com.android.contacts.list;

import android.content.ContentUris;
import android.content.Context;
import android.content.CursorLoader;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.net.Uri;
import android.net.Uri.Builder;
import android.preference.PreferenceManager;
import android.provider.ContactsContract;
import android.provider.ContactsContract.Contacts;
import android.provider.ContactsContract.Data;
import android.provider.ContactsContract.Directory;
import android.provider.ContactsContract.RawContacts;
import android.provider.ContactsContract.SearchSnippets;
import android.text.TextUtils;
import android.view.View;
import com.android.contacts.compat.ContactsCompat;
import com.android.contacts.model.account.AccountWithDataSet;
import com.android.contacts.preference.ContactsPreferences;

import com.mediatek.contacts.util.AccountTypeUtils;
import com.mediatek.contacts.util.ContactsCommonListUtils;
import com.mediatek.contacts.util.ContactsPortableUtils;
import com.mediatek.contacts.util.Log;
import com.mediatek.provider.MtkContactsContract;

import java.util.ArrayList;
import java.util.List;

/**
 * A cursor adapter for the {@link ContactsContract.Contacts#CONTENT_TYPE} content type.
 */
public class DefaultContactListAdapter extends ContactListAdapter {
    private static final String TAG = "DefaultContactListAdapter";

    public static final char SNIPPET_START_MATCH = '[';
    public static final char SNIPPET_END_MATCH = ']';


    public DefaultContactListAdapter(Context context) {
        super(context);
    }

    @Override
    public void configureLoader(CursorLoader loader, long directoryId) {
        if (loader instanceof FavoritesAndContactsLoader) {
            /// M: [SDN] @{
            if (ContactsPortableUtils.MTK_PHONE_BOOK_SUPPORT) {
                mSDNLoader = (FavoritesAndContactsLoader)loader;
            }
            /// @}
            ((FavoritesAndContactsLoader) loader).setLoadFavorites(shouldIncludeFavorites());
        }

        String sortOrder = null;
        Log.d(TAG, "[configureLoader] loader:" + loader + ",isSearchMode:" + isSearchMode());
        if (isSearchMode()) {
            String query = getQueryString();
            if (query == null) query = "";
            query = query.trim();
            if (TextUtils.isEmpty(query)) {
                // Regardless of the directory, we don't want anything returned,
                // so let's just send a "nothing" query to the local directory.
                loader.setUri(Contacts.CONTENT_URI);
                loader.setProjection(getProjection(false));
                loader.setSelection("0");
            } else if (isGroupMembersFilter()) {
                final ContactListFilter filter = getFilter();
                configureUri(loader, directoryId, filter);
                // TODO: This is not the normal type to filter URI so we load the non-search
                // projection. Consider creating a specific group member adapter to make it more
                // clear.
                loader.setProjection(getProjection(/* forSearch */ false));
                loader.setSelection(
                        Contacts.DISPLAY_NAME_PRIMARY + " LIKE ?1 OR " +
                        Contacts.DISPLAY_NAME_ALTERNATIVE + " LIKE ?1");
                final String[] args = new String[1];
                args[0] = query + "%";
                loader.setSelectionArgs(args);
            } else {
                final Builder builder = ContactsCompat.getContentUri().buildUpon();
                appendSearchParameters(builder, query, directoryId);
                loader.setUri(builder.build());
                loader.setProjection(getProjection(true));
                sortOrder = Contacts.SORT_KEY_PRIMARY;
            }
        } else {
            final ContactListFilter filter = getFilter();
            configureUri(loader, directoryId, filter);
            if (filter != null
                    && filter.filterType == ContactListFilter.FILTER_TYPE_DEVICE_CONTACTS) {
                loader.setProjection(getDataProjectionForContacts(false));
            } else {
                loader.setProjection(getProjection(false));
            }
            configureSelection(loader, directoryId, filter);
        }

        /**
         * M: [ALPS00112614]. Descriptions: only show phone contact if
         * it's from sms @{
         */
        if (mOnlyShowPhoneContacts) {
            ContactsCommonListUtils.configureOnlyShowPhoneContactsSelection(loader, directoryId,
                    getFilter());
        }
        /** @} */

        if (getSortOrder() == ContactsPreferences.SORT_ORDER_PRIMARY) {
            if (sortOrder == null) {
                sortOrder = Contacts.SORT_KEY_PRIMARY;
            } else {
                sortOrder += ", " + Contacts.SORT_KEY_PRIMARY;
            }
        } else {
            if (sortOrder == null) {
                sortOrder = Contacts.SORT_KEY_ALTERNATIVE;
            } else {
                sortOrder += ", " + Contacts.SORT_KEY_ALTERNATIVE;
            }
        }
        loader.setSortOrder(sortOrder);
    }

    private boolean isGroupMembersFilter() {
        final ContactListFilter filter = getFilter();
        return filter != null && filter.filterType == ContactListFilter.FILTER_TYPE_GROUP_MEMBERS;
    }

    private void appendSearchParameters(Builder builder, String query, long directoryId) {
        builder.appendPath(query); // Builder will encode the query
        builder.appendQueryParameter(ContactsContract.DIRECTORY_PARAM_KEY,
                String.valueOf(directoryId));
        if (directoryId != Directory.DEFAULT && directoryId != Directory.LOCAL_INVISIBLE) {
            builder.appendQueryParameter(ContactsContract.LIMIT_PARAM_KEY,
                    String.valueOf(getDirectoryResultLimit(getDirectoryById(directoryId))));
        }
        builder.appendQueryParameter(SearchSnippets.DEFERRED_SNIPPETING_KEY, "1");
    }

    protected void configureUri(CursorLoader loader, long directoryId, ContactListFilter filter) {
        Uri uri = Contacts.CONTENT_URI;
        if (filter != null) {
            if (filter.filterType == ContactListFilter.FILTER_TYPE_SINGLE_CONTACT) {
                String lookupKey = getSelectedContactLookupKey();
                if (lookupKey != null) {
                    uri = Uri.withAppendedPath(Contacts.CONTENT_LOOKUP_URI, lookupKey);
                } else {
                    uri = ContentUris.withAppendedId(Contacts.CONTENT_URI, getSelectedContactId());
                }
            } else if (filter.filterType == ContactListFilter.FILTER_TYPE_DEVICE_CONTACTS) {
                uri = Data.CONTENT_URI;
            }
        }

        if (directoryId == Directory.DEFAULT && isSectionHeaderDisplayEnabled()) {
            uri = ContactListAdapter.buildSectionIndexerUri(uri);
        }

        if (filter != null
                && filter.filterType != ContactListFilter.FILTER_TYPE_CUSTOM
                && filter.filterType != ContactListFilter.FILTER_TYPE_SINGLE_CONTACT) {
            final Uri.Builder builder = uri.buildUpon();
            /**
             * M: Change Feature: <br>
             * As Local Phone account contains null account and Phone Account,
             * the Account Query Parameter could not meet this requirement. So,
             * We should keep to query contacts with selection. @{
             */
            if (/*filter.filterType == ContactListFilter.FILTER_TYPE_ACCOUNT
                || */filter.filterType == ContactListFilter.FILTER_TYPE_GROUP_MEMBERS) {
                filter.addAccountQueryParameterToUrl(builder);
            }
            /** @} */
            uri = builder.build();
        }

        loader.setUri(uri);
    }

    ///M: [MTK Multi Opr] change from private to protected for mtk multi operation
    protected void configureSelection(
            CursorLoader loader, long directoryId, ContactListFilter filter) {
        if (filter == null) {
            Log.d(TAG, "[configureSelection] filter is null !!");
            return;
        }

        if (directoryId != Directory.DEFAULT) {
            Log.d(TAG, "[configureSelection] directoryId is not DEFAULT: " + directoryId);
            return;
        }

        StringBuilder selection = new StringBuilder();
        List<String> selectionArgs = new ArrayList<String>();

        switch (filter.filterType) {
            case ContactListFilter.FILTER_TYPE_ALL_ACCOUNTS: {
                // We have already added directory=0 to the URI, which takes care of this
                // filter
                /// M: [SDN] @{
                if (ContactsPortableUtils.MTK_PHONE_BOOK_SUPPORT) {
                    selection.append(
                        MtkContactsContract.RawContactsColumns.IS_SDN_CONTACT + " < 1");
                }
                /// @}
                break;
            }
            case ContactListFilter.FILTER_TYPE_SINGLE_CONTACT: {
                // We have already added the lookup key to the URI, which takes care of this
                // filter
                break;
            }
            case ContactListFilter.FILTER_TYPE_STARRED: {
                selection.append(Contacts.STARRED + "!=0");
                break;
            }
            case ContactListFilter.FILTER_TYPE_WITH_PHONE_NUMBERS_ONLY: {
                selection.append(Contacts.HAS_PHONE_NUMBER + "=1");
                /// M: [SDN] @{
                if (ContactsPortableUtils.MTK_PHONE_BOOK_SUPPORT) {
                    selection.append(" AND " +
                        MtkContactsContract.RawContactsColumns.IS_SDN_CONTACT + " < 1");
                }
                /// @}
                break;
            }
            case ContactListFilter.FILTER_TYPE_CUSTOM: {
                selection.append(Contacts.IN_VISIBLE_GROUP + "=1");
                if (isCustomFilterForPhoneNumbersOnly()) {
                    selection.append(" AND " + Contacts.HAS_PHONE_NUMBER + "=1");
                }
                /// M: [SDN] @{
                if (ContactsPortableUtils.MTK_PHONE_BOOK_SUPPORT) {
                    selection.append(" AND " +
                        MtkContactsContract.RawContactsColumns.IS_SDN_CONTACT + " < 1");
                }
                /// @}
                break;
            }
            case ContactListFilter.FILTER_TYPE_ACCOUNT: {
                // We use query parameters for account filter, so no selection to add here.
                /** M: Change Feature: As Local Phone account contains null account and Phone
                 * Account, the Account Query Parameter could not meet this requirement. So,
                 * We should keep to query contacts with selection. @{ */
                buildSelectionForFilterAccount(filter, selection, selectionArgs);
                /* @} */
                break;
            }
            case ContactListFilter.FILTER_TYPE_GROUP_MEMBERS: {
                /// M: [SDN] @{
                if (ContactsPortableUtils.MTK_PHONE_BOOK_SUPPORT) {
                    selection.append(
                        MtkContactsContract.RawContactsColumns.IS_SDN_CONTACT + " < 1");
                }
                /// @}
                break;
            }
            case ContactListFilter.FILTER_TYPE_DEVICE_CONTACTS: {
                if (filter.accountType != null) {
                    selection.append(ContactsContract.RawContacts.ACCOUNT_TYPE)
                            .append("=?");
                    selectionArgs.add(filter.accountType);
                    if (filter.accountName != null) {
                        selection.append(" AND ").append(ContactsContract.RawContacts.ACCOUNT_NAME)
                                .append(("=?"));
                        selectionArgs.add(filter.accountName);
                    }
                    /// M: [SDN] @{
                    if (ContactsPortableUtils.MTK_PHONE_BOOK_SUPPORT) {
                        selection.append(" AND " +
                            MtkContactsContract.RawContactsColumns.IS_SDN_CONTACT + " < 1");
                    }
                    /// @}
                } else {
                    selection.append(AccountWithDataSet.LOCAL_ACCOUNT_SELECTION);
                }
                break;
            }
        }
        Log.d(TAG, "[configureSelection] selection: " + selection.toString()
                + ", filter.filterType: " + filter.filterType);
        loader.setSelection(selection.toString());
        loader.setSelectionArgs(selectionArgs.toArray(new String[0]));
    }

    @Override
    protected void bindView(View itemView, int partition, Cursor cursor, int position) {
        super.bindView(itemView, partition, cursor, position);
        final ContactListItemView view = (ContactListItemView)itemView;

        view.setHighlightedPrefix(isSearchMode() ? getUpperCaseQueryString() : null);

        if (isSelectionVisible()) {
            view.setActivated(isSelectedContact(partition, cursor));
        }

        bindSectionHeaderAndDivider(view, position, cursor);
        /// M: [RCS-e]. @{
        // view.bindDataForCustomView(cursor.getLong(ContactQuery.CONTACT_ID));
        /// @}
        /// M: [Common Presence] @{
        view.bindDataForCommonPresenceView(cursor.getLong(ContactQuery.CONTACT_ID));
        /// @}
        if (isQuickContactEnabled()) {
            bindQuickContact(view, partition, cursor, ContactQuery.CONTACT_PHOTO_ID,
                    ContactQuery.CONTACT_PHOTO_URI, ContactQuery.CONTACT_ID,
                    ContactQuery.CONTACT_LOOKUP_KEY, ContactQuery.CONTACT_DISPLAY_NAME);
        } else {
            if (getDisplayPhotos()) {
                bindPhoto(view, partition, cursor);
            }
        }

        bindNameAndViewId(view, cursor);
        bindPresenceAndStatusMessage(view, cursor);

        if (isSearchMode()) {
            bindSearchSnippet(view, cursor);
        } else {
            view.setSnippet(null);
        }
    }

    private boolean isCustomFilterForPhoneNumbersOnly() {
        // TODO: this flag should not be stored in shared prefs.  It needs to be in the db.
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getContext());
        return prefs.getBoolean(ContactsPreferences.PREF_DISPLAY_ONLY_PHONES,
                ContactsPreferences.PREF_DISPLAY_ONLY_PHONES_DEFAULT);
    }

    /// M: [ALPS00112614] Only show phone contact if it's from sms @{
    private boolean mOnlyShowPhoneContacts = false;
    public void setOnlyShowPhoneContacts(boolean showPhoneContacts) {
        mOnlyShowPhoneContacts = showPhoneContacts;
    }
    /// @}

    /**
     * M: Change Feature: As Local Phone account contains null account and Phone
     * Account, the Account Query Parameter could not meet this requirement. So,
     * We should keep to query contacts with selection. @{*/
    private void buildSelectionForFilterAccount(ContactListFilter filter, StringBuilder selection,
            List<String> selectionArgs) {
        if (AccountTypeUtils.ACCOUNT_TYPE_LOCAL_PHONE.equals(filter.accountType)) {
            selection.append("EXISTS ("
                            + "SELECT DISTINCT " + RawContacts.CONTACT_ID
                            + " FROM view_raw_contacts"
                            + " WHERE ( ");
            if (ContactsPortableUtils.MTK_PHONE_BOOK_SUPPORT) {
                selection.append(
                    MtkContactsContract.RawContactsColumns.IS_SDN_CONTACT + " < 1 AND ");
            }
            selection.append(RawContacts.CONTACT_ID + " = " + "view_contacts."
                            + Contacts._ID
                            + " AND (" + RawContacts.ACCOUNT_TYPE + " IS NULL "
                            + " AND " + RawContacts.ACCOUNT_NAME + " IS NULL "
                            + " AND " +  RawContacts.DATA_SET + " IS NULL "
                            + " OR " + RawContacts.ACCOUNT_TYPE + "=? "
                            + " AND " + RawContacts.ACCOUNT_NAME + "=? ");
        } else {
            selection.append("EXISTS ("
                            + "SELECT DISTINCT " + RawContacts.CONTACT_ID
                            + " FROM view_raw_contacts"
                            + " WHERE ( ");
            if (ContactsPortableUtils.MTK_PHONE_BOOK_SUPPORT) {
                selection.append(
                    MtkContactsContract.RawContactsColumns.IS_SDN_CONTACT + " < 1 AND ");
            }
            selection.append(RawContacts.CONTACT_ID + " = " + "view_contacts."
                            + Contacts._ID
                            + " AND (" + RawContacts.ACCOUNT_TYPE + "=?"
                            + " AND " + RawContacts.ACCOUNT_NAME + "=?");
        }
        ContactsCommonListUtils.buildSelectionForFilterAccount(filter, selection, selectionArgs);
    }
    /// @}
}
