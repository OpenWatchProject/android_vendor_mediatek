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

package com.mediatek.ims;

import android.content.Context;
import android.os.Handler;
import android.os.Message;
import android.os.Messenger;
import android.os.RemoteException;
import android.telephony.ims.ImsCallProfile;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.ImsCallSessionListener;
import android.telephony.Rlog;

import com.android.ims.internal.IImsCallSession;
import com.android.ims.internal.IImsCallSessionListener;

import com.mediatek.ims.internal.IMtkImsCallSession;
import com.mediatek.ims.internal.IMtkImsCallSessionListener;
import com.mediatek.ims.ril.ImsCommandsInterface;

public class MtkImsCallSessionProxy implements AutoCloseable {
    private static final String LOG_TAG = "MtkImsCallSessionProxy";
    private static final boolean DBG = true;
    private ImsCallSessionProxy mAospImsCallSessionProxy;
    private IMtkImsCallSessionListener mMtkListener;

    private class ImsCallLogLevel {
        public static final int VERBOSE     = 1;
        public static final int DEBUG       = 2;
        public static final int INFO        = 3;
        public static final int WARNNING    = 4;
        public static final int ERROR       = 5;
    }

    private IMtkImsCallSession mServiceImpl = new IMtkImsCallSession.Stub() {

        @Override
        public void close() {
            MtkImsCallSessionProxy.this.close();
        }

        @Override
        public String getCallId() {
            return MtkImsCallSessionProxy.this.getCallId();
        }

        @Override
        public ImsCallProfile getCallProfile() {
            return MtkImsCallSessionProxy.this.getCallProfile();
        }

        @Override
        public void setListener(IMtkImsCallSessionListener listener) {
            MtkImsCallSessionProxy.this.setListener(listener);
        }

        @Override
        public IImsCallSession getIImsCallSession() {
            return MtkImsCallSessionProxy.this.getIImsCallSession();
        }

        @Override
        public void setIImsCallSession(IImsCallSession iSession) {
            MtkImsCallSessionProxy.this.setIImsCallSession(iSession);
        }

        @Override
        public boolean isIncomingCallMultiparty() {
            return MtkImsCallSessionProxy.this.isIncomingCallMultiparty();
        }

        @Override
        public void approveEccRedial(boolean isAprroved) {
            MtkImsCallSessionProxy.this.approveEccRedial(isAprroved);
        }

        @Override
        public void explicitCallTransfer() {
            MtkImsCallSessionProxy.this.explicitCallTransfer();
        }

        @Override
        public void unattendedCallTransfer(String number, int type) {
            MtkImsCallSessionProxy.this.unattendedCallTransfer(number, type);
        }

        @Override
        public void deviceSwitch(String number, String deviceId) {
            MtkImsCallSessionProxy.this.deviceSwitch(number, deviceId);
        }

        @Override
        public void cancelDeviceSwitch() {
            MtkImsCallSessionProxy.this.cancelDeviceSwitch();
        }

        @Override
        public void notifyDeviceSwitchFailed(ImsReasonInfo reasonInfo) {
            MtkImsCallSessionProxy.this.notifyDeviceSwitchFailed(reasonInfo);
        }

        @Override
        public void notifyDeviceSwitched() {
            MtkImsCallSessionProxy.this.notifyDeviceSwitched();
        }

        @Override
        public void resume() {
            MtkImsCallSessionProxy.this.resume();
        }

        @Override
        public void callTerminated() {
            MtkImsCallSessionProxy.this.callTerminated();
        }

        @Override
        public void setImsCallMode(int mode) {
            MtkImsCallSessionProxy.this.setImsCallMode(mode);
        }

        @Override
        public void removeLastParticipant() {
            MtkImsCallSessionProxy.this.removeLastParticipant();
        }

        @Override
        public String getHeaderCallId() {
            return MtkImsCallSessionProxy.this.getHeaderCallId();
        }
    };

    MtkImsCallSessionProxy(Context context, ImsCallProfile profile,
                           ImsCallSessionListener listener, ImsService imsService,
                           Handler handler, ImsCommandsInterface ci,
                           String callId, int phoneId) {
    }

    // Constructor for MO call
    MtkImsCallSessionProxy(Context context, ImsCallProfile profile,
                           ImsCallSessionListener listener, ImsService imsService,
                           Handler handler, ImsCommandsInterface ci, int phoneId) {
        this(context, profile, listener, imsService, handler, ci, null, phoneId);

        logWithCallId("MtkImsCallSessionProxy() : RILAdapter = " + ci, ImsCallLogLevel.DEBUG);
    }

