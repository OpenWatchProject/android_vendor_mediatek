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

import android.content.ContentResolver;
import android.database.Cursor;
import android.net.Uri;
import android.os.PowerManager;
import android.os.Process;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.provider.ContactsContract.Contacts;
import android.text.TextUtils;

import com.android.contacts.R;
import com.android.contacts.vcard.ProcessorBase;

import com.mediatek.contacts.simcontact.SimCardUtils;
import com.mediatek.contacts.simcontact.SubInfoUtils;
import com.mediatek.contacts.simservice.SimServiceUtils;
import com.mediatek.contacts.util.Log;
import com.mediatek.contacts.util.MtkToast;
import com.mediatek.contacts.util.TimingStatistics;
import com.mediatek.internal.telephony.phb.IMtkIccPhoneBook;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;

public class DeleteProcessor extends ProcessorBase {
    private static final String TAG = "DeleteProcessor";

    private final MultiChoiceService mService;
    private final ContentResolver mResolver;
    private final List<MultiChoiceRequest> mRequests;
    private final int mJobId;
    private final MultiChoiceHandlerListener mListener;

    private PowerManager.WakeLock mWakeLock;

    private volatile boolean mIsCanceled;
    private volatile boolean mIsDone;
    private volatile boolean mIsRunning;

    private static final int MAX_OP_COUNT_IN_ONE_BATCH = 100;

    // change max count and max count in one batch for special operator
    private static final int MAX_COUNT = 1551;
    private static final int MAX_COUNT_IN_ONE_BATCH = 50;

    public DeleteProcessor(final MultiChoiceService service,
            final MultiChoiceHandlerListener listener, final List<MultiChoiceRequest> requests,
            final int jobId) {
        Log.i(TAG, "[DeleteProcessor]new.");
        mService = service;
        mResolver = mService.getContentResolver();
        mListener = listener;

        mRequests = requests;
        mJobId = jobId;

        final PowerManager powerManager = (PowerManager) mService.getApplicationContext()
                .getSystemService("power");
        mWakeLock = powerManager.newWakeLock(PowerManager.SCREEN_DIM_WAKE_LOCK
                | PowerManager.ON_AFTER_RELEASE, TAG);
    }

    @Override
    public synchronized boolean cancel(boolean mayInterruptIfRunning) {
        Log.i(TAG, "[cancel]mIsDone = " + mIsDone + ",mIsCanceled = " + mIsCanceled
                + ",mIsRunning = " + mIsRunning);
        if (mIsDone || mIsCanceled) {
            return false;
        }

        mIsCanceled = true;
        if (!mIsRunning) {
            mService.handleFinishNotification(mJobId, false);
            mListener.onCanceled(MultiChoiceService.TYPE_DELETE, mJobId, -1, -1, -1);
        } else {
            /*
             * Bug Fix by Mediatek Begin. Original Android's code: xxx CR ID:
             * ALPS00249590 Descriptions:
             */
            mService.handleFinishNotification(mJobId, false);
            mListener.onCanceling(MultiChoiceService.TYPE_DELETE, mJobId);
            /*
             * Bug Fix by Mediatek End.
             */
        }

        return true;
    }

    @Override
    public int getType() {
        return MultiChoiceService.TYPE_DELETE;
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
        Log.i(TAG, "[run].");
        try {
            mIsRunning = true;
            mWakeLock.acquire();
            Process.setThreadPriority(Process.THREAD_PRIORITY_LOWEST);
            runInternal();
        } finally {
            synchronized (this) {
                mIsDone = true;
            }
            if (mWakeLock != null && mWakeLock.isHeld()) {
                mWakeLock.release();
            }
        }
    }

