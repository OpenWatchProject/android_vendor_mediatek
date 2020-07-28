package com.mediatek.dataprotection;

import java.io.File;
import java.util.ArrayList;
import java.util.List;
import java.util.Set;

import android.content.AsyncTaskLoader;
import android.content.Context;
import android.database.Cursor;
import android.drm.DrmManagerClient;
import android.media.MediaFile;
import android.mtp.MtpConstants;
import android.net.Uri;
import android.provider.MediaStore;

import com.mediatek.dataprotection.utils.FileUtils;
import com.mediatek.dataprotection.utils.PermissionUtils;
import com.mediatek.dataprotection.utils.Log;

class LockedFileResult {
    List<FileInfo> pictures = new ArrayList<FileInfo>();
    List<FileInfo> videos = new ArrayList<FileInfo>();
    List<FileInfo> audios = new ArrayList<FileInfo>();
    List<FileInfo> others = new ArrayList<FileInfo>();
    @Override
    public String toString() {
        return "total = " + getCount() + " (picture = " + pictures.size() + ", video = " + videos.size()
                + ", audio = " + audios.size() + ", other = " + others.size() + ")";
    }
    public int getCount() {
        return pictures.size() + videos.size() + audios.size() + others.size();
    }
}

public class LockedFileLoader extends AsyncTaskLoader<LockedFileResult> {

    private static final String TAG = "LockedFileLoader";

    private static final int FOLDER_VALUE = MtpConstants.FORMAT_ASSOCIATION;
    private static final Uri FILE_URI = MediaStore.Files.getContentUri("external");
    private boolean mIsLoading = false;
    private boolean mNeedLoadAgain = false;
    private LockedFileResult mResult = null;
    private DataProtectionService mService = null;
    private final ForceLoadContentObserver mObserver = new ForceLoadContentObserver();
    private Set<String> mDecryptingFiles = null;
    private Set<String> mDecryptFailFiles = null;

    public LockedFileLoader(Context context, DataProtectionService service) {
        super(context);
        context.getContentResolver().registerContentObserver(FILE_URI, false, mObserver);
        mService = service;
        Log.d(TAG, "LockedFileLoader constructor done");
    }

    @Override
    public LockedFileResult loadInBackground() {
        Log.d(TAG, "LockedFileLoader loadInBackground entry...");
        long start = System.currentTimeMillis();
        mIsLoading = true;
        StringBuilder stringBuilder = new StringBuilder();
        stringBuilder.append(MediaStore.Files.FileColumns.DATA);
        stringBuilder.append(" LIKE '%");
        stringBuilder.append("." + FileInfo.ENCRYPT_FILE_EXTENSION + "'");
        String selection = stringBuilder.toString();
        String[] projection = new String[] { FileInfo.COLUMN_IS_DIRECTORY,
                FileInfo.COLUMN_LAST_MODIFIED,
                FileInfo.COLUMN_PATH, FileInfo.COLUMN_SIZE,
                FileInfo.COLUMN_MIME_TYPE };
        Cursor lockedFileCursor = null;
        LockedFileResult result = new LockedFileResult();
        try {
            mDecryptingFiles = mService.getDecryptingFiles();
            mDecryptFailFiles = mService.getDecryptFailFiles();
            if (PermissionUtils.hasStorageReadPermission(getContext())) {
                lockedFileCursor = getContext().getContentResolver().query(
                    FILE_URI,
                    projection,
                    selection,
                    null,
                    FileInfo.COLUMN_LAST_MODIFIED);
            }
            if (lockedFileCursor != null && lockedFileCursor.moveToFirst()) {
                cursorToFile(result, lockedFileCursor);
                if (mService.isDecrptingFilesUpdate()) {
                    mNeedLoadAgain = true;
                    result = null;
                    Log.d(TAG, "loadInBackground finish with new decrpting files added, need reload again.");
                }
            }
        } catch (Exception e) {
            Log.d(TAG, "data base exception: " + e);
        } finally {
            if (lockedFileCursor != null) {
                lockedFileCursor.close();
                lockedFileCursor = null;
            }
        }
        long end = System.currentTimeMillis();
        Log.d(TAG, "loadInBackground with " + result + " use " + (end - start) + "ms");
        mIsLoading = false;
        return result;
    }

    @Override
    public void onContentChanged() {
        Log.d(TAG, "onContentChanged: mIsLoading = " + mIsLoading + ", isStarted = " + isStarted());
        /// When loader is loading, store the content change and load it again after current finish.
        if (isStarted()) {
            if (mIsLoading) {
                mNeedLoadAgain = true;
            } else {
                forceLoad();
            }
        } else {
            super.onContentChanged();
        }
    }

