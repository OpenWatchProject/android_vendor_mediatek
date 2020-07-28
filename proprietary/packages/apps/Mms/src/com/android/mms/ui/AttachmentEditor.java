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

import android.content.Context;
import android.content.res.Configuration;
import android.graphics.Bitmap;
import android.os.Handler;
import android.os.Message;
import android.util.AttributeSet;
import android.view.View;
import android.view.ViewStub;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.android.mms.ExceedMessageSizeException;
import com.android.mms.MmsConfig;
import com.android.mms.data.WorkingMessage;
import com.android.mms.model.SlideModel;
import com.android.mms.model.SlideshowModel;
import com.android.mms.R;
import com.android.mms.util.MmsLog;

import com.mediatek.mms.callback.IAttachmentEditorCallback;
import com.mediatek.mms.ext.IOpAttachmentEditorExt;
import com.mediatek.mms.util.DrmUtilsEx;
import com.mediatek.mms.util.FileAttachmentUtils;
import com.mediatek.opmsg.util.OpMessageUtils;

//add for attachment enhance by feng
//import packages

/**
 * This is an embedded editor/view to add photos and sound/video clips
 * into a multimedia message.
 */
public class AttachmentEditor extends LinearLayout implements IAttachmentEditorCallback {
    private static final String TAG = "AttachmentEditor";

    static final int MSG_EDIT_SLIDESHOW   = 1;
    static final int MSG_SEND_SLIDESHOW   = 2;
    static final int MSG_PLAY_SLIDESHOW   = 3;
    static final int MSG_REPLACE_IMAGE    = 4;
    static final int MSG_REPLACE_VIDEO    = 5;
    static final int MSG_REPLACE_AUDIO    = 6;
    static final int MSG_PLAY_VIDEO       = 7;
    static final int MSG_PLAY_AUDIO       = 8;
    static final int MSG_VIEW_IMAGE       = 9;
    static final int MSG_REMOVE_ATTACHMENT = 10;

    private final Context mContext;
    private Handler mHandler;

    private SlideViewInterface mView;
    private SlideshowModel mSlideshow;
    private Presenter mPresenter;
    private boolean mCanSend;
    private Button mSendButton;

    /// M: add for vCard
    private View mFileAttachmentView;

    /// OP Plugin
    IOpAttachmentEditorExt mOpAttachmentEditorExt = null;

    public AttachmentEditor(Context context, AttributeSet attr) {
        super(context, attr);
        mContext = context;
        mOpAttachmentEditorExt = OpMessageUtils.getOpMessagePlugin().getOpAttachmentEditorExt();
        mOpAttachmentEditorExt.init(this);
    }

    /**
     * Returns true if the attachment editor has an attachment to show.
     */
    public boolean update(WorkingMessage msg) {
        hideView();
        View tempView = (View) mView;
        mView = null;
        /// M: add for vcard @{
        mFileAttachmentView = null;
        mWorkingMessage = msg;
        /// @}
        // If there's no attachment, we have nothing to do.
        if (!msg.hasAttachment()) {
            return false;
        }

        // Get the slideshow from the message.
        mSlideshow = msg.getSlideshow();
        try {
            /// M: fix bug ALPS00947784, check and remove FileAttachment
            if (!mOpAttachmentEditorExt.update()) {
                checkFileAttacment(msg);
            }
            /// M: for vcard: file attachment view and other views are exclusive to each other
            if (mSlideshow.sizeOfFilesAttach() > 0) {
                mFileAttachmentView = createFileAttachmentView(msg);
                if (mFileAttachmentView != null) {
                    mFileAttachmentView.setVisibility(View.VISIBLE);
                }
            }
            //add for attachment enhance
            if (mSlideshow.size() == 0) {
                //It only has attachment but not slide
                return true;
            }
            /// M: fix bug ALPS01238218
            if (mSlideshow.size() > 1 && !msg.getIsUpdateAttachEditor()) {
                MmsLog.d(TAG, "AttachmentEditor update, IsUpdateAttachEditor == false");
                if (tempView != null && tempView instanceof SlideshowAttachmentView) {
                    tempView.setVisibility(View.VISIBLE);
                }
                return true;
            }
            mView = createView(msg);
        } catch (IllegalArgumentException e) {
            return false;
        }

        if ((mPresenter == null) || !mSlideshow.equals(mPresenter.getModel())) {
            mPresenter = PresenterFactory.getPresenter(
                    "MmsThumbnailPresenter", mContext, mView, mSlideshow);
        } else {
            mPresenter.setView(mView);
        }

        if ((mPresenter != null) && mSlideshow.size() > 1) {
            mPresenter.present(null);
        } else if (mSlideshow.size() == 1) {
            SlideModel sm = mSlideshow.get(0);
            if ((mPresenter != null) && (sm != null)
                    && (sm.hasAudio() || sm.hasImage() || sm.hasVideo())) {
                mPresenter.present(null);
            }
        }
        return true;
    }

