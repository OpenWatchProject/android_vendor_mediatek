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

import android.os.AsyncResult;
import android.os.Message;

import com.android.internal.telephony.RIL;

import java.util.ArrayList;
import java.util.List;

import vendor.mediatek.hardware.mtkradioex.V1_0.IEmRadioIndication;

public class MtkRadioIndication extends IEmRadioIndication.Stub {
    public static final String TAG = "MtkRadioIndication";

    public MtkRadioIndication(RIL ril) {
    }

    public static byte[] arrayListToPrimitiveArray(List<Byte> bytes) {
        byte[] ret = new byte[bytes.size()];
        for (int i = 0; i < ret.length; i++) {
            ret[i] = bytes.get(i);
        }
        return ret;
    }

    public static int[] arrayListToPrimitiveArrayInt(List<Integer> value) {
        int[] ret = new int[value.size()];
        for (int i = 0; i < ret.length; i++) {
            ret[i] = value.get(i);
        }
        return ret;
    }

    public static String[] arrayListToPrimitiveArrayString(List<String> info) {
        String[] ret = new String[info.size()];
        for (int i = 0; i < ret.length; i++) {
            ret[i] = info.get(i);
        }
        return ret;
    }

    private void sendMessageResponse(Message msg, Object ret) {
        try {
            AsyncResult.forMessage(msg, ret, null);
            msg.sendToTarget();
            Elog.d(TAG, "msg send");
        } catch (Exception e) {
            Elog.e(TAG, "sendMessageResponse: " + e.getMessage());
        }
    }

    public void networkInfoInd(int indicationType, ArrayList<String> info) {
        String[] response = arrayListToPrimitiveArrayString(info);
        if (EmRadioHidl.mRadioIndicationType == EmUtils.RADIO_INDICATION_TYPE_NETWORKINFO) {
            Message msg = EmRadioHidl.mHandler.obtainMessage(EmRadioHidl.mWhat);
            sendMessageResponse(msg, response);
        } else {
            Elog.e(TAG, "networkInfoInd not send to app");
        }
    }

    public void onTxPowerIndication(int indicationType, ArrayList<Integer> indPower) {
        int[] response = arrayListToPrimitiveArrayInt(indPower);
        Elog.d(TAG, "onTxPowerIndication: " + indPower.toString());
        if (EmRadioHidl.mRadioIndicationType == EmUtils.RADIO_INDICATION_TYPE_TXPOWER_INFO) {
            Message msg = EmRadioHidl.mHandler.obtainMessage(EmRadioHidl.mWhat);
            sendMessageResponse(msg, response);
        } else {
            Elog.e(TAG, "onTxPowerIndication not send to app");
        }

    }

    public void oemHookRaw(int indicationType, ArrayList<Byte> data) {
        byte[] response = arrayListToPrimitiveArray(data);
        String str = new String(response);
        Elog.d(TAG, "oemHookRaw:" + str);
        if (EmRadioHidl.mRadioIndicationType == EmUtils.RADIO_INDICATION_TYPE_URCINFO) {
            Message msg = EmRadioHidl.mHandler.obtainMessage(EmRadioHidl.mWhat);
            sendMessageResponse(msg, response);
        } else {
            Elog.e(TAG, "oemHookRaw not send to app");
        }

    }

}
