package com.debug.loggerui.taglog;

import android.content.Intent;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;

import com.debug.loggerui.MyApplication;
import com.debug.loggerui.controller.LogControllerUtils;
import com.debug.loggerui.taglog.db.DBManager;
import com.debug.loggerui.taglog.db.MySQLiteHelper;
import com.debug.loggerui.taglog.db.TaglogTable;
import com.debug.loggerui.utils.Utils;

import java.io.File;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

/**
 * @author MTK81255
 *
 */
public class TagLogManager {
    private static final String TAG = Utils.TAG + "/TagLogManager";

    private List<TagLog> mTaglogList = Collections.synchronizedList(new ArrayList<TagLog>());
    private static TagLogManager sInstance = new TagLogManager();

    private Handler mFileManagerHandler;
    private Handler mTaglogManagerHandler;

    private boolean mIsInitDone = false;
    private static final long SYNC_DB_TIME_OUT = 20 * 60 * 1000;
    private static final int DEAL_MAX_OLD_DB_NUM = 20; // No need deal with too old db history

    private boolean mIsLogAutoStart = false;
    private TagLogManager() {
        DBManager.getInstance().init();
        HandlerThread  myHandler = new HandlerThread("taglogManagerThread");
        myHandler.setPriority(Thread.MIN_PRIORITY);
        myHandler.start();
        mTaglogManagerHandler = new TaglogManagerHandler(myHandler.getLooper());

        mFileManagerHandler = LogFilesManager.getInstance();
        mFileManagerHandler.sendEmptyMessage(TagLogUtils.MSG_DO_FILEMANAGER);
        mIsLogAutoStart = isLogWillAutoStart();
    }

    public static final TagLogManager getInstance() {
        return sInstance;
    }
    /**.
     *
     */
    public void startTagLogManager() {
        Utils.logd(TAG, "startTagLogManager--> isInitDone = " + mIsInitDone);
        if (!mIsInitDone) {
            mIsInitDone = true;
            mTaglogManagerHandler
            .obtainMessage(TagLogUtils.MSG_TAGLOG_MANAGER_INIT, this)
            .sendToTarget();
        }
    }

    private void doInit() {
        doResumeTaglog();
        initExceptionInfoToDB();
        ExceptionMonitor.getInstance().startExceptionMonitor();
    }

    private void doResumeTaglog() {
        List<TagLogData> taglogDataList = DBManager.getInstance().getResumeTaglog();
        if (taglogDataList == null) {
            Utils.logi(TAG, "-->resumeTag(), no taglog need resume, just return!");
            return;
        }
        Utils.logi(TAG, "-->resumeTag(), taglogDataList.size() = " + taglogDataList.size());
        for (TagLogData data : taglogDataList) {
            String taglogState = data.getTaglogTable().getState();
            String fileList = data.getTaglogTable().getFileList();
            TagLog taglog = createNewTaglog();
            Utils.logi(TAG, "taglogId = " + data.getTaglogTable().getTagLogId()
                            + ", taglogFolder =" + data.getTaglogTable().getTargetFolder()
                            + ", taglogState = " + taglogState + ", fileList = " + fileList);
            mTaglogList.add(taglog);
            taglog.resumeTag(data);
        }
    }

    private boolean isNeedTag(Intent intent) {
        if (!Utils.isTaglogEnable()) {
            Utils.logi(TAG, "isNeedTag ? false, for taglog is disabled!");
            return false;
        }
        String taglogType =
                intent.getExtras().getString(Utils.EXTRA_KEY_TAG_TYPE, TagLogUtils.TAG_MANUAL);
        if (taglogType.equals(TagLogUtils.TAG_ONLY_COPY_DB)) {
            Utils.logi(TAG, "isNeedTag ? true, for just copy db!");
            return true;
        }
        //is log running
        boolean isAnyLogRunning = LogControllerUtils.isAnyControlledLogRunning();
        if (!isAnyLogRunning) {
            Utils.logi(TAG, "isNeedTag ? false, for all log is stopped!");
            return false;
        } else if (taglogType.equals(TagLogUtils.TAG_ONLY_COPY_DB)) {
            return true;
        }
        String dbPath = intent.getExtras().getString(Utils.EXTRA_KEY_EXP_PATH, "");
        if (isFilterByFile(dbPath)) {
            Utils.logi(TAG, "isNeedTag ? false, for it's be filtered by"
                            + Utils.TAG_LOG_FILTER_FILE);
            return false;
        }
        return true;
    }

