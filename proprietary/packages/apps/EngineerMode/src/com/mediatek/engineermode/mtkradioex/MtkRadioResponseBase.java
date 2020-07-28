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

package com.mediatek.engineermode;

import android.hardware.radio.V1_0.RadioError;
import android.hardware.radio.V1_0.RadioResponseInfo;
import android.os.AsyncResult;
import android.os.Message;

import com.android.internal.telephony.CommandException;
import com.android.internal.telephony.RIL;

import java.util.ArrayList;

import vendor.mediatek.hardware.mtkradioex.V1_0.IEmRadioResponse;
public class MtkRadioResponseBase extends IEmRadioResponse.Stub {
    public static final String TAG = "MtkRadioResponseBase";
    public MtkRadioResponseBase(RIL ril) {
    }

    /**
     * Convert an ArrayList of Bytes to an exactly-sized primitive array
     */
    public static byte[] arrayListToPrimitiveArray(ArrayList<Byte> bytes) {
        byte[] ret = new byte[bytes.size()];
        for (int i = 0; i < ret.length; i++) {
            ret[i] = bytes.get(i);
        }
        return ret;
    }

    private void sendMessageResponse(Message msg, Object ret) {
        if (msg != null) {
            AsyncResult.forMessage(msg, ret, null);
            msg.sendToTarget();
        }
    }

    private void sendMessageResponseError(Message msg, int error, Object ret) {
        CommandException ex;
        ex = CommandException.fromRilErrno(error);
        if (msg != null) {
            AsyncResult.forMessage(msg, ret, ex);
            msg.sendToTarget();
        }
    }

    private void responseStringArrayList(RIL ril, RadioResponseInfo responseInfo,
                                         ArrayList<String> strings) {
        String[] ret = new String[strings.size()];
        for (int i = 0; i < strings.size(); i++) {
            ret[i] = strings.get(i);
        }
        Message msg = EmRadioHidl.mRequestList.get(responseInfo.serial);
        if (msg != null) {
            EmRadioHidl.mRequestList.remove(responseInfo.serial);
        }
        if (responseInfo.error == RadioError.NONE) {
            sendMessageResponse(msg, ret);
        } else {
            sendMessageResponseError(msg, responseInfo.error, ret);
        }
    }

    public void sendRequestRawResponse(RadioResponseInfo responseInfo, ArrayList<Byte> data) {
        Elog.d(TAG, "rsp data = " + data);
        byte[] ret = null;
        Message msg = EmRadioHidl.mRequestList.get(responseInfo.serial);
        if (msg != null) {
            EmRadioHidl.mRequestList.remove(responseInfo.serial);
        }
        if (responseInfo.error == RadioError.NONE) {
            ret = arrayListToPrimitiveArray(data);
            sendMessageResponse(msg, ret);
        } else {
            sendMessageResponseError(msg, responseInfo.error, ret);
        }
    }


    public void sendRequestStringsResponse(RadioResponseInfo responseInfo, ArrayList<String> data) {
        Elog.d(TAG, "rsp data = " + data);
        responseStringArrayList(null, responseInfo, data);
    }
}
