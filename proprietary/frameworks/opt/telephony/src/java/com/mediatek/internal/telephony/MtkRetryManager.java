/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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

package com.mediatek.internal.telephony;

import android.os.IBinder;
import android.os.ServiceManager;
import android.os.RemoteException;

import android.content.Context;
import android.telephony.CarrierConfigManager;
import android.telephony.data.ApnSetting;
import android.telephony.Rlog;
import android.telephony.SubscriptionManager;
import android.text.TextUtils;

import android.telephony.TelephonyManager;
import android.net.NetworkCapabilities;
import android.net.NetworkInfo;

// M: Data Framework - Data Retry enhancement @{
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.RetryManager;
import com.android.internal.telephony.PhoneConstants;
import com.mediatek.internal.telephony.dataconnection.DcFailCauseManager;
import com.android.internal.telephony.TelephonyDevController;
import com.mediatek.internal.telephony.MtkHardwareConfig;
import android.content.BroadcastReceiver;
import android.content.Intent;
import android.content.IntentFilter;
// M: Data Framework - Data Retry enhancement @}

import com.android.ims.ImsManager;
import com.mediatek.ims.internal.IMtkImsService;
import com.mediatek.ims.internal.MtkImsManager;

import java.util.ArrayList;
import java.util.concurrent.atomic.AtomicInteger;

public final class MtkRetryManager extends RetryManager {
    public static final String LOG_TAG = "MtkRetryManager";

    // M: Data Framework - Data Retry enhancement @{
    private DcFailCauseManager mDcFcMgr;

    // M: IMS Data retry manager configuration
    private TelephonyDevController mTelDevController = TelephonyDevController.getInstance();
    private static IMtkImsService mMtkImsService = null;
    private int mPhoneNum;

    // M: Handle AOSP CarrierConfig loaded timing issue
    private Context mContext;
    private boolean mBcastRegistered = false;
    private BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        private final AtomicInteger mPreviousSubId =
                new AtomicInteger(SubscriptionManager.INVALID_SUBSCRIPTION_ID);

