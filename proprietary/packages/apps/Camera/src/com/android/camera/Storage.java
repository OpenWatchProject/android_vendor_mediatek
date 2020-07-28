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

package com.android.camera;

import android.content.Context;
import android.os.Environment;
import android.os.ServiceManager.ServiceNotFoundException;
import android.os.StatFs;
import android.os.storage.StorageManager;
import android.os.storage.StorageVolume;

import com.mediatek.storage.StorageManagerEx;

import java.io.File;
import java.util.Locale;
import java.util.concurrent.atomic.AtomicLong;

public class Storage {
    private static final String TAG = "Storage";

    public static final String DCIM = Environment.getExternalStoragePublicDirectory(
            Environment.DIRECTORY_DCIM).toString();

    private static final String DIRECTORY = DCIM + "/Camera";

    private static final String FOLDER_PATH = "/" + Environment.DIRECTORY_DCIM + "/Camera";

    // Match the code in MediaProvider.computeBucketValues().
    public static final String BUCKET_ID = String.valueOf(DIRECTORY.toLowerCase().hashCode());

    public static final long UNAVAILABLE = -1L;
    public static final long PREPARING = -2L;
    public static final long UNKNOWN_SIZE = -3L;
    public static final long FULL_SDCARD = -4L;
    public static final long LOW_STORAGE_THRESHOLD;
    public static final long RECORD_LOW_STORAGE_THRESHOLD;

    // / M: for more file type and picture type @{
    public static final int CANNOT_STAT_ERROR = -2;
    public static final int PICTURE_TYPE_JPG = 0;
    public static final int PICTURE_TYPE_MPO = 1;
    public static final int PICTURE_TYPE_JPS = 2;
    public static final int PICTURE_TYPE_MPO_3D = 3;
    public static final int PICTURE_TYPE_RAW = 4;

    public static final int FILE_TYPE_PHOTO = 0;
    public static final int FILE_TYPE_VIDEO = 1;
    public static final int FILE_TYPE_PANO = 2;
    public static final int FILE_TYPE_LIV = 3; // live photo
    public static final int FILE_TYPE_PIP_VIDEO = 4;

    public static final int FORMAT_JPEG = 0;
    public static final int FORMAT_AUTORAMA = 1;
    private static final double REF_JPEG_RESOLUTION = 320 * 240;
    private static final int BYTE_PER_REF_JPEG_RESOLUTION = 13312;
    private static final int BYTE_PER_AUTORAMA = 163840;
    private static final int BYER_DEFAULT = 1500000;

    static {
        if (FeatureSwitcher.isMtkFatOnNand() || FeatureSwitcher.isGmoROM()) {
            LOW_STORAGE_THRESHOLD = 10000000;
            RECORD_LOW_STORAGE_THRESHOLD = 9600000;
            Log.d(TAG, "LOW_STORAGE_THRESHOLD= 10000000");
        } else {
            LOW_STORAGE_THRESHOLD = 50000000;
            RECORD_LOW_STORAGE_THRESHOLD = 48000000;
            Log.d(TAG, "LOW_STORAGE_THRESHOLD= 50000000");
        }
    }

    private static StorageManager sStorageManager;
    //TODO add for sanity issue, need check
    private static Context mContext;

    private static boolean mIsExtendStorageCanUse = false;

    private static StorageManager getStorageManager() {
        if (sStorageManager == null) {
            try {
                sStorageManager = new StorageManager(mContext, null);
            } catch (IllegalStateException e) {
                e.printStackTrace();
            } catch (ServiceNotFoundException e) {
                e.printStackTrace();
            }
        }
        return sStorageManager;
    }
    public static void setContext(Context context) {
        mContext = context;
    }

    /**
     * Return the bytes is taken by one image.
     * @param format The format of image.
     * @param solution The solution of image.
     * @return The bytes is taken by one image.
     */
    public static int getBytePerImage(int format, int solution) {
        if (FORMAT_JPEG == format) {
            double scale = (double) solution / REF_JPEG_RESOLUTION;
            return (int) (scale * BYTE_PER_REF_JPEG_RESOLUTION);
        } else if (FORMAT_AUTORAMA == format) {
            return BYTE_PER_AUTORAMA;
        } else {
            return BYER_DEFAULT;
        }
    }

