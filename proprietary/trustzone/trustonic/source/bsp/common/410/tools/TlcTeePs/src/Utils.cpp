/*
 * Copyright (c) 2015-2016 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <sys/wait.h>
#include "log.h"
#include "Utils.h"

#define COPY_BUFFER_SIZE 256

void clear_terminal() {
    printf("\033[2J\033[H");
}

void getCurrentDateTime(char* dst) {
    time_t now = ::time(0);
    struct tm *tstruct = ::localtime(&now);
    if (tstruct != NULL) {
        (void)::strftime(dst, DATE_BUFFER_SIZE, DATE_FORMAT, tstruct);
    } else {
        IP_LOG_E("Can't update time");
    }
}

int showFileContent(const char* file_path) {
    FILE *fd = ::fopen(file_path, "r");
    if (fd == NULL) {
        LOG_ERRNO("open");
        return -1;
    }

    // We don't know the size in case of a debugfs file => size = 0
    // Read the file BUFFER_SIZE-1 char at the time until we reach the end
    char buffer[COPY_BUFFER_SIZE];
    size_t read;
    do {
        // Read BUFFER_SIZE-1 char max and add \0 at the end to print the string
        read = ::fread(buffer, 1, COPY_BUFFER_SIZE-1, fd);
        buffer[read] = '\0';
        printf("%s", buffer);
    } while (read == COPY_BUFFER_SIZE-1);

    if(::fclose(fd)){
        LOG_ERRNO("close");
        return -1;
    }

    return 0;
}
