/*
 * Copyright (C) 2008 The Android Open Source Project
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
 * limitations under the License.=;acikmnprstvwy-
 */

package com.mediatek.voicewakeup;

import android.app.AlertDialog;
import android.app.Dialog;
import android.app.StatusBarManager;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.media.AudioManager;
import android.media.AudioSystem;
import android.media.SoundPool;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.PowerManager;
import android.preference.PreferenceActivity;
import android.provider.Settings;
import android.telephony.TelephonyManager;
import android.text.style.ForegroundColorSpan;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.ProgressBar;
import android.widget.TextView;

import com.mediatek.common.voicecommand.VoiceCommandListener;
import com.mediatek.provider.MtkSettingsExt;
import com.mediatek.voiceunlock.R;
import com.mediatek.voiceunlock.SettingsPreferenceFragment;
import com.mediatek.voicewakeup.VowNoSpeaker.VowNoSpeakerFragment;

import java.text.NumberFormat;

public class VowCommandRecord extends PreferenceActivity {

    @Override
    public Intent getIntent() {
        Intent modIntent = new Intent(super.getIntent());
        modIntent.putExtra(EXTRA_SHOW_FRAGMENT, VowCommandRecordFragment.class.getName());
        modIntent.putExtra(EXTRA_NO_HEADERS, true);
        return modIntent;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        CharSequence msg = getText(R.string.voice_unlock_setup_intro_header);
        showBreadCrumbs(msg, msg);
    }

    @Override
    protected boolean isValidFragment(String fragmentName) {
        return (VowCommandRecordFragment.class.getName().equals(fragmentName));
    }

    public static class VowCommandRecordFragment extends SettingsPreferenceFragment implements
            View.OnClickListener, Utils.VoiceServiceListener {

        private static final String TAG = "VowCommandRecord";

        private static final int RED_PROGRESS_THRESHOLD = 40;
        private static final int YELLOW_PROGRESS_THRESHOLD = 100;

        private static final int MAX_PROGRESS = 100;

        private static final int START_ERROR_DIALOG_ID = 0;
        private static final int PLAY_BACK_DIALOG_ID = 1;
        private static final int TIMEOUT_DIALOG_ID = 2;
        private static final int DURING_CALL_DIALOG_ID = 3;

        private static final int TRAINING_RESULT_ENOUGH = 0;
        private static final int TRAINING_RESULT_NOT_ENOUGH = 1;
        private static final int TRAINING_RESULT_NOISY = 2;
        private static final int TRAINING_RESULT_WEAK = 3;
        private static final int TRAINING_RESULT_DIFF = 4;
        private static final int TRAINING_RESULT_EXIST = 5;
        private static final int TRAINING_RESULT_TIMEOUT = 6;
        private static final int TRAINING_RESULT_DIFF_CUSTOMER = 11;
        private static final int TRAINING_RESULT_HEADSET_SWAP = 100;

        private static final int MSG_START_TRAINING = 0;
        private static final int MSG_UPDATE_INTENSITY = 1;
        private static final int MSG_UPDATE_NOTIFY = 2;
        private static final int MSG_SERVICE_ERROR = 3;
        private static final int MSG_FINISH_NOTIFY = 4;

        private static final int MSG_PLAY_INDICATION = 0;
        private static final int INTENSITY_ANIMATION_INTERVAL = 90;
        private static final int RECORD_INTERVAL = 400;

        private SoundPool mSounds;
        private int mSoundId;
        private int mSoundStreamId;
        private AudioManager mAudioManager;
        private int mUiSoundsStreamType;

        private StatusBarManager mStatusBarManager;

        private ImageView mIntro;
        private TextView mIntroText;
        private View mLine;
        private TextView mCommandDescription;
        private ImageView mMic;
        private ImageView mWave;
        private TextView mProgressText;
        private ProgressBar mProgressBar;
        private TextView mPrompt;
        private int mPromptDefaltColor;
        private Button mFooterLeftButton;
        private Button mFooterRightButton;

        private ForegroundColorSpan mColorSpan;

        private Stage mUiStage = Stage.Introduction;
        private int mProgress = 0;

        private int mCommandId;
        private String mCommandValue;
        private int mRecordType;
        private int mVowMode;
        private String mCommandKeyword;
        private String[] mCmdKeywordArray;
        private NumberFormat mProgressPercentFormat;

        private String mErrorMsg;

        private boolean mTraining; // training is ongoing
        private boolean mCanceled; // training is canceled
        private boolean mBindToService;

        private int mCurOrientation;

        private PowerManager mPM;
        private PowerManager.WakeLock mWakeLock;
        private Context mContext;
        private String mPkgName;
        private Utils mUtils;

        private static int mPreStatus = 0;

        private static boolean mStopCaptureVoice = true;

        private Runnable mIntensityRunnable;
        private Handler mHandler = new Handler() {
            @Override
            public void handleMessage(Message msg) {
                switch (msg.what) {
                case MSG_START_TRAINING:
                    mBindToService = true;
                    playIndication(true);
                    break;
                case MSG_UPDATE_INTENSITY:
                    updateIntensity(msg.arg1);
                    break;
                case MSG_UPDATE_NOTIFY:
                    handleUpdateNotify(msg.arg1, msg.arg2);
                    break;
                case MSG_FINISH_NOTIFY:
                    setStartRecognition(msg.arg1);
                    break;
                case MSG_SERVICE_ERROR:
                    mErrorMsg = (String) msg.obj;
                    showDialog(START_ERROR_DIALOG_ID);
                    break;
                default:
                    break;
                }
            }
        };
        private Handler mIntensityHandler = new Handler();
        private Handler mIndicationHandler = new Handler() {
            @Override
            public void handleMessage(Message msg) {
                switch (msg.what) {
                case MSG_PLAY_INDICATION:
                    playIndication(msg.arg1 == 1);
                    break;

                default:
                    break;
                }
            }
        };

        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);

