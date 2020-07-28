package com.mediatek.dataprotection;

import java.io.FileDescriptor;
import java.util.List;
import java.util.concurrent.atomic.AtomicBoolean;

import android.app.PendingIntent;

public abstract class DataProtectionTask implements Runnable {

    public static final int STATE_ONGOING = 1;
    public static final int STATE_TODO = 2;
    public static final int STATE_COMPLETED = 3;
    public static final int STATE_CANCELLED = 4;
    public static final int STATE_ERROR = 5;
    public static final int STATE_SDCARD_UNMOUNTED = 6;
    public static final int STATE_STORAGE_NOT_ENOUGH = 7;

    public static final int TYPE_ENCRYPT = 1;
    public static final int TYPE_DECRYPT = 2;
    public static final int TYPE_DELETE = 3;

    List<FileInfo> mFiles;
    int state;
    int current;
    private AtomicBoolean mIsCancel = new AtomicBoolean(false);
    protected int progress;
    protected long when = 0;
    protected FileDescriptor currentFd = null;
    protected int failNum = 0;
    protected PendingIntent mPendIntent = null;
    abstract int getTaskType();

    abstract boolean hasNext();

    abstract Object next();

    /*
     * abstract boolean needShowNotification(); abstract int getCurrentState();
     * abstract int getCurrentProcessingFile(); abstract PendingIntent abstract
     * getContentIntent(); abstract boolean cancel();
     */
    boolean needShowNotification() {
        return false;
    }

    int getCurrentState() {
        return state;
    }

    String getCurrentProcessingFile() {
        return null;
    }

    PendingIntent getContentIntent() {
        return null;
    }

    String getNotificationTitle() {
        return null;
    }

    String getNotificationContentInfo() {
        return null;
    }

    String getNotificationContentText() {
        return null;
    }

    boolean cancel() {
        mIsCancel.set(true);
        return true;
    }

    int getProgress() {
        return progress;
    }

    long getCreateTime() {
        return when;
    }

    FileDescriptor getCurrentFd() {
        return currentFd;
    }

    boolean isCancel() {
        return mIsCancel.get();
    }

    int getIcon() {
        return 0;
    }

    void onTaskCancelled() {

    }

    void onSdcardUnmounted(String mountPointPath) {

    }
}

/*
 * class EncryptFileTask extends DataProtectionTask {
 *
 * public EncryptFileTask(List<FileInfo> files) { mFiles = files; state =
 * STATE_TODO; current = 0; }
 *
 * @Override int getTaskType() { return TYPE_ENCRYPT; }
 *
 * boolean hasNext() { if(mFiles != null) { if(current < mFiles.size()) { return
 * true; } else { return false; } } else { return false; } }
 *
 * Object next() { FileInfo file = null; current ++; if(current ==
 * mFiles.size()) { file = mFiles.get(current - 1); } else { file =
 * mFiles.get(current); } return file; }
 *
 * @Override public int compareTo(Object arg0) { return 0; } }
 */

/*
 * class DecryptFileTask extends DataProtectionTask {
 *
 * public DecryptFileTask(List<FileInfo> files) { mFiles = files; state =
 * STATE_TODO; current = -1; }
 *
 * @Override int getTaskType() { return TYPE_DECRYPT; }
 *
 * }
 */
