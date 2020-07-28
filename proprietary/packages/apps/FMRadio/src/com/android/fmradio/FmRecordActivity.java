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

import java.io.File;

import android.Manifest;
import android.app.Activity;
import android.app.FragmentManager;
import android.app.Notification;
import android.app.Notification.Builder;
import android.app.PendingIntent;
import android.content.ComponentName;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.pm.PackageManager;
import android.database.ContentObserver;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.provider.MediaStore;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import com.android.fmradio.FmStation.Station;
import com.android.fmradio.dialogs.FmSaveDialog;
import com.android.fmradio.views.FmVisualizerView;

//import static android.Manifest.permission.READ_MEDIA_AUDIO;

/**
 * This class interact with user, FM recording function.
 */
public class FmRecordActivity extends Activity implements
        FmSaveDialog.OnRecordingDialogClickListener {
    private static final String TAG = "FmRecordActivity";

    private static final String FM_STOP_RECORDING = "fmradio.stop.recording";
    private static final String FM_ENTER_RECORD_SCREEN = "fmradio.enter.record.screen";
    private static final String TAG_SAVE_RECORDINGD = "SaveRecording";
    private static final int MSG_UPDATE_NOTIFICATION = 1000;
    private static final int TIME_BASE = 60;
    private Context mContext;
    private TextView mMintues;
    private TextView mSeconds;
    private TextView mFrequency;
    private View mStationInfoLayout;
    private TextView mStationName;
    private TextView mRadioText;
    private Button mStopRecordButton;
    private FmVisualizerView mPlayIndicator;
    private FmService mService = null;
    private FragmentManager mFragmentManager;
    private boolean mIsInBackground = false;
    private boolean mIsActivityRecreate = false;
    private int mRecordState = FmRecorder.STATE_INVALID;
    private int mCurrentStation = FmUtils.DEFAULT_STATION;
    private Notification.Builder mNotificationBuilder = null;
    private boolean mIsPermissionsRevoked = false;
    private boolean mHaveListener = false; // represents mContentObserver is set or not

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.d(TAG, "onCreate");
        mContext = getApplicationContext();
        mFragmentManager = getFragmentManager();
        setContentView(R.layout.fm_record_activity);

        mMintues = (TextView) findViewById(R.id.minutes);
        mSeconds = (TextView) findViewById(R.id.seconds);

        mFrequency = (TextView) findViewById(R.id.frequency);
        mStationInfoLayout = findViewById(R.id.station_name_rt);
        mStationName = (TextView) findViewById(R.id.station_name);
        mRadioText = (TextView) findViewById(R.id.radio_text);

        mStopRecordButton = (Button) findViewById(R.id.btn_stop_record);
        mStopRecordButton.setEnabled(false);
        mStopRecordButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                // Stop recording and wait service notify stop record state to show dialog
                mService.stopRecordingAsync();
            }
        });

        mPlayIndicator = (FmVisualizerView) findViewById(R.id.fm_play_indicator);

        if (savedInstanceState != null) {
            mCurrentStation = savedInstanceState.getInt(FmStation.CURRENT_STATION);
            mIsActivityRecreate = true;
        } else {
            Intent intent = getIntent();
            mCurrentStation = intent.getIntExtra(FmStation.CURRENT_STATION,
                    FmUtils.DEFAULT_STATION);
            mRecordState = intent.getIntExtra("last_record_state", FmRecorder.STATE_INVALID);
        }
        bindService(new Intent(this, FmService.class), mServiceConnection,
                Context.BIND_AUTO_CREATE);
        updateUi();
    }

    private void updateUi() {
        // TODO it's on UI thread, change to sub thread
        ContentResolver resolver = mContext.getContentResolver();
        mFrequency.setText("FM " + FmUtils.formatStation(mCurrentStation));
        Cursor cursor = null;
        try {
            cursor = resolver.query(
                    Station.CONTENT_URI,
                    FmStation.COLUMNS,
                    Station.FREQUENCY + "=?",
                    new String[] { String.valueOf(mCurrentStation) },
                    null);
            if (cursor != null && cursor.moveToFirst()) {
                // If the station name does not exist, show program service(PS) instead
                String stationName = cursor.getString(cursor.getColumnIndex(Station.STATION_NAME));
                if (TextUtils.isEmpty(stationName)) {
                    stationName = cursor.getString(cursor.getColumnIndex(Station.PROGRAM_SERVICE));
                }
                String radioText = cursor.getString(cursor.getColumnIndex(Station.RADIO_TEXT));
                mStationName.setText(stationName);
                mRadioText.setText(radioText);
                int id = cursor.getInt(cursor.getColumnIndex(Station._ID));
                if (mHaveListener) {
                    mContext.getContentResolver().unregisterContentObserver(mContentObserver);
                    mHaveListener = false;
                }
                resolver.registerContentObserver(
                        ContentUris.withAppendedId(Station.CONTENT_URI, id), false,
                        mContentObserver);
                mHaveListener = true;

                // If no station name and no radio text, hide the view
                if ((!TextUtils.isEmpty(stationName))
                        || (!TextUtils.isEmpty(radioText))) {
                    mStationInfoLayout.setVisibility(View.VISIBLE);
                } else {
                    mStationInfoLayout.setVisibility(View.GONE);
                }
                Log.d(TAG, "updateUi, frequency = " + mCurrentStation + ", stationName = "
                        + stationName + ", radioText = " + radioText);
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
    }


    @Override
    public void onNewIntent(Intent intent) {
        Log.d(TAG, "onNewIntent");
        if (intent != null && intent.getAction() != null) {
            String action = intent.getAction();
            if (FM_STOP_RECORDING.equals(action)) {
                // If click stop button in notification, need to stop recording
                if (mService != null && !isStopRecording()) {
                    mService.stopRecordingAsync();
                }
            } else if (FM_ENTER_RECORD_SCREEN.equals(action)) {
                // Just enter record screen, do nothing
            }
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.d(TAG, "onResume " + mService);
        int recordAudioPermission = checkSelfPermission(Manifest.permission.RECORD_AUDIO);
        int readExtStorage = checkSelfPermission(Manifest.permission.READ_EXTERNAL_STORAGE);
        int writeExtStorage = checkSelfPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE);
        //int readMediaAudio = checkSelfPermission(android.Manifest.permission.READ_MEDIA_AUDIO);
        boolean mRequest = false;
        if (recordAudioPermission != PackageManager.PERMISSION_GRANTED) {
            mIsPermissionsRevoked = true;
        }

        if (readExtStorage != PackageManager.PERMISSION_GRANTED) {
            mIsPermissionsRevoked = true;
        }
        if (writeExtStorage != PackageManager.PERMISSION_GRANTED) {
            mIsPermissionsRevoked = true;
        }
     /* if (readMediaAudio != PackageManager.PERMISSION_GRANTED) {
            mIsPermissionsRevoked = true;
        }
        */
        if (mIsPermissionsRevoked == true) {
            Log.w(TAG, "onResume: Closing activity due to permissions revoked in bg.");
            finish();
            return;
        }
        mIsInBackground = false;
        if (null != mService) {
            mService.setFmRecordActivityForeground(true);

        }

        // Show save dialog if record has stopped and never show it before.
        if (isStopRecording() && !isSaveDialogShown()) {
            showSaveDialog();
        }
        // Trigger to refreshing timer text if still in record
        if (!isStopRecording()) {
            if (mService != null) {
            mHandler.removeMessages(FmListener.MSGID_REFRESH);
                mHandler.sendEmptyMessageDelayed(FmListener.MSGID_REFRESH, 1000);
            }
        }
        // Clear notification, it only need show when in background
        removeNotification();
    }

    @Override
    protected void onPause() {
        Log.d(TAG, "onPause");
        super.onPause();
        mIsInBackground = true;
        if (null != mService) {
            mService.setFmRecordActivityForeground(false);
        }
        // Stop refreshing timer text
        mHandler.removeMessages(FmListener.MSGID_REFRESH);
        // Show notification when switch to background
        showNotification();
    }

    private void showNotification() {
        // If have stopped recording, need not show notification
        if (!isStopRecording()) {
            mHandler.sendEmptyMessage(MSG_UPDATE_NOTIFICATION);
        } else if (isSaveDialogShown()) {
            // Only when save dialog is shown and FM radio is back to background,
            // it is necessary to update playing notification.
            // Otherwise, FmMainActivity will update playing notification.
            if (null != mService) {
                // service has not been binded successfully yet, and this activity
                // is back to background(such as in-call is coming).
            mService.updatePlayingNotification();
        }
    }
    }

    private void removeNotification() {
        mHandler.removeMessages(MSG_UPDATE_NOTIFICATION);
        if (mService != null) {
            mService.removeNotification();
            mService.updatePlayingNotification();
        }
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        outState.putInt(FmStation.CURRENT_STATION, mCurrentStation);
        super.onSaveInstanceState(outState);
    }

    @Override
    protected void onDestroy() {
        Log.d(TAG, "onDestroy");
        removeNotification();
        mHandler.removeCallbacksAndMessages(null);
        if (mService != null) {
            mService.unregisterFmRadioListener(mFmListener);
        }
        unbindService(mServiceConnection);
        mContext.getContentResolver().unregisterContentObserver(mContentObserver);
        super.onDestroy();
    }

    /**
     * Recording dialog click
     *
     * @param recordingName The new recording name
     */
    @Override
    public void onRecordingDialogClick(String recordingName) {
        Log.d(TAG, "onRecordingDialogClick, name = " + recordingName);
        // Happen when activity recreate, such as switch language
        if (mIsInBackground) {
            Log.d(TAG, "onRecordingDialogClick, Activity is in bg");
            return;
        }

        if (recordingName != null && mService != null) {
            mService.saveRecordingAsync(recordingName);
            //returnResult(recordingName, getString(R.string.toast_record_saved));
        } else {
            if (mService != null) {
                // discard recording
                mService.discardRecording();
            }
            returnResult(null, getString(R.string.toast_record_not_saved));
            finish();
        }

    }

    @Override
    public void onBackPressed() {
        Log.d(TAG, "onBackPressed");
        if (mService != null & !isStopRecording()) {
            // Stop recording and wait service notify stop record state to show dialog
            mService.stopRecordingAsync();
            return;
        }
        super.onBackPressed();
    }

    private final ServiceConnection mServiceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, android.os.IBinder service) {
            mService = ((FmService.ServiceBinder) service).getService();
            Log.d(TAG, "onServiceConnected " + mService);
            if (mIsPermissionsRevoked == true) {
                Log.w(TAG, "onServiceConnected: return due to permissions revoked in bg.");
                return;
            }
            mService.registerFmRadioListener(mFmListener);
            mService.setFmRecordActivityForeground(!mIsInBackground);
            // When Activity re-launch, need get latest record state from service.
            if (mIsActivityRecreate) {
                mRecordState = mService.getRecorderState();
            }
            // 1. If have stopped recording, we need check whether need show save dialog again.
            // Because when stop recording in background, we need show it when switch to foreground.
            if (isStopRecording()) {
                if (!isSaveDialogShown()) {
                    showSaveDialog();
                }
                return;
            }
            // 2. If not start recording, start it directly, this case happen when start this
            // activity from main fm activity.
            if (!isStartRecording()) {
                mService.startRecordingAsync();
            }
            mPlayIndicator.startAnimation();
            mStopRecordButton.setEnabled(true);
            mHandler.removeMessages(FmListener.MSGID_REFRESH);
            mHandler.sendEmptyMessage(FmListener.MSGID_REFRESH);
        };

        @Override
        public void onServiceDisconnected(android.content.ComponentName name) {
            mService = null;
        };
    };

    private String addPaddingForString(long time) {
        StringBuilder builder = new StringBuilder();
        if (time >= 0 && time < 10) {
            builder.append("0");
        }
        return builder.append(time).toString();
    }

    private final Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            Log.d(TAG, "handleMessage: msg_what = " + msg.what);
            switch (msg.what) {
                case FmListener.MSGID_REFRESH:
                    if (mService != null) {
                        long recordTimeInMillis = mService.getRecordTime();
                        long recordTimeInSec = recordTimeInMillis / 1000L;
                        mMintues.setText(addPaddingForString(recordTimeInSec / TIME_BASE));
                        mSeconds.setText(addPaddingForString(recordTimeInSec % TIME_BASE));
                        checkStorageSpaceAndStop();
                    }
                    mHandler.sendEmptyMessageDelayed(FmListener.MSGID_REFRESH, 1000);
                    break;

                case MSG_UPDATE_NOTIFICATION:
                    if (mService != null) {
                        mService.updateRecordingNotification(mService.getRecordTime());
                        checkStorageSpaceAndStop();
                    }
                    mHandler.sendEmptyMessageDelayed(MSG_UPDATE_NOTIFICATION, 1000);
                    break;

                case FmListener.LISTEN_RECORDSTATE_CHANGED:
                    // State change from STATE_INVALID to STATE_RECORDING mean begin recording
                    // State change from STATE_RECORDING to STATE_IDLE mean stop recording
                    int newState = mService.getRecorderState();
                    Log.d(TAG, "handleMessage, record state changed: newState = " + newState
                            + ", mRecordState = " + mRecordState);
                    if (mRecordState == FmRecorder.STATE_INVALID
                            && newState == FmRecorder.STATE_RECORDING) {
                        mRecordState = FmRecorder.STATE_RECORDING;
                    } else if (mRecordState == FmRecorder.STATE_RECORDING
                            && newState == FmRecorder.STATE_IDLE) {
                        mRecordState = FmRecorder.STATE_IDLE;
                        mPlayIndicator.stopAnimation();
                        showSaveDialog();
                    } else {
                        String showString = getString(R.string.toast_recorder_internal_error);
                        returnResult(null, showString);
                        Log.w(TAG, "finishing record activity");
                        finish();
                    }
                    break;

                case FmListener.LISTEN_RECORDERROR:
                    Bundle bundle = msg.getData();
                    int errorType = bundle.getInt(FmListener.KEY_RECORDING_ERROR_TYPE);
                    handleRecordError(errorType);
                    break;

                case FmListener.UPDATE_NOTIFICATION:
                    Log.d(TAG, "refreshNotification");
                    if (mIsInBackground == true) {
                        mService.removeNotification();
                        if (mNotificationBuilder != null) {
                            mNotificationBuilder = null;
                        }
                        Log.d(TAG, "removed notification, now update it again");
                        mHandler.sendEmptyMessage(MSG_UPDATE_NOTIFICATION);
                    }
                    break;

                case FmListener.MSGID_SAVERECORDING_FINISHED:
                    Log.d(TAG, "recording finished, show snackbar");
                    bundle = msg.getData();
                    String name = bundle.getString(FmListener.KEY_RECORDING_NAME);
                    Log.d(TAG, "bundle name = " + name);
                    returnResult(name, getString(R.string.toast_record_saved));
                    finish();
                    break;

                default:
                    break;
            }
        };
    };

    private void checkStorageSpaceAndStop() {
        Log.d(TAG, "checkStorageSpaceAndStop");
        long recordTimeInMillis = mService.getRecordTime();
        long recordTimeInSec = recordTimeInMillis / 1000L;
        // Check storage free space
        String recordingSdcard = FmUtils.getDefaultStoragePath();
        if (!FmUtils.hasEnoughSpace(recordingSdcard)) {
            // Need to record more than 1s.
            // Avoid calling MediaRecorder.stop() before native record starts.
            if (recordTimeInSec >= 1) {
                // Insufficient storage
                mService.stopRecordingAsync();
                Toast.makeText(FmRecordActivity.this,
                        R.string.toast_sdcard_insufficient_space,
                        Toast.LENGTH_SHORT).show();
            }
        } else if (mService.getPowerStatus() != FmService.POWER_UP) {
            // Need to record more than 1s.
            // Avoid calling MediaRecorder.stop() before native record starts.
            if (recordTimeInSec >= 1) {
                mService.stopRecordingAsync();
            }
        }
    }

    private void handleRecordError(int errorType) {
        Log.d(TAG, "handleRecordError, errorType = " + errorType);
        String showString = null;
        switch (errorType) {
            case FmRecorder.ERROR_SDCARD_NOT_PRESENT:
                showString = getString(R.string.toast_sdcard_missing);
                returnResult(null, showString);
                finish();
                break;

            case FmRecorder.ERROR_SDCARD_INSUFFICIENT_SPACE:
                showString = getString(R.string.toast_sdcard_insufficient_space);
                returnResult(null, showString);
                finish();
                break;

            case FmRecorder.ERROR_RECORDER_INTERNAL:
                showString = getString(R.string.toast_recorder_internal_error);
                Toast.makeText(mContext, showString, Toast.LENGTH_SHORT).show();
                finish();
                break;

            case FmRecorder.ERROR_SDCARD_WRITE_FAILED:
                showString = getString(R.string.toast_recorder_internal_error);
                returnResult(null, showString);
                finish();
                break;

            default:
                Log.w(TAG, "handleRecordError, invalid record error");
                showString = getString(R.string.toast_recorder_internal_error);
                Toast.makeText(mContext, showString, Toast.LENGTH_SHORT).show();
                finish();
                break;
        }
    }

    // transform the uri from "file://" type to "content://" type
    private Uri transcodeSchemeFileUri(Uri uri) {
        Uri newUri = null;
        Cursor cursor = null;
        String videoPath = uri.getPath();
        Log.d(TAG, "transcodeSchemeFileUri, uri" + uri);
        // convert single quote to double to handle in sql query
        videoPath = videoPath.replaceAll("'", "''");
        Log.d(TAG, "transcodeSchemeFileUri, audiopath " + videoPath);
        try {
            cursor = mContext.getContentResolver()
                  .query(MediaStore.Audio.Media.EXTERNAL_CONTENT_URI,
                  new String[] { MediaStore.Audio.Media._ID },
                  MediaStore.Audio.Media.DATA + " = '"
                  + videoPath + "'", null, null);
            if (cursor != null && cursor.moveToFirst()
                    && cursor.getCount() > 0) {
                int id = cursor.getInt(0);
                newUri = ContentUris.withAppendedId(
                    MediaStore.Audio.Media.EXTERNAL_CONTENT_URI, id);
                Log.d(TAG, "transcodeSchemeFileUri, newUri = " + newUri);
            } else {
                newUri = uri;
                Log.d(TAG, "transcodeSchemeFileUri, The uri not existed in audio table");
            }
            return newUri;
        } finally {
            if (cursor != null) {
                cursor.close();
                cursor = null;
            }
        }
    }

    private void returnResult(String recordName, String resultString) {
        Log.d(TAG, "returnResult, recordName = " + recordName);
        Log.d(TAG, "returnResult, resultString = " + resultString);

        Intent intent = new Intent();
        intent.putExtra(FmMainActivity.EXTRA_RESULT_STRING, resultString);
        if (recordName != null) {
            Uri contentUri =
                    transcodeSchemeFileUri(Uri.parse("file://" + FmService.getRecordingSdcard()
                    + File.separator + FmRecorder.FM_RECORD_FOLDER + File.separator
                    + Uri.encode(recordName) + FmRecorder.RECORDING_FILE_EXTENSION));
            intent.setData(contentUri);
        }
        setResult(RESULT_OK, intent);
    }

    private final ContentObserver mContentObserver = new ContentObserver(new Handler()) {
        public void onChange(boolean selfChange) {
            updateUi();
        };
    };

    // Service listener
    private final FmListener mFmListener = new FmListener() {
        @Override
        public void onCallBack(Bundle bundle) {
            int flag = bundle.getInt(FmListener.CALLBACK_FLAG);
            if (flag == FmListener.MSGID_FM_EXIT) {
                mHandler.removeCallbacksAndMessages(null);
                finish();
            }

            // remove tag message first, avoid too many same messages in queue.
            Message msg = mHandler.obtainMessage(flag);
            msg.setData(bundle);
            mHandler.removeMessages(flag);
            mHandler.sendMessage(msg);
        }
    };

    /**
     * Show save record dialog
     */
    public void showSaveDialog() {
        Log.d(TAG, "showSaveDialog");
        removeNotification();
        if (mIsInBackground) {
            Log.d(TAG, "showSaveDialog, activity is in background, show it later");
            return;
        }
        String sdcard = FmService.getRecordingSdcard();
        String recordingName = mService.getRecordingName();
        String saveName = null;
        if (TextUtils.isEmpty(mStationName.getText())) {
            saveName = FmRecorder.RECORDING_FILE_PREFIX +  "_" + recordingName;
        } else {
            saveName = FmRecorder.RECORDING_FILE_PREFIX + "_" + mStationName.getText() + "_"
                    + recordingName;
        }
        FmSaveDialog newFragment = new FmSaveDialog(sdcard, recordingName, saveName);
        newFragment.show(mFragmentManager, TAG_SAVE_RECORDINGD);
        mFragmentManager.executePendingTransactions();
        mHandler.removeMessages(FmListener.MSGID_REFRESH);
    }

    private boolean isStartRecording() {
        return mRecordState == FmRecorder.STATE_RECORDING;
    }

    private boolean isStopRecording() {
        return mRecordState == FmRecorder.STATE_IDLE;
    }

    private boolean isSaveDialogShown() {
        FmSaveDialog saveDialog = (FmSaveDialog)
                mFragmentManager.findFragmentByTag(TAG_SAVE_RECORDINGD);
        return saveDialog != null;
    }
}