        @Override
        public void onReceive(Context context, Intent intent) {
            if (DBG) Rlog.d(LOG_TAG,"mBroadcastReceiver: action " + intent.getAction()
                + ", mSameApnRetryCount:" + mSameApnRetryCount
                + ", mModemSuggestedDelay:" + mModemSuggestedDelay
                + ", mCurrentApnIndex:" + mCurrentApnIndex);
            if (intent.getAction().equals(CarrierConfigManager.ACTION_CARRIER_CONFIG_CHANGED)) {
               int subId = intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY,
                       SubscriptionManager.INVALID_SUBSCRIPTION_ID);
               if (SubscriptionManager.isValidSubscriptionId(subId)
                       && subId == mPhone.getSubId()
                       && mPreviousSubId.getAndSet(subId) != subId) {
                    int sameApnRetryCountTemp = mSameApnRetryCount;
                    long modemSuggestedDelayTemp = mModemSuggestedDelay;
                    int currentApnIndexTemp = mCurrentApnIndex;
                    configureRetry();
                    mSameApnRetryCount = sameApnRetryCountTemp;
                    mModemSuggestedDelay = modemSuggestedDelayTemp;
                    mCurrentApnIndex = currentApnIndexTemp;
                }
            }
        }
    };
    static {
        // Enlarge MAX_SAME_APN_RETRY as 100 to fulfill the requirement of ePDG and MPS features.
        MAX_SAME_APN_RETRY = 100;
    }
    // M: Data Framework - Data Retry enhancement @}

    /**
     * MTK Retry manager constructor
     * @param phone Phone object
     * @param apnType APN type
     */
    public MtkRetryManager(Phone phone, String apnType) {
        super(phone, apnType);

        mPhoneNum = TelephonyManager.getDefault().getPhoneCount();

        // M: Data Framework - Data Retry enhancement @{
        // M: Initialize data connection fail cause manager
        mDcFcMgr = DcFailCauseManager.getInstance(mPhone);

        // M: IMS Data retry manager configuration
        if (mMtkImsService == null) {
            checkAndBindImsService();
        }
    }

    // M: Data Framework - Data Retry enhancement
    @Override
    protected void finalize() {
        if (DBG) Rlog.d(LOG_TAG, "RetryManager finalized");
        mContext.unregisterReceiver(mBroadcastReceiver);
    }

    /**
     * Configure the retry manager
     */
    @Override
    protected void configureRetry() {
        super.configureRetry();

        String configString = null;

        // M: Data Framework - Data Retry enhancement @{
        // When frameworks try to setup normal data call, it will reset data retry configuration
        // We should reset MD data retry count.
        if (mWaitingApns != null && mWaitingApns.size() != 0) {
            int index = mCurrentApnIndex;
            if (index < 0 || index >= mWaitingApns.size()) {
                index = 0;
            }
            if (DBG) Rlog.d(LOG_TAG, "configureRetry: mCurrentApnIndex: " + mCurrentApnIndex
                + ", reset MD data count for apn: " + mWaitingApns.get(index).getApnName());
            ((MtkRIL)mPhone.mCi).resetMdDataRetryCount(mWaitingApns.get(index).getApnName(), null);
        } else {
            Rlog.e(LOG_TAG, "configureRetry: mWaitingApns is null or empty");
        }
        // M: Data Framework - Data Retry enhancement @}

        // M: IMS Data retry manager configuration
        //    disable retry of ims in 93MD @{
        if(mTelDevController != null && mTelDevController.getModem(0) != null
                && ((MtkHardwareConfig)mTelDevController.getModem(0))
                  .hasMdAutoSetupImsCapability() == true) {
            if (TextUtils.equals(PhoneConstants.APN_TYPE_IMS, mApnType) ||
                    TextUtils.equals(PhoneConstants.APN_TYPE_EMERGENCY, mApnType)) {
                Rlog.d(LOG_TAG, "configureRetry: IMS/EIMS, no retry by mobile.");
                configString = "max_retries=0, -1, -1, -1";
                configure(configString);
            }
        }// @}
    }

    /**
     * Get the delay for trying the next waiting APN from the list.
     * @param failFastEnabled True if fail fast mode enabled. In this case we'll use a shorter
     *                        delay.
     * @return delay in milliseconds
     */
    @Override
    public long getDelayForNextApn(boolean failFastEnabled) {

        if (mWaitingApns == null || mWaitingApns.size() == 0) {
            log("Waiting APN list is null or empty.");
            return NO_RETRY;
        }

        if (mModemSuggestedDelay == NO_RETRY) {
            log("Modem suggested not retrying.");
            return NO_RETRY;
        }

        if (mModemSuggestedDelay != NO_SUGGESTED_RETRY_DELAY &&
                mSameApnRetryCount < MAX_SAME_APN_RETRY) {
            // M: Data Framework - CC 33 @{
            // Failed cause is 33/29
            if (mModemSuggestedDelay
                == DcFailCauseManager.retryConfigForCC33.retryTime.getValue()) {
                if (mDcFcMgr != null
                    && mDcFcMgr.isNetworkOperatorForCC33()
                    && (mSameApnRetryCount
                        >= DcFailCauseManager.retryConfigForCC33.maxRetryCount.getValue())) {
                    return NO_RETRY;
                }
            }
            // M: Data Framework - CC 33 @}

            // If the modem explicitly suggests a retry delay, we should use it, even in fail fast
            // mode.
            log("Modem suggested retry in " + mModemSuggestedDelay + " ms.");
            return mModemSuggestedDelay;
        }

        // In order to determine the delay to try next APN, we need to peek the next available APN.
        // Case 1 - If we will start the next round of APN trying,
        //    we use the exponential-growth delay. (e.g. 5s, 10s, 30s...etc.)
        // Case 2 - If we are still within the same round of APN trying,
        //    we use the fixed standard delay between APNs. (e.g. 20s)

        int index = mCurrentApnIndex;
        while (true) {
            if (++index >= mWaitingApns.size()) index = 0;

            // Stop if we find the non-failed APN.
            if (!mWaitingApns.get(index).getPermanentFailed()) {
                break;
            }

            // If we've already cycled through all the APNs, that means all APNs have
            // permanently failed
            if (index == mCurrentApnIndex) {
                log("All APNs have permanently failed.");
                return NO_RETRY;
            }
        }

        long delay;
        if (index <= mCurrentApnIndex) {
            // Case 1, if the next APN is in the next round.
            if (!mRetryForever && mRetryCount + 1 > mMaxRetryCount) {
                log("Reached maximum retry count " + mMaxRetryCount + ".");
                return NO_RETRY;
            }
            delay = getRetryTimer();
            ++mRetryCount;
        } else {
            // Case 2, if the next APN is still in the same round.
            delay = mInterApnDelay;
        }

        if (failFastEnabled && delay > mFailFastInterApnDelay) {
            // If we enable fail fast mode, and the delay we got is longer than
            // fail-fast delay (mFailFastInterApnDelay), use the fail-fast delay.
            // If the delay we calculated is already shorter than fail-fast delay,
            // then ignore fail-fast delay.
            delay = mFailFastInterApnDelay;
        }

        return delay;
    }

    @Override
    public void setWaitingApns(ArrayList<ApnSetting> waitingApns) {
        super.setWaitingApns(waitingApns);

        // M: Handle AOSP CarrierConfig loaded timing issue
        mContext = mPhone.getContext();
        if (!mBcastRegistered) {
            mContext.registerReceiver(mBroadcastReceiver, new IntentFilter(
                    CarrierConfigManager.ACTION_CARRIER_CONFIG_CHANGED));
            mBcastRegistered = true;
        }
        // M: Data Framework - Data Retry enhancement @}
    }

    /**
     * Bind IMS service to use API.
     *
     * @return non if bind unsuccessfully
     */
    private void checkAndBindImsService() {
        IBinder b = ServiceManager.getService(MtkImsManager.MTK_IMS_SERVICE);
        if (b == null) {
             return;
        }

        mMtkImsService = IMtkImsService.Stub.asInterface(b);
        if (mMtkImsService == null) {
             return;
        }

        Rlog.d(LOG_TAG, "checkAndBindImsService: mMtkImsService = " + mMtkImsService);
    }
}
