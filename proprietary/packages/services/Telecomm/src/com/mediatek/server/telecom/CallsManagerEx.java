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

package com.mediatek.server.telecom;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.Uri;
import android.os.Bundle;
import android.os.PersistableBundle;
import android.provider.Settings;
import android.telecom.PhoneAccount;
import android.telecom.PhoneAccountHandle;
import android.telecom.TelecomManager;
import android.telecom.VideoProfile;
import android.telephony.CarrierConfigManager;
import android.telephony.PhoneNumberUtils;
import android.telephony.ServiceState;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;

import com.android.internal.annotations.VisibleForTesting;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;
import com.android.server.telecom.Call;
import com.android.server.telecom.CallState;
import com.android.server.telecom.CallsManager;
import com.android.server.telecom.R;
import com.android.server.telecom.components.ErrorDialogActivity;

import com.mediatek.provider.MtkSettingsExt;

import mediatek.telephony.MtkCarrierConfigManager;
import mediatek.telecom.MtkPhoneAccount;

import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.concurrent.ConcurrentHashMap;
import java.util.stream.Collectors;

/**
 * Singleton.
 *
 * Handle MTK CallsManager extend features.
 */
@VisibleForTesting
public class CallsManagerEx {

    private static final String TAG = "CallsManagerEx";
    private final CallsManager mCallsManager;
    private final Context mContext;
    /// M: MTK Sprint voice roaming feature @{
    private boolean mCanMakeCall = false;
    private static final Map<Integer, Integer> mVoiceRoamingInfoMap =
            new ConcurrentHashMap<Integer, Integer>();
    /// @}

    // RTT util handler.
    private final RttUtilExt mRttUtilExt;

    public CallsManagerEx(CallsManager callsManager, Context context) {
        mCallsManager = callsManager;
        mContext = context;

        /// M: MTK Sprint voice roaming feature @{
        IntentFilter filter = new IntentFilter();
        filter.addAction(TelephonyIntents.ACTION_SERVICE_STATE_CHANGED);
        filter.addAction(ErrorDialogActivity.ACTION_MAKE_CALL);
        mContext.registerReceiver(mReceiverRoaming, filter);
        /// @}

        mRttUtilExt = new RttUtilExt(context, callsManager);
    }

    /** M: Check whether should resume hold call for specific custimization. */
    public boolean shouldResumeHoldCall(Call call) {
        boolean resume = MtkPhoneAccount.hasCarrierCapabilities(call.getCarrierCapabilities(),
                MtkPhoneAccount
                .CARRIER_CAPABILITY_RESUME_HOLD_CALL_AFTER_ACTIVE_CALL_END_BY_REMOTE);
        Log.v(TAG, "shouldResumeHoldCall = " + resume);
        return resume;
    }

    /** M: Check if allow to disconnect all calls by carrier config. */
    public boolean shouldDisconnectCallsWhenEccByCarrierConfig(Call call) {
        boolean ret = !MtkPhoneAccount.hasCarrierCapabilities(call.getCarrierCapabilities(),
                MtkPhoneAccount.CARRIER_CAPABILITY_DISABLE_DISCONNECT_ALL_CALLS_WHEN_ECC);
        Log.i(this, "shouldDisconnectCallsWhenEccByCarrierConfig " + ret);
        return ret;
    }


    /** M: Check whether supports auto upgrade to RTT call. */
    public boolean shouldAutoUpgradeToRttCallByCarrierConfig(Call call) {
        boolean upgrade = !MtkPhoneAccount.hasCarrierCapabilities(call.getCarrierCapabilities(),
                MtkPhoneAccount.CARRIER_CAPABILITY_DISABLE_RTT_AUTO_UPGRADE);
        Log.v(TAG, "shouldAutoUpgradeToRttCallByCarrierConfig = " + upgrade);
        return upgrade;
    }

