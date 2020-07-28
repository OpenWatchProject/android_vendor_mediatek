/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2008 Esmertec AG.
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
 * limitations under the License.
 */

package com.android.mms.ui;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.ActivityNotFoundException;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.res.Configuration;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.media.MediaPlayer;
import android.net.Uri;
import android.provider.ContactsContract;
import android.provider.ContactsContract.Contacts;
import android.text.ClipboardManager;
import android.text.SpannableString;
import android.text.method.HideReturnsTransformationMethod;
import android.text.style.URLSpan;
import android.text.util.Linkify;
import android.util.AttributeSet;
import android.util.Log;
import android.view.ContextMenu;
import android.view.Display;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import android.view.ContextMenu.ContextMenuInfo;
import android.widget.AbsoluteLayout;
import android.widget.EditText;
import android.widget.FrameLayout;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.TextView;
import android.widget.VideoView;

import com.android.browser.provider.Browser;
import com.android.mms.MmsConfig;
import com.android.mms.data.Contact;
import com.android.mms.layout.LayoutManager;
import com.android.mms.R;
import com.android.mms.util.MmsLog;
import com.bumptech.glide.Glide;
import com.bumptech.glide.load.engine.DiskCacheStrategy;

import com.mediatek.mms.ext.IOpSlideViewExt;
import com.mediatek.mms.util.DrmUtilsEx;
import com.mediatek.opmsg.util.OpMessageUtils;
import com.mediatek.setting.SettingListActivity;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.Map;
import java.util.TreeMap;

/**
 * A basic view to show the contents of a slide.
 */
