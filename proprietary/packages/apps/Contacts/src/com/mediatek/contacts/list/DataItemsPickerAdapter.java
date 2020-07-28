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

import android.content.Context;
import android.content.CursorLoader;
import android.net.Uri;
import android.net.Uri.Builder;
import android.provider.ContactsContract;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.ContactsContract.Directory;
import android.text.TextUtils;
import android.widget.ListView;

import com.android.contacts.list.ContactListFilter;

import com.mediatek.contacts.util.Log;

public class DataItemsPickerAdapter extends PhoneNumbersPickerAdapter {
    private static final String TAG = "DataItemsPickerAdapter";

    private long[] mRestrictPhoneIds;

    private String mMimeType;

    public static final Uri DATA_OTHERS_URI = Uri
            .parse("content://com.android.contacts/data/others");

    public static final Uri DATA_OTHERS_FILTER_URI = Uri
            .withAppendedPath(DATA_OTHERS_URI, "filter");

    public DataItemsPickerAdapter(Context context, ListView lv) {
        super(context, lv);
    }

    @Override
    protected Uri configLoaderUri(long directoryId) {
        Uri uri;
        boolean isSearchMode = isSearchMode();
        Log.i(TAG, "[configLoaderUri]directoryId = " + directoryId + ",isSearchMode = "
                + isSearchMode);
        if (directoryId != Directory.DEFAULT) {
            Log.w(TAG,
                    "[configLoaderUri] MultiDataItemsPickerAdapter is not ready for non-default " +
                            "directory ID (directoryId: " + directoryId + ")");
        }

        if (isSearchMode) {
            String query = getQueryString();
            Builder builder = DATA_OTHERS_FILTER_URI.buildUpon();
            builder.appendQueryParameter("specified_data_mime_type", mMimeType);
            if (TextUtils.isEmpty(query)) {
                builder.appendPath("");
            } else {
                builder.appendPath(query); // Builder will encode the query
            }

            builder.appendQueryParameter(ContactsContract.DIRECTORY_PARAM_KEY,
                    String.valueOf(directoryId));
            builder.appendQueryParameter("checked_ids_arg", DATA_OTHERS_URI.toString());
            uri = builder.build();
        } else {
            uri = DATA_OTHERS_URI
                    .buildUpon()
                    .appendQueryParameter(ContactsContract.DIRECTORY_PARAM_KEY,
                            String.valueOf(Directory.DEFAULT))
                    .appendQueryParameter("specified_data_mime_type", mMimeType)
                    .appendQueryParameter("checked_ids_arg", DATA_OTHERS_URI.toString()).build();
            if (isSectionHeaderDisplayEnabled()) {
                uri = buildSectionIndexerUri(uri);
            }
        }

        return uri;
    }

    protected void configureSelection(CursorLoader loader, long directoryId,
            ContactListFilter filter) {

        super.configureSelection(loader, directoryId, filter);
        StringBuilder selection = new StringBuilder();
        if (mRestrictPhoneIds != null && mRestrictPhoneIds.length > 0) {
            selection.append("( ");
            selection.append(Phone._ID + " IN (");
            for (long id : mRestrictPhoneIds) {
                selection.append(id + ",");
            }
            selection.deleteCharAt(selection.length() - 1);
            selection.append(") )");
        } else {
            selection.append("(0)");
        }
        selection.append(loader.getSelection());
        loader.setSelection(selection.toString());
    }

    public void setRestrictList(long[] phoneIds) {
        mRestrictPhoneIds = phoneIds;
    }

    public void setMimetype(String mimeType) {
        mMimeType = mimeType;
    }
}
