package com.debug.loggerui.file;

import com.debug.loggerui.utils.Utils;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Set;

/**
 * @author MTK81255
 *
 */
public class DeviceStorageLogFile implements ILogFile {
    private static final String TAG = Utils.TAG + "/DeviceStorageLogFile";

    private static DeviceStorageLogFile sInstance = new DeviceStorageLogFile();

    public static DeviceStorageLogFile getInstance() {
        return sInstance;
    }

    @Override
    public boolean delete(File file, Set<String> filters) {
        if (file == null || !file.exists()) {
            return true;
        }
        if (filters != null && filters.contains(file.getName())) {
            return true;
        }
        if (file.isDirectory()) {
            File[] files = file.listFiles();
            if (files != null) {
                for (File subFile : files) {
                    delete(subFile, filters);
                }
            }
        }
        try {
            // Do a short sleep to avoid always lock sd card and ANR happened
            Thread.sleep(10);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        return file.delete();
    }

    @Override
    public boolean createNewFile(File file) {
        try {
            if (file.createNewFile()) {
                upgradePermissionForAdbPull(file);
                return true;
            }
        } catch (IOException e) {
            Utils.logw(TAG, "file.createNewFile() error!");
        }
        return false;
    }

    @Override
    public boolean mkdir(File file) {
        if (file.mkdir()) {
            upgradePermissionForAdbPull(file);
            return true;
        }
        return false;
    }

    @Override
    public boolean mkdirs(File file) {
        List<File> newMkdirFileList = new ArrayList<File>();
        newMkdirFileList.add(file);
        File parentFile = file.getParentFile();
        while (parentFile != null && !parentFile.exists()) {
            newMkdirFileList.add(parentFile);
            parentFile = parentFile.getParentFile();
        }
        if (file.mkdirs()) {
            for (File newMkdirFile : newMkdirFileList) {
                upgradePermissionForAdbPull(newMkdirFile);
            }
            return true;
        }
        return false;
    }

    @Override
    public boolean renameTo(File file, File dest) {
        if (file.renameTo(dest)) {
            upgradePermissionForAdbPull(file);
            return true;
        }
        return false;
    }

    @Override
    public FileOutputStream getFileOutputStream(File file) {
        if (!file.exists()) {
            createNewFile(file);
        }
        try {
            return new FileOutputStream(file);
        } catch (FileNotFoundException e) {
            Utils.logw(TAG, "getFileOutputStream() error!");
        }
        return null;
    }

    private boolean upgradePermissionForAdbPull(File file) {
        return file.setReadable(true, false) & file.setExecutable(true, false);
    }
}
