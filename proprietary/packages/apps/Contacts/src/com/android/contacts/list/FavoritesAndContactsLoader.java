/*
 * Copyright (C) 2011 The Android Open Source Project
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

import android.content.Context;
import android.content.CursorLoader;
import android.database.Cursor;
import android.database.MergeCursor;
import android.database.sqlite.SQLiteException;
import android.os.Bundle;
import android.provider.ContactsContract.Contacts;

import com.google.common.collect.Lists;

import com.mediatek.contacts.util.ContactsCommonListUtils;
import com.mediatek.contacts.util.ContactsPortableUtils;
import com.mediatek.contacts.util.Log;

import java.util.List;

/**
 * A loader for use in the default contact list, which will also query for favorite contacts
 * if configured to do so.
 */
public class FavoritesAndContactsLoader extends CursorLoader {

    private static final String TAG = "FavoritesAndContactsLoader";

    private boolean mLoadFavorites;

    private String[] mProjection;


    public FavoritesAndContactsLoader(Context context) {
        super(context);
    }

    /** Whether to load favorites and merge results in before any other results. */
    public void setLoadFavorites(boolean flag) {
        mLoadFavorites = flag;
    }

    public void setProjection(String[] projection) {
        super.setProjection(projection);
        mProjection = projection;
    }

    @Override
    public Cursor loadInBackground() {
        Log.d(TAG, "[loadInBackground]");
        List<Cursor> cursors = Lists.newArrayList();
        if (mLoadFavorites) {
            cursors.add(loadFavoritesContacts());
        }
        /** M: [SDN] @{ */
        mSdnContactCount = 0;
        if (ContactsPortableUtils.MTK_PHONE_BOOK_SUPPORT) {
            mSdnContactCount = ContactsCommonListUtils.addCursorAndSetSelection(getContext(),
                    this, cursors, mSdnContactCount);
        }
        /** @} */
        final Cursor contactsCursor = loadContacts();
        cursors.add(contactsCursor);
        return new MergeCursor(cursors.toArray(new Cursor[cursors.size()])) {
            @Override
            public Bundle getExtras() {
                // Need to get the extras from the contacts cursor.
                return contactsCursor == null ? new Bundle() : contactsCursor.getExtras();
            }
        };
    }

    private Cursor loadContacts() {
        // ContactsCursor.loadInBackground() can return null; MergeCursor
        // correctly handles null cursors.
        try {
            return super.loadInBackground();

        } catch (NullPointerException | SQLiteException | SecurityException e) {
            // Ignore NPEs, SQLiteExceptions and SecurityExceptions thrown by providers
        }
        return null;
    }

    private Cursor loadFavoritesContacts() {
        final StringBuilder selection = new StringBuilder();
        selection.append(Contacts.STARRED + "=?");
        final ContactListFilter filter =
                ContactListFilterController.getInstance(getContext()).getFilter();
        if (filter != null && filter.filterType == ContactListFilter.FILTER_TYPE_CUSTOM) {
            selection.append(" AND ").append(Contacts.IN_VISIBLE_GROUP + "=1");
        }
        Log.d(TAG, "[loadFavoritesContacts] projection: " + selection.toString());
        return getContext().getContentResolver().query(
                Contacts.CONTENT_URI, mProjection, selection.toString(), new String[]{"1"},
                getSortOrder());
    }

    /// M: [SDN]@{
    private int mSdnContactCount = 0;

    public int getSdnContactCount() {
        return this.mSdnContactCount;
    }

    @Override
    protected void onStartLoading() {
        forceLoad();
    }
    /// @}
}
