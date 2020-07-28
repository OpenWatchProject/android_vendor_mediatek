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
package com.mediatek.contacts.simcontact;

import android.os.AsyncTask;
import android.os.Handler;
import android.os.Looper;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemClock;
import android.telephony.SubscriptionInfo;

import com.android.contacts.R;

import com.mediatek.contacts.util.ContactsPortableUtils;
import com.mediatek.contacts.util.Log;
import com.mediatek.internal.telephony.phb.IMtkIccPhoneBook;

import java.util.concurrent.Executor;
import java.util.concurrent.Executors;
import java.util.HashMap;
import java.util.List;

/**
 * Phb info helper class.
 */
public final class PhbInfoUtils {
    private static final String TAG = "PhbInfoUtils";

    private static final int DEFAULT_SUBINFO_COUNT = 0;
    private static HashMap<Integer, PhbInfoWrapper> mActiveUsimPhbInfoMap = null;
    private static Executor sRefreshExecutor = Executors.newSingleThreadExecutor();

    private PhbInfoUtils() {
    }

    private final static class PhbInfoWrapper {
        private int mSubId = SubInfoUtils.getInvalidSubId();
        private int mUsimGroupMaxNameLength;
        private int mUsimGroupCount;
        private int mUsimAnrCount;
        private int mUsimEmailCount;
        // add for Aas&Sne
        private int mUsimAasCount;
        private int mUsimAasMaxNameLength;
        private int mUsimSneMaxNameLength;
        private boolean mHasSne;
        private boolean mInitialized;
        private final long mStartTime = SystemClock.elapsedRealtime();
        private final Handler mUiHandler = new Handler(
                Looper.getMainLooper());
        private static final int INFO_NOT_READY = -1;
        private static final long REFRESH_INTERVAL = 5*1000;
        private static final long REFRESH_TOTAL_TIME = 2*60*1000;

        public PhbInfoWrapper(int subId) {
            mSubId = subId;
            resetPhbInfo();
            if (ContactsPortableUtils.MTK_PHONE_BOOK_SUPPORT) {
                refreshPhbInfo();
            }
        }

        private void resetPhbInfo() {
            mUsimGroupMaxNameLength = INFO_NOT_READY;
            mUsimGroupCount = INFO_NOT_READY;
            mUsimAnrCount = INFO_NOT_READY;
            mUsimEmailCount = INFO_NOT_READY;
            mInitialized = false;
            // add for Aas&Sne
            mUsimAasCount = INFO_NOT_READY;
            mUsimAasMaxNameLength = INFO_NOT_READY;
            mUsimSneMaxNameLength = INFO_NOT_READY;
            mHasSne = false;
        }

        private void refreshPhbInfo() {
            Log.i(TAG, "[refreshPhbInfo]refreshing phb info for subId: " + mSubId);
            if (!SimCardUtils.isPhoneBookReady(mSubId)) {
                Log.e(TAG, "[refreshPhbInfo]phb not ready, refresh aborted. slot: " + mSubId);
                mInitialized = false;
                return;
            }
            // /TODO: currently, Usim or Csim is necessary for phb infos.
            if (!SimCardUtils.isUsimOrCsimType(mSubId)) {
                Log.i(TAG, "[refreshPhbInfo]not usim phb, nothing to refresh, keep default "
                            + ", subId: "  + mSubId);
                mInitialized = true;
                return;
            }

            if (!mInitialized) {
                Log.d(TAG, "[refreshPhbInfo] schedule GetSimInfoTask");
                new GetSimInfoTask(this).executeOnExecutor(sRefreshExecutor, mSubId);
            }
        }

        private void refreshPhbInfoDelayed() {
            if ((SystemClock.elapsedRealtime() - mStartTime)
                    <= REFRESH_TOTAL_TIME) {
                mUiHandler.postDelayed(new Runnable() {

                    @Override
                    public void run() {
                        refreshPhbInfo();
                    }
                }, REFRESH_INTERVAL);
            }
        }

        private int getUsimGroupMaxNameLength() {
            if (!mInitialized) {
                refreshPhbInfo();
            }
            Log.d(TAG, "[getUsimGroupMaxNameLength] subId = " + mSubId
                    + ",length = " + mUsimGroupMaxNameLength);
            return mUsimGroupMaxNameLength;
        }

