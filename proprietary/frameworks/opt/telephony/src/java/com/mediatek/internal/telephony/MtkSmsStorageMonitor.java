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

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.ContentObserver;
import android.net.Uri;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.SystemProperties;
import android.os.UserHandle;
import android.os.UserManager;
import android.provider.Settings;
import android.provider.Telephony.Sms.Intents;
import android.telephony.Rlog;
import android.telephony.SubscriptionManager;
import com.android.internal.content.PackageMonitor;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.SmsApplication;
import com.android.internal.telephony.SmsStorageMonitor;

/**
 * Monitors the device and ICC storage, and sends the appropriate events.
 *
 * This code was formerly part of {@link SMSDispatcher}, and has been moved
 * into a separate class to support instantiation of multiple SMSDispatchers on
 * dual-mode devices that require support for both 3GPP and 3GPP2 format messages.
 */
public class MtkSmsStorageMonitor extends SmsStorageMonitor {
    private static final String TAG = "MtkSmsStorageMonitor";

    // MTK-START
    /** ME storage is full and receiving a new SMS from network */
    private static final int EVENT_ME_FULL = 100;
    // MTK-END

    private boolean mPendingIccFullNotify = false;
    private ContentObserver mContentObserver;

    private final PackageMonitor mPackageMonitor = new PackageMonitor() {
        @Override
        public void onPackageModified(String packageName) {
            if (mPendingIccFullNotify) {
                handleIccFull();
            }
        }
    };

    /**
     * Creates an SmsStorageMonitor and registers for events.
     * @param phone the Phone to use
     */
    public MtkSmsStorageMonitor(Phone phone) {
        super(phone);
        if (mCi != null && mCi instanceof MtkRIL) {
            MtkRIL ci = (MtkRIL)mCi;
            ci.setOnMeSmsFull(this, EVENT_ME_FULL, null);
        }
        IntentFilter filter = new IntentFilter();
        filter.addAction(Intent.ACTION_BOOT_COMPLETED);
        mContext.registerReceiver(mMtkResultReceiver, filter);
        final Uri defaultSmsAppUri = Settings.Secure.getUriFor(
                Settings.Secure.SMS_DEFAULT_APPLICATION);
        mContentObserver = new ContentObserver(
                new Handler(Looper.getMainLooper())) {
            @Override
            public void onChange(boolean selfChange, Uri uri, int userId) {
                if (mPendingIccFullNotify) {
                    handleIccFull();
                }
            }
        };
        mContext.getContentResolver().registerContentObserver(defaultSmsAppUri,
                false, mContentObserver, UserHandle.USER_ALL);
        mPackageMonitor.register(mContext, mContext.getMainLooper(), UserHandle.ALL, false);
    }

    public void dispose() {
        Rlog.d(TAG, "disposed...");
        if (mCi != null && mCi instanceof MtkRIL) {
            MtkRIL ci = (MtkRIL)mCi;
            ci.unSetOnMeSmsFull(this);
        }
        mContext.unregisterReceiver(mMtkResultReceiver);
        mContext.getContentResolver().unregisterContentObserver(mContentObserver);
        mPackageMonitor.unregister();
        super.dispose();
    }

    @Override
    public void handleIccFull() {
        ComponentName name = SmsApplication.getDefaultSimFullApplication(mContext, false);
        if (name == null) {
            mPendingIccFullNotify = true;
            Rlog.d(TAG, "ComponentName is NULL");
            return;
        }

        UserManager userManager = (UserManager)mContext.getSystemService(Context.USER_SERVICE);
        if (!userManager.isUserUnlocked()) {
            mPendingIccFullNotify = true;
            Rlog.d(TAG, "too early, wait for boot complete to send broadcast");
            return;
        }

        mPendingIccFullNotify = false;
        Rlog.d(TAG, "handleIccFull");
        super.handleIccFull();
    }

    /**
     * Handles events coming from the phone stack. Overridden from handler.
     * @param msg the message to handle
     */
    @Override
    public void handleMessage(Message msg) {
        AsyncResult ar;

        switch (msg.what) {
            case EVENT_RADIO_ON:
                // MTK-START
                /***********************************************************
                 * There are 2 possible  scenarios will turn off modem
                 * 1) MTK_FLIGHT_MODE_POWER_OFF_MD, AP turns off modem while flight mode
                 * 2) MTK_RADIOOFF_POWER_OFF_MD, AP turns off modem while radio off
                 * In next time modem power on, it will missing stroage full notification,
                 * we need to re-send this notification while radio on
                 **********************************************************/
                // if (mReportMemoryStatusPending) {
                {
                // MTK-END
                    Rlog.v(TAG, "Sending pending memory status report : mStorageAvailable = "
                            + mStorageAvailable);
                    mCi.reportSmsMemoryStatus(mStorageAvailable,
                            obtainMessage(EVENT_REPORT_MEMORY_STATUS_DONE));
                }
                break;

            // MTK-START
            case EVENT_ME_FULL:
                handleMeFull();
                break;
            // MTK-END
            default:
                super.handleMessage(msg);
                break;
        }
    }

    // MTK-START
    /**
     * Called when ME_FULL message is received from the RIL.  Notifies interested
     * parties that ME storage for SMS messages is full.
     */
    private void handleMeFull() {
        // broadcast SMS_REJECTED_ACTION intent
        Intent intent = new Intent(Intents.SMS_REJECTED_ACTION);
        intent.putExtra("result", Intents.RESULT_SMS_OUT_OF_MEMORY);
        SubscriptionManager.putPhoneIdAndSubIdExtra(intent, mPhone.getPhoneId());
        mWakeLock.acquire(WAKE_LOCK_TIMEOUT);
        mContext.sendBroadcast(intent, android.Manifest.permission.RECEIVE_SMS);
    }
    // MTK-END

    private final BroadcastReceiver mMtkResultReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent.getAction().equals(Intent.ACTION_BOOT_COMPLETED)) {
                if (mPendingIccFullNotify) {
                    handleIccFull();
                }
            }
        }
    };
}
