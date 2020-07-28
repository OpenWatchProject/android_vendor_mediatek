/*
 * Copyright (C) 2014 The Android Open Source Project
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

package com.android.fmradio;

import android.app.ActivityManager;
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.Notification.BigTextStyle;
import android.app.Notification.Builder;
import android.app.PendingIntent;
import android.app.Service;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothProfile;
import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Configuration;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.media.AudioDeviceInfo;
import android.media.AudioDevicePort;
import android.media.AudioDevicePortConfig;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioManager.OnAudioFocusChangeListener;
import android.media.AudioManager.OnAudioPortUpdateListener;
import android.media.AudioMixPort;
import android.media.AudioPatch;
import android.media.AudioPort;
import android.media.AudioPortConfig;
import android.media.AudioRecord;
import android.media.AudioSystem;
import android.media.AudioTrack;
import android.media.MediaRecorder;
import android.net.Uri;
import android.os.Binder;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.os.SystemProperties;
import android.text.TextUtils;
import android.util.Log;

import com.android.fmradio.FmStation.Station;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Iterator;

/**
 * Background service to control FM or do background tasks.
 */
public class FmService extends Service implements FmRecorder.OnRecorderStateChangedListener {
    // Logging
    private static final String TAG = "FmService";

    // Broadcast messages from other sounder APP to FM service
    private static final String SOUND_POWER_DOWN_MSG = "com.android.music.musicservicecommand";
    private static final String FM_SEEK_PREVIOUS = "fmradio.seek.previous";
    private static final String FM_SEEK_NEXT = "fmradio.seek.next";
    private static final String FM_TURN_OFF = "fmradio.turnoff";
    private static final String CMDPAUSE = "pause";

    private static final String FM_STOP_RECORDING = "fmradio.stop.recording";
    private static final String FM_ENTER_RECORD_SCREEN = "fmradio.enter.record.screen";

    // HandlerThread Keys
    private static final String FM_FREQUENCY = "frequency";
    private static final String OPTION = "option";
    private static final String RECODING_FILE_NAME = "name";

    /*
    private static final String HIFI_UI_STATUS_ENABLED = "hifi_dac=on";
    private static final String GET_HIFI_UI_STATUS = "hifi_dac";
    */

    // SmartPA support
    private static final String SMART_PA_STATUS_ENABLED = "SmartPAandWithoutDSP=true";
    private static final String GET_SMART_PA_STATUS = "SmartPAandWithoutDSP";
    private static boolean sIsSmartPAandWithoutDSP;

    // Vibration Speaker  Support
    private static boolean sIsVibSpeakerSupportOn;

    // RDS events
    // PS
    private static final int RDS_EVENT_PROGRAMNAME = 0x0008;
    // RT
    private static final int RDS_EVENT_LAST_RADIOTEXT = 0x0040;
    // AF
    private static final int RDS_EVENT_AF = 0x0080;

    // Headset
    private static final int HEADSET_PLUG_IN = 1;

    // Notification id
    private static final int NOTIFICATION_ID = 1;

    // ignore audio data
    private static final int AUDIO_FRAMES_TO_IGNORE_COUNT = 3;

    // Set audio policy for FM
    // should check AUDIO_POLICY_FORCE_FOR_MEDIA in audio_policy.h
    private static final int FOR_PROPRIETARY = 1;

    // to get the music output id for checking the fast mixer/normal mixer
    private static int mOutputPortId = -1;

    // Forced Use value
    private int mForcedUseForMedia;

    // FM recorder
    FmRecorder mFmRecorder = null;
    private BroadcastReceiver mSdcardListener = null;
    private int mRecordState = FmRecorder.STATE_INVALID;
    private int mRecorderErrorType = -1;
    // If eject record sdcard, should set Value false to not record.
    // Key is sdcard path(like "/storage/sdcard0"), V is to enable record or
    // not.
    private HashMap<String, Boolean> mSdcardStateMap = new HashMap<String, Boolean>();
    // The show name in save dialog but saved in service
    // If modify the save title it will be not null, otherwise it will be null
    private String mModifiedRecordingName = null;
    // record the listener list, will notify all listener in list
    private ArrayList<Record> mRecords = new ArrayList<Record>();
    // record FM whether in recording mode
    private boolean mIsInRecordingMode = false;
    // record sd card path when start recording
    private static String sRecordingSdcard = FmUtils.getDefaultStoragePath();

    // RDS
    // PS String
    private String mPsString = "";
    // RT String
    private String mRtTextString = "";
    // Notification target class name
    private String mTargetClassName = FmMainActivity.class.getName();
    // RDS thread use to receive the information send by station
    private Thread mRdsThread = null;
    // record whether RDS thread exit
    private boolean mIsRdsThreadExit = false;

    // State variables
    // Record whether FM is in native scan state
    private boolean mIsNativeScanning = false;
    // Record whether FM is in scan thread
    private boolean mIsScanning = false;
    // Record whether FM is in seeking state
    private boolean mIsNativeSeeking = false;
    // Record whether FM is in native seek
    private boolean mIsSeeking = false;
    // Record whether searching progress is canceled
    private boolean mIsStopScanCalled = false;
    // Record whether is speaker used
    private boolean mIsSpeakerUsed = false;
    // Record whether device is open
    private boolean mIsDeviceOpen = false;
    // Record Power Status
    private int mPowerStatus = POWER_DOWN;

    public static int POWER_UP = 0;
    public static int DURING_POWER_UP = 1;
    public static int POWER_DOWN = 2;
    // Record whether service is init
    private boolean mIsServiceInited = false;
    // Fm power down by loss audio focus,should make power down menu item can
    // click
    private boolean mIsPowerDown = false;
    // FmMainActivity foreground
    private boolean mIsFmMainForeground = true;
    // FmFavoriteActivity foreground
    private boolean mIsFmFavoriteForeground = false;
    // FmRecordActivity foreground
    private boolean mIsFmRecordForeground = false;
    // Flag to check if recording permission is present
    private boolean mIsRecordingPermissible = false;
    // Instance variables
    private Context mContext = null;
    private AudioManager mAudioManager = null;
    private ActivityManager mActivityManager = null;
    //private MediaPlayer mFmPlayer = null;
    private WakeLock mWakeLock = null;
    // Audio focus is held or not
    private boolean mIsAudioFocusHeld = false;
    // Focus transient lost
    private boolean mPausedByTransientLossOfFocus = false;
    private int mCurrentStation = FmUtils.DEFAULT_STATION;
    // Headset plug state (0:long antenna plug in, 1:long antenna plug out)
    private int mValueHeadSetPlug = 1;
    // For bind service
    private final IBinder mBinder = new ServiceBinder();
    // Broadcast to receive the external event
    private FmServiceBroadcastReceiver mBroadcastReceiver = null;
    // Async handler
    private FmRadioServiceHandler mFmServiceHandler;

    private boolean mPrevBtHeadsetState = false;
    // Lock for lose audio focus and receive SOUND_POWER_DOWN_MSG
    // at the same time
    // while recording call stop recording not finished(status is still
    // RECORDING), but
    // SOUND_POWER_DOWN_MSG will exitFm(), if it is RECORDING will discard the
    // record.
    // 1. lose audio focus -> stop recording(lock) -> set to IDLE and show save
    // dialog
    // 2. exitFm() -> check the record status, discard it if it is recording
    // status(lock)
    // Add this lock the exitFm() while stopRecording()
    private Object mStopRecordingLock = new Object();
    // The listener for exit, should finish favorite when exit FM
    private static OnExitListener sExitListener = null;
    // The latest status for mute/unmute
    private boolean mIsMuted = false;
    // The latest status for rds supported or not
    private boolean mIsRdsSupported = false;

    // Audio Patch
    private AudioPatch mAudioPatch = null;
    private Object mRenderLock = new Object();
    private Object mRenderingLock = new Object();
    private Object mNotificationLock = new Object();
    private Object mAudioPatchLock = new Object();
    private boolean mIsParametersSet = false;
    private boolean mIsOutputDeviceChanged = false;

    private Notification.Builder mNotificationBuilder = null;
    private BigTextStyle mNotificationStyle = null;

    static final String NOTIFICATION_CHANNEL_FMRADIO = "fm_notification_channel";

    private boolean mRegainAudioFocusAndSetForceUseSpeaker = false;

    @Override
    public IBinder onBind(Intent intent) {
        return mBinder;
    }

    /**
     * class use to return service instance
     */
    public class ServiceBinder extends Binder {
        /**
         * get FM service instance
         *
         * @return service instance
         */
        FmService getService() {
            return FmService.this;
        }
    }

