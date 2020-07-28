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
package com.mediatek.contacts.list.service;

import android.content.ContentProviderOperation;
import android.content.ContentProviderOperation.Builder;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.ContentValues;
import android.database.Cursor;
import android.net.Uri;
import android.os.PowerManager;
import android.provider.ContactsContract;
import android.provider.ContactsContract.CommonDataKinds.Email;
import android.provider.ContactsContract.CommonDataKinds.GroupMembership;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.ContactsContract.CommonDataKinds.StructuredName;
import android.provider.ContactsContract.Contacts;
import android.provider.ContactsContract.Data;
import android.provider.ContactsContract.RawContacts;
import android.telephony.PhoneNumberUtils;
import android.text.TextUtils;

import com.android.contacts.model.account.AccountWithDataSet;
import com.android.contacts.vcard.ProcessorBase;
import com.google.common.collect.Lists;

import com.mediatek.contacts.GlobalEnv;
import com.mediatek.contacts.SubContactsUtils;
import com.mediatek.contacts.model.account.AccountWithDataSetEx;
import com.mediatek.contacts.simcontact.PhbInfoUtils;
import com.mediatek.contacts.simcontact.SimCardUtils;
import com.mediatek.contacts.simcontact.SubInfoUtils;
import com.mediatek.contacts.simservice.SimServiceUtils;
import com.mediatek.contacts.util.AccountTypeUtils;
import com.mediatek.contacts.util.ContactsPortableUtils;
import com.mediatek.contacts.util.ErrorCause;
import com.mediatek.contacts.util.Log;
import com.mediatek.contacts.util.TimingStatistics;
import com.mediatek.provider.MtkContactsContract;

import java.util.ArrayList;
import java.util.List;

public class CopyProcessor extends ProcessorBase {
    private static final String TAG = "CopyProcessor";

    private final MultiChoiceService mService;
    private final ContentResolver mResolver;
    private final List<MultiChoiceRequest> mRequests;
    private final int mJobId;
    private final MultiChoiceHandlerListener mListener;

    private PowerManager.WakeLock mWakeLock;

    private final AccountWithDataSet mAccountSrc;
    private final AccountWithDataSet mAccountDst;

    private volatile boolean mIsCanceled;
    private volatile boolean mIsDone;
    private volatile boolean mIsRunning;

    private static final int MAX_OP_COUNT_IN_ONE_BATCH = 400;
    private static final int RETRYCOUNT = 20;

    private static final String[] DATA_ALLCOLUMNS_INTERNAL = new String[] { Data._ID, Data.MIMETYPE,
            Data.IS_PRIMARY, Data.IS_SUPER_PRIMARY, Data.DATA1, Data.DATA2, Data.DATA3, Data.DATA4,
            Data.DATA5, Data.DATA6, Data.DATA7, Data.DATA8, Data.DATA9, Data.DATA10, Data.DATA11,
            Data.DATA12, Data.DATA13, Data.DATA14, Data.DATA15, Data.SYNC1, Data.SYNC2, Data.SYNC3,
            Data.SYNC4};
    private static final String[] DATA_ALLCOLUMNS;

    static {
        List<String> projectionList = Lists.newArrayList(DATA_ALLCOLUMNS_INTERNAL);
        if (ContactsPortableUtils.MTK_PHONE_BOOK_SUPPORT) {
            /// M: Add some columns for Contacts extensions. @{
            projectionList.add(MtkContactsContract.DataColumns.IS_ADDITIONAL_NUMBER);
            /// @}
        }

        DATA_ALLCOLUMNS = projectionList.toArray(new String[projectionList.size()]);
    }

    public CopyProcessor(final MultiChoiceService service,
            final MultiChoiceHandlerListener listener, final List<MultiChoiceRequest> requests,
            final int jobId, final AccountWithDataSet sourceAccount,
            final AccountWithDataSet destinationAccount) {
        Log.i(TAG, "[CopyProcessor]new.");
        mService = service;
        mResolver = mService.getContentResolver();
        mListener = listener;

        mRequests = requests;
        mJobId = jobId;
        mAccountSrc = sourceAccount;
        mAccountDst = destinationAccount;

        final PowerManager powerManager = (PowerManager) mService.getApplicationContext()
                .getSystemService("power");
        mWakeLock = powerManager.newWakeLock(PowerManager.SCREEN_DIM_WAKE_LOCK
                | PowerManager.ON_AFTER_RELEASE, TAG);
    }

