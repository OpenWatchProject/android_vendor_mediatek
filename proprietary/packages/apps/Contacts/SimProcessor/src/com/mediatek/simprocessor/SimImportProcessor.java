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
package com.mediatek.simprocessor;

import android.accounts.Account;
import android.accounts.AccountManager;
import android.content.ContentProviderOperation;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.OperationApplicationException;
import android.database.Cursor;
import android.location.Country;
import android.location.CountryDetector;
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

import com.android.i18n.phonenumbers.AsYouTypeFormatter;
import com.android.i18n.phonenumbers.PhoneNumberUtil;

import com.mediatek.provider.MtkContactsContract;
import com.mediatek.provider.MtkContactsContract.Aas;
import com.mediatek.simprocessor.SimProcessorUtils.NamePhoneTypePair;
import com.mediatek.simprocessor.SimCardUtils;
import com.mediatek.simprocessor.SubInfoUtils;
import com.mediatek.simprocessor.SimProcessorManager.ProcessorCompleteListener;
import com.mediatek.simprocessor.SimServiceUtils.ServiceWorkData;
import com.mediatek.simprocessor.SimProcessorUtils.USIMGroup;
import com.mediatek.simprocessor.Log;

import java.util.ArrayList;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Locale;

public class SimImportProcessor extends SimProcessorBase {
    private static final String TAG = "SimImportProcessor";

    private static final String[] COLUMN_NAMES = new String[] { "index", "name", "number",
            "emails", "additionalNumber", "groupIds" };

    protected static final int INDEX_COLUMN = 0; // index in SIM
    protected static final int NAME_COLUMN = 1;
    protected static final int NUMBER_COLUMN = 2;
    protected static final int EMAIL_COLUMN = 3;
    protected static final int ADDITIONAL_NUMBER_COLUMN = 4;
    protected static final int GROUP_COLUMN = 5;

    // In order to prevent locking DB too long,
    // set the max operation count 50 in a batch.
    private static final int MAX_OP_COUNT_IN_ONE_BATCH = 50;

    private HashMap<Integer, Integer> mGroupIdMap;

    private int mSubId;
    private Context mContext;
    private AccountManager mAccountManager;

    private final Lock dbLock = new ReentrantLock();

    public SimImportProcessor(Context context, int subId, Intent intent,
            ProcessorCompleteListener listener) {
        super(intent, listener);
        Log.i(TAG, "[SimImportProcessor]new...subId = " + subId);
        mContext = context;
        mSubId = subId;
        mGroupIdMap = new HashMap<Integer, Integer>();
        mAccountManager = AccountManager.get(mContext);
    }

    @Override
    public int getType() {
        return SimServiceUtils.TYPE_IMPORT;
    }

    @Override
    public void doWork() {
        Log.i(TAG, "[dowork]Processor [subId =" + mSubId + "] running...Thread id="
                + Thread.currentThread().getId());
        if (isCancelled()) {
            Log.i(TAG, "[dowork]cancel import work. Thread id="
                    + Thread.currentThread().getId());
            return;
        }
        try {
            SimServiceUtils.deleteSimContact(mContext, mSubId);
            if (isCancelled()) {
                Log.i(TAG, "[dowork]cancel import work after deleteSimContact. Thread id="
                        + Thread.currentThread().getId());
                return;
            }

            if (!checkPhbStateReady()) {
                Log.i(TAG, "[doWork]simStateReady is not ready, return!");
                return;
            }

            int simType = SimCardUtils.getSimTypeBySubId(mSubId);
            // If simType is invalid, return to avoid JE.
            if (simType < SimCardUtils.SimType.SIM_TYPE_SIM
                    || simType > SimCardUtils.SimType.SIM_TYPE_CSIM) {
                Log.w(TAG, "[doWork]can't get simType from TelphonyEx, return!");
                return;
            }
            final Uri iccUri = SubInfoUtils.getIccProviderUri(mSubId);
            Cursor simCursor = querySimContact(mContext, mSubId, simType, iccUri);
            Log.i(TAG, "[dowork]simType = " + simType + ",simType =" + simType + ",mSubId = " + mSubId);
            importAllSimContacts(mContext, mSubId, simCursor, simType);
            if (simCursor != null) {
                simCursor.close();
            }
        } catch (IllegalArgumentException e) {
            Log.e(TAG, "IllegalArgumentException: " + e.getMessage());
        }
    }

