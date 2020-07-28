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

import android.content.ContentResolver;
import android.content.res.Resources;
import android.database.Cursor;
import android.database.sqlite.SQLiteException;
import android.net.Uri;
import android.os.Environment;
import android.provider.MediaStore;
import android.provider.MediaStore.Images;
import android.provider.MediaStore.Images.ImageColumns;
import android.provider.MediaStore.Video;
import android.provider.MediaStore.Video.VideoColumns;

import com.android.gallery3d.R;
import com.android.gallery3d.app.GalleryApp;
import com.android.gallery3d.common.Utils;
import com.android.gallery3d.util.BucketNames;
import com.android.gallery3d.util.GalleryUtils;
import com.android.gallery3d.util.MediaSetUtils;
import com.mediatek.gallerybasic.base.MediaFilterSetting;
import com.mediatek.galleryportable.TraceHelper;

import java.io.File;
import java.util.ArrayList;

// LocalAlbumSet lists all media items in one bucket on local storage.
// The media items need to be all images or all videos, but not both.
public class LocalAlbum extends MediaSet {
    private static final String TAG = "Gallery2/LocalAlbum";
    private static final String[] COUNT_PROJECTION = { "count(*)" };

    private static final int INVALID_COUNT = -1;
    /// M: [FEATURE.MODIFY] @{
    /*private final String mWhereClause;*/
    private String mWhereClause;
    /// @}
    private final String mOrderClause;
    private final Uri mBaseUri;
    private final String[] mProjection;

    private final GalleryApp mApplication;
    private final ContentResolver mResolver;
    private final int mBucketId;
    private final String mName;
    private final boolean mIsImage;
    private final ChangeNotifier mNotifier;
    private final Path mItemPath;
    private int mCachedCount = INVALID_COUNT;

    public LocalAlbum(Path path, GalleryApp application, int bucketId,
            boolean isImage, String name) {
        super(path, nextVersionNumber());
        mApplication = application;
        mResolver = application.getContentResolver();
        mBucketId = bucketId;
        /// M: [BUG.MODIFY] While this is no photo in Camera folder. should set the folder name @{
        /* mName = name;*/
        if (isCameraRoll() && name.equals("")) {
            mName = application.getResources().getString(R.string.folder_camera);
            Log.d(TAG, "<LocalAlbum> mName = " + mName);
        } else {
            mName = name;
        }
        /// @}
        mIsImage = isImage;
        if (isImage) {
            mWhereClause = ImageColumns.BUCKET_ID + " = ?";
            mOrderClause = ImageColumns.DATE_TAKEN + " DESC, "
                    + ImageColumns._ID + " DESC";
            mBaseUri = Images.Media.EXTERNAL_CONTENT_URI;
            /// M: [FEATURE.MODIFY] @{
            /*mProjection = LocalImage.PROJECTION;*/
            mProjection = LocalImage.getProjection();
            /// @}
            mItemPath = LocalImage.ITEM_PATH;
        } else {
            mWhereClause = VideoColumns.BUCKET_ID + " = ?";
            mOrderClause = VideoColumns.DATE_TAKEN + " DESC, "
                    + VideoColumns._ID + " DESC";
            mBaseUri = Video.Media.EXTERNAL_CONTENT_URI;
            /// M: [FEATURE.MODIFY] @{
            /*mProjection = LocalVideo.PROJECTION;*/
            mProjection = LocalVideo.getProjection();
            /// @}
            mItemPath = LocalVideo.ITEM_PATH;
        }
        /// M: [FEATURE.ADD] @{
        exInitializeWhereClause();
        /// @}
        mNotifier = new ChangeNotifier(this, mBaseUri, application);
    }

    public LocalAlbum(Path path, GalleryApp application, int bucketId,
            boolean isImage) {
        this(path, application, bucketId, isImage,
                BucketHelper.getBucketName(
                application.getContentResolver(), bucketId));
    }

    @Override
    public boolean isCameraRoll() {
        return mBucketId == MediaSetUtils.CAMERA_BUCKET_ID;
    }

