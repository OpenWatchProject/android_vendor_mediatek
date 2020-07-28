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
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.media.MediaMetadataRetriever;
import android.net.Uri;
import android.text.TextUtils;
import android.text.method.HideReturnsTransformationMethod;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.android.mms.MmsConfig;
import com.android.mms.R;

import com.mediatek.mms.util.DrmUtilsEx;

import java.io.FileNotFoundException;
import java.io.InputStream;
import java.util.Map;
/* Mem opt*/
import com.android.mms.util.MmsLog;
import android.graphics.drawable.BitmapDrawable;

/**
 * A simplified view of slide in the slides list.
 */
public class SlideListItemView extends LinearLayout implements SlideViewInterface {
    private static final String TAG = "SlideListItemView";

    private TextView mTextPreview;
    private ImageView mImagePreview;
    private TextView mAttachmentName;
    private ImageView mAttachmentIcon;
    /* Mem opt*/
    private boolean mIsVideoThumb = false;
    private boolean mIsImageViewSet = false;

    public SlideListItemView(Context context) {
        super(context);
    }

    public SlideListItemView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    @Override
    protected void onFinishInflate() {
        mTextPreview = (TextView) findViewById(R.id.text_preview);
        mTextPreview.setTransformationMethod(HideReturnsTransformationMethod.getInstance());
        mImagePreview = (ImageView) findViewById(R.id.image_preview);
        mAttachmentName = (TextView) findViewById(R.id.attachment_name);
        mAttachmentIcon = (ImageView) findViewById(R.id.attachment_icon);
    }

    public void startAudio() {
        // Playing audio is not needed in this view.
    }

    public void startVideo() {
        // Playing audio is not needed in this view.
    }

    public void setAudio(Uri audio, String name, Map<String, ?> extras) {
        if (name != null) {
            mAttachmentName.setText(name);
            mAttachmentIcon.setImageResource(R.drawable.ic_mms_music);
            if (audio != null && DrmUtilsEx.checkHasDrmContent(mContext, null, audio)) {
            Bitmap bitmap = DrmUtilsEx.getDrmBitmapWithLockIcon(
                    mContext, R.drawable.slideshow_holo_light, audio);
            mImagePreview.setImageBitmap(bitmap);
        } else {
                mImagePreview.setImageResource(R.drawable.slideshow_holo_light);
            }
        } else {
            mAttachmentName.setText("");
            mAttachmentIcon.setImageDrawable(null);
        }
    }

    public void setImage(String name, Bitmap bitmap) {
        /* Mem opt*/
        MmsLog.d(TAG, "setImage name is " + name + ", this = " + this);
        try {
            if (null == bitmap) {
                mImagePreview.setImageResource(R.drawable.ic_missing_thumbnail_picture);
            } else {
                mImagePreview.setImageBitmap(bitmap);
            }
            mIsVideoThumb = false;
            mIsImageViewSet = true;
        } catch (java.lang.OutOfMemoryError e) {
            Log.e(TAG, "setImage: out of memory: ", e);
            MessageUtils.writeHprofDataToFile();
        }
    }

    public void setImageRegionFit(String fit) {
        // TODO Auto-generated method stub
    }

    public void setImageVisibility(boolean visible) {
        // TODO Auto-generated method stub
    }

    public void setText(String name, String text) {
        mTextPreview.setText(text);
        mTextPreview.setVisibility(TextUtils.isEmpty(text) ? View.GONE : View.VISIBLE);
    }

    public void setTextVisibility(boolean visible) {
        // TODO Auto-generated method stub
    }

    public void setVideo(String name, Uri video) {
        /* Mem opt*/
        MmsLog.d(TAG, "setVideo name is " + name + ", this = " + this);
        if (name != null) {
            mAttachmentName.setText(name);
            mAttachmentIcon.setImageResource(R.drawable.movie);
        } else {
            mAttachmentName.setText("");
            mAttachmentIcon.setImageDrawable(null);
        }

        /// M: Code analyze 002, fix bug ALPS00235064, got the video thumbnail
        /// through MediaMetadataRetriever @{
        MediaMetadataRetriever retriever = new MediaMetadataRetriever();
        try {
            Bitmap bitmap = null;
            if (!DrmUtilsEx.checkHasDrmContent(mContext, null, video)) {
                retriever.setDataSource(mContext, video);
                bitmap = retriever.getFrameAtTime(-1);
            } else {
                bitmap = DrmUtilsEx.getDrmBitmapWithLockIcon(mContext, video, true);
            }

            if (null == bitmap) {
                mImagePreview.setImageResource(R.drawable.ic_missing_thumbnail_video);
                mIsVideoThumb = false;
            } else {
                mImagePreview.setImageBitmap(bitmap);
                mIsVideoThumb = true;
            }
            mIsImageViewSet = true;
        } catch (RuntimeException e) {
            Log.e(TAG, "Unexpected RuntimeException.", e);
        }  catch (java.lang.OutOfMemoryError e) {
            Log.e(TAG, "Unexpected OutOfMemoryError.", e);
            MessageUtils.writeHprofDataToFile();
        } finally {
            try {
                retriever.release();
            } catch (RuntimeException ex) {
                // Ignore failures while cleaning up.
                Log.e(TAG, "Ignore failures while cleaning up.", ex);
            }
        }
        /// @}
    }

