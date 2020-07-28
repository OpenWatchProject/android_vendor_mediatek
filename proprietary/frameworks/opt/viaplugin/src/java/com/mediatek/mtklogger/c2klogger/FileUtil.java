package com.mediatek.mtklogger.c2klogger;

import android.os.Environment;
import android.util.Log;

import java.io.File;
import java.io.FilenameFilter;
import java.util.ArrayList;


/**
 * @author MTK81255
 *
 */
public class FileUtil {

    /**
     * @return String
     */
    public static String getCbpDir() {
        // /mnt/sdcard/
        FilenameFilter sdFilter = new FilenameFilter() {
            public boolean accept(File dir, String name) {
                String lowercaseName = name.toLowerCase();
                if (lowercaseName.indexOf("sdcard") >= 0) {
                    return true;
                } else {
                    return false;
                }
            }
        };

        File dirSdCard = new File("/mnt/");
        File[] filesSdCard = dirSdCard.listFiles(sdFilter);

        if (filesSdCard != null) {
            for (File file : filesSdCard) {
                Log.v("via_ets", file.getPath());

                String pathCbp = file.getPath() + "/cbp";
                File fileCbp = new File(pathCbp);
                if (fileCbp.exists()) {
                    return pathCbp;
                }
            }
        }

        return null;
    }

    /**
     * @return boolean
     */
    public static boolean sdCardExist() {
        if (Environment.getExternalStorageState().equals(
                Environment.MEDIA_MOUNTED)) {
            Log.d("via_ets", "sdCardExist true");
            return true;
        } else {
            Log.d("via_ets", "sdCardExist false");
            return false;
        }
    }

    /**
     * @param path String
     * @return String
     */
    public static String getExtOfPath(String path) {
        String ext = "";
        if ((path != null) && (path.length() > 0)) {
            int i = path.lastIndexOf('.');
            if ((i > -1) && (i < (path.length()))) {
                ext = path.substring(0, i);
            }
        }
        return ext;
    }

    /**
     * @param path String
     * @return String
     */
    public static String getDirOfPath(String path) {
        String dir = "";
        if ((path != null) && (path.length() > 0)) {
            int i = path.lastIndexOf('/');
            if ((i > -1) && (i < (path.length()))) {
                dir = path.substring(0, i + 1);
            }
        }
        return dir;
    }

    /**
     * @param pathDir String
     * @param fileNames ArrayList<String>
     * @param filePaths ArrayList<String>
     * @return boolean
     */
    public static boolean getAllFileAndPath(String pathDir,
            ArrayList<String> fileNames, ArrayList<String> filePaths) {
        fileNames.clear();
        filePaths.clear();

        File dir = new File(pathDir);
        File[] files = dir.listFiles();

        if (files != null) {
            for (File file : files) {
                fileNames.add(file.getName());
                filePaths.add(file.getPath());
            }
        } else {
            return false;
        }

        return true;
    }

    /**
     * @param pathDev String
     * @param timeoutS int
     * @return boolean
     */
    public static boolean waitFileOccur(String pathDev, int timeoutS) {
        for (int i = 0; i < timeoutS * 2; ++i) {
            if (new File(pathDev).exists()) {
                return true;
            }

            try {
                Thread.sleep(500);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

        return false;
    }

}