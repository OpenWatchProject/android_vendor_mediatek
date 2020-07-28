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

import android.content.ContentUris;
import android.content.Context;
import android.content.CursorLoader;
import android.database.Cursor;
import android.net.Uri;
import android.net.Uri.Builder;
import android.provider.ContactsContract;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.ContactsContract.Contacts;
import android.provider.ContactsContract.Data;
import android.provider.ContactsContract.Directory;
import android.provider.ContactsContract.RawContacts;
import android.text.TextUtils;
import android.widget.ListView;

import com.android.contacts.list.ContactListFilter;
import com.android.contacts.list.ContactListItemView;
import com.android.contacts.preference.ContactsPreferences;

import com.google.common.collect.Lists;

import com.mediatek.contacts.ExtensionManager;
import com.mediatek.contacts.util.ContactsPortableUtils;
import com.mediatek.contacts.util.Log;
import com.mediatek.provider.MtkContactsContract;

import java.util.ArrayList;
import java.util.List;

public class PhoneNumbersPickerAdapter extends DataKindBasePickerAdapter {
    private static final String TAG = "PhoneNumbersPickerAdapter";

    protected static final String[] PHONES_PROJECTION_INTERNAL = new String[] { Phone._ID, // 0
            Phone.TYPE, // 1
            Phone.LABEL, // 2
            Phone.NUMBER, // 3
            Phone.DISPLAY_NAME_PRIMARY, // 4
            Phone.DISPLAY_NAME_ALTERNATIVE, // 5
            Phone.CONTACT_ID, // 6
            Phone.LOOKUP_KEY, // 7
            Phone.PHOTO_ID, // 8
            Phone.PHONETIC_NAME, // 9
    };
    protected static final String[] PHONES_PROJECTION;

    static {
        List<String> projectionList = Lists.newArrayList(PHONES_PROJECTION_INTERNAL);

        if (ContactsPortableUtils.MTK_PHONE_BOOK_SUPPORT) {
            /// M: Add some columns for Contacts extensions. @{
            projectionList.add(MtkContactsContract.ContactsColumns.INDICATE_PHONE_SIM); //10
            projectionList.add(MtkContactsContract.ContactsColumns.IS_SDN_CONTACT); //11
            /// @}
        }

        PHONES_PROJECTION = projectionList.toArray(new String[projectionList.size()]);
    }

    protected static final int PHONE_ID_COLUMN_INDEX = 0;
    protected static final int PHONE_TYPE_COLUMN_INDEX = 1;
    protected static final int PHONE_LABEL_COLUMN_INDEX = 2;
    protected static final int PHONE_NUMBER_COLUMN_INDEX = 3;
    protected static final int PHONE_PRIMARY_DISPLAY_NAME_COLUMN_INDEX = 4;
    protected static final int PHONE_ALTERNATIVE_DISPLAY_NAME_COLUMN_INDEX = 5;
    protected static final int PHONE_CONTACT_ID_COLUMN_INDEX = 6;
    protected static final int PHONE_LOOKUP_KEY_COLUMN_INDEX = 7;
    protected static final int PHONE_PHOTO_ID_COLUMN_INDEX = 8;
    protected static final int PHONE_PHONETIC_NAME_COLUMN_INDEX = 9;
    protected static final int PHONE_INDICATE_PHONE_SIM_INDEX = 10;
    protected static final int PHONE_IS_SDN_CONTACT = 11;

    private CharSequence mUnknownNameText;
    private int mDisplayNameColumnIndex;
    private int mAlternativeDisplayNameColumnIndex;
    private Context mContext;

    public PhoneNumbersPickerAdapter(Context context, ListView lv) {
        super(context, lv);
        mContext = context;
        mUnknownNameText = context.getText(android.R.string.unknownName);
        super.displayPhotoOnLeft();
    }

    protected CharSequence getUnknownNameText() {
        return mUnknownNameText;
    }

