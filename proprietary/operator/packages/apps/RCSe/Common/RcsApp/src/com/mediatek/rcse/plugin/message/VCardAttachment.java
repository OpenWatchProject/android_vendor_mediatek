/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2012. All rights reserved.
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

/*
 * Copyright (C) 2008 Esmertec AG.
 * Copyright (C) 2008 The Android Open Source Project
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

package com.mediatek.rcse.plugin.message;

import android.database.Cursor;
import android.content.Context;
import android.net.Uri;
import android.provider.ContactsContract.Data;
import android.provider.ContactsContract.Contacts;
import android.provider.ContactsContract.RawContacts;
import android.text.format.DateFormat;
import android.util.Log;
import android.provider.ContactsContract.CommonDataKinds;

import com.android.vcard.VCardComposer;
import com.android.vcard.exception.VCardException;
import com.mediatek.mms.ipmessage.DefaultIpConfigExt;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcs.R;

import java.io.FileOutputStream;
import java.io.Writer;
import java.io.BufferedWriter;
import java.io.OutputStreamWriter;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.Date;
import java.util.List;
import java.util.ArrayList;

/** M: Support to add text vcard or vcard attachment to compose activity.*/
public class VCardAttachment {
    private Context mContext;
    private String mVCardFileName = "";
    private String mName = "";
    private List<String> mNumbers = new ArrayList<String>();
    private List<String> mEmails = new ArrayList<String>();
    private List<String> mOrganizations = new ArrayList<String>();
    private static final String TAG = "Mms/TextVCardContact";

    public VCardAttachment(Context context) {
        this.mContext = context;
    }

    public void reset() {
        mName = "";
        mNumbers.clear();
        mEmails.clear();
        mOrganizations.clear();
    }

    @Override
    public String toString() {
        String textVCardString = "";
        int i = 1;
        if (mName != null && !mName.equals("")) {
            textVCardString += mContext.getString(R.string.contact_name) + ": " + mName + "\n";
        }
        if (!mNumbers.isEmpty()) {
            if (mNumbers.size() > 1) {
                i = 1;
                StringBuffer buf = new StringBuffer();
                buf.append(textVCardString);
                for (String number : mNumbers) {
                    buf.append(mContext.getString(R.string.contact_tel) + i + ": " + number + "\n");
                    i++;
                }
                textVCardString = buf.toString();
            } else {
                textVCardString += mContext.getString(R.string.contact_tel) + ": " + mNumbers.get(0)
                        + "\n";
            }
        }
        if (!mEmails.isEmpty()) {
            if (mEmails.size() > 1) {
                i = 1;
                StringBuffer buf = new StringBuffer();
                buf.append(textVCardString);
                for (String email : mEmails) {
                    buf.append(mContext.getString(R.string.contact_email) + i + ": " + email + "\n");
                    i++;
                }
                textVCardString = buf.toString();
            } else {
                textVCardString += mContext.getString(R.string.contact_email) + ": " + mEmails.get(0)
                        + "\n";
            }
        }
        if (!mOrganizations.isEmpty()) {
            if (mOrganizations.size() > 1) {
                i = 1;
                StringBuffer buf = new StringBuffer();
                buf.append(textVCardString);
                for (String organization : mOrganizations) {
                    buf.append(mContext.getString(R.string.contact_organization) + i + ": "
                            + organization + "\n");
                    i++;
                }
                textVCardString = buf.toString();
            } else {
                textVCardString += mContext.getString(R.string.contact_organization) + ": "
                        + mOrganizations.get(0) + "\n";
            }
        }
        return textVCardString;
    }

    public String getTextVCardString(long[] contactsIds, String textVCard) {
        Logger.i(TAG, "contactsIds.length() = " + contactsIds.length);
        // String textVCard = TextUtils.isEmpty(mTextEditor.getText())? "":
        // "\n";
        StringBuilder sb = new StringBuilder("");
        for (long contactId : contactsIds) {
            if (contactId == contactsIds[contactsIds.length - 1]) {
                sb.append(contactId);
            } else {
                sb.append(contactId + ",");
            }
        }
        String selection = Data.CONTACT_ID + " in (" + sb.toString() + ")";

        Logger.i(TAG, "compose.addTextVCard(): selection = " + selection);
        Uri dataUri = Uri.parse("content://com.android.contacts/data");
        Cursor cursor = mContext.getContentResolver().query(dataUri, // URI
                new String[] {
                        Data.CONTACT_ID, Data.MIMETYPE, Data.DATA1
                }, // projection
                selection, // selection
                null, // selection args
                RawContacts.SORT_KEY_PRIMARY + ", " + Data.CONTACT_ID); // sortOrder
        if (cursor != null) {
            textVCard = getVCardString(cursor, textVCard);
            // final String textString = textVCard;
            cursor.close();
            return textVCard;
            /*
             * runOnUiThread(new Runnable() { public void run() {
             * insertText(mTextEditor, textString); } });
             */

        }
        return textVCard;

    }

