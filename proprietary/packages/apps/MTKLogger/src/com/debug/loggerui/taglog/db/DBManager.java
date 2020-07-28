package com.debug.loggerui.taglog.db;

import android.content.ContentValues;
import android.content.Intent;

import com.debug.loggerui.taglog.LogInformation;
import com.debug.loggerui.taglog.TagLogData;
import com.debug.loggerui.taglog.TagLogInformation;
import com.debug.loggerui.utils.Utils;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

/**
 * @author MTK81255
 *
 */
public class DBManager {
    private static final String TAG = Utils.TAG + "/DBManager";

    private static MySQLiteHelper sDataBaseHelper;
    private static DBManager sInstance = new DBManager();

    private DBManager() {
    }
    /**
     * @return DBManager
     */
    public static DBManager getInstance() {
        return sInstance;
    }
    /**.
     * init
     */
    public void init() {
        sDataBaseHelper = MySQLiteHelper.getInstance();
    }
    /**
     * @param listLogInformation List<LogInformation>
     * @return String
     */
    public String insertFileInfoToDb(List<LogInformation> listLogInformation) {
        List<FileInfoTable> fileinfoList = new ArrayList<FileInfoTable>();
        for (LogInformation loginformation : listLogInformation) {
            fileinfoList.add(loginformation.getFileInfo());
        }
        return sDataBaseHelper.insertFileInfo(fileinfoList);
    }
    /**
     * @param taglogInfo TagLogInformation
     * @return long
     */
    public long updateTaglogToDb(TagLogInformation taglogInfo) {
        TaglogTable taglogtable = translateToTaglogTable(taglogInfo);

        String selectionStr = MySQLiteHelper.COLUMN_DB_PATH + " = '"
                + taglogInfo.getExpPath() + "'" + " and "
                + MySQLiteHelper.COLUMN_ZZINTERNAL_TIME + " = '"
                + taglogInfo.getZzInternalTime() + "'";
        List<TaglogTable> result = MySQLiteHelper.getInstance()
                .queryTaglogTable(selectionStr);
        if (result == null || result.size() == 0) {
            List<TaglogTable> listTag = new ArrayList<TaglogTable>();
            listTag.add(taglogtable);
            Utils.logw(TAG,
                    "not find taglog id, insert as new. for selectionStr = "
                            + selectionStr);
            return sDataBaseHelper.insertTaglogInfo(listTag);
        }
        int tagid = result.get(0).getTagLogId();
        taglogtable.setTagLogId(tagid);
        sDataBaseHelper.updateTagLogTable(taglogtable);
        return tagid;
    }

    /**
     * @param taglogInfos
     *            TaglogTable
     * @return long
     */
    public long insertTaglogToDb(List<TaglogTable> taglogInfos) {
        return sDataBaseHelper.insertTaglogInfo(taglogInfos);
    }

