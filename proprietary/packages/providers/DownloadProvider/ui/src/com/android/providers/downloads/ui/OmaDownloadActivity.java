/*
 * Copyright (C) 2013 The Android Open Source Project
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

package com.android.providers.downloads.ui;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.DownloadManager;
import android.app.NotificationManager;
import android.content.Context;
import android.content.ContentValues;
import android.content.ContentUris;
import android.content.DialogInterface;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.net.ParseException;
import android.net.WebAddress;
import android.os.Bundle;
import android.provider.Downloads;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.WindowManager;
import android.widget.EditText;

import com.mediatek.mtkdownloadmanager.ext.IDownloadProviderFeatureExt;

import com.mediatek.omadrm.OmaDrmStore;

import com.android.providers.downloads.Constants;
import com.android.providers.downloads.MtkDownloads;
import com.android.providers.downloads.PluginFactory;


import java.util.Queue;
import java.util.LinkedList;

/**
 * DownloadList have not use in kk, so add this class to support oma dl.
 */
public class OmaDownloadActivity extends Activity {

    private static final String LOG_OMA_DL = "DownloadManager/OMA";
    private static final String XTAG_DRM = "DownloadManager/DRM";
    private static final String XTAG_ENHANCE = "DownloadManager/Enhance";

    private static IDownloadProviderFeatureExt sDownloadProviderFeatureExt;
    private DownloadManager mDownloadManager;

