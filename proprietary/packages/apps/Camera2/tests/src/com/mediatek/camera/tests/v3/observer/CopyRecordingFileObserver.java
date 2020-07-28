package com.mediatek.camera.tests.v3.observer;

import android.content.Context;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.ReflectUtils;
import com.mediatek.camera.tests.v3.util.Utils;

import java.io.File;
import java.io.FilenameFilter;
import java.lang.reflect.Constructor;

public class CopyRecordingFileObserver extends BackgroundObserver {
    private static final LogUtil.Tag TAG = Utils.getTestTag(
            CopyRecordingFileObserver.class.getSimpleName());

    private static String TARGET_DIR_NAME = ".CameraAutoTestTmp";
    private static int RESERVE_SPACE_IN_MB = 500;

    private Object mStorage;

    @Override
    protected void doObserveInBackground(int index) {
        String dirPath = getSavingFileDirectory();
        String copyDirPath = dirPath + "/" + TARGET_DIR_NAME;
        File dirFile = new File(dirPath);
        File copyDirFile = new File(copyDirPath);
        copyDirFile.mkdir();

        FilenameFilter recordFileFilter = new FilenameFilter() {
            @Override
            public boolean accept(File dir, String name) {
                return name.contains("video") ||
                        name.contains("Video") ||
                        name.contains("VIDEO") ||
                        name.contains("record") ||
                        name.contains("Record") ||
                        name.contains("RECORD") ||
                        name.contains("VID") ||
                        name.contains("vid") ||
                        name.contains(".3gp");
            }
        };

        File[] oldFiles = dirFile.listFiles(recordFileFilter);
        int oldFileCount = oldFiles == null ? 0 : oldFiles.length;

        File newFile = null;

        // wait until find out new file
        while (!isObserveInterrupted()) {
            File[] newFiles = dirFile.listFiles(recordFileFilter);
            int newFileCount = newFiles == null ? 0 : newFiles.length;
            if (newFileCount <= oldFileCount) {
                continue;
            } else {
                newFile = compareFiles(oldFiles, newFiles);
                LogHelper.d(TAG,
                        "[doObserveInBackground] new file is = " + newFile.getAbsolutePath());
                break;
            }
        }

        if (isObserveInterrupted()) {
            return;
        }

        Utils.assertRightNow(newFile != null);

        // copy new file
        while (!isObserveInterrupted()) {
            if (!isStorageEnough()) {
                LogHelper.d(TAG, "[doObserveInBackground] free space not enough, delete " +
                        copyDirFile.getAbsolutePath());
                File[] filesDelete = copyDirFile.listFiles();
                for (File file : filesDelete) {
                    file.delete();
                }
                copyDirFile.delete();
                copyDirFile.mkdir();
            }

            String fromPath = newFile.getAbsolutePath();
            String toPath = copyDirPath + "/" + newFile.getName() + "_"
                    + System.currentTimeMillis();

            LogHelper.d(TAG, "[doObserveInBackground] copy new file to " + toPath);
            Utils.copyFile(fromPath, toPath);
        }
    }

    @Override
    public int getObserveCount() {
        return 1;
    }

    @Override
    public String getDescription(int index) {
        return "Copy current recording file to other location over and over again";
    }

    private String getSavingFileDirectory() {
        return (String) ReflectUtils.callMethodOnObject(getStorage(), "getFileDirectory");
    }

    private File compareFiles(File[] oldFiles, File[] newFiles) {
        for (File newFile : newFiles) {
            boolean findNewFileInOldFiles = false;
            for (File oldFile : oldFiles) {
                if (newFile.getAbsolutePath().equals(oldFile.getAbsolutePath())) {
                    findNewFileInOldFiles = true;
                    break;
                }
            }
            if (!findNewFileInOldFiles) {
                return newFile;
            }
        }
        return null;
    }

    private boolean isStorageEnough() {
        return (long) ReflectUtils.callMethodOnObject(getStorage(),
                "getAvailableSpace") > RESERVE_SPACE_IN_MB * 1024 * 1024;
    }

    private Object getStorage() {
        if (mStorage == null) {
            Constructor constructor = ReflectUtils.getConstructor(
                    "com.mediatek.camera.common.storage.Storage",
                    Context.class);
            mStorage = ReflectUtils.createInstance(constructor, Utils.getContext());
        }
        return mStorage;
    }
}