public class SlideView extends AbsoluteLayout implements
        AdaptableSlideViewInterface {
    private static final String TAG = "SlideView";
    private static final boolean DEBUG = false;
    private static final boolean LOCAL_LOGV = false;
    // FIXME: Need getHeight from mAudioInfoView instead of constant AUDIO_INFO_HEIGHT.
    private static final int AUDIO_INFO_HEIGHT = 82;

    private View mAudioInfoView;
    /// M: Code analyze 002, fix bug ALPS00099267, set ImageView to paly gif @{
    private ImageView mImageView;
    /// @}
    private VideoView mVideoView;
    private ScrollView mScrollText;
    private TextView mTextView;
    private OnSizeChangedListener mSizeChangedListener;
    private MediaPlayer mAudioPlayer;
    private boolean mIsPrepared;
    private boolean mStartWhenPrepared;
    private int     mSeekWhenPrepared;
    private boolean mStopWhenPrepared;
    private ScrollView mScrollViewPort;
    private LinearLayout mViewPort;
    // Indicates whether the view is in MMS conformance mode.
    private boolean mConformanceMode;
    private MmsMediaController mMediaController;

    /// M: Code analyze 003, fix bug ALPS00111373(new feature), support selection text copy @{
    private static final String M_TAG = "Mms/SlideView";
    private ArrayList<String> mURLs = new ArrayList<String>();

    private Activity mActivity;
    private static final int MENU_COPY_MESSAGE_TEXT         = 1;
    private static final int MENU_ADD_TO_BOOKMARK           = 2;
    private static final int MENU_ADD_TO_CONTACTS           = 3;
    private static final int MENU_ADD_ADDRESS_TO_CONTACTS   = 4;
    private static final int MENU_SEND_EMAIL                = 5;
    private static final int MENU_CALL_BACK                 = 6;
    private static final int MENU_SEND_SMS                  = 7;
    private static final int MENU_SELECT_TEXT               = 8; // add for select text copy

    public static final int REQUEST_CODE_ADD_CONTACT      = 1;
    /// @}

    /// M: Code analyze 004, new feature, Slideshow plugin (unknown) @{
    private IOpSlideViewExt mOpSlideViewExt;
    /// @}

    /// M: fix bug ALPS00952526, must displayAudioInfo() if media player can't ready for Drm Audio
    private boolean mIsHasDrmAudio = false;

    MediaPlayer.OnPreparedListener mPreparedListener = new MediaPlayer.OnPreparedListener() {
        public void onPrepared(MediaPlayer mp) {
            mIsPrepared = true;
            if (mSeekWhenPrepared > 0) {
                mAudioPlayer.seekTo(mSeekWhenPrepared);
                mSeekWhenPrepared = 0;
            }
            if (mStartWhenPrepared) {
                mAudioPlayer.start();
                mStartWhenPrepared = false;
                displayAudioInfo();
            }
            if (mStopWhenPrepared) {
                mAudioPlayer.stop();
                mAudioPlayer.release();
                mAudioPlayer = null;
                mStopWhenPrepared = false;
                hideAudioInfo();
            }
        }
    };

    public SlideView(Context context) {
        super(context);
        mOpSlideViewExt = OpMessageUtils.getOpMessagePlugin().getOpSlideViewExt();
    }

    public SlideView(Context context, AttributeSet attrs) {
        super(context, attrs);
        mOpSlideViewExt = OpMessageUtils.getOpMessagePlugin().getOpSlideViewExt();
    }

    public void setImage(String name, Bitmap bitmap) {
        if (mImageView == null) {
            /// M: Code analyze 002, fix bug ALPS00099267, @{
            mImageView = new ImageView(mContext);
            /// @}
            mImageView.setPadding(0, 5, 0, 5);
            addView(mImageView, new LayoutParams(
                    LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT, 0, 0));
            if (DEBUG) {
                mImageView.setBackgroundColor(0xFFFF0000);
            }
        }
        Glide.clear(mImageView);
        mOpSlideViewExt.setImage(mImageView, this);
        mImageView.setScaleType(ImageView.ScaleType.CENTER_INSIDE);
        try {
            if (null == bitmap) {
                bitmap = BitmapFactory.decodeResource(getResources(),
                        R.drawable.ic_missing_thumbnail_picture);
            }
            mImageView.setVisibility(View.VISIBLE);
            mImageView.setImageBitmap(bitmap);
        } catch (java.lang.OutOfMemoryError e) {
            Log.e(TAG, "setImage: out of memory: ", e);
        }
    }

    public void setImageRegion(int left, int top, int width, int height) {
        // Ignore any requirement of layout change once we are in MMS conformance mode.
        if (mImageView != null && !mConformanceMode) {
            mImageView.setLayoutParams(new LayoutParams(width, height, left, top));
        }
    }

    public void setImageRegionFit(String fit) {
        // TODO Auto-generated method stub
    }

    public void setVideo(String name, Uri video) {
        /// M: Code analyze 005, fix bug ALPS00275655, Play gif image
        /// with the matkImageView, set Image through Uri @{
        // the name == null , the uri is a animation gif
        if (name == null) {
            setImage(video);
            return;
        }
        /// @}
        if (mVideoView == null) {
            mVideoView = new VideoView(mContext);
            /// M: Code analyze 006, fix bug ALPS00249874, fix video resolution problem(unknown) @{
            mVideoView.setWillNotDraw(false);
            /// @}
            addView(mVideoView, new LayoutParams(
                    LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT, 0, 0));
            if (DEBUG) {
                mVideoView.setBackgroundColor(0xFFFF0000);
            }
        }

        if (LOCAL_LOGV) {
            MmsLog.vpi(TAG, "Changing video source to " + video);
        }
        ///M: Before setVisibility, set uri null for dcf video.
        mVideoView.setVideoURI(null);
        mVideoView.setVisibility(View.VISIBLE);
        mVideoView.setVideoURI(video);

        /// M: Code analyze 004, new feature, Slideshow plugin (unknown) @{
        mOpSlideViewExt.setVideo(mVideoView);
        /// @}
    }

    public void setMediaController(MmsMediaController mediaController) {
        mMediaController = mediaController;
    }

    private void initAudioInfoView(String name) {
        if (null == mAudioInfoView) {
            LayoutInflater factory = LayoutInflater.from(getContext());
            mAudioInfoView = factory.inflate(R.layout.playing_audio_info, null);
            int height = mAudioInfoView.getHeight();
            if (mConformanceMode) {
                /// M: Text would be incomplete in huge size.
                mViewPort.addView(mAudioInfoView,
                        new LinearLayout.LayoutParams(LayoutParams.MATCH_PARENT,
                        LayoutParams.WRAP_CONTENT));
            } else {
                addView(mAudioInfoView, new LayoutParams(
                        LayoutParams.MATCH_PARENT, AUDIO_INFO_HEIGHT,
                        0, getHeight() - AUDIO_INFO_HEIGHT));
                if (DEBUG) {
                    mAudioInfoView.setBackgroundColor(0xFFFF0000);
                }
            }

            mOpSlideViewExt.initAudioInfoView(mConformanceMode, (LinearLayout) mAudioInfoView,
                    mViewPort, this);

        }
        /// M: Code analyze 007, fix bug ALPS00072310, show an audio name in a slide @{
        TextView audioName = (TextView) mAudioInfoView.findViewById(R.id.name);
        audioName.setText(name);
        /// @}
        mAudioInfoView.setVisibility(View.GONE);
    }

    public void displayAudioInfo() {
        if (null != mAudioInfoView) {
            mAudioInfoView.setVisibility(View.VISIBLE);

            mOpSlideViewExt.displayAudioInfo();
        }
    }

    private void hideAudioInfo() {
        if (null != mAudioInfoView) {
            mAudioInfoView.setVisibility(View.GONE);
        }

        mOpSlideViewExt.hideAudioInfo();
    }

    public void setAudio(Uri audio, String name, Map<String, ?> extras) {
        if (audio == null) {
            throw new IllegalArgumentException("Audio URI may not be null.");
        }

        mOpSlideViewExt.setAudio(audio);

        if (LOCAL_LOGV) {
            MmsLog.vpi(TAG, "Changing audio source to " + audio);
        }

        if (mAudioPlayer != null) {
            mAudioPlayer.reset();
            mAudioPlayer.release();
            mAudioPlayer = null;
        }

        // Reset state variables
        mIsPrepared = false;
        /// M: Code analyze 008, fix bug ALPS00079766, init mAudioPlayer state(unknown) @{
        mStartWhenPrepared = false;
        mSeekWhenPrepared = 0;
        mStopWhenPrepared = false;
        /// @}
        mSeekWhenPrepared = 0;

        try {
            mAudioPlayer = new MediaPlayer();
            mAudioPlayer.setOnPreparedListener(mPreparedListener);
            mAudioPlayer.setDataSource(mContext, audio);
            mAudioPlayer.prepareAsync();
        } catch (IOException e) {
            Log.e(TAG, "Unexpected IOException.", e);
            mAudioPlayer.release();
            mAudioPlayer = null;
        }
        initAudioInfoView(name);

        /// M: fix bug ALPS00952526, must displayAudioInfo() for Drm Audio
        mIsHasDrmAudio = DrmUtilsEx.isDrm(name);
    }

    public void setText(String name, String text) {
        if (!mConformanceMode) {
            if (null == mScrollText) {
                mScrollText = new ScrollView(mContext);
                /// M: Code analyze 009, fix bug ALPS00111681, set ScrollBar overlay
                /// for avoiding a white padding line @{
                mScrollText.setScrollBarStyle(SCROLLBARS_OUTSIDE_OVERLAY);
                /// @}
                addView(mScrollText, new LayoutParams(
                        LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT, 0, 0));
                if (DEBUG) {
                    mScrollText.setBackgroundColor(0xFF00FF00);
                }
            }
            if (null == mTextView) {
                mTextView = new TextView(mContext);
                /// M: Code analyze 10, new feature, Adjust Text Size @{
                float textSize = MessageUtils.getPreferenceValueFloat(mContext,
                            SettingListActivity.TEXT_SIZE, 18);
                mTextView.setTextSize(textSize);
                /// @}
                /// M: Code analyze 011, fix bug ALPS00110221, black text
                /// let user more earsily to read @{
                mTextView.setTextColor(Color.BLACK);
                /// @}
                mTextView.setAutoLinkMask(
                        Linkify.WEB_URLS|Linkify.PHONE_NUMBERS|Linkify.EMAIL_ADDRESSES);
                mTextView.setLinksClickable(true);
                mTextView.setOnTouchListener(mTouchListener);
                mTextView.setGravity(0x01);
                mTextView.setTransformationMethod(HideReturnsTransformationMethod.getInstance());
                mScrollText.addView(mTextView);
            }
            mScrollText.requestFocus();
        }
        mTextView.setVisibility(View.VISIBLE);
        /// M: Code analyze 003, fix bug ALPS00111373(new feature), support menu operate @{
        mTextView.setOnCreateContextMenuListener(mTextMenuCreateListener);
        mTextView.setAutoLinkMask(Linkify.WEB_URLS|Linkify.PHONE_NUMBERS|Linkify.EMAIL_ADDRESSES);
        mTextView.setLinksClickable(true);
        mTextView.setOnTouchListener(mTouchListener);
        /// @}
        SpannableString ss = new SpannableString(text);
        mTextView.setText(ss);

        /// @}
        /// M: Code analyze 012, fix bug ALPS00116591 244151, Pop up alertDialog
        /// to ask the user whether open the URL via browser @{
        /// M: ALPS00527989, Extend TextView URL handling @ {
        mOpSlideViewExt.setText(mActivity, mTextView, mConformanceMode);
        /// @}
        /// @}
    }

    public void setTextRegion(int left, int top, int width, int height) {
        // Ignore any requirement of layout change once we are in MMS conformance mode.
        if (mScrollText != null && !mConformanceMode) {
            mScrollText.setLayoutParams(new LayoutParams(width, height, left, top));
        }
    }

    public void setVideoRegion(int left, int top, int width, int height) {
        if (mVideoView != null && !mConformanceMode) {
            mVideoView.setLayoutParams(new LayoutParams(width, height, left, top));
        }
    }

    public void setImageVisibility(boolean visible) {
        if (mImageView != null) {
            if (mConformanceMode) {
                mImageView.setVisibility(visible ? View.VISIBLE : View.GONE);
            } else {
                mImageView.setVisibility(visible ? View.VISIBLE : View.INVISIBLE);
            }
        }
    }

    public void setTextVisibility(boolean visible) {
        if (mConformanceMode) {
            if (mTextView != null) {
                mTextView.setVisibility(visible ? View.VISIBLE : View.GONE);
            }
        } else if (mScrollText != null) {
            mScrollText.setVisibility(visible ? View.VISIBLE : View.INVISIBLE);
        }
    }

    public void setVideoVisibility(boolean visible) {
        if (mVideoView != null) {
            if (mConformanceMode) {
                mVideoView.setVisibility(visible ? View.VISIBLE : View.GONE);
            } else {
                mVideoView.setVisibility(visible ? View.VISIBLE : View.INVISIBLE);
            }
        }
    }

    public void startAudio() {
        if ((mAudioPlayer != null) && mIsPrepared) {
            mAudioPlayer.start();
            mStartWhenPrepared = false;
            displayAudioInfo();
        } else {
            mStartWhenPrepared = true;
            /// M: fix bug ALPS00952526, must displayAudioInfo() for Drm Audio
            if (mIsHasDrmAudio) {
                displayAudioInfo();
            }
        }
    }

    public void stopAudio() {
        if ((mAudioPlayer != null) && mIsPrepared) {
            try {
                mAudioPlayer.stop();
            } catch (IllegalStateException e) {
                Log.e(TAG, "stopAudio IllegalStateException");
            }
            mAudioPlayer.release();
            mAudioPlayer = null;
            hideAudioInfo();
        } else {
            mStopWhenPrepared = true;
            /// M: fix bug ALPS00952526, must displayAudioInfo() for Drm Audio
            if (mIsHasDrmAudio) {
                hideAudioInfo();
            }
        }
    }

    public void pauseAudio() {
        if ((mAudioPlayer != null) && mIsPrepared) {
            if (mAudioPlayer.isPlaying()) {
                mAudioPlayer.pause();
            }
        }
        /// M: Code analyze 013, fix bug, should displayAudioInfo when pause @{
        displayAudioInfo();
        /// @}
        mStartWhenPrepared = false;
    }

    public void seekAudio(int seekTo) {
        if ((mAudioPlayer != null) && mIsPrepared) {
            mAudioPlayer.seekTo(seekTo);
        } else {
            mSeekWhenPrepared = seekTo;
        }
    }

    public void startVideo() {
        if (mVideoView != null) {
            if (LOCAL_LOGV) {
                Log.v(TAG, "Starting video playback.");
            }
            mVideoView.start();
        }
    }

    public void stopVideo() {
        if ((mVideoView != null)) {
            if (LOCAL_LOGV) {
                Log.v(TAG, "Stopping video playback.");
            }
            mVideoView.stopPlayback();
        }
    }

    public void pauseVideo() {
        if (mVideoView != null) {
            if (LOCAL_LOGV) {
                Log.v(TAG, "Pausing video playback.");
            }
            mVideoView.pause();
        }
    }

    public void seekVideo(int seekTo) {
        if (mVideoView != null) {
            if (seekTo > 0) {
                if (LOCAL_LOGV) {
                    Log.v(TAG, "Seeking video playback to " + seekTo);
                }
                mVideoView.seekTo(seekTo);
            }
        }
    }

    public void reset() {
        if (null != mScrollText) {
            mScrollText.setVisibility(View.GONE);
            /// M: Code analyze 014, fix bug, scroll should reset when reset @{
            mScrollText.scrollTo(0, 0);
            /// @}
        }

        if (null != mImageView) {
            mImageView.setVisibility(View.GONE);
        }

        if (null != mAudioPlayer || mIsHasDrmAudio) {
            stopAudio();
        }

        if (null != mVideoView) {
            stopVideo();
            mVideoView.setVisibility(View.GONE);
        }

        if (null != mTextView) {
            mTextView.setVisibility(View.GONE);
        }

        if (mScrollViewPort != null) {
            mScrollViewPort.scrollTo(0, 0);
            mScrollViewPort.setLayoutParams(
                    new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT, 0, 0));
        }
        if (mPageDividerView != null) {
            mPageDividerView.setVisibility(View.VISIBLE);
        }
    }

    public void setVisibility(boolean visible) {
        // TODO Auto-generated method stub
    }

    @Override
    protected void onSizeChanged(int w, int h, int oldw, int oldh) {
        super.onSizeChanged(w, h, oldw, oldh);

        if (mSizeChangedListener != null) {
            if (LOCAL_LOGV) {
                Log.v(TAG, "new size=" + w + "x" + h);
            }
            mSizeChangedListener.onSizeChanged(w, h - AUDIO_INFO_HEIGHT);
        }
    }

    public void setOnSizeChangedListener(OnSizeChangedListener l) {
        mSizeChangedListener = l;
    }

    private class Position {
        public Position(int left, int top) {
            mTop = top;
            mLeft = left;
        }
        public int mTop;
        public int mLeft;
    }

    /**
     * Makes the SlideView working on  MMSConformance Mode. The view will be
     * re-layout to the linear view.
     * <p>
     * This is Chinese requirement about mms conformance.
     * The most popular Mms service in China is newspaper which is MMS conformance,
     * normally it mixes the image and text and has a number of slides. The
     * AbsoluteLayout doesn't have good user experience for this kind of message,
     * for example,
     *
     * 1. AbsoluteLayout exactly follows the smil's layout which is not optimized,
     * and actually, no other MMS applications follow the smil's layout, they adjust
     * the layout according their screen size. MMS conformance doc also allows the
     * implementation to adjust the layout.
     *
     * 2. The TextView is fixed in the small area of screen, and other part of screen
     * is empty once there is no image in the current slide.
     *
     * 3. The TextView is scrollable in a small area of screen and the font size is
     * small which make the user experience bad.
     *
     * The better UI for the MMS conformance message could be putting the image/video
     * and text in a linear layout view and making them scrollable together.
     *
     * Another reason for only applying the LinearLayout to the MMS conformance message
     * is that the AbsoluteLayout has ability to play image and video in a same screen.
     * which shouldn't be broken.
     */
    /// M: Code analyze 015, fix bug ALPS00300308, Modified Text layout @{
    public void enableMMSConformanceMode(int textLeft, int textTop, int textWidth, int textHeight,
            int imageLeft, int imageTop) {
        MmsLog.d(M_TAG, "SlideView.enableMMSConformanceMode(): textLeft="
                + textLeft + ", textTop=" + textTop
                + ", imageLeft=" + imageLeft + ", imageTop" + imageTop);
        mConformanceMode = true;
        if (mScrollViewPort == null) {
            mScrollViewPort = new ScrollView(mContext) {
                private int mBottomY;
                @Override
                protected void onLayout(boolean changed, int left, int top,
                        int right, int bottom) {
                    super.onLayout(changed, left, top, right, bottom);
                    if (getChildCount() > 0) {
                        int childHeight = getChildAt(0).getHeight();
                        int height = getHeight();
                        mBottomY = height < childHeight ? childHeight - height : 0;
                    }
                }
                @Override
                protected void onScrollChanged(int l, int t, int oldl, int oldt) {
                    // Shows MmsMediaController when the view
                    // is scrolled to the top/bottom of itself.
                    if (t == 0 || t >= mBottomY) {
                        /// M: google JB.MR1 patch, Fix a BadTokenException when wakeup device
                        if (mMediaController != null
                                && !((SlideshowActivity) mContext).isFinishing()) {
                            mMediaController.show();
                        }
                    }
                }
                /// M: Code analyze 016, new feature,
                /// show MmsMediaController when touch ScrollView @{
                /** M: override the onTouchEvent method
                 *  because we use ScrollView, it will intercept ACTION_UP,
                 *  activity can not callback.
                 */
                @Override
                public boolean onTouchEvent(MotionEvent ev) {
                    boolean ret = super.onTouchEvent(ev);
                    if (ev.getAction() == MotionEvent.ACTION_UP && mMediaController != null) {
                        if (mMediaController.isShowing()) {
                            mMediaController.hide();
                        } else {
                            mMediaController.show();
                        }
                    }
                    return ret;
                }
                /// @}
            };
            /// M: Code analyze 009, fix bug ALPS00111681, set ScrollBar overlay
            /// for avoiding a white padding line @{
            mScrollViewPort.setScrollBarStyle(SCROLLBARS_INSIDE_OVERLAY);
            /// @}
            mViewPort = new LinearLayout(mContext);
            mViewPort.setOrientation(LinearLayout.VERTICAL);
            mViewPort.setGravity(Gravity.CENTER);
            mViewPort.setOnClickListener(new OnClickListener() {
                public void onClick(View v) {
                    if (mMediaController != null) {
                        if (mMediaController.isShowing()) {
                            mMediaController.hide();
                        } else {
                            mMediaController.show();
                        }
                    }
                }
            });
            mScrollViewPort.addView(mViewPort,
                    new FrameLayout.LayoutParams(LayoutParams.MATCH_PARENT,
                    LayoutParams.WRAP_CONTENT));
            addView(mScrollViewPort);
        }
        // Layout views to fit the LinearLayout from left to right, then top to
        // bottom.
        TreeMap<Position, View> viewsByPosition =
            new TreeMap<Position, View>(new Comparator<Position>() {
            public int compare(Position p1, Position p2) {
                int l1 = p1.mLeft;
                int t1 = p1.mTop;
                int l2 = p2.mLeft;
                int t2 = p2.mTop;
                int res = t1 - t2;
                if (res == 0) {
                    res = l1 - l2;
                }
                if (res == 0) {
                    // A view will be lost if return 0.
                    return -1;
                }
                return res;
            }
        });
        /// M: Code analyze 010, fix bug ALPS00267318,
        /// resolve mms slideshow text layout problem(unknown) @{
        /// M: get max window width and height
        WindowManager windowM = (WindowManager) mContext.getSystemService(Context.WINDOW_SERVICE);
        Configuration config = mContext.getResources().getConfiguration();
        Display defDisplay = windowM.getDefaultDisplay();
        int maxWidth = defDisplay.getWidth();
        /// @}

        /// M: Code analyze 015, fix bug ALPS00300308, Modified Text layout @{
        if (textLeft >= 0 && textTop >= 0) {
            mTextView = new TextView(mContext);
            mTextView.setTransformationMethod(HideReturnsTransformationMethod.getInstance());
            /// M: Code analyze 010, new feature, Adjust Text Size @{
            float textSize = MessageUtils.getPreferenceValueFloat(mContext,
                        SettingListActivity.TEXT_SIZE, 18);
            mTextView.setTextSize(textSize);
            /// @}
            mTextView.setTextColor(Color.BLACK);
            mTextView.setAutoLinkMask(
                    Linkify.WEB_URLS|Linkify.PHONE_NUMBERS|Linkify.EMAIL_ADDRESSES);
            mTextView.setLinksClickable(true);
            mTextView.setOnTouchListener(mTouchListener);
            mTextView.setGravity(0x01);
            mTextView.setPadding(5, 5, 5, 5);
            /// M: Code analyze 015, fix bug ALPS00300308, Modified Text layout @{
            int leftAbs = Math.abs((imageLeft - textLeft));
            int topAbs = Math.abs((imageTop - textTop));
            if (leftAbs > topAbs) {
                if (textWidth > 0) {
                    mTextView.setWidth(textWidth);
                } else {
                    if (textLeft > imageLeft) {
                        mTextView.setWidth(maxWidth - textLeft);
                    } else if (imageLeft > 0) {
                        mTextView.setWidth(imageLeft);
                    }
                }
            }
            /// @}
            viewsByPosition.put(new Position(textLeft, textTop), mTextView);
        }
        /// @}
        mOpSlideViewExt.enableMMSConformanceMode(mActivity, mTextView, textLeft, textTop);

        if (imageLeft >= 0 && imageTop >= 0) {
            /// M: Code analyze 002, fix bug ALPS00099267, setPadding for ImageView gif(unknown) @{
            mImageView = new ImageView(mContext);
            /// M: Code analyze 017, fix bug ALPS00096267, add/remove padding top(white line) @{
            mImageView.setPadding(0, 1, 0, 0);
            /// @}
            mImageView.setBackgroundColor(0xFFFFFFFF);
            /// @}
            viewsByPosition.put(new Position(imageLeft, imageTop), mImageView);

            // According MMS Conformance Document, the image and video should use the same
            // region. So, put the VideoView below the ImageView.
            mVideoView = new VideoView(mContext);
            /// M: Code analyze 006, fix bug ALPS00249874, fix video resolution problem(unknown) @{
            mVideoView.setWillNotDraw(false);
            /// @}
            viewsByPosition.put(new Position(imageLeft + 1, imageTop), mVideoView);
        }
        initPageDivider();
        videoViewMatchStatusbar(viewsByPosition);
    }

    private void videoViewMatchStatusbar(TreeMap<Position, View> viewsByPosition) {
        /// M: the height of video view minus the status bar
        final int statusBarHeight =
            (int) (mContext.getResources().getDisplayMetrics().density * 25 + 4);
        for (View view : viewsByPosition.values()) {
            if (view instanceof VideoView) {
                mViewPort.addView(view, new LinearLayout.LayoutParams(LayoutParams.MATCH_PARENT,
                        LayoutManager.getInstance().getLayoutParameters().getHeight()
                        - statusBarHeight));
            } else {
                mViewPort.addView(view, new LinearLayout.LayoutParams(LayoutParams.WRAP_CONTENT,
                        LayoutParams.WRAP_CONTENT));

                mOpSlideViewExt.videoViewMatchStatusbar(view, mViewPort);
            }
            view.setVisibility(View.GONE);
        }
    }

    public void setVideoThumbnail(String name, Bitmap bitmap) {
    }

    /// M: Code analyze 018, useless, @{
    public boolean hasText() {
        return (mTextView != null && mTextView.getText().length() > 0) ? true : false;
    }
    /// @}

    /// M: Code analyze 005, fix bug ALPS00275655, Play gif image
    /// with the matkImageView, set Image through Uri @{
    public void setImage(Uri imageUri) {
        if (mImageView == null) {
            mImageView = new ImageView(mContext);
            mImageView.setPadding(0, 1, 0, 0);
            mImageView.setBackgroundColor(0xFFFFFFFF);
            addView(mImageView, new LayoutParams(
                    LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT, 0, 0));
            if (DEBUG) {
                mImageView.setBackgroundColor(0xFFFF0000);
            }
        }
        Glide.clear(mImageView);
        mImageView.setVisibility(View.VISIBLE);
        mImageView.setScaleType(ImageView.ScaleType.CENTER_INSIDE);
        try {
            if (imageUri == null) {
               Bitmap bitmap = BitmapFactory.decodeResource(getResources(),
                        R.drawable.ic_missing_thumbnail_picture);
               mImageView.setImageBitmap(bitmap);
               return;
            }
            if (!MessageUtils.checkImageOK(mContext, imageUri)) {
                mImageView.setImageBitmap(MessageUtils.getDefaultBrokenImage(mContext));
            } else {
                MmsLog.d(TAG, "Slideview setImage as gif");
                Glide.with(mContext).load(imageUri.toString()).asGif()
                        .skipMemoryCache(true)
                        .diskCacheStrategy(DiskCacheStrategy.NONE)
                        .dontAnimate().into(mImageView);
            }
        } catch (java.lang.OutOfMemoryError e) {
            MmsLog.e(TAG, "setImage: out of memory: ", e);
        }
    }
    /// @}

    /// M: Code analyze 003, fix bug ALPS00111373(new feature), support selection text copy @{
    public void setActivity(Activity activity) {
        mActivity = activity;
    }

    private final OnCreateContextMenuListener mTextMenuCreateListener =
        new OnCreateContextMenuListener() {
        public void onCreateContextMenu(ContextMenu menu, View v, ContextMenuInfo menuInfo) {
            TextMenuClickListener l = new TextMenuClickListener();
            menu.clear();
            /// M: Code analyze 019, fix bug ALPS00236880, add menu dialog title @{
            menu.setHeaderTitle(R.string.message_options);
            /// @}
            menu.add(0, MENU_COPY_MESSAGE_TEXT, 0, R.string.copy_message_text)
                .setOnMenuItemClickListener(l);
            /// M: Code alalyze 020, fix bug ALPS00286716, message text select/copy/paste @{
            menu.add(0, MENU_SELECT_TEXT, 0, R.string.select_text)
                    .setOnMenuItemClickListener(l);
            /// @}
            addCallAndContactMenuItems(menu, l, mTextView.getText());
        }
    };

    private final class TextMenuClickListener implements MenuItem.OnMenuItemClickListener {
        public boolean onMenuItemClick(MenuItem item) {
            switch (item.getItemId()) {
                case MENU_COPY_MESSAGE_TEXT:
                    ClipboardManager clip =
                        (ClipboardManager) getContext()
                            .getSystemService(mContext.CLIPBOARD_SERVICE);
                    clip.setText(mTextView.getText());
                    return true;

                case MENU_ADD_TO_BOOKMARK:
                    if (mURLs.size() == 1) {
                        Browser.saveBookmark(mContext, null, mURLs.get(0));
                    } else if (mURLs.size() > 1) {
                        CharSequence[] items = new CharSequence[mURLs.size()];
                        for (int i = 0; i < mURLs.size(); i++) {
                            items[i] = mURLs.get(i);
                        }
                        new AlertDialog.Builder(mContext)
                            .setTitle(R.string.menu_add_to_bookmark)
                            .setIcon(R.drawable.ic_dialog_menu_generic)
                            .setItems(items, new DialogInterface.OnClickListener() {
                                public void onClick(DialogInterface dialog, int which) {
                                    Browser.saveBookmark(mContext, null, mURLs.get(which));
                                    }
                                })
                            .show();
                    }

                    return true;
                case MENU_ADD_ADDRESS_TO_CONTACTS:
                    Intent mAddContactIntent = item.getIntent();
                    MessageUtils.addNumberOrEmailtoContact(mAddContactIntent
                            .getStringExtra(ContactsContract.Intents.Insert.PHONE),
                            REQUEST_CODE_ADD_CONTACT, mActivity);
                    return true;

                /// M: Code alalyze 020, fix bug ALPS00286716 298189,
                /// message text select/copy/paste @{
                case MENU_SELECT_TEXT:
                    AlertDialog.Builder dialog =
                        new AlertDialog.Builder(mContext).setPositiveButton(R.string.yes, null);
                    LayoutInflater factory = LayoutInflater.from(dialog.getContext());
                    final View textEntryView =
                        factory.inflate(R.layout.alert_dialog_text_entry, null);
                    EditText contentSelector =
                        (EditText) textEntryView.findViewById(R.id.content_selector);
                    contentSelector.setText(mTextView.getText());
                    dialog.setTitle(R.string.select_text)
                                 .setView(textEntryView)
                                 .show();
                    return true;
                /// @}

                case MENU_SEND_EMAIL:
                    try {
                        getContext().startActivity(item.getIntent());
                    } catch (ActivityNotFoundException e) {
                        Intent chooserIntent = Intent.createChooser(item.getIntent(), null);
                        getContext().startActivity(chooserIntent);
                    }
                    return true;
            }
            return false;
        }
    };
    private final void addCallAndContactMenuItems(
            ContextMenu menu, TextMenuClickListener l, CharSequence text) {
        /// M: Add all possible links in the address & message
        StringBuilder textToSpannify = new StringBuilder();
        textToSpannify.append(text);

        SpannableString msg = new SpannableString(textToSpannify.toString());
        Linkify.addLinks(msg, Linkify.EMAIL_ADDRESSES|Linkify.PHONE_NUMBERS|Linkify.WEB_URLS);
        ArrayList<String> uris =
            MessageUtils.extractUris(msg.getSpans(0, msg.length(), URLSpan.class));
        mURLs.clear();
        while (uris.size() > 0) {
            String uriString = uris.remove(0);
            /// M: Remove any dupes so they don't get added to the menu multiple times
            while (uris.contains(uriString)) {
                uris.remove(uriString);
            }
            String prefix = "";
            int seperator = uriString.indexOf(":");
            if (seperator >= 0) {
                prefix = uriString.substring(0, seperator);
                if ("mailto".equalsIgnoreCase(prefix) || "tel".equalsIgnoreCase(prefix)) {
                    uriString = uriString.substring(seperator + 1);
                }
            }
            boolean addToContacts = false;
            if ("mailto".equalsIgnoreCase(prefix)) {
                String sendEmailString = mContext.getString(
                        R.string.menu_send_email).replace("%s", uriString);
                Intent intent = new Intent(Intent.ACTION_VIEW,
                        Uri.parse("mailto:" + uriString));
                intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_WHEN_TASK_RESET);
                MmsLog.ipi(TAG, "send email number: " + sendEmailString);
                menu.add(0, MENU_SEND_EMAIL, 0, sendEmailString)
                    .setOnMenuItemClickListener(l)
                    .setIntent(intent);
                addToContacts = !MessageUtils.haveEmailContact(uriString, mContext);
            } else if ("tel".equalsIgnoreCase(prefix)) {
                String callBackString = mContext.getString(
                        R.string.menu_call_back).replace("%s", uriString);
                Intent intent = new Intent(Intent.ACTION_DIAL,
                        Uri.parse("tel:" + uriString));
                intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_WHEN_TASK_RESET);
                intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                MmsLog.ipi(TAG, "call back number: " + callBackString);
                menu.add(0, MENU_CALL_BACK, 0, callBackString)
                    .setOnMenuItemClickListener(l)
                    .setIntent(intent);

                if (text != null && text.toString().replaceAll("\\-", "").contains(uriString)) {
                    String sendSmsString = mContext.getString(
                        R.string.menu_send_sms).replace("%s", uriString);
                    Intent intentSms = new Intent(Intent.ACTION_SENDTO,
                        Uri.parse("smsto:" + uriString));
                    intentSms.setClassName(mContext, "com.android.mms.ui.SendMessageToActivity");
                    intentSms.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                    MmsLog.ipi(TAG, "send sms number: " + sendSmsString);
                    /// M: disable when non-default sms
                    if (MmsConfig.isSmsEnabled(mContext)) {
                        menu.add(0, MENU_SEND_SMS, 0, sendSmsString)
                            .setOnMenuItemClickListener(l)
                            .setIntent(intentSms);
                    }
                }
                addToContacts = !isNumberInContacts(uriString);
            } else {
                /// M: add URL to book mark
                if (mURLs.size() <= 0) {
                    menu.add(0, MENU_ADD_TO_BOOKMARK, 0, R.string.menu_add_to_bookmark)
                    .setOnMenuItemClickListener(l);
                }
                mURLs.add(uriString);
            }
            if (addToContacts) {
                //Intent intent = ConversationList.createAddContactIntent(uriString);
                Intent intent = new Intent(Intent.ACTION_INSERT, Contacts.CONTENT_URI);
                intent.putExtra(ContactsContract.Intents.Insert.PHONE, uriString);
                String addContactString = mContext.getString(
                        R.string.menu_add_address_to_contacts).replace("%s", uriString);
                menu.add(0, MENU_ADD_ADDRESS_TO_CONTACTS, 0, addContactString)
                    .setOnMenuItemClickListener(l)
                    .setIntent(intent);
            }
        }
    }

    private boolean isNumberInContacts(String phoneNumber) {
        return Contact.get(phoneNumber, false).existsInDatabase();
    }
    /// @}

    /// M: add and update page divider view in slideview. @{
    private View mPageDividerView;
    private TextView mPageIndex;
    public void initPageDivider() {
        if (mPageDividerView == null) {
            LayoutInflater factory = LayoutInflater.from(getContext());
            mPageDividerView = factory.inflate(R.layout.playing_page_divider, null);
            mPageDividerView.setPadding(8, 14, 8, 14);
            if (mConformanceMode) {
                mViewPort.addView(mPageDividerView, new LinearLayout.LayoutParams(
                        LayoutParams.MATCH_PARENT, LayoutParams.WRAP_CONTENT));
            } else {
                addView(mPageDividerView, new LayoutParams(
                        LayoutParams.MATCH_PARENT, LayoutParams.WRAP_CONTENT,
                        0, 0));
            }
        }
        mPageIndex = (TextView) mPageDividerView.findViewById(R.id.page_index);
        mPageDividerView.setVisibility(View.VISIBLE);
    }
    public void setPageDividerView(int index) {
        if ((mPageDividerView == null) || (mPageIndex == null)) {
            initPageDivider();
        }
        String page = mContext.getResources().getString(R.string.page, index);
        mPageIndex.setText(page);
    }
    /// @}

    private OnTouchListener mTouchListener = new OnTouchListener() {

        @Override
        public boolean onTouch(View v, MotionEvent event) {
            if (event.getAction() == MotionEvent.ACTION_UP && mMediaController != null) {
                if (mMediaController.isShowing()) {
                    mMediaController.hide();
                } else {
                    mMediaController.show();
                }
            }
            return false;
        }
    };
}
