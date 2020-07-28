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
package com.mediatek.contacts;

import android.content.ContentProviderOperation;
import android.content.ContentResolver;
import android.content.Context;
import android.content.OperationApplicationException;
import android.database.Cursor;
import android.net.Uri;
import android.os.Handler;
import android.os.Looper;
import android.os.RemoteException;
import android.provider.ContactsContract;
import android.provider.ContactsContract.CommonDataKinds.Photo;
import android.provider.ContactsContract.Data;
import android.provider.ContactsContract.RawContacts;
import android.widget.Toast;

import com.android.contacts.ContactPhotoManager;
import com.android.contacts.R;

import com.mediatek.contacts.util.Log;
import com.mediatek.provider.MtkContactsContract;

import java.util.ArrayList;

public class ContactSaveServiceEx {
    private static final String TAG = "ContactSaveServiceEx";

    private static Handler sMainHandler = new Handler(Looper.getMainLooper());

    /**
     * Shows a toast on the UI thread.
     */
    private static void showToast(final int message) {
        sMainHandler.post(new Runnable() {
            @Override
            public void run() {
                Toast.makeText(ContactsApplicationEx.getContactsApplication(), message,
                        Toast.LENGTH_LONG).show();
            }
        });
    }

    /**
     * fix ALPS00272729
     *
     * @param operations
     * @param resolver
     */
    public static void bufferOperations(ArrayList<ContentProviderOperation> operations,
            ContentResolver resolver) {
        try {
            Log.d(TAG, "[bufferOperatation] begin applyBatch ");
            resolver.applyBatch(ContactsContract.AUTHORITY, operations);
            Log.d(TAG, "[bufferOperatation] end applyBatch");
            operations.clear();
        } catch (RemoteException e) {
            Log.e(TAG, "[bufferOperatation]RemoteException:", e);
            showToast(R.string.contactSavedErrorToast);
        } catch (OperationApplicationException e) {
            Log.e(TAG, "[bufferOperatation]OperationApplicationException:", e);
            showToast(R.string.contactSavedErrorToast);
        }
    }

    public static boolean containSimContact(final long[] contactIds, ContentResolver resolver) {
        final String[] projection = new String[]{
                MtkContactsContract.RawContactsColumns.INDEX_IN_SIM  // column index = 0
                };
        final StringBuilder selection = new StringBuilder();
        selection.append(RawContacts.CONTACT_ID + " IN (");
        for (int i = 0; i < contactIds.length; i++) {
            selection.append(String.valueOf(contactIds[i]));
            if (i < contactIds.length - 1) {
                selection.append(",");
            }
        }
        selection.append(")");
        final Cursor cursor = resolver.query(RawContacts.CONTENT_URI,
                projection, selection.toString(), null, null, null);
        if (cursor == null) {
            Log.e(TAG, "[containSimContact] query fail, cursor is null!");
            return false;
        }
        try {
            while (cursor.moveToNext()) {
                if (cursor.getLong(0) > 0) {
                    Log.d(TAG, "[containSimContact]return true, index_in_sim=" + cursor.getLong(0));
                    return true;
                }
            }
        } finally {
            cursor.close();
        }
        return false;
    }

    public static void refreshPhotoCache(long rawContactId) {
        // get the photo id in data table that belongs to rawContactId
        final Context context = GlobalEnv.getApplicationContext();
        Cursor cursor = null;
        final long photoId;
        String selection = Data.RAW_CONTACT_ID + "=? AND " + Data.MIMETYPE + "=?";
        final String[] selectionArgs = new String[2];
        selectionArgs[0] = String.valueOf(rawContactId);
        selectionArgs[1] = Photo.CONTENT_ITEM_TYPE;
        try {
            cursor = context.getContentResolver().query(
                    Data.CONTENT_URI,
                    new String[] { Photo._ID },
                    selection,
                    selectionArgs,
                    null);
            if (cursor != null && cursor.getCount() == 1) {
                // 1 raw contact is expected has 1 photo
                cursor.moveToPosition(-1);
                cursor.moveToNext();
                photoId = cursor.getLong(0);
                Log.d(TAG, "[refreshPhotoCache] rawContactId = " + rawContactId
                        + ", photoId = " + photoId);

                // [ALPS04672158] ContactPhotoManager should be called from main thread to ensure
                // ContactPhotoManagerImpl's mMainThreadHandler is a Handler of main thread, or else
                // would send messages fail for the thread is already dead.
                sMainHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        ContactPhotoManager.getInstance(context).refreshCacheByKey(photoId);
                    }
                });
            } else {
                Log.w(TAG, "[refreshPhotoCache] return for cursor is null or count is not 1");
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
    }
}
