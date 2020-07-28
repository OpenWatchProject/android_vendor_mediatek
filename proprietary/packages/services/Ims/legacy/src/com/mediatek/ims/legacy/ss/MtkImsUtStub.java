/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.ims.legacy.ss;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;

import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.RemoteException;
import android.os.SystemProperties;

import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.telephony.ims.ImsCallForwardInfo;
import android.telephony.ims.ImsReasonInfo;
import android.util.Log;

import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;

import com.android.ims.internal.IImsUt;
import com.android.ims.ImsUtInterface;

import com.android.internal.telephony.CallForwardInfo;
import com.android.internal.telephony.CommandsInterface;

import com.mediatek.ims.MtkImsCallForwardInfo;
import com.mediatek.ims.MtkImsReasonInfo;
import com.mediatek.ims.internal.IMtkImsUt;
import com.mediatek.ims.internal.IMtkImsUtListener;
import com.mediatek.ims.internal.ImsXuiManager;
import com.mediatek.ims.ImsService;
import com.mediatek.internal.telephony.MtkCallForwardInfo;
import com.mediatek.internal.telephony.MtkPhoneConstants;
import com.mediatek.simservs.xcap.XcapException;
import com.mediatek.ims.feature.MtkImsUtImplBase;
import com.mediatek.ims.feature.MtkImsUtListener;
import com.mediatek.ims.SuppSrvConfig;

import java.net.UnknownHostException;

/**
 * ImsUT class for handle the IMS UT interface.
 *
 * The implementation is based on IR.92
 *
 *  @hide
 */
public class MtkImsUtStub extends MtkImsUtImplBase {
    private static final String TAG = "MtkImsUtService";
    private static final boolean DBG = true;

    private static HashMap<Integer, MtkImsUtStub> sMtkImsUtStubs =
            new HashMap<Integer, MtkImsUtStub>();

    private static final Object mLock = new Object();
    private Context mContext;
    private MtkImsUtListener mListener = null;
    private MMTelSSTransport mMMTelSSTSL;
    private ResultHandler mHandler;
    private ImsUtStub mImsUtStub = null;
    private ImsService mImsService = null;
    private int mPhoneId = 0;

    static final int IMS_UT_EVENT_GET_CF_TIME_SLOT = 1200;
    static final int IMS_UT_EVENT_SET_CF_TIME_SLOT = 1201;
    static final int IMS_UT_EVENT_SET_CB_WITH_PWD  = 1202;
    static final int IMS_UT_EVENT_SETUP_XCAP_USER_AGENT_STRING = 1203;
    static final int IMS_UT_EVENT_GET_CF_WITH_CLASS  = 1204;

    /**
     *
     * Construction function for ImsConfigStub.
     *
     * @param context the application context
     *
     */
    public MtkImsUtStub(Context context, int phoneId, ImsService imsService) {
        mContext = context;
        mImsUtStub = ImsUtStub.getInstance(context, phoneId, imsService);;
        mMMTelSSTSL = MMTelSSTransport.getInstance();
        mMMTelSSTSL.registerUtService(mContext);

        HandlerThread thread = new HandlerThread("MtkImsUtStubResult");
        thread.start();
        Looper looper = thread.getLooper();
        mHandler = new ResultHandler(looper);

        mImsService = imsService;
        mPhoneId = phoneId;
    }

    public static MtkImsUtStub getInstance(Context context, int phoneId, ImsService service) {
        synchronized (sMtkImsUtStubs) {
            if (sMtkImsUtStubs.containsKey(phoneId)) {
                MtkImsUtStub m = sMtkImsUtStubs.get(phoneId);
                return m;
            } else {
                sMtkImsUtStubs.put(phoneId, new MtkImsUtStub(context, phoneId, service));
                return sMtkImsUtStubs.get(phoneId);
            }
        }
    }

    public static MtkImsUtStub getInstance(int phoneId) {
        synchronized (sMtkImsUtStubs) {
            if (sMtkImsUtStubs.containsKey(phoneId)) {
                MtkImsUtStub m = sMtkImsUtStubs.get(phoneId);
                return m;
            }

            return null;
        }
    }

    /**
     * Sets the listener.
     */
    public void setListener(MtkImsUtListener listener) {
        mListener = listener;
    }

    /**
     * Get IMPU info from netowrk
     */
    public String getUtIMPUFromNetwork() {
        Log.d(TAG, "getUtIMPUFromNetwork(): phoneId = " + mPhoneId);
        return ImsXuiManager.getInstance().getXui(mPhoneId);
    }

