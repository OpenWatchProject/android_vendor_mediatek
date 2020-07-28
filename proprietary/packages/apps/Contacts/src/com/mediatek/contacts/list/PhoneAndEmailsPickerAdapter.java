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

import java.util.List;

import android.content.ContentUris;
import android.content.Context;
import android.content.CursorLoader;
import android.database.Cursor;
import android.net.Uri;
import android.net.Uri.Builder;
import android.provider.ContactsContract;
import android.provider.ContactsContract.CommonDataKinds.Email;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.ContactsContract.Contacts;
import android.provider.ContactsContract.Data;
import android.provider.ContactsContract.Directory;
import android.provider.ContactsContract.SearchSnippets;
import android.text.TextUtils;
import android.widget.ListView;
import android.widget.TextView;

import com.android.contacts.list.ContactListFilter;
import com.android.contacts.list.ContactListItemView;
import com.android.contacts.preference.ContactsPreferences;

import com.google.common.collect.Lists;

import com.mediatek.contacts.GlobalEnv;
import com.mediatek.contacts.util.ContactsPortableUtils;
import com.mediatek.contacts.util.Log;
import com.mediatek.provider.MtkContactsContract;

public class PhoneAndEmailsPickerAdapter extends DataKindBasePickerAdapter {
    private static final String TAG = "PhoneAndEmailsPickerAdapter";

    public static final char SNIPPET_START_MATCH = '\u0001';
    public static final char SNIPPET_END_MATCH = '\u0001';
    public static final String SNIPPET_ELLIPSIS = "\u2026";
    public static final int SNIPPET_MAX_TOKENS = 5;

    public static final String SNIPPET_ARGS = SNIPPET_START_MATCH + "," + SNIPPET_END_MATCH + ","
            + SNIPPET_ELLIPSIS + "," + SNIPPET_MAX_TOKENS;

    public static final Uri PICK_PHONE_EMAIL_URI = Uri
            .parse("content://com.android.contacts/data/phone_email");

    public static final Uri PICK_PHONE_EMAIL_FILTER_URI = Uri.withAppendedPath(
            PICK_PHONE_EMAIL_URI, "filter");

    static final String[] PHONE_EMAIL_PROJECTION_INTERNAL = new String[] { Phone._ID, // 0
            Phone.TYPE, // 1
            Phone.LABEL, // 2
            Phone.NUMBER, // 3
            Phone.DISPLAY_NAME_PRIMARY, // 4
            Phone.DISPLAY_NAME_ALTERNATIVE, // 5
            Phone.CONTACT_ID, // 6
            Phone.LOOKUP_KEY, // 7
            Phone.PHOTO_ID, // 8
            Phone.PHONETIC_NAME, // 9
            Phone.MIMETYPE, // 10
    };
    static final String[] PHONE_EMAIL_PROJECTION;

    static {
        List<String> projectionList = Lists.newArrayList(PHONE_EMAIL_PROJECTION_INTERNAL);

        if (ContactsPortableUtils.MTK_PHONE_BOOK_SUPPORT) {
            /// M: Add some columns for Contacts extensions. @{
            projectionList.add(MtkContactsContract.ContactsColumns.INDICATE_PHONE_SIM); //11
            projectionList.add(MtkContactsContract.ContactsColumns.IS_SDN_CONTACT); //12
            /// @}
        }

        PHONE_EMAIL_PROJECTION = projectionList.toArray(new String[projectionList.size()]);
    }

    protected static final int PHONE_EMAIL_ID_INDEX = 0;
    protected static final int PHONE_EMAIL_TYPE_INDEX = 1;
    protected static final int PHONE_EMAIL_LABEL_INDEX = 2;
    protected static final int PHONE_EMAIL_NUMBER_INDEX = 3;
    protected static final int PHONE_EMAIL_PRIMARY_DISPLAY_NAME_INDEX = 4;
    protected static final int PHONE_EMAIL_ALTERNATIVE_DISPLAY_NAME_INDEX = 5;
    protected static final int PHONE_EMAIL_CONTACT_ID_INDEX = 6;
    protected static final int PHONE_EMAIL_LOOKUPKEY_ID_INDEX = 7;
    protected static final int PHONE_EMAIL_PHOTO_ID_INDEX = 8;
    protected static final int PHONE_EMAIL_PHONETIC_NAME_INDEX = 9;
    protected static final int PHONE_EMAIL_MIMETYPE_INDEX = 10;
    protected static final int PHONE_EMAIL_INDICATE_PHONE_SIM_INDEX = 11;
    protected static final int PHONE_EMAIL_IS_SDN_CONTACT = 12;

    private CharSequence mUnknownNameText;
    private int mDisplayNameColumnIndex;
    private int mAlternativeDisplayNameColumnIndex;

    private Context mContext;

