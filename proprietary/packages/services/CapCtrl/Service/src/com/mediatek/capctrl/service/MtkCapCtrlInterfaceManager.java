/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
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

import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.AsyncResult;
import android.os.Binder;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.Process;
import android.os.ResultReceiver;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.text.TextUtils;
import android.util.ArraySet;
import android.util.Log;
import android.util.Pair;

import android.telephony.TelephonyManager;
import android.telephony.Rlog;
import com.android.internal.telephony.CommandException;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneFactory;

import com.mediatek.capctrl.aidl.AuthRequestInfo;
import com.mediatek.capctrl.aidl.AuthResponse;
import com.mediatek.capctrl.aidl.CertRequestInfo;
import com.mediatek.capctrl.aidl.CertResponse;
import com.mediatek.capctrl.aidl.EnableCapabilityRequestInfo;
import com.mediatek.capctrl.aidl.IMtkCapCtrl;

/**
 * Implementation of the MtkCapCtrlInterfaceManager interface.
 */
public class MtkCapCtrlInterfaceManager extends IMtkCapCtrl.Stub {
    protected static final String LOG_TAG = "MtkCapCtrlIntfMgrEx";

    private static final int CMD_ROUTE_CERTIFICATE = 0;
    private static final int EVENT_ROUTE_CERTIFICATE_DONE = 1;
    private static final int CMD_ROUTE_AUTHENTICATION = 2;
    private static final int EVENT_ROUTE_AUTHENTICATION_DONE = 3;
    private static final int CMD_ENABLE_CAPABILITY = 4;
    private static final int EVENT_ENABLE_CAPABILITY_DONE = 5;
    private static final int CMD_CANCEL_CERTIFICATE = 6;
    private static final int EVENT_CANCEL_CERTIFICATE_DONE = 7;

    private static final String MD_MAIN_PROTOCOL = "modem_sys1_ps0";

    /** The singleton instance. */
    private static MtkCapCtrlInterfaceManager sInstance;

    private Context mContext;
    private CapRIL[] mCapRILs;
    private MainThreadHandler mMainThreadHandler;

    /**
     * A request object for use with {@link MainThreadHandler}. Requesters should wait() on the
     * request after sending. The main thread will notify the request when it is complete.
     */
    private static final class MainThreadRequest {
        /** The argument to use for the request */
        public Object argument;
        /** The result of the request that is run on the main thread */
        public Object result;

        public MainThreadRequest(Object argument) {
            this.argument = argument;
        }
    }

    /**
     * A handler that processes messages on the main thread in the phone process. Since many
     * of the Phone calls are not thread safe this is needed to shuttle the requests from the
     * inbound binder threads to the main thread in the phone process.  The Binder thread
     * may provide a {@link MainThreadRequest} object in the msg.obj field that they are waiting
     * on, which will be notified when the operation completes and will contain the result of the
     * request.
     *
     * <p>If a MainThreadRequest object is provided in the msg.obj field,
     * note that request.result must be set to something non-null for the calling thread to
     * unblock.
     */
    public class MainThreadHandler extends Handler {
        @Override
        public void handleMessage(Message msg) {
            MainThreadRequest request;
            Message onCompleted;
            AsyncResult ar;

            Rlog.d(LOG_TAG, "MainThreadHandler.handleMessage : " + msg.what);
            switch (msg.what) {
                case CMD_ROUTE_CERTIFICATE:
                    request = (MainThreadRequest)msg.obj;
                    onCompleted = obtainMessage(EVENT_ROUTE_CERTIFICATE_DONE, request);
                    // always send to main protocol
                    mCapRILs[getMainPhoneId()].
                            routeCertificate((CertRequestInfo)request.argument, onCompleted);
                    break;
                case CMD_ROUTE_AUTHENTICATION:
                    request = (MainThreadRequest)msg.obj;
                    onCompleted = obtainMessage(EVENT_ROUTE_AUTHENTICATION_DONE, request);
                    // always send to main protocol
                    mCapRILs[getMainPhoneId()].
                            routeAuthMessage((AuthRequestInfo)request.argument, onCompleted);
                    break;
                case CMD_ENABLE_CAPABILITY:
                    request = (MainThreadRequest)msg.obj;
                    onCompleted = obtainMessage(EVENT_ENABLE_CAPABILITY_DONE, request);
                    // always send to main protocol
                    mCapRILs[getMainPhoneId()].
                            enableCapability((EnableCapabilityRequestInfo)request.argument,
                            onCompleted);
                    break;
                case CMD_CANCEL_CERTIFICATE:
                    request = (MainThreadRequest)msg.obj;
                    onCompleted = obtainMessage(EVENT_CANCEL_CERTIFICATE_DONE, request);
                    // always send to main protocol
                    mCapRILs[getMainPhoneId()].
                            abortCertificate((int)request.argument, onCompleted);
                    break;
                case EVENT_ROUTE_CERTIFICATE_DONE:
                case EVENT_ROUTE_AUTHENTICATION_DONE:
                case EVENT_ENABLE_CAPABILITY_DONE:
                case EVENT_CANCEL_CERTIFICATE_DONE:
                    ar = (AsyncResult)msg.obj;
                    request = (MainThreadRequest)ar.userObj;
                    request.result = ar;
                    synchronized (request) {
                        request.notifyAll();
                    }
                    break;
                default:
                    Rlog.w(LOG_TAG, "MainThreadHandler: unexpected message code: " + msg.what);
                    break;
            }
        }
    }

