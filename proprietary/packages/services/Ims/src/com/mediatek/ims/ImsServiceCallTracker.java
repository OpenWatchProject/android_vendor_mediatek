/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
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
import android.net.Uri;
import android.telephony.PhoneNumberUtils;
import android.telephony.Rlog;
import android.text.TextUtils;
import android.os.SystemProperties;
import android.os.Build;
import android.util.SparseArray;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.concurrent.ConcurrentHashMap;
import java.util.Map;

import com.mediatek.ims.ImsCallInfo;
import com.mediatek.ims.ImsCallSessionProxy;
import com.mediatek.ims.internal.ImsXuiManager;

import android.telephony.ims.ImsCallProfile;

public class ImsServiceCallTracker {
    private static final String LOG_TAG = "ImsServiceCT";

    // Sensitive log task
    private static final String PROP_FORCE_DEBUG_KEY = "persist.vendor.log.tel_dbg";
    private static final boolean SENLOG = TextUtils.equals(Build.TYPE, "user");
    private static final boolean TELDBG = (SystemProperties.getInt(PROP_FORCE_DEBUG_KEY, 0) == 1);

    // IMS Call Modes
    private static final int INVALID_CALL = -1;
    private static final int IMS_VOICE = 20;
    private static final int IMS_VIDEO = 21;
    private static final int IMS_VOICE_CONF = 22;
    private static final int IMS_VIDEO_CONF = 23;
    private static final int IMS_VOICE_CONF_PARTS = 24;
    private static final int IMS_VIDEO_CONF_PARTS = 25;

    private static final int CALL_MSG_TYPE_MT = 0;
    private static final int CALL_MSG_TYPE_ALERT = 2;
    private static final int CALL_MSG_TYPE_ID_ASSIGN = 130;
    private static final int CALL_MSG_TYPE_HELD = 131;
    private static final int CALL_MSG_TYPE_ACTIVE = 132;
    private static final int CALL_MSG_TYPE_DISCONNECTED = 133;

    private static SparseArray<ImsServiceCallTracker> mImsServiceCTs =
            new SparseArray<ImsServiceCallTracker>();

    private int mPhoneId;
    private boolean mEnableVolteForImsEcc = false;

    private ConcurrentHashMap<String, ImsCallInfo> mCallConnections =
            new ConcurrentHashMap<String, ImsCallInfo>();

    public static ImsServiceCallTracker getInstance(int phoneId) {
        if (mImsServiceCTs.get(phoneId) == null) {
            mImsServiceCTs.put(phoneId, new ImsServiceCallTracker(phoneId));
        }

        return mImsServiceCTs.get(phoneId);
    }

    private ImsServiceCallTracker(int phoneId) {
        mPhoneId = phoneId;
    }

    public boolean getEnableVolteForImsEcc() {
        return mEnableVolteForImsEcc;
    }

    public void setEnableVolteForImsEcc(boolean enable) {
        mEnableVolteForImsEcc = enable;
    }

    public ImsCallInfo getCallInfo(String callId) {

        ImsCallInfo callInfo = mCallConnections.get(callId);

        if (callInfo != null) {
            logWithPhoneId("getCallInfo(callId) : callID: " + callInfo.mCallId + " call num: " +
                    sensitiveEncode(callInfo.mCallNum) + " call State: " + callInfo.mState);
        } else {
            logWithPhoneId("getCallInfo(callId) : callID: " + callId + " is null");
        }

        return callInfo;
    }

    public ImsCallInfo getCallInfo(ImsCallInfo.State state) {
        ImsCallInfo callInfo;
        for (Map.Entry<String, ImsCallInfo> entry : mCallConnections.entrySet()) {
            callInfo = entry.getValue();

            logWithPhoneId("getCallInfo(state) : callID: " + callInfo.mCallId + " call num: " +
                    sensitiveEncode(callInfo.mCallNum) + " call State: " + callInfo.mState);

            if (callInfo.mState == state) {
                return callInfo;
            }
        }
        return null;
    }

    public void removeCallConnection(String callId) {
        if(callId != null) {
            mCallConnections.remove(callId);
        }
    }

