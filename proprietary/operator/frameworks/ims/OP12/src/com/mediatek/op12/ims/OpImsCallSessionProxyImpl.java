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

package com.mediatek.op12.ims;

import android.bluetooth.BluetoothHeadset;
import android.bluetooth.BluetoothProfile;
import android.content.Context;
import android.media.AudioManager;
import android.media.ToneGenerator;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.RemoteException;
import android.telephony.ims.ImsReasonInfo;
import android.content.Intent;
import android.os.Bundle;
import android.os.SystemProperties;
import android.telephony.ims.ImsCallProfile;
import android.telephony.Rlog;

import com.mediatek.internal.telephony.MtkPhoneNumberUtils;
import com.mediatek.ims.ext.OpImsCallSessionProxyBase;
import com.mediatek.ims.internal.MtkImsManager;
import com.mediatek.ims.internal.IMtkImsCallSession;
import com.mediatek.ims.MtkImsCallSessionProxy;
import com.mediatek.ims.ImsCallSessionProxy;
import com.mediatek.ims.ril.ImsCommandsInterface;
import com.mediatek.ims.ril.OpImsCommandsInterface;
import com.mediatek.ims.SipMessage;
import com.mediatek.wfo.WifiOffloadManager;

public class OpImsCallSessionProxyImpl extends OpImsCallSessionProxyBase {
    private static final String TAG = "Op12ImsCallSessionProxyImpl";
    private static final boolean MDMI_SUPPORT = false;
    int mCallStatus = CALL_STATUS_INACTIVE;

    // For MDMI
    private static final int CALL_STATUS_INACTIVE = 0;
    private static final int CALL_STATUS_ACTIVE = 1;
    private static final int CALL_STATUS_ATTEMPTING = 2;
    private static final int CALL_STATUS_HOLD = 3;

    private static final int RAT_INFO_NO_SERVICE = 0;
    private static final int RAT_INFO_GSM = 1;
    private static final int RAT_INFO_UMTS = 2;
    private static final int RAT_INFO_LTE = 3;
    private static final int RAT_INFO_CDMA = 4;
    private static final int RAT_INFO_WIFI = 5;

    private static final int EVENT_BUSY_TONE_TIMEOUT = 1;
    private static final int EVENT_INCOMING_CALL_INDICATION = 2;

    private static final int TONE_RELATIVE_VOLUME_HIPRI = 80;

    private int mBusyToneTimeout = 30000;

    private Context mContext;
    private int mSipErrCode = -1;
    private IMtkImsCallSession mMtkImsCallSession;
    private ImsCommandsInterface mImsRIL;
    private ToneGenerator mToneGenerator;
    private BluetoothHeadset mBluetoothHeadset;
    private boolean mHandlingCallStartFailed;

    private Handler mHandler = new Handler(Looper.getMainLooper()) {
        @Override
        public void handleMessage(Message msg) {
            Rlog.d(TAG, "handleMessage(): " + msg.what);
            switch(msg.what) {
                case EVENT_BUSY_TONE_TIMEOUT:
                case EVENT_INCOMING_CALL_INDICATION:
                    processCallTerminated();
                    break;
                default:
                    break;
            }
        }
    };
    private static final int CALL_TYPE_EMERGENCY = 0;
    private static final int CALL_TYPE_VOICE = 1;
    private static final int CALL_TYPE_VIDEO = 2;

    private static final int CALL_DIRECTION_MO = 0;
    private static final int CALL_DIRECTION_MT = 1;

    public OpImsCallSessionProxyImpl(Context context) {
        mContext = context;
        Rlog.d(TAG, "OpImsCallSessionProxyImpl");
    }

