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
import android.content.ContentProviderResult;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.OperationApplicationException;
import android.database.Cursor;
import android.net.Uri;
import android.os.RemoteException;
import android.provider.ContactsContract;
import android.provider.ContactsContract.CommonDataKinds.Email;
import android.provider.ContactsContract.CommonDataKinds.GroupMembership;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.ContactsContract.CommonDataKinds.StructuredName;
import android.provider.ContactsContract.Data;
import android.provider.ContactsContract.RawContacts;
import android.text.TextUtils;

import com.android.contacts.ContactsUtils;
import com.android.contacts.model.account.AccountWithDataSet;

import com.mediatek.contacts.simcontact.SimCardUtils;
import com.mediatek.contacts.util.Log;
import com.mediatek.provider.MtkContactsContract;
import com.mediatek.provider.MtkContactsContract.Aas;

import java.util.ArrayList;
import java.util.Set;

public class SubContactsUtils extends ContactsUtils {
    private static final String TAG = "SubContactsUtils";

    public static long queryForRawContactId(ContentResolver cr, long contactId) {
        Log.i(TAG, "[queryForRawContactId]contactId:" + contactId);
        Cursor rawContactIdCursor = null;
        long rawContactId = -1;
        try {
            rawContactIdCursor = cr.query(RawContacts.CONTENT_URI,
                    new String[] { RawContacts._ID }, RawContacts.CONTACT_ID + "=" + contactId,
                    null, null);
            if (rawContactIdCursor != null && rawContactIdCursor.moveToFirst()) {
                // Just return the first one.
                rawContactId = rawContactIdCursor.getLong(0);
            }
        } finally {
            if (rawContactIdCursor != null) {
                rawContactIdCursor.close();
            }
        }
        return rawContactId;
    }

    // For index in SIM change feature, we add the 'int indexInSim' argument
    // into the argument list.

    public static Uri insertToDB(AccountWithDataSet mAccount, String name, String number,
            String email, String additionalNumber, ContentResolver resolver, long indicate,
            String simType, long indexInSim, Set<Long> grpAddIds) {
        final ArrayList<ContentProviderOperation> operationList =
                new ArrayList<ContentProviderOperation>();
        buildInsertOperation(operationList, mAccount, name, number, email, additionalNumber,
                resolver, (int) indicate, simType, indexInSim, grpAddIds);
        return insertToDBApplyBatch(resolver, operationList);
    }

    public static void buildInsertOperation(ArrayList<ContentProviderOperation> operationList,
            AccountWithDataSet mAccount, String name, String number, String email,
            String additionalNumber, ContentResolver resolver, int indicate, String simType,
            long indexInSim, Set<Long> grpAddIds) {
        Log.i(TAG, "[buildInsertOperation]name:" + Log.anonymize(name)
                + ", number :" + Log.anonymize(number)
                + ", email:" + Log.anonymize(email)
                + ", additionalNumber = " + Log.anonymize(additionalNumber)
                + ", indicate:" + indicate + ", simType:" + simType + ",indexInSim:" + indexInSim);

        if (operationList == null) {
            Log.w(TAG, "[buildInsertOperation]operationList is null!");
            return;
        }
        int backRef = operationList.size();
        // insert RawContacts info
        insertRawContacts(operationList, mAccount, indicate, indexInSim);

        int phoneType = 7;
        String phoneTypeSuffix = "";
        // ALPS00023212
        if (!TextUtils.isEmpty(name)) {
            final NamePhoneTypePair namePhoneTypePair = new NamePhoneTypePair(name);
            name = namePhoneTypePair.name;
            phoneType = namePhoneTypePair.phoneType;
            phoneTypeSuffix = namePhoneTypePair.phoneTypeSuffix;
        }

        // insert phone number
        insertPhoneNumber(number, operationList, backRef, phoneTypeSuffix);
        // insert name
        insertName(operationList, name, backRef);
        // if USIM
        if (SimCardUtils.SimType.SIM_TYPE_USIM_TAG.equals(simType)
                || SimCardUtils.SimType.SIM_TYPE_CSIM_TAG.equals(simType)) {
            // insert email
            insertEmail(operationList, email, backRef);
            // insert additionalNumber
            if (!GlobalEnv.getSimAasEditor().updateOperationList(
                    mAccount, operationList, backRef)) {
                Log.i(TAG, "[buildInsertOperation]Plugin handled host do nothing");
                insertAdditionalNumber(operationList, additionalNumber, backRef);
            }
            // for USIM Group
            insertGroup(operationList, grpAddIds, backRef);
        }
    }