        private int getUsimGroupCount() {
            if (!mInitialized) {
                refreshPhbInfo();
            }
            Log.d(TAG, "[getUsimGroupMaxCount] subId = " + mSubId
                    + ", count = " + mUsimGroupCount);
            return mUsimGroupCount;
        }

        private int getUsimAnrCount() {
            if (!mInitialized) {
                refreshPhbInfo();
            }
            Log.d(TAG, "[getUsimAnrCount] subId = " + mSubId
                    + ", count = " + mUsimAnrCount);
            return mUsimAnrCount;
        }

        private int getUsimEmailCount() {
            if (!mInitialized) {
                refreshPhbInfo();
            }
            Log.d(TAG, "[getUsimEmailCount] subId = " + mSubId
                    + ", count = " + mUsimEmailCount);
            return mUsimEmailCount;
        }

        private int getUsimAasCount() {
            if (!mInitialized) {
                refreshPhbInfo();
            }
            Log.d(TAG, "[getUsimAasCount] subId = " + mSubId
                    + ", count = " + mUsimAasCount);
            return mUsimAasCount;
        }

        private boolean usimHasSne() {
            if (!mInitialized) {
                refreshPhbInfo();
            }
            Log.d(TAG, "[usimHasSne] subId = " + mSubId
                    + ", mHasSne = " + mHasSne);
            return mHasSne;
        }

        private int getUsimAasMaxNameLength() {
            if (!mInitialized) {
                refreshPhbInfo();
            }
            Log.d(TAG, "[getUsimAasMaxNameLength] subId = " + mSubId
                    + ", length = " + mUsimAasMaxNameLength);
            return mUsimAasMaxNameLength;
        }

        private int getUsimSneMaxNameLength() {
            if (!mInitialized) {
                refreshPhbInfo();
            }
            Log.d(TAG, "[getUsimSneMaxNameLength] subId = " + mSubId
                    + ", length = " + mUsimSneMaxNameLength);
            return mUsimSneMaxNameLength;
        }

        private boolean isInitialized() {
            return mInitialized;
        }
    }

    private static final class GetSimInfoTask extends AsyncTask<Integer, Void, PhbInfoWrapper> {
        private PhbInfoWrapper mPhbInfoWrapper;

        public GetSimInfoTask(PhbInfoWrapper phbInfoWrapper) {
            mPhbInfoWrapper = phbInfoWrapper;
        }

