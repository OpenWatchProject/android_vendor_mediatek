/*
 *   Copyright Statement:
 *
 *     This software/firmware and related documentation ("MediaTek Software") are
 *     protected under relevant copyright laws. The information contained herein is
 *     confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 *     the prior written permission of MediaTek inc. and/or its licensors, any
 *     reproduction, modification, use or disclosure of MediaTek Software, and
 *     information contained herein, in whole or in part, shall be strictly
 *     prohibited.
 *
 *     MediaTek Inc. (C) 2016. All rights reserved.
 *
 *     BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *    THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *     RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 *     ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 *     WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 *     WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 *     NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 *     RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 *     TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 *     RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 *     OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 *     SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 *     RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 *     STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 *     ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 *     RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 *     MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 *     CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     The following software/firmware and/or related documentation ("MediaTek
 *     Software") have been modified by MediaTek Inc. All revisions are subject to
 *     any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.camera.tests.helper;

import android.content.Context;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.Process;
import android.os.RemoteException;
import android.os.SystemProperties;
import android.util.Log;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.lang.reflect.Field;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;

/**
 * Service to catch log.
 */
public class LoggerServiceImpl extends ILoggerService.Stub {
    private static final String TAG = "CamTestsHelper_" + LoggerServiceImpl.class.getSimpleName();

    private static final String MTK_CAMERA_TESTS_DEFAULT_LOG_FILE
            = "/storage/emulated/0/CameraTestsHelper/testslog.txt";
    private static final int WRITE_UNIT_SIZE = 4 * 1024; // 4k
    private static final String COMMAND_CATCH_LOG = "logcat -b main";
    private java.lang.Runtime mRuntime = null;
    private java.lang.Process mLogProcess = null;
    private java.lang.Process mOldLogProcess = null;
    private InputStream mLogcatInputStream = null;
    private InputStream mOldLogcatInputStream = null;
    private BufferedReader mBufferedReader;
    private boolean mStopped = true;

    private ILoggerCallback mLoggerCallback;
    private LoggerCallbackHandler mLoggerCallbackHandler;

    private String mLogPath;
    private FileWriter mFileWriter;
    private BufferedWriter mBufferedWriter;
    private String[] mTagsInArray;
    private String[] mKeysInArray;
    private String mLogTag;
    private String mOldLogTag;

    /**
     * Constructor.
     *
     * @param context The application context.
     */
    public LoggerServiceImpl(Context context) {

    }

    @Override
    public synchronized boolean startLog() {
        Log.d(TAG, "[startLog]...");
        createLogProcess();
        if (mLogProcess == null) {
            return false;
        }
        return true;
    }

    @Override
    public synchronized boolean stopLog() {
        Log.d(TAG, "[stopLog]...");
        destroyLogProcess();
        return true;
    }

    @Override
    public boolean isLogCatching() {
        return !mStopped;
    }

    @Override
    public String getLogPath() {
        return mLogPath;
    }

    @Override
    public long getServiceRunningTime() {
        return 0L;
    }

    @Override
    public void setLogPath(String path) {
        mLogPath = path;
    }

    @Override
    public void setLogMaxSize(int size) {

    }

    @Override
    public synchronized void setLogCallback(ILoggerCallback callback) {
        Log.d(TAG, "[setLogCallback], callback:" + callback);
        if (callback == null) {
            mLoggerCallbackHandler.removeMessages(LoggerCallbackHandler.MSG_CALLBACK_LOG);
            mLoggerCallbackHandler.removeMessages(LoggerCallbackHandler.MSG_LOG_STATUS);
        }
        mLoggerCallback = callback;
    }

    @Override
    public synchronized void setLogTag(String[] tags, String[] keys) {
        String tagsInStr = "";
        for (String tag : tags) {
            tagsInStr = tagsInStr + tag + " ";
        }

        String keysInStr = "";
        for (String key : keys) {
            keysInStr = keysInStr + key + " ";
        }

        mTagsInArray = tags;
        mKeysInArray = keys;

        mOldLogTag = mLogTag;
        mLogTag = tagsInStr;

        Log.d(TAG, "[setLogTag], mLogTag:" + mLogTag + ", mOldLogTag:" + mOldLogTag
                + ", keys:" + keysInStr);
        if (!mStopped && mLogTag != null && !mLogTag.equals(mOldLogTag)) {
            recreateLogProcess();
        }
    }