    public void setupXcapUserAgentString(String userAgent) {
        Log.w(TAG, "Not support this API setupXcapUserAgentString() in current platform");
    }

    /**
     * Retrieves the configuration of the call forward in a time slot.
     */
    public int queryCallForwardInTimeSlot(int condition) {
        int requestId;

        synchronized (mLock) {
            requestId = mImsUtStub.getAndIncreaseRequestId();
        }

        if (DBG) {
            Log.d(TAG, "queryCallForwardInTimeSlot(): requestId = " + requestId);
        }

        Message msg = mHandler.obtainMessage(IMS_UT_EVENT_GET_CF_TIME_SLOT,
                requestId, 0, null);

        SuppSrvConfig ssConfig = SuppSrvConfig.getInstance(mContext);
        ssConfig.update(mPhoneId);

        mMMTelSSTSL.queryCallForwardInTimeSlotStatus(
                mImsUtStub.getCFReasonFromCondition(condition),
                CommandsInterface.SERVICE_CLASS_VOICE,
                msg, mPhoneId);

        return requestId;
    }

    /**
     * Updates the configuration of the call forward in a time slot.
     */
    public int updateCallForwardInTimeSlot(int action, int condition,
            String number, int timeSeconds, long[] timeSlot) {
        int requestId;

        synchronized (mLock) {
            requestId = mImsUtStub.getAndIncreaseRequestId();
        }

        if (DBG) {
            Log.d(TAG, "updateCallForwardInTimeSlot(): requestId = " + requestId);
        }

        Message msg = mHandler.obtainMessage(IMS_UT_EVENT_SET_CF_TIME_SLOT, requestId, 0, null);

        SuppSrvConfig ssConfig = SuppSrvConfig.getInstance(mContext);
        ssConfig.update(mPhoneId);

        mMMTelSSTSL.setCallForwardInTimeSlot(mImsUtStub.getCFActionFromAction(action),
                mImsUtStub.getCFReasonFromCondition(condition),
                CommandsInterface.SERVICE_CLASS_VOICE,
                number, timeSeconds, timeSlot, msg, mPhoneId);

        return requestId;
    }

    /**
     * Updates the configuration of the call barring for specified service class
     * with password added
     */
    public int updateCallBarringForServiceClass(String password, int cbType, int enable,
            String[] barrList, int serviceClass) {
        int requestId;
        String facility;

        synchronized (mLock) {
            requestId = mImsUtStub.getAndIncreaseRequestId();
        }
        if (DBG) {
            Log.d(TAG, "updateCallBarringForServiceClass(): requestId = " + requestId);
        }

        boolean bEnable = enable == 1 ? true : false;

        facility = getFacilityFromCBType(cbType);
        Message msg = mHandler.obtainMessage(IMS_UT_EVENT_SET_CB_WITH_PWD, requestId, 0, null);

        SuppSrvConfig ssConfig = SuppSrvConfig.getInstance(mContext);
        ssConfig.update(mPhoneId);

        mMMTelSSTSL.setFacilityLock(facility, bEnable, null, serviceClass, msg, mPhoneId);

        return requestId;
    }

    /**
     * Retrieves the configuration of the call forward.
     * @param condition Call Forward condition
     * @param number Forwarded to number
     * @param serviceClass Call Forward Service Class
     * @return the request ID
     */
    public int queryCFForServiceClass(int condition, String number, int serviceClass) {
        int requestId;

        synchronized (mLock) {
            requestId = mImsUtStub.getAndIncreaseRequestId();
        }
        if (DBG) {
            Log.d(TAG, "queryCFForServiceClass(): requestId = " + requestId);
        }

        SuppSrvConfig ssConfig = SuppSrvConfig.getInstance(mContext);
        ssConfig.update(mPhoneId);

        Message msg = mHandler.obtainMessage(IMS_UT_EVENT_GET_CF_WITH_CLASS, requestId, 0, null);
        mMMTelSSTSL.queryCallForwardStatus(mImsUtStub.getCFReasonFromCondition(condition), serviceClass,
                number, msg, mPhoneId);

        return requestId;
    }

    public boolean isSupportCFT() {
        return false;
    }

    public String getXcapConflictErrorMessage() {
        return "";
    }

