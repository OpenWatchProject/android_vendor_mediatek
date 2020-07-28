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
import android.telephony.ims.stub.ImsUtImplBase;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.ImsCallForwardInfo;
import android.telephony.ims.ImsSsInfo;
import android.telephony.ims.ImsUtListener;

import android.util.Log;

import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;

import com.android.ims.ImsUtInterface;
import com.android.ims.internal.IImsUt;
import com.android.ims.internal.IImsUtListener;
import com.android.internal.telephony.CallForwardInfo;
import com.android.internal.telephony.CommandsInterface;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.RILConstants;
import com.android.internal.telephony.TelephonyIntents;

import java.net.UnknownHostException;

import com.mediatek.ims.ril.ImsRILConstants;

import com.mediatek.ims.ImsService;
import com.mediatek.ims.MtkImsCallForwardInfo;
import com.mediatek.ims.MtkImsReasonInfo;
import com.mediatek.ims.OperatorUtils;
import com.mediatek.ims.OperatorUtils.OPID;
import com.mediatek.ims.SuppSrvConfig;
import com.mediatek.internal.telephony.MtkCallForwardInfo;
import com.mediatek.internal.telephony.MtkPhoneConstants;
import com.mediatek.internal.telephony.MtkSuppServHelper;
import com.mediatek.simservs.xcap.XcapException;


import static com.android.internal.telephony.imsphone.ImsPhoneMmiCode.UT_BUNDLE_KEY_CLIR;
import static com.android.internal.telephony.imsphone.ImsPhoneMmiCode.UT_BUNDLE_KEY_SSINFO;

/**
 * ImsUT class for handle the IMS UT interface.
 *
 * The implementation is based on IR.92
 *
 *  @hide
 */
public class ImsUtStub extends ImsUtImplBase {
    private static final String TAG = "ImsUtService";
    private static final boolean DBG = true;

    private static HashMap<Integer, ImsUtStub> sImsUtStubs =
            new HashMap<Integer, ImsUtStub>();

    private Context mContext;

    private static final Object mLock = new Object();
    private static int sRequestId = 0;
    private ImsUtListener mListener = null;
    private MMTelSSTransport mMMTelSSTSL;
    private ResultHandler mHandler;

    private ImsService mImsService = null;
    private int mPhoneId = 0;

    private boolean mIsInVoLteCall = false;
    private boolean mIsNeedImsDereg = false;

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
    static final int IMS_UT_EVENT_IMS_DEREG = 1014;

    static final int HTTP_ERROR_CODE_400 = 400;
    static final int HTTP_ERROR_CODE_403 = 403;
    static final int HTTP_ERROR_CODE_404 = 404;
    static final int HTTP_ERROR_CODE_409 = 409;

    private static final int IMS_DEREG_CAUSE_BY_SS_CONFIG = 2;   //For AT command for IMS dereg

    private static final int DEFAULT_INVALID_PHONE_ID = -1;

    private static final String IMS_DEREG_PROP = "vendor.gsm.radio.ss.imsdereg";
    private static final String IMS_DEREG_ON = "1";
    private static final String IMS_DEREG_OFF = "0";

    // Set a system property to avoid that the IMS register is with problem.
    private static final String IMS_DEREG_DISABLE_PROP = "persist.vendor.radio.ss.imsdereg_off";

    private final BroadcastReceiver mIntentReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            // TODO Auto-generated method stub
            if (DBG) {
                Log.d(TAG, "Intent action:" + intent.getAction());
            }