    // for ALPS00023212
    public static class NamePhoneTypePair {
        public String name;
        public int phoneType;
        public String phoneTypeSuffix;

        public NamePhoneTypePair(String nameWithPhoneType) {
            // Look for /W /H /M or /O at the end of the name signifying the
            // type
            int nameLen = nameWithPhoneType.length();
            if (nameLen - 2 >= 0 && nameWithPhoneType.charAt(nameLen - 2) == '/') {
                char c = Character.toUpperCase(nameWithPhoneType.charAt(nameLen - 1));
                phoneTypeSuffix = String.valueOf(nameWithPhoneType.charAt(nameLen - 1));
                if (c == 'W') {
                    phoneType = Phone.TYPE_WORK;
                } else if (c == 'M' || c == 'O') {
                    phoneType = Phone.TYPE_MOBILE;
                } else if (c == 'H') {
                    phoneType = Phone.TYPE_HOME;
                } else {
                    phoneType = Phone.TYPE_OTHER;
                }
                name = nameWithPhoneType.substring(0, nameLen - 2);
            } else {
                phoneTypeSuffix = "";
                phoneType = Phone.TYPE_OTHER;
                name = nameWithPhoneType;
            }
        }
    }

    public static void insertPhoneNumber(String number,
            ArrayList<ContentProviderOperation> operationList,
            int backRef, String phoneTypeSuffix) {
        Log.i(TAG, "[insertPhoneNumber]phoneTypeSuffix is " + Log.anonymize(phoneTypeSuffix)
                + ",backRef:" + backRef + ",number = " + Log.anonymize(number));
        if (!TextUtils.isEmpty(number)) {
            ContentProviderOperation.Builder builder = ContentProviderOperation
                    .newInsert(Data.CONTENT_URI);
            builder.withValueBackReference(Phone.RAW_CONTACT_ID, backRef);
            builder.withValue(Data.MIMETYPE, Phone.CONTENT_ITEM_TYPE);
            builder.withValue(Phone.NUMBER, number);
            builder.withValue(Data.DATA2, Aas.PHONE_TYPE_EMPTY);

            if (!TextUtils.isEmpty(phoneTypeSuffix)) {
                builder.withValue(Data.DATA15, phoneTypeSuffix);
            }
            operationList.add(builder.build());
        }
    }

    public static void insertAdditionalNumber(ArrayList<ContentProviderOperation> operationList,
            String additionalNumber, int backRef) {
        Log.i(TAG, "[insertAdditionalNumber]additionalNumber is "
                + Log.anonymize(additionalNumber) + ",backRef:" + backRef);
        if (!TextUtils.isEmpty(additionalNumber)) {
            ContentProviderOperation.Builder builder = ContentProviderOperation
                    .newInsert(Data.CONTENT_URI);
            builder.withValueBackReference(Phone.RAW_CONTACT_ID, backRef);
            builder.withValue(Data.MIMETYPE, Phone.CONTENT_ITEM_TYPE);
            // builder.withValue(Phone.TYPE, phoneType);
            builder.withValue(Data.DATA2, 7);
            builder.withValue(Phone.NUMBER, additionalNumber);
            builder.withValue(MtkContactsContract.DataColumns.IS_ADDITIONAL_NUMBER, 1);
            operationList.add(builder.build());
        }
    }

    public static void insertName(ArrayList<ContentProviderOperation> operationList, String name,
            int backRef) {
        Log.i(TAG, "[insertName]name is " + Log.anonymize(name) + ",backRef:" + backRef);
        if (!TextUtils.isEmpty(name)) {
            ContentProviderOperation.Builder builder = ContentProviderOperation
                    .newInsert(Data.CONTENT_URI);
            builder.withValueBackReference(StructuredName.RAW_CONTACT_ID, backRef);
            builder.withValue(Data.MIMETYPE, StructuredName.CONTENT_ITEM_TYPE);
            builder.withValue(StructuredName.DISPLAY_NAME, name);
            operationList.add(builder.build());
        }
    }

