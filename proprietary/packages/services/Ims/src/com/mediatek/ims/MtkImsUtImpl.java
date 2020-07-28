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
import android.os.Messenger;
import android.os.Message;
import android.os.RemoteException;
import android.os.SystemProperties;

import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.ImsCallForwardInfo;
import android.util.Log;

import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;

import com.android.ims.internal.IImsUt;
import com.android.ims.ImsUtInterface;

import com.android.internal.telephony.CallForwardInfo;
import com.android.internal.telephony.CommandException;
import com.android.internal.telephony.CommandsInterface;

import com.mediatek.ims.internal.IMtkImsUt;
import com.mediatek.ims.internal.IMtkImsUtListener;
import com.mediatek.ims.internal.ImsXuiManager;
import com.mediatek.ims.plugin.ExtensionFactory;
import com.mediatek.ims.plugin.ImsSSOemPlugin;
import com.mediatek.ims.plugin.ImsSSExtPlugin;
import com.mediatek.ims.MtkImsCallForwardInfo;
import com.mediatek.ims.MtkImsReasonInfo;
import com.mediatek.ims.ril.ImsCommandsInterface;
import com.mediatek.internal.telephony.MtkCallForwardInfo;
import com.mediatek.ims.feature.MtkImsUtImplBase;
import com.mediatek.ims.feature.MtkImsUtListener;

import java.net.UnknownHostException;

/**
 * ImsUT class for handle the IMS UT interface.
 *
 * The implementation is based on IR.92
 *
 *  @hide
 */
public class MtkImsUtImpl extends MtkImsUtImplBase {
    private static final String TAG = "MtkImsUtImpl";
    private static final boolean DBG = true;

    private static HashMap<Integer, MtkImsUtImpl> sMtkImsUtImpls =
            new HashMap<Integer, MtkImsUtImpl>();

    private static final Object mLock = new Object();
    private Context mContext;
    private MtkImsUtListener mListener = null;

    private ResultHandler mHandler;
    private ImsUtImpl mImsUtImpl = null;
    private ImsCommandsInterface mImsRILAdapter;
    private ImsService mImsService = null;
    private int mPhoneId = 0;
    private ImsSSOemPlugin mOemPluginBase;
    private ImsSSExtPlugin mExtPluginBase;

    static final int IMS_UT_EVENT_GET_CF_TIME_SLOT = 1200;
    static final int IMS_UT_EVENT_SET_CF_TIME_SLOT = 1201;
    static final int IMS_UT_EVENT_SET_CB_WITH_PWD  = 1202;
    static final int IMS_UT_EVENT_SETUP_XCAP_USER_AGENT_STRING = 1203;
    static final int IMS_UT_EVENT_GET_CF_WITH_CLASS  = 1204;

    public static MtkImsUtImpl getInstance(Context context, int phoneId, ImsService service) {
        synchronized (sMtkImsUtImpls) {
            if (sMtkImsUtImpls.containsKey(phoneId)) {
                MtkImsUtImpl m = sMtkImsUtImpls.get(phoneId);
                return m;
            } else {
                sMtkImsUtImpls.put(phoneId, new MtkImsUtImpl(context, phoneId, service));
                return sMtkImsUtImpls.get(phoneId);
            }
        }
    }

    public static MtkImsUtImpl getInstance(int phoneId) {
        synchronized (sMtkImsUtImpls) {
            if (sMtkImsUtImpls.containsKey(phoneId)) {
                MtkImsUtImpl m = sMtkImsUtImpls.get(phoneId);
                return m;
            }

            return null;
        }
    }