    @Override
    public ImsReasonInfo getImsReasonInfo(SipMessage sipMsg) {
        ImsReasonInfo info = null;
        if (sipMsg.getMethod() == SipMessage.METHOD_INVITE) {
            mSipErrCode = sipMsg.getCode();
            if (sipMsg.getCode() == SipMessage.CODE_SESSION_INVITE_FAILED_REMOTE_BUSY) {
                info = new ImsReasonInfo(ImsReasonInfo.CODE_SIP_BUSY, 0, sipMsg.getReasonHeader());
            }
        }

        Rlog.d(TAG, "getImsReasonInfo(): " + info);
        return info;
    }

    @Override
    public boolean handleCallStartFailed(IMtkImsCallSession imsCallSession,
            Object imsRILAdapter, boolean hasHoldingCall) {
        if (mSipErrCode != SipMessage.CODE_SESSION_INVITE_FAILED_REMOTE_BUSY) {
            return false;
        }
        Rlog.d(TAG, "handleCallStartFailed()");
        mHandlingCallStartFailed = true;
        mMtkImsCallSession = imsCallSession;
        mImsRIL = ((ImsCommandsInterface) imsRILAdapter);
        registerForIncomingCall();
        if (hasHoldingCall) {
            mBusyToneTimeout = 5000;
        }
        startBusyTone();
        mHandler.sendEmptyMessageDelayed(EVENT_BUSY_TONE_TIMEOUT, mBusyToneTimeout);
        return true;
    }

    @Override
    public boolean handleHangup() {
        if (mHandlingCallStartFailed == false) {
            return false;
        }
        Rlog.d(TAG, "handleHangup()");
        processCallTerminated();
        return true;
    }

    private void processCallTerminated() {
        mHandler.removeCallbacksAndMessages(null);
        stopBusyTone();
        unregisterForIncomingCall();
        if (mMtkImsCallSession != null) {
            try {
                mMtkImsCallSession.callTerminated();
            } catch (RemoteException e) {
                Rlog.e(TAG, "RemoteException callTerminated()");
            }
        }
    }

    private void registerForIncomingCall() {
        if (mImsRIL == null) return;
        mImsRIL.setOnIncomingCallIndication(mHandler, EVENT_INCOMING_CALL_INDICATION, null);
    }

    private void unregisterForIncomingCall() {
        if (mImsRIL == null) return;
        mImsRIL.unsetOnIncomingCallIndication(mHandler);
    }

    private void startBusyTone() {
        Rlog.d(TAG, "startBusyTone()");

        int toneType = ToneGenerator.TONE_SUP_BUSY;
        int toneVolume = TONE_RELATIVE_VOLUME_HIPRI;
        int stream = AudioManager.STREAM_VOICE_CALL;
        if (mBluetoothHeadset != null && mBluetoothHeadset.isAudioOn() == true) {
            stream = AudioManager.STREAM_BLUETOOTH_SCO;
        }

        mToneGenerator = new ToneGenerator(stream, toneVolume);
        mToneGenerator.startTone(toneType);
    }

    private void stopBusyTone() {
        Rlog.d(TAG, "stopBusyTone()");
        if (mToneGenerator == null) return;
        mToneGenerator.stopTone();
        mToneGenerator.release();
        mToneGenerator = null;
    }

    private BluetoothProfile.ServiceListener mBluetoothProfileServiceListener =
            new BluetoothProfile.ServiceListener() {
                public void onServiceConnected(int profile, BluetoothProfile proxy) {
                    mBluetoothHeadset = (BluetoothHeadset) proxy;
                    Rlog.d(TAG,"got BluetoothHeadset: " + mBluetoothHeadset);
                }

                public void onServiceDisconnected(int profile) {
                    mBluetoothHeadset = null;
                }
            };

