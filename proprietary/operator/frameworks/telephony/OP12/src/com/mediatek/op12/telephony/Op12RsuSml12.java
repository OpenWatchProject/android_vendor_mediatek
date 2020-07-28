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

package com.mediatek.op12.telephony;

import android.app.ActivityManager;
import android.content.Context;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.Signature;
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
import com.mediatek.internal.telephony.uicc.IMtkRsuSmlCallback;
import com.mediatek.opcommon.telephony.MtkRilOp;

import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import vendor.mediatek.hardware.radio_op.V2_0.RsuRequest;
import vendor.mediatek.hardware.radio_op.V2_0.RsuResponseInfo;
import vendor.mediatek.hardware.radio_op.V2_0.RsuRequestInfo;

/**
 * RSU SIM LOCK 12 adapter layer, send request and receive response.
 */
public class Op12RsuSml12  extends IMtkRsuSml {
    private static final String TAG = "Op12RsuSml12";

    private static final int OPERATOR_VERIZON = 1;

    private static final int BASE = 0;
    private static final int RSU_MESSAGE_INIT_REQUEST = BASE + 1;
    private static final int RSU_MESSAGE_UPDATE_LOCK_DATA = BASE + 2;
    private static final int RSU_MESSAGE_GET_LOCK_VERSION = BASE + 3;
    private static final int RSU_MESSAGE_RESET_LOCK_DATA = BASE + 4;
    private static final int RSU_MESSAGE_GET_LOCK_STATUS = BASE + 5;
    private static final int RSU_MESSAGE_UNLOCK_TIMER = BASE + 6;
    private static final int RSU_MESSAGE_UNSOL_RSU_EVENT = BASE + 7;

    private static final int REMOTE_SIM_UNLOCK_SUCCESS = 0;
    private static final int REMOTE_SIM_UNLOCK_ERROR = 1;

    private static final int USELESS_INT_VALUE = 0;
    private static final String USELESS_STRING_VALUE = "";

    private static final int DUMMY_TOKEN = -1;

    public ArrayList<IMtkRsuSmlCallback> mCb =
            new ArrayList<IMtkRsuSmlCallback>();
    private MtkRilOp mCi[];
    private Context mContext;

    private static final String[][] PACKAGENAME_SHA256SIG_PAIR = {
        {"com.verizon.mips.services",
        "ff82050bf6bed1f152ac1a12dc83cacbad401775161882872c6665fc5e15c8f2"},
    };

    /**
     * Constructor of OP12 Rsu.
     *
     * @param context The context of caller.
     * @param ci The cis of all slots.
     */
    public Op12RsuSml12(Context context, CommandsInterface[] ci) {
        logi("initialize Op12RsuSml12");

        mContext = context;
        mCi = new MtkRilOp[ci.length];
        for (int i = 0; i < ci.length; i++) {
            mCi[i] = (MtkRilOp) ((MtkRIL) ci[i]).getRilOp();
        }

        mCi[0].registerForRsuEvent(mUrcHandler, RSU_MESSAGE_UNSOL_RSU_EVENT, null);
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
        logi("registerCallback IMtkRsuSmlCallback = " + cb);
        boolean isPass = false;
        isPass = enforcePermissionCheck(
                getCallerSignature(getCallerPackageName()),
                getCallerPackageName(),
                null);
        if (!isPass) {
            loge("registerCallback, permission denied");
            return REMOTE_SIM_UNLOCK_ERROR;
        }

        if (cb != null && cb instanceof IMtkRsuSmlCallback) {
            if (!mCb.contains((IMtkRsuSmlCallback) cb)) {
                if (!mCb.isEmpty()) {
                    for (IMtkRsuSmlCallback icb : mCb) {
                        loge("Unexpected, New register incoming, will remove un-deregistered "
                                + "callback icb = " + icb);
                    }
                    mCb.clear();
                }
                mCb.add((IMtkRsuSmlCallback) cb);
                logi("registerCallback callback size = " + mCb.size());
                return REMOTE_SIM_UNLOCK_SUCCESS;
            }
        }
        logi("registerCallback fail IMtkRsuSmlCallback = " + cb);
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
        logi("deregisterCallback IMtkRsuSmlCallback = " + cb);
        boolean isPass = false;
        isPass = enforcePermissionCheck(
                getCallerSignature(getCallerPackageName()),
                getCallerPackageName(),
                null);
        if (!isPass) {
            loge("deregisterCallback, permission denied");
            return REMOTE_SIM_UNLOCK_ERROR;
        }
        return (mCb.remove((IMtkRsuSmlCallback) cb) ? REMOTE_SIM_UNLOCK_SUCCESS
                : REMOTE_SIM_UNLOCK_ERROR);
    }

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
    public int remoteSimlockProcessSimlockData(int token, byte[] data) {
        logd("remoteSimlockProcessSimlockData data = " + bytes2Hexs(data)
                + " mCi[0] = " + mCi[0] + " token = " + token);
        boolean isPass = false;
        isPass = enforcePermissionCheck(
                getCallerSignature(getCallerPackageName()),
                getCallerPackageName(),
                mRequestHandler.obtainMessage(RSU_MESSAGE_UPDATE_LOCK_DATA, 0, token));
        if (!isPass) {
            loge("remoteSimlockProcessSimlockData, permission denied");
            return REMOTE_SIM_UNLOCK_ERROR;
        }
        if (mCi[0] != null) {
            mCi[0].sendRsuRequest(constructRsuRequestInfoRilRequest(OPERATOR_VERIZON,
                    RsuRequest.RSU_REQUEST_UPDATE_LOCK_DATA,
                    USELESS_INT_VALUE,
                    bytes2Hexs(data)),
                    mRequestHandler.obtainMessage(RSU_MESSAGE_UPDATE_LOCK_DATA, 0, token));
            return REMOTE_SIM_UNLOCK_SUCCESS;
        }

        return REMOTE_SIM_UNLOCK_ERROR;
    }