    /**
     * Posts the specified command to be executed on the main thread,
     * waits for the request to complete, and returns the result.
     * @see #sendRequestAsync
     */
    private Object sendRequest(int command, Object argument) {
        //return sendRequest(command, argument, SubscriptionManager.INVALID_SUBSCRIPTION_ID);
        if (Looper.myLooper() == mMainThreadHandler.getLooper()) {
            throw new RuntimeException("This method will deadlock if called from the main thread.");
        }

        MainThreadRequest request = new MainThreadRequest(argument/*, subId*/);
        Message msg = mMainThreadHandler.obtainMessage(command, request);
        msg.sendToTarget();

        // Wait for the request to complete
        synchronized (request) {
            while (request.result == null) {
                try {
                    request.wait();
                } catch (InterruptedException e) {
                    // Do nothing, go back and wait until the request is complete
                }
            }
        }
        return request.result;
    }

    public static MtkCapCtrlInterfaceManager init(Context context, CapRIL[] capRILs) {
        synchronized (MtkCapCtrlInterfaceManager.class) {
            if (sInstance == null) {
                sInstance = new MtkCapCtrlInterfaceManager(context, capRILs);
            } else {
                Log.wtf(LOG_TAG, "init() called multiple times!  sInstance = " + sInstance);
            }
            return sInstance;
        }
    }

    /** Private constructor; @see init() */
    protected MtkCapCtrlInterfaceManager(Context context, CapRIL[] capRILs) {
        mCapRILs = capRILs;
        mMainThreadHandler = new MainThreadHandler();
        publish();
    }

    private void publish() {
        Rlog.d(LOG_TAG, "publish: " + this);

        ServiceManager.addService("capctrl", this);
    }

    public CertResponse routeCertificate(byte[] cert, byte[] msg) {
        CertResponse response = new CertResponse();
        try {
            CertRequestInfo request = new CertRequestInfo(Binder.getCallingUid(), cert, msg);
            AsyncResult result = (AsyncResult)sendRequest(CMD_ROUTE_CERTIFICATE, request);
            if(result.exception == null) {
                if (result.result != null) {
                    CertResponse responseData = (CertResponse)(result.result);
                    response.mError = responseData.mError;
                    if (responseData.mRnd != null) {
                        response.mRnd = new byte[responseData.mRnd.length];
                        System.arraycopy(responseData.mRnd, 0, response.mRnd, 0,
                                responseData.mRnd.length);
                    }
                    response.mCustId = responseData.mCustId;
                }
            } else {
                CertResponse responseData = (CertResponse)(result.result);
                response.mError = responseData.mError;
                Rlog.e(LOG_TAG, "routeCertificate: Response error " + response.mError);
            }
        } catch (RuntimeException e) {
            response.mError = (CommandException.Error.GENERIC_FAILURE.ordinal());
            Rlog.e(LOG_TAG, "routeCertificate: Runtime Exception " + e);
        }
        return response;
    }

    public AuthResponse routeAuthMessage(byte[] msg) {
        AuthResponse response = new AuthResponse();
        try {
            AuthRequestInfo request = new AuthRequestInfo(Binder.getCallingUid(), msg);
            AsyncResult result = (AsyncResult)sendRequest(CMD_ROUTE_AUTHENTICATION, request);
            if(result.exception == null) {
                if (result.result != null) {
                    AuthResponse responseData = (AuthResponse)(result.result);
                    response.mError = responseData.mError;
                    if (responseData.mDevId != null) {
                        response.mDevId = new byte[responseData.mDevId.length];
                        System.arraycopy(responseData.mDevId, 0, response.mDevId, 0,
                                responseData.mDevId.length);
                    }
                    response.mCapMask = responseData.mCapMask;
                }
            } else {
                AuthResponse responseData = (AuthResponse)(result.result);
                response.mError = responseData.mError;
                Rlog.e(LOG_TAG, "routeAuthMessage: Response error " + response.mError);
            }
        } catch (RuntimeException e) {
            response.mError = (CommandException.Error.GENERIC_FAILURE.ordinal());
            Rlog.e(LOG_TAG, "routeAuthMessage: Runtime Exception " + e);
        }
        return response;
    }

    public int enableCapabaility(String id, int toActive) {
        int response = 0;
        try {
            EnableCapabilityRequestInfo request = new EnableCapabilityRequestInfo(id,
                    Binder.getCallingUid(), toActive);
            AsyncResult result = (AsyncResult)sendRequest(CMD_ENABLE_CAPABILITY, request);
            if (result.result != null) {
                response = (int)(result.result);
            }
        } catch (RuntimeException e) {
            response = (CommandException.Error.GENERIC_FAILURE.ordinal());
            Rlog.e(LOG_TAG, "enableCapabaility: Runtime Exception" + e);
        }
        return response;
    }

    public int abortCertificate() {
        int response = 0;
        try {
            AsyncResult result = (AsyncResult)sendRequest(CMD_CANCEL_CERTIFICATE,
                    Binder.getCallingUid());
            if (result.result != null) {
                response = (int)(result.result);
            }
            Rlog.e(LOG_TAG, "abortCertificate: Response error " + response);
        } catch (RuntimeException e) {
            response = (CommandException.Error.GENERIC_FAILURE.ordinal());
            Rlog.e(LOG_TAG, "abortCertificate: Runtime Exception" + e);
        }
        return response;
    }

    private int getMainPhoneId() {
        int simCount = TelephonyManager.getDefault().getSimCount();
        String modemUuid = null;
        Phone phone;
        for (int phoneId = 0; phoneId < simCount; phoneId++) {
            phone = PhoneFactory.getPhone(phoneId);
            modemUuid = phone.getModemUuId();
            Rlog.d(LOG_TAG, "getMainPhoneId[" + phoneId + "]" + ": modemUuid: " + modemUuid);
            if (MD_MAIN_PROTOCOL.equals(modemUuid)) {
                return phoneId;
            }
        }
        return 0;
    }
}
