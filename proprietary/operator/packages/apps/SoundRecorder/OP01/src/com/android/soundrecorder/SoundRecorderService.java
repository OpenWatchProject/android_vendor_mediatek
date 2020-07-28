/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */
package com.android.soundrecorder;

import android.app.Notification;
import android.app.PendingIntent;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.res.Resources;
import android.database.Cursor;
import android.media.AudioManager;
import android.media.AudioManager.OnAudioFocusChangeListener;
import android.media.MediaScannerConnection;
import android.media.MediaScannerConnection.MediaScannerConnectionClient;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Binder;
import android.os.Environment;
import android.os.FileObserver;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.StatFs;
import android.os.storage.StorageManager;
import android.provider.MediaStore;
import android.view.View;
import android.widget.RemoteViews;
import android.widget.Toast;

import com.android.soundrecorder.RecordParamsSetting.RecordParams;

import com.mediatek.soundrecorder.ext.ExtensionHelper;
import com.mediatek.soundrecorder.ext.IRecordingTimeCalculationExt;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.Date;

/**
 * Sound Recorder Service Class.
 */
public class SoundRecorderService extends Service implements Player.PlayerListener,
        Recorder.RecorderListener, MediaScannerConnectionClient {

    private static final String TAG = "SR/SoundRecorderService";
    public static final int STATE_IDLE = 1;
    public static final int STATE_RECORDING = 2;
    public static final int STATE_PAUSE_RECORDING = 3;
    public static final int STATE_PLAYING = 4;
    public static final int STATE_PAUSE_PLAYING = 5;
    public static final int STATE_ERROR = 6;
    public static final int STATE_ERROR_CODE = 100;
    public static final int STATE_SAVE_SUCESS = 7;
    public static final int EVENT_SAVE_SUCCESS = 1;
    public static final int EVENT_DISCARD_SUCCESS = 2;
    public static final int EVENT_STORAGE_MOUNTED = 3;
    public static final long LOW_STORAGE_THRESHOLD = 2048 * 1024L;
    public static final String SELECTED_RECORDING_FORMAT = "selected_recording_format";
    public static final String SELECTED_RECORDING_MODE = "selected_recording_mode";
    public static final String SELECTED_RECORDING_EFFECT_AEC = "selected_recording_effect_aec";
    public static final String SELECTED_RECORDING_EFFECT_AGC = "selected_recording_effect_agc";
    public static final String SELECTED_RECORDING_EFFECT_NS = "selected_recording_effect_ns";
    public static final String SELECTED_RECORDING_EFFECT_AEC_TMP =
            "selected_recording_effect_aec_tmp";
    public static final String SELECTED_RECORDING_EFFECT_AGC_TMP =
            "selected_recording_effect_agc_tmp";
    public static final String SELECTED_RECORDING_EFFECT_NS_TMP =
            "selected_recording_effect_ns_tmp";
    public static final String HANDLER_THREAD_NAME = "SoundRecorderServiceHandler";
    public static final long WAIT_TIME = 100;
    private static final int ONE_SECOND = 1000;
    public static final int ERROR_PATH_NOT_EXIST = -100;

    // Broadcast messages from other sounder APP or Camera
    public static final String SOUND_POWER_DOWN_MSG = "com.android.music.musicservicecommand";
    public static final String CMDPAUSE = "pause";

    // the intent action from notification
    private static final String ACTION_GOON_RECORD = "goon record";
    private static final String ACTION_GOON_PLAY = "goon play";
    private static final String ACTION_STOP = "stop";
    private static final String ACTION_PAUSE = "pause";
    private static final String RECORDING = "Recording";
    private static final String VOLUME_NAME = "external";
    private static final String COMMAND = "command";
    private static final String ALBUM_RECORDER = "recorder";

    private static final String SOUND_RECORDER_DATA = "sound_recorder_data";

    private static final int START_NOTIFICATION_ID = 1;
    private static final long FACTOR_FOR_SECOND_AND_MINUTE = 1000;
    private static final int PLAYLIST_ID_NULL = -1;

    private SoundRecorderBinder mBinder = new SoundRecorderBinder();
    private OnErrorListener mOnErrorListener = null;
    private OnStateChangedListener mOnStateChangedListener = null;
    private OnEventListener mOnEventListener = null;

    private AudioManager mAudioManager = null;
    private StorageManager mStorageManager = null;
    private Recorder mRecorder = null;
    private Player mPlayer = null;
    private RemainingTimeCalculator mRemainingTimeCalculator = null;
    private RecordParams mCurrentRecordParams = null;
    private MediaScannerConnection mConnection = null;
    private Uri mUri = null;
    private String mCurrentFilePath = null;
    private String mFilePathToScan = null;
    private long mCurrentFileDuration = -1;
    private long mTotalRecordingDuration = -1;
    private int mCurrentState = STATE_IDLE;
    private BroadcastReceiver mStorageBroastReceiver = null;
    private BroadcastReceiver mOtherBroastReceiver = null;
    private boolean mRunForeground = false;
    private boolean mShowNotifiaction = true;
    private RecordingFileObserver mFileObserver = null;
    private Handler mFileObserverHandler = null;
    private OnAudioFocusChangeListener mFocusChangeListener = null;
    private boolean mGetFocus = false;
    /* M: Add for update time view through
     * implements the listener defined by SoundRecorderService. @{*/
    private OnUpdateTimeViewListener mOnUpdateTimeViewListener = null;
    private long mRemainingTime = 0;
    private final Handler mHandler  = new Handler();
    private final Runnable mUpdateTimer = new Runnable() {
        public void run() {
            LogUtils.i(TAG, "run()-mUpdateTimer running");
            if (STATE_RECORDING == mCurrentState) {
                mRemainingTime = mRemainingTimeCalculator.timeRemaining(false, false);
                if (mRemainingTime == ERROR_PATH_NOT_EXIST) {
                    reset();
                    return;
                }
                LogUtils.i(TAG, "run()-mRemainingTime is:" + mRemainingTime);
            }
            if ((mRemainingTime <= 0) && (STATE_RECORDING == mCurrentState)) {
                LogUtils.i(TAG, "run()-stopRecordingAsync case1");
                stopRecordingAsync();
            } else if (mCurrentState == STATE_IDLE) {
                LogUtils.i(TAG, "run()-stopRecordingAsync case2");
                stopRecordingAsync();
            } else {
                if (null != mOnUpdateTimeViewListener) {
                    // Added to resolve the timing problem(runnable can't stop as expected)
                    try {
                        int time = getCurrentProgressInSecond();
                        mOnUpdateTimeViewListener.updateTimerView(time);
                    } catch (IllegalStateException e) {
                        LogUtils.i(TAG, "run()-IllegalStateException");
                        return;
                    }
                }
                mHandler.postDelayed(mUpdateTimer, WAIT_TIME);
            }
        }
    };
    /**@}*/

    // notificaton related
    private RemoteViews mNotificationView = null;

    // recording parameters related
    private int mSelectedFormat = -1;
    private int mSelectedMode = -1;
    private boolean[] mSelectEffectArray = new boolean[3];
    private boolean[] mSelectEffectArrayTemp = new boolean[3];
    private SharedPreferences mPrefs = null;
    private RecordParams mParams;
    private int mFileSizeLimit;
    private SoundRecorderServiceHandler mSoundRecorderServiceHandler;
    private HandlerThread mHandlerThread = null;

    private static final int NOT_AVILABLE = 1;
    private static final int SAVE_SUCCESS = 2;
    private Handler mToastHandler;
    private boolean mRecordSaving = false;

    private void displayToast(int code) {
        mToastHandler.removeMessages(code);
        mToastHandler.sendEmptyMessage(code);
        mCurrentFileDuration = 0;
        setCurrentFilePath(null);
        /**M:If error displayed, then set current state as STATE_IDLE.@{**/
        setState(STATE_IDLE);
        /**@}**/
    }
    /**@}**/

    /**
     * Error callback.
     */
    public interface OnErrorListener {
        /**
         * Error Callback.
         *
         * @param errorCode error to notify
         */
        public void onError(int errorCode);
    }

    /**
     * State change listener.
     */
    public interface OnStateChangedListener {
        /**
         * State Change Callback.
         *
         * @param stateCode state to notify
         */
        public void onStateChanged(int stateCode);
    }

    /**
     * Event interface.
     */
    public interface OnEventListener {
        /**
         * Event callback.
         *
         * @param eventCode event to notify
         */
        public void onEvent(int eventCode);
    }

    /**
     * Time update interface.
     */
    public interface OnUpdateTimeViewListener {
        /**
         * Update timer view callback.
         *
         * @param time time to be updated.
         */
        public void updateTimerView(int time);
    }

    /**
     * Button update interface.
     */
    public interface OnUpdateButtonState {
        /**
         * update button callback.
         *
         * @param enable enable or disable button
         */
        public void updateButtonState(boolean enable);
    }

    @Override
    public void onError(Player player, int errorCode) {
        LogUtils.i(TAG, "<Player onError> errorCode = " + errorCode);
        reset();
        mHandler.removeCallbacks(mUpdateTimer);
        mOnErrorListener.onError(errorCode);
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        LogUtils.i(TAG, "<onStartCommand> start");
        if (null == intent) {
            /**M: service slim @{**/
            return START_NOT_STICKY;
            //return super.onStartCommand(intent, flags, startId);
            /** @}**/
        }

        String action = intent.getAction();
        LogUtils.i(TAG, "<onStartCommand> action = " + action);

        if (null == action) {
            /**M: service slim @{**/
            return START_NOT_STICKY;
            //return super.onStartCommand(intent, flags, startId);
            /** @}**/
        }

        if (action.equals(ACTION_GOON_RECORD) && mCurrentState == STATE_PAUSE_RECORDING) {
            LogUtils.i(TAG, "<onStartCommand> ACTION_GOON_RECORD");
            startRecordingAsync(mCurrentRecordParams, 0, null);
        } else if (action.equals(ACTION_GOON_PLAY) && mCurrentState == STATE_PAUSE_PLAYING) {
            LogUtils.i(TAG, "<onStartCommand> ACTION_GOON_PLAY");
            goonPlaybackAsync();
        } else if (action.equals(ACTION_PAUSE)) {
            LogUtils.i(TAG, "<onStartCommand> ACTION_PAUSE");
            if (mCurrentState == STATE_RECORDING) {
                pauseRecordingAsync();
            } else if (mCurrentState == STATE_PLAYING) {
                pausePlaybackAsync();
            }
        } else if (action.equals(ACTION_STOP)) {
            LogUtils.i(TAG, "<onStartCommand> ACTION_STOP");
            if (mCurrentState == STATE_RECORDING || mCurrentState == STATE_PAUSE_RECORDING) {
                stopRecordingAsync();
            } else if (mCurrentState == STATE_PLAYING || mCurrentState == STATE_PAUSE_PLAYING) {
                stopPlaybackAsync();
            }
        }
        LogUtils.i(TAG, "<onStartCommand> end");
        /**M: service slim @{**/
        return START_NOT_STICKY;
        //return super.onStartCommand(intent, flags, startId);
        /** @}**/
    }

    @Override
    public void onStateChanged(Player player, int stateCode) {
        mRemainingTime = 0;
        if (stateCode == STATE_PLAYING) {
            mCurrentFileDuration = mPlayer.getFileDuration();
        }
        int preState = mCurrentState;
        setState(stateCode);
        LogUtils.i(TAG, "onStateChanged(Player,int) preState = " + preState + ", mCurrentState = "
                + mCurrentState);
        if (mCurrentState == STATE_IDLE) {
            if ((STATE_PLAYING == preState) || (STATE_PAUSE_PLAYING == preState)) {
                LogUtils.i(TAG, "onStateChanged(Player,int) removeCallbacks mUpdateTimer.");
                mHandler.removeCallbacks(mUpdateTimer);
                abandonAudioFocus();
            }
            if (mRunForeground) {
                hideNotifiaction();
            }
        } else {
            if (mCurrentState == STATE_PLAYING) {
                LogUtils.i(TAG, "onStateChanged(Player,int) post mUpdateTimer.");
                mHandler.post(mUpdateTimer);
            } else if (mCurrentState == STATE_PAUSE_PLAYING) {
                LogUtils.i(TAG, "onStateChanged(Player,int) removeCallbacks mUpdateTimer.");
                mHandler.removeCallbacks(mUpdateTimer);
                abandonAudioFocus();
            }
            if (mRunForeground) {
                LogUtils.i(TAG, "onStateChanged(Player,int) update notificaton");
                showNotification(SoundRecorderService.this);
            } else {
                LogUtils.i(TAG, "onStateChanged(Player,int) show notificaton");
                showNotification(SoundRecorderService.this);
            }
        }

    }

    @Override
    public void onStateChanged(Recorder recorder, int stateCode) {
        mRemainingTime = 0;
        /**
         * M: Modified to avoid the mUpdateTimer start while the mCurrentState not
         * yet changed which will cause 1 second auto saving problem. @{
         **/
        if (stateCode != STATE_IDLE) {
            setCurrentFilePath(mRecorder.getSampleFilePath());
        }
        int preState = mCurrentState;
        setState(stateCode);
        LogUtils.i(TAG, "onStateChanged(Recorder,int) preState = " + preState + ", mCurrentState = "
                + mCurrentState);
        if (STATE_IDLE == mCurrentState) {
            if ((STATE_PAUSE_RECORDING == preState) || (STATE_RECORDING == preState)) {
                abandonAudioFocus();
                getRecordInfoAfterStopRecord();
            }
            if (mRunForeground) {
                hideNotifiaction();
            }
            return;
        } else {
            if (STATE_RECORDING == mCurrentState) {
                // Refresh the mRemainingTime
                mRemainingTime = mRemainingTimeCalculator.timeRemaining(false, false);
                mHandler.post(mUpdateTimer);
                LogUtils.i(TAG, "onStateChanged(Recorder,int) post mUpdateTimer.");
            }

            if (mRunForeground) {
                LogUtils.i(TAG, "onStateChanged(Recorder,int) update notificaton");
                showNotification(SoundRecorderService.this);
            } else {
                LogUtils.i(TAG, "onStateChanged(Recorder,int) show notificaton");
                showNotification(SoundRecorderService.this);
            }
        }
    }


    @Override
    public void onError(Recorder recorder, int errorCode) {
        LogUtils.i(TAG, "<Recorder onError> errorCode = " + errorCode);
        reset();
        mHandler.removeCallbacks(mUpdateTimer);
        mOnErrorListener.onError(errorCode);
    }

    @Override
    /**
     * M: after add record to Media database,
     * use MediaScanner to scan file which has been added
     */
    public void onMediaScannerConnected() {
        LogUtils.i(TAG, "<onMediaScannerConnected> scan file: " + mFilePathToScan);
        mConnection.scanFile(mFilePathToScan, null);
    }

    @Override
    /**
     * M: after scan completed, update record item
     */
    public void onScanCompleted(String path, Uri uri) {
        LogUtils.i(TAG, "<onScanCompleted> start, path = " + path);
        Resources res = getResources();
        long current = System.currentTimeMillis();
        Date date = new Date(current);
        SimpleDateFormat simpleDateFormat = new SimpleDateFormat(getResources().getString(
                R.string.audio_db_title_format));
        String title = simpleDateFormat.format(date);

        StringBuilder stringBuilder = new StringBuilder();
        stringBuilder.append(MediaStore.Audio.Media.DATA);
        stringBuilder.append(" LIKE '%");
        stringBuilder.append(path.replaceFirst("file:///", ""));
        stringBuilder.append("'");
        final String where = stringBuilder.toString();
        final int size = 6;
        ContentValues cv = new ContentValues(size);
        cv.put(MediaStore.Audio.Media.IS_MUSIC, "0");
        cv.put(MediaStore.Audio.Media.TITLE, title);
        cv.put(MediaStore.Audio.Media.DATE_ADDED, (int) (current / FACTOR_FOR_SECOND_AND_MINUTE));
        cv.put(MediaStore.Audio.Media.DATA, path);
        cv.put(MediaStore.Audio.Media.ARTIST, res.getString(R.string.unknown_artist_name));
        cv.put(MediaStore.Audio.Media.ALBUM, res.getString(R.string.audio_db_album_name));
        cv.put(MediaStore.Audio.Media.ALBUM_ARTIST, ALBUM_RECORDER);
        ContentResolver resolver = getContentResolver();
        Uri base = MediaStore.Audio.Media.EXTERNAL_CONTENT_URI;
        int result = resolver.update(base, cv, where, null);
        LogUtils.i(TAG, "<onScanCompleted> update result = " + result);
        mConnection.disconnect();
        LogUtils.i(TAG, "<onScanCompleted> end");
    }

    public void setErrorListener(OnErrorListener listener) {
        mOnErrorListener = listener;
    }

    public void setEventListener(OnEventListener listener) {
        mOnEventListener = listener;
    }

    public void setStateChangedListener(OnStateChangedListener listener) {
        mOnStateChangedListener = listener;
    }

    public void setUpdateTimeViewListener(OnUpdateTimeViewListener listener) {
        mOnUpdateTimeViewListener = listener;
    }
    @Override
    public IBinder onBind(Intent arg0) {
        LogUtils.i(TAG, "<onBind>");
        return mBinder;
    }

    /**
     * Set all listener as sevice itself.
     */
    public void setAllListenerSelf() {
        // set mOnErrorListener as a new listener when activity stopped/killed,
        // when error occours, show error info in toast
        LogUtils.i(TAG, "<setAllListenerSelf> set new mOnErrorListener");
        mOnErrorListener = new OnErrorListener() {
            public void onError(int errorCode) {
                final int errCode = errorCode;
                mHandler.post(new Runnable() {
                    public void run() {
                        ErrorHandle.showErrorInfoInToast(getApplicationContext(), errCode);
                    }
                });
            };
        };
        mOnEventListener = null;
        mOnStateChangedListener = null;
        mOnUpdateTimeViewListener = null;
    }

    public void setShowNotification(boolean show) {
        mShowNotifiaction = show;
    }

    /**
     * Service binder.
     */
    public class SoundRecorderBinder extends Binder {
        SoundRecorderService getService() {
            return SoundRecorderService.this;
        }
    }

    @Override
    public void onCreate() {
        PDebug.start("SoundRecorderService - onCreate");
        super.onCreate();
        LogUtils.i(TAG, "<onCreate> start");
        mAudioManager = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
        mStorageManager = (StorageManager) getSystemService(Context.STORAGE_SERVICE);
        mRecorder = new Recorder(mStorageManager, this);
        mPlayer = new Player(this);
        mConnection = new MediaScannerConnection(getApplicationContext(), this);
        mFocusChangeListener = new OnAudioFocusChangeListener() {
            @Override
            public void onAudioFocusChange(int focusChange) {
                LogUtils.i(TAG, "<onAudioFocusChange> audio focus changed to " + focusChange);
                if (focusChange == AudioManager.AUDIOFOCUS_GAIN) {
                    LogUtils.i(TAG, "<onAudioFocusChange> audio focus changed to AUDIOFOCUS_GAIN");
                    mGetFocus = true;
                } else if (focusChange == AudioManager.AUDIOFOCUS_LOSS
                        || focusChange == AudioManager.AUDIOFOCUS_LOSS_TRANSIENT) {
                    LogUtils.i(TAG, "<onAudioFocusChange> audio focus loss, stop recording");
                    mGetFocus = false;
                    if (mCurrentState == STATE_RECORDING ||
                            mCurrentState == STATE_PAUSE_RECORDING) {
                        stopRecordingAsync();
                    } else if (mCurrentState == STATE_PLAYING) {
                        stopPlaybackAsync();
                    }

                    if (isCurrentFileWaitToSave()) {
                        saveRecordAsync();
                    }
                }
            }
        };

        registerBroadcastReceivcer();

        IntentFilter commandFilter = new IntentFilter();
        commandFilter.addAction(ACTION_GOON_PLAY);
        commandFilter.addAction(ACTION_GOON_RECORD);
        commandFilter.addAction(ACTION_PAUSE);
        commandFilter.addAction(ACTION_STOP);

        mHandlerThread = new HandlerThread(HANDLER_THREAD_NAME);
        mHandlerThread.start();
        mSoundRecorderServiceHandler = new SoundRecorderServiceHandler(mHandlerThread.getLooper());

        /**
         * M: To make sure the toast is displayed in UI thread. Handler construct in the UI thread,
         * then uses the main looper for the mToastHandler. @{
         **/
        mToastHandler = new Handler() {
            public void handleMessage(android.os.Message msg) {
                LogUtils.i(TAG, "<mErrorHandler handleMessage>");
                switch (msg.what) {
                    case NOT_AVILABLE:
                        SoundRecorderUtils.getToast(getApplicationContext(),
                                R.string.not_available);
                        break;
                    case SAVE_SUCCESS:
                        SoundRecorderUtils.getToast(getApplicationContext(),
                                R.string.tell_save_record_success);
                        break;
                    default:
                        break;
                }
            };
        };
        /**@}**/
        LogUtils.i(TAG, "<onCreate> end");
        PDebug.end("SoundRecorderService - onCreate");
    }

    @Override
    public void onDestroy() {
        LogUtils.i(TAG, "<onDestroy>");
        storeRecordParamsSettings();
        unregisterBroadcastReceivcer();
        if (null != mSoundRecorderServiceHandler) {
            mSoundRecorderServiceHandler.getLooper().quit();
        }
        super.onDestroy();
    }

    /**
     * Record.
     *
     * @param params recording parameters
     * @param fileSizeLimit the file size limitation
     * @return record result
     */
    public boolean record(RecordParams params, int fileSizeLimit) {
        if (STATE_RECORDING == mCurrentState) {
            LogUtils.i(TAG, "<record> still in STATE_RECORDING, do nothing");
            return true;
        } else if (STATE_PAUSE_RECORDING == mCurrentState) {
            LogUtils.i(TAG, "<record> in STATE_PAUSE_RECORDING, mRecorder.goonRecording()");
            if (requestAudioFocus() && mRecorder.goonRecording()) {
                return true;
            }
            abandonAudioFocus();
            //notify main thread to update ui according current state
            setState(mCurrentState);
            return false;
        } else {
            if ((STATE_PAUSE_PLAYING == mCurrentState) || (STATE_PLAYING == mCurrentState)) {
                LogUtils.i(TAG, "<record> in pause playing or playing, stopPlay first");
                stopPlay();
            }
            if (isCurrentFileEndWithTmp()) {
                LogUtils.i(TAG, "<record> delete not saved file: " + mCurrentFilePath);
                stopWatching();
                File file = new File(mCurrentFilePath);
                file.delete();
            }
            if (!isStorageMounted()) {
                LogUtils.i(TAG, "<record> no storage mounted");
                mOnErrorListener.onError(ErrorHandle.ERROR_NO_SD);
                reset();
                setState(STATE_IDLE);
                return false;
            } else if (isStorageFull(params)) {
                LogUtils.i(TAG, "<record> storage is full");
                mOnErrorListener.onError(ErrorHandle.ERROR_STORAGE_FULL_WHEN_LAUNCH);
                reset();
                setState(STATE_IDLE);
                return false;
            } else {
                IRecordingTimeCalculationExt ext =
                        ExtensionHelper.getRecordingTimeCalculationExt(getApplicationContext());
                mRemainingTimeCalculator = ext.getRemainingTimeCalculator(mStorageManager, this);
                mRemainingTimeCalculator.setBitRate(params.mRemainingTimeCalculatorBitRate);
                LogUtils.i(TAG, "<record> start record");
                boolean res = false;
                if (requestAudioFocus()) {
                    res = mRecorder.startRecording(getApplicationContext(), params, fileSizeLimit);
                    LogUtils.i(TAG, "<record> mRecorder.startRecording res = " + res);
                    mCurrentRecordParams = params;
                    if (res && fileSizeLimit > 0) {
                        mRemainingTimeCalculator.setFileSizeLimit(mRecorder.getSampFile(),
                                fileSizeLimit);
                    } else if (!res) {
                        abandonAudioFocus();
                    }
                } else {
                    displayToast(NOT_AVILABLE);
                }
                return res;
            }
        }
    }

    /**
     * Pause recording.
     *
     * @return pause result
     */
    public boolean pauseRecord() {
        mRemainingTimeCalculator.setPauseTimeRemaining(true);
        if (STATE_PAUSE_RECORDING == mCurrentState) {
            LogUtils.i(TAG, "<pauseRecord> still in STATE_PAUSE_RECORDING, do nothing, return");
            mOnErrorListener.onError(STATE_ERROR_CODE);
            return true;
        } else if (STATE_RECORDING == mCurrentState) {
            // M: we do not abandon audio focus when pause recording
            boolean result = mRecorder.pauseRecording();
            mHandler.removeCallbacks(mUpdateTimer);
            return result;
        } else {
            mOnErrorListener.onError(STATE_ERROR_CODE);
            return false;
        }
    }

    /**
     * Stop recording.
     *
     * @return stop result
     */
    public boolean stopRecord() {
        if ((STATE_PAUSE_RECORDING != mCurrentState) && (STATE_RECORDING != mCurrentState)) {
            LogUtils.i(TAG, "<stopRecord> not in pause record or record state, return");
            mOnErrorListener.onError(STATE_ERROR_CODE);
            return false;
        }
        abandonAudioFocus();
        boolean result = mRecorder.stopRecording();
        mHandler.removeCallbacks(mUpdateTimer);
        return result;
    }

    /**
     * Save recording.
     *
     * @return save result
     */
    public boolean saveRecord() {
        if ((null == mCurrentFilePath) || !mCurrentFilePath.endsWith(Recorder.SAMPLE_SUFFIX)) {
            LogUtils.i(TAG, "<saveRecord> no file need to be saved");
            mOnErrorListener.onError(STATE_ERROR_CODE);
            return false;
        }
        if (!isStorageMounted()) {
            LogUtils.i(TAG, "<saveRecord> no storage mounted");
            mOnErrorListener.onError(ErrorHandle.ERROR_NO_SD);
            return false;
        }
        mRecordSaving = true;
        String currentFilePath = deleteRecordingFileTmpSuffix();
        if (null != currentFilePath) {
            mUri = addToMediaDB(new File(currentFilePath));
            mCurrentFileDuration = 0;
            setCurrentFilePath(null);
            mTotalRecordingDuration = 0;
            if (null != mUri) {
                if (null != mOnEventListener) {
                    mOnEventListener.onEvent(EVENT_SAVE_SUCCESS);
                } else {
                    // M: when mOnEventListener == null, we use service to show toast
                    displayToast(SAVE_SUCCESS);
                }
                mRecordSaving = false;
                return true;
            }
        } else {
            LogUtils.d(TAG, "currentFilePath is null...");
            reset();
            mOnErrorListener.onError(ErrorHandle.ERROR_SAVE_FILE_FAILED);
            mRecordSaving = false;
            return false;
        }
        mOnErrorListener.onError(STATE_ERROR_CODE);
        mRecordSaving = false;
        return false;
    }

    /**
     * Discard recording.
     *
     * @return discard result
     */
    public boolean discardRecord() {
        if (!isStorageMounted()) {
            LogUtils.i(TAG, "<discardRecord> no storage mounted");
            mOnErrorListener.onError(ErrorHandle.ERROR_NO_SD);
            return false;
        }
        if (null == mCurrentFilePath) {
            LogUtils.i(TAG, "<discardRecord> file path is null.");
            mOnErrorListener.onError(STATE_ERROR_CODE);
            return false;
        }
        if (!mCurrentFilePath.isEmpty()) {
            LogUtils.i(TAG, "<discardRecord> mCurrentFilePath = " + mCurrentFilePath);
            String deleteFilePath = mCurrentFilePath;
            // set current file path first, stop watching
            setCurrentFilePath(null);
            File file = new File(deleteFilePath);
            file.delete();
            mCurrentFileDuration = 0;

            if (null != mOnEventListener) {
                mOnEventListener.onEvent(EVENT_DISCARD_SUCCESS);
            }
        } else {
            mOnErrorListener.onError(STATE_ERROR_CODE);
        }
        return true;
    }

    /**
     * Play a file.
     *
     * @param path the file path
     * @return play result
     */
    public boolean playFile(String path) {
        LogUtils.i(TAG, "<playFile> path = " + path);
        if (null == path) {
            return false;
        }
        setCurrentFilePath(path);
        mCurrentFileDuration = 0;
        startPlaybackAsync();
        return true;
    }

    /**
     * Start the playback.
     *
     * @return the playback result
     */
    public boolean startPlayback() {
        LogUtils.i(TAG, "<startPlayback> in idle state, start play");
        mPlayer.setCurrentFilePath(mCurrentFilePath);
        boolean res = false;
        if (requestAudioFocus()) {
            res = mPlayer.startPlayback();
        } else {
            displayToast(NOT_AVILABLE);
        }
        return res;
    }

    /**
     * Pause the playback.
     *
     * @return pause result
     */
    public boolean pausePlay() {
        LogUtils.i(TAG, "<pausePlay> in play state, pause play");
        abandonAudioFocus();
        return mPlayer.pausePlayback();
    }

    /**
     * Resume the playback.
     *
     * @return resume result
     */
    public boolean goonPlayback() {
        LogUtils.i(TAG, "<goonPlayback> in pause play state, goon play");
        boolean res = false;
        if (requestAudioFocus()) {
            res = mPlayer.goonPlayback();
        } else {
            displayToast(NOT_AVILABLE);
        }
        return res;
    }

    /**
     * Stop the playback.
     *
     * @return stop result
     */
    public boolean stopPlay() {
        LogUtils.i(TAG, "<stopPlay>");
        if ((STATE_PAUSE_PLAYING != mCurrentState) && (STATE_PLAYING != mCurrentState)) {
            LogUtils.i(TAG, "<stopPlay> not in play or pause play state, can't stop play");
            mOnErrorListener.onError(STATE_ERROR_CODE);
            return false;
        }
        abandonAudioFocus();
        boolean result = mPlayer.stopPlayback();
        mHandler.removeCallbacks(mUpdateTimer);
        return result;
    }

    /**
     * Get the underlying recorder instance.
     *
     * @return underlying recorder instance
     */
    public Recorder getRecorder() {
        return mRecorder;
    }

    /**
     * Get the current file duration.
     *
     * @return file duration in second
     */
    public long getCurrentFileDurationInSecond() {
        long res = 0;
        long mod = 0;
        if (isCurrentFileEndWithTmp()) {
            res = mTotalRecordingDuration / 1000;
            mod = mTotalRecordingDuration % 1000;
        } else {
            res = mCurrentFileDuration / 1000;
            mod = mCurrentFileDuration % 1000;
        }
        res = (res > 0) ? res : ((mod <= 0) ? 0 : (ONE_SECOND / 1000));
        return res;
    }

    /**
     * Get the current file duration.
     *
     * @return file duration in millisecond
     */
    public long getCurrentFileDurationInMillSecond() {
        long res = 0;
        long mod = 0;
        if (isCurrentFileEndWithTmp()) {
            res = mTotalRecordingDuration;
            mod = mTotalRecordingDuration % 1000;
        } else {
            res = mCurrentFileDuration;
            mod = mTotalRecordingDuration % 1000;
        }
        res = (res > 0) ? res : ((mod <= 0) ? 0 : ONE_SECOND);
        return res;
    }

    /**
     * Get the file path.
     *
     * @return the current file path
     */
    public String getCurrentFilePath() {
        return mCurrentFilePath;
    }

    /**
     * Get the file Uri.
     *
     * @return the file Uri
     */
    public Uri getSaveFileUri() {
        return mUri;
    }

    /**
     * Get the remaining time for recording.
     *
     * @return remaining time
     */
    public long getRemainingTime() {
        return mRemainingTime;
    }

    /**
     * Get the current state of the service.
     *
     * @return the current state of the service.
     */
    public int getCurrentState() {
        return mCurrentState;
    }

    /**
     * Get the currrent recording/playback progress.
     *
     * @return the progress in seconds
     */
    public int getCurrentProgressInSecond() {
        int progress = (int) (getCurrentProgressInMillSecond() / 1000L);
        LogUtils.i(TAG, "<getCurrentProgressInSecond> progress = " + progress);
        return progress;
    }

    /**
     * Get the currrent recording/playback progress.
     *
     * @return the progress in milliseconds
     */
    public long getCurrentProgressInMillSecond() {
        LogUtils.i(TAG, "<getCurrentProgressInMillSecond> called");
        if (mCurrentState == STATE_PAUSE_PLAYING || mCurrentState == STATE_PLAYING) {
            int progress = mPlayer.getCurrentProgress();
            LogUtils.i(TAG, "<getCurrentProgressInMillSecond> progress = " + progress);
            return progress;
        } else if (mCurrentState == STATE_PAUSE_RECORDING || mCurrentState == STATE_RECORDING) {
            return mRecorder.getCurrentProgress();
        }
        return 0;
    }

    /**
     * Check if the current recording file is waiting to save.
     *
     * @return return true if current recording file is waiting to save
     *         otherwise return false
     */
    public boolean isCurrentFileWaitToSave() {
        if (null != mCurrentFilePath && !mRecordSaving) {
            return mCurrentFilePath.endsWith(Recorder.SAMPLE_SUFFIX);
        }
        return false;
    }

    /**
     * Check if the storage is full or not.
     *
     * @param params the recording parameters.
     * @return return true if storage is full, otherwise return false
     */
    public boolean isStorageFull(RecordParams params) {
        IRecordingTimeCalculationExt ext =
                        ExtensionHelper.getRecordingTimeCalculationExt(getApplicationContext());
        RemainingTimeCalculator remainingTimeCalculator =
                ext.getRemainingTimeCalculator(mStorageManager, this);
        remainingTimeCalculator.setBitRate(params.mAudioEncodingBitRate);
        return remainingTimeCalculator.timeRemaining(false, true) < 2;
    }

    /**
     * Check if storage is mounted or not.
     *
     * @return return true if storage is mounted, otherwise return false
     */
    public boolean isStorageMounted() {
        String storageState = mStorageManager.getVolumeState(
                Environment.getExternalStorageDirectory().getAbsolutePath());
        if ((null == storageState) || storageState.equals(Environment.MEDIA_MOUNTED)) {
            return true;
        }
        return false;
    }

    /**
     * Reset the recording.
     *
     * @return always return true
     */
    public boolean reset() {
        if (!mRecorder.reset()) {
            mOnErrorListener.onError(ErrorHandle.ERROR_RECORDING_FAILED);
        }
        mPlayer.reset();
        if ((null != mCurrentFilePath) && mCurrentFilePath.endsWith(Recorder.SAMPLE_SUFFIX)) {
            File file = new File(mCurrentFilePath);
            file.delete();
        }
        setCurrentFilePath(null);
        mCurrentFileDuration = 0;
        hideNotifiaction();
        setState(STATE_IDLE);
        return true;
    }

    /**
     * Save data async task.
     */
    public class SaveDataTask extends AsyncTask<Void, Object, Uri> {
        /**
         * save recording file to database.
         *
         * @return the URI
         */
        protected Uri doInBackground(Void... params) {
            return addToMediaDB(new File(mCurrentFilePath));
        }

        protected void onPostExecute(Uri result) {
            mUri = result;
            mCurrentFileDuration = 0;
            setCurrentFilePath(null);
            mTotalRecordingDuration = 0;
            if (mOnEventListener != null) {
                mOnEventListener.onEvent(EVENT_SAVE_SUCCESS);
            } else {
                Toast.makeText(getApplicationContext(), R.string.tell_save_record_success,
                        Toast.LENGTH_SHORT).show();
            }
        }
    }

    private Uri addToMediaDB(File file) {
        LogUtils.i(TAG, "<addToMediaDB> begin");
        if (null == file) {
            LogUtils.i(TAG, "<addToMediaDB> file is null, return null");
            return null;
        }
        SoundRecorderUtils.deleteFileFromMediaDB(getApplicationContext(), file.getAbsolutePath());
        Resources res = getResources();
        long current = System.currentTimeMillis();
        Date date = new Date(current);
        SimpleDateFormat simpleDateFormat = new SimpleDateFormat(getResources().getString(
                R.string.audio_db_title_format));
        String title = simpleDateFormat.format(date);
        final int size = 8;
        ContentValues cv = new ContentValues(size);
        cv.put(MediaStore.Audio.Media.IS_MUSIC, "0");
        //cv.put(MediaStore.Audio.Media.IS_RECORD, "1");
        cv.put(MediaStore.Audio.Media.TITLE, title);
        cv.put(MediaStore.Audio.Media.DATE_ADDED, (int) (current / FACTOR_FOR_SECOND_AND_MINUTE));
        LogUtils.v(TAG, "<addToMediaDB> File type is " + mCurrentRecordParams.mMimeType);
        cv.put(MediaStore.Audio.Media.MIME_TYPE, mCurrentRecordParams.mMimeType);
        cv.put(MediaStore.Audio.Media.ARTIST, res.getString(R.string.unknown_artist_name));
        cv.put(MediaStore.Audio.Media.ALBUM, res.getString(R.string.audio_db_album_name));
        cv.put(MediaStore.Audio.Media.DATA, file.getAbsolutePath());
        cv.put(MediaStore.Audio.Media.DURATION, mTotalRecordingDuration);
        cv.put(MediaStore.Audio.Media.ALBUM_ARTIST, ALBUM_RECORDER);
        LogUtils.d(TAG, "<addToMediaDB> Reocrding time output to database is :DURATION= "
                + mCurrentFileDuration);
        ContentResolver resolver = getContentResolver();
        Uri base = MediaStore.Audio.Media.EXTERNAL_CONTENT_URI;
        Uri result = null;
        /** M: add exception process @{ */
        try {
            result = resolver.insert(base, cv);
        } catch (UnsupportedOperationException e) {
            LogUtils.e(TAG, "<addToMediaDB> Save in DB failed: " + e.getMessage());
        }
        /** @} */
        if (null == result) {
            mOnErrorListener.onError(ErrorHandle.ERROR_SAVE_FILE_FAILED);
        } else {
            LogUtils.i(TAG, "<addToMediaDB> Save susceeded in DB");
            if (PLAYLIST_ID_NULL == getPlaylistId(res)) {
                createPlaylist(res, resolver);
            }
            int audioId = Integer.valueOf(result.getLastPathSegment());
            if (PLAYLIST_ID_NULL != getPlaylistId(res)) {
                addToPlaylist(resolver, audioId, getPlaylistId(res));
            }
            // Notify those applications such as Music listening to the
            // scanner events that a recorded audio file just created.
            /**
             * M: use MediaScanner to scan record file just be added, replace
             * send broadcast to scan all
             */
            mFilePathToScan = file.getAbsolutePath();
            mConnection.connect();
        }
        return result;
    }

    /**
     * Create a play list with the given default play list name, if no such play
     * list exists.
     *
     * @param res
     *            resource
     * @param resolver
     *            ContentResolver
     * @return the URI of play list that has just be created
     */
    private Uri createPlaylist(Resources res, ContentResolver resolver) {
        ContentValues cv = new ContentValues(1);
        cv.put(MediaStore.Audio.Playlists.NAME, res.getString(R.string.audio_db_playlist_name));
        Uri uri = null;
        /** M: add exception process @{ */
        try {
            uri = resolver.insert(MediaStore.Audio.Playlists.getContentUri(VOLUME_NAME), cv);
        } catch (UnsupportedOperationException e) {
            LogUtils.e(TAG, "<createPlaylist> insert in DB failed: " + e.getMessage());
            uri = null;
        }
        /** @} */

        if (null == uri) {
            /** M: use Handler to show database error dialog */
            mOnErrorListener.onError(ErrorHandle.ERROR_SAVE_FILE_FAILED);
        }
        return uri;
    }

    /**
     * Obtain the id for the default play list from the audio_playlists table.
     *
     * @param res
     *            resource
     * @return get play list id
     */
    private int getPlaylistId(Resources res) {
        Uri uri = MediaStore.Audio.Playlists.getContentUri(VOLUME_NAME);
        final String[] ids = new String[] {
            MediaStore.Audio.Playlists._ID
        };

        final String where = MediaStore.Audio.Playlists.DATA + "=?";
        final String[] args = new String[] { getPlaylistPath(getApplicationContext()) +
                res.getString(R.string.audio_db_playlist_name) };
        Cursor cursor = SoundRecorderUtils.query(getApplicationContext(), uri, ids, where, args,
                null);
        int id = PLAYLIST_ID_NULL;
        /** M: add exception process @{ */
        try {
            if (null != cursor) {
                cursor.moveToFirst();
                if (!cursor.isAfterLast()) {
                    id = cursor.getInt(0);
                }
            }
        } catch (IllegalStateException e) {
            e.printStackTrace();
        } finally {
            if (null != cursor) {
                cursor.close();
            }
        }
        /** @} */
        return id;
    }

    /**
     * Add the given audioId to the play list with the given playlistId; and
     * maintain the play_order in the play list.
     *
     * @param resolver
     *            ContentResolver
     * @param audioId
     *            the audio id that will be added to play list
     * @param playlistId
     *            the play list id which will add to
     */
    private void addToPlaylist(ContentResolver resolver, int audioId, long playlistId) {
        String[] cols = new String[] {
            "count(*)"
        };
        Uri uri = MediaStore.Audio.Playlists.Members.getContentUri(VOLUME_NAME, playlistId);
        Cursor cur = resolver.query(uri, cols, null, null, null);
        if (null == cur) {
            LogUtils.e(TAG, "<addToPlaylist> cursor is null");
            return;
        }
        cur.moveToFirst();
        final int base = cur.getInt(0);
        cur.close();
        ContentValues values = new ContentValues(2);
        values.put(MediaStore.Audio.Playlists.Members.PLAY_ORDER, Integer.valueOf(base + audioId));
        values.put(MediaStore.Audio.Playlists.Members.AUDIO_ID, audioId);
        /** M: add exception process @{ */
        Uri result = null;
        try {
            result = resolver.insert(uri, values);
        } catch (UnsupportedOperationException e) {
            LogUtils.e(TAG, "<addToPlaylist> insert in DB failed: " + e.getMessage());
            result = null;
        }
        if (null == result) {
            mOnErrorListener.onError(ErrorHandle.ERROR_SAVE_FILE_FAILED);
        }
        /** @} */
    }

    private void getRecordInfoAfterStopRecord() {
        mTotalRecordingDuration = mRecorder.getSampleLength();
        mCurrentFileDuration = mRecorder.getSampleLength();
        setCurrentFilePath(mRecorder.getSampleFilePath());
        // M:Add for stop fail case.
        if (!mRecorder.reset()) {
            mOnErrorListener.onError(ErrorHandle.ERROR_RECORDING_FAILED);
        }
    }

    private boolean isCurrentFileEndWithTmp() {
        if (null == mCurrentFilePath) {
            return false;
        }
        return mCurrentFilePath.endsWith(Recorder.SAMPLE_SUFFIX);
    }

    private void receiveBroadcast(Context context, Intent intent) {
        String action = intent.getAction();
        String command = intent.getStringExtra(COMMAND);
        LogUtils.i(TAG, "<onReceive> action = " + action);
        if (Intent.ACTION_MEDIA_EJECT.equals(action)
                || Intent.ACTION_MEDIA_UNMOUNTED.equals(action)) {
            // M: when sd card ejected is the current operation path,
            // call onError
            if (isCurrentAccessStorage(intent.getData())) {
                if ((STATE_RECORDING == mCurrentState) ||
                        (STATE_PAUSE_RECORDING == mCurrentState)) {
                    mOnErrorListener.onError(ErrorHandle.ERROR_SD_UNMOUNTED_ON_RECORD);
                } else {
                    mOnErrorListener.onError(ErrorHandle.ERROR_SD_UNMOUNTED_WHEN_IDLE);
                }
                reset();
            }
        } else if (Intent.ACTION_MEDIA_MOUNTED.equals(action)) {
            // tell activity to remove SD_UNMOUNTED dialog
            if (null != mOnEventListener) {
                mOnEventListener.onEvent(EVENT_STORAGE_MOUNTED);
            }
        } else if (Intent.ACTION_CONFIGURATION_CHANGED.equals(action)) {
            if (STATE_IDLE != mCurrentState) {
                // when change language, update the language of
                // notification
                showNotification(getApplicationContext());
                return;
            }
        } else if (SOUND_POWER_DOWN_MSG.equals(action) && CMDPAUSE.equals(command)) {
            // M: camera begin to record when soundRecorder is
            // background record,
            // it will send SOUND_POWER_DOWN_MSG, we receive and stop
            // record
            // The process is same as audio focus loss
            if ((STATE_RECORDING == mCurrentState) || (STATE_PAUSE_RECORDING == mCurrentState)) {
                stopRecord();
            } else if (STATE_PLAYING == mCurrentState) {
                stopPlay();
            }

            if (isCurrentFileWaitToSave()) {
                saveRecordAsync();
            }
        } else if (Intent.ACTION_SHUTDOWN.equals(action)) {
            // save the recording parameters
            storeRecordParamsSettings();
        }
    }

    private void registerBroadcastReceivcer() {
        if (null == mStorageBroastReceiver) {
            mStorageBroastReceiver = new BroadcastReceiver() {
                @Override
                public void onReceive(Context context, Intent intent) {
                    receiveBroadcast(context, intent);
                }
            };
            IntentFilter iFilter = new IntentFilter();
            iFilter.addAction(Intent.ACTION_MEDIA_EJECT);
            iFilter.addAction(Intent.ACTION_MEDIA_MOUNTED);
            iFilter.addAction(Intent.ACTION_MEDIA_UNMOUNTED);
            iFilter.addDataScheme("file");
            registerReceiver(mStorageBroastReceiver, iFilter);
            LogUtils.i(TAG, "<registerExternalStorageListener> register mStorageBroastReceiver");
        }

        if (null == mOtherBroastReceiver) {
            mOtherBroastReceiver = new BroadcastReceiver() {
                @Override
                public void onReceive(Context context, Intent intent) {
                    receiveBroadcast(context, intent);
                }
            };
            IntentFilter iFilter = new IntentFilter();
            iFilter.addAction(Intent.ACTION_CONFIGURATION_CHANGED);
            iFilter.addAction(SOUND_POWER_DOWN_MSG);
            iFilter.addAction(Intent.ACTION_SHUTDOWN);

            registerReceiver(mOtherBroastReceiver, iFilter);
            LogUtils.i(TAG, "<registerExternalStorageListener> register mOtherBroastReceiver");
        }
    }

    private void unregisterBroadcastReceivcer() {
        if (null != mStorageBroastReceiver) {
            unregisterReceiver(mStorageBroastReceiver);
        }

        if (null != mOtherBroastReceiver) {
            unregisterReceiver(mOtherBroastReceiver);
        }
    }

    private String deleteRecordingFileTmpSuffix() {
        LogUtils.i(TAG, "<deleteRecordingFileTmpSuffix>");
        if (!mCurrentFilePath.endsWith(Recorder.SAMPLE_SUFFIX)) {
            return null;
        }
        File file = new File(mCurrentFilePath);
        if (!file.exists()) {
            LogUtils.i(TAG, "<deleteRecordingFileTmpSuffix> file is not exist.");
            return null;
        }
        String newPath = mCurrentFilePath.substring(0, mCurrentFilePath
                .lastIndexOf(Recorder.SAMPLE_SUFFIX));
        stopWatching();
        File newFile = new File(newPath);
        boolean result = file.renameTo(newFile);
        if (result) {
            return newFile.getAbsolutePath();
        } else {
            return null;
        }
    }

    private void setState(int stateCode) {
        mCurrentState = stateCode;
        if (mOnStateChangedListener != null) {
            mOnStateChangedListener.onStateChanged(stateCode);
        } else {
            LogUtils.i(TAG, "<setState> mOnStateChangedListener = null, mCurrentState = "
                    + mCurrentState);
        }
    }

    private void showNotification(Context context) {
        if (!mShowNotifiaction) {
            hideNotifiaction();
            LogUtils.i(TAG, "<showNotification> mShowNotifiaction == false, return");
            return;
        }
        if (mCurrentState == STATE_IDLE) {
            LogUtils.i(TAG, "<showNotification> not show in STATE_IDLE, return");
            return;
        }
        LogUtils.i(TAG, "<showNotificatoin> create mNotificationView");
        mNotificationView = new RemoteViews(getPackageName(), R.layout.notification);
        mNotificationView.setTextViewText(R.id.app_name, getResources()
                .getString(R.string.app_name));

        Intent intent;
        PendingIntent pIntent;

        intent = new Intent(ACTION_GOON_RECORD);
        intent.setClass(context, SoundRecorderService.class);
        pIntent = PendingIntent.getService(context, 0, intent, 0);
        mNotificationView.setOnClickPendingIntent(R.id.btn_goon_record, pIntent);

        intent = new Intent(ACTION_GOON_PLAY);
        intent.setClass(context, SoundRecorderService.class);
        pIntent = PendingIntent.getService(context, 0, intent, 0);
        mNotificationView.setOnClickPendingIntent(R.id.btn_goon_play, pIntent);

        intent = new Intent(ACTION_STOP);
        intent.setClass(context, SoundRecorderService.class);
        pIntent = PendingIntent.getService(context, 0, intent, 0);
        mNotificationView.setOnClickPendingIntent(R.id.btn_stop, pIntent);

        intent = new Intent(ACTION_PAUSE);
        intent.setClass(context, SoundRecorderService.class);
        pIntent = PendingIntent.getService(context, 0, intent, 0);
        mNotificationView.setOnClickPendingIntent(R.id.btn_pause, pIntent);

        intent = new Intent("com.android.soundrecorder.SoundRecorder");
        pIntent = PendingIntent.getActivity(context, 0, intent, 0);
        mNotificationView.setOnClickPendingIntent(R.id.app_icon, pIntent);

        Notification notification = new Notification();
        notification.contentView = mNotificationView;
        notification.flags |= Notification.FLAG_ONGOING_EVENT;
        notification.icon = R.drawable.notification_ic_small;
        notification.contentIntent = pIntent;

        // before first show notification, setNotification according state
        setNotification();

        LogUtils.i(TAG, "<showNotificatoin> startForeground");
        startForeground(START_NOTIFICATION_ID, notification);
        mRunForeground = true;
    }

    private void hideNotifiaction() {
        LogUtils.i(TAG, "<hideNotifiaction>");
        stopForeground(true);
        mRunForeground = false;
    }

    private void setNotification() {
        LogUtils.i(TAG, "<setNotification>");

        String filePath = getCurrentFilePath();
        if (filePath == null) {
            return;
        }
        String fileName = filePath.substring(filePath.lastIndexOf(File.separator) + 1, filePath
                .length());
        fileName = (fileName.endsWith(Recorder.SAMPLE_SUFFIX)) ? fileName.substring(0, fileName
                .lastIndexOf(Recorder.SAMPLE_SUFFIX)) : fileName;
        mNotificationView.setTextViewText(R.id.file_name, fileName);

        switch (mCurrentState) {
        case STATE_PAUSE_PLAYING:
            mNotificationView.setViewVisibility(R.id.btn_goon_play, View.VISIBLE);
            mNotificationView.setViewVisibility(R.id.btn_goon_record, View.GONE);
            mNotificationView.setViewVisibility(R.id.btn_pause, View.GONE);
            break;
        case STATE_PAUSE_RECORDING:
            mNotificationView.setViewVisibility(R.id.btn_goon_play, View.GONE);
            mNotificationView.setViewVisibility(R.id.btn_goon_record, View.VISIBLE);
            mNotificationView.setViewVisibility(R.id.btn_pause, View.GONE);
            break;
        case STATE_RECORDING:
        case STATE_PLAYING:
            mNotificationView.setViewVisibility(R.id.btn_goon_play, View.GONE);
            mNotificationView.setViewVisibility(R.id.btn_goon_record, View.GONE);
            mNotificationView.setViewVisibility(R.id.btn_pause, View.VISIBLE);
            break;
        default:
            break;
        }
        mNotificationView.setViewVisibility(R.id.btn_stop, View.VISIBLE);
    }

    /**
     * File Observer Class for recording.
     */
    private class RecordingFileObserver extends FileObserver {
        private String mWatchingPath = null;
        // use this to be sure mFileObserverHandler.sendEmptyMessage(0) will be
        // run only once
        private boolean mHasSendMessage = false;

        public RecordingFileObserver(String path) {
            super(path);
            mWatchingPath = path;
        }

        public RecordingFileObserver(String path, int mask) {
            super(path, mask);
            mWatchingPath = path;
        }

        @Override
        public void onEvent(int event, String path) {
            LogUtils.i(TAG, "<RecordingFileObserver.onEvent> event = " + event);
            if (!mHasSendMessage) {
                if ((FileObserver.DELETE_SELF == event) || (FileObserver.ATTRIB == event)
                        || (FileObserver.MOVE_SELF == event)) {
                    LogUtils.i(TAG, "<RecordingFileObserver.onEvent> " + mWatchingPath
                            + " has been deleted/renamed/moved");
                    mFileObserverHandler.sendEmptyMessage(0);
                    mHasSendMessage = true;
                }
            }
        }
    }

    private void setCurrentFilePath(String path) {
        mCurrentFilePath = path;
        if (null != mFileObserver) {
            mFileObserver.stopWatching();
            mFileObserver = null;
            // M: remove message that has not been processed
            mFileObserverHandler.removeMessages(0);
        }
        if (null != mCurrentFilePath) {
            mFileObserver = new RecordingFileObserver(mCurrentFilePath, FileObserver.DELETE_SELF
                    | FileObserver.ATTRIB | FileObserver.MOVE_SELF);
            if (null == mFileObserverHandler) {
                mFileObserverHandler = new Handler(getMainLooper()) {
                    public void handleMessage(android.os.Message msg) {
                        LogUtils.i(TAG, "<mFileObserverHandler handleMessage> reset()");
                        resetAsync();
                    };
                };
            }
            LogUtils.i(TAG, "<setCurrentFilePath> start watching file <" + mCurrentFilePath + ">");
            mFileObserver.startWatching();
        }
    }

    private void stopWatching() {
        if (null != mFileObserver) {
            mFileObserver.stopWatching();
            mFileObserver = null;
        }
    }

    private boolean requestAudioFocus() {
        if (!mGetFocus) {
            int result = mAudioManager.requestAudioFocus(mFocusChangeListener,
                    AudioManager.STREAM_MUSIC, AudioManager.AUDIOFOCUS_GAIN);
            if (result != AudioManager.AUDIOFOCUS_REQUEST_GRANTED) {
                LogUtils.i(TAG, "<requestAudioFocus> request audio focus fail");
                mGetFocus = false;
            } else {
                LogUtils.i(TAG, "<requestAudioFocus> request audio focus success");
                mGetFocus = true;
            }
        }
        return mGetFocus;
    }

    /**
     * M: new private function for abandon audio focus, it will be called when.
     * stop or pause play back
     */
    private void abandonAudioFocus() {
        if (mGetFocus && (null != mAudioManager) && (null != mFocusChangeListener)) {
            if (AudioManager.AUDIOFOCUS_REQUEST_GRANTED == mAudioManager
                    .abandonAudioFocus(mFocusChangeListener)) {
                LogUtils.i(TAG, "<abandonAudioFocus()> abandon audio focus success");
                mGetFocus = false;
            } else {
                LogUtils.e(TAG, "<abandonAudioFocus()> abandon audio focus failed");
                mGetFocus = true;
            }
        }
    }

    /**
     * Check if the given listener is the state changed listener.
     *
     * @param listener the listener to check
     * @return true if match, otherwise return false
     */
    public boolean isListener(OnStateChangedListener listener) {
        if (mOnStateChangedListener == null) {
             return false;
        }
        return mOnStateChangedListener.equals(listener);
    }

    /**
     * M: check if the storage Uri is current operation path.
     * @param storageUri
     *          same as file:///storage/sdcard0
     * @return current operation path return true
     */
    private boolean isCurrentAccessStorage(Uri storageUri) {
        if ((null == mCurrentFilePath) || (null == storageUri)
                || !storageUri.getScheme().endsWith("file")) {
            return false;
        }
        String storagePath = storageUri.getPath();
        String currentOperationPath = mCurrentFilePath.substring(0, storagePath.length());
        return currentOperationPath.equals(storagePath);
    }

    /**
     *  check if current availabel storage is lower than LOW_STORAGE_THRESHOLD.
     *
     * @return if current availabel storage is lower than LOW_STORAGE_THRESHOLD
     */
    public boolean isStorageLower() {
        try {
            String storageState = mStorageManager
                    .getVolumeState(Environment.getExternalStorageDirectory().getAbsolutePath());
            if ((null == storageState)
                    || storageState.equals(Environment.MEDIA_MOUNTED)) {
                String storageDirectory =
                        Environment.getExternalStorageDirectory().getAbsolutePath();
                /**
                 * M:Modified for SD card hot plug-in/out. Should to check the
                 * savePath of the current file rather than default write path.@{
                 */
                if (mCurrentFilePath != null) {
                    int index = mCurrentFilePath.indexOf(RECORDING, 0) - 1;
                    storageDirectory = mCurrentFilePath.substring(0, index);
                }
                /**@}*/
                StatFs stat = null;
                try {
                    stat = new StatFs(storageDirectory);
                } catch (IllegalArgumentException e) {
                    LogUtils.d(TAG, "stat " + storageDirectory + " failed...");
                    return false;
                }
                return ((long) stat.getAvailableBlocks()
                        * (long) stat.getBlockSize()) < LOW_STORAGE_THRESHOLD;
            } else {
                return true;
            }
        } catch (IllegalStateException ex) {
            // if we can't stat the filesystem then we don't know how many
            // free bytes exist. It might be zero but just leave it
            // blank since we really don't know.
            return true;
        }
    }

    /**
     * M: set current record format when user select an item in SelectDialogFragment.
     * @param format to be set
     */
    public void setSelectedFormat(int format) {
        mSelectedFormat = format;
    }

    /**
     * M: set current record mode when user select an item in SelectDialogFragment.
     * @param mode to be set
     */
    public void setSelectedMode(int mode) {
        mSelectedMode = mode;
    }

    /**
     * M: set current record effect that user confirm to select.
     *
     * @param effectArray to be set
     */
    public void setSelectEffectArray(boolean[] effectArray) {
        LogUtils.i(TAG, "<setSelectEffectArray>");
        mSelectEffectArray = effectArray;
    }

    /**
     * M: set current record effect when user select items in
     * SelectDialogFragment.
     *
     * @param tmpEffectArray effectArray to be set
     */
    public void setSelectEffectArrayTmp(boolean[] tmpEffectArray) {
        LogUtils.i(TAG, "<setSelectEffectArrayTmp>");
        mSelectEffectArrayTemp = tmpEffectArray;
    }

    /**
     * M: save the recording parameters.
     */
    public void storeRecordParamsSettings() {
        LogUtils.i(TAG, "<storeRecordParamsSettings> ");
        if (null == mPrefs) {
            mPrefs = getSharedPreferences(SOUND_RECORDER_DATA, 0);
        }
        SharedPreferences.Editor ed = mPrefs.edit();
        ed.clear();
        // If the value is -1, don't save it into preferences
        if (-1 != mSelectedFormat) {
            ed.putInt(SELECTED_RECORDING_FORMAT, mSelectedFormat);
        }
        if (-1 != mSelectedMode) {
            ed.putInt(SELECTED_RECORDING_MODE, mSelectedMode);
        }
        ed.putBoolean(SELECTED_RECORDING_EFFECT_AEC,
                mSelectEffectArray[RecordParamsSetting.EFFECT_AEC]);
        ed.putBoolean(SELECTED_RECORDING_EFFECT_AGC,
                mSelectEffectArray[RecordParamsSetting.EFFECT_AGC]);
        ed.putBoolean(SELECTED_RECORDING_EFFECT_NS,
                mSelectEffectArray[RecordParamsSetting.EFFECT_NS]);
        ed.putBoolean(SELECTED_RECORDING_EFFECT_AEC_TMP,
                mSelectEffectArrayTemp[RecordParamsSetting.EFFECT_AEC]);
        ed.putBoolean(SELECTED_RECORDING_EFFECT_AGC_TMP,
                mSelectEffectArrayTemp[RecordParamsSetting.EFFECT_AGC]);
        ed.putBoolean(SELECTED_RECORDING_EFFECT_NS_TMP,
                mSelectEffectArrayTemp[RecordParamsSetting.EFFECT_NS]);
        ed.commit();
        LogUtils.i(TAG, "mSelectedFormat is:" + mSelectedFormat);
    }

    /**
     * Start the recording with callback.
     *
     * @param recordParams recording parameters
     * @param fileSizeLimit file size limitation
     * @param callback button state callback
     */
    public void startRecordingAsync(
            RecordParams recordParams, int fileSizeLimit, OnUpdateButtonState callback) {
        LogUtils.i(TAG, "<startRecordingAsync>");
        if (mCurrentState == STATE_RECORDING) {
            LogUtils.d(TAG, "already in recording state, not disable button");
            return;
        }
        if (callback != null) {
            callback.updateButtonState(false);
        }
        mParams = recordParams;
        mFileSizeLimit = fileSizeLimit;
        sendThreadHandlerMessage(SoundRecorderServiceHandler.START_REOCRD);
    }

    /**
     * Play the recording with callback.
     *
     * @param callback button state callback
     */
    public void doPlayRecord(OnUpdateButtonState callback) {
        if (STATE_RECORDING == mCurrentState
                || (SoundRecorderService.STATE_PAUSE_RECORDING == mCurrentState)) {
            LogUtils.i(TAG, "<playCurrentFile> in record or pause record state, can't play");
            return;
        } else if (SoundRecorderService.STATE_PAUSE_PLAYING == mCurrentState) {
            if (callback != null) {
                callback.updateButtonState(false);
            }
            goonPlaybackAsync();
        } else if (SoundRecorderService.STATE_PLAYING == mCurrentState) {
            if (callback != null) {
                callback.updateButtonState(false);
            }
            pausePlaybackAsync();
        } else {
            if (callback != null) {
                callback.updateButtonState(false);
            }
            startPlaybackAsync();
        }
    }

    /**
     * Stop the recording with callback.
     *
     * @param callback button state callback
     */
    public void doStop(OnUpdateButtonState callback) {
        if ((SoundRecorderService.STATE_PAUSE_PLAYING == mCurrentState)
                || (SoundRecorderService.STATE_PLAYING == mCurrentState)) {
            LogUtils.i(TAG, "<onClickStopButton> mService.stopPlay()");
            if (callback != null) {
                callback.updateButtonState(false);
            }
            stopPlaybackAsync();
        } else if ((SoundRecorderService.STATE_RECORDING == mCurrentState)
                || (SoundRecorderService.STATE_PAUSE_RECORDING == mCurrentState)) {
            LogUtils.i(TAG, "<onClickStopButton> mService.stopRecord()");
            if (callback != null) {
                callback.updateButtonState(false);
            }
            stopRecordingAsync();
        }
    }

    /**
     * Save the recording with callback.
     *
     * @param callback button state callback
     */
    public void doSaveRecord(OnUpdateButtonState callback) {
        if (mRecordSaving) {
            return;
        }
        if (callback != null) {
            callback.updateButtonState(false);
        }
        if ((SoundRecorderService.STATE_PAUSE_PLAYING == mCurrentState)
                || (SoundRecorderService.STATE_PLAYING == mCurrentState)) {
            LogUtils.i(TAG, "<onClickAcceptButton> mService.stopPlay() first");
            stopPlaybackAsync();
        }
        saveRecordAsync();
    }

    /**
     * Discard the recording with callback.
     *
     * @param callback button state callback
     */
    public void doDiscardRecord(OnUpdateButtonState callback) {
        if (callback != null) {
            callback.updateButtonState(false);
        }
        if ((SoundRecorderService.STATE_PAUSE_PLAYING == mCurrentState)
                || (SoundRecorderService.STATE_PLAYING == mCurrentState)) {
            LogUtils.i(TAG, "<onClickDiscardButton> mService.stopPlay() first");
            stopPlaybackAsync();
        }
        discardRecordAsync();
    }

    /**
     * Pause the recording with callback.
     *
     * @param callback button state callback
     */
    public void doPause(OnUpdateButtonState callback) {
        if (callback != null) {
            callback.updateButtonState(false);
        }
        pauseRecord();
    }

    /**
     * Pause the recording.
     */
    public void pauseRecordingAsync() {
        LogUtils.i(TAG, "<pauseRecordingAsync>");
        sendThreadHandlerMessage(SoundRecorderServiceHandler.PAUSE_REOCRD);
    }

    /**
     * Stop the recording.
     */
    public void stopRecordingAsync() {
        LogUtils.i(TAG, "<stopRecordingAsync>");
        sendThreadHandlerMessage(SoundRecorderServiceHandler.STOP_REOCRD);
    }

    /**
     * Start the playback.
     */
    public void startPlaybackAsync() {
        LogUtils.i(TAG, "<startPlaybackAsync>");
        sendThreadHandlerMessage(SoundRecorderServiceHandler.START_PLAY);
    }

    /**
     * Pause the playback.
     */
    public void pausePlaybackAsync() {
        LogUtils.i(TAG, "<pausePlaybackAsync>");
        sendThreadHandlerMessage(SoundRecorderServiceHandler.PAUSE_PLAY);
    }

    /**
     * Resume the playback.
     */
    public void goonPlaybackAsync() {
        LogUtils.i(TAG, "<goonPlaybackAsync>");
        sendThreadHandlerMessage(SoundRecorderServiceHandler.GOON_PLAY);
    }

    /**
     * Stop the playback.
     */
    public void stopPlaybackAsync() {
        LogUtils.i(TAG, "<stopPlaybackAsync>");
        sendThreadHandlerMessage(SoundRecorderServiceHandler.STOP_PLAY);
    }

    /**
     * Save the recording.
     */
    public void saveRecordAsync() {
        LogUtils.i(TAG, "<saveRecordAsync>");
        sendThreadHandlerMessage(SoundRecorderServiceHandler.SAVE_RECORD);
    }

    /**
     * Discard the recording.
     */
    public void discardRecordAsync() {
        LogUtils.i(TAG, "<discardRecordAsync>");
        sendThreadHandlerMessage(SoundRecorderServiceHandler.DISCARD_RECORD);
    }

    /**
     * Reset the reset the service in an async way.
     */
    public void resetAsync() {
        LogUtils.i(TAG, "<resetAsync>");
        sendThreadHandlerMessage(SoundRecorderServiceHandler.RESET);
    }

    private void sendThreadHandlerMessage(int what) {
        mSoundRecorderServiceHandler.removeCallbacks(mHandlerThread);
        mSoundRecorderServiceHandler.sendEmptyMessage(what);
    }

    /**
     * Sound Recorder Service Handler.
     */
    public class SoundRecorderServiceHandler extends Handler {
        /**
         * Constructor of sound recorder service handler.
         *
         * @param looper looper that the handler runs
         */
        public SoundRecorderServiceHandler(Looper looper) {
            super(looper);
        }

        public static final int START_REOCRD = 0;
        public static final int PAUSE_REOCRD = 1;
        public static final int STOP_REOCRD = 2;
        public static final int START_PLAY = 3;
        public static final int PAUSE_PLAY = 4;
        public static final int GOON_PLAY = 5;
        public static final int STOP_PLAY = 6;
        public static final int SAVE_RECORD = 7;
        public static final int DISCARD_RECORD = 8;
        public static final int RESET = 9;

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case START_REOCRD:
                    record(mParams, mFileSizeLimit);
                    break;
                case PAUSE_REOCRD:
                    pauseRecord();
                    break;
                case STOP_REOCRD:
                    stopRecord();
                    break;
                case START_PLAY:
                    startPlayback();
                    break;
                case PAUSE_PLAY:
                    pausePlay();
                    break;
                case GOON_PLAY:
                    goonPlayback();
                    break;
                case STOP_PLAY:
                    stopPlay();
                    break;
                case SAVE_RECORD:
                    saveRecord();
                    break;
                case DISCARD_RECORD:
                    discardRecord();
                    break;
                case RESET:
                    reset();
                    break;
                default:
                    break;
            }
        }
    }

    private String getPlaylistPath(Context context) {
        File dir = Environment.getExternalStorageDirectory();
        if (null != dir) {
            return dir.getAbsolutePath() + "/Playlists/";
        } else {
            return "";
        }
    }
}
