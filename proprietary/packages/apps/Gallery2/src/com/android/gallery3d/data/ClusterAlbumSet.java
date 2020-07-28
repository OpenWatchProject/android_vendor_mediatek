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

import android.content.Context;
import android.net.Uri;

import com.android.gallery3d.app.GalleryApp;

import java.util.ArrayList;
import java.util.ConcurrentModificationException;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Locale;

public class ClusterAlbumSet extends MediaSet implements ContentListener {
    @SuppressWarnings("unused")
    private static final String TAG = "Gallery2/ClusterAlbumSet";
    private GalleryApp mApplication;
    private MediaSet mBaseSet;
    private int mKind;
    private ArrayList<ClusterAlbum> mAlbums = new ArrayList<ClusterAlbum>();
    private boolean mFirstReloadDone;

    public ClusterAlbumSet(Path path, GalleryApp application, MediaSet baseSet,
            int kind) {
        super(path, INVALID_DATA_VERSION);
        mApplication = application;
        mBaseSet = baseSet;
        mKind = kind;
        baseSet.addContentListener(this);
    }

    @Override
    public MediaSet getSubMediaSet(int index) {
        /// M: [BUG.ADD] @{
        // When index >= mAlbums.size(), exception occours because of
        // multi-thread accessing. Add special check before get MediaSet.
        if (index >= mAlbums.size()) {
            Log.d(TAG, "<getSubMediaSet> index = " + index
                    + ", mAlbums.size() = " + mAlbums.size()
                    + ", return null");
            return null;
        }
        /// @}
        return mAlbums.get(index);
    }

    @Override
    public int getSubMediaSetCount() {
        return mAlbums.size();
    }

    @Override
    public String getName() {
        return mBaseSet.getName();
    }

    /// M: [BUG.MODIFY] @{
    /*
     * @Override
     * public long reload() {
     *     if (mBaseSet.reload() > mDataVersion) {
     *         if (mFirstReloadDone) {
     *             updateClustersContents();
     *         } else {
     *             updateClusters();
     *             mFirstReloadDone = true;
     *         }
     *         mDataVersion = nextVersionNumber();
     *     }
     *         return mDataVersion;
     *     }
     */
    /// @}
    @Override
    public synchronized long reload() {
        //add "synchronized" to avoid multi-thread timing issue
        // if mBaseSet is instance of ComboAlbumSet and is not first
        // reload, should use function synchronizedAlbumData @{
        boolean needSyncAlbum = (mFirstReloadDone)
                && (mBaseSet instanceof LocalAlbumSet || mBaseSet instanceof ComboAlbumSet);
        if (mBaseSet instanceof ClusterAlbum) {
            // mBaseSet is instance of ClusterAlbum,should offsetInStack +1
            mBaseSet.offsetInStack = offsetInStack + 1;
        }
        // if offsetInStack%2 == 1, mean the reload is start by
        // ClusterAlbum, in this case ,should call synchronizedAlbumData
        // if offsetInStack%2 != 1, mean the reload start by ClusterAlbumSet,
        // should call reload of mBaseSet.
        mStopReload = false;
        long mLastDataVersion = ((offsetInStack % 2 == 1) && needSyncAlbum) ? mBaseSet
                .synchronizedAlbumData() : mBaseSet.reload();
        if (mLastDataVersion > mDataVersion) {
            Log.d(TAG, "<reload>total media item count: " + mBaseSet.getTotalMediaItemCount());
            if (mFirstReloadDone) {
                if (!sIsDeleteOperation) {
                    updateClustersContents();
                } else {
                    // add for Cluster delete operation.
                    updateClustersContentsForDeleteOperation();
                }
            } else {
                updateClusters();
                mFirstReloadDone = true;
            }
            if (!mStopReload) {
                mDataVersion = nextVersionNumber();
            }

        } else {
            reloadName();
            Log.d(TAG, "<reload>ClusterAlbumSet: mBaseSet.reload() <= mDataVersion");
        }
        // reassign for next time reload
        mCurrentClusterAlbum = null;
        offsetInStack = 0;
        return mDataVersion;
    }
    /// @}

    /// M: [BUG.ADD] fix bug: "0 image/video toast" pops up
    //  when cluster album is not empty. @{
    @Override
    public boolean isLoading() {
        return mBaseSet.isLoading();
    }
    /// @}

    @Override
    public void onContentDirty() {
        notifyContentChanged();
    }

