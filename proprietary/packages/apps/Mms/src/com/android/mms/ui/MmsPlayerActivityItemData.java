/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
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
import android.util.Log;

import com.android.mms.R;

import com.mediatek.mms.util.DrmUtilsEx;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;

/** M:
 * The data structure describing each item in the MultiSave List.
 */
public class MmsPlayerActivityItemData {

    private static final String TAG = "Mms/MmsPlayerActivityItemData";
    private Bitmap mVideoThumbnail;
    private Uri mImageUri;
    private String mImageType;
    private Uri mVideoUri;
    private String mVideoType;
    private String mAudioName;
    private Uri mAudioUri;
    private String mAudioType;
    private String mText;
    private static Bitmap sThumbDefaultImage;
    private static Bitmap sThumbDefaultVideo;
    private int mImageOrVideoLeft = 0;
    private int mImageOrVideoTop = 0;
    private int mImageOrVideoWidth = 0;
    private int mImageOrVideoHeight = 0;
    private int mTextLeft = 0;
    private int mTextTop = 0;
    private int mTextWidth = 0;
    private int mTextHeight = 0;
    private boolean mIsDefaultVideoThumb = false;
    private boolean mIsDefaultImageThumb = false;

    public MmsPlayerActivityItemData(Context context, Uri imageUri, Uri videoUri,
            String audioName, String text, int imageOrVideoLeft, int imageOrVideoTop,
            int imageOrVideoWidth, int imageOrVideoHeight, int textLeft, int textTop,
            int textWidth, int textHeight, String imageType, String videoType, Uri audioUri,
            String audioType) {
        mImageUri = imageUri;
        mVideoUri = videoUri;
        mAudioName = audioName;
        mText = text;
        mImageOrVideoLeft = imageOrVideoLeft;
        mImageOrVideoTop = imageOrVideoTop;
        mImageOrVideoWidth = imageOrVideoWidth;
        mImageOrVideoHeight = imageOrVideoHeight;
        mTextLeft = textLeft;
        mTextTop = textTop;
        mTextWidth = textWidth;
        mTextHeight = textHeight;
        mImageType = imageType;
        mVideoType = videoType;
        mAudioUri = audioUri;
        mAudioType = audioType;

        final float density = context.getResources().getDisplayMetrics().density;
        mVideoThumbnail = getThumbnailFromVideoUri(mVideoUri, context,
                getDesiredThumbnailWidth(density), getDesiredThumbnailHeight(density));
    }

    public Bitmap getVideoThumbnail() {
        return mVideoThumbnail;
    }

    public String getText() {
        return mText;
    }

    public String getAudioName() {
        return mAudioName;
    }

    public Uri getImageUri() {
        return mImageUri;
    }

    public String getImageType() {
        return mImageType;
    }

    public Uri getVideoUri() {
        return mVideoUri;
    }

    public String getVideoType() {
        return mVideoType;
    }

    public Uri getAudioUri() {
        return mAudioUri;
    }

    public String getAudioType() {
        return mAudioType;
    }

    private int getDesiredThumbnailWidth(float density) {
        return (int) (100 * density);
    }

    private int getDesiredThumbnailHeight(float density) {
        return (int) (100 * density);
    }

    private Bitmap getThumbnailFromImageUri(Uri imageUri, Context context, int width, int height) {
        if (imageUri == null) {
            return null;
        }
        InputStream input = null;
        Bitmap raw = null;
        try {
            try {
                if (!DrmUtilsEx.checkHasDrmContent(context, null, imageUri)) {
                    input = context.getContentResolver().openInputStream(imageUri);
                    raw = BitmapFactory.decodeStream(input, null, null);
                } else {
                    raw = DrmUtilsEx.getDrmBitmapWithLockIcon(context, imageUri, false);
                }
            } finally {
                if (input != null) {
                    input.close();
                }
            }
        } catch (FileNotFoundException e) {
            Log.e(TAG, e.getMessage(), e);
        } catch (IOException e) {
            Log.e(TAG, e.getMessage());
        } catch (OutOfMemoryError ex) {
            MessageUtils.writeHprofDataToFile();
            throw ex;
        }
        Bitmap thumb;
        if (raw == null) {
            if (sThumbDefaultImage == null) {
                sThumbDefaultImage = BitmapFactory.decodeResource(context.getResources(),
                        R.drawable.ic_multi_save_thumb_image);
            }
            thumb = sThumbDefaultImage;
            Log.d(TAG, "getThumbnailFromImageUri IsDefaultImageThumb");
            mIsDefaultImageThumb = true;
        } else {
            thumb = raw;
            if (thumb != raw) {
                raw.recycle();
            }
        }
        return thumb;
    }
    private Bitmap getThumbnailFromVideoUri(Uri VideoUri, Context context, int width, int height) {
        if (VideoUri == null) {
            return null;
        }
        MediaMetadataRetriever retriever = new MediaMetadataRetriever();
        Bitmap raw = null;
        try {
            try {
                if (!DrmUtilsEx.checkHasDrmContent(context, null, VideoUri)) {
                    retriever.setDataSource(context, VideoUri);
                    raw = retriever.getFrameAtTime(-1);
                } else {
                    raw = DrmUtilsEx.getDrmBitmapWithLockIcon(context, VideoUri, true);
                }
            } finally {
                retriever.release();
            }
        } catch (IllegalArgumentException e) {
            // corrupted video
        } catch (RuntimeException e) {
            // corrupted video
        }
        Bitmap thumb;
        if (raw != null) {
            thumb = Bitmap.createScaledBitmap(raw, width, height, true);
            if (thumb != raw) {
                raw.recycle();
            }
        } else {
            if (sThumbDefaultVideo == null) {
                sThumbDefaultVideo = BitmapFactory.decodeResource(context.getResources(),
                        R.drawable.ic_multi_save_thumb_video);
            }
            thumb = sThumbDefaultVideo;
            Log.d(TAG, "getThumbnailFromImageUri IsDefaultVideoThumb");
            mIsDefaultVideoThumb = true;
        }
        return thumb;
    }

    public int getImageOrVideoLeft() {
        return mImageOrVideoLeft;
    }

    public int getImageOrVideoTop() {
        return mImageOrVideoTop;
    }

    public int getImageOrVideoWidth() {
        return mImageOrVideoWidth;
    }

    public int getImageOrVideoHeight() {
        return mImageOrVideoHeight;
    }

    public int getTextLeft() {
        return mTextLeft;
    }

    public int getTextTop() {
        return mTextTop;
    }

    public int getTextWidth() {
        return mTextWidth;
    }

    public int getTextHeight() {
        return mTextHeight;
    }

    public boolean isDefaultImageThumb() {
        return mIsDefaultImageThumb;
    }

    public boolean isDefaultVideoThumb() {
        return mIsDefaultVideoThumb;
    }
}