    /**
     * @param intent
     *            Intent
     */
    public synchronized void startNewTaglog(Intent intent) {
        boolean shouldTag = isNeedTag(intent);
        insertExpToTagDB(intent, shouldTag);
        if (!shouldTag) {
            Utils.logi(TAG, "not need do tag ,just return.");
            return;
        }

        synchronized (TagLogManager.class) {
            String dbPath = intent.getStringExtra(Utils.EXTRA_KEY_EXP_PATH);
            if (!Utils.MANUAL_SAVE_LOG.equalsIgnoreCase(dbPath)) {
                for (TagLog taglog : mTaglogList) {
                    Intent inputIntent = taglog.getInputIntent();
                    if (inputIntent == null) {
                        continue;
                    }
                    String taglogDbPath = inputIntent.getStringExtra(
                            Utils.EXTRA_KEY_EXP_PATH);
                    if (taglogDbPath != null && taglogDbPath.equalsIgnoreCase(dbPath)) {
                        Utils.logw(TAG, "startNewTaglog the intent:" + dbPath
                                + " has been exist!");
                        return;
                    }
                }
            }
            TagLog newTagLog = createNewTaglog();
            newTagLog.beginTag(intent);
            mTaglogList.add(newTagLog);
        }
    }

    private boolean isFilterByFile(String dbPath) {
        String zzInternalFilePath = dbPath + File.separator + Utils.EXTRA_VALUE_EXP_ZZ;
        String filterFilePath =
                Utils.getCurrentLogPath() + "/debuglogger/taglog/"
                                + Utils.TAG_LOG_FILTER_FILE;
        File zzInternalFile = new File(zzInternalFilePath);
        File filterFile = new File(filterFilePath);
        if (!zzInternalFile.exists() || !filterFile.exists()) {
            Utils.logd(TAG, "filter file not exit, not do filter for "
                    + "zzPath = " + zzInternalFilePath);
            return false;
        }
        List<String> listZZInternal = Utils.getLogFolderFromFileTree(zzInternalFile);
        List<String> listFilter = Utils.getLogFolderFromFileTree(filterFile);
        for (String zzWords : listZZInternal) {
            for (String filterwords : listFilter) {
                if (zzWords != null && zzWords.contains(filterwords)) {
                    Utils.logd(TAG, "checkItBeFilter() = true, filterwords = "
                            + filterwords
                            + ", contain in zzInternal file zzWords = "
                            + zzWords);
                    return true;
                }
            }
        }
        return false;
    }

