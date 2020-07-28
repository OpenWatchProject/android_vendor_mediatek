/*
 * Copyright (C) 2014 MediaTek Inc.
 * Modification based on code covered by the mentioned copyright
 * and/or permission notice(s).
 */
/*
 * Copyright (C) 2007 The Android Open Source Project
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

package com.android.gallery3d.app;

import android.annotation.TargetApi;
import android.app.ActionBar;
import android.app.Activity;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.graphics.Bitmap;
import android.graphics.PixelFormat;
import android.graphics.drawable.BitmapDrawable;
import android.media.AudioManager;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.provider.MediaStore;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.ShareActionProvider;

import com.android.gallery3d.R;
import com.android.gallery3d.common.ApiHelper;
import com.mediatek.gallery3d.ext.IActivityHooker;
import com.mediatek.gallery3d.util.Log;
import com.mediatek.gallery3d.video.IMovieItem;
import com.mediatek.gallery3d.video.DefaultMovieItem;
import com.mediatek.gallery3d.video.ExtensionHelper;
import com.mediatek.gallery3d.video.VideoTitleHooker;
import com.mediatek.gallery3d.video.MovieUtils;
import com.mediatek.gallery3d.video.MtkVideoFeature;
import com.mediatek.gallery3d.video.RequestPermissionActivity;

/**
 * This activity plays a video from a specified URI. The client of this activity
 * can pass a logo bitmap in the intent (KEY_LOGO_BITMAP) to set the action bar
 * logo so the playback process looks more seamlessly integrated with the
 * original activity.
 */
public class MovieActivity extends Activity {
    @SuppressWarnings("unused")
    private static final String TAG = "VP_MovieActivity";

    public static final String KEY_LOGO_BITMAP = "logo-bitmap";
    public static final String KEY_TREAT_UP_AS_BACK = "treat-up-as-back";

    private IActivityHooker mMovieHooker;
    private MoviePlayer mPlayer;
    private boolean mFinishOnCompletion;
    private boolean mTreatUpAsBack;
    // /M: add for streaming cookie
    public static final String COOKIE = "Cookie";
    private IMovieItem mMovieItem;
    // / M: add for share menu {@
    private MenuItem mShareMenu;
    private ShareActionProvider mShareProvider;
    // @}

    @TargetApi(Build.VERSION_CODES.JELLY_BEAN)
    private void setSystemUiVisibility(View rootView) {
        if (ApiHelper.HAS_VIEW_SYSTEM_UI_FLAG_LAYOUT_STABLE) {
            rootView.setSystemUiVisibility(View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                    | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                    | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION);
        }
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        Log.d(TAG, "onCreate()");
        super.onCreate(savedInstanceState);
        // / M: [FEATURE.MODIFY] [Runtime permission] {@
        if (RequestPermissionActivity.startPermissionActivity(this)) {
            Log.d(TAG, "onCreate(), need start permission activity, return");
            return;
        }
        // @}
        requestWindowFeature(Window.FEATURE_ACTION_BAR);
        requestWindowFeature(Window.FEATURE_ACTION_BAR_OVERLAY);
        setVolumeControlStream(AudioManager.STREAM_MUSIC);
        setContentView(R.layout.movie_view);
        View rootView = findViewById(R.id.movie_view_root);
        setSystemUiVisibility(rootView);
        Intent intent = getIntent();
        if (!initMovieInfo(intent)) {
            Log.e(TAG, "finish activity");
            finish();
            return;
        }
        initializeActionBar(intent);
        mMovieHooker = ExtensionHelper.getHooker(this);
        mPlayer = new MoviePlayer(rootView, this, mMovieItem,
                savedInstanceState, !mFinishOnCompletion,
                intent.getStringExtra(COOKIE)) {
            @Override
            public void onCompletion() {
                Log.d(TAG, "onCompletion() mFinishOnCompletion="
                        + mFinishOnCompletion);
                if (mFinishOnCompletion) {
                    finish();
                }
            }
        };

        if (intent.hasExtra(MediaStore.EXTRA_SCREEN_ORIENTATION)) {
            int orientation = intent.getIntExtra(
                    MediaStore.EXTRA_SCREEN_ORIENTATION,
                    ActivityInfo.SCREEN_ORIENTATION_UNSPECIFIED);
            if (orientation != getRequestedOrientation()) {
                setRequestedOrientation(orientation);
            }
        }
        // set window parameters
        Window win = getWindow();
        WindowManager.LayoutParams winParams = win.getAttributes();
        winParams.buttonBrightness = WindowManager.LayoutParams.BRIGHTNESS_OVERRIDE_OFF;
        /// M: [FEATURE.ADD] Mutil-window. Do not show full screen in
        // mutil-window mode to avoid overlay with notification bar @{
        if (!isMultiWindowMode()) {
            Log.d(TAG, "onCreate(), add FLAG_FULLSCREEN");
            winParams.flags |= WindowManager.LayoutParams.FLAG_FULLSCREEN;
        }
        // @}
        win.setAttributes(winParams);
        win.setFormat(PixelFormat.TRANSLUCENT);
        // We set the background in the theme to have the launching animation.
        // But for the performance (and battery), we remove the background here.
        win.setBackgroundDrawable(null);

        if (mMovieHooker != null) {
            mMovieHooker.init(this, intent);
            mMovieHooker.setParameter(null, mMovieItem);
            mMovieHooker.setParameter(null, mPlayer);
            mMovieHooker.setParameter(null, mPlayer.getVideoSurface());
            mMovieHooker.setParameter(null, mPlayer.getPlayerWrapper());
            mMovieHooker.onCreate(savedInstanceState);
        }
    }

