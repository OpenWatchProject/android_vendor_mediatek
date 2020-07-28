/*
 * Copyright (C) 2015 The Android Open Source Project
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
 * limitations under the License
 */

package com.android.server.telecom;

import android.media.AudioManager;
import android.os.Message;
/// M: Using mtk log instead of AOSP log.
// import android.telecom.Log;
import android.telecom.Logging.Runnable;
import android.telecom.Logging.Session;
import android.telecom.PhoneAccountHandle;
import android.telephony.SubscriptionManager;
import android.text.TextUtils;
import android.util.SparseArray;

import com.android.internal.telephony.PhoneConstants;
import com.android.internal.util.IState;
import com.android.internal.util.IndentingPrintWriter;
import com.android.internal.util.State;
import com.android.internal.util.StateMachine;
/// M: Mediatek import.
import com.mediatek.server.telecom.Log;
import com.mediatek.server.telecom.MtkUtil;

public class CallAudioModeStateMachine extends StateMachine {
    public static class Factory {
        public CallAudioModeStateMachine create(SystemStateHelper systemStateHelper,
                AudioManager am) {
            return new CallAudioModeStateMachine(systemStateHelper, am);
        }
    }

    public static class MessageArgs {
        public boolean hasActiveOrDialingCalls;
        public boolean hasRingingCalls;
        public boolean hasHoldingCalls;
        public boolean isTonePlaying;
        public boolean foregroundCallIsVoip;
        public Session session;

        public MessageArgs(boolean hasActiveOrDialingCalls, boolean hasRingingCalls,
                boolean hasHoldingCalls, boolean isTonePlaying, boolean foregroundCallIsVoip,
                Session session) {
            this.hasActiveOrDialingCalls = hasActiveOrDialingCalls;
            this.hasRingingCalls = hasRingingCalls;
            this.hasHoldingCalls = hasHoldingCalls;
            this.isTonePlaying = isTonePlaying;
            this.foregroundCallIsVoip = foregroundCallIsVoip;
            this.session = session;
        }

        public MessageArgs() {
            this.session = Log.createSubsession();
        }

        @Override
        public String toString() {
            return "MessageArgs{" +
                    "hasActiveCalls=" + hasActiveOrDialingCalls +
                    ", hasRingingCalls=" + hasRingingCalls +
                    ", hasHoldingCalls=" + hasHoldingCalls +
                    ", isTonePlaying=" + isTonePlaying +
                    ", foregroundCallIsVoip=" + foregroundCallIsVoip +
                    ", session=" + session +
                    '}';
        }
    }

    public static final int INITIALIZE = 1;
    // These ENTER_*_FOCUS commands are for testing.
    public static final int ENTER_CALL_FOCUS_FOR_TESTING = 2;
    public static final int ENTER_COMMS_FOCUS_FOR_TESTING = 3;
    public static final int ENTER_RING_FOCUS_FOR_TESTING = 4;
    public static final int ENTER_TONE_OR_HOLD_FOCUS_FOR_TESTING = 5;
    public static final int ABANDON_FOCUS_FOR_TESTING = 6;

    public static final int NO_MORE_ACTIVE_OR_DIALING_CALLS = 1001;
    public static final int NO_MORE_RINGING_CALLS = 1002;
    public static final int NO_MORE_HOLDING_CALLS = 1003;

    public static final int NEW_ACTIVE_OR_DIALING_CALL = 2001;
    public static final int NEW_RINGING_CALL = 2002;
    public static final int NEW_HOLDING_CALL = 2003;

    public static final int TONE_STARTED_PLAYING = 3001;
    public static final int TONE_STOPPED_PLAYING = 3002;

    public static final int FOREGROUND_VOIP_MODE_CHANGE = 4001;

    public static final int RINGER_MODE_CHANGE = 5001;

    public static final int RUN_RUNNABLE = 9001;