    // get internal volume path
    public static String getInternalVolumePath() {
        StorageManager storageManager = getStorageManager();
        StorageVolume[] volumes = storageManager.getVolumeList();
        for (int i = 0; i < volumes.length; i++) {
            if (!volumes[i].isRemovable()
                    && Environment.MEDIA_MOUNTED.equals(storageManager.getVolumeState(volumes[i]
                            .getPath()))) {
                return volumes[i].getPath();
            }
        }
        return null;
    }

    public static long getAvailableSpace() {
        String state;
        if (mIsExtendStorageCanUse) {
            StorageManager storageManager = getStorageManager();
            state = storageManager.getVolumeState(sMountPoint);
        } else {
            state = Environment.getExternalStorageState();
        }
        // Log.d(TAG, "External storage state=" + state + ", mount point = " +
        // sMountPoint);
        if (Environment.MEDIA_CHECKING.equals(state)) {
            return PREPARING;
        }
        if (!Environment.MEDIA_MOUNTED.equals(state)) {
            return UNAVAILABLE;
        }

        File dir = new File(getFileDirectory());
        dir.mkdirs();
        boolean isDirectory = dir.isDirectory();
        boolean canWrite = dir.canWrite();
        if (!isDirectory || !canWrite) {
            Log.d(TAG, "getAvailableSpace() isDirectory=" + isDirectory + ", canWrite=" + canWrite);
            return FULL_SDCARD;
        }

        try {
            // Here just use one directory to stat fs.
            StatFs stat = new StatFs(getFileDirectory());
            return stat.getAvailableBlocks() * (long) stat.getBlockSize();
        } catch (Exception e) {
            Log.e(TAG, "Fail to access external storage", e);
        }
        return UNKNOWN_SIZE;
    }

    /**
     * OSX requires plugged-in USB storage to have path /DCIM/NNNAAAAA to be
     * imported. This is a temporary fix for bug#1655552.
     */
    public static void ensureOSXCompatible() {
        File nnnAAAAA = new File(DCIM, "100ANDRO"); // should check dcim
        if (!(nnnAAAAA.exists() || nnnAAAAA.mkdirs())) {
            Log.e(TAG, "Failed to create " + nnnAAAAA.getPath());
        }
    }

    private static String sMountPoint;

    public static String getMountPoint() {
        if (mIsExtendStorageCanUse) {
            return sMountPoint;
        }
        return DIRECTORY;
    }

    private static boolean sStorageReady;

    public static boolean isStorageReady() {
        Log.d(TAG, "isStorageReady() mount point = " + sMountPoint + ", return " + sStorageReady);
        return sStorageReady;
    }

    public static void setStorageReady(boolean ready) {
        Log.d(TAG, "setStorageReady(" + ready + ") sStorageReady=" + sStorageReady);
        sStorageReady = ready;
    }

    public static void mkFileDir(String path) {
        File dir = new File(path);
        if (!dir.exists()) {
            Log.d(TAG, "dir not exit,will create this, path = " + path);
            dir.mkdirs();
        }
    }

    public static boolean initializeStorageState() {
        String defaultPath = null;
        StorageManager storageManager = getStorageManager();
        try {
            defaultPath = StorageManagerEx.getDefaultPath();
            mIsExtendStorageCanUse = true;
        } catch (Exception e) {
            e.printStackTrace();
        }
        boolean diff = false;
        Log.i(TAG,"[initializeStorageState] defaultPath = " + defaultPath);
        if (defaultPath != null) {
            String old = sMountPoint;
            sMountPoint = defaultPath;
            if (old != null && old.equalsIgnoreCase(sMountPoint)) {
                diff = true;
            }
            String state = storageManager.getVolumeState(sMountPoint);
            setStorageReady(Environment.MEDIA_MOUNTED.equals(state));
            Log.d(TAG, "initializeStorageState() old=" + old + ", sMountPoint=" + sMountPoint
                    + " return " + diff);
        } else {
            String state = Environment.getExternalStorageState();
            setStorageReady(Environment.MEDIA_MOUNTED.equals(state));
        }
        return diff;
    }

    public static boolean updateDefaultDirectory() {
        mkFileDir(getFileDirectory());
        return initializeStorageState();
    }

