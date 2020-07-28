/*
 * Copyright (C) 2009-2016 The Android Open Source Project
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


#include <errno.h>
#include <malloc.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#define LOG_TAG "NetdagentRespondor"
#include <log/log.h>
#include <CommandRespondor.h>

namespace android {
namespace netdagent {

CommandRespondor::CommandRespondor(int socket) : mConnectSocket(socket) {
    mPid = -1;
    mUid = -1;
    mGid = -1;
    mCmdSeq = 0;

    struct ucred creds;
    socklen_t szCreds = sizeof(creds);
    memset(&creds, 0, szCreds);

    int err = getsockopt(socket, SOL_SOCKET, SO_PEERCRED, &creds, &szCreds);
    if (err == 0) {
        mPid = creds.pid;
        mUid = creds.uid;
        mGid = creds.gid;
    }
}

CommandRespondor::~CommandRespondor() {
        close(mConnectSocket);
}

void CommandRespondor::sendMsg(int code, const char *msg, bool addErrno) {
    char *buf;
    int ret = 0;

    if (addErrno) {
        ret = asprintf(&buf, "%d %d %s (%s)", code, getCmdSeq(), msg, strerror(errno));
    } else {
        ret = asprintf(&buf, "%d %d %s", code, getCmdSeq(), msg);
    }
    // Send the zero-terminated message
    if (ret != -1) {
        if ((ret = sendMsg(buf)) == -1)
            ALOGE("Unable to send msg %s\n", msg);
        free(buf);
    } else
        ALOGE("memory runs out\n");
}

int CommandRespondor::sendMsg(const char *msg) {

    if (mConnectSocket < 0) {
        errno = EHOSTUNREACH;
        return -1;
    }

    struct sigaction new_action, old_action;
    memset(&new_action, 0, sizeof(new_action));
    new_action.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &new_action, &old_action);

#ifdef MTK_DEBUG
    ALOGI("send msg %s via socket %d\n", msg, mConnectSocket);
#endif
    int msgLen = strlen(msg) + 1; // Send null-terminated C strings:
    while (msgLen > 0) {
        ssize_t written = TEMP_FAILURE_RETRY(write(mConnectSocket, msg, msgLen));
        if (written == -1) {
            ALOGE("write error (%s)\n", strerror(errno));
            return -1;
        }
        msg += written;
        msgLen -= written;
    }

    sigaction(SIGPIPE, &old_action, &new_action);
    return 0;
}

}  // namespace netdagent
}  // namespace android
