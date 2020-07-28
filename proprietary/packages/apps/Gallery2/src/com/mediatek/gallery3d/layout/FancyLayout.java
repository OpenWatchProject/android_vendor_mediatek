package com.mediatek.gallery3d.layout;

import android.graphics.Rect;

import com.android.gallery3d.common.Utils;
import com.android.gallery3d.data.MediaItem;
import com.android.gallery3d.ui.AlbumSetSlotRenderer;
import com.android.gallery3d.ui.SlotView;
import com.android.gallery3d.ui.SlotView.SlotEntry;
import com.android.gallery3d.ui.SlotView.SlotRenderer;
import com.android.gallery3d.ui.SlotView.Spec;
import com.mediatek.gallery3d.util.FeatureConfig;
import com.mediatek.gallery3d.util.Log;

import java.util.ArrayList;
import java.util.HashMap;

/**
 * FancyLayout.
 */
public class FancyLayout extends Layout {
    private static final String TAG = "MtkGallery2/FancyLayout";
    private static final int VISIBLE_INVALID = -2;
    private static final int VISIBLE_AT_LEFT = -1;
    private static final int SLOT_IS_VISIBLE = 0;
    private static final int VISIBLE_AT_RIGHT = 1;

    private static final int CALC_VISIBLE_START = 0;
    private static final int CALC_VISIBLE_END = 1;
    private static final int CALC_VISIBLE_DONE = 2;

    private static final int LEFT_COLUMN = 0;
    private static final int RIGHT_COLUMN = 1;

    private int mVisibleStart;
    private int mVisibleEnd;
    private int mSlotCount;
    private int mSlotWidth;
    private int mSlotGap;

    private Spec mSpec;
    private int mPaddingTop;
    private int mPaddingBottom;

    private int mWidth;
    private int mHeight;
    private int mScrollPosition;
    private boolean mForceRefreshFlag = false;
    private volatile int mContentLength;
    private volatile boolean mIsLandCameraFolder = false;

    private ArrayList<SlotEntry> mSlotArray;
    private HashMap<Integer, ArrayList<SlotEntry>> mSlotMapByColumn;
    private Object mSlotMapByColumnLock = new Object();

    private SlotRenderer mRenderer;
    private Rect mTempRect = new Rect();

    /// M: [FEATURE.ADD] Multi-window. @{
    /**
     * Constructor.
     * @param layoutType layout type
     */
    public FancyLayout(int layoutType) {
        super(layoutType);
    }

    /**
     * Constructor.
     */
    public FancyLayout() {
        super();
    }
    /// @}

    @Override
    public void onDataChange(int index, MediaItem item, int size, boolean isCameraFolder) {
        refreshSlotMap(index);
        updateVisibleSlotRange(Math.min(mScrollPosition, getScrollLimit()));
    }

    /**
     * Clear ColumnArray.
     * @param index    start index to clear
     * @param clearAll clear whole map if true
     */
    public void clearColumnArray(int index, boolean clearAll) {
        synchronized (mSlotMapByColumnLock) {
            if (mSlotMapByColumn == null || mSlotArray == null) {
                return;
            }
            for (int i = 0; i < SlotView.COL_NUM; i++) {
                ArrayList<SlotEntry> array = mSlotMapByColumn.get(i);
                if (array == null) {
                    return;
                }
                if (clearAll) {
                    array.clear();
                } else {
                    int arraySize = array.size();
                    for (int j = arraySize - 1; j >= 0; j--) {
                        if (array.get(j).slotIndex >= index) {
                            array.remove(j);
                        }
                    }
                }
            }
        }
    }

    /**
     * Set SlotArray to fancy layout.
     * @param list   list containing slotEntry
     * @param colMap map that contain 2 list, one for left col and one for right col
     */
    public void setSlotArray(ArrayList<SlotEntry> list,
            HashMap<Integer, ArrayList<SlotEntry>> colMap) {
        synchronized (mSlotMapByColumnLock) {
            mSlotArray = list;
            mSlotMapByColumn = colMap;
        }
    }

    public void setSlotRenderer(SlotRenderer renderer) {
        mRenderer = renderer;
    }

    @Override
    public void setScrollPosition(int position) {
        if (!mForceRefreshFlag && mScrollPosition == position) {
            return;
        }
        mScrollPosition = position;
        updateVisibleSlotRange(position);
    }

    @Override
    public int getSlotWidth() {
        return mSlotWidth;
    }

    @Override
    public int getScrollLimit() {
        return Math.max(0, mContentLength - mHeight);
    }

