package com.mediatek.dataprotection;

import java.io.File;
import java.io.FileNotFoundException;

import android.os.ParcelFileDescriptor;

public interface SDCardWritableFile {
    public boolean delete();

    public boolean createNewFile();

    public boolean renameTo(File targetFile);

    public File getFile();

    public String getAbsolutePath();

    public String getName();

    public String getParent();

    public boolean exists();

    public long length();

    public String getMimeType();

    public ParcelFileDescriptor getParcelFileDescriptor() throws FileNotFoundException;
}