    public PhoneAndEmailsPickerAdapter(Context context, ListView lv) {
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
        final Builder builder;
        Uri uri = null;
        boolean isSearchMode = isSearchMode();
        Log.i(TAG, "[configLoaderUri]directoryId = " + directoryId + ",isSearchMode = "
                + isSearchMode);

        if (isSearchMode) {
            String query = getQueryString();
            if (query == null) {
                query = "";
            }
            query = query.trim();
            if (TextUtils.isEmpty(query)) {
                // Regardless of the directory, we don't want anything returned,
                // so let's just send a "nothing" query to the local directory.
                builder = PICK_PHONE_EMAIL_URI.buildUpon();
            } else {
                builder = PICK_PHONE_EMAIL_FILTER_URI.buildUpon();
                builder.appendPath(query); // Builder will encode the query
                builder.appendQueryParameter(ContactsContract.DIRECTORY_PARAM_KEY,
                        String.valueOf(directoryId));
                if (directoryId != Directory.DEFAULT && directoryId != Directory.LOCAL_INVISIBLE) {
                    builder.appendQueryParameter(ContactsContract.LIMIT_PARAM_KEY,
                            String.valueOf(getDirectoryResultLimit()));
                }
                builder.appendQueryParameter(SearchSnippets.SNIPPET_ARGS_PARAM_KEY, SNIPPET_ARGS);
                builder.appendQueryParameter(SearchSnippets.DEFERRED_SNIPPETING_KEY, "1");
            }
        } else {
            builder = PICK_PHONE_EMAIL_URI.buildUpon();

            builder.appendQueryParameter(ContactsContract.DIRECTORY_PARAM_KEY,
                    String.valueOf(directoryId));
        }
        builder.appendQueryParameter("checked_ids_arg", PICK_PHONE_EMAIL_URI.toString());

        uri = builder.build();
        if (isSectionHeaderDisplayEnabled()) {
            uri = buildSectionIndexerUri(uri);
        }
        return uri;
    }

    @Override
    protected String[] configProjection() {
        return PHONE_EMAIL_PROJECTION;
    }

    @Override
    protected void configureSelection(CursorLoader loader, long directoryId,
            ContactListFilter filter) {
        return;
    }

    @Override
    public String getContactDisplayName(int position) {
        return ((Cursor) getItem(position)).getString(mDisplayNameColumnIndex);
    }

    @Override
    public void setContactNameDisplayOrder(int displayOrder) {
        super.setContactNameDisplayOrder(displayOrder);
        if (getContactNameDisplayOrder() == ContactsPreferences.DISPLAY_ORDER_PRIMARY) {
            mDisplayNameColumnIndex = PHONE_EMAIL_PRIMARY_DISPLAY_NAME_INDEX;
            mAlternativeDisplayNameColumnIndex = PHONE_EMAIL_ALTERNATIVE_DISPLAY_NAME_INDEX;
        } else {
            mDisplayNameColumnIndex = PHONE_EMAIL_ALTERNATIVE_DISPLAY_NAME_INDEX;
            mAlternativeDisplayNameColumnIndex = PHONE_EMAIL_PRIMARY_DISPLAY_NAME_INDEX;
        }
        Log.i(TAG, "[setContactNameDisplayOrder]displayOrder = " + displayOrder +
                ",mDisplayNameColumnIndex = " + mDisplayNameColumnIndex);
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
    public int getContactIDColumnIndex() {
        return PHONE_EMAIL_CONTACT_ID_INDEX;
    }

    @Override
    public int getDataColumnIndex() {
        return PHONE_EMAIL_NUMBER_INDEX;
    }

    @Override
    public int getDataLabelColumnIndex() {
        return PHONE_EMAIL_LABEL_INDEX;
    }

    @Override
    public int getDataTypeColumnIndex() {
        return PHONE_EMAIL_TYPE_INDEX;
    }

    @Override
    public Uri getDataUri(int position) {
        long id = ((Cursor) getItem(position)).getLong(PHONE_EMAIL_ID_INDEX);
        return ContentUris.withAppendedId(Data.CONTENT_URI, id);
    }

    @Override
    public long getDataId(int position) {
        /** M: Bug Fix for ALPS01537241 @{ */
        if (null != (Cursor) getItem(position)) {
            return ((Cursor) getItem(position)).getLong(PHONE_EMAIL_ID_INDEX);
        } else {
            Log.w(TAG, "[getDataId] The getItem is null");
            return -1;
        }
        /** @} */
    }

    @Override
    public int getDisplayNameColumnIdex() {
        return PHONE_EMAIL_PRIMARY_DISPLAY_NAME_INDEX;
    }

    @Override
    public int getPhoneticNameColumnIndex() {
        return PHONE_EMAIL_PHONETIC_NAME_INDEX;
    }

    @Override
    public int getPhotoIDColumnIndex() {
        return PHONE_EMAIL_PHOTO_ID_INDEX;
    }

    @Override
    public int getIndicatePhoneSIMColumnIndex() {
        return PHONE_EMAIL_INDICATE_PHONE_SIM_INDEX;
    }

    @Override
    public int getIsSdnContactColumnIndex() {
        return PHONE_EMAIL_IS_SDN_CONTACT;
    }

    @Override
    public int getLookupKeyColumnIndex() {
        return PHONE_EMAIL_LOOKUPKEY_ID_INDEX;
    }
}