    /**
     *
     * Construction function for ImsConfigStub.
     *
     * @param context the application context
     *
     */
    private MtkImsUtImpl(Context context, int phoneId, ImsService imsService) {
        mContext = context;
        mImsUtImpl = ImsUtImpl.getInstance(context, phoneId, imsService);

        HandlerThread thread = new HandlerThread("MtkImsUtImplResult");
        thread.start();
        Looper looper = thread.getLooper();
        mHandler = new ResultHandler(looper);

        mImsService = imsService;
        mImsRILAdapter = mImsService.getImsRILAdapter(phoneId);
        mPhoneId = phoneId;

        mOemPluginBase = ExtensionFactory.makeOemPluginFactory(mContext).makeImsSSOemPlugin(mContext);
        mExtPluginBase = ExtensionFactory.makeExtensionPluginFactory(mContext).makeImsSSExtPlugin(mContext);
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
        if (userAgent != null) {
            Log.d(TAG, "setupXcapUserAgentString(): userAgent = " + userAgent);
            Message msg = mHandler.obtainMessage(IMS_UT_EVENT_SETUP_XCAP_USER_AGENT_STRING);
            mImsRILAdapter.setupXcapUserAgentString(userAgent, msg);
        } else {
            Log.e(TAG, "setupXcapUserAgentString(): userAgent is null");
        }
    }

    /// For OP01 UT @{
    /**
     * Retrieves the configuration of the call forward in a time slot.
     */
    public int queryCallForwardInTimeSlot(int condition) {
        int requestId;

        synchronized (mLock) {
            requestId = mImsUtImpl.getAndIncreaseRequestId();
        }
        if (DBG) {
            Log.d(TAG, "queryCallForwardInTimeSlot(): requestId = " + requestId);
        }

        Message msg = mHandler.obtainMessage(IMS_UT_EVENT_GET_CF_TIME_SLOT,
                requestId, 0, null);
        mImsRILAdapter.queryCallForwardInTimeSlotStatus(
                mImsUtImpl.getCFReasonFromCondition(condition),
                CommandsInterface.SERVICE_CLASS_VOICE, msg);

        return requestId;
    }

    /**
     * Updates the configuration of the call forward in a time slot.
     */
    public int updateCallForwardInTimeSlot(int action, int condition,
            String number, int timeSeconds, long[] timeSlot) {
        int requestId;

        synchronized (mLock) {
            requestId = mImsUtImpl.getAndIncreaseRequestId();
        }
        if (DBG) {
            Log.d(TAG, "updateCallForwardInTimeSlot(): requestId = " + requestId);
        }

        Message msg = mHandler.obtainMessage(IMS_UT_EVENT_SET_CF_TIME_SLOT, requestId, 0, null);

        mImsRILAdapter.setCallForwardInTimeSlot(mImsUtImpl.getCFActionFromAction(action),
                mImsUtImpl.getCFReasonFromCondition(condition),
                CommandsInterface.SERVICE_CLASS_VOICE,
                number, timeSeconds, timeSlot, msg);

        return requestId;
    }

    public boolean isSupportCFT() {
        return mImsService.isSupportCFT(mPhoneId);
    }
    /// @}

    /**
     * Updates the configuration of the call barring for specified service class
     * with password added
     *
     * @param password Call Barring password (for CSFB usage)
     * @param cbType Call Barring Type
     * @param enable lock state
     * @param barrList barring list
     * @param serviceClass service class
     * @return the request ID
     */
    public int updateCallBarringForServiceClass(String password, int cbType, int enable,
            String[] barrList, int serviceClass) {
        int requestId;
        String facility;

        synchronized (mLock) {
            requestId = mImsUtImpl.getAndIncreaseRequestId();
        }
        if (DBG) {
            Log.d(TAG, "updateCallBarringForServiceClass(): requestId = " + requestId);
        }

        boolean bEnable = enable == 1 ? true : false;
        facility = getFacilityFromCBType(cbType);

        Message msg = mHandler.obtainMessage(IMS_UT_EVENT_SET_CB_WITH_PWD, requestId, 0, null);
        mImsRILAdapter.setFacilityLock(facility, bEnable, password,
                serviceClass, msg);

        return requestId;
    }

    /**
     *  Do the explicit Call Transfer between FG call and BG call.
     */
    public void explicitCallTransfer(Message result, Messenger target) {

        mImsService.explicitCallTransfer(mPhoneId, result, target);
    }