    private void updateClusters() {
        Log.d(TAG, "<updateClusters>");
        /// M: [BUG.MARK] @{
        /*
         * mAlbums.clear();
         * Clustering clustering;
         */
        /// @}
        Context context = mApplication.getAndroidContext();
        switch (mKind) {
        case ClusterSource.CLUSTER_ALBUMSET_TIME:
            clustering = new TimeClustering(context);
            break;
        case ClusterSource.CLUSTER_ALBUMSET_LOCATION:
            clustering = new LocationClustering(context);
            break;
        case ClusterSource.CLUSTER_ALBUMSET_TAG:
            clustering = new TagClustering(context);
            break;
        case ClusterSource.CLUSTER_ALBUMSET_FACE:
            clustering = new FaceClustering(context);
            break;
        default: /* CLUSTER_ALBUMSET_SIZE */
            clustering = new SizeClustering(context);
            break;
        }

        clustering.run(mBaseSet);
        int n = clustering.getNumberOfClusters();
        /// M: [BUG.ADD] @{
        Log.d(TAG, "<updateClusters>number of clusters: " + n);
        mAlbums.clear();
        /// @}
        DataManager dataManager = mApplication.getDataManager();
        for (int i = 0; i < n; i++) {
            Path childPath;
            String childName = clustering.getClusterName(i);
            if (mKind == ClusterSource.CLUSTER_ALBUMSET_TAG) {
                childPath = mPath.getChild(Uri.encode(childName));
            } else if (mKind == ClusterSource.CLUSTER_ALBUMSET_SIZE) {
                long minSize = ((SizeClustering) clustering).getMinSize(i);
                childPath = mPath.getChild(minSize);
            } else {
                childPath = mPath.getChild(i);
            }

            ClusterAlbum album;
            synchronized (DataManager.LOCK) {
                album = (ClusterAlbum) dataManager.peekMediaObject(childPath);
                if (album == null) {
                    album = new ClusterAlbum(childPath, dataManager, this);
                }
            }
            album.setMediaItems(clustering.getCluster(i));
            /// M: [BUG.ADD] @{
            album.pathSet();
            // do updateClusters, means not in cluster album delete
            // operation, so should set sIsDeleteOperation = false.
            // and set mNumberOfDeletedImage = 0;
            sIsDeleteOperation = false;
            album.setNumberOfDeletedImage(0);
            /// @}
            album.setName(childName);
            album.setCoverMediaItem(clustering.getClusterCover(i));
            mAlbums.add(album);
        }
    }

    private void updateClustersContents() {
        final HashSet<Path> existing = new HashSet<Path>();
        /// M: [BUG.ADD] @{
        // record MediaItem.
        final HashMap<Path, MediaItem> existingMediaItem = new HashMap<Path, MediaItem>();
        /// @}
        mBaseSet.enumerateTotalMediaItems(new MediaSet.ItemConsumer() {
            @Override
            public void consume(int index, MediaItem item) {
                /// M: [BUG.ADD] Avoid Null pointer exception @{
                if (item == null) {
                    Log.d(TAG, "<updateClustersContents> consume, item is null, return");
                    return;
                }
                /// @}
                existing.add(item.getPath());
                /// M: [BUG.ADD] @{
                existingMediaItem.put(item.getPath(), item);
                /// @}
            }

            /// M: [BUG.ADD] Stop ClusterAlbum and ClusterAlbumset reload @{
            @Override
            public boolean stopConsume() {
                return mStopReload;
            }
            /// @}
        });
        /// M: [BUG.ADD] @{
        // note for oldPath
        HashSet<Path> oldPathHashSet = new HashSet<Path>();
        /// @}
        int n = mAlbums.size();

        // The loop goes backwards because we may remove empty albums from
        // mAlbums.
        /// M: [BUG.ADD] @{
        int allDeletedItem = 0;
        /// @}
        for (int i = n - 1; i >= 0; i--) {
            /// M: [BUG.ADD] @{
            int deletedItem = 0;
            /// @}
            ArrayList<Path> oldPaths = mAlbums.get(i).getMediaItems();
            ArrayList<Path> newPaths = new ArrayList<Path>();
            int m = oldPaths.size();
            for (int j = 0; j < m; j++) {
                Path p = oldPaths.get(j);
                /// M: [BUG.ADD] @{
                // add all path to hashSet
                oldPathHashSet.add(p);
                /// @}
                if (existing.contains(p)) {
                    newPaths.add(p);
                    /// M: [BUG.ADD] @{
                } else {
                    deletedItem++;
                }
            }
            allDeletedItem += deletedItem;
            /// @}
            mAlbums.get(i).setMediaItems(newPaths);
            /// M: [BUG.ADD] data change,update version @{
            if (deletedItem > 0) {
                mAlbums.get(i).nextVersion();
            }
            /// @}
            if (newPaths.isEmpty()) {
                mAlbums.remove(i);
            }
        }
        /// M: [BUG.ADD] @{
        // addedPath is used to save added paths
        if ((existing.size() + allDeletedItem > oldPathHashSet.size())) {
            Log.d(TAG, "<updateClustersContents> offsetOFStack==" + offsetInStack
                    + " currentIndexOfSet=" + currentIndexOfSet);
            if (offsetInStack >= 1) {
                ArrayList<Path> addedPath = new ArrayList<Path>();
                // transfer HashSet to ArrayList.
                ArrayList<Path> mNewPath = new ArrayList<Path>(existing);
                int sizeOfExistingPath = mNewPath.size();

                for (int i = 0; i < sizeOfExistingPath; i++) {
                    if (!oldPathHashSet.contains(mNewPath.get(i))) {
                        addedPath.add(mNewPath.get(i));
                    }
                }
                // there some new path, so should updateClusters again;
                setCurrentIndexOfSet();
                try {
                    addedPath = rollBackAlbumInClusters(addedPath, existingMediaItem);
                    updateAlbumInClusters(addedPath, existingMediaItem);
                } catch (ConcurrentModificationException e) {
                    e.printStackTrace();
                }
            } else {
                updateClusters();
                // modify mDataVersion of all album.
                for (int i = mAlbums.size() - 1; i >= 0; i--) {
                    mAlbums.get(i).nextVersion();
                }
            }
        }
        /// @}
    }

