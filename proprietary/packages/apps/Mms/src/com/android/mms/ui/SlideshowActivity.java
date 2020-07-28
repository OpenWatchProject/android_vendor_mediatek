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
import android.content.Intent;
import android.content.res.Configuration;
import android.graphics.Color;
import android.graphics.PixelFormat;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.Window;
import android.view.View.OnClickListener;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;

import com.android.mms.dom.AttrImpl;
import com.android.mms.dom.smil.SmilDocumentImpl;
import com.android.mms.dom.smil.SmilPlayer;
import com.android.mms.dom.smil.parser.SmilXmlSerializer;
import com.android.mms.model.LayoutModel;
import com.android.mms.model.RegionModel;
import com.android.mms.model.SlideshowModel;
import com.android.mms.model.SmilHelper;
import com.android.mms.R;
import com.android.mms.util.MmsLog;
import com.google.android.mms.MmsException;

import com.mediatek.mms.util.PermissionCheckUtil;
import com.mediatek.opmsg.util.OpMessageUtils;

import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.w3c.dom.events.Event;
import org.w3c.dom.events.EventListener;
import org.w3c.dom.events.EventTarget;
import org.w3c.dom.smil.SMILDocument;
import org.w3c.dom.smil.SMILElement;

import java.io.ByteArrayOutputStream;

/**
 * Plays the given slideshow in full-screen mode with a common controller.
 */
public class SlideshowActivity extends Activity implements EventListener {
    private static final String TAG = "SlideshowActivity";
    private static final boolean DEBUG = false;
    private static final boolean LOCAL_LOGV = false;

    private MmsMediaController mMediaController;
    private SmilPlayer mSmilPlayer;

    private Handler mHandler;

    private SMILDocument mSmilDoc;

    private SlideView mSlideView;
    private int mSlideCount;
    /// M: Code analyze 002, fix bug ALPS00111374, whether need resume @{
    private static boolean bNeedResume = false;
    /// @}

    /// M: Code analyze 003, fix bug ALPS00119632, present slide return
    /// (mActivityRunning=false) when SlideshowActivity stop @{
    private SmilPlayerController mSmilPlayerController;
    private SlideshowPresenter mPresenter;
    /// @}

    private boolean mRotate = false;

    /// M: ALPS02322990. Finish this activity in onResume instead of noHistory property.
    private boolean mNeedFinish = false;

    /**
     * @return whether the Smil has MMS conformance layout.
     * Refer to MMS Conformance Document OMA-MMS-CONF-v1_2-20050301-A
     */
    private static final boolean isMMSConformance(SMILDocument smilDoc) {
        SMILElement head = smilDoc.getHead();
        if (head == null) {
            // No 'head' element
            return false;
        }
        NodeList children = head.getChildNodes();
        if (children == null || children.getLength() != 1) {
            // The 'head' element should have only one child.
            return false;
        }
        Node layout = children.item(0);
        if (layout == null || !"layout".equals(layout.getNodeName())) {
            // The child is not layout element
            return false;
        }
        NodeList layoutChildren = layout.getChildNodes();
        if (layoutChildren == null) {
            // The 'layout' element has no child.
            return false;
        }
        int num = layoutChildren.getLength();
        if (num <= 0) {
            // The 'layout' element has no child.
            return false;
        }
        for (int i = 0; i < num; i++) {
            Node layoutChild = layoutChildren.item(i);
            if (layoutChild == null) {
                // The 'layout' child is null.
                return false;
            }
            String name = layoutChild.getNodeName();
            if ("root-layout".equals(name)) {
                continue;
            } else if ("region".equals(name)) {
                NamedNodeMap map = layoutChild.getAttributes();
                for (int j = 0; j < map.getLength(); j++) {
                    Node node = map.item(j);
                    if (node == null) {
                        return false;
                    }
                    String attrName = node.getNodeName();
                    // The attr should be one of left, top, height, width, fit and id
                    if ("left".equals(attrName) || "top".equals(attrName) ||
                            "height".equals(attrName) || "width".equals(attrName) ||
                            "fit".equals(attrName)) {
                        continue;
                    } else if ("id".equals(attrName)) {
                        String value;
                        if (node instanceof AttrImpl) {
                            value = ((AttrImpl) node).getValue();
                        } else {
                            return false;
                        }
                        if ("Text".equals(value) || "Image".equals(value)) {
                            continue;
                        } else {
                            // The id attr is not 'Text' or 'Image'
                            return false;
                        }
                    } else {
                        return false;
                    }
                }
            } else {
                // The 'layout' element has the child other than 'root-layout' or 'region'
                return false;
            }
        }
        return true;
    }

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        if (PermissionCheckUtil.requestRequiredPermissions(this)) {
            return;
        }
        mHandler = new Handler();
        /// @}
        // Play slide-show in full-screen mode.
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        getWindow().setFormat(PixelFormat.TRANSLUCENT);
        setContentView(R.layout.slideshow);

