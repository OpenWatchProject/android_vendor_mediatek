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

package com.mediatek.opcommon.telephony;

import com.android.internal.telephony.RadioResponse;

import vendor.mediatek.hardware.radio_op.V2_0.IRadioResponseOp;
import vendor.mediatek.hardware.radio_op.V2_0.RsuResponseInfo;

import android.os.AsyncResult;
import android.os.RemoteException;
import android.telephony.Rlog;
import android.text.TextUtils;
import android.util.Log;

import android.hardware.radio.V1_0.RadioError;
import android.hardware.radio.V1_0.RadioResponseInfo;
import android.hardware.radio.V1_0.CardStatus;

import com.android.internal.telephony.RILRequest;

import java.util.ArrayList;

public class MtkRadioResponseOp extends IRadioResponseOp.Stub {
    static final String TAG = "MtkRadioResponseOp";
    private MtkRilOp mMtkRilOp;

    RadioResponse mRadioResponse;

    public MtkRadioResponseOp(MtkRilOp ril) {
        mRadioResponse = new RadioResponse(ril);
        mMtkRilOp = ril;
        mMtkRilOp.log("MtkRadioResponseOp constructor");
    }
    public void log(String text) {
        Rlog.d(TAG, text);
    }

    /**
     * Response for request 'setIncomingVirtualLine'
     * @param info Radio Response Info
     */
    public void setIncomingVirtualLineResponse(RadioResponseInfo responseInfo) {
        mMtkRilOp.log("setIncomingVirtualLineResponse");
        mRadioResponse.responseVoid(responseInfo);
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     * @param respAntConf Ant configuration
     */
    public void setRxTestConfigResponse(RadioResponseInfo responseInfo,
            ArrayList<Integer> respAntConf) {
        mMtkRilOp.log("setRxTestConfigResponse");
        mRadioResponse.responseIntArrayList(responseInfo, respAntConf);
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     * @param respAntInfo Ant Info
     */
    public void getRxTestResultResponse(RadioResponseInfo responseInfo,
            ArrayList<Integer> respAntInfo) {
        mMtkRilOp.log("getRxTestResultResponse");
        mRadioResponse.responseIntArrayList(responseInfo, respAntInfo);
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     */
    public void setDisable2GResponse(RadioResponseInfo responseInfo) {
        mMtkRilOp.log("setDisable2GResponse");
        mRadioResponse.responseVoid(responseInfo);
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     * @param mode Disable2G is enabled when it is true.
     */
    public void getDisable2GResponse(RadioResponseInfo responseInfo, int mode) {
        mMtkRilOp.log("getDisable2GResponse");
        mRadioResponse.responseInts(responseInfo, mode);
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     */
    public void exitSCBMResponse(RadioResponseInfo responseInfo) {
        mRadioResponse.responseVoid(responseInfo);
    }

    public void sendRsuRequestResponse(RadioResponseInfo responseInfo,
            RsuResponseInfo data) {
        RILRequest rr = mMtkRilOp.processResponse(responseInfo);
        if (rr != null) {
            if (responseInfo.error == RadioError.NONE) {
                AsyncResult.forMessage(rr.mResult, data, null);
                rr.mResult.sendToTarget();
            }
            mMtkRilOp.processResponseDone(rr, responseInfo, data);
        }
    }
}