    private static final SparseArray<String> MESSAGE_CODE_TO_NAME = new SparseArray<String>() {{
        put(ENTER_CALL_FOCUS_FOR_TESTING, "ENTER_CALL_FOCUS_FOR_TESTING");
        put(ENTER_COMMS_FOCUS_FOR_TESTING, "ENTER_COMMS_FOCUS_FOR_TESTING");
        put(ENTER_RING_FOCUS_FOR_TESTING, "ENTER_RING_FOCUS_FOR_TESTING");
        put(ENTER_TONE_OR_HOLD_FOCUS_FOR_TESTING, "ENTER_TONE_OR_HOLD_FOCUS_FOR_TESTING");
        put(ABANDON_FOCUS_FOR_TESTING, "ABANDON_FOCUS_FOR_TESTING");
        put(NO_MORE_ACTIVE_OR_DIALING_CALLS, "NO_MORE_ACTIVE_OR_DIALING_CALLS");
        put(NO_MORE_RINGING_CALLS, "NO_MORE_RINGING_CALLS");
        put(NO_MORE_HOLDING_CALLS, "NO_MORE_HOLDING_CALLS");
        put(NEW_ACTIVE_OR_DIALING_CALL, "NEW_ACTIVE_OR_DIALING_CALL");
        put(NEW_RINGING_CALL, "NEW_RINGING_CALL");
        put(NEW_HOLDING_CALL, "NEW_HOLDING_CALL");
        put(TONE_STARTED_PLAYING, "TONE_STARTED_PLAYING");
        put(TONE_STOPPED_PLAYING, "TONE_STOPPED_PLAYING");
        put(FOREGROUND_VOIP_MODE_CHANGE, "FOREGROUND_VOIP_MODE_CHANGE");
        put(RINGER_MODE_CHANGE, "RINGER_MODE_CHANGE");
        /// M: Abandon focus state for ECC retry @{
        put(ABANDON_FOCUS, "ABANDON_FOCUS");
        /// @}

        put(RUN_RUNNABLE, "RUN_RUNNABLE");
    }};

    public static final String TONE_HOLD_STATE_NAME = OtherFocusState.class.getSimpleName();
    public static final String UNFOCUSED_STATE_NAME = UnfocusedState.class.getSimpleName();
    public static final String CALL_STATE_NAME = SimCallFocusState.class.getSimpleName();
    public static final String RING_STATE_NAME = RingingFocusState.class.getSimpleName();
    public static final String COMMS_STATE_NAME = VoipCallFocusState.class.getSimpleName();

    private class BaseState extends State {
        @Override
        public boolean processMessage(Message msg) {
            switch (msg.what) {
                case ENTER_CALL_FOCUS_FOR_TESTING:
                    transitionTo(mSimCallFocusState);
                    return HANDLED;
                case ENTER_COMMS_FOCUS_FOR_TESTING:
                    transitionTo(mVoipCallFocusState);
                    return HANDLED;
                case ENTER_RING_FOCUS_FOR_TESTING:
                    transitionTo(mRingingFocusState);
                    return HANDLED;
                case ENTER_TONE_OR_HOLD_FOCUS_FOR_TESTING:
                    transitionTo(mOtherFocusState);
                    return HANDLED;
                case ABANDON_FOCUS_FOR_TESTING:
                ///M: ALPS02797621 Ecc retry, need to back to normal mode @{
                case ABANDON_FOCUS:
                /// @}
                    transitionTo(mUnfocusedState);
                    return HANDLED;
                case INITIALIZE:
                    mIsInitialized = true;
                    return HANDLED;
                case RUN_RUNNABLE:
                    java.lang.Runnable r = (java.lang.Runnable) msg.obj;
                    r.run();
                    return HANDLED;
                default:
                    return NOT_HANDLED;
            }
        }
    }

