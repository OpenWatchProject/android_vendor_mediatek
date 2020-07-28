/*****************************************************************************
 * Include
 *****************************************************************************/
#include <string.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <poll.h>
#include "RfxRilUtils.h"
#include "RfxDefs.h"
#include "RtcAgpsdAdapter.h"
#include "RfxMainThread.h"
#include "RtcAgpsUtil.h"

/*****************************************************************************
 * class AgpsLooperCallback
 *****************************************************************************/
int AgpsLooperCallback::handleEvent(int fd, int events, void* data) {
    const int size = 2048;
    RFX_ASSERT(events == Looper::EVENT_INPUT);
    RFX_UNUSED(data);
    RtcAgpsdAdapter *adapter = m_adapter.promote().get();
    if (adapter != NULL) {
        char buf[size];
        int len = adapter->safeRead(fd, buf, size);
        if (len > 0) {
            adapter->processInput(buf, len);
        } else {
            RFX_LOG_V(AGPS_TAG, "[RtcAgpsdAdapter]handleEvent no data");
        }
    } else {
        RFX_LOG_V(AGPS_TAG, "[RtcAgpsdAdapter]handleEvent no Adapter");
    }
    return 1;
}


/*****************************************************************************
 * Class RtcAgpsdAdapter
 *****************************************************************************/
RFX_IMPLEMENT_CLASS("RtcAgpsdAdapter", RtcAgpsdAdapter, RfxObject);
RtcAgpsdAdapter::RtcAgpsdAdapter() :
    m_inFd(-1),
    m_looperCallback(NULL) {
    RFX_LOG_V(AGPS_TAG, "[RtcAgpsdAdapter]Constructor 0x%zu", (size_t)this);
}


RtcAgpsdAdapter::~RtcAgpsdAdapter() {
    RFX_LOG_V(AGPS_TAG, "[RtcAgpsdAdapter]Destructor 0x%zu", (size_t)this);
}


void RtcAgpsdAdapter::putByte(char* buff, int* offset, const char input) {
    *((char*)&buff[*offset]) = input;
    *offset += 1;
}


void RtcAgpsdAdapter::putInt16(char* buff, int* offset, const int16_t input) {
    putByte(buff, offset, input & 0xff);
    putByte(buff, offset, (input >> 8) & 0xff);
}


void RtcAgpsdAdapter::putInt32(char* buff, int* offset, const int32_t input) {
    putInt16(buff, offset, input & 0xffff);
    putInt16(buff, offset, (input >> 16) & 0xffff);
}


char RtcAgpsdAdapter::getByte(char* buff, int* offset) {
    char ret = buff[*offset];
    *offset += 1;
    return ret;
}


int16_t RtcAgpsdAdapter::getInt16(char* buff, int* offset) {
    int16_t ret = 0;
    ret |= getByte(buff, offset) & 0xff;
    ret |= (getByte(buff, offset) << 8);
    return ret;
}


int32_t RtcAgpsdAdapter::getInt32(char* buff, int* offset) {
    int32_t ret = 0;
    ret |= getInt16(buff, offset) & 0xffff;
    ret |= (getInt16(buff, offset) << 16);
    return ret;
}


