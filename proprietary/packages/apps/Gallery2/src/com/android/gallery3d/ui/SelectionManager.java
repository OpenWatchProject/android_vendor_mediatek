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

import android.os.Handler;

import com.android.gallery3d.app.AbstractGalleryActivity;
import com.android.gallery3d.data.DataManager;
import com.android.gallery3d.data.MediaItem;
import com.android.gallery3d.data.MediaSet;
import com.android.gallery3d.data.Path;
import com.android.gallery3d.util.Future;
import com.android.gallery3d.util.FutureListener;
import com.android.gallery3d.util.ThreadPool.Job;
import com.android.gallery3d.util.ThreadPool.JobContext;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.Set;

public class SelectionManager {
    @SuppressWarnings("unused")
    private static final String TAG = "Gallery2/SelectionManager";

    public static final int ENTER_SELECTION_MODE = 1;
    public static final int LEAVE_SELECTION_MODE = 2;
    public static final int SELECT_ALL_MODE = 3;
    /// M: [BEHAVIOR.ADD] @{
    // when click deselect all in menu, not leave selection mode
    public static final int DESELECT_ALL_MODE = 4;
    /// @}

    private Set<Path> mClickedSet;
    private MediaSet mSourceMediaSet;
    private SelectionListener mListener;
    private DataManager mDataManager;
    private boolean mInverseSelection;
    private boolean mIsAlbumSet;
    private boolean mInSelectionMode;
    private boolean mAutoLeave = true;
    private int mTotal;

    public interface SelectionListener {
        public void onSelectionModeChange(int mode);
        public void onSelectionChange(Path path, boolean selected);
        /// M: [BEHAVIOR.ADD] @{
        public void onSelectionRestoreDone();
        /// @}
    }

    public SelectionManager(AbstractGalleryActivity activity, boolean isAlbumSet) {
        /// M: [BEHAVIOR.ADD] @{
        mActivity = activity;
        mMainHandler = new Handler(activity.getMainLooper());
        /// @}
        mDataManager = activity.getDataManager();
        mClickedSet = new HashSet<Path>();
        mIsAlbumSet = isAlbumSet;
        mTotal = -1;
    }

    // Whether we will leave selection mode automatically once the number of
    // selected items is down to zero.
    public void setAutoLeaveSelectionMode(boolean enable) {
        mAutoLeave = enable;
    }

    public void setSelectionListener(SelectionListener listener) {
        mListener = listener;
    }

    public void selectAll() {
        Log.d(TAG, "<selectAll>");
        mInverseSelection = true;
        mClickedSet.clear();
        mTotal = -1;
        enterSelectionMode();
        if (mListener != null) mListener.onSelectionModeChange(SELECT_ALL_MODE);
    }

    public void deSelectAll() {
        Log.d(TAG, "<deSelectAll>");
        /// M: [BEHAVIOR.MARK] @{
        // when click deselect all in menu, not leave selection mode
        /* leaveSelectionMode(); */
        /// @}
        mInverseSelection = false;
        mClickedSet.clear();
        /// M: [BEHAVIOR.ADD] @{
        // when click deselect all in menu, not leave selection mode
        if (mListener != null) {
            mListener.onSelectionModeChange(DESELECT_ALL_MODE);
        }
        /// @}
    }

    public boolean inSelectAllMode() {
        /// M: [BUG.ADD] @{
        // Not in select all mode, if not all items are selected now
        if (getTotalCount() != 0) {
            return getTotalCount() == getSelectedCount();
        }
        /// @}
        return mInverseSelection;
    }

    public boolean inSelectionMode() {
        return mInSelectionMode;
    }

    public void enterSelectionMode() {
        if (mInSelectionMode) return;
        Log.d(TAG, "<enterSelectionMode>");
        mInSelectionMode = true;
        if (mListener != null) mListener.onSelectionModeChange(ENTER_SELECTION_MODE);
    }