    public static void insertEmail(ArrayList<ContentProviderOperation> operationList, String email,
            int backRef) {
        Log.i(TAG, "[insertEmail]email is " + Log.anonymize(email) + ",backRef:" + backRef);
        if (!TextUtils.isEmpty(email)) {
            // for (String emailAddress : emailAddressArray) {
            ContentProviderOperation.Builder builder = ContentProviderOperation
                    .newInsert(Data.CONTENT_URI);
            builder.withValueBackReference(Email.RAW_CONTACT_ID, backRef);
            builder.withValue(Data.MIMETYPE, Email.CONTENT_ITEM_TYPE);
            // don't show email type for SIM contact.
            //builder.withValue(Email.TYPE, Email.TYPE_MOBILE);
            builder.withValue(Email.DATA, email);
            operationList.add(builder.build());
        }
    }

    public static void insertGroup(ArrayList<ContentProviderOperation> operationList,
            Set<Long> grpAddIds, int backRef) {
        Log.i(TAG, "[insertGroup]backRef:" + backRef);
        if (grpAddIds != null && grpAddIds.size() > 0) {
            Long[] grpIdArray = grpAddIds.toArray(new Long[0]);
            for (Long grpId : grpIdArray) {
                ContentProviderOperation.Builder builder = ContentProviderOperation
                        .newInsert(Data.CONTENT_URI);
                builder.withValueBackReference(Phone.RAW_CONTACT_ID, backRef);
                builder.withValue(Data.MIMETYPE, GroupMembership.CONTENT_ITEM_TYPE);
                builder.withValue(GroupMembership.GROUP_ROW_ID, grpId);
                operationList.add(builder.build());
            }
        }
    }

    public static void insertRawContacts(ArrayList<ContentProviderOperation> operationList,
            AccountWithDataSet mAccount, int indicate, long indexInSim) {
        Log.i(TAG, "[insertRawContacts]indexInSim:" + indexInSim);
        ContentProviderOperation.Builder builder = ContentProviderOperation
                .newInsert(RawContacts.CONTENT_URI);
        ContentValues contactvalues = new ContentValues();
        contactvalues.put(RawContacts.ACCOUNT_NAME, mAccount.name);
        contactvalues.put(RawContacts.ACCOUNT_TYPE, mAccount.type);
        contactvalues.put(MtkContactsContract.RawContactsColumns.INDICATE_PHONE_SIM, indicate);
        contactvalues.put(RawContacts.AGGREGATION_MODE, RawContacts.AGGREGATION_MODE_DISABLED);
        contactvalues.put(MtkContactsContract.RawContactsColumns.INDEX_IN_SIM, indexInSim);
        builder.withValues(contactvalues);
        operationList.add(builder.build());
    }

    //ALPS03160365: modify to return raw contact uri instead of contact look up uri
    public static Uri insertToDBApplyBatch(ContentResolver resolver,
            ArrayList<ContentProviderOperation> operationList) {
        Log.i(TAG, "[insertToDBApplyBatch]..");
        Uri rawContactUri = null;
        try {
            ContentProviderResult[] result = resolver.applyBatch(ContactsContract.AUTHORITY,
                    operationList); // saved in database
            rawContactUri = result[0].uri;
            //retUri = RawContacts.getContactLookupUri(resolver, rawContactUri);
            Log.d(TAG, "[insertToDBApplyBatch]rawContactUri:" + Log.anonymize(rawContactUri));
            //Log.d(TAG, "[insertToDBApplyBatch]rawContactUri:" + rawContactUri + ",retUri:"
            //        + retUri);
        } catch (RemoteException e) {
            Log.e(TAG, "[insertToDBApplyBatch]RemoteException:" + e);
        } catch (OperationApplicationException e) {
            Log.e(TAG, "[insertToDBApplyBatch]OperationApplicationException:" + e);
        }
        return rawContactUri;
    }
}
