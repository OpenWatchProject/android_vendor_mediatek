/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
//#include <netdb.h>
//#include <net/if.h>
//#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <utils/Log.h>

//#define LOG_TAG "CMD_TC"

//#include <forkexecwrap/fork_exec_wrap.h>
#include <logwrap/logwrap.h>

const char * const TC_PATH = "/system/bin/tc";

namespace android {
namespace thermalcmdwrapper {

int system_nosh(const char *command)
{
    char buffer[255];
    char *argp[32];
    int res;
    char *next = buffer;
    char *tmp;
    int i = 0;

    if (!command)           /* just checking... */
        return(1);

    /*
     * The command to argp splitting is from code that was
     * reverted in Change: 11b4e9b2
     */
    if (strnlen(command, sizeof(buffer) - 1) == sizeof(buffer) - 1) {
        //ALOGE("command line too long while processing: %s", command);
        errno = E2BIG;
        return -1;
    }
    strncpy(buffer, command, strlen(command)+1); // Command len is already checked.
    while ((tmp = strsep(&next, " "))) {
        argp[i++] = tmp;
        if (i == 32) {
            //ALOGE("argument overflow while processing: %s", command);
            errno = E2BIG;
            return -1;
        }
    }
    argp[i] = NULL;
    //res = android_fork_execvp(i, (char **)argp, NULL, false, false);
    res = android_fork_execvp_ext(i, (char **)argp, NULL, false,
                                   2, false, NULL,
                                   NULL, 0);
    return res;
}

int execTcCmd(const char *cmd) {
    char *buffer = NULL;
    size_t len = strnlen(cmd, 255);
    int res = 0;

    if (len == 255) {
        ALOGE("tc command too long");
        errno = E2BIG;
        return -1;
    }

    asprintf(&buffer, "%s %s", TC_PATH, cmd);
#ifdef MTK_DEBUG
    ALOGI("execTcCmd, %s\n", buffer);
#endif
    if (buffer != NULL)
    {
        res = system_nosh(buffer);
        free(buffer);
    }
    return res;
}

}  // namespace thermalcmdwrapper
}  // namespace android