    /**
     * Broadcast monitor external event, Other app want FM stop, Phone shut
     * down, screen state, headset state
     */
    private class FmServiceBroadcastReceiver extends BroadcastReceiver {

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            String command = intent.getStringExtra("command");
            Log.d(TAG, "onReceive, action = " + action + " / command = " + command);
            // other app want FM stop, stop FM
            if ((SOUND_POWER_DOWN_MSG.equals(action) && CMDPAUSE.equals(command))) {
                // need remove all messages, make power down will be execute
                mFmServiceHandler.removeCallbacksAndMessages(null);
                exitFm();
                stopSelf();
                // phone shut down, so exit FM
            } else if (Intent.ACTION_SHUTDOWN.equals(action)) {
                /**
                 * here exitFm, system will send broadcast, system will shut
                 * down, so fm does not need call back to activity
                 */
                mFmServiceHandler.removeCallbacksAndMessages(null);
                exitFm();
                stopSelf();
                // screen on, if FM play, open rds
            } else if (Intent.ACTION_SCREEN_ON.equals(action)) {
                setRdsAsync(true);
                // screen off, if FM play, close rds
            } else if (Intent.ACTION_SCREEN_OFF.equals(action)) {
                setRdsAsync(false);
                // show notification after screen off, because when screen on at landscape,
                // activity will only call onpause, FmMainActivity can't show notificaiton
                updatePlayingNotification();
                // switch antenna when headset plug in or plug out
            } else if (Intent.ACTION_HEADSET_PLUG.equals(action)) {
                // switch antenna should not impact audio focus status
                mValueHeadSetPlug = (intent.getIntExtra("state", -1) == HEADSET_PLUG_IN) ? 0 : 1;
                switchAntennaAsync(mValueHeadSetPlug);

                // Avoid Service is killed,and receive headset plug in
                // broadcast again
                if (!mIsServiceInited) {
                    Log.d(TAG, "onReceive, mIsServiceInited is false");
                    return;
                }
                /*
                 * If ear phone insert and activity is
                 * foreground. power up FM automatic
                 */
                Log.d(TAG, "FmServiceBroadcastReceiver, mValueHeadSetPlug: "
                            + mValueHeadSetPlug + "isActivityForeground:" + isActivityForeground());
                if ((0 == mValueHeadSetPlug) && isActivityForeground() &&
                    mIsRecordingPermissible) {
                    powerUpAsync(FmUtils.computeFrequency(mCurrentStation));
                } else if (1 == mValueHeadSetPlug) {
                    mFmServiceHandler.removeMessages(FmListener.MSGID_SCAN_FINISHED);
                    mFmServiceHandler.removeMessages(FmListener.MSGID_SEEK_FINISHED);
                    mFmServiceHandler.removeMessages(FmListener.MSGID_TUNE_FINISHED);
                    mFmServiceHandler.removeMessages(
                            FmListener.MSGID_POWERDOWN_FINISHED);
                    mFmServiceHandler.removeMessages(
                            FmListener.MSGID_POWERUP_FINISHED);
                    focusChanged(AudioManager.AUDIOFOCUS_LOSS);

                    // Need check to switch to earphone mode for audio will
                    // change to AudioSystem.FORCE_NONE
                    setForceUse(false);
                    FmUtils.setIsSpeakerModeOnFocusLost(mContext, false);

                    // Notify UI change to earphone mode, false means not speaker mode
                    Bundle bundle = new Bundle(2);
                    bundle.putInt(FmListener.CALLBACK_FLAG,
                            FmListener.LISTEN_SPEAKER_MODE_CHANGED);
                    bundle.putBoolean(FmListener.KEY_IS_SPEAKER_MODE, false);
                    notifyActivityStateChanged(bundle);
                }
            }
        }
    }

    /**
     * Handle sdcard mount/unmount event. 1. Update the sdcard state map 2. If
     * the recording sdcard is unmounted, need to stop and notify
     */
    private class SdcardListener extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            // If eject record sdcard, should set this false to not
            // record.
            Log.d(TAG, "SdcardListener.onReceive, action = " + intent.getAction());
            updateSdcardStateMap(intent);

            if (mFmRecorder == null) {
                Log.w(TAG, "SdcardListener.onReceive, mFmRecorder is null");
                return;
            }

            String action = intent.getAction();
            if (Intent.ACTION_MEDIA_EJECT.equals(action) ||
                    Intent.ACTION_MEDIA_UNMOUNTED.equals(action)) {
                // If not unmount recording sd card, do nothing;
                if (isRecordingCardUnmount(intent)) {
                    Log.w(TAG, "SdcardListener.onReceive, sd card unmount");
                    if (mFmRecorder.getState() == FmRecorder.STATE_RECORDING) {
                        Log.w(TAG, "SdcardListener.onReceive, discarding recording");
                        onRecorderError(FmRecorder.ERROR_SDCARD_NOT_PRESENT);
                        mFmRecorder.discardRecording();
                    } else {
                        Bundle bundle = new Bundle(2);
                        bundle.putInt(FmListener.CALLBACK_FLAG,
                                FmListener.LISTEN_RECORDSTATE_CHANGED);
                        bundle.putInt(FmListener.KEY_RECORDING_STATE,
                                FmRecorder.STATE_IDLE);
                        Log.w(TAG, "SdcardListener.onReceive, notifying activity");
                        notifyActivityStateChanged(bundle);
                    }
                }
                return;
            }
        }
    }

    /**
     * whether antenna available
     *
     * @return true, antenna available; false, antenna not available
     */
    public boolean isAntennaAvailable() {
        AudioDeviceInfo[] deviceList = mAudioManager.getDevices(AudioManager.GET_DEVICES_OUTPUTS);

        for (AudioDeviceInfo devInfo : deviceList) {
            int type = devInfo.getType();
            if (type == AudioDeviceInfo.TYPE_WIRED_HEADSET
                    || type == AudioDeviceInfo.TYPE_WIRED_HEADPHONES) {
                return true;
            }
        }

        return false;
    }

    private void setForceUse(boolean isSpeaker) {
        Log.d(TAG, "setForceUse: isSpeaker = " + isSpeaker);
        mForcedUseForMedia = isSpeaker ? AudioSystem.FORCE_SPEAKER : AudioSystem.FORCE_NONE;
        AudioSystem.setForceUse(FOR_PROPRIETARY, mForcedUseForMedia);
        mIsSpeakerUsed = isSpeaker;
    }

    /**
     * Set FM audio from speaker or not
     *
     * @param isSpeaker true if set FM audio from speaker
     */
    public void setSpeakerPhoneOn(boolean isSpeaker) {
        Log.d(TAG, "setSpeakerPhoneOn " + isSpeaker);
        setForceUse(isSpeaker);
    }

    private int getForceUse() {
        return AudioSystem.getForceUse(FOR_PROPRIETARY);
    }

    private void notifySpeakerModeChange() {
            Bundle bundle = new Bundle(2);
            bundle.putInt(FmListener.CALLBACK_FLAG,
                    FmListener.MSGID_BT_STATE_CHANGED);
            bundle.putBoolean(FmListener.KEY_BT_STATE, mPrevBtHeadsetState);
            notifyActivityStateChanged(bundle);
    }

    /**
     * Check if BT headset is connected
     * @return true if current is playing with BT headset
     */
    public boolean isBluetoothHeadsetInUse() {
        BluetoothAdapter btAdapter = BluetoothAdapter.getDefaultAdapter();
        int a2dpState = btAdapter.getProfileConnectionState(BluetoothProfile.HEADSET);
        boolean ret = BluetoothProfile.STATE_CONNECTED == a2dpState
                || BluetoothProfile.STATE_CONNECTING == a2dpState;
        Log.d(TAG, "isBluetoothHeadsetInUse " + ret);
        return ret;
    }

    public boolean isA2DPInUse() {
        BluetoothAdapter btAdapter = BluetoothAdapter.getDefaultAdapter();
        int a2dpState2 = btAdapter.getProfileConnectionState(BluetoothProfile.A2DP);
        boolean ret = (BluetoothProfile.STATE_CONNECTED == a2dpState2
                || BluetoothProfile.STATE_CONNECTING == a2dpState2);
        Log.d(TAG, "isA2DPInUse " + ret);
        return ret;
    }

    private synchronized void startRender() {
        Log.d(TAG, "startRender " + AudioSystem.getForceUse(FOR_PROPRIETARY));

        // need to create new audio record and audio play back track,
        // because input/output device may be changed.
        if (mAudioRecord != null) {
            synchronized (mAudioRecord) {
                if (mAudioRecord.getRecordingState() == AudioRecord.RECORDSTATE_RECORDING) {
                    mAudioRecord.stop();
                }
                mAudioRecord.release();
                mAudioRecord = null;
           }
        }
        if (mAudioTrack != null) {
            stopAudioTrack();
        }

        if (initAudioRecordSink()) {
            mIsRender = true;
            synchronized (mRenderLock) {
                Log.i(TAG, "startRender: notifying for mRenderLock");
                mRenderLock.notify();
            }
        } else {
            Log.i(TAG, "initAudioRecordSink: fail");
        }
    }

    private synchronized void stopRender() {
        if (Log.isLoggable(TAG, Log.VERBOSE)) {
            Log.v(TAG, "stopRender");
        }
        synchronized (mRenderingLock) {
        Log.i(TAG, "stopRender_processing, mIsRender = " + isRender());
            boolean localRender = isRender();
            mIsRender = false;
            if (localRender) {
                try {
                         long wait = 200;
                         Log.i(TAG, "stopRender: waiting for mRenderingLock");
                         mRenderingLock.wait(wait);
                } catch (InterruptedException e) {
                     Log.w(TAG, "stopRender, thread is interrupted");
                 }
                }
        }
    }

    private synchronized void createRenderThread() {
            Log.d(TAG, "createRenderThread");
        if (mRenderThread == null) {
            mRenderThread = new RenderThread();
            mRenderThread.start();
        }
    }

    private synchronized void exitRenderThread() {
            Log.d(TAG, "exitRenderThread");
        stopRender();
        mRenderThread.interrupt();
        mRenderThread = null;
    }

    private Thread mRenderThread = null;
    private AudioRecord mAudioRecord = null;
    private AudioTrack mAudioTrack = null;
    private static final int SAMPLE_RATE = 44100;
    private static final int CHANNEL_CONFIG = AudioFormat.CHANNEL_CONFIGURATION_STEREO;
    private static final int AUDIO_FORMAT = AudioFormat.ENCODING_PCM_16BIT;
    private static final int RECORD_BUF_SIZE = AudioRecord.getMinBufferSize(SAMPLE_RATE,
            CHANNEL_CONFIG, AUDIO_FORMAT);
    private boolean mIsRender = false;

    AudioDevicePort mAudioSource = null;
    AudioDevicePort mAudioSink = null;

    private boolean mIsForbidCreateAudioPatch = false;

    private boolean isRendering() {
        return mIsRender;
    }

    private void startAudioTrack() {
        Log.d(TAG, "startAudioTrack, mAudioTrack = " + mAudioTrack);
        if (mAudioTrack == null) {
            return;
        }
        synchronized (mAudioTrack) {
            if (mAudioTrack != null && mAudioTrack.getPlayState() == AudioTrack.PLAYSTATE_STOPPED) {
                ArrayList<AudioPatch> patches = new ArrayList<AudioPatch>();
                mAudioManager.listAudioPatches(patches);
                mAudioTrack.play();
            }
        }
    }

    private void stopAudioTrack() {
        Log.d(TAG, "stopAudioTrack, mAudioTrack = " + mAudioTrack);
        if (mAudioTrack == null) {
            return;
        }
        try {
                synchronized (mAudioTrack) {
                    if (mAudioTrack.getPlayState() == AudioTrack.PLAYSTATE_PLAYING) {
                        mAudioTrack.stop();
                    }
                    mAudioTrack.release();
                    mAudioTrack = null;
                }
            } catch (IllegalStateException e) {
                Log.d(TAG, "stopAudioTrack, IllegalStateException");
            } catch (NullPointerException e) {
                Log.d(TAG, "stopAudioTrack, NullPointerException");
            }
    }

    class RenderThread extends Thread {
        private int mCurrentFrame = 0;
        private boolean isAudioFrameNeedIgnore() {
            return mCurrentFrame < AUDIO_FRAMES_TO_IGNORE_COUNT;
        }

        @Override
        public void run() {
            try {
                byte[] buffer = new byte[RECORD_BUF_SIZE];
                Log.e(TAG, "RenderThread, interrupted = " + Thread.interrupted());
                while (!Thread.interrupted()) {
                    if (Log.isLoggable(TAG, Log.VERBOSE)) {
                        Log.v(TAG, "RenderThread: run, isRender = " + isRender());
                    }
                    if (isRender()) {
                        // Speaker mode or BT a2dp mode will come here and keep reading and writing.
                        // If we want FM sound output from speaker or BT a2dp, we must record data
                        // to AudioRecrd and write data to AudioTrack.

                        if (mAudioRecord != null ) {
                            synchronized (mAudioRecord) {
                                if (mAudioRecord != null
                                    && mAudioRecord.getState() == AudioRecord.STATE_INITIALIZED
                                    && mAudioRecord.getRecordingState() ==
                                        AudioRecord.RECORDSTATE_STOPPED) {
                                    mAudioRecord.startRecording();
                                }
                            }
                        }
                        if (mAudioTrack != null ) {
                            synchronized (mAudioTrack) {
                                if (mAudioTrack != null
                                    && mAudioTrack.getState() == AudioTrack.STATE_INITIALIZED
                                    && mAudioTrack.getPlayState() == AudioTrack.PLAYSTATE_STOPPED) {
                                    mAudioTrack.play();
                                }
                            }
                        }
                        int size = 0;
                        if (mAudioRecord != null) {
                            size = mAudioRecord.read(buffer, 0, RECORD_BUF_SIZE);
                        }
                        // check whether need to ignore first 3 frames audio data from AudioRecord
                        // to avoid pop noise.
                        if (isAudioFrameNeedIgnore()) {
                            mCurrentFrame += 1;
                            synchronized (mRenderingLock) {
                                Log.i(TAG, "RenderThread: notifying for mRenderingLock");
                                mRenderingLock.notify();
                            }
                            continue ;
                        }
                        if (size <= 0) {
                            Log.e(TAG, "RenderThread read data from AudioRecord "
                                    + "error size: " + size);
                            synchronized (mRenderingLock) {
                                Log.i(TAG, "RenderThread: notifying for mRenderingLock");
                                mRenderingLock.notify();
                            }
                            continue;
                        }
                        byte[] tmpBuf = new byte[size];
                        System.arraycopy(buffer, 0, tmpBuf, 0, size);
                        // Check again to avoid noises, because mIsRender may be changed
                        // while AudioRecord is reading.
                        if (isRender()) {
                            if (mAudioTrack != null )
                                mAudioTrack.write(tmpBuf, 0, tmpBuf.length);
                        }
                        synchronized (mRenderingLock) {
                            if (Log.isLoggable(TAG, Log.VERBOSE)) {
                                Log.v(TAG, "RenderThread: notifying for mRenderingLock");
                            }
                            mRenderingLock.notify();
                        }
                    } else {
                        // Earphone mode will come here and wait.
                        mCurrentFrame = 0;
                        try {
                            if (mAudioTrack != null &&
                                mAudioTrack.getPlayState() == AudioTrack.PLAYSTATE_PLAYING) {
                                mAudioTrack.stop();
                            }

                            if (mAudioRecord != null &&
                                mAudioRecord.getRecordingState() ==
                                AudioRecord.RECORDSTATE_RECORDING) {
                                mAudioRecord.stop();
                            }
                        } catch (IllegalStateException e) {
                            Log.e(TAG, "RenderThread.run, IllegalStateException");
                        } finally {
                            synchronized (mRenderLock) {
                                Log.i(TAG, "RenderThread: waiting for mRenderLock");
                                mRenderLock.wait();
                            }
                        }
                    }
                }
            } catch (InterruptedException e) {
                Log.d(TAG, "RenderThread.run, thread is interrupted, need exit thread");
            } finally {
                if (mAudioRecord != null &&
                    (mAudioRecord.getRecordingState() == AudioRecord.RECORDSTATE_RECORDING)) {
                    mAudioRecord.stop();
                }

                if (mAudioTrack != null &&
                    (mAudioTrack.getPlayState() == AudioTrack.PLAYSTATE_PLAYING)) {
                    mAudioTrack.stop();
                }
            }
        }
    }

    // A2dp or speaker mode should render
    public boolean isRender() {
        return (mIsRender && (mPowerStatus == POWER_UP) && mIsAudioFocusHeld);
    }

    private boolean isSpeakerPhoneOn() {
        return (mForcedUseForMedia == AudioSystem.FORCE_SPEAKER);
    }

    /**
     * open FM device, should be call before power up
     *
     * @return true if FM device open, false FM device not open
     */
    private boolean openDevice() {
            Log.d(TAG, "openDevice");
        if (!mIsDeviceOpen) {
            mIsDeviceOpen = FmNative.openDev();
        }
        return mIsDeviceOpen;
    }

    /**
     * close FM device
     *
     * @return true if close FM device success, false close FM device failed
     */
    private boolean closeDevice() {
            Log.d(TAG, "closeDevice");
        boolean isDeviceClose = false;
        if (mIsDeviceOpen) {
            isDeviceClose = FmNative.closeDev();
            mIsDeviceOpen = !isDeviceClose;
        }
        if (mWakeLock != null && mWakeLock.isHeld()) {
            mWakeLock.release();
            mWakeLock = null;
        }
        // quit looper
        mFmServiceHandler.getLooper().quit();
        return isDeviceClose;
    }

    /**
     * get FM device opened or not
     *
     * @return true FM device opened, false FM device closed
     */
    public boolean isDeviceOpen() {
        Log.d(TAG, "isDeviceOpen" + mIsDeviceOpen);
        return mIsDeviceOpen;
    }

    /**
     * power up FM, and make FM voice output from earphone
     *
     * @param frequency
     */
    public void powerUpAsync(float frequency) {
            Log.d(TAG, "powerUpAsync, frequency = " + frequency);
        final int bundleSize = 1;
        mFmServiceHandler.removeMessages(FmListener.MSGID_POWERUP_FINISHED);
        mFmServiceHandler.removeMessages(FmListener.MSGID_POWERDOWN_FINISHED);
        mFmServiceHandler.removeMessages(FmListener.MSGID_AUDIOFOCUS_CHANGED);
        Log.d(TAG, "remove powerup/powerdown/audiofocus messages");
        Bundle bundle = new Bundle(bundleSize);
        bundle.putFloat(FM_FREQUENCY, frequency);
        Message msg = mFmServiceHandler.obtainMessage(FmListener.MSGID_POWERUP_FINISHED);
        msg.setData(bundle);
        mFmServiceHandler.sendMessage(msg);
    }

    private boolean powerUp(float frequency) {
        Log.d(TAG, "powerUp, frequency = " + frequency);

        if (mPowerStatus == POWER_UP) {
            Log.d(TAG, "powerUp already done");
            return true;
        }

        // check if audio mode is VOIP call, do not start FM
        if  ((mAudioManager.getMode() == AudioSystem.MODE_IN_COMMUNICATION)||
             (mAudioManager.getMode() == AudioSystem.MODE_IN_CALL)) {

        //AudioManager takes time to setMode so we need to wait before checking modes again (maximum 1 sec)
            int i=0;
            do {
                try {
                    final int hundredMillisecond = 250;
                    Thread.sleep(hundredMillisecond);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                i++;
            } while(((mAudioManager.getMode() == AudioSystem.MODE_IN_COMMUNICATION)||
                     (mAudioManager.getMode() == AudioSystem.MODE_IN_CALL))&&i<4);

            if((mAudioManager.getMode() == AudioSystem.MODE_IN_COMMUNICATION)||
                (mAudioManager.getMode() == AudioSystem.MODE_IN_CALL)){
                Log.d(TAG, "Audio is in communication mode, dont start FM");
                return false;
            }
        }

        mPowerStatus = DURING_POWER_UP;
        synchronized (mAudioPatchLock) {
            initAudioRecordSink();
            createAudioPatch();

            if (!FmUtils.isFmSuspendSupport()) {
            if (!mWakeLock.isHeld()) {
                mWakeLock.acquire();
            }
            }
            if (!requestAudioFocus()) {
                // activity used for update powerdown menu
                mPowerStatus = POWER_DOWN;
                mIsForbidCreateAudioPatch = false;
                Log.d(TAG, "release audio patch & stop audio track");
                releaseAudioPatch();
                stopAudioTrack();
                return false;
            }
        }

        // if device open fail when chip reset, it need open device again before
        // power up
        if (!mIsDeviceOpen) {
            openDevice();
        }

        if (!FmNative.powerUp(frequency)) {
            mPowerStatus = POWER_DOWN;
            mIsForbidCreateAudioPatch = false;
            if (mWakeLock != null && mWakeLock.isHeld()) {
                mWakeLock.release();
                Log.d(TAG, "powerUp failed, release wakelock: " + mWakeLock.isHeld());
            }
            return false;
        }
        mPowerStatus = POWER_UP;

        // for 192KHz mp3 play, need use render whatever
        // audio is output by earphone or speaker
        if (!mIsRender && mIsForbidCreateAudioPatch) {
            releaseAudioPatch();
            startRender();
        }
        // need mute after power up
        setMute(true);

        return (mPowerStatus == POWER_UP);
    }

    private boolean playFrequency(float frequency) {
            Log.d(TAG, "playFrequency, frequency = " + frequency);
        mCurrentStation = FmUtils.computeStation(frequency);
        FmStation.setCurrentStation(mContext, mCurrentStation);
        // Add notification to the title bar.
        updatePlayingNotification();

        // Start the RDS thread if RDS is supported.
        if (isRdsSupported()) {
            mIsRdsSupported = true;
            startRdsThread();
        }

        if (!FmUtils.isFmSuspendSupport()) {
        if (!mWakeLock.isHeld()) {
            mWakeLock.acquire();
        }
        }
        /* if (mIsSpeakerUsed != isSpeakerPhoneOn()) {
            setForceUse(mIsSpeakerUsed);
        } */
        if (mRecordState != FmRecorder.STATE_PLAYBACK) {
            enableFmAudio(true);
        }

        setRds(true);
        setMute(false);

        return (mPowerStatus == POWER_UP);
    }

    /**
     * power down FM
     */
    public void powerDownAsync() {
            Log.d(TAG, "powerDownAsync");
        // if power down Fm, should remove message first.
        // not remove all messages, because such as recorder message need
        // to execute after or before power down
        mFmServiceHandler.removeMessages(FmListener.MSGID_SCAN_FINISHED);
        mFmServiceHandler.removeMessages(FmListener.MSGID_SEEK_FINISHED);
        mFmServiceHandler.removeMessages(FmListener.MSGID_TUNE_FINISHED);
        mFmServiceHandler.removeMessages(FmListener.MSGID_POWERDOWN_FINISHED);
        mFmServiceHandler.removeMessages(FmListener.MSGID_POWERUP_FINISHED);
        mFmServiceHandler.sendEmptyMessage(FmListener.MSGID_POWERDOWN_FINISHED);
    }

    /**
     * Power down FM
     *
     * @return true if power down success
     */
    private boolean powerDown() {
            Log.d(TAG, "powerDown");
        if (mPowerStatus == POWER_DOWN) {
            return true;
        }

        setMute(true);
        setRds(false);
        enableFmAudio(false);

        if (!FmNative.powerDown(0)) {

            if (isRdsSupported()) {
                stopRdsThread();
            }

            if (!FmUtils.isFmSuspendSupport()) {
            if (mWakeLock.isHeld()) {
                mWakeLock.release();
            }
            }
            // Remove the notification in the title bar.
            removeNotification();
            return false;
        }
        // activity used for update powerdown menu
        mPowerStatus = POWER_DOWN;

        if (isRdsSupported()) {
            stopRdsThread();
        }

        if (!FmUtils.isFmSuspendSupport()) {
        if (mWakeLock.isHeld()) {
            mWakeLock.release();
        }
        }
        mIsForbidCreateAudioPatch = false;
        // Remove the notification in the title bar.
        removeNotification();
        return true;
    }

    public int getPowerStatus() {
        return mPowerStatus;
    }

    /**
     * Tune to a station
     *
     * @param frequency The frequency to tune
     *
     * @return true, success; false, fail.
     */
    public void tuneStationAsync(float frequency) {
            Log.d(TAG, "tuneStationAsync, frequency = " + frequency);
        mFmServiceHandler.removeMessages(FmListener.MSGID_TUNE_FINISHED);
        final int bundleSize = 1;
        Bundle bundle = new Bundle(bundleSize);
        bundle.putFloat(FM_FREQUENCY, frequency);
        Message msg = mFmServiceHandler.obtainMessage(FmListener.MSGID_TUNE_FINISHED);
        msg.setData(bundle);
        mFmServiceHandler.sendMessage(msg);
    }

    private boolean tuneStation(float frequency) {
            Log.d(TAG, "tuneStation, frequency = " + frequency);
        // clear ps and rt when switch to new station,
        // so that ps and rt can refresh to UI.
        mPsString = "";
        mRtTextString = "";
        if (mPowerStatus == POWER_UP) {
            setRds(false);
            boolean bRet = FmNative.tune(frequency);
            if (bRet) {
                setRds(true);
                mCurrentStation = FmUtils.computeStation(frequency);
                FmStation.setCurrentStation(mContext, mCurrentStation);
                // update notification on main thread
                Handler mainHandler = new Handler(getMainLooper());
                mainHandler.post(new Runnable() {
                    @Override
                    public void run() {
                updatePlayingNotification();
            }
                });
            }
            setMute(false);
            return bRet;
        }

        // if earphone is not insert, not power up
        if (!isAntennaAvailable()) {
            return false;
        }

        // if not power up yet, should powerup first
        boolean tune = false;

        if (powerUp(frequency)) {
            tune = playFrequency(frequency);
        }

        return tune;
    }

    /**
     * Seek station according frequency and direction
     *
     * @param frequency start frequency(100KHZ, 87.5)
     * @param isUp direction(true, next station; false, previous station)
     *
     * @return the frequency after seek
     */
    public void seekStationAsync(float frequency, boolean isUp) {
            Log.d(TAG, "seekStationAsync, frequency = " + frequency + ", isUp = " + isUp);
        mFmServiceHandler.removeMessages(FmListener.MSGID_SEEK_FINISHED);
        final int bundleSize = 2;
        Bundle bundle = new Bundle(bundleSize);
        bundle.putFloat(FM_FREQUENCY, frequency);
        bundle.putBoolean(OPTION, isUp);
        Message msg = mFmServiceHandler.obtainMessage(FmListener.MSGID_SEEK_FINISHED);
        msg.setData(bundle);
        mFmServiceHandler.sendMessage(msg);
    }

    private float seekStation(float frequency, boolean isUp) {
            Log.d(TAG, "seekStation, frequency = " + frequency + ", isUp = " + isUp);
        if (mPowerStatus != POWER_UP) {
            return -1;
        }

        setRds(false);
        mIsNativeSeeking = true;
        float fRet = FmNative.seek(frequency, isUp);
        mIsNativeSeeking = false;
        // make mIsStopScanCalled false, avoid stop scan make this true,
        // when start scan, it will return null.
        mIsStopScanCalled = false;
        return fRet;
    }

    /**
     * Scan stations
     */
    public void startScanAsync() {
            Log.d(TAG, "startScanAsync");
        mFmServiceHandler.removeMessages(FmListener.MSGID_SCAN_FINISHED);
        mFmServiceHandler.sendEmptyMessage(FmListener.MSGID_SCAN_FINISHED);
    }

    private int[] startScan() {
            Log.d(TAG, "startScan");
        int[] stations = null;

        setRds(false);
        setMute(true);
        short[] stationsInShort = null;
        if (!mIsStopScanCalled) {
            mIsNativeScanning = true;
            stationsInShort = FmNative.autoScan();
            mIsNativeScanning = false;
        }

        setRds(true);
        if (mIsStopScanCalled) {
            // Received a message to power down FM, or interrupted by a phone
            // call. Do not return any stations. stationsInShort = null;
            // if cancel scan, return invalid station -100
            stationsInShort = new short[] {
                -100
            };
            mIsStopScanCalled = false;
        }

        if (null != stationsInShort) {
            int size = stationsInShort.length;
            stations = new int[size];
            for (int i = 0; i < size; i++) {
                stations[i] = stationsInShort[i];
            }
        }
        return stations;
    }

    /**
     * Check FM Radio is in scan progress or not
     *
     * @return if in scan progress return true, otherwise return false.
     */
    public boolean isScanning() {
        return mIsScanning;
    }

    /**
     * Stop scan progress
     *
     * @return true if can stop scan, otherwise return false.
     */
    public boolean stopScan() {
            Log.d(TAG, "stopScan");
        if (mPowerStatus != POWER_UP) {
            return false;
        }

        boolean bRet = false;
        mFmServiceHandler.removeMessages(FmListener.MSGID_SCAN_FINISHED);
        mFmServiceHandler.removeMessages(FmListener.MSGID_SEEK_FINISHED);
        if (mIsNativeScanning || mIsNativeSeeking) {
            mIsStopScanCalled = true;
            bRet = FmNative.stopScan();
        }
        return bRet;
    }

    /**
     * Check FM is in seek progress or not
     *
     * @return true if in seek progress, otherwise return false.
     */
    public boolean isSeeking() {
        return mIsNativeSeeking;
    }

    /**
     * Set RDS
     *
     * @param on true, enable RDS; false, disable RDS.
     */
    public void setRdsAsync(boolean on) {
            Log.d(TAG, "setRdsAsync, on = " + on);
        final int bundleSize = 1;
        mFmServiceHandler.removeMessages(FmListener.MSGID_SET_RDS_FINISHED);
        Bundle bundle = new Bundle(bundleSize);
        bundle.putBoolean(OPTION, on);
        Message msg = mFmServiceHandler.obtainMessage(FmListener.MSGID_SET_RDS_FINISHED);
        msg.setData(bundle);
        mFmServiceHandler.sendMessage(msg);
    }

    private int setRds(boolean on) {
            Log.d(TAG, "setRds, on = " + on);
        if (mPowerStatus != POWER_UP) {
            return -1;
        }
        int ret = -1;
        if (isRdsSupported()) {
            ret = FmNative.setRds(on);
        }
        return ret;
    }

    /**
     * Get PS information
     *
     * @return PS information
     */
    public String getPs() {
        return mPsString;
    }

    /**
     * Get RT information
     *
     * @return RT information
     */
    public String getRtText() {
        return mRtTextString;
    }

    /**
     * Get AF frequency
     *
     * @return AF frequency
     */
    public void activeAfAsync() {
        mFmServiceHandler.removeMessages(FmListener.MSGID_ACTIVE_AF_FINISHED);
        mFmServiceHandler.sendEmptyMessage(FmListener.MSGID_ACTIVE_AF_FINISHED);
    }

    private int activeAf() {
        if (mPowerStatus != POWER_UP) {
            Log.w(TAG, "activeAf, FM is not powered up");
            return -1;
        }

        int frequency = FmNative.activeAf();
        return frequency;
    }

    /**
     * Mute or unmute FM voice
     *
     * @param mute true for mute, false for unmute
     *
     * @return (true, success; false, failed)
     */
    public void setMuteAsync(boolean mute) {
            Log.d(TAG, "setMuteAsync, mute = " + mute);
        mFmServiceHandler.removeMessages(FmListener.MSGID_SET_MUTE_FINISHED);
        final int bundleSize = 1;
        Bundle bundle = new Bundle(bundleSize);
        bundle.putBoolean(OPTION, mute);
        Message msg = mFmServiceHandler.obtainMessage(FmListener.MSGID_SET_MUTE_FINISHED);
        msg.setData(bundle);
        mFmServiceHandler.sendMessage(msg);
    }

    /**
     * Mute or unmute FM voice
     *
     * @param mute true for mute, false for unmute
     *
     * @return (1, success; other, failed)
     */
    public int setMute(boolean mute) {
            Log.d(TAG, "setMute, mute = " + mute);
        if (mPowerStatus != POWER_UP) {
            Log.w(TAG, "setMute, FM is not powered up");
            return -1;
        }
        int iRet = FmNative.setMute(mute);
        mIsMuted = mute;
        return iRet;
    }

    /**
     * Check the latest status is mute or not
     *
     * @return (true, mute; false, unmute)
     */
    public boolean isMuted() {
        return mIsMuted;
    }

    /**
     * Check whether RDS is support in driver
     *
     * @return (true, support; false, not support)
     */
    public boolean isRdsSupported() {
        boolean isRdsSupported = (FmNative.isRdsSupport() == 1);
        return isRdsSupported;
    }

    /**
     * Check whether speaker used or not
     *
     * @return true if use speaker, otherwise return false
     */
    public boolean isSpeakerUsed() {
        return mIsSpeakerUsed;
    }

    /**
     * Initial service and current station
     *
     * @param iCurrentStation current station frequency
     */
    public void initService(int iCurrentStation) {
        mIsServiceInited = true;
        mCurrentStation = iCurrentStation;
    }

    /**
     * Check service is initialed or not
     *
     * @return true if initialed, otherwise return false
     */
    public boolean isServiceInited() {
        return mIsServiceInited;
    }

    /**
     * Get FM service current station frequency
     *
     * @return Current station frequency
     */
    public int getFrequency() {
        return mCurrentStation;
    }

    /**
     * Set FM service station frequency
     *
     * @param station Current station
     */
    public void setFrequency(int station) {
        mCurrentStation = station;
    }

    /**
     * resume FM audio
     */
    private void resumeFmAudio() {
            Log.d(TAG, "resumeFmAudio, held = " + mIsAudioFocusHeld + "power = " + mPowerStatus);
        // If not check mIsAudioFocusHeld && power up, when scan canceled,
        // this will be resume first, then execute power down. it will cause
        // nosise.
        if (mIsAudioFocusHeld && (mPowerStatus == POWER_UP)) {
            enableFmAudio(true);
        }
    }

    /**
     * Switch antenna There are two types of antenna(long and short) If long
     * antenna(most is this type), must plug in earphone as antenna to receive
     * FM. If short antenna, means there is a short antenna if phone already,
     * can receive FM without earphone.
     *
     * @param antenna antenna (0, long antenna, 1 short antenna)
     *
     * @return (0, success; 1 failed; 2 not support)
     */
    public void switchAntennaAsync(int antenna) {
            Log.d(TAG, "switchAntennaAsync, antenna = " + antenna);
        final int bundleSize = 1;
        mFmServiceHandler.removeMessages(FmListener.MSGID_SWITCH_ANTENNA);

        Bundle bundle = new Bundle(bundleSize);
        bundle.putInt(FmListener.SWITCH_ANTENNA_VALUE, antenna);
        Message msg = mFmServiceHandler.obtainMessage(FmListener.MSGID_SWITCH_ANTENNA);
        msg.setData(bundle);
        mFmServiceHandler.sendMessage(msg);
    }

    /**
     * Need native support whether antenna support interface.
     *
     * @param antenna antenna (0, long antenna, 1 short antenna)
     *
     * @return (0, success; 1 failed; 2 not support)
     */
    private int switchAntenna(int antenna) {
            Log.d(TAG, "switchAntenna, antenna = " + antenna);
        // if fm not powerup, switchAntenna will flag whether has earphone
        int ret = FmNative.switchAntenna(antenna);
        return ret;
    }

    /**
     * Start recording
     */
    public void startRecordingAsync() {
            Log.d(TAG, "startRecordingAsync");
        mFmServiceHandler.removeMessages(FmListener.MSGID_STARTRECORDING_FINISHED);
        mFmServiceHandler.sendEmptyMessage(FmListener.MSGID_STARTRECORDING_FINISHED);
    }

    private void startRecording() {
            Log.d(TAG, "startRecording");
        sRecordingSdcard = FmUtils.getDefaultStoragePath();
        if (sRecordingSdcard == null || sRecordingSdcard.isEmpty()) {
            Log.d(TAG, "startRecording, may be no sdcard");
            onRecorderError(FmRecorder.ERROR_SDCARD_NOT_PRESENT);
            return;
        }

        if (mFmRecorder == null) {
            mFmRecorder = new FmRecorder();
            mFmRecorder.registerRecorderStateListener(FmService.this);
        }

        if (isSdcardReady(sRecordingSdcard)) {
            mFmRecorder.startRecording(mContext);
        } else {
            onRecorderError(FmRecorder.ERROR_SDCARD_NOT_PRESENT);
        }
    }

    private boolean isSdcardReady(String sdcardPath) {
        if (!mSdcardStateMap.isEmpty()) {
            if (mSdcardStateMap.get(sdcardPath) != null && !mSdcardStateMap.get(sdcardPath)) {
                Log.d(TAG, "isSdcardReady, return false");
                return false;
            }
        }
        return true;
    }

    /**
     * stop recording
     */
    public void stopRecordingAsync() {
            Log.d(TAG, "stopRecordingAsync");
        mFmServiceHandler.removeMessages(FmListener.MSGID_STOPRECORDING_FINISHED);
        mFmServiceHandler.sendEmptyMessage(FmListener.MSGID_STOPRECORDING_FINISHED);
    }

    private boolean stopRecording() {
            Log.d(TAG, "stopRecording, mFmRecorder" + mFmRecorder);
        if (mFmRecorder == null) {
            Log.e(TAG, "stopRecording, called without a valid recorder!!");
            return false;
        }
        synchronized (mStopRecordingLock) {
            mFmRecorder.stopRecording();
        }
        return true;
    }

    /**
     * Save recording file according name or discard recording file if name is
     * null
     *
     * @param newName New recording file name
     */
    public void saveRecordingAsync(String newName) {
            Log.d(TAG, "saveRecordingAsync, name = " + newName);
        mFmServiceHandler.removeMessages(FmListener.MSGID_SAVERECORDING_FINISHED);
        final int bundleSize = 1;
        Bundle bundle = new Bundle(bundleSize);
        bundle.putString(RECODING_FILE_NAME, newName);
        Message msg = mFmServiceHandler.obtainMessage(FmListener.MSGID_SAVERECORDING_FINISHED);
        msg.setData(bundle);
        mFmServiceHandler.sendMessage(msg);
    }

    /**
     * Discard recording
     */
    public void discardRecording() {
        Log.d(TAG, "discardRecording");
        if (mFmRecorder != null) {
            mFmRecorder.discardRecording();
        }
    }

    private void saveRecording(String newName) {
            Log.d(TAG, "saveRecording, name = " + newName);
        if (mFmRecorder != null) {
            if (newName != null) {
                mFmRecorder.saveRecording(FmService.this, newName);
                notifyRecordingFinished(newName);
                return;
            }
            mFmRecorder.discardRecording();
        }
    }

    private void notifyRecordingFinished(String newName) {
        Log.d(TAG, "notifyRecordingFinished");
        Bundle bundle = new Bundle(2);
        bundle.putInt(FmListener.CALLBACK_FLAG, FmListener.MSGID_SAVERECORDING_FINISHED);
        bundle.putString(FmListener.KEY_RECORDING_NAME, newName);
        notifyActivityStateChanged(bundle);
    }

    /**
     * Get record time
     *
     * @return Record time
     */
    public long getRecordTime() {
        if (mFmRecorder != null) {
            return mFmRecorder.getRecordTime();
        }
        return 0;
    }

    /**
     * Set recording mode
     *
     * @param isRecording true, enter recoding mode; false, exit recording mode
     */
    public void setRecordingModeAsync(boolean isRecording) {
            Log.d(TAG, "setRecordingModeAsync, isRecording = " + isRecording);
        mFmServiceHandler.removeMessages(FmListener.MSGID_RECORD_MODE_CHANED);
        final int bundleSize = 1;
        Bundle bundle = new Bundle(bundleSize);
        bundle.putBoolean(OPTION, isRecording);
        Message msg = mFmServiceHandler.obtainMessage(FmListener.MSGID_RECORD_MODE_CHANED);
        msg.setData(bundle);
        mFmServiceHandler.sendMessage(msg);
    }

    private void setRecordingMode(boolean isRecording) {
            Log.d(TAG, "setRecordingMode, isRecording = " + isRecording);
        mIsInRecordingMode = isRecording;
        if (mFmRecorder != null) {
            if (!isRecording) {
                if (mFmRecorder.getState() != FmRecorder.STATE_IDLE) {
                    mFmRecorder.stopRecording();
                }
                resumeFmAudio();
                setMute(false);
                return;
            }
            // reset recorder to unused status
            mFmRecorder.resetRecorder();
        }
    }

    /**
     * Get current recording mode
     *
     * @return if in recording mode return true, otherwise return false;
     */
    public boolean getRecordingMode() {
        return mIsInRecordingMode;
    }

    /**
     * Get record state
     *
     * @return record state
     */
    public int getRecorderState() {
        if (null != mFmRecorder) {
            return mFmRecorder.getState();
        }
        return FmRecorder.STATE_INVALID;
    }

    /**
     * Get recording file name
     *
     * @return recording file name
     */
    public String getRecordingName() {
        if (null != mFmRecorder) {
            return mFmRecorder.getRecordFileName();
        }
        return null;
    }

    @Override
    public void onCreate() {
            Log.d(TAG, "onCreate");
        super.onCreate();
        mContext = getApplicationContext();
        mAudioManager = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
        mActivityManager = (ActivityManager) getSystemService(Context.ACTIVITY_SERVICE);
        PowerManager powerManager = (PowerManager) getSystemService(Context.POWER_SERVICE);
        mWakeLock = powerManager.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, TAG);
        if (!FmUtils.isFmSuspendSupport()) {
        mWakeLock.setReferenceCounted(false);
        }
        sRecordingSdcard = FmUtils.getDefaultStoragePath();

        registerFmBroadcastReceiver();
        registerSdcardReceiver();
        registerAudioPortUpdateListener();

        HandlerThread handlerThread = new HandlerThread("FmRadioServiceThread");
        handlerThread.start();
        mFmServiceHandler = new FmRadioServiceHandler(handlerThread.getLooper());

        openDevice();
        // set speaker to default status, avoid setting->clear data.
        setForceUse(mIsSpeakerUsed);

        initAudioRecordSink();
        createRenderThread();

        // create notification channel. support for SDk version 26
        NotificationManager mgr = (NotificationManager) mContext.
                                getSystemService(Context.NOTIFICATION_SERVICE);
        NotificationChannel mChannel = new NotificationChannel(
                                NOTIFICATION_CHANNEL_FMRADIO,
                                mContext.getString(R.string.app_name),
                                NotificationManager.IMPORTANCE_LOW);
        mChannel.setShowBadge(false);
        mgr.createNotificationChannel(mChannel);
    }

    private void registerAudioPortUpdateListener() {
            Log.d(TAG, "registerAudioPortUpdateListener, list = " + mAudioPortUpdateListener);
        if (mAudioPortUpdateListener == null) {
            mAudioPortUpdateListener = new FmOnAudioPortUpdateListener();
            mAudioManager.registerAudioPortUpdateListener(mAudioPortUpdateListener);
        }
    }

    private void unregisterAudioPortUpdateListener() {
            Log.d(TAG, "unregisterAudioPortUpdateListener, list = " + mAudioPortUpdateListener);
        if (mAudioPortUpdateListener != null) {
            mAudioManager.unregisterAudioPortUpdateListener(mAudioPortUpdateListener);
            mAudioPortUpdateListener = null;
        }
    }

    // This function may be called in different threads.
    // Need to add "synchronized" to make sure mAudioRecord and mAudioTrack are the newest.
    // Thread 1: onCreate() or startRender()
    // Thread 2: onAudioPatchListUpdate() or startRender()
    private synchronized boolean initAudioRecordSink() {

        // free if any previous audiorecord or audiotrack are present,
        if (mAudioRecord != null) {
            synchronized (mAudioRecord) {
                if (mAudioRecord.getRecordingState() == AudioRecord.RECORDSTATE_RECORDING) {
                    mAudioRecord.stop();
                }
                mAudioRecord.release();
                mAudioRecord = null;
            }
        }
        stopAudioTrack();

        mAudioRecord = new AudioRecord(MediaRecorder.AudioSource.RADIO_TUNER,
                SAMPLE_RATE, CHANNEL_CONFIG, AUDIO_FORMAT, RECORD_BUF_SIZE);
        if (mAudioRecord == null) {
            return false;
        }
        mAudioTrack = new AudioTrack(AudioManager.STREAM_MUSIC,
                SAMPLE_RATE, CHANNEL_CONFIG, AUDIO_FORMAT, RECORD_BUF_SIZE, AudioTrack.MODE_STREAM);
        Log.d(TAG, "initAudioRecordSink, mAudioRecord = " + mAudioRecord + ",mAudioTrack = "
              + mAudioTrack);

        if (mAudioTrack == null) {
            if (mAudioRecord != null) {
                mAudioRecord.release();
                mAudioRecord = null;
            }
            return false;
        }
        return true;
    }


    private int getDeviceForStream() {
        Log.d(TAG, "getDeviceForStream");
        return mAudioManager.getDevicesForStream(AudioManager.STREAM_MUSIC) ;
    }


    private synchronized void createAudioPatch() {
        Log.d(TAG, "createAudioPatch, mAudioPatch = " + mAudioPatch);
        if (mIsForbidCreateAudioPatch) {
            releaseAudioPatch();
            if (!mIsRender) {
                startRender();
            }
            return;
        }
        if (mAudioPatch == null) {
            ArrayList<AudioPatch> patches = new ArrayList<AudioPatch>();
            mAudioManager.listAudioPatches(patches);
            int deviceForStream = getDeviceForStream();
            Log.d(TAG, "deviceForStream " + deviceForStream + " sIsSmartPAandWithoutDSP:"
                + sIsSmartPAandWithoutDSP);
            //if (isPatchMixerToEarphone(patches)) {
            if (deviceForStream == AudioManager.DEVICE_OUT_WIRED_HEADSET
                || deviceForStream == AudioManager.DEVICE_OUT_WIRED_HEADPHONE) {
                Log.d(TAG, "createAudioPatch earphone");
                stopRender();
                stopAudioTrack();
                createAudioPatchByEarphone();
            } else if (isRenderForSpeaker()) {
                if (isRender()) {
                    stopRender();
                }
                startRender();

            //} else if (isPatchMixerToSpeaker(patches)) {
            } else if (deviceForStream == AudioManager.DEVICE_OUT_SPEAKER) {
                Log.d(TAG, "createAudioPatch speaker");
                stopRender();
                stopAudioTrack();
                createAudioPatchBySpeaker();
            } else if ((deviceForStream ==
                    (AudioManager.DEVICE_OUT_WIRED_HEADSET | AudioManager.DEVICE_OUT_SPEAKER))
                 || (deviceForStream ==
                    (AudioManager.DEVICE_OUT_WIRED_HEADPHONE | AudioManager.DEVICE_OUT_SPEAKER))) {
                    Log.d(TAG, "createAudioPatch earphone+speaker");
                stopRender();
                stopAudioTrack();
                createAudioPatchBySpeakerAndEarphone();
            } else {
                if (isRender()) {
                    stopRender();
                }
                startRender();
            }
        }
        Log.d(TAG, "createAudioPatch, mAudioPatch created = " + mAudioPatch);
    }

    private synchronized void createAudioPatchByEarphone() {
        Log.d(TAG, "createAudioPatchByEarphone " + mIsForbidCreateAudioPatch);
        if (mIsForbidCreateAudioPatch) {
            releaseAudioPatch();
            if (!mIsRender) {
                startRender();
            }
            return;
        }
        if (mAudioPatch != null) {
            Log.d(TAG, "createAudioPatch, mAudioPatch is not null, return");
            return;
        }

        if (mIsSpeakerUsed) {
            // audio system config has been modified by others,
            // so need update this state.
            mIsSpeakerUsed = false;
            notifySpeakerModeChange();
        }
        mAudioSource = null;
        mAudioSink = null;
        ArrayList<AudioPort> ports = new ArrayList<AudioPort>();
        mAudioManager.listAudioPorts(ports);
        for (AudioPort port : ports) {
            if (port instanceof AudioDevicePort) {
                int type = ((AudioDevicePort) port).type();
                String name = AudioSystem.getOutputDeviceName(type);
                if (type == AudioSystem.DEVICE_IN_FM_TUNER) {
                    mAudioSource = (AudioDevicePort) port;
                } else if (type == AudioSystem.DEVICE_OUT_WIRED_HEADSET ||
                        type == AudioSystem.DEVICE_OUT_WIRED_HEADPHONE) {
                    mAudioSink = (AudioDevicePort) port;
                }
            }
        }
        if (mAudioSource != null && mAudioSink != null) {
            AudioDevicePortConfig sourceConfig = (AudioDevicePortConfig) mAudioSource
                    .activeConfig();
            AudioDevicePortConfig sinkConfig = (AudioDevicePortConfig) mAudioSink.activeConfig();
            AudioPatch[] audioPatchArray = new AudioPatch[] {null};
            int res = mAudioManager.createAudioPatch(audioPatchArray,
                    new AudioPortConfig[] {sourceConfig},
                    new AudioPortConfig[] {sinkConfig});
            if (res == AudioManager.ERROR_INVALID_OPERATION) {
                mIsForbidCreateAudioPatch = true;
            }
            mAudioPatch = audioPatchArray[0];
        }
    }

    private synchronized void createAudioPatchBySpeaker() {
        Log.d(TAG, "createAudioPatchBySpeaker");
        if (mIsForbidCreateAudioPatch) {
            releaseAudioPatch();
            if (!mIsRender) {
                startRender();
            }
            return;
        }
        if (mAudioPatch != null) {
            Log.d(TAG, "createAudioPatch, mAudioPatch is not null, return");
            return;
        }

        if (!mIsSpeakerUsed) {
            // audio system config has been modified by others,
            // so need update this state.
            mIsSpeakerUsed = true;
            notifySpeakerModeChange();
        }

        mAudioSource = null;
        mAudioSink = null;
        ArrayList<AudioPort> ports = new ArrayList<AudioPort>();
        mAudioManager.listAudioPorts(ports);
        for (AudioPort port : ports) {
            if (port instanceof AudioDevicePort) {
                int type = ((AudioDevicePort) port).type();
                String name = AudioSystem.getOutputDeviceName(type);
                if (type == AudioSystem.DEVICE_IN_FM_TUNER) {
                    mAudioSource = (AudioDevicePort) port;
                } else if (type == AudioSystem.DEVICE_OUT_SPEAKER) {
                    mAudioSink = (AudioDevicePort) port;
                }
            }
        }
        if (mAudioSource != null && mAudioSink != null) {
            AudioDevicePortConfig sourceConfig = (AudioDevicePortConfig) mAudioSource
                    .activeConfig();
            AudioDevicePortConfig sinkConfig = (AudioDevicePortConfig) mAudioSink.activeConfig();
            AudioPatch[] audioPatchArray = new AudioPatch[] {null};
            int res = mAudioManager.createAudioPatch(audioPatchArray,
                    new AudioPortConfig[] {sourceConfig},
                    new AudioPortConfig[] {sinkConfig});
            if (res == AudioManager.ERROR_INVALID_OPERATION) {
                mIsForbidCreateAudioPatch = true;
            }
            mAudioPatch = audioPatchArray[0];
        }
    }

    private synchronized void createAudioPatchBySpeakerAndEarphone() {
        Log.d(TAG, "createAudioPatchBySpeakerAndEarphone");
        if (mIsForbidCreateAudioPatch) {
            releaseAudioPatch();
            if (!mIsRender) {
                startRender();
            }
            return;
        }
        if (mAudioPatch != null) {
            Log.d(TAG, "createAudioPatchBySpeakerAndEarphone, mAudioPatch is not null, return");
            return;
        }

        mAudioSource = null;
        AudioDevicePort speakerSink = null;
        AudioDevicePort earphoneSink = null;
        ArrayList<AudioPort> ports = new ArrayList<AudioPort>();
        mAudioManager.listAudioPorts(ports);
        for (AudioPort port : ports) {
            if (port instanceof AudioDevicePort) {
                int type = ((AudioDevicePort) port).type();
                String name = AudioSystem.getOutputDeviceName(type);
                if (type == AudioSystem.DEVICE_IN_FM_TUNER) {
                    mAudioSource = (AudioDevicePort) port;
                } else if (type == AudioSystem.DEVICE_OUT_SPEAKER) {
                    speakerSink = (AudioDevicePort) port;
                } else if (type == AudioSystem.DEVICE_OUT_WIRED_HEADSET ||
                        type == AudioSystem.DEVICE_OUT_WIRED_HEADPHONE) {
                    earphoneSink = (AudioDevicePort) port;
                }
            }
        }
        if (mAudioSource != null && speakerSink != null && earphoneSink != null) {
            AudioDevicePortConfig sourceConfig = (AudioDevicePortConfig) mAudioSource
                    .activeConfig();
            AudioDevicePortConfig speakerSinkConfig = (AudioDevicePortConfig) speakerSink
                    .activeConfig();
            AudioDevicePortConfig earphoneSinkConfig = (AudioDevicePortConfig) earphoneSink
                    .activeConfig();
            AudioPatch[] audioPatchArray = new AudioPatch[] {null};
            int res = mAudioManager.createAudioPatch(audioPatchArray,
                    new AudioPortConfig[] {sourceConfig},
                    new AudioPortConfig[] {speakerSinkConfig, earphoneSinkConfig});
            if (res == AudioManager.ERROR_INVALID_OPERATION) {
                mIsForbidCreateAudioPatch = true;
            }
            mAudioPatch = audioPatchArray[0];
        }
    }

    private FmOnAudioPortUpdateListener mAudioPortUpdateListener = null;

    private class FmOnAudioPortUpdateListener implements OnAudioPortUpdateListener {
        /**
         * Callback method called upon audio port list update.
         * @param portList the updated list of audio ports
         */
        @Override
        public void onAudioPortListUpdate(AudioPort[] portList) {
            // Ingore audio port update
        }

        /**
         * Callback method called upon audio patch list update.
         *
         * @param patchList the updated list of audio patches
         */
        @Override
        public void onAudioPatchListUpdate(AudioPatch[] patchList) {
            if (Log.isLoggable(TAG, Log.VERBOSE)) {
                Log.v(TAG, "onAudioPatchListUpdate: entry");
            }
            if (FmService.this.isBluetoothHeadsetInUse() != mPrevBtHeadsetState) {
                mPrevBtHeadsetState = !mPrevBtHeadsetState;

                // if BT state change from disconnected to connected, set speaker
                // state to false, because audio framework will set force use to none
                if (mPrevBtHeadsetState) {
                    mIsSpeakerUsed = false;
                }
                Bundle bundle = new Bundle(2);
                bundle.putInt(FmListener.CALLBACK_FLAG,
                        FmListener.MSGID_BT_STATE_CHANGED);
                bundle.putBoolean(FmListener.KEY_BT_STATE, mPrevBtHeadsetState);
                notifyActivityStateChanged(bundle);
            }
            if (mPowerStatus == POWER_DOWN) {
                Log.d(TAG, "onAudioPatchListUpdate, not power up" + mPowerStatus);
                return;
            }

            // check if audio mode is VOIP call, do not start FM
            if (mAudioManager.getMode() == AudioSystem.MODE_IN_COMMUNICATION && mIsAudioFocusHeld) {
                Log.d(TAG, "Audio is in communication mode, consider it as audio focus lost");
                synchronized (this) {
                    setMute(true);
                    focusChanged(AudioManager.AUDIOFOCUS_LOSS);
                }
                return ;
            }
            /* Design Change in p0.mp1:
               when FM and 192K Music are playing at same time,
               FM will keep direct mode and use 48K sample rate.
               so removing mIsPlaying192KHz 
            */
            /*
            if (mIsPlaying192KHz == false) {
                String state = mAudioManager.getParameters(GET_HIFI_UI_STATUS);
                mIsPlaying192KHz = HIFI_UI_STATUS_ENABLED.equals(state);
                Log.d(TAG, "HIFI state = " + state);
            }
            */

            synchronized (mAudioPatchLock) {

            Log.d(TAG, "audiopatch" + mAudioPatch + ", misnreder" + mIsRender);

            if (!mIsAudioFocusHeld) {
                Log.d(TAG, "onAudioPatchListUpdate no audio focus");
                return;
            }

            if (mIsForbidCreateAudioPatch) {
                Log.d(TAG, "onAudioPatchListUpdate inPlaying192Khz " + mIsRender);
                releaseAudioPatch();
                if (!mIsRender) {
                    startRender();
                }
                return;
            }

            if (mAudioPatch != null) {
                ArrayList<AudioPatch> patches = new ArrayList<AudioPatch>();
                mAudioManager.listAudioPatches(patches);
                // When BT or WFD is connected, native will remove the patch (mixer -> device).
                // Need to recreate AudioRecord and AudioTrack for this case.
                int deviceForStream = getDeviceForStream();
                Log.d(TAG, "deviceForStream " + deviceForStream + " sIsSmartPAandWithoutDSP:"
                    + sIsSmartPAandWithoutDSP);
                if (isPatchMixerToDeviceRemoved(patches) || isPatchMixerToBt(patches) ||
                                isBtDevice(deviceForStream)) {
                    Log.d(TAG, "onAudioPatchListUpdate reinit for BT or WFD connected");
                    stopRender();
                    releaseAudioPatch();
                    startRender();
                    notifySpeakerModeChange();
                    return;
                }
                if (isPatchContainSpeakerAndEarphone(patches)
                    &&
                    ((deviceForStream == (AudioManager.DEVICE_OUT_WIRED_HEADSET | AudioManager.DEVICE_OUT_SPEAKER))
                    ||
                    (deviceForStream == (AudioManager.DEVICE_OUT_WIRED_HEADPHONE | AudioManager.DEVICE_OUT_SPEAKER)))) {
                    // TODO add this case to avoid noise when play ringtone(output with speaker
                    // and earphone), native audio need FM create FM->Speaker+Earphone patch to
                    // fixed this noise issue when switch output device.
                    stopRender();
                    try {
                        AudioPortConfig[] currentSinks = mAudioPatch.sinks();
                        if (currentSinks.length == 1) {
                            Log.d(TAG, "DEBUG create fm->speaker+earphone patch to avoid noise");
                            releaseAudioPatch();
                            if (isRenderForSpeaker()) {
                                startRender();
                            } else {
                                createAudioPatchBySpeakerAndEarphone();
                            }
                        }
                    } catch (NullPointerException e) {
                        Log.e(TAG, "mAudioPatch released.");
                    }
                } else if (isPatchMixerToEarphone(patches)) {
                    stopRender();
                    if (isOutputDeviceChanged(patches)) {
                        Log.d(TAG, "DEBUG outputDeviceChanged: re-create audio patch");
                        releaseAudioPatch();
                        createAudioPatchByEarphone();
                        mIsSpeakerUsed = false;
                        notifySpeakerModeChange();
                    }
                    else if (mRegainAudioFocusAndSetForceUseSpeaker){
                        releaseAudioPatch();
                        createAudioPatchBySpeaker();
                        mIsSpeakerUsed = true;
                        mRegainAudioFocusAndSetForceUseSpeaker = false;
                        notifySpeakerModeChange();
                    }
                } else if (isPatchMixerToSpeaker(patches)) {
                    stopRender();
                    if (isOutputDeviceChanged(patches)) {
                        Log.d(TAG, "DEBUG outputDeviceChanged: re-create audio patch");
                        releaseAudioPatch();
                        if (isRenderForSpeaker()) {
                            mIsSpeakerUsed = true;
                            notifySpeakerModeChange();
                            Log.d(TAG, "start render for speaker 1");
                            startRender();
                        } else {
                            createAudioPatchBySpeaker();
                            mIsSpeakerUsed = true;
                            notifySpeakerModeChange();
                        }
                    }
                } else {
                    Log.d(TAG, "set mIsOutputDeviceChanged true as none audiopatch is present");
                    mIsOutputDeviceChanged = true;
                }
            } else if (mIsRender) {
                Log.d(TAG, "onAudioPatchListUpdate2");
                ArrayList<AudioPatch> patches = new ArrayList<AudioPatch>();
                mAudioManager.listAudioPatches(patches);

                if (isPatchMixerToBt(patches)) {
                    Log.d(TAG, "already render in progress, no need to start again");
                    return;
                }
                if (isPatchMixerToEarphone(patches)) {
                    stopRender();
                    stopAudioTrack();
                    createAudioPatchByEarphone();
                    mIsSpeakerUsed = false;
                    notifySpeakerModeChange();
                } else if (isPatchMixerToSpeaker(patches)) {
                    if (!(isRenderForSpeaker())) {
                        stopRender();
                        stopAudioTrack();
                        createAudioPatchBySpeaker();
                        mIsSpeakerUsed = true;
                        notifySpeakerModeChange();
                    }
                } else if (isPatchMixerToDeviceRemoved(patches)) {
                    Log.w(TAG, "onAudioPatchListUpdate: native removed patches, restart render");
                    stopRender();
                    startRender();
                }
            }
            Log.i(TAG, "onAudioPatchListUpdate: exit");
            }
        }

        /**
         * Callback method called when the mediaserver dies
         */
        @Override
        public void onServiceDied() {
            Log.d(TAG, "onServiceDied");
            enableFmAudio(false);
        }
    }

    private synchronized void releaseAudioPatch() {
            Log.d(TAG, "releaseAudioPatch, mAudioPatch = " + mAudioPatch);
        if (mAudioPatch != null) {
            mAudioManager.releaseAudioPatch(mAudioPatch);
            mAudioPatch = null;
        }
        mAudioSource = null;
        mAudioSink = null;
    }

    private void registerFmBroadcastReceiver() {
        if (Log.isLoggable(TAG, Log.VERBOSE)) {
            Log.v(TAG, "registerFmBroadcastReceiver");
        }
        IntentFilter filter = new IntentFilter();
        filter.addAction(SOUND_POWER_DOWN_MSG);
        filter.addAction(Intent.ACTION_SHUTDOWN);
        filter.addAction(Intent.ACTION_SCREEN_ON);
        filter.addAction(Intent.ACTION_SCREEN_OFF);
        filter.addAction(Intent.ACTION_HEADSET_PLUG);
        mBroadcastReceiver = new FmServiceBroadcastReceiver();
        registerReceiver(mBroadcastReceiver, filter);
    }

    private void unregisterFmBroadcastReceiver() {
            Log.d(TAG, "unregisterFmBroadcastReceiver");
        if (null != mBroadcastReceiver) {
            unregisterReceiver(mBroadcastReceiver);
            mBroadcastReceiver = null;
        }
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "onDestroy");
        if (mIsParametersSet) {
            mIsParametersSet = false;
            Log.w(TAG, "AudioFmPreStop=0");
            mAudioManager.setParameters("AudioFmPreStop=0");
        }
        setMute(true);
        // stop rds first, avoid blocking other native method
        if (mIsRdsSupported) {
            stopRdsThread();
        }
        unregisterFmBroadcastReceiver();
        unregisterSdcardListener();
        abandonAudioFocus();
        exitFm();
        if (null != mFmRecorder) {
            mFmRecorder = null;
        }
        exitRenderThread();
        releaseAudioPatch();
        unregisterAudioPortUpdateListener();
        super.onDestroy();
    }

    /**
     * Exit FMRadio application
     */
    private void exitFm() {
            Log.d(TAG, "exitFm");
        mIsAudioFocusHeld = false;
        // Stop FM recorder if it is working
        if (null != mFmRecorder) {
            synchronized (mStopRecordingLock) {
                int fmState = mFmRecorder.getState();
                if (FmRecorder.STATE_RECORDING == fmState) {
                    mFmRecorder.stopRecording();
                    saveRecording(getRecordingName());
                } else if (getRecordingName() != null) {
                    saveRecording(getRecordingName());
                }
            }
        }

        // When exit, we set the audio path back to earphone.
        if (mIsNativeScanning || mIsNativeSeeking) {
            stopScan();
        }

        mFmServiceHandler.removeCallbacksAndMessages(null);
        mFmServiceHandler.removeMessages(FmListener.MSGID_FM_EXIT);
        mFmServiceHandler.sendEmptyMessage(FmListener.MSGID_FM_EXIT);
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        Log.d(TAG, "onConfigurationChanged");
        // Change the notification string.
        if (mPowerStatus == POWER_UP) {
            if (getRecorderState() == FmRecorder.STATE_RECORDING) {
                Log.d(TAG, "onConfigurationChanged, recording state");
                Bundle bundle = new Bundle(2);
                bundle.putInt(FmListener.CALLBACK_FLAG, FmListener.UPDATE_NOTIFICATION);
                notifyActivityStateChanged(bundle);
                // set notification null so its recreated again after recording notification
                if (mNotificationBuilder != null) {
                        mNotificationBuilder = null;
                }
            } else {
                //remove the notification & then recreate the notifiction with updated language
                synchronized (mNotificationLock) {
                    removeNotification();
                    if (mNotificationBuilder != null) {
                        mNotificationBuilder = null;
                    }
                    mFmServiceHandler.sendEmptyMessage(FmListener.UPDATE_NOTIFICATION);
                }
            }
        }
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        int ret = super.onStartCommand(intent, flags, startId);

        if (intent != null) {
            String action = intent.getAction();
            if (FM_SEEK_PREVIOUS.equals(action)) {
                seekStationAsync(FmUtils.computeFrequency(mCurrentStation), false);
            } else if (FM_SEEK_NEXT.equals(action)) {
                seekStationAsync(FmUtils.computeFrequency(mCurrentStation), true);
            } else if (FM_TURN_OFF.equals(action)) {
                powerDownAsync();
            }
        }
        return START_NOT_STICKY;
    }

    /**
     * Start RDS thread to update RDS information
     */
    private void startRdsThread() {
            Log.d(TAG, "startRdsThread");
        mIsRdsThreadExit = false;
        if (null != mRdsThread) {
            return;
        }
        mRdsThread = new Thread() {
            public void run() {
                while (true) {
                    if (mIsRdsThreadExit) {
                        break;
                    }

                    int iRdsEvents = FmNative.readRds();
                    if (iRdsEvents != 0) {
                        Log.d(TAG, "startRdsThread, is rds events: " + iRdsEvents);
                    }

                    if (RDS_EVENT_PROGRAMNAME == (RDS_EVENT_PROGRAMNAME & iRdsEvents)) {
                        byte[] bytePS = FmNative.getPs();
                        if (null != bytePS) {
                            String ps = new String(bytePS).trim();
                            if (!mPsString.equals(ps)) {
                                // update notification on main thread
                                Handler mainHandler = new Handler(getMainLooper());
                                mainHandler.post(new Runnable() {
                                    @Override
                                    public void run() {
                                updatePlayingNotification();
                            }
                                });
                            }
                            ContentValues values = null;
                            if (FmStation.isStationExist(mContext, mCurrentStation)) {
                                values = new ContentValues(1);
                                values.put(Station.PROGRAM_SERVICE, ps);
                                FmStation.updateStationToDb(mContext, mCurrentStation, values);
                            } else {
                                values = new ContentValues(2);
                                values.put(Station.FREQUENCY, mCurrentStation);
                                values.put(Station.PROGRAM_SERVICE, ps);
                                FmStation.insertStationToDb(mContext, values);
                            }
                            setPs(ps);
                        }
                    }

                    if (RDS_EVENT_LAST_RADIOTEXT == (RDS_EVENT_LAST_RADIOTEXT & iRdsEvents)) {
                        byte[] byteLRText = FmNative.getLrText();
                        if (null != byteLRText) {
                            String rds = new String(byteLRText).trim();
                            if (!mRtTextString.equals(rds)) {
                                updatePlayingNotification();
                            }
                            setLRText(rds);
                            ContentValues values = null;
                            if (FmStation.isStationExist(mContext, mCurrentStation)) {
                                values = new ContentValues(1);
                                values.put(Station.RADIO_TEXT, rds);
                                FmStation.updateStationToDb(mContext, mCurrentStation, values);
                            } else {
                                values = new ContentValues(2);
                                values.put(Station.FREQUENCY, mCurrentStation);
                                values.put(Station.RADIO_TEXT, rds);
                                FmStation.insertStationToDb(mContext, values);
                            }
                        }
                    }

                    if (RDS_EVENT_AF == (RDS_EVENT_AF & iRdsEvents)) {
                        /*
                         * add for rds AF
                         */
                        if (mIsScanning || mIsSeeking) {
                            Log.d(TAG, "startRdsThread, seek or scan going, no need to tune here");
                        } else if (mPowerStatus == POWER_DOWN) {
                            Log.d(TAG, "startRdsThread, fm is power down, do nothing.");
                        } else {
                            int iFreq = FmNative.activeAf();
                            if (FmUtils.isValidStation(iFreq)) {
                                // if the new frequency is not equal to current
                                // frequency.
                                if (mCurrentStation != iFreq) {
                                    if (!mIsScanning && !mIsSeeking) {
                                        Log.d(TAG, "startRdsThread, seek or scan not going,"
                                                + "need to tune here");
                                        tuneStationAsync(FmUtils.computeFrequency(iFreq));
                                    }
                                }
                            }
                        }
                    }
                    // Do not handle other events.
                    // Sleep 500ms to reduce inquiry frequency
                    try {
                        final int hundredMillisecond = 500;
                        Thread.sleep(hundredMillisecond);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
            }
        };
        mRdsThread.start();
    }

    /**
     * Stop RDS thread to stop listen station RDS change
     */
    private void stopRdsThread() {
            Log.d(TAG, "stopRdsThread");
        if (null != mRdsThread) {
            // Must call closedev after stopRDSThread.
            mIsRdsThreadExit = true;
            mRdsThread = null;
        }
    }

    /**
     * Set PS information
     *
     * @param ps The ps information
     */
    private void setPs(String ps) {
        if (0 != mPsString.compareTo(ps)) {
            mPsString = ps;
            Bundle bundle = new Bundle(3);
            bundle.putInt(FmListener.CALLBACK_FLAG, FmListener.LISTEN_PS_CHANGED);
            bundle.putString(FmListener.KEY_PS_INFO, mPsString);
            notifyActivityStateChanged(bundle);
        } // else New PS is the same as current
    }

    /**
     * Set RT information
     *
     * @param lrtText The RT information
     */
    private void setLRText(String lrtText) {
        if (0 != mRtTextString.compareTo(lrtText)) {
            mRtTextString = lrtText;
            Bundle bundle = new Bundle(3);
            bundle.putInt(FmListener.CALLBACK_FLAG, FmListener.LISTEN_RT_CHANGED);
            bundle.putString(FmListener.KEY_RT_INFO, mRtTextString);
            notifyActivityStateChanged(bundle);
        } // else New RT is the same as current
    }

    /**
     * Open or close FM Radio audio
     *
     * @param enable true, open FM audio; false, close FM audio;
     */
    private void enableFmAudio(boolean enable) {
            Log.d(TAG, "enableFmAudio: " + enable);
        if (enable) {
            if ((mPowerStatus != POWER_UP) || !mIsAudioFocusHeld) {
                Log.d(TAG, "enableFmAudio, current not available return.mIsAudioFocusHeld:"
                    + mIsAudioFocusHeld);
                Log.d(TAG, "unable to start fm, set status as false");
                mPowerStatus = POWER_DOWN;
                releaseAudioPatch();
                stopRender();
                stopAudioTrack();
                return;
            }

            startAudioTrack();
            createAudioPatch();
            if (FmUtils.getIsSpeakerModeOnFocusLost(mContext)) {
                    setForceUse(true);
                    FmUtils.setIsSpeakerModeOnFocusLost(mContext, false);
                    notifySpeakerModeChange();
                }
        } else {
            releaseAudioPatch();
            stopRender();
            stopAudioTrack();
        }
    }


    // Make sure patches count will not be 0
    private boolean isPatchMixerToBt(ArrayList<AudioPatch> patches) {
        for (AudioPatch patch : patches) {
            AudioPortConfig[] sources = patch.sources();
            AudioPortConfig[] sinks = patch.sinks();
            if (sinks.length > 1) {
                continue;
            }
            AudioPortConfig sourceConfig = sources[0];
            AudioPortConfig sinkConfig = sinks[0];
            AudioPort sourcePort = sourceConfig.port();
            AudioPort sinkPort = sinkConfig.port();
            if (sourcePort instanceof AudioMixPort && sinkPort instanceof AudioDevicePort) {
                int type = ((AudioDevicePort) sinkPort).type();
                if (isBtDevice(type)) {
                        Log.d(TAG, "isPatchMixerToBt: true");
                        return true;
                }
            }
        }

        Log.d(TAG, "isPatchMixerToBt: false");
        return false;
    }


    // Make sure patches count will not be 0
    private boolean isPatchMixerToEarphone(ArrayList<AudioPatch> patches) {
        int deviceCount = 0;
        int deviceEarphoneCount = 0;
        for (AudioPatch patch : patches) {
            AudioPortConfig[] sources = patch.sources();
            AudioPortConfig[] sinks = patch.sinks();
            if (sinks.length > 1) {
                continue;
            }
            AudioPortConfig sourceConfig = sources[0];
            AudioPortConfig sinkConfig = sinks[0];
            AudioPort sourcePort = sourceConfig.port();
            AudioPort sinkPort = sinkConfig.port();
            if (sourcePort instanceof AudioMixPort && sinkPort instanceof AudioDevicePort) {
                if (!isMusicOutputId(sourcePort.id())) {
                    continue;
                }
                deviceCount++;
                int type = ((AudioDevicePort) sinkPort).type();
                if (type == AudioSystem.DEVICE_OUT_WIRED_HEADSET ||
                        type == AudioSystem.DEVICE_OUT_WIRED_HEADPHONE) {
                    deviceEarphoneCount++;
                }
            }
        }
        if (deviceEarphoneCount >= 1 && deviceCount == deviceEarphoneCount) {
            Log.d(TAG, "isPatchMixerToEarphone: true");
            return true;
        }
        Log.d(TAG, "isPatchMixerToEarphone: false");
        return false;
    }

    // Make sure patches count will not be 0
    private boolean isPatchMixerToSpeaker(ArrayList<AudioPatch> patches) {
        int deviceCount = 0;
        int deviceEarphoneCount = 0;
        for (AudioPatch patch : patches) {
            AudioPortConfig[] sources = patch.sources();
            AudioPortConfig[] sinks = patch.sinks();
            if (sinks.length > 1) {
                continue;
            }
            AudioPortConfig sourceConfig = sources[0];
            AudioPortConfig sinkConfig = sinks[0];
            AudioPort sourcePort = sourceConfig.port();
            AudioPort sinkPort = sinkConfig.port();
            if (sourcePort instanceof AudioMixPort && sinkPort instanceof AudioDevicePort) {
                if (!isMusicOutputId(sourcePort.id())) {
                    continue;
                }
                deviceCount++;
                int type = ((AudioDevicePort) sinkPort).type();
                if (type == AudioSystem.DEVICE_OUT_SPEAKER) {
                    deviceEarphoneCount++;
                }
            }
        }
        if (deviceEarphoneCount >= 1 && deviceCount == deviceEarphoneCount) {
            Log.d(TAG, "isPatchMixerToSpeaker: true");
            return true;
        }
        Log.d(TAG, "isPatchMixerToSpeaker: false");
        return false;
    }

    private boolean isPatchContainSpeakerAndEarphone(ArrayList<AudioPatch> patches) {
        boolean hasSpeakerSink = false;
        boolean hasEarphoneSink = false;
        for (AudioPatch patch : patches) {
            AudioPortConfig[] sources = patch.sources();
            AudioPortConfig[] sinks = patch.sinks();
            // only when source port is mix with two sink need do next check,
            // return true only when sink is speaker and earphone
            if (sinks.length != 2 || !(sources[0].port() instanceof AudioMixPort)) {
                continue;
            }
            if (!isMusicOutputId(sources[0].port().id())) {
                continue;
            }
            for (AudioPortConfig sink : sinks) {
                AudioPort sinkPort = sink.port();
                if (sinkPort instanceof AudioDevicePort) {
                    int type = ((AudioDevicePort) sinkPort).type();
                    if (type == AudioSystem.DEVICE_OUT_SPEAKER) {
                        hasSpeakerSink = true;
                    } else if (type == AudioSystem.DEVICE_OUT_WIRED_HEADSET ||
                            type == AudioSystem.DEVICE_OUT_WIRED_HEADPHONE) {
                        hasEarphoneSink = true;
                    }
                }
            }
        }
        Log.d(TAG, "isPatchContainSpeakerAndEarphone: " + (hasSpeakerSink && hasEarphoneSink));
        return hasSpeakerSink && hasEarphoneSink;
    }

    // Check whether the patch (mixer -> device) is removed by native.
    // If no patch (mixer -> device), return true.
    private boolean isPatchMixerToDeviceRemoved(ArrayList<AudioPatch> patches) {
        boolean noMixerToDevice = true;
        for (AudioPatch patch : patches) {
            AudioPortConfig[] sources = patch.sources();
            AudioPortConfig[] sinks = patch.sinks();
            AudioPortConfig sourceConfig = sources[0];
            AudioPortConfig sinkConfig = sinks[0];
            AudioPort sourcePort = sourceConfig.port();
            AudioPort sinkPort = sinkConfig.port();

            if (!isMusicOutputId(sourcePort.id())) {
                continue;
            }
            if (sourcePort instanceof AudioMixPort && sinkPort instanceof AudioDevicePort) {
                noMixerToDevice = false;
                break;
            }
        }
        Log.d(TAG, "isPatchMixerToDeviceRemoved: " + noMixerToDevice);
        return noMixerToDevice;
    }

    private boolean isOutputDeviceChanged(ArrayList<AudioPatch> patches) {
        boolean ret = true;
        int minSourcePortId;
        AudioPortConfig[] origSources = null;
        AudioPortConfig[] origSinks = null;
        synchronized (this) {
            // need synchronized to avoid NPE of mAudioPatch, which
            // is reassigned to null in releaseAudioPatch().
            if (mAudioPatch == null) {
                Log.d(TAG, "isOutputDeviceChanged, mAudioPatch is null, return");
                return false;
            }
            origSources = mAudioPatch.sources();
            origSinks = mAudioPatch.sinks();
        }
        if (mIsOutputDeviceChanged) {
            Log.d(TAG, "patch mixer again set to some output device");
            Log.d(TAG, "isOutputDeviceChanged: true");
            mIsOutputDeviceChanged = false;
            Log.d(TAG, "set mIsOutputDeviceChanged to false");
            return true;
        }
        AudioPort origSrcPort = origSources[0].port();
        AudioPort origSinkPort = origSinks[0].port();
        AudioPort sourcePort = null;
        AudioPatch minAudioPatch = null;
        Log.d(TAG, "DEBUG " + origSinkPort);

        minSourcePortId = (patches.get(0).sources()[0]).port().id();
        for (AudioPatch aPatch : patches) {
            AudioPortConfig[] sources = aPatch.sources();
            AudioPortConfig sourceConfig = sources[0];
            sourcePort = sourceConfig.port();

            // find patch with min source port id
            if (sourcePort.id() <= minSourcePortId) {
                minSourcePortId = sourcePort.id();
                minAudioPatch = aPatch;
            }
        }
        if (minAudioPatch == null) {
            Log.d(TAG, "DEBUG: minAudioPatch==null");
            return true;
        }
        AudioPortConfig[] sources = minAudioPatch.sources();
        AudioPortConfig sourceConfig = sources[0];
        AudioPortConfig[] sinks = minAudioPatch.sinks();
        AudioPort sinkPort = null;
        sourcePort = sourceConfig.port();

        // compare minAudioPatch with mAudioPatch
        if (sourcePort instanceof AudioMixPort) {
            int sinkOR = 0;
            int origSinkOR = 0;
            int sinksLength = sinks.length;
            if (sinksLength == origSinks.length) {
                for (int i = 0; i < sinksLength; i++) {
                    sinkPort = sinks[i].port();
                    origSinkPort = origSinks[i].port();
                    if (sinkPort instanceof AudioDevicePort &&
                        origSinkPort instanceof AudioDevicePort) {
                        sinkOR |= ((AudioDevicePort) sinkPort).type();
                        origSinkOR |= ((AudioDevicePort) origSinkPort).type();
                        } else {
                            Log.d(TAG, "DEBUG1: sink_id: " + ((AudioDevicePort) sinkPort).type()
                            + " orig_sink_id: " + ((AudioDevicePort) origSinkPort).type());
                            return true;
                    }
                }
                    if (sinkOR == origSinkOR) {
                    // patches are equal
                    Log.d(TAG, "isOutputDeviceChanged: false");
                    return false;
                }
            }
            else {
                // sinkPort is not an instance of AudioDevicePort
                Log.d(TAG, "DEBUG2: sink lengths not equal");
                return true;
            }
        }

        Log.d(TAG, "isOutputDeviceChanged: " + ret);
        return ret;
    }

    /**
     * Show notification
     */
    private synchronized void showPlayingNotification() {
        synchronized (mNotificationLock) {
            Log.d(TAG, "showPlayingNotification");
            if (isActivityForeground() || mIsScanning
                    || (getRecorderState() == FmRecorder.STATE_RECORDING)) {
                Log.w(TAG, "showPlayingNotification, do not show main notification.");
                return;
            }
            String stationName = "";
            String radioText = "";
            ContentResolver resolver = mContext.getContentResolver();
            Cursor cursor = null;
            try {
                cursor = resolver.query(
                        Station.CONTENT_URI,
                        FmStation.COLUMNS,
                        Station.FREQUENCY + "=?",
                        new String[] { String.valueOf(mCurrentStation) },
                        null);
                if (cursor != null && cursor.moveToFirst()) {
                    // If the station name is not exist, show program service(PS) instead
                    stationName = cursor.getString(cursor.getColumnIndex(Station.STATION_NAME));
                    if (TextUtils.isEmpty(stationName)) {
                        stationName = cursor.getString(
                            cursor.getColumnIndex(Station.PROGRAM_SERVICE));
                    }
                    radioText = cursor.getString(cursor.getColumnIndex(Station.RADIO_TEXT));

                } else {
                    Log.d(TAG, "showPlayingNotification, cursor is null");
                }
            } finally {
                if (cursor != null) {
                    cursor.close();
                }
            }

            Intent aIntent = new Intent(Intent.ACTION_MAIN);
            aIntent.addCategory(Intent.CATEGORY_LAUNCHER);
            aIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            aIntent.setClassName(getPackageName(), mTargetClassName);
            PendingIntent pAIntent = PendingIntent.getActivity(mContext, 0, aIntent, 0);

            if (null == mNotificationBuilder) {
                mNotificationBuilder = new Notification.Builder(mContext,
                                                    NOTIFICATION_CHANNEL_FMRADIO);
                mNotificationBuilder.setSmallIcon(R.drawable.ic_launcher);
                mNotificationBuilder.setShowWhen(false);
                mNotificationBuilder.setAutoCancel(true);

                Intent intent = new Intent(FM_SEEK_PREVIOUS);
                intent.setClass(mContext, FmService.class);
                PendingIntent pIntent = PendingIntent.getService(mContext, 0, intent, 0);
                mNotificationBuilder.addAction(R.drawable.btn_fm_prevstation,
                                        getString(R.string.previous_channel), pIntent);
                intent = new Intent(FM_TURN_OFF);
                intent.setClass(mContext, FmService.class);
                pIntent = PendingIntent.getService(mContext, 0, intent, 0);
                mNotificationBuilder.addAction(R.drawable.btn_fm_rec_stop_enabled,
                                        getString(R.string.stop_fm), pIntent);
                intent = new Intent(FM_SEEK_NEXT);
                intent.setClass(mContext, FmService.class);
                pIntent = PendingIntent.getService(mContext, 0, intent, 0);
                mNotificationBuilder.addAction(R.drawable.btn_fm_nextstation,
                                        getString(R.string.next_channel), pIntent);
            }
            mNotificationBuilder.setContentIntent(pAIntent);
            Bitmap largeIcon = FmUtils.createNotificationLargeIcon(mContext,
                    FmUtils.formatStation(mCurrentStation));
            mNotificationBuilder.setLargeIcon(largeIcon);
            // Show FM Radio if empty
            if (TextUtils.isEmpty(stationName)) {
                stationName = getString(R.string.app_name);
            }
            mNotificationBuilder.setContentTitle(stationName);
            // If radio text is "" or null, we also need to update notification.
            if (TextUtils.isEmpty(radioText)) {
                // If radioText is null, notification has no content text view.
                radioText = null;
            }
            mNotificationBuilder.setContentText(radioText);
            Log.d(TAG, "showPlayingNotification PS:" + stationName + ", RT:" + radioText);
            Notification n = mNotificationBuilder.build();
            n.flags &= ~Notification.FLAG_NO_CLEAR;
            if (mPowerStatus == POWER_UP) {
                startForeground(NOTIFICATION_ID, n);
            } else {
                Log.d(TAG, "fm turned off, do not show notification");
            }
        }
    }

    public void updateRecordingNotification(long recordTime) {
        Log.d(TAG, "service updateRecordingNotification");
        if (mNotificationBuilder == null) {
            Intent intent = new Intent(FM_STOP_RECORDING);
            intent.setClass(mContext, FmRecordActivity.class);
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            PendingIntent pendingIntent = PendingIntent.getActivity(mContext, 0, intent,
                    PendingIntent.FLAG_UPDATE_CURRENT);

            Bitmap largeIcon = FmUtils.createNotificationLargeIcon(mContext,
                    FmUtils.formatStation(mCurrentStation));
            mNotificationBuilder = new Builder(this, NOTIFICATION_CHANNEL_FMRADIO)
                    .setContentText(getText(R.string.record_notification_message))
                    .setShowWhen(false)
                    .setAutoCancel(true)
                    .setSmallIcon(R.drawable.ic_launcher)
                    .setLargeIcon(largeIcon)
                    .addAction(R.drawable.btn_fm_rec_stop_enabled, getText(R.string.stop_record),
                            pendingIntent);

            Intent cIntent = new Intent(FM_ENTER_RECORD_SCREEN);
            cIntent.setClass(mContext, FmRecordActivity.class);
            cIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            PendingIntent contentPendingIntent = PendingIntent.getActivity(mContext, 0, cIntent,
                    PendingIntent.FLAG_UPDATE_CURRENT);
            mNotificationBuilder.setContentIntent(contentPendingIntent);
            mNotificationBuilder.setContentText(getText(R.string.record_notification_message));
        }
        String time = FmUtils.timeFormat(recordTime);
        mNotificationBuilder.setContentTitle(time);
        showRecordingNotification(mNotificationBuilder.build());
    }

    /**
     * Show notification
     */
    public void showRecordingNotification(Notification notification) {
        startForeground(NOTIFICATION_ID, notification);
    }

    /**
     * Remove notification
     */
    public void removeNotification() {
        synchronized (mNotificationLock) {
            Log.d(TAG, "removeNotification()");
            stopForeground(true);
            if (mNotificationBuilder != null) {
                mNotificationBuilder = null;
            }
        }
    }

    /**
     * Update notification
     */
    public void updatePlayingNotification() {
        Log.d(TAG, "updatePlayingNotification()");
        if (mPowerStatus == POWER_UP) {
            showPlayingNotification();
        }
    }

    /**
     * Register sdcard listener for record
     */
    private void registerSdcardReceiver() {
        if (mSdcardListener == null) {
            mSdcardListener = new SdcardListener();
        }
        IntentFilter filter = new IntentFilter();
        filter.addDataScheme("file");
        filter.addAction(Intent.ACTION_MEDIA_MOUNTED);
        filter.addAction(Intent.ACTION_MEDIA_UNMOUNTED);
        filter.addAction(Intent.ACTION_MEDIA_EJECT);
        registerReceiver(mSdcardListener, filter);
    }

    private void unregisterSdcardListener() {
        if (null != mSdcardListener) {
            unregisterReceiver(mSdcardListener);
        }
    }

    private void updateSdcardStateMap(Intent intent) {
        String action = intent.getAction();
        String sdcardPath = null;
        Uri mountPointUri = intent.getData();
        if (mountPointUri != null) {
            sdcardPath = mountPointUri.getPath();
            if (sdcardPath != null) {
                if (Intent.ACTION_MEDIA_EJECT.equals(action)) {
                    mSdcardStateMap.put(sdcardPath, false);
                } else if (Intent.ACTION_MEDIA_UNMOUNTED.equals(action)) {
                    mSdcardStateMap.put(sdcardPath, false);
                } else if (Intent.ACTION_MEDIA_MOUNTED.equals(action)) {
                    mSdcardStateMap.put(sdcardPath, true);
                }
            }
        }
    }

    /**
     * Notify FM recorder state
     *
     * @param state The current FM recorder state
     */
    @Override
    public void onRecorderStateChanged(int state) {
        Log.d(TAG, "onRecorderStateChanged, state = " + state);
        mRecordState = state;
        Bundle bundle = new Bundle(2);
        bundle.putInt(FmListener.CALLBACK_FLAG, FmListener.LISTEN_RECORDSTATE_CHANGED);
        bundle.putInt(FmListener.KEY_RECORDING_STATE, state);
        notifyActivityStateChanged(bundle);
    }

    /**
     * Notify FM recorder error message
     *
     * @param error The recorder error type
     */
    @Override
    public void onRecorderError(int error) {
        Log.d(TAG, "onRecorderError, error = " + error);
        // if media server die, will not enable FM audio, and convert to
        // ERROR_PLAYER_INATERNAL, call back to activity showing toast.
        mRecorderErrorType = error;

        Bundle bundle = new Bundle(2);
        bundle.putInt(FmListener.CALLBACK_FLAG, FmListener.LISTEN_RECORDERROR);
        bundle.putInt(FmListener.KEY_RECORDING_ERROR_TYPE, mRecorderErrorType);
        notifyActivityStateChanged(bundle);
    }

    /**
     * Check and go next(play or show tips) after recorder file play
     * back finish.
     * Two cases:
     * 1. With headset  -> play FM
     * 2. Without headset -> show plug in earphone tips
     */
    private void checkState() {
        if (isHeadSetIn()) {
            // with headset
            if (mPowerStatus == POWER_UP) {
                resumeFmAudio();
                setMute(false);
            } else {
                powerUpAsync(FmUtils.computeFrequency(mCurrentStation));
            }
        } else {
            // without headset need show plug in earphone tips
            switchAntennaAsync(mValueHeadSetPlug);
        }
    }

    /**
     * Check the headset is plug in or plug out
     *
     * @return true for plug in; false for plug out
     */
    private boolean isHeadSetIn() {
        return (0 == mValueHeadSetPlug);
    }

    private void focusChanged(int focusState) {
        mIsAudioFocusHeld = false;
        if (mIsNativeScanning || mIsNativeSeeking) {
            // make stop scan from activity call to service.
            // notifyActivityStateChanged(FMRadioListener.LISTEN_SCAN_CANCELED);
            stopScan();
        }

        // using handler thread to update audio focus state
        updateAudioFocusAync(focusState);
    }

    /**
     * Request audio focus
     *
     * @return true, success; false, fail;
     */
    public boolean requestAudioFocus() {
        Log.d(TAG, "requestAudioFocus");
        if (mIsAudioFocusHeld) {
            return true;
        }

        int audioFocus = mAudioManager.requestAudioFocus(mAudioFocusChangeListener,
                AudioManager.STREAM_MUSIC, AudioManager.AUDIOFOCUS_GAIN);
        mIsAudioFocusHeld = (AudioManager.AUDIOFOCUS_REQUEST_GRANTED == audioFocus);
        Log.d(TAG, "requestAudioFocus, mIsAudioFocusHeld" + mIsAudioFocusHeld);
        return mIsAudioFocusHeld;
    }

    /**
     * Abandon audio focus
     */
    public void abandonAudioFocus() {
        mAudioManager.abandonAudioFocus(mAudioFocusChangeListener);
        mIsAudioFocusHeld = false;
    }

    /**
     * Use to interact with other voice related app
     */
    private final OnAudioFocusChangeListener mAudioFocusChangeListener =
            new OnAudioFocusChangeListener() {
                /**
                 * Handle audio focus change ensure message FIFO
                 *
                 * @param focusChange audio focus change state
                 */
                @Override
                public void onAudioFocusChange(int focusChange) {
                    Log.d(TAG, "onAudioFocusChange " + focusChange);
                    switch (focusChange) {
                        case AudioManager.AUDIOFOCUS_LOSS:
                            synchronized (this) {
                                if (getForceUse() == AudioSystem.FORCE_SPEAKER && mIsSpeakerUsed) {
                                    Log.w(TAG, "check for mIsParametersSet");
                                    if (mIsParametersSet != true) {
                                    mIsParametersSet = true;
                                    Log.w(TAG, "AudioFmPreStop=1");
                                    mAudioManager.setParameters("AudioFmPreStop=1");
                                    }
                                }
                                setMute(true);
                                focusChanged(AudioManager.AUDIOFOCUS_LOSS);
                            }
                            break;

                        case AudioManager.AUDIOFOCUS_LOSS_TRANSIENT:
                            synchronized (this) {
                                    if (getForceUse() == AudioSystem.FORCE_SPEAKER &&
                                        mIsSpeakerUsed) {
                                        Log.w(TAG, "check for mIsParametersSet");
                                        if (mIsParametersSet != true) {
                                        mIsParametersSet = true;
                                        Log.w(TAG, "AudioFmPreStop=1");
                                        mAudioManager.setParameters("AudioFmPreStop=1");
                                        }
                                }
                                setMute(true);
                                focusChanged(AudioManager.AUDIOFOCUS_LOSS_TRANSIENT);
                            }
                            break;

                        case AudioManager.AUDIOFOCUS_GAIN:
                            synchronized (this) {
                                updateAudioFocusAync(AudioManager.AUDIOFOCUS_GAIN);
                            }
                            break;

                        case AudioManager.AUDIOFOCUS_LOSS_TRANSIENT_CAN_DUCK:
                            synchronized (this) {
                                if (getForceUse() == AudioSystem.FORCE_SPEAKER && mIsSpeakerUsed) {
                                    mIsParametersSet = true;
                                    Log.w(TAG, "AudioFmPreStop=1");
                                    mAudioManager.setParameters("AudioFmPreStop=1");
                                }
                                setMute(true);
                                focusChanged(AudioManager.AUDIOFOCUS_LOSS_TRANSIENT_CAN_DUCK);
                            }
                            break;

                        default:
                            break;
                    }
                }
            };

    /**
     * Audio focus changed, will send message to handler thread. synchronized to
     * ensure one message can go in this method.
     *
     * @param focusState AudioManager state
     */
    private synchronized void updateAudioFocusAync(int focusState) {
            Log.d(TAG, "updateAudioFocusAync, focusState = " + focusState);
        final int bundleSize = 1;
        Bundle bundle = new Bundle(bundleSize);
        bundle.putInt(FmListener.KEY_AUDIOFOCUS_CHANGED, focusState);
        Message msg = mFmServiceHandler.obtainMessage(FmListener.MSGID_AUDIOFOCUS_CHANGED);
        msg.setData(bundle);
        mFmServiceHandler.sendMessage(msg);
    }

    /**
     * Audio focus changed, update FM focus state.
     *
     * @param focusState AudioManager state
     */
    private void updateAudioFocus(int focusState) {
            Log.d(TAG, "updateAudioFocus: " + focusState);
        switch (focusState) {
            case AudioManager.AUDIOFOCUS_LOSS:
                mPausedByTransientLossOfFocus = false;
                // play back audio will output with music audio
                // May be affect other recorder app, but the flow can not be
                // execute earlier,
                // It should ensure execute after start/stop record.
                if (mFmRecorder != null) {
                    int fmState = mFmRecorder.getState();
                    // only handle recorder state, not handle playback state
                    if (fmState == FmRecorder.STATE_RECORDING) {
                        mFmServiceHandler.removeMessages(
                                FmListener.MSGID_STARTRECORDING_FINISHED);
                        mFmServiceHandler.removeMessages(
                                FmListener.MSGID_STOPRECORDING_FINISHED);
                        stopRecording();
                    }
                }
                forceToHeadsetMode();
                if (mIsParametersSet) {
                    mIsParametersSet = false;
                    Log.w(TAG, "AudioFmPreStop=0");
                    mAudioManager.setParameters("AudioFmPreStop=0");
                }
                handlePowerDown();
                break;

            case AudioManager.AUDIOFOCUS_LOSS_TRANSIENT:
            case AudioManager.AUDIOFOCUS_LOSS_TRANSIENT_CAN_DUCK:
                if (mPowerStatus == POWER_UP) {
                    mPausedByTransientLossOfFocus = true;
                }
                // play back audio will output with music audio
                // May be affect other recorder app, but the flow can not be
                // execute earlier,
                // It should ensure execute after start/stop record.
                if (mFmRecorder != null) {
                    int fmState = mFmRecorder.getState();
                    if (fmState == FmRecorder.STATE_RECORDING) {
                        mFmServiceHandler.removeMessages(
                                FmListener.MSGID_STARTRECORDING_FINISHED);
                        mFmServiceHandler.removeMessages(
                                FmListener.MSGID_STOPRECORDING_FINISHED);
                        stopRecording();
                    }
                }
                forceToHeadsetMode();
                if (mIsParametersSet) {
                    mIsParametersSet = false;
                    Log.w(TAG, "AudioFmPreStop=0");
                    mAudioManager.setParameters("AudioFmPreStop=0");
                }
                handlePowerDown();
                break;

            case AudioManager.AUDIOFOCUS_GAIN:
                /* if (FmUtils.getIsSpeakerModeOnFocusLost(mContext)) {
                    setForceUse(true);
                    FmUtils.setIsSpeakerModeOnFocusLost(mContext, false);
                } */
                if ((mPowerStatus != POWER_UP) && mPausedByTransientLossOfFocus) {
                    final int bundleSize = 1;
                    mFmServiceHandler.removeMessages(FmListener.MSGID_POWERUP_FINISHED);
                    mFmServiceHandler.removeMessages(FmListener.MSGID_POWERDOWN_FINISHED);
                    Bundle bundle = new Bundle(bundleSize);
                    bundle.putFloat(FM_FREQUENCY, FmUtils.computeFrequency(mCurrentStation));
                    handlePowerUp(bundle);
                }
                setMute(false);
                Log.d(TAG, "audio focus re-gain, force use: " + getForceUse() +
                        " isSpeakerUsed: " + mIsSpeakerUsed);
                if (getForceUse() != AudioSystem.FORCE_SPEAKER && mIsSpeakerUsed) {
                     AudioSystem.setForceUse(FOR_PROPRIETARY, getForceUse());
                     mIsSpeakerUsed = false;
                 } else if (getForceUse() == AudioSystem.FORCE_SPEAKER && !mIsSpeakerUsed) {
                     AudioSystem.setForceUse(FOR_PROPRIETARY, getForceUse());
                     mIsSpeakerUsed = true;
                 } else if (getForceUse() == AudioSystem.FORCE_SPEAKER && mIsSpeakerUsed){
                     mRegainAudioFocusAndSetForceUseSpeaker = true;
                 }
                break;

            default:
                break;
        }
    }

    private void forceToHeadsetMode() {
        Log.d(TAG, "forceToHeadsetMode");
        if (mIsSpeakerUsed && isHeadSetIn()) {
            AudioSystem.setForceUse(FOR_PROPRIETARY, AudioSystem.FORCE_NONE);
            // save user's option to shared preferences.
            FmUtils.setIsSpeakerModeOnFocusLost(mContext, true);
        }
    }

    /**
     * FM Radio listener record
     */
    private static class Record {
        int mHashCode; // hash code
        FmListener mCallback; // call back
    }

    /**
     * Register FM Radio listener, activity get service state should call this
     * method register FM Radio listener
     *
     * @param callback FM Radio listener
     */
    public void registerFmRadioListener(FmListener callback) {
        synchronized (mRecords) {
            // register callback in AudioProfileService, if the callback is
            // exist, just replace the event.
            Record record = null;
            int hashCode = callback.hashCode();
            final int n = mRecords.size();
            for (int i = 0; i < n; i++) {
                record = mRecords.get(i);
                if (hashCode == record.mHashCode) {
                    return;
                }
            }
            record = new Record();
            record.mHashCode = hashCode;
            record.mCallback = callback;
            mRecords.add(record);
        }
    }

    /**
     * Call back from service to activity
     *
     * @param bundle The message to activity
     */
    private void notifyActivityStateChanged(Bundle bundle) {
        if (Log.isLoggable(TAG, Log.VERBOSE)) {
            Log.v(TAG, "notifyActivityStateChanged: " + mRecords.size());
        }
        if (!mRecords.isEmpty()) {
            synchronized (mRecords) {
                Iterator<Record> iterator = mRecords.iterator();
                while (iterator.hasNext()) {
                    Record record = (Record) iterator.next();

                    FmListener listener = record.mCallback;

                    if (listener == null) {
                        iterator.remove();
                        return;
                    }

                    listener.onCallBack(bundle);
                }
            }
        } else {
            if (Log.isLoggable(TAG, Log.VERBOSE)) {
                Log.v(TAG, "notifyActivityStateChanged: " + mRecords.isEmpty());
            }
        }
    }

    /**
     * Call back from service to the current request activity
     * Scan need only notify FmFavoriteActivity if current is FmFavoriteActivity
     *
     * @param bundle The message to activity
     */
    private void notifyCurrentActivityStateChanged(Bundle bundle) {
        Log.d(TAG, "notifyCurrentActivityStateChanged = " + mRecords.size());
        if (!mRecords.isEmpty()) {
            synchronized (mRecords) {
                if (mRecords.size() > 0) {
                    Record record  = mRecords.get(mRecords.size() - 1);
                    FmListener listener = record.mCallback;
                    if (listener == null) {
                        mRecords.remove(record);
                        return;
                    }
                    listener.onCallBack(bundle);
                }
            }
        } else {
            Log.d(TAG, "notifyActivityStateChanged: " + mRecords.isEmpty());
        }
    }

    /**
     * Unregister FM Radio listener
     *
     * @param callback FM Radio listener
     */
    public void unregisterFmRadioListener(FmListener callback) {
        Log.i(TAG, "unregisterFmRadioListener");
        remove(callback.hashCode());
    }

    /**
     * Remove call back according hash code
     *
     * @param hashCode The call back hash code
     */
    private void remove(int hashCode) {
        Log.i(TAG, "remove");
        synchronized (mRecords) {
            Iterator<Record> iterator = mRecords.iterator();
            while (iterator.hasNext()) {
                Record record = (Record) iterator.next();
                if (record.mHashCode == hashCode) {
                    iterator.remove();
                }
            }
        }
    }

    /**
     * Check recording sd card is unmount
     *
     * @param intent The unmount sd card intent
     *
     * @return true or false indicate whether current recording sd card is
     *         unmount or not
     */
    public boolean isRecordingCardUnmount(Intent intent) {
        String unmountSDCard = intent.getData().toString();
        Log.d(TAG, "unmount sd card file path: " + unmountSDCard);
        return unmountSDCard.equalsIgnoreCase("file://" + sRecordingSdcard) ? true : false;
    }

    private int[] updateStations(int[] stations) {
        Log.d(TAG, "updateStations.firstValidstation:" + Arrays.toString(stations));
        int firstValidstation = mCurrentStation;

        int stationNum = 0;
        if (null != stations) {
            int searchedListSize = stations.length;
            // get stations from db
            stationNum = updateDBInLocation(stations);
        }

        Log.d(TAG, "updateStations.firstValidstation:" + firstValidstation +
                ",stationNum:" + stationNum);
        return (new int[] {
                firstValidstation, stationNum
        });
    }

    /**
     * update DB, keep favorite and rds which is searched this time,
     * delete rds from db which is not searched this time.
     * @param stations
     * @return number of valid searched stations
     */
    private int updateDBInLocation(int[] stations) {
        int stationNum = 0;
        int searchedListSize = stations.length;
        ArrayList<Integer> stationsInDB = new ArrayList<Integer>();
        Cursor cursor = null;
        try {
            // get non favorite stations
            cursor = mContext.getContentResolver().query(Station.CONTENT_URI,
                    new String[] { FmStation.Station.FREQUENCY },
                    FmStation.Station.IS_FAVORITE + "=0",
                    null, FmStation.Station.FREQUENCY);
            if ((null != cursor) && cursor.moveToFirst()) {

                do {
                    int freqInDB = cursor.getInt(cursor.getColumnIndex(
                            FmStation.Station.FREQUENCY));
                    stationsInDB.add(freqInDB);
                } while (cursor.moveToNext());

            } else {
                Log.d(TAG, "updateDBInLocation, insertSearchedStation cursor is null");
            }
        } finally {
            if (null != cursor) {
                cursor.close();
            }
        }

        int listSizeInDB = stationsInDB.size();
        // delete station if db frequency is not in searched list
        for (int i = 0; i < listSizeInDB; i++) {
            int freqInDB = stationsInDB.get(i);
            for (int j = 0; j < searchedListSize; j++) {
                int freqSearched = stations[j];
                if (freqInDB == freqSearched) {
                    break;
                }
                if (j == (searchedListSize - 1) && freqInDB != freqSearched) {
                    // delete from db
                    FmStation.deleteStationInDb(mContext, freqInDB);
                }
            }
        }

        // add to db if station is not in db
        for (int j = 0; j < searchedListSize; j++) {
            int freqSearched = stations[j];
            if (FmUtils.isValidStation(freqSearched)) {
                stationNum++;
                if (!stationsInDB.contains(freqSearched)
                        && !FmStation.isFavoriteStation(mContext, freqSearched)) {
                    // insert to db
                    FmStation.insertStationToDb(mContext, freqSearched, "");
                }
            }
        }
        return stationNum;
    }

    /**
     * The background handler
     */
    class FmRadioServiceHandler extends Handler {
        public FmRadioServiceHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            Bundle bundle;
            boolean isPowerup = false;
            boolean isSwitch = true;
        Log.d(TAG, "handleMessage: " + msg.what);
            switch (msg.what) {

                // power up
                case FmListener.MSGID_POWERUP_FINISHED:
                    bundle = msg.getData();
                    handlePowerUp(bundle);
                    break;

                // power down
                case FmListener.MSGID_POWERDOWN_FINISHED:
                    handlePowerDown();
                    break;

                // fm exit
                case FmListener.MSGID_FM_EXIT:
                    if (mIsSpeakerUsed) {
                        setForceUse(false);
                    }
                    powerDown();
                    closeDevice();

                    bundle = new Bundle(1);
                    bundle.putInt(FmListener.CALLBACK_FLAG, FmListener.MSGID_FM_EXIT);
                    notifyActivityStateChanged(bundle);
                    // Finish favorite when exit FM
                    if (sExitListener != null) {
                        sExitListener.onExit();
                    }
                    break;

                // switch antenna
                case FmListener.MSGID_SWITCH_ANTENNA:
                    bundle = msg.getData();
                    int value = bundle.getInt(FmListener.SWITCH_ANTENNA_VALUE);

                    // if ear phone insert, need dismiss plugin earphone
                    // dialog
                    // if earphone plug out and it is not play recorder
                    // state, show plug dialog.
                    if (0 == value) {
                        // powerUpAsync(FMRadioUtils.computeFrequency(mCurrentStation));
                        bundle.putInt(FmListener.CALLBACK_FLAG,
                                FmListener.MSGID_SWITCH_ANTENNA);
                        bundle.putBoolean(FmListener.KEY_IS_SWITCH_ANTENNA, true);
                        notifyActivityStateChanged(bundle);
                    } else {
                        // ear phone plug out, and recorder state is not
                        // play recorder state,
                        // show dialog.
                        if (mRecordState != FmRecorder.STATE_PLAYBACK) {
                            bundle.putInt(FmListener.CALLBACK_FLAG,
                                    FmListener.MSGID_SWITCH_ANTENNA);
                            bundle.putBoolean(FmListener.KEY_IS_SWITCH_ANTENNA, false);
                            notifyActivityStateChanged(bundle);
                        }
                    }
                    break;

                // tune to station
                case FmListener.MSGID_TUNE_FINISHED:
                    bundle = msg.getData();
                    float tuneStation = bundle.getFloat(FM_FREQUENCY);
                    boolean isTune = tuneStation(tuneStation);
                    // if tune fail, pass current station to update ui
                    if (!isTune) {
                        tuneStation = FmUtils.computeFrequency(mCurrentStation);
                    }
                    bundle = new Bundle(3);
                    bundle.putInt(FmListener.CALLBACK_FLAG,
                            FmListener.MSGID_TUNE_FINISHED);
                    bundle.putBoolean(FmListener.KEY_IS_TUNE, isTune);
                    bundle.putFloat(FmListener.KEY_TUNE_TO_STATION, tuneStation);
                    notifyActivityStateChanged(bundle);
                    break;

                // seek to station
                case FmListener.MSGID_SEEK_FINISHED:
                    bundle = msg.getData();
                    mIsSeeking = true;
                    float seekStation = seekStation(bundle.getFloat(FM_FREQUENCY),
                            bundle.getBoolean(OPTION));
                    boolean isStationTunningSuccessed = false;
                    int station = FmUtils.computeStation(seekStation);
                    if (FmUtils.isValidStation(station)) {
                        isStationTunningSuccessed = tuneStation(seekStation);
                    }
                    // if tune fail, pass current station to update ui
                    if (!isStationTunningSuccessed) {
                        seekStation = FmUtils.computeFrequency(mCurrentStation);
                    }
                    bundle = new Bundle(2);
                    bundle.putInt(FmListener.CALLBACK_FLAG,
                            FmListener.MSGID_TUNE_FINISHED);
                    bundle.putBoolean(FmListener.KEY_IS_TUNE, isStationTunningSuccessed);
                    bundle.putFloat(FmListener.KEY_TUNE_TO_STATION, seekStation);
                    notifyActivityStateChanged(bundle);
                    mIsSeeking = false;
                    break;

                // start scan
                case FmListener.MSGID_SCAN_FINISHED:
                    int[] stations = null;
                    int[] result = null;
                    int scanTuneStation = 0;
                    boolean isScan = true;
                    boolean isWakeLockAcquiredBeforeScan = false;
                    mIsScanning = true;
                    // added to handle the case when current station is reset, happens when
                    // fm is closed due to changed permission & restarts from FmFavoriteActivity
                    // in other cases, both values should be same
                    mCurrentStation = FmStation.getCurrentStation(mContext);
                    if ((mPowerStatus == POWER_UP) ||
                            (powerUp(FmUtils.computeFrequency(mCurrentStation)) &&
                                    playFrequency(FmUtils.computeFrequency(mCurrentStation)))) {
                        if (mWakeLock != null && !mWakeLock.isHeld()) {
                            isWakeLockAcquiredBeforeScan = true;
                            mWakeLock.acquire();
                        }
                        Log.i(TAG, "handleMessage: startScan");
                        stations = startScan();
                    }
                    Log.i(TAG, "handleMessage: endScan");
                    // check whether cancel scan
                    if ((null != stations) && stations[0] == -100) {
                        isScan = false;
                        result = new int[] {
                                -1, 0
                        };
                    } else {
                        result = updateStations(stations);
                        scanTuneStation = result[0];
                        if (tuneStation(FmUtils.computeFrequency(mCurrentStation))) {
                            initService(mCurrentStation);
                        }
                    }

                    /*
                     * if there is stop command when scan, so it needs to mute
                     * fm avoid fm sound come out.
                     */
                    if (mIsAudioFocusHeld) {
                        setMute(false);
                    }

                    if (isWakeLockAcquiredBeforeScan && mWakeLock != null && mWakeLock.isHeld()) {
                        mWakeLock.release();
                    }

                    bundle = new Bundle(4);
                    bundle.putInt(FmListener.CALLBACK_FLAG,
                            FmListener.MSGID_SCAN_FINISHED);
                    //bundle.putInt(FmListener.KEY_TUNE_TO_STATION, scanTuneStation);
                    bundle.putInt(FmListener.KEY_STATION_NUM, result[1]);
                    bundle.putBoolean(FmListener.KEY_IS_SCAN, isScan);

                    mIsScanning = false;
                    // Only notify the newest request activity
                    notifyCurrentActivityStateChanged(bundle);
                    break;

                // audio focus changed
                case FmListener.MSGID_AUDIOFOCUS_CHANGED:
                    bundle = msg.getData();
                    int focusState = bundle.getInt(FmListener.KEY_AUDIOFOCUS_CHANGED);
                    updateAudioFocus(focusState);
                    break;

                case FmListener.MSGID_SET_RDS_FINISHED:
                    bundle = msg.getData();
                    setRds(bundle.getBoolean(OPTION));
                    break;

                case FmListener.MSGID_SET_MUTE_FINISHED:
                    bundle = msg.getData();
                    setMute(bundle.getBoolean(OPTION));
                    break;

                case FmListener.MSGID_ACTIVE_AF_FINISHED:
                    activeAf();
                    break;

                /********** recording **********/
                case FmListener.MSGID_STARTRECORDING_FINISHED:
                    startRecording();
                    break;

                case FmListener.MSGID_STOPRECORDING_FINISHED:
                    stopRecording();
                    break;

                case FmListener.MSGID_RECORD_MODE_CHANED:
                    bundle = msg.getData();
                    setRecordingMode(bundle.getBoolean(OPTION));
                    break;

                case FmListener.MSGID_SAVERECORDING_FINISHED:
                    bundle = msg.getData();
                    saveRecording(bundle.getString(RECODING_FILE_NAME));
                    break;

                case FmListener.UPDATE_NOTIFICATION:
                    // need handle normal notification. Recording already handled in FmRecorder
                    showPlayingNotification();
                    break;

                default:
                    break;
            }
        }

    }

    /**
     * handle power down, execute power down and call back to activity.
     */
    private void handlePowerDown() {
            Log.d(TAG, "handlePowerDown");
        Bundle bundle;
        boolean isPowerdown = powerDown();
        bundle = new Bundle(1);
        bundle.putInt(FmListener.CALLBACK_FLAG, FmListener.MSGID_POWERDOWN_FINISHED);
        notifyActivityStateChanged(bundle);
    }

    /**
     * handle power up, execute power up and call back to activity.
     *
     * @param bundle power up frequency
     */
    private void handlePowerUp(Bundle bundle) {
            Log.d(TAG, "handlePowerUp");
        boolean isPowerUp = false;
        boolean isSwitch = true;
        float curFrequency = bundle.getFloat(FM_FREQUENCY);

        if (!isAntennaAvailable()) {
            Log.d(TAG, "handlePowerUp, earphone is not ready");
            bundle = new Bundle(2);
            bundle.putInt(FmListener.CALLBACK_FLAG, FmListener.MSGID_SWITCH_ANTENNA);
            bundle.putBoolean(FmListener.KEY_IS_SWITCH_ANTENNA, false);
            notifyActivityStateChanged(bundle);
            return;
        }
        if (powerUp(curFrequency)) {
            if (FmUtils.isFirstTimePlayFm(mContext)) {
                isPowerUp = firstPlaying(curFrequency);
                FmUtils.setIsFirstTimePlayFm(mContext);
            } else {
                isPowerUp = playFrequency(curFrequency);
            }
            mPausedByTransientLossOfFocus = false;
        }
        bundle = new Bundle(2);
        bundle.putInt(FmListener.CALLBACK_FLAG, FmListener.MSGID_POWERUP_FINISHED);
        bundle.putInt(FmListener.KEY_TUNE_TO_STATION, mCurrentStation);
        notifyActivityStateChanged(bundle);
    }

    /**
     * check FM is foreground or background
     */
    public boolean isActivityForeground() {
        return (mIsFmMainForeground || mIsFmFavoriteForeground || mIsFmRecordForeground);
    }

    /**
     * mark FmMainActivity is foreground or not
     * @param isForeground
     */
    public void setFmMainActivityForeground(boolean isForeground) {
        mIsFmMainForeground = isForeground;
    }

    /**
     * mark FmFavoriteActivity activity is foreground or not
     * @param isForeground
     */
    public void setFmFavoriteForeground(boolean isForeground) {
        mIsFmFavoriteForeground = isForeground;
    }

    /**
     * mark FmRecordActivity activity is foreground or not
     * @param isForeground
     */
    public void setFmRecordActivityForeground(boolean isForeground) {
        mIsFmRecordForeground = isForeground;
    }

    /**
     * mark recording permission
     * @param isPermissionEnabled
     */
    public void setRecordingPermission(boolean isPermissionEnabled) {
        mIsRecordingPermissible = isPermissionEnabled;
    }

    /**
     * Get the recording sdcard path when staring record
     *
     * @return sdcard path like "/storage/sdcard0"
     */
    public static String getRecordingSdcard() {
        return sRecordingSdcard;
    }

    /**
     * The listener interface for exit
     */
    public interface OnExitListener {
        /**
         * When Service finish, should notify FmFavoriteActivity to finish
         */
        void onExit();
    }

    /**
     * Register the listener for exit
     *
     * @param listener The listener want to know the exit event
     */
    public static void registerExitListener(OnExitListener listener) {
        sExitListener = listener;
    }

    /**
     * Unregister the listener for exit
     *
     * @param listener The listener want to know the exit event
     */
    public static void unregisterExitListener(OnExitListener listener) {
        sExitListener = null;
    }

    /**
     * Get the latest recording name the show name in save dialog but saved in
     * service
     *
     * @return The latest recording name or null for not modified
     */
    public String getModifiedRecordingName() {
        return mModifiedRecordingName;
    }

    /**
     * Set the latest recording name if modify the default name
     *
     * @param name The latest recording name or null for not modified
     */
    public void setModifiedRecordingName(String name) {
        mModifiedRecordingName = name;
    }

    @Override
    public void onTaskRemoved(Intent rootIntent) {
        Log.d(TAG, "onTaskRemoved");
        if (mPowerStatus != POWER_UP) {
            exitFm();
            stopSelf();
        }
        if (getRecorderState() == FmRecorder.STATE_RECORDING) {
            // recording was in progress. discard recording and
            // update playing notification
            discardRecording();
            updatePlayingNotification();
        }
        super.onTaskRemoved(rootIntent);
    }

    private boolean firstPlaying(float frequency) {
          Log.d(TAG, "firstPlaying, freq: " + frequency);
        if (mPowerStatus != POWER_UP) {
            Log.w(TAG, "firstPlaying, FM is not powered up");
            return false;
        }
        boolean isSeekTune = false;
        float seekStation = FmNative.seek(frequency, false);
        int station = FmUtils.computeStation(seekStation);
        if (FmUtils.isValidStation(station)) {
            isSeekTune = FmNative.tune(seekStation);
            if (isSeekTune) {
                playFrequency(seekStation);
            }
        }
        // if tune fail, pass current station to update ui
        if (!isSeekTune) {
            seekStation = FmUtils.computeFrequency(mCurrentStation);
        }
        return isSeekTune;
    }

    /**
     * Set notification class name
     * @param clsName The target class name of activity
     */
    public void setNotificationClsName(String clsName) {
        mTargetClassName = clsName;
    }

    // FM Radio EM start
    /**
     * Inquiry if fm stereo mono(true, stereo; false mono)
     *
     * @return (true, stereo; false, mono)
     */
    public boolean getStereoMono() {
        Log.d(TAG, "FMRadioService.getStereoMono");
        return FmNative.stereoMono();
    }

    /**
     * Force set to stero/mono mode
     *
     * @param isMono
     *            (true, mono; false, stereo)
     * @return (true, success; false, failed)
     */
    public boolean setStereoMono(boolean isMono) {
        Log.d(TAG, "FMRadioService.setStereoMono: isMono=" + isMono);
        return FmNative.setStereoMono(isMono);
    }

    /**
     * set RSSI, desense RSSI, mute gain soft
     * @param index flag which will execute
     * (0:rssi threshold,1:desense rssi threshold,2: SGM threshold)
     * @param value send to native
     * @return execute ok or not
     */
    public boolean setEmth(int index, int value) {
        Log.d(TAG, ">>> FMRadioService.setEmth: index=" + index + ",value=" + value);
        boolean isOk = FmNative.emsetth(index, value);
        Log.d(TAG, "<<< FMRadioService.setEmth: isOk=" + isOk);
        return isOk;
    }

    /**
     * send variables to native, and get some variables return.
     * @param val send to native
     * @return get value from native
     */
    public short[] emcmd(short[] val) {
        Log.d(TAG, ">>FMRadioService.emcmd: val=" + val);
        short[] shortCmds = null;
        shortCmds = FmNative.emcmd(val);
        Log.d(TAG, "<<FMRadioService.emcmd:" + shortCmds);
        return shortCmds;
    }

    /**
     * Get hardware version not need async
     */
    public int[] getHardwareVersion() {
        return FmNative.getHardwareVersion();
    }

    /**
     * Read cap array method not need async
     */
    public int getCapArray() {
        Log.d(TAG, "FMRadioService.readCapArray");
        if (mPowerStatus != POWER_UP) {
            Log.w(TAG, "FM is not powered up");
            return -1;
        }
        return FmNative.readCapArray();
    }

    /**
     * Get rssi not need async
     */
    public int getRssi() {
        Log.d(TAG, "FMRadioService.readRssi");
        if (mPowerStatus != POWER_UP) {
            Log.w(TAG, "FM is not powered up");
            return -1;
        }
        return FmNative.readRssi();
    }

    /**
     * read rds bler not need async
     */
    public int getRdsBler() {
        Log.d(TAG, "FMRadioService.readRdsBler");
        if (mPowerStatus != POWER_UP) {
            Log.w(TAG, "FM is not powered up");
            return -1;
        }
        return FmNative.readRdsBler();
    }
    // FM Radio EM end

    private boolean isMusicOutputId(int sourcePortId) {
        if (mOutputPortId == -1) {
            mOutputPortId  = Integer.parseInt(SystemProperties.get("af.music.outputid", "0"));
            Log.d(TAG, "getMusicOutputId");
        }

        if (Log.isLoggable(TAG, Log.VERBOSE)) {
            Log.v(TAG, "getMusicOutputId = " + mOutputPortId + "sourcePortId = " + sourcePortId);
        }
        if (mOutputPortId == 0) {
            Log.d(TAG, "getMusicOutputId not set, consider all as primary mixer ");
            return true;
        } else if (mOutputPortId == sourcePortId) {
            return true;
        }
        return false;
    }

    private boolean isBtDevice(int device) {
        if (device == AudioSystem.DEVICE_OUT_BLUETOOTH_A2DP ||
                    device == AudioSystem.DEVICE_OUT_BLUETOOTH_A2DP_HEADPHONES ||
                    device == AudioSystem.DEVICE_OUT_BLUETOOTH_A2DP_SPEAKER ||
                    device == AudioSystem.DEVICE_OUT_AUX_DIGITAL ||
                    device == AudioSystem.DEVICE_OUT_USB_ACCESSORY ||
                    device == AudioSystem.DEVICE_OUT_USB_DEVICE ||
                    device == AudioSystem.DEVICE_OUT_REMOTE_SUBMIX ||
                    device == AudioSystem.DEVICE_OUT_USB_HEADSET) {
            Log.d(TAG, "isBtDevice:true, device =" + device);
            return true;
        } else {
            Log.d(TAG, "isBtDevice:false, device =" + device);
            return false;
        }
    }

    static {
        initSmartPAandWithoutDSP();
        initVibSpkSupportOn();
    }

    public static void initSmartPAandWithoutDSP() {
        if (sIsSmartPAandWithoutDSP == false) {
           String state = AudioSystem.getParameters(GET_SMART_PA_STATUS);
           sIsSmartPAandWithoutDSP = SMART_PA_STATUS_ENABLED.equals(state);
           Log.d(TAG, "SMART_PA state = " + state);
           Log.d(TAG, "sIsSmartPAandWithoutDSP = " + sIsSmartPAandWithoutDSP);
        }
    }
    private boolean isRenderForSpeaker() {
        Log.d(TAG, "sIsSmartPAandWithoutDSP:" + sIsSmartPAandWithoutDSP + " sIsVibSpeakerSupportOn:"
              + sIsVibSpeakerSupportOn);
        if (sIsVibSpeakerSupportOn || sIsSmartPAandWithoutDSP) {
            return true;
        }
        return false;
    }
    public static void initVibSpkSupportOn() {
        if(sIsVibSpeakerSupportOn == false) {
            sIsVibSpeakerSupportOn = SystemProperties.getBoolean("ro.vendor.mtk_vibspk_support", false);
            Log.d(TAG, "sIsVibSpeakerSupportOn : " + sIsVibSpeakerSupportOn);
       }
    }
}