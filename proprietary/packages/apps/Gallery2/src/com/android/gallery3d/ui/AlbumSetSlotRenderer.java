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

import com.android.gallery3d.R;
import com.android.gallery3d.app.AbstractGalleryActivity;
import com.android.gallery3d.app.AlbumSetDataLoader;
import com.android.gallery3d.data.Path;
import com.android.gallery3d.glrenderer.ColorTexture;
import com.android.gallery3d.glrenderer.FadeInTexture;
import com.android.gallery3d.glrenderer.GLCanvas;
import com.android.gallery3d.glrenderer.ResourceTexture;
import com.android.gallery3d.glrenderer.Texture;
import com.android.gallery3d.glrenderer.TiledTexture;
import com.android.gallery3d.glrenderer.UploadedTexture;
import com.android.gallery3d.ui.AlbumSetSlidingWindow.AlbumSetEntry;
import com.mediatek.gallery3d.layout.FancyHelper;
import com.mediatek.gallery3d.util.FeatureConfig;

public class AlbumSetSlotRenderer extends AbstractSlotRenderer {
    @SuppressWarnings("unused")
    private static final String TAG = "Gallery2/AlbumSetView";
    /// M: [BUG.MODIFY] only cache 32 thumbnail for low memory device @{
    /// M: [FEATURE.MODIFY] fancy layout @{
    //private static final int CACHE_SIZE = 96;
    //public static final int CACHE_SIZE = 96;
    /// @}
    public static final int CACHE_SIZE = FeatureConfig.sIsLowRamDevice ? 32 : 96;
    /// @}
    private final int mPlaceholderColor;

    private final ColorTexture mWaitLoadingTexture;
    private final ResourceTexture mCameraOverlay;
    private final AbstractGalleryActivity mActivity;
    private final SelectionManager mSelectionManager;
    protected final LabelSpec mLabelSpec;

    protected AlbumSetSlidingWindow mDataWindow;
    private SlotView mSlotView;

    private int mPressedIndex = -1;
    private boolean mAnimatePressedUp;
    private Path mHighlightItemPath = null;
    private boolean mInSelectionMode;

    /// M: [PERF.ADD]  added for performance auto test@{
    public static long mWaitFinishedTime = 0;
    /// @}

    public static class LabelSpec {
        public int labelBackgroundHeight;
        public int titleOffset;
        public int countOffset;
        public int titleFontSize;
        public int countFontSize;
        public int leftMargin;
        public int iconSize;
        public int titleRightMargin;
        public int backgroundColor;
        public int titleColor;
        public int countColor;
        public int borderSize;
    }