    private void importAllSimContacts(Context context, int subId, Cursor simCursor, int simType) {
        Log.d(TAG, "[importAllSimContacts]subId:" + subId + ",sim type:" + simType);
        if (isCancelled()) {
            Log.d(TAG, "[importAllSimContacts]cancel import work,Thread id="
                    + Thread.currentThread().getId());
            return;
        }

        final ContentResolver resolver = context.getContentResolver();
        if (simCursor != null) {
            if (subId > 0) {
                synchronized (this) {
                    importAllSimContacts(context, simCursor, resolver, subId, simType, null, false);
                }
            }
            simCursor.close();

            if (SimCardUtils.isPhoneBookReady(subId)) {
                Cursor sdnCursor = null;
                final Uri iccSdnUri = SubInfoUtils.getIccProviderSdnUri(subId);
                Log.d(TAG, "[importAllSimContacts]iccSdnUri" + iccSdnUri);
                sdnCursor = resolver.query(iccSdnUri, COLUMN_NAMES, null, null, null);
                if (sdnCursor != null) {
                    Log.d(TAG, "[importAllSimContacts]sdnCursor.getCount() = "
                            + sdnCursor.getCount());
                    try {
                        if (sdnCursor.getCount() > 0) {
                            importAllSimContacts(context, sdnCursor, resolver, subId, simType,
                                    null, true);
                        }
                    } catch (Exception e) {
                        Log.e(TAG, "[importAllSimContacts]exception:" + e.toString());
                    } finally {
                        sdnCursor.close();
                    }
                }
            }
        }

        if (isCancelled()) {
            Log.i(TAG, "[ImportAllSimContactsThread] cancel.");
            return;
        }
    }

    private void importAllSimContacts(Context context, final Cursor cursor,
            final ContentResolver resolver, int subId, int simType, HashSet<Long> insertSimIdSet,
            boolean importSdnContacts) {
        Log.d(TAG, "[importAllSimContacts]subId = " + subId + ",simType = " + simType
                + ",importSdnContacts = " + importSdnContacts);
        if (isCancelled()) {
            Log.i(TAG, "[importAllSimContacts]cancel, Thread id="
                    + Thread.currentThread().getId());
            return;
        }

        boolean isUsim = (simType == SimServiceUtils.SIM_TYPE_USIM ||
                simType == SimServiceUtils.SIM_TYPE_CSIM);

        final ArrayList<ContentProviderOperation> operationList =
                new ArrayList<ContentProviderOperation>();

        if (cursor != null) {
            cursor.moveToPosition(-1);
            // Bug Fix ALPS00289127:
            String countryCode = getCurrentCountryIso(context);
            Log.i(TAG, "[importAllSimContacts] countryCode :" + Log.anonymize(countryCode));
            int i = 0;
            while (cursor.moveToNext()) {
                if (isCancelled()) {
                    Log.i(TAG, "[importAllSimContacts]Cancel in while loop, Thread id="
                            + Thread.currentThread().getId());
                    return;
                }
                long indexInSim = cursor.getLong(INDEX_COLUMN); // index in SIM
                // Do nothing if sim contacts is already inserted into contacts
                // DB.
                if (insertSimIdSet != null && insertSimIdSet.contains(indexInSim)) {
                    Log.d(TAG, "[importAllSimContacts]sub id:" + subId + "||indexInSim:"
                            + indexInSim + "||isInserted is true,contine to do next.");
                    continue;
                }

                i = actuallyImportOneSimContact(context, cursor, resolver, subId, simType,
                        indexInSim, importSdnContacts, operationList, i, isUsim, countryCode);

                if (i > MAX_OP_COUNT_IN_ONE_BATCH) {
                    try {
                        // TBD: The deleting and inserting of SIM contacts will
                        // be controled in the same operation queue in the
                        // future.
                        if (!SimServiceUtils.checkPhoneBookState(subId)
                                || isCancelled()) {
                            Log.d(TAG, "[importAllSimContacts]check sim State: false");
                            break;
                        }
                        Log.d(TAG, "[importAllSimContacts]Before applyBatch. ");
                        dbLock.lock();
                        try {
                            resolver.applyBatch(ContactsContract.AUTHORITY, operationList);
                        } finally {
                            dbLock.unlock();
                        }
                        Log.d(TAG, "[importAllSimContacts]After applyBatch ");
                    } catch (RemoteException e) {
                        Log.e(TAG, String.format("%s: %s", e.toString(), e.getMessage()));
                    } catch (OperationApplicationException e) {
                        Log.e(TAG, String.format("%s: %s", e.toString(), e.getMessage()));
                    }
                    i = 0;
                    operationList.clear();
                }
            }

            // fix CR ALPS00754984
            mGroupIdMap.clear();
            if (isCancelled()) {
                Log.d(TAG,
                        "[importAllSimContacts]cancel import work on after while{}. Thread id="
                                + Thread.currentThread().getId());
                return;
            }
            try {
                if (SimServiceUtils.checkPhoneBookState(subId)) {
                    Log.d(TAG, "[importSimContactcheck] sim State: true");
                    if (!operationList.isEmpty() && !isCancelled()) {
                        Log.d(TAG, "[importAllSimContacts]final,Before applyBatch ");
                        dbLock.lock();
                        try {
                            resolver.applyBatch(ContactsContract.AUTHORITY, operationList);
                        } finally {
                            dbLock.unlock();
                        }
                        Log.d(TAG, "[importAllSimContacts]final,After applyBatch ");
                    }
                }
            } catch (RemoteException e) {
                Log.e(TAG, String.format("%s: %s", e.toString(), e.getMessage()));
            } catch (OperationApplicationException e) {
                Log.e(TAG, String.format("%s: %s", e.toString(), e.getMessage()));
            }
        }
    }

