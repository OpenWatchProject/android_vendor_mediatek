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

package com.mediatek.internal.telephony;

import android.content.Context;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.SystemProperties;
import android.telephony.Rlog;
import android.telephony.TelephonyManager;

import com.android.internal.telephony.Phone;

import java.util.ArrayList;
import java.util.Vector;

 /**
 * {@hide}
 */
public class MtkSuppServQueueHelper {
    public static final String LOG_TAG = "SuppServQueueHelper";
    private static final boolean DBG = true;

    /* Singleton instance for MtkSuppServQueueHelper */
    private static MtkSuppServQueueHelper instance = null;

    private Phone[] mPhones;           // store all the Phone objects for different slots
    private Context mContext;
    private SuppServQueueHelperHandler mHandler;

    /* Event operation */
    private static final int EVENT_SS_SEND = 1;
    private static final int EVENT_SS_RESPONSE = 2;

    /* Static objects to do synchronized */
    private static Object pausedSync = new Object();

    private MtkSuppServQueueHelper(Context context, Phone[] phones) {
        mContext = context;
        mPhones = phones;
    }

    public static MtkSuppServQueueHelper makeSuppServQueueHelper(Context context, Phone[] phones) {
        if (context == null || phones == null) {
            return null;
        }

        if (instance == null) {
            Rlog.d(LOG_TAG, "Create MtkSuppServQueueHelper singleton instance, phones.length = " +
                    phones.length);
            instance = new MtkSuppServQueueHelper(context, phones);
        } else {
            instance.mContext = context;
            instance.mPhones = phones;
        }
        return instance;
    }

    public void init(Looper looper) {
        Rlog.d(LOG_TAG, "Initialize SuppServQueueHelper!");
        mHandler = new SuppServQueueHelperHandler(looper);
    }

    /* Dispose SuppServQueueHelperHandler thread */
    public void dispose() {
        Rlog.d(LOG_TAG, "dispose.");
    }

    /* Message Handler class that supports buffering SS requests. */
    class SuppServQueueHelperHandler extends Handler implements Runnable {

        private Vector<Message> requestBuffer = new Vector<Message>(); /* Message Queue Buffer */
        private boolean paused = false;  /* Flag indicating the pause state of handler*/

        public SuppServQueueHelperHandler(Looper looper) {
            super(looper);
        }

        /* Runnable implementation */
        public void run() {
            //setup if needed
        }

        @Override
        public void handleMessage(Message msg) {
            Rlog.d(LOG_TAG, "handleMessage(), msg.what = " + msg.what + " , paused = " + paused);
            switch (msg.what) {
                case EVENT_SS_SEND:
                    /* If the state is paused, add the new request to the queue to pend.
                     * Otherwise, process the SS request immediately.
                     */
                    synchronized (pausedSync) {
                        if (paused) {
                            Rlog.d(LOG_TAG, "A SS request ongoing, add it into the queue");
                            Message msgCopy = new Message();
                            msgCopy.copyFrom(msg);
                            requestBuffer.add(msgCopy);
                        } else {
                            /* process the request by passing SuppSrvRequest object and phoneId */
                            processRequest(msg.obj, msg.arg1);
                            paused = true;
                        }
                    }
                    break;
                case EVENT_SS_RESPONSE:
                    synchronized (pausedSync) {
                        processResponse(msg.obj);
                        paused = false;
                        if (requestBuffer.size() > 0) {
                            Message request = requestBuffer.elementAt(0);
                            requestBuffer.removeElementAt(0);
                            sendMessage(request);
                        }
                    }
                    break;
                default:
                    Rlog.d(LOG_TAG, "handleMessage(), msg.what must be SEND or RESPONSE");
            }
        }

