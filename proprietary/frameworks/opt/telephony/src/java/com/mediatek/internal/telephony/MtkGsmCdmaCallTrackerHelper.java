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

package com.mediatek.internal.telephony;

import com.android.internal.telephony.Connection;
import com.android.internal.telephony.DriverCall;
import com.android.internal.telephony.GsmCdmaCall;
import com.android.internal.telephony.PhoneConstants;
import com.mediatek.internal.telephony.MtkGsmCdmaCallTracker;
import com.mediatek.internal.telephony.MtkGsmCdmaConnection;
import com.mediatek.internal.telephony.MtkRIL;

import android.content.Context;
import android.os.AsyncResult;
import android.os.Process;
import android.provider.CallLog;
import android.telecom.PhoneAccountHandle;
import android.telecom.TelecomManager;
import android.telephony.DisconnectCause;
import android.telephony.PhoneNumberUtils;
import android.telephony.Rlog;
import android.telephony.SubscriptionManager;

import java.util.Date;
import java.util.Iterator;


public final class MtkGsmCdmaCallTrackerHelper
        implements MtkIncomingCallChecker.OnCheckCompleteListener {

    static final String LOG_TAG = "GsmCallTkrHlpr";

    protected static final int EVENT_POLL_CALLS_RESULT             = 1;
    protected static final int EVENT_CALL_STATE_CHANGE             = 2;
    protected static final int EVENT_REPOLL_AFTER_DELAY            = 3;
    protected static final int EVENT_OPERATION_COMPLETE            = 4;
    protected static final int EVENT_GET_LAST_CALL_FAIL_CAUSE      = 5;

    protected static final int EVENT_SWITCH_RESULT                 = 8;
    protected static final int EVENT_RADIO_AVAILABLE               = 9;
    protected static final int EVENT_RADIO_NOT_AVAILABLE           = 10;
    protected static final int EVENT_CONFERENCE_RESULT             = 11;
    protected static final int EVENT_SEPARATE_RESULT               = 12;
    protected static final int EVENT_ECT_RESULT                    = 13;
    protected static final int EVENT_EXIT_ECM_RESPONSE_CDMA        = 14;
    protected static final int EVENT_CALL_WAITING_INFO_CDMA        = 15;
    protected static final int EVENT_THREE_WAY_DIAL_L2_RESULT_CDMA = 16;
    protected static final int EVENT_THREE_WAY_DIAL_BLANK_FLASH    = 20;

    protected static final int EVENT_MTK_BASE = 1000;

    /// M: CC: Proprietary incoming call handling
    protected static final int EVENT_INCOMING_CALL_INDICATION      = EVENT_MTK_BASE + 0;

    /// M: CC: Modem reset related handling
    protected static final int EVENT_RADIO_OFF_OR_NOT_AVAILABLE    = EVENT_MTK_BASE + 1;

    /// M: CC: Hangup special handling @{
    protected static final int EVENT_DIAL_CALL_RESULT              = EVENT_MTK_BASE + 2;
    protected static final int EVENT_HANG_UP_RESULT                = EVENT_MTK_BASE + 3;
    /// @}

    private Context mContext;
    private MtkGsmCdmaCallTracker mMtkTracker;

    /// M: CC: Forwarding number via EAIC @{
    // To store forwarding address from incoming call indication
    private boolean mContainForwardingAddress = false;
    private String  mForwardingAddress = null;
    private int     mForwardingAddressCallId = 0;
    /// @}

    /// M: CC: Softbank blacklist requirement
    private MtkIncomingCallChecker mIncomingCallChecker = null;

    private static final int MT_CALL_REJECTED = 1;
    private static final int MT_CALL_MISSED = 2;
    private static final int MT_CALL_NUMREDIRECT = 3;

    private static final int MT_CALL_GWSD = 10;
    private boolean mIsGwsdCall = false;

    public MtkGsmCdmaCallTrackerHelper(Context context, MtkGsmCdmaCallTracker tracker) {

        mContext = context;
        mMtkTracker = tracker;
    }

    void logD(String msg) {
        Rlog.d(LOG_TAG, msg + " (slot " + mMtkTracker.mPhone.getPhoneId() + ")");
    }

    void logI(String msg) {
        Rlog.i(LOG_TAG, msg + " (slot " + mMtkTracker.mPhone.getPhoneId() + ")");
    }

    void logW(String msg) {
        Rlog.w(LOG_TAG, msg + " (slot " + mMtkTracker.mPhone.getPhoneId() + ")");
    }

    void logE(String msg) {
        Rlog.e(LOG_TAG, msg + " (slot " + mMtkTracker.mPhone.getPhoneId() + ")");
    }

    public void LogerMessage(int msgType) {

        switch (msgType) {
        case EVENT_POLL_CALLS_RESULT:
            logD("handle EVENT_POLL_CALLS_RESULT");
            break;
        case EVENT_CALL_STATE_CHANGE:
            logD("handle EVENT_CALL_STATE_CHANGE");
            break;
        case EVENT_REPOLL_AFTER_DELAY:
            logD("handle EVENT_REPOLL_AFTER_DELAY");
            break;
        case EVENT_OPERATION_COMPLETE:
            logD("handle EVENT_OPERATION_COMPLETE");
            break;
        case EVENT_GET_LAST_CALL_FAIL_CAUSE:
            logD("handle EVENT_GET_LAST_CALL_FAIL_CAUSE");
            break;
        case EVENT_SWITCH_RESULT:
            logD("handle EVENT_SWITCH_RESULT");
            break;
        case EVENT_RADIO_AVAILABLE:
            logD("handle EVENT_RADIO_AVAILABLE");
            break;
        case EVENT_RADIO_NOT_AVAILABLE:
            logD("handle EVENT_RADIO_NOT_AVAILABLE");
            break;
        case EVENT_CONFERENCE_RESULT:
            logD("handle EVENT_CONFERENCE_RESULT");
            break;
        case EVENT_SEPARATE_RESULT:
            logD("handle EVENT_SEPARATE_RESULT");
            break;
        case EVENT_ECT_RESULT:
            logD("handle EVENT_ECT_RESULT");
            break;
        /* M: CC part start */
        case EVENT_HANG_UP_RESULT:
            logD("handle EVENT_HANG_UP_RESULT");
            break;
        case EVENT_DIAL_CALL_RESULT:
            logD("handle EVENT_DIAL_CALL_RESULT");
            break;
        case EVENT_INCOMING_CALL_INDICATION:
            logD("handle EVENT_INCOMING_CALL_INDICATION");
            break;
        case EVENT_RADIO_OFF_OR_NOT_AVAILABLE:
            logD("handle EVENT_RADIO_OFF_OR_NOT_AVAILABLE");
            break;
        /* M: CC part end */
        default:
            logD("handle XXXXX");
            break;
        }
    }

    public void LogState() {

        int callId = 0;
        int count = 0;

        for (int i = 0, s = mMtkTracker.getMaxConnections(); i < s; i++) {
            if (mMtkTracker.mConnections[i] != null) {
                callId = mMtkTracker.mConnections[i].mIndex + 1;
                count++;
                logI("* conn id " + callId + " existed");
            }
        }
        logI("* GsmCT has " + count + " connection");
    }

    public int getCurrentTotalConnections() {
        int count = 0;
        for (int i = 0; i < mMtkTracker.getMaxConnections(); i++) {
            if (mMtkTracker.mConnections[i] != null) {
                count++;
            }
        }
        return count;
    }

    /// M: CC: Proprietary incoming call handling @{
    public void CallIndicationProcess(AsyncResult ar) {
        CallIndicationProcess(ar, false, false);
    }

    /// M: CC: Softbank blacklist requirement
    public void CallIndicationProcess(AsyncResult ar, boolean isIncomingNumberChecked,
            boolean isBlocked) {
        int mode = 0;
        String[] incomingCallInfo = (String[]) ar.result;
        int callId = Integer.parseInt(incomingCallInfo[0]);
        String number = incomingCallInfo[1];
        int callMode = Integer.parseInt(incomingCallInfo[3]);
        int seqNumber = Integer.parseInt(incomingCallInfo[4]);

        int subId = mMtkTracker.getPhone().getSubId();
        int rejectCause = DisconnectCause.INCOMING_MISSED;

        logD("CallIndicationProcess " + mode + " callId " + callId +
                " seqNumber " + seqNumber
                + "(subId=" + subId + ", isIncomingNumberChecked=" + isIncomingNumberChecked
                + ", isBlocked=" + isBlocked + ")");

        /// M: CC: Forwarding number via EAIC @{
        /* Check if EAIC message contains forwarding address(A calls B and it is forwarded to C,
             C may receive forwarding address - B's phone number). */
        mForwardingAddress = null;
        if ((incomingCallInfo[5] != null) && (incomingCallInfo[5].length() > 0)) {
            /* This value should be set to true after CallManager approves the incoming call */
            mContainForwardingAddress = false;
            mForwardingAddress = incomingCallInfo[5];
            mForwardingAddressCallId = callId;
            logD("EAIC message contains forwarding address - " + mForwardingAddress + "," + callId);
        }
        /// @}

        /// Reject MT when another MT already exists via EAIC disapproval
        if (mMtkTracker.mState == PhoneConstants.State.RINGING) {
            mode = 1;
            rejectCause = DisconnectCause.INCOMING_MISSED;
        }

        /// M: CC: Incoming call block number check
        ///
        ///   For blocked number check enabled project, it would come here twice
        ///   for a new incoming call.
        ///
        ///   The first time, when sub Id is valid, it will enter following 2 checks:
        ///       contact registration check
        ///       block number check
        ///
        ///   When it has done the check, it would re-enter this CallIndicationProcess
        ///   function and set isIncomingNumberChecked true. In this case, the check result
        ///   is carried by isBlocked parameter for callback function handling.
        ///
        ///   CallIndicationProcess -> MtkIncomingCallChecker
        ///       -> onCheckComplete -> CallIndicationProcess
        ///
        if (mode == 0) {
            if (isIncomingNumberChecked) {
                if (isBlocked) {
                    mode = 1;
                    rejectCause = DisconnectCause.INCOMING_REJECTED;
                }
            } else {
                if (SubscriptionManager.isValidSubscriptionId(subId) &&
                        MtkIncomingCallChecker.isMtkEnhancedCallBlockingEnabled(mContext, subId)) {
                    mIncomingCallChecker =
                            new MtkIncomingCallChecker("" + callId + "_" + seqNumber, ar);
                    boolean bCheckStart = mIncomingCallChecker.startIncomingCallNumberCheck(
                        mContext, subId, number, this);
                    if (bCheckStart) {
                        logD("startIncomingCallNumberCheck true. start check (callId_seqNo="
                                + callId + "_" + seqNumber + ", subId=" + subId + ", number="
                                + number + ")");
                        return;
                    } else {
                        logE("startIncomingCallNumberCheck false,"
                                + " and flow continues. (callId_seqNo="
                                + callId + "_" + seqNumber + ", subId=" + subId + ", number="
                                + number + ")");
                    }
                }
            }
        }


        if (mode == 0) {
            /* To raise PhoneAPP priority to avoid delaying incoming call screen to be showed,
                 mtk04070, 20120307 */
            int pid = Process.myPid();
            Process.setThreadPriority(pid, Process.THREAD_PRIORITY_DEFAULT - 10);
            logD("Adjust the priority of process - " + pid + " to " +
                    Process.getThreadPriority(pid));

            /// M: CC: Forwarding number via EAIC @{
            /* EAIC message contains forwarding address(A calls B and it is forwarded to C,
                 C may receive forwarding number - B's phone number). */
            if (mForwardingAddress != null) {
                mContainForwardingAddress = true;
            }
            /// @}

            mMtkTracker.mMtkCi.setCallIndication(mode, callId, seqNumber, -1, null);
        }

        /// Reject MT when another MT already exists via EAIC disapproval @{
        if (mode == 1) {
            if (rejectCause != DisconnectCause.INCOMING_MISSED) {
                mMtkTracker.mMtkCi.setCallIndication(mode, callId, seqNumber, rejectCause, null);

                if (rejectCause == DisconnectCause.INCOMING_REJECTED) {
                    addCallLog(mContext, mMtkTracker.mPhone.getIccSerialNumber(), number,
                            CallLog.Calls.REJECTED_TYPE);
                } else {
                    addCallLog(mContext, mMtkTracker.mPhone.getIccSerialNumber(), number,
                            CallLog.Calls.MISSED_TYPE);
                }
            } else {
                mMtkTracker.mMtkCi.setCallIndication(mode, callId, seqNumber, -1, null);
            }
        }
        /// @}
    }

    public void CallIndicationEnd() {

        /* To adjust PhoneAPP priority to normal, mtk04070, 20120307 */
        int pid = Process.myPid();
        if (Process.getThreadPriority(pid) != Process.THREAD_PRIORITY_DEFAULT) {
            Process.setThreadPriority(pid, Process.THREAD_PRIORITY_DEFAULT);
            logD("Current priority = " + Process.getThreadPriority(pid));
        }
    }

    @Override
    public void onCheckComplete(boolean result, Object obj) {
        CallIndicationProcess((obj != null ? (AsyncResult) obj : null), true, result);
    }

    private void addCallLog(Context context, String iccId, String number, int type) {
        // get PhoneAccountHandle
        PhoneAccountHandle phoneAccountHandle = null;
        final TelecomManager telecomManager = TelecomManager.from(context);
        final Iterator<PhoneAccountHandle> phoneAccounts =
                telecomManager.getCallCapablePhoneAccounts().listIterator();
        while (phoneAccounts.hasNext()) {
            final PhoneAccountHandle handle = phoneAccounts.next();
            String id = handle.getId();
            if (id != null && id.equals(iccId)) {
                phoneAccountHandle = handle;
                break;
            }
        }
        // call number
        if (number == null) {
            number = "";
        }

        // presentation type
        int presentationMode;
        if (number == null || number.equals("")) {
            presentationMode = PhoneConstants.PRESENTATION_RESTRICTED;
        } else {
            presentationMode = PhoneConstants.PRESENTATION_ALLOWED;
        }

        // check is video incoming call
        int features = 0;

        // add call log
        CallLog.Calls.addCall(null, context, number,
                presentationMode, type, features,
                phoneAccountHandle,
                new Date().getTime(), 0, new Long(0));
    }


    public void handleCallAdditionalInfo(AsyncResult ar) {
        String[] callAdditionalInfo = (String[]) ar.result;
        int type = Integer.parseInt(callAdditionalInfo[0]);

        // Skip type not handled here to avoid parameter parsing error (number & callMode)
        if (type != MT_CALL_MISSED && type != MT_CALL_REJECTED && type != MT_CALL_NUMREDIRECT &&
            type != MT_CALL_GWSD) {
            logD("handleCallAdditionalInfo not handle event");
            return;
        }

        String number = callAdditionalInfo[1];
        int callMode = Integer.parseInt(callAdditionalInfo[2]);

        if (callMode != 0) { // CS Voice
            logD("handleCallAdditionalInfo unexpected callMode");
            return;
        }
        logD("handleCallAdditionalInfo" +
            " type:" + type + " mode:" + callMode);

        switch (type) {
            case MT_CALL_MISSED:
                addCallLog(mContext, mMtkTracker.mPhone.getIccSerialNumber(), number,
                        CallLog.Calls.MISSED_TYPE);
                break;
            case MT_CALL_REJECTED:
                addCallLog(mContext, mMtkTracker.mPhone.getIccSerialNumber(), number,
                        CallLog.Calls.REJECTED_TYPE);
                break;
            case MT_CALL_NUMREDIRECT:
                String redirectNumber = callAdditionalInfo[3];
                int callId = Integer.parseInt(callAdditionalInfo[4]);
                mForwardingAddress = null;
                if ((redirectNumber != null) && (redirectNumber.length() > 0)) {
                    mContainForwardingAddress = true;
                    mForwardingAddress = redirectNumber;
                    mForwardingAddressCallId = callId;
                    logD("Forwarding address: " + mForwardingAddress + "," + callId);
                }
                break;
            case MT_CALL_GWSD:
                mIsGwsdCall = true;
                break;
            default:
                break;
        }
    }

    /// @}

    public boolean isGwsdCall() {
        return mIsGwsdCall;
    }

    public void setGwsdCall(boolean isGwsd) {
        mIsGwsdCall = isGwsd;
    }


    /// M: CC: Forwarding number via EAIC @{
    /**
      *  To clear forwarding address variables
      */
    public void clearForwardingAddressVariables(int index) {
        if (mContainForwardingAddress && (mForwardingAddressCallId == (index + 1))) {
            mContainForwardingAddress = false;
            mForwardingAddress = null;
            mForwardingAddressCallId = 0;
        }
    }

    /**
      *  To clear forwarding address variables
      */
    public void setForwardingAddressToConnection(int index, Connection conn) {
        if (mContainForwardingAddress && (mForwardingAddress != null)
                && (mForwardingAddressCallId == (index + 1))) {
            ((MtkGsmCdmaConnection) conn).setForwardingAddress(mForwardingAddress);
            logD("Store forwarding address - " + mForwardingAddress);
            logD("Get forwarding address - "
                    + ((MtkGsmCdmaConnection) conn).getForwardingAddress());
            clearForwardingAddressVariables(index);
        }
    }
    /// @}

}
