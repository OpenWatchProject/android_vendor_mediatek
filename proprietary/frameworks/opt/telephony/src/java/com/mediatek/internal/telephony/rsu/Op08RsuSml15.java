/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained hereis
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, whole or part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAFROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.internal.telephony.rsu;

import android.app.ActivityManager;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.Signature;
import android.os.AsyncResult;
import android.os.Binder;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Message;
import android.os.PowerManager;
import android.os.RemoteException;
import android.telephony.Rlog;
import android.text.TextUtils;
import android.view.WindowManager;

import com.android.internal.telephony.CommandsInterface;
import com.android.internal.telephony.uicc.IccUtils;

import com.mediatek.internal.telephony.MtkRIL;
import com.mediatek.internal.telephony.uicc.IMtkRsuSml;

import vendor.mediatek.hardware.mtkradioex.V1_9.RsuRequest;
import vendor.mediatek.hardware.mtkradioex.V1_9.RsuRequestInfo;
import vendor.mediatek.hardware.mtkradioex.V1_9.RsuResponseInfo;

/**
 * RSU SIM LOCK 15 adapter layer, send request and receive response.
 */
public class Op08RsuSml15 extends IMtkRsuSml {
    private static final String TAG = "Op08RsuSml15";

    private static final int OPERATOR_TMO = 3;
    private static final int EVENT_NEED_REBOOT = 0;

    private static final int USELESS_INT_VALUE = 0;
    private static final String USELESS_STRING_VALUE = "";

    private static final int RSU_MESSAGE_UNSOL_RSU_EVENT = 100;

    private MtkRIL mCi;
    private Context mContext;
    private HandlerThread mWorker = null;
    private Handler mUrcHandler = null;

    /**
     * Constructor of OP08 Rsu.
     *
     * @param context The context of caller.
     * @param ci The cis of all slots.
     */
    public Op08RsuSml15(Context context, CommandsInterface[] ci) {
        logi("initialize Op08RsuSml15: " + ci);

        mContext = context;
        mCi = (MtkRIL)ci[0];
        mWorker = new HandlerThread("RsuWorker");
        mWorker.start();
        mUrcHandler = new Handler(mWorker.getLooper()) {
            @Override
            public void handleMessage(Message msg) {
                AsyncResult ar = (AsyncResult) msg.obj;
                logd("handleMessage what = " + msg.what);

                switch (msg.what) {
                    case RSU_MESSAGE_UNSOL_RSU_EVENT:
                    /* it is different with TC1 branch, it will be unified in future.
                    String[] event = (String[]) ar.result;
                    logd("RSU_MESSAGE_UNSOL_RSU_EVENT eventId = " + event[0]
                            + " eventString = " + event[1]);
                    int eventId = Integer.valueOf(event[0]);
                    */
                        int[] event = (int[]) ar.result;
                        logd("RSU_MESSAGE_UNSOL_RSU_EVENT eventId = " + event[0]);
                        int eventId = event[0];

                        boolean needReboot = true;
                        if (((eventId & 0xFFFF0000) >> 16) == OPERATOR_TMO) {
                            handleUnlockExpires((eventId & 0xFFFF) == EVENT_NEED_REBOOT);
                        } else {
                            loge("unexpected eventId: " + eventId);
                        }
                        break;
                    default:
                        logi("Unknown urc or urc that need not handle");
                        break;
                }
            }
        };


        mCi.registerForRsuSimLockChanged(mUrcHandler, RSU_MESSAGE_UNSOL_RSU_EVENT, null);
    }

    /**
    * Request SLE Vendor Service Adapter for the shared symmetric key data that will be
    * sent to the Unlock Server for registration.
    *
    * @param data the input data that contains the IMEI
    *
    * @param onComplete callback message
    */
    public void sleGetRegisterRequest(byte[] data, Message onComplete) {
        logd("sleGetRegisterRequest data = " + bytes2Hexs(data)
                + " mCi = " + mCi);
        if (mCi != null) {
            mCi.sendRsuRequest(constructRsuRequestInfoRilRequest(OPERATOR_TMO,
                    RsuRequest.RSU_REQUEST_GET_SHARED_KEY,
                    USELESS_INT_VALUE,
                    bytes2Hexs(data),
                    USELESS_STRING_VALUE),
                    onComplete);
        }
    }

    /**
    * Request Vendor Service Adapter to generate and return the formatted unlock
    * request that will be sent to the Unlock Server.
    *
    * @param data the input data that contains the unlock request type and the IMSI
    *
    * @param onComplete callback message
    */
    public void sleCreateUnlockRequest(byte[] data, Message onComplete) {
        logd("sleCreateUnlockRequest data = " + bytes2Hexs(data)
                + " mCi = " + mCi);
        if (mCi != null) {
            mCi.sendRsuRequest(constructRsuRequestInfoRilRequest(OPERATOR_TMO,
                    RsuRequest.RSU_REQUEST_INIT_REQUEST,
                    USELESS_INT_VALUE,
                    bytes2Hexs(data),
                    USELESS_STRING_VALUE),
                    onComplete);
        }
    }