    @Override
    public void setAdbCommand(String key, String value) {
        Log.d(TAG, "[setAdbCommand], key:" + key + ", value:" + value);
        SystemProperties.set(key, value);
    }

    private void createLogProcess() {
        Log.d(TAG, "[createLogProcess], mLogTag:" + mLogTag);

        if (mLogProcess == null) {
            try {
                mRuntime =  Runtime.getRuntime();
                mLogProcess = mRuntime.exec(COMMAND_CATCH_LOG + " -s " + mLogTag);
                mOldLogProcess = mLogProcess;
                Log.d(TAG, "[createLogProcess], mLogProcess:" + mLogProcess);
            } catch (IOException e) {
                Log.d(TAG, "[createLogProcess], failed");
                e.printStackTrace();
            }

            if (mLogProcess == null) {
                return;
            }
            mLogcatInputStream = mLogProcess.getInputStream();
            mBufferedReader = new BufferedReader(
                    new InputStreamReader(mLogcatInputStream), WRITE_UNIT_SIZE);
            mOldLogcatInputStream = mLogcatInputStream;

            if (mLogPath != null) {
                createFile(mLogPath);
            }

            mStopped = false;
            HandlerThread ht2 = new HandlerThread("CameraTest-LoggerCallbackThread");
            ht2.start();
            mLoggerCallbackHandler = new LoggerCallbackHandler(ht2.getLooper());

            Thread logcatThread = new LogcatThread("CameraTest-LogcatThread");
            logcatThread.start();
        }
    }

