package com.debug.loggerui.utils;

import android.os.FileObserver;

import java.io.File;

/**
 * @author MTK81255
 *
 */
public class FileMonitor extends FileObserver {
    private static final String TAG = Utils.TAG + "/FileMonitor";

    String mFileName = "";
    /**
     * @param path String
     */
    public FileMonitor(String path) {
        this(path, FileObserver.ALL_EVENTS);
    }

    /**
     * @param path String
     * @param event int
     */
    public FileMonitor(String path, int event) {
        super(new File(path).getParent(), event);
        mFileName = new File(path).getName();
    }

    @Override
    public void onEvent(int event, String path) {
        Utils.logi(TAG, "onEvent(). event = " + event +  ", path = " + path);
        switch (event) {
        case FileObserver.MODIFY:
            if (mFileName.equalsIgnoreCase(path)) {
                notifyModified();
            }
            break;
        default:
            break;
        }
    }

    protected void notifyModified() {
    }
}