    private void runInternal() {
        if (isCancelled()) {
            Log.i(TAG, "[runInternal]Canceled before actually handling");
            return;
        }

        boolean succeessful = true;
        int totalItems = mRequests.size();
        int successfulItems = 0;
        int currentCount = 0;
        int iBatchDel = MAX_OP_COUNT_IN_ONE_BATCH;
        if (totalItems > MAX_COUNT) {
            iBatchDel = MAX_COUNT_IN_ONE_BATCH;
            Log.i(TAG, "[runInternal]iBatchDel = " + iBatchDel);
        }
        long startTime = System.currentTimeMillis();
        final ArrayList<Long> contactIdsList = new ArrayList<Long>();
        int times = 0;
        boolean skipAllSimContacts = false;

        int subId = SubInfoUtils.getInvalidSubId();
        HashMap<Integer, Uri> delSimUriMap = new HashMap<Integer, Uri>();
        TimingStatistics iccProviderTiming = new TimingStatistics(
                DeleteProcessor.class.getSimpleName());
        TimingStatistics contactsProviderTiming = new TimingStatistics(
                DeleteProcessor.class.getSimpleName());
        for (MultiChoiceRequest request : mRequests) {
            if (mIsCanceled) {
                Log.d(TAG, "[runInternal] run: mCanceled = true, break looper");
                break;
            }
            currentCount++;

            mListener.onProcessed(MultiChoiceService.TYPE_DELETE, mJobId, currentCount, totalItems,
                    request.mContactName);
            Log.d(TAG, "[runInternal]Indicator: " + request.mIndicator);
            // delete contacts from sim card
            if (request.mIndicator > 0) {
                subId = request.mIndicator;
                if (skipAllSimContacts) {
                    succeessful = false;
                    continue;
                }
                if (!SimCardUtils.isPhoneBookReady(subId)) {
                    Log.d(TAG, "[runInternal]phb not ready, skip all of sim contacts");
                    skipAllSimContacts = true;
                    succeessful = false;
                    MtkToast.toastFromNoneUiThread(R.string.icc_phone_book_invalid);
                    continue;
                }
                /// M: change for SIM Service refactoring
                if (SimServiceUtils.isServiceRunning(
                                mService.getApplicationContext(), subId)) {
                    Log.d(TAG, "[runInternal]sim service is running, skip all of sim contacts");
                    skipAllSimContacts = true;
                    succeessful = false;
                    MtkToast.toastFromNoneUiThread(R.string.phone_book_busy);
                    continue;
                }

                Uri delSimUri = null;
                if (delSimUriMap.containsKey(subId)) {
                    delSimUri = delSimUriMap.get(subId);
                } else {
                    delSimUri = SubInfoUtils.getIccProviderUri(subId);
                    delSimUriMap.put(subId, delSimUri);
                }

                String where = ("index = " + request.mSimIndex);

                iccProviderTiming.timingStart();
                int deleteCount = mResolver.delete(delSimUri, where, null);
                iccProviderTiming.timingEnd();
                if (deleteCount <= 0) {
                    if (isReadyForDelete(subId) && isAdnReady(subId)
                            && isSimContactDisappear(delSimUri, request.mSimIndex)
                                    && SimCardUtils.isPhoneBookReady(subId)) {
                        successfulItems++;
                        contactIdsList.add(Long.valueOf(request.mContactId));
                        Log.w(TAG, "[runInternal]handle as delete success: " + request.mSimIndex);
                    } else {
                        succeessful = false;
                    }
                } else {
                    successfulItems++;
                    contactIdsList.add(Long.valueOf(request.mContactId));
                }
            } else {
                successfulItems++;
                contactIdsList.add(Long.valueOf(request.mContactId));
            }

            // delete contacts from database
            if (contactIdsList.size() >= iBatchDel) {
                contactsProviderTiming.timingStart();
                actualBatchDelete(contactIdsList);
                contactsProviderTiming.timingEnd();
                Log.i(TAG, "[runInternal]the " + (++times) + ",iBatchDel = " + iBatchDel);
                contactIdsList.clear();
                if ((totalItems - currentCount) <= MAX_COUNT) {
                    iBatchDel = MAX_OP_COUNT_IN_ONE_BATCH;
                }
            }
        }

        if (contactIdsList.size() > 0) {
            contactsProviderTiming.timingStart();
            actualBatchDelete(contactIdsList);
            contactsProviderTiming.timingEnd();
            contactIdsList.clear();
        }

        Log.d(TAG, "[runInternal]totaltime: " + (System.currentTimeMillis() - startTime));

        if (mIsCanceled) {
            Log.d(TAG, "[runInternal]run: mCanceled = true, return");
            succeessful = false;
            mService.handleFinishNotification(mJobId, false);
            mListener.onCanceled(MultiChoiceService.TYPE_DELETE, mJobId, totalItems,
                    successfulItems, totalItems - successfulItems);
            return;
        }
        mService.handleFinishNotification(mJobId, succeessful);
        if (succeessful) {
            mListener.onFinished(MultiChoiceService.TYPE_DELETE, mJobId, totalItems);
        } else {
            mListener.onFailed(MultiChoiceService.TYPE_DELETE, mJobId, totalItems, successfulItems,
                    totalItems - successfulItems);
        }

        iccProviderTiming.log("runInternal():IccProviderTiming");
        contactsProviderTiming.log("runInternal():ContactsProviderTiming");
    }

