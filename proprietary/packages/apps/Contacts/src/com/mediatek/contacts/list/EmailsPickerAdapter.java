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
import android.provider.ContactsContract.CommonDataKinds.Email;
import android.provider.ContactsContract.Contacts;
import android.provider.ContactsContract.Data;
import android.text.TextUtils;
import android.widget.ListView;
import android.widget.TextView;

import com.android.contacts.list.ContactListFilter;
import com.android.contacts.list.ContactListItemView;
import com.android.contacts.preference.ContactsPreferences;

import com.google.common.collect.Lists;

import com.mediatek.contacts.util.ContactsPortableUtils;
import com.mediatek.contacts.util.Log;
import com.mediatek.provider.MtkContactsContract;

import java.util.List;

public class EmailsPickerAdapter extends DataKindBasePickerAdapter {
    private static final String TAG = "EmailsPickerAdapter";

    static final String[] EMAILS_PROJECTION_INTERNAL = new String[] {
            Email._ID, // 0
            Email.TYPE, // 1
            Email.LABEL, // 2
            Email.DATA, // 3
            Email.DISPLAY_NAME_PRIMARY, // 4
            Email.DISPLAY_NAME_ALTERNATIVE, // 5
            Email.CONTACT_ID, // 6
            Email.LOOKUP_KEY, //7
            Email.PHOTO_ID, // 8
            Email.PHONETIC_NAME, // 9
    };
    static final String[] EMAILS_PROJECTION;

    static {
        List<String> projectionList = Lists.newArrayList(EMAILS_PROJECTION_INTERNAL);

        if (ContactsPortableUtils.MTK_PHONE_BOOK_SUPPORT) {
            /// M: Add some columns for Contacts extensions. @{
            projectionList.add(MtkContactsContract.ContactsColumns.INDICATE_PHONE_SIM);   //10
            projectionList.add(MtkContactsContract.ContactsColumns.IS_SDN_CONTACT); //11
            /// @}
        }

        EMAILS_PROJECTION = projectionList.toArray(new String[projectionList.size()]);
    }

    protected static final int EMAIL_ID_COLUMN_INDEX = 0;
    protected static final int EMAIL_TYPE_COLUMN_INDEX = 1;
    protected static final int EMAIL_LABEL_COLUMN_INDEX = 2;
    protected static final int EMAIL_ADDRESS_COLUMN_INDEX = 3;
    protected static final int EMAIL_PRIMARY_DISPLAY_NAME_COLUMN_INDEX = 4;
    protected static final int EMAIL_ALTERNATIVE_DISPLAY_NAME_COLUMN_INDEX = 5;
    protected static final int EMAIL_CONTACT_ID_COLUMN_INDEX = 6;
    protected static final int EMAIL_LOOKUPKEY_COLUMN_INDEX = 7;
    protected static final int EMAIL_PHOTO_ID_COLUMN_INDEX = 8;
    protected static final int EMAIL_PHONETIC_NAME_COLUMN_INDEX = 9;
    protected static final int EMAIL_INDICATE_PHONE_SIM_INDEX = 10;
    protected static final int EMAIL_IS_SDN_CONTACT = 11;


    private CharSequence mUnknownNameText;
    private int mDisplayNameColumnIndex;
    private int mAlternativeDisplayNameColumnIndex;

    public EmailsPickerAdapter(Context context, ListView lv) {
        super(context, lv);
        mUnknownNameText = context.getText(android.R.string.unknownName);
        super.displayPhotoOnLeft();
    }

    protected CharSequence getUnknownNameText() {
        return mUnknownNameText;
    }