            mContext = getActivity().getBaseContext();
            mUtils = Utils.getInstance();

            Intent intent = getActivity().getIntent();
            mCommandId = intent.getIntExtra(Utils.KEY_COMMAND_ID, -1);
            mCommandValue = intent.getStringExtra(Utils.KEY_COMMAND_VALUE);
            mRecordType = intent.getIntExtra(Utils.KEY_COMMAND_TYPE, -1);
            mVowMode = intent.getIntExtra(Utils.KEY_COMMAND_MODE, -1);
            Log.e("@M_" + TAG, "mCommandKey = " + mCommandId + ", mRecordType = " + mRecordType
                    + ", mVowMode = " + mVowMode);
            if (mVowMode == Utils.VOW_NO_SPEAKER_MODE) { // no speaker id mode
                mCommandKeyword = intent
                        .getStringExtra(VowNoSpeakerFragment.KEY_COMMAND_KEYWORD);
            }
            if (mCommandId == -1 || mRecordType == -1 || mVowMode == -1) {
                getActivity().finish();
                return;
            }
            mStatusBarManager = (StatusBarManager) getSystemService(Context.STATUS_BAR_SERVICE);

            mAudioManager = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
            mSounds = new SoundPool(1, AudioManager.STREAM_SYSTEM, 0);
            mSoundId = mSounds.load(getActivity(), R.raw.dock, 0);

            mPM = (PowerManager) getSystemService(Context.POWER_SERVICE);
            mWakeLock = mPM.newWakeLock(PowerManager.FULL_WAKE_LOCK
                    | PowerManager.ACQUIRE_CAUSES_WAKEUP, "VoiceUnlock");
            mWakeLock.setReferenceCounted(false);

            mCurOrientation = getResources().getConfiguration().orientation;
            mPkgName = mContext.getPackageName();