    /**
    * Request Vendor Service Adapter to process the input message that was received
    * from the Unlock Server.
    *
    * @param msg the message that needs to be processed by the SLE service
    * (e.g.: an unlock request)
    *
    * @param imei the device IMEI as bytes
    *
    * @param onComplete callback message
    */
    public void sleProcessMessage(byte[] msg, byte[] imei, Message onComplete) {
        logd("sleProcessMessage msg = " + bytes2Hexs(msg) + " mCi = " + mCi);
        if (mCi != null) {
            mCi.sendRsuRequest(constructRsuRequestInfoRilRequest(OPERATOR_TMO,
                    RsuRequest.RSU_REQUEST_UPDATE_LOCK_DATA,
                    USELESS_INT_VALUE,
                    bytes2Hexs(msg),
                    bytes2Hexs(imei)),
                    onComplete);
        }
    }

    /**
    * Retrieve the current SIM-Lock status from the SLE.
    *
    * @param data the input data is always null (used for future usages and compatibility)
    *
    * @param onComplete callback message
    */
    public void sleGetSimlockStatus(byte[] data, Message onComplete) {
        logd("sleGetSimlockStatus data = " + bytes2Hexs(data)
                + " mCi = " + mCi);
        if (mCi != null) {
            mCi.sendRsuRequest(constructRsuRequestInfoRilRequest(OPERATOR_TMO,
                    RsuRequest.RSU_REQUEST_GET_LOCK_STATUS,
                    USELESS_INT_VALUE,
                    bytes2Hexs(data),
                    USELESS_STRING_VALUE),
                    onComplete);
        }
    }

    /**
    * Retrieve the SLE vendor adapter configuration (Unlock Server URL and other specific
    * information for delivering the request messages to the server).
    *
    * @param data the input data that contains the caller version code
    *
    * @param onComplete callback message
    */
    public void sleGetAdapterConfig(byte[] data, Message onComplete) {
        logd("sleGetAdapterConfig data = " + bytes2Hexs(data)
                + " mCi = " + mCi);
        if (mCi != null) {
            mCi.sendRsuRequest(constructRsuRequestInfoRilRequest(OPERATOR_TMO,
                    RsuRequest.RSU_REQUEST_GET_LOCK_VERSION,
                    USELESS_INT_VALUE,
                    bytes2Hexs(data),
                    USELESS_STRING_VALUE),
                    onComplete);
        }
    }

    private RsuRequestInfo constructRsuRequestInfoRilRequest(int opId, int requestId,
            int requestType, String data, String reserveString1) {
        RsuRequestInfo rri = new RsuRequestInfo();
        rri.opId = opId;
        rri.requestId = requestId;
        rri.requestType = requestType;
        if (TextUtils.isEmpty(data)) {
            rri.data = "";
        } else {
            rri.data = data;
        }
        if (TextUtils.isEmpty(reserveString1)) {
            rri.reserveString1 = "";
        } else {
            rri.reserveString1 = reserveString1;
        }
        return rri;
    }

    /**
     * Reboot listener.
     */
    private class RebootClickListener implements DialogInterface.OnClickListener {
        @Override
        public void onClick(DialogInterface dialog, int which) {
            logd("[RSU-SIMLOCK] Unlock Phone onClick");
            PowerManager pm = (PowerManager) mContext.getSystemService(Context.POWER_SERVICE);
            pm.reboot("Unlock state changed");
        }
    }

    private void handleUnlockExpires(boolean needReboot) {
        RebootClickListener listener = null;

        String title, message;
        if (!needReboot) {
            title = new String("Unlock Expired!");
            message = new String("T-Mobile Temporary Device Unlock has Expired!");
        } else {
            listener = new RebootClickListener();
            title = new String("T-Mobile Temporary Device Unlock has Expired!");
            message = new String("You will need to reboot the device right now.");
        }
        AlertDialog alertDialog = new AlertDialog.Builder(mContext)
                .setTitle(title)
                .setMessage(message)
                .setPositiveButton("OK", listener)
                .create();
        alertDialog.setCancelable(false);
        alertDialog.setCanceledOnTouchOutside(false);

        alertDialog.getWindow().setType(
                WindowManager.LayoutParams.TYPE_SYSTEM_ALERT);
        alertDialog.show();
    }

    private String bytes2Hexs(byte[] bytes) {
        return IccUtils.bytesToHexString(bytes);
    }

    private void loge(String s) {
        Rlog.e(TAG, "[RSU-SIMLOCK] " + s);
    }

    private void logd(String s) {
        Rlog.d(TAG, "[RSU-SIMLOCK] " + s);
    }

    private void logi(String s) {
        Rlog.i(TAG, "[RSU-SIMLOCK] " + s);
    }
}