    public void setHandler(Handler handler) {
        mHandler = handler;
        mOpAttachmentEditorExt.setHandler(handler);
    }

    public void setCanSend(boolean enable) {
        if (mCanSend != enable) {
            mCanSend = enable;
            updateSendButton();
        }
    }

    private void updateSendButton() {
        if (null != mSendButton) {
            if (mCanSend && MmsConfig.isSmsEnabled(mContext)) {
                mSendButton.setEnabled(true);
                mSendButton.setFocusable(true);
            } else {
                mSendButton.setEnabled(false);
                mSendButton.setFocusable(false);
            }
        }
        mOpAttachmentEditorExt.updateSendButton(mCanSend);
    }

    public void hideView() {
        if (mView != null) {
            ((View) mView).setVisibility(View.GONE);
        }
        /// M: add for vcard
        if (mFileAttachmentView != null) {
            mFileAttachmentView.setVisibility(View.GONE);
        }
    }

    private View getStubView(int stubId, int viewId) {
        View view = findViewById(viewId);
        if (view == null) {
            ViewStub stub = (ViewStub) findViewById(stubId);
            view = stub.inflate();
        }
        return view;
    }

    private class MessageOnClick implements OnClickListener {
        private int mWhat;

        public MessageOnClick(int what) {
            mWhat = what;
        }

        public void onClick(View v) {
            MmsLog.d(TAG, "AttachmentEditor onclick: mWhat = " + mWhat);
            Message msg = Message.obtain(mHandler, mWhat);
            msg.sendToTarget();
        }
    }

    /// M: private SlideViewInterface createView() {
    private SlideViewInterface createView(WorkingMessage msg) {

        boolean inPortrait = inPortraitMode();

        if (mSlideshow.size() > 1) {
            return createSlideshowView(inPortrait, msg);
        }

        SlideModel slide = mSlideshow.get(0);
        /// M: before using SlideModel's function,we should make sure it is null or not
        if (null == slide) {
            throw new IllegalArgumentException();
        }
        if (slide.hasImage()) {
            return createMediaView(R.id.image_attachment_view_stub, R.id.image_attachment_view,
                    R.id.view_image_button, R.id.replace_image_button,
                    R.id.remove_image_button,
                    R.id.media_size_info, msg.getCurrentMessageSize(), MSG_VIEW_IMAGE,
                    MSG_REPLACE_IMAGE, MSG_REMOVE_ATTACHMENT, msg);
        } else if (slide.hasVideo()) {
            return createMediaView(R.id.video_attachment_view_stub, R.id.video_attachment_view,
                    R.id.view_video_button, R.id.replace_video_button,
                    R.id.remove_video_button,
                    R.id.media_size_info, msg.getCurrentMessageSize(), MSG_PLAY_VIDEO,
                    MSG_REPLACE_VIDEO, MSG_REMOVE_ATTACHMENT, msg);
        } else if (slide.hasAudio()) {
            return createMediaView(R.id.audio_attachment_view_stub, R.id.audio_attachment_view,
                    R.id.play_audio_button, R.id.replace_audio_button,
                    R.id.remove_audio_button,
                    R.id.media_size_info, msg.getCurrentMessageSize(), MSG_PLAY_AUDIO,
                    MSG_REPLACE_AUDIO, MSG_REMOVE_ATTACHMENT, msg);
        } else {
            throw new IllegalArgumentException();
        }
    }