    /**
     * @param taglogInfos
     *            TaglogTable
     * @return long
     */
    public long insertTaglogToDb(TaglogTable taglogInfos) {
        List<TaglogTable> taglist = new ArrayList<TaglogTable>();
        taglist.add(taglogInfos);
        return sDataBaseHelper.insertTaglogInfo(taglist);
    }
    /**
     * @param taglogId long
     * @param taglogFolder String
     */
    public void updateTaglogFolder(long taglogId, String taglogFolder) {
        ContentValues contentValues = new ContentValues();
        contentValues.put(MySQLiteHelper.COLUMN_TARGET_FOLDER, taglogFolder);
        sDataBaseHelper.updateRecordsByTable(MySQLiteHelper.TAGLOG_TABLE,
                         MySQLiteHelper.COLUMN_ID, taglogId, contentValues);
    }
    /**
     * @param taglogId long
     * @param newState String
     */
    public void updateTaglogState(long taglogId, String newState) {
        Utils.logd(TAG, "updateTaglogState: taglogid = " + taglogId + ", new state = "
                + newState);
        if (!newState.equals(MySQLiteHelper.TAGLOG_STATE_INIT)
                && !newState.equals(MySQLiteHelper.TAGLOG_STATE_DOTAG)
                && !newState.equals(MySQLiteHelper.TAGLOG_STATE_DONE)) {
            Utils.logw(TAG, "input error taglog state : " + newState);
            return;
        }
        ContentValues contentValues = new ContentValues();
        contentValues.put(MySQLiteHelper.COLUMN_STATE, newState);

        sDataBaseHelper.updateRecordsByTable(MySQLiteHelper.TAGLOG_TABLE,
                         MySQLiteHelper.COLUMN_ID, taglogId, contentValues);
    }
    /**
     * @param taglogId long
     * @param fileListStr String
     */
    public void updateTaglogFileList(long taglogId, String fileListStr) {
        Utils.logd(TAG, "updateTaglogState: taglogid = " + taglogId
                + ", fileListStr = " + fileListStr);
        ContentValues contentValues = new ContentValues();
        contentValues.put(MySQLiteHelper.COLUMN_FILE_LIST, fileListStr);
        contentValues.put(MySQLiteHelper.COLUMN_STATE, MySQLiteHelper.TAGLOG_STATE_DOTAG);

        sDataBaseHelper.updateRecordsByTable(MySQLiteHelper.TAGLOG_TABLE,
                             MySQLiteHelper.COLUMN_ID, taglogId, contentValues);
    }
    /**
     * @param fileid long
     * @param newState String
     * @param newFileProgress int
     */
    public void updateFileInfo(long fileid, String newState, int newFileProgress) {
        ContentValues contentValues = new ContentValues();
        if (MySQLiteHelper.FILEINFO_STATE_WAITING.equals(newState)
                || MySQLiteHelper.FILEINFO_STATE_DOING.equals(newState)
                || MySQLiteHelper.FILEINFO_STATE_DONE.equals(newState)) {
            contentValues.put(MySQLiteHelper.COLUMN_STATE, newState);
        }
        contentValues.put(MySQLiteHelper.COLUMN_FILE_PROGRESS, newFileProgress);
        Utils.logd(TAG, "updateFileProgressAndState: fileid = " + fileid +
                   ", newState = " + newState + ", newFileProgress = " + newFileProgress);

        sDataBaseHelper.updateRecordsByTable(MySQLiteHelper.FILEINFO_TABLE,
                                    MySQLiteHelper.COLUMN_ID, fileid, contentValues);
    }
    /**
     * @return List<LogInformation>
     */
    public List<LogInformation> getWaitingDoingLogInformationList() {
        String selectStr = MySQLiteHelper.COLUMN_STATE + " in ('"
                           + MySQLiteHelper.FILEINFO_STATE_WAITING + "', '"
                           + MySQLiteHelper.FILEINFO_STATE_DOING + "')";

        List<FileInfoTable> fileInfoList = sDataBaseHelper.queryFileInfoTable(selectStr);
        if (fileInfoList == null || fileInfoList.size() < 1) {
            return null;
        }
        List<LogInformation> logInfoList = new ArrayList<LogInformation>();
        for (FileInfoTable fileInfo : fileInfoList) {
            logInfoList.add(new LogInformation(fileInfo));
        }
        return logInfoList;
    }
    /**
     * @param fileId long
     * @return FileInfoTable
     */
    public FileInfoTable getFileInfoById(long fileId) {
        FileInfoTable result = null;
        String selectStr = MySQLiteHelper.COLUMN_ID + "=" + fileId;
        List<FileInfoTable> listResult = sDataBaseHelper.queryFileInfoTable(selectStr);
        if (listResult != null && listResult.size() > 0) {
            result = listResult.get(0);
            Utils.logd(TAG, "find file fileId = " + fileId + ", by selectstr = " + selectStr);
        }
        return result;
    }
    /**
     * @param originalPath String
     * @return FileInfoTable
     */
    public FileInfoTable getFileInfoByOriginalPath(String originalPath) {
        FileInfoTable result = null;
        if (originalPath == null || originalPath.isEmpty()) {
            return result;
        }
        String selectStr = MySQLiteHelper.COLUMN_ORIGINAL_PATH
                           + " = '" + originalPath + "'" ;
        List<FileInfoTable> listResult = sDataBaseHelper.queryFileInfoTable(selectStr);
        if (listResult != null && listResult.size() > 0) {
            result = listResult.get(0);
            Utils.logd(TAG, "find file fileId = " + result.getFileId()
                       + ", by selectstr = " + selectStr);
        }
        return result;
    }
    /**
     * @return List<TagLogData>
     */
    public List<TagLogData> getResumeTaglog() {
        String selectStr = MySQLiteHelper.COLUMN_STATE + " in ('"
                + MySQLiteHelper.TAGLOG_STATE_INIT + "', '"
                + MySQLiteHelper.TAGLOG_STATE_DOTAG + "')";

        List<TaglogTable> listTaglogTable = sDataBaseHelper.queryTaglogTable(selectStr);
        if (listTaglogTable == null || listTaglogTable.size() < 1) {
            Utils.logw(TAG, "getNotCompleteTaglogList = null");
            return null;
        }

        List<TagLogData> listTaglogData = new ArrayList<TagLogData>();
        for (TaglogTable taglogtable : listTaglogTable) {
            listTaglogData.add(new TagLogData(taglogtable));
        }
        Utils.logd(TAG, "getTaglogFromDb, listTaglogData.size() = " + listTaglogData.size());
        return listTaglogData;
    }

