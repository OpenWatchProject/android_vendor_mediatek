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

package com.mediatek.op129.telephony;

import android.app.ActivityManager;
import android.content.Context;
import android.os.AsyncResult;
import android.os.Binder;
import android.os.Handler;
import android.os.Message;
import android.os.RemoteException;
import android.telephony.Rlog;
import android.text.TextUtils;

import com.android.internal.telephony.CommandException;
import com.android.internal.telephony.CommandsInterface;
import com.android.internal.telephony.RILConstants;
import com.android.internal.telephony.uicc.IccUtils;

import com.mediatek.internal.telephony.MtkRIL;
import com.mediatek.internal.telephony.uicc.IMtkRsuSml;
import com.mediatek.opcommon.telephony.MtkRilOp;
import com.kddi.remoteSimlock.IKDDIRemoteSimlockServiceCallback;
import com.kddi.remoteSimlock.SimLockData;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import vendor.mediatek.hardware.radio_op.V2_0.RsuRequest;
import vendor.mediatek.hardware.radio_op.V2_0.RsuResponseInfo;
import vendor.mediatek.hardware.radio_op.V2_0.RsuRequestInfo;

/**
 * RSU SIM LOCK 13 adapter layer, send request and receive response.
 */
public class Op129RsuSml13 extends IMtkRsuSml {
    private static final String TAG = "Op129RsuSml13";

    private MtkRilOp mCi[];
    private Context mContext;

    public ArrayList<IKDDIRemoteSimlockServiceCallback> mCb =
            new ArrayList<IKDDIRemoteSimlockServiceCallback>();

    private static final int OPERATOR_KDDI = 2;
    protected static final int USELESS_INT_VALUE = 0;
    protected static final String USELESS_STRING_VALUE = "";

    protected static final int REMOTE_SIM_UNLOCK_SUCCESS = 0;
    protected static final int REMOTE_SIM_UNLOCK_ERROR = 1;

    private static final int MESSAGE_BASE = 0;
    private static final int RSU_MESSAGE_SET_SIM_LOCK_DATA = MESSAGE_BASE + 1;
    private static final int RSU_MESSAGE_GET_ALLOWED_LOCK_DATA = MESSAGE_BASE + 2;
    private static final int RSU_MESSAGE_GET_EXCLUDED_LOCK_DATA = MESSAGE_BASE + 3;

    private static final int RSU_REQUEST_BASE = 50;
    private static final int RSU_REQUEST_SET_SIM_LOCK_DATA = RSU_REQUEST_BASE + 1;
    private static final int RSU_REQUEST_GET_ALLOWED_LOCK_DATA = RSU_REQUEST_BASE + 2;
    private static final int RSU_REQUEST_GET_EXCLUDED_LOCK_DATA = RSU_REQUEST_BASE + 3;
    private int mSlotId = 0;
    private boolean DEBUG = false;

    /**
     * Constructor of OP129 Rsu.
     *
     * @param context The context of caller.
     * @param ci The cis of all slots.
     */
    public Op129RsuSml13(Context context, CommandsInterface[] ci) {
        logi("initialize Op129RsuSml13");

        mContext = context;

        mCi = new MtkRilOp[ci.length];
        for (int i = 0; i < ci.length; i++) {
            mCi[i] = (MtkRilOp) ((MtkRIL) ci[i]).getRilOp();
        }
        logi("initialize Op129RsuSml13 done");
    }

    /**
    * Client calls this API to registerCallback, so it can receive asynchronous responses.
    *
    * @param cb
    * Instance of callback
    *
    * @return
    * SIMLOCK_SUCCESS =  0. Successfully registered callback with SimLock service;
    * SIMLOCK_ERROR  = 1. Failed to register call back;
    */
    public synchronized int registerCallback(Object cb) {
        logi("registerCallback IKDDIRemoteSimlockServiceCallback = " + cb);
        try {
            if (cb != null && cb instanceof IKDDIRemoteSimlockServiceCallback) {
                logi("instanceof IKDDIRemoteSimlockServiceCallback");
                if (!mCb.contains((IKDDIRemoteSimlockServiceCallback) cb)) {
                    logi("add IKDDIRemoteSimlockServiceCallback");
                    mCb.add((IKDDIRemoteSimlockServiceCallback) cb);
                    return REMOTE_SIM_UNLOCK_SUCCESS;
                }
            }
        } catch (NoClassDefFoundError e) {
            loge("NoClassDefFoundError exception");
            e.printStackTrace();
        }

        return REMOTE_SIM_UNLOCK_ERROR;
    }