    private class UnfocusedState extends BaseState {
        @Override
        public void enter() {
            if (mIsInitialized) {
                /// M: ALPS04897020
                /// In ECC retry, we cannot turn off speaker.
                /// Because the audio route is still speaker.
                boolean inEccRetry = true;
                Call foregroundCall = mCallAudioManager.getForegroundCall();
                if ((foregroundCall == null)
                        || (foregroundCall != null && !foregroundCall.isEmergencyCall())) {
                    inEccRetry = false;
                }
                /// M: ALPS04868053 @{
                /// Scenario:
                /// 1.Play music with headset
                /// 2.Incoming call and accept, switch to speaker
                /// 3.Hangup the call
                /// 4.The music will be heared from speaker for a while then
                /// switch to headset.
                /// Reason:
                /// Abandon audio focus before turning off speaker
                /// The music application will resume before the speaker is turn off
                /// Solution:
                /// Turn off speaker before abandoning audio focus
                if (!inEccRetry) {
                    mAudioManager.setSpeakerphoneOn(false);
                }
                /// @}
                Log.i(LOG_TAG, "Abandoning audio focus: now UNFOCUSED");
                mAudioManager.abandonAudioFocusForCall();
                /// M: ALPS04882178 @{
                /// Scenario:
                /// 1.Make a video call and the audio route will be speaker
                /// 2.End the call and make another video call immediately.
                /// Root Cause:
                /// setMode may cost time which delay the time to restore audio
                /// route to idle. Before setCallAudioRouteFocusState is called,
                /// The 2nd call's CallAudioManager.setAudioRoute(Speaker) is called.
                /// However, it will not take effect, because the current audio route
                /// is still speaker.
                /// Solution: call setMode after setCallAudioRouteFocusState
                /// mAudioManager.setMode(AudioManager.MODE_NORMAL);
                /// mMostRecentMode = AudioManager.MODE_NORMAL;
                if (MtkUtil.isInDsdaMode()) {
                    mCurFgPhoneId = getForegroundPhoneId();
                }
                ///M: Ecc retry after MD reset ALPS02797621/ALPS03016322 @{
                // Notify AudioRouteStateMachine NO_FOCUS at below conditions:
                // 1.There is no foreground call.
                // 2.Foreground call is not ECC.
                //mCallAudioManager.setCallAudioRouteFocusState(CallAudioRouteStateMachine.NO_FOCUS);
                if (!inEccRetry) {
                    mCallAudioManager.
                        setCallAudioRouteFocusState(CallAudioRouteStateMachine.NO_FOCUS);
                }
                /// @}
                /// M: ALPS04882178 @{
                mAudioManager.setMode(AudioManager.MODE_NORMAL);
                mMostRecentMode = AudioManager.MODE_NORMAL;
                /// @}
            }
        }

        @Override
        public boolean processMessage(Message msg) {
            if (super.processMessage(msg) == HANDLED) {
                return HANDLED;
            }
            MessageArgs args = (MessageArgs) msg.obj;
            switch (msg.what) {
                case NO_MORE_ACTIVE_OR_DIALING_CALLS:
                    /// M: @{ ALPS03533886
                    /// During ECC retry changed Call from DIALING TO CONNECTING, we force abandon
                    /// focus and enter UnFocusState. And then in UnFocusState's enter(), we skip
                    /// notify Audio Route state machine NO_FOCUS. If this ECC is hangup by user
                    /// at CONNECTING state, then Audio Route state machine will stay at
                    /// ActiveSpeakerRoute.
                    /// Solution: If we get NO_MORE_ACTIVE_OR_DIALING_CALLS msg in UnFocusState
                    /// and no foreground exists, notify NO_FOCUS msg to Audio Route state machine.
                    if (mCallAudioManager.getForegroundCall() == null) {
                        mCallAudioManager.
                                setCallAudioRouteFocusState(CallAudioRouteStateMachine.NO_FOCUS);
                    }
                    /// @}
                    return HANDLED;
                case NO_MORE_RINGING_CALLS:
                    // Do nothing.
                    return HANDLED;
                case NO_MORE_HOLDING_CALLS:
                    // Do nothing.
                    return HANDLED;
                case NEW_ACTIVE_OR_DIALING_CALL:
                    transitionTo(args.foregroundCallIsVoip
                            ? mVoipCallFocusState : mSimCallFocusState);
                    return HANDLED;
                case NEW_RINGING_CALL:
                    transitionTo(mRingingFocusState);
                    return HANDLED;
                case NEW_HOLDING_CALL:
                    // This really shouldn't happen, but transition to the focused state anyway.
                    Log.w(LOG_TAG, "Call was surprisingly put into hold from an unknown state." +
                            " Args are: \n" + args.toString());
                    transitionTo(mOtherFocusState);
                    return HANDLED;
                case TONE_STARTED_PLAYING:
                    // This shouldn't happen either, but perform the action anyway.
                    Log.w(LOG_TAG, "Tone started playing unexpectedly. Args are: \n"
                            + args.toString());
                    return HANDLED;
                default:
                    // The forced focus switch commands are handled by BaseState.
                    return NOT_HANDLED;
            }
        }
    }

    private class RingingFocusState extends BaseState {
        private void tryStartRinging() {
            if (mCallAudioManager.startRinging()) {
                mAudioManager.requestAudioFocusForCall(AudioManager.STREAM_RING,
                        AudioManager.AUDIOFOCUS_GAIN_TRANSIENT);
                mAudioManager.setMode(AudioManager.MODE_RINGTONE);
                mCallAudioManager.setCallAudioRouteFocusState(
                        CallAudioRouteStateMachine.RINGING_FOCUS);
            } else {
                Log.i(LOG_TAG, "RINGING state, try start ringing but not acquiring audio focus");
            }
        }

