package com.mediatek.dataprotection;

import android.content.ContentResolver;
import android.content.Context;
import android.net.Uri;
import android.os.ParcelFileDescriptor;
import android.provider.DocumentsContract;
import android.support.annotation.NonNull;

import java.io.File;
import java.io.FileNotFoundException;
import com.mediatek.dataprotection.utils.Log;

public class SDCardFile implements SDCardWritableFile {
    private static final String TAG = "Dataprotection/SDCardFile";

    private static final String AUTHORITY_EXTERNAL_STORAGE =
            "com.android.externalstorage.documents";
    private static final String FILE_PATH_INDEX = "storage/";

    private ContentResolver mContentResolver;
    private File mFile;
    private String mMimeType;
    private Uri mUri;

    public SDCardFile(@NonNull File file, @NonNull String mimeType,
            @NonNull Context context) {
        mFile = file;
        mMimeType = mimeType;
        mContentResolver = context.getContentResolver();
        mUri = getFileUri(file);
    }

    public boolean delete() {
        Log.w(TAG, "deleteFile Uri: " + mUri);

        try {
            return DocumentsContract.deleteDocument(mContentResolver, mUri);
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }
        return false;
    }

    public boolean createNewFile() {
        String displayName = mFile.getName();
        try {
            mUri = DocumentsContract.createDocument(mContentResolver,
                    getFileUri(mFile.getParentFile()), mMimeType, displayName);
            Log.w(TAG, "createFile Uri: " + mUri);
            return mUri != null;
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }
        return false;
    }

    public boolean moveFile(File targetFile) {
        Uri sourceParentDocumentUri = getFileUri(mFile.getParentFile());
        Uri targetParentDocumentUri = getFileUri(targetFile);
        Log.w(TAG,
                "sourceDocumentUri = " + mUri.toString()
                        + ", sourceParentDocumentUri = "
                        + sourceParentDocumentUri.toString()
                        + ", targetParentDocumentUri = "
                        + targetParentDocumentUri.toString());
        try {
            Uri fileUri = DocumentsContract.moveDocument(mContentResolver,
                    mUri, sourceParentDocumentUri, targetParentDocumentUri);
            return fileUri != null;
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }
        return false;
    }

    public boolean renameTo(File targetFile) {
        try {
            Uri fileUri = DocumentsContract.renameDocument(mContentResolver,
                    mUri, targetFile.getName());
            return fileUri != null;
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }
        return false;
    }

    public ParcelFileDescriptor getParcelFileDescriptor()
            throws FileNotFoundException {
        return mContentResolver.openFileDescriptor(mUri, "rw");
    }

    public File getFile() {
        return mFile;
    }

    public String getAbsolutePath() {
        return mFile.getAbsolutePath();
    }

    public String getName() {
        return mFile.getName();
    }

    public String getParent() {
        return mFile.getParent();
    }

    public boolean exists() {
        return mFile.exists();
    }

    public long length() {
        return mFile.length();
    }

    public String getMimeType() {
        return mMimeType;
    }

    private Uri getFileUri(File file) {
        // /storage/3558-110E/mtklog/netlog/NTLog_2018_0620_142338
        String filePath = file.getAbsolutePath();
        int externalStroageIndex = filePath.indexOf(FILE_PATH_INDEX);
        if (externalStroageIndex == -1) {
            return null;
        }
        String docmentId = filePath.substring(externalStroageIndex
                + FILE_PATH_INDEX.length());
        if (!docmentId.contains("/")) {
            docmentId = docmentId + "/";
        }
        docmentId = docmentId.replaceFirst("/", ":");
        Log.w(TAG, "docmentId: " + docmentId);
        return DocumentsContract.buildDocumentUri(AUTHORITY_EXTERNAL_STORAGE,docmentId);
    }
}
