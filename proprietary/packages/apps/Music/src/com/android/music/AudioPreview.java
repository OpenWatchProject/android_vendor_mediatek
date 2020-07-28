/*
 * Copyright (C) 2014 MediaTek Inc.
 * Modification based on code covered by the mentioned copyright
 * and/or permission notice(s).
 */

/*
 * Copyright (C) 2010 The Android Open Source Project
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

package com.android.music;

import java.io.IOException;

import android.app.Activity;
import android.content.AsyncQueryHandler;
import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.Cursor;
import android.drm.DrmManagerClient;
import android.media.AudioManager;
//Not Required
//import android.media.MediaPlayer;
import android.media.AudioManager.OnAudioFocusChangeListener;
import android.media.MediaPlayer;
import android.media.MediaPlayer.OnCompletionListener;
import android.media.MediaPlayer.OnErrorListener;
import android.media.MediaPlayer.OnInfoListener;
import android.media.MediaPlayer.OnPreparedListener;
//import android.media.MediaPlayer.OnDurationUpdateListener;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.provider.MediaStore;
import android.provider.OpenableColumns;
import android.text.TextUtils;
//Not Required
//import android.util.Log;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.Window;
//Not Required
//import android.view.WindowManager;
import android.widget.ImageButton;
import android.widget.ProgressBar;
import android.widget.SeekBar;
//Not Required
//import android.widget.TextView;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.TextView;
import android.widget.Toast;

//import com.mediatek.drm.OmaDrmClient;
import com.mediatek.omadrm.OmaDrmUtils;
//Not Required
//import java.io.IOException;

/**
 * Dialog that comes up in response to various music-related VIEW intents.
 */
public class AudioPreview extends Activity
        implements
            OnPreparedListener,
            OnErrorListener,
            OnCompletionListener,
            OnInfoListener