    @Override
    public void setSlotSpec(Spec spec) {
        mSpec = spec;
        mSlotGap = spec.slotGap;
    }

    @Override
    public void setPaddingSpec(int paddingTop, int paddingBottom) {
        mPaddingTop = paddingTop;
        mPaddingBottom = paddingBottom;
        Log.d(TAG, "<setPaddingSpec> <Fancy> paddingTop " + paddingTop + ", paddingBottom "
                + paddingBottom);
    }

    @Override
    public Spec getSlotSpec() {
        return mSpec;
    }

    @Override
    public int getViewWidth() {
        return mWidth;
    }

    @Override
    public int getViewHeight() {
        return mHeight;
    }

    @Override
    public int getSlotGap() {
        return mSlotGap;
    }

    @Override
    public boolean setSlotCount(int slotCount) {
        if (!mForceRefreshFlag && slotCount == mSlotCount) {
            return false;
        }
        mSlotCount = slotCount;
        setVisibleRange(0, Math.min(mSlotCount, AlbumSetSlotRenderer.CACHE_SIZE));
        Log.d(TAG, "<setSlotCount> <Fancy> slotCount " + slotCount);
        return true;
    }

    @Override
    public int getSlotCount() {
        return mSlotCount;
    }

    @Override
    public void setSize(int width, int height) {
        // mWidth: view width; mHeight: view height
        mWidth = width;
        mHeight = height;
        //sViewHeight = height;
        /// M: [BUG.ADD] multi-window @{
        FancyHelper.doFancyInitialization(mWidth, mHeight);
        mSlotWidth = (mWidth - (SlotView.COL_NUM - 1) * mSlotGap) / 2;
        // width may change if rotate DialogPicker, upate SlotEntry
        Log.d(TAG, "<setSize> <Fancy> getScreenWidthAtFancyMode "
                + FancyHelper.getScreenWidthAtFancyMode());
        if (FeatureConfig.IS_TABLET && mSlotArray != null
                && (width < height)) {
        /// @}
            // don't update when width >= height (Landscape), should use orientation instead
            for (SlotEntry slotEntry : mSlotArray) {
                slotEntry.update(mSlotWidth, mSlotGap);
            }
            clearColumnArray(0, true);
            refreshSlotMap(0);
            updateVisibleSlotRange(Math.min(mScrollPosition, getScrollLimit()));
        }
        // add this to refresh label size, otherwise the label will not be displayed
        if (mRenderer != null) {
            mRenderer.onSlotSizeChanged(getSlotWidth(), getSlotHeight());
        }
    }

