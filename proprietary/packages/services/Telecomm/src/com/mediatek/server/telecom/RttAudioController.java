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

import android.content.Context;
import android.media.AudioManager;
import android.provider.Settings;
import android.telecom.Connection;
import android.telephony.TelephonyManager;

import com.android.server.telecom.Call;
import com.android.server.telecom.CallsManager;
import com.mediatek.provider.MtkSettingsExt;

/**
 * #Requirement: Mute microphone and reduce or turn-off speaker volume during RTT call,
 * if reduced voice option is selected for RTT audio mode except for RTT Emergency call.
 * #Basic rule: For Telecomm audio part, we define RTT audio mode as foreground call
 * with RTT mode. (Eg, active normal call + hold RTT call => non-RTT audio mode).
 * For mute & volume, we restore the values before entering RTT audio mode. When back to
 * non-RTT audio mode, resume mute & volume, no matter changes in RTT audio mode.
 * #Details: Two events may trigger RTT audio mode change: foreground call change & foreground
 * call's RTT mode change.
 */
public class RttAudioController {
    private static final String RTT_PROPERTY = "persist.vendor.mtk_rtt_support";

    private static final int RTT_CALL_AUDIO_MODE_NORMAL_VOICE = 0;
    private static final int RTT_CALL_AUDIO_MODE_REDUCED_VOICE = 1;

    private Context mContext;
    private CallsManager mCallsManager;
    private AudioManager mAudioManager;
    private boolean mRTTAudioMode = false;
    private boolean mSavedIsMuted = false; // Record mute status before enter RTT Audio mode.

    private final Call.Listener mCallListener = new Call.ListenerBase() {
        @Override
        public void onConnectionPropertiesChanged(Call call, boolean didRttChange) {
            updateRttAudioMode(call);
        }
    };

    public RttAudioController(Context context, CallsManager callsManager) {
        mContext = context;
        mCallsManager = callsManager;
        mAudioManager = (AudioManager) mContext.getSystemService(Context.AUDIO_SERVICE);
    }

    public void onForegroundCallChanged(Call oldCall, Call newCall) {
        Log.d(this, "onForegroundCallChanged()...");
        if (oldCall != null) {
            oldCall.removeListener(mCallListener);
        }
        if (newCall != null) {
            newCall.addListener(mCallListener);
        }
        updateRttAudioMode(newCall);
    }

    /**
     * Combine three things together to determine enter or exit RTT Audio Mode.
     */
    private void updateRttAudioMode(Call fgCall) {
        if (isRttSupport()) {
            boolean isRttSettingOn = isRttAudioSettingOn();
            boolean isRttCall = isRttCall(fgCall);
            Log.d(this, "maybeChangeRTTAudioMode()...mRttAudioMode / Setting / RttCall : "
                    + mRTTAudioMode + " / " + isRttSettingOn + " / " + isRttCall);
            if (!mRTTAudioMode && isRttSettingOn && isRttCall) {
                enterRttAudioMode();
            } else if (mRTTAudioMode && (!isRttSettingOn || !isRttCall)) {
                exitRttAudioMode();
            }
        }
    }

    private void enterRttAudioMode() {
        // Store mute state before enter RTT Audio Mode.
        mSavedIsMuted = mAudioManager.isMicrophoneMute();
        mCallsManager.getCallAudioManager().mute(true);
        mRTTAudioMode = true;
    }

    private void exitRttAudioMode() {
        // Restore mute state after exit RTT Audio Mode.
        mCallsManager.getCallAudioManager().mute(mSavedIsMuted);
        mRTTAudioMode = false;
    }

    private boolean isRttCall(Call call) {
        return call != null && !call.isEmergencyCall()
                && call.hasProperty(Connection.PROPERTY_IS_RTT);
    }

    /**
     * Read RTT audio mode value from RTT setting (Normal voice VS. Reduced voice)
     *
     * @return
     */
    private boolean isRttAudioSettingOn() {
        return Settings.Global.getInt(mContext.getContentResolver(),
                 MtkSettingsExt.Global.TELECOM_RTT_AUDIO_MODE,
                RTT_CALL_AUDIO_MODE_NORMAL_VOICE) == RTT_CALL_AUDIO_MODE_REDUCED_VOICE;
    }

    private boolean isRttSupport() {
        TelephonyManager tm = (TelephonyManager)
                mContext.getSystemService(Context.TELEPHONY_SERVICE);
        return tm.isRttSupported();
    }
}
