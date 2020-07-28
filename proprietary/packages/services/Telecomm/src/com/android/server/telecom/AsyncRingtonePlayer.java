/*
 * Copyright 2014, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.server.telecom;

import android.annotation.NonNull;
import android.annotation.Nullable;
import android.media.AudioAttributes;
import android.media.Ringtone;
import android.media.VolumeShaper;
import android.net.Uri;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Message;
/// M: Using mtk log instead of AOSP log.
// import android.telecom.Log;

import com.android.internal.annotations.VisibleForTesting;
import com.android.internal.os.SomeArgs;
import com.android.internal.util.Preconditions;

/// M: Mediatek import.
import com.mediatek.server.telecom.Log;

import java.util.concurrent.CompletableFuture;

/**
 * Plays the default ringtone. Uses {@link Ringtone} in a separate thread so that this class can be
 * used from the main thread.
 */
@VisibleForTesting
public class AsyncRingtonePlayer {
    // Message codes used with the ringtone thread.
    private static final int EVENT_PLAY = 1;
    private static final int EVENT_STOP = 2;
    private static final int EVENT_REPEAT = 3;
    /// M: Add event to handle looping play.
    private static final int EVENT_LOOPING = 4;

    // The interval in which to restart the ringer.
    private static final int RESTART_RINGER_MILLIS = 3000;

    /// @{ Fix Ringtone conflict with line ALPS03524486
    /// The internal in which to play the ringtone.
    private static final int POST_PLAY_RINGER_MILLIS = 200;
    /// @}

    /** Handler running on the ringtone thread. */
    private Handler mHandler;

    /** The current ringtone. Only used by the ringtone thread. */
    private Ringtone mRingtone;

    /**
     * CompletableFuture which signals a caller when we know whether a ringtone will play haptics
     * or not.
     */
    private CompletableFuture<Boolean> mHapticsFuture = null;

    /**
     * ISSUE:ALPS04813751
     * In some kind of rare condition, mStopHapticsFuture's value will can not be synchronized in
     * time. So, using modifier(volatile) to make sure that the variable can be synchronized
     * immediately.
     */
    private volatile CompletableFuture<Boolean>  mStopHapticsFuture = null;
    /**
     * Determines if the {@link AsyncRingtonePlayer} should pause between repeats of the ringtone.
     * When {@code true}, the system will check if the ringtone has stopped every
     * {@link #RESTART_RINGER_MILLIS} and restart the ringtone if it has stopped.  This does not
     * guarantee that there is {@link #RESTART_RINGER_MILLIS} between each repeat of the ringtone,
     * rather it ensures that for short ringtones, or ringtones which are not a multiple of
     * {@link #RESTART_RINGER_MILLIS} in duration that there will be some pause between repetitions.
     * When {@code false}, the ringtone will be looped continually with no attempt to pause between
     * repeats.
     */
    private boolean mShouldPauseBetweenRepeat = true;

    public AsyncRingtonePlayer() {
        // Empty
    }

    public AsyncRingtonePlayer(boolean shouldPauseBetweenRepeat) {
        mShouldPauseBetweenRepeat = shouldPauseBetweenRepeat;
    }

    /**
     * Plays the appropriate ringtone for the specified call.
     * If {@link VolumeShaper.Configuration} is specified, it is applied to the ringtone to change
     * the volume of the ringtone as it plays.
     *
     * @param factory The {@link RingtoneFactory}.
     * @param incomingCall The ringing {@link Call}.
     * @param volumeShaperConfig An optional {@link VolumeShaper.Configuration} which is applied to
     *                           the ringtone to change its volume while it rings.
     * @param isVibrationEnabled {@code true} if the settings and DND configuration of the device
     *                           is such that the vibrator should be used, {@code false} otherwise.
     * @return A {@link CompletableFuture} which on completion indicates whether or not the ringtone
     *         has a haptic track.  {@code True} indicates that a haptic track is present on the
     *         ringtone; in this case the default vibration in {@link Ringer} should not be played.
     *         {@code False} indicates that a haptic track is NOT present on the ringtone;
     *         in this case the default vibration in {@link Ringer} should be trigger if needed.
     */
    public @NonNull CompletableFuture<Boolean> play(RingtoneFactory factory, Call incomingCall,
            @Nullable VolumeShaper.Configuration volumeShaperConfig, boolean isVibrationEnabled) {
        Log.d(this, "Posting play.");
        if (mHapticsFuture == null) {
            mHapticsFuture = new CompletableFuture<>();
        }
        SomeArgs args = SomeArgs.obtain();
        args.arg1 = factory;
        args.arg2 = incomingCall;
        args.arg3 = volumeShaperConfig;
        args.arg4 = isVibrationEnabled;
        postMessage(EVENT_PLAY, true /* shouldCreateHandler */, args);
        return mHapticsFuture;
    }

