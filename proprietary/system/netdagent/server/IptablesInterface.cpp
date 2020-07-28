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
#include <netdb.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/socket.h>

#define LOG_TAG "NetdagentIptables"
#include "log/log.h"
#include <forkexecwrap/fork_exec_wrap.h>
#include "IptablesInterface.h"

namespace android {
namespace netdagent {

static void logExecError(const char* argv[], int res, int status) {
    const char** argp = argv;
    std::string args = "";
    while (*argp) {
        args += *argp;
        args += ' ';
        argp++;
    }
    ALOGE("exec() res=%d, status=%d for %s", res, status, args.c_str());
}

static int execCommand(int argc, const char *argv[], bool silent) {
    int res;
    int status;

    res = android_fork_execvp(argc, (char **)argv, &status, false, !silent);
    if (res || !WIFEXITED(status) || WEXITSTATUS(status)) {
        if (!silent) {
            logExecError(argv, res, status);
        }
        if (res)
            return res;
        if (!WIFEXITED(status))
            return ECHILD;
    }
    return WEXITSTATUS(status);
}

static int execIptables(IptablesTarget target, bool silent, va_list args) {
    /* Read arguments from incoming va_list; we expect the list to be NULL terminated. */
    std::list<const char*> argsList;
    argsList.push_back(NULL);
    const char* arg;

    // Wait to avoid failure due to another process holding the lock
    argsList.push_back("-w");

    do {
        arg = va_arg(args, const char *);
        argsList.push_back(arg);
    } while (arg);

    int i = 0;
    const char* argv[argsList.size()];
    std::list<const char*>::iterator it;
    for (it = argsList.begin(); it != argsList.end(); it++, i++) {
        argv[i] = *it;
    }

#ifdef MTK_DEBUG
    const char** temp = argv + 1; //skip argv[0]
    std::string debug = "";
    while (*temp) {
        debug += *temp;
        debug += " ";
        temp++;
    }
    ALOGI("execIptables %s\n", debug.c_str());
#endif

    int res = 0;
    if (target == V4 || target == V4V6) {
        argv[0] = IPTABLES_PATH;
        res |= execCommand(argsList.size(), argv, silent);
    }
    if (target == V6 || target == V4V6) {
        argv[0] = IP6TABLES_PATH;
        res |= execCommand(argsList.size(), argv, silent);
    }
    return res;
}

int execIptables(IptablesTarget target, ...) {
    va_list args;
    va_start(args, target);
    int res = execIptables(target, false, args);
    va_end(args);
    return res;
}

int execIptablesSilently(IptablesTarget target, ...) {
    va_list args;
    va_start(args, target);
    int res = execIptables(target, true, args);
    va_end(args);
    return res;
}

static int execNdcCmd(const char *command, bool silent, va_list args) {
    /* Read arguments from incoming va_list; we expect the list to be NULL terminated. */
    std::list<const char*> argsList;
    argsList.push_back(NULL);
    const char* arg;
    argsList.push_back(command);
    do {
        arg = va_arg(args, const char *);
        argsList.push_back(arg);
    } while (arg);

    int i = 0;
    const char* argv[argsList.size()];
    std::list<const char*>::iterator it;
    for (it = argsList.begin(); it != argsList.end(); it++, i++) {
        argv[i] = *it;
    }

#ifdef MTK_DEBUG
    const char** temp = argv + 1; //skip argv[0]
    std::string debug = "";
    while (*temp) {
        debug += *temp;
        debug += " ";
        temp++;
    }
    ALOGI("execNdcCmd %s\n", debug.c_str());
#endif

    int res = 0;
    argv[0] = NDC_PATH;
    res = execCommand(argsList.size(), argv, silent);
    return res;
  
}

int execNdcCmd(const char *command, ...) {
    va_list args;
    va_start(args, command);
    int res = execNdcCmd(command, false, args);
    va_end(args);
    return res;
}

static int execIpCmd(int family, bool silent, va_list args) {
    /* Read arguments from incoming va_list; we expect the list to be NULL terminated. */
    std::list<const char*> argsList;
    argsList.push_back(NULL);
    argsList.push_back(NULL);
    const char* arg;

    do {
        arg = va_arg(args, const char *);
        argsList.push_back(arg);
    } while (arg);

    int i = 0;
    const char* argv[argsList.size()];
    std::list<const char*>::iterator it;
    for (it = argsList.begin(); it != argsList.end(); it++, i++) {
        argv[i] = *it;
    }

#ifdef MTK_DEBUG
    const char** temp = argv + 2; //skip argv[0] and argv[1]
    std::string debug = "";
    while (*temp) {
        debug += *temp;
        debug += " ";
        temp++;
    }
    ALOGI("execIpCmd %s\n", debug.c_str());
#endif

    int res = 0;
    argv[0] = IP_PATH;
    if (family == AF_INET) {
        argv[1] = "-4";
        res |= execCommand(argsList.size(), argv, silent);
    }
    if (family == AF_INET6) {
        argv[1] = "-6";
        res |= execCommand(argsList.size(), argv, silent);
    }
    return res;
  
}

int execIpCmd(int family, ...) {
    va_list args;
    va_start(args, family);
    int res = execIpCmd(family, false, args);
    va_end(args);
    return res;
}

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
    res = android_fork_execvp(i, (char **)argp, NULL, false, false);
    return res;
}

int execTcCmd(const char *cmd) {
    char *buffer;
    size_t len = strnlen(cmd, 255);
    int res;

    if (len == 255) {
        ALOGE("tc command too long");
        errno = E2BIG;
        return -1;
    }

    asprintf(&buffer, "%s %s", TC_PATH, cmd);
#ifdef MTK_DEBUG
    ALOGI("execTcCmd, %s\n", buffer);
#endif
    res = system_nosh(buffer);
    free(buffer);
    return res;
}

void createChildChains(IptablesTarget target, const char* table, const char* op, const char* parentChain,
        const char** childChains) {
    const char** childChain = childChains;
    do {
        // Order is important:
        // -D to delete any pre-existing jump rule (removes references
        //    that would prevent -X from working)
        // -F to flush any existing chain
        // -X to delete any existing chain
        // -N to create the chain
        // -A to append the chain to parent

        execIptablesSilently(target, "-t", table, "-D", parentChain, "-j", *childChain, NULL);
        execIptablesSilently(target, "-t", table, "-F", *childChain, NULL);
        execIptablesSilently(target, "-t", table, "-X", *childChain, NULL);
        execIptables(target, "-t", table, "-N", *childChain, NULL);
        execIptables(target, "-t", table, op, parentChain, "-j", *childChain, NULL);
    } while (*(++childChain) != NULL);
}

}  // namespace netdagent
}  // namespace android