    public void leaveSelectionMode() {
        if (!mInSelectionMode) return;

        Log.d(TAG, "<leaveSelectionMode>");
        mInSelectionMode = false;
        mInverseSelection = false;
        mClickedSet.clear();
        /// M: [BUG.ADD] @{
        // Clear mTotal so that it will be re-calculated
        // next time user enters selection mode
        mTotal = -1;
        /// @}
        /// M: [BEHAVIOR.ADD] @{
        if (mRestoreSelectionTask != null) {
            mRestoreSelectionTask.cancel();
        }
        /// @}
        if (mListener != null) mListener.onSelectionModeChange(LEAVE_SELECTION_MODE);
    }

    public boolean isItemSelected(Path itemId) {
        return mInverseSelection ^ mClickedSet.contains(itemId);
    }

    private int getTotalCount() {
        if (mSourceMediaSet == null) return -1;

        if (mTotal < 0) {
            mTotal = mIsAlbumSet
                    ? mSourceMediaSet.getSubMediaSetCount()
                    : mSourceMediaSet.getMediaItemCount();
        }
        return mTotal;
    }

    public int getSelectedCount() {
        int count = mClickedSet.size();
        if (mInverseSelection) {
            count = getTotalCount() - count;
        }
        return count;
    }

    public void toggle(Path path) {
        /// M: [DEBUG.ADD] @{
        Log.d(TAG, "<toggle> path = " + path);
        /// @}
        if (mClickedSet.contains(path)) {
            mClickedSet.remove(path);
        } else {
            enterSelectionMode();
            mClickedSet.add(path);
        }

        // Convert to inverse selection mode if everything is selected.
        int count = getSelectedCount();
        if (count == getTotalCount()) {
            selectAll();
        }

        if (mListener != null) mListener.onSelectionChange(path, isItemSelected(path));
        if (count == 0 && mAutoLeave) {
            leaveSelectionMode();
        }
    }

    private static boolean expandMediaSet(ArrayList<Path> items, MediaSet set, int maxSelection) {
        int subCount = set.getSubMediaSetCount();
        for (int i = 0; i < subCount; i++) {
            if (!expandMediaSet(items, set.getSubMediaSet(i), maxSelection)) {
                return false;
            }
        }
        int total = set.getMediaItemCount();
        int batch = 50;
        int index = 0;

        while (index < total) {
            int count = index + batch < total
                    ? batch
                    : total - index;
            ArrayList<MediaItem> list = set.getMediaItem(index, count);
            if (list != null
                    && list.size() > (maxSelection - items.size())) {
                return false;
            }
            for (MediaItem item : list) {
                items.add(item.getPath());
            }
            index += batch;
        }
        return true;
    }

    public ArrayList<Path> getSelected(boolean expandSet) {
        return getSelected(expandSet, Integer.MAX_VALUE);
    }

