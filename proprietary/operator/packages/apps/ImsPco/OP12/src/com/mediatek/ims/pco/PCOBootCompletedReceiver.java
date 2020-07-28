/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.ims.pco;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;
import android.os.SystemProperties;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.PhoneConstants;
import com.mediatek.ims.pco.ui.PCOStartUPWizardMainActivity;

import static com.mediatek.ims.pco.PCOConfig.DEBUG;
/**
 *  Handles BOOT_COMPLETED intent or notification, and start 'PCO service' on
 *  branded phones.
 */
public class PCOBootCompletedReceiver extends BroadcastReceiver {

    private static final String TAG = "PCOBootCompletedReceiver";
    private static final String PROPERTY_OP12_DEVICE_MODEL = "persist.op12.model";
    private static final String OP12_BRANDED_DEVICE = "0";
    private static final String PCO_SERVICE = "pco";
    private static Context mContext;

    private static boolean isOp12BrandedDevice() {
       return (OP12_BRANDED_DEVICE.equalsIgnoreCase(
                SystemProperties.get(PROPERTY_OP12_DEVICE_MODEL))) ? true : false;

    }

    @Override
    public void onReceive(Context context, Intent intent) {
        mContext = context;
        PCONwUtils.setAppContext(mContext);
        boolean mIsBrandedDevice = isOp12BrandedDevice();
        log("onReceive: mIsBrandedDevice = " + mIsBrandedDevice);
        if (mIsBrandedDevice == false){
            return;
        }
        if (intent == null) {
            return;
        }
        if (TelephonyIntents.ACTION_CARRIER_SIGNAL_PCO_VALUE.equals(intent.getAction())) {
            Log.i(TAG, "onReceive ACTION_CARRIER_SIGNAL_PCO_VALUE");
            handlePCOcase(context, intent);
        }
    }

    public void handlePCOcase(Context context, Intent intent) {
        Log.i(TAG, "handlePCOcase start");
        String apnType =
                intent.getStringExtra(TelephonyIntents.EXTRA_APN_TYPE_KEY);
        String protoKeyAPN = intent.getStringExtra(TelephonyIntents.EXTRA_APN_PROTO_KEY);
        int pcoIDKey = intent.getIntExtra(TelephonyIntents.EXTRA_PCO_ID_KEY, 0);
        byte[] pcoContent = intent.getByteArrayExtra(TelephonyIntents.EXTRA_PCO_VALUE_KEY);
        if (pcoContent == null || apnType == null) {
            Log.i(TAG, "handlePCOcase pcoContent == null || apnType == null");
            return;
        }

        byte pcoVal = (byte) pcoContent[0];
        int pcoValInt = pcoVal - '0';
        log("apn=" + apnType + "protoKeyAPN=" + protoKeyAPN
                + "pcoVal =" + pcoVal + " pcoValInt = " + pcoValInt);
        switch (apnType) {
        case PhoneConstants.APN_TYPE_IMS: {
            if (pcoValInt == 5) {
                handlePCO5case();
            } else if (pcoValInt == 0) {
//                handlePCO3case();

            }
        } break;
        default:
            break;
        }

    }

    private void handlePCO5case() {
        log("Handling IMS PCO error case = 5");
        sendMessagetoUI(Constants.ACTION_SIGN_UP,
                Constants.NOT_ACTIVATED,
                Constants.USE_MOBILE_NW_ONLY,
                Constants.SCREEN_SHOW_URL_DIALOGUE);
    }
    private void sendMessagetoUI(String mAction,
            String mActStatus,
            String mNWToUse,
            String mScreentype) {

        log("[==>]sendMessagetoUI" + ", Action=" + mAction
                + ", mNWToUse =" + mNWToUse + ", Screentype=" + mScreentype);
        Intent i = new Intent(mContext, PCOStartUPWizardMainActivity.class);
        i.setAction(mAction);
        i.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TASK);
        i.putExtra(Constants.EXTRA_ACTIVATION_STATUS, mActStatus);
        i.putExtra(Constants.EXTRA_NETWORK_TYPE_TO_USE, mNWToUse);
        i.putExtra(Constants.EXTRA_SCREEN_TYPE, mScreentype);
        mContext.startActivity(i);
    }
    private static void log(String s) {
        if (DEBUG) {
            Log.d(TAG, s);
        }
    }

}