        @Override
        public void enter() {
            Log.i(LOG_TAG, "Audio focus entering RINGING state");
            tryStartRinging();
            mCallAudioManager.stopCallWaiting();
        }

        @Override
        public void exit() {
            // Audio mode and audio stream will be set by the next state.
            mCallAudioManager.stopRinging();
        }

        @Override
        public boolean processMessage(Message msg) {
            if (super.processMessage(msg) == HANDLED) {
                return HANDLED;
            }
            MessageArgs args = (MessageArgs) msg.obj;
            switch (msg.what) {
                case NO_MORE_ACTIVE_OR_DIALING_CALLS:
                    // Do nothing. Loss of an active call should not impact ringer.
                    return HANDLED;
                case NO_MORE_HOLDING_CALLS:
                    // Do nothing and keep ringing.
                    return HANDLED;
                case NO_MORE_RINGING_CALLS:
                    // If there are active or holding calls, switch to the appropriate focus.
                    // Otherwise abandon focus.
                    if (args.hasActiveOrDialingCalls) {
                        if (args.foregroundCallIsVoip) {
                            transitionTo(mVoipCallFocusState);
                        } else {
                            transitionTo(mSimCallFocusState);
                        }
                    } else if (args.hasHoldingCalls || args.isTonePlaying) {
                        transitionTo(mOtherFocusState);
                    } else {
                        transitionTo(mUnfocusedState);
                    }
                    return HANDLED;
                case NEW_ACTIVE_OR_DIALING_CALL:
                    // If a call becomes active suddenly, give it priority over ringing.
                    transitionTo(args.foregroundCallIsVoip
                            ? mVoipCallFocusState : mSimCallFocusState);
                    return HANDLED;
                case NEW_RINGING_CALL:
                    Log.w(LOG_TAG, "Unexpected behavior! New ringing call appeared while in " +
                            "ringing state.");
                    return HANDLED;
                case NEW_HOLDING_CALL:
                    // This really shouldn't happen, but transition to the focused state anyway.
                    Log.w(LOG_TAG, "Call was surprisingly put into hold while ringing." +
                            " Args are: " + args.toString());
                    transitionTo(mOtherFocusState);
                    return HANDLED;
                case RINGER_MODE_CHANGE: {
                    Log.i(LOG_TAG, "RINGING state, received RINGER_MODE_CHANGE");
                    tryStartRinging();
                    return HANDLED;
                }
                default:
                    // The forced focus switch commands are handled by BaseState.
                    return NOT_HANDLED;
            }
        }
    }

    private class SimCallFocusState extends BaseState {
        @Override
        public void enter() {
            Log.i(LOG_TAG, "Audio focus entering SIM CALL state");
            // Google default code:
//          mAudioManager.requestAudioFocusForCall(AudioManager.STREAM_VOICE_CALL,
//                  AudioManager.AUDIOFOCUS_GAIN_TRANSIENT);
//          mAudioManager.setMode(AudioManager.MODE_IN_CALL);
//          mMostRecentMode = AudioManager.MODE_IN_CALL;
//          mCallAudioManager.setCallAudioRouteFocusState(CallAudioRouteStateMachine.ACTIVE_FOCUS);
//
            mAudioManager.requestAudioFocusForCall(AudioManager.STREAM_VOICE_CALL,
                    AudioManager.AUDIOFOCUS_GAIN_TRANSIENT);
            Log.d(LOG_TAG, "start to set has_foucs");
            mCallAudioManager.setCallAudioRouteFocusState(
                    CallAudioRouteStateMachine.ACTIVE_FOCUS);
            Log.d(LOG_TAG, "finish to set has_focus");

            /// M: @{
            // support SVLTE audio mode
            if (MtkUtil.isInDsdaMode()) {
                mPreFgPhoneId = mCurFgPhoneId;
                mCurFgPhoneId = getForegroundPhoneId();
                Log.d(LOG_TAG, "mPreFgPhoneId=" + mPreFgPhoneId
                        + " mCurFgPhoneId=" + mCurFgPhoneId);
                // add mode adjugement to deal some rare cases. ex.
                // hold CDMA call will set mode as normal in OtherFocusState
                // unhold CDMA call enters this sim state,
                // the foreground phone id is not changed
                if (mPreFgPhoneId != mCurFgPhoneId
                        || mAudioManager.getMode() != AudioManager.MODE_IN_CALL) {
                    if (mCurFgPhoneId == SubscriptionManager.INVALID_PHONE_INDEX) {
                        Log.d(LOG_TAG, "invalid mCurFgPhoneId, return, wait next time trigger");
                        return;
                    }
                    if (mAudioManager.getMode() == AudioManager.MODE_IN_CALL) {
                        Log.d(LOG_TAG, "reset normal first");
                        mAudioManager.setMode(AudioManager.MODE_NORMAL);
                    }

                    // To support DSDA, we use AudioManager.setParameter(foregroundPhoneId)
                    // to notify audio hal to open corresponding MD speech driver.
                    mAudioManager.setParameters("ForegroundPhoneId=" + mCurFgPhoneId);
                    mAudioManager.setMode(AudioManager.MODE_IN_CALL);
                    mMostRecentMode = AudioManager.MODE_IN_CALL;
                    ///M: @{
                    // background: mute will auto cancel when set audio mode as normal_mode
                    // scenario: (1) only cdma call exist, hold call-->unhold call
                    // (2) Gsm/Cdma call swap
                    if (mCallAudioManager.getCallAudioState().isMuted()) {
                        Log.d(LOG_TAG, "restore mute state after set audio mode to in call!");
                        mCallAudioManager.restoreMuteOnWhenInCallMode();
                    }
                    /// @}
                }
                /// @}
            } else {
                Log.d(LOG_TAG, "start to set mode");
                mAudioManager.setMode(AudioManager.MODE_IN_CALL);
                Log.d(LOG_TAG, "finish to set mode");
                mMostRecentMode = AudioManager.MODE_IN_CALL;
            }
        }

