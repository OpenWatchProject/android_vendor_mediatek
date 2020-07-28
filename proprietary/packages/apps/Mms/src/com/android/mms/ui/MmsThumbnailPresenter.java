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
import android.net.Uri;

import com.android.mms.model.AudioModel;
import com.android.mms.model.ImageModel;
import com.android.mms.model.Model;
import com.android.mms.model.SlideModel;
import com.android.mms.model.SlideshowModel;
import com.android.mms.model.VideoModel;
import com.android.mms.util.FeatureOption;
import com.android.mms.util.ItemLoadedCallback;
import com.android.mms.util.ItemLoadedFuture;
import com.android.mms.util.MmsLog;
import com.android.mms.util.ThumbnailManager;
import com.android.mms.util.ThumbnailManager.ImageLoaded;
import com.mediatek.mms.util.DrmUtilsEx;
import com.mediatek.opmsg.util.OpMessageUtils;


public class MmsThumbnailPresenter extends Presenter {
    private static final String TAG = "MmsThumbnailPresenter";
    private ItemLoadedCallback mOnLoadedCallback;
    private ItemLoadedFuture mItemLoadedFuture;

    /// M: Code analyze 002, new feature @{
    private Context mContext;
    private int mSlideCount = 0;
    /// @}

    public MmsThumbnailPresenter(Context context, ViewInterface view, Model model) {
        super(context, view, model);
        mContext = context;
    }

    @Override
    public void present(ItemLoadedCallback callback) {
        mOnLoadedCallback = callback;
        SlideModel slide = ((SlideshowModel) mModel).get(0);
        /// M: Code analyze 002, new feature  @{
        mSlideCount = ((SlideshowModel) mModel).size();
        /// @}
        if (slide != null) {
            //MmsLog.i(TAG, "The first slide is not null.");
            presentFirstSlide((SlideViewInterface) mView, slide);
        }
    }

    private void presentFirstSlide(SlideViewInterface view, SlideModel slide) {
        view.reset();
        /// M: Code analyze 002, new feature  @{
        boolean imageVisibility = true;
        /// @}
        if (slide.hasImage()) {
            MmsLog.i(TAG, "The first slide has image.");
            presentImageThumbnail(view, slide.getImage());
        } else if (slide.hasVideo()) {
            MmsLog.i(TAG, "The first slide has video.");
            presentVideoThumbnail(view, slide.getVideo());
        } else if (slide.hasAudio()) {
            MmsLog.i(TAG, "The first slide has audio.");
            presentAudioThumbnail(view, slide.getAudio());
            /// M: Code analyze 001, For fix bug ALPS00116961, shown in the
            // attachment list after replaces it with an audio in MMS.When slide
            // has audio, we should hide prvious thumbnail. @{
            imageVisibility = false;
            /// @}
        /// M: Code analyze 002, new feature  @{
        } else {
            MmsLog.i(TAG, "The first slide has only text.");
            imageVisibility = false;
        }
        view.setImageVisibility(imageVisibility);
        /// @}
    }

