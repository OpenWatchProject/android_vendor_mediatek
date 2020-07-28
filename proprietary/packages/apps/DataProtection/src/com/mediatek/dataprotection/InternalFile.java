package com.mediatek.dataprotection;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;

import android.os.ParcelFileDescriptor;
import android.support.annotation.NonNull;
import com.mediatek.dataprotection.utils.Log;

public class InternalFile implements SDCardWritableFile {
    private static final String TAG = "Dataprotection/InternalFile";

    private File mFile;
    private String mMimeType;

    public InternalFile(@NonNull File file, @NonNull String mimeType) {
        Log.s(TAG, "InternalFile file: " + file);
        mFile = file;
        mMimeType = mimeType;
    }

    @Override
    public boolean delete() {
        return mFile.delete();
    }

    @Override
    public boolean createNewFile() {
        try {
            return mFile.createNewFile();
        } catch (IOException e) {
            e.printStackTrace();
        }
        return false;
    }

    @Override
    public boolean renameTo(File targetFile) {
        return mFile.renameTo(targetFile);
    }

    @Override
    public File getFile() {
        return mFile;
    }

    @Override
    public String getAbsolutePath() {
        return mFile.getAbsolutePath();
    }

    @Override
    public String getName() {
        return mFile.getName();
    }

    @Override
    public String getParent() {
        return mFile.getParent();
    }

    @Override
    public boolean exists() {
        return mFile.exists();
    }

    @Override
    public long length() {
        return mFile.length();
    }

    @Override
    public String getMimeType() {
        return mMimeType;
    }

    @Override
    public ParcelFileDescriptor getParcelFileDescriptor()
            throws FileNotFoundException {
        return ParcelFileDescriptor.open(mFile, ParcelFileDescriptor.MODE_READ_WRITE);
    }

}