    /**
     * Refresh SlotMap.
     * put landCameraFolder into the left column, for the right
     * column, we need to add landCameraFolder height when refreshSlotMap(),
     * getSlotIndexByPosition(), and getSlotRect()
     *
     * @param index start index for refreshing
     */
    public void refreshSlotMap(int index) {
        synchronized (mSlotMapByColumnLock) {
            if (mSlotMapByColumn == null || mSlotArray == null) {
                return;
            }

            int leftSum = 0;
            int rightSum = 0;
            SlotEntry entry = null;
            int rectLeft = 0;
            int rectTop = 0;
            int rectRight = 0;
            int rectBottom = 0;

            ArrayList<SlotEntry> left = mSlotMapByColumn.get(LEFT_COLUMN);
            ArrayList<SlotEntry> right = mSlotMapByColumn.get(RIGHT_COLUMN);
            if (left == null || right == null) {
                return;
            }

            for (int i = index; i < mSlotArray.size(); i++) {
                entry = mSlotArray.get(i);
                if (i == 0) {
                    entry.inWhichCol = LEFT_COLUMN;
                    entry.inWhichRow = 0;
                    rectLeft = 0;
                    rectTop = 0;
                    rectRight = entry.scaledWidth;
                    rectBottom = entry.scaledHeight;
                    if (entry.slotRect == null) {
                        entry.slotRect = new Rect(rectLeft, rectTop, rectRight, rectBottom);
                    } else {
                        entry.slotRect.set(rectLeft, rectTop, rectRight, rectBottom);
                    }
                    leftSum = entry.scaledHeight;
                    left.add(0, entry);
                    if (entry.isLandCameraFolder) {
                        mIsLandCameraFolder = true;
                    } else {
                        mIsLandCameraFolder = false;
                    }
                    continue;
                }
                if (i == 1) {
                    if (mIsLandCameraFolder) {
                        entry.inWhichCol = LEFT_COLUMN;
                        entry.inWhichRow = 1;
                        rectLeft = 0;
                        rectTop = left.get(0).slotRect.bottom + mSlotGap;
                        rectRight = entry.scaledWidth;
                        rectBottom = rectTop + entry.scaledHeight;
                        if (entry.slotRect == null) {
                            entry.slotRect = new Rect(rectLeft, rectTop, rectRight, rectBottom);
                        } else {
                            entry.slotRect.set(rectLeft, rectTop, rectRight, rectBottom);
                        }
                        leftSum = left.get(0).scaledHeight + entry.scaledHeight + mSlotGap;
                        left.add(1, entry);
                    } else {
                        entry.inWhichCol = RIGHT_COLUMN;
                        entry.inWhichRow = 0;
                        rectLeft = entry.scaledWidth + mSlotGap;
                        rectTop = 0;
                        rectRight = rectLeft + entry.scaledWidth;
                        rectBottom = entry.scaledHeight;
                        if (entry.slotRect == null) {
                            entry.slotRect = new Rect(rectLeft, rectTop, rectRight, rectBottom);
                        } else {
                            entry.slotRect.set(rectLeft, rectTop, rectRight, rectBottom);
                        }
                        leftSum = left.get(0).scaledHeight;
                        rightSum = entry.scaledHeight;
                        right.add(0, entry);
                    }
                    continue;
                }
                if (i == 2 && mIsLandCameraFolder) {
                    entry.inWhichCol = RIGHT_COLUMN;
                    entry.inWhichRow = 0;
                    rectLeft = entry.scaledWidth + mSlotGap;
                    rectTop = left.get(0).scaledHeight + mSlotGap;
                    rectRight = rectLeft + entry.scaledWidth;
                    rectBottom = rectTop + entry.scaledHeight;
                    if (entry.slotRect == null) {
                        entry.slotRect = new Rect(rectLeft, rectTop, rectRight, rectBottom);
                    } else {
                        entry.slotRect.set(rectLeft, rectTop, rectRight, rectBottom);
                    }
                    leftSum = left.get(0).scaledHeight + mSlotGap + left.get(1).scaledHeight;
                    rightSum = left.get(0).scaledHeight + mSlotGap + entry.scaledHeight;
                    right.add(0, entry);
                    continue;
                }
                /*
                 * when insert another index, e.g.5,
                 * we should check the sum of height of left col or the right col
                 * if left of left < right, then put index 5 to left col
                 * if left of left > right, then put index 5 to right col
                 * specially,
                 * if left of left = right, also put index 5 to left col
                 */
                leftSum = left.get(left.size() - 1).slotRect.bottom;
                rightSum = right.get(right.size() - 1).slotRect.bottom;
                if (leftSum <= rightSum) {
                    entry.inWhichCol = LEFT_COLUMN;
                    entry.inWhichRow = left.size();
                    rectLeft = 0;
                    rectTop = leftSum + mSlotGap;
                    rectRight = rectLeft + entry.scaledWidth;
                    rectBottom = rectTop + entry.scaledHeight;
                    if (entry.slotRect == null) {
                        entry.slotRect = new Rect(rectLeft, rectTop, rectRight, rectBottom);
                    } else {
                        entry.slotRect.set(rectLeft, rectTop, rectRight, rectBottom);
                    }
                    left.add(left.size(), entry);
                    leftSum = rectBottom;
                } else {
                    entry.inWhichCol = RIGHT_COLUMN;
                    entry.inWhichRow = right.size();
                    rectLeft = entry.scaledWidth + mSlotGap;
                    rectTop = rightSum + mSlotGap;
                    rectRight = rectLeft + entry.scaledWidth;
                    rectBottom = rectTop + entry.scaledHeight;
                    if (entry.slotRect == null) {
                        entry.slotRect = new Rect(rectLeft, rectTop, rectRight, rectBottom);
                    } else {
                        entry.slotRect.set(rectLeft, rectTop, rectRight, rectBottom);
                    }
                    right.add(right.size(), entry);
                    rightSum = rectBottom;
                }
            }
            mContentLength = Math.max(leftSum, rightSum);
        }
    }

    @Override
    public Rect getSlotRect(int index, Rect rect) {
        if (mSlotArray == null || index < 0 || index >= mSlotArray.size()
                || mSlotArray.get(index).slotRect == null) {
            return new Rect(0, 0, 1, 1);
        }
        Rect cachedRect = mSlotArray.get(index).slotRect;
        if (rect != null) {
            rect.set(cachedRect.left, cachedRect.top, cachedRect.right, cachedRect.bottom);
        } else {
            rect = new Rect(cachedRect.left, cachedRect.top, cachedRect.right, cachedRect.bottom);
        }
        return rect;
    }