        @Override
        protected PhbInfoWrapper doInBackground(Integer... params) {
            final int subId = params[0];
            Log.d(TAG, "[GetSimInfoTask] subId = " + subId);
            String serviceName = SubInfoUtils.getMtkPhoneBookServiceName();
            try {
                final IMtkIccPhoneBook iIccPhb = IMtkIccPhoneBook.Stub.asInterface(ServiceManager
                        .getService(serviceName));
                if (iIccPhb == null) {
                    Log.e(TAG, "[GetSimInfoTask] IIccPhoneBook is null!");
                    mPhbInfoWrapper.mInitialized = false;
                    return null;
                }
                mPhbInfoWrapper.mUsimGroupMaxNameLength = iIccPhb.getUsimGrpMaxNameLen(subId);
                mPhbInfoWrapper.mUsimGroupCount = iIccPhb.getUsimGrpMaxCount(subId);
                mPhbInfoWrapper.mUsimAnrCount = iIccPhb.getAnrCount(subId);
                mPhbInfoWrapper.mUsimEmailCount = iIccPhb.getEmailCount(subId);
                mPhbInfoWrapper.mHasSne = iIccPhb.hasSne(subId);
                mPhbInfoWrapper.mUsimAasCount = iIccPhb.getUsimAasMaxCount(subId);
                mPhbInfoWrapper.mUsimAasMaxNameLength = iIccPhb.getUsimAasMaxNameLen(subId);
                mPhbInfoWrapper.mUsimSneMaxNameLength = iIccPhb.getSneRecordLen(subId);
                if (PhbInfoWrapper.INFO_NOT_READY == mPhbInfoWrapper.mUsimGroupMaxNameLength
                        || PhbInfoWrapper.INFO_NOT_READY == mPhbInfoWrapper.mUsimGroupCount
                        || PhbInfoWrapper.INFO_NOT_READY == mPhbInfoWrapper.mUsimAnrCount
                        || PhbInfoWrapper.INFO_NOT_READY == mPhbInfoWrapper.mUsimEmailCount
                        || PhbInfoWrapper.INFO_NOT_READY == mPhbInfoWrapper.mUsimAasCount
                        || PhbInfoWrapper.INFO_NOT_READY == mPhbInfoWrapper.mUsimAasMaxNameLength
                        || PhbInfoWrapper.INFO_NOT_READY == mPhbInfoWrapper.mUsimSneMaxNameLength) {
                    mPhbInfoWrapper.mInitialized = false;
                    Log.d(TAG, "[GetSimInfoTask] Initialize = false. Not all info ready,"
                            + "still need refresh next time");
                    /* M: [ALPS03737363][ALPS03765513] try to refresh multiple times
                     * until all phb info ready for caching them ASAP. Then Editor
                     * can show up all sim data kind earlier. */
                    mPhbInfoWrapper.refreshPhbInfoDelayed();
                } else {
                    mPhbInfoWrapper.mInitialized = true;
                    Log.d(TAG, "[GetSimInfoTask] Initialize = true");
                }
            } catch (RemoteException e) {
                Log.e(TAG, "[GetSimInfoTask]Exception happened when refreshing phb info");
                e.printStackTrace();
                mPhbInfoWrapper.mInitialized = false;
                return null;
            }
            Log.i(TAG, "[GetSimInfoTask]refreshing done,UsimGroupMaxNameLenght = "
                    + mPhbInfoWrapper.mUsimGroupMaxNameLength
                    + ", UsimGroupMaxCount = " + mPhbInfoWrapper.mUsimGroupCount
                    + ", UsimAnrCount = " + mPhbInfoWrapper.mUsimAnrCount
                    + ", UsimEmailCount = " + mPhbInfoWrapper.mUsimEmailCount
                    + ", mHasSne = " + mPhbInfoWrapper.mHasSne
                    + ", mUsimAasMaxCount = " + mPhbInfoWrapper.mUsimAasCount
                    + ", mUsimAasMaxNameLength = " + mPhbInfoWrapper.mUsimAasMaxNameLength
                    + ", mUsimSneMaxNameLength = " + mPhbInfoWrapper.mUsimSneMaxNameLength);
            return mPhbInfoWrapper;
        }
    }

    public static void clearActiveUsimPhbInfoMap() {
        Log.d(TAG, "clearActiveUsimPhbInfoMap");
        mActiveUsimPhbInfoMap = null;
    }

    public static HashMap<Integer, PhbInfoWrapper> getActiveUsimPhbInfoMap() {
        if (mActiveUsimPhbInfoMap == null) {
            mActiveUsimPhbInfoMap = new HashMap<Integer, PhbInfoWrapper>();
            List<SubscriptionInfo> subscriptionInfoList = SubInfoUtils.getActivatedSubInfoList();
            Log.sensitive(TAG, "[getActiveUsimPhbInfoMap] subscriptionInfoList: " + subscriptionInfoList);
            if (subscriptionInfoList != null && subscriptionInfoList.size() > 0) {
                for (SubscriptionInfo subscriptionInfo : subscriptionInfoList) {
                    mActiveUsimPhbInfoMap.put(subscriptionInfo.getSubscriptionId(),
                            new PhbInfoWrapper(subscriptionInfo.getSubscriptionId()));
                }
            }
        }
        return mActiveUsimPhbInfoMap;
    }

