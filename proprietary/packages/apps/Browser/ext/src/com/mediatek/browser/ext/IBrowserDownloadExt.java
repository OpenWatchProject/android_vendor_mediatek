package com.mediatek.browser.ext;

import android.app.Activity;
import android.app.DownloadManager.Request;

public interface IBrowserDownloadExt {

    /**
     * Check the storage free space before download file
     * @param activity the activity
     * @param activity the download path
     * @param contentLength the download file content length
     * @return whether the storage free space is enough or not
     */
    boolean checkStorageBeforeDownload(Activity activity, String downloadPath, long contentLength);

    /**
     * Show the download toast with file size.
     * @param activity the activity
     * @param contentLength the download file content length
     * @param text toast information
     */
    void showToastWithFileSize(Activity activity, long contentLength, String text);

    /**
     * Set the HTTP request destination direction
     * @param downloadPath the download path
     * @param request the download request
     * @param filename the download filename
     * @param mimeType the download file mime type
     */
    void setRequestDestinationDir(String downloadPath, Request request,
            String filename, String mimeType);

}