    /// M: [BUG.MODIFY] @{
    /*
    public ArrayList<Path> getSelected(boolean expandSet, int maxSelection) {
        ArrayList<Path> selected = new ArrayList<Path>();
    */
    public ArrayList<Path> getSelected(boolean expandSet, final int maxSelection) {
        final ArrayList<Path> selected = new ArrayList<Path>();
    /// @}
        if (mIsAlbumSet) {
            if (mInverseSelection) {
                int total = getTotalCount();
                for (int i = 0; i < total; i++) {
                    MediaSet set = mSourceMediaSet.getSubMediaSet(i);
                    /// M: [BUG.ADD] if set is null, should continue and return directly. @{
                    if (set == null) {
                        continue;
                    }
                    /// @}
                    Path id = set.getPath();
                    if (!mClickedSet.contains(id)) {
                        if (expandSet) {
                            if (!expandMediaSet(selected, set, maxSelection)) {
                                return null;
                            }
                        } else {
                            selected.add(id);
                            if (selected.size() > maxSelection) {
                                return null;
                            }
                        }
                    }
                }
            } else {
                for (Path id : mClickedSet) {
                    if (expandSet) {
                        if (!expandMediaSet(selected, mDataManager.getMediaSet(id),
                                maxSelection)) {
                            return null;
                        }
                    } else {
                        selected.add(id);
                        if (selected.size() > maxSelection) {
                            return null;
                        }
                    }
                }
            }
        } else {
            if (mInverseSelection) {
                int total = getTotalCount();
                int index = 0;
                while (index < total) {
                    int count = Math.min(total - index, MediaSet.MEDIAITEM_BATCH_FETCH_COUNT);
                    ArrayList<MediaItem> list = mSourceMediaSet.getMediaItem(index, count);
                    for (MediaItem item : list) {
                        Path id = item.getPath();
                        if (!mClickedSet.contains(id)) {
                            selected.add(id);
                            if (selected.size() > maxSelection) {
                                return null;
                            }
                        }
                    }
                    index += count;
                }
            } else {
                /// M: [BUG.MODIFY] @{
                /*
                for (Path id : mClickedSet) {
                    selected.add(id);
                    if (selected.size() > maxSelection) {
                        return null;
                    }
                }
                */
                // Check if items in click set are still in mSourceMediaSet,
                // if not, we do not add it to selected list.
                ArrayList<Path> selectedPathTemple = new ArrayList<Path>();
                selectedPathTemple.addAll(mClickedSet);
                mDataManager.mapMediaItems(selectedPathTemple, new MediaSet.ItemConsumer() {
                    public void consume(int index, MediaItem item) {
                        if (selected.size() < maxSelection && item != null) {
                            selected.add(item.getPath());
                        }
                    }

                    /// M: [BUG.ADD] @{
                    @Override
                    public boolean stopConsume() {
                        return false;
                    }
                    /// @}
                }, 0);
                /// @}
            }
        }
        return selected;
    }

    public void setSourceMediaSet(MediaSet set) {
        mSourceMediaSet = set;
        mTotal = -1;
    }


    //********************************************************************
    //*                              MTK                                 *
    //********************************************************************

    ArrayList<Path> mPrepared;
    public static final Object LOCK = new Object();

    // Save and restore selection in thread pool to avoid ANR
    private AbstractGalleryActivity mActivity = null;
    private final Handler mMainHandler;
    private ArrayList<Path> mSelectionPath = null;
    private ArrayList<Long> mSelectionGroupId = null;
    private Future<?> mSaveSelectionTask;
    private Future<?> mRestoreSelectionTask;

    public ArrayList<Path> getPrepared() {
        return mPrepared;
    }

    public void setPrepared(ArrayList<Path> prepared) {
        mPrepared = prepared;
    }

    public boolean contains(Path path) {
        if (inSelectAllMode()) {
            return true;
        }
        return mClickedSet.contains(path);
    }

    public void onSourceContentChanged() {
        // reset and reload total count since source set data has changed
        mTotal = -1;
        int count = getTotalCount();
        Log.d(TAG, "<onSourceContentChanged> New total=" + count);
        if (count == 0) {
            leaveSelectionMode();
        }
    }

