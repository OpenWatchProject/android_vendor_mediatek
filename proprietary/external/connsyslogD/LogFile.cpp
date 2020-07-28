/******************************************************************************
 *
 * Filename:
 * ---------
 *   logfile.cpp
 *
 * Description:
 * ------------
 *  
 *
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "LogWriter.h"
#include "ConsysLog.h"
#include "ConsysLogger.h"

namespace consyslogger {

BaseFile::BaseFile(int nBufId) {
    m_nBufId = nBufId;
    m_pFile = 0;
    m_lSize = -1;
    m_nFailedNumber = 0;
}

/**
 * This value only changes after file closed.
 */
long BaseFile::closedSize() {
    struct stat file_stat;
    if (stat(m_szPath, &file_stat) == -1) {
        LOGE("Get the closed size: Failed to get the file status, errno=%d", errno);
        return -1;
    } else {
        LOGV("Get the closed size: the size is %ldB", (long)file_stat.st_size);
        return (long) file_stat.st_size;
    }
}

long BaseFile::size() {
    return m_lSize;
}

bool BaseFile::openFile(const char *path) {
    memset(m_szPath, 0, sizeof(m_szPath));
    strncpy(m_szPath, path, strlen(path));
    m_szPath[PATH_LEN - 1] = '\0';
    m_pFile = open(m_szPath, O_WRONLY|O_CREAT|O_APPEND|O_TRUNC,
            S_IRWXU|S_IWGRP|S_IRGRP|S_IROTH);
    m_lSize = 0;
    if (m_pFile < 0) {
        if (errno == EINTR) {
            int nRetry = 10;
            while (nRetry-- > 0) {
                usleep(1000);
                if ((m_pFile = open(m_szPath, O_WRONLY|O_CREAT|O_APPEND,
                        S_IRWXU|S_IWGRP|S_IRGRP| S_IROTH)) >= 0) {
                    m_nFailedNumber = 0;
                    if (isEngineerBuild()){
                    LOGD("Open log file %s.", m_szPath);
                    }
                    m_lSize = 0;
                    return true;
                }
            }
            LOGE("retry access file fail: %s. errno = %d", m_szPath, errno);
        }
        LOGE("Failed to open/create %s, errno=%d", m_szPath, errno);
        return false;
    } else {
        m_nFailedNumber = 0;
        if(isEngineerBuild()){
        LOGD("Open log file %s", m_szPath);
        }
        // Current logging speed is not fast while IO block size is large(32768B on test SD card)
        // So we use Non-buf IO
        //setbuf(m_pFile, NULL);
        return true;
    }
}

bool BaseFile::closeFile() {
    LOGV("Close file %s.", m_szPath);
    if (m_pFile <= 0) {
        LOGE("Close log file: File pointer is null.");
        return false;
    }

    if (close(m_pFile) == -1) {
        LOGE("Close log file: Failed to close file %s, errno=%d", m_szPath, errno);
        m_pFile = 0;
        return false;
    } else {
        LOGD("Close log file: Success.");
        m_pFile = 0;
        return true;
    }
}

bool BaseFile::cleanCloseFile() {
    LOGV("Clean Close file %s.", m_szPath);
    long fileSize;
    if (!closeFile()) {
        LOGE("Failed to close file %s, errno:%d, %s",
                m_szPath, errno, strerror(errno));
        return false;
    }
    fileSize = closedSize();
    if (0 == fileSize) {
        //Delete the file if it has no content
        if (remove(m_szPath) < 0) {
            LOGE("Failed to delete emoty file %s, errno=%d", m_szPath, errno);
            return false;
        } else {
            LOGD("Deleted empty file %s.", m_szPath);
        }
    } else {
        char newName[BaseFile::PATH_LEN];

        memset(newName, '\0', BaseFile::PATH_LEN);
        strncpy(newName, m_szPath, strlen(m_szPath) - strlen(".curf"));
        newName[sizeof(newName) - 1] = '\0';

        LOGD("File's new name is: %s", newName);
        int err = rename(m_szPath, newName);

        if (err < 0) {
            LOGE("rename file error. %d", err);
            return false;
        }
        memset(m_szPath, '\0', BaseFile::PATH_LEN);
        strncpy(m_szPath, newName, sizeof(m_szPath) - 1);
        m_szPath[sizeof(m_szPath) - 1] = '\0';
    }
    return true;
}