            if (intent.getAction().equals(PhoneConstants
                    .ACTION_SUBSCRIPTION_PHONE_STATE_CHANGED)) {

                String state = intent.getStringExtra(PhoneConstants.STATE_KEY);
                onReceivePhoneStateChange(
                        intent.getIntExtra(PhoneConstants.SLOT_KEY, DEFAULT_INVALID_PHONE_ID),
                        intent.getIntExtra(MtkPhoneConstants.PHONE_TYPE_KEY, RILConstants.NO_PHONE),
                        Enum.valueOf(PhoneConstants.State.class, state));
            }
        }
    };

    /**
     *
     * Construction function for ImsConfigStub.
     *
     * @param context the application context
     *
     */
    private ImsUtStub(Context context, int phoneId, ImsService imsService) {
        mContext = context;
        mMMTelSSTSL = MMTelSSTransport.getInstance();
        mMMTelSSTSL.registerUtService(mContext);

        HandlerThread thread = new HandlerThread("ImsUtStubResult");
        thread.start();
        Looper looper = thread.getLooper();
        mHandler = new ResultHandler(looper);

        IntentFilter intentFilter =
                new IntentFilter(PhoneConstants.ACTION_SUBSCRIPTION_PHONE_STATE_CHANGED);
        mContext.registerReceiver(mIntentReceiver, intentFilter);

        mImsService = imsService;
        mPhoneId = phoneId;
    }

    public static ImsUtStub getInstance(Context context, int phoneId, ImsService service) {
        synchronized (sImsUtStubs) {
            if (sImsUtStubs.containsKey(phoneId)) {
                ImsUtStub m = sImsUtStubs.get(phoneId);
                return m;
            } else {
                sImsUtStubs.put(phoneId, new ImsUtStub(context, phoneId, service));
                return sImsUtStubs.get(phoneId);
            }
        }
    }

    private void onReceivePhoneStateChange(int phoneId, int phoneType,
            PhoneConstants.State phoneState) {
        if (DBG) {
            Log.d(TAG, "onReceivePhoneStateChange phoneId:" + phoneId +
                    " phoneType: " + phoneType + " phoneState: " + phoneState +
                    " mIsInVoLteCall: " + mIsInVoLteCall);
        }

        if (phoneId != mPhoneId) {
            return;
        }

        if (mIsInVoLteCall == true) {
            if (phoneState == PhoneConstants.State.IDLE) {
                mIsInVoLteCall = false;
                if (mIsNeedImsDereg) {
                    mHandler.sendMessage(
                               mHandler.obtainMessage(IMS_UT_EVENT_IMS_DEREG, null));
                    mIsNeedImsDereg = false;
                }
            }
        } else {
            if (phoneState != PhoneConstants.State.IDLE
                    && phoneType == RILConstants.IMS_PHONE) {
                mIsInVoLteCall = true;
            }
        }
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
                        } else if (ar.exception instanceof UnknownHostException) {
                            if (DBG) {
                                Log.d(TAG, "IMS_UT_EVENT_GET_CB: UnknownHostException.");
                            }
                            mListener.onUtConfigurationQueryFailed(msg.arg1,
                                    new ImsReasonInfo(MtkImsReasonInfo.CODE_UT_UNKNOWN_HOST, 0));
                        } else if (ar.exception instanceof XcapException) {
                            XcapException xcapException = (XcapException) ar.exception;
                            mListener.onUtConfigurationQueryFailed(msg.arg1,
                                    xcapExceptionToImsReasonInfo(xcapException,mPhoneId));
                        } else {
                            mListener.onUtConfigurationQueryFailed(msg.arg1,
                                    new ImsReasonInfo(ImsReasonInfo.CODE_UT_NETWORK_ERROR,
                                            0));
                        }
                    }
                    break;
                case IMS_UT_EVENT_GET_CF:
                    if (null != mListener) {
                        AsyncResult ar = (AsyncResult) msg.obj;

                        if (null == ar.exception) {
                            CallForwardInfo[] cfInfo = (CallForwardInfo[]) ar.result;
                            ImsCallForwardInfo[] imsCfInfo = null;

                            if (cfInfo != null) {
                                imsCfInfo = new ImsCallForwardInfo[cfInfo.length];
                                for (int i = 0; i < cfInfo.length; i++) {
                                    imsCfInfo[i] = getImsCallForwardInfo(cfInfo[i]);
                                    if (DBG) {
                                        Log.d(TAG, "IMS_UT_EVENT_SET_CF: cfInfo[" + i + "] = "
                                                + ", Condition: " + imsCfInfo[i].getCondition()
                                                + ", Status: " + ((imsCfInfo[i].getStatus() == 0) ? "disabled" : "enabled")
                                                + ", ToA: " + imsCfInfo[i].getToA()
                                                + ", Service Class: " + imsCfInfo[i].getServiceClass()
                                                + ", Number=" + MtkSuppServHelper.encryptString(imsCfInfo[i].getNumber())
                                                + ", Time (seconds): " + imsCfInfo[i].getTimeSeconds());
                                    }
                                }
                            }

                            if (ssConfig.isNeedIMSDereg()) {
                                boolean enable = IMS_DEREG_ON.equals(
                                        SystemProperties.get(IMS_DEREG_PROP, IMS_DEREG_OFF));

                                SystemProperties.set(IMS_DEREG_PROP, IMS_DEREG_OFF);

                                boolean disableIMSDereg =
                                        "1".equals(SystemProperties.get(IMS_DEREG_DISABLE_PROP, "-1"));
                                if (enable && !disableIMSDereg) {
                                    if (mIsInVoLteCall) {
                                        Log.d(TAG, "During call and later do IMS dereg");
                                        mIsNeedImsDereg = true;
                                    } else {
                                        Log.d(TAG, "IMS dereg.");
                                        mImsService.deregisterImsWithCause(
                                                mPhoneId, IMS_DEREG_CAUSE_BY_SS_CONFIG);
                                    }
                                } else {
                                    Log.d(TAG, "Skip IMS dereg.");
                                }
                            }

                            mListener.onUtConfigurationCallForwardQueried(msg.arg1, imsCfInfo);
                        } else {
                            if (ar.exception instanceof XcapException) {
                                XcapException xcapException = (XcapException) ar.exception;
                                mListener.onUtConfigurationQueryFailed(msg.arg1,
                                    xcapExceptionToImsReasonInfo(xcapException,mPhoneId));
                            } else if (ar.exception instanceof UnknownHostException) {
                                if (DBG) {
                                    Log.d(TAG, "IMS_UT_EVENT_GET_CF: UnknownHostException.");
                                }
                                mListener.onUtConfigurationQueryFailed(msg.arg1,
                                        new ImsReasonInfo(MtkImsReasonInfo.CODE_UT_UNKNOWN_HOST,
                                        0));
                            } else {
                                mListener.onUtConfigurationQueryFailed(msg.arg1,
                                        new ImsReasonInfo(ImsReasonInfo.CODE_UT_NETWORK_ERROR,
                                                0));
                            }
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
                        } else if (ar.exception instanceof UnknownHostException) {
                            if (DBG) {
                                Log.d(TAG, "IMS_UT_EVENT_GET_CW: UnknownHostException.");
                            }
                            mListener.onUtConfigurationQueryFailed(msg.arg1,
                                    new ImsReasonInfo(MtkImsReasonInfo.CODE_UT_UNKNOWN_HOST, 0));
                        } else if (ar.exception instanceof XcapException) {
                            XcapException xcapException = (XcapException) ar.exception;
                            mListener.onUtConfigurationQueryFailed(msg.arg1,
                                    xcapExceptionToImsReasonInfo(xcapException,mPhoneId));
                        } else {
                            mListener.onUtConfigurationQueryFailed(msg.arg1,
                                    new ImsReasonInfo(ImsReasonInfo.CODE_UT_NETWORK_ERROR,
                                            0));
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
                        } else if (ar.exception instanceof UnknownHostException) {
                            if (DBG) {
                                Log.d(TAG, "IMS_UT_EVENT_GET_CLIR: UnknownHostException.");
                            }
                            mListener.onUtConfigurationQueryFailed(msg.arg1,
                                    new ImsReasonInfo(MtkImsReasonInfo.CODE_UT_UNKNOWN_HOST, 0));
                        } else if (ar.exception instanceof XcapException) {
                            XcapException xcapException = (XcapException) ar.exception;
                            mListener.onUtConfigurationQueryFailed(msg.arg1,
                                    xcapExceptionToImsReasonInfo(xcapException,mPhoneId));
                        } else {
                            mListener.onUtConfigurationQueryFailed(msg.arg1,
                                    new ImsReasonInfo(ImsReasonInfo.CODE_UT_NETWORK_ERROR,
                                            0));
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
                            if (ar.exception instanceof XcapException) {
                                XcapException xcapException = (XcapException) ar.exception;
                                mListener.onUtConfigurationQueryFailed(msg.arg1,
                                        xcapExceptionToImsReasonInfo(xcapException,mPhoneId));
                            } else if (ar.exception instanceof UnknownHostException) {
                                if (DBG) {
                                    Log.d(TAG, "UnknownHostException. event = " + msg.what);
                                }
                                mListener.onUtConfigurationQueryFailed(msg.arg1,
                                        new ImsReasonInfo(MtkImsReasonInfo.CODE_UT_UNKNOWN_HOST,
                                        0));
                            } else {
                                mListener.onUtConfigurationQueryFailed(msg.arg1,
                                        new ImsReasonInfo(ImsReasonInfo.CODE_UT_NETWORK_ERROR,
                                                0));
                            }
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
                                        if (DBG) {
                                            Log.d(TAG, "IMS_UT_EVENT_SET_CF: cfInfo[" + i + "] = "
                                                    + cfInfo[i]);
                                        }
                                        imsCfInfo[i] = getImsCallForwardInfo(cfInfo[i]);
                                    }
                                }

                                mListener.onUtConfigurationCallForwardQueried(msg.arg1, imsCfInfo);
                                        //Use this function to append the cfinfo.
                                break;  //Break here and no need to do the below process.
                                        //If ar.result is null, then use the original flow.
                            }
                        } else if (null == ar.exception) {
                            if (ssConfig.isNeedIMSDereg()) {
                                boolean enable = IMS_DEREG_ON.equals(
                                        SystemProperties.get(IMS_DEREG_PROP, IMS_DEREG_OFF));

                                SystemProperties.set(IMS_DEREG_PROP, IMS_DEREG_OFF);

                                boolean disableIMSDereg =
                                        "1".equals(SystemProperties.get(IMS_DEREG_DISABLE_PROP,
                                                "-1"));
                                if (enable && !disableIMSDereg) {
                                    if (mIsInVoLteCall) {
                                        Log.d(TAG, "During call and later do IMS dereg");
                                        mIsNeedImsDereg = true;
                                    } else {
                                        Log.d(TAG, "IMS dereg.");
                                        mImsService.deregisterImsWithCause(
                                                mPhoneId, IMS_DEREG_CAUSE_BY_SS_CONFIG);
                                    }
                                } else {
                                    Log.d(TAG, "Skip IMS dereg.");
                                }
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
                            if (ar.exception instanceof XcapException) {
                                XcapException xcapException = (XcapException) ar.exception;
                                mListener.onUtConfigurationUpdateFailed(msg.arg1,
                                        xcapExceptionToImsReasonInfo(xcapException,mPhoneId));
                            } else if (ar.exception instanceof UnknownHostException) {
                                if (DBG) {
                                    Log.d(TAG, "UnknownHostException. event = " + msg.what);
                                }
                                mListener.onUtConfigurationUpdateFailed(msg.arg1,
                                        new ImsReasonInfo(MtkImsReasonInfo.CODE_UT_UNKNOWN_HOST,
                                        0));
                            } else {
                                mListener.onUtConfigurationUpdateFailed(msg.arg1,
                                        new ImsReasonInfo(ImsReasonInfo.CODE_UT_NETWORK_ERROR,
                                                0));
                            }
                        }
                    }
                    break;
                case IMS_UT_EVENT_IMS_DEREG:
                    mImsService.deregisterImsWithCause(
                            mPhoneId, IMS_DEREG_CAUSE_BY_SS_CONFIG);
                    break;
                default:
                    Log.d(TAG, "Unknown Event: " + msg.what);
                    break;
            }
        }
    };

    /**
     * Closes the object. This object is not usable after being closed.
     */
    @Override
    public void close() {
        mContext.unregisterReceiver(mIntentReceiver);
    }

    protected String getFacilityFromCBType(int cbType) {
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

    protected int getCFActionFromAction(int cfAction) {
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

    protected int getCFReasonFromCondition(int condition) {
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

    protected int getConditionFromCFReason(int reason) {
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

    protected ImsCallForwardInfo getImsCallForwardInfo(CallForwardInfo info) {
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
     */
    @Override
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
        Message msg = mHandler.obtainMessage(IMS_UT_EVENT_GET_CB, requestId, 0, null);

        SuppSrvConfig ssConfig = SuppSrvConfig.getInstance(mContext);
        ssConfig.update(mPhoneId);

        int serviceClass = MMTelSSUtils.getServiceClass() != -1 ?
                MMTelSSUtils.getServiceClass() : CommandsInterface.SERVICE_CLASS_VOICE;

        mMMTelSSTSL.queryFacilityLock(facility, null, serviceClass, msg, mPhoneId);

        MMTelSSUtils.resetServcieClass();

        return requestId;
    }

    /**
     * Retrieves the configuration of the call barring.
     */
    @Override
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

        SuppSrvConfig ssConfig = SuppSrvConfig.getInstance(mContext);
        ssConfig.update(mPhoneId);

        mMMTelSSTSL.queryFacilityLock(facility, null, serviceClass, msg, mPhoneId);

        return requestId;
    }

    /**
     * Retrieves the configuration of the call forward.
     */
    @Override
    public int queryCallForward(int condition, String number) {
        int requestId;

        synchronized (mLock) {
            requestId = sRequestId;
            sRequestId++;
        }
        if (DBG) {
            Log.d(TAG, "queryCallForward(): requestId = " + requestId);
        }

        Message msg = mHandler.obtainMessage(IMS_UT_EVENT_GET_CF, requestId, 0, null);

        SuppSrvConfig ssConfig = SuppSrvConfig.getInstance(mContext);
        ssConfig.update(mPhoneId);

        int serviceClass = MMTelSSUtils.getServiceClass() != -1 ? MMTelSSUtils.getServiceClass()
                : CommandsInterface.SERVICE_CLASS_NONE;

        mMMTelSSTSL.queryCallForwardStatus(getCFReasonFromCondition(condition), serviceClass,
                number, msg, mPhoneId);

        MMTelSSUtils.resetServcieClass();

        return requestId;
    }

    /**
     * Retrieves the configuration of the call waiting.
     */
    @Override
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

        SuppSrvConfig ssConfig = SuppSrvConfig.getInstance(mContext);
        ssConfig.update(mPhoneId);

        mMMTelSSTSL.queryCallWaiting(CommandsInterface.SERVICE_CLASS_VOICE, msg, mPhoneId);

        return requestId;
    }

    /**
     * Retrieves the default CLIR setting.
     */
    @Override
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

        SuppSrvConfig ssConfig = SuppSrvConfig.getInstance(mContext);
        ssConfig.update(mPhoneId);

        mMMTelSSTSL.getCLIR(msg, mPhoneId);

        return requestId;
    }

    /**
     * Retrieves the CLIP call setting.
     */
    @Override
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

        SuppSrvConfig ssConfig = SuppSrvConfig.getInstance(mContext);
        ssConfig.update(mPhoneId);

        mMMTelSSTSL.queryCLIP(msg, mPhoneId);

        return requestId;
    }

    /**
     * Retrieves the COLR call setting.
     */
    @Override
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

        SuppSrvConfig ssConfig = SuppSrvConfig.getInstance(mContext);
        ssConfig.update(mPhoneId);

        mMMTelSSTSL.getCOLR(msg, mPhoneId);

        return requestId;
    }

    /**
     * Retrieves the COLP call setting.
     */
    @Override
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

        SuppSrvConfig ssConfig = SuppSrvConfig.getInstance(mContext);
        ssConfig.update(mPhoneId);

        mMMTelSSTSL.getCOLP(msg, mPhoneId);

        return requestId;
    }

    /**
     * Updates or retrieves the supplementary service configuration.
     */
    @Override
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
     */
    @Override
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
        Message msg = mHandler.obtainMessage(IMS_UT_EVENT_SET_CB, requestId, 0, null);

        SuppSrvConfig ssConfig = SuppSrvConfig.getInstance(mContext);
        ssConfig.update(mPhoneId);

        int serviceClass = MMTelSSUtils.getServiceClass() != -1 ? MMTelSSUtils.getServiceClass()
                : CommandsInterface.SERVICE_CLASS_VOICE;

        mMMTelSSTSL.setFacilityLock(facility, bEnable, null, serviceClass, msg, mPhoneId);

        MMTelSSUtils.resetServcieClass();

        return requestId;
    }

    /**
     * Updates the configuration of the call barring.
     */
    @Override
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

        SuppSrvConfig ssConfig = SuppSrvConfig.getInstance(mContext);
        ssConfig.update(mPhoneId);

        mMMTelSSTSL.setFacilityLock(facility, bEnable, null, serviceClass, msg, mPhoneId);

        return requestId;
    }

    /**
     * Updates the configuration of the call forward.
     */
    @Override
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

        SuppSrvConfig ssConfig = SuppSrvConfig.getInstance(mContext);
        ssConfig.update(mPhoneId);

        mMMTelSSTSL.setCallForward(getCFActionFromAction(action),
                getCFReasonFromCondition(condition), serviceClass,
                number, timeSeconds, msg, mPhoneId);

        return requestId;
    }

    /**
     * Updates the configuration of the call waiting.
     */
    @Override
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

        SuppSrvConfig ssConfig = SuppSrvConfig.getInstance(mContext);
        ssConfig.update(mPhoneId);

        mMMTelSSTSL.setCallWaiting(enable, serviceClass, msg, mPhoneId);

        return requestId;
    }

    /**
     * Updates the configuration of the CLIR supplementary service.
     */
    @Override
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

        SuppSrvConfig ssConfig = SuppSrvConfig.getInstance(mContext);
        ssConfig.update(mPhoneId);

        mMMTelSSTSL.setCLIR(clirMode, msg, mPhoneId);

        return requestId;
    }

    /**
     * Updates the configuration of the CLIP supplementary service.
     */
    @Override
    public int updateCLIP(boolean enable) {
        int requestId;

        synchronized (mLock) {
            requestId = sRequestId;
            sRequestId++;
        }
        if (DBG) {
            Log.d(TAG, "updateCLIP(): requestId = " + requestId);
        }

        int enableClip = (enable) ? 1 : 0;
        Message msg = mHandler.obtainMessage(IMS_UT_EVENT_SET_CLIP, requestId, 0, null);

        SuppSrvConfig ssConfig = SuppSrvConfig.getInstance(mContext);
        ssConfig.update(mPhoneId);

        mMMTelSSTSL.setCLIP(enableClip, msg, mPhoneId);

        return requestId;
    }

    /**
     * Updates the configuration of the COLR supplementary service.
     */
    @Override
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

        SuppSrvConfig ssConfig = SuppSrvConfig.getInstance(mContext);
        ssConfig.update(mPhoneId);

        mMMTelSSTSL.setCOLR(presentation, msg, mPhoneId);

        return requestId;
    }

    /**
     * Updates the configuration of the COLP supplementary service.
     */
    @Override
    public int updateCOLP(boolean enable) {
        int requestId;

        synchronized (mLock) {
            requestId = sRequestId;
            sRequestId++;
        }
        if (DBG) {
            Log.d(TAG, "updateCOLP(): requestId = " + requestId);
        }

        int enableColp = (enable) ? 1 : 0;
        Message msg = mHandler.obtainMessage(IMS_UT_EVENT_SET_COLP, requestId, 0, null);

        SuppSrvConfig ssConfig = SuppSrvConfig.getInstance(mContext);
        ssConfig.update(mPhoneId);

        mMMTelSSTSL.setCOLP(enableColp, msg, mPhoneId);

        return requestId;
    }

    /**
     * Sets the listener.
     */
    @Override
    public void setListener(ImsUtListener listener) {
        mListener = listener;
    }

    /**
     * Convert XcapExcaption to ImsReasonInfo.
     * @param xcapEx the XcapExcaption
     * @return the converted ImsReasonInfo
     */
    static ImsReasonInfo xcapExceptionToImsReasonInfo(XcapException xcapEx, int phoneId) {
        ImsReasonInfo reason;

        if ((DBG) && (xcapEx != null)) {
            Log.d(TAG, "xcapExceptionToImsReasonInfo(): XcapException: "
                    + "code = " + xcapEx.getExceptionCodeCode()
                    + ", http error = " + xcapEx.getHttpErrorCode()
                    + ", isConnectionError = " + xcapEx.isConnectionError()
                    + ", phoneId = " + phoneId);
        }

        if(OperatorUtils.isMatched(OPID.OP02, phoneId)
                && xcapEx.getHttpErrorCode() == HTTP_ERROR_CODE_400) {
            Log.d(TAG, "xcapExceptionToImsReasonInfo - translate 400 error cause to 403");
            reason = new ImsReasonInfo(MtkImsReasonInfo.CODE_UT_XCAP_403_FORBIDDEN, 0);
            return reason;
        }

        if ((xcapEx != null) && (xcapEx.getHttpErrorCode() == HTTP_ERROR_CODE_403)) {
            reason = new ImsReasonInfo(MtkImsReasonInfo.CODE_UT_XCAP_403_FORBIDDEN, 0);
        } else if ((xcapEx != null) && (xcapEx.getHttpErrorCode() == HTTP_ERROR_CODE_404)) {
            reason = new ImsReasonInfo(MtkImsReasonInfo.CODE_UT_XCAP_404_NOT_FOUND, 0);
        } else if ((xcapEx != null) && (xcapEx.getHttpErrorCode() == HTTP_ERROR_CODE_409)) {
            reason = new ImsReasonInfo(MtkImsReasonInfo.CODE_UT_XCAP_409_CONFLICT, 0);
        } else {
            reason = new ImsReasonInfo(ImsReasonInfo.CODE_UT_NETWORK_ERROR, 0);
        }

        return reason;
    }

    public static int getAndIncreaseRequestId() {
        int requestId = 0;
        synchronized (mLock) {
            requestId = sRequestId;
            sRequestId++;
        }
        return requestId;
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

    public void notifyUtConfigurationCallForwardQueried(Message msg, ImsCallForwardInfo[] cfInfo) {
        mListener.onUtConfigurationCallForwardQueried(msg.arg1, cfInfo);
    }
}