    /**
    * This API gets highest major/minor version of incoming blob (Response message) that is
    * supported by Network Lock Module.
    *
    * @param token
    * Identifier that can be used by Client to match the asynchronous response for this request.
    *
    * @return
    * SIMLOCK_SUCCESS =  0;
    * SIMLOCK_ERROR  =  1;
    */
    public int remoteSimlockGetVersion(int token) {
        logd("remoteSimlockGetVersion token = " + token + " mCi[0] = " + mCi[0]);
        boolean isPass = false;
        isPass = enforcePermissionCheck(
                getCallerSignature(getCallerPackageName()),
                getCallerPackageName(),
                mRequestHandler.obtainMessage(RSU_MESSAGE_GET_LOCK_VERSION, 0, token));
        if (!isPass) {
            loge("remoteSimlockGetVersion, permission denied");
            return REMOTE_SIM_UNLOCK_ERROR;
        }
        if (mCi[0] != null) {
            mCi[0].sendRsuRequest(constructRsuRequestInfoRilRequest(OPERATOR_VERIZON,
                    RsuRequest.RSU_REQUEST_GET_LOCK_VERSION,
                    USELESS_INT_VALUE,
                    USELESS_STRING_VALUE),
                    mRequestHandler.obtainMessage(RSU_MESSAGE_GET_LOCK_VERSION, 0, token));
            return REMOTE_SIM_UNLOCK_SUCCESS;
        }

        return REMOTE_SIM_UNLOCK_ERROR;
    }

    /**
    * This API gets Sim lock status of the device.
    *
    * @param token
    * Identifier that can be used by Client to match the asynchronous response for this request.
    *
    * @return
    * SIMLOCK_SUCCESS =  0;
    * SIMLOCK_ERROR  =  1;
    */
    public int remoteSimlockGetSimlockStatus(int token) {
        logd("remoteSimlockGetSimlockStatus token = " + token + " mCi[0] = " + mCi[0]);
        boolean isPass = false;
        isPass = enforcePermissionCheck(
                getCallerSignature(getCallerPackageName()),
                getCallerPackageName(),
                mRequestHandler.obtainMessage(RSU_MESSAGE_GET_LOCK_STATUS, 0, token));
        if (!isPass) {
            loge("remoteSimlockGetSimlockStatus, permission denied");
            return REMOTE_SIM_UNLOCK_ERROR;
        }
        if (mCi[0] != null) {
            mCi[0].sendRsuRequest(constructRsuRequestInfoRilRequest(OPERATOR_VERIZON,
                    RsuRequest.RSU_REQUEST_GET_LOCK_STATUS,
                    USELESS_INT_VALUE,
                    USELESS_STRING_VALUE),
                    mRequestHandler.obtainMessage(RSU_MESSAGE_GET_LOCK_STATUS, 0, token));
            return REMOTE_SIM_UNLOCK_SUCCESS;
        }

        return REMOTE_SIM_UNLOCK_ERROR;
    }

