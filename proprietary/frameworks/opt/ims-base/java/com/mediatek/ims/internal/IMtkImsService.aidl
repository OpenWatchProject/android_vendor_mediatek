/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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
/*
 * Copyright (c) 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.ims.internal;

import android.telephony.ims.ImsCallProfile;
import android.telephony.ims.aidl.IImsCallSessionListener;

import com.android.ims.internal.IImsUt;
import com.android.ims.internal.IImsCallSession;

import com.mediatek.gba.NafSessionKey;
import com.mediatek.ims.internal.IMtkImsConfig;
import com.mediatek.ims.internal.IMtkImsCallSession;
import com.mediatek.ims.internal.IMtkImsUt;

import com.android.ims.internal.IImsEcbm;
// SMS-START
import android.telephony.ims.aidl.IImsSmsListener;
// SMS-END
import com.android.ims.internal.IImsRegistrationListener;
import com.mediatek.ims.internal.IMtkImsRegistrationListener;
import android.telephony.ims.feature.CapabilityChangeRequest;

/**
 * {@hide}
 */
interface IMtkImsService {
    /**
     * call interface for allowing/refusing the incoming call indication send to App.
     */
    void setCallIndication(int phoneId, String callId, String callNum, int seqNum,
            String toNumber, boolean isAllow, int cause);

    IMtkImsCallSession createMtkCallSession(int phoneId, in ImsCallProfile profile,
            in IImsCallSessionListener listener, in IImsCallSession aospCallSessionImpl);

    IMtkImsCallSession getPendingMtkCallSession(int phoneId, String callId);

    /**
     * Use to query ims enable/disable status.
     */
    int getImsState(int phoneId);

    /**
     * Use to Query ims reg ext info.
     */
    int getImsRegUriType(int phoneId);

    /**
     * Use to hang up all calls.
     */
    void hangupAllCall(int phoneId);

    ///M: Used to deregister IMS @ {
    /**
     * Used to deregister IMS.
     */
    void deregisterIms(int phoneId);
    /// @}

    ///M: Used to notify radio state change @ {
    /**
     * Used to notify radio state change.
     */
    void updateRadioState(int radioState, int phoneId);
    /// @}

    ///M: Used to Update Ims state change @ {
    /**
     * Used to Update Ims state change.
     */
    void UpdateImsState(int phoneId);
    /// @}

    /**
     * MTK Config interface to get/set IMS service/capability parameters.
     */
    IMtkImsConfig getConfigInterfaceEx(int phoneId);

    /**
     * MTK UT interface to get/set call forwarding with timeslot.
     */
    IMtkImsUt getMtkUtInterface(int serviceId);

    ///M: Used to trigger GBA in native SS solution @ {
    /**
     * Used to trigger GBA in native SS solution.
     */
    NafSessionKey runGbaAuthentication(String nafFqdn,
            in byte[] nafSecureProtocolId, boolean forceRun, int netId, int phoneId);
    /// @}

    /**
     * Used to get modem multiple IMS count.
     */
    int getModemMultiImsCount();

    /*
     * Provide IMS current call count.
     */
    int getCurrentCallCount(int phoneId);

    /*
     * Provide modem IMS/Emergency pdn current state.
     * @param capability PDN capability used to query specific PDN state and the type of PDN
     *                   capability is defined in NetworkCapabilities.
     * @return int[]     return PDN state of each phone and the type of PDN state is defined
     *                   in NetworkInfo.State.
     */
    int[] getImsNetworkState(int capability);

    /**
     * Register the listener from ImsSmsDispatcher
     * @param phoneId the listener from the phone
     * @param listener ImsSmsDispatcher's listener
     */
    void addImsSmsListener(int phoneId, IImsSmsListener listener);

    /**
     * Send SMS over IMS
     * @param phoneId The sender phone identify
     * @param token The request's token
     * @param messageRef message reference. default is 0
     * @param format the format of the short message
     * @param smsc SMS center
     * @param isRetry Retry the SMS or not
     * @param pdu the SMS PDU
     */
    void sendSms(int phoneId, int token, int messageRef, String format, String smsc,
            boolean isRetry, in byte[] pdu);

    /**
     * Register the listener for the proprietary capability changed
     * @param phoneId the listener from the phone
     * @param listener the proprietary listener
     */
    void registerProprietaryImsListener(int phoneId, IImsRegistrationListener listener,
            IMtkImsRegistrationListener mtklistener, boolean notifyOnly);

    /**
     * Used to get camera support or not.
     */
    boolean isCameraAvailable();

    void setMTRedirect(int phoneId, boolean enable);
    void fallBackAospMTFlow(int phoneId);
    void setSipHeader(int phoneId, in Map extraHeaders, String fromUri);
    void changeEnabledCapabilities(int phoneId, in CapabilityChangeRequest request);

    /**
     * Set pre-call info for dialing IMS call.
     * @param phoneId to indicate which phone
     * @param mode to indicate the purpose, 3 = RCS enriched calling
     * @param address prepare to dialing to
     * @param fromUri set the "from:" header in the SIP header
     * @param extraHeaders additional header for this call
     * @param location including latitude, longtitude and accuracy
     */
    void setImsPreCallInfo(int phoneId, int mode, String address, String fromUri,
            in Map extraHeaders, in String[] location);
}