    public void setVideoThumbnail(String name, Bitmap thumbnail) {
    }

    public void setVideoVisibility(boolean visible) {
        // TODO Auto-generated method stub
    }

    public void stopAudio() {
        // Stopping audio is not needed in this view.
    }

    public void stopVideo() {
        // Stopping video is not needed in this view.
    }
    /* Mem opt*/
    public void reset() {
    }

    private void clearViews() {
        MmsLog.d(TAG, "clearViews, mIsImageViewSet = " + mIsImageViewSet);
        if (mIsImageViewSet == false) {
            return;
        }
        /* Mem opt dbg*/
        if (mIsVideoThumb == true) {
            MmsLog.d(TAG, "reset video");
            recycleBitmap(mImagePreview);
        } else {
            MmsLog.d(TAG, "reset image");
            if (mImagePreview != null) {
                mImagePreview.setImageBitmap(null);
                //recycleBitmap(mImagePreview);
            }
        }
        mIsImageViewSet = false;
    }

    public void setVisibility(boolean visible) {
        // TODO Auto-generated method stub
    }

    public void pauseAudio() {
        // TODO Auto-generated method stub

    }

    public void pauseVideo() {
        // TODO Auto-generated method stub

    }

    public void seekAudio(int seekTo) {
        // TODO Auto-generated method stub

    }

    public void seekVideo(int seekTo) {
        // TODO Auto-generated method stub

    }

    /// M: Code analyze 003, fix bug ALPS00275655, Play gif image
    /// with the matkImageView, set Image through Uri @{
    public void setImage(Uri mUri) {
        try {
            Bitmap bitmap = null;
            if (mUri != null && DrmUtilsEx.checkHasDrmContent(mContext, null, mUri)) {
                bitmap = DrmUtilsEx.getDrmBitmapWithLockIcon(mContext, mUri, false);
            } else if (null != mUri) {
                InputStream mInputStream = null;
                try {
                    mInputStream = this.getContext().getContentResolver().openInputStream(mUri);
                    if (mInputStream != null) {
                        bitmap = BitmapFactory.decodeStream(mInputStream);
                    }
                    /// M: Code analyze 004, fix bug ALPS00284522 282196, Bitmap too large
                    /// to be uploaded into a slide @{
                    bitmap = MessageUtils.getResizedBitmap(bitmap,
                            MmsConfig.getMaxImageWidth(), MmsConfig
                            .getMaxImageHeight());
                    /// @}
                } catch (FileNotFoundException e) {
                    bitmap = null;
                } finally {
                    if (mInputStream != null) {
                        mInputStream.close();
                    }
                }
            }
            setImage("", bitmap);
        } catch (java.lang.OutOfMemoryError e) {
            Log.e(TAG, "setImage(Uri): out of memory: ", e);
        } catch (Exception e) {
            Log.e(TAG, "setImage(uri) error." + e);
        }
    }
    /// @}
    /* Mem opt*/
    private void recycleBitmap(ImageView view) {
        MmsLog.d(TAG, "recycleBitmap");
        if (view == null) {
            MmsLog.d(TAG, "recycleBitmap1");
            return;
        }
        MmsLog.d(TAG, "recycleBitmap2");
        BitmapDrawable bd = (BitmapDrawable) view.getDrawable();
        view.setImageBitmap(null);
        if (bd == null) {
            MmsLog.d(TAG, "recycleBitmap3");
            return;
        }
        MmsLog.d(TAG, "recycleBitmap4");
        Bitmap bm = bd.getBitmap();
        MmsLog.d(TAG, "bm = " + bm);

        if (bm != null && !bm.isRecycled()) {
            MmsLog.d(TAG, "recycleBitmap5");
            bm.recycle();
            bm = null;
        }
        MmsLog.d(TAG, "recycleBitmap6");
    }

    @Override
    protected void onDetachedFromWindow() {
        super.onDetachedFromWindow();
        MmsLog.d(TAG, "onDetachedFromWindow" + ", this = " + this);
        clearViews();
    }
}