    private ItemLoadedCallback<ImageLoaded> mImageLoadedCallback =
            new ItemLoadedCallback<ImageLoaded>() {
        public void onItemLoaded(ImageLoaded imageLoaded, Throwable exception) {
            if (exception == null) {
                /// M: google jb.mr1 pathc, remove and fully reloaded the next time
                /// When a pdu or image is canceled during loading @{
                if (mItemLoadedFuture != null) {
                    synchronized (mItemLoadedFuture) {
                        mItemLoadedFuture.setIsDone(true);
                    }
                }

                /// @}
                if (mOnLoadedCallback != null) {
                    mOnLoadedCallback.onItemLoaded(imageLoaded, exception);
                } else {
                    // Right now we're only handling image and video loaded callbacks.
                    SlideModel slide = ((SlideshowModel) mModel).get(0);
                    if (slide != null) {
                        if (slide.hasVideo() && imageLoaded.mIsVideo) {
                            /*
                             * M: It means that the thumbnail may be wrong if
                             * the thumbnail's uri is not same with the
                             * slide model's uri. But there is one case we must
                             * be consider that: after we add video,
                             * compose will save the saft, so the slide model's
                             * uri will be changed. At the same time,
                             * compose will try to get thumbnail with the old
                             * media uri which must be not start with
                             * "content://mms/part". So, when present it, the thumbnail's
                             * uri will be different with the
                             * slide model's uri. @{
                             */
                            Uri slideUri = ThumbnailManager.getThumbnailUri(slide.getVideo());
                            String thumbnailUriStr = imageLoaded.getUri().toString();
                            if (slideUri.equals(imageLoaded.getUri())) {
                                ((SlideViewInterface) mView).setVideoThumbnail(null,
                                    imageLoaded.mBitmap);
                            } else if (!thumbnailUriStr.startsWith("content://mms/part")) {
                                ((SlideViewInterface) mView).setVideoThumbnail(null,
                                    imageLoaded.mBitmap);
                            }
                            /**@}*/
                        } else if (slide.hasImage() && !imageLoaded.mIsVideo) {
                            /*
                             * M: It means that the thumbnail may be wrong if the
                             * thumbnail's uri is not same with the
                             * slide model's uri. But there is one case we must be
                             * consider that: after we add video,
                             * compose will save the saft, so the slide model's
                             * uri will be changed. At the same time,
                             * compose will try to get thumbnail with the old
                             * media uri which must be not start with
                             * "content://mms/part". So, when present it, the
                             * thumbnail's uri will be different with the
                             * slide model's uri. @{
                             */
                            Uri slideUri = ThumbnailManager.getThumbnailUri(slide.getImage());
                            String thumbnailUriStr = imageLoaded.getUri().toString();
                            if (slideUri.equals(imageLoaded.getUri())) {
                                ((SlideViewInterface) mView).setImage(null, imageLoaded.mBitmap);
                            } else if (!thumbnailUriStr.startsWith("content://mms/part")) {
                                ((SlideViewInterface) mView).setImage(null, imageLoaded.mBitmap);
                            }
                            /**@}*/
                        }
                    }
                }
            }
        }
    };

    private void presentVideoThumbnail(SlideViewInterface view, VideoModel video) {
        if (video == null) {
            MmsLog.e(TAG, "presentVideoThumbnail(). video is null");
            return;
        }

        if (FeatureOption.MTK_DRM_APP) {
            String extName = video.getSrc().substring(video.getSrc().lastIndexOf('.') + 1);
            if (extName.equals("dcf") && mSlideCount == 1 && !(view instanceof MessageListItem)) {
                Bitmap drmBitmap = DrmUtilsEx.getDrmBitmapWithLockIcon(mContext,
                        video.getUri(), true);
                view.setVideoThumbnail(video.getSrc(), drmBitmap);
            } else {
                mItemLoadedFuture = video.loadThumbnailBitmap(mImageLoadedCallback);
            }
        } else {
            mItemLoadedFuture = video.loadThumbnailBitmap(mImageLoadedCallback);
        }
    }

    private void presentImageThumbnail(SlideViewInterface view, ImageModel image) {
        /// M: Code analyze 002, new feature  @{
        if (image == null) {
            MmsLog.e(TAG, "presentImageThumbnail(). iamge is null");
            return;
        }

        if (FeatureOption.MTK_DRM_APP) {
            String extName = image.getSrc().substring(image.getSrc().lastIndexOf('.') + 1);
            if (extName.equals("dcf") && mSlideCount == 1) {
                Bitmap drmBitmap = DrmUtilsEx.getDrmBitmapWithLockIcon(mContext,
                        image.getUri(), false);
                view.setImage(image.getSrc(), drmBitmap);
            } else {
                mItemLoadedFuture = image.loadThumbnailBitmap(mImageLoadedCallback);
            }
        } else {
            mItemLoadedFuture = image.loadThumbnailBitmap(mImageLoadedCallback);
        }
        /// @}
    }

    protected void presentAudioThumbnail(SlideViewInterface view, AudioModel audio) {
        view.setAudio(audio.getUri(), audio.getSrc(), audio.getExtras());
    }

    public void onModelChanged(Model model, boolean dataChanged) {
        // TODO Auto-generated method stub
    }

    public void cancelBackgroundLoading() {
        // Currently we only support background loading of thumbnails. If we extend background
        // loading to other media types, we should add a cancelLoading API to Model.
        SlideModel slide = ((SlideshowModel) mModel).get(0);
        if (slide != null && slide.hasImage()) {
            slide.getImage().cancelThumbnailLoading();
        }
    }
}
