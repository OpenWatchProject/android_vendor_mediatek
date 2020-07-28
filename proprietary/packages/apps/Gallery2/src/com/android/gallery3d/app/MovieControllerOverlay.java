/*
 * Copyright (C) 2014 MediaTek Inc.
 * Modification based on code covered by the mentioned copyright
 * and/or permission notice(s).
 */
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

package com.android.gallery3d.app;

import android.app.Activity;
import android.content.Context;
import android.graphics.Color;
import android.graphics.Rect;
import android.graphics.drawable.Drawable;
import android.os.Handler;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.View;
import android.view.ViewParent;
import android.view.ViewGroup;
import android.view.animation.Animation;
import android.view.animation.Animation.AnimationListener;
import android.view.animation.AnimationUtils;
import android.widget.FrameLayout;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;

import com.android.gallery3d.R;
import com.mediatek.gallery3d.ext.IActivityHooker;
import com.mediatek.gallery3d.util.Log;
import com.mediatek.gallery3d.video.IMovieItem;
import com.mediatek.gallery3d.video.MediaPlayerWrapper;
import com.mediatek.gallery3d.video.MovieUtils;
import com.mediatek.gallery3d.video.ExtensionHelper;
import com.mediatek.gallery3d.video.IContrllerOverlayExt;

/**
 * The playback controller for the Movie Player.
 */
