/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

import android.content.ActivityNotFoundException;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.Typeface;
import android.net.Uri;
import android.text.SpannableString;
import android.text.SpannableStringBuilder;
import android.text.style.StyleSpan;
import android.util.DisplayMetrics;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.android.mms.R;
import com.android.mms.util.MmsContentType;
import com.android.mms.util.MmsLog;
import com.bumptech.glide.Glide;
import com.bumptech.glide.load.engine.DiskCacheStrategy;

import com.mediatek.mms.ext.IOpMmsPlayerActivityAdapterExt;
import com.mediatek.mms.util.DrmUtilsEx;
import com.mediatek.opmsg.util.OpMessageUtils;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
/* Mem opt*/
import android.graphics.drawable.BitmapDrawable;

/** M:
 * The back-end data adapter for MmsPlayerActivity.
 */
public class MmsPlayerActivityAdapter extends BaseAdapter {
    private static final String TAG = "Mms/MmsPlayerAdapter";
    private static final boolean LOCAL_LOGV = false;

    private final LayoutInflater mFactory;
    private ArrayList<MmsPlayerActivityItemData> mListItem;
    private int mAllCount;
    private Context mContext;
    private float textSize = 0;
    private DisplayMetrics mDisplayMetrics;
    private HashMap<Integer, View> mListItemViewCache = new HashMap<Integer, View>();

    /// M: ALPS00619099, highlight matched search string in mms player @ {
    private Pattern mHighlight = null;
    /// @}
    /// M: ALPS00994554, for showing notification when
    /// view mms with video player in full screen model.
    private static final String EXTRA_FULLSCREEN_NOTIFICATION =
        "mediatek.intent.extra.FULLSCREEN_NOTIFICATION";

    /// M: ALPS01586782, more click issue @ {
    private boolean sIsClick = false;
    ///@}

    private IOpMmsPlayerActivityAdapterExt mOpMmsPlayerActivityAdapterExt;

    public void setTextSize(float size) {
        textSize = size;
    }

    public MmsPlayerActivityAdapter(Context context,
            ArrayList<MmsPlayerActivityItemData> listItem) {
        mFactory = LayoutInflater.from(context);
        mListItem = listItem;
        mAllCount = mListItem.size();
        mContext = context;
        mDisplayMetrics = context.getResources().getDisplayMetrics();
        mOpMmsPlayerActivityAdapterExt = OpMessageUtils.getOpMessagePlugin()
                .getOpMmsPlayerActivityAdapterExt();
    }

    public void onMovedToScrapHeap(View view) {
    }

    public ArrayList<MmsPlayerActivityItemData> getListItem() {
        return mListItem;
    }

    @Override
    public int getCount() {
        return mAllCount;
    }

    @Override
    public MmsPlayerActivityItemData getItem(int arg0) {
        return mListItem.get(arg0);
    }

    @Override
    public long getItemId(int arg0) {
        return 0;
    }