        Intent intent = getIntent();
        Uri msg = intent.getData();
        final SlideshowModel model;

        try {
            model = SlideshowModel.createFromMessageUri(this, msg);
            mSlideCount = model.size();
            /** M: Confirm that we have at least 1 supported_slide to display @{ */
            if (mSlideCount == 0) {
                Toast.makeText(this,
                        getString(R.string.unsupport_media_type), Toast.LENGTH_SHORT).show();
                throw new MmsException();
            }
            /** @} */
        } catch (MmsException e) {
            Log.e(TAG, "Cannot present the slide show.", e);
            finish();
            return;
        }
        mSlideView = (SlideView) findViewById(R.id.slide_view);

        /// M: Code analyze 003, fix bug ALPS00119632, present slide return
        /// (mActivityRunning=false) when SlideshowActivity stop @{
        mPresenter = (SlideshowPresenter) PresenterFactory.getPresenter(
                "SlideshowPresenter", this, mSlideView, model);
        /// @}
        mPresenter.setIsSlideshowPlayMode(true);
        mRotate = true;

        /// M: Code analyze 005, fix bug ALPS00071669, Handel delay problem
        /// when progress bar update @{
        mSmilPlayer = SmilPlayer.getPlayer();
        initMediaController();
        mSlideView.setMediaController(mMediaController);
        mSlideView.setActivity(this);
        // Use SmilHelper.getDocument() to ensure rebuilding the
        /// M: entire SMIL document.
        mSmilDoc = SmilHelper.getDocument(model);
        /// @}
        mHandler.post(new Runnable() {
            private boolean isRotating() {
                return mSmilPlayer.isPausedState()
                        || mSmilPlayer.isPlayingState()
                        || mSmilPlayer.isPlayedState();
            }

            public void run() {
                if (isMMSConformance(mSmilDoc)) {
                    int imageLeft = 0;
                    int imageTop = 0;
                    int textLeft = 0;
                    int textTop = 0;
                    /// M: Code analyze 006, fix bug ALPS00300308, Modified Text layout @{
                    int textWidth = 0;
                    int textHeight = 0;
                    /// @}
                    LayoutModel layout = model.getLayout();
                    if (layout != null) {
                        RegionModel imageRegion = layout.getImageRegion();
                        if (imageRegion != null) {
                            imageLeft = imageRegion.getLeft();
                            imageTop = imageRegion.getTop();
                        }
                        RegionModel textRegion = layout.getTextRegion();
                        if (textRegion != null) {
                            textLeft = textRegion.getLeft();
                            textTop = textRegion.getTop();
                        }
                    }
                    /// M: Code analyze 006, fix bug ALPS00300308, Modified Text layout @{
                    mSlideView.enableMMSConformanceMode(
                            textLeft, textTop, textWidth, textHeight, imageLeft, imageTop);
                    /// @}
                } else {
                    /// M: init page divider view
                    mSlideView.initPageDivider();
                }
                if (DEBUG) {
                    ByteArrayOutputStream ostream = new ByteArrayOutputStream();
                    SmilXmlSerializer.serialize(mSmilDoc, ostream);
                    if (LOCAL_LOGV) {
                        Log.v(TAG, ostream.toString());
                    }
                }

                // Add event listener.
                ((EventTarget) mSmilDoc).addEventListener(
                        SmilDocumentImpl.SMIL_DOCUMENT_END_EVENT,
                        SlideshowActivity.this, false);

                mSmilPlayer.init(mSmilDoc);
                if (isRotating()) {
                    mSmilPlayer.reload();
                } else {
                    mSmilPlayer.play();
                    bNeedResume = true;
                }
            }
        });
    }

    private void initMediaController() {
        mMediaController = new MmsMediaController(SlideshowActivity.this, false);
        /// M: Code analyze 002, fix bug ALPS00111374, (CMCC feature) pause / not auto play
        /// but need prepareToPlay when enter the slideshow activity @{
        mSmilPlayerController = new SmilPlayerController(mSmilPlayer);
        mMediaController.setMediaPlayer(mSmilPlayerController);
        /// @}
        mMediaController.setAnchorView(findViewById(R.id.slide_view));
        /// M: Code analyze 007, fix bug ALPS00111602, set Color for showing clearly @{
        mMediaController.setBackgroundColor(Color.BLACK);
        TextView currentTime = (TextView) mMediaController.findViewById(R.id.time_current);
        TextView time = (TextView) mMediaController.findViewById(R.id.time);
        currentTime.setTextColor(Color.WHITE);
        time.setTextColor(Color.WHITE);
        /// @}
        mMediaController.setPrevNextListeners(
            new OnClickListener() {
              public void onClick(View v) {
                  /// M: Code analyze 012, new feature, check MmsMediaController show @{
                  if ((mSmilPlayer != null) && (mMediaController != null)) {
                      mMediaController.show();
                  }
                  /// @}
                  mSmilPlayer.next();
              }
            },
            new OnClickListener() {
              public void onClick(View v) {
                  /// M: Code analyze 012, new feature, check MmsMediaController show @{
                  if ((mSmilPlayer != null) && (mMediaController != null)) {
                      mMediaController.show();
                  }
                  if (mSmilPlayer.getCurrentSlide() <= 1) {
                      return;
                  }
                  /// @}
                  mSmilPlayer.prev();
              }
            });
    }

    @Override
    public boolean onTouchEvent(MotionEvent ev) {
        /// M: Code analyze 012, new feature, check MmsMediaController show @{
        if ((ev.getAction() == MotionEvent.ACTION_UP)
                && (mSmilPlayer != null) && (mMediaController != null)) {
            mMediaController.show();
        }
        /// @}
        return false;
    }

    @Override
    protected void onPause() {
        super.onPause();
        /// M: Code analyze 008, fix bug ALPS00236567, hide the mediaControl when activity paused
        /// so onTouchEvent excute show @{
        if (mMediaController != null) {
            /// M: Must do this so we don't leak a window.
            mMediaController.hide();
        }
        /// @}
        if (mSmilDoc != null) {
            ((EventTarget) mSmilDoc).removeEventListener(
                    SmilDocumentImpl.SMIL_DOCUMENT_END_EVENT, this, false);
        }
        /// M: Code analyze 002, fix bug ALPS00111374, whether need resume @{
        if ((null != mSmilPlayer)) {
            if (mSmilPlayer.isPlayingState()) {
                mSmilPlayer.pause();
                bNeedResume = true;
            } else if (mSmilPlayer.isPausedState()) {
                bNeedResume = false;
            }
        }
        /// @}
    }

    @Override
    protected void onStop() {
        super.onStop();
        /// M: Code analyze 009, fix bug ALPS00335430, should close in lockscreen @{
        /** M: when this activity is invisiable, it should exit.
         *  though it has nohistory property, lock screen is an exception.
         *  so we must destory this activity in onStop.
         */
        //finish();
        /// @}
        if ((null != mSmilPlayer)) {
            if (isFinishing()) {
                mSmilPlayer.stop();
            } else {
                mSmilPlayer.stopWhenReload();
            }
            if (mMediaController != null) {
                // Must set the seek bar change listener null, otherwise if we rotate it
                // while tapping progress bar continuously, window will leak.
                View seekBar = mMediaController
                        .findViewById(com.android.internal.R.id.mediacontroller_progress);
                if (seekBar instanceof SeekBar) {
                    ((SeekBar) seekBar).setOnSeekBarChangeListener(null);
                }
                // Must do this so we don't leak a window.
                mMediaController.hide();
            }
        }
        /// M: Code analyze 003, fix bug ALPS00119632, present slide return
        /// (mActivityRunning=false) when SlideshowActivity stop @{
        if (mPresenter != null) {
            mPresenter.onStop();
            mPresenter = null;
        }
        /// @}
    }

    @Override
    protected void onDestroy() {
        if (mSlideView != null) {
            mSlideView.setMediaController(null);
        }
        super.onDestroy();
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        switch (keyCode) {
            case KeyEvent.KEYCODE_VOLUME_DOWN:
            case KeyEvent.KEYCODE_VOLUME_UP:
            case KeyEvent.KEYCODE_VOLUME_MUTE:
            case KeyEvent.KEYCODE_DPAD_UP:
            case KeyEvent.KEYCODE_DPAD_DOWN:
            case KeyEvent.KEYCODE_DPAD_LEFT:
            case KeyEvent.KEYCODE_DPAD_RIGHT:
            /// M: Code analyze 010, fix bug ALPS00264030, no response for menu key @{
            case KeyEvent.KEYCODE_MENU:
            /// @}
                break;
            case KeyEvent.KEYCODE_BACK:
                if ((mSmilPlayer != null) &&
                        (mSmilPlayer.isPausedState()
                        || mSmilPlayer.isPlayingState()
                        || mSmilPlayer.isPlayedState())) {
                    mSmilPlayer.stop();
                }
                break;
            default:
                if ((mSmilPlayer != null) && (mMediaController != null)) {
                    mMediaController.show();
                }
        }
        return super.onKeyDown(keyCode, event);
    }

    private class SmilPlayerController implements MmsMediaController.MediaPlayerControl {
        private final SmilPlayer mPlayer;
        /**
         * We need to cache the playback state because when the MmsMediaController issues a play or
         * pause command, it expects subsequent calls to {@link #isPlaying()} to return the right
         * value immediately. However, the SmilPlayer executes play and pause asynchronously, so
         * {@link #isPlaying()} will return the wrong value for some time. That's why we keep our
         * own version of the state of whether the player is playing.
         *
         * Initialized to true because we always programatically start the SmilPlayer upon creation
         */
        private boolean mCachedIsPlaying = true;

        public SmilPlayerController(SmilPlayer player) {
            mPlayer = player;
        }

        public int getBufferPercentage() {
            // We don't need to buffer data, always return 100%.
            return 100;
        }

        public int getCurrentPosition() {
            /// M: Code analyze 005, fix bug ALPS00071669, Handel delay problem
            /// when progress bar update @{
            if (mPlayer != null) {
                return mPlayer.getCurrentPosition();
            } else {
                return 0;
            }
            /// @}
        }

        public int getDuration() {
            return mPlayer.getDuration();
        }

        public boolean isPlaying() {
            return mCachedIsPlaying;
        }

        public void pause() {
            mPlayer.pause();
            mCachedIsPlaying = false;
        }

        public void seekTo(int pos) {
            // Don't need to support.
        }

        public void start() {
            mPlayer.start();
            mCachedIsPlaying = true;
        }

        public boolean canPause() {
            return true;
        }

        public boolean canSeekBackward() {
            return true;
        }

        public boolean canSeekForward() {
            return true;
        }
    }

    public void handleEvent(Event evt) {
        final Event event = evt;
        mHandler.post(new Runnable() {
            public void run() {
                String type = event.getType();
                if (type.equals(SmilDocumentImpl.SMIL_DOCUMENT_END_EVENT)) {
                    finish();
                }
            }
        });
    }

    /// M: Code analyze 011, new feature, hide MmsMediaController @{
    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        // TODO Auto-generated method stub
        super.onConfigurationChanged(newConfig);
        if ((mSmilPlayer != null) && (mMediaController != null)) {
            mMediaController.hide();
        }
    }
    /// @}

    @Override
    protected void onResume() {
        super.onResume();
        if (mNeedFinish) {
            Log.d(TAG, "onResume finish");
            finish();
            return;
        }
        mNeedFinish = true;

        if (mMediaController != null) {
            //mMediaController.setMdragging(false);
            mMediaController.hide();
        }
        if (mSmilDoc != null) {
            ((EventTarget) mSmilDoc).addEventListener(
                    SmilDocumentImpl.SMIL_DOCUMENT_END_EVENT,
                    SlideshowActivity.this, false);
        }
        /// M: Code analyze 002, fix bug ALPS00111374, whether need resume @{
        if (!bNeedResume) {
            mRotate = false;
            return;
        }
        /// @}
        if (null == mSmilPlayer) {
            mSmilPlayer = SmilPlayer.getPlayer();
        }
        if (null != mSmilPlayer) {
            if (!isFinishing()) {
                if (mSmilPlayer.isPausedState()) {
                    if (mRotate) {
                        // if need resume the player, set the state playing.
                        mSmilPlayer.setStateStart();
                    } else {
                        mSmilPlayer.start();
                    }
                }
            }
        }
        mRotate = false;
    }
}
