/******************************************************************************
 *
 * Filename:
 * ---------
 *   logwriter.h
 *
 * Description:
 * ------------
 *   Log recycle operation interface definitions
 *
 *
 *****************************************************************************/
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include "LogWriter.h"
#include "ConsysLog.h"


bool setLogRecycleSize(int size) {
    int fd = -1;
    if (access(CONSYSLOGGER_RECYCLE_SIZE_FILE, F_OK) == 0) {
        fd = open(CONSYSLOGGER_RECYCLE_SIZE_FILE, O_RDWR|O_SYNC);
    } else {
        fd = creat(CONSYSLOGGER_RECYCLE_SIZE_FILE, 0664);
       // chown(CONSYSLOGGER_RECYCLE_SIZE_FILE, 2000, 1000);
    }
    // Configuration file not exist, create it

    if (fd != -1) {
        // Initialize the content
        if (write(fd, &(size), sizeof(int)) == sizeof(int)) {
            LOGD("Default recycle size is set to %d", size);
        } else {
            LOGE("write Log recycle size data length error");
        }
        close(fd);
        return true;
    } else {
        LOGE("Failed to create Logger recycle size file %s, errno=%d", CONSYSLOGGER_RECYCLE_SIZE_FILE, errno);
    }

    return false;
}

int getLogRecycleSize() {
    int fd = 0;
    int ret = 2000;
    int size = 0;
    ssize_t retvalue;
    if ((fd = open(CONSYSLOGGER_RECYCLE_SIZE_FILE, O_RDONLY)) > 0) {
        // Read the logging mode set for next time
        retvalue = read(fd, &(size), sizeof(int));
        if (-1 == retvalue) {
            LOGE("Failed to read log recycle size from config file, errno=%d", retvalue);
        } else if (sizeof(int) == (unsigned int) retvalue) {
            ret = size;
        } else {
            LOGE("Failed to read log recycle size data from config file, returned %ld", (long)retvalue);
        }
        close(fd);

    } else {
        LOGE("Failed to open Logger log recycle file %s, errno=%d", CONSYSLOGGER_RECYCLE_SIZE_FILE, errno);
        ret = 2000;
    }
    LOGV("Get recycle size: %d M", ret);
    return ret;
}

