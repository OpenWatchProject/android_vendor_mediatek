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
import android.graphics.BitmapFactory;
import android.os.Handler;
import android.util.Log;
import android.view.WindowManager;

import com.android.mms.model.AudioModel;
import com.android.mms.model.ImageModel;
import com.android.mms.model.LayoutModel;
import com.android.mms.model.MediaModel;
import com.android.mms.model.Model;
import com.android.mms.model.RegionMediaModel;
import com.android.mms.model.RegionModel;
import com.android.mms.model.SlideModel;
import com.android.mms.model.SlideshowModel;
import com.android.mms.model.TextModel;
import com.android.mms.model.VideoModel;
import com.android.mms.model.MediaModel.MediaAction;
import com.android.mms.R;
import com.android.mms.ui.AdaptableSlideViewInterface.OnSizeChangedListener;
import com.android.mms.util.ItemLoadedCallback;
import com.android.mms.util.MmsContentType;
import com.android.mms.util.MmsLog;

import com.mediatek.mms.util.DrmUtilsEx;
import com.mediatek.opmsg.util.OpMessageUtils;

/**
 * A basic presenter of slides.
 */
public class SlideshowPresenter extends Presenter {
    private static final String TAG = "SlideshowPresenter";
    private static final boolean DEBUG = false;
    private static final boolean LOCAL_LOGV = false;
    /// M: read image by specified size
    private static final int PREVIEW_IMAGE_SIZE = (int) (60 * 1.5f);

    protected int mLocation;
    protected final int mSlideNumber;

    protected float mWidthTransformRatio = 1.0f;
    protected float mHeightTransformRatio = 1.0f;

    // Since only the original thread that created a view hierarchy can touch
    // its views, we have to use Handler to manage the views in the some
    // callbacks such as onModelChanged().
    protected final Handler mHandler = new Handler();
    /// M: Code analyze 002, useless, set Height according to orientation @{
    private int mHeight = 800;
    /// @}

    /// M: Code analyze 003, fix bug ALPS00119632, present slide return
    /// (mActivityRunning=false) when SlideshowActivity stop @{
    private boolean mActivityRunning = true;
    /// @}

    /// M: Record current page of model is changed or not. @{
    private int mCurPage = -1;
    private boolean mPageChanged = false;
    /// @}
    private boolean mSlideshowPlayMode = false;
    private String mLastPlayImageSrc;

    public SlideshowPresenter(Context context, ViewInterface view, Model model) {
        super(context, view, model);
        mLocation = 0;
        mSlideNumber = ((SlideshowModel) mModel).size();
        /// M: Code analyze 002, useless, set Height according to orientation @{
        WindowManager winM = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
        if (mContext.getResources().getConfiguration().orientation
                == Configuration.ORIENTATION_PORTRAIT) {
            mHeight = winM.getDefaultDisplay().getHeight();
        } else {
            mHeight = winM.getDefaultDisplay().getWidth();
        }
        /// @}
        if (view instanceof AdaptableSlideViewInterface) {
            ((AdaptableSlideViewInterface) view).setOnSizeChangedListener(
                    mViewSizeChangedListener);
        }
        /// M: Code analyze 003, fix bug ALPS00119632, present slide return
        /// (mActivityRunning=false) when SlideshowActivity stop @{
        mActivityRunning = true;
        /// @}
    }

    private final OnSizeChangedListener mViewSizeChangedListener =
        new OnSizeChangedListener() {
        public void onSizeChanged(int width, int height) {
            LayoutModel layout = ((SlideshowModel) mModel).getLayout();
            mWidthTransformRatio = getWidthTransformRatio(
                    width, layout.getLayoutWidth());
            mHeightTransformRatio = getHeightTransformRatio(
                    height, layout.getLayoutHeight());
            // The ratio indicates how to reduce the source to match the View,
            // so the larger one should be used.
            float ratio = mWidthTransformRatio > mHeightTransformRatio ?
                    mWidthTransformRatio : mHeightTransformRatio;
            mWidthTransformRatio = ratio;
            mHeightTransformRatio = ratio;
            if (LOCAL_LOGV) {
                Log.v(TAG, "ratio_w = " + mWidthTransformRatio
                        + ", ratio_h = " + mHeightTransformRatio);
            }
        }
    };

