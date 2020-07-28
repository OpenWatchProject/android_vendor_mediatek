/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2019. All rights reserved.
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

package com.mediatek.capctrl.service;

import android.hardware.radio.V1_0.RadioError;
import android.hardware.radio.V1_0.RadioResponseInfo;
import android.os.AsyncResult;
import android.os.Message;

import java.util.ArrayList;

import vendor.mediatek.hardware.mtkradioex.V1_0.ICapRadioResponse;

import com.mediatek.capctrl.aidl.AuthResponse;
import com.mediatek.capctrl.aidl.CertResponse;

/**
 * CapRadioResponse, handle RILrequest response.
 * Current only for syncDataSettings and setFdMode.
 */
public class CapRadioResponse extends ICapRadioResponse.Stub {
    private CapRIL mCapRIL;

    public CapRadioResponse(CapRIL ril) {
        mCapRIL = ril;
    }

    public void routeCertificateResponse(RadioResponseInfo responseInfo, ArrayList<Byte> rnd,
            int custId) {
        RILRequest rr = mCapRIL.processResponse(responseInfo);
        if (rr != null) {
            CertResponse response = new CertResponse();
            response.mError = responseInfo.error;
            response.mCustId = custId;
            if (responseInfo.error == RadioError.NONE) {
                response.mRnd = CapRIL.arrayListToPrimitiveArray(rnd);
                sendMessageResponse(rr.mResult, response);
            } else {
                response.mRnd = null;
            }
            mCapRIL.processResponseDone(rr, responseInfo, response);
        }
    }

    public void routeAuthMessageResponse(RadioResponseInfo responseInfo, ArrayList<Byte> devId,
            int capMask) {
        RILRequest rr = mCapRIL.processResponse(responseInfo);
        if (rr != null) {
            AuthResponse response = new AuthResponse();
            response.mError = responseInfo.error;
            response.mCapMask = capMask;
            if (responseInfo.error == RadioError.NONE) {
                response.mDevId = CapRIL.arrayListToPrimitiveArray(devId);
                sendMessageResponse(rr.mResult, response);
            } else {
                response.mDevId = null;
            }
            mCapRIL.processResponseDone(rr, responseInfo, response);
        }
    }

    public void enableCapabilityResponse(RadioResponseInfo responseInfo) {
        RILRequest rr = mCapRIL.processResponse(responseInfo);
        if (rr != null) {
            int response = responseInfo.error;
            if (response == RadioError.NONE) {
                sendMessageResponse(rr.mResult, response);
            }
            mCapRIL.processResponseDone(rr, responseInfo, response);
        }
    }

    public void abortCertificateResponse(RadioResponseInfo responseInfo) {
        RILRequest rr = mCapRIL.processResponse(responseInfo);
        if (rr != null) {
            int response = responseInfo.error;
            if (response == RadioError.NONE) {
                sendMessageResponse(rr.mResult, response);
            }
            mCapRIL.processResponseDone(rr, responseInfo, response);
        }
    }

    private void sendMessageResponse(Message msg, Object ret) {
        if (msg != null) {
            AsyncResult.forMessage(msg, ret, null);
            msg.sendToTarget();
        }
    }
}