    /**
     * M: check if video call over wifi is allowed or not and downgrade it to voice.
     *
     * @param call outgoing call object.
     * @param videoState outgoing call videoState.
     * @return int the changed videoState of call.
     */
    public int checkForVideoCallOverWifi(Call call, int videoState) {
        Log.v(TAG, "checkForVideoCallOverWifi call = " + call);
        if (call == null) {
            return videoState;
        }
        Log.v(TAG, "checkForVideoCallOverWifi videoState=" + videoState);
        if (!VideoProfile.isVideo(videoState)) {
            return videoState;
        }
        final TelephonyManager telephonyManager = TelephonyManager.from(mContext);
        boolean isWfcEnabled = telephonyManager.isWifiCallingAvailable();
        Log.v(TAG, "checkForVideoCallOverWifi WFC=" + isWfcEnabled);
        if (!isWfcEnabled) {
            return videoState;
        }
        if (call.getTargetPhoneAccount() == null) {
            return videoState;
        }
        boolean isVideoAllowed = isVideoCallOverWifiAllowed(call);
        if (!isVideoAllowed) {
            MtkTelecomGlobals.getInstance()
                        .showToast(R.string.video_over_wifi_not_available);
            call.setVideoState(VideoProfile.STATE_AUDIO_ONLY);
            return VideoProfile.STATE_AUDIO_ONLY;
        }
        return videoState;
    }

    /// M: Customize for specific operator, block outgoning call in roaming @{
    public boolean blockOutgoingCallInRoaming(Context context, Uri handle, Call call,
            Bundle extras) {
        int subId = mCallsManager.getPhoneAccountRegistrar()
                .getSubscriptionIdForPhoneAccount(call.getTargetPhoneAccount());
        Log.d(TAG, "blockOutgoingCallInRoaming subId = " + subId);
        if (!mVoiceRoamingInfoMap.containsKey(subId)
                || subId == SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
            return false;
        }
        Log.d(this, "blockOutgoingCallInRoaming mVoiceRoaming = "
                + mVoiceRoamingInfoMap.get(subId).toString());
        if (mCanMakeCall) {
            Log.d(TAG, "make roaming call");
            mCanMakeCall = false;
            return false;
        }

        if (mVoiceRoamingInfoMap.get(subId).intValue() == ServiceState.ROAMING_TYPE_NOT_ROAMING) {
            Log.d(TAG, "not in roaming, return");
            return false;
        }

        if (!operatorCheck(call)) {
            Log.d(this, "Not Valid for this OP MCC/MNC, no handling further");
            return false;
        }

        String contactNumber = getContactNumber(handle);
        if (PhoneNumberUtils.isEmergencyNumber(contactNumber)) {
            Log.d(TAG, "emergency number, return");
            return false;
        }

        int videoState = VideoProfile.STATE_AUDIO_ONLY;
        if (extras != null) {
            videoState = extras.getInt(TelecomManager.EXTRA_START_CALL_WITH_VIDEO_STATE,
                    VideoProfile.STATE_AUDIO_ONLY);
        }

        int domesticVoiceRoamingButton = Settings.Global.getInt(context.getContentResolver(),
                MtkSettingsExt.Global.DOMESTIC_VOICE_TEXT_ROAMING + subId, 0);
        int internationalVoiceRoamingButton = Settings.Global.getInt(context.getContentResolver(),
                MtkSettingsExt.Global.INTERNATIONAL_VOICE_TEXT_ROAMING + subId, 0);

        int domesticVoiceRoamingGuard = Settings.Global.getInt(context.getContentResolver(),
                MtkSettingsExt.Global.DOMESTIC_VOICE_TEXT_ROAMING_GUARD + subId, 0);
        int internationalVoiceRoamingGuard = Settings.Global.getInt(context.getContentResolver(),
                MtkSettingsExt.Global.INTERNATIONAL_VOICE_ROAMING_GUARD + subId, 0);

        Log.d(TAG, "domesticVoiceRoamingButton = " + domesticVoiceRoamingButton +
                " internationalVoiceRoamingButton = " + internationalVoiceRoamingButton +
                " domesticVoiceRoamingGuard = " + domesticVoiceRoamingGuard +
                " internationalVoiceRoamingGuard = " + internationalVoiceRoamingGuard +
                "SubId = " + subId +
                "blockOutgoingCall, videoState =" + videoState);

        if ((mVoiceRoamingInfoMap.get(subId).intValue() == ServiceState.ROAMING_TYPE_DOMESTIC
                && domesticVoiceRoamingButton == 1 && domesticVoiceRoamingGuard == 1)
                || (mVoiceRoamingInfoMap.get(subId).intValue()
                        == ServiceState.ROAMING_TYPE_INTERNATIONAL
                && internationalVoiceRoamingButton == 1 && internationalVoiceRoamingGuard == 1)) {
            Log.d(TAG, "show roaming guard");
            Intent intent = new Intent(context, ErrorDialogActivity.class);
            intent.putExtra(ErrorDialogActivity.SHOW_ROAMING_ALERT_DIALOG, true);
            if (mVoiceRoamingInfoMap.get(subId).intValue() == ServiceState.ROAMING_TYPE_DOMESTIC) {
                intent.putExtra(ErrorDialogActivity.IS_DOMESTIC_ROAMING, true);
            }
            intent.putExtra(ErrorDialogActivity.IS_VIDEO_CALL, VideoProfile.isVideo(videoState));
            intent.putExtra(ErrorDialogActivity.CONTACT_NUMBER, contactNumber);
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            context.startActivity(intent);
            return true;
        }
        return false;
    }

