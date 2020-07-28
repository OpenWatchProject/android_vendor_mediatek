/*
 * Copyright (C) 2011 The Android Open Source Project
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

package com.mediatek.cellbroadcastreceiver;

import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.res.AssetFileDescriptor;
import android.content.res.Resources;
import android.media.AudioManager;
import android.media.MediaPlayer;
import android.media.MediaPlayer.OnErrorListener;
import android.media.RingtoneManager;
import android.net.Uri;
import android.provider.Settings;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.VibrationEffect;
import android.os.Vibrator;
import android.speech.tts.TextToSpeech;
import android.telephony.PhoneStateListener;
import android.telephony.TelephonyManager;
import android.util.Log;
import java.io.IOException;
import java.util.HashMap;
import java.util.ArrayList;
import java.util.Locale;

import static com.mediatek.cellbroadcastreceiver.CellBroadcastReceiver.DBG;
import com.mediatek.cmas.ext.ICmasMainSettingsExt;
/**
 * Manages alert audio and vibration and text-to-speech. Runs as a service so that
 * it can continue to play if another activity overrides the CellBroadcastListActivity.
 */
public class CellBroadcastAlertAudio extends Service implements TextToSpeech.OnInitListener,
        TextToSpeech.OnUtteranceCompletedListener {
    private static final String TAG = "[CMAS]CellBroadcastAlertAudio";
    private static final int PRESIDENT_ALERT_ID = 4370;
    /** Action to start playing alert audio/vibration/speech. */
    static final String ACTION_START_ALERT_AUDIO = "ACTION_START_ALERT_AUDIO";

    /** Extra for alert audio duration (from settings). */
    public static final String ALERT_AUDIO_DURATION_EXTRA =
            "com.mediatek.cellbroadcastreceiver.ALERT_AUDIO_DURATION";

    /** Extra for message body to speak (if speech enabled in settings). */
    public static final String ALERT_AUDIO_MESSAGE_BODY =
            "com.mediatek.cellbroadcastreceiver.ALERT_AUDIO_MESSAGE_BODY";

    /** Extra for text-to-speech language (if speech enabled in settings). */
    public static final String ALERT_AUDIO_MESSAGE_LANGUAGE =
            "com.mediatek.cellbroadcastreceiver.ALERT_AUDIO_MESSAGE_LANGUAGE";

    /** Extra for message ID . */
    public static final String ALERT_AUDIO_MESSAGE_ID =
            "com.mediatek.cellbroadcastreceiver.ALERT_AUDIO_MESSAGE_ID";

    /** Extra for alert audio vibration enabled (from settings). */
    public static final String ALERT_AUDIO_VIBRATE_EXTRA =
            "com.mediatek.cellbroadcastreceiver.ALERT_AUDIO_VIBRATE";

    /** Extra for alert audio ETWS behavior (always vibrate, even in silent mode). */
    public static final String ALERT_AUDIO_ETWS_VIBRATE_EXTRA =
            "com.mediatek.cellbroadcastreceiver.ALERT_AUDIO_ETWS_VIBRATE";

    private static final String TTS_UTTERANCE_ID =
                    "com.mediatek.cellbroadcastreceiver.UTTERANCE_ID";

    /** Pause duration between alert sound and alert speech. */
    private static final int PAUSE_DURATION_BEFORE_SPEAKING_MSEC = 1000;

    //volume change action
    private static final String VOLUME_CHANGED_ACTION = "android.media.VOLUME_CHANGED_ACTION";

    private static final int STATE_IDLE = 0;
    private static final int STATE_ALERTING = 1;
    private static final int STATE_PAUSING = 2;
    private static final int STATE_SPEAKING = 3;

    // Internal messages
    private static final int ALERT_SOUND_FINISHED = 1000;
    private static final int ALERT_PAUSE_FINISHED = 1001;
    private static final int ALERT_SOUND_CONTINUE = 1003;
    private static final int ALERT_SOUND_STOP = 1004;
    /// M: add for repeat alert feature. @{
    private static final String PREF_NAME = "com.mediatek.cellbroadcastreceiver_preferences";
    private static final int REPEAT_ALERT_START = 1002;
    private static final int[] sRepeatTimes = { 60000, 60000 * 3, 60000 * 5 };
    private boolean mEnableRepeat = true;
    private int mDuration;
    private ArrayList<Integer> mRepeatPattern;
    /// @}

    private int mState;

    private TextToSpeech mTts;
    private boolean mTtsEngineReady;
    private int mMessageId;
    private String mMessageBody;
    private String mMessageLanguage;
    private boolean mTtsLanguageSupported;
    private boolean mEnableVibrate;
    private boolean mEnableAudio;
    private boolean mRegisteredVolumeReceiver = false;

    private boolean mResetAlarmVolumeNeeded;
    private int mUserSetAlarmVolume;
    private int mAlarmStreamType = AudioManager.STREAM_ALARM;
    private boolean mUseFullVolume;

    private Vibrator mVibrator;
    private MediaPlayer mMediaPlayer;
    private AudioManager mAudioManager;
    private TelephonyManager mTelephonyManager;
    private int mInitialCallState;

    private final Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case ALERT_SOUND_FINISHED:
                    if (DBG) log("ALERT_SOUND_FINISHED " + mRepeatPattern.size());
                    /// @}
                    stop();     // stop alert sound
                    // if we can speak the message text
                    if (mMessageBody != null && mTtsEngineReady && mTtsLanguageSupported) {
                        mHandler.sendMessageDelayed(mHandler.obtainMessage(ALERT_PAUSE_FINISHED),
                                PAUSE_DURATION_BEFORE_SPEAKING_MSEC);
                        mState = STATE_PAUSING;
                    } else if (!mEnableRepeat || mRepeatPattern.isEmpty()) { ///M:
                        // add for repeat alert feature. do not stop service until end repeat.
                        stopSelf();
                        mState = STATE_IDLE;
                    }
                    break;

                case ALERT_PAUSE_FINISHED:
                    int res = TextToSpeech.ERROR;
                    if (DBG) log("ALERT_PAUSE_FINISHED");
                    if (mMessageBody != null && mTtsEngineReady && mTtsLanguageSupported) {
                        if (DBG) log("Speaking broadcast text: " + mMessageBody);
                        Bundle params = new Bundle();
                        // Play TTS in notification stream.
                        params.putInt(TextToSpeech.Engine.KEY_PARAM_STREAM,
                                AudioManager.STREAM_NOTIFICATION);
                        res = mTts.speak(mMessageBody, TextToSpeech.QUEUE_FLUSH, params,
                                TTS_UTTERANCE_ID);
                        mState = STATE_SPEAKING;
                    } else if (!mEnableRepeat || mRepeatPattern.isEmpty()) { ///M:
                        // add for repeat alert feature. do not stop service until end repeat.
                        Log.w(TAG, "TTS engine not ready or language not supported");
                        stopSelf();
                        mState = STATE_IDLE;
                    }

                    if (res != TextToSpeech.SUCCESS) {
                        Log.w(TAG, "TTS engine failed");
                        stopSelf();
                        mState = STATE_IDLE;
                    }
                    break;

                ///M: add for repeat alert feature. @{
                case REPEAT_ALERT_START:
                    if (DBG) log("REPEAT_ALERT_START");
                    if (!mRepeatPattern.isEmpty()) {
                        mRepeatPattern.remove(0);
                        play();
                    }
                    break;
                /// @}
                 case ALERT_SOUND_CONTINUE:
                    if (DBG) {
                        log("ALERT_SOUND_CONTINUE");
                    }
                    stop();     // stop alert sound
                    play();
                    break;
                case ALERT_SOUND_STOP:
                    if (DBG) {
                        log("ALERT_SOUND_STOP");
                    }
                    stop();
                    stopSelf();
                    mState = STATE_IDLE;
                default:
                    Log.e(TAG, "Handler received unknown message, what=" + msg.what);
            }
        }
    };

    private final PhoneStateListener mPhoneStateListener = new PhoneStateListener() {
        @Override
        public void onCallStateChanged(int state, String ignored) {
            // Stop the alert sound and speech if the call state changes.
            if (state != TelephonyManager.CALL_STATE_IDLE
                    && state != mInitialCallState) {
                stopSelf();
            }
        }
    };

    private BroadcastReceiver volumeChangeReceiver = new BroadcastReceiver() {

        @Override
        public void onReceive(Context arg0, Intent arg1) {
            Log.d(TAG, "receive volume change broadcast, stop sound and vibration alert");
            stop();
        }

    };

    /**
     * Callback from TTS engine after initialization.
     * @param status {@link TextToSpeech#SUCCESS} or {@link TextToSpeech#ERROR}.
     */
    @Override
    public void onInit(int status) {
        if (DBG) log("onInit() TTS engine status: " + status);
        if (status == TextToSpeech.SUCCESS) {
            mTtsEngineReady = true;
            // try to set the TTS language to match the broadcast
            mTts.setOnUtteranceCompletedListener(this);
            setTtsLanguage();
        } else {
            mTtsEngineReady = false;
            mTts = null;
            Log.e(TAG, "onInit() TTS engine error: " + status);
        }
    }

    /**
     * Try to set the TTS engine language to the value of mMessageLanguage.
     * mTtsLanguageSupported will be updated based on the response.
     */
    private void setTtsLanguage() {
        if (mMessageLanguage != null) {
            if (DBG) log("Setting TTS language to '" + mMessageLanguage + '\'');
            int result = mTts.setLanguage(new Locale(mMessageLanguage));
            // success values are >= 0, failure returns negative value
            if (DBG) log("TTS setLanguage() returned: " + result);
            mTtsLanguageSupported = result >= 0;
        } else {
            // try to use the default TTS language for broadcasts with no language specified
            if (DBG) log("No language specified in broadcast: using default");
            mTtsLanguageSupported = true;
        }
    }

    /**
     * Callback from TTS engine.
     * @param utteranceId the identifier of the utterance.
     */
    @Override
    public void onUtteranceCompleted(String utteranceId) {
        ///M: if support repeat alert feature, do not stop service until repeat over.
        if (utteranceId.equals(TTS_UTTERANCE_ID) && (!mEnableRepeat || mRepeatPattern.isEmpty())) {
            stopSelf();
        }
    }

    @Override
    public void onCreate() {
        mVibrator = (Vibrator) getSystemService(Context.VIBRATOR_SERVICE);
        mAudioManager = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
        // Listen for incoming calls to kill the alarm.
        mTelephonyManager =
                (TelephonyManager) getSystemService(Context.TELEPHONY_SERVICE);
        mTelephonyManager.listen(
                mPhoneStateListener, PhoneStateListener.LISTEN_CALL_STATE);
    }

    @Override
    public void onDestroy() {
        //unregister volume register
        if (mRegisteredVolumeReceiver && !CmasConfigManager.isTraProfile()) {
            mRegisteredVolumeReceiver = false;
            unregisterReceiver(volumeChangeReceiver);
        }

        /// M: add for repeat alert feature. cancel the next schedule when destroy. @{
        if (mEnableRepeat && (mRepeatPattern != null) && !mRepeatPattern.isEmpty()) {
            cancelRepeat();
        }
        /// @}

        // stop audio, vibration and TTS
        stop();
        mHandler.removeMessages(ALERT_SOUND_STOP);
        // Stop listening for incoming calls.
        mTelephonyManager.listen(mPhoneStateListener, 0);
        // shutdown TTS engine
        if (mTts != null) {
            try {
                mTts.shutdown();
            } catch (IllegalStateException e) {
                // catch "Unable to retrieve AudioTrack pointer for stop()" exception
                Log.e(TAG, "exception trying to shutdown text-to-speech");
            }
        }
        // release CPU wake lock acquired by CellBroadcastAlertService
        CellBroadcastAlertWakeLock.releaseCpuLock();
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        // No intent, tell the system not to restart us.
        if (intent == null) {
            stopSelf();
            return START_NOT_STICKY;
        }

        mUseFullVolume = CmasConfigManager.getUseFullVolume();
        ///M: add for repeat alert feature.
        cancelRepeat();
        initAlertSequence();

        // This extra should always be provided by CellBroadcastAlertService,
        // but default to 10.5 seconds just to be safe (CMAS requirement).
        int duration = intent.getIntExtra(ALERT_AUDIO_DURATION_EXTRA, 10500);
        mMessageId = intent.getIntExtra(ALERT_AUDIO_MESSAGE_ID, 0);
        // Get text to speak (if enabled by user)
        mMessageBody = intent.getStringExtra(ALERT_AUDIO_MESSAGE_BODY);
        mMessageLanguage = intent.getStringExtra(ALERT_AUDIO_MESSAGE_LANGUAGE);

        mEnableVibrate = intent.getBooleanExtra(ALERT_AUDIO_VIBRATE_EXTRA, true);
        boolean forceVibrate = intent.getBooleanExtra(ALERT_AUDIO_ETWS_VIBRATE_EXTRA, false);

        switch (mAudioManager.getRingerMode()) {
            case AudioManager.RINGER_MODE_SILENT:
                if (DBG) log("Ringer mode: silent");
                mEnableVibrate = forceVibrate;
                mEnableAudio = false;
                break;

            case AudioManager.RINGER_MODE_VIBRATE:
                if (DBG) log("Ringer mode: vibrate");
                mEnableAudio = false;
                break;

            case AudioManager.RINGER_MODE_NORMAL:
            default:
                /// M: add to distinguish general and outdoor mode. Do not vibrate
                // in general mode.0 is off,1 is on @{
                boolean shouldVibrate = Settings.System.getInt(getContentResolver(),
                    Settings.System.VIBRATE_WHEN_RINGING, 0) != 0;
                if (DBG) log("Ringer mode: normal   shouldVibrate = " + shouldVibrate);
                if (!shouldVibrate) {
                    mEnableVibrate = forceVibrate;
                }
                /// @}
                mEnableAudio = true;
                break;
        }
        if (CmasConfigManager.isChileProfile()) {
            mEnableVibrate = true;
        }
        if (mUseFullVolume) {
            mEnableAudio = true;
        }

        ICmasMainSettingsExt optSetAlertAudioVibration = (ICmasMainSettingsExt)
                CellBroadcastPluginManager.getCellBroadcastPluginObject(
                CellBroadcastPluginManager.CELLBROADCAST_PLUGIN_TYPE_MAIN_SETTINGS);

            mEnableAudio = optSetAlertAudioVibration.setAlertVolumeVibrate(mMessageId, mEnableAudio);
            mEnableVibrate = optSetAlertAudioVibration.setAlertVolumeVibrate(mMessageId, mEnableVibrate);
        if (CmasConfigManager.isTraProfile()) {
            mEnableAudio = true;
            mEnableVibrate = true;
        }
        Log.d(TAG, "mEnableAudio:" + mEnableAudio + ", mEnableVibrate:" + mEnableVibrate);
        if (mMessageBody != null && mEnableAudio) {
            if (mTts == null) {
                mTts = new TextToSpeech(this, this);
            } else if (mTtsEngineReady) {
                setTtsLanguage();
            }
        }
        ///M: add for repeat alert. @{
        mDuration = duration;
        schedulePlay();
        /// @}
        //register volume change receiver
        if (!mRegisteredVolumeReceiver && !CmasConfigManager.isTraProfile()) {
            IntentFilter intentFilter = new IntentFilter();
            intentFilter.addAction(VOLUME_CHANGED_ACTION);
            registerReceiver(volumeChangeReceiver, intentFilter);
            mRegisteredVolumeReceiver = true;
        }

        // Record the initial call state here so that the new alarm has the
        // newest state.
        mInitialCallState = mTelephonyManager.getCallState();

        return START_STICKY;
    }

    // Volume suggested by media team for in-call alarms.
    private static final float IN_CALL_VOLUME = 0.125f;

    /**
     * Start playing the alert sound, and send delayed message when it's time to stop.
     * @param duration the alert sound duration in milliseconds
     */
    private void play() {
        // stop() checks to see if we are already playing.
        stop();

        if (DBG) log("play()");

        // Start the vibration first.
        if (mEnableVibrate || forceAlert()) {
            if (CmasConfigManager.isTwProfile() &&
                    (mMessageId == 919 || mMessageId == 911)) {
                final long[] sVibratePattern919 = {0, 2000, 500, 500};
                mVibrator.vibrate(sVibratePattern919, -1);
            } else {
                setVibrator();
            }
        }

        if (mEnableAudio || forceAlert()) {
            // future optimization: reuse media player object
            mMediaPlayer = new MediaPlayer();
            mMediaPlayer.setOnErrorListener(new OnErrorListener() {
                public boolean onError(MediaPlayer mp, int what, int extra) {
                    Log.e(TAG, "Error occurred while playing audio.");
                    mp.stop();
                    mp.release();
                    mMediaPlayer = null;
                    return true;
                }
            });

            try {
                // Check if we are in a call. If we are, play the alert
                // sound at a low volume to not disrupt the call.
                if (mTelephonyManager.getCallState()
                        != TelephonyManager.CALL_STATE_IDLE) {
                    Log.v(TAG, "in call: reducing volume");
                    mMediaPlayer.setVolume(IN_CALL_VOLUME, IN_CALL_VOLUME);
                } else {
                    float volume = 1.0f;

                    ICmasMainSettingsExt optGetAlertVolume = (ICmasMainSettingsExt)
                        CellBroadcastPluginManager.getCellBroadcastPluginObject(
                        CellBroadcastPluginManager.CELLBROADCAST_PLUGIN_TYPE_MAIN_SETTINGS);

                if (optGetAlertVolume != null) {
                    volume = optGetAlertVolume.getAlertVolume(mMessageId);
                  }
                Log.d(TAG , "Alert volume: " + volume);
                mMediaPlayer.setVolume(volume, volume);
               }

                // start playing alert audio (unless master volume is vibrate only or silent).
                if (CmasConfigManager.isTwProfile() &&
                        (mMessageId == 919 || mMessageId == 911)) {
                    Uri notification = RingtoneManager.getDefaultUri(
                            RingtoneManager.TYPE_NOTIFICATION);
                    mMediaPlayer.setDataSource(this, notification);
                } else {
                    setRingtone();
                }

                int streamType = AudioManager.STREAM_NOTIFICATION;
                if (forceAlert() || forcePresidentAlert()) {
                    streamType = AudioManager.STREAM_ALARM;
                }
                mAudioManager.requestAudioFocus(null, streamType,
                        AudioManager.AUDIOFOCUS_GAIN_TRANSIENT);
                startAlarm(mMediaPlayer);
            } catch (IllegalStateException ex) {
                Log.e(TAG, "Failed to play alert sound", ex);
            } catch (IOException e) {
                Log.e(TAG, "IOException");
            }
        }

        // stop alert after the specified duration
        if (CmasConfigManager.isTwProfile() &&
                (mMessageId == 911 || mMessageId == 919)) {
             mHandler.sendMessageDelayed(mHandler.obtainMessage(ALERT_SOUND_FINISHED), 1000);
        } else if (CmasConfigManager.isChileProfile()) {
            // stop alert after the specified duration
            mHandler.sendMessageDelayed(mHandler.obtainMessage(ALERT_SOUND_CONTINUE), mDuration);
        } else {
             mHandler.sendMessageDelayed(mHandler.obtainMessage(ALERT_SOUND_FINISHED), mDuration);
        }

        mState = STATE_ALERTING;
    }

    // Do the common stuff when starting the alarm.
    private void startAlarm(MediaPlayer player)
            throws java.io.IOException, IllegalArgumentException, IllegalStateException {
        int streamType = AudioManager.STREAM_NOTIFICATION;
        if (forceAlert() || forcePresidentAlert()) {
            streamType = AudioManager.STREAM_ALARM;
        }
        player.setAudioStreamType(streamType);
        setAlarmStreamVolumeToFull(streamType);
        player.setLooping(true);
        player.prepare();
        player.start();
    }

    private static void setDataSourceFromResource(Resources resources,
            MediaPlayer player, int res) throws java.io.IOException {
        AssetFileDescriptor afd = resources.openRawResourceFd(res);
        if (afd != null) {
            player.setDataSource(afd.getFileDescriptor(), afd.getStartOffset(),
                    afd.getLength());
            afd.close();
        }
    }

    /**
     * Stops alert audio and speech.
     */
    public void stop() {
        if (DBG) log("stop()");

        mHandler.removeMessages(ALERT_SOUND_FINISHED);
        mHandler.removeMessages(ALERT_PAUSE_FINISHED);
        mHandler.removeMessages(ALERT_SOUND_CONTINUE);
        resetAlarmStreamVolume();
        if (mState == STATE_ALERTING) {
            // Stop audio playing
            if (mMediaPlayer != null) {
                try {
                    mMediaPlayer.stop();
                    mMediaPlayer.release();
                } catch (IllegalStateException e) {
                    // catch "Unable to retrieve AudioTrack pointer for stop()" exception
                    Log.e(TAG, "exception trying to stop media player");
                }
                mMediaPlayer = null;
            }

            // Stop vibrator
            mVibrator.cancel();
        } else if (mState == STATE_SPEAKING && mTts != null) {
            try {
                mTts.stop();
            } catch (IllegalStateException e) {
                // catch "Unable to retrieve AudioTrack pointer for stop()" exception
                Log.e(TAG, "exception trying to stop text-to-speech");
            }
        }
        mAudioManager.abandonAudioFocus(null);
        mState = STATE_IDLE;
    }

    private static void log(String msg) {
        Log.d(TAG, msg);
    }

    /**
     * M: add for repeat alert feature.
     */
    private void initAlertSequence() {
        SharedPreferences pre = getSharedPreferences(PREF_NAME, 0);
        mEnableRepeat = pre.getBoolean(CheckBoxAndSettingsPreference.KEY_ENABLE_REPEAT_ALERT, true);
        if (CmasConfigManager.isTwProfile()) {
            mEnableRepeat =  false;
        }
        mRepeatPattern = new ArrayList<Integer>();
        if (mEnableRepeat) {
            for (int i : sRepeatTimes) {
                mRepeatPattern.add(i);
            }
        }
    }

    /**
     * M: add for repeat alert feature.
     * @param duration the duration of one alert time.
     */
    private void schedulePlay() {
         //if (mAudioManager.getRingerMode() != AudioManager.RINGER_MODE_SILENT) {
            if (CmasConfigManager.isChileProfile()) {
                mEnableRepeat =  false;
                mHandler.sendMessageDelayed(mHandler.obtainMessage(ALERT_SOUND_STOP), 300000);
            }
            play();
         //}
        if (mEnableRepeat) {
            for (int i = 0; i < mRepeatPattern.size(); i++) {
                mHandler.sendMessageDelayed(mHandler.obtainMessage(REPEAT_ALERT_START),
                        mRepeatPattern.get(i));
            }
        }
    }

    /**
     * M: add for repeat alert feature.
     */
    public void cancelRepeat() {
        mHandler.removeMessages(REPEAT_ALERT_START);
    }

    /**
     * M: force alarm (except first alert) in silent mode.
     * @return
     */
    private boolean forceAlert() {
        if (CmasConfigManager.isTraProfile()) {
            return true;
        }
        boolean isSilentMode = mAudioManager.getRingerMode() == AudioManager.RINGER_MODE_SILENT;
        boolean isRepeating = mEnableRepeat && (mRepeatPattern.size() < sRepeatTimes.length);
        return isSilentMode && isRepeating;
    }

    private boolean forcePresidentAlert() {
         boolean modeValue = (mAudioManager.getRingerMode() == AudioManager.RINGER_MODE_SILENT) ||
                (mAudioManager.getRingerMode() == AudioManager.RINGER_MODE_VIBRATE);
        Log.d(TAG, "forcePresidentAlert");
        return (PRESIDENT_ALERT_ID == mMessageId) && modeValue;
    }

    private void setVibrator() {
        int[] patternArray = CmasConfigManager.getVibrationPattern();
        long[] vibrationPattern = new long[patternArray.length];

        for (int i = 0; i < patternArray.length; i++) {
            vibrationPattern[i] = patternArray[i];
        }
        mVibrator.vibrate(VibrationEffect.createWaveform(vibrationPattern, -1));
    }

    private void setRingtone() {
        try {
            String ringtone = CmasConfigManager.getRingtone();
            int resId = getResources().getIdentifier(ringtone, "raw", this.getPackageName());
            log("setUserConfiguredRingtone: ringtong = " + ringtone + ", resId = " + resId);

            if (resId > 0 && mMediaPlayer != null) {
                setDataSourceFromResource(getResources(), mMediaPlayer, resId);
            }
        } catch (IOException e) {
            log("setUserConfiguredRingtone: " + e.toString());
        }
    }

    /**
     * Set volume of STREAM_ALARM to full.
     */
    private void setAlarmStreamVolumeToFull(int streamType) {
        log("Set alarm volume to full for emergency alerts. mUseFullVolume=" + mUseFullVolume);
        if (mUseFullVolume) {
            mAlarmStreamType = streamType;
            mUserSetAlarmVolume = mAudioManager.getStreamVolume(streamType);
            mResetAlarmVolumeNeeded = true;
            mAudioManager.setStreamVolume(streamType,
                    mAudioManager.getStreamMaxVolume(streamType), 0);
        }
    }

    /**
     * Reset volume of STREAM_ALARM, if needed.
     */
    private void resetAlarmStreamVolume() {
        if (mResetAlarmVolumeNeeded) {
            log("resetting alarm volume to back to " + mUserSetAlarmVolume);
            mAudioManager.setStreamVolume(mAlarmStreamType, mUserSetAlarmVolume, 0);
            mResetAlarmVolumeNeeded = false;
            mAlarmStreamType = AudioManager.STREAM_ALARM;
        }
    }
}
