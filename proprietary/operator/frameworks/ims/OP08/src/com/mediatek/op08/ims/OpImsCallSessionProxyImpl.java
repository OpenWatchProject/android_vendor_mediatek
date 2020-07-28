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

package com.mediatek.op08.ims;

import android.os.AsyncResult;
import android.os.Message;
import android.os.RemoteException;
import android.os.SystemProperties;
import android.content.Context;
import android.content.Intent;
import android.telephony.Rlog;
import android.telephony.ims.ImsCallSession;
import android.telephony.ims.ImsReasonInfo;
import android.text.TextUtils;

import com.android.ims.ImsManager;
import com.mediatek.ims.ext.OpImsCallSessionProxyBase;
import com.mediatek.ims.internal.MtkImsManager;
import com.mediatek.ims.internal.IMtkImsCallSession;

import android.telephony.ims.ImsCallSessionListener;
import com.mediatek.ims.MtkImsCallSessionProxy;
import com.mediatek.ims.ril.ImsCommandsInterface;
import com.mediatek.ims.ril.OpImsCommandsInterface;

public class OpImsCallSessionProxyImpl extends OpImsCallSessionProxyBase {
    private static final String TAG = "Op08ImsCallSessionProxyImpl";
    Context mContext;
    boolean mIsDeviceSwitching = false;
    private static String US_COUNTRY_CODE = "+1";
    private static String UUID_PREFIX = "urn:uuid:";

    private static final String PROP_IMS_PCT_CONFIG = "vendor.ril.volte.mal.pctid";
    private static String PCT_OP_ID = "OP16387";
    private static final String PERMISSION_RECEIVE_NOT_RINGING_INCOMING_CALL =
            "com.mediatek.permission.RECEIVE_NOT_RINGING_INCOMING_CALL";

    public OpImsCallSessionProxyImpl(Context context) {
        mContext = context;
    }

    @Override
    public void broadcastForNotRingingMtIfRequired(boolean sipSessionProgress, int state, int
            serviceId, String callNumber, Context context) {
        if (sipSessionProgress != true || state != ImsCallSession.State.IDLE || mContext == null) {
            Rlog.d(TAG, "broadcastForNotRingingMtIfRequired: sipSessionProgress = " +
                    sipSessionProgress + ", state = " + state + ", mContext = " + mContext);
            return;
        }
        Intent intent = new Intent(MtkImsManager.ACTION_IMS_NOT_RINGING_INCOMING_CALL);
        intent.putExtra(MtkImsManager.EXTRA_DIAL_STRING, callNumber);
        intent.putExtra(ImsManager.EXTRA_SERVICE_ID, serviceId);
        mContext.sendBroadcast(intent, PERMISSION_RECEIVE_NOT_RINGING_INCOMING_CALL);
    }

    @Override
    public void deviceSwitch(
            Object imsRILAdapter, String number, String deviceId, Message response) {
        OpImsCommandsInterface opUtil =
                ((ImsCommandsInterface) imsRILAdapter).getOpCommandsInterface();

        if (number == null) {
            number = "";
        }

        deviceId = normalizeDeviceId(deviceId);

        opUtil.deviceSwitch(number, deviceId, response);

        Rlog.d(TAG, "deviceSwitch: number: " + number + " deviceId: " + deviceId);

        mIsDeviceSwitching = true;
    }

    @Override
    public void cancelDeviceSwitch(Object imsRILAdapter) {
        OpImsCommandsInterface opUtil =
                ((ImsCommandsInterface) imsRILAdapter).getOpCommandsInterface();
        opUtil.cancelDeviceSwitch(null);

        Rlog.d(TAG, "cancelDeviceSwitch");

        mIsDeviceSwitching = false;
    }

    @Override
    public void handleDeviceSwitchResponse(
            IMtkImsCallSession imsCallSession, AsyncResult result) {
        if (result == null) {
            Rlog.d(TAG, "handleDeviceSwitchResponse(): ar is null");
            return;
        }

        if (result.exception == null) {
            Rlog.d(TAG, "handleDeviceSwitchResponse(): device switch success");
            return;
        }

        mIsDeviceSwitching = false;

        try {
            imsCallSession.notifyDeviceSwitchFailed(new ImsReasonInfo());
        } catch (RemoteException e) {
            Rlog.e(TAG, "RemoteException notifyDeviceSwitchFailed()");
        }
    }

    @Override
    public boolean handleDeviceSwitchResult(
            String callId, IMtkImsCallSession imsCallSession, AsyncResult result) {
        if (mIsDeviceSwitching == false) {
            Rlog.d(TAG, "handleDeviceSwitchResult(): do not handled");
            return false;
        }

        if (result == null) {
            Rlog.d(TAG, "handleDeviceSwitchResult(): ar is null");
            return false;
        }

        int[] params = (int[]) result.result;
        if (callId == null || params[0] != Integer.parseInt(callId)) {
            Rlog.d(TAG, "handleDeviceSwitchResult(): call id mismatched");
            return false;
        }

        mIsDeviceSwitching = false;

        boolean isSuccess = (params[1] == 1);

        if (isSuccess) {
            try {
                imsCallSession.notifyDeviceSwitched();
            } catch (RemoteException e) {
                Rlog.e(TAG, "RemoteException notifyDeviceSwitched()");
            }
        } else {
            try {
                imsCallSession.notifyDeviceSwitchFailed(new ImsReasonInfo());
                // the device switch will hide hold, resume it if the device switch failed
                imsCallSession.resume();
            } catch (RemoteException e) {
                Rlog.e(TAG, "RemoteException notifyDeviceSwitchFailed()");
            }
        }


        return true;
    }

    @Override
    public boolean isValidVtDialString(String number) {
        boolean isPctTest = SystemProperties.get(PROP_IMS_PCT_CONFIG).equals(PCT_OP_ID);
        if (isPctTest) {
            Rlog.d(TAG, "isValidVtDialString(): PCT Test mode");
            return true;
        }
        // TMO Number patterns (Mandatory) ID: GID-MTRREQ-6874
        // The user must not be able to make video calls to the following number patterns:
        // 1. Numbers that include #, * or non-digit characters
        // 2. Numbers with less than 7 digits including emergency numbers (911)
        if (number.length() < 7) {
            return false;
        } else if (number.startsWith("+")) {
            number = number.substring(1);
        }
        if (!TextUtils.isDigitsOnly(number)) {
            return false;
        }

        return true;
    }

    @Override
    public String normalizeVtDialString(String number) {
        // TMO Number patterns (Mandatory) ID: GID-MTRREQ-6873
        // The user must not be able to make video calls to the following number patterns:
        // 1. Extend phone number to (7-digits) to +1##########
        // 2. Precede 10 digit phone number with a +1.
        String normalizedNumber = number;
        if (TextUtils.isDigitsOnly(number)) {
            if (number.length() == 10) {
                normalizedNumber = US_COUNTRY_CODE + number;
            }
        }
        Rlog.d(TAG, "normalizeVtDialString(): " + normalizedNumber);
        return normalizedNumber;
    }

    @Override
    public boolean isDeviceSwitching() {
        return mIsDeviceSwitching;
    }

    private String normalizeDeviceId(String deviceId) {
        // Device Id should normalize to pure uuid without header.
        // eg. "urn:uuid:FCA9F133-A6C7-40FF-ABA5-8DB47E906FFD"
        // should normalize to "FCA9F133A6C740FFABA58DB47E906FFD"
        if (deviceId == null) return "";
        return deviceId.replace(UUID_PREFIX,"").replace("-","");
    }
}