//            OnDurationUpdateListener
            {
    private static final String TAG = "AudioPreview";
    private PreviewPlayer mPlayer;
    private TextView mTextLine1;
    private TextView mTextLine2;
    private TextView mLoadingText;
    private SeekBar mSeekBar;
    private Handler mProgressRefresher;
    private boolean mSeeking = false;
    private int mDuration = -1;
    private Uri mUri;
    private long mMediaId = -1;
    private static final int OPEN_IN_MUSIC = 1;
    private AudioManager mAudioManager;
    private boolean mPausedByTransientLossOfFocus;
    /// M: add to check the activity status for power saving
    private boolean mPauseRefreshingProgressBar = false;
    /// M: add to check if the media can seek for special case
    private boolean mMediaCanSeek = true;
    /// M:add to check whether the file complete be complete.
    private boolean mIsComplete = false;
    /// M: Use member variable to show toast to avoid show the
    ///toast on screen for a long time if user click many time.
    private Toast mToast;
    // M: Use to listen to unmount the sdcard
    private BroadcastReceiver mUnmountReceiver = null;

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        MusicLogUtils.d(TAG, "onCreate");
        Intent intent = getIntent();
        AudioManager audioMngr =
             (AudioManager) getApplicationContext().getSystemService(Context.AUDIO_SERVICE);
        if (audioMngr.getMode() == AudioManager.MODE_IN_COMMUNICATION) {
            showToast(getString(R.string.audiofocus_request_failed_message));
            MusicLogUtils.v(TAG, "onCreate: phone call is ongoing, can not play music!");
            finish();
            return;
        }
        if (intent == null) {
            finish();
            return;
        }
        mUri = intent.getData();
        if (mUri == null) {
            finish();
            return;
        }
        String scheme = mUri.getScheme();

        setVolumeControlStream(AudioManager.STREAM_MUSIC);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        setContentView(R.layout.audiopreview);
        /// M: set the Activity dosen't close when touch the outside of itself
        getWindow().setCloseOnTouchOutside(false);
        mTextLine1 = (TextView) findViewById(R.id.line1);
        mTextLine2 = (TextView) findViewById(R.id.line2);
        mLoadingText = (TextView) findViewById(R.id.loading);
        if (scheme.equals("http")) {
            String msg = getString(R.string.streamloadingtext, mUri.getHost());
            mLoadingText.setText(msg);
        } else {
            mLoadingText.setVisibility(View.GONE);
        }
        /// M: Show the given title and displayname provided by started APP. @{
        String title = intent.getStringExtra("title");
        if (title != null) {
            mTextLine1.setText(title);
        }
        String displayName = intent.getStringExtra("displayName");
        if (displayName != null) {
            mTextLine2.setText(displayName);
        }
        /// @}
        mSeekBar = (SeekBar) findViewById(R.id.progress);
        mProgressRefresher = new Handler();
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
                MusicLogUtils.d(TAG, "Failed to open file: " + ex);
                Toast.makeText(this, R.string.playback_failed, Toast.LENGTH_SHORT).show();
                finish();
                return;
            }
        } else {
            mPlayer = player;
            mAudioManager.requestAudioFocus(mAudioFocusListener,
                    AudioManager.STREAM_MUSIC, AudioManager.AUDIOFOCUS_GAIN_TRANSIENT);
            mPlayer.setActivity(this);
            MusicLogUtils.d(TAG, "onCreate,mPlayer.isPrepared():" + mPlayer.isPrepared());
            if (mPlayer.isPrepared()) {
                showPostPrepareUI();
            }
        }

        registerExternalStorageListener();
        AsyncQueryHandler mAsyncQueryHandler = new AsyncQueryHandler(getContentResolver()) {
            @Override
            protected void onQueryComplete(int token, Object cookie, Cursor cursor) {
                if (cursor != null && cursor.moveToFirst()) {

                    int titleIdx = cursor.getColumnIndex(MediaStore.Audio.Media.TITLE);
                    /// M: get the artist name's column index.
                    int artistIdx = cursor.getColumnIndex(MediaStore.Audio.Media.ARTIST);
                    int idIdx = cursor.getColumnIndex(MediaStore.Audio.Media._ID);
                    int displaynameIdx = cursor.getColumnIndex(OpenableColumns.DISPLAY_NAME);
                    /// M: define members variable and get the string of unknown artist
                    /// name with switched language @{
                    String unknownArtist = getString(R.string.unknown_artist_name);
                    String displayName = null;
                   /// @}
                    if (idIdx >= 0) {
                        mMediaId = cursor.getLong(idIdx);
                    }

                    if (titleIdx >= 0) {
                        String title = cursor.getString(titleIdx);
                        mTextLine1.setText(title);
                        if (artistIdx >= 0) {
                            /// M: get the string of artist name,if unknown,
                            ///set string of unknown artist with
                            //     switched language,otherwise,set artist name on Textline2. @{
                            displayName = cursor.getString(artistIdx);
                            MusicLogUtils.d(TAG, "displayname" + displayName);
                            if (displayName == null ||
                            displayName.equals(MediaStore.UNKNOWN_STRING)) {
                                displayName = unknownArtist;
                            }
                            /// @}
                            mTextLine2.setText(displayName);
                        }
                    } else if (displaynameIdx >= 0) {
                        String name = cursor.getString(displaynameIdx);
                        mTextLine1.setText(name);
                    } else {
                        // Couldn't find anything to display, what to do now?
                        MusicLogUtils.d(TAG, "Cursor had no names for us");
                    }
                } else {
                    MusicLogUtils.d(TAG, "empty cursor");
                }

                if (cursor != null) {
                    cursor.close();
                }
                setNames();
            }
        };

        if (scheme.equals(ContentResolver.SCHEME_CONTENT)) {
            if (mUri.getAuthority().equals(MediaStore.AUTHORITY)) {
                // try to get title and artist from the media content provider
                mAsyncQueryHandler.startQuery(0, null, mUri, new String [] {
                        MediaStore.Audio.Media.TITLE, MediaStore.Audio.Media.ARTIST},
                        null, null, null);
            } else {
                // Try to get the display name from another content provider.
                // Don't specifically ask for the display name though, since the
                // provider might not actually support that column.
                mAsyncQueryHandler.startQuery(0, null, mUri, null, null, null, null);
            }
        } else if (scheme.equals("file")) {
            // check if this file is in the media database (clicking on a download
            // in the download manager might follow this path
            String path = mUri.getPath();
            mAsyncQueryHandler.startQuery(0, null,  MediaStore.Audio.Media.EXTERNAL_CONTENT_URI,
                    new String [] {MediaStore.Audio.Media._ID,
                        MediaStore.Audio.Media.TITLE, MediaStore.Audio.Media.ARTIST},
                    MediaStore.Audio.Media.DATA + "=?", new String [] {path}, null);
        } else {
            // We can't get metadata from the file/stream itself yet, because
            // that API is hidden, so instead we display the URI being played
            if (mPlayer.isPrepared()) {
                setNames();
            }
        }
    }

    @Override
    public Object onRetainNonConfigurationInstance() {
        MusicLogUtils.d(TAG, "onRetainNonConfigurationInstance:");
        PreviewPlayer player = mPlayer;
        mPlayer = null;
        mAudioManager.abandonAudioFocus(mAudioFocusListener);
        return player;
    }

    @Override
    public void onDestroy() {
        MusicLogUtils.d(TAG, "onDestroy()");
        stopPlayback();
        if (mUnmountReceiver != null) {
            unregisterReceiver(mUnmountReceiver);
            mUnmountReceiver = null;
        }
        super.onDestroy();
    }

    private void stopPlayback() {
        MusicLogUtils.d(TAG, "stopPlayback(),mPlayer:" + mPlayer);
        if (mProgressRefresher != null) {
            mProgressRefresher.removeCallbacksAndMessages(null);
        }
        if (mPlayer != null) {
            mPlayer.release();
            mPlayer = null;
            mAudioManager.abandonAudioFocus(mAudioFocusListener);
        }
    }

    @Override
    public void onUserLeaveHint() {
        MusicLogUtils.d(TAG, "onUserLeaveHint()");
        stopPlayback();
        finish();
        super.onUserLeaveHint();
    }

    @Override
    public void onPrepared(MediaPlayer mp) {
        AudioManager audioMngr =
             (AudioManager) getApplicationContext().getSystemService(Context.AUDIO_SERVICE);
        if (isFinishing()) {
            return;
        }
        mPlayer = (PreviewPlayer) mp;
        setNames();
        /// M: When request AudioFocus failed,
        /// we should not play music and need toast user maybe the
        /// phone call is ongoing. @{
        if ((AudioManager.AUDIOFOCUS_REQUEST_FAILED ==
         mAudioManager.requestAudioFocus(mAudioFocusListener,
                AudioManager.STREAM_MUSIC, AudioManager.AUDIOFOCUS_GAIN_TRANSIENT))
                || (audioMngr.getMode() == AudioManager.MODE_IN_COMMUNICATION)) {
            showToast(getString(R.string.audiofocus_request_failed_message));
            MusicLogUtils.d(TAG, "onPrepared: phone call is ongoing, can not play music!");
        } else {
            mPlayer.start();
        }
        /// @}
        showPostPrepareUI();
    }

    private void showPostPrepareUI() {
        /// M: work around for infinite duration imy file seeking @{
        MusicLogUtils.d(TAG, "showPostPrepareUI");
        ProgressBar pb = (ProgressBar) findViewById(R.id.spinner);
        pb.setVisibility(View.GONE);
        mDuration = mPlayer.getDuration();
        MusicLogUtils.d(TAG, "mDuration:" + mDuration);
        /// M: work around for infinite duration imy file seeking @{
        String path = mUri.getPath();
        MusicLogUtils.d(TAG, path);
        mMediaCanSeek = true;
        final String imySuffix = ".imy";
        if ((path.toLowerCase().endsWith(imySuffix)) && (mDuration == Integer.MAX_VALUE)) {
            mMediaCanSeek = false;
        }
        /// @}
        if (mDuration != 0) {
            mSeekBar.setMax(mDuration);
            mSeekBar.setVisibility(View.VISIBLE);
            if (!mSeeking) {
                mSeekBar.setProgress(mPlayer.getCurrentPosition());
            }
        }
        mSeekBar.setOnSeekBarChangeListener(mSeekListener);
        /// M: request focus when the seekbar is not in touch mode @{
        if (!mSeekBar.isInTouchMode()) {
            mSeekBar.requestFocus();
        }
        /// @}
        mLoadingText.setVisibility(View.GONE);
        View v = findViewById(R.id.titleandbuttons);
        v.setVisibility(View.VISIBLE);
        mAudioManager.requestAudioFocus(mAudioFocusListener, AudioManager.STREAM_MUSIC,
                AudioManager.AUDIOFOCUS_GAIN_TRANSIENT);
        if (mProgressRefresher != null) {
            mProgressRefresher.removeCallbacksAndMessages(null);
            mProgressRefresher.postDelayed(new ProgressRefresher(), 200);
        }
        updatePlayPause();
    }

    private OnAudioFocusChangeListener mAudioFocusListener = new OnAudioFocusChangeListener() {
        public void onAudioFocusChange(int focusChange) {
            MusicLogUtils.d(TAG, "mPlayer:" + mPlayer + ",focusChange:" + focusChange);
            if (mPlayer == null) {
                // this activity has handed its MediaPlayer off to the next activity
                // (e.g. portrait/landscape switch) and should abandon its focus
                mAudioManager.abandonAudioFocus(this);
                return;
            }
            switch (focusChange) {
                case AudioManager.AUDIOFOCUS_LOSS:
                    mPausedByTransientLossOfFocus = false;
                    mPlayer.pause();
                    break;
                case AudioManager.AUDIOFOCUS_LOSS_TRANSIENT:
                case AudioManager.AUDIOFOCUS_LOSS_TRANSIENT_CAN_DUCK:
                    if (mPlayer.isPlaying()) {
                        mPausedByTransientLossOfFocus = true;
                        mPlayer.pause();
                    }
                    break;
                case AudioManager.AUDIOFOCUS_GAIN:
                    if (mPausedByTransientLossOfFocus) {
                        mPausedByTransientLossOfFocus = false;
                        start();
                    }
                    break;
            }
            updatePlayPause();
        }
    };

    private void start() {
        /// M: When request AudioFocus failed,
        /// we should not play music and need toast user maybe the
        /// phone call is ongoing. @{
        if (AudioManager.AUDIOFOCUS_REQUEST_FAILED ==
         mAudioManager.requestAudioFocus(mAudioFocusListener,
                AudioManager.STREAM_MUSIC, AudioManager.AUDIOFOCUS_GAIN_TRANSIENT)) {
            showToast(getString(R.string.audiofocus_request_failed_message));
            MusicLogUtils.d(TAG, "start: phone call is ongoing, can not play music!");
            return;
        }
        /// @}
        mPlayer.start();
        mProgressRefresher.postDelayed(new ProgressRefresher(), 0);
    }

    public void setNames() {
        if (TextUtils.isEmpty(mTextLine1.getText())) {
            mTextLine1.setText(mUri.getLastPathSegment());
        }
        if (TextUtils.isEmpty(mTextLine2.getText())) {
            mTextLine2.setVisibility(View.GONE);
        } else {
            mTextLine2.setVisibility(View.VISIBLE);
        }
    }

    class ProgressRefresher implements Runnable {


        public void run() {
            if (mPlayer != null && !mSeeking && mDuration != 0) {
                /// M: Remove dummy varible.
                int position = mPlayer.getCurrentPosition();
                //int progress = mPlayer.getCurrentPosition() / mDuration;
                /// M: if the media file is complete and
                /// position is at end of duration,we set SeekBar
                /// to the end(avoid auto play audio file, such as OGG) @{
                if (mIsComplete && (position + 500 >= mDuration)) {
                    position = mDuration;
                }
                /// @}
                MusicLogUtils.d(TAG, "ProgressRefresher Position:" + position);
                mSeekBar.setProgress(mPlayer.getCurrentPosition());
            }
            mProgressRefresher.removeCallbacksAndMessages(null);
            /// M: check if the activity is pause for power saving @{
            if (!mPauseRefreshingProgressBar) {
                mProgressRefresher.postDelayed(new ProgressRefresher(), 200);
            }
            /// @}
        }
    }

    private void updatePlayPause() {
        ImageButton b = (ImageButton) findViewById(R.id.playpause);
        if (b != null && mPlayer != null) {
            if (mPlayer.isPlaying()) {
                b.setImageResource(R.drawable.ic_appwidget_music_pause);
            } else {
                b.setImageResource(R.drawable.ic_appwidget_music_play);
                if (mProgressRefresher != null) {
                    mProgressRefresher.removeCallbacksAndMessages(null);
                }
            }
        }
    }

    private OnSeekBarChangeListener mSeekListener = new OnSeekBarChangeListener() {
        public void onStartTrackingTouch(SeekBar bar) {
            /// M: check the media can seek fisrtly.
            if (mMediaCanSeek) {
                mSeeking = true;
            }
        }
        public void onProgressChanged(SeekBar bar, int progress, boolean fromuser) {
            if (!fromuser) {
                return;
            }
            /// M:  check if the mPlayer is not a null reference for cr alps00066845
            if (!mSeeking && (mPlayer != null) && mMediaCanSeek) {
                mPlayer.seekTo(progress);
            }
            //Not required to seek
            //mPlayer.seekTo(progress);
        }
        public void onStopTrackingTouch(SeekBar bar) {
            /// M:  check if the mPlayer is not a null reference for cr alps00066845
            if ((mPlayer != null) && mMediaCanSeek) {
                mPlayer.seekTo(bar.getProgress());
            }
            mSeeking = false;
            mIsComplete = false;
        }
    };

    @Override
    public boolean onError(MediaPlayer mp, int what, int extra) {
        Toast.makeText(this, R.string.playback_failed, Toast.LENGTH_SHORT).show();
        finish();
        return true;
    }

    @Override
    public void onCompletion(MediaPlayer mp) {
        /// M: mark for setting the progress to the positon of the media finish.
        //mSeekBar.setProgress(mDuration);
        MusicLogUtils.d(TAG, "onCompletion Position:" + mPlayer.getCurrentPosition());
        updatePlayPause();
        /// M:set the progress to end avoid when the file play complete but the
        // the progress is not to the end of seekbar. @{
        mSeekBar.setProgress(mSeekBar.getMax());
        mIsComplete = true;
        /// @}
    }

    public void playPauseClicked(View v) {
        // Protection for case of simultaneously tapping on play/pause and exit
        if (mPlayer == null) {
            return;
        }
        if (mPlayer.isPlaying()) {
            mPlayer.pause();
        } else {
            start();
        }
        mIsComplete = false;
        updatePlayPause();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        super.onCreateOptionsMenu(menu);
        // TODO: if mMediaId != -1, then the playing file has an entry in the media
        // database, and we could open it in the full music app instead.
        // Ideally, we would hand off the currently running mediaplayer
        // to the music UI, which can probably be done via a public static
        menu.add(0, OPEN_IN_MUSIC, 0, "open in music");
        return true;
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        MenuItem item = menu.findItem(OPEN_IN_MUSIC);
        if (mMediaId >= 0) {
            item.setVisible(true);
            /// M: temporarily disable options menu since OPEN_IN_MUSIC is half-finished by Google
            return false;
        }
        item.setVisible(false);
        return false;
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        switch (keyCode) {
            case KeyEvent.KEYCODE_HEADSETHOOK:
            case KeyEvent.KEYCODE_MEDIA_PLAY_PAUSE:
                if (mPlayer.isPlaying()) {
                    mPlayer.pause();
                } else {
                    mIsComplete = false;
                    start();
                }
                updatePlayPause();
                return true;
            case KeyEvent.KEYCODE_MEDIA_PLAY:
                start();
                updatePlayPause();
                return true;
            case KeyEvent.KEYCODE_MEDIA_PAUSE:
                if (mPlayer.isPlaying()) {
                    mPlayer.pause();
                }
                updatePlayPause();
                return true;
            case KeyEvent.KEYCODE_MEDIA_FAST_FORWARD:
            case KeyEvent.KEYCODE_MEDIA_NEXT:
            case KeyEvent.KEYCODE_MEDIA_PREVIOUS:
            case KeyEvent.KEYCODE_MEDIA_REWIND:
                return true;
            case KeyEvent.KEYCODE_MEDIA_STOP:
            case KeyEvent.KEYCODE_BACK:
                stopPlayback();
                finish();
                return true;
        }
        return super.onKeyDown(keyCode, event);
    }

    /*
     * Wrapper class to help with handing off the MediaPlayer to the next instance
     * of the activity in case of orientation change, without losing any state.
     */
    private static class PreviewPlayer extends MediaPlayer implements OnPreparedListener {
        AudioPreview mActivity;
        boolean mIsPrepared = false;

        public void setActivity(AudioPreview activity) {
            mActivity = activity;
            setOnPreparedListener(this);
            setOnErrorListener(mActivity);
            setOnCompletionListener(mActivity);
            /// M: add for handle the non-suport media
            setOnInfoListener(mActivity);
            /// M: add for monitor media file's duration update.
           // setOnDurationUpdateListener(mActivity);
        }

        public void setDataSourceAndPrepare(Uri uri) throws IllegalArgumentException,
                        SecurityException, IllegalStateException, IOException {
            setDataSource(mActivity, uri);
            prepareAsync();
        }

        /* (non-Javadoc)
         * @see android.media.MediaPlayer.OnPreparedListener#onPrepared(android.media.MediaPlayer)
         */
        @Override
        public void onPrepared(MediaPlayer mp) {
            mIsPrepared = true;
            mActivity.onPrepared(mp);
        }

        boolean isPrepared() {
            return mIsPrepared;
        }
    }

    /**
     * M: stop to update the pregress bar when activity pausefor power saving
     */
    @Override
    public void onPause() {
        mPauseRefreshingProgressBar = true;
        mProgressRefresher.removeCallbacksAndMessages(null);
        /// M: Add for CTA level 5, if open from DataProtection,
        /// finish when pause to background
        Intent intent = getIntent();
        String token = intent.getStringExtra("TOKEN");
        if (token != null) {
            MusicLogUtils.d(TAG,
                    "open from DataProtection, clear token and finish acivity when pause");
            DrmManagerClient drmClient = null;
            try {
                drmClient = new DrmManagerClient(this);
                String tokenKey = intent.getStringExtra("TOKEN_KEY");
                OmaDrmUtils.clearToken(drmClient, tokenKey, token);
            } finally {
                if (drmClient != null) {
                    drmClient.release();
                }
            }
            stopPlayback();
            finish();
        }
        MusicLogUtils.d(TAG, "onPause for stop ProgressRefresher!: token = " + token);
        super.onPause();
    }

    /**
     * M: start the pregress bar update.
     */
    @Override
    public void onResume() {
        super.onResume();
        final int refreshTime = 200;
        MusicLogUtils.d(TAG, "onResume for start ProgressRefresher!");
        if (mPauseRefreshingProgressBar) {
            mPauseRefreshingProgressBar = false;
            mProgressRefresher.postDelayed(new ProgressRefresher(), refreshTime);
        }
    }

    /**
     * M: monitor the current playing media file's
     * duration update and reset the Maxprocess of SeekBar.
     */
  /*  @Override
      public void onDurationUpdate(MediaPlayer mp, int duration) {
          if (duration > 0) {
              mDuration = duration;
              mSeekBar.setMax(mDuration);
          }
          MusicLogUtils.v(TAG, "onDurationUpdate(" + mDuration + ")");
      }

    /**
     * M: monitor the information and handle the non-suport media
     * @return boolean
     */
    @Override
    public boolean onInfo(MediaPlayer mp, int what, int msg) {
      /*  MusicLogUtils.d(TAG, "onInfo: " + what);
        if (what == MediaPlayer.MEDIA_INFO_AUDIO_NOT_SUPPORTED) {
            showToast(getString(R.string.playback_failed));
            finish();
            return true;
        } */
        return false;
    }

    /**
     * M: Don't respond to quick search request in AudioPreview
     */
    @Override
    public boolean onSearchRequested() {
        return false;
    }

    /**
     * M: Show the given text to screen.
     *
     * @param toastText Need show text.
     */
    private void showToast(CharSequence toastText) {
        if (mToast == null) {
            mToast = Toast.makeText(getApplicationContext(), toastText, Toast.LENGTH_SHORT);
        }
        mToast.setText(toastText);
        mToast.show();
    }

    /**
     * Registers an intent to listen for ACTION_MEDIA_EJECT notifications.
     * The intent will call closeExternalStorageFiles() if the external media
     * is going to be ejected, so applications can clean up any files they have open.
     */
    public void registerExternalStorageListener() {

        if (mUnmountReceiver == null) {
            mUnmountReceiver = new BroadcastReceiver() {
                @Override
                public void onReceive(Context context, Intent intent) {
                    String action = intent.getAction();
                    String ejectingCardPath = null;
                    /// M: modify for Multi-SD card support @{
                    if (action.equals(Intent.ACTION_MEDIA_EJECT)) {
                        MusicLogUtils.d(TAG, "MEDIA_EJECT");
                        /// M: For multi-SD card, we should decide whether
                        ///the internal SD card is ejected
                        ejectingCardPath = intent.getData().getPath();
                        MusicLogUtils.v(TAG, "card eject: ejectingCardPath=" + ejectingCardPath);
                        // / M: see if the currently playing track is on the
                        // removed card...
                        String curTrackPath = mUri.getPath();
                        if (curTrackPath != null && curTrackPath.contains(ejectingCardPath)) {
                            MusicLogUtils.d(TAG,
                            "MEDIA_EJECT: current track on an unmounting external card");
                            finish();
                        }
                    }
                }
            };
            IntentFilter iFilter = new IntentFilter();
            iFilter.addAction(Intent.ACTION_MEDIA_EJECT);
            iFilter.addDataScheme("file");
            registerReceiver(mUnmountReceiver, iFilter);
        }
    }
}
