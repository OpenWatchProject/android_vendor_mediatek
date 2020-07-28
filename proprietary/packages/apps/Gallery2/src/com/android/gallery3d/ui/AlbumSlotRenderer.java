/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2010 The Android Open Source Project
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

package com.android.gallery3d.ui;


import android.content.Context;
import com.android.gallery3d.app.AbstractGalleryActivity;
import com.android.gallery3d.app.AlbumDataLoader;
import com.android.gallery3d.data.MediaObject;
import com.android.gallery3d.data.Path;
import com.android.gallery3d.glrenderer.ColorTexture;
import com.android.gallery3d.glrenderer.FadeInTexture;
import com.android.gallery3d.glrenderer.GLCanvas;
import com.android.gallery3d.glrenderer.Texture;
import com.android.gallery3d.glrenderer.TiledTexture;

import com.mediatek.gallery3d.adapter.FeatureManager;
import com.mediatek.gallery3d.util.FeatureConfig;
import com.mediatek.gallerybasic.base.IAlbumSlotRenderer;

public class AlbumSlotRenderer extends AbstractSlotRenderer {
    @SuppressWarnings("unused")
    private static final String TAG = "Gallery2/AlbumView";

    public interface SlotFilter {
        public boolean acceptSlot(int index);
    }

    private final int mPlaceholderColor;
    /// M: [BUG.MODIFY] only cache 32 thumbnail for low memory device @{
    // private static final int CACHE_SIZE = 96;
    private static final int CACHE_SIZE = FeatureConfig.sIsLowRamDevice ? 32 : 96;
    /// @}

    private AlbumSlidingWindow mDataWindow;
    private final AbstractGalleryActivity mActivity;
    private final ColorTexture mWaitLoadingTexture;
    private final SlotView mSlotView;
    private final SelectionManager mSelectionManager;

    private int mPressedIndex = -1;
    private boolean mAnimatePressedUp;
    private Path mHighlightItemPath = null;
    private boolean mInSelectionMode;

    private SlotFilter mSlotFilter;

    /// M: [PERF.ADD] add for performance test case@{
    private boolean mHasShowLog = false;
    public static boolean sPerformanceCaseRunning = false;
    /// M: added for performance auto test
    public static long sWaitFinishedTime = 0;
    /// @}

    public AlbumSlotRenderer(AbstractGalleryActivity activity, SlotView slotView,
            SelectionManager selectionManager, int placeholderColor) {
        super(activity);
        mActivity = activity;
        mSlotView = slotView;
        mSelectionManager = selectionManager;
        mPlaceholderColor = placeholderColor;

        mWaitLoadingTexture = new ColorTexture(mPlaceholderColor);
        mWaitLoadingTexture.setSize(1, 1);

        /// M: [FEATURE.ADD] @{
        mExtRenders =
        (IAlbumSlotRenderer[]) FeatureManager.getInstance().getImplement(
                IAlbumSlotRenderer.class, (Context) mActivity, mActivity.getResources());
        /// @}
    }

    public void setPressedIndex(int index) {
        if (mPressedIndex == index) return;
        mPressedIndex = index;
        mSlotView.invalidate();
    }

    public void setPressedUp() {
        if (mPressedIndex == -1) return;
        mAnimatePressedUp = true;
        mSlotView.invalidate();
    }

    public void setHighlightItemPath(Path path) {
        if (mHighlightItemPath == path) return;
        mHighlightItemPath = path;
        mSlotView.invalidate();
    }

    public void setModel(AlbumDataLoader model) {
        if (mDataWindow != null) {
            mDataWindow.setListener(null);
            mSlotView.setSlotCount(0);
            mDataWindow = null;
        }
        if (model != null) {
            mDataWindow = new AlbumSlidingWindow(mActivity, model, CACHE_SIZE);
            mDataWindow.setListener(new MyDataModelListener());
            mSlotView.setSlotCount(model.size());
        }
    }

    private static Texture checkTexture(Texture texture) {
        return (texture instanceof TiledTexture)
                && !((TiledTexture) texture).isReady()
                ? null
                : texture;
    }

