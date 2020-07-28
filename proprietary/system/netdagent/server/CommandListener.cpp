/*
 * Copyright (C) 2008 The Android Open Source Project
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
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <linux/if.h>
#include <cutils/sockets.h>

#include <NetdagentUtils.h>
#include <CommandListener.h>
#define LOG_TAG "NetdagentListener"
#include "log/log.h"

namespace android {
namespace netdagent {

CommandListener::CommandListener(const char *socketName) {
    init(socketName, -1);
    registerCmd(new FirewallCmd());
    registerCmd(new ThrottleCmd());
    registerCmd(new NetworkCmd());
#if 0
    registerCmd(new ThroughputCmd());
    registerCmd(new PerfCmd());
    registerCmd(new BridgeCmd());
#endif
}

void CommandListener::init(const char *socketName, int socket) {
    mListenSocketName = socketName;
    mListenSocket = socket;
    mThread = 0;
    mCommandRespondorList = new tCommandRespondorList();
    mCommandDispatchList = new tCommandDispatchList();
}

CommandListener::~CommandListener() {
    tCommandRespondorList::iterator it_cr;
    tCommandDispatchList::iterator it_cd;
    if (mListenSocketName && (mListenSocket > -1))
        close(mListenSocket);
    for (it_cr = mCommandRespondorList->begin(); it_cr != mCommandRespondorList->end();) {
        delete (*it_cr);
        it_cr = mCommandRespondorList->erase(it_cr);
    }
    delete mCommandRespondorList;

    for (it_cd = mCommandDispatchList->begin(); it_cd != mCommandDispatchList->end();) {
        delete (*it_cd);
        it_cd = mCommandDispatchList->erase(it_cd);
    }
    delete mCommandDispatchList;
}

int CommandListener::startListener() {

    if (!mListenSocketName && mListenSocket == -1) {
        ALOGE("failed to start unbound listener\n");
        errno = EINVAL;
        return -1;
    }

    if (mListenSocketName) {
        if ((mListenSocket = socket_local_server(mListenSocketName,
                       ANDROID_SOCKET_NAMESPACE_ABSTRACT, SOCK_STREAM | SOCK_CLOEXEC)) == -1) {
            ALOGE("failed to create local socket for %s\n", mListenSocketName);
            return -1;
        }
        ALOGI("mListenSocket %d for mListenSocketName %s\n", mListenSocket, mListenSocketName);
    }

    if (pthread_create(&mThread, NULL, CommandListener::threadStart, this)) {
        ALOGE("pthread_create (%s)", strerror(errno));
        return -1;
    }
	return 0;
}

void *CommandListener::threadStart(void *obj) {
    CommandListener *me = reinterpret_cast<CommandListener *>(obj);

    me->runListener();
    pthread_exit(NULL);
    return NULL;
}

void CommandListener::runListener() {
    tCommandRespondorList pendingList;

    while (1) {
        int rc = 0;
        int max_fd = -1;
        fd_set read_fds;
        tCommandRespondorList::iterator it;

        FD_ZERO(&read_fds);
        FD_SET(mListenSocket, &read_fds);
        if (mListenSocket > max_fd)
            max_fd = mListenSocket;
        for (it = mCommandRespondorList->begin(); it != mCommandRespondorList->end(); ++it) {
            int connectSocket = (*it)->getConnectSocket();
            FD_SET(connectSocket, &read_fds);
            if (connectSocket > max_fd)
                max_fd = connectSocket;
        }

        if ((rc = select(max_fd + 1, &read_fds, NULL, NULL, NULL)) < 0) {
            if (errno == EINTR)
                continue;
            ALOGE("select failed (%s)\n", strerror(errno));
            sleep(1);
            continue;
        } else if (!rc)
            continue;

        //handle listen socket
        if (FD_ISSET(mListenSocket, &read_fds)) {
            sockaddr_storage peerAddrStorage;
            sockaddr *peerAddr = reinterpret_cast<sockaddr*>(&peerAddrStorage);
            socklen_t peerAddrLen;
            int connectSocket;
            do {
                peerAddrLen = sizeof(peerAddrStorage);
                connectSocket = accept(mListenSocket, peerAddr, &peerAddrLen);
            } while (connectSocket < 0 && errno == EINTR);
            if (connectSocket < 0) {
                ALOGE("accept failed (%s)\n", strerror(errno));
                sleep(1);
                continue;
            }
            ALOGI("connected socket %d\n", connectSocket);
            mCommandRespondorList->push_back(new CommandRespondor(connectSocket));
        }

        //handle connect socket
        pendingList.clear();
        for (it = mCommandRespondorList->begin(); it != mCommandRespondorList->end(); ++it) {
            int connectSocket = (*it)->getConnectSocket();
            if (FD_ISSET(connectSocket, &read_fds))
                pendingList.push_back(*it);
        }
        while (!pendingList.empty()) {
            it = pendingList.begin();
            CommandRespondor *cr = *it;
            pendingList.erase(it);
            if (!handleEvents(cr))
                releaseSockets(cr);
            }
        }//end while(1)
}

bool CommandListener::handleEvents(CommandRespondor *cr) {
    /* store command */
    int readCmdLen;
    char cmdBuffer[CMD_ARG_SIZE];
    /* parse command */
    const char *delim = " ";
    char *token = nullptr;
    char *argv[CMD_ARG_COUNT];
    int argc = 0;
    /* caculate command sequence */
    char *endPtr = NULL;
    unsigned int cmdSeq;
    tCommandDispatchList::iterator it;

    //read command
    readCmdLen = TEMP_FAILURE_RETRY(read(cr->getConnectSocket(), cmdBuffer, sizeof(cmdBuffer)));
    if (readCmdLen < 0) {
        ALOGE("read() failed (%s)", strerror(errno));
        return false;
    } else if (!readCmdLen) {
        return false;
    } else if (cmdBuffer[readCmdLen-1] != '\0') {
        ALOGE("String is not zero-terminated");
        cr->sendMsg(500, "Command too large for buffer", false);
        return false;
    }