    private void destroyLogProcess() {
        Log.d(TAG, "[destroyLogProcess]");
        mStopped = true;
        if (mLogProcess == null) {
            return;
        }

        if (mLoggerCallbackHandler != null) {
            mLoggerCallbackHandler.getLooper().quit();
        }

        killProcess(mLogProcess);

        closeInputStream(mLogcatInputStream);

        if (mBufferedWriter != null) {
            try {
                mBufferedWriter.close();
                mFileWriter.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }

        mLogProcess = null;
    }

    private void recreateLogProcess() {
        Log.d(TAG, "[recreateLogProcess], mLogTag " + mLogTag);
        try {
            mLogProcess = mRuntime.exec(COMMAND_CATCH_LOG + " -s " + mLogTag);
            Log.d(TAG, "[recreateLogProcess], create process, mLogProcess:" + mLogProcess);
        } catch (IOException e) {
            Log.d(TAG, "[recreateLogProcess], create process failed");
            e.printStackTrace();
        }

        if (mLogProcess == null) {
            return;
        }
        mLogcatInputStream = mLogProcess.getInputStream();
        mBufferedReader = new BufferedReader(
                new InputStreamReader(mLogcatInputStream), WRITE_UNIT_SIZE);

        //closeInputStream(mOldLogcatInputStream);
        //mOldLogcatInputStream = mLogcatInputStream;
        killProcess(mOldLogProcess);
        mOldLogProcess = mLogProcess;
    }

    private void closeInputStream(InputStream stream) {
        Log.d(TAG, "[closeInputStream], stream :" + stream);
        if (stream == null) {
            return;
        }

        try {
            stream.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private void killProcess(java.lang.Process process) {
        Log.d(TAG, "[killProcess], process :" + process);
        if (process == null) {
            return;
        }

        int pid = getProcessPid(process);
        if (pid != -1) {
            Process.killProcess(pid);
            Log.d(TAG, "[killProcess], kill success process pid :" + pid);
        } else {
            if (process != null) {
                process.destroy();
                Log.d(TAG, "[[killProcess]], do not get process pid , destroy process");
            }
        }
    }

    private int getProcessPid(java.lang.Process process) {
        int pid = -1;
        try {
            Field pidfiled = process.getClass().getDeclaredField("pid");
            pidfiled.setAccessible(true);
            pid = pidfiled.getInt(process);
            pidfiled.setAccessible(false);
        } catch (NoSuchFieldException e) {
            pid = -1;
        } catch (IllegalAccessException e) {
            pid = -1;
        }
        Log.d(TAG, "[getProcessPid], Process" + process + ", pid = " + pid);
        return pid;
    }

    private void createFolder(String path) {
        File f = new File(path);
        if (!f.exists()) {
            boolean ret = f.mkdirs();
            if (!ret) {
                Log.w(TAG, "[createFolder], mkdir failed for path:" + path);
            }
        }
    }

    private void createFile(String logPath) {
        int index = logPath.lastIndexOf("/");
        String dirPath = logPath.substring(0, index);
        String fileName = logPath.substring(index + 1, logPath.length());
        Log.e(TAG, "[createFile], dirPath:" + dirPath + ", fileName:" + fileName);
        createFolder(dirPath);
        try {
            File file = new File(dirPath + "/" + fileName);
            file.createNewFile();
            mFileWriter = new FileWriter(file, true);
            mBufferedWriter = new BufferedWriter(mFileWriter);
        } catch (IOException e) {
            Log.e(TAG, "[createLogFile], create failed.");
        }
    }

    private static long getTimeMillsFromLog(String logLine) {
        String logDateString = logLine.substring(0, "01-02 23:36:48.561".length());
        logDateString = Calendar.getInstance().get(Calendar.YEAR) + "-" + logDateString;
        SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss.SSS");
        try {
            Date logDate = sdf.parse(logDateString);
            return logDate.getTime();
        } catch (ParseException e) {
            return 0;
        }
    }

    /**
     * Thread to catch log.
     */
    private class LogcatThread extends Thread {
        public LogcatThread(String threadName) {
            super(threadName);
        }

        @Override
        public void run() {
            Log.d(TAG, "[doCatchingLog] begin, mStopped:" + mStopped);
            byte[] fileBuffer = new byte[WRITE_UNIT_SIZE];

            int len = 0;
            String line = null;
            long startTime = System.currentTimeMillis();
            try {
                Message msg1 = mLoggerCallbackHandler.obtainMessage(
                        LoggerCallbackHandler.MSG_LOG_STATUS);
                msg1.obj = true;
                mLoggerCallbackHandler.sendMessage(msg1);
                boolean isTimely = false;
                while (!mStopped) {
                    line = mBufferedReader.readLine();
                    if (line == null) {
                        continue;
                    }

                    if (mBufferedWriter != null) {
                        try {
                            mBufferedWriter.write(line);
                            mBufferedWriter.write("\r\n");
                        } catch (IOException e) {
                            Log.e(TAG, "[onLogReceived], write log failed.");
                        }
                    }

                    if (!isTimely) {
                        if (getTimeMillsFromLog(line) < startTime) {
                            isTimely = false;
                            continue;
                        } else {
                            isTimely = true;
                        }
                    }

                    boolean isContains = true;
                    if (mKeysInArray != null) {
                        isContains = false;
                        for (int i = 0; i < mTagsInArray.length; i++) {
                            if (line.contains(mTagsInArray[i])
                                    && line.contains(mKeysInArray[i])) {
                                isContains = true;
                                break;
                            }
                        }
                    }

                    if (!isContains) {
                        continue;
                    }

                    if (mLoggerCallback != null) {
                        Message msg = mLoggerCallbackHandler.obtainMessage(
                                LoggerCallbackHandler.MSG_CALLBACK_LOG);
                        msg.obj = line;
                        mLoggerCallbackHandler.sendMessage(msg);
                    }
                }
            } catch (Exception e) {
                Log.e(TAG, "[doCatchingLog], exception:" + e.getMessage());
            } finally {
                Log.d(TAG, "[doCatchingLog] end, mStopped:" + mStopped
                        + ", len:" + len + "line:" + line);
                destroyLogProcess();
            }
        }
    }

    /**
     * Callback log status.
     */
    private class LoggerCallbackHandler extends Handler {
        public static final int MSG_CALLBACK_LOG = 0;
        public static final int MSG_LOG_STATUS = 1;

        public LoggerCallbackHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case MSG_CALLBACK_LOG:
                try {
                    mLoggerCallback.onLogReceived((String) msg.obj);
                } catch (RemoteException e) {
                    Log.e(TAG, "callback on log received failed");
                }
                break;

            case MSG_LOG_STATUS:
                boolean started = (boolean) msg.obj;
                Log.d(TAG, "callback log status, log status:" + (started ? "started" : "stopped"));
                try {
                    if (started) {
                        mLoggerCallback.onLogStarted();
                    } else {
                        mLoggerCallback.onLogStopped();
                    }
                } catch (RemoteException e) {
                    Log.e(TAG, "callback on log received failed");
                }
                break;

            default:
                break;
            }
        }
    }
}