    @Override
    public void cancelLoadInBackground() {
        Log.d(TAG, "LockedFileLoader cancelLoadInBackground...");
        super.cancelLoadInBackground();
    }

    @Override
    public void deliverResult(LockedFileResult result) {
        Log.d(TAG, "deliverResult: result = " + result + ", mNeedLoadAgain = " + mNeedLoadAgain);
        if (isReset()) {
            return;
        }
        mResult = result;

        if (isStarted()) {
            super.deliverResult(result);
            if (mNeedLoadAgain) {
                forceLoad();
                mNeedLoadAgain = false;
            }
        }
    }

    @Override
    protected void onStartLoading() {
        Log.d(TAG, "onStartLoading: " + mResult);
        if (mResult != null) {
            deliverResult(mResult);
        }
        // / M: show previous loader's result @{
        boolean contentChanged = takeContentChanged();
        if (!contentChanged && mIsLoading) {
            return;
        }
        if (contentChanged || mResult == null) {
            forceLoad();
        }
        /// @}
    }

    @Override
    protected void onForceLoad() {
        Log.d(TAG, "onForceLoad: mIsLoading = " + mIsLoading + ", mNeedLoadAgain = " + mNeedLoadAgain);
        super.onForceLoad();
        mNeedLoadAgain = false;
    }

    @Override
    protected void onStopLoading() {
        Log.d(TAG, "onStopLoading: " + mResult);
        cancelLoad();
    }

    @Override
    public void onCanceled(LockedFileResult result) {
        Log.d(TAG, "onCanceled: " + result);
        // If there is no result show, we need show last result first
        if (mResult == null && result != null) {
            mResult = result;
            deliverResult(result);
        }
    }

    @Override
    protected void onReset() {
        Log.d(TAG, "LockedFileLoader onReset...");
        super.onReset();

        // Ensure the loader is stopped
        onStopLoading();

        mResult = null;
        mNeedLoadAgain = false;
        getContext().getContentResolver().unregisterContentObserver(mObserver);
    }

    private void cursorToFile(LockedFileResult result, Cursor cursor) {

        Log.d(TAG, "query count: " + cursor.getCount() + " filter count: " + mDecryptingFiles.size());

        String filePath = null;
        boolean isDirectory = false;
        String originalMimeType = null;
        String mimeType = null;
        long lastModified = 0;
        long size = 0;
        DrmManagerClient client = new DrmManagerClient(getContext());

        do {
            filePath = FileUtils.getCursorString(cursor, FileInfo.COLUMN_PATH);
            File curFile = new File(filePath);
            if (!curFile.exists()) {
                Log.s(TAG, "file: " + filePath + " not exist. do next");
                continue;
            }
            if (mDecryptingFiles.contains(curFile.getAbsolutePath())) {
                Log.s(TAG, "file: " + filePath + " is in decrypting, not show in UI");
                continue;
            }
            if (mDecryptFailFiles.contains(curFile.getAbsolutePath())) {
                Log.s(TAG, "file: " + filePath + " is decrypt fail(bad key), not show in main UI");
                continue;
            }
            lastModified = FileUtils.getCursorLong(cursor, FileInfo.COLUMN_LAST_MODIFIED);
            size = FileUtils.getCursorLong(cursor, FileInfo.COLUMN_SIZE);
            isDirectory = FileUtils.getCursorInt(cursor, FileInfo.COLUMN_IS_DIRECTORY) == FOLDER_VALUE;
            mimeType = FileUtils.getCursorString(cursor, FileInfo.COLUMN_MIME_TYPE);
            originalMimeType = mimeType;
            FileInfo fileInfo = new FileInfo(filePath, isDirectory, lastModified * 1000,
            size, mimeType);
            if (fileInfo.isDrmFile()) {
                originalMimeType = client.getOriginalMimeType(filePath);
            }
            if (originalMimeType != null) {
                if (FileUtils.isImage(originalMimeType)) {
                    result.pictures.add(fileInfo);
                } else if (FileUtils.isAudio(originalMimeType)) {
                    result.audios.add(fileInfo);
                } else if (FileUtils.isVideo(originalMimeType)) {
                    result.videos.add(fileInfo);
                } else {
                    result.others.add(fileInfo);
                }
            } else {
                result.others.add(fileInfo);
            }
        } while (cursor.moveToNext());
        client.release();
    }
}