    @Override
    public synchronized boolean cancel(boolean mayInterruptIfRunning) {
        Log.i(TAG, "[cancel]mIsDone=" + mIsDone + ",mCanceled=" + mIsCanceled
                + ",mIsRunning = " + mIsRunning);

        if (mIsDone || mIsCanceled) {
            return false;
        }
        mIsCanceled = true;
        if (!mIsRunning) {
            mService.handleFinishNotification(mJobId, false);
            mListener.onCanceled(MultiChoiceService.TYPE_COPY, mJobId, -1, -1, -1);
        }
        return true;
    }

    @Override
    public int getType() {
        return MultiChoiceService.TYPE_COPY;
    }

    @Override
    public synchronized boolean isCancelled() {
        return mIsCanceled;
    }

    @Override
    public synchronized boolean isDone() {
        return mIsDone;
    }

    @Override
    public void run() {
        Log.i(TAG, "[run]");
        try {
            mIsRunning = true;
            mWakeLock.acquire();
            if (AccountTypeUtils.isAccountTypeIccCard(mAccountDst.type)) {
                copyContactsToSimWithRadioStateCheck();
            } else {
                copyContactsToAccount();
            }
        } finally {
            synchronized (this) {
                mIsDone = true;
            }
            if (mWakeLock != null && mWakeLock.isHeld()) {
                mWakeLock.release();
            }
        }
    }

    private void copyContactsToSimWithRadioStateCheck() {
        if (mIsCanceled) {
            Log.w(TAG, "[copyContactsToSimWithRadioCheck]mIsCanceled is true,return.");
            return;
        }

        int errorCause = ErrorCause.NO_ERROR;

        AccountWithDataSetEx account = (AccountWithDataSetEx) mAccountDst;
        Log.d(TAG, "[copyContactsToSimWithRadioCheck]AccountName: " + Log.anonymize(account.name)
                + " | accountType: " + account.type);
        int dstSubId = account.getSubId();

        if (!isPhoneBookReady(dstSubId)) {
            int i = 0;
            while (i++ < RETRYCOUNT) {
                try {
                    Thread.sleep(500);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }

                if (isPhoneBookReady(dstSubId)) {
                    break;
                }
            }
        }
        if (!isPhoneBookReady(dstSubId)) {
            errorCause = ErrorCause.SIM_NOT_READY;
            mService.handleFinishNotification(mJobId, false);
            mListener.onFailed(MultiChoiceService.TYPE_COPY, mJobId, mRequests.size(), 0,
                    mRequests.size(), errorCause);
            return;
        }
        copyContactsToSim();
    }

