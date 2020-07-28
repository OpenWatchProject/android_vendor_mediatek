package com.mediatek.plugin.preload;

import android.content.Context;
import android.content.SharedPreferences;

import com.mediatek.plugin.utils.IoUtils;
import com.mediatek.plugin.utils.Log;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.jar.JarFile;
import java.util.zip.ZipEntry;

/**
 * Do so check, copy so and save so information.
 */
public class SoOperater {
    private static final String TAG = "PluginManager/SoOperater";
    public static final String PREFERENCE_NAME = "dynamic_load_configs";
    public static final String CPU_ARMEABI = "armeabi";
    public static final String CPU_X86 = "x86";
    public static final String CPU_MIPS = "mips";
    public static final int STEP = 1024;
    private String mSoFileName;
    private JarFile mJarFile;
    private ZipEntry mZipEntry;
    private String mSoPath;
    private String mCpuName;
    private String mCpuArchitect;

    /**
     * Construct.
     */
    public SoOperater() {
        mCpuName = getCpuName();
        mCpuArchitect = getCpuArch(mCpuName);
    }

    /**
     * Check current so.
     * @param context
     *            Use the context for SharedPreferences.
     * @param soName
     *            The so path name.
     * @param lastModify
     *            The so Lib last modify time.
     * @return whether need copy so or no.
     */
    public boolean isNewSo(Context context, String soName, long lastModify) {
        return soName.contains(mCpuArchitect)
                && (lastModify == getSoLastModifiedTime(context, soName));
    }

    /**
     * Copy so operation.
     * @param context
     *            Use the context for get SharedPreferences.
     * @param jarFile
     *            Get so file from the jar file.
     * @param zipEntry
     *            The zipEntry which has the so Lib.
     * @param soPath
     *            The destination of so Lib.
     */
    public void copy(Context context, JarFile jarFile, ZipEntry zipEntry,
                     String soPath) {
        init(jarFile, zipEntry, soPath);
        try {
            writeSoFile2LibDir();
            Log.d(TAG, "<copy> copy so lib success: " + mZipEntry.getName());
        } catch (IOException e) {
            Log.e(TAG, "<copy> copy so lib failed: ", e);
        }
        setSoLastModifiedTime(context, zipEntry.getName(), zipEntry.getTime());
    }

    private void init(JarFile jarFile, ZipEntry zipEntry, String soPath) {
        mJarFile = jarFile;
        mZipEntry = zipEntry;
        mSoFileName = parseSoFileName(zipEntry.getName());
        Log.d(TAG, "<init> mSoFileName = " + mSoFileName);
        mSoPath = soPath;
    }

    private final String parseSoFileName(String zipEntryName) {
        return zipEntryName.substring(zipEntryName.lastIndexOf("/") + 1);
    }

    private void writeSoFile2LibDir() throws IOException {
        InputStream is = null;
        FileOutputStream fos = null;
        is = mJarFile.getInputStream(mZipEntry);
        Log.d(TAG, "<writeSoFile2LibDir> mSoPath = " + mSoPath);
        Log.d(TAG, "<writeSoFile2LibDir> mSoFileName = " + mSoFileName);
        File saveDirectory = new File(mSoPath);
        // Create the directory if it doesn't exist
        if (!saveDirectory.exists()) {
            saveDirectory.mkdirs();
        }
        fos = new FileOutputStream(new File(saveDirectory, mSoFileName));
        copy(is, fos);
        IoUtils.closeQuietly(fos);
    }

    private void copy(InputStream is, OutputStream os) {
        if (is == null || os == null) {
            return;
        }
        BufferedInputStream bis = new BufferedInputStream(is);
        BufferedOutputStream bos = new BufferedOutputStream(os);
        int size;
        try {
            size = getAvailableSize(bis);
            byte[] buf = new byte[size];
            int i = 0;
            while ((i = bis.read(buf, 0, size)) != -1) {
                bos.write(buf, 0, i);
            }
            bos.flush();
            bos.close();
            bis.close();
        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }

    private int getAvailableSize(InputStream is) throws IOException {
        if (is == null) {
            return 0;
        }
        int available = is.available();
        return available <= 0 ? STEP : available;
    }

    /**
     * get cpu name, according cpu type parse relevant so lib.
     * @return ARM, ARMV7, X86, MIPS.
     */
    private String getCpuName() {
        try {
            FileReader fr = new FileReader("/proc/cpuinfo");
            BufferedReader br = new BufferedReader(fr);
            String text = br.readLine();
            br.close();
            if (text == null) {
                return "";
            }
            String[] array = text.split(":\\s+", 2);
            if (array.length >= 2) {
                return array[1];
            }
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }

        return "";
    }

    private String getCpuArch(String cpuName) {
        String cpuArchitect = CPU_ARMEABI;
        if (cpuName.toLowerCase().contains("arm")) {
            cpuArchitect = CPU_ARMEABI;
        } else if (cpuName.toLowerCase().contains("x86")) {
            cpuArchitect = CPU_X86;
        } else if (cpuName.toLowerCase().contains("mips")) {
            cpuArchitect = CPU_MIPS;
        }

        return cpuArchitect;
    }

    private void setSoLastModifiedTime(Context cxt, String soName, long time) {
        SharedPreferences prefs =
                cxt.getSharedPreferences(PREFERENCE_NAME, Context.MODE_PRIVATE
                        | Context.MODE_MULTI_PROCESS);
        prefs.edit().putLong(soName, time).apply();
    }

    private long getSoLastModifiedTime(Context cxt, String soName) {
        SharedPreferences prefs =
                cxt.getSharedPreferences(PREFERENCE_NAME, Context.MODE_PRIVATE
                        | Context.MODE_MULTI_PROCESS);
        return prefs.getLong(soName, 0);
    }
}
