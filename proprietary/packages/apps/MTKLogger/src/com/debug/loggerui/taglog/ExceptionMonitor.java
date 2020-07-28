package com.debug.loggerui.taglog;

import android.content.Intent;
import android.os.FileObserver;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;

import com.debug.loggerui.utils.FileMonitor;
import com.debug.loggerui.utils.Utils;

import java.io.File;
import java.util.List;

/**
 * @author MTK11515
 *
 */
public class ExceptionMonitor {
    private static final String TAG = Utils.TAG + "/ExceptionMonitor";
    private static ExceptionMonitor sInstance;
    private Handler mMessageHandler;

    private ExceptionMonitor() {
        HandlerThread handlerThread = new HandlerThread("ExceptionMonitorHandlerThread");
        handlerThread.start();
        mMessageHandler = new MyHandler(handlerThread.getLooper());
    }

    /**
     * @return ExceptionMonitor
     */
    public static ExceptionMonitor getInstance() {
        if (sInstance == null) {
            synchronized (ExceptionMonitor.class) {
                if (sInstance == null) {
                    sInstance = new ExceptionMonitor();
                }
            }
        }
        return sInstance;
    }

    /**
     * . return void
     */
    public synchronized void startExceptionMonitor() {
        Utils.logd(TAG, "startExceptionMonitor");
        if (mDBHistorySystemMonitor == null || mDBHistoryVendorMonitor == null) {
            Utils.logi(TAG, "startExceptionMonitor is not running, start it!");
            startFileLisener();
        }
    }

    /**
     * . return void
     */
    public void stopExceptionMonitor() {
        Utils.logi(TAG, "stopExceptionMonitor");
        if (mDBHistorySystemMonitor != null) {
            mDBHistorySystemMonitor.stopWatching();
            mDBHistorySystemMonitor = null;
        }
        if (mDBHistoryVendorMonitor != null) {
            mDBHistoryVendorMonitor.stopWatching();
            mDBHistoryVendorMonitor = null;
        }
    }

    private FileMonitor mDBHistorySystemMonitor;
    private FileMonitor mDBHistoryVendorMonitor;

    private void startFileLisener() {
        final String dbHistorySystemPath = Utils.AEE_SYSTEM_PATH + Utils.AEE_DB_HISTORY_FILE;
        Utils.logi(TAG, "Start file monitor for " + dbHistorySystemPath);
        mDBHistorySystemMonitor = new FileMonitor(dbHistorySystemPath,
                                  FileObserver.MODIFY | FileObserver.CREATE) {
            @Override
            protected void notifyModified() {
                notifyNewException(dbHistorySystemPath);
                try {
                    Thread.sleep(1000);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                Utils.logd(TAG, "monitor file change : " + dbHistorySystemPath);
            }
        };
        mDBHistorySystemMonitor.startWatching();

        final String dbHistoryVendorPath = Utils.AEE_VENDOR_PATH + Utils.AEE_DB_HISTORY_FILE;
        Utils.logi(TAG, "Start file monitor for " + dbHistoryVendorPath);
        mDBHistoryVendorMonitor = new FileMonitor(dbHistoryVendorPath, FileObserver.MODIFY) {
            @Override
            protected void notifyModified() {
                Utils.logd(TAG, "monitor file change : " + dbHistoryVendorPath);
                notifyNewException(dbHistoryVendorPath);
                try {
                    Thread.sleep(1000);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        };
        mDBHistoryVendorMonitor.startWatching();
    }

    private void notifyNewException(String dbHistoryPath) {
        Intent exceptionIntent = getLastExceptionIntent(dbHistoryPath);
        if (exceptionIntent != null) {
            Message msg = new Message();
            msg.what = MSG_EXCEPTION_HAPPEND_SEND_BROADCAST;
            msg.obj = exceptionIntent;
            mMessageHandler.sendMessageDelayed(msg, 1000);
            TagLogManager.getInstance().startNewTaglog(exceptionIntent);
        }
    }

    private Intent getLastExceptionIntent(String dbHistory) {
        Intent exceptionIntent = null;

        List<String> dbInfors = Utils.getLogFolderFromFileTree(new File(dbHistory));
        if (dbInfors == null || dbInfors.size() == 0) {
            Utils.logi(TAG, "no db infors in db_history");
            return null;
        }
        String lastException = dbInfors.get(dbInfors.size() - 1);

        String[] dbStr = lastException.split(",");
        if (dbStr.length < 2) {
            Utils.logi(TAG, "wrong db string format, length = " + dbStr.length);
            return null;
        }
        String dbPath = dbStr[0];
        String zzTime = dbStr[1].trim();
        String dbFolderPath = dbPath + File.separator;

        String dbFileName = dbPath.substring(dbPath.lastIndexOf(File.separator) + 1) + ".dbg";
        String zzFileName = "ZZ_INTERNAL";

        exceptionIntent = new Intent(Utils.ACTION_EXP_HAPPENED);
        exceptionIntent.putExtra(Utils.EXTRA_KEY_EXP_PATH, dbFolderPath);
        exceptionIntent.putExtra(Utils.EXTRA_KEY_EXP_NAME, dbFileName);
        exceptionIntent.putExtra(Utils.EXTRA_KEY_EXP_ZZ, zzFileName);
        exceptionIntent.putExtra(Utils.EXTRA_KEY_EXP_TIME, zzTime);
        exceptionIntent.putExtra(Utils.EXTRA_KEY_TAG_TYPE, TagLogUtils.TAG_EXCEPTION);

        Utils.logd(TAG, "new excepion from db_history, exp =" + lastException);
        Utils.logd(TAG, "new excepion from db_history, dbFolderPath =" + dbFolderPath
                + ", dbFileName = " + dbFileName + ", zzTime = " + zzTime);
        return exceptionIntent;
    }

    private static final int MSG_EXCEPTION_HAPPEND_SEND_BROADCAST = 1;

    /**
     * @author MTK81255
     *
     */
    class MyHandler extends Handler {
        public MyHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message message) {
            Utils.logi(TAG, " MyHandler handleMessage --> start " + message.what);
            switch (message.what) {
            case MSG_EXCEPTION_HAPPEND_SEND_BROADCAST:
                Intent exceptionIntent = (Intent) message.obj;
                Utils.sendBroadCast(exceptionIntent);
                Utils.logi(TAG, "send expception happened broadcast.");
                break;
            default:
                Utils.logw(TAG, "Not supported message: " + message.what);
                break;
            }
        }
    };
}