    @Override
    public int getSlotIndexByPosition(float x, float y) {
        Rect rect =  null;
        for (int i = mVisibleStart; i < mVisibleEnd; i++) {
            rect = getSlotRect(i, mTempRect);
            if (rect.contains(Math.round(x), Math.round(y + mScrollPosition))) {
                return i;
            }
        }
        return -1;
    }

    @Override
    public int getVisibleStart() {
        return mVisibleStart;
    }

    @Override
    public int getVisibleEnd() {
        return mVisibleEnd;
    }

    public boolean isFancyLayout() {
        return true;
    }

    public void setForceRefreshFlag(boolean needForceRefresh) {
        mForceRefreshFlag = needForceRefresh;
    }

    /**
     * Set ViewHeight.
     * @param height view height
     */
    public void setViewHeight(int height) {
        mHeight = height;
        Log.d(TAG, "<setViewHeight> <Fancy> mHeight " + mHeight);
    }

    @Override
    public void updateSlotCount(int slotCount) {
        mSlotCount = slotCount;
        Log.d(TAG, "<updateSlotCount> <Fancy> slotCount " + slotCount);
    }


    /**
     * Check current index visible or not.
     *
     * @param index slot index
     * @param scrollPosition scroll bar position
     * @return VISIBLE_INVALID , means invalid value
     *         VISIBLE_AT_LEFT , means visibleSlot at the left side of input index
     *         VISIBLE_AT_RIGHT, means visibleSlot at the right side of input index
     *         SLOT_IS_VISIBLE , means current slot is visible
     */
    private int isVisible(int index, int scrollPosition) {
        Rect rect = getSlotRect(index, mTempRect);
        if (rect == null) {
            return VISIBLE_INVALID;
        }

        //int viewTop = mScrollPosition - sActionBarHeight - mSlotGap;
        int viewTop = Math.max(0, scrollPosition - sActionBarHeight - mPaddingTop);
        int viewBottom = scrollPosition + mHeight + mPaddingBottom;
        if ((rect.top <= viewTop && viewTop < rect.bottom)
                || (rect.top < viewBottom && viewBottom <= rect.bottom)
                || (viewTop < rect.top && rect.bottom < viewBottom)) {
            return SLOT_IS_VISIBLE;
        } else if (rect.top >= viewBottom) {
            return VISIBLE_AT_LEFT;
        } else if (rect.bottom <= viewTop) {
            return VISIBLE_AT_RIGHT;
        }
        return VISIBLE_INVALID;
    }