    public void close() {

        logWithCallId("close() : MtkImsCallSessionProxy is going to be closed!!! ",
                ImsCallLogLevel.DEBUG);

        if (mAospImsCallSessionProxy != null) {
            mAospImsCallSessionProxy.close();
            mAospImsCallSessionProxy = null;
        }
        mMtkListener = null;
    }

    public String getCallId() {
        if (mAospImsCallSessionProxy == null || mAospImsCallSessionProxy.getServiceImpl() == null) {
            logWithCallId("getCallId() : mCallSessionImpl is null", ImsCallLogLevel.ERROR);
            return "";
        }

        try {
            return mAospImsCallSessionProxy.getServiceImpl().getCallId();
        } catch (RemoteException e) {
            logWithCallId("getCallId() : RemoteException getCallId()", ImsCallLogLevel.ERROR);
            return "";
        }
    }

    public ImsCallProfile getCallProfile(){
        if (mAospImsCallSessionProxy == null || mAospImsCallSessionProxy.getServiceImpl() == null) {
            logWithCallId("getCallProfile() : mCallSessionImpl is null", ImsCallLogLevel.ERROR);
            return null;
        }

        try {
            return mAospImsCallSessionProxy.getServiceImpl().getCallProfile();
        } catch (RemoteException e) {
            logWithCallId("getCallProfile() : RemoteException getCallProfile()",
                    ImsCallLogLevel.ERROR);
            return null;
        }

    }

    public void setListener(IMtkImsCallSessionListener listener) {
        mMtkListener = listener;
    }

    public IImsCallSession getIImsCallSession() {
        if (mAospImsCallSessionProxy == null) {
            logWithCallId("getIImsCallSession() : mAospImsCallSessionProxy is null",
                    ImsCallLogLevel.ERROR);
            return null;
        }
        return mAospImsCallSessionProxy.getServiceImpl();
    }

    void setIImsCallSession(IImsCallSession iSession) {
        if (mAospImsCallSessionProxy != null) {
            mAospImsCallSessionProxy.setServiceImpl(iSession);
        }
    }

    public boolean isIncomingCallMultiparty() {
        if (mAospImsCallSessionProxy == null || mAospImsCallSessionProxy.getServiceImpl() == null) {
            logWithCallId("isIncomingCallMultiparty() : mCallSessionImpl is null",
                    ImsCallLogLevel.ERROR);
            return false;
        }
        return mAospImsCallSessionProxy.isIncomingCallMultiparty();
    }

    public void approveEccRedial(boolean isAprroved) {
        if (mAospImsCallSessionProxy == null || mAospImsCallSessionProxy.getServiceImpl() == null) {
            logWithCallId("approveEccRedial() : mCallSessionImpl is null",
                    ImsCallLogLevel.ERROR);
            return;
        }
        mAospImsCallSessionProxy.approveEccRedial(isAprroved);
    }

    public void explicitCallTransfer() {
        if (mAospImsCallSessionProxy == null) {
            logWithCallId("explicitCallTransfer() : mAospImsCallSessionProxy is null",
                    ImsCallLogLevel.ERROR);
            return;
        }
        mAospImsCallSessionProxy.explicitCallTransfer();
    }

    public void unattendedCallTransfer(String number, int type) {
        if (mAospImsCallSessionProxy == null) {
            logWithCallId("unattendedCallTransfer() : mAospImsCallSessionProxy is null",
                    ImsCallLogLevel.ERROR);
            return;
        }
        mAospImsCallSessionProxy.unattendedCallTransfer(number, type);
    }

    public void deviceSwitch(String number, String deviceId) {
        if (mAospImsCallSessionProxy == null) {
            logWithCallId("deviceSwitch() : mAospImsCallSessionProxy is null", ImsCallLogLevel.ERROR);
            return;
        }
        mAospImsCallSessionProxy.deviceSwitch(number, deviceId);
    }

    public void cancelDeviceSwitch() {
        if (mAospImsCallSessionProxy == null) {
            logWithCallId("cancelDeviceSwitch() : mAospImsCallSessionProxy is null", ImsCallLogLevel.ERROR);
            return;
        }
        mAospImsCallSessionProxy.cancelDeviceSwitch();
    }

    void notifyTransferred() {
        if (mMtkListener == null) {
            logWithCallId("notifyTransferred() : mMtkListener is null", ImsCallLogLevel.ERROR);
            return;
        }
        try {
            mMtkListener.callSessionTransferred(mServiceImpl);
        } catch (RemoteException e) {
            logWithCallId("notifyTransferred() : RemoteException callSessionTransferred()",
                    ImsCallLogLevel.ERROR);
        }
    }