    /// M: These variable is used to store COLUMN_ID when download dd file @{
    private AlertDialog mDialog;
    private AlertDialog mCurrentDialog;
    private Queue<AlertDialog> mDownloadsToShow = new LinkedList<AlertDialog>();
    /// @}

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.d("@M_" + LOG_OMA_DL, "OmaDownloadActivity:onCreate");
        super.onCreate(savedInstanceState);
        mDownloadManager = (DownloadManager) getSystemService(Context.DOWNLOAD_SERVICE);
        mDownloadManager.setAccessAllDownloads(true);
    }

    @Override
    protected void onNewIntent(Intent intent) {
        Log.d("@M_" + LOG_OMA_DL, "OmaDownloadActivity:onNewIntent, intent: " + intent);
        super.onNewIntent(intent);
        setIntent(intent);
    }

    @Override
    protected void onResume() {
        Log.d("@M_" + LOG_OMA_DL, "OmaDownloadActivity:onResume");
        super.onResume();
        Intent intent = getIntent();
        Log.d("@M_" + LOG_OMA_DL, "getIntent(), intent: " + intent);
        if (intent != null) {
            String action = intent.getAction();
            if (action.equals(Constants.ACTION_OMA_DL_DIALOG)) {
                setIntent(null);
                Log.d("@M_" + LOG_OMA_DL, "handleOmaDownload()");
                handleOmaDownload();
            }
        }

        if (mCurrentDialog != null) {
            if (!mCurrentDialog.isShowing()) {
                Log.d("@M_" + LOG_OMA_DL, "OmaDownloadActivity, mCurrentDialog is not showing");
                mCurrentDialog.show();
                return;
            }
            Log.d("@M_" + LOG_OMA_DL, "OmaDownloadActivity, mCurrentDialog is showing");
        } else {
            finish();
            Log.d("@M_" + LOG_OMA_DL, "OmaDownloadActivity will finish");
        }
    }

    @Override
    protected void onDestroy() {
        Log.d("@M_" + XTAG_ENHANCE, "OmaDownloadActivity:onDestroy");
        super.onDestroy();
    }


    /**
     *  M: Add this to handle MTK DRM
     */
    private void handleDRMRight() {
    //ANuj

        String selection = "(" + Downloads.Impl.COLUMN_MIME_TYPE + " == "
                + OmaDrmStore.DrmObjectMimeType.MIME_TYPE_RIGHTS_WBXML + ") OR ("
                + Downloads.Impl.COLUMN_MIME_TYPE + " == "
                + OmaDrmStore.DrmObjectMimeType.MIME_TYPE_RIGHTS_XML
                + ")";
        Cursor cursor = null;
        try {
            cursor = getContentResolver().query(
                    Downloads.Impl.ALL_DOWNLOADS_CONTENT_URI,
                    new String[] {
                            Downloads.Impl._ID, Downloads.Impl.COLUMN_STATUS
                    },
                    "mimetype = ? OR mimetype = ?",
                    new String[] {
                            OmaDrmStore.DrmObjectMimeType.MIME_TYPE_RIGHTS_WBXML,
                            OmaDrmStore.DrmObjectMimeType.MIME_TYPE_RIGHTS_XML
                    }, null);
            Log.i("@M_" + XTAG_DRM, "handleDRMRight: before query");
            if (cursor != null) {
                for (cursor.moveToFirst(); !cursor.isAfterLast(); cursor.moveToNext()) {
                    Log.v("@M_" + XTAG_DRM, "handleDRMRight: cursor is not null");
                    // Has DRM rights need to delete
                    long downloadID = cursor.getLong(cursor
                            .getColumnIndexOrThrow(Downloads.Impl._ID));
                    int downloadStatus = cursor.getInt(cursor
                            .getColumnIndexOrThrow(Downloads.Impl.COLUMN_STATUS));
                    if (Downloads.Impl.isStatusCompleted(downloadStatus)) {
                        Log.v("@M_" + XTAG_DRM,
                         "handleDRMRight: DRM right is complete and need delete");
                        deleteDownload(downloadID);
                    }
                }
            }
        } catch (IllegalStateException e) {
            Log.e("@M_" + XTAG_DRM, "handleDRMRight: query encounter exception");
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
    }



    /**
     *  M: This class is contain the info of OMA DL
     */
    private class DownloadInfo {
        public String mName;
        public String mVendor;
        public String mType;
        public String mObjectUrl;
        public String mNextUrl;
        public String mInstallNotifyUrl;
        public String mDescription;
        public boolean mSupportByDevice;
        public int mSize;


        DownloadInfo(String name, String vendor, String type, String objectUrl,
                String nextUrl, String installNotifyUrl, String description,
                int size, boolean isSupportByDevice) {
            mName = name;
            mVendor = vendor;
            mType = type;
            mObjectUrl = objectUrl;
            mNextUrl = nextUrl;
            mInstallNotifyUrl = installNotifyUrl;
            mDescription = description;
            mSize = size;
            mSupportByDevice = isSupportByDevice;
        }
    }

    /**
     * M: This function is used to handle OMA DL. Include .dd file and download MediaObject
     */
    private void handleOmaDownload() {
        String whereClause = null;
        if (MtkDownloads.Impl.OMA_DOWNLOAD_SUPPORT) {
            whereClause =  "(" + Downloads.Impl.COLUMN_STATUS +
             " == '" + MtkDownloads.Impl.STATUS_NEED_HTTP_AUTH + "') OR (" +
                            Downloads.Impl.COLUMN_STATUS + " == '" +
                            MtkDownloads.Impl.OMADL_STATUS_DOWNLOAD_COMPLETELY + "' AND " +
                            Downloads.Impl.COLUMN_VISIBILITY +
                             " != '" + Downloads.Impl.VISIBILITY_HIDDEN + "'" + " AND " +
                            Downloads.Impl.COLUMN_DELETED + " != '1' AND " +
                            MtkDownloads.Impl.COLUMN_OMA_DOWNLOAD_FLAG +
                             " == '" + "1' AND (" + // '1' means it is OMA DL
                            MtkDownloads.Impl.COLUMN_OMA_DOWNLOAD_STATUS + " == '" +
                            MtkDownloads.Impl.OMADL_STATUS_PARSE_DDFILE_SUCCESS + "' OR " +
                            MtkDownloads.Impl.COLUMN_OMA_DOWNLOAD_STATUS + " == '" +
                            MtkDownloads.Impl.OMADL_STATUS_HAS_NEXT_URL + "'))";
            // Note: OMA_Download_Status '201': .dd file download and parsed success.
            // OMA_Download_Status '203':
            // Download OMA Download media object success and it has next url.
        } else {
            whereClause = Downloads.Impl.COLUMN_STATUS +
             " == '" + MtkDownloads.Impl.STATUS_NEED_HTTP_AUTH + "'";
        }

        Cursor cursor = null;

        try {
            if (MtkDownloads.Impl.OMA_DOWNLOAD_SUPPORT) {
            cursor = getContentResolver().query(Downloads.Impl.ALL_DOWNLOADS_CONTENT_URI,
                    new String[] { Downloads.Impl._ID,
                    Downloads.Impl.COLUMN_STATUS,
                    MtkDownloads.Impl.COLUMN_OMA_DOWNLOAD_DD_FILE_INFO_NAME,
                    MtkDownloads.Impl.COLUMN_OMA_DOWNLOAD_DD_FILE_INFO_VENDOR,
                    MtkDownloads.Impl.COLUMN_OMA_DOWNLOAD_DD_FILE_INFO_SIZE,
                    MtkDownloads.Impl.COLUMN_OMA_DOWNLOAD_DD_FILE_INFO_TYPE,
                    MtkDownloads.Impl.COLUMN_OMA_DOWNLOAD_DD_FILE_INFO_DESCRIPTION,
                    MtkDownloads.Impl.COLUMN_OMA_DOWNLOAD_FLAG,
                    MtkDownloads.Impl.COLUMN_OMA_DOWNLOAD_STATUS,
                    MtkDownloads.Impl.COLUMN_OMA_DOWNLOAD_OBJECT_URL,
                    MtkDownloads.Impl.COLUMN_OMA_DOWNLOAD_NEXT_URL,
                    MtkDownloads.Impl.COLUMN_OMA_DOWNLOAD_INSTALL_NOTIFY_URL}, whereClause, null,
                    Downloads.Impl.COLUMN_LAST_MODIFICATION + " DESC");
            } else {
                cursor = getContentResolver().query(Downloads.Impl.ALL_DOWNLOADS_CONTENT_URI,
                        new String[] { Downloads.Impl._ID,
                        Downloads.Impl.COLUMN_STATUS}, whereClause, null,
                        Downloads.Impl.COLUMN_LAST_MODIFICATION + " DESC");
            }
            if (cursor != null) {
                showAlertDialog(cursor);
            }

        } catch (IllegalStateException e) {
            Log.e("@M_" + LOG_OMA_DL, "DownloadList:handleOmaDownload()", e);
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }

    }

    /**
     *  Delete dd file of OMA download
     */
    private void deleteOMADownloadDDFile(long downloadID) {
        Log.d("@M_" + LOG_OMA_DL, "deleteOMADownload(): downloadID is: " + downloadID);
        mDownloadManager.markRowDeleted(downloadID);
        NotificationManager mNotifManager =
         (NotificationManager) this.getApplicationContext().getSystemService(
                Context.NOTIFICATION_SERVICE);
        mNotifManager.cancel((int) downloadID);
        Log.d("@M_" + LOG_OMA_DL, "deleteOMADownload(): cancel notification, id : " + downloadID);
    }

    /**
     *  M: Pop up the alert dialog. Show the OMA DL info or Authenticate info
     */
    private void showAlertDialog(Cursor cursor) {
        //if (cursor.moveToFirst()) {
        for (cursor.moveToFirst(); !cursor.isAfterLast(); cursor.moveToNext()) {
            StringBuilder message = new StringBuilder();
            StringBuilder title = new StringBuilder();
            int showReason = 0;
            ContentValues values = new ContentValues();
            int omaDownloadID =
             cursor.getInt(cursor.getColumnIndexOrThrow(DownloadManager.COLUMN_ID));
            int downloadStatus =
             cursor.getInt(cursor.getColumnIndexOrThrow(Downloads.Impl.COLUMN_STATUS));
            if (downloadStatus == MtkDownloads.Impl.STATUS_NEED_HTTP_AUTH) {
                title.append(getText(R.string.authenticate_dialog_title));
                showReason = downloadStatus;
                Log.d("@M_" + LOG_OMA_DL,
                 "DownloadList: showAlertDialog(): Show Alert dialog reason is "
                        + showReason);

                values.put(Downloads.Impl.COLUMN_STATUS,
                        MtkDownloads.Impl.OMADL_STATUS_ERROR_ALERTDIALOG_SHOWED);
                int row = getContentResolver().update(
                        ContentUris.withAppendedId(
                                Downloads.Impl.ALL_DOWNLOADS_CONTENT_URI,
                                omaDownloadID), values, null, null);
                popAlertDialog(omaDownloadID,
                 null, title.toString(), message.toString(), showReason);
            } else {
                if (!MtkDownloads.Impl.OMA_DOWNLOAD_SUPPORT) {
                    return;
                }
                Log.d("@M_" + LOG_OMA_DL,
                 "DownloadList: showAlertDialog(): Show Alert dialog reason is "
                        + showReason);
                showReason = cursor.getInt(cursor.getColumnIndexOrThrow(
                        MtkDownloads.Impl.COLUMN_OMA_DOWNLOAD_STATUS));
                String name = cursor.getString(cursor.getColumnIndexOrThrow(
                        MtkDownloads.Impl.COLUMN_OMA_DOWNLOAD_DD_FILE_INFO_NAME));
                String vendor = cursor.getString(cursor.getColumnIndexOrThrow(
                        MtkDownloads.Impl.COLUMN_OMA_DOWNLOAD_DD_FILE_INFO_VENDOR));
                String type = cursor.getString(cursor.getColumnIndexOrThrow(
                        MtkDownloads.Impl.COLUMN_OMA_DOWNLOAD_DD_FILE_INFO_TYPE));
                String objectUrl = cursor.getString(cursor.getColumnIndexOrThrow(
                        MtkDownloads.Impl.COLUMN_OMA_DOWNLOAD_OBJECT_URL));
                String nextUrl = cursor.getString(cursor.getColumnIndexOrThrow(
                        MtkDownloads.Impl.COLUMN_OMA_DOWNLOAD_NEXT_URL));
                String notifyUrl = cursor.getString(cursor.getColumnIndexOrThrow(
                        MtkDownloads.Impl.COLUMN_OMA_DOWNLOAD_INSTALL_NOTIFY_URL));
                String description = cursor.getString(cursor.getColumnIndexOrThrow(
                        MtkDownloads.Impl.COLUMN_OMA_DOWNLOAD_DD_FILE_INFO_DESCRIPTION));
                int size = cursor.getInt(cursor.getColumnIndexOrThrow(
                        MtkDownloads.Impl.COLUMN_OMA_DOWNLOAD_DD_FILE_INFO_SIZE));

                boolean isSupportByDevice = true;
                Intent intent = new Intent(Intent.ACTION_VIEW);
                PackageManager pm = getPackageManager();
                intent.setDataAndType(Uri.fromParts("file", "", null), type);
                ResolveInfo ri = pm.resolveActivity(intent, PackageManager.MATCH_DEFAULT_ONLY);
                if (ri == null) {
                    isSupportByDevice = false;
                }
                DownloadInfo downloadInfo = new DownloadInfo(name, vendor, type,
                        objectUrl, nextUrl, notifyUrl, description, size, isSupportByDevice);

                if (showReason == MtkDownloads.Impl.OMADL_STATUS_PARSE_DDFILE_SUCCESS) {
                    title.append(getText(R.string.confirm_oma_download_title));

                    if (name != null) {
                        message.append(getText(R.string.oma_download_name) + " " + name + "\n");
                    }
                    if (vendor != null) {
                        message.append(getText(R.string.oma_download_vendor) + " " + vendor + "\n");
                    }
                    if (type != null) {
                        message.append(getText(R.string.oma_download_type) + " " + type + "\n");
                    }
                    message.append(getText(R.string.oma_download_size) + " " + size + "\n");
                    if (description != null) {
                        message.append(getText(R.string.oma_download_description)
                                + " " + description + "\n");
                    }

                    if (!isSupportByDevice) {
                        message.append("\n" + getText(R.string.oma_download_content_not_supported));
                    }


                } else if (showReason == MtkDownloads.Impl.OMADL_STATUS_HAS_NEXT_URL) {
                    //title.append("OMA Download");
                    message.append(getText(R.string.confirm_oma_download_next_url)
                            + "\n\n" + downloadInfo.mNextUrl);
                }

                //update the status so that this download
                // item can not be queried and show alert dialog.
                values.put(MtkDownloads.Impl.COLUMN_OMA_DOWNLOAD_STATUS,
                        MtkDownloads.Impl.OMADL_STATUS_ERROR_ALERTDIALOG_SHOWED);
                int row = getContentResolver().update(
                        ContentUris.withAppendedId(
                                Downloads.Impl.ALL_DOWNLOADS_CONTENT_URI,
                                omaDownloadID), values, null, null);
                popAlertDialog(omaDownloadID, downloadInfo, title.toString(),
                        message.toString(), showReason);
            }
        }
    }

    private void popAlertDialog(final int downloadID,
            final DownloadInfo downloadInfo, final String title,
            final String message, final int showReason) {

        final View v =  LayoutInflater.from(this).inflate(R.layout.http_authentication, null);
        String positiveString = null;
        if (showReason == MtkDownloads.Impl.STATUS_NEED_HTTP_AUTH && downloadInfo == null) {
            positiveString = getString(R.string.action);
        } else {
            positiveString = getString(R.string.ok);
        }
        mDialog = new AlertDialog.Builder(OmaDownloadActivity.this)
                        .setTitle(title)
                        .setPositiveButton(positiveString,
                                getOmaDownloadPositiveClickHandler
                                (downloadInfo, downloadID, showReason, v))
                        .setNegativeButton(R.string.cancel,
                                getOmaDownloadCancelClickHandler
                                (downloadInfo, downloadID, showReason))
                        .setOnCancelListener
                        (getOmaDownloadBackKeyClickHanlder(downloadInfo, downloadID, showReason))
                        .create();

        if (showReason == MtkDownloads.Impl.STATUS_NEED_HTTP_AUTH && downloadInfo == null) {
            mDialog.setView(v);
            mDialog.getWindow().setSoftInputMode(
                    WindowManager.LayoutParams.SOFT_INPUT_STATE_VISIBLE);
            v.findViewById(R.id.username_edit).requestFocus();
        } else {
            mDialog.setMessage(message);
        }

        mDownloadsToShow.add(mDialog);
        Log.d("@M_" + LOG_OMA_DL,
         "OmaDownloadActivity: Popup Alert dialog is: **" + mDialog + "**");
        showNextDialog();
    }

    private void showNextDialog() {
        if (mCurrentDialog != null) {
            return;
        }

        if (mDownloadsToShow.isEmpty()) {
            finish();
            return;
        }

        if (mDownloadsToShow != null && !mDownloadsToShow.isEmpty()) {
            synchronized (mDownloadsToShow) {
                mCurrentDialog = mDownloadsToShow.poll();
                if (mCurrentDialog != null && !mCurrentDialog.isShowing()) {
                    Log.d("@M_" + LOG_OMA_DL,
                     "OmaDownloadActivity: Current dialog is: **" + mCurrentDialog + "**");
                    mCurrentDialog.show();
                }
            }
        }
    }



    /**
     *  M: Click "OK" to download the media object
     */
    // Define for Authenticate, will move to Framework
    //public static final String Downloads_Impl_COLUMN_USERNAME = "username";
    //public static final String Downloads_Impl_COLUMN_PASSWORD = "password";
    private DialogInterface.OnClickListener getOmaDownloadPositiveClickHandler
    (final DownloadInfo downloadInfo,
            final int downloadID, final int showReason, final View v) {
        return new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                Log.i("@M_" + LOG_OMA_DL, "DownloadList: getOmaDownloadPositiveClickHandler");
                if (showReason == MtkDownloads.Impl.OMADL_STATUS_PARSE_DDFILE_SUCCESS) {
                    // Insert database to download media object.
                    // We don't use DownloadManager, because of it didn't handle OMA_Download column
                    ContentValues values = new ContentValues();
                    values.put(Downloads.Impl.COLUMN_URI, downloadInfo.mObjectUrl);
                    Log.d("@M_" + LOG_OMA_DL,
                     "DownloadList:getOmaDownloadClickHandler(): onClick(): object url is"
                            + downloadInfo.mObjectUrl + "mime Type is: " + downloadInfo.mType);
                    values.put(Downloads.Impl.COLUMN_NOTIFICATION_PACKAGE, getPackageName());
                    values.put(Downloads.Impl.COLUMN_NOTIFICATION_CLASS,
                            OMADLOpenDownloadReceiver.class.getCanonicalName());
                    values.put(Downloads.Impl.COLUMN_VISIBILITY,
                            Downloads.Impl.VISIBILITY_VISIBLE_NOTIFY_COMPLETED);
                    values.put(Downloads.Impl.COLUMN_MIME_TYPE, downloadInfo.mType);
                    values.put(MtkDownloads.Impl.COLUMN_OMA_DOWNLOAD_FLAG,
                     1); // 1 means it is a OMA_DL
                    values.put(Downloads.Impl.COLUMN_DESTINATION,
                            Downloads.Impl.DESTINATION_EXTERNAL);
                    if (downloadInfo.mNextUrl != null) {
                        values.put(MtkDownloads.Impl.COLUMN_OMA_DOWNLOAD_NEXT_URL,
                                downloadInfo.mNextUrl); // Insert the next url
                        Log.d("@M_" + LOG_OMA_DL,
                         "DownloadList:getOmaDownloadClickHandler(): onClick():" +
                                " next url is" + downloadInfo.mNextUrl);
                    }
                    if (downloadInfo.mInstallNotifyUrl != null) {
                        values.put(MtkDownloads.Impl.COLUMN_OMA_DOWNLOAD_INSTALL_NOTIFY_URL,
                                downloadInfo.mInstallNotifyUrl);
                        Log.d("@M_" + LOG_OMA_DL,
                         "DownloadList:getOmaDownloadClickHandler(): onClick():" +
                                " install Notify url is" + downloadInfo.mInstallNotifyUrl);
                    }

                    ///M: Add user agent string to oma download. @{
                    Cursor cursor = null;
                    try {
                        cursor = getContentResolver().query(
                                ContentUris.withAppendedId
                                (Downloads.Impl.ALL_DOWNLOADS_CONTENT_URI, downloadID),
                                new String[] {MtkDownloads.Impl.COLUMN_DOWNLOAD_PATH_SELECTED,
                                     Downloads.Impl.COLUMN_USER_AGENT},
                                null, null, null);

                        if (cursor != null && cursor.moveToFirst()) {
                            String userAgentString =
                             cursor.getString(cursor.getColumnIndex
                             (Downloads.Impl.COLUMN_USER_AGENT));
                            values.put(Downloads.Impl.COLUMN_USER_AGENT, userAgentString);
                            Log.d("@M_" + LOG_OMA_DL,
                             "DownloadList:getOmaDownloadClickHandler(): onClick():" +
                                    " userAgent is " + userAgentString);

                            /// M: Operator Feature get download path from file manager APP. @{
                            sDownloadProviderFeatureExt = PluginFactory.getDefault(
                                    OmaDownloadActivity.this);
                            String selectedPath = cursor.getString(
                                    cursor.getColumnIndex(
                                            MtkDownloads.Impl.COLUMN_DOWNLOAD_PATH_SELECTED));
                            sDownloadProviderFeatureExt.setDownloadPathSelectFileMager(
                                    MtkDownloads.Impl.COLUMN_DOWNLOAD_PATH_SELECTED,
                                    selectedPath, values);
                            ///@}
                        }
                    } catch (IllegalStateException e) {
                        Log.e("@M_" + LOG_OMA_DL, "Query selected download path failed");
                    } finally {
                        if (cursor != null) {
                            cursor.close();
                        }
                    }
                    ///@}

                    try {
                        WebAddress webAddress = new WebAddress(downloadInfo.mObjectUrl);
                        values.put(Downloads.Impl.COLUMN_DESCRIPTION, webAddress.getHost());
                        getContentResolver().insert(Downloads.Impl.CONTENT_URI, values);
                    } catch (ParseException e) {
                        Log.e("@M_" + LOG_OMA_DL,
                         "Exception trying to parse url:" + downloadInfo.mObjectUrl);
                        getContentResolver().insert(Downloads.Impl.CONTENT_URI, values);
                        mCurrentDialog = null;
                        showNextDialog();
                    }

                    // Delete the .dd file
                    ContentValues ddFilevalues = new ContentValues();
                    ddFilevalues.put(MtkDownloads.Impl.COLUMN_OMA_DOWNLOAD_STATUS,
                            MtkDownloads.Impl.OMADL_STATUS_ERROR_USER_DOWNLOAD_MEDIA_OBJECT);
                    int row = getContentResolver().update(ContentUris.withAppendedId(
                                    Downloads.Impl.ALL_DOWNLOADS_CONTENT_URI,
                                    downloadID), ddFilevalues, null, null);
                    //mDownloadManager.markRowDeleted(downloadID);
                    deleteOMADownloadDDFile(downloadID);
                    // If button is shown, dismiss it.
                    // clearSelection();

                    // If we follow the Google dafault delete flow, the dd file no need to deleted.
                    // So, call deleteDownload(downloadID) function.
                    // deleteDownload(downloadID);

                } else if (showReason == MtkDownloads.Impl.OMADL_STATUS_HAS_NEXT_URL) {
                    if (downloadInfo.mNextUrl != null) {
                        Uri uri = Uri.parse(downloadInfo.mNextUrl);
                        Intent intent = new Intent(Intent.ACTION_VIEW, uri);
                        intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_WHEN_TASK_RESET);
                        startActivity(intent);
                    }
                } else if (showReason == MtkDownloads.Impl.STATUS_NEED_HTTP_AUTH && v != null) {
                    String nm = ((EditText) v
                            .findViewById(R.id.username_edit))
                            .getText().toString();
                    String pw = ((EditText) v
                            .findViewById(R.id.password_edit))
                            .getText().toString();
                    Log.d("@M_" + XTAG_ENHANCE,
                     "DownloadList:getOmaDownloadClickHandler:onClick():" +
                            "Autenticate UserName is " + nm + " Password is " + pw);

                    ContentValues values = new ContentValues();
                    values.put(MtkDownloads.Impl.COLUMN_USERNAME, nm);
                    values.put(MtkDownloads.Impl.COLUMN_PASSWORD, pw);
                    values.put(Downloads.Impl.COLUMN_STATUS, Downloads.Impl.STATUS_PENDING);
                    int row = getContentResolver().update(
                            ContentUris.withAppendedId(
                                    Downloads.Impl.ALL_DOWNLOADS_CONTENT_URI,
                                    downloadID), values, null, null);
                }

                mCurrentDialog = null;
                showNextDialog();
            }
        };
    }

    /**
     *  M: Click "Cancel" to cancel download media object
     */
    private DialogInterface.OnClickListener getOmaDownloadCancelClickHandler
    (final DownloadInfo downloadInfo,
            final int downloadID,  final int showReason) {
        return new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                if (showReason == MtkDownloads.Impl.OMADL_STATUS_PARSE_DDFILE_SUCCESS) {
                    Log.i("@M_" + LOG_OMA_DL,
                     "DownloadList:getOmaDownloadClickHandler(): user click Cancel");
                    // Delete the .dd file
                    ContentValues values = new ContentValues();
                    if (!downloadInfo.mSupportByDevice) {
                        values.put(MtkDownloads.Impl.COLUMN_OMA_DOWNLOAD_STATUS,
                                MtkDownloads.Impl.OMADL_STATUS_ERROR_NON_ACCEPTABLE_CONTENT);
                    } else {
                        values.put(MtkDownloads.Impl.COLUMN_OMA_DOWNLOAD_STATUS,
                                MtkDownloads.Impl.OMADL_STATUS_ERROR_USER_CANCELLED);
                    }
                    int row = getContentResolver().update(ContentUris.withAppendedId(
                                    Downloads.Impl.ALL_DOWNLOADS_CONTENT_URI,
                                    downloadID), values, null, null);
                    //mDownloadManager.markRowDeleted(downloadID);
                    deleteOMADownloadDDFile(downloadID);
                    // If button is shown, dismiss it.
                    // clearSelection();

                    // If we follow the Google dafault delete flow, the dd file no need to deleted.
                    // So, call deleteDownload(downloadID) function.
                    //deleteDownload(downloadID);

                } else if (showReason == MtkDownloads.Impl.STATUS_NEED_HTTP_AUTH) {
                    Log.i("@M_" + XTAG_ENHANCE, "DownloadList:getOmaDownloadClickHandler():" +
                            " Authencticate Download:user click Cancel");
                    ContentValues values = new ContentValues();
                    values.put(Downloads.Impl.COLUMN_STATUS, Downloads.Impl.STATUS_UNKNOWN_ERROR);
                    int row = getContentResolver().update(ContentUris.withAppendedId(
                            Downloads.Impl.ALL_DOWNLOADS_CONTENT_URI,
                            downloadID), values, null, null);
                }

                mCurrentDialog = null;
                showNextDialog();
            }
        };
    }

    /**
     *  M: Click "Back key" to cancel download media object
     */
    private DialogInterface.OnCancelListener getOmaDownloadBackKeyClickHanlder
    (final DownloadInfo downloadInfo,
            final int downloadID, final int showReason) {
        return new DialogInterface.OnCancelListener() {

            @Override
            public void onCancel(DialogInterface dialog) {
                if (showReason == MtkDownloads.Impl.OMADL_STATUS_PARSE_DDFILE_SUCCESS) {
                    Log.i("@M_" + LOG_OMA_DL,
                     "DownloadList:getOmaDownloadClickHandler(): user click Back key");
                    // DeleteDownloads
                    ContentValues values = new ContentValues();
                    if (!downloadInfo.mSupportByDevice) {
                        values.put(MtkDownloads.Impl.COLUMN_OMA_DOWNLOAD_STATUS,
                                MtkDownloads.Impl.OMADL_STATUS_ERROR_NON_ACCEPTABLE_CONTENT);
                    } else {
                        values.put(MtkDownloads.Impl.COLUMN_OMA_DOWNLOAD_STATUS,
                                MtkDownloads.Impl.OMADL_STATUS_ERROR_USER_CANCELLED);
                    }
                    int row = getContentResolver().update(ContentUris.withAppendedId(
                                    Downloads.Impl.ALL_DOWNLOADS_CONTENT_URI,
                                    downloadID), values, null, null);
                    //mDownloadManager.markRowDeleted(downloadID);
                    deleteOMADownloadDDFile(downloadID);
                    // If button is shown, dismiss it.
                    // clearSelection();

                    // If we follow the Google dafault delete flow, the dd file no need to deleted.
                    // So, call deleteDownload(downloadID) function.
                    //deleteDownload(downloadID);
                } else if (showReason == MtkDownloads.Impl.STATUS_NEED_HTTP_AUTH) {
                    Log.i("@M_" + XTAG_ENHANCE, "DownloadList:getOmaDownloadClickHandler(): " +
                            "Authencticate Download:user click Cancel");
                    ContentValues values = new ContentValues();
                    values.put(Downloads.Impl.COLUMN_STATUS, Downloads.Impl.STATUS_UNKNOWN_ERROR);
                    int row = getContentResolver().update(ContentUris.withAppendedId(
                            Downloads.Impl.ALL_DOWNLOADS_CONTENT_URI,
                            downloadID), values, null, null);
        }
                mCurrentDialog = null;
                showNextDialog();
            }
        };
    }

    private void deleteDownload(long downloadId) {
        // let DownloadService do the job of cleaning up the downloads db, mediaprovider db,
        // and removal of file from sdcard
        // TODO do the following in asynctask - not on main thread.

        mDownloadManager.markRowDeleted(downloadId);
    }

}
