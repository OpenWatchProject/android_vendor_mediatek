package com.mediatek.op18.mms.utils;


import java.io.File;
import java.io.IOException;

import android.content.Context;
import android.content.Intent;
import android.os.Environment;
import android.os.storage.StorageManager;
import android.os.storage.StorageVolume;
import android.util.Log;

import com.mediatek.op18.mms.R;
import com.mediatek.op18.mms.utils.Constants.ModulePath;
import com.mediatek.storage.StorageManagerEx;


public class SDCardUtils {

    private static final String CLASS_TAG = MyLogger.LOG_TAG + "/SDCardUtils";

    public final static int MINIMUM_SIZE = 512;

    public static boolean hasInternalStorage(Context context) {
        StorageManager storageManager = StorageManager.from(context);
        StorageVolume[] volumes = storageManager.getVolumeList();
        if (volumes != null) {
            for (int i = 0; i < volumes.length; ++i) {
                if (!volumes[i].isRemovable()) {
                    return true;
                }
            }
        }
        return false;
    }

    public static String getExternalStoragePath(Context context) {
        String storagePath = null;
        StorageManager storageManager = StorageManager.from(context);
        storagePath = StorageManagerEx.getExternalStoragePath();
        if (storagePath == null || storagePath.isEmpty()) {
            MyLogger.logE(CLASS_TAG, "storagePath is null");
            return null;
        }
        if (!Environment.MEDIA_MOUNTED.equals(storageManager.getVolumeState(storagePath))) {
            MyLogger.logE(CLASS_TAG, "Media not mounted");
            return null;
        }
        return storagePath;
    }

    public static String getSDStatueMessage(Context context) {
        String message = context.getString(R.string.nosdcard_notice);
        /** M: Bug Fix for CR ALPS01271088 @{ */
        String status = null;
        StorageManager storageManager = StorageManager.from(context);
        String storagePath = StorageManagerEx.getExternalStoragePath();
        if (storagePath != null && !storagePath.isEmpty() && storageManager != null) {
            status = storageManager.getVolumeState(storagePath);
        }
        MyLogger.logD(CLASS_TAG, "getSDStatueMessage: status is " + status);
        /** @} */
        if (Environment.MEDIA_SHARED.equals(status) ||
                Environment.MEDIA_UNMOUNTED.equals(status)) {
            message = context.getString(R.string.sdcard_busy_message);
        }
        return message;
    }

    public static String getStoragePath(Context context) {
        String storagePath = getExternalStoragePath(context);
        if (storagePath == null) {
            MyLogger.logD(CLASS_TAG, "getStoragePath: storagePath = " + storagePath);
            return null;
        }
        storagePath = storagePath + File.separator + "backup";
        MyLogger.logD(CLASS_TAG, "getStoragePath: path is " + storagePath);
        File file = new File(storagePath);

        if (file.exists() && file.isDirectory()) {
            return storagePath;
        } else if (file.mkdir()) {
            return storagePath;
        } else {
            MyLogger.logD(CLASS_TAG, "getStoragePath: mkdir fail, return null ");
            return null;
        }
    }

    public static String getPersonalDataBackupPath(Context context) {
        String path = getStoragePath(context);
        if (path != null) {
            return path + File.separator + ModulePath.FOLDER_DATA;
        }

        return path;
    }

    public static String getAppsBackupPath(Context context) {
        String path = getStoragePath(context);
        MyLogger.logD(CLASS_TAG, "getAppsBackupPath path = " + path);
        if (path != null) {
            return path + File.separator + ModulePath.FOLDER_APP;
        }
        return path;
    }

    public static boolean isSdCardAvailable(Context context) {
        return (getStoragePath(context) != null);
    }

    public static long getAvailableSize(String file) {
        android.os.StatFs stat = new android.os.StatFs(file);
        long count = stat.getAvailableBlocks();
        long size = stat.getBlockSize();
        long totalSize = count * size;
        MyLogger.logD(CLASS_TAG, "file remain size = " + totalSize);
        return totalSize;
    }

    public static boolean isSdCardMissingOrFull(Context context) {
        return isSdCardMissing(context)
                || getAvailableSize(getStoragePath(context)) <= SDCardUtils.MINIMUM_SIZE;
    }

    public static boolean isSdCardMissing(Context context) {
        boolean isSDCardMissing = false;
        String path = getStoragePath(context);
        if (path == null) {
            isSDCardMissing = true;
        } else {
            // create file to check for sure
            File temp = new File(path + File.separator + ".temp");
            if (temp.exists()) {
                if (!temp.delete()) {
                    isSDCardMissing = true;
                }
            } else {
                try {
                    if (!temp.createNewFile()) {
                        isSDCardMissing = true;;
                    }
                } catch (IOException e) {
                    e.printStackTrace();
                    MyLogger.logE(CLASS_TAG, "Cannot create temp file");
                    isSDCardMissing = true;
                } finally {
                    temp.delete();
                }
            }
        }
        return isSDCardMissing;
    }
    /*
     * If SD card is removed or full, kill this process
     */
    public static void killProcessIfNecessary(Context context) {
        if (isSdCardMissing(context)) {
            Log.i(CLASS_TAG, "SD card removed, kill process");
            Utils.killMyProcess();
        } else {
            String path = getStoragePath(context);
            if (getAvailableSize(path) <= MINIMUM_SIZE) {
                Log.i(CLASS_TAG, "SD full, kill process");
                Utils.killMyProcess();
            }
        }
        Log.i(CLASS_TAG, "SD card OK, no need to kill process");
    }
}

