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
import android.os.Build;
import android.os.SystemProperties;
import android.telephony.ims.ImsCallProfile;
import android.telephony.ims.aidl.IImsCallSessionListener;
import android.telephony.Rlog;

import com.android.ims.internal.IImsCallSession;
import com.android.ims.internal.IImsUt;

import com.mediatek.gba.NafSessionKey;
import com.mediatek.ims.internal.IMtkImsCallSession;
import com.mediatek.ims.internal.IMtkImsService;
import com.mediatek.ims.internal.IMtkImsConfig;
import com.mediatek.ims.internal.IMtkImsUt;
import com.mediatek.ims.internal.ImsVTProviderUtil;
import com.mediatek.ims.internal.IMtkImsRegistrationListener;

import com.android.ims.internal.IImsEcbm;

// SMS-START
import android.telephony.ims.aidl.IImsSmsListener;
// SMS-END
import com.android.ims.internal.IImsRegistrationListener;
import com.android.ims.ImsServiceClass;
import android.telephony.ims.feature.CapabilityChangeRequest;


import java.util.HashMap;
import java.util.Map;

public class MtkImsService extends IMtkImsService.Stub {
    private static final String LOG_TAG = "MtkImsService";
    private static final boolean DBG = true;
    private static final boolean ENGLOAD = "eng".equals(Build.TYPE);

    private ImsService mImsService = null;

    public MtkImsService(Context context, ImsService imsService) {
        mImsService = imsService;
        log("init");
    }

    ///M: MTK add new interface @{
    /**
     * call interface for allowing/refusing the incoming call indication send to App.
     *@hide
     */
    @Override
    public void setCallIndication(int phoneId, String callId, String callNum, int seqNum,
            String toNumber, boolean isAllow, int cause) {
        mImsService.onSetCallIndication(
                phoneId, callId, callNum, seqNum, toNumber, isAllow, cause);
    }

    ///M: MTK add new interface @{
    /**
     * Used to Update Ims state change.
     *@hide
     */
    @Override
    public void UpdateImsState(int phoneId) {
        mImsService.onUpdateImsSate(phoneId);
    }

    /**
     * Use to query ims enable/disable status.
     *@hide
     */
    @Override
    public int getImsState(int phoneId) {
        return mImsService.getImsState(phoneId);
    }

    /**
     * Query ims reg ext info.
     *@hide
     */
    @Override
    public int getImsRegUriType(int phoneId) {
        return mImsService.getImsRegUriType(phoneId);
    }

    /**
     * Use to hang up all calls.
     *@hide
     */
    @Override
    public void hangupAllCall(int phoneId) {
        mImsService.onHangupAllCall(phoneId);
    }

    /**
     * Used to deregister IMS.
     *@hide
     */
    @Override
    public void deregisterIms(int phoneId) {
        mImsService.deregisterIms(phoneId);
    }

    /**
     * Used to notify radio state change.
     *@hide
     */
    @Override
    public void updateRadioState(int radioState, int phoneId) {
        mImsService.updateRadioState(radioState, phoneId);
    }
    /// @}

    public IMtkImsConfig getConfigInterfaceEx(int phoneId) {
        mImsService.bindAndRegisterWifiOffloadService();

        return mImsService.getImsConfigManager().getEx(phoneId);
    }

    @Override
    public IMtkImsCallSession createMtkCallSession(int phoneId, ImsCallProfile profile,
                                                IImsCallSessionListener listener,
                                                IImsCallSession aospCallSessionImpl) {
        return mImsService.onCreateMtkCallSession(phoneId, profile, listener, aospCallSessionImpl);
    }

    @Override
    public IMtkImsCallSession getPendingMtkCallSession(int phoneId, String callId) {
        return mImsService.onGetPendingMtkCallSession(phoneId, callId);
    }

    @Override
    public IMtkImsUt getMtkUtInterface(int phoneId) {
        return mImsService.onGetMtkUtInterface(phoneId);
    }

    @Override
    public NafSessionKey runGbaAuthentication(String nafFqdn,
            byte[] nafSecureProtocolId, boolean forceRun, int netId, int phoneId) {
        return mImsService.onRunGbaAuthentication(nafFqdn, nafSecureProtocolId,
                                                  forceRun, netId, phoneId);
    }

    @Override
    public int getModemMultiImsCount() {
        return mImsService.getModemMultiImsCount();
    }

    @Override
    public int getCurrentCallCount(int phoneId) {
        return mImsService.getCurrentCallCount(phoneId);
    }

    @Override
    public int[] getImsNetworkState(int capability) {
        return mImsService.getImsNetworkState(capability);
    }

    public  boolean isCameraAvailable() {
        return ImsVTProviderUtil.isCameraAvailable();
    }

    private void log(String s) {
        if (DBG) {
            Rlog.d(LOG_TAG, s);
        }
    }

    private void englog(String s) {
        if (ENGLOAD) {
            log(s);
        }
    }

    private void logw(String s) {
        Rlog.w(LOG_TAG, s);
    }

    private void loge(String s) {
        Rlog.e(LOG_TAG, s);
    }

    // SMS-START
    @Override
    public void addImsSmsListener(int phoneId, IImsSmsListener listener) {
        mImsService.onAddImsSmsListener(phoneId, listener);
    }

    @Override
    public void sendSms(int phoneId, int token, int messageRef, String format, String smsc,
            boolean isRetry, byte[] pdu) {
        mImsService.sendSms(phoneId, token, messageRef, format, smsc, isRetry, pdu);
    }
    // SMS-END

    public void registerProprietaryImsListener(int phoneId,
            IImsRegistrationListener listener, IMtkImsRegistrationListener mtklistener,
            boolean notifyOnly) {
        mImsService.onAddRegistrationListener(phoneId, ImsServiceClass.MMTEL, listener,
                mtklistener, notifyOnly);
    }

    // Client API
    public void setMTRedirect(int phoneId, boolean enable) {
        mImsService.setMTRedirect(phoneId, enable);
    }

    public void fallBackAospMTFlow(int phoneId) {
        mImsService.fallBackAospMTFlow(phoneId);
    }

    public void setSipHeader(int phoneId, Map extraHeaders, String fromUri) {
        mImsService.setSipHeader(phoneId, (HashMap<String, String>) extraHeaders, fromUri);
    }
    //

    public void changeEnabledCapabilities(int phoneId, CapabilityChangeRequest request) {
        mImsService.changeEnabledCapabilities(phoneId, request);
    }

    public void setImsPreCallInfo(int phoneId, int mode, String address, String fromUri,
            Map extraHeaders, String[] location) {
        mImsService.setImsPreCallInfo(phoneId, mode, address, fromUri,
                (HashMap<String, String>) extraHeaders, location);
    }
}