        /* Before we send the SS request to RILJ, we need to wrap it for response callback.
         * After the RILJ returns the result, the lock "paused" should be released.
         */
        private void processRequest(Object obj, int phoneId) {
            MtkSuppSrvRequest ss = (MtkSuppSrvRequest) obj;
            ss.mParcel.setDataPosition(0);
            Message respCallback = mHandler.obtainMessage(EVENT_SS_RESPONSE, ss);
            Rlog.d(LOG_TAG, "processRequest(), ss.mRequestCode = " + ss.mRequestCode +
                    ", ss.mResultCallback = " + ss.mResultCallback +
                    ", phoneId = " + phoneId);

            switch (ss.mRequestCode) {
                case MtkSuppSrvRequest.SUPP_SRV_REQ_GET_CF: {
                    int cfReason     = ss.mParcel.readInt();
                    int serviceClass = ss.mParcel.readInt();
                    ((MtkGsmCdmaPhone) mPhones[phoneId]).getCallForwardingOptionInternal(
                            cfReason, serviceClass, respCallback);
                    break;
                }
                case MtkSuppSrvRequest.SUPP_SRV_REQ_SET_CF: {
                    int action       = ss.mParcel.readInt();
                    int cfReason     = ss.mParcel.readInt();
                    String number    = ss.mParcel.readString();
                    int timerSeconds = ss.mParcel.readInt();
                    int serviceClass = ss.mParcel.readInt();
                    ((MtkGsmCdmaPhone) mPhones[phoneId]).setCallForwardingOptionInternal(
                            action, cfReason, number, timerSeconds, serviceClass, respCallback);
                    break;
                }
                case MtkSuppSrvRequest.SUPP_SRV_REQ_GET_CW: {
                    ((MtkGsmCdmaPhone) mPhones[phoneId]).getCallWaitingInternal(respCallback);
                    break;
                }
                case MtkSuppSrvRequest.SUPP_SRV_REQ_SET_CW: {
                    boolean enable = ss.mParcel.readInt() == 1 ? true : false;
                    ((MtkGsmCdmaPhone) mPhones[phoneId]).setCallWaitingInternal(
                            enable, respCallback);
                    break;
                }
                case MtkSuppSrvRequest.SUPP_SRV_REQ_GET_CB: {
                    String facility  = ss.mParcel.readString();
                    String password  = ss.mParcel.readString();
                    int serviceClass = ss.mParcel.readInt();
                    ((MtkGsmCdmaPhone) mPhones[phoneId]).getCallBarringInternal(
                            facility, password, respCallback, serviceClass);
                    break;
                }
                case MtkSuppSrvRequest.SUPP_SRV_REQ_SET_CB: {
                    String facility  = ss.mParcel.readString();
                    boolean lockState   = ss.mParcel.readInt() == 1 ? true : false;
                    String password  = ss.mParcel.readString();
                    int serviceClass = ss.mParcel.readInt();
                    ((MtkGsmCdmaPhone) mPhones[phoneId]).setCallBarringInternal(
                            facility, lockState, password, respCallback, serviceClass);
                    break;
                }
                case MtkSuppSrvRequest.SUPP_SRV_REQ_GET_CLIR: {
                    ((MtkGsmCdmaPhone) mPhones[phoneId]).getOutgoingCallerIdDisplayInternal(
                            respCallback);
                    break;
                }
                case MtkSuppSrvRequest.SUPP_SRV_REQ_SET_CLIR: {
                    int clirMode = ss.mParcel.readInt();
                    ((MtkGsmCdmaPhone) mPhones[phoneId]).setOutgoingCallerIdDisplayInternal(
                            clirMode, respCallback);
                    break;
                }
                case MtkSuppSrvRequest.SUPP_SRV_REQ_CFU_QUERY: {
                    int reason       = ss.mParcel.readInt();
                    int withTimeSlot = ss.mParcel.readInt();

                    MtkSuppServHelper ssHelper =
                            MtkSuppServManager.getSuppServHelper(phoneId);
                    if (ssHelper != null) {
                        ssHelper.queryCallForwardingOption(
                                reason, withTimeSlot == 1 ? true : false, respCallback);
                    }
                    break;
                }
                default:
                    Rlog.d(LOG_TAG, "processRequest(), no match mRequestCode");
            }
        }

        /* Return back the async result to UI */
        private void processResponse(Object obj) {
            AsyncResult ar = (AsyncResult) obj;
            MtkSuppSrvRequest ss = (MtkSuppSrvRequest) ar.userObj;
            Message resp = ss.mResultCallback;

            Rlog.d(LOG_TAG, "processResponse, resp = " + resp +
                    " , ar.result = " + ar.result +
                    " , ar.exception = " + ar.exception);
            if (resp != null) {
                AsyncResult.forMessage(resp, ar.result, ar.exception);
                resp.sendToTarget();
            }

            /* Dispose MtkSuppSrvRequest */
            ss.setResultCallback(null);
            ss.mParcel.recycle();
        }
    }