        @Override
        public boolean processMessage(Message msg) {
            if (super.processMessage(msg) == HANDLED) {
                return HANDLED;
            }
            MessageArgs args = (MessageArgs) msg.obj;
            switch (msg.what) {
                case NO_MORE_ACTIVE_OR_DIALING_CALLS:
                    // Switch to either ringing, holding, or inactive
                    transitionTo(destinationStateAfterNoMoreActiveCalls(args));
                    return HANDLED;
                case NO_MORE_RINGING_CALLS:
                    // Don't transition state, but stop any call-waiting tones that may have been
                    // playing.
                    if (args.isTonePlaying) {
                        mCallAudioManager.stopCallWaiting();
                    }
                    // If a MT-audio-speedup call gets disconnected by the connection service
                    // concurrently with the user answering it, we may get this message
                    // indicating that a ringing call has disconnected while this state machine
                    // is in the SimCallFocusState.
                    if (!args.hasActiveOrDialingCalls) {
                        transitionTo(destinationStateAfterNoMoreActiveCalls(args));
                    }
                    return HANDLED;
                case NO_MORE_HOLDING_CALLS:
                    // Do nothing.
                    return HANDLED;
                case NEW_ACTIVE_OR_DIALING_CALL:
                    /// M:  ALPS04805979 Fix audio mode error in whatsapp video call then sim
                    /// vilte incoming call case.
                    /// Switch state again when receiving NEW_ACTIVE_OR_DIALING_CALL to ensure
                    /// audio mode state correctly. @{
                    if (args.foregroundCallIsVoip) {
                        transitionTo(mVoipCallFocusState);
                    }
                    /// @}
                    return HANDLED;
                case NEW_RINGING_CALL:
                    // Don't make a call ring over an active call, but do play a call waiting tone.
                    mCallAudioManager.startCallWaiting("call already active");
                    return HANDLED;
                case NEW_HOLDING_CALL:
                    // Don't do anything now. Putting an active call on hold will be handled when
                    // NO_MORE_ACTIVE_CALLS is processed.
                    return HANDLED;
                case FOREGROUND_VOIP_MODE_CHANGE:
                    if (args.foregroundCallIsVoip) {
                        transitionTo(mVoipCallFocusState);
                    }
                    return HANDLED;
                /// M: For 2A -> 1A
                case FOREGROUND_MODE_CHANGE:
                    transitionTo(mSimCallFocusState);
                    return HANDLED;
                default:
                    // The forced focus switch commands are handled by BaseState.
                    return NOT_HANDLED;
            }
        }
    }

