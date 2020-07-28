/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.browser;

import android.Manifest;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.DownloadManager;
import android.content.ActivityNotFoundException;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.net.Uri;
import android.net.WebAddress;
import android.os.Environment;
import android.os.storage.StorageManager;
import android.os.storage.StorageVolume;
import android.text.TextUtils;
import android.util.Log;
import android.webkit.CookieManager;
import android.webkit.URLUtil;
import android.widget.Toast;

import com.mediatek.browser.ext.IBrowserDownloadExt;
import com.mediatek.storage.StorageManagerEx;

import java.io.File;
import java.net.URI;

/**
 * Handle download requests
 */
public class DownloadHandler {

    /// M: add for debug @ {
    private static final boolean DEBUG = Browser.DEBUG;
    /// @ }
    private static final boolean LOGD_ENABLED =
            com.android.browser.Browser.LOGD_ENABLED;

    private static final String LOGTAG = "DLHandler";
    private static final String XLOGTAG = "browser/DLHandler";
    private static IBrowserDownloadExt sBrowserDownloadExt = null;
    /**
     * Notify the host application a download should be done, or that
     * the data should be streamed if a streaming viewer is available.
     * @param activity Activity requesting the download.
     * @param url The full url to the content that should be downloaded
     * @param userAgent User agent of the downloading application.
     * @param contentDisposition Content-disposition http header, if present.
     * @param mimetype The mimetype of the content reported by the server
     * @param referer The referer associated with the downloaded url
     * @param privateBrowsing If the request is coming from a private browsing tab.
     */
    public static void onDownloadStart(Activity activity, String url,
            String userAgent, String contentDisposition, String mimetype,
            String referer, boolean privateBrowsing, long  contentLength) {
        onDownloadStartNoStream(activity, url, userAgent, contentDisposition,
                mimetype, referer, privateBrowsing, contentLength);
    }

    // This is to work around the fact that java.net.URI throws Exceptions
    // instead of just encoding URL's properly
    // Helper method for onDownloadStartNoStream
    private static String encodePath(String path) {
        char[] chars = path.toCharArray();

        boolean needed = false;
        for (char c : chars) {
            if (c == '[' || c == ']' || c == '|') {
                needed = true;
                break;
            }
        }
        if (needed == false) {
            return path;
        }

        StringBuilder sb = new StringBuilder("");
        for (char c : chars) {
            if (c == '[' || c == ']' || c == '|') {
                sb.append('%');
                sb.append(Integer.toHexString(c));
            } else {
                sb.append(c);
            }
        }

        return sb.toString();
    }

