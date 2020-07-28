package com.mediatek.engineermode.rsc;

import com.mediatek.engineermode.Elog;

import java.util.ArrayList;


/**
 * This class describe the XML info.
 * */
public class ConfigXMLData {
    private static final String TAG = "rcs/ConfigXMLData";
    String mTarPartName;
    String mTarPartOffset;
    int mVersion;
    String mMagic;
    ArrayList<ProjectData> mProjectList;
    /**
     * This class AudioLoggerXMLData.
     */
    public ConfigXMLData() {
        mProjectList = new ArrayList<ProjectData>();
    }

    void setTarPartName(String str) {
        mTarPartName = str;
        Elog.d(TAG, "mTarPartName:" + mTarPartName);
    }

    String getTarPartName() {
        return mTarPartName;
    }
    void setTarPartOffset(String str) {
        mTarPartOffset = str;
        Elog.d(TAG, "mTarPartOffset:" + mTarPartOffset);
    }

    String getTarPartOffset() {
        return mTarPartOffset;
    }

    void addProjectName(ProjectData proj) {
        mProjectList.add(proj);
    }

    void setVersion(int version) {
        mVersion = version;
        Elog.d(TAG, "mVersion:" + mVersion);
    }

    void setMagic(String magic) {
        mMagic = magic;
        Elog.d(TAG, "mMagic:" + mMagic);
    }
    int getVersion() {
        return mVersion;
    }
    String getMagic() {
        return mMagic;
    }
    ArrayList<ProjectData> getProjectList() {
        return mProjectList;
    }

    /**
     * Class for recording project information.
     *
     */
    class ProjectData {
        private int mPrjIndex;
        private String mPrjName;
        private String mPrjOptr;
        void setIndex(int index) {
            mPrjIndex = index;
            Elog.d(TAG, "mPrjIndex:" + mPrjIndex);
        }
        void setName(String name) {
            mPrjName = name;
            Elog.d(TAG, "mPrjName:" + mPrjName);
        }
        void setOptr(String optr) {
            mPrjOptr = optr;
            Elog.d(TAG, "mPrjOptr:" + mPrjOptr);
        }
        String getName() {
            return mPrjName;
        }
        String getOptr() {
            return mPrjOptr;
        }
        int getIndex() {
            return mPrjIndex;
        }
    }
};
