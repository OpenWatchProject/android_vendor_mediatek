/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/

/*
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

package com.android.music;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.database.ContentObserver;
import android.graphics.Bitmap;
import android.graphics.PixelFormat;
import android.graphics.Rect;
import android.graphics.drawable.Drawable;
import android.os.Handler;
import android.provider.MediaStore;
import android.util.AttributeSet;
//Not required
//import android.util.Log;
import android.view.GestureDetector;
import android.view.GestureDetector.SimpleOnGestureListener;
import android.view.Gravity;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewConfiguration;
import android.view.ViewGroup;
import android.view.ViewTreeObserver.OnPreDrawListener;
import android.view.WindowManager;
//Not required
//import android.view.GestureDetector.SimpleOnGestureListener;
import android.widget.AdapterView;
import android.widget.ImageView;
import android.widget.ListView;

public class TouchInterceptor extends ListView {

    private static final String TAG = "TouchInterceptor";
    private ImageView mDragView;
    private WindowManager mWindowManager;
    private WindowManager.LayoutParams mWindowParams;
    /**
     * At which position is the item currently being dragged. Note that this
     * takes in to account header items.
     */
    private int mDragPos;
    /**
     * At which position was the item being dragged originally
     */
    private int mSrcDragPos;
    private int mDragPointX;    // at what x offset inside the item did the user grab it
    private int mDragPointY;    // at what y offset inside the item did the user grab it
    private int mXOffset;  // the difference between screen coordinates and coordinates in this view
    private int mYOffset;  // the difference between screen coordinates and coordinates in this view
    private DragListener mDragListener;
    private DropListener mDropListener;
    private RemoveListener mRemoveListener;
    private int mUpperBound;
    private int mLowerBound;
    private int mHeight;
    private GestureDetector mGestureDetector;
    private static final int FLING = 0;
    private static final int SLIDE = 1;
    private static final int TRASH = 2;
    private int mRemoveMode = -1;
    private Rect mTempRect = new Rect();
    private Bitmap mDragBitmap;
    private final int mTouchSlop;
    private int mItemHeightNormal;
    private int mItemHeightExpanded;
    private int mItemHeightHalf;
    private Drawable mTrashcan;


    /// M: Add to wait for listview finish measured, so that ap can get accurate width and height
    private Object mSizeChangedLock = new Object();
    private int mListviewWidth = 0;
    private int mListViewHeight = 0;
    private int mSrcDragPosition;

    private static final int LAST_BUT_ONE_POSITION = 2;
    private static final int MIN_MOVE_DIV = 3;
    /// M: whether need draw the view.
    private boolean mIsDraw = true;

    /// M: Upgrade album art listener
    private UpgradeAlbumArtListener mUpgradeAlbumArtListener;
    /**
     *  M: add content observer to monitor media file change.
     */
    private final ContentObserver mContentObserver = new ContentObserver(new Handler()) {
        @Override
        public void onChange(boolean selfChange) {
            super.onChange(selfChange);
            mIsDraw = true;
            MusicLogUtils.v(TAG, "onChange: mIsDraw = " + mIsDraw);
        }
    };
    /**
     * M: check if true,draw the view,otherwise stop draw view,to avoid the item
     * flashes should stop draw view and then draw the view after the database changed.
     *
     */
    OnPreDrawListener mPreDrawListener = new OnPreDrawListener() {

        @Override
        public boolean onPreDraw() {
            MusicLogUtils.v(TAG, "onPreDraw: mIsDraw = " + mIsDraw);
            return mIsDraw;
        }
    };

    public TouchInterceptor(Context context, AttributeSet attrs) {
        super(context, attrs);
        SharedPreferences pref = context.getSharedPreferences("Music", 0);
        mRemoveMode = pref.getInt("deletemode", -1);
        mTouchSlop = ViewConfiguration.get(context).getScaledTouchSlop();
        Resources res = getResources();
        mItemHeightNormal = res.getDimensionPixelSize(R.dimen.normal_height);
        mItemHeightHalf = mItemHeightNormal / 2;
        mItemHeightExpanded = res.getDimensionPixelSize(R.dimen.expanded_height);

        /// M: Monitor draw the view or stop draw view when needed.
        getRootView().getViewTreeObserver().addOnPreDrawListener(mPreDrawListener);
        /// @}
    }

    @Override
    public boolean onInterceptTouchEvent(MotionEvent ev) {
        if (mRemoveListener != null && mGestureDetector == null) {
            if (mRemoveMode == FLING) {
                mGestureDetector = new GestureDetector(getContext(), new SimpleOnGestureListener() {
                    @Override
                    public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX,
                            float velocityY) {
                        if (mDragView != null) {
                            if (velocityX > 1000) {
                                Rect r = mTempRect;
                                mDragView.getDrawingRect(r);
                                if (e2.getX() > r.right * 2.0 / 3.0) {
                                // fast fling right with release near the right edge of the screen
                                    stopDragging();
                                    mRemoveListener.remove(mSrcDragPos);
                                    unExpandViews(true);
                                }
                            }
                            // flinging while dragging should have no effect
                            return true;
                        }
                        return false;
                    }
                });
            }
        }
        if (mDragListener != null || mDropListener != null) {
            switch (ev.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    int x = (int) ev.getX();
                    int y = (int) ev.getY();
                    int itemnum = pointToPosition(x, y);
                    if (itemnum == AdapterView.INVALID_POSITION) {
                        break;
                    }
                    ViewGroup item = (ViewGroup) getChildAt(itemnum - getFirstVisiblePosition());
                    mDragPointX = x - item.getLeft();
                    mDragPointY = y - item.getTop();
                    int[] location = new int[2];
                    getLocationInWindow(location);
                    mXOffset = location[0];
                    mYOffset = location[1];
                    // The left side of the item is the grabber for dragging the item
                    if (x < 64) {
                        /// M: clear the double buffer at first.
                        item.setDrawingCacheEnabled(false);
                        item.setDrawingCacheEnabled(true);
                        // Create a copy of the drawing cache so that it does not get recycled
                        // by the framework when the list tries to clean up memory
                        Bitmap bitmap = Bitmap.createBitmap(item.getDrawingCache());
                        mSrcDragPosition = y;
                        startDragging(bitmap, x, y);
                        mDragPos = itemnum;
                        mSrcDragPos = mDragPos;
                        mHeight = getHeight();
                        int touchSlop = mTouchSlop;
                        mUpperBound = Math.min(y - touchSlop, mHeight / 3);
                        mLowerBound = Math.max(y + touchSlop, mHeight * 2 / 3);
                        return false;
                    }
                    stopDragging();
                    break;
            }
        }
        return super.onInterceptTouchEvent(ev);
    }

    /*
     * pointToPosition() doesn't consider invisible views, but we
     * need to, so implement a slightly different version.
     */
    private int myPointToPosition(int x, int y) {

        if (y < 0) {
            // when dragging off the top of the screen, calculate position
            // by going back from a visible item
            int pos = myPointToPosition(x, y + mItemHeightNormal);
            if (pos > 0) {
                return pos - 1;
            }
        }

        Rect frame = mTempRect;
        final int count = getChildCount();
        for (int i = count - 1; i >= 0; i--) {
            final View child = getChildAt(i);
            child.getHitRect(frame);
            if (frame.contains(x, y)) {
                return getFirstVisiblePosition() + i;
            }
        }
        return INVALID_POSITION;
    }

    private int getItemForPosition(int y) {
        int adjustedy = y - mDragPointY - mItemHeightHalf;
        int pos = myPointToPosition(0, adjustedy);
        if (pos >= 0) {
            if (pos <= mSrcDragPos) {
                pos += 1;
            }
        } else if (adjustedy < 0) {
            // this shouldn't happen anymore now that myPointToPosition deals
            // with this situation
            pos = 0;
        }
        /// M: For move to bottom easy, add the bottom case check
        /// if the position is in last item area, move position is larger then Item height/3,
        /// and the number is last but one, we add the number. {@
        if ((y > mHeight - mItemHeightNormal)
              && (pos == getCount() - LAST_BUT_ONE_POSITION)
              && (y >= mSrcDragPosition + mItemHeightNormal / MIN_MOVE_DIV)) {
             pos += 1;
        }
        /// @}
        return pos;
    }

    private void adjustScrollBounds(int y) {
        if (y >= mHeight / 3) {
            mUpperBound = mHeight / 3;
        }
        if (y <= mHeight * 2 / 3) {
            mLowerBound = mHeight * 2 / 3;
        }
    }

    /*
     * Restore size and visibility for all listitems
     */
    private void unExpandViews(boolean deletion) {
        /// M: force children to be recreated, so that can get right view form it.
        try {
            layoutChildren();
        } catch (IllegalStateException e) {
            MusicLogUtils.v(TAG, "layoutChildren IllegalStateException", e);
        }

        for (int i = 0;; i++) {
            View v = getChildAt(i);
            if (v == null) {
                if (deletion) {
                    // HACK force update of mItemCount
                    int position = getFirstVisiblePosition();
                    int y = getChildAt(0).getTop();
                    setAdapter(getAdapter());
                    setSelectionFromTop(position, y);
                    // end hack
                }
                try {
                    layoutChildren(); // force children to be recreated where needed
                    v = getChildAt(i);
                } catch (IllegalStateException ex) {
                    // layoutChildren throws this sometimes, presumably because we're
                    // in the process of being torn down but are still getting touch
                    // events
                }
                if (v == null) {
                    MusicLogUtils.v(TAG, "unExpandViews with null view, return");
                    return;
                }
            }
            ViewGroup.LayoutParams params = v.getLayoutParams();
            params.height = mItemHeightNormal;
            v.setLayoutParams(params);
            v.setVisibility(View.VISIBLE);
        }
    }

    /* Adjust visibility and size to make it appear as though
     * an item is being dragged around and other items are making
     * room for it:
     * If dropping the item would result in it still being in the
     * same place, then make the dragged listitem's size normal,
     * but make the item invisible.
     * Otherwise, if the dragged listitem is still on screen, make
     * it as small as possible and expand the item below the insert
     * point.
     * If the dragged item is not on screen, only expand the item
     * below the current insertpoint.
     */
    private void doExpansion() {
        int childnum = mDragPos - getFirstVisiblePosition();
        if (mDragPos > mSrcDragPos) {
            childnum++;
        }
        int numheaders = getHeaderViewsCount();

        View first = getChildAt(mSrcDragPos - getFirstVisiblePosition());
        for (int i = 0;; i++) {
            View vv = getChildAt(i);
            if (vv == null) {
                break;
            }

            int height = mItemHeightNormal;
            int visibility = View.VISIBLE;
            if (mDragPos < numheaders && i == numheaders) {
                // dragging on top of the header item, so adjust the item below
                // instead
                if (vv.equals(first)) {
                    visibility = View.INVISIBLE;
                } else {
                    height = mItemHeightExpanded;
                }
            } else if (vv.equals(first)) {
                // processing the item that is being dragged
                if (mDragPos == mSrcDragPos || getPositionForView(vv) == getCount() - 1) {
                    // hovering over the original location
                    visibility = View.INVISIBLE;
                } else {
                    // not hovering over it
                    // Ideally the item would be completely gone, but neither
                    // setting its size to 0 nor settings visibility to GONE
                    // has the desired effect.
                    height = 1;
                    /**M: add to resolve cr ALPS02299803 @{**/
                    visibility = View.INVISIBLE;
                    /** @}**/
                }
            } else if (i == childnum) {
                if (mDragPos >= numheaders && mDragPos < getCount() - 1) {
                    height = mItemHeightExpanded;
                }
            }
            ViewGroup.LayoutParams params = vv.getLayoutParams();
            params.height = height;
            vv.setLayoutParams(params);
            vv.setVisibility(visibility);
        }
    }

    @Override
    public boolean onTouchEvent(MotionEvent ev) {
        if (mGestureDetector != null) {
            mGestureDetector.onTouchEvent(ev);
        }
        if ((mDragListener != null || mDropListener != null) && mDragView != null) {
            int action = ev.getAction();
            switch (action) {
                case MotionEvent.ACTION_UP:
                case MotionEvent.ACTION_CANCEL:
                    Rect r = mTempRect;
                    mDragView.getDrawingRect(r);
                    stopDragging();
                    /// M: stop draw the view to avoid draw the old information that will cause
                    //     the item flashes.
                    mIsDraw = false;
                    if (mRemoveMode == SLIDE && ev.getX() > r.right * 3.0 / 4.0) {
                        if (mRemoveListener != null) {
                            mRemoveListener.remove(mSrcDragPos);
                        }
                        unExpandViews(true);

                    } else {
                        if (mDropListener != null && mDragPos >= 0 && mDragPos < getCount()) {
                            mDropListener.drop(mSrcDragPos, mDragPos);
                        }
                        /// M: if drag the first and last item,do not need stop the draw. @{
                            /// M: mSrcDragPos == mDragPos means
                            ///the first position item be drag up that
                             // the database will not be change,so need not stop draw view.
                             // the second condition means,
                             //the last item will be drag down that the database
                             // will not be change also,so need not stop draw view.
                        if ((mSrcDragPos == mDragPos)
                                || ((mSrcDragPos == (getCount() - 1))
                                && (mDragPos >= mSrcDragPos))) {
                            mIsDraw = true;
                        }
                        /// @}
                        unExpandViews(false);
                    }
                    break;

                case MotionEvent.ACTION_DOWN:
                case MotionEvent.ACTION_MOVE:
                    int x = (int) ev.getX();
                    int y = (int) ev.getY();
                    dragView(x, y);
                    int itemnum = getItemForPosition(y);
                    if (itemnum >= 0) {
                        if (action == MotionEvent.ACTION_DOWN || itemnum != mDragPos) {
                            if (mDragListener != null) {
                                mDragListener.drag(mDragPos, itemnum);
                            }
                            mDragPos = itemnum;
                            doExpansion();
                        }
                        int speed = 0;
                        adjustScrollBounds(y);
                        if (y > mLowerBound) {
                            // scroll the list up a bit
                            if (getLastVisiblePosition() < getCount() - 1) {
                                speed = y > (mHeight + mLowerBound) / 2 ? 16 : 4;
                            } else {
                                speed = 1;
                            }
                        } else if (y < mUpperBound) {
                            // scroll the list down a bit
                            speed = y < mUpperBound / 2 ? -16 : -4;
                            if (getFirstVisiblePosition() == 0
                                    && getChildAt(0).getTop() >= getPaddingTop()) {
                                // if we're already at the top, don't try to scroll, because
                                // it causes the framework to do some extra drawing that messes
                                // up our animation
                                speed = 0;
                            }
                        }
                        if (speed != 0) {
                            smoothScrollBy(speed, 30);
                        }
                    }
                    break;
            }
            return true;
        }
        return super.onTouchEvent(ev);
    }

    private void startDragging(Bitmap bm, int x, int y) {
        MusicLogUtils.v(TAG, "startDragging ");
        stopDragging();

        mWindowParams = new WindowManager.LayoutParams();
        mWindowParams.gravity = Gravity.TOP | Gravity.LEFT;
        mWindowParams.x = x - mDragPointX + mXOffset;
        mWindowParams.y = y - mDragPointY + mYOffset;

        mWindowParams.height = WindowManager.LayoutParams.WRAP_CONTENT;
        mWindowParams.width = WindowManager.LayoutParams.WRAP_CONTENT;
        mWindowParams.flags = WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE
                | WindowManager.LayoutParams.FLAG_NOT_TOUCHABLE
                | WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON
                | WindowManager.LayoutParams.FLAG_LAYOUT_IN_SCREEN
                | WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS;
        mWindowParams.format = PixelFormat.TRANSLUCENT;
        mWindowParams.windowAnimations = 0;

        Context context = getContext();
        ImageView v = new ImageView(context);
        //int backGroundColor = context.getResources().getColor(R.color.dragndrop_background);
        //v.setBackgroundColor(backGroundColor);
        v.setBackgroundResource(R.drawable.playlist_tile_drag);
        v.setPadding(0, 0, 0, 0);
        v.setImageBitmap(bm);
        mDragBitmap = bm;

        mWindowManager = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
        mWindowManager.addView(v, mWindowParams);
        mDragView = v;
    }

    private void dragView(int x, int y) {
        if (mRemoveMode == SLIDE) {
            float alpha = 1.0f;
            int width = mDragView.getWidth();
            if (x > width / 2) {
                alpha = ((float) (width - x)) / (width / 2);
            }
            mWindowParams.alpha = alpha;
        }

        if (mRemoveMode == FLING || mRemoveMode == TRASH) {
            mWindowParams.x = x - mDragPointX + mXOffset;
        } else {
            mWindowParams.x = 0;
        }
        mWindowParams.y = y - mDragPointY + mYOffset;
        mWindowManager.updateViewLayout(mDragView, mWindowParams);

        if (mTrashcan != null) {
            int width = mDragView.getWidth();
            if (y > getHeight() * 3 / 4) {
                mTrashcan.setLevel(2);
            } else if (width > 0 && x > width / 4) {
                mTrashcan.setLevel(1);
            } else {
                mTrashcan.setLevel(0);
            }
        }
    }

    private void stopDragging() {
        MusicLogUtils.v(TAG, "stopDragging() ");
        if (mDragView != null) {
            mDragView.setVisibility(GONE);
            WindowManager wm = (WindowManager)
                getContext().getSystemService(Context.WINDOW_SERVICE);
            wm.removeView(mDragView);
            mDragView.setImageDrawable(null);
            mDragView = null;
        }
        if (mDragBitmap != null) {
            mDragBitmap.recycle();
            mDragBitmap = null;
        }
        if (mTrashcan != null) {
            mTrashcan.setLevel(0);
        }
    }

    public void setTrashcan(Drawable trash) {
        mTrashcan = trash;
        mRemoveMode = TRASH;
    }

    public void setDragListener(DragListener l) {
        mDragListener = l;
    }

    public void setDropListener(DropListener l) {
        mDropListener = l;
    }

    public void setRemoveListener(RemoveListener l) {
        mRemoveListener = l;
    }

    public interface DragListener {
        void drag(int from, int to);
    }
    public interface DropListener {
        void drop(int from, int to);
    }
    public interface RemoveListener {
        void remove(int which);
    }

    /**
     * M: Set setUpgradeAlbumArtListener
     * @param l
     */
    public void setUpgradeAlbumArtListener(UpgradeAlbumArtListener l) {
        mUpgradeAlbumArtListener = l;
    }

    /**
     * M: upgrade album art background listener, call when onSizeChanged finish.
     * @author mtk54154
     *
     */
    public interface UpgradeAlbumArtListener {
        void UpgradeAlbumArt();
    }

    /**
     * M: Call when view size changed.
     */
    @Override
    protected void onSizeChanged(int w, int h, int oldw, int oldh) {
        super.onSizeChanged(w, h, oldw, oldh);
        if ((mListviewWidth == 0) || (mListViewHeight == 0)) {
            mListviewWidth = w;
            mListViewHeight = h;
            //MusicLogUtils.v(TAG, "onSizeChanged initial:
            //new = "+ "(w" + w + ",h" + h + "), old = "
            //        +  "(w" + oldw + ",h" + oldh + ")");
            return;
        }
        if ((mListviewWidth == oldw) && (mListViewHeight == oldh)) {
            mListviewWidth = w;
            mListViewHeight = h;
            int orientation = getResources().getConfiguration().orientation;
            //MusicLogUtils.v(TAG, "onSizeChanged: new = " + "(w" + w + ",h" + h + "), old = "
            //        +  "(w" + oldw + ",h" + oldh + ")");
            synchronized (mSizeChangedLock) {
                if (mUpgradeAlbumArtListener != null
                        && (orientation == Configuration.ORIENTATION_LANDSCAPE ?
                        (mListviewWidth > mListViewHeight)
                                : (mListviewWidth < mListViewHeight))) {
                    mUpgradeAlbumArtListener.UpgradeAlbumArt();
                    MusicLogUtils.v(TAG,
                    "onSizeChanged with right size, call upgrade album art:orientation="
                    + orientation);
                }
                mSizeChangedLock.notify();
            }
        }
        //else {
        //    MusicLogUtils.v(TAG, "onSizeChanged: not finish measure: new = " + "(w" + w + ",h"
        //            + h + "), old = " + "(w" + oldw + ",h" + oldh + ")");
        //}

    }

    /**
     * M: Wait for measure finished, so that we can get the accurate width and height.
     *
     * @param isLandscape whether the device is in landscape.
     */
    public void waitMeasureFinished(boolean isLandscape) {
        if (isLandscape ? (mListviewWidth > mListViewHeight) : (mListviewWidth < mListViewHeight)) {
            /// M: The view has finished measuring, so just return
            MusicLogUtils.v(TAG, "waitMeasureFinished: " + "(w" + mListviewWidth + ",h"
                    + mListViewHeight + ")");
            return;
        }

        synchronized (mSizeChangedLock) {
            final int totalTime = 3000;
            final int waitTime = 200;
            int time = 0;
            while (!(isLandscape ? (mListviewWidth > mListViewHeight)
                    : (mListviewWidth < mListViewHeight))) {
                try {
                    mSizeChangedLock.wait(waitTime);
                    time += waitTime;
                    if (time > totalTime) {
                        break;
                    }
                    MusicLogUtils.v(TAG, "Wait for listview onMeasure finished!" + time);
                } catch (InterruptedException e) {
                    MusicLogUtils.v(TAG, "wait has been interupted " + e);
                }
            }
        }
        MusicLogUtils.v(TAG, "waitMeasureFinished: listview has finished measure!");
    }

    /**
     * M: Register the new playlist observer to monitor the database change
     *
     * @param context
     */
    void registerContentObserver(Context context) {
        context.getContentResolver().registerContentObserver(
                MediaStore.Audio.Playlists.EXTERNAL_CONTENT_URI, true, mContentObserver);
         MusicLogUtils.v(TAG, "registerContentObserver " + mContentObserver);
    }

    /**
     * M: Unregister the new playlist observer to monitor the database change
     *
     * @param context
     */
    void unregisterContentObserver(Context context) {
        context.getContentResolver().unregisterContentObserver(mContentObserver);
        MusicLogUtils.v(TAG, "unregisterContentObserver " + mContentObserver);
    }

    /**
     * M: Reset predraw status so that native will pre-draw again.
     */
    public void resetPredrawStatus() {
        mIsDraw = true;
    }
}
