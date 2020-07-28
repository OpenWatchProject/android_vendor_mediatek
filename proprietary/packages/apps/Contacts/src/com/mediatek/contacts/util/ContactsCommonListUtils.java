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
package com.mediatek.contacts.util;

import android.content.Context;
import android.content.CursorLoader;
import android.database.Cursor;
import android.database.MatrixCursor;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.os.UserHandle;
import android.provider.ContactsContract.Contacts;
import android.provider.ContactsContract.Directory;
import android.provider.ContactsContract.RawContacts;
import android.text.TextUtils;
//import android.util.Log;
import android.view.View;
import android.widget.ProgressBar;
import android.widget.TextView;

import com.android.contacts.ContactPhotoManager.DefaultImageRequest;
import com.android.contacts.R;
import com.android.contacts.list.ContactListFilter;
import com.android.contacts.list.FavoritesAndContactsLoader;
import com.android.contacts.model.account.AccountType;
import com.android.contacts.model.account.AccountWithDataSet;
import com.android.contacts.util.AccountFilterUtil;

import com.mediatek.contacts.ContactsSystemProperties;
import com.mediatek.contacts.model.account.AccountWithDataSetEx;
import com.mediatek.contacts.util.ContactsPortableUtils;
import com.mediatek.contacts.util.Log;
import com.mediatek.provider.MtkContactsContract;

import java.util.ArrayList;
import java.util.List;

/** define some util functions for ContactsCommon/list */
public class ContactsCommonListUtils {
    private static final String TAG = "ContactsCommonListUtils";


    /**
     * for SIM name display
     */
    public static void setAccountTypeText(Context context, AccountType accountType,
            TextView accountTypeView, TextView accountUserNameView, ContactListFilter filter) {
        String displayName = null;
        displayName = AccountFilterUtil.getAccountDisplayNameByAccount(filter.accountType,
                filter.accountName);
        if (TextUtils.isEmpty(displayName)) {
            accountTypeView.setText(filter.accountName);
        } else {
            accountTypeView.setText(displayName);
        }
        if (AccountWithDataSetEx.isLocalPhone(accountType.accountType)) {
            accountUserNameView.setVisibility(View.GONE);
            accountTypeView.setText(accountType.getDisplayLabel(context));
        }
    }

    /**
     * For multiuser in 3gdatasms
     */
    public static boolean isAccountTypeSimUsim(AccountType accountType) {
        if (accountType != null && AccountTypeUtils.isAccountTypeIccCard(
                accountType.accountType)) {
            return true;
        }
        return false;
    }

    /**
     * Bug Fix CR ID: ALPS00112614 Descriptions: only show phone contact if it's
     * from sms
     */
    public static void configureOnlyShowPhoneContactsSelection(CursorLoader loader,
            long directoryId, ContactListFilter filter) {
        Log.d(TAG, "[configureOnlyShowPhoneContactsSelection] directoryId :" + directoryId
                + ",filter : " + filter);
        if (filter == null) {
            return;
        }

        if (directoryId != Directory.DEFAULT) {
            return;
        }

        if (ContactsPortableUtils.MTK_PHONE_BOOK_SUPPORT) {
            StringBuilder selection = new StringBuilder();
            List<String> selectionArgs = new ArrayList<String>();
            selection.append(MtkContactsContract.ContactsColumns.INDICATE_PHONE_SIM + "= ?");
            selectionArgs.add("-1");
            loader.setSelection(selection.toString());
            loader.setSelectionArgs(selectionArgs.toArray(new String[0]));
        }
    }

    /**
     * Change Feature: As Local Phone account contains null account and Phone
     * Account, the Account Query Parameter could not meet this requirement. So,
     * We should keep to query contacts with selection.
     */
    public static void buildSelectionForFilterAccount(ContactListFilter filter,
            StringBuilder selection, List<String> selectionArgs) {
        selectionArgs.add(filter.accountType);
        selectionArgs.add(filter.accountName);
        if (filter.dataSet != null) {
            selection.append(" AND " + RawContacts.DATA_SET + "=? )");
            selectionArgs.add(filter.dataSet);
        } else {
            selection.append(" AND " + RawContacts.DATA_SET + " IS NULL )");
        }
        selection.append("))");
    }