    @Override
    protected boolean waitForCancel(int timeout) {
        Log.d(TAG, "[waitForCancel]timeout = " + timeout);
        boolean ret = false;
        try {
            if (dbLock.tryLock(timeout, TimeUnit.SECONDS)) {
                ret = true;
                dbLock.unlock();
            }
        } catch (InterruptedException | NullPointerException e) {
            e.printStackTrace();
        }
        Log.d(TAG, "[waitForCancel]ret = " + ret);
        return ret;
    }

    private int actuallyImportOneSimContact(Context context, final Cursor cursor,
            final ContentResolver resolver, int subId, int simType, long indexInSim,
            boolean importSdnContacts, ArrayList<ContentProviderOperation> operationList,
            int loopCheck, boolean isUsim, String countryCode) {
        int i = loopCheck;
        if (isCancelled()) {
            Log.d(TAG, "[actuallyImportOneSimContact]cancel, Thread id="
                    + Thread.currentThread().getId());
            return i;
        }

        final NamePhoneTypePair namePhoneTypePair = new NamePhoneTypePair(
                cursor.getString(NAME_COLUMN));
        final String name = namePhoneTypePair.name;
        final int phoneType = namePhoneTypePair.phoneType;
        final String phoneTypeSuffix = namePhoneTypePair.phoneTypeSuffix;
        String phoneNumber = cursor.getString(NUMBER_COLUMN);
        Log.d(TAG, "indexInSim = " + indexInSim + ",phoneType = " + phoneType
                + ",phoneTypeSuffix" + phoneTypeSuffix + ",name = " + Log.anonymize(name)
                + ",phoneNumber = " + Log.anonymize(phoneNumber));

        int j = 0;

        ContentProviderOperation.Builder builder = ContentProviderOperation
                .newInsert(RawContacts.CONTENT_URI);
        ContentValues values = new ContentValues();

        String accountName = SimProcessorUtils.getAccountNameUsingSubId(subId);
        String accountType = SimProcessorUtils.getAccountTypeUsingSubId(subId);
        // If accountType is null, return to avoid JE.
        if (accountType == null) {
            Log.w(TAG, "[actuallyImportOneSimContact]can't get simType from TelphonyEx, return!");
            return 0;
        }
        if (!"".equals(accountName) && !"".equals(accountType)) {
            values.put(RawContacts.ACCOUNT_NAME, accountName);
            values.put(RawContacts.ACCOUNT_TYPE, accountType);
        }
        values.put(MtkContactsContract.RawContactsColumns.INDICATE_PHONE_SIM, subId);
        values.put(RawContacts.AGGREGATION_MODE, RawContacts.AGGREGATION_MODE_DISABLED);
        values.put(MtkContactsContract.RawContactsColumns.INDEX_IN_SIM, indexInSim);

        if (importSdnContacts) {
            values.put(MtkContactsContract.RawContactsColumns.IS_SDN_CONTACT, 1);
        }

        builder.withValues(values);
        operationList.add(builder.build());
        j++;

        if (!TextUtils.isEmpty(phoneNumber)) {
            Log.d(TAG, "[actuallyImportOneSimContact] phoneNumber before : " + Log.anonymize(
                    phoneNumber));
            AsYouTypeFormatter mFormatter = PhoneNumberUtil.getInstance().getAsYouTypeFormatter(
                    countryCode);
            char[] cha = phoneNumber.toCharArray();
            int ii = cha.length;
            for (int num = 0; num < ii; num++) {
                phoneNumber = mFormatter.inputDigit(cha[num]);
            }
            Log.d(TAG, "[actuallyImportOneSimContact] phoneNumber after : " + Log.anonymize(
                    phoneNumber));

            builder = ContentProviderOperation.newInsert(Data.CONTENT_URI);
            builder.withValueBackReference(Phone.RAW_CONTACT_ID, i);
            builder.withValue(Data.MIMETYPE, Phone.CONTENT_ITEM_TYPE);
            builder.withValue(Data.DATA2, Aas.PHONE_TYPE_EMPTY);

            builder.withValue(Phone.NUMBER, phoneNumber);
            if (!TextUtils.isEmpty(phoneTypeSuffix)) {
                builder.withValue(Data.DATA15, phoneTypeSuffix);
            }
            operationList.add(builder.build());
            j++;
        }

        if (!TextUtils.isEmpty(name)) {
            builder = ContentProviderOperation.newInsert(Data.CONTENT_URI);
            builder.withValueBackReference(StructuredName.RAW_CONTACT_ID, i);
            builder.withValue(Data.MIMETYPE, StructuredName.CONTENT_ITEM_TYPE);
            builder.withValue(StructuredName.DISPLAY_NAME, name);
            operationList.add(builder.build());
            j++;
        }

        // if USIM
        if (isUsim) {
            j = importUSimPart(cursor, operationList, i, j, countryCode, accountType, subId);
        }
        i = i + j;
        return i;
    }

