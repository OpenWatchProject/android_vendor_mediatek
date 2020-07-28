/**
 *
 */
package com.mediatek.mtklogger.c2klogger;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.LinkedList;
import java.util.List;

/**
 * @author MTK81255
 *
 */
public class C2KLogRecycleConfig {

    private List<String> mContents = new LinkedList<String>();

    private String mRecycleConfig = "";

    private boolean mIsChanged = false;

    /**
     * @param recycleConfig String
     */
    public C2KLogRecycleConfig(String recycleConfig) {
        this.mRecycleConfig = recycleConfig;
        mIsChanged = true;
        readContents();
    }

    /**
     * @param logPath String
     */
    public void addLogpathToLastLine(String logPath) {
        mIsChanged = true;
        synchronized (mContents) {
            mContents.add(logPath);
        }
    }

    /**
     */
    public void removeLogpathFromFirstLine() {
        mIsChanged = true;
        synchronized (mContents) {
            if (mContents.size() > 0) {
                mContents.remove(0);
            }
        }
    }

    /**
     * @return String
     */
    public String getLogpathFromFirstLine() {
        synchronized (mContents) {
            return mContents.size() > 0 ? mContents.get(0) : "";
        }
    }

    /**
     * @return List<String>
     */
    public List<String> getContents() {
        synchronized (mContents) {
            return mContents;
        }
    }

    /**
     */
    public void readContents() {
        synchronized (mContents) {
            try {
                if (!mIsChanged) {
                    return;
                }
                mIsChanged = false;
                mContents.clear();
                FileReader fr = new FileReader(new File(mRecycleConfig));
                BufferedReader br = new BufferedReader(fr);
                String line = null;
                while ((line = br.readLine()) != null) {
                    mContents.add(line);
                }
                br.close();
                fr.close();
            } catch (FileNotFoundException e) {
                e.printStackTrace();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    /**
     */
    public void writeContents() {
        synchronized (mContents) {
            if (!mIsChanged) {
                return;
            }
            mIsChanged = false;
            File recycleConfigFile = new File(mRecycleConfig);
            if (recycleConfigFile.exists()) {
                recycleConfigFile.delete();
            }
            try {
                FileWriter fw = new FileWriter(recycleConfigFile);
                BufferedWriter bw = new BufferedWriter(fw);
                for (String line : mContents) {
                    bw.write(line + "\r\n");
                }
                bw.close();
                fw.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

}
