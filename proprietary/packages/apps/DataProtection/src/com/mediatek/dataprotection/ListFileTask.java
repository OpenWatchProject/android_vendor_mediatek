package com.mediatek.dataprotection;

import java.io.File;
import java.io.FileFilter;
import java.util.ArrayList;
import java.util.List;
import java.util.Set;

import android.content.Context;
import android.drm.DrmManagerClient;
import android.media.MediaFile;
import android.os.AsyncTask;
import android.os.Handler;
import com.mediatek.dataprotection.utils.Log;

import com.mediatek.dataprotection.DataProtectionService.FileOperationEventListener;

public class ListFileTask extends AsyncTask<Void, Void, Integer> {

    private static final String TAG = "ListFileTask";
    private static final int NOTIFY_INTERNAL = 400;
    protected Context mContext;
    protected List<FileInfo> mFiles;
    protected String mPath;
    protected FileOperationEventListener mCallback;
    protected int mTotalNum = 0;
    protected Handler mHandler = new Handler();
    protected DataProtectionService mService = null;
    protected Set<String> mEncryptingFiles = null;
    protected long mOldModifiedTime = 0;
    protected long mModifiedTime = 0;

    protected final Runnable mNotifyProgress = new Runnable() {
        @Override
        public void run() {
            if (isCancelled()) {
                Log.d(TAG, "notify progress " + isCancelled());
                mHandler.removeCallbacks(mNotifyProgress);
                return;
            }
            if (mCallback != null && mTotalNum != 0) {
                mCallback.onTaskProgress(mFiles.size(), mTotalNum);
            }
            if (mFiles.size() <= mTotalNum) {
                mHandler.postDelayed(mNotifyProgress, NOTIFY_INTERNAL);
                //Log.d(TAG,"notify progress " + mCallback);
            }
        }
    };

    public ListFileTask(Context context, List<FileInfo> arrays, String path) {
        mContext = context;
        mFiles = new ArrayList<FileInfo>();
        mPath = path;
    }

    public ListFileTask(Context context, List<FileInfo> arrays, String path,
            FileOperationEventListener listener, DataProtectionService service) {
        this(context, arrays, path);
        mCallback = listener;
        mService = service;
    }

    /**
     * Constructor.
     *
     * @param context Context to use
     * @param arrays not used now
     * @param path String file path
     * @param listener Listen the result of Task
     * @param service for filter
     * @param lastModified folder last modified time
     */
    public ListFileTask(Context context, List<FileInfo> arrays, String path,
            FileOperationEventListener listener, DataProtectionService service, long lastModified) {
        this(context, arrays, path);
        mCallback = listener;
        mService = service;
        mOldModifiedTime = lastModified;
    }

    @Override
    protected void onPreExecute() {
        // mCallback.onTaskPrepare();
        Log.s(TAG, "onPreExecute ... " + mPath);
        mHandler.postDelayed(mNotifyProgress, NOTIFY_INTERNAL);
    }

    @Override
    protected Integer doInBackground(Void... params) {
        if (null == mFiles || null == mPath || mPath.isEmpty()) {
            return FileOperationEventListener.ERROR_CODE_INVALID_ARGUMENT;
        }

        File destFile = new File(mPath);
        if (!destFile.exists()) {
            return FileOperationEventListener.ERROR_CODE_FILE_EXIST;
        }

        long nowModifiedTime = destFile.lastModified();
        if (mOldModifiedTime != 0 && mOldModifiedTime == nowModifiedTime) {
            Log.d(TAG, "doInBackground this folder content hasnot changed. return.");
            return FileOperationEventListener.ERROR_CODE_NO_CHANGE;
        }

        mEncryptingFiles = mService.getEncryptingFiles();
        Log.d(TAG, "doInBackground " + mEncryptingFiles.size());
        File[] files = null;
        mModifiedTime = destFile.lastModified();
        files = destFile.listFiles(new FileFilter() {
            @Override
            public boolean accept(File arg0) {
                if (arg0.isHidden()) {
                    return false;
                }
                if (mEncryptingFiles.contains(arg0.getAbsolutePath())
                    || arg0.getAbsolutePath().endsWith("." + FileInfo.ENCRYPT_FILE_EXTENSION)) {
                    return false;
                }
                return true;
            }
        });
        mTotalNum = (files != null ? files.length : 0);
        //ALPS03626589
        //DrmManagerClient client = DataProtectionApplication.getCtaClient(mContext);
        if (files != null && files.length > 0) {
            for (int i = 0; i < files.length; i++) {
                if (isCancelled()) {
                    break;
                }
                if (!files[i].exists()) {
                    continue;
                }
                FileInfo file = new FileInfo(files[i]);
                if (file.isDrmFile()) {
                    file.setMimeType("drm/dcf");
                } else {
                    file.setMimeType(MediaFile.getMimeTypeForFile(file.getPath()));
                }
                mFiles.add(file);
            }
        }
        return FileOperationEventListener.ERROR_CODE_SUCCESS;
    }

    @Override
    protected void onPostExecute(Integer result) {
        Log.s(TAG, "onPostExecute ... " + result + " " + mPath);
        if (isCancelled()) {
            mHandler.removeCallbacks(mNotifyProgress);
            return;
        }
        mHandler.removeCallbacks(mNotifyProgress);
        if (mCallback != null) {
            mCallback.onTaskResult(result, mFiles, mModifiedTime);
            mCallback = null;
            Log.d(TAG, "onPostExecute ... " + result + "listener isnot null");
        }
    }

    public void setListener(FileOperationEventListener listener) {
        Log.d(TAG, "setListener " + listener);
        mCallback = listener;
    }
}