    private void copyContactsToAccount() {
        Log.d(TAG, "[copyContactsToAccount]mIsCanceled = " + mIsCanceled);
        if (mIsCanceled) {
            return;
        }
        int successfulItems = 0;
        int currentCount = 0;

        StringBuilder sb = new StringBuilder();
        sb.append("(");
        for (MultiChoiceRequest request : this.mRequests) {
            sb.append(String.valueOf(request.mContactId));
            sb.append(",");
        }
        sb.deleteCharAt(sb.length() - 1);
        sb.append(")");
        Log.d(TAG, "[copyContactsToAccount]contactIds " + sb.toString() + " ");
        TimingStatistics contactsProviderTiming = new TimingStatistics(
                CopyProcessor.class.getSimpleName());
        contactsProviderTiming.timingStart();
        Cursor rawContactsCursor = mResolver.query(RawContacts.CONTENT_URI, new String[] {
                RawContacts._ID, RawContacts.DISPLAY_NAME_PRIMARY }, RawContacts.CONTACT_ID
                + " IN " + sb.toString(), null, null);

        contactsProviderTiming.timingEnd();
        int totalItems = rawContactsCursor == null ? 0 : rawContactsCursor.getCount();

        final ArrayList<ContentProviderOperation> operationList =
                new ArrayList<ContentProviderOperation>();
        // Process request one by one
        if (rawContactsCursor != null) {
            Log.d(TAG,
                    "[copyContactsToAccount]rawContactsCursor.size = "
                            + rawContactsCursor.getCount());

            long nOldRawContactId;
            while (rawContactsCursor.moveToNext()) {
                if (mIsCanceled) {
                    Log.d(TAG, "[copyContactsToAccount]runInternal run: mCanceled = true");
                    break;
                }
                currentCount++;
                String displayName = rawContactsCursor.getString(1);

                mListener.onProcessed(MultiChoiceService.TYPE_COPY, mJobId, currentCount,
                        totalItems, displayName);

                nOldRawContactId = rawContactsCursor.getLong(0);

                Cursor dataCursor = mResolver.query(Data.CONTENT_URI, DATA_ALLCOLUMNS,
                        Data.RAW_CONTACT_ID + "=? ",
                        new String[] { String.valueOf(nOldRawContactId) }, null);
                if (dataCursor == null) {
                    continue;
                } else if (dataCursor.getCount() <= 0) {
                    Log.d(TAG, "[copyContactsToAccount]dataCursor is empty");
                    dataCursor.close();
                    continue;
                }

                int backRef = operationList.size();
                ContentProviderOperation.Builder builder = ContentProviderOperation
                        .newInsert(RawContacts.CONTENT_URI);
                if (!TextUtils.isEmpty(mAccountDst.name) && !TextUtils.isEmpty(mAccountDst.type)) {
                    builder.withValue(RawContacts.ACCOUNT_NAME, mAccountDst.name);
                    builder.withValue(RawContacts.ACCOUNT_TYPE, mAccountDst.type);
                } else {
                    builder.withValues(new ContentValues());
                }
                builder.withValue(RawContacts.AGGREGATION_MODE,
                        RawContacts.AGGREGATION_MODE_DISABLED);
                operationList.add(builder.build());

                dataCursor.moveToPosition(-1);
                String[] columnNames = dataCursor.getColumnNames();
                while (dataCursor.moveToNext()) {
                    // do not copy group data between different account.
                    String mimeType = dataCursor
                            .getString(dataCursor.getColumnIndex(Data.MIMETYPE));
                    Log.i(TAG, "mimeType:" + mimeType);
                    if (GroupMembership.CONTENT_ITEM_TYPE.equals(mimeType)) {
                        continue;
                    }
                    builder = ContentProviderOperation.newInsert(Data.CONTENT_URI);
                    generateDataBuilder(dataCursor, builder, columnNames, mimeType,
                            mAccountSrc.type);
                    builder.withValueBackReference(Data.RAW_CONTACT_ID, backRef);
                    operationList.add(builder.build());
                }
                dataCursor.close();
                successfulItems++;
                if (operationList.size() > MAX_OP_COUNT_IN_ONE_BATCH) {
                    try {
                        Log.d(TAG, "[copyContactsToAccount]Before applyBatch. ");
                        contactsProviderTiming.timingStart();
                        mResolver.applyBatch(ContactsContract.AUTHORITY, operationList);
                        contactsProviderTiming.timingEnd();
                        Log.d(TAG, "[copyContactsToAccount]After applyBatch ");
                    } catch (android.os.RemoteException e) {
                        Log.e(TAG, String.format("%s: %s", e.toString(), e.getMessage()));
                    } catch (android.content.OperationApplicationException e) {
                        Log.e(TAG, String.format("%s: %s", e.toString(), e.getMessage()));
                    }
                    operationList.clear();
                }
            }
            rawContactsCursor.close();
            if (operationList.size() > 0) {
                try {
                    Log.d(TAG, "[copyContactsToAccount]Before end applyBatch. ");
                    contactsProviderTiming.timingStart();
                    mResolver.applyBatch(ContactsContract.AUTHORITY, operationList);
                    contactsProviderTiming.timingEnd();
                    Log.d(TAG, "[copyContactsToAccount]After end applyBatch ");
                } catch (android.os.RemoteException e) {
                    Log.e(TAG, String.format("%s: %s", e.toString(), e.getMessage()));
                } catch (android.content.OperationApplicationException e) {
                    Log.e(TAG, String.format("%s: %s", e.toString(), e.getMessage()));
                }
                operationList.clear();
            }
            if (mIsCanceled) {
                Log.d(TAG, "[copyContactsToAccount]runInternal run: mCanceled = true");
                mService.handleFinishNotification(mJobId, false);
                mListener.onCanceled(MultiChoiceService.TYPE_COPY, mJobId, totalItems,
                        successfulItems, totalItems - successfulItems);
                if (rawContactsCursor != null && !rawContactsCursor.isClosed()) {
                    rawContactsCursor.close();
                }
                return;
            }
        }

        mService.handleFinishNotification(mJobId, successfulItems == totalItems);
        if (successfulItems == totalItems) {
            mListener.onFinished(MultiChoiceService.TYPE_COPY, mJobId, totalItems);
        } else {
            mListener.onFailed(MultiChoiceService.TYPE_COPY, mJobId, totalItems, successfulItems,
                    totalItems - successfulItems);
        }

        Log.d(TAG, "[copyContactsToAccount]end");
        contactsProviderTiming.log("copyContactsToAccount():ContactsProviderTiming");
    }

