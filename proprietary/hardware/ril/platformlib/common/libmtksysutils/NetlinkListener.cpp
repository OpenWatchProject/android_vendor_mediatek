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
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <string.h>

#define LOG_TAG "NetlinkListener"
#include <log/mtk_log.h>
#include <cutils/uevent.h>

#include <sysutils/NetlinkEvent.h>
#include <cutils/fs.h>

#define SLOGE(...) (mtkLogE(LOG_TAG, __VA_ARGS__))

ssize_t uevent_kernel_recv(int socket, void *buffer, size_t length, bool require_group, uid_t *uid)
{
    struct iovec iov = { buffer, length };
    struct sockaddr_nl addr;
    char control[CMSG_SPACE(sizeof(struct ucred))];
    struct msghdr hdr = {
        &addr,
        sizeof(addr),
        &iov,
        1,
        control,
        sizeof(control),
        0,
    };

    *uid = -1;
    ssize_t n = recvmsg(socket, &hdr, 0);
    if (n <= 0) {
        return n;
    }

    struct cmsghdr *cmsg = CMSG_FIRSTHDR(&hdr);

    struct ucred *cred = (struct ucred *)CMSG_DATA(cmsg);
    if (cmsg == NULL || cmsg->cmsg_type != SCM_CREDENTIALS) {
        /* ignoring netlink message with no sender credentials */
        goto out;
    }

//    struct ucred *cred = (struct ucred *)CMSG_DATA(cmsg);
    *uid = cred->uid;
    if (cred->uid != 0) {
        /* ignoring netlink message from non-root user */
        goto out;
    }

    if (addr.nl_pid != 0) {
        /* ignore non-kernel */
        goto out;
    }
    if (require_group && addr.nl_groups == 0) {
        /* ignore unicast messages when requested */
        goto out;
    }

    return n;

out:
    /* clear residual potentially malicious data */
    memset(buffer, 0, length);
    errno = EIO;
    return -1;
}

#if 1
/* temporary version until we can get Motorola to update their
 * ril.so.  Their prebuilt ril.so is using this private class
 * so changing the NetlinkListener() constructor breaks their ril.
 */
NetlinkListener::NetlinkListener(int socket) :
                            SocketListener(socket, false) {
    mFormat = NETLINK_FORMAT_ASCII;
}
#endif

NetlinkListener::NetlinkListener(int socket, int format) :
                            SocketListener(socket, false), mFormat(format) {
}

bool NetlinkListener::onDataAvailable(SocketClient *cli)
{
    int socket = cli->getSocket();
    ssize_t count;
    uid_t uid = -1;

    bool require_group = true;
    if (mFormat == NETLINK_FORMAT_BINARY_UNICAST) {
        require_group = false;
    }

    count = TEMP_FAILURE_RETRY(uevent_kernel_recv(socket,
            mBuffer, sizeof(mBuffer), require_group, &uid));
    if (count < 0) {
        SLOGE("recvmsg failed (%s)", strerror(errno));
        return false;
    }

    NetlinkEvent *evt = new NetlinkEvent();
    if (evt->decode(mBuffer, count, mFormat)) {
        onEvent(evt);
    } else if (mFormat != NETLINK_FORMAT_BINARY) {
        // Don't complain if parseBinaryNetlinkMessage returns false. That can
        // just mean that the buffer contained no messages we're interested in.
        SLOGE("Error decoding NetlinkEvent");
    }

    delete evt;
    return true;
}