public class MovieControllerOverlay extends CommonControllerOverlay implements
        AnimationListener {

    private static final String TAG = "VP_MovieController";

    private boolean mHidden;
    private final Handler mHandler;
    private final Runnable mStartHidingRunnable;
    private final Animation mHideAnimation;
    private Context mContext;
    private OverlayExtension mOverlayExt;
    // / M: View used to show logo picture from metadata
    private ImageView mLogoView;
    private LogoViewExt mLogoViewExt = new LogoViewExt();
    private IMovieItem mMovieItem;

    public MovieControllerOverlay(Context context) {
        super(context);

        mHandler = new Handler();
        mStartHidingRunnable = new Runnable() {
            @Override
            public void run() {
                startHiding();
            }
        };

        mHideAnimation = AnimationUtils.loadAnimation(context, R.anim.player_out);
        mHideAnimation.setAnimationListener(this);

        hide();
    }

    public MovieControllerOverlay(Context context,
            MediaPlayerWrapper playerWrapper, IMovieItem movieItem) {
        super(context);
        mContext = context;
        setMediaPlayerWrapper(playerWrapper);
        mMovieItem = movieItem;
        mHandler = new Handler();
        mStartHidingRunnable = new Runnable() {
            @Override
            public void run() {
                if (mListener != null && mListener.powerSavingNeedShowController()) {
                    hide();
                } else {
                    startHiding();
                }
            }
        };

        mHideAnimation = AnimationUtils
                .loadAnimation(context, R.anim.player_out);
        mHideAnimation.setAnimationListener(this);

        mOverlayExt = new OverlayExtension();

        mLogoViewExt.init(context);
        hide();
    }

    public Animation getHideAnimation() {
        return mHideAnimation;
    }

    public boolean isPlayPauseEanbled() {
        return mPlayPauseReplayView.isEnabled();
    }

    public boolean isTimeBarEnabled() {
        return mTimeBar.getScrubbing();
    }

    public void showPlaying() {
        if (!mOverlayExt.handleShowPlaying()) {
            mState = State.PLAYING;
            showMainView(mPlayPauseReplayView);
        }
        Log.v(TAG, "showPlaying() state=" + mState);
    }

    public void showPaused() {
        if (!mOverlayExt.handleShowPaused()) {
            mState = State.PAUSED;
            showMainView(mPlayPauseReplayView);
        }
        Log.v(TAG, "showPaused() state=" + mState);
    }

    public void showEnded() {
        mOverlayExt.onShowEnded();
        mState = State.ENDED;
        showMainView(mPlayPauseReplayView);
        Log.v(TAG, "showEnded() state=" + mState);
    }

    /**
     * Show loading icon.
     *
     * @param isHttp Whether the video is a http video or not.
     */
    public void showLoading(boolean isHttp) {
        mOverlayExt.onShowLoading(isHttp);
        mState = State.LOADING;
        showMainView(mLoadingView);
        Log.v(TAG, "showLoading() state=" + mState);
    }

    public void showErrorMessage(String message) {
        mOverlayExt.onShowErrorMessage(message);
        mState = State.ERROR;
        int padding = (int) (getMeasuredWidth() * ERROR_MESSAGE_RELATIVE_PADDING);
        mErrorView.setPadding(padding, mErrorView.getPaddingTop(), padding,
                mErrorView.getPaddingBottom());
        mErrorView.setText(message);
        showMainView(mErrorView);
    }

    @Override
    protected void createTimeBar(Context context) {
        mTimeBar = new TimeBar(context, this);
        // / M: set timebar id for test case @{
        int mTimeBarId = 8;
        mTimeBar.setId(mTimeBarId);
        // / @}
    }

    @Override
    public void setTimes(int currentTime, int totalTime, int trimStartTime,
            int trimEndTime) {
        mTimeBar.setTime(currentTime, totalTime, trimStartTime, trimEndTime);
    }

    @Override
    public void hide() {
        boolean wasHidden = mHidden;
        mHidden = true;
        if (mListener == null
                || (mListener != null && !mListener.powerSavingNeedShowController())) {
            mPlayPauseReplayView.setVisibility(View.INVISIBLE);
            mLoadingView.setVisibility(View.INVISIBLE);
            // /M:pure video only show background
            if (!mOverlayExt.handleHide()) {
                setVisibility(View.INVISIBLE);
            }
            mBackground.setVisibility(View.INVISIBLE);
            mTimeBar.setVisibility(View.INVISIBLE);
        }
        // /@}
        setFocusable(true);
        requestFocus();
        if (mListener != null && wasHidden != mHidden) {
            mListener.onHidden();
        }

        Log.v(TAG, "hide() wasHidden=" + wasHidden + ", hidden=" + mHidden);
    }

    private void showMainView(View view) {
        mMainView = view;
        mErrorView.setVisibility(mMainView == mErrorView ? View.VISIBLE
                : View.INVISIBLE);
        mLoadingView.setVisibility(mMainView == mLoadingView ? View.VISIBLE
                : View.INVISIBLE);
        mPlayPauseReplayView
                .setVisibility(mMainView == mPlayPauseReplayView ? View.VISIBLE
                        : View.INVISIBLE);
        mOverlayExt.onShowMainView();
        show();
    }

    @Override
    public void show() {
        if (mListener != null) {
            boolean wasHidden = mHidden;
            mHidden = false;
            updateViews();
            setVisibility(View.VISIBLE);
            setFocusable(false);
            if (mListener != null && wasHidden != mHidden) {
                mListener.onShown();
            }
            maybeStartHiding();
            Log.v(TAG, "show() wasHidden=" + wasHidden + ", hidden="
                        + mHidden + ", listener=" + mListener);
        }
    }

    private void maybeStartHiding() {
        cancelHiding();
        if (mState == State.PLAYING) {
            mHandler.postDelayed(mStartHidingRunnable, 3000);
        }
        Log.v(TAG, "maybeStartHiding() state=" + mState);
    }

    private void startHiding() {
        startHideAnimation(mBackground);
        startHideAnimation(mTimeBar);
        startHideAnimation(mPlayPauseReplayView);
    }

    private void startHideAnimation(View view) {
        if (view.getVisibility() == View.VISIBLE) {
            view.startAnimation(mHideAnimation);
        }
    }

    private void cancelHiding() {
        mHandler.removeCallbacks(mStartHidingRunnable);
        mBackground.setAnimation(null);
        mTimeBar.setAnimation(null);
        mPlayPauseReplayView.setAnimation(null);
    }

    @Override
    public void onAnimationStart(Animation animation) {
        // Do nothing.
    }

    @Override
    public void onAnimationRepeat(Animation animation) {
        // Do nothing.
    }

    @Override
    public void onAnimationEnd(Animation animation) {
        hide();
    }

    public void onClick(View view) {
        Log.v(TAG, "onClick(" + view + ") listener=" + mListener
                    + ", state=" + mState + ", canReplay=" + mCanReplay);
        if (mListener != null) {
            if (view == mPlayPauseReplayView) {
                // / M: when state is retry connecting error, user can replay
                // video
                if (mState == State.ENDED
                        || mState == State.RETRY_CONNECTING_ERROR) {
                    mListener.onReplay();
                } else if (mState == State.PAUSED || mState == State.PLAYING) {
                    mListener.onPlayPause();
                }
            }
        }
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (mHidden) {
            show();
        }
        return super.onKeyDown(keyCode, event);
    }

    @Override
    protected void onLayout(boolean changed, int left, int top, int right,
            int bottom) {
        int width = ((Activity) mContext).getWindowManager()
                .getDefaultDisplay().getWidth();
        Rect insets = mWindowInsets;
        int pl = insets.left; // the left paddings
        int pr = insets.right;
        int pt = insets.top;
        int pb = insets.bottom;

        int h = bottom - top;
        int w = right - left;

        int y = h - pb;
        // Put both TimeBar and Background just above the bottom system
        // component.
        // But extend the background to the width of the screen, since we don't
        // care if it will be covered by a system component and it looks better.

        // Needed, otherwise the framework will not re-layout in case only the
        // padding is changed
        mBackground.layout(0, y - mTimeBar.getPreferredHeight(), w, y);
        mTimeBar.layout(pl, y - mTimeBar.getPreferredHeight(), w - pr, y);

        // Put the play/pause/next/ previous button in the center of the screen
        layoutCenteredView(mPlayPauseReplayView, 0, 0, w, h);
        layoutCenteredView(mAudioOnlyView, 0, 0, w, h);
        if (mMainView != null) {
            layoutCenteredView(mMainView, 0, 0, w, h);
        }
    }

    protected void updateViews() {
        if (mHidden) {
            return;
        }
        mBackground.setVisibility(View.VISIBLE);
        mTimeBar.setVisibility(View.VISIBLE);
        mPlayPauseReplayView
                .setImageResource(mState == State.PAUSED ? R.drawable.videoplayer_play
                        : mState == State.PLAYING ? R.drawable.videoplayer_pause
                                : R.drawable.videoplayer_reload);
        if (!mOverlayExt.handleUpdateViews()) {
            mPlayPauseReplayView.setVisibility((
                    mState != State.LOADING && mState != State.ERROR
                            && !(mState == State.ENDED && !mCanReplay)) ? View.VISIBLE : View.GONE);
        }
        requestLayout();
        Log.v(TAG, "updateViews() state=" + mState + ", canReplay="
                    + mCanReplay);
    }

    // TimeBar listener

    @Override
    public void onScrubbingStart() {
        cancelHiding();
        super.onScrubbingStart();
    }

    @Override
    public void onScrubbingMove(int time) {
        cancelHiding();
        super.onScrubbingMove(time);
    }

    @Override
    public void onScrubbingEnd(int time, int trimStartTime, int trimEndTime) {
        maybeStartHiding();
        super.onScrubbingEnd(time, trimStartTime, trimEndTime);
    }

    public IContrllerOverlayExt getOverlayExt() {
        return mOverlayExt;
    }

    private class OverlayExtension implements IContrllerOverlayExt {
        private State mLastState = State.PLAYING;
        private String mPlayingInfo;
        // The logo picture from metadata
        private Drawable mLogoPic;
        // for pause feature
        private boolean mCanPause = true;
        private boolean mEnableScrubbing = false;

        public void showBuffering(boolean fullBuffer, int percent) {
            Log.v(TAG, "showBuffering(" + fullBuffer + ", " + percent
                    + ") " + "lastState=" + mLastState + ", state="
                    + mState);
            if (fullBuffer) {
                // do not show text and loading
                mTimeBar.setSecondaryProgress(percent);
                return;
            }
            if (mState == State.PAUSED || mState == State.PLAYING) {
                mLastState = mState;
            }
            if (percent >= 0 && percent < 100) { // valid value
                mState = State.BUFFERING;
                int msgId = R.string.media_controller_buffering;
                String text = String.format(getResources().getString(msgId),
                        percent);
                mTimeBar.setInfo(text);
                showMainView(mLoadingView);
            } else if (percent == 100) {
                mState = mLastState;
                mTimeBar.setInfo(null);
                showMainView(mPlayPauseReplayView); // restore play pause state
            } else { // here to restore old state
                mState = mLastState;
                mTimeBar.setInfo(null);
            }
        }

        // set buffer percent to unknown value
        public void clearBuffering() {
            Log.v(TAG, "clearBuffering()");
            mTimeBar.setSecondaryProgress(TimeBar.UNKNOWN);
            showBuffering(false, TimeBar.UNKNOWN);
        }

        public void onCancelHiding() {
            cancelHiding();
        }

        public void showReconnecting(int times) {
            clearBuffering();
            mState = State.RETRY_CONNECTING;
            int msgId = R.string.VideoView_error_text_cannot_connect_retry;
            String text = getResources().getString(msgId, times);
            mTimeBar.setInfo(text);
            showMainView(mLoadingView);
            Log.v(TAG, "showReconnecting(" + times + ")");
        }

        public void showReconnectingError() {
            clearBuffering();
            mState = State.RETRY_CONNECTING_ERROR;
            int msgId = R.string.VideoView_error_text_cannot_connect_to_server;
            String text = getResources().getString(msgId);
            mTimeBar.setInfo(text);
            showMainView(mPlayPauseReplayView);
            Log.v(TAG, "showReconnectingError()");
        }

        public void setPlayingInfo(boolean liveStreaming) {
            int msgId;
            if (liveStreaming) {
                msgId = R.string.media_controller_live;
            } else {
                msgId = R.string.media_controller_playing;
            }
            mPlayingInfo = getResources().getString(msgId);
            Log.v(TAG, "setPlayingInfo(" + liveStreaming + ") playingInfo="
                    + mPlayingInfo);
        }

        public void setCanPause(boolean canPause) {
            this.mCanPause = canPause;
            Log.v(TAG, "setCanPause(" + canPause + ")");
        }

        public void setCanScrubbing(boolean enable) {
            mEnableScrubbing = enable;
            mTimeBar.setScrubbing(enable);
            Log.v(TAG, "setCanScrubbing(" + enable + ")");
        }

        // /M:for only audio feature.
        private boolean mAlwaysShowBottom;

        public void setBottomPanel(boolean alwaysShow, boolean foreShow) {
            mAlwaysShowBottom = alwaysShow;
            if (!alwaysShow) { // clear background
                mAudioOnlyView.setVisibility(View.INVISIBLE);
                setBackgroundColor(Color.TRANSPARENT);
                // Do not show mLogoView when change from audio-only video to
                // A/V video.
                if (mLogoPic != null) {
                    Log.v(TAG, "setBottomPanel() dissmiss orange logo picuture");
                    mLogoPic = null;
                    mLogoView.setImageDrawable(null);
                    mLogoView.setBackgroundColor(Color.TRANSPARENT);
                    mLogoView.setVisibility(View.GONE);
                }
            } else {
                // Don't set the background again when there is a logo picture
                // of the audio-only video
                if (mLogoPic != null) {
                    mAudioOnlyView.setVisibility(View.INVISIBLE);
                    mLogoView.setImageDrawable(mLogoPic);
                } else {
                    setBackgroundColor(Color.BLACK);
                    mAudioOnlyView.setVisibility(View.VISIBLE);
                }
                if (foreShow) {
                    setVisibility(View.VISIBLE);
                    // show();//show the panel
                    // hide();//hide it for jelly bean doesn't show control when
                    // enter the video.
                }
            }
            Log.v(TAG, "setBottomPanel(" + alwaysShow + ", " + foreShow + ")");
        }

        public boolean handleHide() {
            Log.v(TAG, "handleHide() mAlwaysShowBottom" + mAlwaysShowBottom);
            return mAlwaysShowBottom;
        }

        /**
         * Set the picture which get from metadata.
         *
         * @param byteArray The picture in byteArray.
         */
        public void setLogoPic(byte[] byteArray) {
            Drawable backgound = MovieUtils.bytesToDrawable(byteArray);
            setBackgroundDrawable(null);
            mLogoView.setBackgroundColor(Color.BLACK);
            mLogoView.setImageDrawable(backgound);
            mLogoView.setVisibility(View.VISIBLE);
            mLogoPic = backgound;
        }

        public boolean isPlayingEnd() {
            Log.v(TAG, "isPlayingEnd() state=" + mState);
            boolean end = false;
            if (State.ENDED == mState || State.ERROR == mState
                    || State.RETRY_CONNECTING_ERROR == mState) {
                end = true;
            }
            return end;
        }

        /**
         * Show playing information will be ignored when there is buffering
         * information updated.
         *
         * @return True if mState is changed from PLAYING to BUFFERING during
         *         showPlaying is called.
         */
        public boolean handleShowPlaying() {
            if (mState == State.BUFFERING) {
                mLastState = State.PLAYING;
                return true;
            }
            return false;
        }

        public boolean handleShowPaused() {
            mTimeBar.setInfo(null);
            if (mState == State.BUFFERING) {
                mLastState = State.PAUSED;
                return true;
            }
            return false;
        }

        /**
         * Show a information when loading or seeking
         *
         * @param isHttp Whether the video is a http video or not.
         */
        public void onShowLoading(boolean isHttp) {
            int msgId;
            if (isHttp) {
                msgId = R.string.VideoView_info_buffering;
            } else {
                msgId = R.string.media_controller_connecting;
            }
            String text = getResources().getString(msgId);
            mTimeBar.setInfo(text);
        }

        public void onShowEnded() {
            clearBuffering();
            mTimeBar.setInfo(null);
        }

        public void onShowErrorMessage(String message) {
            clearBuffering();
        }

        public boolean handleUpdateViews() {
            mPlayPauseReplayView
                    .setVisibility((mState != State.LOADING
                            && mState != State.ERROR
                            &&
                            // !(state == State.ENDED && !canReplay) && //show
                            // end when user stopped it.
                            mState != State.BUFFERING
                            && mState != State.RETRY_CONNECTING && !(mState != State.ENDED
                            && mState != State.RETRY_CONNECTING_ERROR && !mCanPause))
                            // for live streaming
                            ? View.VISIBLE
                            : View.GONE);

            if (mPlayingInfo != null && mState == State.PLAYING) {
                mTimeBar.setInfo(mPlayingInfo);
            }
            return true;
        }

        public void onShowMainView() {
            Log.v(TAG, "onShowMainView() enableScrubbing="
                    + mEnableScrubbing + ", state=" + mState);
            if (mEnableScrubbing
                    && (mState == State.PAUSED || mState == State.PLAYING)) {
                mTimeBar.setScrubbing(true);
            } else {
                mTimeBar.setScrubbing(false);
            }
        }
    }

    // /M:Add LogoView for audio-only video.
    class LogoViewExt {
        private void init(Context context) {
            if (context instanceof MovieActivity) {
                // Add logo picture
                RelativeLayout movieView = (RelativeLayout) ((MovieActivity) mContext)
                        .findViewById(R.id.movie_view_root);
                FrameLayout.LayoutParams matchParent = new FrameLayout.LayoutParams(
                        LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT,
                        Gravity.CENTER);
                mLogoView = new ImageView(mContext);
                mLogoView.setAdjustViewBounds(true);
                mLogoView.setMaxWidth(((MovieActivity) mContext)
                        .getWindowManager().getDefaultDisplay().getWidth());
                mLogoView.setMaxHeight(((MovieActivity) mContext)
                        .getWindowManager().getDefaultDisplay().getHeight());
                movieView.addView(mLogoView, matchParent);
                mLogoView.setVisibility(View.GONE);
            }
        }
    }

    // / @}

}