    @Override
    public Uri getContentUri() {
        if (mIsImage) {
            return MediaStore.Images.Media.EXTERNAL_CONTENT_URI.buildUpon()
                    .appendQueryParameter(LocalSource.KEY_BUCKET_ID,
                            String.valueOf(mBucketId)).build();
        } else {
            return MediaStore.Video.Media.EXTERNAL_CONTENT_URI.buildUpon()
                    .appendQueryParameter(LocalSource.KEY_BUCKET_ID,
                            String.valueOf(mBucketId)).build();
        }
    }

    @Override
    public ArrayList<MediaItem> getMediaItem(int start, int count) {
        DataManager dataManager = mApplication.getDataManager();
        Uri uri = mBaseUri.buildUpon()
                .appendQueryParameter("limit", start + "," + count).build();
        ArrayList<MediaItem> list = new ArrayList<MediaItem>();
        GalleryUtils.assertNotInRenderThread();
        /// M: [DEBUG.ADD] @{
        TraceHelper.beginSection(">>>>LocalAlbum-query");
        /// @}
        Cursor cursor = mResolver.query(
                uri, mProjection, mWhereClause,
                new String[]{String.valueOf(mBucketId)},
                mOrderClause);
        /// M: [DEBUG.ADD] @{
        TraceHelper.endSection();
        /// @}
        if (cursor == null) {
            Log.w(TAG, "query fail: " + uri);
            return list;
        }

        try {
            while (cursor.moveToNext()) {
                int id = cursor.getInt(0);  // _id must be in the first column
                Path childPath = mItemPath.getChild(id);
                /// M: [DEBUG.ADD] @{
                TraceHelper.beginSection(">>>>LocalAlbum-loadOrUpdateItem");
                /// @}
                MediaItem item = loadOrUpdateItem(childPath, cursor,
                        dataManager, mApplication, mIsImage);
                /// M: [DEBUG.ADD] @{
                TraceHelper.endSection();
                /// @}
                list.add(item);
            }
        } finally {
            cursor.close();
        }
        return list;
    }

    private static MediaItem loadOrUpdateItem(Path path, Cursor cursor,
            DataManager dataManager, GalleryApp app, boolean isImage) {
        synchronized (DataManager.LOCK) {
            /// M: [DEBUG.ADD] @{
            TraceHelper.beginSection(">>>>LocalAlbum-loadOrUpdateItem-peekMediaObject");
            /// @}
            LocalMediaItem item = (LocalMediaItem) dataManager.peekMediaObject(path);
            /// M: [DEBUG.ADD] @{
            TraceHelper.endSection();
            /// @}
            if (item == null) {
                if (isImage) {
                    /// M: [DEBUG.ADD] @{
                    TraceHelper.beginSection(">>>>LocalAlbum-loadOrUpdateItem-new LocalImage");
                    /// @}
                    item = new LocalImage(path, app, cursor);
                    /// M: [DEBUG.ADD] @{
                    TraceHelper.endSection();
                    /// @}
                } else {
                    /// M: [DEBUG.ADD] @{
                    TraceHelper.beginSection(">>>>LocalAlbum-loadOrUpdateItem-new LocalVideo");
                    /// @}
                    item = new LocalVideo(path, app, cursor);
                    /// M: [DEBUG.ADD] @{
                    TraceHelper.endSection();
                    // @}
                }
            } else {
                /// M: [DEBUG.ADD] @{
                TraceHelper.beginSection(">>>>LocalAlbum-loadOrUpdateItem-updateContent");
                /// @}
                item.updateContent(cursor);
                /// M: [DEBUG.ADD] @{
                TraceHelper.endSection();
                /// @}
            }
            return item;
        }
    }