    public String getXcapConflictErrorMessage() {
        return mOemPluginBase.getXCAPErrorMessageFromSysProp(CommandException.Error.OEM_ERROR_25,
                mPhoneId);
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
            requestId = mImsUtImpl.getAndIncreaseRequestId();
        }
        if (DBG) {
            Log.d(TAG, "queryCFForServiceClass(): requestId = " + requestId);
        }

        Message msg = mHandler.obtainMessage(IMS_UT_EVENT_GET_CF_WITH_CLASS, requestId, 0, null);
        mImsRILAdapter.queryCallForwardStatus(mImsUtImpl.getCFReasonFromCondition(condition),
                serviceClass, number, msg);

        return requestId;
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
                            mImsUtImpl.notifyUtConfigurationUpdated(msg);
                        } else {

                            ImsReasonInfo reason;
                            if (ar.exception instanceof CommandException) {
                                reason = mOemPluginBase.commandExceptionToReason(
                                        (CommandException)(ar.exception), mPhoneId);
                            } else {
                                reason = new ImsReasonInfo(ImsReasonInfo.CODE_UT_NETWORK_ERROR, 0);
                            }
                            mImsUtImpl.notifyUtConfigurationUpdateFailed(msg, reason);
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
                                     MtkImsCallForwardInfo info = new MtkImsCallForwardInfo();
                                     info.mCondition =
                                             mImsUtImpl.getConditionFromCFReason(cfInfo[i].reason);
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

                            ImsReasonInfo reason;
                            if (ar.exception instanceof CommandException) {
                                reason = mOemPluginBase.commandExceptionToReason(
                                        (CommandException)(ar.exception), mPhoneId);
                            } else {
                                reason = new ImsReasonInfo(ImsReasonInfo.CODE_UT_NETWORK_ERROR, 0);
                            }

                            mImsUtImpl.notifyUtConfigurationQueryFailed(msg, reason);
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
                            mImsUtImpl.notifyUtConfigurationUpdated(msg);
                        } else {
                            ImsReasonInfo reason;
                            if (ar.exception instanceof CommandException) {
                                reason = mOemPluginBase.commandExceptionToReason(
                                        (CommandException)(ar.exception), mPhoneId);
                            } else {
                                reason = new ImsReasonInfo(ImsReasonInfo.CODE_UT_NETWORK_ERROR, 0);
                            }
                            mImsUtImpl.notifyUtConfigurationUpdateFailed(msg, reason);
                        }
                    }
                    break;
                case IMS_UT_EVENT_SETUP_XCAP_USER_AGENT_STRING:
                    if (null != mListener) {
                        AsyncResult ar = (AsyncResult) msg.obj;
                        if (null == ar.exception) {
                            Log.d(TAG, "Execute setupXcapUserAgentString succeed!" +
                                    "event = " + msg.what);
                        } else {
                            Log.e(TAG, "Execute setupXcapUserAgentString failed!" +
                                    "event = " + msg.what);
                        }
                    }
                    break;
                case IMS_UT_EVENT_GET_CF_WITH_CLASS:
                    if (null != mListener) {
                        AsyncResult ar = (AsyncResult) msg.obj;
                        if (null == ar.exception) {

                            CallForwardInfo[] cfInfo = (CallForwardInfo[]) ar.result;
                            ImsCallForwardInfo[] imsCfInfo =
                                    mExtPluginBase.getImsCallForwardInfo(cfInfo);

                            mListener.onUtConfigurationCallForwardQueried(msg.arg1, imsCfInfo);
                        } else {
                            ImsReasonInfo reason;
                            if (ar.exception instanceof CommandException) {
                                reason = mOemPluginBase.commandExceptionToReason(
                                        (CommandException)(ar.exception), mPhoneId);
                            } else {
                                reason = new ImsReasonInfo(ImsReasonInfo.CODE_UT_NETWORK_ERROR, 0);
                            }

                            mImsUtImpl.notifyUtConfigurationQueryFailed(msg, reason);
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