    private int importUSimPart(final Cursor cursor,
            ArrayList<ContentProviderOperation> operationList, int loopCheck, int loop,
            String countryCode, String accountType, int subId) {
        int i = loopCheck;
        int j = loop;
        ContentProviderOperation.Builder builder = ContentProviderOperation
                .newInsert(Data.CONTENT_URI);

        // insert USIM email
        final String emailAddresses = cursor.getString(EMAIL_COLUMN);
        if (!TextUtils.isEmpty(emailAddresses)) {
            final String[] emailAddressArray;
            emailAddressArray = emailAddresses.split(",");
            for (String emailAddress : emailAddressArray) {
                Log.d(TAG, "[actuallyImportOneSimContact]emailAddress IS " + Log.anonymize(
                        emailAddress));
                if (!TextUtils.isEmpty(emailAddress) && !emailAddress.equals("null")) {
                    builder.withValueBackReference(Email.RAW_CONTACT_ID, i);
                    builder.withValue(Data.MIMETYPE, Email.CONTENT_ITEM_TYPE);
                    // don't show email type for SIM contact.
                    //builder.withValue(Email.TYPE, Email.TYPE_MOBILE);
                    builder.withValue(Email.DATA, emailAddress);
                    operationList.add(builder.build());
                    j++;
                }
            }
        }

        // insert USIM additional number
        String additionalNumber = cursor.getString(ADDITIONAL_NUMBER_COLUMN);
        if (!TextUtils.isEmpty(additionalNumber)) {
            Log.i(TAG, "[importUSimPart] additionalNumber before : " + Log.anonymize(
                    additionalNumber));
            AsYouTypeFormatter mFormatter = PhoneNumberUtil.getInstance().getAsYouTypeFormatter(
                    countryCode);
            char[] cha = additionalNumber.toCharArray();
            int ii = cha.length;
            for (int num = 0; num < ii; num++) {
                additionalNumber = mFormatter.inputDigit(cha[num]);
            }
            Log.i(TAG, "[importUSimPart] additionalNumber after : " + Log.anonymize(
                    additionalNumber));
            builder = ContentProviderOperation.newInsert(Data.CONTENT_URI);
            builder.withValueBackReference(Phone.RAW_CONTACT_ID, i);
            builder.withValue(Data.MIMETYPE, Phone.CONTENT_ITEM_TYPE);
            // builder.withValue(Phone.TYPE, phoneType);

            builder.withValue(Data.DATA2, 7);
            // AAS primary number doesn't have type. [COMMD_FOR_AAS]
            int aasColumn = cursor.getColumnIndex("aas");
            if (aasColumn >= 0) {
                String aas = cursor.getString(aasColumn);
                Log.d(TAG, "[checkAasOperationBuilder] subId: " + subId + " aas " + aas);
                builder.withValue(Data.DATA2, Aas.PHONE_TYPE_AAS);
                if (!"0".equals(aas)) {//when aas=0 means has no aas label.not write data3.
                    builder.withValue(Data.DATA3, Aas.buildIndicator(subId, Integer.valueOf(aas)));
                }
            }
            builder.withValue(Phone.NUMBER, additionalNumber);
            builder.withValue(MtkContactsContract.DataColumns.IS_ADDITIONAL_NUMBER, 1);
            operationList.add(builder.build());
            j++;
        }

        j += SimProcessorUtils.importSimSne(operationList, cursor, i);

        // USIM group
        final String ugrpStr = cursor.getString(GROUP_COLUMN);
        if (!TextUtils.isEmpty(ugrpStr)) {
            String[] ugrpIdArray = null;
            if (!TextUtils.isEmpty(ugrpStr)) {
                ugrpIdArray = ugrpStr.split(",");
            }
            for (String ugrpIdStr : ugrpIdArray) {
                int ugrpId = -1;
                try {
                    if (!TextUtils.isEmpty(ugrpIdStr)) {
                        ugrpId = Integer.parseInt(ugrpIdStr);
                    }
                } catch (Exception e) {
                    Log.d(TAG, "[importUSimPart] catched exception");
                    e.printStackTrace();
                    continue;
                }
                if (ugrpId > 0) {
                    // / M: fix CR ALPS00754984
                    Integer grpId = mGroupIdMap.get(ugrpId);
                    if (grpId == null) {
                        continue;
                    }
                    builder = ContentProviderOperation.newInsert(Data.CONTENT_URI);
                    builder.withValue(Data.MIMETYPE, GroupMembership.CONTENT_ITEM_TYPE);
                    builder.withValue(GroupMembership.GROUP_ROW_ID, grpId);
                    builder.withValueBackReference(Phone.RAW_CONTACT_ID, i);
                    operationList.add(builder.build());
                    j++;
                }
            }
        }
        Log.d(TAG, "[importUSimPart]import a USIM contact. EmailAddresses:"
                + Log.anonymize(emailAddresses) + "| AdditionalNumber:"
                + Log.anonymize(additionalNumber) + "| GroupId string: " + ugrpStr);
        return j;
    }