    @Override
    protected Uri configLoaderUri(long directoryId) {
        Uri uri;
        boolean isSearchMode = isSearchMode();
        Log.i(TAG, "[configLoaderUri]directoryId = " + directoryId + ",isSearchMode = "
                + isSearchMode);
        if (directoryId != Directory.DEFAULT) {
            Log.w(TAG, "[configLoaderUri] PhoneNumberListAdapter is not ready for non-default " +
                    "directory ID (directoryId: " + directoryId + ")");
        }

        if (isSearchMode) {
            String query = getQueryString();
            Builder builder = Phone.CONTENT_FILTER_URI.buildUpon();
            if (TextUtils.isEmpty(query)) {
                builder.appendPath("");
            } else {
                builder.appendPath(query); // Builder will encode the query
            }

            builder.appendQueryParameter(ContactsContract.DIRECTORY_PARAM_KEY,
                    String.valueOf(directoryId));
            builder.appendQueryParameter("checked_ids_arg", Phone.CONTENT_URI.toString());
            uri = builder.build();
        } else {
            uri = Phone.CONTENT_URI
                    .buildUpon()
                    .appendQueryParameter(ContactsContract.DIRECTORY_PARAM_KEY,
                            String.valueOf(Directory.DEFAULT))
                    .appendQueryParameter("checked_ids_arg", Phone.CONTENT_URI.toString()).build();
            if (isSectionHeaderDisplayEnabled()) {
                uri = buildSectionIndexerUri(uri);
            }
            // M:OP01 RCS config list Uri. @{
            uri = ExtensionManager.getInstance().getRcsExtension().configListUri(uri);
            /** @} */
        }

        return uri;
    }

    @Override
    protected String[] configProjection() {
        return PHONES_PROJECTION;
    }

    protected void configureSelection(CursorLoader loader, long directoryId,
            ContactListFilter filter) {
        if (filter == null || directoryId != Directory.DEFAULT) {
            Log.w(TAG, "[configureSelection]directoryId = " + directoryId + ",filter = "
                    + filter);
            return;
        }

        final StringBuilder selection = new StringBuilder();
        final List<String> selectionArgs = new ArrayList<String>();
        Log.i(TAG, "[configureSelection]filterType = " + filter.filterType);
        switch (filter.filterType) {
        case ContactListFilter.FILTER_TYPE_CUSTOM:
            selection.append(Contacts.IN_VISIBLE_GROUP + "=1");
            selection.append(" AND " + Contacts.HAS_PHONE_NUMBER + "=1");
            break;

        case ContactListFilter.FILTER_TYPE_ACCOUNT:
            selection.append("(");

            selection.append(RawContacts.ACCOUNT_TYPE + "=?" + " AND " + RawContacts.ACCOUNT_NAME
                    + "=?");
            selectionArgs.add(filter.accountType);
            selectionArgs.add(filter.accountName);
            if (filter.dataSet != null) {
                selection.append(" AND " + RawContacts.DATA_SET + "=?");
                selectionArgs.add(filter.dataSet);
            } else {
                selection.append(" AND " + RawContacts.DATA_SET + " IS NULL");
            }
            selection.append(")");
            break;

        case ContactListFilter.FILTER_TYPE_ALL_ACCOUNTS:
        case ContactListFilter.FILTER_TYPE_DEFAULT:
            // No selection needed.
            break;

        case ContactListFilter.FILTER_TYPE_WITH_PHONE_NUMBERS_ONLY:
            // This adapter is always "phone only", so no selection needed
            // either.
            break;

        default:
            Log.w(TAG, "Unsupported filter type came " + "(type: " + filter.filterType
                    + ", toString: " + filter + ")" + " showing all contacts.");
            // No selection.
            break;
        }
        // M:Op01 Rcs set list filter @{
        ExtensionManager.getInstance().getRcsExtension().setListFilter(selection, mContext);
        /** @} */
        loader.setSelection(selection.toString());
        loader.setSelectionArgs(selectionArgs.toArray(new String[0]));
    }