    /**
    * This API requests Network Lock Module to generate a Request Message.
    *
    * @param token
    * Identifier that can be used by Client to match the asynchronous response for this request.
    *
    * @param requestType
    * Indicates the type of request to generate.
    * Current supported value
    * SIMLOCK_REMOTE_GENERATE_BLOB_REQUEST_TYPE = 1;
    *
    * @return
    * SIMLOCK_SUCCESS =  0;
    * SIMLOCK_ERROR  =  1;
    */
    public int remoteSimlockGenerateRequest(int token, int requestType) {
        logd("remoteSimlockGenerateRequest token = " + token + " requestType = " + requestType
                + " mCi[0] = " + mCi[0]);
        boolean isPass = false;
        isPass = enforcePermissionCheck(
                getCallerSignature(getCallerPackageName()),
                getCallerPackageName(),
                mRequestHandler.obtainMessage(RSU_MESSAGE_INIT_REQUEST, 0, token));
        if (!isPass) {
            loge("remoteSimlockGenerateRequest, permission denied");
            return REMOTE_SIM_UNLOCK_ERROR;
        }
        if (mCi[0] != null) {
            mCi[0].sendRsuRequest(constructRsuRequestInfoRilRequest(OPERATOR_VERIZON,
                    RsuRequest.RSU_REQUEST_INIT_REQUEST,
                    requestType,
                    USELESS_STRING_VALUE),
                    mRequestHandler.obtainMessage(RSU_MESSAGE_INIT_REQUEST, 0, token));
            return REMOTE_SIM_UNLOCK_SUCCESS;
        }

        return REMOTE_SIM_UNLOCK_ERROR;
    }

    /**
    * This API request Network Lock Module to start/stop unlock device timer to unlock the device
    * for the specific time. Temporary Unlocking allows device to have data connectivity to
    * download the blob.
    * The Network Lock Module unlocks the device only once per UICC power cycle.
    *
    * @param token
    * Identifier that can be used by Client to match the asynchronous response for this request.
    *
    * @param requestType
    * SIMLOCK_REMOTE_START_UNLOCK_TIMER = 1;
    * SIMLOCK_REMOTE_STOP_UNLOCK_TIMER = 2;
    *
    * @return
    * SIMLOCK_SUCCESS =  0;
    * SIMLOCK_ERROR  =  1;
    */
    public int remoteSimlockUnlockTimer(int token, int requestType) {
        logd("remoteSimlockUnlockTimer token = "
                + token + " requestType = " + requestType + " mCi[0] = " + mCi[0]);
        boolean isPass = false;
        isPass = enforcePermissionCheck(
                getCallerSignature(getCallerPackageName()),
                getCallerPackageName(),
                mRequestHandler.obtainMessage(RSU_MESSAGE_UNLOCK_TIMER, 0, token));
        if (!isPass) {
            loge("remoteSimlockUnlockTimer, permission denied");
            return REMOTE_SIM_UNLOCK_ERROR;
        }
        if (mCi[0] != null) {
            mCi[0].sendRsuRequest(constructRsuRequestInfoRilRequest(OPERATOR_VERIZON,
                    RsuRequest.RSU_REQUEST_UNLOCK_TIMER,
                    requestType,
                    USELESS_STRING_VALUE),
                    mRequestHandler.obtainMessage(RSU_MESSAGE_UNLOCK_TIMER, 0, token));
            return REMOTE_SIM_UNLOCK_SUCCESS;
        }

        return REMOTE_SIM_UNLOCK_ERROR;
    }

    /**
    * This API gets Sim lock status of the device. Exposed to other modules without permission.
    * Common API for all RSU features.
    *
    * @param cb
    * message callback.
    *
    * @return
    * SIMLOCK_SUCCESS =  0;
    * SIMLOCK_ERROR  =  1;
    */
    public int remoteSimlockGetSimlockStatusEx(Object cb) {
        logd("remoteSimlockGetSimlockStatusEx mCi[0] = " + mCi[0]);
        if (mCi[0] != null) {
            mCi[0].sendRsuRequest(constructRsuRequestInfoRilRequest(OPERATOR_VERIZON,
                    RsuRequest.RSU_REQUEST_GET_LOCK_STATUS,
                    USELESS_INT_VALUE,
                    USELESS_STRING_VALUE),
                    mRequestHandler.obtainMessage(RSU_MESSAGE_GET_LOCK_STATUS, 0, DUMMY_TOKEN,
                            cb));
            return REMOTE_SIM_UNLOCK_SUCCESS;
        }

        return REMOTE_SIM_UNLOCK_ERROR;
    }

    private Handler mRequestHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            AsyncResult ar = (AsyncResult) msg.obj;
            RsuResponseInfo rri = (RsuResponseInfo) ar.result;
            int token = msg.arg2;

            logd("callback size = " + mCb.size()
                    + " exception = " + ar.exception
                    + " errCode = " + (rri != null ? rri.errCode : "NULL")
                    + " token = " + token
                    + " what = " + msg.what);

