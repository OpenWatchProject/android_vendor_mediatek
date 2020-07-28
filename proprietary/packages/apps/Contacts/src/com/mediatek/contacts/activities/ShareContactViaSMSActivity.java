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

package com.mediatek.contacts.activities;

import android.app.Activity;
import android.app.ProgressDialog;
import android.content.ActivityNotFoundException;
import android.content.DialogInterface;
import android.content.DialogInterface.OnCancelListener;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.provider.ContactsContract.CommonDataKinds;
import android.provider.ContactsContract.Contacts;
import android.provider.ContactsContract.Data;
import android.provider.ContactsContract.Profile;
import android.text.TextUtils;
import android.widget.Toast;


import com.android.contacts.R;

import com.mediatek.contacts.util.Log;

import java.util.ArrayList;
import java.util.List;

public class ShareContactViaSMSActivity extends Activity {
    private static final String TAG = "ShareContactViaSMSActivity";

    private String mAction;
    private Uri mDataUri;
    private int mSingleContactId = -1;
    private boolean mIsUserProfile = false;
    String mLookUpUris;
    Intent mIntent;
    private ProgressDialog mProgressDialog;
    private SearchContactThread mSearchContactThread;

    static final String[] CONTACTS_PROJECTION = new String[] { Contacts._ID, // 0
            Contacts.DISPLAY_NAME_PRIMARY, // 1
            Contacts.DISPLAY_NAME_ALTERNATIVE, // 2
            Contacts.SORT_KEY_PRIMARY, // 3
            Contacts.DISPLAY_NAME, // 4
    };

    static final int PHONE_ID_COLUMN_INDEX = 0;

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        mIntent = getIntent();
        mAction = mIntent.getAction();
        String contactId = mIntent.getStringExtra("contactId");
        String userProfile = mIntent.getStringExtra("userProfile");
        if (userProfile != null && "true".equals(userProfile)) {
            mIsUserProfile = true;
        }

        if (contactId != null && !"".equals(contactId)) {
            mSingleContactId = Integer.parseInt(contactId);
        }

        /**Bug Fix for CR: ALPS00395378 @{
         * Original Code:
         * mLookUpUris = mIntent.getStringExtra("LOOKUPURIS");
         */
        final Uri extraUri = (Uri) mIntent.getExtra(Intent.EXTRA_STREAM);
        mLookUpUris = null;
        if (null != extraUri) {
            mLookUpUris = extraUri.getLastPathSegment();
        }
        /** @} Bug fix for CR: ALPS00395378 */