    private class ResultHandler extends Handler {
        public ResultHandler(Looper looper) {
             super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            if (DBG) {
                Log.d(TAG, "handleMessage(): event = " + msg.what + ", requestId = " + msg.arg1
                         + ", mListener=" + mListener);
            }
            SuppSrvConfig ssConfig = SuppSrvConfig.getInstance(mContext);

            switch (msg.what) {
                case IMS_UT_EVENT_SET_CF_TIME_SLOT: // For OP01 UT
                    if (null != mListener) {
                        AsyncResult ar = (AsyncResult) msg.obj;

                        if (null == ar.exception) {
                            if (DBG) {
                                Log.d(TAG, "utConfigurationUpdated(): "
                                        + "event = " + msg.what);
                            }
                            mImsUtStub.notifyUtConfigurationUpdated(msg);
                        } else {
                            if (ar.exception instanceof XcapException) {
                                XcapException xcapException = (XcapException) ar.exception;
                                mImsUtStub.notifyUtConfigurationUpdateFailed(msg,
                                        mImsUtStub.xcapExceptionToImsReasonInfo(xcapException,mPhoneId));
                            } else if (ar.exception instanceof UnknownHostException) {
                                if (DBG) {
                                    Log.d(TAG, "UnknownHostException. event = " + msg.what);
                                }
                                mImsUtStub.notifyUtConfigurationUpdateFailed(msg,
                                        new ImsReasonInfo(MtkImsReasonInfo.CODE_UT_UNKNOWN_HOST, 0));
                            } else {
                                mImsUtStub.notifyUtConfigurationUpdateFailed(msg,
                                        new ImsReasonInfo(ImsReasonInfo.CODE_UT_NETWORK_ERROR, 0));
                            }
                        }
                    }
                    break;
                 case IMS_UT_EVENT_GET_CF_TIME_SLOT:
                     if (null != mListener) {
                         AsyncResult ar = (AsyncResult) msg.obj;

                         if (null == ar.exception) {
                             MtkCallForwardInfo[] cfInfo = (MtkCallForwardInfo[]) ar.result;
                             MtkImsCallForwardInfo[] imsCfInfo = null;

                             if (cfInfo != null) {
                                 imsCfInfo = new MtkImsCallForwardInfo[cfInfo.length];
                                 for (int i = 0; i < cfInfo.length; i++) {
                                     MtkImsCallForwardInfo info = new MtkImsCallForwardInfo
                                        ();
                                     info.mCondition =
                                             mImsUtStub.getConditionFromCFReason(cfInfo[i].reason);
                                     info.mStatus = cfInfo[i].status;
                                     info.mServiceClass = cfInfo[i].serviceClass;
                                     info.mToA = cfInfo[i].toa;
                                     info.mNumber = cfInfo[i].number;
                                     info.mTimeSeconds = cfInfo[i].timeSeconds;
                                     info.mTimeSlot = cfInfo[i].timeSlot;
                                     imsCfInfo[i] = info;
                                 }
                             }

                                 mListener.onUtConfigurationCallForwardInTimeSlotQueried(
                                        msg.arg1, imsCfInfo);
                         } else {
                             if (ar.exception instanceof XcapException) {
                                 XcapException xcapException = (XcapException) ar.exception;
                                 mImsUtStub.notifyUtConfigurationQueryFailed(msg,
                                         mImsUtStub.xcapExceptionToImsReasonInfo(xcapException,mPhoneId));
                             } else if (ar.exception instanceof UnknownHostException) {
                                 if (DBG) {
                                     Log.d(TAG, "IMS_UT_EVENT_GET_CF_TIME_SLOT: "
                                             + "UnknownHostException.");
                                 }
                                 mImsUtStub.notifyUtConfigurationQueryFailed(msg,
                                         new ImsReasonInfo(MtkImsReasonInfo.CODE_UT_UNKNOWN_HOST, 0));
                             } else {
                                 mImsUtStub.notifyUtConfigurationQueryFailed(msg,
                                         new ImsReasonInfo(ImsReasonInfo.CODE_UT_NETWORK_ERROR, 0));
                             }
                         }
                     }
                     break;
                case IMS_UT_EVENT_SET_CB_WITH_PWD:
                    if (null != mListener) {
                        AsyncResult ar = (AsyncResult) msg.obj;

                        if (null == ar.exception) {
                            if (DBG) {
                                Log.d(TAG, "utConfigurationUpdated(): "
                                        + "event = " + msg.what);
                            }
                            mImsUtStub.notifyUtConfigurationUpdated(msg);
                        } else {
                            if (ar.exception instanceof XcapException) {
                                XcapException xcapException = (XcapException) ar.exception;
                                mImsUtStub.notifyUtConfigurationUpdateFailed(msg,
                                        mImsUtStub.xcapExceptionToImsReasonInfo(xcapException,mPhoneId));
                            } else if (ar.exception instanceof UnknownHostException) {
                                if (DBG) {
                                    Log.d(TAG, "UnknownHostException. event = " + msg.what);
                                }
                                mImsUtStub.notifyUtConfigurationUpdateFailed(msg,
                                        new ImsReasonInfo(MtkImsReasonInfo.CODE_UT_UNKNOWN_HOST, 0));
                            } else {
                                mImsUtStub.notifyUtConfigurationUpdateFailed(msg,
                                        new ImsReasonInfo(ImsReasonInfo.CODE_UT_NETWORK_ERROR, 0));
                            }
                        }
                    }
                    break;
                case IMS_UT_EVENT_GET_CF_WITH_CLASS:
                    if (null != mListener) {
                        AsyncResult ar = (AsyncResult) msg.obj;

                        if (null == ar.exception) {
                            CallForwardInfo[] cfInfo = (CallForwardInfo[]) ar.result;
                            ImsCallForwardInfo[] imsCfInfo = null;

                            if (cfInfo != null) {
                                imsCfInfo = new ImsCallForwardInfo[cfInfo.length];
                                for (int i = 0; i < cfInfo.length; i++) {
                                    if (DBG) {
                                        Log.d(TAG, "IMS_UT_EVENT_GET_CF_WITH_CLASS: cfInfo[" + i + "] = "
                                                + cfInfo[i]);
                                    }
                                    imsCfInfo[i] = mImsUtStub.getImsCallForwardInfo(cfInfo[i]);
                                }
                            }

                            mImsUtStub.notifyUtConfigurationCallForwardQueried(msg, imsCfInfo);
                        } else {
                            if (ar.exception instanceof XcapException) {
                                XcapException xcapException = (XcapException) ar.exception;
                                mImsUtStub.notifyUtConfigurationQueryFailed(msg,
                                    mImsUtStub.xcapExceptionToImsReasonInfo(xcapException,mPhoneId));
                            } else if (ar.exception instanceof UnknownHostException) {
                                if (DBG) {
                                    Log.d(TAG, "IMS_UT_EVENT_GET_CF_WITH_CLASS: UnknownHostException.");
                                }
                                mImsUtStub.notifyUtConfigurationQueryFailed(msg,
                                        new ImsReasonInfo(MtkImsReasonInfo.CODE_UT_UNKNOWN_HOST,
                                        0));
                            } else {
                                mImsUtStub.notifyUtConfigurationQueryFailed(msg,
                                        new ImsReasonInfo(ImsReasonInfo.CODE_UT_NETWORK_ERROR,
                                                0));
                            }
                        }
                    }
                    break;
                default:
                    Log.d(TAG, "Unknown Event: " + msg.what);
                    break;
            }
        }
    };

