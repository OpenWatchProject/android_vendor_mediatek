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
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.provider.ContactsContract.Contacts;

import com.mediatek.contacts.util.Log;

public class MultiVCardPickerFragment extends MultiBasePickerFragment {
    private static final String TAG = "MultiVCardPickerFragment";

    private static final String[] LOOKUPPROJECT = new String[] { Contacts.LOOKUP_KEY };

    @Override
    public void onOptionAction() {
        final long[] idArray = getCheckedItemIds();
        if (idArray == null) {
            Log.w(TAG, "[onOptionAction]idArray is null!");
            return;
        }

        // don't distict whether one or more contacts selected
        // if (idArray.length == 1) {
        // // Get lookup uri for single contact
        // uri = getLookupUriForEmail("Single_Contact", idArray);
        // }
        // Get lookup uri for more than one contacts
        Uri uri = getLookupUriForEmail("Multi_Contact", idArray);

        Log.d(TAG, "[onOptionAction] The result uri is " + uri);

        final Intent retIntent = new Intent();
        final Activity activity = getActivity();
        retIntent.putExtra(RESULT_INTENT_EXTRA_NAME, uri);

        activity.setResult(Activity.RESULT_OK, retIntent);
        activity.finish();
    }

    private Uri getLookupUriForEmail(String type, long[] contactsIds) {
        Cursor cursor = null;
        Uri uri = null;
        Log.d(TAG, "[getLookupUriForEmail]type :" + type);
        if ("Single_Contact".equals(type)) {
            uri = Uri.withAppendedPath(Contacts.CONTENT_URI, Long.toString(contactsIds[0]));

            cursor = getActivity().getContentResolver().query(uri, LOOKUPPROJECT, null, null, null);

            if (cursor != null && cursor.moveToNext()) {
                Log.i(TAG, "Single_Contact  cursor.getCount() is " + cursor.getCount());

                uri = Uri.withAppendedPath(Contacts.CONTENT_VCARD_URI, cursor.getString(0));
            }
        } else if ("Multi_Contact".equals(type)) {
            StringBuilder sb = new StringBuilder("");
            for (long contactId : contactsIds) {
                if (contactId == contactsIds[contactsIds.length - 1]) {
                    sb.append(contactId);
                } else {
                    sb.append(contactId + ",");
                }
            }
            String selection = Contacts._ID + " in (" + sb.toString() + ")";
            cursor = getActivity().getContentResolver().query(Contacts.CONTENT_URI, LOOKUPPROJECT,
                    selection, null, null);
            if (cursor == null) {
                return null;
            }

            Log.i(TAG, "[getLookupUriForEmail]  cursor.getCount() is " + cursor.getCount());
            if (!cursor.moveToFirst()) {
                cursor.close();
                return null;
            }

            StringBuilder uriListBuilder = new StringBuilder();
            int index = 0;
            for (; !cursor.isAfterLast(); cursor.moveToNext()) {
                if (index != 0) {
                    uriListBuilder.append(':');
                }
                uriListBuilder.append(cursor.getString(0));
                index++;
            }
            uri = Uri.withAppendedPath(Contacts.CONTENT_MULTI_VCARD_URI,
                    Uri.encode(uriListBuilder.toString()));
        }

        if (cursor != null) {
            cursor.close();
        }

        return uri;

    }

}