    // create the String of vCard via Contacts message
    public String getVCardString(Cursor cursor, String textVCard) {
        final int dataContactId = 0;
        final int dataMimeType = 1;
        final int dataString = 2;
        long contactId = 0L;
        long contactCurrentId = 0L;
        int i = 1;
        String mimeType;
        VCardAttachment tvc = new VCardAttachment(mContext);
        int j = 0;
        while (cursor.moveToNext()) {
            contactId = cursor.getLong(dataContactId);
            mimeType = cursor.getString(dataMimeType);
            if (contactCurrentId == 0L) {
                contactCurrentId = contactId;
            }

            // put one contact information into textVCard string
            if (contactId != contactCurrentId) {
                contactCurrentId = contactId;
                textVCard += tvc.toString();
                tvc.reset();
            }

            // get cursor data
            if (CommonDataKinds.StructuredName.CONTENT_ITEM_TYPE.equals(mimeType)) {
                tvc.mName = cursor.getString(dataString);
            }
            if (CommonDataKinds.Phone.CONTENT_ITEM_TYPE.equals(mimeType)) {
                tvc.mNumbers.add(cursor.getString(dataString));
            }
            if (CommonDataKinds.Email.CONTENT_ITEM_TYPE.equals(mimeType)) {
                tvc.mEmails.add(cursor.getString(dataString));
            }
            if (CommonDataKinds.Organization.CONTENT_ITEM_TYPE.equals(mimeType)) {
                tvc.mOrganizations.add(cursor.getString(dataString));
            }
            // put the last one contact information into textVCard string
            if (cursor.isLast()) {
                textVCard += tvc.toString();
            }
            j++;
            if (j % 10 == 0) {
                if (textVCard.length() > IpMessageManager.getInstance(mContext).getMaxTextLimit()) {
                    break;
                }
            }
        }
        Logger.i(TAG, "textVCard= " + textVCard);
        return textVCard;
    }

    public String getVCardFileNameByContactsId(long[] contactsIds, boolean isAddingIpMessageVCardFile) {
        Logger.d(TAG, "getVCardFileNameByContactsId(): isAddingIpMessageVCardFile = "
                + isAddingIpMessageVCardFile +"contactsIds=" + contactsIds);
        // make contacts' id string
        StringBuilder contactsIdsStr = new StringBuilder("");
        for (int i = 0; i < contactsIds.length - 1; i++) {
            contactsIdsStr.append(contactsIds[i]);
            contactsIdsStr.append(',');
        }
        contactsIdsStr.append(contactsIds[contactsIds.length - 1]);
        final String ids = contactsIdsStr.toString();
        String fileName = "";
        if (!ids.equals("")) {
           /* if (isAddingIpMessageVCardFile) {
                mVCardFileName = getVCardFileNameForIpMessage(contactsIds);
            }*/
            Logger.d(TAG, "getVCardFileNameByContactsId(): mVCardFileName = "
                + mVCardFileName);
            fileName = attachVCard(ids, isAddingIpMessageVCardFile);
            mVCardFileName = fileName;
        }
        return fileName;
    }