    // The pids array are sorted by the (path) id.
    public static MediaItem[] getMediaItemById(
            GalleryApp application, boolean isImage, ArrayList<Integer> ids) {
        // get the lower and upper bound of (path) id
        MediaItem[] result = new MediaItem[ids.size()];
        if (ids.isEmpty()) return result;
        int idLow = ids.get(0);
        int idHigh = ids.get(ids.size() - 1);

        // prepare the query parameters
        Uri baseUri;
        String[] projection;
        Path itemPath;
        if (isImage) {
            baseUri = Images.Media.EXTERNAL_CONTENT_URI;
            /// M: [FEATURE.MODIFY] @{
            /*projection = LocalImage.PROJECTION;*/
            projection = LocalImage.getProjection();
            /// @}
            itemPath = LocalImage.ITEM_PATH;
        } else {
            baseUri = Video.Media.EXTERNAL_CONTENT_URI;
            /// M: [FEATURE.MODIFY] @{
            /*projection = LocalVideo.PROJECTION;*/
            projection = LocalVideo.getProjection();
            /// @}
            itemPath = LocalVideo.ITEM_PATH;
        }

        ContentResolver resolver = application.getContentResolver();
        DataManager dataManager = application.getDataManager();
        /// M: [DEBUG.ADD] @{
        TraceHelper.beginSection(">>>>LocalAlbum-getMediaItemById-query");
        /// @}
        Cursor cursor = resolver.query(baseUri, projection, "_id BETWEEN ? AND ?",
                new String[]{String.valueOf(idLow), String.valueOf(idHigh)},
                "_id");
        /// M: [DEBUG.ADD] @{
        TraceHelper.endSection();
        // @}
        if (cursor == null) {
            Log.w(TAG, "query fail" + baseUri);
            return result;
        }
        try {
            int n = ids.size();
            int i = 0;

            while (i < n && cursor.moveToNext()) {
                int id = cursor.getInt(0);  // _id must be in the first column

                // Match id with the one on the ids list.
                if (ids.get(i) > id) {
                    continue;
                }

                while (ids.get(i) < id) {
                    if (++i >= n) {
                        return result;
                    }
                }

                Path childPath = itemPath.getChild(id);
                MediaItem item = loadOrUpdateItem(childPath, cursor, dataManager,
                        application, isImage);
                result[i] = item;
                ++i;
            }
            return result;
        } finally {
            cursor.close();
        }
    }

    public static Cursor getItemCursor(ContentResolver resolver, Uri uri,
            String[] projection, int id) {
        /// M: [DEBUG.MODIFY] @{
        /*
        return resolver.query(uri, projection, "_id=?",
                 new String[]{String.valueOf(id)}, null);
        */
        TraceHelper.beginSection(">>>>LocalAlbum-getItemCursor-query");
        Cursor cursor = resolver.query(uri, projection, "_id=?",
                new String[]{String.valueOf(id)}, null);
        TraceHelper.endSection();
        return cursor;
        /// @}
    }

    @Override
    public int getMediaItemCount() {
        if (mCachedCount == INVALID_COUNT) {
            /// M: [DEBUG.ADD] @{
            TraceHelper.beginSection(">>>>LocalAlbum-getMediaItemCount-query");
            /// @}
            /// M: [BUG.MODIFY] @{
            // When SdCard eject, query may throw IllegalStateException, catch it here
            /*
            Cursor cursor = mResolver.query(
                    mBaseUri, COUNT_PROJECTION, mWhereClause,
                    new String[]{String.valueOf(mBucketId)}, null);
            */
            Cursor cursor = null;
            try {
                cursor = mResolver.query(
                        mBaseUri, COUNT_PROJECTION,
                        mWhereClause,
                        new String[]{String.valueOf(mBucketId)},
                        null);
            } catch (IllegalStateException e) {
                Log.w(TAG, "<getMediaItemCount> query IllegalStateException:" + e.getMessage());
                return 0;
            } catch (SQLiteException e) {
                Log.w(TAG, "<getMediaItemCount> query SQLiteException:" + e.getMessage());
                return 0;
            }
            /// @}
            /// M: [DEBUG.ADD] @{
            TraceHelper.endSection();
            /// @}
            if (cursor == null) {
                Log.w(TAG, "query fail");
                return 0;
            }
            try {
                Utils.assertTrue(cursor.moveToNext());
                mCachedCount = cursor.getInt(0);
            } finally {
                cursor.close();
            }
        }
        return mCachedCount;
    }

