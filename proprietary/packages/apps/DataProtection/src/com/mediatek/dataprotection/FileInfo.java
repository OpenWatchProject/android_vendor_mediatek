package com.mediatek.dataprotection;

import static android.content.ContentResolver.MIME_TYPE_DEFAULT;
import android.annotation.NonNull;
import android.annotation.Nullable;
import android.content.Context;
import android.media.MediaFile;
import android.os.SystemProperties;
import android.provider.MediaStore;
import libcore.net.MimeUtils;

import com.mediatek.dataprotection.utils.FeatureOptionsUtils;
import com.mediatek.dataprotection.utils.FileUtils;
import com.mediatek.dataprotection.utils.Log;

import java.io.File;

public class FileInfo {

    private static final String TAG = "FileInfo";
    public static final int IMAGE_FILE = 1;
    public static final int VIDEO_FILE = 2;
    public static final int AUDIO_FILE = 3;
    public static final int UNKOWN_FILE = 4;

    public static final String COLUMN_PATH = MediaStore.Files.FileColumns.DATA;
    public static final String COLUMN_LAST_MODIFIED = MediaStore.Files.FileColumns.DATE_MODIFIED;
    public static final String COLUMN_SIZE = MediaStore.Files.FileColumns.SIZE;
    public static final String COLUMN_IS_DIRECTORY = "format";
    public static final String COLUMN_MIME_TYPE = MediaStore.Files.FileColumns.MIME_TYPE;
    public static final String COLUMN_IS_DRM = MediaStore.MediaColumns.IS_DRM;

    public static final String ENCRYPT_FILE_EXTENSION = "mudp";
    private static final String EXT_DRM_CONTENT = "dcf";

    private String mPath;
    private File mFile;
    private boolean mIsChecked = false;
    private boolean mIsDirectory = false;
    private boolean mIsShowTemp = true;
    private long mLastModified = 0;
    private long mSize = 0;
    private String mMimeType = null;

    public FileInfo(String path, boolean isDirectory, long lastModified,
            long size, String mimeType) {
        mPath = path;
        mIsDirectory = isDirectory;
        mLastModified = lastModified;
        mSize = size;
        mMimeType = mimeType;
        mFile = new File(path);
    }

    public FileInfo(File file) {
        setPath(file.getAbsolutePath());
        setFile(file);
        mIsDirectory = file.isDirectory();
        mLastModified = file.lastModified();
        if (!file.isDirectory()) {
            mSize = file.length();
        }
        mMimeType = MediaFile.getMimeTypeForFile(getFileAbsolutePath());
    }

    public FileInfo(String path, long modify, long size) {
        mPath = path;
        mLastModified = modify;
        mSize = size;
    }

    public long getLastModified() {
        return mLastModified;
    }

    public long getFileSize() {
        return mSize;
    }

    public boolean isChecked() {
        return mIsChecked;
    }

    public void setChecked(boolean mIsChecked) {
        this.mIsChecked = mIsChecked;
    }

    public String getPath() {
        return mPath;
    }

    public void setPath(String mPath) {
        this.mPath = mPath;
    }

    public File getFile() {
        return mFile;
    }

    public SDCardWritableFile getSDCardWritableFile(Context context) {
        if (MountPointManager.getInstance().isExternalFile(this)) {
            return new SDCardFile(mFile, mMimeType, context);
        } else {
            return new InternalFile(mFile, mMimeType);
        }
    }

    public String getMimeType() {
        return mMimeType;
    }

    public void setMimeType(String mimeType) {
        mMimeType = mimeType;
    }

    public void setFile(File mFile) {
        this.mFile = mFile;
    }

    public String getShowName() {

        return FileUtils.getFileName(getShowPath());
    }

    public String getLockedFileShowName() {
        String name = mFile.getName();
        if (name != null && !name.isEmpty()) {
            if (name.startsWith(".")) {
                // name.replace(".", newChar)
                name = name.substring(1);
            }
            if (name.endsWith("." + FileInfo.ENCRYPT_FILE_EXTENSION)) {
                name = name.substring(0, name.lastIndexOf("."
                        + FileInfo.ENCRYPT_FILE_EXTENSION));
            }
        }
        return name;
    }

    public String getFileAbsolutePath() {
        if (null != mFile) {
            return mFile.getAbsolutePath();
        } else if (mPath != null) {
            return mPath;
        }
        return "";
    }

    public String getShowPath() {
        return MountPointManager.getInstance().getDescriptionPath(
                getFileAbsolutePath());
    }

    public boolean isDirectory() {
        return mIsDirectory;
    }

    public int getFileType() {
        String filePath = mFile.getAbsolutePath();
        int sepIndex = filePath.lastIndexOf(".");
        String ext = null;
        if (sepIndex != -1) {
            ext = filePath.substring(sepIndex + 1);
        }

        String mimeType = MediaFile.getMimeTypeForFile(filePath);
        String subFolder = "/.decrypted/";
        if (mimeType != null) {
            if (FileUtils.isImage(mimeType)) {
                return IMAGE_FILE;
            } else if (FileUtils.isVideo(mimeType)) {
                return VIDEO_FILE;
            } else if (FileUtils.isAudio(mimeType)) {
                return AUDIO_FILE;
            } else {
                return UNKOWN_FILE;
            }
        } else {
            return UNKOWN_FILE;
        }
    }

    public boolean isNeedToShow() {
        return this.mIsShowTemp;
    }

    public void hiddenEncryptFile(boolean res) {
        mIsShowTemp = res;
    }

    public String getFileName() {
        return mFile.getName();
    }

    /**
     * The method check the file is DRM file, or not.
     *
     * @return true for DRM file, false for not DRM file.
     */
    public boolean isDrmFile() {
        if (mIsDirectory) {
            return false;
        }
        boolean isDrmMime = isDrmMimeType(mMimeType);
        Log.d(TAG,"isDrmMime = " + isDrmMime);
        return isDrmMime;
    }
    private boolean isDrmMimeType(@Nullable String mimeType) {
        if (normalizeMimeType(mimeType).equals("cta/mudp")) {
            return true;
        }
        if (SystemProperties.getBoolean("ro.vendor.mtk_oma_drm_support", false)) {
            if (normalizeMimeType(mimeType).equals("application/vnd.oma.drm.content")
                || normalizeMimeType(mimeType).equals("application/vnd.oma.drm.message")) {
                return true;
            }
        }
        return (normalizeMimeType(mimeType).equals("application/x-android-drm-fl"));
    }
    /**
     * Normalize the given MIME type by bouncing through a default file
     * extension, if defined. This handles cases like "application/x-flac" to
     * ".flac" to "audio/flac".
     */
    private @NonNull String normalizeMimeType(@Nullable String mimeType) {
        final String extension = MimeUtils.guessExtensionFromMimeType(mimeType);
        if (extension != null) {
            final String extensionMimeType = MimeUtils.guessMimeTypeFromExtension(extension);
            if ( extensionMimeType != null) {
                return extensionMimeType;
            }
        }
        return (mimeType != null) ? mimeType : MIME_TYPE_DEFAULT;
    }
}