    @Override
    public int renderSlot(GLCanvas canvas, int index, int pass, int width, int height) {
        if (mSlotFilter != null && !mSlotFilter.acceptSlot(index)) return 0;

        AlbumSlidingWindow.AlbumEntry entry = mDataWindow.get(index);

        int renderRequestFlags = 0;

        Texture content = checkTexture(entry.content);
        /// M: [MEMORY.ADD] Recycle the bitmap after uploaded texture for saving memory @{
        if (content != null) {
            mDataWindow.recycle(entry);
        }
        /// @}
        if (content == null) {
            content = mWaitLoadingTexture;
            entry.isWaitDisplayed = true;
        } else if (entry.isWaitDisplayed) {
            entry.isWaitDisplayed = false;
            /// M: [BUG.MODIFY] FadeInTexture will be transparent when launch gallery @{
            /*content = new FadeInTexture(mPlaceholderColor, entry.bitmapTexture);*/
            content = entry.bitmapTexture;
            /// @}
            entry.content = content;
            /// M: [PERF.ADD]added for performance auto test. @{
            sWaitFinishedTime = System.currentTimeMillis();
            /// @}
        }
        /// M: [FEATURE.ADD] @{
        boolean skipStandardFlow = false;
        if (entry.item != null) {
            for (IAlbumSlotRenderer render : mExtRenders) {
                skipStandardFlow =
                        render.renderContent(canvas.getMGLCanvas(), width, height, entry.item
                                .getMediaData());
            }
        }
        if (!skipStandardFlow) {
        /// @}
            drawContent(canvas, content, width, height, entry.rotation);
        /// M: [FEATURE.ADD] @{
        }
        /// @}
        if ((content instanceof FadeInTexture) &&
                ((FadeInTexture) content).isAnimating()) {
            renderRequestFlags |= SlotView.RENDER_MORE_FRAME;
        }

        /// M: [FEATURE.ADD] @{
        skipStandardFlow = false;
        if (entry.item != null) {
            for (IAlbumSlotRenderer render : mExtRenders) {
                skipStandardFlow |=
                        render.renderCover(canvas.getMGLCanvas(), width, height, entry.item
                                .getMediaData());
            }
        }
        if (!skipStandardFlow) {
        /// @}
            /// M: [FEATURE.ADD] video @{
            if (entry.mediaType == MediaObject.MEDIA_TYPE_VIDEO) {
                drawVideoOverlay(canvas, width, height);
            }
            /// @}

            if (entry.isPanorama) {
                drawPanoramaIcon(canvas, width, height);
            }
        /// M: [FEATURE.ADD] @{
        }
        /// @}

        renderRequestFlags |= renderOverlay(canvas, index, entry, width, height);

        return renderRequestFlags;
    }

    private int renderOverlay(GLCanvas canvas, int index,
            AlbumSlidingWindow.AlbumEntry entry, int width, int height) {
        int renderRequestFlags = 0;
        if (mPressedIndex == index) {
            if (mAnimatePressedUp) {
                /// M: [PERF.ADD] add for performance test case@{
                if (!mHasShowLog && sPerformanceCaseRunning) {
                    Log.d(TAG, "[CMCC Performance test][Gallery2][Gallery] "
                            + "load 1M image time start [" + System.currentTimeMillis() + "]");
                    mHasShowLog = true;
                }
                /// @}
                drawPressedUpFrame(canvas, width, height);
                renderRequestFlags |= SlotView.RENDER_MORE_FRAME;
                if (isPressedUpFrameFinished()) {
                    mAnimatePressedUp = false;
                    mPressedIndex = -1;
                }
            } else {
                drawPressedFrame(canvas, width, height);
            }
        } else if ((entry.path != null) && (mHighlightItemPath == entry.path)) {
            drawSelectedFrame(canvas, width, height);
        } else if (mInSelectionMode && mSelectionManager.isItemSelected(entry.path)) {
            drawSelectedFrame(canvas, width, height);
        }
        return renderRequestFlags;
    }

    private class MyDataModelListener implements AlbumSlidingWindow.Listener {
        @Override
        public void onContentChanged() {
            mSlotView.invalidate();
        }

        @Override
        public void onSizeChanged(int size) {
            mSlotView.setSlotCount(size);
            /// M: [BUG.ADD] photo won't disappear when it has been deleted @{
            mSlotView.invalidate();
            /// @}
        }
    }

    public void resume() {
        mDataWindow.resume();
    }

    public void pause() {
        mDataWindow.pause();
    }

    @Override
    public void prepareDrawing() {
        mInSelectionMode = mSelectionManager.inSelectionMode();
    }

    @Override
    public void onVisibleRangeChanged(int visibleStart, int visibleEnd) {
        if (mDataWindow != null) {
            mDataWindow.setActiveWindow(visibleStart, visibleEnd);
        }
    }

    @Override
    public void onSlotSizeChanged(int width, int height) {
        // Do nothing
    }

    public void setSlotFilter(SlotFilter slotFilter) {
        mSlotFilter = slotFilter;
    }

    /// M: [FEATURE.ADD] @{
    private IAlbumSlotRenderer[] mExtRenders;
    /// @}
}
