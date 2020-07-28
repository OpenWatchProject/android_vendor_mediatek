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

import android.hardware.radio.V1_0.RadioResponseInfo;

import com.android.internal.telephony.RadioResponse;
import com.android.internal.telephony.RIL;
import com.android.internal.telephony.RILRequest;
import com.android.internal.telephony.ServiceStateTracker;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneFactory;

import android.content.Context;
import android.os.RemoteException;
import android.os.SystemProperties;
import android.text.TextUtils;
import android.telephony.SubscriptionManager;
import android.telephony.ServiceState;
import android.util.Log;

import java.util.ArrayList;

public class MtkRadioResponse extends RadioResponse {

    // TAG
    private static final String TAG = "MtkRadioResp";
    private MtkRIL mMtkRil;

    public MtkRadioResponse(RIL ril) {
        super(ril);
        mMtkRil = (MtkRIL)ril;
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     */
    @Override
    public void switchWaitingOrHoldingAndActiveResponse(RadioResponseInfo responseInfo) {
        mMtkRil.riljLog("clear mIsSendChldRequest");
        mMtkRil.mDtmfReqQueue.resetSendChldRequest();
        super.switchWaitingOrHoldingAndActiveResponse(responseInfo);
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     */
    @Override
    public void conferenceResponse(RadioResponseInfo responseInfo) {
        mMtkRil.riljLog("clear mIsSendChldRequest");
        mMtkRil.mDtmfReqQueue.resetSendChldRequest();
        super.conferenceResponse(responseInfo);
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     */
    @Override
    public void startDtmfResponse(RadioResponseInfo responseInfo) {
        mMtkRil.handleDtmfQueueNext(responseInfo.serial);
        super.startDtmfResponse(responseInfo);
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     */
    @Override
    public void stopDtmfResponse(RadioResponseInfo responseInfo) {
        mMtkRil.handleDtmfQueueNext(responseInfo.serial);
        super.stopDtmfResponse(responseInfo);
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     */
    @Override
    public void separateConnectionResponse(RadioResponseInfo responseInfo) {
        mMtkRil.riljLog("clear mIsSendChldRequest");
        mMtkRil.mDtmfReqQueue.resetSendChldRequest();
        super.separateConnectionResponse(responseInfo);
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     */
    @Override
    public void explicitCallTransferResponse(RadioResponseInfo responseInfo) {
        mMtkRil.riljLog("clear mIsSendChldRequest");
        mMtkRil.mDtmfReqQueue.resetSendChldRequest();
        super.explicitCallTransferResponse(responseInfo);
    }

    private int getSubId(int phoneId) {
        int subId = SubscriptionManager.INVALID_SUBSCRIPTION_ID;
        int[] subIds = SubscriptionManager.getSubId(phoneId);
        if (subIds != null && subIds.length > 0) {
            subId = subIds[0];
        }
        return subId;
    }

    @Override
    public void getAvailableNetworksResponse(RadioResponseInfo responseInfo,
                                             ArrayList<android.hardware.radio.V1_0.OperatorInfo>
                                                     networkInfos) {
        // mMtkRil.riljLog("Override getAvailableNetworksResponse");

        int mLac = -1;
        String mPlmn = null;
        Phone phone = PhoneFactory.getPhone(mMtkRil.mInstanceId);
        if (phone != null) {
            ServiceStateTracker sst = phone.getServiceStateTracker();
            ServiceState ss = sst.mSS;
            mPlmn = ss.getOperatorNumeric();
            mLac = ((MtkServiceStateTracker) sst).getLac();
            // mMtkRil.riljLog("mPlmn=" + mPlmn + ", mLac=" + mLac);
        }
        for (int i = 0; i < networkInfos.size(); i++) {
            String mccmnc = networkInfos.get(i).operatorNumeric;
            // mMtkRil.riljLog("mccmnc=" + mccmnc);
            String optr = SystemProperties.get("persist.vendor.operator.optr");
            if (!TextUtils.isEmpty(optr)) {
                if (optr.equals("OP07")) { // AT&T consider EONS/TS.25 for all records
                    String operatorLongName = mMtkRil.lookupOperatorNameEons(
                            getSubId(mMtkRil.mInstanceId),
                            mccmnc, true, mLac);
                    if (TextUtils.isEmpty(operatorLongName)) {
                        operatorLongName = mMtkRil.lookupOperatorNameMVNO(
                                getSubId(mMtkRil.mInstanceId), mccmnc, true);
                    }
                    networkInfos.get(i).alphaLong = operatorLongName;
                    String operatorShortName = mMtkRil.lookupOperatorNameEons(
                            getSubId(mMtkRil.mInstanceId),
                            mccmnc, false, mLac);
                    if (TextUtils.isEmpty(operatorShortName)) {
                        operatorShortName = mMtkRil.lookupOperatorNameMVNO(
                                getSubId(mMtkRil.mInstanceId), mccmnc, false);
                    }
                    networkInfos.get(i).alphaShort = operatorShortName;
                }
            } else if (!TextUtils.isEmpty(mPlmn) && !TextUtils.isEmpty(mccmnc) &&
                mPlmn.equals(mccmnc)) {
                // mMtkRil.riljLog("Before: long name=" + networkInfos.get(i).alphaLong +
                //     ", short name=" + networkInfos.get(i).alphaShort);
                networkInfos.get(i).alphaLong = mMtkRil.lookupOperatorName(
                            getSubId(mMtkRil.mInstanceId),
                            mccmnc, true, mLac);
                networkInfos.get(i).alphaShort = mMtkRil.lookupOperatorName(
                            getSubId(mMtkRil.mInstanceId),
                            mccmnc, false, mLac);
                // mMtkRil.riljLog("After: long name=" + networkInfos.get(i).alphaLong +
                //     ", short name=" + networkInfos.get(i).alphaShort);
            } else {
                networkInfos.get(i).alphaLong =
                    mMtkRil.lookupOperatorNameMVNO(
                    getSubId(mMtkRil.mInstanceId), mccmnc, true);
                networkInfos.get(i).alphaShort =
                    mMtkRil.lookupOperatorNameMVNO(
                    getSubId(mMtkRil.mInstanceId), mccmnc, false);
            }
        }

        super.getAvailableNetworksResponse(responseInfo, networkInfos);
    }

}
