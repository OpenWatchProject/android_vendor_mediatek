/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2014. All rights reserved.
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

package com.mediatek.ims.ril;

import android.os.Handler;
import android.os.Message;
import android.telephony.ims.ImsCallProfile;

import com.android.internal.telephony.UUSInfo;

import com.mediatek.ims.ImsCallInfo;

import java.util.ArrayList;

public interface ImsCommandsInterface {
    enum RadioState {
        RADIO_OFF,         /* Radio explicitly powered off (eg CFUN=0) */
        RADIO_UNAVAILABLE, /* Radio unavailable (eg, resetting or not booted) */
        RADIO_ON;          /* Radio is on */

        public boolean isOn() /* and available...*/ {
            return this == RADIO_ON;
        }

        public boolean isAvailable() {
            return this != RADIO_UNAVAILABLE;
        }
    }

    /**
     * Get WWOP Extension APIs
     * @return Instance of ImsOpCommandInterface
     */
    OpImsCommandsInterface getOpCommandsInterface();

    // AOSP RILJ Interfaces below

    /**
     * Dial a voice call
     * @param address
     * @param clirMode
     * @param uusInfo
     * @param phoneId
     * @param result
     */
    void dial(String address, int clirMode, UUSInfo uusInfo,
            Message result);

    /**
     * Dial a voice call (AOSP Command)
     * @param address
     * @param clirMode
     * @param phoneId
     * @param result
     */
    void dial(String address, int clirMode, Message result);

    /**
     * Pull Call
     * @param target
     * @param isVideoCall
     * @param result
     */
    void pullCall(String target, boolean isVideoCall, Message result);

    /**
     * Send DTML (AOSP RIL Command)
     * @param c
     * @param result
     */
    void sendDtmf(char c, Message result);

    /**
     *
     * @param c
     * @param result
     */
    void startDtmf(char c, Message result);

    /**
     * STOP DTML (AOSP RIL Command)
     * @param result
     */
    void stopDtmf(Message result);


    // MTK Proprietary IMS Interfaces below

    /**
     * Get Provision Value
     * @param phoneId
     * @param provisionStr
     * @param response
     */
    void getProvisionValue(String provisionStr, Message response);

    /**
     * Set Provision Value
     * @param phoneId
     * @param provisionStr
     * @param provisionValue
     * @param response
     */
    void setProvisionValue(String provisionStr,
                           String provisionValue, Message response);

    /**
     * Set IMS Features Enable/Disable
     * @param params
     * @param response
     */
    void setImsCfg(int[] params, Message response);

    /**
     * Turn ON Video on IMS
     * @param phoneId
     * @param response
     */
    void turnOnImsVideo(Message response);

    /**
     * Turn OFF Video over IMS
     * @param phoneId
     * @param response
     */
    void turnOffImsVideo(Message response);

    /**
     * Turn OFF Voice over Wi-Fi
     * @param phoneId
     * @param response
     */
    void turnOnViwifi(Message response);

    /**
     * Turn OFF Voice over Wi-Fi
     * @param phoneId
     * @param response
     */
    void turnOffViwifi(Message response);

    /**
     * Turn on Rcs Ua
     * @param response
     */
    void turnOnRcsUa(Message response);

    /**
     * Turn off Rcs ua
     * @param response
     */
    void turnOffRcsUa(Message response);

    /**
     * Turn ON Video over LTE
     * @param response
     */
    void turnOnVilte(Message response);

    /**
     * Turn OFF Video over LTE
     * @param response
     */
    void turnOffVilte(Message response);

    /**
     * Turn ON VoLTE
     * @param response
     */
    void turnOnVolte(Message response);

    /**
     * Turn OFF VoLTE
     * @param response
     */
    void turnOffVolte(Message response);

    /**
     * Turn OFF WFC
     * @param response
     */
    void turnOffWfc(Message response);

    /**
     * Turn ON WFC
     * @param response
     */
    void turnOnWfc(Message response);

    /**
     * Turn ON IMS
     * @param response
     */
    void turnOffIms(Message response);

    /**
     * Turn OFF IMS
     * @param response
     */
    void turnOnIms(Message response);

    /**
     * De-register IMS
     * @param response
     */
    void deregisterIms(Message response);

    /**
     * Set Mute
     * @param enableMute
     * @param response
     */
    void setMute(boolean enableMute, Message response);