    @Override
    public void onStart() {
        Log.d(TAG, "onStart()");
        super.onStart();
        if (mMovieHooker != null) {
            mMovieHooker.onStart();
        }
    }

    @Override
    public void onResume() {
        Log.d(TAG, "onResume()");
        super.onResume();
        /// M: [FEATURE.ADD] Mutil-window. Refresh shareProvider when resume
        // in mutil-window to avoid share with incorrect video uri @{
        if (isMultiWindowMode()) {
            refreshShareProvider(mMovieItem);
        }
        // @}
        if (mPlayer != null) {
            mPlayer.onResume();
        }
        if (mMovieHooker != null) {
            mMovieHooker.onResume();
        }
    }

    @Override
    public void onPause() {
        Log.d(TAG, "onPause()");
        super.onPause();
        if (mPlayer != null) {
            mPlayer.onPause();
        }
        if (mMovieHooker != null) {
            mMovieHooker.onPause();
        }
    }

    @Override
    protected void onStop() {
        Log.d(TAG, "onStop()");
        super.onStop();
        if (mPlayer != null) {
            mPlayer.onStop();
        }
        if (mMovieHooker != null) {
            mMovieHooker.onStop();
        }
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "onDestroy()");
        if (mPlayer != null) {
            mPlayer.onDestroy();
        }
        if (mMovieHooker != null) {
            mMovieHooker.onDestroy();
        }
        super.onDestroy();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        super.onCreateOptionsMenu(menu);
        Log.d(TAG, "onCreateOptionsMenu");
        if (mMovieItem == null) {
            Log.w(TAG, "onCreateOptionsMenu, mMovieItem is null, return");
            return false;
        }
        boolean local = MovieUtils.isLocalFile(mMovieItem.getUri(),
                mMovieItem.getMimeType());