            // Avoid mCb concurrent operation
            synchronized (Op12RsuSml12.this) {
                switch (msg.what) {
                    case RSU_MESSAGE_UPDATE_LOCK_DATA:
                        byte[] lockData = null;
                        if (rri != null && rri.data != null) {
                            lockData = hexs2Bytes(rri.data);
                        }
                        for (IMtkRsuSmlCallback cb : mCb) {
                            logd("callback cb = " + cb);
                            if (ar.exception != null || lockData == null
                                    || lockData.length == 0) {
                                cb.remoteSimlockProcessSimlockDataResponse(token,
                                        REMOTE_SIM_UNLOCK_ERROR, null);
                            } else {
                                cb.remoteSimlockProcessSimlockDataResponse(token,
                                        REMOTE_SIM_UNLOCK_SUCCESS, lockData);
                            }
                        }
                        break;
                    case RSU_MESSAGE_GET_LOCK_VERSION:
                        int version = -1;
                        int majorVersion = -1;
                        int minorVersion = -1;
                        if (rri != null) {
                            version = rri.version;
                        }
                        if (version >= 0) {
                            majorVersion = (version >> 16 & 0xFFFF);
                            minorVersion = (version & 0xFFFF);
                        }
                        logd("majorVersion = " + majorVersion + ", minorVersion = "
                                + minorVersion);
                        for (IMtkRsuSmlCallback cb : mCb) {
                            if (ar.exception != null || majorVersion == -1 || minorVersion == -1) {
                                cb.remoteSimlockGetVersionResponse(token,
                                        REMOTE_SIM_UNLOCK_ERROR, -1, -1);
                            } else {
                                cb.remoteSimlockGetVersionResponse(token,
                                        REMOTE_SIM_UNLOCK_SUCCESS, majorVersion, minorVersion);
                            }
                        }
                        break;
                    case RSU_MESSAGE_GET_LOCK_STATUS:
                        if (rri == null) {
                            logd("RSU_MESSAGE_GET_LOCK_STATUS, rri == null");
                        } else {
                            logd("unlockStatus = " + rri.status + ", unlockTime = " + rri.time);
                        }
                        if ((token == DUMMY_TOKEN) && (ar.userObj != null)) {
                            // for remoteSimlockGetSimlockStatusEx
                            Message cb = (Message) ar.userObj;
                            if (ar.exception != null) {
                                AsyncResult.forMessage(cb).exception = ar.exception;;
                            } else {
                                AsyncResult.forMessage(cb, rri.status, null);
                            }
                            cb.sendToTarget();
                            break;
                        }
                        for (IMtkRsuSmlCallback cb : mCb) {
                            // for remoteSimlockGetSimlockStatus
                            if (ar.exception != null) {
                                cb.remoteSimlockGetSimlockStatusResponse(token,
                                        REMOTE_SIM_UNLOCK_ERROR, -1, -1);
                            } else {
                                cb.remoteSimlockGetSimlockStatusResponse(token,
                                        REMOTE_SIM_UNLOCK_SUCCESS, rri.status, rri.time);
                            }
                        }
                        break;
                    case RSU_MESSAGE_INIT_REQUEST:
                        byte[] initRequest = null;
                        if (rri != null && rri.data != null) {
                            logi("RSU_MESSAGE_INIT_REQUEST rri.data = " + rri.data);
                            initRequest = hexs2Bytes(rri.data);
                        }
                        for (IMtkRsuSmlCallback cb : mCb) {
                            if (ar.exception != null || initRequest == null
                                    || initRequest.length == 0) {
                                cb.remoteSimlockGenerateRequestResponse(token,
                                        REMOTE_SIM_UNLOCK_ERROR, null);
                            } else {
                                cb.remoteSimlockGenerateRequestResponse(token,
                                        REMOTE_SIM_UNLOCK_SUCCESS, initRequest);
                            }
                        }
                        break;
                    case RSU_MESSAGE_UNLOCK_TIMER:
                        if (rri == null) {
                            logd("RSU_MESSAGE_UNLOCK_TIMER, rri == null");
                        } else {
                            logd("timerStatus = " + rri.errCode + ", passedTime = " + rri.time);
                        }
                        for (IMtkRsuSmlCallback cb : mCb) {
                            if (ar.exception != null) {
                                cb.remoteSimlockUnlockTimerResponse(token,
                                        REMOTE_SIM_UNLOCK_ERROR, -1);
                            } else {
                                cb.remoteSimlockUnlockTimerResponse(token, rri.errCode,
                                        (int) rri.time);
                            }
                        }
                        break;
                    default:
                        logi("Unknown request or request that need not handle");
                        break;
                }
            }
        }
    };

    private Handler mUrcHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            AsyncResult ar = (AsyncResult) msg.obj;
            logd(" what = " + msg.what);
            switch (msg.what) {
                case RSU_MESSAGE_UNSOL_RSU_EVENT:
                    String[] event = (String[]) ar.result;
                    logd("RSU_MESSAGE_UNSOL_RSU_EVENT eventId = " + event[0]
                            + " eventString = " + event[1]);
                    break;
                default:
                    logi("Unknown urc or urc that need not handle");
                    break;
            }
        }
    };

    private RsuRequestInfo constructRsuRequestInfoRilRequest(int opId, int requestId,
            int requestType, String data) {
        RsuRequestInfo rri = new RsuRequestInfo();
        rri.opId = opId;
        rri.requestId = requestId;
        rri.requestType = requestType;
        rri.data = data;
        return rri;
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

    private String getCallerPackageName() {
        int callerPid = Binder.getCallingPid();
        int callerUid = Binder.getCallingUid();
        String packageName = mContext.getPackageManager().getNameForUid(callerUid);
        if (TextUtils.isEmpty(packageName)) {
            ActivityManager activityManager = (ActivityManager) mContext.getSystemService(
                Context.ACTIVITY_SERVICE);
            if (activityManager != null) {
                List<ActivityManager.RunningAppProcessInfo> list = activityManager
                        .getRunningAppProcesses();
                for (ActivityManager.RunningAppProcessInfo info : list) {
                    if (info.pid == callerPid) {
                        packageName = info.processName;
                        break;
                    }
                }
            }
        }
        logd("getCallerPackageName callerPid = " + callerPid + "callerUid = " + callerUid
                + " processName = " + packageName);
        return packageName;
    }

    private Signature[] getCallerSignature(String packageName) {
        if ((packageName == null) || (packageName.length() == 0)) {
            logd("getCallerSignature fail, packageName is null");
            return null;
        }
        PackageManager pm = mContext.getPackageManager();
        PackageInfo pi;
        try {
            pi = pm.getPackageInfo(packageName, PackageManager.GET_SIGNATURES);
            if (pi == null) {
                logd("getCallerSignature fail, package information is null for packageName "
                        + packageName);
                return null;
            }
        } catch (PackageManager.NameNotFoundException localNameNotFoundException) {
            logd("getCallerSignature fail, has no packageName " + packageName);
            return null;
        }
        logd("getCallerSignature packageName " + packageName + ", signatures " + pi.signatures);
        return pi.signatures;
     }

    private boolean enforcePermissionCheck(Signature[] signature, String packageName,
            Message msg) {
        mContext.checkCallingOrSelfPermission("com.verizon.permission.ACCESS_REMOTE_SIMLOCK");
        if (signature != null) {
            for (Signature s : signature) {
                byte[] certHash256 = getCertHash(s, "SHA-256");
                if (matches(certHash256, packageName)) {
                    return true;
                }
            }
        }
        return false;
        //sendErrorToCallback(msg);
        //throw new SecurityException(packageName + ",  does not pass permission check");
    }

    /**
     * Converts a Signature into a Certificate hash usable for comparison.
     */
    private byte[] getCertHash(Signature signature, String algo) {
        try {
            MessageDigest md = MessageDigest.getInstance(algo);
            return md.digest(signature.toByteArray());
        } catch (NoSuchAlgorithmException ex) {
            loge("getCertHash NoSuchAlgorithmException " + ex);
        }
        return null;
    }

    private boolean matches(byte[] certHash, String packageName) {
        for (String[] pkgSsigP : PACKAGENAME_SHA256SIG_PAIR) {
            if (certHash != null && Arrays.equals(hexs2Bytes(pkgSsigP[1]), certHash) &&
                (!TextUtils.isEmpty(packageName) && packageName.equals(pkgSsigP[0]))) {
                logd("matches return true");
                return true;
            }
        }
        loge("matches packageName = " + packageName + " certHashHex = " + bytes2Hexs(certHash)
                + ", hasSystemFeature() = " + mContext.getPackageManager().hasSystemFeature(
                "com.verizon.feature.networklock"));
        return false;
    }

    private void sendErrorToCallback(Message msg) {
        CommandException ex = CommandException.fromRilErrno(RILConstants.GENERIC_FAILURE);
        RsuResponseInfo ret = new RsuResponseInfo();
        if (msg != null) {
            AsyncResult.forMessage(msg, ret, ex);
            msg.sendToTarget();
        }
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