    /**
     * Start Call
     * @param callee
     * @param callProfile
     * @param clirMode
     * @param isEmergency
     * @param isVideoCall
     * @param response
     */
    void start(String callee, ImsCallProfile callProfile, int clirMode, boolean isEmergency,
               boolean isVideoCall, Message response);

    /**
     * Start Conference
     * @param participants
     * @param clirMode
     * @param isVideoCall
     * @param response
     */
    void startConference(String[] participants, int clirMode,
                         boolean isVideoCall, Message response);


    /**
     * Explicit Call Transfer
     * @param response
     */
    void explicitCallTransfer(Message response);

    /**
     * Unattended Call Transfer
     * @param number
     * @param type
     * @param result
     */
    void unattendedCallTransfer(String number, int type,
                                Message response);

    /**
     * Hangup Call
     * @param callId
     * @param response
     */
    void hangup(int callId, Message response);

    /**
     * Hangup Call
     * @param callId
     * @param response
     */
    void hangup(int callId, int reason, Message response);

    /**
     * Reject a call
     * @param callId
     * @param reason
     */
    @Deprecated
    void reject(int callId, int reason);

    /**
     * Terminal Call
     * @param callId
     * @param reason
     */
    @Deprecated
    void terminate(int callId, int reason);

    /**
     * Conference call
     * @param response
     */
    void conference(Message response);

    /**
     * Resume Call
     * @param callId
     * @param response
     */
    void resume(int callId, Message response);

    /**
     * Release all call
     * @param response
     */
    void hangupAllCall(Message response);

    /**
     * Get the Last Fail Cause
     * @param response
     */
    void getLastCallFailCause(Message response);

    /**
     * Remove Conference Participants
     * @param confCallId
     * @param participant
     * @param response
     */
    void removeParticipants(int confCallId, String participant,
                            Message response);

    /**
     * Invite Conference Participants
     * @param confCallId
     * @param participant
     * @param response
     */
    void inviteParticipants(int confCallId, String participant,
                            Message response);

    /**
     * Invite Participants by CallId
     * @param confCallId
     * @param callInfo
     * @param response
     */
    void inviteParticipantsByCallId(int confCallId, ImsCallInfo callInfo,
            Message response);


    /**
     * Swap Call
     * @param result
     */
    void swap(Message result);

    /**
     * Send USSI
     * @param ussiString
     * @param response
     */
    void sendUSSI(String ussiString, Message response);

    /**
     * Cancel Send Ussi
     * @param response
     */
    void cancelPendingUssi(Message response);

    /**
     * Dial a video call
     * @param address
     * @param clirMode
     * @param uusInfo
     * @param response
     */
    void vtDial(String address, int clirMode, UUSInfo uusInfo,
                Message response);

    /**
     * Dial a conference call.
     * @param participants participants' dailing number.
     * @param clirMode indication to present the dialing number or not.
     * @param isVideoCall indicate the call is belong to video call or voice call.
     * @param result the command result.
     */
    void conferenceDial(String[] participants, int clirMode,
                        boolean isVideoCall, Message result);

    /**
     * Hold a call
     * @param callId
     * @param response
     */
    void hold(int callId, Message response);

    /**
     * Set call indication
     * @param mode
     * @param callId
     * @param seqNum
     */
    @Deprecated
    void setCallIndication(int mode, int callId, int seqNum, int cause);

    /**
     * Set call indication
     * @param mode
     * @param callId
     * @param seqNum
     * @param response
     */
    void setCallIndication(int mode, int callId, int seqNum, int cause,
                           Message response);

    /**
     * De-register IMS with cause
     * @param phoneId
     * @param cause
     * @param response
     */
    void deregisterImsWithCause(int cause, Message response);

    /**
     * Accept Video Call
     * @param videoMode
     * @param callId
     * @param response
     */
    void acceptVideoCall(int videoMode, int callId,
                         Message response);

    /**
     * Approve ECC Redial or not
     * @param approve
     * @param callId
     * @param response
     */
    void approveEccRedial(int approve, int callId, Message response);

    /**
     * Accept call
     * @param response
     */
    void accept(Message response);

    /**
     * Send a ACK for 'BearActivationDone/BearDeactivationDone'
     * @param aid
     * @param action
     * @param status
     * @param response
     */
    void responseBearerStateConfirm(int aid, int action, int status,
                                      Message response);

    /**
     * Set ims bearer notification
     * @param enable
     * @param response
     */
    public void setImsBearerNotification(int enable, Message response);

    /**
     * Force Release a call
     * @param callId
     * @param response
     */
    void forceHangup(int callId, Message response);