        if (!MovieUtils.canShare(getIntent().getExtras())
                || (local && !ExtensionHelper.getMovieDrmExtension(this)
                        .canShare(this, mMovieItem))
                || getIntent().getBooleanExtra(
                        VideoTitleHooker.SCREEN_ORIENTATION_LANDSCAPE, false)
                || !mMovieItem.canShare()) {
            Log.w(TAG, "do not show share");
        } else {
            getMenuInflater().inflate(R.menu.movie, menu);
            mShareMenu = menu.findItem(R.id.action_share);
            ShareActionProvider provider = (ShareActionProvider) mShareMenu.getActionProvider();
            mShareProvider = provider;
            refreshShareProvider(mMovieItem);
        }
        if (mMovieHooker != null) {
            return mMovieHooker.onCreateOptionsMenu(menu);
        }
        return true;
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        super.onPrepareOptionsMenu(menu);
        Log.d(TAG, "onPrepareOptionsMenu");
        if (mMovieHooker != null) {
            return mMovieHooker.onPrepareOptionsMenu(menu);
        }
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        int id = item.getItemId();
        if (id == android.R.id.home) {
            if (mTreatUpAsBack) {
                finish();
            } else {
                startActivity(new Intent(this, GalleryActivity.class));
                finish();
            }
            return true;
        } else if (id == R.id.action_share) {
            return true;
        }
        if (mMovieHooker != null) {
            return mMovieHooker.onOptionsItemSelected(item);
        }
        return true;
    }

    @Override
    public void onSaveInstanceState(Bundle outState) {
        Log.d(TAG, "onSaveInstanceState()");
        super.onSaveInstanceState(outState);
        if (mPlayer != null) {
            mPlayer.onSaveInstanceState(outState);
        }
    }

    private boolean initMovieInfo(Intent intent) {
        Uri originalUri = intent.getData();
        Log.d(TAG, "initMovieInfo, original uri is " + originalUri);
        if (originalUri == null) {
            Log.e(TAG, "initMovieInfo, acquired uri is null");
            return false;
        }
        String mimeType = intent.getType();
        mMovieItem = new DefaultMovieItem(getApplicationContext(), originalUri,
                mimeType, null);
        mFinishOnCompletion = intent.getBooleanExtra(
                MediaStore.EXTRA_FINISH_ON_COMPLETION, true);
        mTreatUpAsBack = intent.getBooleanExtra(KEY_TREAT_UP_AS_BACK, false);

        Log.d(TAG, "initMovieInfo() mMovieInfo = " + mMovieItem
                + ", mFinishOnCompletion = " + mFinishOnCompletion
                + ", mTreatUpAsBack = " + mTreatUpAsBack);
        return true;
    }

    private void setActionBarLogoFromIntent(Intent intent) {
        Bitmap logo = intent.getParcelableExtra(KEY_LOGO_BITMAP);
        if (logo != null) {
            getActionBar().setLogo(
                    new BitmapDrawable(getResources(), logo));
        }
    }

    private void initializeActionBar(Intent intent) {
        final ActionBar actionBar = getActionBar();
        if (actionBar == null) {
            return;
        }
        setActionBarLogoFromIntent(intent);
        actionBar.setDisplayOptions(ActionBar.DISPLAY_HOME_AS_UP,
                ActionBar.DISPLAY_HOME_AS_UP);
        // / M: show title for video playback
        actionBar.setDisplayOptions(actionBar.getDisplayOptions()
                | ActionBar.DISPLAY_SHOW_TITLE);
    }

    private void setActionBarTitle(String title) {
        ActionBar actionBar = getActionBar();
        Log.d(TAG, "setActionBarTitle(" + title + ") actionBar = " + actionBar);
        if (actionBar != null && title != null) {
            actionBar.setTitle(title);
        }
    }

    public boolean isMultiWindowMode() {
        boolean isMultiWindow = false;
        if (MtkVideoFeature.isMultiWindowSupport() && isInMultiWindowMode()) {
            isMultiWindow = true;
        }
        Log.d(TAG, "isInMultiWindowMode = " + isMultiWindow
                + ", sdk version = " + Build.VERSION.SDK_INT);
        return isMultiWindow;
    }

    public void refreshMovieInfo(IMovieItem info) {
        Log.d(TAG, "refreshMovieInfo(" + info + ")");
        mMovieItem = info;
        setActionBarTitle(info.getTitle());
        refreshShareProvider(info);
        mMovieHooker.setParameter(null, mMovieItem);
    }

    private void refreshShareProvider(IMovieItem info) {
        // Document says EXTRA_STREAM should be a content: Uri
        // So, we only share the video if it's "content:".
        // / M: the upper is JellyBean's comment, here we enhance the share
        // action.
        if (mShareProvider != null) {
            Intent intent = new Intent(Intent.ACTION_SEND);
            if (MovieUtils.isLocalFile(info.getUri(), info.getMimeType())) {
                intent.setType("video/*");
                intent.putExtra(Intent.EXTRA_STREAM, info.getUri());
            } else {
                intent.setType("text/plain");
                intent.putExtra(Intent.EXTRA_TEXT,
                        String.valueOf(info.getUri()));
                Log.d(TAG,
                        "share as text/plain, info.getUri() = " + info.getUri());
            }
            mShareProvider.setShareIntent(intent);
        }
        Log.d(TAG, "refreshShareProvider() mShareProvider=" + mShareProvider);
    }

    /**
     * M: MoviePlayer call this function to set IMoviePlayer
     */
    public void setMovieHookerParameter(String key, Object value) {
        Log.d(TAG, "setMovieHookerParameter key = " + key + " value = " + value);
        if (mMovieHooker != null) {
            mMovieHooker.setParameter(key, value);
        }
    }

    public IMovieItem getMovieItem() {
        return mMovieItem;
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (mPlayer != null) {
            return mPlayer.onKeyDown(keyCode, event)
                    || super.onKeyDown(keyCode, event);
        }
        return false;
    }

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        if (mPlayer != null) {
            return mPlayer.onKeyUp(keyCode, event) || super.onKeyUp(keyCode, event);
        }
        return false;
    }

    @Override
    @TargetApi(Build.VERSION_CODES.N)
    public void onMultiWindowModeChanged(boolean isInMultiWIndowMode) {
        Log.d(TAG, "[onMultiWindowModeChanged] isInMultiWIndowMode = " + isInMultiWIndowMode);
        if (isInMultiWIndowMode) {
            Log.d(TAG, "[onMultiWindowModeChanged] clear FLAG_FULLSCREEN");
            getWindow().clearFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
        } else {
            Log.d(TAG, "[onMultiWindowModeChanged] add FLAG_FULLSCREEN");
            getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
        }
    }
}