    /* Add Request to the queue */
    private void addRequest(MtkSuppSrvRequest ss, int phoneId) {
        if (mHandler != null) {
            Message msg = mHandler.obtainMessage(EVENT_SS_SEND, phoneId, 0, ss);
            msg.sendToTarget();
        }
    }

    public void getCallForwardingOptionForServiceClass(int cfReason, int serviceClass,
            Message response, int phoneId) {
        MtkSuppSrvRequest ss = MtkSuppSrvRequest.obtain(MtkSuppSrvRequest.SUPP_SRV_REQ_GET_CF,
                response);
        ss.mParcel.writeInt(cfReason);
        ss.mParcel.writeInt(serviceClass);
        addRequest(ss, phoneId);
    }

    public void setCallForwardingOptionForServiceClass(int action, int cfReason, String number,
            int timeSeconds, int serviceClass, Message response, int phoneId) {
        MtkSuppSrvRequest ss = MtkSuppSrvRequest.obtain(MtkSuppSrvRequest.SUPP_SRV_REQ_SET_CF,
                response);
        ss.mParcel.writeInt(action);
        ss.mParcel.writeInt(cfReason);
        ss.mParcel.writeString(number);
        ss.mParcel.writeInt(timeSeconds);
        ss.mParcel.writeInt(serviceClass);
        addRequest(ss, phoneId);
    }

    public void getCallWaiting(Message response, int phoneId) {
        MtkSuppSrvRequest ss = MtkSuppSrvRequest.obtain(MtkSuppSrvRequest.SUPP_SRV_REQ_GET_CW,
                response);
        addRequest(ss, phoneId);
    }

    public void setCallWaiting(boolean enable, Message response, int phoneId) {
        MtkSuppSrvRequest ss = MtkSuppSrvRequest.obtain(MtkSuppSrvRequest.SUPP_SRV_REQ_SET_CW,
                response);
        ss.mParcel.writeInt(enable ? 1 : 0);
        addRequest(ss, phoneId);
    }

    public void getCallBarring(String facility, String password, int serviceClass,
            Message response, int phoneId) {
        MtkSuppSrvRequest ss = MtkSuppSrvRequest.obtain(MtkSuppSrvRequest.SUPP_SRV_REQ_GET_CB,
                response);
        ss.mParcel.writeString(facility);
        ss.mParcel.writeString(password);
        ss.mParcel.writeInt(serviceClass);
        addRequest(ss, phoneId);
    }

    public void setCallBarring(String facility, boolean lockState, String password,
            int serviceClass, Message response, int phoneId) {
        MtkSuppSrvRequest ss = MtkSuppSrvRequest.obtain(MtkSuppSrvRequest.SUPP_SRV_REQ_SET_CB,
                response);
        ss.mParcel.writeString(facility);
        ss.mParcel.writeInt(lockState ? 1 : 0);
        ss.mParcel.writeString(password);
        ss.mParcel.writeInt(serviceClass);
        addRequest(ss, phoneId);
    }

    public void getOutgoingCallerIdDisplay(Message response, int phoneId) {
        MtkSuppSrvRequest ss = MtkSuppSrvRequest.obtain(MtkSuppSrvRequest.SUPP_SRV_REQ_GET_CLIR,
                response);
        addRequest(ss, phoneId);
    }

    public void setOutgoingCallerIdDisplay(int clirMode, Message response, int phoneId) {
        MtkSuppSrvRequest ss = MtkSuppSrvRequest.obtain(MtkSuppSrvRequest.SUPP_SRV_REQ_SET_CLIR,
                response);
        ss.mParcel.writeInt(clirMode);
        addRequest(ss, phoneId);
    }

    public void getCallForwardingOption(
            int reason, int withTimeSlot, Message response, int phoneId) {
        MtkSuppSrvRequest ss = MtkSuppSrvRequest.obtain(MtkSuppSrvRequest.SUPP_SRV_REQ_CFU_QUERY,
                response);
        ss.mParcel.writeInt(reason);
        ss.mParcel.writeInt(withTimeSlot);
        addRequest(ss, phoneId);
    }
}
