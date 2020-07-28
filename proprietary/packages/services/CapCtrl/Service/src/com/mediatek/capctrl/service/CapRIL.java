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

import android.content.Context;
import android.hardware.radio.V1_0.RadioResponseInfo;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.HwBinder;
import android.os.Message;
import android.os.RemoteException;
import android.os.SystemClock;
import android.os.WorkSource;
import android.util.SparseArray;

import com.android.internal.telephony.CommandException;
import android.telephony.Rlog;

import vendor.mediatek.hardware.mtkradioex.V1_0.IMtkRadioEx;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Random;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicLong;

import com.mediatek.capctrl.aidl.AuthRequestInfo;
import com.mediatek.capctrl.aidl.CertRequestInfo;
import com.mediatek.capctrl.aidl.EnableCapabilityRequestInfo;

/**
 * RILRequest is copied from Google default RILRequest.java
 * but has some simplification.
 */
class RILRequest {
    private static Random sRandom = new Random();
    private static AtomicInteger sNextSerial = new AtomicInteger(0);
    private static Object sPoolSync = new Object();
    private static RILRequest sPool = null;
    private static int sPoolSize = 0;
    private static final int MAX_POOL_SIZE = 4;

    int mSerial;
    int mRequest;
    Message mResult;
    RILRequest mNext;
    int mWakeLockType;
    WorkSource mWorkSource;
    String mClientId;
    long mStartTimeMs;

    static RILRequest obtain(int request, Message result) {
        RILRequest rr = null;
        synchronized(sPoolSync) {
            if (sPool != null) {
                rr = sPool;
                sPool = rr.mNext;
                rr.mNext = null;
                sPoolSize--;
            }
        }

        if (rr == null) {
            rr = new RILRequest();
        }

        rr.mSerial = sNextSerial.getAndIncrement();
        rr.mRequest = request;
        rr.mResult = result;
        rr.mWakeLockType = -1;
        rr.mWorkSource = null;
        rr.mStartTimeMs = SystemClock.elapsedRealtime();

        if (result != null && result.getTarget() == null) {
            throw new NullPointerException("Message target must not be null");
        }

        return rr;
    }

    static RILRequest obtain(int request, Message result, WorkSource workSource) {
        RILRequest rr = null;
        rr = obtain(request, result);
        if(workSource != null) {
            rr.mWorkSource = workSource;
            rr.mClientId = String.valueOf(workSource.get(0)) + ":" + workSource.getName(0);
        }
        return rr;
    }

    void release() {
        synchronized (sPoolSync) {
            if (sPoolSize < MAX_POOL_SIZE) {
                mNext = sPool;
                sPool = this;
                sPoolSize++;
                mResult = null;
            }
        }
    }

    static void resetSerial() {
        sNextSerial.set(sRandom.nextInt());
    }

    String serialString() {
        StringBuilder sb = new StringBuilder(8);
        long adjustedSerial = (((long)mSerial) - Integer.MIN_VALUE) % 10000;
        String sn = Long.toString(adjustedSerial);
        sb.append('[');
        for (int i = 0, s = sn.length() ; i < 4 - s; i++) {
            sb.append('0');
        }
        sb.append(sn);
        sb.append(']');
        return sb.toString();
    }

    void onError(int error, Object ret) {
        CommandException ex = CommandException.fromRilErrno(error);
        if (mResult != null) {
            AsyncResult.forMessage(mResult, ret, ex);
            mResult.sendToTarget();
        }
    }
}

/**
 * CapRIL, getService of IMtkRadioEx and send RILRequest.
 * Current only for syncDataSettings and syncScreenState.
 */
public class CapRIL {
    private static final String LOG_TAG = "CapRIL";
    static final boolean RILJ_LOGD = true;

    private static final String[] HIDL_SERVICE_NAME_MTK =
            {"mtkCap1", "mtkCap2", "mtkCap3", "mtkCap4"};

    private static final int IRADIO_GET_SERVICE_DELAY_MILLIS = 4 * 1000;
    private static final int RADIO_NOT_AVAILABLE = 1;

    private static final int RIL_REQUEST_ROUTE_CERTIFICATE = 0;
    private static final int RIL_REQUEST_ROUTE_AUTH = 1;
    private static final int RIL_REQUEST_ENABLE_CAPABILITY = 2;
    private static final int RIL_REQUEST_CANCEL_CERTIFICATE = 3;

    private static final int BASE = 0;
    private static final int EVENT_RADIO_PROXY_DEAD = BASE + 1;

    volatile IMtkRadioEx mRadioProxyMtk;
    private CapRadioResponse mCapRadioResponse;
    private final CapRadioProxyDeathRecipient mCapRadioProxyDeathRecipient;
    private final AtomicLong mRadioProxyCookie;
    protected WorkSource mRILDefaultWorkSource;
    private SparseArray<RILRequest> mRequestList = new SparseArray<RILRequest>();
    private int mPhoneId;