    @Override
    public String getName() {
        return getLocalizedName(mApplication.getResources(), mBucketId, mName);
    }

    @Override
    public long reload() {
        if (mNotifier.isDirty()) {
            mDataVersion = nextVersionNumber();
            mCachedCount = INVALID_COUNT;
            /// M: [FEATURE.ADD] @{
            // After MediaFilterSetting change,
            // we need to reload mWhereClause parameters
            reloadWhereClause();
            /// @}
        }
        return mDataVersion;
    }

    @Override
    public int getSupportedOperations() {
        return SUPPORT_DELETE | SUPPORT_SHARE | SUPPORT_INFO;
    }

    @Override
    public void delete() {
        GalleryUtils.assertNotInRenderThread();
        /// M: [FEATURE.MODIFY] @{
        /*
        mResolver.delete(mBaseUri, mWhereClause,
                new String[]{String.valueOf(mBucketId)});
        */
        // For some feature, the where clause for delete is different from that
        // for query, so we use special where clause when delete, it will be
        // initialized in exInitializeWhereClause()
        mResolver.delete(mBaseUri, mWhereClauseForDelete,
                new String[]{String.valueOf(mBucketId)});
        /// @}
        /// M: [BUG.ADD] @{
        mApplication.getDataManager().broadcastUpdatePicture();
        /// @}
    }

    @Override
    public boolean isLeafAlbum() {
        return true;
    }

    public static String getLocalizedName(Resources res, int bucketId,
            String name) {
        if (bucketId == MediaSetUtils.CAMERA_BUCKET_ID) {
            return res.getString(R.string.folder_camera);
        } else if (bucketId == MediaSetUtils.DOWNLOAD_BUCKET_ID) {
            return res.getString(R.string.folder_download);
        } else if (bucketId == MediaSetUtils.IMPORTED_BUCKET_ID) {
            return res.getString(R.string.folder_imported);
        } else if (bucketId == MediaSetUtils.SNAPSHOT_BUCKET_ID) {
            return res.getString(R.string.folder_screenshot);
        } else if (bucketId == MediaSetUtils.EDITED_ONLINE_PHOTOS_BUCKET_ID) {
            return res.getString(R.string.folder_edited_online_photos);
        } else {
            return name;
        }
    }

    // Relative path is the absolute path minus external storage path
    public static String getRelativePath(int bucketId) {
        String relativePath = "/";
        if (bucketId == MediaSetUtils.CAMERA_BUCKET_ID) {
            relativePath += BucketNames.CAMERA;
        } else if (bucketId == MediaSetUtils.DOWNLOAD_BUCKET_ID) {
            relativePath += BucketNames.DOWNLOAD;
        } else if (bucketId == MediaSetUtils.IMPORTED_BUCKET_ID) {
            relativePath += BucketNames.IMPORTED;
        } else if (bucketId == MediaSetUtils.SNAPSHOT_BUCKET_ID) {
            relativePath += BucketNames.SCREENSHOTS;
        } else if (bucketId == MediaSetUtils.EDITED_ONLINE_PHOTOS_BUCKET_ID) {
            relativePath += BucketNames.EDITED_ONLINE_PHOTOS;
        /// M: [FEATURE.ADD] stereo - copy & paste @{
        } else if (bucketId == MediaSetUtils.STEREO_CLIPPINGS_BUCKET_ID) {
            relativePath += BucketNames.STEREO_CLIPPINGS;
        /// @}
        } else {
            // If the first few cases didn't hit the matching path, do a
            // thorough search in the local directories.
            File extStorage = Environment.getExternalStorageDirectory();
            String path = GalleryUtils.searchDirForPath(extStorage, bucketId);
            if (path == null) {
                Log.w(TAG, "Relative path for bucket id: " + bucketId + " is not found.");
                relativePath = null;
            } else {
                relativePath = path.substring(extStorage.getAbsolutePath().length());
            }
        }
        return relativePath;
    }