    // used by ActionModeHandler computeShareIntent
    public ArrayList<Path> getSelected(JobContext jc, boolean expandSet, final int maxSelection) {
        final ArrayList<Path> selected = new ArrayList<Path>();
        if (mIsAlbumSet) {
            if (mInverseSelection) {
                int total = getTotalCount();
                for (int i = 0; i < total; i++) {
                    if (jc.isCancelled()) {
                        Log.d(TAG, "<getSelected> jc.isCancelled() - 1");
                        return null;
                    }
                    MediaSet set = mSourceMediaSet.getSubMediaSet(i);
                    // if set is null, should continue and return directly.
                    if (set == null) {
                        continue;
                    }
                    Path id = set.getPath();
                    if (!mClickedSet.contains(id)) {
                        if (expandSet) {
                            if (!expandMediaSet(jc, selected, set, maxSelection)) {
                                return null;
                            }
                        } else {
                            selected.add(id);
                            if (selected.size() > maxSelection) {
                                return null;
                            }
                        }
                    }
                }
            } else {
                for (Path id : mClickedSet) {
                    if (jc.isCancelled()) {
                        Log.d(TAG, "<getSelected> jc.isCancelled() - 2");
                        return null;
                    }
                    if (expandSet) {
                        if (!expandMediaSet(jc, selected, mDataManager.getMediaSet(id),
                                maxSelection)) {
                            return null;
                        }
                    } else {
                        selected.add(id);
                        if (selected.size() > maxSelection) {
                            return null;
                        }
                    }
                }
            }
        } else {
            if (mInverseSelection) {
                int total = getTotalCount();
                int index = 0;
                while (index < total) {
                    int count = Math.min(total - index, MediaSet.MEDIAITEM_BATCH_FETCH_COUNT);
                    ArrayList<MediaItem> list = mSourceMediaSet.getMediaItem(index, count);
                    for (MediaItem item : list) {
                        if (jc.isCancelled()) {
                            Log.d(TAG, "<getSelected> jc.isCancelled() - 3");
                            return null;
                        }
                        Path id = item.getPath();
                        if (!mClickedSet.contains(id)) {
                             selected.add(id);
                            if (selected.size() > maxSelection) {
                                return null;
                            }
                        }
                    }
                    index += count;
                }
            } else {
                //  we check if items in click set are still in mSourceMediaSet,
                // if not, we do not add it to selected list.
                ArrayList<Path> selectedPathTemple = new ArrayList<Path>();
                selectedPathTemple.addAll(mClickedSet);
                mDataManager.mapMediaItems(selectedPathTemple, new MediaSet.ItemConsumer() {
                    public void consume(int index, MediaItem item) {
                        if (selected.size() < maxSelection) {
                            selected.add(item.getPath());
                        }
                    }

                    @Override
                    public boolean stopConsume() {
                        return false;
                    }
                }, 0);
            }
        }
        return selected;
    }

    private static boolean expandMediaSet(JobContext jc, ArrayList<Path> items, MediaSet set,
            int maxSelection) {
        if (jc.isCancelled()) {
            Log.d(TAG, "<expandMediaSet> jc.isCancelled() - 1");
            return false;
        }
        if (set == null) {
            Log.d(TAG, "<expandMediaSet> set == null, return false");
            return false;
        }
        int subCount = set.getSubMediaSetCount();
        for (int i = 0; i < subCount; i++) {
            if (jc.isCancelled()) {
                Log.d(TAG, "<expandMediaSet> jc.isCancelled() - 2");
                return false;
            }
            if (!expandMediaSet(items, set.getSubMediaSet(i), maxSelection)) {
                return false;
            }
        }
        int total = set.getMediaItemCount();
        int batch = 50;
        int index = 0;

        while (index < total) {
            if (jc.isCancelled()) {
                Log.d(TAG, "<expandMediaSet> jc.isCancelled() - 3");
                return false;
            }
            int count = index + batch < total
                    ? batch
                    : total - index;
            ArrayList<MediaItem> list = set.getMediaItem(index, count);
            if (list == null) {
                Log.d(TAG, "<expandMediaSet> list == null, return false");
                return false;
            }
            if (list.size() > (maxSelection - items.size())) {
                return false;
            }
            for (MediaItem item : list) {
                if (jc.isCancelled()) {
                    Log.d(TAG, "<expandMediaSet> jc.isCancelled() - 4");
                    return false;
                }
                items.add(item.getPath());
            }
            index += batch;
        }
        return true;
    }

