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
 * MediaTek Inc. (C) 2014. All rights reserved.
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
package com.mediatek.voicecommand.ui.unlock;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.media.AudioManager;
import android.media.AudioManager.OnAudioFocusChangeListener;
import android.media.MediaPlayer;
import android.media.MediaPlayer.OnCompletionListener;
import android.media.MediaPlayer.OnErrorListener;
import android.media.MediaPlayer.OnPreparedListener;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.view.View;
import android.view.Window;
import android.widget.ImageButton;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.voicecommand.R;
import com.mediatek.voicecommand.mgr.ConfigurationManager;
import com.mediatek.voicecommand.util.Log;

import java.io.File;
import java.io.IOException;

/**
 * An activity to show voice training record playing preferences to the user.
 * 
 */
public class PswPreviewActivity extends Activity implements OnPreparedListener, OnErrorListener,
        OnCompletionListener {
    private static final String TAG = "PswPreviewActivity";

    private static final String KEY_COMMAND_SUMMARY = "command_summary";
    private static final String KEY_COMMAND_ID = "command_id";
    private static final String KEY_COMMAND_MODE = "command_mode";

    private boolean mIsSeeking = false;
    private boolean mIsComplete = false;
    private boolean mIsPausedByTransientLossOfFocus;
    // check the activity status for power saving
    private boolean mIsPauseRefreshingProgressBar = false;

    private int mDuration = -1;

    private PreviewPlayer mPlayer;
    private TextView mTitleView;
    private CharSequence mTitle;
    private SeekBar mSeekBar;
    private Handler mProgressRefresherHandler;

    private Uri mUri;
    private AudioManager mAudioManager;

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);

        Intent intent = getIntent();
        if (intent == null) {
            Log.e(TAG, "[onCreate]intent is null");
            finish();
            return;
        }

        ConfigurationManager voiceConfigMgr = ConfigurationManager.getInstance(this);
        if (voiceConfigMgr == null) {
            Log.e(TAG, "ConfigurationManager is null");
            finish();
            return;
        }

        int commandId = intent.getIntExtra(KEY_COMMAND_ID, 0);
        int commandMode = intent.getIntExtra(KEY_COMMAND_MODE, 0);
        mTitle = intent.getStringExtra(KEY_COMMAND_SUMMARY);
        String path = voiceConfigMgr.getPasswordFilePath(commandMode);
        String filepath = path + commandId + ".dat";
        Log.i(TAG, "[onCreate]filepath: " + filepath + ",commandId = " + commandId
                + ",commandMode = " + commandMode + ",mTitle = " + mTitle);
        File file = new File(filepath);
        mUri = Uri.fromFile(file);

        setVolumeControlStream(AudioManager.STREAM_MUSIC);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        setContentView(R.layout.pwd_preview);

        mTitleView = (TextView) findViewById(R.id.title);
        mSeekBar = (SeekBar) findViewById(R.id.progress);
        mProgressRefresherHandler = new Handler();
        mAudioManager = (AudioManager) getSystemService(Context.AUDIO_SERVICE);

        PreviewPlayer player = (PreviewPlayer) getLastNonConfigurationInstance();
        if (player == null) {
            mPlayer = new PreviewPlayer();
            mPlayer.setActivity(this);
            try {
                mPlayer.setDataSourceAndPrepare(mUri);
            } catch (IOException ex) {
                // catch generic Exception, since we may be called with a media
                // content URI, another content provider's URI, a file URI,
                // an http URI, and there are different exceptions associated
                // with failure to open each of those.
                Log.e(TAG, "[onCreate]Failed to open file: " + ex);
                Toast.makeText(this, R.string.playback_failed, Toast.LENGTH_SHORT).show();
                finish();
                return;
            }
        } else {
            mPlayer = player;
            mPlayer.setActivity(this);
            if (mPlayer.isPrepared()) {
                showPostPrepareUi();
            }
        }
    }

    @Override
    public void onPrepared(MediaPlayer mp) {
        Log.i(TAG, "[onPrepared]...");
        if (isFinishing()) {
            Log.i(TAG, "[onPrepared]isFinishing,return!");
            return;
        }
        mPlayer = (PreviewPlayer) mp;
        setCommandTitle();
        mPlayer.start();
        showPostPrepareUi();
    }

    @Override
    public boolean onError(MediaPlayer mp, int what, int extra) {
        Log.i(TAG, "[onError]what = " + what + ",extra = " + extra);
        Toast.makeText(this, R.string.playback_failed, Toast.LENGTH_SHORT).show();
        finish();
        return true;
    }

    @Override
    public void onCompletion(MediaPlayer mp) {
        Log.i(TAG, "[onCompletion]...");
        updatePlayPause();
        // set the progress to end avoid when the file play complete but the
        // the progress is not to the end of seekbar.
        mSeekBar.setProgress(mSeekBar.getMax());
        mIsComplete = true;
    }

    @Override
    public Object onRetainNonConfigurationInstance() {
        Log.i(TAG, "[onRetainNonConfigurationInstance]...");
        PreviewPlayer player = mPlayer;
        mPlayer = null;
        return player;
    }

    @Override
    public void onDestroy() {
        Log.i(TAG, "[onDestroy]...");
        stopPlayback();
        super.onDestroy();
    }

    @Override
    public void onUserLeaveHint() {
        Log.i(TAG, "[onUserLeaveHint]...");
        stopPlayback();
        finish();
        super.onUserLeaveHint();
    }

    /**
     * Stop to update the pregress bar when activity pausefor power saving.
     */
    @Override
    public void onPause() {
        Log.i(TAG, "[onPause]...");
        mIsPauseRefreshingProgressBar = true;
        mProgressRefresherHandler.removeCallbacksAndMessages(null);
        super.onPause();
    }

    /**
     * Start the pregress bar update.
     */
    @Override
    public void onResume() {
        super.onResume();
        Log.i(TAG, "[onResume]mIsPauseRefreshingProgressBar = " + mIsPauseRefreshingProgressBar
              + ", mTitle = " + mTitle);
        if (mPlayer != null && mPlayer.isPrepared()) {
            setCommandTitle();
        }
        final int refreshTime = 200;
        if (mIsPauseRefreshingProgressBar) {
            mIsPauseRefreshingProgressBar = false;
            mProgressRefresherHandler.postDelayed(new ProgressRefresher(), refreshTime);
        }
    }

    /**
     * Click pause button.
     *
     * @param v
     *            view instance
     */
    public void playPauseClicked(View v) {
        Log.i(TAG, "[playPauseClicked]...");
        if (mPlayer.isPlaying()) {
            mPlayer.pause();
        } else {
            start();
        }
        mIsComplete = false;
        updatePlayPause();
    }

    /**
     * Monitor the current playing media file's duration update and reset the
     * Maxprocess of SeekBar.
     * 
     * @param mp
     *            MediaPlayer instance
     * @param duration
     *            SeekBar duration
     */
    public void onDurationUpdate(MediaPlayer mp, int duration) {
        Log.i(TAG, "[onDurationUpdate]duration = " + duration);
        if (duration > 0) {
            mDuration = duration;
            mSeekBar.setMax(mDuration);
        }
    }

    /**
     * Set password command play title.
     */
    public void setCommandTitle() {
        Log.i(TAG, "[setCommandTitle]mTitle = " + mTitle);
        mTitleView.setText(mTitle);
    }

    private void showPostPrepareUi() {
        mDuration = mPlayer.getDuration();
        Log.d(TAG, "[showPostPrepareUi]mDuration = " + mDuration);
        if (mDuration != 0) {
            mSeekBar.setMax(mDuration);
            mSeekBar.setVisibility(View.VISIBLE);
        }
        mSeekBar.setOnSeekBarChangeListener(mSeekListener);
        // request focus when the seekbar is not in touch mode
        if (!mSeekBar.isInTouchMode()) {
            mSeekBar.requestFocus();
        }

        View v = findViewById(R.id.titleandbuttons);
        v.setVisibility(View.VISIBLE);
        mAudioManager.requestAudioFocus(mAudioFocusListener, AudioManager.STREAM_MUSIC,
                AudioManager.AUDIOFOCUS_GAIN_TRANSIENT);
        mProgressRefresherHandler.postDelayed(new ProgressRefresher(), 200);
        updatePlayPause();
    }

    /**
     * Use for control playback of audio/video files and streams.
     * 
     */
    private static class PreviewPlayer extends MediaPlayer implements OnPreparedListener {
        PswPreviewActivity mActivity;
        boolean mIsPrepared = false;

        public void setActivity(PswPreviewActivity activity) {
            Log.d(TAG, "[setActivity]...");
            mActivity = activity;
            setOnPreparedListener(this);
            setOnErrorListener(mActivity);
            setOnCompletionListener(mActivity);
        }

        public void setDataSourceAndPrepare(Uri uri) throws IllegalArgumentException,
                SecurityException, IllegalStateException, IOException {
            Log.d(TAG, "[setDataSourceAndPrepare]uri = " + uri);
            setDataSource(mActivity, uri);
            prepareAsync();
        }

        @Override
        public void onPrepared(MediaPlayer mp) {
            Log.d(TAG, "[onPrepared]mIsPrepared = " + mIsPrepared);
            mIsPrepared = true;
            mActivity.onPrepared(mp);
        }

        boolean isPrepared() {
            return mIsPrepared;
        }
    }

    private OnSeekBarChangeListener mSeekListener = new OnSeekBarChangeListener() {
        public void onStartTrackingTouch(SeekBar bar) {
            mIsSeeking = true;
        }

        public void onProgressChanged(SeekBar bar, int progress, boolean fromuser) {
            Log.d(TAG, "[onProgressChanged]progress = " + progress + ",fromuser = " + fromuser
                    + ",mIsSeeking = " + mIsSeeking);
            if (!fromuser) {
                return;
            }
            // check if the mPlayer is not a null reference
            if (!mIsSeeking && (mPlayer != null)) {
                mPlayer.seekTo(progress);
            }
        }

        public void onStopTrackingTouch(SeekBar bar) {
            Log.d(TAG, "[onStopTrackingTouch]...");
            // check if the mPlayer is not a null reference
            if ((mPlayer != null)) {
                mPlayer.seekTo(bar.getProgress());
            }
            mIsSeeking = false;
            mIsComplete = false;
        }
    };

    private OnAudioFocusChangeListener mAudioFocusListener = new OnAudioFocusChangeListener() {
        public void onAudioFocusChange(int focusChange) {
            if (mPlayer == null) {
                // this activity has handed its MediaPlayer off to the next
                // activity
                // (e.g. portrait/landscape switch) and should abandon its focus
                mAudioManager.abandonAudioFocus(this);
                Log.w(TAG, "[onAudioFocusChange]player is null,return!");
                return;
            }

            Log.d(TAG, "[onAudioFocusChange]focusChange = " + focusChange);
            switch (focusChange) {
            case AudioManager.AUDIOFOCUS_LOSS:
                mIsPausedByTransientLossOfFocus = false;
                mPlayer.pause();
                break;

            case AudioManager.AUDIOFOCUS_LOSS_TRANSIENT:
            case AudioManager.AUDIOFOCUS_LOSS_TRANSIENT_CAN_DUCK:
                if (mPlayer.isPlaying()) {
                    mIsPausedByTransientLossOfFocus = true;
                    mPlayer.pause();
                }
                break;

            case AudioManager.AUDIOFOCUS_GAIN:
                if (mIsPausedByTransientLossOfFocus) {
                    mIsPausedByTransientLossOfFocus = false;
                    start();
                }
                break;

            default:
                break;
            }

            updatePlayPause();
        }
    };

    /**
     * Represents the progress refresh command that can be executed.
     * 
     */
    class ProgressRefresher implements Runnable {

        public void run() {
            Log.d(TAG, "[run]mIsComplete = " + mIsComplete + ",mDuration = " + mDuration
                    + ",mIsPauseRefreshingProgressBar = " + mIsPauseRefreshingProgressBar);
            if (mPlayer != null && !mIsSeeking && mDuration != 0) {
                // Remove dummy varible.
                int position = mPlayer.getCurrentPosition();
                Log.d(TAG, "[run]ProgressRefresher Position:" + position);
                // if the media file is complete ,we set SeekBar to the end @{
                if (mIsComplete) {
                    position = mDuration;
                }
                mSeekBar.setProgress(position);
            }
            mProgressRefresherHandler.removeCallbacksAndMessages(null);
            // check if the activity is pause for power saving
            if (!mIsPauseRefreshingProgressBar) {
                mProgressRefresherHandler.postDelayed(new ProgressRefresher(), 200);
            }
        }
    }

    private void stopPlayback() {
        Log.d(TAG, "[stopPlayback]...");
        if (mProgressRefresherHandler != null) {
            mProgressRefresherHandler.removeCallbacksAndMessages(null);
        }
        if (mPlayer != null) {
            mPlayer.release();
            mPlayer = null;
            mAudioManager.abandonAudioFocus(mAudioFocusListener);
        }
    }

    private void start() {
        Log.d(TAG, "[start]mIsComplete = " + mIsComplete);
        mAudioManager.requestAudioFocus(mAudioFocusListener, AudioManager.STREAM_MUSIC,
                AudioManager.AUDIOFOCUS_GAIN_TRANSIENT);
        mPlayer.start();
        if (mIsComplete) {
            mSeekBar.setProgress(0);
        }
        mProgressRefresherHandler.postDelayed(new ProgressRefresher(), 200);
    }

    private void updatePlayPause() {
        Log.d(TAG, "[updatePlayPause]...");
        ImageButton b = (ImageButton) findViewById(R.id.playpause);
        if (b != null) {
            if (mPlayer.isPlaying()) {
                b.setImageResource(R.drawable.btn_playback_ic_pause_small);
            } else {
                b.setImageResource(R.drawable.btn_playback_ic_play_small);
                mProgressRefresherHandler.removeCallbacksAndMessages(null);
            }
        }
    }
}