    /**
     * Force Release a call
     * @param callId
     * @param reason
     */
    void forceHangup(int callId, int reason);

    /**
     * Set IMS RTP Info
     * @param pdnId PDN Id
     * @param networkId Network Id
     * @param timer Timer
     * @param response Response Object
     */
    void setImsRtpInfo(int pdnId, int networkId, int timer, Message response);

    /**
     * Request Exit Emergency Callback Mode
     * @param response
     */
    void requestExitEmergencyCallbackMode(Message response);


    /**
     * Set IMS Registration Report
     * @param response
     */
    void setImsRegistrationReport(Message response);

    /**
     * Set WFC Profile Info
     * @param wfcPreference
     * @param response
     */
    void sendWfcProfileInfo(int wfcPreference, Message response);

    /**
     * Set a registrant for 'OnUSSI'
     * @param h
     */
    void unSetOnUSSI(Handler h);

    /**
     * Set a registrant for 'OnUSSI'
     * @param h
     * @param what
     * @param obj
     */
    void setOnUSSI(Handler h, int what, Object obj);

    /* MTK SS Feature : Start */

    void queryFacilityLock(String facility, String password, int serviceClass, Message response);

    void queryFacilityLockForApp(String facility, String password, int serviceClass, String appId,
            Message response);

    void setFacilityLock (String facility, boolean lockState, String password,
            int serviceClass, Message response);

    void setFacilityLockForApp(String facility, boolean lockState, String password,
            int serviceClass, String appId, Message response);

    void setCallForward(int action, int cfReason, int serviceClass,
            String number, int timeSeconds, Message response);

    void queryCallForwardStatus(int cfReason, int serviceClass, String number, Message response);

    void queryCallWaiting(int serviceClass, Message response);

    void setCallWaiting(boolean enable, int serviceClass, Message response);

    void getCLIR(Message result);

    void setCLIR(int clirMode, Message result);

    void queryCLIP(Message response);

    void setCLIP(int enable, Message result);

    void getCOLR(Message result);

    void setCOLR(int presentation, Message result);

    void getCOLP(Message result);

    void setCOLP(int enable, Message result);

    void queryCallForwardInTimeSlotStatus(int cfReason, int serviceClass, Message result);

    void setCallForwardInTimeSlot(int action, int cfReason, int serviceClass,
            String number, int timeSeconds, long[] timeSlot, Message result);

    void getXcapStatus(Message result);

    void resetSuppServ(Message result);

    void setupXcapUserAgentString(String userAgent, Message result);

    /**
     * run Gba Authentication
     * @param nafFqdn NAF FQDN String
     * @param nafSecureProtocolId nafSecureProtocolId Value
     * @param forceRun boolean
     * @param netId Integer
     * @param phoneId Integer
     * @param result Response Data Parcel
     */
    void runGbaAuthentication(String nafFqdn, String nafSecureProtocolId,
            boolean forceRun, int netId, Message response);

    /* MTK SS Feature : End */

    /**
     * Set a registrant for 'IncomingCallIndicatio'
     * @param h
     * @param what
     * @param obj
     */
    void setOnIncomingCallIndication(Handler h, int what, Object obj);

    /**
     * Unset a registrant for 'IncomingCallIndicatio'
     * @param h
     */
    void unsetOnIncomingCallIndication(Handler h);

    /**
     * Set a registrant for 'CallAdditionalInfo'
     * @param h
     * @param what
     * @param obj
     */
    public void registerForCallAdditionalInfo(Handler h, int what, Object obj);

    /**
     * Unset a registrant for 'CallAdditionalInfo'
     * @param h
     */
    public void unregisterForCallAdditionalInfo(Handler h);

    /**
     * Register a registrant for 'CallRatIndication'
     * @param h
     * @param what
     * @param obj
     */
    public void registerForCallRatIndication(Handler h, int what, Object obj);

    /**
     * Unregister a registrant for 'CallRatIndication'
     * @param h
     */
    public void unregisterForCallRatIndication(Handler h);

    /**
     * Register a registrant for 'ImsEventPackage'
     * @param h
     * @param what
     * @param obj
     */
    void registerForImsEventPackage(Handler h, int what, Object obj);

    /**
     * Unregister a registrant for 'ImsEventPackage'
     * @param h
     */
    void unregisterForImsEventPackage(Handler h);