    private static Cursor loadSDN(Context context,
            FavoritesAndContactsLoader favoritesAndContactsLoader) {
        Log.d(TAG, "[loadSDN]...");
        Cursor sdnCursor = null;
        if (null != favoritesAndContactsLoader.getSelection()
                && favoritesAndContactsLoader.getSelection().indexOf(
                        MtkContactsContract.RawContactsColumns.IS_SDN_CONTACT + " < 1") >= 0) {
            Uri uri = favoritesAndContactsLoader.getUri();
            String[] projection = favoritesAndContactsLoader.getProjection();
            String newSelection = favoritesAndContactsLoader.getSelection().replace(
                    MtkContactsContract.RawContactsColumns.IS_SDN_CONTACT + " < 1",
                    MtkContactsContract.RawContactsColumns.IS_SDN_CONTACT + " = 1");
            String[] selectionArgs = favoritesAndContactsLoader.getSelectionArgs();
            String sortOrder = favoritesAndContactsLoader.getSortOrder();
            sdnCursor = context.getContentResolver().query(uri, projection, newSelection,
                    selectionArgs, sortOrder);
            if (sdnCursor == null) {
                Log.w(TAG, "[loadSDN]sdnCursor is null need to check");
                return null;
            }
            MatrixCursor matrix = new MatrixCursor(projection);
            try {
                Object[] row = new Object[projection.length];
                while (sdnCursor.moveToNext()) {
                    for (int i = 0; i < row.length; i++) {
                        row[i] = sdnCursor.getString(i);
                    }
                    matrix.addRow(row);
                }
                return matrix;
            } finally {
                if (null != sdnCursor) {
                    sdnCursor.close();
                }
            }
        }
        Log.d(TAG, "[loadSDN] return null");
        return null;
    }

    /**
     * [SDN]
     * */
    public static int addCursorAndSetSelection(Context context,
            FavoritesAndContactsLoader favoritesAndContactsLoader, List<Cursor> cursors,
            int sdnContactCount) {
        String oldSelection = favoritesAndContactsLoader.getSelection();
        Cursor sdnCursor = loadSDN(context, favoritesAndContactsLoader);
        if (sdnCursor != null) {
            sdnContactCount = sdnCursor.getCount();
        }
        if (null != sdnCursor) {
            cursors.add(sdnCursor);
        }
        favoritesAndContactsLoader.setSelection(oldSelection);
        return sdnContactCount;
    }

    /**
     * For SIM contact there must be pass subId and sdnId, in case to draw sub
     * and sdn icons. Cursor cursor The contact cursor. String displayName
     * Contact display name. String lookupKey
     *
     */
    public static DefaultImageRequest getDefaultImageRequest(Cursor cursor, String displayName,
            String lookupKey, boolean circularPhotos) {
        DefaultImageRequest request = new DefaultImageRequest(displayName, lookupKey,
                circularPhotos);
        if (ContactsPortableUtils.MTK_PHONE_BOOK_SUPPORT) {
            final int subId = cursor.getInt(cursor.getColumnIndexOrThrow(
                    MtkContactsContract.ContactsColumns.INDICATE_PHONE_SIM));
            if (subId > 0) {
                request.subId = subId;
                request.photoId = getSdnPhotoId(cursor);
            }
        }
        return request;
    }

    private static final long SIM_PHOTO_ID_SDN_LOCKED = -14;

    private static long getSdnPhotoId(Cursor cursor) {
        long sdnId = 0;
        int isSdnContact = cursor.getInt(cursor.getColumnIndexOrThrow(
                MtkContactsContract.ContactsColumns.IS_SDN_CONTACT));
        if (isSdnContact > 0) {
            sdnId = SIM_PHOTO_ID_SDN_LOCKED;
        }
        return sdnId;
    }

    public static boolean isSdnPhotoId(long id) {
        return id == SIM_PHOTO_ID_SDN_LOCKED ? true : false;
    }
}