    private class VoipCallFocusState extends BaseState {
        @Override
        public void enter() {
            Log.i(LOG_TAG, "Audio focus entering VOIP CALL state");
            mAudioManager.requestAudioFocusForCall(AudioManager.STREAM_VOICE_CALL,
                    AudioManager.AUDIOFOCUS_GAIN_TRANSIENT);
            mAudioManager.setMode(AudioManager.MODE_IN_COMMUNICATION);
            mMostRecentMode = AudioManager.MODE_IN_COMMUNICATION;
            mCallAudioManager.setCallAudioRouteFocusState(CallAudioRouteStateMachine.ACTIVE_FOCUS);
        }

        @Override
        public boolean processMessage(Message msg) {
            if (super.processMessage(msg) == HANDLED) {
                return HANDLED;
            }
            MessageArgs args = (MessageArgs) msg.obj;
            switch (msg.what) {
                case NO_MORE_ACTIVE_OR_DIALING_CALLS:
                    // Switch to either ringing, holding, or inactive
                    transitionTo(destinationStateAfterNoMoreActiveCalls(args));
                    return HANDLED;
                case NO_MORE_RINGING_CALLS:
                    // Don't transition state, but stop any call-waiting tones that may have been
                    // playing.
                    if (args.isTonePlaying) {
                        mCallAudioManager.stopCallWaiting();
                    }
                    return HANDLED;
                case NO_MORE_HOLDING_CALLS:
                    // Do nothing.
                    return HANDLED;
                case NEW_ACTIVE_OR_DIALING_CALL:
                    /// M:  ALPS04805979 Fix audio mode error in whatsapp video call then sim
                    /// vilte incoming call case.
                    /// Cause:
                    /// 1. Accept whatsapp call then call audio mode switch to VoipCallFocusState.
                    /// Current audio mode is AudioManager.MODE_IN_COMMUNICATION.
                    /// 2. Sim incoming call then receiving FOREGROUND_VOIP_MODE_CHANGE, because
                    /// foreground call is still whatsapp call, do nothing.
                    /// 3. Accept sim call then receiving NEW_ACTIVE_OR_DIALING_CALL, but do
                    /// nothing, so call audio mode will stay in VoipCallFocusState, has no chance
                    /// switch to SimCallFocusState. Since telecom has no set audio mode to
                    /// AudioManager.MODE_IN_CALL, this sim vilte incoming call will accept
                    /// failed finally.
                    /// Solution:
                    /// Switch state again when receiving NEW_ACTIVE_OR_DIALING_CALL to ensure
                    /// audio mode state correctly. @{
                    if (!args.foregroundCallIsVoip) {
                        transitionTo(mSimCallFocusState);
                    }
                    /// @}
                    return HANDLED;
                case NEW_RINGING_CALL:
                    // Don't make a call ring over an active call, but do play a call waiting tone.
                    mCallAudioManager.startCallWaiting("call already active");
                    return HANDLED;
                case NEW_HOLDING_CALL:
                    // Don't do anything now. Putting an active call on hold will be handled when
                    // NO_MORE_ACTIVE_CALLS is processed.
                    return HANDLED;
                case FOREGROUND_VOIP_MODE_CHANGE:
                    if (!args.foregroundCallIsVoip) {
                        transitionTo(mSimCallFocusState);
                    }
                    return HANDLED;
                default:
                    // The forced focus switch commands are handled by BaseState.
                    return NOT_HANDLED;
            }
        }
    }

    /**
     * This class is used for calls on hold and end-of-call tones.
     */
    private class OtherFocusState extends BaseState {
        @Override
        public void enter() {
            Log.i(LOG_TAG, "Audio focus entering TONE/HOLDING state");
            mAudioManager.requestAudioFocusForCall(AudioManager.STREAM_VOICE_CALL,
                    AudioManager.AUDIOFOCUS_GAIN_TRANSIENT);

            /// M: hold c2k call, need to set audio as normal mode @{
            if (MtkUtil.isInDsdaMode()) {
                Call foregroundCall = mCallAudioManager.getPossiblyHeldForegroundCall();
                if (foregroundCall != null && foregroundCall.getState() == CallState.ON_HOLD
                        && foregroundCall.isCdma()) {
                    mMostRecentMode = AudioManager.MODE_NORMAL;
                    Log.d(LOG_TAG, "reset mCurFgPhoneId in OtherFocusState");
                    mCurFgPhoneId = SubscriptionManager.INVALID_PHONE_INDEX;
                }
            }
            /// @}

            mAudioManager.setMode(mMostRecentMode);
            mCallAudioManager.setCallAudioRouteFocusState(CallAudioRouteStateMachine.ACTIVE_FOCUS);
        }