    /**
     * Register a registrant for 'EconfResult'
     * @param h
     * @param what
     * @param obj
     */
    void registerForEconfResult(Handler h, int what, Object obj);

    /**
     * Unregister a registrant for 'EconfResult'
     * @param h
     */
    void unregisterForEconfResult(Handler h);

    /**
     * Register a registrant for 'CallInfo'
     * @param h
     * @param what
     * @param obj
     */
    void registerForCallInfo(Handler h, int what, Object obj);

    /**
     * Unregister a registrant for 'CallInfo'
     * @param h
     */
    void unregisterForCallInfo(Handler h);

    /**
     * Register a registrant for 'ImsDisableStart'
     * @param h
     * @param what
     * @param obj
     */
    void registerForImsDisableStart(Handler h, int what, Object obj);

    /**
     * Register a registrant for 'ImsEnableStart'
     * @param h
     * @param what
     * @param obj
     */
    void registerForImsEnableStart(Handler h, int what, Object obj);

    /**
     * Unregister a registrant for 'ImsEnableStart'
     * @param h
     */
    void unregisterForImsEnableStart(Handler h);

    /**
     * Register a registrant for 'XuiInfo'
     * @param h
     * @param what
     * @param obj
     */
    void registerForXuiInfo(Handler h, int what, Object obj);

    /**
     * Unregister a registrant for 'XuiInfo'
     * @param h
     */
    void unregisterForXuiInfo(Handler h);

     /**
      * Register a registrant for 'VolteSubscription'
      * @param h
      * @param what
      * @param obj
      */
    void registerForVolteSubscription(Handler h, int what, Object obj);

    /**
     * Unregister a registrant for 'VolteSubscription'
     * @param h
     */
    void unregisterForVolteSubscription(Handler h);

    /**
     * Set a registrant for 'SuppServiceNotification'
     * @param h
     * @param what
     * @param obj
     */
    void setOnSuppServiceNotification(Handler h, int what, Object obj);

    /**
     * Unset a registrant for 'SuppServiceNotification'
     * @param h
     */
    void unSetOnSuppServiceNotification(Handler h);

    /**
     * Register a registrant for 'CallProgressIndicator'
     * @param h
     * @param what
     * @param obj
     */
    void registerForCallProgressIndicator(Handler h, int what, Object obj);

    /**
     * Unregister a registrant for 'CallProgressIndicator'
     * @param h
     */
    void unregisterForCallProgressIndicator(Handler h);

    /**
     * Unregister a registrant for 'ImsDisableStart'
     * @param h
     */
    void unregisterForImsDisableStart(Handler h);

    /**
     * Register a registrant for 'ImsEnableComplete'
     * @param h
     * @param what
     * @param obj
     */
    void registerForImsEnableComplete(Handler h, int what, Object obj);

    /**
     * Unregister a registrant for 'ImsEnableComplete'
     * @param h
     */
    void unregisterForImsEnableComplete(Handler h);

    /**
     * Register a registrant for 'ImsDisableComplete'
     * @param h
     * @param what
     * @param obj
     */
    void registerForImsDisableComplete(Handler h, int what, Object obj);

    /**
     * Unregister a registrant for 'ImsDisableComplete'
     * @param h
     */
    void unregisterForImsDisableComplete(Handler h);

    /**
     * Register a registrant for 'ImsRegistrationInfo'
     * @param h
     * @param what
     * @param obj
     */
    void registerForImsRegistrationInfo(Handler h, int what, Object obj);

    /**
     * Unregister a registrant for 'ImsRegistrationInfo'
     * @param h
     */
    void unregisterForImsRegistrationInfo(Handler h);

    /**
     * Register a registrant for 'OnEnterECBM'
     * @param h
     * @param what
     * @param obj
     */
    void registerForOnEnterECBM(Handler h, int what, Object obj);

    /**
     * Unregister a registrant for 'OnEnterECBM'
     * @param h
     */
    void unregisterForOnEnterECBM(Handler h);

    /**
     * Register a registrant for 'OnExitECBM'
     * @param h
     * @param what
     * @param obj
     */
    void registerForOnExitECBM(Handler h, int what, Object obj);

    /**
     * Unregister a registrant for 'OnExitECBM'
     * @param h
     */
    void unregisterForOnExitECBM(Handler h);

    /**
     * Register a registrant for 'OnNoECBM'
     * @param h
     * @param what
     * @param obj
     */
    void registerForOnNoECBM(Handler h, int what, Object obj);