    private int[] calcVisibleStartAndEnd(int scrollPosition) {
        int[] visibleArray = new int[2];
            synchronized (mSlotMapByColumnLock) {
            if (mSlotArray == null || mSlotArray.size() == 0
                    || mSlotMapByColumn == null || mSlotMapByColumn.size() == 0
                    || mSlotMapByColumn.get(LEFT_COLUMN).size() == 0) {
                visibleArray[0] = visibleArray[1] = 0;
                return visibleArray;
            }

            if (mSlotArray.size() == 1) {
                visibleArray[0] = 0;
                visibleArray[1] = 1;
                return visibleArray;
            }

            if (mSlotArray.size() == 2) {
                visibleArray[0] = 0;
                visibleArray[1] = 2;
                return visibleArray;
            }

            int slotIndex = 0;
            int start = 0;
            int end = 0;
            int halfIndex = 0;
            int direction = VISIBLE_INVALID;
            int visibleStartInLeftCol = 0;
            int visibleEndInLeftCol = 0;
            int visibleStartInRightCol = 0;
            int visibleEndInRightCol = 0;

            for (int i = LEFT_COLUMN; i < SlotView.COL_NUM; i++) {
                for (int j = CALC_VISIBLE_START; j < CALC_VISIBLE_DONE; j++) {
                    start = 0;
                    end = Math.max(0, mSlotMapByColumn.get(i).size() - 1);
                    halfIndex = (start + end) / 2;
                    direction = VISIBLE_INVALID;

                    do {
                        direction = isVisible(mSlotMapByColumn.get(i).get(halfIndex).slotIndex,
                                scrollPosition);
                        if (direction == VISIBLE_AT_LEFT) {
                            end  = halfIndex;
                        } else if (direction == VISIBLE_AT_RIGHT) {
                            start = halfIndex;
                        } else if (direction == SLOT_IS_VISIBLE) {
                            if (j == CALC_VISIBLE_START) {
                                end  = halfIndex;
                            } else {
                                start = halfIndex;
                            }
                        }
                        halfIndex = (start + end) / 2;
                    } while (!(start == end || end == start + 1));

                    int targetIndex = 0;
                    if (j == CALC_VISIBLE_START) {
                        targetIndex = end;
                        if (SLOT_IS_VISIBLE == isVisible(
                                mSlotMapByColumn.get(i).get(Math.max(end - 1, 0)).slotIndex,
                                scrollPosition)) {
                            targetIndex = Math.max(end - 1, 0);
                        }
                    } else if (j == CALC_VISIBLE_END) {
                        targetIndex = start;
                        if (SLOT_IS_VISIBLE == isVisible(
                                mSlotMapByColumn.get(i).get(
                                        Math.min(start + 1, mSlotMapByColumn.get(i).size() - 1))
                                        .slotIndex, scrollPosition)) {
                            targetIndex = Math.min(start + 1, mSlotMapByColumn.get(i).size() - 1);
                        }
                    }

                    // when find array index, still need change array index to slotIndex
                    if (i == LEFT_COLUMN && j == CALC_VISIBLE_START) {
                        visibleStartInLeftCol = mSlotMapByColumn.get(i).get(targetIndex).slotIndex;
                    } else if (i == LEFT_COLUMN && j == CALC_VISIBLE_END) {
                        visibleEndInLeftCol = mSlotMapByColumn.get(i).get(targetIndex).slotIndex;
                    } else if (i == RIGHT_COLUMN && j == CALC_VISIBLE_START) {
                        visibleStartInRightCol = mSlotMapByColumn.get(i).get(targetIndex).slotIndex;
                    } else if (i == RIGHT_COLUMN && j == CALC_VISIBLE_END) {
                        visibleEndInRightCol = mSlotMapByColumn.get(i).get(targetIndex).slotIndex;
                    }
                }
            }

            visibleArray[0] = Utils.clamp(Math.min(visibleStartInLeftCol, visibleStartInRightCol),
                    0, mSlotArray.size() - 1);
            // slotview render from "visible start" to "visible end - 1"
            visibleArray[1] = Utils.clamp(Math.max(visibleEndInLeftCol, visibleEndInRightCol) + 1,
                    visibleArray[0] + 1, mSlotArray.size());
            // error handle
            visibleArray[1] = Math.min(visibleArray[1], visibleArray[0]
                    + AlbumSetSlotRenderer.CACHE_SIZE);
        }
        return visibleArray;
    }

    private synchronized void updateVisibleSlotRange(int scrollPosition) {
        int[] visibleRange = calcVisibleStartAndEnd(scrollPosition);
        if (mSlotArray == null || (mSlotArray != null && mSlotArray.size() == 0)) {
            Log.d(TAG, "<updateVisibleSlotRange> <Fancy> set visible as [0, 0], mSlotArray "
                    + mSlotArray);
            setVisibleRange(0, 0);
        }
        int start = visibleRange[0];
        int end = visibleRange[1];
        if (start <= end && end - start <= AlbumSetSlotRenderer.CACHE_SIZE
                && end <= mSlotArray.size()) {
            setVisibleRange(start, end);
        } else {
            Log.d(TAG,
                    "<updateVisibleSlotRange> <Fancy> correct visible range: mSlotArray.size() "
                            + mSlotArray.size() + ", [" + start + ", " + end + "] -> [0" + ", "
                            + Math.min(mSlotArray.size(), AlbumSetSlotRenderer.CACHE_SIZE) + "]");
            setVisibleRange(0, Math.min(mSlotArray.size(),
                    AlbumSetSlotRenderer.CACHE_SIZE));
        }
    }

    /**
     * Set VisibleRange.
     * when switch between different layout,
     * e.g. Fancy layout visible range:3~7, so set slidding window active window as (3~7)
     * Default layout visible range:4~8, so set slidding window active window as (4~8)
     * so change condition from <if (start == mVisibleStart && end == mVisibleEnd) return;>
     * to <if (start == SlotView.mVisibleStart && end == SlotView.mVisibleEnd) return;>>
     * @param start visible start
     * @param end   visible end
     */
    private void setVisibleRange(int start, int end) {
        if (!mForceRefreshFlag && start == mVisibleStart && end == mVisibleEnd) {
            return;
        }
        if (start < end) {
            mVisibleStart = start;
            mVisibleEnd = end;
        } else {
            mVisibleStart = mVisibleEnd = 0;
        }
        if (mRenderer != null) {
            mRenderer.onVisibleRangeChanged(mVisibleStart, mVisibleEnd);
        }
    }
}