    // ********************************************************************
    // *                             MTK                                  *
    // ********************************************************************
    public int currentIndexOfSet;
    private String mCurrentLanguage = Locale.getDefault().getLanguage().toString();
    private String mTimeFormat;
    // while Cluster object delete operation. should set sIsDeleteOperation
    // = true in delete operation Thread.
    private static boolean sIsDeleteOperation = false;
    private Clustering clustering;
    private boolean mStopReload;

    private void reloadName() {
        String language = Locale.getDefault().getLanguage().toString();
        String timeFormat = android.provider.Settings.System.getString(
                mApplication.getContentResolver(),
                android.provider.Settings.System.TIME_12_24);
        if (language != null && !language.equals(mCurrentLanguage)
                || (timeFormat != null && !timeFormat.equals(mTimeFormat))) {
            Log.d(TAG, "<reloadName> Change Language > current language = " + mCurrentLanguage
                    + " New language = " + language
                    + " current timeFormat = " + mTimeFormat
                    + " New timeFormat = " + timeFormat);
            synchronized (clustering) {
                clustering.reGenerateName();
                for (int i = 0; i < mAlbums.size(); i++) {
                    ClusterAlbum album = mAlbums.get(i);
                    album.setName(clustering.getClusterName(i));
                }
                mCurrentLanguage = language;
                mTimeFormat = timeFormat;
                mDataVersion = nextVersionNumber();
            }
        }
    }

    private void updateClustersContentsForDeleteOperation() {
        int n = mAlbums.size();
        for (int i = n - 1; i >= 0; i--) {
            // If the number of deleted image equal with albums size
            // ,means there is not image in this album.
            // So should delete the album.
            if (mAlbums.get(i).getNumberOfDeletedImage() == mAlbums.get(i).getMediaItemCount()) {
                mAlbums.get(i).setNumberOfDeletedImage(0);
                mAlbums.remove(i);
            }
        }
    }

    // synchronous reload to avoid loading item before it's got from db @{
    @Override
    public long reloadForSlideShow() {
        // if mBaseSet is instance of ComboAlbumSet and is not first
        // reload, should use function synchronizedAlbumData @{
        boolean needSyncAlbum = (mFirstReloadDone)
                && (mBaseSet instanceof LocalAlbumSet
                       || mBaseSet instanceof ComboAlbumSet);
        if (mBaseSet instanceof ClusterAlbum) {
            /// M: mBaseSet is instance of ClusterAlbum,should offsetInStack +1
            mBaseSet.offsetInStack = offsetInStack + 1;
        }
        // if offsetInStack%2 == 1, mean the reload is start by
        // ClusterAlbum, in this case ,should call synchronizedAlbumData
        // if offsetInStack%2 != 1, mean the reload start by ClusterAlbumSet,
        // should call reload of mBaseSet.
        long mLastDataVersion = ((offsetInStack % 2 == 1) && needSyncAlbum) ? mBaseSet
                .synchronizedAlbumData() : mBaseSet.reloadForSlideShow();
        if (mLastDataVersion > mDataVersion) {
            Log.d(TAG, "<reloadForSlideShow>total media item count: "
                    + mBaseSet.getTotalMediaItemCount());
            if (mFirstReloadDone) {
                if (!sIsDeleteOperation) {
                    updateClustersContents();
                } else {
                    // add for Cluster delete operation.
                    updateClustersContentsForDeleteOperation();
                }
            } else {
                updateClusters();
                mFirstReloadDone = true;
            }
            mDataVersion = nextVersionNumber();
        } else {
            Log.d(TAG, "<reloadForSlideShow>ClusterAlbumSet: mBaseSet.reload() <= mDataVersion");
        }
        // reassign for next time reload
        mCurrentClusterAlbum = null;
        offsetInStack = 0;

        return mDataVersion;
    }