    private float getWidthTransformRatio(int width, int layoutWidth) {
        if (width > 0) {
            return (float) layoutWidth / (float) width;
        }
        return 1.0f;
    }

    private float getHeightTransformRatio(int height, int layoutHeight) {
        if (height > 0) {
            return (float) layoutHeight / (float) height;
        }
        return 1.0f;
    }

    private int transformWidth(int width) {
        return (int) (width / mWidthTransformRatio);
    }

    private int transformHeight(int height) {
        return (int) (height / mHeightTransformRatio);
    }

    @Override
    public void present(ItemLoadedCallback callback) {
        // This is called to show a full-screen slideshow. Presently, all parts of
        // a slideshow (images, sounds, etc.) are loaded and displayed on the UI thread.
        presentSlide((SlideViewInterface) mView, ((SlideshowModel) mModel).get(mLocation));
    }

    /**
     * @param view
     * @param model
     */
    protected void presentSlide(SlideViewInterface view, SlideModel model) {
        /// M: Code analyze 003, fix bug ALPS00119632, present slide return
        /// (mActivityRunning=false) when SlideshowActivity stop @{
        if (!mActivityRunning) {
            return;
        }
        /// @}
        /// M: update page index while playing slides. @{
        if (view instanceof SlideView) {
            int curPage = model.getCurrentPage();
            ((SlideView) view).setPageDividerView(curPage);
            if (mCurPage != curPage) {
                mCurPage = curPage;
                mPageChanged = true;
                view.reset();
            } else {
                mPageChanged = false;
            }
        } else {
            view.reset();
        }
        /// @}
        for (MediaModel media : model) {
            if (media instanceof RegionMediaModel) {
                presentRegionMedia(view, (RegionMediaModel) media, true);
            } else if (media.isAudio()) {
                /// M: Code analyze 004, fix bug ALPS00267249 272575,
                /// Set audio name to BasicSlideEditorView @{
                if (view instanceof SlideView) {
                    /// M: Code analyze 005, fix bug ALPS00259726, unknown @{
                    MediaAction action = media.getCurrentAction();
                    if (action == MediaAction.NO_ACTIVE_ACTION) {
                        /// M: Code analyze 006, fix bug ALPS00272535,
                        /// can't return even if NO_ACTIVE_ACTION @{

                        /// M: fix bug ALPS00958199, ALPS00547255, show audio name @{
                        if (mPageChanged) {
                            presentAudio(view, (AudioModel) media, true);
                            ((SlideView) view).displayAudioInfo();
                        }
                        continue;
                        /// @}
                    } else {
                        media.appendActionAtFirst(action);
                    }
                    /// @}
                    presentAudio(view, (AudioModel) media, true);
                } else {
                    presentAudio(view, (AudioModel) media, true);
                }
                /// @}
            }
        }
    }

    /**
     * @param view
     */
    protected void presentRegionMedia(SlideViewInterface view,
            RegionMediaModel rMedia, boolean dataChanged) {
        RegionModel r = (rMedia).getRegion();
        if (dataChanged && !rMedia.isImage()) {
            mLastPlayImageSrc = "";
            // empty the last image played if other media model is being played
            Log.d(TAG, "clear the mLastPlayImageSrc ");
        }
        if (rMedia.isText()) {
            presentText(view, (TextModel) rMedia, r, dataChanged);
        } else if (rMedia.isImage()) {
            presentImage(view, (ImageModel) rMedia, r, dataChanged);
        } else if (rMedia.isVideo()) {
            presentVideo(view, (VideoModel) rMedia, r, dataChanged);
        }
    }