    private boolean isSimContactDisappear(Uri simUri, int index) {
        boolean ret = true;
        Cursor cursor = mResolver.query(simUri, new String[] {"index"},null, null, null);
        if (cursor == null) {
            Log.e(TAG, "[isSimContactDisappear] Cursor is null");
            return false;
        }
        try {
            while (cursor.moveToNext()) {
                if (cursor.getLong(0) == (long)index) {
                    ret = false;
                }
            }
        } finally {
            cursor.close();
        }
        Log.d(TAG, "[isSimContactDisappear]ret=" + ret + ", index=" + index + "simUri:" + simUri);
        return ret;
    }

    private boolean isAdnReady(int subId) {
        boolean ret = false;
        final String serviceName = SubInfoUtils.getMtkPhoneBookServiceName();
        final IMtkIccPhoneBook iIccPhb = IMtkIccPhoneBook.Stub.asInterface(
                ServiceManager.getService(serviceName));
        try {
            if (iIccPhb == null) {
                Log.e(TAG, "[isAdnReady] iIccPhb is null");
            } else {
                ret = iIccPhb.isAdnAccessible(subId);
            }
        } catch (RemoteException e) {
            Log.e(TAG, "[isAdnReady]Exception happened", e);
        }
        Log.d(TAG, "[isAdnReady] isAdnAccessible:" + ret + ", subId=" + subId);
        return ret;
    }

    private int actualBatchDelete(ArrayList<Long> contactIdList) {
        Log.d(TAG, "[actualBatchDelete]");
        if (contactIdList == null || contactIdList.size() == 0) {
            Log.w(TAG, "[actualBatchDelete]input error,contactIdList = " + contactIdList);
            return 0;
        }

        final StringBuilder whereBuilder = new StringBuilder();
        final ArrayList<String> whereArgs = new ArrayList<String>();
        final String[] questionMarks = new String[contactIdList.size()];
        for (long contactId : contactIdList) {
            whereArgs.add(String.valueOf(contactId));
        }
        Arrays.fill(questionMarks, "?");
        whereBuilder.append(Contacts._ID + " IN (").append(TextUtils.join(",", questionMarks))
                .append(")");

        int deleteCount = mResolver.delete(
                Contacts.CONTENT_URI.buildUpon().appendQueryParameter("batch", "true").build(),
                whereBuilder.toString(), whereArgs.toArray(new String[0]));
        Log.d(TAG, "[actualBatchDelete]deleteCount:" + deleteCount + " Contacts");
        return deleteCount;
    }

    private boolean isReadyForDelete(int subId) {
        return SimCardUtils.isSimStateIdle(mService.getApplicationContext(), subId);
    }
}