    public AlbumSetSlotRenderer(AbstractGalleryActivity activity,
            SelectionManager selectionManager,
            SlotView slotView, LabelSpec labelSpec, int placeholderColor) {
        super (activity);
        mActivity = activity;
        mSelectionManager = selectionManager;
        mSlotView = slotView;
        mLabelSpec = labelSpec;
        mPlaceholderColor = placeholderColor;

        mWaitLoadingTexture = new ColorTexture(mPlaceholderColor);
        mWaitLoadingTexture.setSize(1, 1);
        mCameraOverlay = new ResourceTexture(activity,
                R.drawable.ic_cameraalbum_overlay);
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

    public void setModel(AlbumSetDataLoader model) {
        if (mDataWindow != null) {
            mDataWindow.setListener(null);
            mDataWindow = null;
            mSlotView.setSlotCount(0);
        }
        if (model != null) {
            mDataWindow = new AlbumSetSlidingWindow(
                    mActivity, model, mLabelSpec, CACHE_SIZE);
            mDataWindow.setListener(new MyCacheListener());
            mSlotView.setSlotCount(mDataWindow.size());
        }
    }

    private static Texture checkLabelTexture(Texture texture) {
        return ((texture instanceof UploadedTexture)
                && ((UploadedTexture) texture).isUploading())
                ? null
                : texture;
    }

    private static Texture checkContentTexture(Texture texture) {
        return ((texture instanceof TiledTexture)
                && !((TiledTexture) texture).isReady())
                ? null
                : texture;
    }

    @Override
    public int renderSlot(GLCanvas canvas, int index, int pass, int width, int height) {
        /// M: [BUG.MODIFY] @{
        /*AlbumSetEntry entry = mDataWindow.get(index);*/
        AlbumSetEntry entry = null;
        try {
            entry = mDataWindow.get(index);
        } catch (AssertionError e) {
            Log.d(TAG, "<renderSlot> AssertionError when mDataWindow.get, " + e.getMessage());
        }
        /// @}
        /// M: [BUG.ADD] avoid nullPointer JE @{
        if (entry == null) {
            Log.d(TAG, "<renderSlot> entry is null, so return");
            return 0;
        }
        /// @}
        int renderRequestFlags = 0;
        /// M: [FEATURE.MODIFY] fancy layout @{
        // renderRequestFlags |= renderContent(canvas, entry, width, height);
        renderRequestFlags |= renderContent(canvas, index, entry, width,
                height, FancyHelper.isFancyLayoutSupported());
        /// @}
        renderRequestFlags |= renderLabel(canvas, entry, width, height);
        /// M: [FEATURE.MODIFY] fancy layout @{
        // renderRequestFlags |= renderOverlay(canvas, index, entry, width, height);
        renderRequestFlags |= renderOverlay(canvas, index, entry, width,
                height, FancyHelper.isFancyLayoutSupported());
        /// @}
        return renderRequestFlags;
    }

    protected int renderOverlay(
            GLCanvas canvas, int index, AlbumSetEntry entry, int width, int height) {
        int renderRequestFlags = 0;
        if (entry.album != null && entry.album.isCameraRoll()) {
            int uncoveredHeight = height - mLabelSpec.labelBackgroundHeight;
            int dim = uncoveredHeight / 2;
            mCameraOverlay.draw(canvas, (width - dim) / 2,
                    (uncoveredHeight - dim) / 2, dim, dim);
        }
        if (mPressedIndex == index) {
            if (mAnimatePressedUp) {
                drawPressedUpFrame(canvas, width, height);
                renderRequestFlags |= SlotView.RENDER_MORE_FRAME;
                if (isPressedUpFrameFinished()) {
                    mAnimatePressedUp = false;
                    mPressedIndex = -1;
                }
            } else {
                drawPressedFrame(canvas, width, height);
            }
        } else if ((mHighlightItemPath != null) && (mHighlightItemPath == entry.setPath)) {
            drawSelectedFrame(canvas, width, height);
        } else if (mInSelectionMode && mSelectionManager.isItemSelected(entry.setPath)) {
            drawSelectedFrame(canvas, width, height);
        }
        return renderRequestFlags;
    }

    protected int renderContent(
            GLCanvas canvas, AlbumSetEntry entry, int width, int height) {
        int renderRequestFlags = 0;

        Texture content = checkContentTexture(entry.content);
        /// M: [MEMORY.ADD] Recycle the bitmap after uploaded texture for saving memory @{
        if (content != null) {
            mDataWindow.recycle(entry);
        }
        /// @}
        if (content == null) {
            content = mWaitLoadingTexture;
            entry.isWaitLoadingDisplayed = true;
        } else if (entry.isWaitLoadingDisplayed) {
            entry.isWaitLoadingDisplayed = false;
            /// M: [BUG.MODIFY] FadeInTexture will be transparent when launch gallery @{
            /*content = new FadeInTexture(mPlaceholderColor, entry.bitmapTexture);*/
            content = entry.bitmapTexture;
            /// @}
            entry.content = content;
            /// M: [PERF.ADD] added for performance auto test @{
            mWaitFinishedTime = System.currentTimeMillis();
            /// @}
        }
        drawContent(canvas, content, width, height, entry.rotation);
        if ((content instanceof FadeInTexture) &&
                ((FadeInTexture) content).isAnimating()) {
            renderRequestFlags |= SlotView.RENDER_MORE_FRAME;
        }

        return renderRequestFlags;
    }

    protected int renderLabel(
            GLCanvas canvas, AlbumSetEntry entry, int width, int height) {
        Texture content = checkLabelTexture(entry.labelTexture);
        if (content == null) {
            content = mWaitLoadingTexture;
        }
        int b = AlbumLabelMaker.getBorderSize();
        int h = mLabelSpec.labelBackgroundHeight;
        content.draw(canvas, -b, height - h + b, width + b + b, h);

        return 0;
    }

    @Override
    public void prepareDrawing() {
        mInSelectionMode = mSelectionManager.inSelectionMode();
    }

    private class MyCacheListener implements AlbumSetSlidingWindow.Listener {

        @Override
        public void onSizeChanged(int size) {
            mSlotView.setSlotCount(size);
        }

        @Override
        public void onContentChanged() {
            mSlotView.invalidate();
        }
    }

    public void pause() {
        mDataWindow.pause();
    }

    public void resume() {
        mDataWindow.resume();
    }

    @Override
    public void onVisibleRangeChanged(int visibleStart, int visibleEnd) {
        if (mDataWindow != null) {
            mDataWindow.setActiveWindow(visibleStart, visibleEnd);
        }
    }

    @Override
    public void onSlotSizeChanged(int width, int height) {
        if (mDataWindow != null) {
            mDataWindow.onSlotSizeChanged(width, height);
        }
    }

    /// M: [FEATURE.ADD] fancy layout @{
    public void onEyePositionChanged(int orientation) {
        mDataWindow.onEyePositionChanged(orientation);
    }

    @Override
    protected void drawContent(GLCanvas canvas,
            Texture content, int width, int height, int rotation) {
        canvas.save(GLCanvas.SAVE_FLAG_MATRIX);

        if (rotation == 90) {
            canvas.translate(height / 2, height / 2);
            canvas.rotate(rotation, 0, 0, 1);
            canvas.translate(-height / 2, -height / 2 - (width - height));
        } else if (rotation == 270) {
            canvas.translate(height / 2, height / 2);
            canvas.rotate(rotation, 0, 0, 1);
            canvas.translate(-height / 2, -height / 2);
        } else if (rotation == 180) {
            canvas.translate(width / 2, height / 2);
            canvas.rotate(rotation, 0, 0, 1);
            canvas.translate(-width / 2, -height / 2);
        }

        if (rotation == 90 || rotation == 270) {
            content.draw(canvas, 0, 0, height, width);
        } else {
            content.draw(canvas, 0, 0, width, height);
        }
        canvas.restore();
    }


    protected int renderOverlay(GLCanvas canvas, int index, AlbumSetEntry entry, int width,
            int height, boolean enableFancy) {
        int renderRequestFlags = 0;
        if (entry.album != null && entry.album.isCameraRoll()) {
            int uncoveredHeight = height - mLabelSpec.labelBackgroundHeight;
            if (enableFancy) {
                int screenHeight = Math.max(FancyHelper.getHeightPixels(),
                        FancyHelper.getWidthPixels());
                int dim = (int) (FancyHelper.FANCY_CAMERA_ICON_SIZE_RATE * (float) screenHeight);
                mCameraOverlay.draw(canvas, (width - dim) / 2,
                        (height - dim) / 2, dim, dim);
            } else {
                int dim = uncoveredHeight / 2;
                mCameraOverlay.draw(canvas, (width - dim) / 2,
                        (uncoveredHeight - dim) / 2, dim, dim);
            }
        }
        if (mPressedIndex == index) {
            if (mAnimatePressedUp) {
                drawPressedUpFrame(canvas, width, height);
                renderRequestFlags |= SlotView.RENDER_MORE_FRAME;
                if (isPressedUpFrameFinished()) {
                    mAnimatePressedUp = false;
                    mPressedIndex = -1;
                }
            } else {
                drawPressedFrame(canvas, width, height);
            }
        } else if ((mHighlightItemPath != null) && (mHighlightItemPath == entry.setPath)) {
            drawSelectedFrame(canvas, width, height);
        } else if (mInSelectionMode && mSelectionManager.isItemSelected(entry.setPath)) {
            drawSelectedFrame(canvas, width, height);
        }
        return renderRequestFlags;
    }

    protected int renderContent(GLCanvas canvas, int index, AlbumSetEntry entry, int width,
            int height, boolean enableFancy) {
        int renderRequestFlags = 0;

        Texture content = checkContentTexture(entry.content);
        if (content == null) {
            content = mWaitLoadingTexture;
            if (enableFancy) {
                mWaitLoadingTexture.setSize(width, height);
            }
            entry.isWaitLoadingDisplayed = true;
        } else if (entry.isWaitLoadingDisplayed) {
            entry.isWaitLoadingDisplayed = false;
            // content = new FadeInTexture(mPlaceholderColor, entry.bitmapTexture);
            content = entry.bitmapTexture;
            entry.content = content;
        }
        if (enableFancy) {
            //placeholder, set roation to 0
            int rotation = entry.isWaitLoadingDisplayed ? 0 : entry.rotation;
            drawContent(canvas, content, width, height, rotation);
        } else {
            super.drawContent(canvas, content, width, height, entry.rotation);
        }
        if ((content instanceof FadeInTexture) &&
                ((FadeInTexture) content).isAnimating()) {
            renderRequestFlags |= SlotView.RENDER_MORE_FRAME;
        }
        return renderRequestFlags;
    }
    /// @}
}