    public ImsCallSessionProxy getFgCall() {
        for (Map.Entry<String, ImsCallInfo> entry : mCallConnections.entrySet()) {
            ImsCallInfo callInfo = entry.getValue();
            if (ImsCallInfo.State.ACTIVE  == callInfo.mState) {
                return callInfo.mCallSession;
            }
        }
        return null;
    }

    public ImsCallSessionProxy getConferenceHostCall() {
        for (Map.Entry<String, ImsCallInfo> entry : mCallConnections.entrySet()) {
            ImsCallInfo callInfo = entry.getValue();
            if (callInfo.mIsConferenceHost == true) {
                return callInfo.mCallSession;
            }
        }
        return null;
    }

    public int getCurrentCallCount() {
        if (mCallConnections == null) return 0;
        return mCallConnections.size();
    }

    public int getParticipantCallId(String callNumber) {
        int participantCallId = -1;

        for (Map.Entry<String, ImsCallInfo> entry : mCallConnections.entrySet()) {
            ImsCallInfo callInfo = entry.getValue();
            if (callNumber.equals(callInfo.mCallNum)) {
                participantCallId = Integer.parseInt(callInfo.mCallId);
                break;
            }
        }

        logWithPhoneId("getParticipantCallId() : participantCallId = " + participantCallId);

        return participantCallId;
    }

    public boolean isVoiceCall(int mode) {

        if (IMS_VOICE == mode || IMS_VOICE_CONF == mode || IMS_VOICE_CONF_PARTS == mode) {
            return true;
        }
        return false;
    }

    public boolean isVideoCall(int mode) {

        if (IMS_VIDEO == mode || IMS_VIDEO_CONF == mode || IMS_VIDEO_CONF_PARTS == mode) {
            return true;
        }
        return false;
    }

    public boolean isConferenceCall(int mode) {

        if (IMS_VOICE_CONF == mode || IMS_VOICE_CONF_PARTS == mode || IMS_VIDEO_CONF == mode || IMS_VIDEO_CONF_PARTS == mode) {
            return true;
        }
        return false;
    }

    public boolean isConferenceCallHost(int mode) {

        if (IMS_VOICE_CONF == mode || IMS_VIDEO_CONF == mode) {
            return true;
        }
        return false;
    }

    public boolean isInCall() {

        for (Map.Entry<String, ImsCallInfo> entry : mCallConnections.entrySet()) {
            ImsCallInfo callInfo = entry.getValue();
            if (ImsCallInfo.State.ACTIVE == callInfo.mState || ImsCallInfo.State.HOLDING == callInfo.mState) {
                return true;
            }
        }
        return false;
    }

    public boolean isInCall(ImsCallInfo callInfo) {

        if (ImsCallInfo.State.ACTIVE == callInfo.mState || ImsCallInfo.State.HOLDING == callInfo.mState) {
            return true;
        }
        return false;
    }

    public static boolean isEccExistOnAnySlot() {
        for (int i = 0; i < mImsServiceCTs.size(); i++) {
            ImsServiceCallTracker imsServiceCT = mImsServiceCTs.valueAt(i);
            boolean isEccExist = false;

            if (imsServiceCT != null) {
                if (imsServiceCT.isEccExist()) {
                    return true;
                }
            }
        }
        return false;
    }

    public boolean isEccExist() {

        for (Map.Entry<String, ImsCallInfo> entry : mCallConnections.entrySet()) {
            ImsCallInfo callInfo = entry.getValue();
            if (callInfo.mIsEcc && isInCall(callInfo)) {
                return true;
            }
        }
        return false;
    }

    public boolean isVideoCallExist() {

        for (Map.Entry<String, ImsCallInfo> entry : mCallConnections.entrySet()) {
            ImsCallInfo callInfo = entry.getValue();
            if (callInfo.mIsVideo && isInCall(callInfo)) {
                return true;
            }
        }
        return false;
    }

    public boolean isConferenceCallExist() {

        for (Map.Entry<String, ImsCallInfo> entry : mCallConnections.entrySet()) {
            ImsCallInfo callInfo = entry.getValue();
            if ((callInfo.mIsConference || callInfo.mIsConferenceHost) && isInCall(callInfo)) {
                return true;
            }
        }
        return false;
    }

    public boolean isConferenceHostCallExist() {

        for (Map.Entry<String, ImsCallInfo> entry : mCallConnections.entrySet()) {
            ImsCallInfo callInfo = entry.getValue();
            if (callInfo.mIsConferenceHost && isInCall(callInfo)) {
                return true;
            }
        }
        return false;
    }

