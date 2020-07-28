package com.mediatek.camera.tests.v3.operator;

import android.content.Context;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.Operator;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.ReflectUtils;
import com.mediatek.camera.tests.v3.util.Utils;

import java.io.File;
import java.io.IOException;
import java.lang.reflect.Constructor;

public class FillStorageOperator extends Operator {
    public static final int INDEX_CLEAR = 0;
    public static final int INDEX_NEAR_FULL = 1;
    public static final int INDEX_FULL = 2;

    private static final LogUtil.Tag TAG = Utils.getTestTag(FillStorageOperator.class
            .getSimpleName());
    private static final String FILL_DIR = ".fillByCameraTest";
    private static final String FILL_FILE_PREFIX = "fill_";
    private static long NEAR_FULL_RESERVE_BYTE = 60 * 1024 * 1024;
    private static final long FULL_RESERVE_BYTE = 8 * 1024 * 1024;
    private static final long MAX_FILL_FILE_SIZE_IN_BYTE;

    static {
        Long tempLong = new Long(1) * 2 * 1024 * 1024 * 1024;
        MAX_FILL_FILE_SIZE_IN_BYTE = tempLong.longValue();
    }

    @Override
    public int getOperatorCount() {
        return 3;
    }

    @Override
    public Page getPageBeforeOperate(int index) {
        return null;
    }

    @Override
    public Page getPageAfterOperate(int index) {
        return null;
    }

    @Override
    public String getDescription(int index) {
        switch (index) {
            case INDEX_CLEAR:
                return "Clear filling content on default storage";
            case INDEX_NEAR_FULL:
                return "Fill default storage until near full";
            case INDEX_FULL:
                return "Fill default storage until full";
            default:
                return null;
        }
    }

    @Override
    protected void doOperate(int index) {
        switch (index) {
            case INDEX_CLEAR:
                clear();
                break;
            case INDEX_NEAR_FULL:
                while (Utils.getReserveSpaceInByte() <= NEAR_FULL_RESERVE_BYTE) {
                    clearOneFillFile();
                }
                fillReserve(NEAR_FULL_RESERVE_BYTE);
                break;
            case INDEX_FULL:
                while (Utils.getReserveSpaceInByte() <= FULL_RESERVE_BYTE) {
                    clearOneFillFile();
                }
                fillReserve(FULL_RESERVE_BYTE);
            default:
                break;
        }
        LogHelper.d(TAG, "[doOperate] reserve space of " + Utils.getDefaultStoragePath() + " is "
                + Utils.getReserveSpaceInByte() + " byte, about " + Utils.getReserveSpaceInMB() +
                " MB");
    }

    private void clear() {
        Utils.delete(new File(getFillDirPath()));
    }

    private void fillReserve(long targetByte) {
        while (Utils.getReserveSpaceInByte() > targetByte) {
            try {
                File fillFile = new File(generateOneFillFilePath());
                LogHelper.d(TAG, "[fillReserve] fill file " + fillFile.getAbsolutePath());
                String command = "dd if=/dev/zero of=" + fillFile.getAbsolutePath();
                Process process = Runtime.getRuntime().exec(command);
                while (Utils.getReserveSpaceInByte() > targetByte &&
                        fillFile.length() < MAX_FILL_FILE_SIZE_IN_BYTE) {
                    synchronized (this) {
                        try {
                            wait(Utils.TIME_OUT_RIGHT_NOW);
                        } catch (InterruptedException e) {
                        }
                    }
                }
                process.destroy();
            } catch (IOException e) {
                return;
            }
        }
    }

    private String getFillDirPath() {
        String dirPath = Utils.getDefaultStoragePath() + "/" + FILL_DIR;
        File dirFile = new File(dirPath);
        dirFile.mkdir();
        return dirPath;
    }

    private String generateOneFillFilePath() {
        return getFillDirPath() + "/" + FILL_FILE_PREFIX + System.currentTimeMillis();
    }

    private void clearOneFillFile() {
        Utils.delete(new File(Utils.getDefaultStoragePath() + "/DCIM"));

        File fillFolder = new File(getFillDirPath());
        File[] files = fillFolder.listFiles();
        long minSize = Long.MAX_VALUE;
        File minSizeFile = null;
        for (File file : files) {
            if (file.length() < minSize) {
                minSize = file.length();
                minSizeFile = file;
            }
        }
        if (minSizeFile != null) {
            minSizeFile.delete();
        } else if (files != null && files.length > 0 && files[0] != null) {
            files[0].delete();
        }
    }

    static {
        Constructor cons = ReflectUtils.getConstructor(
                "com.mediatek.camera.common.storage.Storage", Context.class);
        Object storage = ReflectUtils.createInstance(cons, Utils.getContext());
        final long CAPTURE_RESERVE_BYTE = (long) ReflectUtils.callMethodOnObject(
                storage, "getCaptureThreshold");
        final long RECORD_RESERVE_BYTE = (long) ReflectUtils.callMethodOnObject(
                storage, "getRecordThreshold");
        NEAR_FULL_RESERVE_BYTE = Math.max(CAPTURE_RESERVE_BYTE, RECORD_RESERVE_BYTE)
                + 10 * 1024 * 1024;
    }
}