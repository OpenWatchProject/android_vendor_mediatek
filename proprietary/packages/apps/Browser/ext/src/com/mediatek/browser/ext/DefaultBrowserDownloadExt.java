package com.mediatek.browser.ext;

import android.app.Activity;
import android.app.DownloadManager.Request;
import android.net.Uri;
import android.util.Log;
import android.widget.Toast;

import java.io.File;

public class DefaultBrowserDownloadExt implements IBrowserDownloadExt {

    private static final String TAG = "DefaultBrowserDownloadExt";

    @Override
    public boolean checkStorageBeforeDownload(Activity activity,
                                              String downloadPath,
                                              long contentLength) {
        Log.i("@M_" + TAG, "Enter: " + "checkStorageBeforeDownload" + " --default implement");
        return false;
    }

    @Override
    public void showToastWithFileSize(Activity activity, long contentLength, String text) {
        Log.i("@M_" + TAG, "Enter: " + "showToastWithFileSize" + " --default implement");
        Toast.makeText(activity, text, Toast.LENGTH_SHORT).show();
    }

    @Override
    public void setRequestDestinationDir(String downloadPath, Request request,
            String filename, String mimeType) {
        Log.i("@M_" + TAG, "Enter: " + "setRequestDestinationDir" + " --default implement");

        String dir = "file://" + downloadPath + File.separator + filename;
        Uri pathUri = Uri.parse(dir);
        request.setDestinationUri(pathUri);
        Log.d("@M_" + TAG, "mRequest.setDestinationUri, dir: " + dir);
    }

}