    /**
     * What is the current orientation?
     */
    private boolean inPortraitMode() {
        final Configuration configuration = mContext.getResources().getConfiguration();
        return configuration.orientation == Configuration.ORIENTATION_PORTRAIT;
    }

    private SlideViewInterface createMediaView(
            int stub_view_id, int real_view_id,
            int view_button_id, int replace_button_id, int remove_button_id,
            /// M: @{
            // int viewMessage, int replaceMessage, int removeMessage) {
            int sizeViewId, int msgSize,
            int viewMessage, int replaceMessage, int removeMessage, WorkingMessage msg) {
            /// @}
        LinearLayout view = (LinearLayout) getStubView(stub_view_id, real_view_id);
        view.setVisibility(View.VISIBLE);

        Button viewButton = (Button) view.findViewById(view_button_id);
        Button replaceButton = (Button) view.findViewById(replace_button_id);
        Button removeButton = (Button) view.findViewById(remove_button_id);
        /// M: disable when non-default sms
        boolean smsEnable = MmsConfig.isSmsEnabled(mContext);
        replaceButton.setEnabled(smsEnable);
        removeButton.setEnabled(smsEnable);

        /// M: @{
        /// M: show Mms Size
        mMediaSize = (TextView) view.findViewById(sizeViewId);
        int sizeShow = (msgSize - 1) / 1024 + 1;
        String info = sizeShow + "K/" + MmsConfig.getUserSetMmsSizeLimit(false) + "K";
        mMediaSize.setText(info);
        /// @}

        viewButton.setOnClickListener(new MessageOnClick(viewMessage));
        replaceButton.setOnClickListener(new MessageOnClick(replaceMessage));
        removeButton.setOnClickListener(new MessageOnClick(removeMessage));

        /// M: @{
        if (mFlagMini) {
            replaceButton.setVisibility(View.GONE);
        }
        /// @}

        mOpAttachmentEditorExt.createMediaView(removeButton);
        return (SlideViewInterface) view;
    }

    /// M: @{
    // private SlideViewInterface createSlideshowView(boolean inPortrait) {
    private SlideViewInterface createSlideshowView(boolean inPortrait, WorkingMessage msg) {
    /// @}
        LinearLayout view = (LinearLayout) getStubView(
                R.id.slideshow_attachment_view_stub,
                R.id.slideshow_attachment_view);
        view.setVisibility(View.VISIBLE);

        Button editBtn = (Button) view.findViewById(R.id.edit_slideshow_button);
        mSendButton = (Button) view.findViewById(R.id.send_slideshow_button);
       /// M: @{
        mSendButton.setOnClickListener(new MessageOnClick(MSG_SEND_SLIDESHOW));
        /// @}

        //updateSendButton();
        final ImageButton playBtn = (ImageButton) view.findViewById(
                R.id.play_slideshow_button);
        /// M: show Drm lock icon @{
        Bitmap drmBitmap = DrmUtilsEx.getDrmBitmapWithLockIcon(mContext, msg,
                R.drawable.mms_play_btn);
        if (drmBitmap != null) {
            playBtn.setImageBitmap(drmBitmap);
        } else {
            playBtn.setImageResource(R.drawable.mms_play_btn);
        }

        /// M: show Mms Size
        mMediaSize = (TextView) view.findViewById(R.id.media_size_info);
               int sizeShow = (msg.getCurrentMessageSize() - 1) / 1024 + 1;
        String info = sizeShow + "K/" + MmsConfig.getUserSetMmsSizeLimit(false) + "K";
        mMediaSize.setText(info);
        /// @}

        editBtn.setEnabled(true);
        editBtn.setOnClickListener(new MessageOnClick(MSG_EDIT_SLIDESHOW));
        mSendButton.setOnClickListener(new MessageOnClick(MSG_SEND_SLIDESHOW));
        playBtn.setOnClickListener(new MessageOnClick(MSG_PLAY_SLIDESHOW));
        Button removeButton = (Button) view.findViewById(R.id.remove_slideshow_button);
        removeButton.setOnClickListener(new MessageOnClick(MSG_REMOVE_ATTACHMENT));

        /// M: disable when non-default sms
        boolean smsEnable = MmsConfig.isSmsEnabled(mContext);
        editBtn.setEnabled(smsEnable);
        removeButton.setEnabled(smsEnable);

        mOpAttachmentEditorExt.createSlideshowView(mContext,
                (LinearLayout) findViewById(R.id.ct_button_slideshow_panel),
                mSendButton, removeButton);
        updateSendButton();
        return (SlideViewInterface) view;
    }