    private void copyContactsToSim() {
        int errorCause = ErrorCause.NO_ERROR;

        // Process sim data, sim id or slot
        AccountWithDataSetEx account = (AccountWithDataSetEx) mAccountDst;
        int dstSubId = account.getSubId();
        Log.d(TAG, "[copyContactsToSim] AccountName:" + Log.anonymize(account.name)
                + ",accountType:" + account.type +",dstSubId:" + dstSubId);
        // The UIM and SIM type has same property, CSIM and USIM also same.
        // So in this process, we set there has only two type 'SIM' and 'USIM'
        // Notice that, the type is not account type.
        // the type is only use for import Email/Group info or not.

        boolean isTargetUsim = SimCardUtils.isUsimOrCsimType(dstSubId);
        String dstSimType = isTargetUsim
                ? SimCardUtils.SimType.SIM_TYPE_USIM_TAG
                : SimCardUtils.SimType.SIM_TYPE_SIM_TAG;

        Log.d(TAG, "[copyContactsToSim]dstSimType:" + dstSimType);

        if (!isPhoneBookReady(dstSubId)) {
            errorCause = ErrorCause.SIM_NOT_READY;
            mService.handleFinishNotification(mJobId, false);
            mListener.onFailed(MultiChoiceService.TYPE_COPY, mJobId, mRequests.size(), 0,
                    mRequests.size(), errorCause);
            return;
        }

        /// M:[ALPS03630305] The phbInfo maybe not ready while import to sim card,
        /// so sleep for 100ms to wait refreshing and check PhbInfo again. @{
        if (isTargetUsim && !PhbInfoUtils.isInitialized(dstSubId)) {
            if (PhbInfoUtils.getUsimEmailCount(dstSubId) <= 0 ||
                    PhbInfoUtils.getUsimAnrCount(dstSubId) <= 0) {
                Log.w(TAG, "[copyContactsToSim]PhbInfo not ready so wait and try again:" +
                        PhbInfoUtils.isInitialized(dstSubId));
                try {
                    Thread.sleep(100);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                if (!PhbInfoUtils.isInitialized(dstSubId) &&
                    (PhbInfoUtils.getUsimEmailCount(dstSubId) <= 0 ||
                     PhbInfoUtils.getUsimAnrCount(dstSubId) <= 0)) {
                    errorCause = ErrorCause.SIM_NOT_READY;
                    mService.handleFinishNotification(mJobId, false);
                    mListener.onFailed(MultiChoiceService.TYPE_COPY, mJobId, mRequests.size(), 0,
                            mRequests.size(), errorCause);
                    Log.w(TAG, "[copyContactsToSim]Still fail after try again. init=" +
                            PhbInfoUtils.isInitialized(dstSubId));
                    return;
                }
            }
        }
        /// @}

        ArrayList<String> numberArray = new ArrayList<String>();
        ArrayList<String> additionalNumberArray = new ArrayList<String>();
        ArrayList<String> emailArray = new ArrayList<String>();

        String targetName = null;

        ContentResolver resolver = this.mResolver;

        // Process request one by one
        int totalItems = mRequests.size();
        int successfulItems = 0;
        int currentCount = 0;
        int iccCardMaxEmailCount = PhbInfoUtils.getUsimEmailCount(dstSubId);

        boolean isSimStorageFull = false;
        final ArrayList<ContentProviderOperation> operationList =
                new ArrayList<ContentProviderOperation>();
        TimingStatistics iccProviderTiming = new TimingStatistics(
                CopyProcessor.class.getSimpleName());
        TimingStatistics contactsProviderTiming = new TimingStatistics(
                CopyProcessor.class.getSimpleName());
        TimingStatistics checkStatusTiming = new TimingStatistics(
                CopyProcessor.class.getSimpleName());
        for (MultiChoiceRequest request : this.mRequests) {
            if (mIsCanceled) {
                Log.d(TAG, "[copyContactsToSim]mIsCanceled is true.");
                break;
            }
            /** M: Bug Fix for ALPS00695093 @{ */
            checkStatusTiming.timingStart();
            if (!isPhoneBookReady(dstSubId) || SimServiceUtils.isServiceRunning(
                            mService.getApplicationContext(), dstSubId)) {
                Log.d(TAG, "[copyContactsToSim] run: sim not ready");
                errorCause = ErrorCause.ERROR_UNKNOWN;
                operationList.clear();
                break;
            }
            checkStatusTiming.timingEnd();

            currentCount++;
            // Notify the copy process on notification bar
            mListener.onProcessed(MultiChoiceService.TYPE_COPY, mJobId, currentCount, totalItems,
                    request.mContactName);

            // reset data
            numberArray.clear();
            additionalNumberArray.clear();

            emailArray.clear();
            targetName = null;

            int contactId = request.mContactId;

            // Query to get all src data resource.
            Uri dataUri = Uri.withAppendedPath(
                    ContentUris.withAppendedId(Contacts.CONTENT_URI, contactId),
                    Contacts.Data.CONTENT_DIRECTORY);
            final String[] projection = new String[] { Contacts._ID, Contacts.Data.MIMETYPE,
                    Contacts.Data.DATA1, MtkContactsContract.DataColumns.IS_ADDITIONAL_NUMBER,
                    Contacts.NAME_RAW_CONTACT_ID, Contacts.Data.RAW_CONTACT_ID };

            contactsProviderTiming.timingStart();
            Cursor c = resolver.query(dataUri, projection, null, null, null);
            contactsProviderTiming.timingEnd();

            if (c != null && c.moveToFirst()) {
                do {
                    String mimeType = c.getString(1);
                    if (Phone.CONTENT_ITEM_TYPE.equals(mimeType)) {
                        // For phone number
                        String number = c.getString(2);
                        int isAdditionalNumber = c.getInt(3);
                        if (isAdditionalNumber == 1) {
                            additionalNumberArray.add(number);
                        } else {
                            numberArray.add(number);
                        }
                    } else if (StructuredName.CONTENT_ITEM_TYPE.equals(mimeType)
                            && c.getInt(c.getColumnIndexOrThrow(Contacts.NAME_RAW_CONTACT_ID))
                            == c.getInt(c.getColumnIndexOrThrow(Contacts.Data.RAW_CONTACT_ID))) {
                        // For name
                        targetName = c.getString(2);
                    }
                    if (isTargetUsim) {
                        // /M:Bug Fix for ALPS00566570,some USIM Card do not
                        // support storing Email address.
                        if (Email.CONTENT_ITEM_TYPE.equals(mimeType) && iccCardMaxEmailCount > 0) {
                            // For email
                            String email = c.getString(2);
                            emailArray.add(email);
                        }
                    }
                } while (c.moveToNext());
            }
            if (c != null) {
                c.close();
            }

            // copy new resournce to target sim or usim,
            // and insert into database if sucessful
            Uri dstSimUri = SubInfoUtils.getIccProviderUri(dstSubId);
            int maxCount = TextUtils.isEmpty(targetName) ? 0 : 1;
            /** M: Bug Fix for ALPS00557517 @{ */
            int maxAnrCount = PhbInfoUtils.getUsimAnrCount(dstSubId);
            int usimMaxAnrCount = maxAnrCount;
            /** @ } */
            if (isTargetUsim) {
                int numberCount = numberArray.size();
                int additionalCount = additionalNumberArray.size();
                int emailCount = emailArray.size();

                maxCount = (maxCount > additionalCount) ? maxCount : additionalCount;
                maxCount = (maxCount > emailCount) ? maxCount : emailCount;
                int numberQuota = (int) ((numberCount + additionalCount) / (1.0 + maxAnrCount)
                        + (float) maxAnrCount / (1.0 + maxAnrCount));
                Log.d(TAG, "[copyContactsToSim]maxAnr=" + maxAnrCount + "; numberQuota="
                        + numberQuota + ",additionalCount:" + additionalCount);

                maxCount = maxCount > numberQuota ? maxCount : numberQuota;
            } else {
                numberArray.addAll(additionalNumberArray);
                additionalNumberArray.clear();
                int numberCount = numberArray.size();
                maxCount = maxCount > numberCount ? maxCount : numberCount;
            }
            int sameNameCount = 0;
            ContentValues values = new ContentValues();
            String simTag = null;
            String simNum = null;
            String simAnrNum = null;
            String simEmail = null;

            simTag = sameNameCount > 0 ? (targetName + sameNameCount) : targetName;
            simTag = TextUtils.isEmpty(simTag) ? "" : simTag;
            if ((simTag == null || simTag.isEmpty() || simTag.length() == 0)
                    && numberArray.isEmpty()) {
                Log.e(TAG, " [copyContactsToSim]name and number are empty");
                errorCause = ErrorCause.ERROR_UNKNOWN;
                continue;
            }

            int subContact = 0;
            for (int i = 0; i < maxCount; i++) {
                values.clear();
                values.put("tag", simTag);
                Log.d(TAG, "[copyContactsToSim] simTag: " + Log.anonymize(simTag));
                simNum = null;
                simAnrNum = null;
                simEmail = null;
                if (!numberArray.isEmpty()) {
                    simNum = numberArray.remove(0);
                    simNum = TextUtils.isEmpty(simNum) ? "" : simNum.replace("-", "");
                    values.put("number", PhoneNumberUtils.stripSeparators(simNum));
                    Log.d(TAG, "[copyContactsToSim] number is " + Log.anonymize(simNum));
                }

                if (isTargetUsim) {
                    Log.d(TAG, "[copyContactsToSim] copy to USIM");
                    if (!additionalNumberArray.isEmpty()) {
                        /**
                         * M: Bug Fix for ALPS00557517 origin code:
                         * LogUtils.d(LOG_TAG,
                         * "additional number array is not empty"); simAnrNum =
                         * additionalNumberArray.remove(0); simAnrNum =
                         * TextUtils.isEmpty(simAnrNum) ? "" :
                         * simAnrNum.replace("-",""); values.put("anr",
                         * PhoneNumberUtils.stripSeparators(simAnrNum));
                         * LogUtils.d(LOG_TAG, "copyContactsToSim anr is " +
                         * simAnrNum); @ {
                         */
                        int loop = additionalNumberArray.size() < usimMaxAnrCount ?
                                additionalNumberArray.size() : usimMaxAnrCount;
                        for (int j = 0; j < loop; j++) {
                            simAnrNum = additionalNumberArray.remove(0);
                            simAnrNum = TextUtils.isEmpty(simAnrNum) ? "" : simAnrNum.replace("-",
                                    "");
                            values.put("anr", PhoneNumberUtils.stripSeparators(simAnrNum));
                        }
                        if (!additionalNumberArray.isEmpty()) {
                            numberArray.addAll(additionalNumberArray);
                            additionalNumberArray.clear();
                        }
                        /** @ } */
                    } else if (!numberArray.isEmpty()) {
                        /**
                         * M: Bug Fix for ALPS00557517 origin code:
                         * LogUtils.d(LOG_TAG,
                         * "additional number array is empty and fill it with ADN number"
                         * ); simAnrNum = numberArray.remove(0); simAnrNum =
                         * TextUtils.isEmpty(simAnrNum) ? "" :
                         * simAnrNum.replace("-", ""); values.put("anr",
                         * PhoneNumberUtils.stripSeparators(simAnrNum));
                         * LogUtils.d(LOG_TAG, "copyContactsToSim anr is " +
                         * simAnrNum); @ {
                         */
                        int loop = numberArray.size() < usimMaxAnrCount ? numberArray.size()
                                : usimMaxAnrCount;
                        for (int k = 0; k < loop; k++) {
                            simAnrNum = numberArray.remove(0);
                            simAnrNum = TextUtils.isEmpty(simAnrNum) ? "" : simAnrNum.replace("-",
                                    "");
                            values.put("anr", PhoneNumberUtils.stripSeparators(simAnrNum));
                        }
                        /** @ } */
                    }

                    if (!emailArray.isEmpty()) {
                        simEmail = emailArray.remove(0);
                        simEmail = TextUtils.isEmpty(simEmail) ? "" : simEmail;
                        values.put("emails", simEmail);
                        Log.d(TAG, "[copyContactsToSim] emails is " + Log.anonymize(simEmail));
                    }
                    GlobalEnv.getSimAasEditor().updateValuesforCopy(dataUri,
                            dstSubId, mAccountDst.type, values);
                    GlobalEnv.getSimSneEditor().updateValuesforCopy(dataUri,
                            dstSubId, mAccountDst.type, values);

                }

                /** M: Bug Fix for ALPS00695093 @{ */
                // / M: change for SIM Service Refactoring
                if (!isPhoneBookReady(dstSubId) || SimServiceUtils.isServiceRunning(
                                mService.getApplicationContext(), dstSubId)) {
                    break;
                }
                Log.sensitive(TAG, "[copyContactsToSim]Before insert Sim card. values=" + values);
                iccProviderTiming.timingStart();
                Uri retUri = resolver.insert(dstSimUri, values);
                iccProviderTiming.timingEnd();
                Log.i(TAG, "[copyContactsToSim]After insert Sim card,retUri = " + retUri);

                if (retUri != null) {
                    List<String> checkUriPathSegs = retUri.getPathSegments();
                    if ("error".equals(checkUriPathSegs.get(0))) {
                        String errorCode = checkUriPathSegs.get(1);
                        Log.i(TAG, "[copyContactsToSim]error code = " + errorCode);
                        printSimErrorDetails(errorCode);
                        if (errorCause != ErrorCause.ERROR_USIM_EMAIL_LOST) {
                            errorCause = ErrorCause.ERROR_UNKNOWN;
                        }
                        if ("-3".equals(checkUriPathSegs.get(1))) {
                            errorCause = ErrorCause.SIM_STORAGE_FULL;
                            isSimStorageFull = true;
                            Log.e(TAG, "[copyContactsToSim]Fail to insert sim contacts fail"
                                    + " because sim storage is full.");
                            break;
                        } else if ("-12".equals(checkUriPathSegs.get(1))) {
                            errorCause = ErrorCause.ERROR_USIM_EMAIL_LOST;
                            Log.e(TAG, "[copyContactsToSim]Fail to save USIM email "
                                    + " because emial slot is full in USIM.");
                            Log.d(TAG, "[copyContactsToSim]Ignore this error and "
                                    + "remove the email address to save this item again");
                            values.remove("emails");
                            iccProviderTiming.timingStart();
                            retUri = resolver.insert(dstSimUri, values);
                            iccProviderTiming.timingEnd();
                            Log.d(TAG, "[copyContactsToSim]The retUri is " + retUri);
                            if (retUri != null && ("error".
                                    equals(retUri.getPathSegments().get(0)))) {
                                if ("-3".equals(retUri.getPathSegments().get(1))) {
                                    errorCause = ErrorCause.SIM_STORAGE_FULL;
                                    isSimStorageFull = true;
                                    Log.e(TAG,
                                            "[copyContactsToSim]Fail to insert sim contacts fail"
                                                    + " because sim storage is full.");
                                    break;
                                }
                            }
                            if (retUri != null
                                    && !("error".equals(retUri.getPathSegments().get(0)))) {
                                long indexInSim = ContentUris.parseId(retUri);
                                int backRef = operationList.size();

                                SubContactsUtils.buildInsertOperation(operationList, mAccountDst,
                                        simTag, simNum, null, simAnrNum, resolver, dstSubId,
                                        dstSimType, indexInSim, null);
                                GlobalEnv.getSimSneEditor().copySimSneToAccount(operationList,
                                        mAccountDst, dataUri,backRef);
                                subContact++;
                            }
                        }
                    } else {
                        Log.d(TAG, "[copyContactsToSim]insertUsimFlag = true");
                        long indexInSim = ContentUris.parseId(retUri);

                        int backRef = operationList.size();

                        SubContactsUtils.buildInsertOperation(operationList, mAccountDst, simTag,
                                simNum, simEmail, simAnrNum, resolver, dstSubId, dstSimType,
                                indexInSim, null);
                        GlobalEnv.getSimSneEditor().copySimSneToAccount(operationList,
                                mAccountDst, dataUri, backRef);
                        subContact++;
                        // successfulItems++;
                    }
                } else {
                    errorCause = ErrorCause.ERROR_UNKNOWN;
                }
                if (operationList.size() > MAX_OP_COUNT_IN_ONE_BATCH) {
                    try {
                        Log.i(TAG, "[copyContactsToSim]Before applyBatch. ");
                        /** M: Bug Fix for ALPS00695093 @{ */
                        if (isPhoneBookReady(dstSubId)
                                && !SimServiceUtils.isServiceRunning(
                                        mService.getApplicationContext(), dstSubId)) {
                            contactsProviderTiming.timingStart();
                            resolver.applyBatch(ContactsContract.AUTHORITY, operationList);
                            contactsProviderTiming.timingEnd();
                        }
                        Log.i(TAG, "[copyContactsToSim]After applyBatch ");
                    } catch (android.os.RemoteException e) {
                        Log.e(TAG, String.format("%s: %s", e.toString(), e.getMessage()));
                    } catch (android.content.OperationApplicationException e) {
                        Log.e(TAG, String.format("%s: %s", e.toString(), e.getMessage()));
                    }
                    operationList.clear();
                }
            } // inner looper
            if (subContact > 0) {
                successfulItems++;
            }
            if (isSimStorageFull) {
                break;
            }
        }

        if (operationList.size() > 0) {
            try {
                Log.i(TAG, "[copyContactsToSim]Before end applyBatch. ");
                /** M: Bug Fix for ALPS00695093 @{ */
                if (isPhoneBookReady(dstSubId) && !SimServiceUtils.isServiceRunning(
                                mService.getApplicationContext(), dstSubId)) {
                    contactsProviderTiming.timingStart();
                    resolver.applyBatch(ContactsContract.AUTHORITY, operationList);
                    contactsProviderTiming.timingEnd();
                }
                Log.i(TAG, "[copyContactsToSim]After end applyBatch ");
            } catch (android.os.RemoteException e) {
                Log.e(TAG, String.format("%s: %s", e.toString(), e.getMessage()));
            } catch (android.content.OperationApplicationException e) {
                Log.e(TAG, String.format("%s: %s", e.toString(), e.getMessage()));
            }
            operationList.clear();
        }

        if (mIsCanceled) {
            Log.d(TAG, "[copyContactsToSim] run: mCanceled = true");
            errorCause = ErrorCause.USER_CANCEL;
            mService.handleFinishNotification(mJobId, false);
            mListener.onCanceled(MultiChoiceService.TYPE_COPY, mJobId, totalItems, successfulItems,
                    totalItems - successfulItems);
            return;
        }

        mService.handleFinishNotification(mJobId, errorCause == ErrorCause.NO_ERROR);
        if (errorCause == ErrorCause.NO_ERROR) {
            mListener.onFinished(MultiChoiceService.TYPE_COPY, mJobId, totalItems);
        } else {
            mListener.onFailed(MultiChoiceService.TYPE_COPY, mJobId, totalItems, successfulItems,
                    totalItems - successfulItems, errorCause);
        }

        iccProviderTiming.log("copyContactsToSim():IccProviderTiming");
        contactsProviderTiming.log("copyContactsToSim():ContactsProviderTiming");
        checkStatusTiming.log("copyContactsToSim():CheckStatusTiming");
    }

    private boolean isPhoneBookReady(int subId) {
        boolean result = SimCardUtils.isPhoneBookReady(subId);
        Log.d(TAG, "[isPhoneBookReady]result= " + result);
        return result;
    }

    private void cursorColumnToBuilder(Cursor cursor, String[] columnNames, int index,
            ContentProviderOperation.Builder builder) {
        switch (cursor.getType(index)) {
        case Cursor.FIELD_TYPE_NULL:
            // don't put anything in the content values
            break;
        case Cursor.FIELD_TYPE_INTEGER:
            builder.withValue(columnNames[index], cursor.getLong(index));
            break;
        case Cursor.FIELD_TYPE_STRING:
            builder.withValue(columnNames[index], cursor.getString(index));
            break;
        case Cursor.FIELD_TYPE_BLOB:
            builder.withValue(columnNames[index], cursor.getBlob(index));
            break;
        default:
            throw new IllegalStateException("Invalid or unhandled data type");
        }
    }

    private void printSimErrorDetails(String errorCode) {
        int iccError = Integer.valueOf(errorCode);
        switch (iccError) {
        case ErrorCause.SIM_NUMBER_TOO_LONG:
            Log.d(TAG, "ERROR PHONE NUMBER TOO LONG");
            break;
        case ErrorCause.SIM_NAME_TOO_LONG:
            Log.d(TAG, "ERROR NAME TOO LONG");
            break;
        case ErrorCause.SIM_STORAGE_FULL:
            Log.d(TAG, "ERROR STORAGE FULL");
            break;
        case ErrorCause.SIM_ICC_NOT_READY:
            Log.d(TAG, "ERROR ICC NOT READY");
            break;
        case ErrorCause.SIM_PASSWORD_ERROR:
            Log.d(TAG, "ERROR ICC PASSWORD ERROR");
            break;
        case ErrorCause.SIM_ANR_TOO_LONG:
            Log.d(TAG, "ERROR ICC ANR TOO LONG");
            break;
        case ErrorCause.SIM_GENERIC_FAILURE:
            Log.d(TAG, "ERROR ICC GENERIC FAILURE");
            break;
        case ErrorCause.SIM_ADN_LIST_NOT_EXIT:
            Log.d(TAG, "ERROR ICC ADN LIST NOT EXIST");
            break;
        case ErrorCause.ERROR_USIM_EMAIL_LOST:
            Log.d(TAG, "ERROR ICC USIM EMAIL LOST");
            break;
        default:
            Log.d(TAG, "ERROR ICC UNKNOW");
            break;
        }
    }

    private void generateDataBuilder(Cursor dataCursor, Builder builder, String[] columnNames,
            String mimeType, String srcAccountType) {
        for (int i = 1; i < columnNames.length; i++) {
            /** M:AAS [COMMD_FOR_AAS]@ { */
            if (GlobalEnv.getSimAasEditor().cursorColumnToBuilder(dataCursor, builder,
                    srcAccountType, mimeType, ((AccountWithDataSetEx) mAccountDst).getSubId(), i)) {
                continue;
            }
            // USIM email has no type, add default type for destination account's email
            if (AccountTypeUtils.isAccountTypeIccCard(srcAccountType) &&
                    Email.CONTENT_ITEM_TYPE.equals(mimeType) &&
                    Email.TYPE.equals(columnNames[i]) &&
                    dataCursor.isNull(i)) {
                // HOME is the default type of phone account email
                builder.withValue(Email.TYPE, Email.TYPE_HOME);
                continue;
            }
            cursorColumnToBuilder(dataCursor, columnNames, i, builder);
        }
    }

}