    /**
     * Unregister a registrant for 'OnNoECBM'
     * @param h
     */
    void unregisterForOnNoECBM(Handler h);

    /**
     * Register a registrant for 'GetProvisionComplete'
     * @param h
     * @param what
     * @param obj
     */
    void registerForGetProvisionComplete(Handler h, int what, Object obj);

    /**
     * Unregister a registrant for 'GetProvisionComplete'
     * @param h
     */
    void unregisterForGetProvisionComplete(Handler h);

    /**
     * Register for Ims config dynamic ims switch complete indication
     * @param h
     * @param what
     * @param obj
     */
    public void registerForImsCfgDynamicImsSwitchComplete(Handler h, int what, Object obj);

    /**
     * Unregister for Ims config dynamic ims switch complete indication
     * @param h
     */
    public void unregisterForImsCfgDynamicImsSwitchComplete(Handler h);

    /**
     * Register for Ims config feature changed indication
     * @param h
     * @param what
     * @param obj
     */
    public void registerForImsCfgFeatureChanged(Handler h, int what, Object obj);

    /**
     * Unregister for Ims config feature changed indication
     * @param h
     */
    public void unregisterForImsCfgFeatureChanged(Handler h);

    /**
     * Register for Ims config configInt changed indication
     * @param h
     * @param what
     * @param obj
     */
    public void registerForImsCfgConfigChanged(Handler h, int what, Object obj);

    /**
     * Unregister for Ims config changed indication
     * @param h
     */
    public void unregisterForImsCfgConfigChanged(Handler h);

    /**
     * Register for Ims config loaded indication
     * @param h
     * @param what
     * @param obj
     */
    public void registerForImsCfgConfigLoaded(Handler h, int what, Object obj);

    /**
     * Unregister for Ims config loaded indication
     * @param h
     */
    public void unregisterForImsCfgConfigLoaded(Handler h);

    /**
     * Register a registrant for 'EctResult'
     * @param h
     * @param what
     * @param obj
     */
    void registerForEctResult(Handler h, int what, Object obj);

    /**
     * Unregister a registrant for 'EctResult'
     * @param h
     */
    void unregisterForEctResult(Handler h);

    /**
     * Register a registrant for 'CallModeChangeIndicator'
     * URC for RIL_UNSOL_CALLMOD_CHANGE_INDICATOR
     * @param h
     * @param what
     * @param obj
     */
    void registerForCallModeChangeIndicator(Handler h, int what, Object obj);

    /**
     * Unregister a registrant for 'CallModeChangeIndicator'
     * URC for RIL_UNSOL_CALLMOD_CHANGE_INDICATOR
     * @param h
     */
    void unregisterForCallModeChangeIndicator(Handler h);

    /**
     * Register a registrant for 'VideoCapabilityIndicator'
     * @param h
     * @param what
     * @param obj
     */
    void registerForVideoCapabilityIndicator(Handler h, int what, Object obj);

    /**
     * Unregister a registrant for 'VideoCapabilityIndicator'
     * @param h
     */
    void unregisterForVideoCapabilityIndicator(Handler h);

    /**
     * Register a registrant for 'ImsRTPInfo'
     * @param h
     * @param what
     * @param obj
     */
    void registerForImsRTPInfo(Handler h, int what, Object obj);

    /**
     * Unregister a registrant for 'ImsRTPInfo'
     * @param h
     */
    void unregisterForImsRTPInfo(Handler h);

    /**
     * Register a registrant for 'VolteSettingChanged'
     * @param h
     * @param what
     * @param obj
     */
    void registerForVolteSettingChanged(Handler h, int what, Object obj);

    /**
     * Unregister a registrant for 'VolteSettingChanged'
     * @param h
     */
    void unregisterForVolteSettingChanged(Handler h);

    /**
     * Register a registrant for 'BearerState'
     * @param h
     * @param what
     * @param obj
     */
    void registerForBearerState(Handler h, int what, Object obj);

    /**
     * Unregister a registrant for 'BearerState'
     * @param h
     */
    void unregisterForBearerState(Handler h);

    /** Register for IMS bearer initialize event.
     *
     * @param h handler
     * @param what message
     * @param obj object
     * @hide
     */
    void registerForBearerInit(Handler h, int what, Object obj);

    /**
     * Un-register for IMS bearer initialize event.
     *
     * @param h handler
     * @hide
     */
    void unregisterForBearerInit(Handler h);

    /**
     * Register a registrant for 'BearerDeactivation'
     * @param h
     * @param what
     * @param obj
     */
    void registerForImsDataInfoNotify(Handler h, int what, Object obj);