    void notifyTransferFailed(ImsReasonInfo reasonInfo) {
        if (mMtkListener == null) {
            logWithCallId("notifyTransferFailed() : mMtkListener is null", ImsCallLogLevel.ERROR);
            return;
        }
        try {
            mMtkListener.callSessionTransferFailed(mServiceImpl, reasonInfo);
        } catch (RemoteException e) {
            logWithCallId("notifyTransferFailed() : RemoteException callSessionTransferFailed()",
                    ImsCallLogLevel.ERROR);
        }
    }

    public void notifyDeviceSwitched() {
        if (mMtkListener == null) {
            logWithCallId("notifyDeviceSwitched() : mMtkListener is null", ImsCallLogLevel.ERROR);
            return;
        }
        try {
            mMtkListener.callSessionDeviceSwitched(mServiceImpl);
        } catch (RemoteException e) {
            logWithCallId("notifyDeviceSwitched() : RemoteException notifyDeviceSwitched()",
                    ImsCallLogLevel.ERROR);
        }
    }

    public void notifyDeviceSwitchFailed(ImsReasonInfo reasonInfo) {

        Rlog.d(LOG_TAG, "notifyTransferFailed()");
        if (mMtkListener == null) {
            logWithCallId("notifyDeviceSwitchFailed() : mMtkListener is null",
                    ImsCallLogLevel.ERROR);
            return;
        }
        try {
            mMtkListener.callSessionDeviceSwitchFailed(mServiceImpl, reasonInfo);
        } catch (RemoteException e) {
            logWithCallId("notifyDeviceSwitchFailed() : RemoteException notifyDeviceSwitchFailed()",
                    ImsCallLogLevel.ERROR);
        }
    }

    public void notifyTextCapabilityChanged(int localCapability, int remoteCapability,
            int localTextStatus, int realRemoteCapability) {
        if (mMtkListener == null) {
            Rlog.d(LOG_TAG, "notifyTextCapabilityChanged() listener is null");
            return;
        }
        try {
            mMtkListener.callSessionTextCapabilityChanged(mServiceImpl,
                    localCapability, remoteCapability,
                    localTextStatus, realRemoteCapability);
        } catch (RemoteException e) {
            Rlog.e(LOG_TAG, "RemoteException callSessionTextCapabilityChanged()");
        }
    }

    public void notifyRttECCRedialEvent() {
        if (mMtkListener == null) {
            Rlog.d(LOG_TAG, "notifyRttECCRedialEvent() listener is null");
            return;
        }
        try {
            mMtkListener.callSessionRttEventReceived(mServiceImpl, 137);
        } catch (RemoteException e) {
            Rlog.e(LOG_TAG, "RemoteException callSessionRttEventReceived()");
        }
    }

    public void resume() {
        if (mAospImsCallSessionProxy == null) {
            logWithCallId("resume() : mAospImsCallSessionProxy is null", ImsCallLogLevel.ERROR);
            return;
        }

        mAospImsCallSessionProxy.resume(null);
    }

    public void callTerminated() {
        if (mAospImsCallSessionProxy == null) {
            logWithCallId("callTerminated() : mAospImsCallSessionProxy is null",
                    ImsCallLogLevel.ERROR);
            return;
        }

        mAospImsCallSessionProxy.callTerminated();
    }

    public void setImsCallMode(int mode) {
        mAospImsCallSessionProxy.setImsCallMode(mode);
    }

    public void removeLastParticipant() {
        mAospImsCallSessionProxy.removeLastParticipant();
    }

    public String getHeaderCallId() {
        return mAospImsCallSessionProxy.getHeaderCallId();
    }

    void notifyCallSessionMergeStarted(IMtkImsCallSession mtkConfSession,
                                       ImsCallProfile imsCallProfile) {
        if (mMtkListener == null) {
            logWithCallId("notifyCallSessionMergeStarted() : mMtkListener is null",
                    ImsCallLogLevel.ERROR);
            return;
        }
        try {
            mMtkListener.callSessionMergeStarted(mServiceImpl, mtkConfSession, imsCallProfile);
        } catch (RemoteException e) {
            logWithCallId("notifyCallSessionMergeStarted() : RemoteException when MTK session merged started",
                    ImsCallLogLevel.ERROR);
        }
    }

    void notifyCallSessionMergeComplete(IMtkImsCallSession mtkConfSession) {
        if (mMtkListener == null) {
            logWithCallId("notifyCallSessionMergeComplete() : mMtkListener is null",
                    ImsCallLogLevel.ERROR);
            return;
        }
        try {
            mMtkListener.callSessionMergeComplete(mtkConfSession);
        } catch (RemoteException e) {
            logWithCallId("notifyCallSessionMergeComplete() : RemoteException when MTK session merged started",
                    ImsCallLogLevel.ERROR);
        }
    }

