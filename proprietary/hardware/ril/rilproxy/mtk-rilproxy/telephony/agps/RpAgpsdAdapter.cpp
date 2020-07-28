/*****************************************************************************
 * Include
 *****************************************************************************/
#include <string.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <poll.h>
#include "RpAgpsdAdapter.h"
#include "RpAgpsHandler.h"
#include "RfxMainThread.h"
#include "RpAgpsLog.h"

/*****************************************************************************
 * Class RpAgpsdAdapter
 *****************************************************************************/
RFX_IMPLEMENT_CLASS("RpAgpsdAdapter", RpAgpsdAdapter, RfxObject);
RFX_OBJ_IMPLEMENT_SINGLETON_CLASS(RpAgpsdAdapter);


RpAgpsdAdapter::RpAgpsdAdapter() :
    m_inFd(-1),
    m_looperCallback(NULL),
    m_testMode(false) {
    AGPS_LOGD("RpAgpsdAdapter Constructor 0x%x", this);
}


RpAgpsdAdapter::~RpAgpsdAdapter() {
    // Singletone Class, Destructor will not be called
    AGPS_LOGD("RpAgpsdAdapter Destructor 0x%x", this);
}


void RpAgpsdAdapter::putByte(char* buff, int* offset, const char input) {
    *((char*)&buff[*offset]) = input;
    *offset += 1;
}


void RpAgpsdAdapter::putInt16(char* buff, int* offset, const int16_t input) {
    putByte(buff, offset, input & 0xff);
    putByte(buff, offset, (input >> 8) & 0xff);
}


void RpAgpsdAdapter::putInt32(char* buff, int* offset, const int32_t input) {
    putInt16(buff, offset, input & 0xffff);
    putInt16(buff, offset, (input >> 16) & 0xffff);
}


char RpAgpsdAdapter::getByte(char* buff, int* offset) {
    char ret = buff[*offset];
    *offset += 1;
    return ret;
}


int16_t RpAgpsdAdapter::getInt16(char* buff, int* offset) {
    int16_t ret = 0;
    ret |= getByte(buff, offset) & 0xff;
    ret |= (getByte(buff, offset) << 8);
    return ret;
}


int32_t RpAgpsdAdapter::getInt32(char* buff, int* offset) {
    int32_t ret = 0;
    ret |= getInt16(buff, offset) & 0xffff;
    ret |= (getInt16(buff, offset) << 16);
    return ret;
}


int RpAgpsdAdapter::createSocket(
        const char *name,
        int type,
        struct sockaddr_un *addr,
        socklen_t *len) {
    int s = socket(AF_UNIX, type, 0);
    if (s < 0) {
        AGPS_LOGE("RpAgpsdAdapter::createSocket Error[%s]",
            strerror(errno));
        return -1;
    }
    int err = makeAddress(name, addr, len);
    if (err < 0) {
        close(s);
        return -1;
    }
    return s;
}


int RpAgpsdAdapter::makeAddress(
        const char *name,
        struct sockaddr_un *addr,
        socklen_t *len) {
    size_t namelen;
    memset(addr, 0, sizeof(struct sockaddr_un));
    namelen = strlen(name);
    if (namelen + 1 > sizeof(addr->sun_path))
    {
        AGPS_LOGE("RpAgpsdAdapter::makeAddress Error");
        return -1;
    }
    addr->sun_path[0] = 0;
    memcpy(&addr->sun_path[1], name, namelen);
    addr->sun_family = AF_UNIX;
    *len = namelen + offsetof(struct sockaddr_un, sun_path) + 1;
    return 0;
}


