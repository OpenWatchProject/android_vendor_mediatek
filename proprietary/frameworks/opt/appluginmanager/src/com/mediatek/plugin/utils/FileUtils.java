package com.mediatek.plugin.utils;

import java.io.File;
import java.util.ArrayList;

/**
 * Utils to operate File.
 */
public class FileUtils {

    /**
     * Get all files in directory, include the files in sub_dir.
     * @param dirPath
     *            The path of directory
     * @return The file path list
     */
    public static ArrayList<String> getAllFile(String dirPath) {
        ArrayList<String> res = new ArrayList<String>();
        if (dirPath == null || dirPath.equals("")) {
            return res;
        }

        File dir = new File(dirPath);
        if (!dir.isDirectory()) {
            return res;
        }

        ArrayList<File> needProcess = new ArrayList<File>();
        needProcess.add(dir);

        for (int i = 0; i < needProcess.size(); i++) {
            File current = needProcess.get(i);
            if (current.isFile()) {
                res.add(current.getAbsolutePath());
            } else {
                File[] fileInDir = current.listFiles();
                if (fileInDir == null) {
                    continue;
                }
                for (File file : fileInDir) {
                    needProcess.add(file);
                }
            }
        }
        return res;
    }
}