    @Override
    public String getContactDisplayName(int position) {
        return ((Cursor) getItem(position)).getString(mDisplayNameColumnIndex);
    }

    @Override
    public void setContactNameDisplayOrder(int displayOrder) {
        super.setContactNameDisplayOrder(displayOrder);
        int nameDisplayOrder = getContactNameDisplayOrder();
        Log.i(TAG, "[setContactNameDisplayOrder]displayOrder = " + displayOrder
                + ",nameDisplayOrder = " + nameDisplayOrder);
        if (nameDisplayOrder == ContactsPreferences.DISPLAY_ORDER_PRIMARY) {
            mDisplayNameColumnIndex = PHONE_PRIMARY_DISPLAY_NAME_COLUMN_INDEX;
            mAlternativeDisplayNameColumnIndex = PHONE_ALTERNATIVE_DISPLAY_NAME_COLUMN_INDEX;
        } else {
            mDisplayNameColumnIndex = PHONE_ALTERNATIVE_DISPLAY_NAME_COLUMN_INDEX;
            mAlternativeDisplayNameColumnIndex = PHONE_PRIMARY_DISPLAY_NAME_COLUMN_INDEX;
        }
    }

    @Override
    public void bindName(ContactListItemView view, Cursor cursor) {
        view.showDisplayName(cursor, mDisplayNameColumnIndex, mAlternativeDisplayNameColumnIndex);
        view.showPhoneticName(cursor, getPhoneticNameColumnIndex());
    }

    @Override
    protected void bindData(ContactListItemView view, Cursor cursor) {
        view.showData(cursor, getDataColumnIndex());
    }

    @Override
    public void bindQuickContact(ContactListItemView view, int partitionIndex, Cursor cursor) {
        return;
    }

    @Override
    public int getDisplayNameColumnIdex() {
        return PHONE_PRIMARY_DISPLAY_NAME_COLUMN_INDEX;
    }

    @Override
    public int getPhotoIDColumnIndex() {
        return PHONE_PHOTO_ID_COLUMN_INDEX;
    }

    @Override
    public int getDataColumnIndex() {
        return PHONE_NUMBER_COLUMN_INDEX;
    }

    @Override
    public int getDataLabelColumnIndex() {
        return PHONE_LABEL_COLUMN_INDEX;
    }

    @Override
    public int getDataTypeColumnIndex() {
        return PHONE_TYPE_COLUMN_INDEX;
    }

    @Override
    public int getContactIDColumnIndex() {
        return PHONE_CONTACT_ID_COLUMN_INDEX;
    }

    @Override
    public int getPhoneticNameColumnIndex() {
        return PHONE_PHONETIC_NAME_COLUMN_INDEX;
    }

    @Override
    public int getIndicatePhoneSIMColumnIndex() {
        return PHONE_INDICATE_PHONE_SIM_INDEX;
    }

    @Override
    public int getIsSdnContactColumnIndex() {
        return PHONE_IS_SDN_CONTACT;
    }

    /**
     * Builds a {@link Data#CONTENT_URI} for the given cursor position.
     *
     * @return Uri for the data. may be null if the cursor is not ready.
     */
    public Uri getDataUri(int position) {
        Cursor cursor = ((Cursor) getItem(position));
        if (cursor != null) {
            long id = cursor.getLong(PHONE_ID_COLUMN_INDEX);
            return ContentUris.withAppendedId(Data.CONTENT_URI, id);
        } else {
            Log.w(TAG, "[getDataUri] Cursor was null.Returning null instead.");
            return null;
        }
    }

    public long getDataId(int position) {
        Cursor cursor = ((Cursor) getItem(position));
        if (cursor != null) {
            return cursor.getLong(PHONE_ID_COLUMN_INDEX);
        } else {
            Log.w(TAG, "[getDataId] Cursor was null in getDataId() call. Returning 0 instead.");
            return 0;
        }
    }

    @Override
    public int getLookupKeyColumnIndex() {
        return PHONE_LOOKUP_KEY_COLUMN_INDEX;
    }
}