#ifdef MTK_DEBUG
    ALOGI("Netdagent command %s from socket %d\n", cmdBuffer, cr->getConnectSocket());
#endif
    //parse command
    token = strtok(cmdBuffer, delim);
    while (token != NULL) {
        if (argc >= CMD_ARG_COUNT) {
            ALOGE("Command contains too many parameters\n");
            cr->sendMsg(500, "Command contains too many parameters", false);
            goto _release;
        }
        argv[argc++] = strdup(token);
        token = strtok(NULL, delim);
    }
    //get command sequence
    cmdSeq = strtoul(argv[0], &endPtr, 0);
    if (endPtr == NULL || *endPtr != '\0') {
        ALOGE("Command contains invalid sequence number\n");
        cr->sendMsg(500, "Command contains invalid sequence number", false);
        goto _release;
    }
    cr->setCmdSeq(cmdSeq);
    //dispatch command
    //pthread_mutex_lock(&android::netdagent::gLock);
    for (it = mCommandDispatchList->begin(); it != mCommandDispatchList->end(); ++it) {
        if (!strcmp(argv[1], (*it)->getCommand())) {
            if ((*it)->runCommand(cr, argc-1, &argv[1]))
                ALOGE("run command %s failed (%s)\n", (*it)->getCommand(), strerror(errno));
            goto _release;
        }
    }
    //pthread_mutex_unlock(&android::netdagent::gLock);
    cr->sendMsg(500, "Command not recognized", false);

_release:
    int i;
    for (i = 0; i < argc; i++)
        free(argv[i]);
    return true;
}

void CommandListener::releaseSockets(CommandRespondor *cr)
{
    tCommandRespondorList::iterator it;
    for (it = mCommandRespondorList->begin(); it != mCommandRespondorList->end(); ++it) {
        if (*it == cr) {
#ifdef MTK_DEBUG
            ALOGI("release sockets %d\n", (*it)->getConnectSocket());
#endif
            mCommandRespondorList->erase(it);
            delete (*it);
            break;
        }
    }
}

}  // namespace netdagent
}  // namespace android