int RtcAgpsdAdapter::createSocket(
        const char *name,
        int type,
        struct sockaddr_un *addr,
        socklen_t *len) {
    int s = socket(AF_UNIX, type, 0);
    if (s < 0) {
        RFX_LOG_V(AGPS_TAG, "[RtcAgpsdAdapter]createSocket Error[%s]",
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


int RtcAgpsdAdapter::makeAddress(
        const char *name,
        struct sockaddr_un *addr,
        socklen_t *len) {
    size_t namelen;
    memset(addr, 0, sizeof(struct sockaddr_un));
    namelen = strlen(name);
    if (namelen + 1 > sizeof(addr->sun_path))
    {
        RFX_LOG_E(AGPS_TAG, "[RtcAgpsdAdapter]makeAddress Error");
        return -1;
    }
    addr->sun_path[0] = 0;
    memcpy(&addr->sun_path[1], name, namelen);
    addr->sun_family = AF_UNIX;
    *len = namelen + offsetof(struct sockaddr_un, sun_path) + 1;
    return 0;
}


int RtcAgpsdAdapter::createOutSocket() {
    struct sockaddr_un addr;
    socklen_t len;
    const char *name;
    if (RfxRilUtils::getRilRunMode() == RilRunMode::RIL_RUN_MODE_MOCK) {
        name = c_strOutSocketTest;
    } else {
        name = c_strOutSocket;
    }
    int s = createSocket(name, SOCK_STREAM, &addr, &len);
    if (s < 0) {
        return -1;
    }
    if(connect(s, (struct sockaddr *) &addr, len) < 0) {
        RFX_LOG_E(AGPS_TAG, "[RtcAgpsdAdapter]createOutSocket Connect Error[%s]",
            strerror(errno));
        close(s);
        return -1;
    }
    return s;
}


int RtcAgpsdAdapter::createInSocket() {
    struct sockaddr_un addr;
    socklen_t len;
    const char *name;
    if (RfxRilUtils::getRilRunMode() == RilRunMode::RIL_RUN_MODE_MOCK) {
        name = c_strInSocketTest;
    } else {
        name = c_strInSocket;
    }
    int s = createSocket(name, SOCK_DGRAM, &addr, &len);
    if (s < 0) {
        return -1;
    }
    int n = 1;
    int ret = setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &n, sizeof(n));
    if (ret < 0) {
        RFX_LOG_E(AGPS_TAG, "[RtcAgpsdAdapter]createInSocket setOpt Error[%s]",
            strerror(errno));
        close(s);
        return -1;
    }
    if (bind(s, (struct sockaddr *) &addr, len) < 0) {
        RFX_LOG_E(AGPS_TAG, "[RtcAgpsdAdapter]createInSocket Bind Error[%s][%s]",
            strerror(errno), name);
        close(s);
        return -1;
    }
    return s;
}


int RtcAgpsdAdapter::safeWrite(int fd, void* buf, int len) {
    int n, retry = 10;

    if(fd < 0 || buf == NULL || len < 0) {
        RFX_LOG_E(AGPS_TAG, "[RtcAgpsdAdapter]safeWrite fd=%d buf=%p len=%d\n",
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
    RFX_LOG_E(AGPS_TAG, "[RtcAgpsdAdapter]reason=[%s]%d\n", strerror(errno), errno);
    return -1;
}


int RtcAgpsdAdapter::safeRead(int fd, void* buf, int len) {
    int n, retry = 10;

    if(fd < 0 || buf == NULL || len < 0) {
        RFX_LOG_E(AGPS_TAG, "[RtcAgpsdAdapter]safeRead fd=%d buf=%p len=%d\n",
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
        RFX_LOG_E(AGPS_TAG, "[RtcAgpsdAdapter]safeRead reason=[%s] fd=%d len=%d buf=%p\n",
            strerror(errno), fd, len, buf);
    }
    return -1;
}


int RtcAgpsdAdapter::checkCanWrite(int sock_fd) {
    int status = 0;
    struct pollfd pollfds[1];
    pollfds[0].fd = sock_fd;
    pollfds[0].events = POLLRDHUP;
    pollfds[0].revents = 0;
    do {
        status = TEMP_FAILURE_RETRY(poll(pollfds, 1, 0));
        if ((status < 0 && errno != EINTR) || (pollfds[0].revents & POLLRDHUP)) {
            RFX_LOG_E(AGPS_TAG, "[RtcAgpsdAdapter]checkCanWrite peer socket closed or error");
            return -1;
        }
    } while (status < 0 && errno == EINTR);
    return 0;
}


int RtcAgpsdAdapter::sendToAgpsd(sp<RtcAgpsMessage> &message) {
    int fd = createOutSocket();
    RFX_LOG_D(AGPS_TAG, "[RtcAgpsdAdapter]sendToAgpsd %d", message->getId());
    if (fd < 0) {
        RFX_LOG_D(AGPS_TAG, "[RtcAgpsdAdapter]sendToAgpsd err. fd=%d", fd);
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
        case EVENT_MTK_RILP_INIT:
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
            RFX_LOG_D(AGPS_TAG, "[RtcAgpsdAdapter]sendToAgpsd Error event %d",
                message->getId());
            break;
    }
    close(fd);
    return 0;
}


void RtcAgpsdAdapter::processOutputMessage(sp<RtcAgpsMessage> & message) {
    sendToAgpsd(message);
}


void RtcAgpsdAdapter::onInit() {
    RFX_LOG_V(AGPS_TAG, "[RtcAgpsdAdapter]onInit");
    m_inFd = createInSocket();
    m_looperCallback = new AgpsLooperCallback(this);
}


void RtcAgpsdAdapter::onDeinit() {
    RFX_LOG_V(AGPS_TAG, "[RtcAgpsdAdapter]onDeinit");
    if (m_inFd > 0) {
        close(m_inFd);
    }
    if (m_looperCallback != NULL) {
        m_looperCallback.clear();
    }
}


void RtcAgpsdAdapter::registerFd(sp<Looper> looper) {
    RFX_LOG_V(AGPS_TAG, "[RtcAgpsdAdapter]registerFd");
    looper->addFd(m_inFd,
        Looper::POLL_CALLBACK, Looper::EVENT_INPUT, m_looperCallback, NULL);
}


void RtcAgpsdAdapter::processInput(void *buf, int len) {
    int offset = 0;
    int32_t msg = getInt32((char *)buf, &offset);
    switch (msg) {
        case EVENT_UPDATE_STATE_TO_AGPSD:
        {
            sp<RtcAgpsMessage> msg =
                RtcAgpsMessage::obtainMessage(EVENT_UPDATE_STATE_TO_AGPSD, NULL);
            sp<RtcAgpsMainThreadHandler> handler = new RtcAgpsMainThreadHandler(msg);
            handler->sendMessage(RfxMainThread::getLooper());
            break;
        }

        case EVENT_SET_APN_RESULT:
        {
            int32_t result = getInt32((char *)buf, &offset);
            RFX_LOG_V(AGPS_TAG, "[RtcAgpsdAdapter]EVENT_SET_APN_RESULT %d", result);
            Parcel *p = new Parcel();
            p->writeInt32(result);
            sp<RtcAgpsMessage> msg =
                RtcAgpsMessage::obtainMessage(EVENT_SET_APN_RESULT, p);
            sp<RtcAgpsMainThreadHandler> handler = new RtcAgpsMainThreadHandler(msg);
            handler->sendMessage(RfxMainThread::getLooper());
            break;
        }

        default:
            RFX_LOG_V(AGPS_TAG, "[RtcAgpsdAdapter]processInput Error msg %d", msg);
            break;
    }
    if (offset > len) {
        RFX_ASSERT(0);
    }
}
