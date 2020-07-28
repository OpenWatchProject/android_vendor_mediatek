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

package com.android.gallery3d.util;

import com.android.gallery3d.data.MediaSet;
import com.android.gallery3d.data.Path;

import com.mediatek.gallery3d.adapter.FeatureHelper;

import java.util.Comparator;

public class MediaSetUtils {
    public static final Comparator<MediaSet> NAME_COMPARATOR = new NameComparator();

    /// M: [BUG.MODIFY]
    /*
    public static final int CAMERA_BUCKET_ID = GalleryUtils.getBucketId(
            Environment.getExternalStorageDirectory().toString() + "/"
            + BucketNames.CAMERA);
    public static final int DOWNLOAD_BUCKET_ID = GalleryUtils.getBucketId(
            Environment.getExternalStorageDirectory().toString() + "/"
            + BucketNames.DOWNLOAD);
    public static final int EDITED_ONLINE_PHOTOS_BUCKET_ID = GalleryUtils.getBucketId(
            Environment.getExternalStorageDirectory().toString() + "/"
            + BucketNames.EDITED_ONLINE_PHOTOS);
    public static final int IMPORTED_BUCKET_ID = GalleryUtils.getBucketId(
            Environment.getExternalStorageDirectory().toString() + "/"
            + BucketNames.IMPORTED);
    public static final int SNAPSHOT_BUCKET_ID = GalleryUtils.getBucketId(
            Environment.getExternalStorageDirectory().toString() +
            "/" + BucketNames.SCREENSHOTS);
    */
    public static int CAMERA_BUCKET_ID = GalleryUtils.getBucketId(
            FeatureHelper.getDefaultPath().toString() + "/" + BucketNames.CAMERA);
    public static int DOWNLOAD_BUCKET_ID = GalleryUtils.getBucketId(
            FeatureHelper.getDefaultPath().toString() + "/"
            + BucketNames.DOWNLOAD);
    public static int EDITED_ONLINE_PHOTOS_BUCKET_ID = GalleryUtils.getBucketId(
            FeatureHelper.getDefaultPath().toString() + "/"
            + BucketNames.EDITED_ONLINE_PHOTOS);
    public static int IMPORTED_BUCKET_ID = GalleryUtils.getBucketId(
            FeatureHelper.getDefaultPath().toString() + "/"
            + BucketNames.IMPORTED);
    public static int SNAPSHOT_BUCKET_ID = GalleryUtils.getBucketId(
            FeatureHelper.getDefaultPath().toString() +
            "/" + BucketNames.SCREENSHOTS);
    /// @}

    /// M: [BUG.MODIFY] @{
    /* private static final Path[] CAMERA_PATHS = { */
    private static Path[] sCameraPaths = {
    /// @}
            Path.fromString("/local/all/" + CAMERA_BUCKET_ID),
            Path.fromString("/local/image/" + CAMERA_BUCKET_ID),
            Path.fromString("/local/video/" + CAMERA_BUCKET_ID)};

    public static boolean isCameraSource(Path path) {
        return sCameraPaths[0] == path || sCameraPaths[1] == path
                || sCameraPaths[2] == path;
    }

    // Sort MediaSets by name
    public static class NameComparator implements Comparator<MediaSet> {
        @Override
        public int compare(MediaSet set1, MediaSet set2) {
            int result = set1.getName().compareToIgnoreCase(set2.getName());
            if (result != 0) return result;
            return set1.getPath().toString().compareTo(set2.getPath().toString());
        }
    }

    //********************************************************************
    //*                              MTK                                 *
    //********************************************************************

    private static String[] sCameraPathStrings = {
        sCameraPaths[0].toString(),
        sCameraPaths[1].toString(),
        sCameraPaths[2].toString()
    };

    private static Path[] sTempCameraPaths = {
        sCameraPaths[0], sCameraPaths[1], sCameraPaths[2]};

    /// M: [BUG.ADD] @{
    /**
     * When default storage has been changed, we should refresh bucked id
     * or else the icon showing on the album set slot can not update.
     */
    public static void refreshBucketId() {
        CAMERA_BUCKET_ID = GalleryUtils.getBucketId(
                FeatureHelper.getDefaultPath().toString() + "/DCIM/Camera");
        DOWNLOAD_BUCKET_ID = GalleryUtils.getBucketId(
                FeatureHelper.getDefaultPath().toString() + "/"
                + BucketNames.DOWNLOAD);
        EDITED_ONLINE_PHOTOS_BUCKET_ID = GalleryUtils.getBucketId(
                FeatureHelper.getDefaultPath().toString() + "/"
                + BucketNames.EDITED_ONLINE_PHOTOS);
        IMPORTED_BUCKET_ID = GalleryUtils.getBucketId(
                FeatureHelper.getDefaultPath().toString() + "/"
                + BucketNames.IMPORTED);
        SNAPSHOT_BUCKET_ID = GalleryUtils.getBucketId(
                FeatureHelper.getDefaultPath().toString() +
                "/Pictures/Screenshots");
        // stereo - copy & paste
        STEREO_CLIPPINGS_BUCKET_ID = GalleryUtils.getBucketId(
                FeatureHelper.getDefaultPath().toString() + "/"
                + BucketNames.STEREO_CLIPPINGS);
        sCameraPaths[0] = Path.fromString("/local/all/" + CAMERA_BUCKET_ID);
        sCameraPaths[1] = Path.fromString("/local/image/" + CAMERA_BUCKET_ID);
        sCameraPaths[2] = Path.fromString("/local/video/" + CAMERA_BUCKET_ID);
        sCameraPathStrings[0] = sCameraPaths[0].toString();
        sCameraPathStrings[1] = sCameraPaths[1].toString();
        sCameraPathStrings[2] = sCameraPaths[2].toString();
        sTempCameraPaths[0] = sCameraPaths[0];
        sTempCameraPaths[1] = sCameraPaths[1];
        sTempCameraPaths[2] = sCameraPaths[2];
    }
    /// @}
    /// M: stereo - copy & paste @{
    public static int STEREO_CLIPPINGS_BUCKET_ID = GalleryUtils.getBucketId(
            FeatureHelper.getDefaultPath().toString() + "/"
            + BucketNames.STEREO_CLIPPINGS);
    /// @}
}