    protected void presentAudio(SlideViewInterface view, AudioModel audio,
            boolean dataChanged) {
        // Set audio only when data changed.
        Log.d(TAG, "presentAudio: " + audio.getUri() + " dataChanged:" + dataChanged);
        if (dataChanged) {
            view.setAudio(audio.getUri(), audio.getSrc(), audio.getExtras());
            mLastPlayImageSrc = "";
            // empty the last image played if other media model is being played
            Log.d(TAG, "clear the mLastPlayImageSrc ");
        }

        MediaAction action = audio.getCurrentAction();
        if (action == MediaAction.START) {
            view.startAudio();
        } else if (action == MediaAction.PAUSE) {
            view.pauseAudio();
        } else if (action == MediaAction.STOP) {
            view.stopAudio();
        } else if (action == MediaAction.SEEK) {
            view.seekAudio(audio.getSeekTo());
        }
    }

    protected void presentText(SlideViewInterface view, TextModel text,
            RegionModel r, boolean dataChanged) {
        if (dataChanged) {
            view.setText(text.getSrc(), text.getText());
        }

        if (view instanceof AdaptableSlideViewInterface) {
            ((AdaptableSlideViewInterface) view).setTextRegion(
                    transformWidth(r.getLeft()),
                    transformHeight(r.getTop()),
                    transformWidth(r.getWidth()),
                    transformHeight(r.getHeight()));
        }
        view.setTextVisibility(text.isVisible());
    }

    /**
     * @param view
     * @param image
     * @param r
     */
    protected void presentImage(SlideViewInterface view, ImageModel image,
            RegionModel r, boolean dataChanged) {
        int transformedWidth = transformWidth(r.getWidth());
        int transformedHeight = transformWidth(r.getHeight());
        if (LOCAL_LOGV) {
            Log.v(TAG, "presentImage r.getWidth: " + r.getWidth()
                    + ", r.getHeight: " + r.getHeight() +
                    " transformedWidth: " + transformedWidth +
                    " transformedHeight: " + transformedHeight);
        }
        Log.d(TAG, "presentImage name: " + image.getSrc()
                +  " type: " + image.getContentType() + " dataChanged:" + dataChanged);
        /// M: Code analyze 007, fix bug ALPS00275655 294088, Play gif image
        /// with the matkImageView, set Image through Uri @{
        if (dataChanged) {
            try {
                String name = image.getSrc();
                if (mSlideshowPlayMode && (name != null) && name.equals(mLastPlayImageSrc)) {
                    Log.d(TAG, "Ignore duplicated image play!");
                    return;
                }
                mLastPlayImageSrc = name;
                String lowerName = name.toLowerCase();
                if (lowerName.endsWith(".gif") &&
                        image.getContentType().equalsIgnoreCase(MmsContentType.IMAGE_GIF)) {
                    view.setImage(image.getUri());
                } else {
                    /*
                     * M: optimization RAM: read image by specified size.
                     * If the image format is jpeg type, memory would be greatly reduced.
                     * But the image would become somewhat blurred. @{
                     */
                    Bitmap bitmap = image.getBitmap(transformedWidth, transformedHeight);

                    if (bitmap == null
                            && view instanceof SlideListItemView
                            && DrmUtilsEx.checkHasDrmContent(mContext, null, image.getUri())) {
                        bitmap = DrmUtilsEx.getDrmBitmapWithLockIcon(
                                mContext, image.getUri(), false);
                    }
                    view.setImage(image.getSrc(), bitmap);
                    /// @}
                    /* @} */
                }
            /// M: Code analyze 009, fix bug ALPS00293754,
            /// show the system picture for bad picture @{
            } catch (IllegalArgumentException e) {
                Log.e(TAG, e.getMessage(), e);
                view.setImage(image.getSrc(), BitmapFactory.decodeResource(this.mContext
                        .getResources(), R.drawable.ic_missing_thumbnail_picture));
            }
            /// @}
        }
        /// @}

        if (view instanceof AdaptableSlideViewInterface) {
            ((AdaptableSlideViewInterface) view).setImageRegion(
                    transformWidth(r.getLeft()),
                    transformHeight(r.getTop()),
                    transformedWidth,
                    transformedHeight);
        }
        view.setImageRegionFit(r.getFit());
        view.setImageVisibility(image.isVisible());

    }

