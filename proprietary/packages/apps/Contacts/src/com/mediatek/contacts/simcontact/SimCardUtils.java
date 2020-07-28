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

import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.os.AsyncTask;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.provider.Settings;
import android.telephony.SubscriptionInfo;
import android.telephony.TelephonyManager;
import android.widget.Toast;

import com.android.contacts.R;
import com.android.internal.telephony.ITelephony;
import com.google.common.annotations.VisibleForTesting;
import com.mediatek.contacts.simservice.SimServiceUtils;
import com.mediatek.contacts.util.ContactsPortableUtils;
import com.mediatek.contacts.util.Log;
import com.mediatek.internal.telephony.IMtkTelephonyEx;

import java.util.HashMap;
import java.util.List;
/**
 * Help class for sim card properties, e.g. sim state, sim type
 */
public class SimCardUtils {
    private static final String TAG = "SimCardUtils";

    public interface SimType {
        public static final String SIM_TYPE_USIM_TAG = "USIM";
        public static final String SIM_TYPE_SIM_TAG = "SIM";
        public static final String SIM_TYPE_RUIM_TAG = "RUIM";
        public static final String SIM_TYPE_CSIM_TAG = "CSIM";
        public static final String SIM_TYPE_UNKNOWN_TAG = "UNKNOWN";
    }

    public static boolean isSimInsertedBySlot(int slotId) {
        final ITelephony iTel = ITelephony.Stub.asInterface(ServiceManager
                .getService(Context.TELEPHONY_SERVICE));
        boolean isSimInsert = false;
        try {
            if (iTel != null) {
                isSimInsert = iTel.hasIccCardUsingSlotIndex(slotId);
            }
        } catch (RemoteException e) {
            Log.e(TAG, "[isSimInserted]catch exception:");
            e.printStackTrace();
            isSimInsert = false;
        }
        Log.d(TAG, "[isSimInserted]slotId:" + slotId + ",isSimInsert:" + isSimInsert);
        return isSimInsert;
    }

    public static boolean isSimInsertedBySub(int subId) {
        int slotId = SubInfoUtils.getSlotIdUsingSubId(subId);;
        if (slotId == SubInfoUtils.getInvalidSlotId()) {
            return false;
        }
        return isSimInsertedBySlot(slotId);
    }

    /**
     * check PhoneBook State is ready if ready, then return true.
     *
     * @param subId
     * @return
     */
    public static boolean isPhoneBookReady(int subId) {
        final IMtkTelephonyEx telephonyEx = IMtkTelephonyEx.Stub.asInterface(ServiceManager
                .getService("phoneEx"));
        if (null == telephonyEx) {
            Log.w(TAG, "[isPhoneBookReady]phoneEx == null");
            return false;
        }
        boolean isPbReady = false;
        try {
            isPbReady = telephonyEx.isPhbReady(subId);
        } catch (RemoteException e) {
            Log.e(TAG, "[isPhoneBookReady]catch exception:");
            e.printStackTrace();
        }
        Log.d(TAG, "[isPhoneBookReady]subId:" + subId + ", isPbReady:" + isPbReady);
        return isPbReady;
    }

    /**
     * Check that whether the phone book is ready only
     *
     * @param context
     *            the caller's context.
     * @param subId
     *            the slot to check.
     * @return true the phb is ready false the phb is not ready
     */
    public static boolean isPhoneBookReady(Context context, int subId) {
        boolean hitError = false;
        int errorToastId = -1;
        if (!isPhoneBookReady(subId)) {
            hitError = true;
            errorToastId = R.string.icc_phone_book_invalid;
        }
        if (context == null) {
            Log.w(TAG, "[isPhoneBookReady] context is null,subId:" + subId);
        }
        if (hitError && context != null) {
            Toast.makeText(context, errorToastId, Toast.LENGTH_LONG).show();
            Log.d(TAG, "[isPhoneBookReady] hitError=" + hitError);
        }
        return !hitError;
    }

    /**
     * M: Check that whether the phone book is ready, and whether the sim card
     * storage is full.
     *
     * @param context
     *            the caller's context.
     * @param subId
     *            the slot to check.
     * @return true the phb is ready and sim card storage is OK, false the phb
     *         is not ready or sim card storage is full.
     */
    public static boolean checkPHBStateAndSimStorage(Context context, int subId) {
        long startTime = System.currentTimeMillis();
        boolean hitError = false;
        int errorToastId = -1;
        if (!isPhoneBookReady(subId)) {
            hitError = true;
            errorToastId = R.string.icc_phone_book_invalid;
        } else if (0 == ShowSimCardStorageInfoTask.getAvailableCount(subId)) {
            hitError = true;
            errorToastId = R.string.storage_full;
        }
        if (context == null) {
            Log.w(TAG, "[checkPHBStateAndSimStorage] context is null,subId:" + subId);
        }
        if (hitError && context != null) {
            Toast.makeText(context, errorToastId, Toast.LENGTH_LONG).show();
            Log.d(TAG, "[checkPHBStateAndSimStorage] hitError=" + hitError);
        }
        return !hitError;
    }