    //********************************************************************
    //*                              MTK                                 *
    //********************************************************************

    private String mDefaultWhereClause;
    private String mWhereClauseForDelete;

    /// M: [BUG.ADD] Relative path is the absolute path minus external storage path.@{
    public String getRelativePath() {
        String relativePath = "/";
        if (mBucketId == MediaSetUtils.CAMERA_BUCKET_ID) {
            relativePath += BucketNames.CAMERA;
        } else if (mBucketId == MediaSetUtils.DOWNLOAD_BUCKET_ID) {
            relativePath += BucketNames.DOWNLOAD;
        } else if (mBucketId == MediaSetUtils.IMPORTED_BUCKET_ID) {
            relativePath += BucketNames.IMPORTED;
        } else if (mBucketId == MediaSetUtils.SNAPSHOT_BUCKET_ID) {
            relativePath += BucketNames.SCREENSHOTS;
        } else if (mBucketId == MediaSetUtils.EDITED_ONLINE_PHOTOS_BUCKET_ID) {
            relativePath += BucketNames.EDITED_ONLINE_PHOTOS;
        // stereo - copy & paste {
        } else if (mBucketId == MediaSetUtils.STEREO_CLIPPINGS_BUCKET_ID) {
            relativePath += BucketNames.STEREO_CLIPPINGS;
        // stereo - copy & paste }
        } else {
            // If the first few cases didn't hit the matching path, do a
            // thorough search in the local directories.
            /// M: SearchDirForPath is a recursive procedure,
            /// if there are a large number of folder on storage, it will take a long time,
            /// so we change the way of getting relative path @{
            MediaItem cover = getCoverMediaItem();
            File extStorage = Environment.getExternalStorageDirectory();
            if (cover != null) {
                relativePath = null;
                String storage = extStorage.getAbsolutePath();
                String path = cover.getFilePath();
                Log.d(TAG, "<getRelativePath> Absolute path of this alum cover is " + path);
                if (path != null && storage != null && !storage.equals("")
                        && path.startsWith(storage)) {
                    relativePath = path.substring(storage.length());
                    relativePath = relativePath.substring(0, relativePath
                            .lastIndexOf("/"));
                    Log.d(TAG, "<getRelativePath> 1.RelativePath for bucket id: "
                                    + mBucketId + " is " + relativePath);
                }
                /// @}
            } else {
                String path = GalleryUtils.searchDirForPath(extStorage, mBucketId);
                if (path == null) {
                    Log.w(TAG, "<getRelativePath> 2.Relative path for bucket id: "
                            + mBucketId + " is not found.");
                    relativePath = null;
                } else {
                    relativePath = path.substring(extStorage.getAbsolutePath().length());
                    Log.d(TAG, "<getRelativePath> 3.RelativePath for bucket id: "
                            + mBucketId + " is " + relativePath);
                }
            }
        }
        Log.d(TAG, "<getRelativePath> return " + relativePath);
        return relativePath;
    }
    /// @}

    private void exInitializeWhereClause() {
        mDefaultWhereClause = mWhereClause;
        reloadWhereClause();
    }

    private void reloadWhereClause() {
        if (mIsImage) {
            mWhereClauseForDelete = MediaFilterSetting
                    .getExtDeleteWhereClauseForImage(mDefaultWhereClause,
                            mBucketId);
            mWhereClause = MediaFilterSetting.getExtWhereClauseForImage(
                    mDefaultWhereClause, mBucketId);
        } else {
            mWhereClauseForDelete = MediaFilterSetting
                    .getExtDeleteWhereClauseForVideo(mDefaultWhereClause,
                            mBucketId);
            mWhereClause = MediaFilterSetting.getExtWhereClauseForVideo(
                    mDefaultWhereClause, mBucketId);
        }
    }
}