    /** Stops playing the ringtone.
    * @return A {@link CompletableFuture} which on completion indicates whether or not the ringtone
    *        has a haptic track.
    */
    public @NonNull CompletableFuture<Boolean>  stop() {
        Log.d(this, "Posting stop.");
         if (mStopHapticsFuture == null) {
            mStopHapticsFuture = new CompletableFuture<>();
        }
        postMessage(EVENT_STOP, false /* shouldCreateHandler */, null);
        return mStopHapticsFuture;
    }

    /**
     * Posts a message to the ringtone-thread handler. Creates the handler if specified by the
     * parameter shouldCreateHandler.
     *
     * @param messageCode The message to post.
     * @param shouldCreateHandler True when a handler should be created to handle this message.
     */
    private void postMessage(int messageCode, boolean shouldCreateHandler, SomeArgs args) {
        synchronized(this) {
            if (mHandler == null && shouldCreateHandler) {
                mHandler = getNewHandler();
            }

            if (mHandler == null) {
                Log.d(this, "Message %d skipped because there is no handler.", messageCode);
                // M: Call vibration cannot stop issue @{
                if (messageCode == EVENT_STOP && mStopHapticsFuture != null) {
                    Log.d(this, "reset mStopHapticsFuture to null");
                    mStopHapticsFuture = null;
                }
                // @}
            } else {
                /// @{ Fix Ringtone conflict with line ALPS03524486
                /// During line in call, if an incoming Telecom call request to play Ringtone,
                /// Telecom call set stream type to 2, and then line force update to 0.
                /// This will cause voice change to speaker first and then back to earpiece.
                /// Solution is delay playing Telecom call Ringtone until line force update
                /// stream type to 0.
                /// M: Change looping play using post delay. @{
                if ((messageCode == EVENT_REPEAT || messageCode == EVENT_LOOPING)
                        && args != null) {
                    int delay = args.argi1;
                    args.recycle();
                    Log.d(this, "Message %d post delayed with %d ms", messageCode, delay);
                    mHandler.sendMessageDelayed(mHandler.obtainMessage(messageCode), delay);
                /// @}
                } else {
                    mHandler.obtainMessage(messageCode, args).sendToTarget();
                }
                /// @}
            }
        }
    }

    /**
     * Creates a new ringtone Handler running in its own thread.
     */
    private Handler getNewHandler() {
        Preconditions.checkState(mHandler == null);

        HandlerThread thread = new HandlerThread("ringtone-player");
        thread.start();

        return new Handler(thread.getLooper(), null /*callback*/, true /*async*/) {
            @Override
            public void handleMessage(Message msg) {
                switch(msg.what) {
                    case EVENT_PLAY:
                        handlePlay((SomeArgs) msg.obj);
                        break;
                    case EVENT_REPEAT:
                        handleRepeat();
                        break;
                    case EVENT_STOP:
                        handleStop();
                        break;
                    /// M: Add handleLooping for looping play. @{
                    case EVENT_LOOPING:
                        handleLooping();
                        break;
                    /// @}
                }
            }
        };
    }