        /** Bug Fix for ALPS00407311 @{ */
        if ((null != extraUri && extraUri.toString().startsWith("file") &&
                mSingleContactId == -1 && mIsUserProfile == false)
                || TextUtils.isEmpty(mLookUpUris)) {
            Log.i(TAG, "[onCreate]send file vis sms error,return.");
            Toast.makeText(this.getApplicationContext(), getString(R.string.send_file_sms_error),
                    Toast.LENGTH_SHORT).show();
            finish();
            return;
        }
        /** @} */
    }

    @Override
    public void onBackPressed() {
        super.onBackPressed();
        Log.i(TAG, "[onBackPressed]In onBackPressed.");
        finish();
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (Intent.ACTION_SEND.equals(mAction)
                && mIntent.hasExtra(Intent.EXTRA_STREAM)) {
            createSearchContactThread();
            showProgressDialog();
        }
    }

    private void createSearchContactThread() {
        if (mSearchContactThread == null) {
            mSearchContactThread = new SearchContactThread();
        }
    }

    private void showProgressDialog() {
        if (mProgressDialog == null) {
            String title = getString(R.string.please_wait);
            String message = getString(R.string.please_wait);
            mProgressDialog = ProgressDialog.show(this, title, message, true,
                    false);
            mProgressDialog.setOnCancelListener(mSearchContactThread);
            mSearchContactThread.start();
        }
    }

    private void shareViaSMS(String shareLookUpUris) {
        StringBuilder contactsID = new StringBuilder();
        int curIndex = 0;
        Cursor cursor = null;
        String id = null;
        String textVCard = "";
        Log.i(TAG, "[shareViaSMS]mUserProfile = " + mIsUserProfile);
        if (mIsUserProfile) {
            cursor = getContentResolver().query(
                    Profile.CONTENT_URI.buildUpon().appendPath("data").build(),
                    new String[]{Data.CONTACT_ID, Data.MIMETYPE, Data.DATA1}, null, null, null);
            if (cursor != null) {
                textVCard = getVCardString(cursor);
                cursor.close();
            }
        }
        else {
            if (mSingleContactId == -1) {
                String[] tempUris = shareLookUpUris.split(":");
                StringBuilder selection = new StringBuilder(Contacts.LOOKUP_KEY
                        + " in (");
                int index = 0;
                for (int i = 0; i < tempUris.length; i++) {
                    selection.append("'" + tempUris[i] + "'");
                    if (index != tempUris.length - 1) {
                        selection.append(",");
                    }
                    index++;
                }

                selection.append(")");
                cursor = getContentResolver().query(
                        /* dataUri */Contacts.CONTENT_URI, CONTACTS_PROJECTION,
                        selection.toString(), null, Contacts.SORT_KEY_PRIMARY);
                if (null != cursor) {
                    while (cursor.moveToNext()) {
                        if (cursor != null) {
                            id = cursor.getString(PHONE_ID_COLUMN_INDEX);
                        }
                        if (curIndex++ != 0) {
                            contactsID.append("," + id);
                        } else {
                            contactsID.append(id);
                        }
                    }
                    cursor.close();
                }
            } else {
                id = Integer.toString(mSingleContactId);
                contactsID.append(id);
            }

            Log.i(TAG, "[shareViaSMS]contactsID:" + contactsID.toString());

            long[] contactsIds = null;
            final String contactsIDStr = contactsID.toString();
            if (contactsIDStr != null && !"".equals(contactsIDStr)) {
                String[] vCardConIds = contactsIDStr.split(",");
                Log.d(TAG, "[shareViaSMS]vCardConIds.length:" + vCardConIds.length
                        + ",contactsIDStr:" + contactsIDStr);
                contactsIds = new long[vCardConIds.length];
                try {
                    for (int i = 0; i < vCardConIds.length; i++) {
                        contactsIds[i] = Long.parseLong(vCardConIds[i]);
                    }
                } catch (NumberFormatException e) {
                    Log.e(TAG, "[shareViaSMS]NumberFormatException:" + e.toString());
                    contactsIds = null;
                }
            }
            if (contactsIds != null && contactsIds.length > 0) {
                Log.d(TAG, "[shareViaSMS]contactsIds.length() = "
                        + contactsIds.length);

                StringBuilder sb = new StringBuilder("");
                for (long contactId : contactsIds) {
                    if (contactId == contactsIds[contactsIds.length - 1]) {
                        sb.append(contactId);
                    } else {
                        sb.append(contactId + ",");
                    }
                }
                String selection = Data.CONTACT_ID + " in (" + sb.toString() + ")";

                Log.i(TAG, "[shareViaSMS]selection = " + selection);
                Uri shareDataUri = Uri.parse("content://com.android.contacts/data");
                Log.i(TAG, "[shareViaSMS]Before query to build contact name  string ");
                Cursor c = getContentResolver()
                        .query(
                                shareDataUri, // URI
                                new String[] { Data.CONTACT_ID, Data.MIMETYPE,
                                        Data.DATA1 }, // projection
                                selection, // selection
                                null, // selection args
                                Contacts.SORT_KEY_PRIMARY + " , " + Data.CONTACT_ID); // sortOrder
                Log.i(TAG, "[shareViaSMS]After query to build contact name and  string ");
                if (c != null) {
                    Log.i(TAG, "[shareViaSMS]Before getVCardString ");
                    textVCard = getVCardString(c);
                    Log.i(TAG, "[shareViaSMS]After getVCardString ");
                    c.close();
                }
            }
        }
        Log.i(TAG, "[shareViaSMS]textVCard is :" + " \n" + textVCard);
        Intent i = new Intent(Intent.ACTION_SENDTO, Uri.fromParts("sms", "",
                null));
        i.putExtra("sms_body", textVCard);
        try {
            ShareContactViaSMSActivity.this.startActivity(i);
        } catch (ActivityNotFoundException e) {
            ShareContactViaSMSActivity.this.runOnUiThread(new Runnable() {
                public void run() {
                    Toast.makeText(
                            ShareContactViaSMSActivity.this.getApplicationContext(),
                            getString(R.string.quickcontact_missing_app),
                            Toast.LENGTH_SHORT).show();
                }
            });
            Log.e(TAG, "ActivityNotFoundException :" + e.toString());
        }

        finish();
    }

    // create the String of vCard via Contacts message
    private String getVCardString(Cursor cursor) {
        final int dataContactId = 0;
        final int dataMimeType = 1;
        final int dataString = 2;
        long contactId = 0L;
        long contactCurrentId = 0L;
        String mimeType;
        TextVCardContact tvc = new TextVCardContact();
        StringBuilder vcards = new StringBuilder();

        while (cursor.moveToNext()) {
            contactId = cursor.getLong(dataContactId);
            mimeType = cursor.getString(dataMimeType);
            if (contactCurrentId == 0L) {
                contactCurrentId = contactId;
            }

            // put one contact information into textVCard string
            if (contactId != contactCurrentId) {
                contactCurrentId = contactId;
                vcards.append(tvc.toString());
                tvc.reset();
            }

            // get cursor data
            if (CommonDataKinds.StructuredName.CONTENT_ITEM_TYPE
                    .equals(mimeType)) {
                tvc.mName = cursor.getString(dataString);
            }
            if (CommonDataKinds.Phone.CONTENT_ITEM_TYPE.equals(mimeType)) {
                tvc.mNumbers.add(cursor.getString(dataString));
            }
            if (CommonDataKinds.Email.CONTENT_ITEM_TYPE.equals(mimeType)) {
                tvc.mOmails.add(cursor.getString(dataString));
            }
            if (CommonDataKinds.Organization.CONTENT_ITEM_TYPE.equals(mimeType)) {
                tvc.mOrganizations.add(cursor.getString(dataString));
            }
            // put the last one contact information into textVCard string
            if (cursor.isLast()) {
                ///fix CR:ALPS001006656,the blank space was so much when share contacts by message
                vcards.append(tvc.toString());
            }
        }

        return vcards.toString();
    }

    private class SearchContactThread extends Thread implements
            OnCancelListener, OnClickListener {
        public SearchContactThread() {
        }

        @Override
        public void run() {
            String type = mIntent.getType();
            mDataUri = (Uri) mIntent.getParcelableExtra(Intent.EXTRA_STREAM);
            Log.i(TAG, "[run]dataUri is :" + Log.anonymize(mDataUri) + ",type:" + type);
            if (mDataUri != null && type != null) {
                shareViaSMS(mLookUpUris);
            }
        }

        public void onCancel(DialogInterface dialog) {
            finish();
        }

        public void onClick(DialogInterface dialog, int which) {
            if (which == DialogInterface.BUTTON_NEGATIVE) {
                finish();
            }
        }
    }

    private class TextVCardContact {
        protected String mName = "";
        protected List<String> mNumbers = new ArrayList<String>();
        protected List<String> mOmails = new ArrayList<String>();
        protected List<String> mOrganizations = new ArrayList<String>();

        protected void reset() {
            mName = "";
            mNumbers.clear();
            mOmails.clear();
            mOrganizations.clear();
        }

        @Override
        public String toString() {
            String textVCardString = "";
            int i = 1;
            if (mName != null && !mName.equals("")) {
                textVCardString += getString(R.string.nameLabelsGroup) + ": "
                        + mName + "\n";
            }
            if (!mNumbers.isEmpty()) {
                if (mNumbers.size() > 1) {
                    i = 1;
                    for (String number : mNumbers) {
                        textVCardString += "Tel" + i + ": " + number + "\n";
                        i++;
                    }
                } else {
                    textVCardString += "Tel" + ": " + mNumbers.get(0) + "\n";
                }
            }
            if (!mOmails.isEmpty()) {
                if (mOmails.size() > 1) {
                    i = 1;
                    for (String email : mOmails) {
                        textVCardString += getString(R.string.email_other) + i
                                + ": " + email + "\n";
                        i++;
                    }
                } else {
                    textVCardString += getString(R.string.email_other) + ": "
                            + mOmails.get(0) + "\n";
                }
            }
            if (!mOrganizations.isEmpty()) {
                if (mOrganizations.size() > 1) {
                    i = 1;
                    for (String organization : mOrganizations) {
                        textVCardString += getString(R.string.organizationLabelsGroup)
                                + i + ": " + organization + "\n";
                        i++;
                    }
                } else {
                    textVCardString += getString(R.string.organizationLabelsGroup)
                            + ": " + mOrganizations.get(0) + "\n";
                }
            }
            return textVCardString;
        }
    }
}
