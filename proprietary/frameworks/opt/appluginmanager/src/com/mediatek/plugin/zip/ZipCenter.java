package com.mediatek.plugin.zip;

import com.mediatek.plugin.utils.ReflectUtils;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.lang.reflect.Constructor;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map.Entry;
import java.util.concurrent.locks.ReadWriteLock;
import java.util.concurrent.locks.ReentrantReadWriteLock;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

/**
 * The class to save zip file type, and create zip file for file path.
 */
public class ZipCenter {
    private static HashMap<String, Class<? extends ZipFile>> sFileList =
            new HashMap<String, Class<? extends ZipFile>>();
    private static ReadWriteLock sFileListLock = new ReentrantReadWriteLock();

    /**
     * Add the ZipFile class.
     * @param suffix
     *            The suffix of file that supported
     * @param clazz
     *            The ZipFile class
     */
    public static void registerZipFile(String suffix, Class<? extends ZipFile> clazz) {
        sFileListLock.writeLock().lock();
        sFileList.put(suffix, clazz);
        sFileListLock.writeLock().unlock();
    }

    /**
     * Create a ZipFile for given plugin.
     * @param filePath
     *            The file path of plugin
     * @return The ZipFile
     */
    public static ZipFile createZipFile(String filePath) {
        if (!isValidZipFileName(filePath)) {
            return null;
        }
        sFileListLock.readLock().lock();
        Iterator<Entry<String, Class<? extends ZipFile>>> itr =
                sFileList.entrySet().iterator();
        while (itr.hasNext()) {
            Entry<String, Class<? extends ZipFile>> entry = itr.next();
            if (filePath.endsWith(entry.getKey())) {
                Constructor<?> cons =
                        ReflectUtils.getConstructor(entry.getValue(), filePath.getClass());
                sFileListLock.readLock().unlock();
                return (ZipFile) ReflectUtils.createInstance(cons, filePath);
            }
        }
        sFileListLock.readLock().unlock();
        return null;
    }

    private static boolean isValidZipFileName(String filePath) {
        FileInputStream fis = null;
        ZipInputStream zipInputStream = null;
        ZipEntry localEntry = null;
        try {
            fis = new FileInputStream(filePath);
            zipInputStream = new ZipInputStream(fis);
            while ((localEntry = zipInputStream.getNextEntry()) != null) {
                String enrtyName = localEntry.getName();
                if (enrtyName.contains("../")) {
                    return false;
                }
            }
        } catch (FileNotFoundException e1) {
            e1.printStackTrace();
            return false;
        } catch (IOException e) {
            e.printStackTrace();
            return false;
        } finally {
            try {
                if (fis != null) {
                    fis.close();
                }
                if (zipInputStream != null) {
                    zipInputStream.close();
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        return true;
    }
}
