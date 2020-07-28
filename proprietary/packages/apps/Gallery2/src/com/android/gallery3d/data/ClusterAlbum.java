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

package com.android.gallery3d.data;

import java.util.ArrayList;
import java.util.ConcurrentModificationException;
import java.util.HashSet;

public class ClusterAlbum extends MediaSet implements ContentListener {
    @SuppressWarnings("unused")
    private static final String TAG = "Gallery2/ClusterAlbum";
    private ArrayList<Path> mPaths = new ArrayList<Path>();
    private String mName = "";
    private DataManager mDataManager;
    private MediaSet mClusterAlbumSet;
    private MediaItem mCover;

    public ClusterAlbum(Path path, DataManager dataManager,
            MediaSet clusterAlbumSet) {
        super(path, nextVersionNumber());
        mDataManager = dataManager;
        mClusterAlbumSet = clusterAlbumSet;
        mClusterAlbumSet.addContentListener(this);
    }

    public void setCoverMediaItem(MediaItem cover) {
        mCover = cover;
    }

    /// M: [BUG.MARK] @{
    /*
     * @Override public MediaItem getCoverMediaItem() { return mCover != null ?
     * mCover : super.getCoverMediaItem(); }
     */
    /// @}

    void setMediaItems(ArrayList<Path> paths) {
        mPaths = paths;
    }

    /// M: [BUG.ADD] @{
    // add paths ordered. @{
    void addMediaItems(Path paths, int index) {
        if (paths != null) {
            mPaths.add(index, paths);
            nextVersion();
        }
    }
    /// @}

    ArrayList<Path> getMediaItems() {
        return mPaths;
    }

    public void setName(String name) {
        mName = name;
    }

    @Override
    public String getName() {
        return mName;
    }

    @Override
    public int getMediaItemCount() {
        return mPaths.size();
    }

    @Override
    public ArrayList<MediaItem> getMediaItem(int start, int count) {
        return getMediaItemFromPath(mPaths, start, count, mDataManager);
    }

    public static ArrayList<MediaItem> getMediaItemFromPath(
            ArrayList<Path> paths, int start, int count, DataManager dataManager) {
        if (start >= paths.size()) {
            return new ArrayList<MediaItem>();
        }
        int end = Math.min(start + count, paths.size());
        ArrayList<Path> subset = new ArrayList<Path>(paths.subList(start, end));
        final MediaItem[] buf = new MediaItem[end - start];
        ItemConsumer consumer = new ItemConsumer() {
            @Override
            public void consume(int index, MediaItem item) {
                buf[index] = item;
            }

            /// M: [BUG.ADD] Stop ClusterAlbum and ClusterAlbumset reload @{
            @Override
            public boolean stopConsume() {
                return false;
            }
            /// @}
        };
        dataManager.mapMediaItems(subset, consumer, 0);
        ArrayList<MediaItem> result = new ArrayList<MediaItem>(end - start);
        for (int i = 0; i < buf.length; i++) {
            /// M: [BUG.MODIFY] @{
            /* result.add(buf[i]); */
            if (buf[i] != null) {
                // we only add available items
                result.add(buf[i]);
            }
            /// @}
        }
        return result;
    }

    @Override
    protected int enumerateMediaItems(ItemConsumer consumer, int startIndex) {
        mDataManager.mapMediaItems(mPaths, consumer, startIndex);
        return mPaths.size();
    }

    @Override
    public int getTotalMediaItemCount() {

        /// M: [BUG.MODIFY] @{
        /* return mPaths.size(); */
        // For Cluster album delete operation, do not fresh clusterContent
        // and use mNumberOfDeletedImage note
        // the deleted images.
        return mPaths.size() - mNumberOfDeletedImage;
        /// @}
    }

    /// M: [BUG.MODIFY] @{
    /*
     * @Override public long reload() { if (mClusterAlbumSet.reload() >
     * mDataVersion) { mDataVersion = nextVersionNumber(); } return
     * mDataVersion; }
     */
    /// @}

    @Override
    public void onContentDirty() {
        notifyContentChanged();
    }

    @Override
    public int getSupportedOperations() {
        return SUPPORT_SHARE | SUPPORT_DELETE | SUPPORT_INFO;
    }

    @Override
    public void delete() {
        ItemConsumer consumer = new ItemConsumer() {
            @Override
            public void consume(int index, MediaItem item) {
                if ((item.getSupportedOperations() & SUPPORT_DELETE) != 0) {
                    item.delete();
                    /// M: [BUG.ADD] @{
                    // note the number of deleted image. for Cluster album
                    // delete operation.
                    mNumberOfDeletedImage++;
                    /// @}
                }
            }

            /// M: [BUG.ADD] Stop ClusterAlbum and ClusterAlbumset reload @{
            @Override
            public boolean stopConsume() {
                return false;
            }
            /// @}
        };
        mDataManager.mapMediaItems(mPaths, consumer, 0);
    }

    @Override
    public boolean isLeafAlbum() {
        return true;
    }

    // ********************************************************************
    // *                             MTK                                   *
    // ********************************************************************

    private MediaItem mCoverBackUp;
    // note the number of deleted image.
    private int mNumberOfDeletedImage;
    private final HashSet<Path> mPathSet = new HashSet<Path>();
    // stop reload in time.
    private boolean mStopReload = false;
    @Override
    public synchronized long reload() {
        Log.d(TAG, "<reload>");
        //  assign this object to ClusterAlbumSet.
        mClusterAlbumSet.mCurrentClusterAlbum = this;
        // note the offset to stack top
        mClusterAlbumSet.offsetInStack = offsetInStack + 1;
        mStopReload = false;
        if (mClusterAlbumSet.reload() > mDataVersion) {
            if (!mStopReload) {
                mDataVersion = nextVersionNumber();
            }
            Log.d(TAG,
                    " <reload>mClusterAlbumSet.synchronizedAlbumData() > mDataVersion");
        }
        offsetInStack = 0;
        return mDataVersion;
    }

    // if the album is been update, should modify mDataVersion.
    public long nextVersion() {
        mDataVersion = nextVersionNumber();
        return mDataVersion;
    }

    @Override
    public MediaItem getCoverMediaItem() {
        if (mCover != null) {
            return mCover;
        }
        try {
            mCoverBackUp = super.getCoverMediaItem();
        } catch (ConcurrentModificationException e) {
            e.printStackTrace();
        }
        return mCoverBackUp;
    }

    // set and get delete number operation
    public void setNumberOfDeletedImage(int number) {
        mNumberOfDeletedImage = number;
    }

    public int getNumberOfDeletedImage() {
        return mNumberOfDeletedImage;
    }

    public void pathSet() {
        if (mPathSet == null) {
            return;
        }
        mPathSet.clear();
        mPathSet.addAll(mPaths);
    }

    public HashSet< Path > getPathSet() {
        return mPathSet;
    }

    @Override
    public void stopReload() {
        Log.d(TAG, "<stopReload> ......");
        mStopReload = true;
        if (null != mClusterAlbumSet) {
            mClusterAlbumSet.stopReload();
        }
    }
}