    /**
     * Notify the host application a download should be done, even if there
     * is a streaming viewer available for thise type.
     * @param activity Activity requesting the download.
     * @param url The full url to the content that should be downloaded
     * @param userAgent User agent of the downloading application.
     * @param contentDisposition Content-disposition http header, if present.
     * @param mimetype The mimetype of the content reported by the server
     * @param referer The referer associated with the downloaded url
     * @param privateBrowsing If the request is coming from a private browsing tab.
     */
    /*package */ public static void onDownloadStartNoStream(Activity activity,
            String url, String userAgent, String contentDisposition,
            String mimetype, String referer, boolean privateBrowsing, long contentLength) {

        String[] perm = { Manifest.permission.READ_EXTERNAL_STORAGE,
            Manifest.permission.WRITE_EXTERNAL_STORAGE };
        for (int i = 0; i < perm.length; i++) {
            if (activity.checkSelfPermission(perm[i]) != PackageManager.PERMISSION_GRANTED) {
                if (DEBUG) {
                    Log.d(XLOGTAG, "no permission: " + perm[i]);
                }
                return;
            }
        }

        /// M: ALPS02251196.Fix incorrect mimetype which surround by quotations @{
        if (null != mimetype && mimetype.startsWith("\"")
                && mimetype.endsWith("\"") && mimetype.length() > 2) {
            mimetype = mimetype.substring(1, mimetype.length() - 1);
        }
        /// @}

        String filename = URLUtil.guessFileName(url,
                contentDisposition, mimetype);
        if (DEBUG) {
            Log.d(XLOGTAG, "Guess file name is: " + filename +
                " mimetype is: " + mimetype);
        }

        // Check to see if we have an SDCard
        String status = Environment.getExternalStorageState();
        if (!status.equals(Environment.MEDIA_MOUNTED)) {
            int title;
            String msg;

            // Check to see if the SDCard is busy, same as the music app
            if (status.equals(Environment.MEDIA_SHARED)) {
                msg = activity.getString(R.string.download_sdcard_busy_dlg_msg);
                title = R.string.download_sdcard_busy_dlg_title;
            } else {
                msg = activity.getString(R.string.download_no_sdcard_dlg_msg, filename);
                title = R.string.download_no_sdcard_dlg_title;
            }

            new AlertDialog.Builder(activity)
                .setTitle(title)
                .setIconAttribute(android.R.attr.alertDialogIcon)
                .setMessage(msg)
                .setPositiveButton(R.string.ok, null)
                .show();
            return;
        }

        /// M: Check whether the download path of browser is available before begin to download. @{
        String mDownloadPath = BrowserSettings.getInstance().getDownloadPath();
        final String storagePath = "/storage/";
        if (mDownloadPath.startsWith(storagePath)) {
            // get the path like "/storage/7D3C-1517", which used to check if
            // it's sd card.
            int i = mDownloadPath.indexOf("/", storagePath.length());
            if (i > 0) {
                String rootPath = mDownloadPath.substring(0, i);
                if (DEBUG) {
                    Log.d(XLOGTAG, "rootPath = " + rootPath);
                }
                StorageManager storageManager = (StorageManager) activity
                                                .getSystemService(Context.STORAGE_SERVICE);
                StorageVolume volume = storageManager.getStorageVolume(new File(rootPath));
                if (volume == null) {
                    if (DEBUG) {
                        Log.d(XLOGTAG, "volume is null: " + mDownloadPath);
                    }
                } else {
                    if (volume.isRemovable()) {
                        // check if it can be written if it's SD Card.
                        if (!new File(rootPath).canWrite()) {
                            if (DEBUG) {
                                Log.d(XLOGTAG, "  DownloadPath " + mDownloadPath
                                    + " can't write!");
                            }
                            int mTitle = R.string.download_path_unavailable_dlg_title;
                            String mMsg = activity
                                .getString(R.string.download_path_unavailable_dlg_msg);
                            new AlertDialog.Builder(activity).setTitle(mTitle)
                                .setIcon(android.R.drawable.ic_dialog_alert)
                                .setMessage(mMsg).setPositiveButton(
                                        R.string.ok, null).show();
                            return;
                        }
                    }
                }
            }
        }
        /// @}

        ///M: Check storage in download path before download. @{
        sBrowserDownloadExt = Extensions.getDownloadPlugin(activity);
        if (sBrowserDownloadExt.checkStorageBeforeDownload(activity,
                mDownloadPath, contentLength)) {
            return;
        }
        /// @}

        // java.net.URI is a lot stricter than KURL so we have to encode some
        // extra characters. Fix for b 2538060 and b 1634719
        WebAddress webAddress;
        try {
            webAddress = new WebAddress(url);
            webAddress.setPath(encodePath(webAddress.getPath()));
        } catch (Exception e) {
            // This only happens for very bad urls, we want to chatch the
            // exception here
            if (DEBUG) {
                Log.e(LOGTAG, "Exception trying to parse url:" + url);
            }
            return;
        }

        String addressString = webAddress.toString();
        Uri uri = Uri.parse(addressString);
        final DownloadManager.Request request;
        try {
            request = new DownloadManager.Request(uri);
        } catch (IllegalArgumentException e) {
            Toast.makeText(activity, R.string.cannot_download, Toast.LENGTH_SHORT).show();
            return;
        }
        request.setMimeType(mimetype);

        try {
            /// M: Operator Feature set RequestDestinationDir @{
            sBrowserDownloadExt.setRequestDestinationDir(
                BrowserSettings.getInstance().getDownloadPath(), request, filename, mimetype);
            /// @}
        } catch (IllegalStateException ex) {
            // This only happens when directory Downloads can't be created or it isn't a directory
            // this is most commonly due to temporary problems with sdcard so show appropriate
            // string.
            Log.w(LOGTAG, "Exception trying to create Download dir:", ex);
            Toast.makeText(activity, R.string.download_sdcard_busy_dlg_title,
                    Toast.LENGTH_SHORT).show();
            return;
        }

        // let this downloaded file be scanned by MediaScanner - so that it can
        // show up in Gallery app, for example.
        request.allowScanningByMediaScanner();
        request.setDescription(webAddress.getHost());
        // XXX: Have to use the old url since the cookies were stored using the
        // old percent-encoded url.
        String cookies = CookieManager.getInstance().getCookie(url, privateBrowsing);
        request.addRequestHeader("cookie", cookies);
        request.addRequestHeader("User-Agent", userAgent);
        request.addRequestHeader("Referer", referer);
        request.setNotificationVisibility(
                DownloadManager.Request.VISIBILITY_VISIBLE_NOTIFY_COMPLETED);
        //request.setUserAgent(userAgent);
        if (mimetype == null) {
            if (TextUtils.isEmpty(addressString)) {
                return;
            }

            /// M: fix BUG: ALPS00256340 @{
            try {
                URI.create(addressString);
            } catch (IllegalArgumentException e) {
                Toast.makeText(activity, R.string.cannot_download, Toast.LENGTH_SHORT).show();
                return;
            }
            /// @}

            // We must have long pressed on a link or image to download it. We
            // are not sure of the mimetype in this case, so do a head request
            new FetchUrlMimeType(activity, request, addressString, cookies,
                    userAgent).start();
        } else {
            final DownloadManager manager
                    = (DownloadManager) activity.getSystemService(Context.DOWNLOAD_SERVICE);
            new Thread("Browser download") {
                public void run() {
                    manager.enqueue(request);
                }
            }.start();
        }

        /// M: Show toast with file size. @{
        sBrowserDownloadExt.showToastWithFileSize(activity, contentLength,
            activity.getResources().getString(R.string.download_pending));
        /// @}

        /// M: Add to start Download activity. @{
        Intent pageView = new Intent(DownloadManager.ACTION_VIEW_DOWNLOADS);
        pageView.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TASK);
        activity.startActivity(pageView);
        /// @}
    }

    /**
     * M: Notify the user download the content or open
     * the content. Add this to support Operator customization
     * @param intent The Intent.ACTION_VIEW intent
     * @param url The full url to the content that should be downloaded
     * @param contentDisposition Content-disposition http header, if
     *                           present.
     * @param mimetype The mimetype of the content reported by the server
     * @param contentLength The file size reported by the server
     */
    public static void showDownloadOrOpenContent(final Activity activity, final Intent intent,
            final String url, final String userAgent,
            final String contentDisposition, final String mimetype,
            // final boolean privateBrowsing) {
            final boolean privateBrowsing, final long contentLength) {
        new AlertDialog.Builder(activity)
            .setTitle(R.string.application_name)
            .setIcon(android.R.drawable.ic_dialog_info)
            .setMessage(R.string.download_or_open_content)
            .setPositiveButton(R.string.save_content,
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog,
                                int whichButton) {
                            onDownloadStartNoStream(activity, url, userAgent,
                                    // contentDisposition, mimetype, privateBrowsing);
                                    contentDisposition, mimetype, null,
                                    privateBrowsing, contentLength);
                            Log.d(XLOGTAG, "User decide to download the content");
                            return;
                        }
                    })
            .setNegativeButton(R.string.open_content,
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog,
                                int whichButton) {
                            int nFlags = intent.getFlags();
                            nFlags &= (~Intent.FLAG_ACTIVITY_NEW_TASK);
                            intent.setFlags(nFlags);
                            if (url != null) {
                                String urlCookie = CookieManager.getInstance().getCookie(url);
                                if (DEBUG) {
                                    Log.i(XLOGTAG, "url: " + url + " url cookie: " + urlCookie);
                                }
                                if (urlCookie != null) {
                                    intent.putExtra("url-cookie", urlCookie);
                                }
                            }
                            activity.startActivity(intent);
                            Log.d(XLOGTAG, "User decide to open the content by startActivity");
                            return;
                        } })
            .setOnCancelListener(
                    new DialogInterface.OnCancelListener() {
                        public void onCancel(DialogInterface dialog) {
                            Log.d(XLOGTAG, "User cancel the download action");
                            return;
                        }
                    })
            .show();
    }
}