    /**
     * Unregister a registrant for 'BearerDeactivation'
     * @param h
     */
    void unregisterForImsDataInfoNotify(Handler h);

    /**
     * Register for ON
     * @param h
     * @param what
     * @param obj
     */
    void registerForOn(Handler h, int what, Object obj);

    /**
     * Unregister for ON
     * @param h
     */
    void unregisterForOn(Handler h);

    /**
     * Register for OFF
     * @param h
     * @param what
     * @param obj
     */
    void registerForOff(Handler h, int what, Object obj);

    /**
     * Unregister for OFF
     * @param h
     */
    void unregisterForOff(Handler h);

    /**
     * Register for Not available
     * @param h
     * @param what
     * @param obj
     */
    void registerForNotAvailable(Handler h, int what, Object obj);

    /**
     * Unregister for not available
     * @param h
     */
    void unregisterForNotAvailable(Handler h);
    /**
     * Register for IMS Deregister Complete
     * @param h
     * @param what
     * @param obj
     */
    void registerForImsDeregisterComplete(Handler h, int what, Object obj);

    /**
     * Unregister for IMS Deregister Complete
     * @param h
     */
    void unregisterForImsDeregisterComplete(Handler h);

    /**
     * Register for MD support IMS count indication
     * @param h
     * @param what
     * @param obj
     */
    public void registerForMultiImsCount(Handler h, int what, Object obj);

    /**
     * Unregister for MD support IMS count indication
     * @param h
     */
    public void unregisterForMultiImsCount(Handler h);

    public void registerForSpeechCodecInfo(Handler h, int what, Object obj);

    public void unregisterForSpeechCodecInfo(Handler h);

    public void registerForSipHeaderInd(Handler h, int what, Object obj);

    public void unregisterForSipHeaderInd(Handler h);

    /**
     * Register for redial IMS ECC indication event. (RIL_UNSOL_REDIAL_EMERGENCY_INDICATION)
     *
     * @param h handler
     * @param what message
     * @param obj object
     */
    public void registerForImsRedialEccInd(Handler h, int what, Object obj);

    /**
     * Unregister for redial IMS ECC indication event. (RIL_UNSOL_REDIAL_EMERGENCY_INDICATION)
     *
     * @param h handler
     */
    public void unregisterForImsRedialEccInd(Handler h);

    /**
     * Register a registrant for 'ImsConferenceInfo'
     * @param h
     * @param what
     * @param obj
     */
    void registerForImsConfInfoUpdate(Handler h, int what, Object obj);

    /**
     * Unregister a registrant for 'ImsConferenceInfo'
     * @param h
     */
    void unregisterForImsConfInfoUpdate(Handler h);

    /**
     * Register a registrant for 'LteMsgWaiting'
     * @param h
     * @param what
     * @param obj
     */
    void registerForLteMsgWaiting(Handler h, int what, Object obj);

    /**
     * Unregister a registrant for 'ImsEventPackage'
     * @param h
     */
    void unregisterForLteMsgWaiting(Handler h);

    public void registerForImsDialog(Handler h, int what, Object obj);

    public void unregisterForImsDialog(Handler h);

    public void registerForSsacStateInd(Handler h, int what, Object obj);

    public void unregisterForSsacStateInd(Handler h);

    // ========== Call Control APIs =====================================================
    /**
     * Set Voice Call Preference
     * @param vdp
     * @param response
     */
    void setVoiceDomainPreference(int vdp, Message response);
    // SMS-START
    /**
     * This method will be triggered by the platform when the user attempts to send an SMS. This
     * method should be implemented by the IMS providers to provide implementation of sending an SMS
     * over IMS.
     *
     * @param token unique token generated by the platform that should be used when triggering
     *             callbacks for this specific message.
     * @param messageRef the message reference.
     * @param format the format of the message. Valid values are {@link SmsMessage#FORMAT_3GPP} and
     *               {@link SmsMessage#FORMAT_3GPP2}.
     * @param smsc the Short Message Service Center address.
     * @param isRetry whether it is a retry of an already attempted message or not.
     * @param pdu PDUs representing the contents of the message.
     * @param response response message used to notify result
     */
    void sendSms(int token, int messageRef, String format, String smsc, boolean isRetry,
            byte[] pdu, Message response);
    void acknowledgeLastIncomingGsmSms(boolean success, int cause, Message response);
    void acknowledgeLastIncomingCdmaSmsEx(boolean success, int cause, Message response);
    // SMS-END

