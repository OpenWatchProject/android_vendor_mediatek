/**
 *
 */
package com.mediatek.mtklogger.c2klogger;

import android.content.SharedPreferences;
import android.util.Log;

import java.io.File;

/**
 * @author MTK81255
 *
 */
public class C2KLogRecycle implements Runnable {

    private static C2KLogRecycle sInstance = new C2KLogRecycle();

    private C2KLogRecycleConfig mC2KLogRecycleConfig = null;

    private String mC2KLogPath = "";

    private long mC2KLogMaxSize = 0;

    private boolean mIsStop = false;

    private boolean mIsRecycleDoing = false;

    private SharedPreferences mSharedPreferences;

    private Thread mThread;

    public static C2KLogRecycle getInstance() {
        return sInstance;
    }

    private C2KLogRecycle() {
    }

    /**
     * @param c2KLogPath String
     * @param c2KLogMaxSize long
     * @param sharedPreferences SharedPreferences
     */
    public void startRecycle(String c2KLogPath, long c2KLogMaxSize,
            SharedPreferences sharedPreferences) {
        Log.d("saber", "-->startRecycle()");
        mIsStop = false;
        this.mC2KLogPath = c2KLogPath;
        this.mC2KLogMaxSize = c2KLogMaxSize;
        this.mSharedPreferences = sharedPreferences;
        mC2KLogRecycleConfig = new C2KLogRecycleConfig(c2KLogPath + "/"
                + C2KLogUtils.C2KLOG_RECYCLE_CONFIG_FILE);
        if (!mIsRecycleDoing) {
            mIsRecycleDoing = true;
            mThread = new Thread(sInstance);
            mThread.start();
        }
    }

    /**
     */
    public void stopRecycle() {
        mIsStop = true;
        if (mThread != null) {
            mThread.interrupt();
            mThread = null;
        }
        mC2KLogRecycleConfig.writeContents();
    }

    /**
     * @param logPath String
     */
    public void addLogPathToRecycleConfig(String logPath) {
        mC2KLogRecycleConfig.addLogpathToLastLine(logPath);
    }

    /*
     * (non-Javadoc)
     * @see java.lang.Runnable#run()
     */
    @Override
    public void run() {
        Log.d("saber", "startRecycle->run() mIsRecycleDoing ? " + mIsRecycleDoing);
        while (!mIsStop) {
            while (isNeedRecycle()) {
                deleteC2KLogFile(mC2KLogRecycleConfig.getLogpathFromFirstLine());
                mC2KLogRecycleConfig.removeLogpathFromFirstLine();
                try {
                    Thread.sleep(500);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
            try {
                Thread.sleep(30000);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
        mIsRecycleDoing = false;
    }

    private boolean isNeedRecycle() {
        String currentSavingLogPath = mSharedPreferences.getString(
                C2KLogUtils.KEY_C2K_MODEM_LOGGING_PATH, "");
        try {
            deleteEmptyC2KLogFolder(new File(currentSavingLogPath).getAbsolutePath());
        } catch (NullPointerException e) {
            e.printStackTrace();
        }

        long c2kLogSize = getFileSize(mC2KLogPath);
        return c2kLogSize >= (mC2KLogMaxSize * 1024 * 1024);
    }

    private static long getFileSize(String filePath) {
        long size = 0;
        if (filePath == null) {
            return 0;
        }
        File fileRoot = new File(filePath);
        if (fileRoot == null || !fileRoot.exists()) {
            return 0;
        }
        if (!fileRoot.isDirectory()) {
            size = fileRoot.length();
        } else {
            File[] files = fileRoot.listFiles();
            // why get a null here ?? maybe caused by permission denied
            if (files == null || files.length == 0) {
                Log.v("saber", "Loop folder [" + filePath + "] get a null/empty list");
                return 0;
            }
            for (File file : files) {
                if (file == null) {
                    continue;
                }
                size += getFileSize(file.getAbsolutePath());
            }
        }
        return size;
    }

    private void deleteEmptyC2KLogFolder(String filterFolder) {
        File[] files = new File(mC2KLogPath).listFiles();
        if (files == null) {
            return;
        }
        for (File file : files) {
            if (file.isDirectory() && !file.getAbsolutePath().equalsIgnoreCase(filterFolder)) {
                File[] subFiles = file.listFiles();
                if (subFiles == null || subFiles.length == 0) {
                    Log.w("saber", "C2KLog folder is empty, delete it :" + file.getAbsolutePath());
                    file.delete();
                    try {
                        Thread.sleep(10);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
            }
        }
    }

    private void deleteC2KLogFile(String logFile) {
        File file = new File(logFile);
        if (file.exists()) {
            Log.w("saber", "File is auto recycled :" + file.getPath());
            file.delete();
        }
    }

}