    private boolean operatorCheck(Call call) {
        boolean blockOutgoingCall = MtkPhoneAccount.hasCarrierCapabilities(
                call.getCarrierCapabilities(),
                MtkPhoneAccount.CARRIER_CAPABILITY_ROAMING_BAR_GUARD);
        Log.d(this, "blockOutgoingCall: %s" + blockOutgoingCall);
        return blockOutgoingCall;
    }

    private String getContactNumber(Uri handle) {
        String contNumber = "";
        if (handle != null) {
            contNumber = handle.getSchemeSpecificPart();
        }
        if (!TextUtils.isEmpty(contNumber)) {
            boolean isUriNumber = PhoneNumberUtils.isUriNumber(contNumber);
            if (!isUriNumber) {
                contNumber = PhoneNumberUtils.convertKeypadLettersToDigits(contNumber);
                contNumber = PhoneNumberUtils.stripSeparators(contNumber);
            }
        }
        return contNumber;
    }

    private BroadcastReceiver mReceiverRoaming = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (intent.getAction().equals(TelephonyIntents.ACTION_SERVICE_STATE_CHANGED)) {
                int subId = intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY,
                        SubscriptionManager.INVALID_SUBSCRIPTION_ID);
                ServiceState serviceState = ServiceState.newFromBundle(intent.getExtras());
                if (subId != SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
                    mVoiceRoamingInfoMap.put(subId, serviceState.getVoiceRoamingType());
                    Log.d(TAG, "broadcast receiver onReceive roaming value = " +
                            serviceState.getVoiceRoamingType() + "subid=" + subId);
                }
            } else if (intent.getAction().equals(ErrorDialogActivity.ACTION_MAKE_CALL)) {
                mCanMakeCall = true;
            }
        }
    };
    /// @}

    /**
     * M: check whether video call over wifi is allowed.
     *
     * @param call outgoing call object.
     * @return true if allow video call over wifi, false otherwise.
     */
    private boolean isVideoCallOverWifiAllowed(Call call) {
        boolean allowVideoCallOverWifi = !MtkPhoneAccount.hasCarrierCapabilities(
                call.getCarrierCapabilities(),
                MtkPhoneAccount.CARRIER_CAPABILITY_DISABLE_VT_OVER_WIFI);
        Log.v(TAG, "calling isVideoCallOverWifiAllowed " + allowVideoCallOverWifi);
        return allowVideoCallOverWifi;
    }

    /**
     * M: check whether outgoing call is allowed during conference call.
     *
     * @param accountHandle the PhoneAccountHandle to check.
     * @return true if should disallow outgoing call during conference call.
     */
    public boolean shouldDisallowOutgoingCallsDuringConference(Call call) {
        boolean disallowMoCallDuringConfernence = MtkPhoneAccount.hasCarrierCapabilities(
                call.getCarrierCapabilities(),
                MtkPhoneAccount.CARRIER_CAPABILITY_DISABLE_MO_CALL_DURING_CONFERENCE);
        Log.v(TAG, "calling shouldDisallowOutgoingCallsDuringConference " +
                disallowMoCallDuringConfernence);
        return disallowMoCallDuringConfernence;
    }

    /**
     * Get RTT util handler.
     *
     * @return RttUtilExt object.
     */
    public RttUtilExt getRttUtilExt() {
        return mRttUtilExt;
    }

    /**
     * M: Need disconnect another phone account`s calls when answer call in 5G DSDA mode.
     * @param incomingCall incoming call object.
     * @param videoState The video state of the incomingCall.
     * @return true if the ringing call will be answered via WaitingCallAction.
     */
    public boolean disconnectCallInDiffPhoneAccountAndAnswerRinging(Call incomingCall,
            int videoState) {
       if (MtkUtil.isInDsdaMode()) {
           return false;
       }

       boolean ret = false;
       List<Call> calls = getNonEccPstnCallInDiffPhoneAccount(incomingCall);
       for (Call call : calls) {
           if (call.getState() == CallState.RINGING) {
               call.reject(false, null);
           } else {
               call.disconnect();
           }
           if (call.getState() == CallState.ACTIVE) {
               mCallsManager.addWaitingCallAction(call, incomingCall,
                       CallsManager.WaitingCallAction.WAITING_ACTION_ANSWER, videoState);
               ret = true;
           }
       }
       return ret;
    }

    /**
     * M: Check whether c0 phone account and c1 phone account are different pstn phone account.
     * @param c0 the call object.
     * @param c1 the call object.
     * @return true if c0 phone account and c1 phone account are different pstn phone account.
     */
    public boolean areDiffPstnPhoneAccount(Call c0, Call c1) {
       return isPstnPhoneAccount(c0) && isPstnPhoneAccount(c1)
               && !Objects.equals(c0.getTargetPhoneAccount(), c1.getTargetPhoneAccount());
    }

    private List<Call> getNonEccPstnCallInDiffPhoneAccount(Call incomingCall) {
        if (!isPstnPhoneAccount(incomingCall)) {
            return Collections.EMPTY_LIST;
        }

        List<Call> calls = mCallsManager.getCalls().stream()
                .filter(call -> !Objects.equals(call.getTargetPhoneAccount(),
                        incomingCall.getTargetPhoneAccount()) && !call.isEmergencyCall()
                        && (isPstnPhoneAccount(call) || call.getTargetPhoneAccount() == null))
                .collect(Collectors.toList());
        return calls;
    }

    private boolean isPstnPhoneAccount(Call call) {
        PhoneAccount phoneAccount = mCallsManager.getPhoneAccountRegistrar()
                .getPhoneAccountUnchecked(call.getTargetPhoneAccount());
        return phoneAccount != null
                && phoneAccount.hasCapabilities(PhoneAccount.CAPABILITY_SIM_SUBSCRIPTION);
    }

    /** M: Check whether should resume hold call for specific customization. */
    public boolean shouldDisallowOutgoingCallsDuringVideoOrVoiceCall(Call call) {

        boolean ret = MtkPhoneAccount.hasCarrierCapabilities(call.getCarrierCapabilities(),
                MtkPhoneAccount.CARRIER_CAPABILITY_DISALLOW_OUTGOING_CALLS_DURING_VIDEO_OR_VOICE_CALL);
        Log.v(TAG, "shouldDisallowOutgoingCallsDuringVideoOrVoiceCall = " + ret);
        return ret;
    }

    /** M: Check whether should reject call without forward for Softbank requirement. */
    public boolean shouldRejectCallWithoutForward(Call call) {
        boolean ret = MtkPhoneAccount.hasCarrierCapabilities(call.getCarrierCapabilities(),
                MtkPhoneAccount.CARRIER_CAPABILITY_SUPPORT_MTK_ENHANCED_CALL_BLOCKING);
        Log.v(TAG, "shouldDisallowOutgoingCallsDuringVideoOrVoiceCall = " + ret);
        return ret;
    }
}
