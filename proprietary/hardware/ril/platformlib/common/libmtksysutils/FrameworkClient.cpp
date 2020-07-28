#include <alloca.h>
#include <errno.h>
#include <sys/types.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <log/mtk_log.h>
#include <unistd.h>

#include <sysutils/FrameworkClient.h>

#define LOG_TAG "FrameworkClient"
#define SLOGW(...) (mtkLogW(LOG_TAG, __VA_ARGS__))

FrameworkClient::FrameworkClient(int socket) {
    mSocket = socket;
    pthread_mutex_init(&mWriteMutex, NULL);
}

int FrameworkClient::sendMsg(const char *msg) {
    int ret;
    if (mSocket < 0) {
        errno = EHOSTUNREACH;
        return -1;
    }

    pthread_mutex_lock(&mWriteMutex);
    ret = TEMP_FAILURE_RETRY(write(mSocket, msg, strlen(msg) +1));
    if (ret < 0) {
        SLOGW("Unable to send msg '%s' (%s)", msg, strerror(errno));
    }
    pthread_mutex_unlock(&mWriteMutex);
    return 0;
}

int FrameworkClient::sendMsg(const char *msg, const char *data) {
    size_t bufflen = strlen(msg) + strlen(data) + 1;
    char *buffer = (char *) alloca(bufflen);
    if (!buffer) {
        errno = -ENOMEM;
        return -1;
    }
    snprintf(buffer, bufflen, "%s%s", msg, data);
    return sendMsg(buffer);
}