    public boolean isSelfAddress(String addr) {
        Uri[] selfUri = ImsXuiManager.getInstance().getSelfIdentifyUri(mPhoneId);
        if (selfUri == null) {
            return false;
        }
        for (int i = 0; i < selfUri.length; ++i) {
            String address = selfUri[i].getSchemeSpecificPart();
            String numberParts[] = address.split("[@;:]");
            if (numberParts.length == 0) continue;
            logWithPhoneId("isSelfAddress() selfId: " + sensitiveEncode(numberParts[0])
                    + " addr: " + sensitiveEncode(addr));
            if (PhoneNumberUtils.compareLoosely(addr, numberParts[0])) {
                return true;
            }
        }
        return false;
    }

    public ArrayList<String> getSelfAddressList() {
        Uri[] selfUri = ImsXuiManager.getInstance().getSelfIdentifyUri(mPhoneId);
        if (selfUri == null) {
            return null;
        }
        ArrayList<String> selfAddressList = new ArrayList<String>();
        for (int i = 0; i < selfUri.length; ++i) {
            String address = selfUri[i].getSchemeSpecificPart();
            String numberParts[] = address.split("[@;:]");
            if (numberParts.length == 0) continue;
            logWithPhoneId("isSelfAddress() selfId: " + sensitiveEncode(numberParts[0]));
            selfAddressList.add(numberParts[0]);
        }
        return selfAddressList;
    }

