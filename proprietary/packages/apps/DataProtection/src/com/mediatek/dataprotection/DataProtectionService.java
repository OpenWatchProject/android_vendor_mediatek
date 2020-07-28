package com.mediatek.dataprotection;

import android.app.Notification;
import android.app.PendingIntent;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.drm.DrmInfoEvent;
import android.drm.DrmManagerClient;
import android.net.Uri;
import android.os.Binder;
import android.os.Bundle;
import android.os.CountDownTimer;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.ParcelFileDescriptor;
import android.os.PowerManager;
import android.os.SystemClock;
import android.os.PowerManager.WakeLock;
import android.preference.PreferenceManager;
import android.provider.MediaStore;
import android.text.TextUtils;
import android.widget.RemoteViews;

import com.mediatek.dataprotection.UnlockHistoryActivity.OnUnlockHistoryUpdate;
import com.mediatek.dataprotection.utils.CtaDrmUtils;
import com.mediatek.dataprotection.utils.DataBaseHelper;
import com.mediatek.dataprotection.utils.FileUtils;
import com.mediatek.dataprotection.utils.PermissionUtils;
import com.mediatek.dataprotection.utils.UiUtils;

import java.io.File;
import java.io.FileDescriptor;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Queue;
import java.util.Set;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicInteger;

import com.mediatek.dataprotection.utils.Log;

public class DataProtectionService extends Service {

    public static String ACTION_MAIN = "com.mediatek.dataprotection.ACTION_START_MAIN";
    public static String ACTION_START_SELECT_ACTIVITY = "com.mediatek.dataprotection.ACTION_START_ADD";
    public static String ACTION_START_UNLOCKHISTORY = "com.mediatek.dataprotection.ACTION_START_ADD";
    public static String ACTION_START_MAIN_PATTERN_REQUEST = "com.mediatek.dataprotection.ACTION_START_MAIN_PATTERN_INPUT";
    public static String KEY_TASK_ID = "TASK_ID";
    public static final String KEY_INVOKE = "INVOKE";
    public static final int TIMES_USER_ATTEMPT_LIMIT = 5;
    public static final int SEC_FORBIDDEN_TRY_TIME = 30;

    private static final String HANDLER_THREAD_NAME = "dataprotectionservice";
    private static final Uri FILE_URI = MediaStore.Files
            .getContentUri("external");
    private static final int START_NOTIFICATION_ID = 100;
    private static final int ENCRYPT_MESSAGE = 1;
    private static final int DECRYPT_MESSAGE = 2;
    private static final int DELETE_FILE = 3;
    private static final int CREAT_CLIENT = 4;
    private static final int CHANGE_PASSWORD = 5;
    private static final int TRY_TO_DECRYPT = 6;
    private static final int MSG_LOADING_FILE = 7;
    private static final int MSG_DO_NEXT = 8;
    private static final int MSG_START_NEXT_TASK = 9;
    private static final int MSG_SDCARD_UNMOUNTED = 10;
    private static final int MSG_SDCARD_MOUNTED = 11;

    private static final String TAG = "DataProtectionService";
    private static final String FILE_SCHEME = "file://";

    private static final long SPACE_LIMIT_REMAIN = 4 * 1024 * 1024;
    private DataProtectionBinder mBinder = new DataProtectionBinder();
    private HandlerThread mHandlerThread = null;
    private Handler mHandler = new Handler();
    private Runnable mNotificationProcessor = null;

    private ArrayList<String> mNavigationList = new ArrayList<String>();
    private RemoteViews mNotificationView;
    private DataProtectNotifier mNotifier;
    private int mProgress = 0;
    private DataProtectionServiceHandler mServiceHandler = null;
    private AtomicBoolean mIsCancelled = new AtomicBoolean();

    private List<FileInfo> mToDecryptFiles = null;

    private HashMap<String, FileInfo> mEncryptingFiles = new HashMap<String, FileInfo>();
    private HashMap<String, FileInfo> mDecryptingFiles = new HashMap<String, FileInfo>();
    private HashMap<String, FileInfo> mDeletingFiles = new HashMap<String, FileInfo>();

    private boolean mIsDecrptingFilesUpdate = false;

    private List<String> mUnlockHistoryFiles = new ArrayList<String>();

    private Queue<DataProtectionTask> mTasks = new LinkedBlockingQueue<DataProtectionTask>();

    private boolean mProcessorStarted = false; // the looper is ongoing...
    public static String mCurrentStartActivity = ACTION_MAIN;
    private ArrayList<String> mDecryptingFailFiles = new ArrayList<String>();
    private Set<String> mBadKeyFiles = new HashSet<String>();

    private DrmManagerClient mClient = null;
    private BroadcastReceiver mStorageBroastReceiver = null;
    private MountListener mMountPointListener = null;
    private DecryptFailListener mDecryptFailListener = null;

    private int mServiceStartId = -1;
    private static final int IDLE_DELAY = 10000;
    private boolean mServiceInUse = false;
    // private AtomicBoolean mInChangePattern = new AtomicBoolean(false);
    private AtomicInteger mChangePatternTimes = new AtomicInteger(0);
    private Set<String> mTempFiles = new HashSet<String>();
    private PowerManager mPowerManager = null;
    private WakeLock mTaskWakeLock = null;
    private Notification mNotification = null;
    private int mNotificationId = 0xffffffff;

    @Override
    public void onCreate() {
        Log.d(TAG, "onCreate");
        mHandlerThread = new HandlerThread(HANDLER_THREAD_NAME);
        mHandlerThread.start();
        mServiceHandler = new DataProtectionServiceHandler(
                mHandlerThread.getLooper());
        Message msg = mServiceHandler.obtainMessage(CREAT_CLIENT);
        mServiceHandler.sendMessage(msg);
        mNotifier = new DataProtectNotifier(this);
        mNotifier.cancelAll();
        registerBroadcastReceivcer();
        loadUnlockHistory();
        loadDecryptFailHistory();
        loadTempFileFromPreference();
        mPowerManager = (PowerManager) getSystemService(POWER_SERVICE);
        // If the service was idle, but got killed before it stopped itself, the
        // system will relaunch it. Make sure it gets stopped again in that
        // case.
        mDelayedStopHandler.sendEmptyMessageDelayed(0, IDLE_DELAY);
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "onDestroy");
        for (DataProtectionTask task : mTasks) {
            cancelTask(task.getCreateTime());
        }