    /**
     * @param path String
     * @return List<Intent>
     */
    public List<Intent> getRequestNewTaglog(String path) {
        if (!Utils.isTaglogEnable()) {
            Utils.logi(TAG, "isTaglogEnable = false, not need RequestNewTaglog");
            return null;
        }
        File dbHistoryFile = new File(path);
        Utils.logi(TAG, path + "is exist ?= " + dbHistoryFile.exists());

        List<String> dbInfors = Utils.getLogFolderFromFileTree(dbHistoryFile);
        if (dbInfors == null || dbInfors.size() == 0) {
            Utils.logi(TAG, "no db infors in db_history");
            return null;
        }
        List<Intent> intentList = new ArrayList<Intent>();

        int maxDbSize = 20; // No need deal with too old db history
        for (int index = dbInfors.size() - 1;
                index >= 0 && index >= dbInfors.size() - maxDbSize; index--) {
            String dbInfor = dbInfors.get(index);
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
                continue;
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
            intentList.add(intent);
            if (intentList.size() >= 3) {
                break;
            }
        }
        return intentList;
    }
    /**
     * @param taglogData TagLogData
     */
    public void deleteFromDb(TagLogData taglogData) {

    }

    /**
     * @param data TagLogData
     */
    public void updateDb(TagLogData data) {

    }
    /**
     * @param fileIds String
     */
    public void changeFileStateToWaiting(String fileIds) {
        Utils.logd(TAG, "typelogStart: fileIds = " + fileIds);
        ContentValues contentValues = new ContentValues();
        contentValues.put(MySQLiteHelper.COLUMN_STATE, MySQLiteHelper.FILEINFO_STATE_WAITING);

        String selectionStr = MySQLiteHelper.COLUMN_STATE + " = '"
                             + MySQLiteHelper.FILEINFO_STATE_PREPARE + "'";
        List<String> listPrepareIds = sDataBaseHelper.queryFileIds(selectionStr);

        String[] keyIds = fileIds.split(",");
        for (String key : keyIds) {
            if (listPrepareIds.contains(key)) {
                sDataBaseHelper.updateRecordsByTable(MySQLiteHelper.FILEINFO_TABLE,
                        MySQLiteHelper.COLUMN_ID, Long.valueOf(key), contentValues);
            }
        }
    }