    @Override
    public View getView(int arg0, View arg1, ViewGroup arg2) {
        MmsLog.i(TAG, "getView, for position " + arg0);
        MmsPlayerActivityItem itemView = (MmsPlayerActivityItem) mListItemViewCache.get(arg0);
        if (itemView != null) {
            String text = mListItem.get(arg0).getText();
            if (text != null) {
                TextView mText = itemView.getCurrentTextView();
                MmsLog.i(TAG, "getView(): text view is null? " + (mText == null));
                if (mText != null && textSize != 0) {
                    mText.setTextSize(textSize);
                }
            }
            return itemView;
        }
        MmsLog.i(TAG, "getView(): create new one.");

        itemView =
            (MmsPlayerActivityItem) mFactory.inflate(R.layout.mms_player_activity_item, null);

        TextView mPageIndex = (TextView) itemView.findViewById(R.id.page_index);
        ImageView mImage = (ImageView) itemView.findViewById(R.id.image);
        ImageView mVideo = (ImageView) itemView.findViewById(R.id.video);
        ImageView mVideoPlayBtn = (ImageView) itemView.findViewById(R.id.video_play_overlay);
        View mAudio = (View) itemView.findViewById(R.id.audio);
        TextView mAudioName = (TextView) itemView.findViewById(R.id.audio_name);
        ImageView mAudioIcon = (ImageView) itemView.findViewById(R.id.audio_icon);
        TextView mText = (TextView) itemView.findViewById(R.id.top_text);

        final MmsPlayerActivityItemData item = mListItem.get(arg0);

        // show page index
        String index = mContext.getResources().getString(R.string.page, arg0 + 1);
        mPageIndex.setText(index);

        // show image
        final Uri imageUri = item.getImageUri();
        final String imageType = item.getImageType();

        String audioName = item.getAudioName();
        final Uri audioUri = item.getAudioUri();
        String audioType = item.getAudioType();

        // show video thumbnail
        Bitmap t = item.getVideoThumbnail();
        final Uri videoUri = item.getVideoUri();
        final String videoType = item.getVideoType();

        if (imageUri != null) {
            MmsLog.ipi(TAG, "MmsplayerActivity set image: "
                    + imageUri + " image type = " + imageType);
            if (DrmUtilsEx.checkHasDrmContent(mContext, null, imageUri)) {
                Bitmap bitmap = DrmUtilsEx.getDrmBitmapWithLockIcon(mContext, imageUri, false);
                mImage.setImageBitmap(bitmap);
            } else if (!MessageUtils.checkImageOK(mContext, imageUri)) {
                mImage.setImageBitmap(MessageUtils.getDefaultBrokenImage(mContext));
            } else {
                if (imageType.equalsIgnoreCase(MmsContentType.IMAGE_GIF)) {
                    Glide.with(mContext).load(imageUri.toString()).asGif()
                            .skipMemoryCache(true).diskCacheStrategy(DiskCacheStrategy.NONE)
                            .dontAnimate().into(mImage);
                } else {
                    Glide.with(mContext).load(imageUri.toString())
                            .skipMemoryCache(true).diskCacheStrategy(DiskCacheStrategy.NONE)
                            .into(mImage);
                }
            }
            mImage.setVisibility(View.VISIBLE);
            mImage.setClickable(true);
            mImage.setOnClickListener(new mediaClick(imageUri, imageType, ""));
        } else {
            mImage.setVisibility(View.GONE);
        }


        if (t != null) {
            MmsLog.ipi(TAG, "MmsplayerActivity set video: " + videoUri);
            mVideo.setImageBitmap(t);
            mVideo.setVisibility(View.VISIBLE);
            mVideoPlayBtn.setVisibility(View.VISIBLE);
            mVideo.setClickable(true);
            mVideo.setOnClickListener(new mediaClick(videoUri, videoType, ""));
        } else {
            mVideo.setVisibility(View.GONE);
        }


        if (audioName != null) {
            MmsLog.ipi(TAG, "MmsplayerActivity show audio name:" + audioName);
            mAudioName.setText(audioName);
            mAudioName.setTextSize(18);
            mAudioIcon.setVisibility(View.VISIBLE);
            mAudio.setVisibility(View.VISIBLE);
            mAudioName.setVisibility(View.VISIBLE);
            mAudioName.setClickable(true);
            mAudioName.setOnClickListener(new mediaClick(audioUri, audioType, audioName));
        } else {
            mAudioIcon.setVisibility(View.GONE);
            mAudioName.setVisibility(View.GONE);
            mAudio.setVisibility(View.GONE);
        }

        mOpMmsPlayerActivityAdapterExt.getView(imageUri, imageType, mImage, t, videoType, mVideo,
                audioName, audioType, mAudioIcon, mAudio, (LinearLayout) itemView.getChildAt(0));

        String text = item.getText();
        if ((imageUri != null || t != null) && text != null) {
            int leftAbs = Math.abs((item.getImageOrVideoLeft() - item.getTextLeft()));
            int topAbs = Math.abs(item.getImageOrVideoTop() - item.getTextTop());
            if (leftAbs > topAbs) {
                if (item.getImageOrVideoLeft() > item.getTextLeft()) {
                    mText = (TextView) itemView.findViewById(R.id.left_text);
                    if (item.getTextWidth() > 0) {
                        mText.setWidth(item.getTextWidth());
                    } else {
                        mText.setWidth(item.getImageOrVideoLeft());
                    }
                } else {
                    mText = (TextView) itemView.findViewById(R.id.right_text);
                    if (item.getTextWidth() > 0) {
                        mText.setWidth(item.getTextWidth());
                    } else {
                        mText.setWidth(item.getTextLeft());
                    }
                }
            } else if (item.getTextTop() > item.getImageOrVideoTop()) {
                mText = (TextView) itemView.findViewById(R.id.bottom_text);
            }
        }
        if (text != null) {
            /// M: ALPS00619099, highlight matched search string in mms player @ {
            if (mHighlight != null) {
                setHighlightText(mText, text);
            } else {
                SpannableString ss = new SpannableString(text);
                mText.setText(ss);
            }
            /// @}
            if (textSize != 0) {
                mText.setTextSize(textSize);
            }
            mText.setVisibility(View.VISIBLE);

            /// M: ALPS00527989, Extend TextView URL handling @ {
            mOpMmsPlayerActivityAdapterExt.setExtendUrlSpan(mText);
            /// @}
            itemView.setCurrentTextView(mText);
        } else {
            mText.setVisibility(View.GONE);
        }

        mListItemViewCache.put(arg0, itemView);
        return itemView;
    }

