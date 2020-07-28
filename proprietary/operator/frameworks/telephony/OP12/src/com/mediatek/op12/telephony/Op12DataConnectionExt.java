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

package com.mediatek.op12.telephony;

import android.content.Context;
import android.content.Intent;
import android.os.AsyncResult;
import android.os.SystemProperties;
import android.telephony.data.ApnSetting;
import android.telephony.PcoData;
import android.telephony.Rlog;
import android.text.TextUtils;

import com.android.internal.telephony.dataconnection.ApnContext;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;

import com.mediatek.internal.telephony.dataconnection.DataConnectionExt;
import com.mediatek.internal.telephony.dataconnection.PcoDataAfterAttached;

import java.util.ArrayList;

public class Op12DataConnectionExt extends DataConnectionExt {
    static final String TAG = "Op12DataConnectionExt";

    public Op12DataConnectionExt(Context context) {
        super(context);
    }

    /**
     * handle PCO data after PS attached
     */
    @Override
    public void handlePcoDataAfterAttached(final AsyncResult ar, final Phone phone,
            final ArrayList<ApnSetting> settings) {
        if (ar == null) {
            Rlog.e(TAG, "handlePcoDataAfterAttached ar is null");
            return;
        } else if (ar.exception != null) {
            Rlog.e(TAG, "handlePcoDataAfterAttached PCO_DATA exception: " + ar.exception);
            return;
        } else if (phone == null) {
            Rlog.e(TAG, "handlePcoDataAfterAttached phone is null");
            return;
        }

        PcoDataAfterAttached pcoData = (PcoDataAfterAttached)(ar.result);
        if (pcoData == null) {
            Rlog.d(TAG, "handlePcoDataAfterAttached pco status is null");
            return;
        }

        Rlog.d(TAG, "handlePcoDataAfterAttached " + pcoData.toString());
        Rlog.d(TAG, "handlePcoDataAfterAttached pcoData.contents first byte: "
               + pcoData.contents[0]);

        if (pcoData.apnName == null || pcoData.apnName.isEmpty()) {
            Rlog.d(TAG, "handlePcoDataAfterAttached apn name null, send value only");
            sendPcoIdAndContentOnly(phone, pcoData);
            return;
        }

        String[] aryApnType = getApnType(pcoData.apnName, settings);
        if (aryApnType == null) {
            Rlog.d(TAG, "handlePcoDataAfterAttached getApnType() return null, send value only");
            sendPcoIdAndContentOnly(phone, pcoData);
            return;
        }

        for (String apnType: aryApnType) {
            Rlog.d(TAG, "handlePcoDataAfterAttached send intent for apn type: " + apnType);
            final Intent intent = new Intent(TelephonyIntents.ACTION_CARRIER_SIGNAL_PCO_VALUE);
            intent.putExtra(TelephonyIntents.EXTRA_APN_TYPE_KEY, apnType);
            intent.putExtra(TelephonyIntents.EXTRA_APN_PROTO_KEY, pcoData.bearerProto);
            intent.putExtra(TelephonyIntents.EXTRA_PCO_ID_KEY, pcoData.pcoId);
            intent.putExtra(TelephonyIntents.EXTRA_PCO_VALUE_KEY, pcoData.contents);
            phone.getCarrierSignalAgent().notifyCarrierSignalReceivers(intent);
        }
    }

    private void sendPcoIdAndContentOnly(Phone phone, PcoDataAfterAttached pcoData) {
        final Intent intent = new Intent(TelephonyIntents.ACTION_CARRIER_SIGNAL_PCO_VALUE);
        intent.putExtra(TelephonyIntents.EXTRA_APN_TYPE_KEY, "");
        intent.putExtra(TelephonyIntents.EXTRA_APN_PROTO_KEY, "");
        intent.putExtra(TelephonyIntents.EXTRA_PCO_ID_KEY, pcoData.pcoId);
        intent.putExtra(TelephonyIntents.EXTRA_PCO_VALUE_KEY, pcoData.contents);
        phone.getCarrierSignalAgent().notifyCarrierSignalReceivers(intent);
    }

    private String[] getApnType(String apnName, final ArrayList<ApnSetting> settings) {
        ApnSetting apn;
        if (settings != null) {
            for (int i = 0; i < settings.size(); i++) {
                apn = settings.get(i);

                if (apnMatcher(apnName, apn.getApnName())) {
                    return ApnSetting.getApnTypesStringFromBitmask(
                            apn.getApnTypeBitmask()).split(",");
                }
            }
        }
        return null;
    }

    // Porting from native layer.
    // apn is from MD
    // compareapn is from allApnSettings.
    private boolean apnMatcher(String apn, String compareApn) {
        //The APN is composed of two parts as follows:
        //The APN Network Identifier & The APN Operator Identifier
        //The APN Operator Identifier is in the format of "mnc<MNC>.mcc<MCC>.gprs"
        //The valid APN format: <apn>[.mnc<MNC>.mcc<MCC>[.gprs]]
        boolean ret = false;
        if (apn.length() < compareApn.length()) {
            Rlog.d(TAG, "apnMatcher: length not match.");
            return ret;
        }

        String tmpApn = apn.toLowerCase();
        String tmpCompareApn = compareApn.toLowerCase();

        ret = tmpApn.equals(tmpCompareApn) ? true : tmpApn.indexOf(compareApn) != -1;

        if (ret) {
            if (!((tmpApn.length() == tmpCompareApn.length())
                    || tmpApn.startsWith(".mnc", tmpCompareApn.length())
                    || tmpApn.startsWith(".mcc", tmpCompareApn.length()))) {
                Rlog.d(TAG, "apnMatcher: apns are equal but format unexpected");
                ret = false;
            }
        }
        if (tmpApn.indexOf(compareApn) != -1
                && tmpApn.startsWith(".", tmpCompareApn.length())) {
            Rlog.d(TAG, "apnMatcher: apn are equal as apn.xxx type");
            ret = true;
        }

        Rlog.d(TAG, "apnMatcher: match: " + ret);
        return ret;
    }

    private boolean mIsPcoAllowedDefault = true;

    /**
     * Check if 'default' type PDN is allowed based on the current PCO value
     */
    @Override
    public boolean getIsPcoAllowedDefault() {
        return mIsPcoAllowedDefault;
    }

    /**
     * Set if 'default' type PDN is allowed
     */
    @Override
    public void setIsPcoAllowedDefault(boolean allowed) {
        mIsPcoAllowedDefault = allowed;
    }

    /**
     * Perform operator's specific PCO actions according to current PCO value and apn type
     */
    @Override
    public int getPcoActionByApnType(ApnContext apnContext, PcoData pcoData) {
        String apnType = apnContext.getApnType();
        if (TextUtils.equals(apnType, PhoneConstants.APN_TYPE_DEFAULT)) {
            if (pcoData.contents[0] == (byte) 0x33) {
                // Receiving PCO3 by default will not tear down internet PDN,
                // but can be customized such that the tear down can be controlled
                // the by apps.
                if (SystemProperties.getInt("persist.vendor.pco3.teardown.internet", 0) == 1) {
                    Rlog.d(TAG, "getPcoActionByApnType PCO:3 disable and teardown default PDN");
                    setIsPcoAllowedDefault(false);
                    return 1;
                }
            } else if (pcoData.contents[0] == (byte) 0x30) {
                Rlog.d(TAG, "getPcoActionByApnType PCO:0 setIsPcoAllowedDefault true");
                setIsPcoAllowedDefault(true);
            }
        }
        return 0;
    }
}