    /**
     * Process Call Info Indication
     * Add connection status in connections list
     * @param msgType Message Type
     * @param callMide Call Mode
     * @param callId Call id
     * @param callNum Call number
     */
    public void processCallInfoIndication(String[] callInfo, ImsCallSessionProxy callSession, ImsCallProfile profile) {
        String callId = callInfo[0];
        int msgType = Integer.parseInt(callInfo[1]);
        int callMode = TextUtils.isEmpty(callInfo[5])? INVALID_CALL: Integer.parseInt(callInfo[5]);
        String callNum = callInfo[6];

        boolean isConference = false;
        if (isConferenceCall(callMode)) {
            isConference = true;
        }

        boolean isConferenceHost = false;
        if (isConferenceCallHost(callMode)) {
            isConferenceHost = true;
        }

        boolean isVideo = false;
        if (isVideoCall(callMode)) {
            isVideo = true;
        }

        boolean isEcc = false;
        if (profile.getServiceType() == ImsCallProfile.SERVICE_TYPE_EMERGENCY) {
            isEcc = true;
        }

        ImsCallInfo imsCallInfo = null;
        switch(msgType) {
            case CALL_MSG_TYPE_MT:  // Record call string to mCallConnections for MT case.
            {
                logWithPhoneId("processCallInfoIndication() : CALL_MSG_TYPE_MT => "
                               + "callId = " + callId
                               + ", callMode = " + callMode
                               + ", isConference = " + isConference
                               + ", isConferenceHost = " + isConferenceHost
                               + ", isVideo = " + isVideo
                               + ", isEcc = " + isEcc);

                ImsCallInfo.State state = ImsCallInfo.State.INCOMING;
                mCallConnections.put(callId,
                        new ImsCallInfo(
                            callId,
                            callNum,
                            isConference,
                            isConferenceHost,
                            isVideo,
                            isEcc,
                            state,
                            callSession));
                break;
            }
            case CALL_MSG_TYPE_ID_ASSIGN: // Record call string to mCallConnections for MO case.
            {

                logWithPhoneId("processCallInfoIndication() : CALL_MSG_TYPE_ID_ASSIGN => "
                               + "callId = " + callId
                               + ", callMode = " + callMode
                               + ", isConference = " + isConference
                               + ", isConferenceHost = " + isConferenceHost
                               + ", isVideo = " + isVideo
                               + ", isEcc = " + isEcc);

                ImsCallInfo.State state = ImsCallInfo.State.ALERTING;
                mCallConnections.put(callId,
                        new ImsCallInfo(
                            callId,
                            callNum,
                            isConference,
                            isConferenceHost,
                            isVideo,
                            isEcc,
                            state,
                            callSession));
                break;
            }
            case CALL_MSG_TYPE_ALERT:   // Record call string to mCallConnections for MO case.
            {
                logWithPhoneId("processCallInfoIndication() : CALL_MSG_TYPE_ALERT => "
                               + "callId = " + callId
                               + ", callMode = " + callMode
                               + ", isConference = " + isConference
                               + ", isConferenceHost = " + isConferenceHost
                               + ", isVideo = " + isVideo
                               + ", isEcc = " + isEcc);

                imsCallInfo = mCallConnections.get(callId);
                if (imsCallInfo == null) return;
                imsCallInfo.mIsConference = isConference;
                imsCallInfo.mIsConferenceHost = isConferenceHost;
                imsCallInfo.mIsVideo = isVideo;
                mCallConnections.put(callId, imsCallInfo);
                break;
            }
            case CALL_MSG_TYPE_HELD:
            {
                logWithPhoneId("processCallInfoIndication() : CALL_MSG_TYPE_HELD => "
                               + "callId = " + callId
                               + ", callMode = " + callMode
                               + ", isConference = " + isConference
                               + ", isConferenceHost = " + isConferenceHost
                               + ", isVideo = " + isVideo
                               + ", isEcc = " + isEcc);

                imsCallInfo = mCallConnections.get(callId);
                if (imsCallInfo == null) return;
                imsCallInfo.mState = ImsCallInfo.State.HOLDING;
                imsCallInfo.mIsConference = isConference;
                imsCallInfo.mIsConferenceHost = isConferenceHost;
                mCallConnections.put(callId, imsCallInfo);
                break;
            }
            case CALL_MSG_TYPE_ACTIVE:
            {
                logWithPhoneId("processCallInfoIndication() : CALL_MSG_TYPE_ACTIVE => "
                               + "callId = " + callId
                               + ", callMode = " + callMode
                               + ", isConference = " + isConference
                               + ", isConferenceHost = " + isConferenceHost
                               + ", isVideo = " + isVideo
                               + ", isEcc = " + isEcc);

                imsCallInfo = mCallConnections.get(callId);
                if (imsCallInfo == null) return;
                imsCallInfo.mState = ImsCallInfo.State.ACTIVE;
                imsCallInfo.mIsConference = isConference;
                imsCallInfo.mIsConferenceHost = isConferenceHost;
                imsCallInfo.mIsVideo = isVideo;
                mCallConnections.put(callId, imsCallInfo);
                break;
            }
            case CALL_MSG_TYPE_DISCONNECTED: // Clear call string when call is disconnected.
            {
                logWithPhoneId("processCallInfoIndication() : CALL_MSG_TYPE_DISCONNECTED => "
                               + "callId = " + callId
                               + ", callMode = " + callMode
                               + ", isConference = " + isConference
                               + ", isConferenceHost = " + isConferenceHost
                               + ", isVideo = " + isVideo
                               + ", isEcc = " + isEcc);

                mCallConnections.remove(callId);
                break;
            }
            default:
                break;
        }
    }

    /**
     * Process Call mode changed Indication
     * Add connection status in connections list
     * @param callModeInfo call mode Info
     */
    public void processCallModeChangeIndication(String[] callModeInfo) {

        int callMode = INVALID_CALL;

        if (callModeInfo == null) {
            return;
        }

        String callId = callModeInfo[0];
        if ((callModeInfo[1] != null) && (!callModeInfo[1].equals(""))) {
            callMode = Integer.parseInt(callModeInfo[1]);
        }
        logWithPhoneId("processCallModeChangeIndication() :"
               + "callId = " + callId
               + ", callMode = " + callMode);

        ImsCallInfo imsCallInfo = mCallConnections.get(callId);
        if (imsCallInfo == null) return;

        imsCallInfo.mIsVideo = isVideoCall(callMode);
        mCallConnections.put(callId, imsCallInfo);
    }

    private void logWithPhoneId(String msg) {

        if (!TELDBG) return;

        Rlog.d(LOG_TAG, "[PhoneId = " + mPhoneId + "] " + msg);
    }

    public static String sensitiveEncode(String msg) {
        if (!SENLOG || TELDBG) {
            return Rlog.pii(LOG_TAG, msg);
        } else {
            return "[hidden]";
        }
    }
}
