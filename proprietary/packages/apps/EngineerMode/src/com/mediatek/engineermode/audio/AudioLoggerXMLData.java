package com.mediatek.engineermode.audio;

import java.util.ArrayList;

/**
 * @author MTK09919.
 *
 */
class DumpOptions {
    String mCategoryTitle;
    ArrayList<String> mType;
    ArrayList<String> mCmd;
    ArrayList<String> mCheck;
    ArrayList<String> mUncheck;
    ArrayList<String> mCmdName;

    DumpOptions() {
        mType = new ArrayList<String>();
        mCmd = new ArrayList<String>();
        mCheck = new ArrayList<String>();
        mUncheck = new ArrayList<String>();
        mCmdName = new ArrayList<String>();
    }

}

/**
 * This class describe the XML info.
 * */
public class AudioLoggerXMLData {
    ArrayList<DumpOptions> mAudioDumpOperation;

    ArrayList<String> mAudioCommandSetOperation;
    ArrayList<String> mAudioCommandGetOperation;

    ArrayList<String> mParametersSetOperationItems;
    ArrayList<String> mParametersGetOperationItems;

    /**
     * This class AudioLoggerXMLData.
     */
    public AudioLoggerXMLData() {
        mAudioDumpOperation = new ArrayList<DumpOptions>();

        mAudioCommandSetOperation = new ArrayList<String>();
        mAudioCommandGetOperation = new ArrayList<String>();
        mParametersSetOperationItems = new ArrayList<String>();
        mParametersGetOperationItems = new ArrayList<String>();
    }
    /**
     * This class setAudioCommandSetOperation.
     * @param operation
     *            operation
     */
    public void setAudioCommandSetOperation(String operation) {
        mAudioCommandSetOperation.add(operation);
    }

    /**
     * This class setAudioCommandGetOperation.
     * @param operation
     *            operation
     */
    public void setAudioCommandGetOperation(String operation) {
        mAudioCommandGetOperation.add(operation);
    }

    /**
     * This class setParametersSetOperation.
     * @param operation
     *            operation
     */
    public void setParametersSetOperation(String operation) {
        mParametersSetOperationItems.add(operation);
    }

    /**
     * This class setParametersGetOperation.
     * @param operation
     *            operation
     */
    public void setParametersGetOperation(String operation) {
        mParametersGetOperationItems.add(operation);
    }
};