    private boolean checkPhbStateReady() {
        boolean phbStateReady = SimServiceUtils.checkPhoneBookState(mSubId);
        int i = 10;
        while (i > 0) {
            if (!phbStateReady) {
                try {
                    Thread.sleep(1000);
                } catch (Exception e) {
                    Log.w(TAG, "[checkPhbStateReady]excepiotn:" + e.toString());
                }
                phbStateReady = SimServiceUtils.checkPhoneBookState(mSubId);
            } else {
                break;
            }
            i--;
        }
        return phbStateReady;
    }

    private Cursor querySimContact(Context context, int subId, int simType, Uri iccUri) {
        Log.d(TAG, "[querySimContact]subId:" + subId + "|simType:" + simType + ",iccUri = "
                + iccUri);
        if (isCancelled()) {
            Log.d(TAG, "[querySimContact]canceled,return.");
            return null;
        }

        Cursor cursor = null;
        try {
            cursor = context.getContentResolver().query(iccUri, COLUMN_NAMES, null, null, null);
        } catch (java.lang.NullPointerException e) {
            Log.e(TAG, "[querySimContact]exception:" + e.toString());
            return null;
        }
        if (cursor != null) {
            int count = cursor.getCount();
            Log.d(TAG, "[querySimContact]count:" + count);
        }

        if (simType == SimServiceUtils.SIM_TYPE_USIM) {
            mGroupIdMap.clear();
            ServiceWorkData workData = new ServiceWorkData(subId, simType, cursor);
            USIMGroup.syncUSIMGroupContactsGroup(context, workData, mGroupIdMap);
        } else {
            USIMGroup.deleteUSIMGroupOnPhone(context, subId);
        }
        return cursor;
    }

    private String getCurrentCountryIso(Context context) {
        CountryDetector countryDetector = (CountryDetector) context
                .getSystemService(Context.COUNTRY_DETECTOR);
        Country country = countryDetector.detectCountry();

        if (country == null) {
            // Fallback to Locale if there are issues with CountryDetector
            return Locale.getDefault().getCountry();
        }

        return country.getCountryIso();
    }
}