        @Override
        public boolean processMessage(Message msg) {
            if (super.processMessage(msg) == HANDLED) {
                return HANDLED;
            }
            MessageArgs args = (MessageArgs) msg.obj;
            switch (msg.what) {
                case NO_MORE_HOLDING_CALLS:
                    if (args.hasActiveOrDialingCalls) {
                        transitionTo(args.foregroundCallIsVoip
                                ? mVoipCallFocusState : mSimCallFocusState);
                    } else if (args.hasRingingCalls) {
                        transitionTo(mRingingFocusState);
                    } else if (!args.isTonePlaying) {
                        transitionTo(mUnfocusedState);
                    }
                    // Do nothing if a tone is playing.
                    return HANDLED;
                case NEW_ACTIVE_OR_DIALING_CALL:
                    transitionTo(args.foregroundCallIsVoip
                            ? mVoipCallFocusState : mSimCallFocusState);
                    return HANDLED;
                case NEW_RINGING_CALL:
                    // TODO: consider whether to move this into MessageArgs if more things start
                    // to use it.
                    if (args.hasHoldingCalls && mSystemStateHelper.isDeviceAtEar()) {
                        mCallAudioManager.startCallWaiting(
                                "Device is at ear with held call");
                    } else {
                        transitionTo(mRingingFocusState);
                    }
                    return HANDLED;
                case NEW_HOLDING_CALL:
                    // Do nothing.
                    return HANDLED;
                case NO_MORE_RINGING_CALLS:
                    // If there are no more ringing calls in this state, then stop any call-waiting
                    // tones that may be playing.
                    mCallAudioManager.stopCallWaiting();
                    /// M:  ALPS03582350 @{
                    // Issue scenario step by step:
                    // 1. SIM1 C and SIM2 G in dsda project, SIM1 has actvie call.
                    // 2. SIM2 incomming call, accept this call but hung up it immidiately by
                    // remote phone.
                    // 3. Device still has sound when SIM1 hold call state.
                    // Root cause:
                    // In step 2, SIM1 call state: ACTIVE->ON_HLOD then SIM2 call state:
                    // RING->DISCONNECTED. CallAudioMode: SimCallFocusState->OtherFocusState.
                    // Usually, hold c2k call need to set audio as normal mode in dsda mode.
                    // but in this case, when SIM1 Call enter hold state, foreground call will
                    // change to SIM2 Call, finnaly it did not set audio mode to normal when
                    // enter OtherFocusState. If we do not set audio mode to normal, the cdma
                    // on hold call will still has sound.
                    // Solution:
                    // Enter OtherFocusState again for set audio as normal mode when ringing
                    // call disconnected.
                    transitionTo(destinationStateAfterNoMoreActiveCalls(args));
                    /// @}
                    return HANDLED;
                case TONE_STOPPED_PLAYING:
                    transitionTo(destinationStateAfterNoMoreActiveCalls(args));
                default:
                    return NOT_HANDLED;
            }
        }
    }

    private static final String LOG_TAG = CallAudioModeStateMachine.class.getSimpleName();

    private final BaseState mUnfocusedState = new UnfocusedState();
    private final BaseState mRingingFocusState = new RingingFocusState();
    private final BaseState mSimCallFocusState = new SimCallFocusState();
    private final BaseState mVoipCallFocusState = new VoipCallFocusState();
    private final BaseState mOtherFocusState = new OtherFocusState();

    private final AudioManager mAudioManager;
    private final SystemStateHelper mSystemStateHelper;
    private CallAudioManager mCallAudioManager;

    private int mMostRecentMode;
    private boolean mIsInitialized = false;

    public CallAudioModeStateMachine(SystemStateHelper systemStateHelper,
            AudioManager audioManager) {
        super(CallAudioModeStateMachine.class.getSimpleName());
        mAudioManager = audioManager;
        mSystemStateHelper = systemStateHelper;
        mMostRecentMode = AudioManager.MODE_NORMAL;

        addState(mUnfocusedState);
        addState(mRingingFocusState);
        addState(mSimCallFocusState);
        addState(mVoipCallFocusState);
        addState(mOtherFocusState);
        setInitialState(mUnfocusedState);
        start();
        sendMessage(INITIALIZE, new MessageArgs());
    }

    public void setCallAudioManager(CallAudioManager callAudioManager) {
        mCallAudioManager = callAudioManager;
    }

    public String getCurrentStateName() {
        IState currentState = getCurrentState();
        return currentState == null ? "no state" : currentState.getName();
    }