    public void setCurrentIndexOfSet() {
        int albumSize = mAlbums.size();
        if (mCurrentClusterAlbum != null) {
            boolean hasFindSet = false;
            for (int i = 0; i < albumSize; i++) {
                if (mAlbums.get(i) == mCurrentClusterAlbum) {
                    currentIndexOfSet = i;
                    hasFindSet = true;
                    break;
                }
            }
            if (!hasFindSet) {
                currentIndexOfSet = 0;
                Log.d(TAG, "[setCurrentIndexOfSet]: has not find set");
            }
        }
    }

    // update one Album, instant of doing updateClusters.
    public void updateAlbumInClusters(ArrayList<Path> paths,
            HashMap<Path, MediaItem> exisingMediaItem) {
        if (mAlbums != null) {
            if (currentIndexOfSet < mAlbums.size() && currentIndexOfSet >= 0) {
                try {
                    addNewPathToAlbum(mAlbums.get(currentIndexOfSet), paths, exisingMediaItem);
                } catch (OutOfMemoryError e) {
                    Log.w(TAG, "<updateAlbumInClusters> maybe sizeOldMediaItems is too big:" + e);
                }
            }
            Log.d(TAG, "<updateAlbumInClusters>currentIndexOfSet==" + currentIndexOfSet);
        }
    }

    public ArrayList<Path> rollBackAlbumInClusters(ArrayList<Path> addedPath,
            HashMap<Path, MediaItem> existingMediaItem) {
        if (mAlbums != null) {
            try {
                int numberOfAlbum = mAlbums.size();
                for (int i = 0; i < numberOfAlbum; i++) {
                    ArrayList<Path> albumPath = new ArrayList<Path>();
                    ClusterAlbum clusterAlbum = mAlbums.get(i);
                    if (clusterAlbum != null) {
                        HashSet<Path> oldPath = clusterAlbum.getPathSet();
                        int size = addedPath.size();
                        for (int j = size - 1; j >= 0; j--) {
                            if (oldPath.contains(addedPath.get(j))) {
                                albumPath.add(addedPath.remove(j));
                            }
                        }
                        addNewPathToAlbum(clusterAlbum, albumPath, existingMediaItem);
                    }
                }
            } catch (OutOfMemoryError e) {
                Log.w(TAG, "<rollBackAlbumInClusters> maybe sizeOldMediaItems is too big:" + e);
            }
        }
        return addedPath;
    }

    private void addNewPathToAlbum(ClusterAlbum album, ArrayList<Path> paths,
            HashMap<Path, MediaItem> exisingMediaItem) {
        MediaSet mMediaSet = mApplication.getDataManager().getMediaSet(album.mPath);
        if (null == mMediaSet) {
            return;
        }
        ArrayList<MediaItem> mOldItem = mMediaSet.getMediaItem(0, mMediaSet.getMediaItemCount());
        int sizeOfOldMediaItems = mOldItem.size();
        int sizeofAddPaths = paths.size();
        // get each Path that need to add in old path list.
        for (int j = 0; j < sizeofAddPaths; j++) {
            MediaItem item = exisingMediaItem.get(paths.get(j));
            if (null == item) {
                continue;
            }
            int k = 0;
            for (; k < sizeOfOldMediaItems; k++) {
                if (item.getDateInMs() == mOldItem.get(k).getDateInMs()) {
                    album.addMediaItems(paths.get(j), k);
                    break;
                }
            }
            // if not find the item ,should insert the item in index 0
            if (k == sizeOfOldMediaItems) {
                album.addMediaItems(paths.get(j), 0);
            }
        }
    }

    // synchronizedAlbumData should be started by ClusterAlbum. Using reload
    // should not update Albums in time.
    public long synchronizedAlbumData() {
        if (mBaseSet.synchronizedAlbumData() > mDataVersion) {
            updateClustersContents();
            mDataVersion = nextVersionNumber();
        }
        return mDataVersion;
    }

    // set and get ClusterDeleteOperation.
    public static void setClusterDeleteOperation(boolean deleteOperation) {
        Log.d(TAG, "<setClusterDeleteOperation>setClusterDeleteOperation sIsDeleteOperation: "
                + deleteOperation);
        sIsDeleteOperation = deleteOperation;
    }

    public static boolean getClusterDeleteOperation() {
        Log.d(TAG, "<getClusterDeleteOperation> sIsDeleteOperation: " + sIsDeleteOperation);
        return sIsDeleteOperation;
    }

    // set stop reload flag.
    @Override
    public void stopReload() {
        Log.d(TAG, "<stopReload> ClusterAlbumSet ------------------------");
        mStopReload = true;
        if (null != clustering) {
            clustering.mStopEnumerate = true;
        }
    }

}
