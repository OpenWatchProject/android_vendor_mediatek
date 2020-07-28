package com.debug.loggerui.taglog;

import com.debug.loggerui.taglog.db.TaglogTable;

/**
 * @author MTK81255
 *
 */
public class TagLogData {

    private String mTaglogFolder = "";
//    private List<LogInformation> mLogInfoList = new ArrayList<LogInformation>();
    private TaglogTable mTaglogTable = null;

    /**
     * void.
     */
    public TagLogData() {
    }

    /**
     * @param taglogFolder String
     */
    public TagLogData(String taglogFolder) {
        mTaglogFolder = taglogFolder;
//        mLogInfoList = logInfoList;
    }
    /**
     * @param taglogTable TaglogTable
     */
    public TagLogData(TaglogTable taglogTable) {
        setTaglogTable(taglogTable);
    }

    public String getTaglogFolder() {
        return mTaglogFolder;
    }

    public void setTaglogFolder(String targetFolder) {
        this.mTaglogFolder = targetFolder;
    }
    /**
     * @param taglogTable TaglogTable
     */
    public void setTaglogTable(TaglogTable taglogTable) {
        this.mTaglogFolder = taglogTable.getTargetFolder();
        this.mTaglogTable = taglogTable;
    }

    public TaglogTable getTaglogTable() {
        return this.mTaglogTable;
    }
}