    @Override
    public Uri configLoaderUri(long directoryId) {
        final Builder builder;
        Uri uri = null;
        boolean isSearchMode = isSearchMode();
        Log.i(TAG, "[configLoaderUri]directoryId = " + directoryId + ",isSearchMode = "
                + isSearchMode);
        if (isSearchMode) {
            builder = Email.CONTENT_FILTER_URI.buildUpon();
            String query = getQueryString();
            builder.appendPath(TextUtils.isEmpty(query) ? "" : query);
        } else {
            builder = Email.CONTENT_URI.buildUpon();
        }
        builder.appendQueryParameter(ContactsContract.DIRECTORY_PARAM_KEY, String
                .valueOf(directoryId));
        builder.appendQueryParameter("checked_ids_arg", Email.CONTENT_URI.toString());
        uri = builder.build();
        if (isSectionHeaderDisplayEnabled()) {
            uri = buildSectionIndexerUri(uri);
        }

        return uri;
    }

    @Override
    public String[] configProjection() {
        return EMAILS_PROJECTION;
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
            mDisplayNameColumnIndex = EMAIL_PRIMARY_DISPLAY_NAME_COLUMN_INDEX;
            mAlternativeDisplayNameColumnIndex = EMAIL_ALTERNATIVE_DISPLAY_NAME_COLUMN_INDEX;
        } else {
            mDisplayNameColumnIndex = EMAIL_ALTERNATIVE_DISPLAY_NAME_COLUMN_INDEX;
            mAlternativeDisplayNameColumnIndex = EMAIL_PRIMARY_DISPLAY_NAME_COLUMN_INDEX;
        }
    }

    @Override
    public void bindName(ContactListItemView view, Cursor cursor) {
        Log.d(TAG,"[bindName]");
        view.showDisplayName(cursor, mDisplayNameColumnIndex, mAlternativeDisplayNameColumnIndex);
        view.showPhoneticName(cursor, getPhoneticNameColumnIndex());
    }

    @Override
    protected void bindData(ContactListItemView view, Cursor cursor) {
        //M:fixed CR ALPS02327619.use emptyLabel cover label position.
        TextView emptyLabel = view.getLabelView();
        emptyLabel.setText("");

        view.showData(cursor, getDataColumnIndex());
    }

    @Override
    public void bindQuickContact(ContactListItemView view, int partitionIndex, Cursor cursor) {
        return;
    }

    @Override
    public int getContactIDColumnIndex() {
        return EMAIL_CONTACT_ID_COLUMN_INDEX;
    }

    @Override
    public int getDataColumnIndex() {
        return EMAIL_ADDRESS_COLUMN_INDEX;
    }

    @Override
    public int getDataLabelColumnIndex() {
        return EMAIL_LABEL_COLUMN_INDEX;
    }

    @Override
    public int getDataTypeColumnIndex() {
        return EMAIL_TYPE_COLUMN_INDEX;
    }

    /**
     * Builds a {@link Data#CONTENT_URI} for the current cursor position.
     */
    @Override
    public Uri getDataUri(int position) {
        long id = ((Cursor) getItem(position)).getLong(EMAIL_ID_COLUMN_INDEX);
        return ContentUris.withAppendedId(Data.CONTENT_URI, id);
    }

    @Override
    public long getDataId(int position) {
        return ((Cursor) getItem(position)).getLong(EMAIL_ID_COLUMN_INDEX);
    }

    @Override
    public int getDisplayNameColumnIdex() {
        return EMAIL_PRIMARY_DISPLAY_NAME_COLUMN_INDEX;
    }

    @Override
    public int getPhotoIDColumnIndex() {
        return EMAIL_PHOTO_ID_COLUMN_INDEX;
    }

    @Override
    public int getPhoneticNameColumnIndex() {
        return EMAIL_PHONETIC_NAME_COLUMN_INDEX;
    }

    @Override
    public int getIndicatePhoneSIMColumnIndex() {
        return EMAIL_INDICATE_PHONE_SIM_INDEX;
    }

    @Override
    public int getIsSdnContactColumnIndex() {
        return EMAIL_IS_SDN_CONTACT;
    }

    @Override
    public int getLookupKeyColumnIndex() {
        return EMAIL_LOOKUPKEY_COLUMN_INDEX;
    }
}