    private String getFacilityFromCBType(int cbType) {
        switch (cbType) {
            case ImsUtInterface.CB_BAIC:
                return CommandsInterface.CB_FACILITY_BAIC;
            case ImsUtInterface.CB_BAOC:
                return CommandsInterface.CB_FACILITY_BAOC;
            case ImsUtInterface.CB_BOIC:
                return CommandsInterface.CB_FACILITY_BAOIC;
            case ImsUtInterface.CB_BOIC_EXHC:
                return CommandsInterface.CB_FACILITY_BAOICxH;
            case ImsUtInterface.CB_BIC_WR:
                return CommandsInterface.CB_FACILITY_BAICr;
            // TODO: Barring of Anonymous Communication Rejection (ACR)
            case ImsUtInterface.CB_BIC_ACR:
                return "ACR";
            case ImsUtInterface.CB_BA_ALL:
                return CommandsInterface.CB_FACILITY_BA_ALL;
            case ImsUtInterface.CB_BA_MO:
                return CommandsInterface.CB_FACILITY_BA_MO;
            case ImsUtInterface.CB_BA_MT:
                return CommandsInterface.CB_FACILITY_BA_MT;
            // TODO: Barring of Specific Incoming calls
            case ImsUtInterface.CB_BS_MT:
                return "BS_MT";
            default:
                return null;
        }
    }
}
