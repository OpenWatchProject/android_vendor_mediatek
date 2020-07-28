package com.debug.loggerui.file;

import java.io.File;
import java.io.FileOutputStream;
import java.util.Set;

/**
 * @author MTK81255
 *
 */
public interface ILogFile {

    /**
     * @param file File
     * @param filterFileNames Set<String>
     * @return boolean
     */
    public boolean delete(File file, Set<String> filterFileNames);

    /**
     * @param file
     *            File
     * @return boolean
     */
    public boolean createNewFile(File file);

    /**
     * @param file
     *            File
     * @return boolean
     */
    public boolean mkdir(File file);

    /**
     * @param file
     *            File
     * @return boolean
     */
    public boolean mkdirs(File file);

    /**
     * @param file
     *            File
     * @param dest
     *            File
     * @return boolean
     */
    public boolean renameTo(File file, File dest);

    /**
     * @param file
     *            File
     * @return FileOutputStream
     */
    public FileOutputStream getFileOutputStream(File file);
}