    // For MDMI
    @Override
    public void sendCallEventWithRat(Bundle extras) {
        if (!MDMI_SUPPORT) {
            return;
        }

        int msgType = extras.getInt(ImsCallSessionProxy.EXTRA_CALL_INFO_MESSAGE_TYPE, -1);
        int callType = extras.getInt(ImsCallSessionProxy.EXTRA_CALL_TYPE, CALL_TYPE_VOICE);
        int ratType = extras.getInt(ImsCallSessionProxy.EXTRA_RAT_TYPE, RAT_INFO_LTE);
        boolean isIncoming = extras.getBoolean(ImsCallSessionProxy.EXTRA_INCOMING_CALL, true);
        boolean isEmergency = extras.getBoolean(ImsCallSessionProxy.EXTRA_EMERGENCY_CALL, false);

        int callStatus;
        int callTypeToSend;
        int ratInfo;
        int callDirection;

        Rlog.d(TAG, "sendCallEventWithRat: msgType: " + msgType + " callType: " + callType +
                " ratType: " + ratType + " isIncoming: " + isIncoming +
                " isEmergency: " + isEmergency);

        // callStatus
        switch (msgType) {
            case ImsCallSessionProxy.CALL_INFO_MSG_TYPE_ALERT:
            case ImsCallSessionProxy.CALL_INFO_MSG_TYPE_SETUP:
            case ImsCallSessionProxy.CALL_INFO_MSG_TYPE_MO_CALL_ID_ASSIGN:
                callStatus = CALL_STATUS_ATTEMPTING;
                break;
            case ImsCallSessionProxy.CALL_INFO_MSG_TYPE_HELD:
                callStatus = CALL_STATUS_HOLD;
                break;
            case ImsCallSessionProxy.CALL_INFO_MSG_TYPE_CONNECTED:
            case ImsCallSessionProxy.CALL_INFO_MSG_TYPE_ACTIVE:
                callStatus = CALL_STATUS_ACTIVE;
                break;
            case ImsCallSessionProxy.CALL_INFO_MSG_TYPE_DISCONNECTED:
                callStatus = CALL_STATUS_INACTIVE;
                break;
            case ImsCallSessionProxy.CALL_INFO_MSG_TYPE_REMOTE_HOLD:
            case ImsCallSessionProxy.CALL_INFO_MSG_TYPE_REMOTE_RESUME:
                Rlog.d(TAG, "Don't update callStatus when remote hold/resume, msgType: " + msgType);
                return;
            default:
                Rlog.e(TAG, "Not defined msgType: " + msgType + ", ignore it.");
                return;
        }

        if (mCallStatus == callStatus) {
            // ignore duplicate callStatus
            Rlog.d(TAG, "Ignore duplicate callStatus: " + mCallStatus + ", " + callStatus);
            return;
        }
        mCallStatus = callStatus;

        // call type
        if (isEmergency) {
            callTypeToSend = CALL_TYPE_EMERGENCY;
        } else if (callType == ImsCallProfile.CALL_TYPE_VOICE) {
            callTypeToSend = CALL_TYPE_VOICE;
        } else {
            callTypeToSend = CALL_TYPE_VIDEO;
        }

        // rat info
        switch (ratType) {
            case WifiOffloadManager.RAN_TYPE_MOBILE_3GPP:
                ratInfo = RAT_INFO_LTE;
                break;
            case WifiOffloadManager.RAN_TYPE_WIFI:
                ratInfo = RAT_INFO_WIFI;
                break;
            default:
                ratInfo = RAT_INFO_NO_SERVICE;
                break;
        }

        // call direction
        if (isIncoming) {
            callDirection = CALL_DIRECTION_MT;
        } else {
            callDirection = CALL_DIRECTION_MO;
        }

        Rlog.i(TAG, "sendCallEventWithRat: callStatus: " + callStatus +
                " callTypeToSend: " + callTypeToSend +
                " ratInfo: " + ratInfo + " callDirection: " + callDirection);
        int[] psCallEvent = new int[4];
        psCallEvent[0] = callStatus;
        psCallEvent[1] = callTypeToSend;
        psCallEvent[2] = ratInfo;
        psCallEvent[3] = callDirection;
    }
}
