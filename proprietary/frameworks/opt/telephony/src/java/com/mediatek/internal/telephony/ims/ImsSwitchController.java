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

import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.content.Context;
import android.telephony.Rlog;
import android.telephony.TelephonyManager;
import com.android.ims.ImsManager;
import com.android.internal.telephony.CommandsInterface;

import com.mediatek.ims.internal.IMtkImsService;
import com.mediatek.ims.internal.MtkImsManager;

public class ImsSwitchController extends Handler  {
    static final String LOG_TAG = "ImsSwitchController";
    /// @}
    private Context mContext;

    // deprecated after Android O
    private CommandsInterface[] mCi;
    // deprecated after Android O
    private int mPhoneCount;
    private static IMtkImsService mMtkImsService = null;
    private RadioPowerInterface mRadioPowerIf;
    private ImsServiceDeathRecipient mDeathRecipient = new ImsServiceDeathRecipient();

    private static final int BIND_IMS_SERVICE_DELAY_IN_MILLIS = 2000;

    ImsSwitchController(Context context , int phoneCount, CommandsInterface[] ci) {
        log("Initialize ImsSwitchController");
        mContext = context;
        mCi = ci;
        mPhoneCount = phoneCount;
        // For TC1, do not use MTK IMS stack solution
        if (SystemProperties.get("persist.vendor.ims_support").equals("1") &&
            !SystemProperties.get("ro.vendor.mtk_tc1_feature").equals("1")) {
            mRadioPowerIf = new RadioPowerInterface();
            RadioManager.registerForRadioPowerChange(LOG_TAG, mRadioPowerIf);
            if (mMtkImsService == null) {
                checkAndBindImsService(0);
            }
        }
    }

    class RadioPowerInterface implements IRadioPower {
        public void notifyRadioPowerChange(boolean power, int phoneId) {
            log("notifyRadioPowerChange, power:" + power + " phoneId:" + phoneId);

            if (MtkImsManager.isSupportMims() == false) {
                if (RadioCapabilitySwitchUtil.getMainCapabilityPhoneId() != phoneId) {
                    log("radio power change ignore due to phone id isn't LTE phone");
                    return;
                }
            }

            if (SystemProperties.get("ro.vendor.md_auto_setup_ims").equals("1")) {
                log("[" + phoneId
                        + "] Modem auto registration so that we don't trigger"
                        + "ImsService updateRadioState");
                return;
            }

            if (mMtkImsService == null) {
                checkAndBindImsService(phoneId);
            }

            if (mMtkImsService != null) {
                try {
                    int radioState = (power ?
                            TelephonyManager.RADIO_POWER_ON : TelephonyManager.RADIO_POWER_OFF);
                    mMtkImsService.updateRadioState(radioState, phoneId);
                } catch (RemoteException e) {
                    Rlog.e(LOG_TAG, "RemoteException can't notify power state change");
                }
            } else {
                Rlog.w(LOG_TAG, "notifyRadioPowerChange: ImsService not ready !!!");
            }
            log("radio power change processed");
        }
    }

    /**
     * Death recipient class for monitoring IMS service.
     *
     * @param phoneId  to indicate which phone.
     * @return true if bind successfully
     */
    private boolean checkAndBindImsService(int phoneId) {
        IBinder b = ServiceManager.getService(MtkImsManager.MTK_IMS_SERVICE);
        if (b != null) {
            try {
                b.linkToDeath(mDeathRecipient, 0);
            } catch (RemoteException e) {
                return false;
            }
        } else {
            return false;
        }
        mMtkImsService = IMtkImsService.Stub.asInterface(b);
        log("checkAndBindImsService: mMtkImsService = " + mMtkImsService);
        return true;
    }

    /**
     * Death recipient class for monitoring IMS service.
     */
    private class ImsServiceDeathRecipient implements IBinder.DeathRecipient {
        @Override
        public void binderDied() {
            Rlog.w(LOG_TAG, "ImsService died detected");
            mMtkImsService = null;
            // Post this runnable so we will automatically bind to ImsService again
            postDelayed(mBindImsServiceRunnable, BIND_IMS_SERVICE_DELAY_IN_MILLIS);
        }
    }

    // A runnable which is used to automatically bind to ImsService in case it died
    private Runnable mBindImsServiceRunnable = new Runnable() {
        @Override
        public void run() {
            Rlog.w(LOG_TAG, "try to bind ImsService again");
            if (checkAndBindImsService(0) == false) {
                postDelayed(this, BIND_IMS_SERVICE_DELAY_IN_MILLIS);
            } else {
                log("manually updateImsServiceConfig");
                if (MtkImsManager.isSupportMims() == false) {
                    int phoneId = RadioCapabilitySwitchUtil.getMainCapabilityPhoneId();
                    ImsManager.updateImsServiceConfig(mContext, phoneId, true);
                } else {
                    for (int i = 0; i < TelephonyManager.getDefault().getPhoneCount(); i++) {
                        ImsManager.updateImsServiceConfig(mContext, i, true);
                    }
                }
            }
        }
    };

    private static void log(String s) {
        Rlog.d(LOG_TAG, s);
    }
}

