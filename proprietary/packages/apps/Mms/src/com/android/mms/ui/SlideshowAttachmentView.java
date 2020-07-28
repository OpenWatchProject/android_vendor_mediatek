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
import android.net.Uri;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.android.mms.R;

import java.io.FileNotFoundException;
import java.io.InputStream;
import java.util.Map;
/* Mem opt*/
import com.android.mms.util.MmsLog;
import android.graphics.drawable.BitmapDrawable;

/**
 * This class provides an embedded editor/viewer of slide-show attachment.
 */
public class SlideshowAttachmentView extends LinearLayout implements
        SlideViewInterface {
    private static final String TAG = "SlideshowAttachmentView";

    private ImageView mImageView;
    private TextView mTextView;

    public SlideshowAttachmentView(Context context) {
        super(context);
    }

    public SlideshowAttachmentView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    @Override
    protected void onFinishInflate() {
        mImageView = (ImageView) findViewById(R.id.slideshow_image);
        mTextView = (TextView) findViewById(R.id.slideshow_text);
    }

    public void startAudio() {
        // TODO Auto-generated method stub
    }

    public void startVideo() {
        // TODO Auto-generated method stub
    }

    public void setAudio(Uri audio, String name, Map<String, ?> extras) {
        // TODO Auto-generated method stub
    }

    public void setImage(String name, Bitmap bitmap) {
        if (null == bitmap) {
            try {
                bitmap = BitmapFactory.decodeResource(getResources(),
                        R.drawable.ic_missing_thumbnail_picture);
            } catch (java.lang.OutOfMemoryError e) {
                // We don't even have enough memory to load the "missing thumbnail" image
                Log.e(TAG, "setImage(String name, Bitmap bitmap): out of memory: ", e);
                MessageUtils.writeHprofDataToFile();
            }
        }
        if (bitmap != null) {
            mImageView.setImageBitmap(bitmap);// implementation doesn't appear to be null-safe
        }
    }

    public void setImageRegionFit(String fit) {
        // TODO Auto-generated method stub
    }

    public void setImageVisibility(boolean visible) {
        mImageView.setVisibility(visible ? View.VISIBLE : View.INVISIBLE);
    }

    public void setText(String name, String text) {
        mTextView.setText(text);
    }

    public void setTextVisibility(boolean visible) {
        mTextView.setVisibility(visible ? View.VISIBLE : View.INVISIBLE);
    }

    public void setVideo(String name, Uri video) {
        Bitmap bitmap = VideoAttachmentView.createVideoThumbnail(mContext, video);
        if (null == bitmap) {
            try {
                bitmap = BitmapFactory.decodeResource(getResources(),
                        R.drawable.ic_missing_thumbnail_video);
            } catch (java.lang.OutOfMemoryError e) {
                // We don't even have enough memory to load the
                // "missing thumbnail" image
                Log.e(TAG, "setVideo(Uri): out of memory: ", e);
                MessageUtils.writeHprofDataToFile();
            }
        }
        mImageView.setImageBitmap(bitmap);
    }

    public void setVideoVisibility(boolean visible) {
        // TODO Auto-generated method stub
    }

    public void stopAudio() {
        // TODO Auto-generated method stub
    }

    public void stopVideo() {
        // TODO Auto-generated method stub
    }

    public void reset() {
        /* Mem opt*/
        MmsLog.d(TAG, "reset");
        if (mImageView != null) {
            //recycleBitmap(mImageView);
            mImageView.setImageBitmap(null);
        }
        mTextView.setText("");
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

    public void setVideoThumbnail(String name, Bitmap bitmap) {
        /// M: @{
        if (bitmap != null) {
            mImageView.setImageBitmap(bitmap);
        }
        /// @}
    }

    /// M: Code analyze 001, fix bug ALPS00275655, Play gif image
    /// with the matkImageView, set Image through Uri @{
    @Override
    public void setImage(Uri mUri) {
        try {
            Bitmap bitmap = null;
            if (null == mUri) {
                bitmap = BitmapFactory.decodeResource(getResources(),
                        R.drawable.ic_missing_thumbnail_picture);
            } else {
                InputStream mInputStream = null;
                try {
                    mInputStream = this.getContext().getContentResolver().openInputStream(mUri);
                    if (mInputStream != null) {
                        bitmap = BitmapFactory.decodeStream(mInputStream);
                    }
                } catch (FileNotFoundException e) {
                    bitmap = null;
                } finally {
                    if (mInputStream != null) {
                        mInputStream.close();
                    }
                }
                setImage("", bitmap);
            }
        } catch (java.lang.OutOfMemoryError e) {
            Log.e(TAG, "setImage(Uri): out of memory: ", e);
            MessageUtils.writeHprofDataToFile();
        } catch (Exception e) {
            Log.e(TAG, "setImage(uri) error." + e);
        }
    }
    /// @}
}