    /// M:
    private WorkingMessage mWorkingMessage;
    private TextView mMediaSize;
    private TextView mFileAttachSize;
    private boolean mFlagMini = false;

    public void update(WorkingMessage msg, boolean isMini) {
        mFlagMini = isMini;
        update(msg);
    }

    public void onTextChangeForOneSlide() throws ExceedMessageSizeException {
        if (mWorkingMessage == null || mWorkingMessage.hasSlideshow()) {
            return;
        } else {
            /// M: fix bug ALPS01270248, update FileAttachment Size
            if (mFileAttachSize != null && mWorkingMessage.hasAttachedFiles()
                    && mSlideshow != null) {
                if (!mOpAttachmentEditorExt.onTextChangeForOneSlide()) {
                    String info = MessageUtils.getHumanReadableSize(
                            mSlideshow.getCurrentSlideshowSize())
                            + "/" + MmsConfig.getUserSetMmsSizeLimit(false) + "K";
                    mFileAttachSize.setText(info);
                }
            }
        }

        if (mMediaSize == null) {
            return;
        }
        /// M: borrow this method to get the encoding type
        /// int[] params = SmsMessage.calculateLength(s, false);
        int totalSize = 0;
        if (mWorkingMessage.hasAttachment()) {
            totalSize = mWorkingMessage.getCurrentMessageSize();
        }
        /// M: show mms size
        int sizeShow = (totalSize - 1) / 1024 + 1;
        String info = sizeShow + "K/" + MmsConfig.getUserSetMmsSizeLimit(false) + "K";
        mMediaSize.setText(info);
    }

    /// M: add for vcard
    private View createFileAttachmentView(WorkingMessage msg) {
        final View view = new FileAttachmentUtils().createFileAttachmentView(
                mContext, getStubView(R.id.file_attachment_view_stub,
                        R.id.file_attachment_view), mSlideshow);
        if (view == null) {
            return null;
        }

        final ImageView remove = (ImageView) view.findViewById(R.id.file_attachment_button_remove);
        final ImageView divider = (ImageView) view.findViewById(R.id.file_attachment_divider);
        divider.setVisibility(View.VISIBLE);
        remove.setVisibility(View.VISIBLE);

        remove.setOnClickListener(new MessageOnClick(MSG_REMOVE_ATTACHMENT));
        /// M: disable when non-default sms
        boolean smsEnable = MmsConfig.isSmsEnabled(mContext);
        remove.setEnabled(smsEnable);

        mFileAttachSize = (TextView) view.findViewById(R.id.file_attachment_size_info);

        mOpAttachmentEditorExt.createFileAttachmentView(remove);
        return view;
    }

    /// M: fix bug ALPS00947784, check and remove FileAttachment
    private void checkFileAttacment(WorkingMessage msg) {
        if (msg.hasMediaAttachments() && msg.getSlideshow().sizeOfFilesAttach() > 0) {
            msg.removeAllFileAttaches();
        }
    }

    /// M: IOpAttachmentEditorCallback @{
    public void setOnClickListener(View view, int message) {
        view.setOnClickListener(new MessageOnClick(message));
    }
    /// @}
    /* Mem opt*/
    public void reset() {
        MmsLog.d(TAG, "reset");
        if (mView != null) {
            MmsLog.d(TAG, "reset1");
            mView.reset();
        }
    }
}
