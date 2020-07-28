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
package com.mediatek.internal.telephony.carrierexpress;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.Message;
import android.os.SystemProperties;
import android.telephony.Rlog;
import android.telephony.TelephonyManager;

import com.android.internal.telephony.CommandsInterface;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.mediatek.common.carrierexpress.CarrierExpressManager;
import com.mediatek.internal.telephony.MtkRIL;
import com.mediatek.internal.telephony.MtkRILConstants;
import vendor.mediatek.hardware.mtkradioex.V1_0.VendorSetting;

import static com.android.internal.util.Preconditions.checkNotNull;

public class CarrierExpressFwkHandler extends Handler{
    private static final String LOG_TAG = "CarrierExpress";

    /** The singleton instance. */
    private static CarrierExpressFwkHandler sInstance = null;
    private Phone mPhone = null;
    private MtkRIL mCi;
    private Context mContext;
    private static final String ACTION_CXP_SET_VENDOR_PROP =
        "com.mediatek.common.carrierexpress.cxp_set_vendor_prop";

    public CarrierExpressFwkHandler() {
        try {
            mPhone = PhoneFactory.getDefaultPhone();
        } catch (IllegalStateException e) {
            Rlog.e(LOG_TAG, "failed to get default phone from PhoneFactory: " + e.toString());
        }
        checkNotNull(mPhone, "default phone is null");
        mContext = mPhone.getContext();
        checkNotNull(mContext, "missing Context");
        IntentFilter filter = new IntentFilter();
        filter.addAction(CarrierExpressManager.ACTION_CXP_RESET_MODEM);
        filter.addAction(ACTION_CXP_SET_VENDOR_PROP);
        mContext.registerReceiver(mCarrierExpressReceiver, filter);
    }
    /**
     * Initialize the singleton CarrierExpressFwkHandler instance.
     * This is only done once, at startup, from PhoneFactory.makeDefaultPhone().
     */
    public static void init() {
        synchronized (CarrierExpressFwkHandler.class) {
            if (sInstance == null) {
                sInstance = new CarrierExpressFwkHandler();
            } else {
                Rlog.d(LOG_TAG, "init() called multiple times!  sInstance = " + sInstance);
            }
        }
    }

    private final BroadcastReceiver mCarrierExpressReceiver = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Rlog.d(LOG_TAG, "BroadcastReceiver(), action= " + action);

            if (mPhone != null) {
                mCi = (MtkRIL)mPhone.mCi;
            } else {
                Rlog.e(LOG_TAG, "phone is null, cannot reset modem");
                return;
            }
            if (mCi == null) {
                Rlog.e(LOG_TAG, "MtkRIL is null");
                return;
            }

            if (CarrierExpressManager.ACTION_CXP_RESET_MODEM.equals(action)) {
                startResetModem();
            } else if (ACTION_CXP_SET_VENDOR_PROP.equals(action)) {
                mCi.setVendorSetting(VendorSetting.VENDOR_SETTING_CXP_CONFIG_OPTR,
                    intent.getStringExtra("OPTR"), null);
                mCi.setVendorSetting(VendorSetting.VENDOR_SETTING_CXP_CONFIG_SPEC,
                    intent.getStringExtra("SPEC"), null);
                mCi.setVendorSetting(VendorSetting.VENDOR_SETTING_CXP_CONFIG_SEG,
                    intent.getStringExtra("SEG"), null);
                mCi.setVendorSetting(VendorSetting.VENDOR_SETTING_CXP_CONFIG_SBP,
                    intent.getStringExtra("SBP"), null);
                mCi.setVendorSetting(VendorSetting.VENDOR_SETTING_CXP_CONFIG_SUBID,
                    intent.getStringExtra("SUBID"), null);
            }
        }
    };

    private void startResetModem(){
        if (mCi != null) {
            mCi.restartRILD(null);
            Rlog.d(LOG_TAG, "Reset modem");
        } else {
            Rlog.e(LOG_TAG, "MtkRIL is null, cannot reset modem");
        }
    }

    void dispose(){
        mContext.unregisterReceiver(mCarrierExpressReceiver);
    }
}