    void notifyRedialEcc(boolean isNeedUserConfirm) {
        if (mMtkListener == null) {
            logWithCallId("notifyRedialEcc() : mMtkListener is null", ImsCallLogLevel.ERROR);
            return;
        }
        try {
            mMtkListener.callSessionRedialEcc(mServiceImpl, isNeedUserConfirm);
        } catch (RemoteException e) {
            logWithCallId("notifyRedialEcc() : RemoteException callSessionRedialEcc()",
                    ImsCallLogLevel.ERROR);
        }
    }

    public void notifyCallSessionRinging(ImsCallProfile imsCallProfile) {
        if (mMtkListener == null) {
            logWithCallId("notifyCallSessionRinging() : mMtkListener is null", ImsCallLogLevel.DEBUG);
            return;
        }
        try {
            mMtkListener.callSessionRinging(mServiceImpl, imsCallProfile);
        } catch (RemoteException e) {
            logWithCallId("notifyCallSessionRinging() : RemoteException notifyCallSessionRinging()", ImsCallLogLevel.ERROR);
        }
    }

    public void notifyCallSessionCalling() {
        if (mMtkListener == null) {
            logWithCallId("notifyCallSessionCalling() : mMtkListener is null", ImsCallLogLevel.DEBUG);
            return;
        }
        try {
            mMtkListener.callSessionCalling(mServiceImpl);
        } catch (RemoteException e) {
            logWithCallId("notifyCallSessionCalling() : RemoteException notifyCallSessionCalling()", ImsCallLogLevel.ERROR);
        }
    }

    public void notifyCallSessionBusy() {
        if (mMtkListener == null) {
            logWithCallId("notifyCallSessionBusy() : mMtkListener is null", ImsCallLogLevel.DEBUG);
            return;
        }
        try {
            mMtkListener.callSessionBusy(mServiceImpl);
        } catch (RemoteException e) {
            logWithCallId("notifyCallSessionBusy() : RemoteException notifyCallSessionBusy()", ImsCallLogLevel.ERROR);
        }
    }

    ImsCallSessionProxy.ConferenceEventListener getConfEvtListener() {
        if (mAospImsCallSessionProxy == null) {
            logWithCallId("ConferenceEventListener() : mAospImsCallSessionProxy is null",
                    ImsCallLogLevel.ERROR);
            return null;
        }
        return mAospImsCallSessionProxy.getConfEvtListener();
    }

    void terminate(int reason) {
        if (mAospImsCallSessionProxy == null) {
            logWithCallId("terminate() : mAospImsCallSessionProxy is null", ImsCallLogLevel.ERROR);
            return;
        }
        mAospImsCallSessionProxy.terminate(reason);
    }

    public IMtkImsCallSession getServiceImpl() {
        return mServiceImpl;
    }

    public void setServiceImpl(IMtkImsCallSession serviceImpl) {
        mServiceImpl = serviceImpl;
    }

    public ImsCallSessionProxy getAospCallSessionProxy() {
        return mAospImsCallSessionProxy;
    }

    public void setAospCallSessionProxy(ImsCallSessionProxy callSessionProxy) {
        mAospImsCallSessionProxy = callSessionProxy;
    }

    private void logWithCallId(String msg, int lvl) {

        if (!DBG) return;

        if (mAospImsCallSessionProxy == null) {
            Rlog.d(LOG_TAG, "logWithCallId with mAospImsCallSessionProxy = null");
            return;
        }

        String mCallId = mAospImsCallSessionProxy.getCallId();


        if (ImsCallLogLevel.VERBOSE == lvl) {
            Rlog.v(LOG_TAG, "[callId = " + mCallId + "] " + msg);
        } else if (ImsCallLogLevel.DEBUG == lvl) {
            Rlog.d(LOG_TAG, "[callId = " + mCallId + "] " + msg);
        } else if (ImsCallLogLevel.INFO== lvl) {
            Rlog.i(LOG_TAG, "[callId = " + mCallId + "] " + msg);
        } else if (ImsCallLogLevel.WARNNING == lvl) {
            Rlog.w(LOG_TAG, "[callId = " + mCallId + "] " + msg);
        } else if (ImsCallLogLevel.ERROR == lvl) {
            Rlog.e(LOG_TAG, "[callId = " + mCallId + "] " + msg);
        } else {
            Rlog.d(LOG_TAG, "[callId = " + mCallId + "] " + msg);
        }
    }
}