            mProgressPercentFormat = NumberFormat.getPercentInstance();
            mProgressPercentFormat.setMaximumFractionDigits(0);
            mColorSpan = new ForegroundColorSpan(getResources().getColor(
                    android.R.color.holo_blue_light));
            mIntensityRunnable = new Runnable() {
                @Override
                public void run() {
                    if (mUtils.mVCmdMgrService != null) {
                        Log.d("@M_" + TAG, "sendCommand TRAINING_INTENSITY");
                        mUtils.sendVoiceCommand(mPkgName,
                                VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING,
                                VoiceCommandListener.ACTION_VOICE_TRAINING_INTENSITY, null);
                    }
                    mIntensityHandler.postDelayed(this, INTENSITY_ANIMATION_INTERVAL);
                }
            };
            Log.d("@M_" + TAG, "onCreate mCommandKey = " + mCommandId + " mCommandValue = "
                    + mCommandValue);
        }

        @Override
        public void onPause() {
            Log.d("@M_" + TAG, "onPause()  " + " mTraining = " + mTraining + " mCanceled = " + mCanceled);

            stopVoiceCommandService();

            if (mTraining && !mCanceled) {
                updateStage(Stage.Introduction);
            }
            stopUpdateIntensity();
            mIndicationHandler.removeMessages(MSG_PLAY_INDICATION);

            mUtils.setOnChangedListener(null);
            super.onPause();
        }

        @Override
        public void onResume() {
            super.onResume();
            Log.d("@M_" + TAG, "onResume()");
            mUtils.setOnChangedListener(this);
        }

        @Override
        public View onCreateView(LayoutInflater inflater, ViewGroup container,
                Bundle savedInstanceState) {
            Log.d("@M_" + TAG, "onCreateView");
            View view = inflater.inflate(R.layout.voice_command_recordx, null);
            mIntro = (ImageView) view.findViewById(R.id.image_intro);
            mIntroText = (TextView) view.findViewById(R.id.text_intro);
            mLine = view.findViewById(R.id.line_view);
            mCommandDescription = (TextView) view.findViewById(R.id.command_description);
            mMic = (ImageView) view.findViewById(R.id.mic);
            mWave = (ImageView) view.findViewById(R.id.wave);
            mWave.setImageResource(com.mediatek.internal.R.drawable.voice_wave);
            mProgressText = (TextView) view.findViewById(R.id.progress_text);
            mProgressBar = (ProgressBar) view.findViewById(R.id.progress_bar);
            mPrompt = (TextView) view.findViewById(R.id.prompt);
            mPromptDefaltColor = mPrompt.getCurrentTextColor();
            mFooterLeftButton = (Button) view.findViewById(R.id.footer_left_button);
            mFooterLeftButton.setOnClickListener(this);
            mFooterLeftButton.setSoundEffectsEnabled(false);
            mFooterRightButton = (Button) view.findViewById(R.id.footer_right_button);
            mFooterRightButton.setOnClickListener(this);
            mFooterRightButton.setSoundEffectsEnabled(false);

            mUtils.setContext(mContext);

            ComponentName cn = ComponentName.unflattenFromString(mCommandValue);
            mCommandDescription.setText(getActivity().getString(
                    R.string.voice_command_record_description_command, mUtils.getAppLabel(cn)));

            updateStage(mUiStage);
            setTrainingProgress(mProgress);
            return view;
        }

        private void updateIntroductionUi(int status) {
            mIntro.setVisibility(status);
            mIntroText.setVisibility(status);
        }

        private void updateTranningUi(int status) {
            mLine.setVisibility(status);
            mCommandDescription.setVisibility(status);
            mMic.setVisibility(status);
            mWave.setVisibility(status);
            mProgressText.setVisibility(status);
            mProgressBar.setVisibility(status);
            mPrompt.setVisibility(status);
        }

        @Override
        public Dialog onCreateDialog(int dialogId) {
            switch (dialogId) {
            case START_ERROR_DIALOG_ID:
                return new AlertDialog.Builder(getActivity())
                        .setTitle(R.string.voice_service_error_title).setCancelable(false)
                        .setMessage(mErrorMsg).setPositiveButton(R.string.voice_unlock_ok_label,
                                new DialogInterface.OnClickListener() {
                                    public void onClick(DialogInterface dialog, int whichButton) {
                                        getActivity().finish();
                                    }
                                }).create();
            case PLAY_BACK_DIALOG_ID:
                return new AlertDialog.Builder(getActivity())
                        .setTitle(R.string.stop_playing_title)
                        .setMessage(R.string.stop_playing_message).setPositiveButton(
                                R.string.voice_unlock_ok_label, null).create();
            case TIMEOUT_DIALOG_ID:
                return new AlertDialog.Builder(getActivity())
                        .setTitle(R.string.time_out_title)
                        .setCancelable(false).setMessage(R.string.time_out_message)
                        .setPositiveButton(R.string.voice_unlock_ok_label,
                                new AlertDialog.OnClickListener() {
                                    public void onClick(DialogInterface arg0, int arg1) {
                                        updateStage(Stage.Introduction);
                                    }
                                }).create();
            case DURING_CALL_DIALOG_ID:
                return new AlertDialog.Builder(getActivity())
                        .setTitle(R.string.during_call_title)
                        .setMessage(R.string.during_call_message).setPositiveButton(
                                R.string.voice_unlock_ok_label, null).create();
            default:
                return super.onCreateDialog(dialogId);
            }
        }

        private void handleUpdateNotify(int resultId, int progress) {
            mFooterRightButton.setEnabled(true);
            Log.d("@M_" + TAG,"handleUpdateNotify , resultId : " + resultId + ",progress: " + progress + ",mProgress: " + mProgress);
            switch (resultId) {
            case TRAINING_RESULT_ENOUGH:
                updateStage(Stage.RecordingOK);
                break;
            case TRAINING_RESULT_NOT_ENOUGH:
                if (mProgress == 0 && progress != 0) {
                    updateStage(Stage.Retry);
                } else {
                    updateStage(Stage.OneRoundOK);
                }
                // oneRecordOk(); // no need to show "Got it!" message
                break;
            case TRAINING_RESULT_NOISY:
                updateStage(Stage.OneRoundNoisy);
                break;
            case TRAINING_RESULT_WEAK:
                updateStage(Stage.OneRoundWeak);
                break;
            case TRAINING_RESULT_DIFF:
                updateStage(Stage.OneRoundDiff);
                break;
            case TRAINING_RESULT_EXIST:
                updateStage(Stage.OneRoundExist);
                break;
            case TRAINING_RESULT_TIMEOUT:
                updateStage(Stage.OneRoundTimeout);
                break;
            case TRAINING_RESULT_HEADSET_SWAP:
                getActivity().finish();
                break;
            case TRAINING_RESULT_DIFF_CUSTOMER:
                updateStage(Stage.OneRoundDiffCustom);
                break;
            default:
                break;
            }
            setTrainingProgress(progress);
        }

        private void oneRecordOk() {
            mIndicationHandler.removeMessages(MSG_PLAY_INDICATION);
            mIndicationHandler.sendMessageDelayed(mIndicationHandler.obtainMessage(
                    MSG_PLAY_INDICATION, 0, 0), RECORD_INTERVAL);
            stopUpdateIntensity();
        }

        private void updateIntensity(int intensity) {
            Log.d("@M_" + TAG, "updateIntensity intensity = " + intensity);
            intensity -= 200; // we don't want voice wave too sensitive
            if (intensity < 128) {
                Log.d("@M_" + TAG, "updateIntensity 0");
                mWave.setImageLevel(0);
            } else if (intensity < 256) {
                Log.d("@M_" + TAG, "updateIntensity 1");
                mWave.setImageLevel(1);
            } else if (intensity < 512) {
                Log.d("@M_" + TAG, "updateIntensity 2");
                mWave.setImageLevel(2);
            } else if (intensity < 1024) {
                Log.d("@M_" + TAG, "updateIntensity 3");
                mWave.setImageLevel(3);
            } else if (intensity < 2048) {
                Log.d("@M_" + TAG, "updateIntensity 4");
                mWave.setImageLevel(4);
            }
        }

        private int getAvailableCommand() {
            int cmdSet = 0;
            for (int i = mUtils.mKeyCommandInfoMap.size() - 1; i >= 0; i--) {
                Log.d("@M_" + TAG, "updateCommandStatus key: " + i);
                if (MtkSettingsExt.System.getVoiceCommandValue(mContext.getContentResolver(),
                        MtkSettingsExt.System.BASE_VOICE_WAKEUP_COMMAND_KEY,
                        mUtils.mKeyCommandInfoMap.get(i).mId) != null) {
                    cmdSet = cmdSet | (0x01 << i);
                }
            }
            return cmdSet;
        }

        // start a record
        private void playIndication(final boolean first) {
            Log.d("@M_" + TAG, "playIndication first = " + first + " mBindToService = " + mBindToService);
            if (mBindToService) { // To fix ALPS00441146, don't post any
                // runnable after we unbind from service.
                if (first) {
                    updateStage(Stage.FirstRecording);
                }
                if (mStopCaptureVoice) {
                    Log.d("@M_" + TAG,"mStopCaptureVoice = true,playIndication return");
                    return;
                }
                Log.d("@M_" + TAG, "start to update Intensity");
                mIntensityHandler.postDelayed(mIntensityRunnable, 1200);
                playSound();
            }
        }

        // stop a record
        private void stopUpdateIntensity() {
            mIntensityHandler.removeCallbacks(mIntensityRunnable);
            mWave.setImageLevel(0);
            mMic.setImageResource(R.drawable.ic_voice_unlock_microphone);
        }

        private void playSound() {
            mSounds.stop(mSoundStreamId);

            if (mAudioManager != null) {
                mUiSoundsStreamType = mAudioManager.getUiSoundsStreamType();
            }
            // If the stream is muted, don't play the sound
            if (mAudioManager.isStreamMute(mUiSoundsStreamType)) {
                return;
            }

            mSoundStreamId = mSounds
                    .play(mSoundId, 1, 1, 1/* priortiy */, 0/* loop */, 1.0f/* rate */);
        }

        // check Audio playing
        private boolean checkPlayback() {
            boolean isPlaying = AudioSystem.isStreamActive(AudioSystem.STREAM_MUSIC, 0);
            if (isPlaying) {
                showDialog(PLAY_BACK_DIALOG_ID);
            }
            return isPlaying;
        }

        // check in phone call
        private boolean phoneIsInUse() {
            boolean phoneInUse = true;
            TelephonyManager telephonyManager = (TelephonyManager) getSystemService(Context.TELEPHONY_SERVICE);
            if (telephonyManager != null) {
                if (telephonyManager.getCallState() == TelephonyManager.CALL_STATE_IDLE) {
                    phoneInUse = false;
                }
            }
            if (phoneInUse) {
                showDialog(DURING_CALL_DIALOG_ID);
            }
            return phoneInUse;
        }

        private void disableNotification(boolean disabled) {
            Log.d("@M_" + TAG, "disableNotification disabled = " + disabled);
            int flags = StatusBarManager.DISABLE_NONE;
            if (disabled) {
                flags |= StatusBarManager.DISABLE_NOTIFICATION_ALERTS;
            }
            mStatusBarManager.disable(flags);
        }

        private void stopVoiceCommandService() {
            mUtils.onPause();
            mBindToService = false;
            disableNotification(false);
            mWakeLock.release();
            // restore the status only get the status is not 2
            int status = Settings.System.getInt(getContentResolver(),
                    MtkSettingsExt.System.VOICE_WAKEUP_COMMAND_STATUS, 0);
            if (status != 2) {
                Settings.System.putInt(getContentResolver(),
                        MtkSettingsExt.System.VOICE_WAKEUP_COMMAND_STATUS, mPreStatus);
            }
        }

        private void startVoiceCommandService() {
            // firstly , set the command status to 1 if current is 2
            mPreStatus = Settings.System.getInt(getContentResolver(),
                    MtkSettingsExt.System.VOICE_WAKEUP_COMMAND_STATUS, 0);
            if (mPreStatus == 2) {
                Settings.System.putInt(getContentResolver(),
                        MtkSettingsExt.System.VOICE_WAKEUP_COMMAND_STATUS, 1);
            }
            mUtils.onResume();
        }

        private void voiceTrainingStart() {
            if (mRecordType == Utils.COMMAND_TYPE_MODIFY) {
                Bundle modifyExtra = new Bundle();
                modifyExtra.putInt(VoiceCommandListener.ACTION_EXTRA_SEND_INFO, mCommandId);
                modifyExtra.putInt(VoiceCommandListener.ACTION_EXTRA_SEND_INFO1, mVowMode);
                mUtils.sendVoiceCommand(mPkgName, VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING,
                        VoiceCommandListener.ACTION_VOICE_TRAINING_MODIFY, modifyExtra);
            }
            Bundle extra = new Bundle();
            int availableCmd = getAvailableCommand();
            int[] cmds = { availableCmd, mUtils.mKeyCommandInfoMap.size() };
            Log.d("@M_" + TAG, "sendCommand TRAINING_START commandId = " + mCommandId + " availableCmd = "
                    + availableCmd + " COMMAND_COUNT = " + mUtils.mKeyCommandInfoMap.size());
            // Info : mode, Info1: mCommandKey, Info2: cmds
            extra.putInt(VoiceCommandListener.ACTION_EXTRA_SEND_INFO, mCommandId);
            extra.putInt(VoiceCommandListener.ACTION_EXTRA_SEND_INFO1, mVowMode);
            extra.putIntArray(VoiceCommandListener.ACTION_EXTRA_SEND_INFO2, cmds);
            mUtils.sendVoiceCommand(mPkgName, VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING,
                    VoiceCommandListener.ACTION_VOICE_TRAINING_START, extra);

            disableNotification(true);
            mWakeLock.acquire();
        }

        public void onClick(View v) {
            if (v == mFooterLeftButton) {
                if (mUiStage.leftMode.equals(LeftButtonMode.Cancel)) {
                    // Canceling, so finish all
                    mCanceled = true;
                    getActivity().finish();
                } else if (mUiStage.leftMode.equals(LeftButtonMode.Retry)){
                    Log.d("@M_" + TAG, "click on left button , retry, enter Prepare stage");
                    //TODO,confirm
                    stopVoiceCommandService();
                    setTrainingProgress(0);
                    updateStage(Stage.Prepare);
                    mIntensityHandler.postDelayed(mIntensityRunnable, 1200);
                    mMic.setImageResource(R.drawable.ic_vow_active_microphone);
                }

            } else if (v == mFooterRightButton) {
                Log.d("@M_" + TAG, "click on right button, disable it");
                mFooterRightButton.setEnabled(false);
                if (mUiStage.rightMode.equals(RightButtonMode.Record_Done)) {
                    Log.d("@M_" + TAG, "right , done ");
                    mUtils.sendVoiceCommand(mPkgName,
                            VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING,
                            VoiceCommandListener.ACTION_VOICE_TRAINING_FINISH, null);

                    if (mRecordType == Utils.COMMAND_TYPE_RECORD) { // record
                        MtkSettingsExt.System.setVoiceCommandValue(getContentResolver(),
                                MtkSettingsExt.System.BASE_VOICE_WAKEUP_COMMAND_KEY, mCommandId,
                                mCommandValue);
                    }
                } else if ( mUiStage.rightMode.equals(RightButtonMode.Introduction_Continue)) {
                    Log.d("@M_" + TAG, "right , Introduction_Continue ");
                    if (!phoneIsInUse() && !checkPlayback()) {
                            updateStage(Stage.Prepare);
                        } else {
                            // ALPS02381316
                            mFooterRightButton.setEnabled(true);
                        }
                    mIntensityHandler.postDelayed(mIntensityRunnable, 1200);
                    mMic.setImageResource(R.drawable.ic_vow_active_microphone);
                } else if (mUiStage.rightMode.equals(RightButtonMode.Record_ContinueEnable)) { //retry stage
                    Log.d("@M_" + TAG, "right , Record_Continue ");
                    updateStage(Stage.NonFirstRecording);
                    mUtils.onContinue();
                    mIntensityHandler.postDelayed(mIntensityRunnable, 1200);
                    mMic.setImageResource(R.drawable.ic_vow_active_microphone);
                }
            }
        }

        /**
         * After received training finish, call flow for startRecognition.
         *
         * @param snr
         *            Quality of recorded training
         */
        private void setStartRecognition(int snr) {
            if (Settings.System.getInt(getContentResolver(),
                    MtkSettingsExt.System.VOICE_WAKEUP_COMMAND_STATUS, 0) !=
                        VoiceCommandListener.VOICE_WAKEUP_STATUS_COMMAND_CHECKED) {
                // first record command should set command status to 2
                Settings.System.putInt(getContentResolver(),
                        MtkSettingsExt.System.VOICE_WAKEUP_COMMAND_STATUS,
                        VoiceCommandListener.VOICE_WAKEUP_STATUS_COMMAND_CHECKED);
            }
            if (mVowMode == Utils.VOW_NO_SPEAKER_MODE) {
                VowNoSpeakerFragment.setNeedUpdate(mCommandId);
            }
            mTraining = false;
            Intent intent = new Intent();
            intent.setClassName("com.mediatek.voiceunlock",
                    "com.mediatek.voiceunlock.VoiceUnlockSetupEnd");
            startActivity(intent);
            getActivity().finish();
        }

        private void setTrainingProgress(int progress) {
            mProgress = progress;
            float p = ((float) progress) / MAX_PROGRESS;
            mProgressText.setText(mProgressPercentFormat.format(p));

            if (progress < RED_PROGRESS_THRESHOLD) {
                mProgressBar.setProgressDrawable(getResources().getDrawable(
                        R.drawable.voice_training_progress_red));
            } else if (progress >= RED_PROGRESS_THRESHOLD && progress < YELLOW_PROGRESS_THRESHOLD) {
                mProgressBar.setProgressDrawable(getResources().getDrawable(
                        R.drawable.voice_training_progress_yellow));
            } else {
                mProgressBar.setProgressDrawable(getResources().getDrawable(
                        R.drawable.voice_training_progress_green));
            }
            mProgressBar.setProgress(progress);
        }

        private void updateStage(Stage stage) {
            Log.d("@M_" + TAG, "updateStage stage = " + stage.toString());
            mUiStage = stage;
            if (mUiStage == Stage.Introduction) {
                updateIntroductionUi(View.VISIBLE);
                updateTranningUi(View.GONE);
            } else {
                updateIntroductionUi(View.GONE);
                updateTranningUi(View.VISIBLE);
            }
            mFooterLeftButton.setEnabled(stage.leftMode.enabled);
            mFooterLeftButton.setText(stage.leftMode.text);
            mFooterRightButton.setEnabled(stage.rightMode.enabled);
            mFooterRightButton.setText(stage.rightMode.text);
            switch (mUiStage) {
            case Introduction:
                mPrompt.setTextColor(mPromptDefaltColor);
                setTrainingProgress(0);
                stopUpdateIntensity();
                break;
            case Prepare:
                mPrompt.setTextColor(mPromptDefaltColor);
                startVoiceCommandService();
                break;

            case FirstRecording:
            case NonFirstRecording:
                mTraining = true;
                mPrompt.setTextColor(mPromptDefaltColor);
                break;
            case RecordingOK:
                mIndicationHandler.removeMessages(MSG_PLAY_INDICATION);
                mPrompt.setTextColor(mPromptDefaltColor);
                stopUpdateIntensity();
                break;
            case OneRoundExist:
                mPrompt.setTextColor(getResources().getColor(android.R.color.holo_red_dark));
                mTraining = false;
                stopUpdateIntensity();
                break;
            case OneRoundOK:
            case Retry:
                mPrompt.setTextColor(getResources().getColor(android.R.color.holo_blue_dark));
                mIndicationHandler.removeMessages(MSG_PLAY_INDICATION);
                mIndicationHandler.sendMessageDelayed(mIndicationHandler.obtainMessage(
                        MSG_PLAY_INDICATION, 0, 0), RECORD_INTERVAL);
                stopUpdateIntensity();
                break;
            case OneRoundNoisy:
            case OneRoundWeak:
            case OneRoundDiff:
            case OneRoundTimeout: // New flow introduced.
            case OneRoundDiffCustom:
                mPrompt.setTextColor(getResources().getColor(android.R.color.holo_red_dark));
                mIndicationHandler.removeMessages(MSG_PLAY_INDICATION);
                mIndicationHandler.sendMessageDelayed(mIndicationHandler.obtainMessage(
                        MSG_PLAY_INDICATION, 0, 0), RECORD_INTERVAL);
                stopUpdateIntensity();
                break;
            default:
                break;
            }
            Log.d("@M_" + TAG, "promptMessage = " + getResources().getText(stage.promptMessage));
            if (mVowMode == Utils.VOW_NO_SPEAKER_MODE) {
                if (mUiStage == Stage.Introduction) {
                    mIntroText.setText(getActivity().getString(R.string.vow_setup_intro_no_speaker,
                            mCommandKeyword));
                } else if (mUiStage == Stage.FirstRecording || mUiStage == Stage.NonFirstRecording) {
                    mPrompt.setText(getActivity().getString(R.string.vow_recording_no_speaker,
                            mCommandKeyword));
                } else if (mUiStage == Stage.OneRoundDiffCustom) {
                    mPrompt.setText(getActivity().getString(stage.promptMessage, mCommandKeyword));
                } else {
                    mPrompt.setText(stage.promptMessage);
                }
            } else if (mVowMode == Utils.VOW_WITH_SPEAKER_MODE) {
                if (mUiStage == Stage.Introduction) {
                    mIntroText.setText(stage.promptMessage);
                } else {
                    mPrompt.setText(stage.promptMessage);
                }
            }
        }

        enum RightButtonMode {
            //naming rule : stage_label
            Introduction_Continue(R.string.voice_unlock_continue_label, true),
            Prepare_Continue(R.string.voice_unlock_continue_label, false),
            Record_ContinueEnable(R.string.voice_unlock_continue_label, true),
            Record_ContinueDisable(R.string.voice_unlock_continue_label, false),
            Record_Done(R.string.vow_done_label,true);

            /**
             * @param text
             *            The displayed text for this mode.
             * @param enabled
             *            Whether the button should be enabled.
             */
            RightButtonMode(int text, boolean enabled) {
                this.text = text;
                this.enabled = enabled;
            }

            final int text;
            final boolean enabled;
        }

        enum LeftButtonMode {
            Cancel(R.string.voice_unlock_cancel_label, true),
            Retry(R.string.voice_unlock_retry_label, true);
            /**
             * @param text
             *            The displayed text for this mode.
             * @param enabled
             *            Whether the button should be enabled.
             */
            LeftButtonMode(int text, boolean enabled) {
                this.text = text;
                this.enabled = enabled;
            }

            final int text;
            final boolean enabled;
        }

        enum Stage {

            Introduction(
                    R.string.vow_setup_intro_with_speaker, LeftButtonMode.Cancel,
                    RightButtonMode.Introduction_Continue),
            Prepare(R.string.voice_command_record_prepare,
                    LeftButtonMode.Cancel, RightButtonMode.Prepare_Continue),
            FirstRecording(
                    R.string.vow_record_first_recording_with_speaker, LeftButtonMode.Cancel,
                    RightButtonMode.Prepare_Continue),
            NonFirstRecording(
                    R.string.voice_command_record_non_first_recording, LeftButtonMode.Cancel,
                    RightButtonMode.Record_ContinueDisable),
            OneRoundOK(
                    R.string.vow_one_round_ok,
                    LeftButtonMode.Cancel, RightButtonMode.Record_ContinueEnable),
            OneRoundNoisy(
                    R.string.voice_command_record_one_round_noisy, LeftButtonMode.Cancel,
                    RightButtonMode.Record_ContinueEnable),
            OneRoundWeak(
                    R.string.voice_command_record_one_round_weak, LeftButtonMode.Cancel,
                    RightButtonMode.Record_ContinueEnable),
            OneRoundDiff(
                    R.string.voice_command_record_one_round_diff, LeftButtonMode.Cancel,
                    RightButtonMode.Record_ContinueEnable),
            OneRoundExist(
                    R.string.voice_command_record_one_round_exist, LeftButtonMode.Cancel,
                    RightButtonMode.Record_ContinueEnable),
            OneRoundTimeout( // New flow introduced.
                    R.string.time_out_message, LeftButtonMode.Cancel,
                    RightButtonMode.Record_ContinueEnable),
            RecordingOK(
                    R.string.voice_command_record_recording_ok,
                    LeftButtonMode.Cancel, RightButtonMode.Record_Done),
            OneRoundDiffCustom(
                    R.string.voice_command_record_different_with_customized, LeftButtonMode.Cancel,
                    RightButtonMode.Record_ContinueEnable),
            Retry(
                    R.string.vow_one_round_ok, LeftButtonMode.Retry,
                    RightButtonMode.Record_ContinueEnable);
            /**
             * @param headerMessage
             *            The message displayed at the top.
             * @param leftMode
             *            The mode of the left button.
             * @param rightMode
             *            The mode of the right button.
             * @param footerMessage
             *            The footer message.
             * @param patternEnabled
             *            Whether the pattern widget is enabled.
             */
            Stage(int promptMessage, LeftButtonMode leftMode, RightButtonMode rightMode) {
                this.promptMessage = promptMessage;
                this.leftMode = leftMode;
                this.rightMode = rightMode;
            }

            final int promptMessage;
            final LeftButtonMode leftMode;
            final RightButtonMode rightMode;
        }

        @Override
        public void handleVoiceCommandNotified(int mainAction, int subAction, Bundle extraData) {
            int result = extraData.getInt(VoiceCommandListener.ACTION_EXTRA_RESULT);
            Log.d("@M_" + TAG, "onNotified result=" + result + " mainAction = " + mainAction
                    + " subAction = " + subAction);
            if (result == VoiceCommandListener.ACTION_EXTRA_RESULT_SUCCESS) {
                switch (subAction) {
                case VoiceCommandListener.ACTION_VOICE_TRAINING_START:
                    Log.d("@M_" + TAG, "onNotified TRAINING_START");
                    mHandler.sendMessage(mHandler.obtainMessage(MSG_START_TRAINING));
                    break;
                case VoiceCommandListener.ACTION_VOICE_TRAINING_INTENSITY:
                    int intensity = extraData.getInt(VoiceCommandListener.ACTION_EXTRA_RESULT_INFO);
                    Log.d("@M_" + TAG, "onNotified TRAINING_INTENSITY intensity = " + intensity);
                    mHandler.removeMessages(MSG_UPDATE_INTENSITY);
                    mHandler
                            .sendMessage(mHandler.obtainMessage(MSG_UPDATE_INTENSITY, intensity, 0));
                    break;
                case VoiceCommandListener.ACTION_VOICE_TRAINING_NOTIFY:
                    int resultId = extraData.getInt(VoiceCommandListener.ACTION_EXTRA_RESULT_INFO);
                    int progress = extraData.getInt(VoiceCommandListener.ACTION_EXTRA_RESULT_INFO1);
                    Log.d("@M_" + TAG, "onNotified TRAINING_NOTIFY progress = " + progress
                            + " resultId = " + resultId);
                    mHandler.sendMessage(mHandler.obtainMessage(MSG_UPDATE_NOTIFY, resultId,
                            progress));
                    break;
                case VoiceCommandListener.ACTION_VOICE_TRAINING_NOTIFY_FINISH:
                    int snr = extraData.getInt(VoiceCommandListener.ACTION_EXTRA_RESULT_INFO);
                    Log.d("@M_" + TAG, "onNotified TRAINING_NOTIFY finish snr = " + snr);
                    mHandler.sendMessage(mHandler.obtainMessage(MSG_FINISH_NOTIFY, snr));
                    break;
                default:
                    break;
                }
            } else if (result == VoiceCommandListener.ACTION_EXTRA_RESULT_ERROR) {
                String errorMsg = extraData
                        .getString(VoiceCommandListener.ACTION_EXTRA_RESULT_INFO1);
                Log.d("@M_" + TAG, "onNotified RESULT_ERROR errorMsg = " + errorMsg);
                mHandler.sendMessage(mHandler.obtainMessage(MSG_SERVICE_ERROR, errorMsg));
            }
        }

        @Override
        public void onVoiceServiceConnect() {
            voiceTrainingStart();
        }
    }
}