        if (mClient != null) {
            mClient.release();
        }
        mDelayedStopHandler.removeCallbacksAndMessages(null);
        saveUnlockHistory();
        saveDecryptFailHistory();
        saveTempFileToPreference();
        unregisterBroadcastReceivcer();
        if (null != mServiceHandler) {
            mServiceHandler.getLooper().quit();
        }
        super.onDestroy();
    }

    public class DataProtectionServiceHandler extends Handler {
        public DataProtectionServiceHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case ENCRYPT_MESSAGE:
                Log.d(TAG, " hadleMessage: encrypt ");
                if (msg.obj instanceof List<?>) {
                    List<FileInfo> decryptFiles = (List<FileInfo>) msg.obj;
                    EncryptFileTask encryptTask = new EncryptFileTask(
                            decryptFiles);
                    mTasks.add(encryptTask);
                    mNotifier.updateTaskQueue(encryptTask);
                    startNextTask();
                }
                break;
            case DECRYPT_MESSAGE:
                Log.d(TAG, " hadleMessage: decrypt ");
                if (msg.obj instanceof List<?>) {
                    List<FileInfo> decryptFiles = (List<FileInfo>) msg.obj;
                    DataProtectionTask decrypt = new DecryptFileTask(
                            decryptFiles);
                    mTasks.add(decrypt);
                    mNotifier.updateTaskQueue(decrypt);
                    startNextTask();
                }
                break;
            case DELETE_FILE:
                Log.d(TAG, " hadleMessage: delete ");
                if (msg.obj instanceof List<?>) {
                    DeleteLockedFilesTask deleteTask = new DeleteLockedFilesTask(
                            (List<FileInfo>) msg.obj);
                    mTasks.add(deleteTask);
                    mNotifier.updateTaskQueue(deleteTask);
                    startNextTask();
                }
                break;
            case CREAT_CLIENT:
                Log.d(TAG, "create client...");
                mClient = new DrmManagerClient(getApplicationContext());
                break;
            case CHANGE_PASSWORD:
                Log.d(TAG, " hadleMessage: changepattern ");
                String[] patterns = (String[]) msg.obj;
                if (patterns != null) {
                    ChangePatternTask changePatternTask = new ChangePatternTask(
                            patterns[0], patterns[1]);
                    mTasks.add(changePatternTask);
                    mNotifier.updateTaskQueue(changePatternTask);
                    startNextTask();
                }
                break;
            case TRY_TO_DECRYPT:
                Log.d(TAG, " hadleMessage: try to decrypt ");
                if (msg.obj instanceof DecryptFileTask) {
                    DataProtectionTask task = (DataProtectionTask) msg.obj;

                    ArrayList<FileInfo> fileInfos = new ArrayList<FileInfo>(
                            task.mFiles.size());
                    for (int i = 0; i < task.mFiles.size(); i++) {
                        fileInfos.add(new FileInfo(new File(task.mFiles.get(i)
                                .getFileAbsolutePath())));
                    }
                    task.mFiles = fileInfos;
                    mNotifier.updateTaskQueue(task);
                    mTasks.add(task);
                    startNextTask();
                }
                break;
            case MSG_DO_NEXT:
                Log.d(TAG, "service handler do next file");
                startNextFileInCurrentTask();
                break;
            case MSG_SDCARD_UNMOUNTED:
                String path = (String) msg.obj;
                Log.s(TAG, "onSdcard unmounted" + path + ", task empty " + mTasks.isEmpty());

                if (!mTasks.isEmpty()) {
                    for (DataProtectionTask unTask : mTasks) {
                        unTask.onSdcardUnmounted(path);
                    }
                }
                break;
            case MSG_SDCARD_MOUNTED:
                String sdmountPath = (String) msg.obj;
                Log.s(TAG, "onSdcard mounted: " + sdmountPath
                        + " temp file num: " + mTempFiles.size());
                deleteTempFile(sdmountPath);
                break;
            case MSG_START_NEXT_TASK:
                startNextTask();
                break;
            default:
                break;
            }
        }

        private void deleteTempFile(String sdmountPath) {
            for (String tmpFilePath : mTempFiles) {
                if (TextUtils.isEmpty(tmpFilePath)) {
                    continue;
                }
                File file = new File(tmpFilePath);
                if (file.exists()) {
                    file.delete();
                    Log.s(TAG, "delete temp file: " + tmpFilePath);
                }
            }
        }
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        super.onStartCommand(intent, flags, startId);
        Log.d(TAG, "onStartCommand with flags: " + flags + " startId: "
                + startId);

        Bundle data = (intent != null ? intent.getExtras() : null);
        boolean userIn = DataProtectionApplication
                .getActivityState(getApplicationContext());
        Log.d(TAG, "onStartCommand " + userIn);
        if (!userIn && data != null) {
            String action = data.getString("ACTION", null);
            if (action != null && action.equals("cancel")) {
                DataProtectionApplication.setNeedShowCancel(this, true);
            } else if (action != null && action.equals("view_decrypt_fail_files")) {
                DataProtectionApplication.setNeedShowDecryptFail(this, true);
            }
            Intent toIntent = new Intent(getApplicationContext(),
                    ViewLockedFilesActivity.class);
            toIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK
                    | Intent.FLAG_ACTIVITY_CLEAR_TASK);
            toIntent.putExtras(data);
            try {
                getApplicationContext().startActivity(toIntent);
            } catch (Exception e) {

            }
        } else if (data != null && userIn) {
            if (mNotificationListener != null) {
                String action = data.getString("ACTION");
                if (action != null && action.equals("cancel")) {
                    Log.d(TAG, "taskId " + data.getLong(KEY_TASK_ID));
                    mNotificationListener.onCancel(data.getLong(KEY_TASK_ID),
                            data.getInt("TITLE"));
                } else if (action != null
                        && action.equals("view_decrypt_fail_files")) {
                    mNotificationListener.onViewDecryptFailHistory();
                }
            } else {
                Log.e(TAG, "userIn " + userIn
                        + " but no notification listener "
                        + mNotificationListener);
            }
        }

        mServiceStartId = startId;
        mDelayedStopHandler.removeCallbacksAndMessages(null);
        mDelayedStopHandler.sendEmptyMessageDelayed(0, IDLE_DELAY);
        return START_NOT_STICKY;
    }

    @Override
    public IBinder onBind(Intent intent) {
        mServiceInUse = true;
        mDelayedStopHandler.removeCallbacksAndMessages(null);
        return mBinder;
    }

    @Override
    public void onRebind(Intent intent) {
        mServiceInUse = true;
        mDelayedStopHandler.removeCallbacksAndMessages(null);
        super.onRebind(intent);
    }

    /**
     * When all task finish or unbind service, we need send delay message to
     * this handle to check whether need stop service, only when server is idle
     * it can be stopped.
     */
    private Handler mDelayedStopHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            if (!isServiceIdle() || mServiceInUse) {
                return;
            }
            stopSelf(mServiceStartId);
            Log.d(TAG, "Stop service with start id " + mServiceStartId);
        }
    };

    /**
     * Check service whether is busy, only when all task finish and finish
     * handle all fail decrypt files, the service can be stopped.
     *
     * @return true means service idle, it can be stopped.
     */
    private boolean isServiceIdle() {
        Log.d(TAG, "isServiceIdle " + mTasks.isEmpty());
        return mTasks.isEmpty()/* && mDecryptingFailFiles.isEmpty() */;
    }

    @Override
    public boolean onUnbind(Intent intent) {
        Log.d(TAG, "onUnbind with " + intent);
        mServiceInUse = false;
        mDelayedStopHandler.removeCallbacksAndMessages(null);
        mDelayedStopHandler.sendEmptyMessageDelayed(0, IDLE_DELAY);
        return true;
    }

    public class DataProtectionBinder extends Binder {
        DataProtectionService getService() {
            return DataProtectionService.this;
        }
    }

    public boolean listFiles(String path, FileOperationEventListener callback) {
        return false;
    }

    public boolean encrypt(List<FileInfo> files) {
        if (null == files || files.size() == 0) {
            return false;
        }
        for (int i = 0; i < files.size(); i++) {
            mEncryptingFiles.put(files.get(i).getFileAbsolutePath(),
                    files.get(i));
        }
        Message msg = mServiceHandler.obtainMessage(ENCRYPT_MESSAGE);
        msg.obj = files;
        mServiceHandler.sendMessage(msg);
        return true;
    }

    public boolean decrypt(List<FileInfo> files, byte[] key) {
        mIsDecrptingFilesUpdate = true;
        mToDecryptFiles = files;
        for (int i = 0; i < files.size(); i++) {
            mDecryptingFiles.put(files.get(i).getFileAbsolutePath(),
                    files.get(i));
        }
        Message msg = mServiceHandler.obtainMessage(DECRYPT_MESSAGE);
        msg.obj = files;
        mServiceHandler.sendMessage(msg);
        return true;
    }

    public boolean isDecrptingFilesUpdate() {
        return mIsDecrptingFilesUpdate;
    }

    public Set<String> getEncryptingFiles() {
        synchronized (mEncryptingFiles) {
            return mEncryptingFiles.keySet();
        }
    }

    public Set<String> getDecryptingFiles() {
        mIsDecrptingFilesUpdate = false;
        Set<String> filePaths = new HashSet<String>();
        synchronized (mDecryptingFiles) {
            filePaths.addAll(mDecryptingFiles.keySet());
        }
        synchronized (mDeletingFiles) {
            filePaths.addAll(mDeletingFiles.keySet());
        }
        Log.d(TAG, "getDecryptingFiles: " + filePaths.size());
        return filePaths;
    }

    public Set<String> getDecryptFailFiles() {
        Set<String> failFiles = new HashSet<String>();
        synchronized (mBadKeyFiles) {
            for (String fileInfo : mBadKeyFiles) {
                fileInfo = (fileInfo.indexOf(File.separator) > 0 ? fileInfo
                        .substring(fileInfo.indexOf(File.separator) + 1)
                        : fileInfo);
                String filePath = null;
                if (fileInfo != null) {
                    filePath = fileInfo.indexOf(File.separator) > 0 ? fileInfo
                            .substring(fileInfo.indexOf(File.separator)) : null;
                }
                boolean res = (filePath != null) ? failFiles.add(filePath)
                        : false;
            }
        }
        return failFiles;
    }

    private void removeFileFromDecryptingFiles(String file) {
        if (null == file || file.isEmpty()) {
            return;
        }
        synchronized (mDecryptingFiles) {
            if (mDecryptingFiles.get(file) != null) {
                mDecryptingFiles.remove(file);
            }
        }
    }

    private void removeFileFromEncryptingFiles(String file) {
        if (null == file || file.isEmpty()) {
            return;
        }
        synchronized (mEncryptingFiles) {
            if (mEncryptingFiles.get(file) != null) {
                Log.s(TAG, "removeFileFromEncryptingFiles " + file);
                mEncryptingFiles.remove(file);
            }
        }
    }

    private void addBadKeyFile(FileInfo file) {
        if (file != null && !TextUtils.isEmpty(file.getFileAbsolutePath())) {
            String mergeString = String
                    .format("%d", System.currentTimeMillis())
                    + "/"
                    + String.format("%d", file.getFileSize())
                    + file.getFileAbsolutePath();
            Log.d(TAG, "addBadKeyFile " + mergeString);
            Iterator<String> iter = mBadKeyFiles.iterator();
            boolean isInSet = false;
            while (iter.hasNext()) {
                String filePathInSet = iter.next();
                if (filePathInSet.endsWith(file.getFileAbsolutePath())) {
                    isInSet = true;
                    break;
                }
            }
            if (!isInSet) {
                mBadKeyFiles.add(mergeString);
            }
        }
    }

    public void listenNotificationEvent(Runnable notifyProcessor) {
        mNotificationProcessor = notifyProcessor;
    }

    public void unListenNotificationEvent(Runnable notifyProcessor) {
        mNotificationProcessor = null;
    }

    public boolean cancelEncryptAndDecrypt() {
        mServiceHandler.removeMessages(ENCRYPT_MESSAGE);
        mIsCancelled.set(true);
        return true;
    }

    private void doDelete(List<FileInfo> toDeleteFiles) {
        Context context = getApplicationContext();
        for (int i = 0; (toDeleteFiles != null) && (i < toDeleteFiles.size()); i++) {
            if (PermissionUtils.hasStorageWritePermission(context)) {
                DataBaseHelper.deleteFileInMediaStore(context, toDeleteFiles.get(i)
                    .getFileAbsolutePath());
                boolean res = toDeleteFiles.get(i).getSDCardWritableFile(context).delete();
                if (!res) {
                    Log.e(TAG, "delete file: "
                        + toDeleteFiles.get(i).getFileAbsolutePath()
                        + "failed.");
                }
            } else {
                Log.d(TAG, "no write storage permission");
                return;
            }
        }
    }

    public interface FileOperationEventListener {
        int ERROR_CODE_NAME_VALID = 100;
        int ERROR_CODE_SUCCESS = 0;

        int ERROR_CODE_UNSUCCESS = -1;
        int ERROR_CODE_NAME_EMPTY = -2;
        int ERROR_CODE_NAME_TOO_LONG = -3;
        int ERROR_CODE_FILE_EXIST = -4;
        int ERROR_CODE_NOT_ENOUGH_SPACE = -5;
        int ERROR_CODE_DELETE_FAILS = -6;
        int ERROR_CODE_USER_CANCEL = -7;
        int ERROR_CODE_PASTE_TO_SUB = -8;
        int ERROR_CODE_UNKOWN = -9;
        int ERROR_CODE_COPY_NO_PERMISSION = -10;
        int ERROR_CODE_MKDIR_UNSUCCESS = -11;
        int ERROR_CODE_CUT_SAME_PATH = -12;
        int ERROR_CODE_BUSY = -100;
        int ERROR_CODE_DELETE_UNSUCCESS = -13;
        int ERROR_CODE_PASTE_UNSUCCESS = -14;
        int ERROR_CODE_DELETE_NO_PERMISSION = -15;
        int ERROR_CODE_COPY_GREATER_4G_TO_FAT32 = -16;
        int ERROR_CODE_INVALID_ARGUMENT = -17;
        int ERROR_CODE_NO_CHANGE = -18;

        void onTaskPrepare();

        void onTaskProgress(int now, int total);

        void onTaskResult(int result);

        /**
         * This method create one Fragment and show.
         *
         * @param result error code for task excuted
         * @param list file info list
         * @param modifiedTime current time for this listing
         */
        void onTaskResult(int result, List<FileInfo> list, long modifiedTime);
    }

    public void deleteLockedFiles(List<FileInfo> files) {
        if (files == null) {
            Log.e(TAG, "error, files is null " + files);
            return;
        }
        for (FileInfo file : files) {
            // mDecryptingFiles.put(file.getFileAbsolutePath(), file);
            mDeletingFiles.put(file.getFileAbsolutePath(), file);
        }
        Message msg = mServiceHandler.obtainMessage(DELETE_FILE);
        msg.obj = files;
        mServiceHandler.sendMessage(msg);
    }

    public void tryToDecryptFiles(String userPattern, List<FileInfo> files) {
        for (FileInfo file : files) {
            // mDecryptingFailFiles.remove(file.getFileAbsolutePath());
            String mergeString = String.format("%d", file.getLastModified())
                    + "/" + String.format("%d", file.getFileSize())
                    + file.getFileAbsolutePath();
            if (mBadKeyFiles.contains(mergeString)) {
                mBadKeyFiles.remove(mergeString);
            } else {
                Log.d(TAG, "here not file: ");
            }

            mDecryptingFiles.put(file.getFileAbsolutePath(), file);
        }
        DecryptFileTask task = new DecryptFileTask(files, userPattern);
        Message msg = mServiceHandler.obtainMessage(TRY_TO_DECRYPT);
        msg.obj = task;
        mServiceHandler.sendMessage(msg);
        getApplicationContext().getContentResolver().notifyChange(FILE_URI,
                null);
    }

    public void changePassword(String old, String newPattern) {
        Message msg = mServiceHandler.obtainMessage(CHANGE_PASSWORD);
        String[] password = new String[2];
        password[0] = old;
        password[1] = newPattern;
        msg.obj = password;
        mServiceHandler.sendMessage(msg);
    }

    public boolean isChangePattern() {
        return mChangePatternTimes.get() != 0;
    }

    public boolean cancelTask(long id) {
        DataProtectionTask task = getTask(id);

        if (task != null) {
            Log.d(TAG, "EYES cancelTask " + task.state);
            if (task.state == DataProtectionTask.STATE_ONGOING) {
                FileDescriptor fd = task.getCurrentFd();
                if (fd != null) {
                    int res = CtaDrmUtils.cancel(mClient, fd);
                    if (res == CtaDrmUtils.CTA_ERROR_CANCEL) {
                        task.cancel();
                        task.onTaskCancelled();
                        Log.d(TAG, "cancel task framework return " + res
                                + " fd: " + fd);
                        return true;
                    }
                    Log.d(TAG, "cancel task framework return " + res + " fd: "
                            + fd);
                }
            }
            task.cancel();
            if (task.state == DataProtectionTask.STATE_TODO) {
                task.onTaskCancelled();
            }
            Log.d(TAG, "cancelTask " + id);
        } else {
            Log.d(TAG, "cancelTask " + id + " not in queue");
        }
        return true;
    }

    public void registerMountListener(MountListener listener) {
        Log.d(TAG, "register ");
        mMountPointListener = listener;
    }

    public void unRegisterMountListener(MountListener listener) {
        if (listener == mMountPointListener) {
            mMountPointListener = null;
            Log.d(TAG, "unregister ");
        }
    }

    public void registerDecryptFailListener(DecryptFailListener listener) {
        mDecryptFailListener = listener;
    }

    public void unRegisterDecryptFailListener(DecryptFailListener listener) {
        if (listener == mDecryptFailListener) {
            mDecryptFailListener = null;
            Log.d(TAG, "unregister ");
        }
    }

    private void acquireWakeLock() {
        if (mTaskWakeLock == null) {
            mTaskWakeLock = mPowerManager.newWakeLock(
                    PowerManager.PARTIAL_WAKE_LOCK, "DataProtectionService");
        }
        if (!mTaskWakeLock.isHeld()) {
            mTaskWakeLock.acquire();
            Log.d(TAG, "dataprotection service acquire wakelock.");
        }
    }

    private void releaseWakeLock() {
        if (mTaskWakeLock != null && mTaskWakeLock.isHeld()) {
            mTaskWakeLock.release();
            mTaskWakeLock = null;
            Log.d(TAG, "dataprotection service release wake...");
        }
    }

    private void startNextTask() {
        Log.d(TAG, "startNextTask " + mTasks.size() +
                   ", mProcessorStarted = " + mProcessorStarted);
        if (mTasks.isEmpty()) {
            mProcessorStarted = false;
            releaseWakeLock();
            // All task finish, schedule to stop service
            mDelayedStopHandler.removeCallbacksAndMessages(null);
            mDelayedStopHandler.sendEmptyMessageDelayed(0, IDLE_DELAY);
            return;
        } else if (!mProcessorStarted) {
            DataProtectionTask task = mTasks.peek();
            while (task != null
                    && (task.state != DataProtectionTask.STATE_TODO && task.state != DataProtectionTask.STATE_ONGOING)) {
                mTasks.poll();
                task = mTasks.peek();
                Log.d(TAG, "startNextTask, a task complete." + mTasks.size());
            }
            if (null != task) {
                acquireWakeLock();
                mNotification = mNotifier.updateTaskProgressReturn(task);
                startForeground(mNotificationId, mNotification);
                task.run();
                mProcessorStarted = true;
            } else {
                mProcessorStarted = false;
                releaseWakeLock();
                stopForeground(true);
            }
        } else {
            DataProtectionTask task = mTasks.peek();
            while (task != null
                    && (task.state != DataProtectionTask.STATE_TODO && task.state != DataProtectionTask.STATE_ONGOING)) {
                mTasks.poll();
                task = mTasks.peek();
                Log.d(TAG, "startNextTask, a task complete." + mTasks.size());
            }
            if (null != task && task.state == DataProtectionTask.STATE_TODO) {
                mNotification = mNotifier.updateTaskProgressReturn(task);
                startForeground(mNotificationId, mNotification);
                task.run();
                mProcessorStarted = true;
            } else if (task == null) {
                releaseWakeLock();
                stopForeground(true);
                mProcessorStarted = false;
            } else {
                Log.d(TAG, "task is running,");
            }
        }
    }

    private void startNextFileInCurrentTask() {
        if (!mTasks.isEmpty()) {
            DataProtectionTask task = mTasks.peek();
            int taskState = task.state;
            if (taskState == DataProtectionTask.STATE_TODO
                    || taskState == DataProtectionTask.STATE_ONGOING) {
                task.run();
            } else {
                Log.e(TAG,
                        "startNextFileInCurrentTask task is empty error to here...");
            }
        } else {
            Log.e(TAG, "startNextFileInCurrentTask error to here...");
        }
    }

    private PendingIntent getDecryptFailNotificationIntent(String fileName) {

        Intent intent = new Intent(ACTION_START_MAIN_PATTERN_REQUEST);
        Bundle bundle = new Bundle();
        bundle.putString("FILE_NAME", fileName);
        intent.putExtras(bundle);
        return PendingIntent.getActivity(getApplicationContext(), 0, intent,
                PendingIntent.FLAG_UPDATE_CURRENT);
    }

    private DataProtectionTask getTask(long id) {
        if (mTasks.size() == 0) {
            return null;
        } else {
            Iterator<DataProtectionTask> iter = mTasks.iterator();
            Log.d(TAG, "task size: " + mTasks.size());
            while (iter.hasNext()) {
                DataProtectionTask task = iter.next();
                Log.d(TAG, "task id: " + task.getCreateTime());
                if (task.getCreateTime() == id) {
                    return task;
                }
            }
            return null;
        }
    }

    private boolean checkAvailableSpace(String filePath, long needSize) {
        MountPointManager mountPointManager = MountPointManager.getInstance();
        long availableSize = new File(
                mountPointManager.getRealMountPointPath(filePath))
                .getFreeSpace();
        if ((needSize + SPACE_LIMIT_REMAIN) > availableSize) {
            Log.d(TAG, "needSize " + needSize + " availableSize: "
                    + availableSize);
            return false;
        } else {
            return true;
        }
    }

    // temp file cache is for sdcard plug in, service can delete this temp file.
    private void addTempFile(String filePath) {
        if (!TextUtils.isEmpty(filePath)) {
            mTempFiles.add(filePath);
        }
    }

    private void removeTempFile(String filePath) {
        if (!TextUtils.isEmpty(filePath) && mTempFiles.contains(filePath)) {
            mTempFiles.remove(filePath);
        }
    }

    private static final String TEMP_FILE_HISTORY = "temp_file_history";
    private static final String SHARE_PREFERENCE_SEPERATOR = ":";

    private void saveTempFileToPreference() {
        SharedPreferences prefs = PreferenceManager
                .getDefaultSharedPreferences(getApplicationContext());
        SharedPreferences.Editor editor = prefs.edit();
        // editor.clear();
        StringBuilder builder = new StringBuilder();
        for (String filePath : mTempFiles) {
            builder.append(filePath).append(SHARE_PREFERENCE_SEPERATOR);
        }
        String toString = builder.toString();
        if (TextUtils.isEmpty(toString)) {
            editor.putString(TEMP_FILE_HISTORY, toString);
            editor.commit();
        }
        Log.d(TAG, "saveTempFileToPreference: count = " + mTempFiles.size());
    }

    private void loadTempFileFromPreference() {
        SharedPreferences prefs = PreferenceManager
                .getDefaultSharedPreferences(getApplicationContext());
        String history = prefs.getString(TEMP_FILE_HISTORY, null);
        if (history != null) {
            // history = history.substring(1, history.length() - 1);
            if (history.indexOf(SHARE_PREFERENCE_SEPERATOR) > 0) {
                Collections.addAll(mTempFiles,
                        history.split(SHARE_PREFERENCE_SEPERATOR));
            }
        }
        Log.d(TAG, "loadTempFileFromPreference: count = " + mTempFiles.size());
    }

    class EncryptFileTask extends DataProtectionTask {

        private FileInfo mCurrentFile = null;
        private AtomicBoolean mHasCancelledDone = new AtomicBoolean(false);

        public EncryptFileTask(List<FileInfo> files) {
            mFiles = files;
            state = STATE_TODO;
            current = 0;
            progress = 0;
            when = System.currentTimeMillis();
        }

        void onSdcardUnmounted(String path) {
            Log.s(TAG, "onSdcard unmounted " + path + ", state = " + state);
            if (state == STATE_TODO || state == STATE_ONGOING || state == STATE_SDCARD_UNMOUNTED) {
                if (mFiles != null && mFiles.size() > 0) {
                    String filePath = mFiles.get(0).getFileAbsolutePath();
                    Log.s(TAG, "onSdcard unmounted ,filePath = " + filePath);
                    if (filePath != null && filePath.startsWith(path)) {
                        this.clearFilterFiles();
                        state = STATE_SDCARD_UNMOUNTED;
                        mNotifier.updateTaskUnmoundted(this);
                    }
                }
            }
        }

        FileInfo getCurrentFileInfo() {
            return mCurrentFile;
        }

        void onTaskCancelled() {
            Log.d(TAG, "onTaskCancelled ... " + mHasCancelledDone.get());
            if (mHasCancelledDone.get()) {
                return;
            } else {
                mHasCancelledDone.set(true);
            }
            switch (state) {
            case STATE_TODO:
            case STATE_ONGOING:
                clearFilterFiles();
                // may should synchronized data observer to avoid NPE
                state = STATE_CANCELLED;
                mNotifier.updateTaskCancelled(this);
                if (mDataObserver != null) {
                    List<FileInfo> files = new ArrayList<FileInfo>();
                    while (hasNext()) {
                        files.add(next());
                    }
                    mDataObserver.onEncryptCancel(files);
                }
                mServiceHandler.sendEmptyMessage(MSG_START_NEXT_TASK);
                break;
            }
        }

        private void clearFilterFiles() {
            Log.d(TAG, "clearFilterFiles " + mFiles.size());
            if (mFiles == null || mFiles.size() == 0) {
                Log.e(TAG, " error happen when task no files to do.");
                return;
            }
            for (FileInfo file : mFiles) {
                removeFileFromEncryptingFiles(file.getFileAbsolutePath());
            }
        }

        @Override
        int getTaskType() {
            return TYPE_ENCRYPT;
        }

        boolean hasNext() {
            if (mFiles != null) {
                FileInfo file = null;
                if (current < mFiles.size() && current >= 0) {
                    file = mFiles.get(current);
                }

                // pass not exist file.
                File oFile = file != null ? file.getFile() : null;
                while (oFile != null && !oFile.exists()) {
                    failNum += 1;
                    current++;
                    FileInfo fileInfo = (((current < mFiles.size()) && (current >= 0))) ? mFiles
                            .get(current) : null;
                    oFile = (fileInfo != null ? fileInfo.getFile() : null);
                }
                if (oFile != null) {
                    return true;
                } else {
                    return false;
                }
            } else {
                return false;
            }

        }

        FileInfo next() {
            FileInfo file = null;

            if (current > -1 && current < mFiles.size()) {
                file = mFiles.get(current);
            } else {
                Log.d(TAG, "error encrypt task next exceed file nums..");
            }
            mCurrentFile = file; // cache currentFile;
            current++;
            return file;
        }

        @Override
        public void run() {
            Log.d(TAG, "EncryptFileTask run...");

            if (isCancel()) {
                onTaskCancelled();
                return;
            }
            state = STATE_ONGOING;
            FileInfo current = null;
            if (hasNext()) {
                current = (FileInfo) next();
            } else {
                // to do other judge
                onTaskCompleted();
                return;
                // startNextTask();
            }

            if (current != null && mClient != null) {
                ParcelFileDescriptor roFile = null;
                ParcelFileDescriptor rnFile = null;
                SDCardWritableFile destTmpFile = null;
                try {
                    SDCardWritableFile oldFile = current.getSDCardWritableFile(
                            getApplicationContext());
                    if (!checkAvailableSpace(oldFile.getAbsolutePath(),
                            oldFile.length())) {
                        String msg = null;
                        MountPointManager mountPoint = MountPointManager
                                .getInstance();
                        if (mountPoint.isInternalMountPath(mountPoint
                                .getMountPointPath(oldFile.getAbsolutePath()))) {
                            msg = getString(R.string.msg_not_enough_space_phone_storage);
                        } else if (mountPoint.isExternalMountPath(mountPoint
                                .getMountPointPath(oldFile.getAbsolutePath()))) {
                            msg = getString(R.string.msg_not_enough_space_sdcard);
                        }
                        final String message = msg;
                        if (message != null) {
                            mHandler.post(new Runnable() {
                                public void run() {
                                    UiUtils.showToast(getApplicationContext(),
                                            message);
                                }
                            });
                        }
                        ArrayList<FileInfo> files = new ArrayList<FileInfo>();
                        FileInfo curFileInfo = getCurrentFileInfo();
                        if (curFileInfo != null) {
                            curFileInfo.setChecked(false);
                            files.add(curFileInfo);
                        }

                        if (mDataObserver != null) {
                            mDataObserver.onStorageNotEnough(files);
                        }
                        failNum += 1;
                        mServiceHandler.sendEmptyMessage(MSG_DO_NEXT);
                        return;
                    }
                    String destinationFile = null;
                    String fileName = oldFile.getName();
                    String folderPath = oldFile.getParent();
                    destinationFile = getEncryptTempFilePath(oldFile.getFile());
                    if (destinationFile == null) {
                        failNum += 1;
                        mServiceHandler.sendEmptyMessage(MSG_DO_NEXT);
                        return;
                    }

                    destTmpFile = new FileInfo(new File(destinationFile)).
                            getSDCardWritableFile(getApplicationContext());
                    if (!destTmpFile.exists()) {
                        boolean res = destTmpFile.createNewFile();
                        if (!res) {
                            Log.d(TAG, "can't create temp file "
                                    + destinationFile + " " + res);
                            failNum += 1;
                            mServiceHandler.sendEmptyMessage(MSG_DO_NEXT);
                            return;
                        }
                    } else {
                        destTmpFile.delete();
                        destTmpFile.createNewFile();
                    }

                    roFile = oldFile.getParcelFileDescriptor();
                    rnFile = destTmpFile.getParcelFileDescriptor();
                    Log.d(TAG, "to encrypt file type: " + oldFile.getMimeType());
                    currentFd = roFile.getFileDescriptor();
                    EncryptProgressListener mEncryptProgressListener = new
                        EncryptProgressListener(roFile, rnFile, oldFile, destTmpFile, this);

                    ProgressInfoListener mProgressInfoListener = new
                        ProgressInfoListener(mEncryptProgressListener);

                    mClient.setOnInfoListener(mProgressInfoListener);
                    CtaDrmUtils.encrypt(mClient, roFile.getFileDescriptor(),
                        rnFile.getFileDescriptor(), oldFile.getMimeType());
                } catch (IOException e) {
                    Log.d(TAG, "ioexception " + e);
                    if (destTmpFile != null && destTmpFile.exists()) {
                        boolean tempDeleteRes = destTmpFile.delete();
                        if (!tempDeleteRes) {
                            addTempFile(destTmpFile.getAbsolutePath());
                        }
                    }
                    boolean isUnmounted = checkSdcardIsUnmounted(current);
                    if (isUnmounted) {
                        addTempFile(destTmpFile.getAbsolutePath());
                    }
                    failNum += 1;
                    FileUtils.closeQuietly(roFile);
                    FileUtils.closeQuietly(rnFile);
                    mServiceHandler.sendEmptyMessage(MSG_DO_NEXT);
                }
            }
        }

        public void onTaskCompleted() {
            Log.d(TAG, "EncryptFileTask done " + mFiles.size() + " current "
                    + current);
            for (FileInfo file : mFiles) {
                removeFileFromEncryptingFiles(file.getFileAbsolutePath());
            }
            state = STATE_COMPLETED;
            mNotifier.updateTaskCompleted(this);
            getApplicationContext().getContentResolver().notifyChange(FILE_URI,
                    null);
            mServiceHandler.sendEmptyMessage(MSG_START_NEXT_TASK);
        }

        private boolean checkSdcardIsUnmounted(FileInfo current) {
            if (current != null
                    && !MountPointManager.getInstance().isFilePathValid(
                            current.getFileAbsolutePath())) {
                Log.s(TAG, "path is invalid, " + current.getFileAbsolutePath()
                        + " may unmounted.");
                state = STATE_SDCARD_UNMOUNTED;
                mNotifier.updateTaskUnmoundted(this);
                return true;
            }
            return false;
        }

        private void onError() {

        }

        private String getEncryptTempFilePath(File file) {
            if (null != file) {
                String fileName = file.getName();
                String folderName = file.getParent();
                String destFilePath = folderName + File.separator;
                if (fileName == null || fileName.isEmpty()) {
                    return null;
                } else {
                    destFilePath += fileName + "." + FileInfo.ENCRYPT_FILE_EXTENSION + ".tmp";
                }
                return destFilePath;
            } else {
                Log.s(TAG, "error: file is null " + file);
                return null;
            }
        }

        public boolean needShowNotification() {
            if (state == DataProtectionTask.STATE_ONGOING
                    || state == DataProtectionTask.STATE_TODO
                    && !mIsCancelled.get()) {
                return true;
            }
            return false;
        }

        String getCurrentProcessingFile() {
            if (mCurrentFile != null) {
                return mCurrentFile.getFileName();
            } else {
                if (mFiles != null && mFiles.size() > 0) {
                    return mFiles.get(0).getFileName();
                }
            }
            return "";
        }

        PendingIntent getContentIntent() {
            if (state == STATE_COMPLETED || state == STATE_CANCELLED
                    || state == STATE_SDCARD_UNMOUNTED) {
                return getServiceIntent(getCreateTime());
                // return null;
            }
            if ((state == STATE_TODO || state == STATE_ONGOING)) {
                if (null == mPendIntent) {
                    Intent intent = new Intent(getApplicationContext(),
                            DataProtectionService.class);
                    Bundle bundle = new Bundle();
                    bundle.putLong(KEY_TASK_ID, getCreateTime());
                    bundle.putString("ACTION", "cancel");
                    bundle.putInt("TITLE", R.string.cancel_locking);
                    intent.putExtras(bundle);
                    mPendIntent = PendingIntent.getService(getApplicationContext(),
                            (int) getCreateTime(), intent, PendingIntent.FLAG_UPDATE_CURRENT);
                }
                return mPendIntent;
            } else {
                return getServiceIntent(getCreateTime());
            }
        }

        String getNotificationTitle() {
            String title = null;
            if (state == STATE_TODO) {
                title = String.format(getString(R.string.notify_wait_lock));
            } else if (state == STATE_ONGOING) {
                title = String.format(getString(R.string.notify_locking),
                        getCurrentProcessingFile());
            } else if (state == STATE_COMPLETED) {
                title = getString(R.string.notify_title_finished_lock);
            } else if (state == STATE_CANCELLED) {
                title = getString(R.string.notify_title_cancelled_lock);
            } else if (state == STATE_SDCARD_UNMOUNTED) {
                title = getString(R.string.notify_locking_stop);
            }
            return title;
        }

        int getIcon() {
            int iconId = android.R.drawable.stat_notify_error;
            switch (state) {
            case STATE_ONGOING:
            case STATE_TODO:
            case STATE_COMPLETED:
                iconId = R.drawable.ic_dataprotection_notify_lock;
                break;
            case STATE_CANCELLED:
                iconId = android.R.drawable.stat_notify_error;
                break;
            case STATE_SDCARD_UNMOUNTED:
                iconId = android.R.drawable.stat_notify_error;
                break;
            }
            return iconId;
        }

        String getNotificationContentInfo() {
            String content = "";
            if (state == STATE_TODO) {
                content = "0/" + mFiles.size();
            } else if (state == STATE_ONGOING) {
                content = current + "/" + mFiles.size();
            }
            return content;
        }

        String getNotificationContentText() {
            String content = null;
            if (state == STATE_ONGOING || state == STATE_TODO) {
                content = String.format("%d", progress) + "%";
            } else if (state == STATE_COMPLETED) {
                content = String.format(
                        getString(R.string.notify_lock_completed_done),
                        (current - failNum) < 0 ? 0 : (current - failNum))
                        + ", "
                        + String.format(getString(R.string.failed_msg), failNum);
            } else if (state == STATE_CANCELLED) {
                content = String.format(
                        getString(R.string.notify_locking_remain),
                        (mFiles.size() - current));
            } else if (state == STATE_SDCARD_UNMOUNTED) {
                content = getString(R.string.notify_sdcard_unmount);
            }
            return content;
        }
    }

    private class EncryptProgressListener implements
            ProgressListener {
        private ParcelFileDescriptor mOldFile = null;
        private ParcelFileDescriptor mNewFile = null;
        private SDCardWritableFile mOriginalFile = null;
        private SDCardWritableFile mTmpFile = null;
        private EncryptFileTask mTask = null;

        EncryptProgressListener(ParcelFileDescriptor oldFile,
                ParcelFileDescriptor newFile, SDCardWritableFile oFile,
                SDCardWritableFile tmpFile, EncryptFileTask task) {
            mOldFile = oldFile;
            mNewFile = newFile;
            mOriginalFile = oFile;
            mTmpFile = tmpFile;
            mTask = task;
        }

        public int onProgressUpdate(String inPath, Long currentSize,
                Long totalSize, int error) {
            long progress = 0;
            if (totalSize != 0) {
                progress = (currentSize * 100) / totalSize;
            }
            mTask.progress = (int) progress;
            // mNotifier.updateTaskProgress(mTask);
            mNotifier.updateNotification(mTasks);

            if (error == CtaDrmUtils.CTA_DONE) {
                // for encryting whole file
                if (!MountPointManager.getInstance().isFilePathValid(
                        mOriginalFile.getAbsolutePath()) || inPath == null) {
                    // origina file and temp file already can't access
                    FileUtils.closeQuietly(mNewFile);
                    FileUtils.closeQuietly(mOldFile);
                    addTempFile(mTmpFile.getAbsolutePath());

                    removeFileFromEncryptingFiles(mOriginalFile
                                .getAbsolutePath());
                    // delete files from encrypting files
                    for (FileInfo file : mTask.mFiles) {
                        removeFileFromEncryptingFiles(file
                                .getFileAbsolutePath());
                    }

                    mTask.state = DataProtectionTask.STATE_SDCARD_UNMOUNTED;
                    mNotifier.updateTaskUnmoundted(mTask);
                    mServiceHandler.sendEmptyMessage(MSG_START_NEXT_TASK);
                    return 0;
                }
                FileUtils.closeQuietly(mNewFile);
                FileUtils.closeQuietly(mOldFile);
                String tempFilePath = mTmpFile.getAbsolutePath();
                File toFile = renameTempFile(mTmpFile);
                removeTempFile(tempFilePath);
                if (PermissionUtils.hasStorageWritePermission(getApplicationContext())) {
                    DataBaseHelper.updateInMediaStore(getApplicationContext(),
                        toFile.getAbsolutePath(),
                        mOriginalFile.getAbsolutePath(), 1,
                        toFile.lastModified() / 1000, toFile.length());
                    mOriginalFile.delete();
                } else {
                    mTask.failNum += 1;
                }
            } else if (error == CtaDrmUtils.CTA_MULTI_MEDIA_ENCRYPT_DONE) {
                FileUtils.closeQuietly(mNewFile);
                FileUtils.closeQuietly(mOldFile);
                String oldPath = mOriginalFile.getAbsolutePath();
                String fileName = mOriginalFile.getParent() + File.separator
                        + mOriginalFile.getName() + "."
                        + FileInfo.ENCRYPT_FILE_EXTENSION;
                File toFile = new File(fileName);
                mOriginalFile.renameTo(toFile);
                Log.s(TAG,
                        "originalFile " + inPath + " to: "
                                + mOriginalFile.getAbsolutePath());
                int type = 1;
                if (toFile.lastModified() > 9999999999L) {
                    type = 1000;
                }
                if (PermissionUtils.hasStorageWritePermission(getApplicationContext())) {
                    DataBaseHelper.updateInMediaStore(getApplicationContext(),
                        fileName, oldPath, 1, toFile.lastModified()/type,
                        toFile.length());
                }
                removeTempFile(mTmpFile.getAbsolutePath());
                boolean tmpDeleteResult = mTmpFile.delete();
                Log.d(TAG, "temp file delete result: " + tmpDeleteResult);
            } else if (error == CtaDrmUtils.CTA_CANCEL_DONE) {
                Log.d(TAG, "native cancell done, callback: " + error);
                // native cancelled done
                FileUtils.closeQuietly(mNewFile);
                FileUtils.closeQuietly(mOldFile);
                removeTempFile(mTmpFile.getAbsolutePath());
                mTmpFile.delete();

                mTask.state = DataProtectionTask.STATE_CANCELLED;
                mTask.current = (mTask.current - 1) >= 0 ? mTask.current - 1
                        : 0;
                mNotifier.updateTaskCancelled(mTask);
                for (FileInfo file : mTask.mFiles) {
                    removeFileFromEncryptingFiles(file.getFileAbsolutePath());
                }
                if (mDataObserver != null) {
                    ArrayList<FileInfo> cancelledFiles = new ArrayList<FileInfo>();
                    cancelledFiles.add(mTask.getCurrentFileInfo());
                    FileInfo curProcessFile = mTask.getCurrentFileInfo();
                    while (mTask.hasNext()) {
                        FileInfo file = mTask.next();
                        if (curProcessFile == file) {
                            Log.d("DPEYES",
                                    "currentFileIn"
                                            + curProcessFile
                                                    .getFileAbsolutePath());
                        } else {
                            cancelledFiles.add(file);
                        }
                    }

                    mDataObserver.onEncryptCancel(cancelledFiles);
                }
                mServiceHandler.sendEmptyMessage(MSG_START_NEXT_TASK);
            } else if (error != CtaDrmUtils.CTA_UPDATING) {
                // for unknown error
                if (!MountPointManager.getInstance().isFilePathValid(
                        mOriginalFile.getAbsolutePath()) ||
                        inPath == null) {
                    // origina file and temp file already can't access
                    FileUtils.closeQuietly(mNewFile);
                    FileUtils.closeQuietly(mOldFile);
                    addTempFile(mTmpFile.getAbsolutePath());

                    // delete files from encrypting files

                    removeFileFromEncryptingFiles(mOriginalFile
                                .getAbsolutePath());
                    for (FileInfo file : mTask.mFiles) {
                        removeFileFromEncryptingFiles(file
                                .getFileAbsolutePath());
                    }

                    mTask.state = DataProtectionTask.STATE_SDCARD_UNMOUNTED;
                    mNotifier.updateTaskUnmoundted(mTask);
                    mServiceHandler.sendEmptyMessage(MSG_START_NEXT_TASK);
                    return 0;
                } else if (mOriginalFile.length() == 0) {
                    Log.d(TAG, "error : " + inPath
                            + " length is zero, will not encrypt this file");
                    FileUtils.closeQuietly(mNewFile);
                    FileUtils.closeQuietly(mOldFile);

                    // delete files from encrypting files
                    if (mDataObserver != null
                            && mTask.getCurrentFileInfo() != null) {
                        // mDataObserver.onEncryptFail();
                        Log.d(TAG, "for empty file ");
                        mDataObserver.onEncryptFail(mTask.getCurrentFileInfo());
                    }
                    removeFileFromEncryptingFiles(mTask.getCurrentFileInfo()
                            .getFileAbsolutePath());
                    removeTempFile(mTmpFile.getAbsolutePath());
                    mTmpFile.delete();
                    mTask.failNum += 1;
                } else if (mTask.isCancel()) {
                    FileUtils.closeQuietly(mNewFile);
                    FileUtils.closeQuietly(mOldFile);
                    removeTempFile(mTmpFile.getAbsolutePath());
                    mTmpFile.delete();

                    mTask.state = DataProtectionTask.STATE_CANCELLED;
                    mTask.current = (mTask.current - 1) >= 0 ? mTask.current - 1
                            : 0;
                    mNotifier.updateTaskCancelled(mTask);
                    for (FileInfo file : mTask.mFiles) {
                        removeFileFromEncryptingFiles(file
                                .getFileAbsolutePath());
                    }
                    if (mDataObserver != null) {
                        ArrayList<FileInfo> cancelledFiles = new ArrayList<FileInfo>();
                        cancelledFiles.add(mTask.getCurrentFileInfo());
                        while (mTask.hasNext()) {
                            cancelledFiles.add(mTask.next());
                        }
                        mDataObserver.onEncryptCancel(cancelledFiles);
                    }
                    mServiceHandler.sendEmptyMessage(MSG_START_NEXT_TASK);
                    return 0;
                } else {
                    mTask.failNum += 1;
                    FileUtils.closeQuietly(mNewFile);
                    FileUtils.closeQuietly(mOldFile);

                    // delete files from encrypting files
                    if (mDataObserver != null
                            && mTask.getCurrentFileInfo() != null) {
                        // mDataObserver.onEncryptFail();
                        Log.d(TAG, "for empty file ");
                        mDataObserver.onEncryptFail(mTask.getCurrentFileInfo());
                    }
                    // delete files from encrypting files
                    mEncryptingFiles.remove(mOriginalFile.getAbsolutePath());
                    removeTempFile(mTmpFile.getAbsolutePath());
                    mTmpFile.delete();
                    // to do next.
                }
            }
            if (error != CtaDrmUtils.CTA_UPDATING && !mTask.isCancel()) {
                progress = 100;
                // mNotifier.updateTaskProgress(mTask);
                mNotifier.updateNotification(mTasks);
                mServiceHandler.sendEmptyMessage(MSG_DO_NEXT);
            }
            return 0;
        }

        private void doNext() {
            if (mTask.hasNext()) {

            }
        }

        private File renameTempFile(SDCardWritableFile file) {
            Log.s(TAG,
                    " encrypt done, renameTempFile: " + file.getAbsolutePath());
            File toFile = null;
            if (null != file) {
                String path = file.getAbsolutePath();
                int idx = path.lastIndexOf(".tmp");
                if (idx > 0) {
                    path = path.substring(0, idx);
                }
                toFile = new File(path);
                // String path = toFile.getAbsolutePath();
                path = toFile.getAbsolutePath();
                Log.s(TAG, "toName: " + toFile.getAbsolutePath());
                file.renameTo(toFile);
                return new File(path);
            } else {
                Log.e(TAG, "error: " + "file: " + file);
            }
            return toFile;
        }
    }

    private void showDecryptFailNotification(String fileName) {
        String title = getString(R.string.notify_unlock_fail);
        String contentInfo = getString(R.string.notify_request_input_pattern);
        Intent intent = new Intent(getApplicationContext(),
                DataProtectionService.class);
        Bundle extra = new Bundle();
        extra.putString("ACTION", "view_decrypt_fail_files");
        intent.putExtras(extra);
        PendingIntent toIntent = PendingIntent.getService(
                getApplicationContext(), 400, intent,
                PendingIntent.FLAG_UPDATE_CURRENT);
        mNotifier.updateDecryptFailNotification(title, contentInfo,
                (long) 1000, toIntent);
    }

    class DecryptFileTask extends DataProtectionTask {

        private String mKey = null;
        protected List<String> mCompletedFiles = new ArrayList<String>();
        protected String mMountPointPath = null;
        private FileInfo mCurrentFileInfo = null;
        private AtomicBoolean mHasCancelled = new AtomicBoolean(false);

        DecryptFileTask(List<FileInfo> files) {
            mFiles = files;
            state = STATE_TODO;
            current = 0;
            progress = 0;
            when = System.currentTimeMillis();
        }

        DecryptFileTask(List<FileInfo> files, String key) {
            this(files);
            mKey = key;
        }

        void onSdcardUnmounted(String path) {
            Log.s(TAG, "onSdcard unmounted " + path);
            if (state == STATE_TODO || state == STATE_ONGOING || state == STATE_SDCARD_UNMOUNTED) {
                if (mFiles != null && mFiles.size() > 0) {
                    String filePath = mFiles.get(0).getFileAbsolutePath();
                    if (filePath != null && filePath.startsWith(path)) {
                        this.clearDecryptingFiles();
                        state = STATE_SDCARD_UNMOUNTED;
                        mNotifier.updateTaskUnmoundted(this);
                    }
                }
            }
        }

        void onTaskCancelled() {
            Log.d(TAG, "onTaskCancelled ... " + mHasCancelled.get());
            if (mHasCancelled.get()) {
                return;
            } else {
                mHasCancelled.set(true);
            }
            switch (state) {
            case STATE_TODO:
            case STATE_ONGOING:
                clearDecryptingFiles();
                // may should synchronized data observer to avoid NPE
                state = STATE_CANCELLED;
                mNotifier.updateTaskCancelled(this);
                if (mDataObserver != null) {
                    List<FileInfo> files = new ArrayList<FileInfo>();
                    while (hasNext()) {
                        files.add(next());
                    }
                    mDataObserver.onEncryptCancel(files);
                }
                mServiceHandler.sendEmptyMessage(MSG_START_NEXT_TASK);
                break;
            }
        }

        public void doNext() {
            if (hasNext()) {
                run();
            } else {
                if (isCancel()) {
                    state = STATE_CANCELLED;
                    mNotifier.updateTaskCancelled(this);
                } else {
                    state = STATE_COMPLETED;
                    mNotifier.updateTaskCompleted(this);
                }
                startNextTask();
            }
        }

        private void onCancel() {
            state = STATE_CANCELLED;
            mNotifier.updateTaskCancelled(this);
            startNextTask();
        }

        protected void clearDecryptingFiles() {
            if (null != mFiles) {
                for (FileInfo file : mFiles) {
                    removeFileFromDecryptingFiles(file.getFileAbsolutePath());
                }
            }
        }

        @Override
        public void run() {

            state = STATE_ONGOING;
            if (isCancel()) {
                onTaskCancelled();
                return;
            }
            Log.d(TAG, " DecryptFileTask haNex: " + hasNext());
            if (hasNext()) {
                Context context = getApplicationContext();
                FileInfo file = (FileInfo) next();
                SDCardWritableFile oFile = file.getSDCardWritableFile(context);
                if (!checkAvailableSpace(oFile.getAbsolutePath(),
                        oFile.length())) {
                    String msg = null;
                    MountPointManager mountPoint = MountPointManager
                            .getInstance();
                    if (mountPoint.isInternalMountPath(mountPoint
                            .getMountPointPath(oFile.getAbsolutePath()))) {
                        msg = getString(R.string.msg_not_enough_space_phone_storage);
                    } else if (mountPoint.isExternalMountPath(mountPoint
                            .getMountPointPath(oFile.getAbsolutePath()))) {
                        msg = getString(R.string.msg_not_enough_space_sdcard);
                    }

                    final String message = msg;
                    if (msg != null) {
                        mHandler.post(new Runnable() {
                            public void run() {
                                UiUtils.showToast(getApplicationContext(),
                                        message);
                            }
                        });
                    }
                    ArrayList<FileInfo> files = new ArrayList<FileInfo>();
                    FileInfo curFileInfo = getCurrentFileInfo();
                    if (curFileInfo != null) {
                        curFileInfo.setChecked(false);
                        files.add(curFileInfo);
                    }

                    if (mDataObserver != null) {
                        mDataObserver.onStorageNotEnough(files);
                    }
                    failNum += 1;
                    mServiceHandler.sendEmptyMessage(MSG_DO_NEXT);
                    return;
                }
                String dstFilePath = oFile.getAbsolutePath() + ".tmp";
                Log.s(TAG, "to decrypt...file: " + dstFilePath);

                SDCardWritableFile tempFile = new FileInfo(new File(dstFilePath)).
                        getSDCardWritableFile(context);
                ParcelFileDescriptor orFile = null;
                ParcelFileDescriptor nrFile = null;
                try {
                    if (!tempFile.exists()) {
                        boolean res = tempFile.createNewFile();
                        if (!res) {
                            Log.e(TAG,
                                    "create file failed "
                                            + tempFile.getAbsolutePath());
                            failNum += 1;
                            mServiceHandler.sendEmptyMessage(MSG_DO_NEXT);
                            return;
                        }
                    } else {
                        tempFile.delete();
                        tempFile.createNewFile();
                    }

                    orFile = oFile.getParcelFileDescriptor();
                    nrFile = tempFile.getParcelFileDescriptor();

                    DecryptProgressListener mDecryptProgressListener = new
                        DecryptProgressListener(
                            orFile, nrFile, oFile, tempFile, this);

                    ProgressInfoListener mProgressInfoListener = new
                        ProgressInfoListener(mDecryptProgressListener);

                    mClient.setOnInfoListener(mProgressInfoListener);

                    CtaDrmUtils.decrypt(mClient, orFile.getFileDescriptor(),
                            nrFile.getFileDescriptor(),
                            (mKey == null ? null : mKey.getBytes()));
                    currentFd = orFile.getFileDescriptor();
                    Log.s(TAG, "file: " + file.getFileAbsolutePath() + " "
                            + orFile.getFileDescriptor() + "key: " + mKey);
                } catch (IOException e) {
                    // to do next file?
                    if (tempFile.exists()) {
                        tempFile.delete();
                    }
                    FileUtils.closeQuietly(nrFile);
                    FileUtils.closeQuietly(orFile);
                    onError(e, tempFile.getFile());
                    return;
                }
            } else {
                state = STATE_COMPLETED;
                mNotifier.updateTaskCompleted(this);
                Log.d(TAG, "decrypt task: " + getCreateTime()
                        + " finish successfully");
                clearDecryptingFiles();
                getApplicationContext().getContentResolver().notifyChange(
                        FILE_URI, null);
                mServiceHandler.sendEmptyMessage(MSG_START_NEXT_TASK);
            }
        }

        private void onError(Exception e, File tempFile) {
            Log.d(TAG, "onError exception " + e);

            FileInfo currentFileInfo = getCurrentFileInfo();
            if (currentFileInfo != null
                    && !MountPointManager.getInstance().isFilePathValid(
                            currentFileInfo.getFileAbsolutePath())) {
                Log.s(TAG,
                        "path is invalid, "
                                + currentFileInfo.getFileAbsolutePath()
                                + " may unmounted.");

                // sdcard unmounted, cache this for deleting this tmp file when
                // sdcard plug in
                if (tempFile != null) {
                    addTempFile(tempFile.getAbsolutePath());
                }
                clearDecryptingFiles();
                failNum += 1;
                // no need to notify...
                state = STATE_SDCARD_UNMOUNTED;
                mNotifier.updateTaskUnmoundted(this);
                mServiceHandler.sendEmptyMessage(MSG_START_NEXT_TASK);
            } else {
                Log.d(TAG, " exception " + e);
                failNum += 1;
                if (mDataObserver != null && currentFileInfo != null) {
                    mDataObserver.onDecryptFail(currentFileInfo);
                }
                mServiceHandler.sendEmptyMessage(MSG_DO_NEXT);
            }
        }

        @Override
        int getTaskType() {
            return DataProtectionTask.TYPE_DECRYPT;
        }

        @Override
        boolean hasNext() {
            if (mFiles != null) {
                if (current < mFiles.size() && current >= 0) {
                    return true;
                } else {
                    return false;
                }
            } else {
                return false;
            }
        }

        @Override
        FileInfo next() {
            FileInfo file = null;

            if (current < mFiles.size()) {
                file = mFiles.get(current);
            } else {
                Log.d(TAG, " error: not go to here.");
            }
            mCurrentFileInfo = file;
            current++;
            return file;
        }

        FileInfo getCurrentFileInfo() {
            return mCurrentFileInfo;
        }

        public boolean needShowNotification() {
            if (state == DataProtectionTask.STATE_ONGOING
                    || state == DataProtectionTask.STATE_TODO
                    && !mIsCancelled.get()) {
                return true;
            }
            return false;
        }

        String getCurrentProcessingFile() {
            String fileName = null;
            if (mCurrentFileInfo != null) {
                fileName = mCurrentFileInfo.getFileName();
            } else {
                if (mFiles != null && mFiles.size() > 0) {
                    fileName = mFiles.get(0).getFileName();
                }
            }
            if (fileName != null) {
                if (fileName.startsWith(".")) {
                    fileName = fileName.substring(1);
                }
                if (fileName.endsWith("." + FileInfo.ENCRYPT_FILE_EXTENSION)) {
                    fileName = fileName.substring(
                            0,
                            fileName.indexOf("."
                                    + FileInfo.ENCRYPT_FILE_EXTENSION));
                }
            }
            return fileName;
        }

        String getNotificationTitle() {
            String title = null;

            if (state == STATE_TODO) {
                title = String.format(getString(R.string.notify_wait_unlock));
            } else if (state == STATE_ONGOING) {
                title = String.format(getString(R.string.notify_unlocking),
                        getCurrentProcessingFile());
            } else if (state == STATE_COMPLETED) {
                title = getString(R.string.notify_title_finished_unlock);
            } else if (state == STATE_CANCELLED) {
                title = getString(R.string.notify_title_cancelled_unlock);
            } else if (state == STATE_SDCARD_UNMOUNTED) {
                title = getString(R.string.notify_unlocking_stop);
            }
            return title;
        }

        int getIcon() {
            int iconId = android.R.drawable.stat_notify_error;
            switch (state) {
            case STATE_ONGOING:
            case STATE_TODO:
            case STATE_COMPLETED:
                iconId = R.drawable.ic_dataprotection_notify_unlock;
                break;
            case STATE_CANCELLED:
                iconId = android.R.drawable.stat_notify_error;
                break;
            case STATE_SDCARD_UNMOUNTED:
                iconId = android.R.drawable.stat_notify_error;
                break;
            }
            return iconId;
        }

        PendingIntent getContentIntent() {
            if (state == STATE_COMPLETED || state == STATE_CANCELLED
                    || state == STATE_SDCARD_UNMOUNTED) {
                return getServiceIntent(getCreateTime());
                // return null;
            }
            if ((state == STATE_TODO || state == STATE_ONGOING)) {
                if (null == mPendIntent) {
                    Intent intent = new Intent(getApplicationContext(),
                            DataProtectionService.class);
                    Bundle bundle = new Bundle();
                    bundle.putLong(KEY_TASK_ID, getCreateTime());
                    bundle.putString("ACTION", "cancel");
                    bundle.putInt("TITLE", R.string.cancel_unlocking);
                    intent.putExtras(bundle);
                    mPendIntent = PendingIntent.getService(getApplicationContext(),
                            (int) getCreateTime(), intent, PendingIntent.FLAG_UPDATE_CURRENT);
                }
                return mPendIntent;
            }
            return null;
        }

        String getNotificationContentInfo() {
            String content = "";
            if (state == STATE_TODO) {
                content = 0 + "/" + mFiles.size();
            } else if (state == STATE_ONGOING) {
                content = current + "/" + mFiles.size();
            }
            return content;
        }

        String getNotificationContentText() {
            String content = null;
            if (state == STATE_ONGOING || state == STATE_TODO) {
                content = String.format("%d", progress) + "%";
            } else if (state == STATE_COMPLETED) {
                content = String.format(
                        getString(R.string.notify_unlock_completed_done),
                        (current - failNum) < 0 ? 0 : (current - failNum))
                        + ", "
                        + String.format(getString(R.string.unlock_failed_msg),
                                failNum);
            } else if (state == STATE_CANCELLED) {
                int remain = mFiles.size() - current;
                content = String.format(
                        getString(R.string.notify_unlocking_remain), remain);
            } else if (state == STATE_SDCARD_UNMOUNTED) {
                content = getString(R.string.notify_sdcard_unmount);
            }
            return content;
        }

        private class DecryptProgressListener implements
                ProgressListener {
            private ParcelFileDescriptor mEncryptedFile = null;
            private ParcelFileDescriptor mDecryptedTmpFile = null;
            private SDCardWritableFile mOriginalFile = null;
            private SDCardWritableFile mTempFile = null;
            private DecryptFileTask mTask = null;

            public DecryptProgressListener(ParcelFileDescriptor oldFile,
                    ParcelFileDescriptor newFile, SDCardWritableFile oFile,
                    SDCardWritableFile tmpFile, DecryptFileTask task) {
                mEncryptedFile = oldFile;
                mDecryptedTmpFile = newFile;
                mOriginalFile = oFile;
                mTempFile = tmpFile;
                mTask = task;
            }

            public int onProgressUpdate(String inPath, Long currentSize,
                    Long totalSize, int error) {
                long progress = 0;
                if (totalSize != 0) {
                    progress = (currentSize * 100) / totalSize;
                }
                mTask.progress = (int) progress;
                mNotifier.updateNotification(mTasks);

                if (error == CtaDrmUtils.CTA_DONE) {
                    // for file is encrypted by whole
                   if (!MountPointManager.getInstance().isFilePathValid(
                        mOriginalFile.getAbsolutePath()) ||
                        inPath == null) {
                    // origina file and temp file already can't access
                    FileUtils.closeQuietly(mEncryptedFile);
                    FileUtils.closeQuietly(mDecryptedTmpFile);
                    addTempFile(mTempFile.getAbsolutePath());

                    removeFileFromDecryptingFiles(mOriginalFile
                                .getAbsolutePath());
                    clearDecryptingFiles();

                    mTask.state = DataProtectionTask.STATE_SDCARD_UNMOUNTED;
                    mNotifier.updateTaskUnmoundted(mTask);
                    mServiceHandler.sendEmptyMessage(MSG_START_NEXT_TASK);
                    return 0;
                    }

                    FileUtils.closeQuietly(mEncryptedFile);
                    FileUtils.closeQuietly(mDecryptedTmpFile);

                    String tempFilePath = mTempFile.getAbsolutePath();
                    File toName = renameFile(mTempFile);
                    removeTempFile(tempFilePath);

                    if (PermissionUtils.hasStorageWritePermission(getApplicationContext())) {
                        DataBaseHelper.deleteFileInMediaStore(
                            getApplicationContext(),
                            mOriginalFile.getAbsolutePath());
                        mTask.mCompletedFiles.add(mOriginalFile.getAbsolutePath());
                        // mDecryptingFiles.remove(mOriginalFile.getAbsolutePath());
                        addUnlockHistory(toName.getAbsolutePath());
                        mOriginalFile.delete();
                        DataBaseHelper.scanFile(getApplicationContext(),
                            toName.getAbsolutePath());
                        doNext();
                    }
                } else if (error == CtaDrmUtils.CTA_UPDATING) {
                    // do nothing
                } else if (error == CtaDrmUtils.CTA_MULTI_MEDIA_DECRYPT_DONE) {
                    FileUtils.closeQuietly(mEncryptedFile);
                    FileUtils.closeQuietly(mDecryptedTmpFile);

                    String oldPath = mOriginalFile.getAbsolutePath();
                    String toName = getName(mTempFile.getFile());

                    // delete temp file
                    String tempFilePath = mTempFile.getAbsolutePath();
                    removeTempFile(tempFilePath);
                    mTempFile.delete();

                    Log.s(TAG, " state " + error + " toName: " + toName
                            + " originalPath: " + inPath + " size: "
                            + mOriginalFile.length());

                    // mDecryptingFiles.remove(mOriginalFile.getAbsolutePath());
                    mTask.mCompletedFiles.add(mOriginalFile.getAbsolutePath());
                    mOriginalFile.renameTo(new File(toName));
                    if (PermissionUtils.hasStorageWritePermission(getApplicationContext())) {
                        DataBaseHelper.deleteFileInMediaStore(
                            getApplicationContext(), oldPath);
                        Log.s(TAG,
                            "originalFile: " + mOriginalFile.getAbsolutePath());
                        // mOriginalFile.delete();
                        DataBaseHelper.scanFile(getApplicationContext(), toName);
                        mUnlockHistoryFiles.add(toName);
                        doNext();
                    }
                } else if (error == CtaDrmUtils.CTA_ERROR_BADKEY) {
                    // here is decrypt fail file because default password can't
                    // decrypt this file;
                    Log.s(TAG, "decrypt fail " + inPath + " code: " + error
                            + " " + mDecryptingFailFiles.size());
                    removeFileFromDecryptingFiles(mOriginalFile
                            .getAbsolutePath());
                    FileUtils.closeQuietly(mEncryptedFile);
                    FileUtils.closeQuietly(mDecryptedTmpFile);

                    mTask.progress = 100;
                    // mNotifier.updateTaskProgress(mTask);
                    mNotifier.updateNotification(mTasks);
                    // delete temp file
                    String tempFilePath = mTempFile.getAbsolutePath();
                    removeTempFile(tempFilePath);
                    mTempFile.delete();

                    addBadKeyFile(new FileInfo(mOriginalFile.getFile()));
                    showDecryptFailNotification(null);
                    if (mDataObserver != null) {
                        // mDataObserver.onDecryptFile(mTask.getCurrentFileInfo());
                        mDataObserver.onDecryptFail(mTask.getCurrentFileInfo());
                    }
                    failNum += 1;
                    doNext();
                } else if (error == CtaDrmUtils.CTA_CANCEL_DONE) {
                    FileUtils.closeQuietly(mEncryptedFile);
                    FileUtils.closeQuietly(mDecryptedTmpFile);

                    String tempFilePath = mTempFile.getAbsolutePath();
                    removeTempFile(tempFilePath);
                    mTempFile.delete();

                    mTask.mCompletedFiles.add(mOriginalFile.getAbsolutePath());
                    clearDecryptingFiles();
                    // need to notify main ui to update
                    mTask.state = STATE_CANCELLED;
                    mTask.current = (mTask.current - 1) >= 0 ? mTask.current - 1
                            : 0;
                    mNotifier.updateTaskCancelled(mTask);
                    if (mDataObserver != null) {
                        // mDataObserver.onDecryptFile(mTask.getCurrentFileInfo());
                        ArrayList<FileInfo> files = new ArrayList<FileInfo>();
                        files.add(getCurrentFileInfo());
                        while (hasNext()) {
                            files.add(next());
                        }
                        mDataObserver.onDecryptCancel(files);
                    }
                    mServiceHandler.sendEmptyMessage(MSG_START_NEXT_TASK);
                    return 0;
                } else {
                    //Log.d(TAG, " other error " + error + " path: " + inPath);
                    // other unknown error.
                    if (!MountPointManager.getInstance().isFilePathValid(
                            mOriginalFile.getAbsolutePath()) ||
                            inPath == null) {
                        // original file and temp file already can't access
                        FileUtils.closeQuietly(mEncryptedFile);
                        FileUtils.closeQuietly(mDecryptedTmpFile);

                        addTempFile(mTempFile.getAbsolutePath());
                        // delete decrypting files
                        removeFileFromDecryptingFiles(mOriginalFile
                                .getAbsolutePath());
                        clearDecryptingFiles();

                        // notify ui to update data
                        if (mDataObserver != null) {
                            // mDataObserver.onDecryptFile(mTask.getCurrentFileInfo());
                        }
                        failNum += 1;
                        mTask.state = DataProtectionTask.STATE_SDCARD_UNMOUNTED;
                        mNotifier.updateTaskUnmoundted(mTask);
                        mServiceHandler.sendEmptyMessage(MSG_START_NEXT_TASK);
                        return 0;
                    } else {
                        if (mTempFile.exists()) {
                            removeTempFile(mTempFile.getAbsolutePath());
                            mTempFile.delete();
                        }
                        mDecryptingFiles
                                .remove(mOriginalFile.getAbsolutePath());
                        if (mDataObserver != null) {
                            mDataObserver.onDecryptFail(mTask
                                    .getCurrentFileInfo());
                        }
                        FileUtils.closeQuietly(mEncryptedFile);
                        FileUtils.closeQuietly(mDecryptedTmpFile);
                        mTempFile.delete();
                        // mDecryptingFailFiles.add(new
                        // FileInfo(mOriginalFile));
                        failNum += 1;
                        mTask.progress = 100;
                        // mNotifier.updateTaskProgress(mTask);
                        mNotifier.updateNotification(mTasks);
                        doNext();
                    }
                }
                return 0;
            }

            private void doNext() {
                if (mTask.hasNext()) {
                    mServiceHandler.sendEmptyMessage(MSG_DO_NEXT);
                } else if(!isCancel()) {
                    state = STATE_COMPLETED;
                    mNotifier.updateTaskCompleted(mTask);
                    clearDecryptingFiles();
                    getApplicationContext().getContentResolver().notifyChange(
                            FILE_URI, null);
                    mServiceHandler.sendEmptyMessage(MSG_START_NEXT_TASK);
                }
            }

            private File renameFile(SDCardWritableFile file) {
                //Log.d(TAG,
                //        " decrypt done, renameFile: " + file.getAbsolutePath());
                String fileName = file.getName();
                String toName = file.getParent() + File.separator;
                fileName = FileUtils.trimDot(fileName);
                int idx = fileName.lastIndexOf("."
                        + FileInfo.ENCRYPT_FILE_EXTENSION + ".tmp");
                if (idx > 0) {
                    toName += fileName.substring(0, idx);
                }
                File newFile = new File(toName);
                file.renameTo(newFile);
                //Log.d(TAG, "renameToFile: " + newFile.getAbsolutePath());
                return newFile;
            }

            private String getName(File file) {
                String fileName = file.getName();
                String toName = file.getParent() + File.separator;
                if (fileName.startsWith(".")) {
                    fileName = fileName.substring(1); // delete the first dot
                }
                int idx = fileName.lastIndexOf("."
                        + FileInfo.ENCRYPT_FILE_EXTENSION + ".tmp");
                if (idx > 0) {
                    toName += fileName.substring(0, idx);
                }
                return toName;
            }

            private void removeCacheDecryptingFiles() {
                for (int i = mTask.current; i < mTask.mFiles.size(); i++) {
                    mDecryptingFiles.remove(mTask.mFiles.get(i));
                }
                for (int i = 0; i < mTask.mCompletedFiles.size(); i++) {
                    mDecryptingFiles.remove(mTask.mCompletedFiles.get(i));
                }
            }
        }
    }

    class DeleteLockedFilesTask extends DataProtectionTask {

        private ArrayList<String> mDeleteList = new ArrayList<String>();
        private FileInfo mCurrentFileInfo = null;

        DeleteLockedFilesTask(List<FileInfo> files) {
            mFiles = files;
            state = STATE_TODO;
            current = 0;
            progress = 0;
            when = System.currentTimeMillis();
        }

        void onSdcardUnmounted(String path) {
            //Log.d(TAG, "onSdcard unmounted " + path);
            if (state == STATE_TODO || state == STATE_ONGOING || state == STATE_SDCARD_UNMOUNTED) {
                if (mFiles != null && mFiles.size() > 0) {
                    String filePath = mFiles.get(0).getFileAbsolutePath();
                    if (filePath != null && filePath.startsWith(path)) {
                        this.clearFilterFiles();
                        state = STATE_SDCARD_UNMOUNTED;
                        mNotifier.updateTaskUnmoundted(this);
                    }
                }
            }
        }

        void onTaskCancelled() {
            Log.d(TAG, "DeleteFileTask onTaskCancelled, current state: "
                    + state);
            if (state == STATE_CANCELLED) {
                return;
            }
            state = STATE_CANCELLED;
            Log.d(TAG, "DeleteFileTask has been cacelled");
            mNotifier.updateTaskCancelled(this);
            // update data base
            if (mDeleteList.size() > 0) {
                if (PermissionUtils.hasStorageWritePermission(getApplicationContext())) {
                    DataBaseHelper.deleteFilesInMediaStore(getApplicationContext(),
                        mDeleteList);
                }
            }
            clearFilterFiles();
            getApplicationContext().getContentResolver().notifyChange(FILE_URI,
                    null);
            mServiceHandler.sendEmptyMessage(MSG_START_NEXT_TASK);
        }

        @Override
        public void run() {
            Log.d(TAG, "DeleteFileTask run() " + current + " hasNext: "
                    + hasNext());
            if (isCancel()) {
                state = STATE_CANCELLED;
                Log.d(TAG, "DeleteFileTask has been cacelled");
                mNotifier.updateTaskCancelled(this);
                // update data base
                if (mDeleteList.size() > 0) {
                    if (PermissionUtils.hasStorageWritePermission(getApplicationContext())) {
                        DataBaseHelper.deleteFilesInMediaStore(
                            getApplicationContext(), mDeleteList);
                    }
                }
                clearFilterFiles();
                getApplicationContext().getContentResolver().notifyChange(
                        FILE_URI, null);
                mServiceHandler.sendEmptyMessage(MSG_START_NEXT_TASK);
            } else {
                switch (state) {
                case STATE_TODO:
                    state = STATE_ONGOING;
                case STATE_ONGOING:
                    if (hasNext()) {
                        FileInfo fileInfo = next();
                        Context context = getApplicationContext();
                        boolean res = fileInfo.getSDCardWritableFile(context).delete();
                        if (!res) {
                            failNum += 1;
                            mDeletingFiles.remove(fileInfo
                                    .getFileAbsolutePath());
                            if (mDataObserver != null) {
                                mDataObserver.onDeleteFail(fileInfo);
                            }
                        } else {
                            mDeleteList.add(fileInfo.getFileAbsolutePath());
                        }
                        progress = (mFiles != null && mFiles.size() != 0) ? (current)
                                * 100 / mFiles.size()
                                : 0;
                        mNotifier.updateNotification(mTasks);
                        mServiceHandler.sendEmptyMessage(MSG_DO_NEXT);
                    } else {
                        onTaskCompleted();
                    }
                    break;
                default:
                    break;
                }
            }
        }

        private void onTaskCompleted() {
            if (PermissionUtils.hasStorageWritePermission(getApplicationContext())) {
                DataBaseHelper.deleteFilesInMediaStore(getApplicationContext(),
                    mDeleteList);
            }
            // delete files in filter list
            clearFilterFiles();
            state = STATE_COMPLETED;
            mNotifier.updateTaskCompleted(this);
            getApplicationContext().getContentResolver().notifyChange(FILE_URI,
                    null);
            mServiceHandler.sendEmptyMessage(MSG_START_NEXT_TASK);
        }

        private void clearFilterFiles() {
            for (FileInfo file : mFiles) {
                mDeletingFiles.remove(file.getFileAbsolutePath());
            }
        }

        @Override
        int getTaskType() {
            return DataProtectionTask.TYPE_DELETE;
        }

        @Override
        boolean hasNext() {
            if (mFiles != null) {
                if (current < mFiles.size()) {
                    return true;
                }
            }
            return false;
        }

        @Override
        FileInfo next() {
            FileInfo file = null;
            if (current < mFiles.size()) {
                file = mFiles.get(current);
            } else {
                Log.d(TAG, " error: not go to here.");
            }
            mCurrentFileInfo = file;
            current++;
            return file;
        }

        public boolean needShowNotification() {
            if (state == DataProtectionTask.STATE_ONGOING
                    || state == DataProtectionTask.STATE_TODO
                    && !mIsCancelled.get()) {
                return true;
            }
            return false;
        }

        String getCurrentProcessingFile() {
            String fileName = null;
            if (mCurrentFileInfo != null) {
                fileName = mCurrentFileInfo.getFileName();
            } else {
                if (mFiles != null && mFiles.size() > 0) {
                    fileName = mFiles.get(0).getFileName();
                }
            }
            if (fileName != null) {
                if (fileName.startsWith(".")) {
                    fileName = fileName.substring(1);
                }
                if (fileName.endsWith("." + FileInfo.ENCRYPT_FILE_EXTENSION)) {
                    fileName = fileName.substring(
                            0,
                            fileName.indexOf("."
                                    + FileInfo.ENCRYPT_FILE_EXTENSION));
                }
            }
            return fileName;
        }

        int getIcon() {
            int icon = R.drawable.ic_dataprotection_notify_delete;
            switch (state) {
            case STATE_TODO:
            case STATE_ONGOING:
                break;
            case STATE_CANCELLED:
                icon = android.R.drawable.stat_notify_error;
                break;
            default:
                break;
            }
            return icon;
        }

        String getNotificationTitle() {
            // to do
            String title = "";
            switch (state) {
            case STATE_TODO:
            case STATE_ONGOING:
                title = String.format(getString(R.string.notify_deleting),
                        getCurrentProcessingFile());
                break;
            case STATE_CANCELLED:
                title = getString(R.string.notify_delete_cancel);
                break;
            case STATE_COMPLETED:
                title = getString(R.string.notify_delete_completed);
                break;
            default:
                break;
            }
            return title;
        }

        String getNotificationContentInfo() {

            String contentInfo = "";
            switch (state) {
            case STATE_TODO:
            case STATE_ONGOING:
                contentInfo = current + "/" + mFiles.size();
                break;
            case STATE_CANCELLED:
                break;
            case STATE_COMPLETED:
                break;
            case STATE_SDCARD_UNMOUNTED:
                break;
            default:
                break;
            }
            return contentInfo;
        }

        protected String getNotificationContentText() {
            String contentInfo = "";
            switch (state) {
            case STATE_TODO:
            case STATE_ONGOING:
                contentInfo = "" + progress + "%";
                break;
            case STATE_CANCELLED:
                // title = getString(R.string.notify_delete_cancel);
                contentInfo = String.format(
                        getString(R.string.notify_deleting_remain),
                        (mFiles.size() - current));
                break;
            case STATE_COMPLETED:
                contentInfo = String.format(
                        getString(R.string.notify_deleted_completed_done),
                        (current - failNum) < 0 ? 0 : (current - failNum))
                        + ", "
                        + String.format(getString(R.string.delete_failed_msg),
                                failNum);
                break;
            case STATE_SDCARD_UNMOUNTED:
                break;
            default:
                break;
            }
            return contentInfo;
        }

        PendingIntent getContentIntent() {
            switch (state) {
            case STATE_TODO:
            case STATE_ONGOING:
                if (null == mPendIntent) {
                    Intent intent = new Intent(getApplicationContext(),
                            DataProtectionService.class);
                    Bundle bundle = new Bundle();
                    bundle.putLong(KEY_TASK_ID, getCreateTime());
                    bundle.putString("ACTION", "cancel");
                    bundle.putInt("TITLE", R.string.cancel_deleting);
                    intent.putExtras(bundle);
                    mPendIntent = PendingIntent.getService(getApplicationContext(),
                            (int) getCreateTime(), intent, PendingIntent.FLAG_UPDATE_CURRENT);
                }
                return mPendIntent;
            default:
                return getServiceIntent(getCreateTime());
            }
        }
    }

    class ChangePatternTask extends DataProtectionTask {

        private String mOldPattern;
        private String mNewPattern;
        private Cursor lockedFileCursor = null;
        private int total = 0;

        ChangePatternTask(String oldPattern, String newPattern) {
            state = STATE_TODO;
            mOldPattern = oldPattern;
            mNewPattern = newPattern;
            when = System.currentTimeMillis();
        }

        @Override
        public void run() {

            if (mOldPattern == null || mNewPattern == null
                    || mOldPattern.equals(mNewPattern)) {
                state = STATE_COMPLETED;
                mNotifier.updateTaskCompleted(this);
                mServiceHandler.sendEmptyMessage(MSG_START_NEXT_TASK);
                Log.d(TAG, "to do change password run return directly");
                return;
            }
            if (state == STATE_TODO) {
                Log.d(TAG, "to do change password");
                mChangePatternTimes.incrementAndGet();
                state = STATE_ONGOING;
                current = 0;
                StringBuilder stringBuilder = new StringBuilder();
                stringBuilder.append(MediaStore.Files.FileColumns.DATA);
                stringBuilder.append(" LIKE '%");
                stringBuilder.append("." + FileInfo.ENCRYPT_FILE_EXTENSION
                        + "'");
                String selection = stringBuilder.toString();

                // try {
                try {
                    lockedFileCursor = getApplicationContext()
                            .getContentResolver().query(
                                    MediaStore.Files.getContentUri("external"),
                                    new String[] { FileInfo.COLUMN_PATH, },
                                    selection, null, null);
                } catch (Exception e) {
                    Log.d(TAG, "data base exception: " + e);
                }
                String filePath = null;
                if (lockedFileCursor != null) {
                    total = lockedFileCursor.getCount();
                }
                ParcelFileDescriptor file = null;
                ParcelFileDescriptor rwFile = null;
                try {
                    if (lockedFileCursor != null
                            && lockedFileCursor.moveToFirst()) {
                        // do {
                        filePath = FileUtils.getCursorString(lockedFileCursor,
                                FileInfo.COLUMN_PATH);
                        if (filePath != null && !TextUtils.isEmpty(filePath)) {
                            SDCardWritableFile sdFile = new FileInfo(new File(filePath)).
                                    getSDCardWritableFile(getApplicationContext());
                            rwFile = sdFile.getParcelFileDescriptor();
                            int res = CtaDrmUtils.changePassword(mClient,
                                    rwFile.getFileDescriptor(),
                                    mOldPattern.getBytes(), mNewPattern.getBytes());
                            Log.d(TAG, "change result: " + res);
                        } else {
                            Log.s(TAG, "rwFile " + filePath + " error. do next.");
                        }
                        // } while (lockedFileCursor.moveToNext());
                    }
                } catch (IOException e) {
                    failNum += 1;
                    Log.d(TAG, " changePattern exception " + e);
                } finally {
                    if (null != rwFile) {
                        try {
                            rwFile.close();
                        } catch (IOException e) {
                            Log.e(TAG, " close random access file happens IOException " + e);
                        }
                        rwFile = null;
                   }
               }
                if (lockedFileCursor == null || total == 0) {
                    CtaDrmUtils.setKey(mClient, mNewPattern.getBytes());
                }
                if (lockedFileCursor != null && lockedFileCursor.getCount() > 1) {
                    current += 1;
                    progress = total == 0 ? 100 : ((current * 100) / total);
                    // mNotifier.updateTaskProgress(this);
                    mNotifier.updateNotification(mTasks);
                    mServiceHandler.sendEmptyMessage(MSG_DO_NEXT);
                } else {
                    Log.d(TAG, "change password done..");
                    state = STATE_COMPLETED;
                    mChangePatternTimes.decrementAndGet();
                    mNotifier.updateTaskCompleted(this);
                    DataBaseHelper.closeQuietly(lockedFileCursor);
                    mServiceHandler.sendEmptyMessage(MSG_START_NEXT_TASK);
                }
            } else if (state == STATE_ONGOING) {
                if (lockedFileCursor != null && lockedFileCursor.moveToNext()) {
                    ParcelFileDescriptor  file = null;
                    try {
                        String filePath = FileUtils.getCursorString(
                                lockedFileCursor, FileInfo.COLUMN_PATH);
                        if (filePath != null && !TextUtils.isEmpty(filePath)) {
                            SDCardWritableFile sdFile = new FileInfo(new File(filePath)).
                                    getSDCardWritableFile(getApplicationContext());
                            file = sdFile.getParcelFileDescriptor();
                            int res = CtaDrmUtils.changePassword(mClient, file.getFileDescriptor(),
                                    mOldPattern.getBytes(), mNewPattern.getBytes());
                            Log.d(TAG, "change result: " + res);
                        } else {
                            failNum += 1;
                            Log.s(TAG, "file " + filePath + " error. do next.");
                        }
                    } catch (IOException e) {
                        failNum += 1;
                        Log.d(TAG, " changePattern exception " + e);
                    } finally {
                        if (null != file) {
                            try {
                                file.close();
                            } catch (IOException e) {
                                Log.e(TAG, " close random access file happens IOException " + e);
                            }
                            file = null;
                        }
                    }
                    current += 1;
                    progress = total == 0 ? 100 : ((current * 100) / total);
                    // mNotifier.updateTaskProgress(this);
                    mNotifier.updateNotification(mTasks);
                    mServiceHandler.sendEmptyMessage(MSG_DO_NEXT);
                } else {
                    // task already finished
                    Log.d(TAG, "change password done..");
                    state = STATE_COMPLETED;
                    mChangePatternTimes.decrementAndGet();
                    DataBaseHelper.closeQuietly(lockedFileCursor);
                    mNotifier.updateTaskCompleted(this);
                    mServiceHandler.sendEmptyMessage(MSG_START_NEXT_TASK);
                }
            }
        }

        @Override
        int getTaskType() {
            return 0;
        }

        @Override
        boolean hasNext() {
            return false;
        }

        @Override
        Object next() {
            return null;
        }

        public boolean needShowNotification() {
            if (state == DataProtectionTask.STATE_ONGOING
                    || state == DataProtectionTask.STATE_TODO
                    && !mIsCancelled.get()) {
                return true;
            }
            return false;
        }

        String getCurrentProcessingFile() {
            String fileName = null;
            if (mFiles != null && mFiles.size() > 0) {
                fileName = mFiles.get(0).getFileName();
            }
            if (fileName != null) {
                if (fileName.startsWith(".")) {
                    fileName = fileName.substring(1);
                }
                if (fileName.endsWith("." + FileInfo.ENCRYPT_FILE_EXTENSION)) {
                    fileName = fileName.substring(
                            0,
                            fileName.indexOf("."
                                    + FileInfo.ENCRYPT_FILE_EXTENSION));
                }
            }
            return fileName;
        }

        int getIcon() {
            int icon = R.drawable.ic_dataprotection_notify_setting;
            switch (state) {
            case STATE_TODO:
            case STATE_ONGOING:
                break;
            case STATE_CANCELLED:
                icon = R.drawable.ic_dataprotection_notify_setting;
                break;
            default:
                break;
            }
            return icon;
        }

        String getNotificationTitle() {
            // to do
            String title = "";
            switch (state) {
            case STATE_TODO:
            case STATE_ONGOING:
                title = String.format(
                        getString(R.string.notify_change_pattern),
                        getCurrentProcessingFile());
                break;
            case STATE_CANCELLED:
                // title = getString(R.string.notify_change_pattern_cancel);
                break;
            case STATE_COMPLETED:
                title = getString(R.string.notify_change_pattern_completed);
                break;
            default:
                break;
            }
            return title;
        }

        String getNotificationContentInfo() {

            String contentInfo = null;
            return contentInfo;
        }

        protected String getNotificationContentText() {
            String contentInfo = "";
            switch (state) {
            case STATE_TODO:
            case STATE_ONGOING:
                contentInfo = "" + progress + "%";
                break;
            case STATE_CANCELLED:
                break;
            case STATE_COMPLETED:
                break;
            case STATE_SDCARD_UNMOUNTED:
                break;
            default:
                break;
            }
            return contentInfo;
        }

        PendingIntent getContentIntent() {
            switch (state) {
            case STATE_TODO:
            case STATE_ONGOING:
                Intent intent = new Intent(getApplicationContext(),
                        DataProtectionService.class);
                Bundle bundle = new Bundle();
                bundle.putString("ACTION", "change_pattern");
                intent.putExtras(bundle);
                return PendingIntent.getService(getApplicationContext(), 300,
                        intent, PendingIntent.FLAG_UPDATE_CURRENT);
                // break;
            default:
                return getServiceIntent(getCreateTime());
                // break;
            }
        }
    }

    private PendingIntent getServiceIntent(long notifyId) {
        PendingIntent intent = null;
        Bundle data = new Bundle();
        data.putString("ACTION", "NO");
        data.putLong("NOTIFY_ID", notifyId);
        Intent tointent = new Intent(getApplicationContext(),
                DataProtectionService.class);
        tointent.putExtras(data);
        intent = PendingIntent.getService(getApplicationContext(),
                (int) notifyId, tointent, PendingIntent.FLAG_ONE_SHOT);
        return intent;
        // Intent toIntent =
    }

    interface DecryptFailListener {
        void onDecryptFail(String failFile);
    }

    interface NotificationListener {
        void onCancel(long taskId, int resId);

        void onViewDecryptFailHistory();
    }

    private NotificationListener mNotificationListener = null;

    public void registerNotificationListener(NotificationListener listener) {
        mNotificationListener = listener;
    }

    interface EncryptDecryptListener {
        void onStorageNotEnough(List<FileInfo> file);

        void onEncryptCancel(List<FileInfo> files);

        void onDecryptCancel(List<FileInfo> files);

        void onDeleteCancel(List<FileInfo> files);

        void onDecryptFail(FileInfo file);

        void onEncryptFail(FileInfo file);

        void onDeleteFail(FileInfo file);
    }

    private EncryptDecryptListener mDataObserver = null;

    public void registerDataListener(EncryptDecryptListener listener) {
        mDataObserver = listener;
    }

    public void unRegisterDataListener() {
        mDataObserver = null;
    }

    public interface MountListener {
        /**
         * This method will be called when receive a mounted intent.
         */
        void onMounted(String mountPoint);

        /**
         * This method will be implemented by its class who implements this
         * interface, and called when receive a unMounted intent.
         *
         * @param mountPoint
         *            the path of mount point
         */
        void onUnMounted(String unMountPoint);

        /**
         * This method cancel the current action on the SD card which will be
         * unmounted.
         */
        void onEjected(String unMountPoint);

        /**
         * This method re-load volume info when sd swap on/off
         *
         */
        void onSdSwap();
    }

    private void registerBroadcastReceivcer() {
        if (null == mStorageBroastReceiver) {
            mStorageBroastReceiver = new BroadcastReceiver() {
                @Override
                public void onReceive(Context context, Intent intent) {
                    receiveBroadcast(context, intent);
                }
            };
            IntentFilter iFilter = new IntentFilter();
            iFilter.addAction(Intent.ACTION_MEDIA_EJECT);
            iFilter.addAction(Intent.ACTION_MEDIA_MOUNTED);
            iFilter.addAction(Intent.ACTION_MEDIA_UNMOUNTED);
            iFilter.addDataScheme("file");
            registerReceiver(mStorageBroastReceiver, iFilter);
            Log.i(TAG,
                    "<registerExternalStorageListener> register mStorageBroastReceiver");
        }
    }

    private void receiveBroadcast(Context context, Intent intent) {
        String action = intent.getAction();
        Log.i(TAG,
                "<onReceive> action = " + action + " string: "
                        + intent.getDataString() + " " + mMountPointListener);
        if (Intent.ACTION_MEDIA_EJECT.equals(action)) {
            MountPointManager.getInstance().init(context);
            if (mMountPointListener != null) {
                mMountPointListener.onEjected(intent.getDataString());
            }

        } else if (Intent.ACTION_MEDIA_UNMOUNTED.equals(action)) {
            if (mMountPointListener != null) {
                mMountPointListener.onUnMounted(intent.getDataString());
            }
            String unMountPoint = intent.getDataString();
            if (unMountPoint.startsWith(FILE_SCHEME)) {
                unMountPoint = unMountPoint.substring(FILE_SCHEME.length());
            }

            if (!TextUtils.isEmpty(unMountPoint)
                    && MountPointManager.getInstance().isExternalMountPath(
                            unMountPoint)) {
                Message msg = mServiceHandler
                        .obtainMessage(MSG_SDCARD_UNMOUNTED);
                msg.obj = unMountPoint;
                mServiceHandler.sendMessage(msg);
            }
            MountPointManager.getInstance().init(context);
        } else if (Intent.ACTION_MEDIA_MOUNTED.equals(action)) {
            MountPointManager.getInstance().init(context);
            if (mMountPointListener != null) {
                mMountPointListener.onMounted(intent.getDataString());
            }
            String unMountPoint = intent.getDataString();

            if (!TextUtils.isEmpty(unMountPoint)
                    && MountPointManager.getInstance().isExternalMountPath(
                            unMountPoint)) {
                Message msg = mServiceHandler.obtainMessage(MSG_SDCARD_MOUNTED);
                msg.obj = unMountPoint;
                mServiceHandler.sendMessage(msg);
            }
        }

    }

    private void unregisterBroadcastReceivcer() {
        if (null != mStorageBroastReceiver) {
            unregisterReceiver(mStorageBroastReceiver);
        }
    }

    // ///////////////////////////////////////////////////////////////////
    // //////////////////////// Unlock History ///////////////////////////
    // ///////////////////////////////////////////////////////////////////
    private static final String UNLOCK_HISTORY = "unlock_history";
    private static final String SEPARATOR = ", ";
    private OnUnlockHistoryUpdate mUnlockListener = null;

    private void loadUnlockHistory() {
        SharedPreferences prefs = PreferenceManager
                .getDefaultSharedPreferences(getApplicationContext());
        String history = prefs.getString(UNLOCK_HISTORY, null);
        if (history != null) {
            if (history.indexOf(SHARE_PREFERENCE_SEPERATOR) > 0) {
                Collections.addAll(mUnlockHistoryFiles,
                        history.split(SHARE_PREFERENCE_SEPERATOR));
            }
        }
        Log.d(TAG, "loadUnlockHistory: count = " + mUnlockHistoryFiles.size());
    }

    private void saveUnlockHistory() {
        SharedPreferences prefs = PreferenceManager
                .getDefaultSharedPreferences(getApplicationContext());
        SharedPreferences.Editor editor = prefs.edit();
        // editor.clear();
        StringBuilder builder = new StringBuilder();
        for (String filePath : mUnlockHistoryFiles) {
            builder.append(filePath).append(SHARE_PREFERENCE_SEPERATOR);
        }
        editor.putString(UNLOCK_HISTORY, builder.toString());
        editor.commit();
        Log.d(TAG, "saveUnlockHistory: count = " + mUnlockHistoryFiles.size());
    }

    public boolean clearUnlockHistory() {
        mUnlockHistoryFiles.clear();
        saveUnlockHistory();
        return true;
    }

    private void addUnlockHistory(String filePath) {
        if (filePath == null || filePath.isEmpty()) {
            return;
        }
        if (mUnlockHistoryFiles.size() >= 100) {
            mUnlockHistoryFiles.remove(0);
        }
        // / save unlock history as: time+path
        String history = System.currentTimeMillis() + filePath;
        mUnlockHistoryFiles.add(history);
        if (mUnlockListener != null) {
            mUnlockListener.onUpdated();
        }
        Log.s(TAG, " addUnlockHistory: " + history);
    }

    public List<String> getUnlockHistory() {
        List<String> history = new ArrayList<String>(mUnlockHistoryFiles.size());
        history.addAll(mUnlockHistoryFiles);
        return history;
    }

    public void listenUnlockUpdate(OnUnlockHistoryUpdate listener) {
        mUnlockListener = listener;
    }

    // ///////////////////////////////////////////////////////////////////
    // //////////////////////// Decrypt Fail History ///////////////////////////
    // ///////////////////////////////////////////////////////////////////
    private static final String DECRYPT_FAIL_HISTORY = "decrypt_fail_history";
    private static final String TRY_DECRYPT_TIMES = "times_try_to_decrypt";
    private static final String TRY_DECYPT_LOCKOUT_DEADLINE = "deadline_try_decrypt";

    public ArrayList<String> getDecryptFailHistory() {

        ArrayList<String> history = new ArrayList<String>(mBadKeyFiles.size());
        Log.s(TAG, "current Size " + mBadKeyFiles.size() + " file "
                + mBadKeyFiles);

        history.addAll(mBadKeyFiles);
        /*
         * if(mBadKeyFiles.size() > 0) { for(int i = mBadKeyFiles.size() - 1; i
         * >=0; i--) { history.add(mBadKeyFiles.get(i)); } }
         */
        return history;

    }

    public void clearDecryptingFiles() {
        mDecryptingFailFiles.clear();
    }

    private void loadDecryptFailHistory() {
        SharedPreferences prefs = PreferenceManager
                .getDefaultSharedPreferences(getApplicationContext());
        String history = prefs.getString(DECRYPT_FAIL_HISTORY, null);
        mUserAttemptDecryptTime = prefs.getInt(TRY_DECRYPT_TIMES, 0);
        mTryDecryptLockOutDeadline = prefs.getLong(TRY_DECYPT_LOCKOUT_DEADLINE,
                0);
        if (history != null) {
            // history = history.substring(0, history.length() - 1);
            if (history.indexOf(SHARE_PREFERENCE_SEPERATOR) > 0) {
                Collections.addAll(mBadKeyFiles,
                        history.split(SHARE_PREFERENCE_SEPERATOR));
            }
        }
        Log.d(TAG,
                "loadDecryptFailHistory: count = "
                        + mDecryptingFailFiles.size());

    }

    private void saveDecryptFailHistory() {
        SharedPreferences prefs = PreferenceManager
                .getDefaultSharedPreferences(getApplicationContext());
        SharedPreferences.Editor editor = prefs.edit();
        // editor.clear();
        StringBuilder builder = new StringBuilder();
        for (String filePath : mBadKeyFiles) {
            builder.append(filePath).append(SHARE_PREFERENCE_SEPERATOR);
        }
        String toString = builder.toString();
        editor.putString(DECRYPT_FAIL_HISTORY, toString);
        editor.putInt(TRY_DECRYPT_TIMES, mUserAttemptDecryptTime);
        editor.putLong(TRY_DECYPT_LOCKOUT_DEADLINE, mTryDecryptLockOutDeadline);
        boolean res = editor.commit();
        Log.d(TAG,
                "saveDecryptFailHistory: count = "
                        + mDecryptingFailFiles.size() + " " + toString);
    }

    // / Attempt decrypting
    private CountDownTimer mCountdownTimer;
    private int mUserAttemptDecryptTime = 0;
    private long mTryDecryptLockOutDeadline = 0L;

    public int queryUserAttemptTimes() {
        return mUserAttemptDecryptTime;
    }

    public void updateUserAttemptTimes(int times) {
        mUserAttemptDecryptTime = times;
    }

    public long setTryDecryptLockOutDeadline() {
        mTryDecryptLockOutDeadline = DataProtectionApplication.LOCKPATTERN_ATTEMPT_TIMEOUT
                + SystemClock.elapsedRealtime();
        return mTryDecryptLockOutDeadline;
    }

    public long getTryDecryptLockOutDeadline() {
        final long now = SystemClock.elapsedRealtime();
        if (mTryDecryptLockOutDeadline < now
                || mTryDecryptLockOutDeadline > (now + DataProtectionApplication.LOCKPATTERN_ATTEMPT_TIMEOUT)) {
            return 0L;
        }
        return mTryDecryptLockOutDeadline;
    }

    public void unRegistenerNotificationListener() {
        mNotificationListener = null;
    }

    private HashMap<String, String> parseEventMsg(String message) {
        HashMap<String, String> hashMap = new HashMap<String, String>(4);
        String[] dataArr = message.split("::");
        int size = dataArr.length;
        for (int i = 0; i < size; i++) {
            String[] keyValue = dataArr[i].split(":");
            if (2 == keyValue.length) {
                hashMap.put(keyValue[0], keyValue[1]);
            } else {
                //Log.e(TAG, "hongen map is not right,key=" + keyValue[0]);
                //hashMap.put(keyValue[0], "error");
                Log.e(TAG, "hongen map is not right:" + Arrays.toString(dataArr));
            }
        }
        return hashMap;
    }

    /**
     * A listnerner to noify decrypt/encrypt progress
     */
    public interface ProgressListener {
        /**
         * Notify decrypt/encrypt progress
         * @param fd The file processed
         * @param currentSize The decrypted/encrypted file size
         * @param totalSize The total size of file
         * @param error Error code
         * @return {@link CTA_ERROR_NONE} if sucessfully, otherwise return {@link CTA_ERROR_GENRIC}
         */
        public int onProgressUpdate(String inPath, Long currentSize, Long totalSize, int error);
    }

    private class ProgressInfoListener implements DrmManagerClient.OnInfoListener {
        ProgressListener mProgressListener;

        ProgressInfoListener(ProgressListener progressListener) {
            mProgressListener = progressListener;
        }

        @Override
        public void onInfo(DrmManagerClient client, DrmInfoEvent event) {
            if (event.getType() == /*DrmInfoEvent.TYPE_CTA_DECRTYPT_PROGESS*/10001) {
                if (mProgressListener != null) {
                    String message = event.getMessage();
                    Log.s(TAG, "hongen callback: message=" + message);
                    HashMap<String, String> hashMap = parseEventMsg(message);
                    Long dataSize = Long.parseLong(hashMap.get("data_s"));
                    Long cntSize = Long.parseLong(hashMap.get("cnt_s"));
                    String path = hashMap.get("path");
                    String result = hashMap.get("result");
                    if ("cta5_cancel_done".equals(result)) {
                        mProgressListener.onProgressUpdate(path, cntSize, dataSize,
                            CtaDrmUtils.CTA_CANCEL_DONE);
                    } else if ("cta5_done".equals(result)) {
                        mProgressListener.onProgressUpdate(path, cntSize, dataSize,
                            CtaDrmUtils.CTA_DONE);
                    } else if ("cta5_error".equals(result)) {
                        mProgressListener.onProgressUpdate(path, cntSize, dataSize,
                            CtaDrmUtils.CTA_ERROR);
                    } else if ("cta5_updating".equals(result)) {
                        mProgressListener.onProgressUpdate(path, cntSize, dataSize,
                            CtaDrmUtils.CTA_UPDATING);
                    } else if ("cta5_multimedia_encrypt_done".equals(result)) {
                        mProgressListener.onProgressUpdate(path, cntSize, dataSize,
                            CtaDrmUtils.CTA_MULTI_MEDIA_ENCRYPT_DONE);
                    } else if ("cta5_multimedia_decrypt_done".equals(result)) {
                        mProgressListener.onProgressUpdate(path, cntSize, dataSize,
                            CtaDrmUtils.CTA_MULTI_MEDIA_DECRYPT_DONE);
                    } else if ("cta5_error_key".equals(result)) {
                        mProgressListener.onProgressUpdate(path, cntSize, dataSize,
                            CtaDrmUtils.CTA_ERROR_BADKEY);
                    } else {
                        mProgressListener.onProgressUpdate(path, cntSize, dataSize,
                            CtaDrmUtils.CTA_ERROR_GENRIC);
                    }

                }
            } else {
                Log.e(TAG, "type can not be known:type=" + event.getType());
            }

        }
    }
}