int RpAgpsdAdapter::createOutSocket() {
    struct sockaddr_un addr;
    socklen_t len;
    const char *name;
    if (m_testMode) {
        name = c_strOutSocketTest;
    } else {
        name = c_strOutSocket;
    }
    int s = createSocket(name, SOCK_STREAM, &addr, &len);
    if (s < 0) {
        return -1;
    }
    if(connect(s, (struct sockaddr *) &addr, len) < 0) {
        AGPS_LOGE("RpAgpsdAdapter::createOutSocket Connect Error[%s]",
            strerror(errno));
        close(s);
        return -1;
    }
    return s;
}


int RpAgpsdAdapter::createInSocket() {
    struct sockaddr_un addr;
    socklen_t len;
    const char *name;
#ifdef __AUTO_TEST_MODE__
    name = c_strInSocketTest;
#else
    name = c_strInSocket;
#endif
    int s = createSocket(name, SOCK_DGRAM, &addr, &len);
    if (s < 0) {
        return -1;
    }
    int n = 1;
    int ret = setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &n, sizeof(n));
    if (ret < 0) {
        AGPS_LOGE("RpAgpsdAdapter::createInSocket setOpt Error[%s]",
            strerror(errno));
        close(s);
        return -1;
    }
    if (bind(s, (struct sockaddr *) &addr, len) < 0) {
        AGPS_LOGE("RpAgpsdAdapter::createInSocket Bind Error[%s][%s]",
            strerror(errno), name);
        close(s);
        return -1;
    }
    return s;
}


int RpAgpsdAdapter::safeWrite(int fd, void* buf, int len) {
    int n, retry = 10;

    if(fd < 0 || buf == NULL || len < 0) {
        AGPS_LOGE("RpAgpsdAdapter::safeWrite fd=%d buf=%p len=%d\n",
            fd, buf, len);
        return -1;
    }

    while((n = write(fd, buf, len)) != len) {
        if(errno == EINTR) continue;
        if(errno == EAGAIN) {
            if(retry-- > 0) {
                usleep(100 * 1000);
                continue;
            }
            goto exit;
        }
        goto exit;
    }
    return n;
exit:
    AGPS_LOGE("RpAgpsdAdapter reason=[%s]%d\n", strerror(errno), errno);
    return -1;
}


int RpAgpsdAdapter::safeRead(int fd, void* buf, int len) {
    int n, retry = 10;

    if(fd < 0 || buf == NULL || len < 0) {
        AGPS_LOGE("RpAgpsdAdapter::safeRead fd=%d buf=%p len=%d\n",
            fd, buf, len);
        return -1;
    }

    if(len == 0) {
        return 0;
    }

    while((n = read(fd, buf, len)) < 0) {
        if(errno == EINTR) continue;
        if(errno == EAGAIN) {
            if(retry-- > 0) {
                usleep(100 * 1000);
                continue;
            }
            goto exit;
        }
        goto exit;
    }
    return n;

exit:
    if(errno != EAGAIN) {
        AGPS_LOGE("RpAgpsdAdapter::safeRead reason=[%s] fd=%d len=%d buf=%p\n",
            strerror(errno), fd, len, buf);
    }
    return -1;
}


int RpAgpsdAdapter::checkCanWrite(int sock_fd) {
    int status = 0;
    struct pollfd pollfds[1];
    pollfds[0].fd = sock_fd;
    pollfds[0].events = POLLRDHUP;
    pollfds[0].revents = 0;
    do {
        status = TEMP_FAILURE_RETRY(poll(pollfds, 1, 0));
        if ((status < 0 && errno != EINTR) || (pollfds[0].revents & POLLRDHUP)) {
            AGPS_LOGE("RpAgpsdAdapter::checkCanWrite peer socket closed or error");
            return -1;
        }
    } while (status < 0 && errno == EINTR);
    return 0;
}