    public static boolean updateDirectory(String path) {
        StorageManager storageManager = getStorageManager();
        boolean diff = false;
        String old = sMountPoint;
        sMountPoint = path;
        if (old != null && old.equalsIgnoreCase(sMountPoint)) {
            diff = true;
        }
        mkFileDir(getFileDirectory());
        String state = storageManager.getVolumeState(sMountPoint);
        setStorageReady(Environment.MEDIA_MOUNTED.equals(state));
        Log.d(TAG, "updateDefaultDirectory() old=" + old + ", sMountPoint=" + sMountPoint
                + " return " + diff);
        return diff;
    }

    public static String getFileDirectory() {
        Log.i(TAG,"[getFileDirectory] mIsExtendStorageCanUse = " + mIsExtendStorageCanUse);
        String path;
        if (mIsExtendStorageCanUse) {
            path = sMountPoint + FOLDER_PATH;
        } else {
            path = DIRECTORY;
        }
        return path;
    }

    public static String getCameraScreenNailPath() {

        String path = sMountPoint + FOLDER_PATH;

        final String prefix = "/local/all/";
        String cameraPath = null;
        cameraPath = prefix + getBucketId(getFileDirectory());
        Log.d(TAG, "getCameraScreenNailPath() " + ", return " + cameraPath);
        return cameraPath;
    }

    public static String getBucketId(String directory) {
        return String.valueOf(directory.toLowerCase(Locale.ENGLISH).hashCode());
    }

    public static String getBucketId() {
        return getBucketId(getFileDirectory());
    }

    public static String generateFileName(String title, int pictureType) {
        if (pictureType == PICTURE_TYPE_MPO || pictureType == PICTURE_TYPE_MPO_3D) {
            return title + ".mpo";
        } else if (pictureType == PICTURE_TYPE_JPS) {
            return title + ".jps";
        } else if (pictureType == PICTURE_TYPE_JPG) {
            return title + ".jpg";
        } else if (pictureType == PICTURE_TYPE_RAW) {
            return title + ".dng";
        } else {
            // this case we just want return the
            // source data,such as intermedia data
            return title;
        }
    }

    public static String generateMimetype(String title, int pictureType) {
        if (pictureType == PICTURE_TYPE_MPO || pictureType == PICTURE_TYPE_MPO_3D) {
            return "image/mpo";
        } else if (pictureType == PICTURE_TYPE_JPS) {
            return "image/x-jps";
        } else if (pictureType == PICTURE_TYPE_RAW) {
            return "image/x-adobe-dng";
        } else {
            return "image/jpeg";
        }
    }

    /*
     * public static int generateStereoType(String stereoType) { if
     * (Parameters.STEREO3D_TYPE_SIDEBYSIDE.equals(stereoType)) { return
     * MediaStore.ThreeDimensionColumns.STEREO_TYPE_SIDE_BY_SIDE; } else if
     * (Parameters.STEREO3D_TYPE_TOPBOTTOM.equals(stereoType)) { return
     * MediaStore.ThreeDimensionColumns.STEREO_TYPE_TOP_BOTTOM; } else if
     * (Parameters.STEREO3D_TYPE_FRAMESEQ.equals(stereoType)) { return
     * MediaStore.ThreeDimensionColumns.STEREO_TYPE_FRAME_SEQUENCE; } else {
     * return MediaStore.ThreeDimensionColumns.STEREO_TYPE_2D; } }
     */

    public static String generateFilepath(String fileName) {
        // if filename have the folder name ,so need first create the folder
        int lastIndex = fileName.lastIndexOf("/");
        if (lastIndex != -1) {
            mkFileDir(getFileDirectory() + "/" + fileName.substring(0, lastIndex));
        }
        return getFileDirectory() + '/' + fileName;
    }

    private static final AtomicLong LEFT_SPACE = new AtomicLong(0);

    public static long getLeftSpace() {
        long left = LEFT_SPACE.get();
        Log.d(TAG, "getLeftSpace() return " + left);
        return LEFT_SPACE.get();
    }

    public static void setLeftSpace(long left) {
        LEFT_SPACE.set(left);
        Log.d(TAG, "setLeftSpace(" + left + ")");
    }
}
