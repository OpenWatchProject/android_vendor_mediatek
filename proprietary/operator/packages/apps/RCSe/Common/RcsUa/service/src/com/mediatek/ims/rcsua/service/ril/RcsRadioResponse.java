/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2014. All rights reserved.
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


package com.mediatek.ims.rcsua.service.ril;

import android.hardware.radio.V1_0.RadioResponseInfo;
import android.hardware.radio.V1_0.RadioError;
import android.os.AsyncResult;
import android.os.Message;
import android.os.RemoteException;

import com.mediatek.ims.rcsua.service.ril.RcsRIL;
import com.mediatek.ims.rcsua.service.utils.Logger;
import vendor.mediatek.hardware.radio_op.V2_0.IRcsRadioResponse;

public class RcsRadioResponse extends IRcsRadioResponse.Stub {

    private final static String TAG = "RcsRadioResponse";
    private Logger logger = Logger.getLogger(RcsRadioResponse.class.getName());

    private RcsRIL mRil;
    private int mPhoneId;

    public RcsRadioResponse(RcsRIL ril, int phoneId) {
        mRil = ril;
        mPhoneId = phoneId;
        logger.debug("RcsRadioResponse, phone = " + mPhoneId);
    }


    /**
     * Helper function to send response msg
     * @param msg Response message to be sent
     * @param ret Return object to be included in the response message
     */
    static void sendMessageResponse(Message msg, Object ret) {
        if (msg != null) {
            AsyncResult.forMessage(msg, ret, null);
            msg.sendToTarget();
        }
    }

    /**
     * Response for request 'setDigitsLine'
     * @param info Radio Response Info
     */
    @Override
    public void setDigitsRegStatusResponse(RadioResponseInfo info) {
        responseVoid(info);
    }


    @Override
    public void switchRcsRoiStatusResponse(RadioResponseInfo info) {
        logger.debug("switchRcsRoiStatusResponse");
        responseVoid(info);
    }

    @Override
    public void updateRcsCapabilitiesResponse(RadioResponseInfo info) {
        logger.debug("updateRcsCapabilitiesResponse");
        responseVoid(info);
    }

    @Override
    public void updateRcsSessionInfoResponse(RadioResponseInfo info) {
        logger.debug("updateRcsSessionInfoResponse");
        responseVoid(info);
    }

    /* Send a void response message
     * @param responseInfo
     */
    private void responseVoid(RadioResponseInfo responseInfo) {

        RILRequest rr = mRil.processResponse(responseInfo);
        if (rr != null) {
            Object ret = null;
            if (responseInfo.error == RadioError.NONE) {
                sendMessageResponse(rr.mResult, ret);
            }
            mRil.processResponseDone(rr, responseInfo, ret);
        }
    }

}