unsigned int BaseFile::writeFile(const char *ptr, unsigned int len) {
    LOGV("Write log file %s", m_szPath);
    if (m_pFile <= 0) {
        LOGE("Try to write log file, but the file pointer is null.");
        return false;
    }

    size_t res;
    unsigned int bytesWriten = 0;
    while (bytesWriten < len) {
        res = write(m_pFile, ptr + bytesWriten, len - bytesWriten);
        if (res == 0) {
            LOGE("Failed to write log file %s, errno=%d", m_szPath, errno);
            m_nFailedNumber++;
            if (m_nFailedNumber >= MAX_NUMBER_FAILED) {
                //TODO:notify user write failed?
            }
            break;
        } else {
            bytesWriten += res;
            // Count size
            m_lSize += res;
            m_nFailedNumber = 0;
        }
    }
    LOGV("Success to write log file %s by %u bytes", m_szPath, bytesWriten);
    return bytesWriten;
}


bool BaseFile::flushFile() {
    return true;
}

bool BaseFile::isExceeded() {
    return false;
}

bool BaseFile::isExist() {
    if (0 == access(m_szPath, F_OK)) {
        return true;
    }
    LOGE("access file fail: %s. errno = %d", m_szPath, errno);

    if (errno == EINTR) {
        int nRetry = 10;
        while (nRetry-- > 0) {
            if (0 == access(m_szPath, F_OK)) {
                return true;
            }
            usleep(1000);
        }
        LOGE("retry access file fail: %s. errno = %d", m_szPath, errno);
    }
    return false;
}

long LogFile::m_capacity = 1024 * 1024 * 20;
//const char FileHdr[] = { 0x02, 0x00, 0x00, 0x00};

LogFile::LogFile(int nBufId) :
        BaseFile(nBufId) {
}

bool LogFile::openFile(const char *path) {
    bool ret = false;
    ret = BaseFile::openFile(path);
  /*  if (ret == true) {
        unsigned int bytesWriten = BaseFile::writeFile(FileHdr,sizeof(FileHdr));
        if (bytesWriten == false) {
            ret = false;
        }
    }*/

    return ret;
}

bool LogFile::cleanCloseFile() {
    if(isEngineerBuild()){
    LOGD("Clean Close file %s.", m_szPath);
    }

    long fileSize;
    if (!BaseFile::closeFile()) {
        return false;
    }
    fileSize = closedSize();
    if (fileSize <= 0) {
        //Delete the file if it has no content
        if (remove(m_szPath) < 0) {
            LOGE("Failed to delete empty log file %s, errno=%d", m_szPath, errno);
            return false;
        } else {
            LOGD("Success to delete empty log file %s.", m_szPath);
        }
    } else {
        char newName[BaseFile::PATH_LEN];

        memset(newName, '\0', BaseFile::PATH_LEN);
        strncpy(newName, m_szPath, strlen(m_szPath) - strlen(".curf"));

        LOGD("File's new name is: %s", newName);
        int err = rename(m_szPath, newName);

        if (err < 0) {
            LOGE("rename file error. %d", err);
            return false;
        }
        memset(m_szPath, '\0', BaseFile::PATH_LEN);
        strncpy(m_szPath, newName, sizeof(m_szPath) - 1);
        m_szPath[sizeof(m_szPath) - 1] = '\0';
    }
    return true;
}

bool LogFile::isExceeded() {
    if (size() >= m_capacity) {
        if (isEngineerBuild()) {
        LOGD("Log file %s exceeds size limit %ldB.",
                m_szPath, m_capacity);
        }
        return true;
    } else {
        return false;
    }
}

bool LogFile::setCapacity(long size) {
    m_capacity = size;
    return true;
}

long LogFile::getCapacity() {
    return m_capacity;
}

}