    private static final String[] UICCCARD_PROPERTY_TYPE = {
        "gsm.ril.uicctype",
        "gsm.ril.uicctype2",
        "gsm.ril.uicctype3",
        "gsm.ril.uicctype4",
    };

    private static String getSimTypeByProperty(int subId) {
        int slotId = SubInfoUtils.getSlotIdUsingSubId(subId);
        String cardType = null;

        if (slotId >= 0 && slotId < 4) {
            cardType = SystemProperties.get(UICCCARD_PROPERTY_TYPE[slotId]);
        }
        Log.d(TAG, "[getSimTypeByProperty]slotId=" + slotId + ", cardType=" + cardType);
        return cardType;
    }

    /**
     * get sim type by subId, sim type is defined in
     * SimCardUtils.SimType
     *
     * @param subId
     * @return SimCardUtils.SimType
     */
    public static String getSimTypeBySubId(int subId) {
        String simType = SimType.SIM_TYPE_UNKNOWN_TAG;
        final IMtkTelephonyEx iTel = IMtkTelephonyEx.Stub.asInterface(ServiceManager
                .getService("phoneEx"));
        if (iTel == null) {
            Log.w(TAG, "[getSimTypeBySubId]iTel == null");
            return simType;
        }
        try {
            simType = iTel.getIccCardType(subId);
            if (simType == null || simType.isEmpty()) {
                simType = getSimTypeByProperty(subId);
            }
        } catch (RemoteException e) {
            Log.e(TAG, "[getSimTypeBySubId]catch exception:");
            e.printStackTrace();
        }
        return simType;
    }

    /**
     * check whether a slot is insert a usim card
     *
     * @param subId
     */
    public static boolean isUsimType(int subId) {
        String simType = getSimTypeBySubId(subId);
        return isUsimType(simType);
    }

    public static boolean isUsimType(String simType) {
        if (SimType.SIM_TYPE_USIM_TAG.equals(simType)) {
            Log.d(TAG, "[isUsimType] true");
            return true;
        }
        Log.d(TAG, "[isUsimType] false");
        return false;
    }

    /**
     * check whether a slot is insert a csim card
     *
     * @param subId
     */
    public static boolean isCsimType(int subId) {
        String simType = getSimTypeBySubId(subId);
        return isCsimType(simType);
    }

    public static boolean isCsimType(String simType) {
        if (SimType.SIM_TYPE_CSIM_TAG.equals(simType)) {
            Log.d(TAG, "[isCsimType] true");
            return true;
        }
        Log.d(TAG, "[isCsimType] false");
        return false;
    }

    /**
     * check whether a slot is insert a ruim card
     *
     * @param subId
     */
    public static boolean isRuimType(int subId) {
        String simType = getSimTypeBySubId(subId);
        return isRuimType(simType);
    }

    public static boolean isRuimType(String simType) {
        if (SimType.SIM_TYPE_RUIM_TAG.equals(simType)) {
            Log.d(TAG, "[isRuimType] true");
            return true;
        }
        Log.d(TAG, "[isRuimType] false");
        return false;
    }

    /**
     * check whether a slot is insert a sim card
     *
     * @param subId
     */
    public static boolean isSimType(int subId) {
        String simType = getSimTypeBySubId(subId);
        return isSimType(simType);
    }

    public static boolean isSimType(String simType) {
        if (SimType.SIM_TYPE_SIM_TAG.equals(simType)) {
            Log.d(TAG, "[isSimType] true");
            return true;
        }
        Log.d(TAG, "[isSimType] false");
        return false;
    }

    /**
     * check whether a slot is insert a usim or csim card
     *
     * @param subId
     * @return true if it is usim or csim card
     */
    public static boolean isUsimOrCsimType(int subId) {
        String simType = getSimTypeBySubId(subId);
        return isUsimType(simType) || isCsimType(simType);
    }

    /**
     * M: [Gemini+] not only ready, but also idle for all sim operations its
     * requirement is: 1. iccCard is insert 2. radio is on 3. FDN is off 4. PHB
     * is ready 5. simstate is ready 6. simService is not running
     *
     * @param slotId
     *            the slotId to check
     * @return true if idle
     */
    public static boolean isSimStateIdle(Context context, int subId) {
        Log.i(TAG, "[isSimStateIdle] subId: " + subId);
        if (!SubInfoUtils.checkSubscriber(subId)) {
            return false;
        }
        // /change for SIM Service Refactoring
        boolean isSimServiceRunning = SimServiceUtils.isServiceRunning(context, subId);
        Log.i(TAG, "[isSimStateIdle], isSimServiceRunning = " + isSimServiceRunning);
        return isPhoneBookReady(subId) && !isSimServiceRunning;
    }