    public CapRIL(Context context, int phoneId) {
        mPhoneId = phoneId;

        mCapRadioProxyDeathRecipient = new CapRadioProxyDeathRecipient();
        mRadioProxyCookie = new AtomicLong(0);
        mRILDefaultWorkSource = new WorkSource(context.getApplicationInfo().uid,
                context.getPackageName());

        mCapRadioResponse = new CapRadioResponse(this);
        getMtkRadioExProxy(null);
    }

    final class CapRadioProxyDeathRecipient implements HwBinder.DeathRecipient {
        @Override
        public void serviceDied(long cookie) {
            mHandler.sendMessage(mHandler.obtainMessage(EVENT_RADIO_PROXY_DEAD, cookie));
        }
    }

    private IMtkRadioEx getMtkRadioExProxy(Message result) {
        if (mRadioProxyMtk != null) {
            return mRadioProxyMtk;
        }

        try {
            mRadioProxyMtk = IMtkRadioEx.getService(HIDL_SERVICE_NAME_MTK[mPhoneId], false);
            if (mRadioProxyMtk != null) {
                mRadioProxyMtk.linkToDeath(mCapRadioProxyDeathRecipient,
                        mRadioProxyCookie.incrementAndGet());
                mRadioProxyMtk.setResponseFunctionsCap(mCapRadioResponse);
            } else {
                Rlog.d(LOG_TAG,
                        "getMtkRadioExProxy: mRadioProxyMtk == null");
            }
        } catch (RemoteException | RuntimeException e) {
            mRadioProxyMtk = null;
            Rlog.d(LOG_TAG,
                    "MtkRadioProxy getService/setResponseFunctions: " + e);
        }

        if (mRadioProxyMtk == null) {
            if (result != null) {
                AsyncResult.forMessage(result, null,
                        CommandException.fromRilErrno(RADIO_NOT_AVAILABLE));
                result.sendToTarget();
            }

            mHandler.sendMessageDelayed(
                    mHandler.obtainMessage(EVENT_RADIO_PROXY_DEAD, mRadioProxyCookie.get()),
                    IRADIO_GET_SERVICE_DELAY_MILLIS);
        }
        return mRadioProxyMtk;
    }

    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case EVENT_RADIO_PROXY_DEAD:
                    if ((long) msg.obj == mRadioProxyCookie.get()) {
                        resetProxyAndRequestList();
                    }
                    break;
                default:
                    Rlog.d(LOG_TAG,
                            "Unhandled message with number: " + msg.what);
                    break;
            }
        }
    };

    private void clearRequestList(int error) {
        synchronized (mRequestList) {
            int count = mRequestList.size();
            for (int i = 0; i < count; i++) {
                RILRequest rr = mRequestList.valueAt(i);
                rr.onError(error, null);
                rr.release();
            }
            mRequestList.clear();
        }
    }

    private void resetProxyAndRequestList() {
        Rlog.d(LOG_TAG, "resetProxyAndRequestList");
        mRadioProxyMtk = null;
        mRadioProxyCookie.incrementAndGet();
        RILRequest.resetSerial();
        clearRequestList(RADIO_NOT_AVAILABLE);
        getMtkRadioExProxy(null);
    }

    public void routeCertificate(CertRequestInfo request, Message result) {
        IMtkRadioEx radioProxy = getMtkRadioExProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(RIL_REQUEST_ROUTE_CERTIFICATE, result,
                    mRILDefaultWorkSource);
            if (RILJ_LOGD) {
                Rlog.d(LOG_TAG, rr.serialString() + "> " + requestToString(rr.mRequest)
                        + " uid " + request.mCallerId
                        + " request.mCert: %s" + Arrays.toString(request.mCert)
                        + " request.mMsg: %s" + Arrays.toString(request.mMsg));
            }

            ArrayList<Byte> certArrList = new ArrayList<>();
            for (int i = 0; i < request.mCert.length; i++) {
                certArrList.add(request.mCert[i]);
            }
            ArrayList<Byte> msgArrList = new ArrayList<>();
            for (int i = 0; i < request.mMsg.length; i++) {
                msgArrList.add(request.mMsg[i]);
            }

            try {
                radioProxy.routeCertificate(rr.mSerial, request.mCallerId, certArrList, msgArrList);
            } catch (RemoteException | RuntimeException e) {
                resetProxyAndRequestList();
            }
        }
    }

    public void routeAuthMessage(AuthRequestInfo request, Message result) {
        IMtkRadioEx radioProxy = getMtkRadioExProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(RIL_REQUEST_ROUTE_AUTH, result,
                    mRILDefaultWorkSource);
            if (RILJ_LOGD) {
                Rlog.d(LOG_TAG, rr.serialString() + "> " + requestToString(rr.mRequest)
                        + " uid " + request.mCallerId
                        + " request.mAuthMsg: %s" + Arrays.toString(request.mAuthMsg));
            }

            ArrayList<Byte> msgArrList = new ArrayList<>();
            for (int i = 0; i < request.mAuthMsg.length; i++) {
                msgArrList.add(request.mAuthMsg[i]);
            }

            try {
                radioProxy.routeAuthMessage(rr.mSerial, request.mCallerId, msgArrList);
            } catch (RemoteException | RuntimeException e) {
                resetProxyAndRequestList();
            }
        }
    }

    public void enableCapability(EnableCapabilityRequestInfo request, Message result) {
        IMtkRadioEx radioProxy = getMtkRadioExProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(RIL_REQUEST_ENABLE_CAPABILITY, result,
                    mRILDefaultWorkSource);
            if (RILJ_LOGD) {
                Rlog.d(LOG_TAG, rr.serialString() + "> " + requestToString(rr.mRequest)
                        + " uid " + request.mCallerId
                        + "feature name " + request.mFeatureName
                        + "toActive " + request.mToActive);
            }

            try {
                radioProxy.enableCapabaility(rr.mSerial,
                        convertNullToEmptyString(request.mFeatureName), request.mCallerId,
                        request.mToActive);
            } catch (RemoteException | RuntimeException e) {
                resetProxyAndRequestList();
            }
        }
    }

    public void abortCertificate(int uid, Message result) {
        IMtkRadioEx radioProxy = getMtkRadioExProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(RIL_REQUEST_CANCEL_CERTIFICATE, result,
                    mRILDefaultWorkSource);
            if (RILJ_LOGD) {
                Rlog.d(LOG_TAG, rr.serialString() + "> " + requestToString(rr.mRequest)
                        + " uid: " + uid);
            }

            try {
                radioProxy.abortCertificate(rr.mSerial, uid);
            } catch (RemoteException | RuntimeException e) {
                resetProxyAndRequestList();
            }
        }
    }

    private RILRequest findAndRemoveRequestFromList(int serial) {
        RILRequest rr = null;
        synchronized (mRequestList) {
            rr = mRequestList.get(serial);
            if (rr != null) {
                mRequestList.remove(serial);
            }
        }
        return rr;
    }

    public RILRequest processResponse(RadioResponseInfo responseInfo) {
        int serial = responseInfo.serial;
        int error = responseInfo.error;
        int type = responseInfo.type;

        RILRequest rr = findAndRemoveRequestFromList(serial);
        if (rr == null) {
            Rlog.d(LOG_TAG, "processResponse: Unexpected response! serial: "
                    + serial + " error: " + error);
        }

        return rr;
    }

    public void processResponseDone(RILRequest rr, RadioResponseInfo responseInfo, Object ret) {
        if (responseInfo.error == 0) {
            Rlog.d(LOG_TAG, rr.serialString()
                    + "< " + requestToString(rr.mRequest) + " [Phone" + mPhoneId + "]");
        } else {
            Rlog.d(LOG_TAG, rr.serialString()
                    + "< " + requestToString(rr.mRequest)
                    + " [Phone" + mPhoneId + "]"
                    + " error " + responseInfo.error);
            rr.onError(responseInfo.error, ret);
        }

        if (rr != null) {
            rr.release();
        }
    }

    private RILRequest obtainRequest(int request, Message result, WorkSource workSource) {
        RILRequest rr = RILRequest.obtain(request, result, workSource);
        synchronized (mRequestList) {
            mRequestList.append(rr.mSerial, rr);
        }

        return rr;
    }

    private String requestToString(int request) {
        switch(request) {
            case RIL_REQUEST_ROUTE_CERTIFICATE:
                return "RIL_REQUEST_ROUTE_CERTIFICATE";
            case RIL_REQUEST_ROUTE_AUTH:
                return "RIL_REQUEST_ROUTE_AUTH";
            case RIL_REQUEST_ENABLE_CAPABILITY:
                return "RIL_REQUEST_ENABLE_CAPABILITY";
            case RIL_REQUEST_CANCEL_CERTIFICATE:
                return "RIL_REQUEST_CANCEL_CERTIFICATE";
            default: return "<unknown request>";
        }
    }

    /**
     * Convert Null String to Empty String
     * @param string
     * @return
     */
    protected String convertNullToEmptyString(String string) {
        return string != null ? string : "";
    }

    /** Convert an ArrayList of Bytes to an exactly-sized primitive array */
    public static byte[] arrayListToPrimitiveArray(ArrayList<Byte> bytes) {
        byte[] ret = new byte[bytes.size()];
        for (int i = 0; i < ret.length; i++) {
            ret[i] = bytes.get(i);
        }
        return ret;
    }
}