    public void sendMessageWithArgs(int messageCode, MessageArgs args) {
        sendMessage(messageCode, args);
    }

    @Override
    protected void onPreHandleMessage(Message msg) {
        if (msg.obj != null && msg.obj instanceof MessageArgs) {
            Log.continueSession(((MessageArgs) msg.obj).session, "CAMSM.pM_" + msg.what);
            Log.i(LOG_TAG, "Message received: %s.", MESSAGE_CODE_TO_NAME.get(msg.what));
        } else if (msg.what == RUN_RUNNABLE && msg.obj instanceof Runnable) {
            Log.i(LOG_TAG, "Running runnable for testing");
        } else {
                Log.w(LOG_TAG, "Message sent must be of type nonnull MessageArgs, but got " +
                        (msg.obj == null ? "null" : msg.obj.getClass().getSimpleName()));
                Log.w(LOG_TAG, "The message was of code %d = %s",
                        msg.what, MESSAGE_CODE_TO_NAME.get(msg.what));
        }
    }

    public void dumpPendingMessages(IndentingPrintWriter pw) {
        getHandler().getLooper().dump(pw::println, "");
    }

    @Override
    protected void onPostHandleMessage(Message msg) {
        Log.endSession();
    }

    private BaseState destinationStateAfterNoMoreActiveCalls(MessageArgs args) {
        if (args.hasHoldingCalls) {
            return mOtherFocusState;
        } else if (args.hasRingingCalls) {
            return mRingingFocusState;
        } else if (args.isTonePlaying) {
            return mOtherFocusState;
        } else {
            return mUnfocusedState;
        }
    }

    public static final int ABANDON_FOCUS = 10001;
    /// M: For 2A -> 1A @{
    public static final int FOREGROUND_MODE_CHANGE = 4501;
    private int mPreFgPhoneId = SubscriptionManager.INVALID_PHONE_INDEX;
    private int mCurFgPhoneId = SubscriptionManager.INVALID_PHONE_INDEX;

    private boolean isInCallMode(int mode) {
        return mode == AudioManager.MODE_IN_CALL;
    }

    /**
     * M: get foreground call phone id
     * @return the phone id
     */
    private int getForegroundPhoneId() {
        Log.d(LOG_TAG, "enter getForegroundPhoneId " +
                "isDsda: " + MtkUtil.isInDsdaMode());
        int phoneId = SubscriptionManager.INVALID_PHONE_INDEX;
        if (MtkUtil.isInDsdaMode()) {
            Call call = mCallAudioManager.getForegroundCall();
            /* Do not need to switch in call mode if the foreground call is not alive */
            Log.d(LOG_TAG, "getForegroundCall, call:" + call);
            //android.telecom.Call.Details.CAPABILITY_SPEED_UP_MT_AUDIO capability
            //is used in InCallUI. Connection.CAPABILITY_SPEED_UP_MT_AUDIO is used in Telecom call
            //these two capability constant values are 0x00040000.
            //AOSP CallAudioManager.onIncomingCallAnswered() might use wrong variable.
            //here use the same variable to accordance with CallAudioManager.java
            if (call != null
                    && (call.isAlive()
                    || call.can(android.telecom.Call.Details.CAPABILITY_SPEED_UP_MT_AUDIO))) {
                phoneId = getPhoneId(call.getTargetPhoneAccount());
            }
        }
        Log.d(LOG_TAG, "foreground phoneId = " + phoneId);
        return phoneId;
    }

    /**
     * M: Since the getId() of PhoneAccountHandle not stands for
     * subId any more in m0, we need to calculate the subId based
     * on the PhoneAccountHandle instance.
     * @param handle
     * @return
     */
    private int getPhoneId(PhoneAccountHandle handle) {
        Log.d(LOG_TAG, "getPhoneId, handle:" + handle);
        if (handle == null) {
            return SubscriptionManager.INVALID_PHONE_INDEX;
        }
        ///M: when no sim cards inserted, we will pass the phoneId
        // in mId. @{
        if (TextUtils.isDigitsOnly(handle.getId())
                && handle.getId().length() < 2) {
            return Integer.parseInt(handle.getId());
        }
        /// @}
        int subId = MtkUtil.getSubIdForPhoneAccountHandle(handle);
        Log.d(LOG_TAG, "getPhoneId, subId = " + subId);
        return SubscriptionManager.getPhoneId(subId);
    }
}