int RpAgpsdAdapter::sendToAgpsd(sp<RpAgpsMessage> &message) {
    int fd = createOutSocket();
    AGPS_LOGD("RpAgpsdAdapter::sendToAgpsd %d", message->getId());
    if (fd < 0) {
        return -1;
    }
    int offset = 0;
    char buf[2048];
    putInt32(buf, &offset, message->getId());
    switch (message->getId()) {
        case EVENT_AGPS_NETWORK_TYPE:
        case EVENT_AGPS_CDMA_PHONE_STATUS:
        case EVENT_AGPS_MOBILE_DATA_STATUS:
        case EVENT_AGPS_SET_APN:
        case EVENT_AGPS_DESTROY_APN:
        {
            Parcel *p = message->getParcel();
            p->setDataPosition(0);
            putInt32(buf, &offset, p->readInt32());
            if (checkCanWrite(fd) < 0) {
                break;
            }
            safeWrite(fd, buf, offset);
            break;
        }

        default:
            AGPS_LOGE("RpAgpsdAdapter::sendToAgpsd Error event %d",
                message->getId());
            break;
    }
    close(fd);
    return 0;
}


void RpAgpsdAdapter::processOutputMessage(sp<RpAgpsMessage> & message) {
    sendToAgpsd(message);
}


void RpAgpsdAdapter::onInit() {
    AGPS_LOGD("RpAgpsdAdapter::onInit");
    m_inFd = createInSocket();
    m_looperCallback = new AgpsLooperCallback(this);
}


void RpAgpsdAdapter::onDeinit() {
    AGPS_LOGD("RpAgpsdAdapter::onDeinit");
    if (m_inFd > 0) {
        close(m_inFd);
    }
    if (m_looperCallback != NULL) {
        m_looperCallback.clear();
    }
}


void RpAgpsdAdapter::registerFd(sp<Looper> looper) {
    AGPS_LOGD("RpAgpsdAdapter::registerFd");
    looper->addFd(m_inFd,
        Looper::POLL_CALLBACK, Looper::EVENT_INPUT, m_looperCallback, NULL);
}


void RpAgpsdAdapter::processInput(void *buf, int len) {
    int offset = 0;
    int32_t msg = getInt32((char *)buf, &offset);
    switch (msg) {
        case EVENT_UPDATE_STATE_TO_AGPSD:
        {
            sp<RpAgpsMessage> msg =
                RpAgpsMessage::obtainMessage(EVENT_UPDATE_STATE_TO_AGPSD, NULL);
            sp<RpAgpsMainThreadHandler> handler = new RpAgpsMainThreadHandler(msg);
            handler->sendMessage(RfxMainThread::getLooper());
            break;
        }

        case EVENT_SET_APN_RESULT:
        {
            int32_t result = getInt32((char *)buf, &offset);
            AGPS_LOGD("RpAgpsdAdapter::EVENT_SET_APN_RESULT %d", result);
            Parcel *p = new Parcel();
            p->writeInt32(result);
            sp<RpAgpsMessage> msg =
                RpAgpsMessage::obtainMessage(EVENT_SET_APN_RESULT, p);
            sp<RpAgpsMainThreadHandler> handler = new RpAgpsMainThreadHandler(msg);
            handler->sendMessage(RfxMainThread::getLooper());
            break;
        }

        default:
            AGPS_LOGE("RpAgpsdAdapter::processInput Error msg %d", msg);
            break;
    }
    if (offset > len) {
        RFX_ASSERT(0);
    }
}


int AgpsLooperCallback::handleEvent(int fd, int events, void* data) {
    const int size = 2048;
    RFX_ASSERT(events == Looper::EVENT_INPUT);
    RFX_UNUSED(data);
    RpAgpsdAdapter *adapter = m_adapter.promote().get();
    if (adapter != NULL) {
        char buf[size];
        int len = adapter->safeRead(fd, buf, size);
        if (len > 0) {
            adapter->processInput(buf, len);
        } else {
            AGPS_LOGE("AgpsLooperCallback::handleEvent no data");
        }
    } else {
        AGPS_LOGE("AgpsLooperCallback::handleEvent no Adapter");
    }
    return 1;
}