    // do save and restore selection in thread pool to avoid ANR @{
    public void saveSelection() {
        if (mSaveSelectionTask != null) {
            mSaveSelectionTask.cancel();
        }
        Log.d(TAG, "<saveSelection> submit task");
        mSaveSelectionTask = mActivity.getThreadPool().submit(new Job<Void>() {
            @Override
            public Void run(final JobContext jc) {
                synchronized (LOCK) {
                    Log.d(TAG, "<saveSelection> task begin");
                    if (jc.isCancelled()) {
                        Log.d(TAG, "<saveSelection> task cancelled");
                        return null;
                    }
                    if (mSelectionPath != null) {
                        mSelectionPath.clear();
                    }
                    if (mSelectionGroupId != null) {
                        mSelectionGroupId.clear();
                    }
                    try {
                        mSelectionPath = getSelected(false);
                        exitInverseSelectionAfterSave();
                    } catch (Exception e) {
                        // this probably means that the actual items are changing
                        // while fetching selected items, so we do not save selection
                        // under this situation
                        /// TODO: find more suitable method to protect this part
                        mSelectionPath = null;
                        mSelectionGroupId = null;
                    }
                    Log.d(TAG, "<saveSelection> task end");
                    return null;
                }
            }
        });
    }

    private void exitInverseSelectionAfterSave() {
        if (mInverseSelection && mSelectionPath != null) {
            mClickedSet.clear();
            int restoreSize = mSelectionPath.size();
            for (int i = 0; i < restoreSize; i++) {
                mClickedSet.add(mSelectionPath.get(i));
            }
            mInverseSelection = false;
        }
    }

    private class RestoreSelectionJobListener implements FutureListener<Void> {
        @Override
        public void onFutureDone(Future<Void> future) {
            mMainHandler.post(new Runnable() {
                @Override
                public void run() {
                    mListener.onSelectionRestoreDone();
                }
            });
        }
    }

    private class RestoreSelectionJob implements Job<Void> {
        @Override
        public Void run(final JobContext jc) {
            synchronized (LOCK) {
                Log.d(TAG, "<restoreSelection> task begin");
                if (jc.isCancelled()) {
                    Log.d(TAG, "<restoreSelection> task cancelledin job run 1");
                    return null;
                }
                if (mSourceMediaSet == null || mSelectionPath == null) {
                    return null;
                }
                mTotal = mIsAlbumSet ? mSourceMediaSet.getSubMediaSetCount() : mSourceMediaSet
                        .getMediaItemCount();
                Path path = null;
                Set<Path> availablePaths = new HashSet<Path>();
                // remove dirty entry
                if (mIsAlbumSet) {
                    MediaSet set = null;
                    for (int i = 0; i < mTotal; ++i) {
                        set = mSourceMediaSet.getSubMediaSet(i);
                        if (jc.isCancelled()) {
                            Log.d(TAG, "<restoreSelection> task cancelled, in job run 2");
                            return null;
                        }
                        if (set != null) {
                            path = set.getPath();
                            if (mSelectionPath.contains(path)) {
                                availablePaths.add(path);
                            }
                        }
                    }
                } else {
                    ArrayList<MediaItem> items = mSourceMediaSet.getMediaItem(0, mTotal);
                    if (items != null && items.size() > 0) {
                        for (MediaItem item : items) {
                            if (jc.isCancelled()) {
                                Log.d(TAG, "<restoreSelection> task cancelledin job run 3");
                                return null;
                            }
                            path = item.getPath();
                            if (mSelectionPath.contains(path)) {
                                availablePaths.add(path);
                            }
                        }
                    }
                }
                // leave select all mode and set clicked set
                mInverseSelection = false;
                mClickedSet.clear();
                mClickedSet = availablePaths;
                // clear saved selection when done
                mSelectionPath.clear();
                mSelectionPath = null;
                if (mSelectionGroupId != null) {
                    mSelectionGroupId.clear();
                    mSelectionGroupId = null;
                }
                Log.d(TAG, "<restoreSelection> task end");
                return null;
            }
        }
    }

    public void restoreSelection() {
        if (mRestoreSelectionTask != null) {
            mRestoreSelectionTask.cancel();
        }
        Log.d(TAG, "<restoreSelection> submit task");
        mRestoreSelectionTask = mActivity.getThreadPool().submit(new RestoreSelectionJob(),
                new RestoreSelectionJobListener());
    }
}