    // ========== RTT APIs =====================================================
    /**
     * Set RTT Mode
     * RIL_REQUEST_SET_RTT_MODE
     * @param mode Mode
     * @param response Response object
     */
    void setRttMode(int mode, Message response);

    /**
     * Send RTT Modify Request
     * RIL_REQUEST_SEND_RTT_MODIFY_REQUEST
     * @param callId Call id
     * @param newMode New mode
     * @param response Response object
     */
    void sendRttModifyRequest(int callId, int newMode, Message response);

    /**
     * Send RTT Text
     * RIL_REQUEST_SEND_RTT_TEXT
     * @param callId Call Id
     * @param text Text
     * @param response Response object
     */
    void sendRttText(int callId, String text, int length, Message response);

    /**
     * Request RTT Modify Response
     * RIL_REQUEST_RTT_MODIFY_REQUEST_RESPONSE
     * @param callId Call id
     * @param result Result
     * @param response Response object
     */
    void setRttModifyRequestResponse(int callId, int result, Message response);

    /**
     * Request RTT Audio Indication
     * RIL_REQUEST_TOGGLE_RTT_AUDIO_INDICATION
     * @param callId Call id
     * @param enable if enable RTT audio
     * @param response Response object
     */
    void toggleRttAudioIndication(int callId, int enable, Message response);

    /**
     * Registers the handler for Rtt Modify Response event.
     * @param h Handler for notification message.
     * @param what User-defined message code.
     * @param obj User object.
     *
     */
    void registerForRttModifyResponse(Handler h, int what, Object obj);

    /**
     * Unregisters the handler for Rtt Modify Response event.
     *
     * @param h Handler for notification message.
     *
     */
    void unregisterForRttModifyResponse(Handler h);

    /**
     * Registers the handler for Rtt Text Receive event.
     * @param h Handler for notification message.
     * @param what User-defined message code.
     * @param obj User object.
     *
     */
    void registerForRttTextReceive(Handler h, int what, Object obj);

    /**
     * Unregisters the handler for Rtt Text Receive event.
     *
     * @param h Handler for notification message.
     *
     */
    void unregisterForRttTextReceive(Handler h);

    /**
     * Registers the handler for Rtt Modify Request Receive event.
     * @param h Handler for notification message.
     * @param what User-defined message code.
     * @param obj User object.
     *
     */
    void registerForRttModifyRequestReceive(Handler h, int what, Object obj);

    /**
     * Unregisters the handler for Rtt Modify Request Receive event.
     *
     * @param h Handler for notification message.
     *
     */
    void unregisterForRttModifyRequestReceive(Handler h);

    /**
     * Registers the handler for RTT capability changed event.
     * @param h Handler for notification message.
     * @param what User-defined message code.
     * @param obj User object.
     *
     */
    void registerForRttCapabilityIndicator(Handler h, int what, Object obj);


    /**
     * Unregisters the handler for RTT capability changed event.
     *
     * @param h Handler for notification message.
     *
     */
    void unregisterForRttCapabilityIndicator(Handler h);


    /**
     * Registers the handler for RTT audio changed event.
     * @param h Handler for notification message.
     * @param what User-defined message code.
     * @param obj User object.
     *
     */
    void registerForRttAudioIndicator(Handler h, int what, Object obj);


    /**
     * Unregisters the handler for RTT audio changed event.
     *
     * @param h Handler for notification message.
     *
     */
    void unregisterForRttAudioIndicator(Handler h);


    // ========== Depreted APIs =========================================================
    /**
     * Get Provision Value
     * @param phoneId
     * @param provisionStr
     * @param response
     */
    @Deprecated
    void getProvisionValue(int phoneid, String provisionStr, Message response);

    /**
     * Set Provision Value
     * @param phoneId
     * @param provisionStr
     * @param provisionValue
     * @param response
     */
    @Deprecated
    void setProvisionValue(int phoneid, String provisionStr,
                           String provisionValue, Message response);


    /**
     * Set WFC Profile Info
     * @param phoneid
     * @param wfcPreference
     * @param response
     */
    @Deprecated
    void sendWfcProfileInfo(int phoneid, int wfcPreference, Message response);


    /**
     * Send a ACK for 'BearActivationDone/BearDeactivationDone'
     * @param phoneid
     * @param aid
     * @param status
     * @param action
     */
    @Deprecated
    void responseBearerStateConfirm(int phoneid, int aid, int action, int status);