    /**
     * Starts the actual playback of the ringtone. Executes on ringtone-thread.
     */
    private void handlePlay(SomeArgs args) {
        RingtoneFactory factory = (RingtoneFactory) args.arg1;
        Call incomingCall = (Call) args.arg2;
        VolumeShaper.Configuration volumeShaperConfig = (VolumeShaper.Configuration) args.arg3;
        boolean isVibrationEnabled = (boolean) args.arg4;
        args.recycle();
        // don't bother with any of this if there is an EVENT_STOP waiting.
        if (mHandler.hasMessages(EVENT_STOP)) {
            mHapticsFuture.complete(false /* ringtoneHasHaptics */);
            mHapticsFuture = null;
            return;
        }

        // If the Ringtone Uri is EMPTY, then the "None" Ringtone has been selected. Do not play
        // anything.
        if(Uri.EMPTY.equals(incomingCall.getRingtone())) {
            mRingtone = null;
            mHapticsFuture.complete(false /* ringtoneHasHaptics */);
            mHapticsFuture = null;
            return;
        }

        ThreadUtil.checkNotOnMainThread();
        Log.i(this, "handlePlay: Play ringtone.");

        if (mRingtone == null) {
            mRingtone = factory.getRingtone(incomingCall, volumeShaperConfig);
            if (mRingtone == null) {
                Uri ringtoneUri = incomingCall.getRingtone();
                String ringtoneUriString = (ringtoneUri == null) ? "null" :
                        ringtoneUri.toSafeString();
                Log.addEvent(null, LogUtils.Events.ERROR_LOG, "Failed to get ringtone from " +
                        "factory. Skipping ringing. Uri was: " + ringtoneUriString);
                mHapticsFuture.complete(false /* ringtoneHasHaptics */);
                mHapticsFuture = null;
                return;
            }

            // With the ringtone to play now known, we can determine if it has haptic channels or
            // not; we will complete the haptics future so the default vibration code in Ringer
            // can know whether to trigger the vibrator.
            if (mHapticsFuture != null && !mHapticsFuture.isDone()) {
                boolean hasHaptics = factory.hasHapticChannels(mRingtone);

                Log.i(this, "handlePlay: hasHaptics=%b, isVibrationEnabled=%b", hasHaptics,
                        isVibrationEnabled);
                if (hasHaptics) {
                    AudioAttributes attributes = mRingtone.getAudioAttributes();
                    Log.d(this, "handlePlay: %s haptic channel",
                            (isVibrationEnabled ? "unmuting" : "muting"));
                    mRingtone.setAudioAttributes(
                            new AudioAttributes.Builder(attributes)
                                    .setHapticChannelsMuted(!isVibrationEnabled)
                                    .build());
                }
                mHapticsFuture.complete(hasHaptics);
                mHapticsFuture = null;
            }
        }

        /// M: Using post delay to play ringtone.
        // Google add playing ringtone by looping on Q, change it to using post delay too. @{
        SomeArgs args1 = SomeArgs.obtain();
        args1.argi1 = POST_PLAY_RINGER_MILLIS;
        if (mShouldPauseBetweenRepeat) {
            // We're trying to pause between repeats, so the ringtone will not intentionally loop.
            // Instead, we'll use a handler message to perform repeats.
            postMessage(EVENT_REPEAT, true /* shouldCreateHandler */, args1);
        } else {
            postMessage(EVENT_LOOPING, true /* shouldCreateHandler */, args1);
        }
        /// @}
    }

    private void handleRepeat() {
        if (mRingtone == null) {
            return;
        }

        if (mRingtone.isPlaying()) {
            Log.d(this, "Ringtone already playing.");
        } else {
            mRingtone.play();
            Log.i(this, "Repeat ringtone.");
        }

        // Repost event to restart ringer in {@link RESTART_RINGER_MILLIS}.
        synchronized(this) {
            if (!mHandler.hasMessages(EVENT_REPEAT)) {
                mHandler.sendEmptyMessageDelayed(EVENT_REPEAT, RESTART_RINGER_MILLIS);
            }
        }
    }

    /**
     * Stops the playback of the ringtone. Executes on the ringtone-thread.
     */
    private void handleStop() {
        ThreadUtil.checkNotOnMainThread();
        Log.i(this, "Stop ringtone.");

        if (mStopHapticsFuture != null) {
            Log.d(this, "handleStop.");
            mStopHapticsFuture.complete(null);
            mStopHapticsFuture = null;
        }

        if (mRingtone != null) {
            Log.d(this, "Ringtone.stop() invoked.");
            mRingtone.stop();
            mRingtone = null;
        }

        synchronized(this) {
            // At the time that STOP is handled, there should be no need for repeat messages in the
            // queue.
            mHandler.removeMessages(EVENT_REPEAT);
            /// M: Added for support looping play.
            mHandler.removeMessages(EVENT_LOOPING);


            if (mHandler.hasMessages(EVENT_PLAY)) {
                Log.v(this, "Keeping alive ringtone thread for subsequent play request.");
            } else {
                mHandler.removeMessages(EVENT_STOP);
                mHandler.getLooper().quitSafely();
                mHandler = null;
                Log.v(this, "Handler cleared.");
            }
        }
    }

    /// M: Add handleLooping for looping play.@{
    private void handleLooping() {
        if (mRingtone == null) {
            return;
        }

        mRingtone.setLooping(true);
        if (mRingtone.isPlaying()) {
            Log.d(this, "Ringtone already playing.");
            return;
        }
        mRingtone.play();
        Log.i(this, "Play ringtone, looping.");
    }
    /// @}
}
