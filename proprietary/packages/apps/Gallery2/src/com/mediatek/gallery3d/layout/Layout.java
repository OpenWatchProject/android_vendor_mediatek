package com.mediatek.gallery3d.layout;

import android.graphics.Rect;

import com.android.gallery3d.data.MediaItem;
import com.android.gallery3d.ui.SlotView.SlotEntry;
import com.android.gallery3d.ui.SlotView.SlotRenderer;
import com.android.gallery3d.ui.SlotView.Spec;
import com.mediatek.gallery3d.util.Log;

import java.util.ArrayList;
import java.util.HashMap;

/**
 * Gallery layout.
 */
public class Layout {
    private final static String TAG = "MtkGallery2/Layout";
    protected static int sActionBarHeight;
    protected static int sViewHeightWhenPortrait = -1;

    /// M: [FEATURE.ADD] Multi-window. @{
    private int mLayoutType = -1;

    /**
     * Constructor.
     * @param layoutType layout type
     */
    public Layout(int layoutType) {
        mLayoutType = layoutType;
    }

    /**
     * Constructor.
     */
    public Layout() {
    }

    /**
     * Get layout type.
     * @return layout type
     */
    public int getLayoutType() {
        return mLayoutType;
    }
    /// @}

    /**
     * Set ActionBarHeight.
     * @param height action bar height
     */
    public void setActionBarHeight(int height) {
        sActionBarHeight = height;
        Log.d(TAG, "<setActionBarHeight> <Fancy> sActionBarHeight "
                + sActionBarHeight);
    }

    public void setViewHeightWhenPortrait(int height) {
        sViewHeightWhenPortrait = height;
    }

    public int getViewHeightWhenPortrait() {
        return sViewHeightWhenPortrait;
    }

    /**
     * Set SlotSpec.
     * @param spec slot spec
     */
    public void setSlotSpec(Spec spec) {}

    /**
     * Set PaddingSpec.
     * @param paddingTop    padding top
     * @param paddingBottom padding bottom
     */
    public void setPaddingSpec(int paddingTop, int paddingBottom) {}

    public Spec getSlotSpec() {
        return null;
    }

    /**
     * Set SlotCount.
     * @param slotCount slot count
     * @return true if has been set
     */
    public boolean setSlotCount(int slotCount) {
        return false;
    }

    /**
     * Set ViewHeight.
     * @param height view height
     */
    public void setViewHeight(int height) {}

    /**
     * Refresh SlotMap.
     * @param index start index for refreshing
     */
    public void refreshSlotMap(int index) {}

    /**
     * Set SlotRenderer.
     * @param renderer render
     */
    public void setSlotRenderer(SlotRenderer renderer) {}

    public int getSlotCount() {
        return -1;
    }

    /**
     * Update SlotCount.
     * @param slotCount slot count
     */
    public void updateSlotCount(int slotCount) {}

    /**
     * Get SlotRect.
     * @param index slot index
     * @param rect  input rect
     * @return      slot rect
     */
    public Rect getSlotRect(int index, Rect rect) {
        return null;
    }

    public int getSlotWidth() {
        return -1;
    }

    public int getSlotHeight() {
        return -1;
    }

    /**
     * Set Size.
     * @param width  view width
     * @param height view height
     */
    public void setSize(int width, int height) {}

    /**
     * Set ScrollPosition.
     * @param position scroll position
     */
    public void setScrollPosition(int position) {}

    public int getVisibleStart() {
        return -1;
    }

    public int getVisibleEnd() {
        return -1;
    }

    /**
     * Get SlotIndexByPosition.
     * @param x x coord
     * @param y y coord
     * @return  slot index
     */
    public int getSlotIndexByPosition(float x, float y) {
        return -1;
    }

    public int getScrollLimit() {
        return -1;
    }

    /**
     * DataChange call back.
     * @param index slot index
     * @param item  cover item
     * @param size  album count
     * @param isCameraFolder true if it is landcamera cover item
     */
    public void onDataChange(int index, MediaItem item, int size,
            boolean isCameraFolder) {
    }

    /**
     * Clear ColumnArray.
     * @param index    start index
     * @param clearAll clear all if set as true
     */
    public void clearColumnArray(int index, boolean clearAll) {}

    public int getViewWidth() {
        return -1;
    }

    public int getViewHeight() {
        return -1;
    }

    public int getSlotGap() {
        return -1;
    }

    /**
     * Set ForceRefreshFlag.
     * @param needForceRefresh forceRefreshFlag
     */
    public void setForceRefreshFlag(boolean needForceRefresh) {}

    /**
     * Set SlotArray to fancy layout.
     * @param list   list containing slotEntry
     * @param colMap map that contain 2 list, one for left col and one for right col
     */
    public void setSlotArray(ArrayList<SlotEntry> list,
            HashMap<Integer, ArrayList<SlotEntry>> colMap) {
    }

    /**
     * AdvanceAnimation.
     * @param animTime animation time
     * @return true if done
     */
    public boolean advanceAnimation(long animTime) {
        return false;
    }

    /**
     * DataChangeListener.
     */
    public static interface DataChangeListener {
        /**
         * onDataChange.
         * @param index          slot index
         * @param item           cover item
         * @param size           album count
         * @param isCameraFolder is land camera folder or not
         * @param albumName      album name
         */
        public void onDataChange(int index, MediaItem item, int size,
                boolean isCameraFolder, String albumName);
    }
}