    private void insertExpToTagDB(Intent intent, boolean needTag) {
        String dbPath = intent.getExtras().getString(Utils.EXTRA_KEY_EXP_PATH, "");
        String zzTime = intent.getExtras().getString(Utils.EXTRA_KEY_EXP_TIME, "");
        String taglogFolderSuffix = "";
        if (Utils.MANUAL_SAVE_LOG.equalsIgnoreCase(dbPath)) {
            taglogFolderSuffix = intent.getExtras().getString(Utils.EXTRA_KEY_EXP_NAME, "");
        } else {
            File expFile = new File(dbPath);
            if (expFile.exists()) {
                String expFileName = expFile.getName();
                int index = expFileName.lastIndexOf(".");
                if (index != -1 && index < expFileName.length()) {
                    taglogFolderSuffix = expFileName.substring(index + 1);
                }
            }
        }
        String taglogTargetFolder = Utils.geMtkLogPath() + "/" + TagLogUtils.TAGLOG_FOLDER_NAME
                       + "/" + TagLogUtils.TAGLOG_TEMP_FOLDER_PREFIX + "TagLog_"
                       + TagLogUtils.getCurrentTimeString()
                       + ((taglogFolderSuffix == null || "".equals(taglogFolderSuffix))
                           ? "" : ("_" + taglogFolderSuffix));
        String dbfileName = intent.getExtras().getString(Utils.EXTRA_KEY_EXP_NAME, "");
        String state = needTag ? MySQLiteHelper.TAGLOG_STATE_INIT
                               : MySQLiteHelper.TAGLOG_STATE_IGNORE;
        String isneedzip = intent.getExtras()
                          .getBoolean(Utils.EXTRAL_KEY_IS_NEED_ZIP, true) ? "1" : "0";
        String isneedalllogs = intent.getExtras()
                          .getBoolean(Utils.EXTRAL_KEY_IS_NEED_ALL_LOGS, false) ? "1" : "0";
        int needlogtype = intent.getExtras().getInt(Utils.EXTRAL_KEY_IS_NEED_LOG_TYPE, 23);
        String reason = intent.getExtras().getString(Utils.EXTRA_KEY_EXP_REASON, "");
        String fromwhere = intent.getExtras().getString(Utils.EXTRA_KEY_EXP_FROM_WHERE, "");

        TaglogTable taglogTable = new TaglogTable(1000, taglogTargetFolder,
                          state, "", dbPath, dbfileName, "ZZ_INTERNAL", isneedzip, isneedalllogs,
                          needlogtype, reason, fromwhere, zzTime);
        DBManager.getInstance().insertTaglogToDb(taglogTable);
    }

    private TagLog createNewTaglog() {
        Boolean isReleaseToCustomer1 = Utils.isReleaseToCustomer1();
        Utils.logd(TAG, "isReleaseToCustomer1 ? " + isReleaseToCustomer1);
        if (isReleaseToCustomer1) {
            return new Customer1TagLog(mTaglogManagerHandler);
        } else {
            return new CommonTagLog(mTaglogManagerHandler);
        }
    }
    private int mUILockNumber = 0;

    /**
     * @return boolean
     */
    public boolean isUILocked() {
        Utils.logd(TAG, "isUILocked() mUILockNumber = " + mUILockNumber);
        return mUILockNumber > 0;
    }

