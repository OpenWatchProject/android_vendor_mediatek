package com.mediatek.mtkdownloadmanager.ext;

import android.app.AlertDialog;
import android.content.ContentValues;
import android.content.Context;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.net.Uri;




public interface IDownloadProviderFeatureExt {

    /**
     * Save continue download to the downloadInfo.
     *
     * @param contiDownload The continue download element in downloadInfo.
     * @param value The value that query from db. if ture, it means user choose
     *        continue download when file already exists.
     * @internal
     */
    void setContinueDownload(boolean contiDownload, boolean value);

    /**
     * Save download path to the downloadInfo.
     *
     * @param downloadPath The download path element in downloadInfo.
     * @param value The value that query form db.
     * @internal
     */
    void setDownloadPath(String downloadPath, String value);

    /**
     * Used to show full download path in download notificaiton.
     *
     * @param packageName The app package name which call download.
     * @param mimeType Download file mimetype
     * @param fullFileName The download file fulll name
     * @return it will return the notification text content.
     * @internal
     */
    String getNotificationText(String packageName,
            String mimeType, String fullFileName);

    /**
     * Copy the given key and relevant value from one Contentvalues to another
     *
     * @param key The key string which in ContentValues copy from.
     * @param from The ContentValues that copy from.
     * @param to The CotentValues that copy to.
     * @internal
     */
    void copyContentValues(String key, ContentValues from, ContentValues to);

    /**
     * Send notify intent when download file already exsits in storage.
     *
     * @param uri The download item uri.
     * @param packageName Package name of app that notify intent send to.
     * @param className Class name of app that notify intent send to.
     * @param fullFileName The full file name which notify.
     * @param context Context that send intent.
     * @internal
     */
    void notifyFileAlreadyExistIntent(Uri uri, String packageName, String className,
            String fullFileName, Context context);

    /**
     * Get the dialog reason value from received intent.
     *
     * @param intent The intent received.
     * @return Dialog reason value.
     * @internal
     */
    int getShowDialogReasonInt(Intent intent);

    /**
     * Get the default download dir according to mimetype.
     *
     * @param mimeType The mimetype of donwload file.
     * @return Directory string
     * @internal
     */
    String getStorageDirectory(String mimeType);

    /**
     * Set column value to ContentValues.
     *
     * @param columnName Column name in table.
     * @param value Column value
     * @param contentValues ContentValues that will insert to.
     * @internal
     */
    void setDownloadPathSelectFileMager(String columnName,
     String value, ContentValues contentValues);

    /**
     * process file already exists condition.
     *
     * @return result
     * @internal
     */
    int processFileExistCondition(boolean continueDownload);

    /**
     * The fuction will show file already exist dialog for end user.
     * if user choose "ok", it will download continue, if choose "cancel",
     * it will cancel this download.
     *
     * @param builder AlertDialog builder
     * @param appLable Dialog lable
     * @param message Dialog message
     * @param positiveButtonString Positive button string
     * @param negativeButtonString Negative button string
     * @param listener The click listener registered
     * @internal
     */
    void showFileAlreadyExistDialog(AlertDialog.Builder builder, CharSequence appLable,
            CharSequence message, String positiveButtonString,
             String negativeButtonString, OnClickListener listener);
}