    public static void refreshActiveUsimPhbInfoMap(Boolean isPhbReady, Integer subId) {
        Log.i(TAG, "[refreshActiveUsimPhbInfoMap] subId: " + subId + ", isPhbReady: " +
                isPhbReady + ",mActiveUsimPhbInfoMap: " + mActiveUsimPhbInfoMap);
        if (mActiveUsimPhbInfoMap == null) {
            getActiveUsimPhbInfoMap();
            Log.i(TAG, "[refreshActiveUsimPhbInfoMap] get all PhbInfoMap done," +
                    ",mActiveUsimPhbInfoMap: " + mActiveUsimPhbInfoMap);
            return;
        }
        if (subId < 0) {
            Log.d(TAG, "refreshActiveUsimPhbInfoMap subId wrong");
            return;
        }
        if (isPhbReady) {
            Log.d(TAG, "[refreshActiveUsimPhbInfoMap] phb ready, put subId = " + subId);
            mActiveUsimPhbInfoMap.put(subId, new PhbInfoWrapper(subId));
        } else {
            Log.d(TAG, "[refreshActiveUsimPhbInfoMap] phb not ready, try to remove subId:" + subId);
            if (mActiveUsimPhbInfoMap.containsKey(subId)) {
                Log.d(TAG, "[refreshActiveUsimPhbInfoMap] remove subId: " + subId);
                mActiveUsimPhbInfoMap.remove(subId);
            }
        }
    }

    public static int getUsimGroupMaxNameLength(int subId) {
        PhbInfoWrapper usimPhbInfo = getActiveUsimPhbInfoMap().get(subId);
        if (usimPhbInfo != null) {
            return usimPhbInfo.getUsimGroupMaxNameLength();
        }
        return -1;
    }

    public static int getUsimGroupMaxCount(int subId) {
        PhbInfoWrapper usimPhbInfo = getActiveUsimPhbInfoMap().get(subId);
        if (null == usimPhbInfo) {
            return DEFAULT_SUBINFO_COUNT;
        }
        int count = usimPhbInfo.getUsimGroupCount();
        if (PhbInfoWrapper.INFO_NOT_READY == count) {
            return DEFAULT_SUBINFO_COUNT;
        }
        return count;
    }

    public static int getUsimAnrCount(int subId) {
        PhbInfoWrapper usimPhbInfo = getActiveUsimPhbInfoMap().get(subId);
        if (null == usimPhbInfo) {
            return DEFAULT_SUBINFO_COUNT;
        }
        int count = usimPhbInfo.getUsimAnrCount();
        if (PhbInfoWrapper.INFO_NOT_READY == count) {
            return DEFAULT_SUBINFO_COUNT;
        }
        return count;
    }

    public static int getUsimEmailCount(int subId) {
        PhbInfoWrapper usimPhbInfo = getActiveUsimPhbInfoMap().get(subId);
        if (null == usimPhbInfo) {
            return DEFAULT_SUBINFO_COUNT;
        }
        int count = usimPhbInfo.getUsimEmailCount();
        if (PhbInfoWrapper.INFO_NOT_READY == count) {
            return DEFAULT_SUBINFO_COUNT;
        }
        return count;
    }

    public static int getUsimAasCount(int subId) {
        PhbInfoWrapper usimPhbInfo = getActiveUsimPhbInfoMap().get(subId);
        if (null == usimPhbInfo) {
            return DEFAULT_SUBINFO_COUNT;
        }
        int count = usimPhbInfo.getUsimAasCount();
        if (PhbInfoWrapper.INFO_NOT_READY == count) {
            return DEFAULT_SUBINFO_COUNT;
        }
        return count;
    }

    public static boolean usimHasSne(int subId) {
        PhbInfoWrapper usimPhbInfo = getActiveUsimPhbInfoMap().get(subId);
        if (usimPhbInfo != null) {
            return usimPhbInfo.usimHasSne();
        }
        return false;
    }

    public static int getUsimAasMaxNameLength(int subId) {
        PhbInfoWrapper usimPhbInfo = getActiveUsimPhbInfoMap().get(subId);
        if (usimPhbInfo != null) {
            return usimPhbInfo.getUsimAasMaxNameLength();
        }
        return -1;
    }

    public static int getUsimSneMaxNameLength(int subId) {
        PhbInfoWrapper usimPhbInfo = getActiveUsimPhbInfoMap().get(subId);
        if (usimPhbInfo != null) {
            return usimPhbInfo.getUsimSneMaxNameLength();
        }
        return -1;
    }

    public static boolean isInitialized(int subId) {
        PhbInfoWrapper usimPhbInfo = getActiveUsimPhbInfoMap().get(subId);
        if (usimPhbInfo != null) {
            return usimPhbInfo.isInitialized();
        }
        return false;
    }
}