    /**
    * This API de-registers the callback.
    *
    * @param cb
    * Instance of callback
    *
    * @return
    * SIMLOCK_SUCCESS =  0. Successfully deregistered callback with SimLock service;
    * SIMLOCK_ERROR  =  1. Failed to deregister call back;
    */
    public synchronized int deregisterCallback(Object cb) {
        logi("deregisterCallback IKDDIRemoteSimlockServiceCallback = " + cb);
        return (mCb.remove((IKDDIRemoteSimlockServiceCallback) cb) ? REMOTE_SIM_UNLOCK_SUCCESS
                : REMOTE_SIM_UNLOCK_ERROR);
    }

    private Handler mRequestHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            AsyncResult ar = (AsyncResult) msg.obj;
            RsuResponseInfo rri = (RsuResponseInfo) ar.result;
            int slotId = msg.arg1;
            int token = msg.arg2;

            logd("callback size = " + mCb.size()
                    + " exception = " + ar.exception
                    + " errCode = " + (rri != null ? rri.errCode : "NULL")
                    + " token = " + token
                    + " what = " + msg.what
                    + " slotId = " + slotId);

            switch (msg.what) {
                case RSU_MESSAGE_SET_SIM_LOCK_DATA:
                    try {
                        for (IKDDIRemoteSimlockServiceCallback cb : mCb) {
                            logd("RSU_MESSAGE_SET_SIM_LOCK_DATA cb = " + cb);
                            if (ar.exception != null) {
                                cb.setSimLockPolicyDataResponse(token,
                                        REMOTE_SIM_UNLOCK_ERROR, slotId);
                            } else {
                                cb.setSimLockPolicyDataResponse(token,
                                        REMOTE_SIM_UNLOCK_SUCCESS, slotId);
                            }
                        }
                    } catch (RemoteException e) {
                        loge("RemoteException");
                    }
                    break;
                case RSU_MESSAGE_GET_ALLOWED_LOCK_DATA:
                case RSU_MESSAGE_GET_EXCLUDED_LOCK_DATA:
                    List<SimLockData> lockDataList = new ArrayList<SimLockData>();
                    if (rri != null && rri.data != null) {
                        logd("RSU_MESSAGE_GET_ALLOWED_LOCK_DATA");
                        List<String> tripleList = Arrays.asList(rri.data.split(","));
                        for (String triple : tripleList) {
                            String[] tripleToken = triple.split(":");
                            if (tripleToken.length != 4) {
                                continue;
                            }
                            SimLockData simLockData = new SimLockData(
                                    Integer.valueOf(tripleToken[0]), tripleToken[1], tripleToken[2],tripleToken[3]);
                            lockDataList.add(simLockData);
                            if (DEBUG) {
                                logd("simLockData mCategory = " + simLockData.getCategory());
                                logd("simLockData mIMSI = " + simLockData.getImsi());
                                logd("simLockData mGID1 = " + simLockData.getGid1());
                                logd("simLockData mGID2 = " + simLockData.getGid2());
                            }
                        }
                    }
                    try {
                        for (IKDDIRemoteSimlockServiceCallback cb : mCb) {
                            logd("RSU_MESSAGE_GET_ALLOWED_LOCK_DATA cb = " + cb);
                            if (ar.exception != null) {
                                if (msg.what == RSU_MESSAGE_GET_ALLOWED_LOCK_DATA) {
                                    cb.getAllowedSimLockDataResponse(token,
                                            REMOTE_SIM_UNLOCK_ERROR, slotId, null);
                                } else {
                                    cb.getExcludedSimLockDataResponse(token,
                                            REMOTE_SIM_UNLOCK_ERROR, slotId, null);
                                }
                            } else {
                                if (msg.what == RSU_MESSAGE_GET_ALLOWED_LOCK_DATA) {
                                    cb.getAllowedSimLockDataResponse(token,
                                            REMOTE_SIM_UNLOCK_SUCCESS, slotId, lockDataList);
                                } else {
                                    cb.getExcludedSimLockDataResponse(token,
                                            REMOTE_SIM_UNLOCK_SUCCESS, slotId, lockDataList);
                                }
                            }
                        }
                    } catch (RemoteException e) {
                        loge("RemoteException");
                    }
                    break;
                default:
                    logi("Unknown request or request that need not handle");
                    break;
            }
        }
    };
    /**
    * This API sends lock/unlock blob (also known as Response Message) to Network Lock Module.
    * Note: Network Lock module use this blob to perform lock/unlock operation
    *
    * @param token
    * Identifier that can be used by Client to match the asynchronous response for this request.
    *
    * @param data
    * This contains simlock blob for lock and unlock operation
    *
    * @return
    * SIMLOCK_SUCCESS =  0;
    * SIMLOCK_ERROR  =  1;
    */
    public int setSimLockPolicyData(int token, byte[] data, int slotId) {
        logd("setSimLockPolicyData data = " + bytes2Hexs(data)
                + " mCi[0] = " + mCi[0] + " token = " + token + " slotId = " + slotId);
        if (mCi[0] != null) {
            mCi[0].sendRsuRequest(constructRsuRequestInfoRilRequest(OPERATOR_KDDI,
                    RSU_REQUEST_SET_SIM_LOCK_DATA,
                    USELESS_INT_VALUE,
                    bytes2Hexs(data)),
                    mRequestHandler.obtainMessage(RSU_MESSAGE_SET_SIM_LOCK_DATA, slotId, token));
            return REMOTE_SIM_UNLOCK_SUCCESS;
        }

        return REMOTE_SIM_UNLOCK_ERROR;
    }

    /**
    * Client calls this API to get sim lock data of white list, so it can receive asynchronous responses.
    *
    * @param token
    * Identifier that can be used by Client to match the asynchronous response for this request.
    *
    * @param slotId
    *
    * @return
    * SIMLOCK_SUCCESS =  0. Successfully get sim lock data with SimLock service;
    * SIMLOCK_ERROR  = 1. Failed to get sim lock data;
    */
    public int getAllowedSimLockData(int token, int slotId) {
        logd("getAllowedSimLockData token = " + token + " slotId = " + slotId);
        if (mCi[0] != null) {
            mCi[0].sendRsuRequest(constructRsuRequestInfoRilRequest(OPERATOR_KDDI,
                    RSU_REQUEST_GET_ALLOWED_LOCK_DATA,
                    USELESS_INT_VALUE,
                    USELESS_STRING_VALUE),
                    mRequestHandler.obtainMessage(RSU_MESSAGE_GET_ALLOWED_LOCK_DATA, slotId, token));
            return REMOTE_SIM_UNLOCK_SUCCESS;
        }

        return REMOTE_SIM_UNLOCK_ERROR;
    }

    /**
    * Client calls this API to get sim lock data of black list, so it can receive asynchronous responses.
    *
    * @param token
    * Identifier that can be used by Client to match the asynchronous response for this request.
    *
    * @param slotId
    *
    * @return
    * SIMLOCK_SUCCESS =  0. Successfully get sim lock data with SimLock service;
    * SIMLOCK_ERROR  = 1. Failed to get sim lock data;
    */
    public int getExcludedSimLockData(int token, int slotId) {
        logd("getAllowedSimLockData token = " + token + " slotId = " + slotId);
        if (mCi[0] != null) {
            mCi[0].sendRsuRequest(constructRsuRequestInfoRilRequest(OPERATOR_KDDI,
                    RSU_REQUEST_GET_EXCLUDED_LOCK_DATA,
                    USELESS_INT_VALUE,
                    USELESS_STRING_VALUE),
                    mRequestHandler.obtainMessage(RSU_MESSAGE_GET_EXCLUDED_LOCK_DATA, slotId, token));
            return REMOTE_SIM_UNLOCK_SUCCESS;
        }

        return REMOTE_SIM_UNLOCK_ERROR;
    }

    private RsuRequestInfo constructRsuRequestInfoRilRequest(int opId, int requestId,
            int requestType, String data) {
        RsuRequestInfo rri = new RsuRequestInfo();
        rri.opId = opId;
        rri.requestId = requestId;
        rri.requestType = requestType;
        rri.data = data;
        return rri;
    }

    private void sendErrorToCallback(Message msg) {
        CommandException ex = CommandException.fromRilErrno(RILConstants.GENERIC_FAILURE);
        RsuResponseInfo ret = new RsuResponseInfo();
        if (msg != null) {
            AsyncResult.forMessage(msg, ret, ex);
            msg.sendToTarget();
        }
    }

    private String bytes2Hexs(byte[] bytes) {
        return IccUtils.bytesToHexString(bytes);
    }

    private byte[] hexs2Bytes(String data) {
        if (data == null || data.length() % 2 != 0) {
            logi("The length of hex string is not even numbers");
            return null;
        }
        return IccUtils.hexStringToBytes(data);
    }

    public void loge(String s) {
        Rlog.e(TAG, "[RSU-SIMLOCK] " + s);
    }

    public void logd(String s) {
        Rlog.d(TAG, "[RSU-SIMLOCK] " + s);
    }

    public void logi(String s) {
        Rlog.i(TAG, "[RSU-SIMLOCK] " + s);
    }

}
