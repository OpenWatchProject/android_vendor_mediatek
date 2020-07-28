package com.android.browser;

import android.os.AsyncTask;
import android.os.Handler;
import android.util.Log;
import java.util.ArrayList;
import java.util.concurrent.CopyOnWriteArrayList;

public class CheckMemoryTask extends AsyncTask<Object, Void, Void> {
    private static final String LOGTAG = "browser";
    private static final int PARAMS_LENGTH = 6;
    // handler to send message to release memory
    private Handler mHandler;

    public CheckMemoryTask(Handler handler) {
        this.mHandler = handler;
    }

    @Override
    protected Void doInBackground(Object... params) {
        if (params.length != PARAMS_LENGTH) {
            Log.d(LOGTAG, "Incorrect parameters to CheckMemoryTask doInBackground(): "
                    + String.valueOf(params.length));
        } else {
            // Numbers of visible webview
            int visibleWebviewNums = (Integer) params[0];
            // If isFreeMemory is true, it's current tab index
            // If isFreeMemory is false, it's closed tab index
            ArrayList<Integer> tabIndex = (ArrayList<Integer>) params[1];
            // If true, it is free memory
            boolean isFreeMemory = (Boolean) params[2];
            // The current webview's url
            String url = (String) params[3];
            // tabindex of all free tabs
            CopyOnWriteArrayList<Integer> freeTabIndexs = (CopyOnWriteArrayList<Integer>) params[4];
            // If true, it is close tab
            boolean isRemoveTab = (Boolean) params[5];
            boolean shouldReleaseTab = Performance.checkShouldReleaseTabs(
                    visibleWebviewNums, tabIndex,
                    isFreeMemory, url, freeTabIndexs, isRemoveTab);
            if (shouldReleaseTab) {
                if (mHandler != null && !mHandler.hasMessages(Controller.MSG_RELEASE_TABS)) {
                    mHandler.sendEmptyMessage(Controller.MSG_RELEASE_TABS);
                }
            }
        }
        return null;
    }

}
