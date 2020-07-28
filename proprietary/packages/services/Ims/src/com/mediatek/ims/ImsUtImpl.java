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

package com.mediatek.ims;

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
import android.telephony.ims.stub.ImsUtImplBase;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.ImsCallForwardInfo;
import android.telephony.ims.ImsSsInfo;
import android.telephony.ims.ImsUtListener;

import android.util.Log;

import java.util.Base64;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;

import com.android.ims.ImsUtInterface;

import com.android.internal.telephony.CallForwardInfo;
import com.android.internal.telephony.CommandException;
import com.android.internal.telephony.CommandsInterface;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.RILConstants;
import com.android.internal.telephony.TelephonyIntents;

import static com.android.internal.telephony.imsphone.ImsPhoneMmiCode.UT_BUNDLE_KEY_CLIR;
import static com.android.internal.telephony.imsphone.ImsPhoneMmiCode.UT_BUNDLE_KEY_SSINFO;



import com.mediatek.ims.ril.ImsRILConstants;

import com.mediatek.ims.OperatorUtils;
import com.mediatek.ims.OperatorUtils.OPID;
import com.mediatek.ims.ril.ImsCommandsInterface;
import com.mediatek.ims.plugin.ExtensionFactory;
import com.mediatek.ims.plugin.ImsSSOemPlugin;
import com.mediatek.ims.plugin.ImsSSExtPlugin;
import com.mediatek.internal.telephony.MtkCallForwardInfo;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.TimeZone;

/**
 * ImsUT class for handle the IMS UT interface.
 *
 * The implementation is based on IR.92
 *
 *  @hide
 */
public class ImsUtImpl extends ImsUtImplBase {
    private static final String TAG = "ImsUtImpl";
    private static final boolean DBG = true;
    static private final boolean SDBG = SystemProperties.get("ro.build.type").equals("user")
            ? false : true;

    private ImsSSOemPlugin mOemPluginBase;
    private ImsSSExtPlugin mExtPluginBase;

    private static HashMap<Integer, ImsUtImpl> sImsUtImpls =
            new HashMap<Integer, ImsUtImpl>();

    private Context mContext;

    private static final Object mLock = new Object();
    private static int sRequestId = 0;
    private ImsUtListener mListener = null;
    private ResultHandler mHandler;

    private ImsCommandsInterface mImsRILAdapter;
    private ImsService mImsService = null;
    private int mPhoneId = 0;

    static final int IMS_UT_EVENT_GET_CB = 1000;
    static final int IMS_UT_EVENT_GET_CF = 1001;
    static final int IMS_UT_EVENT_GET_CW = 1002;
    static final int IMS_UT_EVENT_GET_CLIR = 1003;
    static final int IMS_UT_EVENT_GET_CLIP = 1004;
    static final int IMS_UT_EVENT_GET_COLR = 1005;
    static final int IMS_UT_EVENT_GET_COLP = 1006;
    static final int IMS_UT_EVENT_SET_CB = 1007;
    static final int IMS_UT_EVENT_SET_CF = 1008;
    static final int IMS_UT_EVENT_SET_CW = 1009;
    static final int IMS_UT_EVENT_SET_CLIR = 1010;
    static final int IMS_UT_EVENT_SET_CLIP = 1011;
    static final int IMS_UT_EVENT_SET_COLR = 1012;
    static final int IMS_UT_EVENT_SET_COLP = 1013;

    /* M: SS for 93
     * Define the mapping between CME error and
     * OEM ERROR CODE in CommandException
     *
     * CME_409_CONFLICT                 CommandException.Error.OEM_ERROR_25
     * CME_403_FORBIDDEN                CommandException.Error.OEM_ERROR_2
     * CME_NETWORK_TIMEOUT              CommandException.Error.OEM_ERROR_3
     * CME_404_NOT_FOUND                CommandException.Error.OEM_ERROR_4
     * CME_412_PRECONDITION_FAILED      CommandException.Error.OEM_ERROR_6
     */
    private static final int DEFAULT_INVALID_PHONE_ID = -1;

    private static final String SS_SERVICE_CLASS_PROP = "vendor.gsm.radio.ss.sc";

    public static ImsUtImpl getInstance(Context context, int phoneId, ImsService service) {
        synchronized (sImsUtImpls) {
            if (sImsUtImpls.containsKey(phoneId)) {
                ImsUtImpl m = sImsUtImpls.get(phoneId);
                return m;
            } else {
                sImsUtImpls.put(phoneId, new ImsUtImpl(context, phoneId, service));
                return sImsUtImpls.get(phoneId);
            }
        }
    }