    public void clearAllCache() {
        /* Mem opt*/
        MmsLog.i(TAG, "clearAllCache");
        int count = mListItemViewCache.size();
        for (int i = 0; i < count; i++) {
            MmsLog.i(TAG, "clearAllCache " + i);
            MmsPlayerActivityItem itemView = (MmsPlayerActivityItem) mListItemViewCache.get(i);
            MmsPlayerActivityItemData itemData = mListItem.get(i);

            if (itemView != null) {
                MmsLog.i(TAG, "clearAllCache1 " + i);
                ImageView imageView = itemView.getImageView();
                if (imageView != null) {
                    MmsLog.i(TAG, "clearAllCache imageview " + i);
                    if (itemData != null && !itemData.isDefaultImageThumb()) {
                        MmsLog.i(TAG, "is not default image thumb");
                        //imageView.setImageBitmap(null);
                        Glide.clear(imageView);
                        recycleBitmap(imageView);
                    }
                }

                ImageView videoView = itemView.getVideoView();
                if (videoView != null) {
                    MmsLog.i(TAG, "clearAllCache videoview " + i);
                    if (itemData != null && !itemData.isDefaultVideoThumb()) {
                        MmsLog.i(TAG, "is not default video thumb");
                        //videoView.setImageBitmap(null);
                        recycleBitmap(videoView);
                    }
                }
            }
        }
        if (mListItemViewCache.size() > 0) {
            mListItemViewCache.clear();
        }
    }
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

    /// M: new feature, tap picture,audio,video play in slideshow repview @{
    class mediaClick implements OnClickListener {
        private Uri mUri;
        private String mType;
        private String mTitle;

        public mediaClick(Uri uri, String type, String title) {
            mUri = uri;
            mType = type;
            mTitle = title;
        }

        public void onClick(View v) {
            /// M: ALPS01586782, more click issue @ {
            if (sIsClick) {
                return;
            }
            ///@}

            /// M: ALPS01891063. Do not show drm media. @{
            if (DrmUtilsEx.checkUriContainsDrm(MmsPlayerActivityAdapter.this.mContext, mUri)) {
                DrmUtilsEx.showDrmAlertDialog(MmsPlayerActivityAdapter.this.mContext);
                return;
            }
            /// @}
            setIsClick(true);

            Intent intent = new Intent(Intent.ACTION_VIEW);
            intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
            intent.putExtra("SingleItemOnly", true);
            intent.putExtra("CanShare", false);
            intent.putExtra("title", mTitle);
            /// M: ALPS00994554, for showing notification when
            /// view mms with video player in full screen model @{
            intent.putExtra(EXTRA_FULLSCREEN_NOTIFICATION, true);
            /// @}
            mType = MessageUtils.getContentType(mType, mTitle);
            intent.setDataAndType(mUri, mType);
            try {
                mContext.startActivity(intent);
            } catch (ActivityNotFoundException e) {
                Intent mchooserIntent = Intent.createChooser(intent, null);
                mContext.startActivity(mchooserIntent);
            }
        }
    };
    /// @}

    /// M: ALPS00619099, highlight matched search string in mms player @ {
    public MmsPlayerActivityAdapter(Context context,
            ArrayList<MmsPlayerActivityItemData> listItem, Pattern highlightPattern) {
        mFactory = LayoutInflater.from(context);
        mListItem = listItem;
        mAllCount = mListItem.size();
        mContext = context;
        mHighlight = highlightPattern;
        mOpMmsPlayerActivityAdapterExt = OpMessageUtils.getOpMessagePlugin()
                .getOpMmsPlayerActivityAdapterExt();
    }

    private void setHighlightText(TextView textView, String text) {
        MmsLog.d(TAG, " setHighlightText");

        SpannableStringBuilder buf = new SpannableStringBuilder();
        buf.append(text);
        Matcher m = mHighlight.matcher(buf.toString());
        while (m.find()) {
            buf.setSpan(new StyleSpan(Typeface.BOLD), m.start(), m.end(), 0);
        }

        textView.setText(buf);
    }
    /// @}

    /// M: ALPS01586782, more click issue @ {
    public void setIsClick(boolean isClick) {
        sIsClick = isClick;
    }
    ///@}
}