    /**
     * return void.
     */
    public void initExceptionInfoToDB() {
        List<String> historyPaths = new ArrayList<String>();
        // /data/aee_exp/db_history
        historyPaths.add(Utils.AEE_SYSTEM_PATH +  Utils.AEE_DB_HISTORY_FILE);
        // /data/vendor/aee_exp/db_history
        historyPaths.add(Utils.AEE_VENDOR_PATH +  Utils.AEE_DB_HISTORY_FILE);
        ExecutorService executorPool = Executors.newCachedThreadPool();
        for (final String path : historyPaths) {
            Thread thread = new Thread(new Runnable() {
                @Override
                public void run() {
                    syncHistoryInfoToDB(path);
                }
            });
            executorPool.execute(thread);
        }
        executorPool.shutdown();
        try {
            executorPool.awaitTermination(SYNC_DB_TIME_OUT, TimeUnit.MILLISECONDS);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }
    /**.
    * @param historyPath String
    */
    public void syncHistoryInfoToDB(String historyPath) {
        Utils.logi(TAG, "syncHistoryInfoToDB() : historyPath = " + historyPath);
        // /data/aee_exp/db_history or /data/vendor/aee_exp/db_history
        File dbHistorySysFile = new File(historyPath);
        List<String> dbSysInfors = Utils.getLogFolderFromFileTree(dbHistorySysFile);
        if (dbSysInfors == null || dbSysInfors.size() == 0) {
            Utils.logi(TAG, "no db infors at " + historyPath);
            return;
        }
        for (int index = dbSysInfors.size() - 1;
                index >= 0 && index >= dbSysInfors.size() - DEAL_MAX_OLD_DB_NUM; index--) {
            String dbInfor = dbSysInfors.get(index);
            String[] dbStr = dbInfor.split(",");
            if (dbStr.length < 2) {
                Utils.logi(TAG, "wrong db string format, length = " + dbStr.length);
                continue;
            }
            String dbPath = dbStr[0];
            if (dbPath.endsWith(File.separator)) {
                dbPath = dbPath.substring(0, dbPath.length() - 1);
            }
            if (!new File(dbPath).exists()) {
                Utils.logi(TAG, "db file not exist, path = " + dbPath);
                continue;
            }
            String dbFolderPath = dbPath + File.separator;
            String zzTime = dbStr[1].trim();
            boolean taglogExist = DBManager.getInstance().isTaglogExist(dbFolderPath,
                    zzTime);
            Utils.logd(TAG, "taglogExist ? " + taglogExist + ", for " + dbFolderPath
                    + "," + zzTime);
            if (taglogExist) {
                break;
            }
            String dbFileName = dbPath.substring(dbPath.lastIndexOf(File.separator) + 1)
                    + ".dbg";
            String zzFileName = "ZZ_INTERNAL";
            Intent intent = new Intent();
            intent.putExtra(Utils.EXTRA_KEY_EXP_PATH, dbFolderPath);
            intent.putExtra(Utils.EXTRA_KEY_EXP_NAME, dbFileName);
            intent.putExtra(Utils.EXTRA_KEY_EXP_ZZ, zzFileName);
            intent.putExtra(Utils.EXTRA_KEY_EXP_TIME, zzTime);

            Utils.logd(TAG, "request new taglog from db_history, path=" + dbFolderPath
                    + ", dbName=" + dbFileName + ", zzName=" + zzFileName);
            if (mIsLogAutoStart && Utils.isTaglogEnable()) {
                intent.putExtra(Utils.EXTRA_KEY_TAG_TYPE, TagLogUtils.TAG_EXCEPTION);
            } else {
                intent.putExtra(Utils.EXTRA_KEY_TAG_TYPE, TagLogUtils.TAG_ONLY_COPY_DB);
            }
            startNewTaglog(intent);
        }
    }
    /**
     * Judge whether need to start up DebugLoggerUI service at boot time. If none log instance was
     * set to start automatically when boot up, just remove this process to avoid confuse user
     */
    private boolean isLogWillAutoStart() {
        boolean needStart = false;
        for (Integer logType : Utils.LOG_TYPE_SET) {
            if (logType == Utils.LOG_TYPE_MET) {
                continue;
            }
            if (Utils.VALUE_START_AUTOMATIC_ON == MyApplication.getInstance()
                    .getDefaultSharedPreferences().getBoolean(
                    Utils.KEY_START_AUTOMATIC_MAP.get(logType),
                    Utils.DEFAULT_CONFIG_LOG_AUTO_START_MAP.get(logType))) {
                needStart = true;
                break;
            }
        }
        Utils.logd(TAG, "-->needStartLogAtBootTime(), needStart=" + needStart);
        return needStart;
    }
    /**
     * @author MTK11515
     *
     */
    class TaglogManagerHandler extends Handler {
        public TaglogManagerHandler(Looper looper) {
            super(looper);
        }
        @Override
        public void handleMessage(Message msg) {
            Utils.logi(TAG, "-->mTaglogManagerHandler msg.what = " + msg.what);
            switch (msg.what) {
            case TagLogUtils.MSG_UI_LOCK:
                mUILockNumber++;
                break;
            case TagLogUtils.MSG_UI_RELEASE:
                mUILockNumber--;
                break;
            case TagLogUtils.MSG_TAGLOG_DONE:
                Object obj = msg.obj;
                if (obj instanceof TagLog) {
                    mTaglogList.remove(obj);
                }
                break;
            case TagLogUtils.MSG_DO_FILEMANAGER:
                mFileManagerHandler.sendEmptyMessage(TagLogUtils.MSG_DO_FILEMANAGER);
                break;
            case TagLogUtils.MSG_TAGLOG_MANAGER_INIT:
                doInit();
                break;
            default:
                Utils.logw(TAG, "-->mTaglogManagerHandler msg.what = " + msg.what
                        + " is not supported!");
            }
        }
    }
}