    public static class ShowSimCardStorageInfoTask extends AsyncTask<Void, Void, Boolean> {
        private Context mContext = null;
        private int mSubIdToShow = SubInfoUtils.getInvalidSubId();
        private boolean mNeedToastIfFull = false;
        private static HashMap<Integer, Integer> sAvailableStorageMap =
                new HashMap<Integer, Integer>();

        public static void showSimCardStorageInfo(Context context) {
            showSimCardStorageInfo(context, false, SubInfoUtils.getInvalidSubId());
        }

        public static void showSimCardStorageInfo(Context context,
                boolean needToastIfFull, int subIdToShow) {
            new ShowSimCardStorageInfoTask(context,needToastIfFull, subIdToShow).execute();
            Log.i(TAG, "[showSimCardStorageInfo] needToastIfFull = " + needToastIfFull
                    + ", subIdToShow = " + subIdToShow);
        }

        public ShowSimCardStorageInfoTask(Context context, boolean needToastIfFull,
                int subIdToShow) {
            mContext = context;
            mNeedToastIfFull = needToastIfFull;
            mSubIdToShow = subIdToShow;
        }

        @Override
        protected Boolean doInBackground(Void... args) {
            sAvailableStorageMap.clear();
            List<SubscriptionInfo> subscriptionInfoList = SubInfoUtils.getActivatedSubInfoList();
            Log.i(TAG, "[ShowSimCardStorageInfoTask]: subInfos.size = "
                            + SubInfoUtils.getActivatedSubInfoCount());
            if (subscriptionInfoList == null || subscriptionInfoList.size() <= 0) {
                Log.w(TAG, "[ShowSimCardStorageInfoTask] no subscriptionInfo");
                return false;
            }
            for (SubscriptionInfo subscriptionInfo : subscriptionInfoList) {
                int[] storageInfos = null;
                Log.d(TAG, "[ShowSimCardStorageInfoTask]"
                        + "; simName = " + subscriptionInfo.getDisplayName()
                        + "; simSlot = "+ subscriptionInfo.getSimSlotIndex()
                        + "; subId = " + subscriptionInfo.getSubscriptionId());
                try {
                    IMtkTelephonyEx phoneEx = IMtkTelephonyEx.Stub.asInterface(ServiceManager
                            .checkService("phoneEx"));
                    if (phoneEx == null) {
                        Log.w(TAG, "[ShowSimCardStorageInfoTask] phone = null");
                        return false;
                    }
                    /// [ALPS04245655] only phb ready can call getAdnStorageInfo(),
                    /// otherwise it may return wrong value. @{
                    if (!phoneEx.isPhbReady(subscriptionInfo.getSubscriptionId())) {
                        Log.w(TAG, "[ShowSimCardStorageInfoTask] phb is not ready, subId = "
                                + subscriptionInfo.getSubscriptionId());
                        return false;
                    }
                    /// @}
                    storageInfos = phoneEx.getAdnStorageInfo(subscriptionInfo.getSubscriptionId());
                    if (storageInfos == null) {
                        Log.w(TAG, "[ShowSimCardStorageInfoTask] storageInfos = null.");
                        return false;
                    }
                    //Refresh the USIM Anr info, not means need use it immediately.
                    PhbInfoUtils.getUsimAnrCount(subscriptionInfo.getSubscriptionId());
                } catch (RemoteException ex) {
                    Log.i(TAG, "[ShowSimCardStorageInfoTask]_exception: " + ex);
                    return false;
                }
                Log.i(TAG, "[ShowSimCardStorageInfoTask]"
                        + ", subId:" + subscriptionInfo.getSubscriptionId()
                        + ": storage:" + storageInfos[1] + ", used:" + storageInfos[0]);
                if (storageInfos[1] > 0) {
                    sAvailableStorageMap.put(subscriptionInfo.getSubscriptionId(),
                            storageInfos[1] - storageInfos[0]);
                }
            }
            if (mNeedToastIfFull
                    && mSubIdToShow != SubInfoUtils.getInvalidSubId()
                    && 0 == ShowSimCardStorageInfoTask.getAvailableCount(mSubIdToShow)) {
                return true;
            }
            return false;
        }

        @Override
        protected void onPostExecute(Boolean needToast) {
            if (needToast) {
                Toast.makeText(mContext, R.string.storage_full, Toast.LENGTH_LONG).show();
            }
        }

        public static int getAvailableCount(int subId) {
            if (null != sAvailableStorageMap && sAvailableStorageMap.containsKey(subId)) {
                int result = sAvailableStorageMap.get(subId);
                Log.d(TAG, "[getAvailableCount] result : " + result + ",subId = " + subId);
                return result;
            } else {
                Log.i(TAG, "[getAvailableCount] return -1");
                return -1;
            }
        }
    }
}