    /**
     * M: turn contacts id into *.vcf file attachment
     *
     * @param mContactIds
     */
    public String attachVCard(String mContactIds, boolean isAddingIpMessageVCardFile) {
        if (mContactIds == null || mContactIds.equals("")) {
            return new String();
        }
        Logger.d(TAG, "attachVCard(): isAddingIpMessageVCardFile = "
                + isAddingIpMessageVCardFile);
        String fileName = createVCardFileName();
        Logger.d(TAG, "attachVCard(): fileName = "
                + fileName);
        try {
            VCardComposer composer = null;
            Writer writer = null;
            try {
                if (isAddingIpMessageVCardFile) {
                    writer = new BufferedWriter(new OutputStreamWriter(new FileOutputStream(
                        IpMessageUtils.getCachePath(mContext) + fileName)));
                } else {
                    writer = new BufferedWriter(new OutputStreamWriter(mContext.openFileOutput(fileName,
                        Context.MODE_PRIVATE)));
                }
                Logger.d(TAG, "attachVCard()22: isAddingIpMessageVCardFile = "
                        + isAddingIpMessageVCardFile);
                composer = new VCardComposer(mContext);
                if (!composer.init(Contacts._ID + " IN (" + mContactIds + ")", null)) {
                    Log.d(TAG, "attachVCard init exception, ");
                    throw new VCardException("Canot initialize " + composer.getErrorReason());
                }
                while (!composer.isAfterLast()) {
                    writer.write(composer.createOneEntry());
                }
            } finally {
                if (composer != null) {
                    composer.terminate();
                }
                if (writer != null) {
                    writer.close();
                }
            }
        } catch (VCardException e) {
            Log.e(TAG, "export vcard file, vcard exception " + e.getMessage());
        } catch (FileNotFoundException e) {
            Log.e(TAG, "export vcard file, file not found exception " + e.getMessage());
        } catch (IOException e) {
            Log.e(TAG, "export vcard file, IO exception " + e.getMessage());
        } catch (Exception e) {
            Log.e(TAG, "export vcard file, exception " + e.getMessage());
        }

        Logger.d(TAG, "write vCard file done!");
        return fileName;
        // setFileAttachment(fileName, WorkingMessage.VCARD, false);
    }

    private String createVCardFileName() {
        Logger.d(TAG, "createVCardFileName(): entry");
        final String fileExtension = ".vcf";
        // base on time stamp
        String name = DateFormat.format("yyyyMMdd_hhmmss", new Date(System.currentTimeMillis()))
                .toString();
        name = name.trim();
        return name + fileExtension;
    }

    // the uri must be a vcard uri created by Contacts
    public String getVCardFileNameByUri(Uri uri) {
        if (uri == null) {
            return new String();
        }
        final String filename = createVCardFileName();
        try {
            InputStream in = null;
            OutputStream out = null;
            try {
                in = mContext.getContentResolver().openInputStream(uri);
                out = mContext.openFileOutput(filename, Context.MODE_PRIVATE);
                byte[] buf = new byte[8096];
                int size = 0;
                while ((size = in.read(buf)) != -1) {
                    out.write(buf, 0, size);
                }
            } finally {
                if (in != null) {
                    in.close();
                }
                if (out != null) {
                    out.close();
                }
            }
        } catch (Exception e) {
            Log.e(TAG, "exception getVCardFileNameByUri ", e);
        }
        return filename;
    }

    private String getVCardFileNameForIpMessage(long[] contactIds) {
        try {
            Logger.d(TAG, "getVCardFileNameForIpMessage()contactIds length=" + contactIds.length);
        String fileName = "";
        if (contactIds.length == 1) {
            long contactId = contactIds[0];
                Logger.d(TAG, "getVCardFileNameForIpMessage() contactId= " + contactId);
            String selection = Data.CONTACT_ID + " = " + contactId;
            Cursor cursor = mContext.getContentResolver().query(
                Data.CONTENT_URI, //dataUri, // URI
                    new String[]{Data.CONTACT_ID, Data.MIMETYPE, Data.DATA1}, // projection
                selection, // selection
                null, // selection args
                RawContacts.SORT_KEY_PRIMARY); // sortOrder
                Logger.d(TAG, "getVCardFileNameForIpMessage() cursor" + cursor);
            if (null != cursor) {
                String mimeType;
                    Logger.d(TAG, "getVCardFileNameForIpMessage() cursor size" + cursor.getCount());
                while (cursor.moveToNext()) {
                        Logger.d(TAG, "getVCardFileNameForIpMessage() inside cursor ");
                    if (CommonDataKinds.StructuredName.CONTENT_ITEM_TYPE.equals(cursor.getString(0))) {
                            Logger.d(TAG, "getVCardFileNameForIpMessage() inside CommonKinds ");
                        fileName = cursor.getString(1);
                        break;
                    }
                }
                cursor.close();
            }
        } else {
            fileName = IpMessageResourceMananger.getInstance(mContext)
                .getSingleString(IpMessageConsts.string.ipmsg_vcard_file_name);
        }
            Logger.d(TAG, "getVCardFileNameForIpMessage() filename= " + fileName);
        return fileName + ".vcf";
        } catch (Exception e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
            Logger.d(TAG, "getVCardFileNameForIpMessage() exception" + e.getMessage().toString());
            return "NE.vcf";
        }
    }
}