    /**
     * @param logInfo
     *            LogInformation
     * @param status
     *            String
     * @param fileCount
     *            int
     */
    public void updateLogInforState(LogInformation logInfo, String status, int fileCount) {
        FileInfoTable fileInfo = logInfo.getFileInfo();
        if (fileInfo == null) {
            Utils.logw(TAG, "typelogDone, just return! for fileInfo = null");
            return;
        }
        DBManager.getInstance().updateFileInfo(fileInfo.getFileId(), status, fileCount);
    }

    private TaglogTable translateToTaglogTable(TagLogInformation taglogInfo) {
        TaglogTable taglogtable = new TaglogTable();
        taglogtable.setTargetFolder(taglogInfo.getTaglogTargetFolder());
        taglogtable.setState(MySQLiteHelper.TAGLOG_STATE_INIT);
        taglogtable.setFileList("");
        taglogtable.setDBPath(taglogInfo.getExpPath());

        taglogtable.setDBFileName(taglogInfo.getDbFileName());
        taglogtable.setDBZZFileName(taglogInfo.getZzFilename());
        taglogtable.setIsNeedZip(taglogInfo.isNeedZip() ? "1" : "0");
        taglogtable.setIsNeedAllLogs(taglogInfo.isNeedAllLogs() ? "1" : "0");

        taglogtable.setNeedLogType(taglogInfo.getNeedLogType());
        taglogtable.setReason(taglogInfo.getReason());
        taglogtable.setFromWhere(taglogInfo.getFromWhere());
        taglogtable.setZzInternalTime(taglogInfo.getZzInternalTime());
        return taglogtable;
    }
    /**
     * @param fileName String
     * @return boolean
     */
    public boolean isFileInDependence(String fileName) {
        //state in ('WAITTING', 'DOING')
        String selectStr = MySQLiteHelper.COLUMN_STATE + " in ('"
                + MySQLiteHelper.FILEINFO_STATE_DOING + "')";
        boolean result = true;

        List<FileInfoTable> fileInfoList = sDataBaseHelper.queryFileInfoTable(selectStr);
        if (fileInfoList == null || fileInfoList.size() < 1) {
            Utils.logi(TAG, "isFileInDependence = true, for no other taglog dependent it.");
            return result;
        }
        for (FileInfoTable fileInfo : fileInfoList) {
            if (fileInfo.getSourcePath().startsWith(fileName)) {
                result = false;
                break;
            }
        }
        Utils.logi(TAG, "File = " + fileName + ", isFileInDependence ? " + result);
        return result;
    }
    /**
     * @param mFileListStr String
     * @return List<LogInformation>
     */
    public List<LogInformation> getFileInfoByIds(String mFileListStr) {
        // TODO Auto-generated method stub
        String[] ids = mFileListStr.split(",");
        String selectStr = MySQLiteHelper.COLUMN_ID + " in (";
        for (String id : ids) {
            selectStr += "'" + id + "',";
        }
        selectStr = selectStr.substring(0, selectStr.length() - 1) + ")";
        List<FileInfoTable> fileInfoList = sDataBaseHelper.queryFileInfoTable(selectStr);
        if (fileInfoList == null || fileInfoList.size() < 1) {
            return null;
        }
        List<LogInformation> logInfoList = new ArrayList<LogInformation>();
        for (FileInfoTable fileInfo : fileInfoList) {
            logInfoList.add(new LogInformation(fileInfo));
        }
        return logInfoList;

    }

    /**
     * @param dbPath String
     * @param zzInternalTime String
     * @return boolean
     */
    public boolean isTaglogExist(String dbPath, String zzInternalTime) {
        String selectionStr =
                MySQLiteHelper.COLUMN_DB_PATH + " = '" + dbPath + "'"
                + " and "
                + MySQLiteHelper.COLUMN_ZZINTERNAL_TIME + " = '" + zzInternalTime + "'";
        List<TaglogTable> result = MySQLiteHelper.getInstance().queryTaglogTable(selectionStr);
        return result.size() > 0;
    }
}