    /**
     * Set ims bearer notification
     * @param phoneid
     * @param enable
     */
    @Deprecated
    void setImsBearerNotification(int phoneid, int enable);

    /**
     * Start Conference
     * @param participants
     * @param clirMode
     * @param isVideoCall
     * @param phoneid
     * @param response
     */
    @Deprecated
    void startConference(String[] participants, int clirMode,
                         boolean isVideoCall, int phoneid, Message response);


    /**
     * Accept Video Call
     * @param videoMode
     * @param callId
     */
    @Deprecated
    void acceptVideoCall(int videoMode, int callId);

    /**
     * Accept call
     */
    @Deprecated
    void accept();

    /**
     * Reject a call
     * @param callId
     * @param response
     */
    @Deprecated
    void reject(int callId);

    /**
     * Force Release a call
     * @param callId
     * @param response
     */
    @Deprecated
    void forceHangup(int callId);

    /**
     * Terminal Call
     * @param callId
     * @param response
     */
    @Deprecated
    void terminate(int callId);

    /**
     * Get the Last Fail Cause
     * @param phoneid
     * @param response
     */
    @Deprecated
    void getLastCallFailCause(int phoneid, Message response);

    /**
     * Set IMS Features Enable/Disable
     * @param params
     * @param phoneid
     * @param response
     */
    @Deprecated
    void setImsCfg(int[] params, int phoneid, Message response);

    /**
     * To set multiple IMS configurations to modem
     *
     * @param keys Defined in ImsConfig.ConfigConstants, entries divided by ",".
     * @param values The values for each keys, divided by ",".
     * @param phoneId The phone ID to be set.
     * @param type The type of the config, maps to AT+ECFGSET, AT+EWOCFGSET and AT+EIWLCFGSET AT commands.
     * @param reponse The response of the request
     *
     * Response callback is IRadioResponse.setModemImsConfigResponse()
     */
    void setModemImsCfg(String keys, String values, int type, Message response);

    /**
     * Set IMS Registration Report
     * @param phoneId
     * @param response
     */
    @Deprecated
    void setImsRegistrationReport(int phoneid, Message response);

    /**
     * Turn ON IMS
     * @param phoneid
     * @param response
     */
    @Deprecated
    void turnOffIms(int phoneid, Message response);

    /**
     * Turn OFF IMS
     * @param phoneid
     * @param response
     */
    @Deprecated
    void turnOnIms(int phoneid, Message response);

    /**
     * Merge Call
     * @param response
     */
    @Deprecated
    void merge(Message response);

    /**
     * Set IMS Config Feature Value
     * @param featureid
     * @param network
     * @param value
     * @param isLast, is last command of set multi feature values.
     * @param response
     */
    void setImsCfgFeatureValue(int featureId, int network, int value, int isLast,
                               Message response);

    /**
     * Get IMS Config Feature Value
     * @param featureid
     * @param network
     * @param response
     */
    void getImsCfgFeatureValue(int featureId, int network, Message response);

    /**
     * Set IMS Config Provision Value
     * @param configId
     * @param value
     * @param response
     */
    void setImsCfgProvisionValue(int configId, String value, Message response);

    /**
     * Get IMS Config Provision Value
     * @param configId
     * @param response
     */
    void getImsCfgProvisionValue(int configId, Message response);

    /**
     * Get IMS Config Resource Capability Value
     * @param featureId
     * @param response
     */
    void getImsCfgResourceCapValue(int featureId, Message response);

    /**
     * Query Voice over PS status
     *
     * @param response
     */
    public void queryVopsStatus(Message result);

    /**
     * Notify ImsService is ready to receive pending indication
     */
    public void notifyImsServiceReady();

    /**
     * Set SIP header
     * @param total
     * @param index
     * @param headerCount
     * @param headerValuePair
     * @param response
     */
    void setSipHeader(int total, int index, int headerCount, String headerValuePair, Message response);

    /**
     * Enable SIP header report
     * @param callId
     * @param headerType
     * @param response
     */
    void setSipHeaderReport(String callId, String headerType, Message response);

    /**
     * Set IMS call mode
     * @param mode
     * @param response
     */
    void setImsCallMode(int mode, Message response);

    public void querySsacStatus(Message response);

    /**
     * Set additional call information to modem.
     * @param info
     * @param response
     */
    void setCallAdditionalInfo(ArrayList<String> info, Message response);
}