    /**
     * @param view
     * @param video
     * @param r
     */
    protected void presentVideo(SlideViewInterface view, VideoModel video,
            RegionModel r, boolean dataChanged) {
        MediaAction action = video.getCurrentAction();
        Log.d(TAG, "presentVideo: " + video.getUri() + " dataChanged:" + dataChanged);
        if (dataChanged) {
            if (!(view instanceof SlideView) || mPageChanged) {
                view.setVideo(video.getSrc(), video.getUri());
            }
        }

        if (view instanceof AdaptableSlideViewInterface) {
            ((AdaptableSlideViewInterface) view).setVideoRegion(
                    transformWidth(r.getLeft()),
                    transformHeight(r.getTop()),
                    transformWidth(r.getWidth()),
                    transformHeight(r.getHeight()));
        }
        view.setVideoVisibility(video.isVisible());

        if (action == MediaAction.START) {
            view.startVideo();
        } else if (action == MediaAction.PAUSE) {
            view.pauseVideo();
        } else if (action == MediaAction.STOP) {
            view.stopVideo();
        } else if (action == MediaAction.SEEK) {
            view.seekVideo(video.getSeekTo());
        }
    }

    public void setLocation(int location) {
        mLocation = location;
    }

    public int getLocation() {
        return mLocation;
    }

    public void goBackward() {
        if (mLocation > 0) {
            mLocation--;
        }
    }

    public void goForward() {
        if (mLocation < (mSlideNumber - 1)) {
            mLocation++;
        }
    }

    public void onModelChanged(final Model model, final boolean dataChanged) {
        /// M: Code analyze 003, fix bug ALPS00119632, present slide return
        /// (mActivityRunning=false) when SlideshowActivity stop @{
        if (!mActivityRunning) {
            Log.w(TAG, "onModelChanged after mActivityRunning = false");
            if (mView != null && mView instanceof SlideViewInterface) {
                ((SlideViewInterface) mView).stopAudio();
            }
            return;
        }
        /// @}
        final SlideViewInterface view = (SlideViewInterface) mView;
        Log.d(TAG, "onModelChanged!!!");
        // FIXME: Should be optimized.
        if (model instanceof SlideshowModel) {
            // TODO:
        } else if (model instanceof SlideModel) {
            if (((SlideModel) model).isVisible()) {
                mHandler.post(new Runnable() {
                    public void run() {
                        Log.d(TAG, "onModelChanged present slide");
                        presentSlide(view, (SlideModel) model);
                    }
                });
            } else {
                mHandler.post(new Runnable() {
                    public void run() {
                        goForward();
                    }
                });
            }
        } else if (model instanceof MediaModel) {
            if (model instanceof RegionMediaModel) {
                mHandler.post(new Runnable() {
                    public void run() {
                        Log.d(TAG, "onModelChanged present region media model");
                        presentRegionMedia(view, (RegionMediaModel) model, dataChanged);
                    }
                });
            } else if (((MediaModel) model).isAudio()) {
                mHandler.post(new Runnable() {
                    public void run() {
                        presentAudio(view, (AudioModel) model, dataChanged);
                    }
                });
            }
        } else if (model instanceof RegionModel) {
            // TODO:
        }
    }

    @Override
    public void cancelBackgroundLoading() {
        // For now, the SlideshowPresenter do no background loading so there is nothing to cancel.
    }

    /// M: Code analyze 003, fix bug ALPS00119632, present slide return
    /// (mActivityRunning=false) when SlideshowActivity stop @{
    public void onStop() {
        mHandler.removeCallbacks(null);
        mActivityRunning = false;
    }
    /// @}

    public void setIsSlideshowPlayMode(boolean mode) {
        mSlideshowPlayMode = mode;
    }
}