    /**
    *
    * Construction function for ImsConfigStub.
    *
    * @param context the application context
    *
    */
    private ImsUtImpl(Context context, int phoneId, ImsService imsService) {
        mContext = context;

        HandlerThread thread = new HandlerThread("ImsUtImplResult");
        thread.start();
        Looper looper = thread.getLooper();
        mHandler = new ResultHandler(looper);

        mImsService = imsService;
        mImsRILAdapter = mImsService.getImsRILAdapter(phoneId);
        mPhoneId = phoneId;

        mOemPluginBase = ExtensionFactory.makeOemPluginFactory(mContext).makeImsSSOemPlugin(mContext);
        mExtPluginBase = ExtensionFactory.makeExtensionPluginFactory(mContext).makeImsSSExtPlugin(mContext);
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

            switch (msg.what) {
                case IMS_UT_EVENT_GET_CB:
                    if (null != mListener) {
                        AsyncResult ar = (AsyncResult) msg.obj;

                        if (null == ar.exception) {

                            int[] result = (int []) ar.result;
                            ImsSsInfo[] info = new ImsSsInfo[1];
                            info[0] = new ImsSsInfo();
                            info[0].mStatus = result[0];
                            // TODO: add ServiceClass information
                            if (DBG) {
                                Log.d(TAG, "IMS_UT_EVENT_GET_CB: status = " + result[0]);
                            }

                            mListener.onUtConfigurationCallBarringQueried(msg.arg1, info);
                        } else {

                            ImsReasonInfo reason;
                            if (ar.exception instanceof CommandException) {
                                reason = mOemPluginBase.commandExceptionToReason(
                                        (CommandException)(ar.exception), mPhoneId);
                            } else {
                                reason = new ImsReasonInfo(ImsReasonInfo.CODE_UT_NETWORK_ERROR, 0);
                            }

                            mListener.onUtConfigurationQueryFailed(msg.arg1, reason);
                        }
                    }
                    break;
                case IMS_UT_EVENT_GET_CF:
                    if (null != mListener) {
                        AsyncResult ar = (AsyncResult) msg.obj;
                        if (null == ar.exception) {

                            CallForwardInfo[] cfInfo = (CallForwardInfo[]) ar.result;
                            ImsCallForwardInfo[] imsCfInfo = mExtPluginBase.getImsCallForwardInfo(cfInfo);

                            mListener.onUtConfigurationCallForwardQueried(msg.arg1, imsCfInfo);
                        } else {

                            ImsReasonInfo reason;
                            if (ar.exception instanceof CommandException) {
                                reason = mOemPluginBase.commandExceptionToReason(
                                        (CommandException)(ar.exception), mPhoneId);
                            } else {
                                reason = new ImsReasonInfo(ImsReasonInfo.CODE_UT_NETWORK_ERROR, 0);
                            }

                            mListener.onUtConfigurationQueryFailed(msg.arg1, reason);
                        }
                    }
                    break;
                case IMS_UT_EVENT_GET_CW:
                    if (null != mListener) {
                        AsyncResult ar = (AsyncResult) msg.obj;
                        if (null == ar.exception) {

                            int[] result = (int []) ar.result;
                            ImsSsInfo[] info = new ImsSsInfo[1];
                            info[0] = new ImsSsInfo();
                            info[0].mStatus = result[0];
                            // TODO: add ServiceClass information
                            if (DBG) {
                                Log.d(TAG, "IMS_UT_EVENT_GET_CW: status = " + result[0]);
                            }

                            mListener.onUtConfigurationCallWaitingQueried(msg.arg1, info);
                        } else {

                            ImsReasonInfo reason;
                            if (ar.exception instanceof CommandException) {
                                reason = mOemPluginBase.commandExceptionToReason(
                                        (CommandException)(ar.exception), mPhoneId);
                            } else {
                                reason = new ImsReasonInfo(ImsReasonInfo.CODE_UT_NETWORK_ERROR, 0);
                            }

                            mListener.onUtConfigurationQueryFailed(msg.arg1, reason);
                        }
                    }
                    break;
                case IMS_UT_EVENT_GET_CLIR:
                    if (null != mListener) {
                        AsyncResult ar = (AsyncResult) msg.obj;
                        if (null == ar.exception) {

                            int[] result = (int []) ar.result;
                            Bundle info = new Bundle();
                            info.putIntArray(UT_BUNDLE_KEY_CLIR, result);

                            mListener.onUtConfigurationQueried(msg.arg1, info);
                        } else {

                            ImsReasonInfo reason;
                            if (ar.exception instanceof CommandException) {
                                reason = mOemPluginBase.commandExceptionToReason(
                                        (CommandException)(ar.exception), mPhoneId);
                            } else {
                                reason = new ImsReasonInfo(ImsReasonInfo.CODE_UT_NETWORK_ERROR, 0);
                            }

                            mListener.onUtConfigurationQueryFailed(msg.arg1, reason);
                        }
                    }
                    break;
                case IMS_UT_EVENT_GET_CLIP:
                case IMS_UT_EVENT_GET_COLR: // fall through
                case IMS_UT_EVENT_GET_COLP: // fall through
                    if (null != mListener) {
                        AsyncResult ar = (AsyncResult) msg.obj;
                        if (null == ar.exception) {

                            int[] result = (int []) ar.result;
                            ImsSsInfo ssInfo = new ImsSsInfo();
                            ssInfo.mStatus = result[0];
                            Bundle info = new Bundle();
                            info.putParcelable(UT_BUNDLE_KEY_SSINFO, ssInfo);

                            mListener.onUtConfigurationQueried(msg.arg1, info);
                        } else {

                            ImsReasonInfo reason;
                            if (ar.exception instanceof CommandException) {
                                reason = mOemPluginBase.commandExceptionToReason(
                                        (CommandException)(ar.exception), mPhoneId);
                            } else {
                                reason = new ImsReasonInfo(ImsReasonInfo.CODE_UT_NETWORK_ERROR, 0);
                            }

                            mListener.onUtConfigurationQueryFailed(msg.arg1, reason);
                        }
                    }
                    break;
                case IMS_UT_EVENT_SET_CB:
                case IMS_UT_EVENT_SET_CF: // fall through
                    // Handle the special case if update CF return cfinto object.
                    // Currently, only DTAG (OP05) and CFU would go though here.
                    // Need carefully handle this part because SET CB would here.
                    if (null != mListener) {
                        AsyncResult ar = (AsyncResult) msg.obj;
                        if (null == ar.exception && ar.result != null) {

                            if (ar.result instanceof CallForwardInfo[]) {
                                CallForwardInfo[] cfInfo = (CallForwardInfo[]) ar.result;
                                ImsCallForwardInfo[] imsCfInfo = null;
                                if (cfInfo != null && cfInfo.length != 0) {
                                    imsCfInfo = new ImsCallForwardInfo[cfInfo.length];
                                    for (int i = 0; i < cfInfo.length; i++) {
                                        imsCfInfo[i] = getImsCallForwardInfo(cfInfo[i]);
                                        if (DBG) {
                                            Log.d(TAG, "IMS_UT_EVENT_SET_CF: cfInfo[" + i + "] = "
                                                    + ", Condition: " + imsCfInfo[i].getCondition()
                                                    + ", Status: " + ((imsCfInfo[i].getStatus() == 0) ? "disabled" : "enabled")
                                                    + ", ToA: " + imsCfInfo[i].getToA()
                                                    + ", Service Class: " + imsCfInfo[i].getServiceClass()
                                                    + ", Number=" + encryptString(imsCfInfo[i].getNumber())
                                                    + ", Time (seconds): " + imsCfInfo[i].getTimeSeconds());
                                        }
                                    }
                                }
                                //Use this function to append the cfinfo.
                                mListener.onUtConfigurationCallForwardQueried(
                                        msg.arg1, imsCfInfo);
                                break;  //Break here and no need to do the below process.
                                        //If ar.result is null, then use the original flow.
                            }

                        }
                   }
                case IMS_UT_EVENT_SET_CW: // fall through
                case IMS_UT_EVENT_SET_CLIR: // fall through
                case IMS_UT_EVENT_SET_CLIP: // fall through
                case IMS_UT_EVENT_SET_COLR: // fall through
                case IMS_UT_EVENT_SET_COLP: // fall through
                    if (null != mListener) {
                        AsyncResult ar = (AsyncResult) msg.obj;
                        if (null == ar.exception) {
                            if (DBG) {
                                Log.d(TAG, "utConfigurationUpdated(): "
                                        + "event = " + msg.what);
                            }

                            mListener.onUtConfigurationUpdated(msg.arg1);
                        } else {

                            ImsReasonInfo reason;
                            if (ar.exception instanceof CommandException) {
                                reason = mOemPluginBase.commandExceptionToReason(
                                        (CommandException)(ar.exception), mPhoneId);
                            } else {
                                reason = new ImsReasonInfo(ImsReasonInfo.CODE_UT_NETWORK_ERROR, 0);
                            }

                            mListener.onUtConfigurationUpdateFailed(msg.arg1, reason);
                        }
                    }
                    break;
                /// TC3 requirement {@
                case IMS_UT_EVENT_GET_CF_TIME_SLOT:
                    if (null != mListener) {
                        AsyncResult ar = (AsyncResult) msg.obj;
                        if (null == ar.exception) {
                            MtkCallForwardInfo[] cfInfo = (MtkCallForwardInfo[]) ar.result;
                            ImsCallForwardInfo[] imsCfInfo = null;

                            if (cfInfo != null) {
                                imsCfInfo = new ImsCallForwardInfo[cfInfo.length];
                                for (int i = 0; i < cfInfo.length; i++) {
                                    // imsCfInfo[i] = getImsCallForwardInfo(cfInfo[i]);
                                    ImsCallForwardInfo info = new ImsCallForwardInfo();
                                    info.mCondition = getConditionFromCFReason(cfInfo[i].reason);
                                    info.mStatus = cfInfo[i].status;
                                    info.mServiceClass = cfInfo[i].serviceClass;
                                    info.mToA = cfInfo[i].toa;
                                    info.mNumber = cfInfo[i].number;
                                    info.mTimeSeconds = cfInfo[i].timeSeconds;
                                    imsCfInfo[i] = info;
                                }
                            }

                            mListener.onUtConfigurationCallForwardQueried(msg.arg1, imsCfInfo);
                        } else {
                            ImsReasonInfo reason;
                            if (ar.exception instanceof CommandException) {
                                reason = mOemPluginBase.commandExceptionToReason(
                                        (CommandException) (ar.exception), mPhoneId);
                            } else {
                                reason = new ImsReasonInfo(ImsReasonInfo.CODE_UT_NETWORK_ERROR, 0);
                            }

                            mListener.onUtConfigurationQueryFailed(msg.arg1, reason);
                        }
                    }
                    break;
                case IMS_UT_EVENT_SET_CF_TIME_SLOT:
                    if (null != mListener) {
                        AsyncResult ar = (AsyncResult) msg.obj;
                        if (null == ar.exception) {
                            if (DBG) {
                                Log.d(TAG, "utConfigurationUpdated(): " + "event = " + msg.what);
                            }
                            notifyUtConfigurationUpdated(msg);
                        } else {
                            ImsReasonInfo reason;
                            if (ar.exception instanceof CommandException) {
                                reason = mOemPluginBase.commandExceptionToReason(
                                        (CommandException) (ar.exception), mPhoneId);
                            } else {
                                reason = new ImsReasonInfo(ImsReasonInfo.CODE_UT_NETWORK_ERROR, 0);
                            }
                            notifyUtConfigurationUpdateFailed(msg, reason);
                        }
                    }
                    break;
                /// @}
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

    public int getCFActionFromAction(int cfAction) {
        switch(cfAction) {
            case ImsUtInterface.ACTION_DEACTIVATION:
                return CommandsInterface.CF_ACTION_DISABLE;
            case ImsUtInterface.ACTION_ACTIVATION:
                return CommandsInterface.CF_ACTION_ENABLE;
            case ImsUtInterface.ACTION_ERASURE:
                return CommandsInterface.CF_ACTION_ERASURE;
            case ImsUtInterface.ACTION_REGISTRATION:
                return CommandsInterface.CF_ACTION_REGISTRATION;
            default:
                break;
        }

        return CommandsInterface.CF_ACTION_DISABLE;
    }

    public int getCFReasonFromCondition(int condition) {
        switch(condition) {
            case ImsUtInterface.CDIV_CF_UNCONDITIONAL:
                return CommandsInterface.CF_REASON_UNCONDITIONAL;
            case ImsUtInterface.CDIV_CF_BUSY:
                return CommandsInterface.CF_REASON_BUSY;
            case ImsUtInterface.CDIV_CF_NO_REPLY:
                return CommandsInterface.CF_REASON_NO_REPLY;
            case ImsUtInterface.CDIV_CF_NOT_REACHABLE:
                return CommandsInterface.CF_REASON_NOT_REACHABLE;
            case ImsUtInterface.CDIV_CF_ALL:
                return CommandsInterface.CF_REASON_ALL;
            case ImsUtInterface.CDIV_CF_ALL_CONDITIONAL:
                return CommandsInterface.CF_REASON_ALL_CONDITIONAL;
            case ImsUtInterface.CDIV_CF_NOT_LOGGED_IN:
                return ImsRILConstants.CF_REASON_NOT_REGISTERED;
            default:
                break;
        }

        return CommandsInterface.CF_REASON_NOT_REACHABLE;
    }

    public static int getConditionFromCFReason(int reason) {
        switch(reason) {
            case CommandsInterface.CF_REASON_UNCONDITIONAL:
                return ImsUtInterface.CDIV_CF_UNCONDITIONAL;
            case CommandsInterface.CF_REASON_BUSY:
                return ImsUtInterface.CDIV_CF_BUSY;
            case CommandsInterface.CF_REASON_NO_REPLY:
                return ImsUtInterface.CDIV_CF_NO_REPLY;
            case CommandsInterface.CF_REASON_NOT_REACHABLE:
                return ImsUtInterface.CDIV_CF_NOT_REACHABLE;
            case CommandsInterface.CF_REASON_ALL:
                return ImsUtInterface.CDIV_CF_ALL;
            case CommandsInterface.CF_REASON_ALL_CONDITIONAL:
                return ImsUtInterface.CDIV_CF_ALL_CONDITIONAL;
            case ImsRILConstants.CF_REASON_NOT_REGISTERED:
                return ImsUtInterface.CDIV_CF_NOT_LOGGED_IN;
            default:
                break;
        }

        return ImsUtInterface.INVALID;
    }

    private ImsCallForwardInfo getImsCallForwardInfo(CallForwardInfo info) {
        ImsCallForwardInfo imsCfInfo = new ImsCallForwardInfo();
        imsCfInfo.mCondition = getConditionFromCFReason(info.reason);
        imsCfInfo.mStatus = info.status;
        imsCfInfo.mServiceClass = info.serviceClass;
        imsCfInfo.mToA = info.toa;
        imsCfInfo.mNumber = info.number;
        imsCfInfo.mTimeSeconds = info.timeSeconds;
        return imsCfInfo;
    }

    /**
     * Retrieves the configuration of the call barring.
     * @param cbType Call Barring Type
     * @return the request ID
     */
    public int queryCallBarring(int cbType) {
        int requestId;
        String facility;

        synchronized (mLock) {
            requestId = sRequestId;
            sRequestId++;
        }
        if (DBG) {
            Log.d(TAG, "queryCallBarring(): requestId = " + requestId);
        }

        facility = getFacilityFromCBType(cbType);
        int serviceClass = getServiceClass() != -1 ?
                getServiceClass() : CommandsInterface.SERVICE_CLASS_NONE;

        Message msg = mHandler.obtainMessage(IMS_UT_EVENT_GET_CB, requestId, 0, null);
        mImsRILAdapter.queryFacilityLock(facility, null, serviceClass, msg);

        resetServcieClass();
        return requestId;
    }

    /**
     * Retrieves the configuration of the call barring.
     * @param cbType Call Barring Type
     * @param serviceClass service class
     * @return the request ID
     */
    public int queryCallBarringForServiceClass(int cbType, int serviceClass) {
        int requestId;
        String facility;

        synchronized (mLock) {
            requestId = sRequestId;
            sRequestId++;
        }
        if (DBG) {
            Log.d(TAG, "queryCallBarringForServiceClass(): requestId = " + requestId);
        }

        facility = getFacilityFromCBType(cbType);

        Message msg = mHandler.obtainMessage(IMS_UT_EVENT_GET_CB, requestId, 0, null);
        mImsRILAdapter.queryFacilityLock(facility, null, serviceClass, msg);

        resetServcieClass();
        return requestId;
    }

    /**
     * Retrieves the configuration of the call forward.
     * @param condition Call Forward condition
     * @param number Forwarded to number
     * @return the request ID
     */
    public int queryCallForward(int condition, String number) {
        int requestId;

        synchronized (mLock) {
            requestId = sRequestId;
            sRequestId++;
        }
        if (DBG) {
            Log.d(TAG, "queryCallForward(): requestId = " + requestId);
        }

        int serviceClass = getServiceClass() != -1 ?
                getServiceClass() : CommandsInterface.SERVICE_CLASS_NONE;

        Message msg = mHandler.obtainMessage(IMS_UT_EVENT_GET_CF, requestId, 0, null);
        mImsRILAdapter.queryCallForwardStatus(getCFReasonFromCondition(condition),
                serviceClass, number, msg);

        resetServcieClass();
        return requestId;
    }

    /**
     * Retrieves the configuration of the call waiting.
     * @return the request ID
     */
    public int queryCallWaiting() {
        int requestId;

        synchronized (mLock) {
            requestId = sRequestId;
            sRequestId++;
        }
        if (DBG) {
            Log.d(TAG, "queryCallWaiting(): requestId = " + requestId);
        }

        Message msg = mHandler.obtainMessage(IMS_UT_EVENT_GET_CW, requestId, 0, null);
        mImsRILAdapter.queryCallWaiting(CommandsInterface.SERVICE_CLASS_VOICE, msg);

        return requestId;
    }

    /**
     * Retrieves the default CLIR setting.
     * @return the request ID
     */
    public int queryCLIR() {
        int requestId;

        synchronized (mLock) {
            requestId = sRequestId;
            sRequestId++;
        }
        if (DBG) {
            Log.d(TAG, "queryCLIR(): requestId = " + requestId);
        }

        Message msg = mHandler.obtainMessage(IMS_UT_EVENT_GET_CLIR, requestId, 0, null);
        mImsRILAdapter.getCLIR(msg);

        return requestId;
    }

    /**
     * Retrieves the CLIP call setting.
     * @return the request ID
     */
    public int queryCLIP() {
        int requestId;

        synchronized (mLock) {
            requestId = sRequestId;
            sRequestId++;
        }
        if (DBG) {
            Log.d(TAG, "queryCLIP(): requestId = " + requestId);
        }

        Message msg = mHandler.obtainMessage(IMS_UT_EVENT_GET_CLIP, requestId, 0, null);
        mImsRILAdapter.queryCLIP(msg);

        return requestId;
    }

    /**
     * Retrieves the COLR call setting.
     * @return the request ID
     */
    public int queryCOLR() {
        int requestId;

        synchronized (mLock) {
            requestId = sRequestId;
            sRequestId++;
        }
        if (DBG) {
            Log.d(TAG, "queryCOLR(): requestId = " + requestId);
        }

        Message msg = mHandler.obtainMessage(IMS_UT_EVENT_GET_COLR, requestId, 0, null);
        mImsRILAdapter.getCOLR(msg);

        return requestId;
    }

    /**
     * Retrieves the COLP call setting.
     * @return the request ID
     */
    public int queryCOLP() {
        int requestId;

        synchronized (mLock) {
            requestId = sRequestId;
            sRequestId++;
        }
        if (DBG) {
            Log.d(TAG, "queryCOLP(): requestId = " + requestId);
        }

        Message msg = mHandler.obtainMessage(IMS_UT_EVENT_GET_COLP, requestId, 0, null);
        mImsRILAdapter.getCOLP(msg);

        return requestId;
    }

    /**
     * Updates or retrieves the supplementary service configuration.
     * @param ssInfo supplementary service information
     * @return the request ID
     */
    public int transact(Bundle ssInfo) {
        int requestId;

        synchronized (mLock) {
            requestId = sRequestId;
            sRequestId++;
        }

        return requestId;
    }

    /**
     * Updates the configuration of the call barring.
     * @param cbType Call Barring Type
     * @param enable lock state
     * @param barrList barring list
     * @return the request ID
     */
    public int updateCallBarring(int cbType, int enable, String[] barrList) {
        int requestId;
        String facility;

        synchronized (mLock) {
            requestId = sRequestId;
            sRequestId++;
        }
        if (DBG) {
            Log.d(TAG, "updateCallBarring(): requestId = " + requestId);
        }

        boolean bEnable = enable == 1 ? true : false;

        facility = getFacilityFromCBType(cbType);

        int serviceClass = getServiceClass() != -1 ?
                getServiceClass() : CommandsInterface.SERVICE_CLASS_NONE;

        Message msg = mHandler.obtainMessage(IMS_UT_EVENT_SET_CB, requestId, 0, null);
        mImsRILAdapter.setFacilityLock(facility, bEnable, "", serviceClass, msg);

        resetServcieClass();

        return requestId;
    }

    /**
     * Updates the configuration of the call barring.
     * @param cbType Call Barring Type
     * @param enable lock state
     * @param barrList barring list
     * @param serviceClass service class
     * @return the request ID
     */
    public int updateCallBarringForServiceClass(int cbType, int enable, String[] barrList,
            int serviceClass) {
        int requestId;
        String facility;

        synchronized (mLock) {
            requestId = sRequestId;
            sRequestId++;
        }
        if (DBG) {
            Log.d(TAG, "updateCallBarringForServiceClass(): requestId = " + requestId);
        }

        boolean bEnable = enable == 1 ? true : false;
        facility = getFacilityFromCBType(cbType);

        Message msg = mHandler.obtainMessage(IMS_UT_EVENT_SET_CB, requestId, 0, null);
        mImsRILAdapter.setFacilityLock(facility, bEnable, "", serviceClass, msg);

        return requestId;
    }

    /**
     * Updates the configuration of the call forward.
     * @param action the call forwarding action
     * @param condition the call forwarding condition
     * @param number the call forwarded to number
     * @param timeSeconds seconds for no reply
     * @return the request ID
     */
    public int updateCallForward(int action, int condition, String number, int serviceClass,
                                     int timeSeconds) {
        int requestId;

        synchronized (mLock) {
            requestId = sRequestId;
            sRequestId++;
        }
        if (DBG) {
            Log.d(TAG, "updateCallForward(): requestId = " + requestId);
        }

        Message msg = mHandler.obtainMessage(IMS_UT_EVENT_SET_CF, requestId, 0, null);
        mImsRILAdapter.setCallForward(getCFActionFromAction(action),
                getCFReasonFromCondition(condition), serviceClass,
                number, timeSeconds, msg);

        return requestId;
    }

    /**
     * Updates the configuration of the call waiting.
     * @param enable activate flag
     * @return the request ID
     */
    public int updateCallWaiting(boolean enable, int serviceClass) {
        int requestId;

        synchronized (mLock) {
            requestId = sRequestId;
            sRequestId++;
        }
        if (DBG) {
            Log.d(TAG, "updateCallWaiting(): requestId = " + requestId);
        }

        Message msg = mHandler.obtainMessage(IMS_UT_EVENT_SET_CW, requestId, 0, null);
        mImsRILAdapter.setCallWaiting(enable, serviceClass, msg);

        return requestId;
    }

    /**
     * Updates the configuration of the CLIR supplementary service.
     * @param clirMode CLIR mode
     * @return the request ID
     */
    public int updateCLIR(int clirMode) {
        int requestId;

        synchronized (mLock) {
            requestId = sRequestId;
            sRequestId++;
        }
        if (DBG) {
            Log.d(TAG, "updateCLIR(): requestId = " + requestId);
        }

        Message msg = mHandler.obtainMessage(IMS_UT_EVENT_SET_CLIR, requestId, 0, null);
        mImsRILAdapter.setCLIR(clirMode, msg);

        return requestId;
    }

    /**
     * Updates the configuration of the CLIP supplementary service.
     * @param enable activate flag
     * @return the request ID
     */
    public int updateCLIP(boolean enable) {
        int requestId;

        synchronized (mLock) {
            requestId = sRequestId;
            sRequestId++;
        }
        if (DBG) {
            Log.d(TAG, "updateCLIP(): requestId = " + requestId);
        }

        Message msg = mHandler.obtainMessage(IMS_UT_EVENT_SET_CLIP, requestId, 0, null);
        mImsRILAdapter.setCLIP((enable)?1:0, msg);

        return requestId;
    }

    /**
     * Updates the configuration of the COLR supplementary service.
     * @param presentation presentation flag
     * @return the request ID
     */
    public int updateCOLR(int presentation) {
        int requestId;

        synchronized (mLock) {
            requestId = sRequestId;
            sRequestId++;
        }
        if (DBG) {
            Log.d(TAG, "updateCOLR(): requestId = " + requestId);
        }

        Message msg = mHandler.obtainMessage(IMS_UT_EVENT_SET_COLR, requestId, 0, null);
        mImsRILAdapter.setCOLR(presentation, msg);

        return requestId;
    }

    /**
     * Updates the configuration of the COLP supplementary service.
     * @param enable activate flag
     * @return the request ID
     */
    public int updateCOLP(boolean enable) {
        int requestId;

        synchronized (mLock) {
            requestId = sRequestId;
            sRequestId++;
        }
        if (DBG) {
            Log.d(TAG, "updateCOLP(): requestId = " + requestId);
        }

        Message msg = mHandler.obtainMessage(IMS_UT_EVENT_SET_COLP, requestId, 0, null);
        mImsRILAdapter.setCOLP((enable)?1:0, msg);

        return requestId;
    }

    /**
     * Sets the listener.
     * @param listener callback interface
     */
    public void setListener(ImsUtListener listener) {
        mListener = listener;
    }

    public static int getAndIncreaseRequestId() {
        int requestId = 0;
        synchronized (mLock) {
            requestId = sRequestId;
            sRequestId++;
        }
        return requestId;
    }

    private static int getServiceClass() {
        return Integer.parseInt(SystemProperties.get(SS_SERVICE_CLASS_PROP, "-1"));
    }

    private static void resetServcieClass() {
        SystemProperties.set(SS_SERVICE_CLASS_PROP, "-1");
    }

    public void notifyUtConfigurationUpdated(Message msg) {
        mListener.onUtConfigurationUpdated(msg.arg1);
    }

    public void notifyUtConfigurationUpdateFailed(Message msg, ImsReasonInfo error) {
        mListener.onUtConfigurationUpdateFailed(msg.arg1, error);
    }

    public void notifyUtConfigurationQueried(Message msg, Bundle ssInfo) {
        mListener.onUtConfigurationQueried(msg.arg1, ssInfo);
    }

    public void notifyUtConfigurationQueryFailed(Message msg, ImsReasonInfo error) {
        mListener.onUtConfigurationQueryFailed(msg.arg1, error);
    }

    /// TC3 requirement {@
    static final int IMS_UT_EVENT_GET_CF_TIME_SLOT = 1200;
    static final int IMS_UT_EVENT_SET_CF_TIME_SLOT = 1201;

    static final int TIME_VALUE_MIN = 0;
    static final int TIME_VALUE_HOUR_MAX = 23;
    static final int TIME_VALUE_MINUTE_MAX = 59;

    private long[] convertToTimeSlotArray(String timeSlotString) {
        long[] timeSlot = null;
        if (timeSlotString != null) {
            String[] timeArray = timeSlotString.split(",", 2);
            if (timeArray.length == 2) {
                timeSlot = new long[2];
                for (int i = 0; i < 2; i++) {
                    SimpleDateFormat dateFormat = new SimpleDateFormat("HH:mm");
                    dateFormat.setTimeZone(TimeZone.getTimeZone("GMT+8"));
                    try {
                        Date date = dateFormat.parse(timeArray[i]);
                        timeSlot[i] = date.getTime();
                    } catch (ParseException e) {
                        e.printStackTrace();
                        return null;
                    }
                }
            }
        }
        return timeSlot;
    }

    /**
     * Updates the configuration of the call forward uncondion in time slot.
     * @param startHour call forward start time(hour)
     * @param startMinute call forward start time
     * @param endHour call forward end time
     * @param endMinute call forward end time
     * @param action the call forwarding action
     * @param condition the call forwarding condition
     * @param number the call forwarded to number
     * @return the request ID
     */
    public int updateCallForwardUncondTimer(int startHour, int startMinute, int endHour,
            int endMinute, int action, int condition, String number) {
        int requestId;

        if (startHour == TIME_VALUE_MIN
                && startMinute == TIME_VALUE_MIN
                && endHour == TIME_VALUE_MIN
                && endMinute == TIME_VALUE_MIN) {
            Log.i(TAG, "updateCallForwardUncondTimer(): Time is all zero! use updateCallForward");
            return updateCallForward(action, condition, number,
                    CommandsInterface.SERVICE_CLASS_VOICE, 0);
        }

        synchronized (mLock) {
            requestId = sRequestId;
            sRequestId++;
        }
        if (DBG) {
            Log.d(TAG, "updateCallForwardUncondTimer(): requestId = " + requestId);
        }

        if (startHour < TIME_VALUE_MIN || startHour > TIME_VALUE_HOUR_MAX
                || startMinute < TIME_VALUE_MIN || startMinute > TIME_VALUE_MINUTE_MAX
                || endHour < TIME_VALUE_MIN || endHour > TIME_VALUE_HOUR_MAX
                || endMinute < TIME_VALUE_MIN || endMinute > TIME_VALUE_MINUTE_MAX) {
            Log.e(TAG, "updateCallForwardUncondTimer(): Time is wrong! ");
        }

        String timer = String.format("%02d:%02d,%02d:%02d",
                startHour, startMinute, endHour, endMinute);

        Message msg = mHandler.obtainMessage(IMS_UT_EVENT_SET_CF_TIME_SLOT, requestId, 0, null);
        mImsRILAdapter.setCallForwardInTimeSlot(getCFActionFromAction(action),
                getCFReasonFromCondition(condition),
                CommandsInterface.SERVICE_CLASS_VOICE,
                number, 0, convertToTimeSlotArray(timer), msg);

        return requestId;
    }
    /// @}

    public static String encryptString(String message) {
        Base64.Encoder encoder = Base64.getEncoder();
        byte[] textByte = null;
        try {
            textByte = message.getBytes("UTF-8");
        } catch (Exception e) {
            e.printStackTrace();
            textByte = null;
        }

        if (textByte == null) return "";

        String encryptedString = encoder.encodeToString(textByte);
        return encryptedString;
    }
}
