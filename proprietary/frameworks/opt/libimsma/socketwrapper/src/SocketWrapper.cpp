#include <utils/Log.h>


#include <sys/socket.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/udp.h>
#include <netinet/in.h>
#include <linux/errno.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <net/if.h>
#include <cutils/qtaguid.h>

#include <sys/un.h>
#include <sys/types.h>

#include "NetdClient.h"

#include "SocketWrapper.h"

#define ATRACE_TAG  ATRACE_TAG_VIDEO
#include <utils/Trace.h>
#include <utils/Timers.h>

#undef LOG_TAG
#define LOG_TAG "[VT][SocketWapper]"

using namespace android;
using android::status_t;

#define UDP_CONNECT 1

static uint16_t u16at(const uint8_t *data)
{
    return data[0] << 8 | data[1];
}

static uint16_t getRtpSN(const sp<ABuffer> &buf)
{
    return u16at(& ((buf->data()) [2]));
}

/* constructor */
SocketWrapper::SocketWrapper()
{
    SOCKETWRAPPER_LOGI("%s:enter",__FUNCTION__);
    memset(&mParam, 0, sizeof(Sock_param_t));
    mRxCb = NULL;

    /* for thread wakeup */
    int pipefd[2];

    if(pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    mReadPipe = pipefd[0];
    mWritePipe = pipefd[1];
    SOCKETWRAPPER_LOGI("%s: readPipe=%d, writePipe=%d",__FUNCTION__, mReadPipe, mWritePipe);
    m_Tid=0;
    mCookie=0;
    m_bStarted=false;
    m_bSelfCreate=false;
    mSendDataUasage = 0;
    mrecvDataUasage = 0;

    mWriteCount = 0;
    mWriteFail = 0;
    mReceiveCount = 0;
    mError=false;
}

/* destructor */
SocketWrapper::~SocketWrapper()
{
    SOCKETWRAPPER_LOGI("%s:enter",__FUNCTION__);

    /* disable recv thread */
    if(m_bStarted) {
        setRxCallBack(NULL, NULL);
    }

    SOCKETWRAPPER_LOGI("%s:dataUsage fd=%d mSendDataUasage=%lld, mrecvDataUasage=%lld",__FUNCTION__,
                       mParam.sockfd, (long long) mSendDataUasage, (long long) mrecvDataUasage);
    int res = qtaguid_untagSocket(mParam.sockfd);
    if (res != 0) {
        SOCKETWRAPPER_LOGE("Failed untagging socket %d (My UID=%d)", mParam.sockfd, geteuid());
    }

    /*Close self create socket*/
    if(m_bSelfCreate) {
        SOCKETWRAPPER_ASSERT(mParam.sockfd > 0, "sockfd is not greater than zero");
        close(mParam.sockfd);
    }

    close(mReadPipe);
    close(mWritePipe);
}

int SocketWrapper::setParam(Sock_param_t param)
{
    SOCKETWRAPPER_LOGI("%s:enter",__FUNCTION__);

    //at present only support peer port change
    if(mParam.peer_port != 0) {
        SOCKETWRAPPER_LOGI("%s: param change %d", __FUNCTION__, mParam.peer_port);

        if(mParam.peer_port != param.peer_port) {
            SOCKETWRAPPER_LOGI("%s: peer port from %d to %d and put address", __FUNCTION__, mParam.peer_port, param.peer_port);
            mParam.peer_port = param.peer_port;
            memcpy(mParam.peer_address, param.peer_address, sizeof(param.peer_address));
            setUdpConnect();
        }

        //for datausage tag change
        if(mParam.tag != param.tag) {
            SOCKETWRAPPER_LOGI("%s: tag from 0x%x to 0x%x", __FUNCTION__, mParam.tag, param.tag);
            mParam.tag = param.tag;
            SOCKETWRAPPER_LOGI("%s: RegisterSocketUserTag fd[%d] uid[%d] tag[0x%x]",
                               __FUNCTION__, mParam.sockfd, mParam.uid, mParam.tag);

            int res = qtaguid_untagSocket(mParam.sockfd);
            if (res != 0) {
                SOCKETWRAPPER_LOGE("Failed untagging socket %d (My UID=%d)", mParam.sockfd, geteuid());
            }

            int res1 = qtaguid_tagSocket(mParam.sockfd, mParam.tag, mParam.uid);
            if (res1 != 0) {
                SOCKETWRAPPER_LOGE("Failed tagging socket %d for uid %d (My UID=%d)", mParam.sockfd, mParam.uid, geteuid());
            }
        }

        return 0;
    }

    memcpy(&mParam, &param, sizeof(Sock_param_t));

    /* create socket if not exist and connect to socket */
    setSock();

    return 0;
}

int SocketWrapper::getParam(Sock_param_t *param)
{
    SOCKETWRAPPER_LOGI("%s:enter",__FUNCTION__);

    SOCKETWRAPPER_ASSERT(param!=NULL, "param is NULL");
    memcpy(param, &mParam, sizeof(Sock_param_t));
    return 0;
}

int SocketWrapper::setRxCallBack(void *cookie, Sock_RxCB_t rx_cb)
{
    SOCKETWRAPPER_LOGI("%s:enter cookie=%p",__FUNCTION__, cookie);
    int ret = 0;

    if(rx_cb == NULL) {
        SOCKETWRAPPER_LOGI("%s:rx_cb=NULL, m_bStarted=%d",__FUNCTION__, m_bStarted);

        if(m_bStarted) {
            m_bStarted = false;

            /* wakeup recv thread */
            SOCKETWRAPPER_LOGI("%s:writePipe=%d", __FUNCTION__, mWritePipe);
            ret = write(mWritePipe, "\n", 1);

            if(ret == 1) {
                SOCKETWRAPPER_LOGI("%s:wakeup success, ret=%d",__FUNCTION__, ret);
            } else {
                SOCKETWRAPPER_LOGI("%s:wakeup fail, ret=%d, %s(%d)",__FUNCTION__, ret, strerror(errno), errno);
            }

            pthread_join(m_Tid, NULL);

            mRxCb = NULL;
        } else {
            SOCKETWRAPPER_LOGI("thread already stopped");
        }
    } else {
        SOCKETWRAPPER_LOGI("%s:rx_cb!=NULL, m_bStarted=%d",__FUNCTION__, m_bStarted);

        if(m_bStarted == false) {
            m_bStarted = true;
            mCookie = cookie;
            mRxCb = rx_cb;
            pthread_create(&m_Tid, NULL, SocketWrapper::receiveThread, this);
        } else {
            SOCKETWRAPPER_ASSERT(0, "rx_cb=NULL, but thread already started");
        }
    }

    return 0;
}

int SocketWrapper::writeSock(const sp<ABuffer>& buffer)
{
    //SOCKETWRAPPER_LOGI("%s:enter fd=%d",__FUNCTION__, mParam.sockfd);
    int fd = mParam.sockfd;
    int ret = 0;
    int error_no=0;

    if(mError == true){
        return buffer->size();
    }

    ATRACE_ASYNC_BEGIN("RTS-SOCKWRP", buffer->int32Data());

#ifdef UDP_CONNECT
    ret = send(fd, buffer->data(), buffer->size(), MSG_NOSIGNAL);
#else
    struct sockaddr_storage remote_addr;
    struct sockaddr_in *addr_ptr=NULL;
    struct sockaddr_in6 *addr6_ptr=NULL;
    socklen_t addr_len = sizeof(struct sockaddr_storage);

    SOCKETWRAPPER_ASSERT(fd>=0, "mParam.fd(%d) < 0", fd);
    SOCKETWRAPPER_ASSERT(buffer->data() !=NULL, "buffer->data() = NULL");
    SOCKETWRAPPER_ASSERT(buffer->size() >0, "buffer->size()(%d) <= 0", (int) buffer->size());

    memset((char*) &remote_addr, 0, sizeof(remote_addr));

    switch(mParam.protocol_version) {
    case VoLTE_Event_IPv4:
        addr_ptr = (struct sockaddr_in *) &remote_addr;
        addr_ptr->sin_family = AF_INET;
        addr_ptr->sin_port = htons(mParam.peer_port);
        memcpy((char *) & (addr_ptr->sin_addr), mParam.peer_address, sizeof(addr_ptr->sin_addr));
        addr_len = sizeof(struct sockaddr_in);
        break;
    case VoLTE_Event_IPv6:
        addr6_ptr = (struct sockaddr_in6 *) &remote_addr;
        addr6_ptr->sin6_family = AF_INET6;
        addr6_ptr->sin6_port = htons(mParam.peer_port);
        memcpy((char *) & (addr6_ptr->sin6_addr), mParam.peer_address, sizeof(addr6_ptr->sin6_addr));
        addr_len = sizeof(struct sockaddr_in6);
        break;
    default:
        SOCKETWRAPPER_ASSERT(0, "%s: uknow domain %d", __FUNCTION__, mParam.protocol_version);
        break;
    }

    ret = sendto(fd, buffer->data(), buffer->size(), MSG_NOSIGNAL,
                 (struct sockaddr *) &remote_addr, addr_len);
#endif

    if(ret < 0) {
        mWriteFail++;
        error_no= errno;
        SOCKETWRAPPER_LOGE("%s: socket write failed %s(%d)", __FUNCTION__, strerror(error_no), error_no);

        if(!(error_no == EMSGSIZE || error_no==EAGAIN || error_no == EWOULDBLOCK || error_no == EPERM
                || error_no == ECONNREFUSED || error_no == ENETUNREACH || error_no == EINVAL)) {
            SOCKETWRAPPER_ASSERT(0, "unexpect error_no=%d", error_no);
        } else {
            return error_no* (-1);
        }
    }

    mWriteCount++;
    mSendDataUasage += buffer->size();
    ATRACE_ASYNC_END("RTS-SOCKWRP", buffer->int32Data());

    return ret;
}


/**** private ****/
#define  MAX_MTU_SIZE 4096
int SocketWrapper::dumpAddr(struct sockaddr *addr_ptr)
{
    SOCKETWRAPPER_LOGI("%s:enter",__FUNCTION__);
    char addr_str[256]= {0};
    uint16_t  port = 0;
    socklen_t addr_len = sizeof(struct sockaddr_storage);
    addr_len = sizeof(struct sockaddr_storage);

    switch(addr_ptr->sa_family) {
    case AF_INET:
        inet_ntop(AF_INET, & (((struct sockaddr_in *) addr_ptr)->sin_addr), addr_str, sizeof(addr_str));
        port = ntohs(((struct sockaddr_in *) addr_ptr)->sin_port);
        break;
    case AF_INET6:
        inet_ntop(AF_INET6, & (((struct sockaddr_in6 *) addr_ptr)->sin6_addr), addr_str, sizeof(addr_str));
        port = ntohs(((struct sockaddr_in6 *) addr_ptr)->sin6_port);
        break;
    default:
        SOCKETWRAPPER_LOGE("    unknow family type %d", addr_ptr->sa_family);
        return -1;
    }

    SOCKETWRAPPER_LOGI("    ip:%s, port:%d", addr_str, port);
    return 0;
}

int SocketWrapper::readSock(int fd, const sp<ABuffer>& buffer)
{
    //SOCKETWRAPPER_LOGI("%s:enter",__FUNCTION__);
    int ret = 0;

#ifdef CONNECT_UDP
    ret = recv(fd, (char *) buffer->data(), buffer->size(), 0);
#else
    struct sockaddr_storage remote_addr, local_addr;
    struct sockaddr *raddr_ptr = NULL;
    struct sockaddr *laddr_ptr = NULL;
    socklen_t addr_len = sizeof(struct sockaddr_storage);

    SOCKETWRAPPER_ASSERT(fd>=0, "fd(%d) < 0", fd);
    SOCKETWRAPPER_ASSERT(buffer->data() !=NULL, "buffer->data() = NULL");
    SOCKETWRAPPER_ASSERT(buffer->size() >0, "buffer->size()(%d) <= 0", (int) buffer->size());

    raddr_ptr = (struct sockaddr *) &remote_addr;
    laddr_ptr = (struct sockaddr *) &local_addr;

    ret = recvfrom(fd, (char *) buffer->data(), buffer->size(), 0, raddr_ptr, (socklen_t *) &addr_len);
#endif

    if(ret < 0) {
        //No asser since sockfd may be closed by imsma_rtp and SocketBind
        SOCKETWRAPPER_LOGE("%s: socket read failed %s(%d)", __FUNCTION__, strerror(errno), errno);
        return ret;
    } else if((unsigned int) ret > buffer->size()) {
        /* check if buffer size is enough */
        SOCKETWRAPPER_ASSERT(((unsigned int) ret <= buffer->size()),
                             "ret(%d) > buffer_size(%d), MAX_MTU_SIZE(%d)", ret, (int) buffer->size(), MAX_MTU_SIZE);
        return -1;
    }


    buffer->setRange(0, ret);
    mrecvDataUasage += ret;
    mReceiveCount++;

#ifdef DEBUG
    socklen_t addr_len = sizeof(struct sockaddr_storage);
    addr_len = sizeof(struct sockaddr_storage);

    if(getsockname(fd, laddr_ptr, addr_len) != -1) {
        SOCKETWRAPPER_LOGE("fail to get local socket %s(%d)", strerror(errno));
    } else {
        SOCKETWRAPPER_LOGI("local addr: ");
        dumpAddr(laddr_ptr);
    }

    SOCKETWRAPPER_LOGI("remote addr: ");
    dumpAddr(raddr_ptr);
#endif

    return ret;
}

/* just for workaround that socket bind is not ready */
int SocketWrapper::createSock(void)
{
    SOCKETWRAPPER_LOGI("%s:enter",__FUNCTION__);
    int sockfd = -1;
    struct sockaddr_storage local_addr;
    struct sockaddr_in *addr_ptr;
    struct sockaddr_in6 *addr6_ptr;
    socklen_t addr_len;
    char str[INET6_ADDRSTRLEN] = {0};


    memset(&local_addr, 0, sizeof(local_addr));

    /* create socket */
    if(mParam.protocol_version == VoLTE_Event_IPv4) {
        addr_ptr = (struct sockaddr_in *) &local_addr;
        addr_ptr->sin_family = AF_INET;
        addr_ptr->sin_port = htons(mParam.local_port);
        memcpy((char *) & (addr_ptr->sin_addr), mParam.local_address, sizeof(addr_ptr->sin_addr));
        sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        addr_len = sizeof(struct sockaddr_in);
    } else if(mParam.protocol_version == VoLTE_Event_IPv6) {
        addr6_ptr = (struct sockaddr_in6 *) &local_addr;
        addr6_ptr->sin6_family = AF_INET6;
        addr6_ptr->sin6_port = htons(mParam.local_port);
        memcpy((char *) & (addr6_ptr->sin6_addr), mParam.local_address, sizeof(addr6_ptr->sin6_addr));
        sockfd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
        addr_len = sizeof(struct sockaddr_in6);
    } else {
        SOCKETWRAPPER_ASSERT(0, "%s: uknow protocol_version %d", __FUNCTION__, mParam.protocol_version);
    }

    if(sockfd < 0) {
        SOCKETWRAPPER_ASSERT(0, "%s: socket create failed %s(%d)", __FUNCTION__, strerror(errno), errno);
    }

    /* bind socket */
    if(bind(sockfd, (struct sockaddr *) &local_addr, addr_len) < 0) {
        SOCKETWRAPPER_ASSERT(0, "%s: socket bind failed %s(%d)", __FUNCTION__, strerror(errno), errno);
    }

    return sockfd;
}

void SocketWrapper::setUdpConnect(void)
{
    SOCKETWRAPPER_LOGI("%s:enter",__FUNCTION__);

    struct sockaddr_storage remote_addr;
    struct sockaddr_in *addr_ptr;
    struct sockaddr_in6 *addr6_ptr;
    socklen_t addr_len;

    /* connect the UDP socket, so that we can use send/recv instead of sendto/recvfrom */
#ifdef CONNECT_UDP

    switch(mParam.protocol_version) {
    case VoLTE_Event_IPv4:
        addr_ptr = (struct sockaddr_in *) &remote_addr;
        addr_ptr->sin_family = AF_INET;
        addr_ptr->sin_port = htons(mParam.peer_port);
        memcpy((char *) & (addr_ptr->sin_addr), mParam.peer_address, sizeof(addr_ptr->sin_addr));
        addr_len = sizeof(struct sockaddr_in);
        break;
    case VoLTE_Event_IPv6:
        addr6_ptr = (struct sockaddr_in6 *) &remote_addr;
        addr6_ptr->sin6_family = AF_INET6;
        addr6_ptr->sin6_port = htons(mParam.peer_port);
        memcpy((char *) & (addr6_ptr->sin6_addr), mParam.peer_address, sizeof(addr6_ptr->sin6_addr));
        addr_len = sizeof(struct sockaddr_in6);
        break;
    default:
        SOCKETWRAPPER_ASSERT(0, "%s: uknow domain %d", __FUNCTION__, mParam.protocol_version);
        break;
    }

    if(connect(mParam.sockfd, (struct sockaddr *) &remote_addr, addr_len) < 0) {
        //SOCKETWRAPPER_ASSERT(0, "%s: socket connect failed %s(%d)", __FUNCTION__, strerror(errno), errno);
        //tester maybe hot plug sim card, so we set ERROR flag instead of trigger exception
        SOCKETWRAPPER_LOGE("%s: socket connect failed %s(%d)", __FUNCTION__, strerror(errno), errno);
        mError = true;
    }

#endif
}

/* 1. create socket if sock is not exist */
/* 2. connect to socket */
int SocketWrapper::setSock(void)
{
    SOCKETWRAPPER_LOGI("%s:enter, sockfd=%d, protocol=%d, port=%d",__FUNCTION__,
                       mParam.sockfd, mParam.protocol_version, mParam.peer_port);

    /*
        struct sockaddr_storage remote_addr;
        struct sockaddr_in *addr_ptr;
        struct sockaddr_in6 *addr6_ptr;
        socklen_t addr_len;
    */

    /* create socket if it is not exist */
    if(mParam.sockfd < 0) {
        SOCKETWRAPPER_LOGI("socket is not exist, create myself! ");
        mParam.sockfd = createSock();
        m_bSelfCreate = true;
    }

    /* set dscp */
    unsigned char optv4 = mParam.dscp << 2;
    unsigned int optv6 = mParam.dscp << 2;
    SOCKETWRAPPER_LOGI("set ToS val: 0x%x\n", mParam.dscp);

    switch(mParam.protocol_version) {
    case VoLTE_Event_IPv4:

        if(setsockopt(mParam.sockfd, IPPROTO_IP, IP_TOS, &optv4, sizeof(optv4)) < 0) {
            SOCKETWRAPPER_ASSERT(0, "%s: socket setsockopt dscp failed %s(%d)", __FUNCTION__, strerror(errno), errno);
        }

        break;
    case VoLTE_Event_IPv6:

        if(setsockopt(mParam.sockfd, IPPROTO_IPV6, IPV6_TCLASS, &optv6, sizeof(optv6)) < 0) {
            SOCKETWRAPPER_ASSERT(0, "%s: socket setsockopt dscp failed %s(%d)", __FUNCTION__, strerror(errno), errno);
        }

        break;
    default:
        SOCKETWRAPPER_ASSERT(0, "%s: uknow domain %d", __FUNCTION__, mParam.protocol_version);
        break;
    }

    /* set priority */
    SOCKETWRAPPER_LOGI("set CoS val: 0x%x\n", mParam.priority);

    if(setsockopt(mParam.sockfd, SOL_SOCKET, SO_PRIORITY, & (mParam.priority), sizeof(int)) < 0) {
        SOCKETWRAPPER_ASSERT(0, "%s: socket setsockopt pirority failed %s(%d)", __FUNCTION__, strerror(errno), errno);
    }

    if(m_bSelfCreate == true){

        if(strlen(mParam.ifname) != 0) {
            struct ifreq ifr;
            memset(&ifr, 0, sizeof(ifr));
            strncpy(ifr.ifr_name, mParam.ifname,16);
            ifr.ifr_name[15] = '\0';

            SOCKETWRAPPER_LOGI("set ifname: size=%zu sockfd=%d str=%s:%s\n", strlen(mParam.ifname), mParam.sockfd, mParam.ifname, ifr.ifr_name);

            if(setsockopt(mParam.sockfd, SOL_SOCKET, SO_BINDTODEVICE, &ifr, sizeof(ifr)) < 0) {
                SOCKETWRAPPER_ASSERT(0, "%s: socket SO_BINDTODEVICE failed %s(%d)", __FUNCTION__, strerror(errno), errno);
            }
        } else if(mParam.network_id > 0) {
            SOCKETWRAPPER_LOGI("set network_id: 0x%x, sockfd=%d, uid=%d\n", mParam.network_id, mParam.sockfd, mParam.uid);

            /* set network id */
            if(setNetworkForSocket(mParam.network_id, mParam.sockfd) < 0) {
                SOCKETWRAPPER_LOGI("setNetworkForSocket fail maybe 93 md\n");

                if(setsockopt(mParam.sockfd, SOL_SOCKET, SO_MARK, &mParam.network_id, sizeof(int)) < 0) {
                    SOCKETWRAPPER_ASSERT(0, "%s: socket SO_MARK failed %s(%d)", __FUNCTION__, strerror(errno), errno);
                }
            }
        } else {
            SOCKETWRAPPER_LOGW("skip to set network_id/ifname <= 0\n");
        }

        setUdpConnect();
    }

    int res1 = qtaguid_tagSocket(mParam.sockfd, mParam.tag, mParam.uid);
    if (res1 != 0) {
        SOCKETWRAPPER_LOGE("Failed tagging socket %d for uid %d (My UID=%d)", mParam.sockfd, mParam.uid, geteuid());
    }


    /* set non-blocking */
    int error_no=0;
    int flags = fcntl(mParam.sockfd, F_GETFL, 0);

    if(mParam.isBlock == false) {
        int res = fcntl(mParam.sockfd, F_SETFL, flags | O_NONBLOCK);

        if(res < 0) {
            error_no = errno;
            SOCKETWRAPPER_LOGE("%s: socket set to non-blocking failed %s(%d)", __FUNCTION__, strerror(error_no), error_no);
        } else {
            SOCKETWRAPPER_LOGI("set to non-blocking, flags=%d", flags);
        }
    } else {
        SOCKETWRAPPER_LOGI("mParam.isBlock = true, socket stay in blocking mode");
    }

    /* set buffer size */
    if(mParam.sendBufferSize != 0) {
        if(setsockopt(mParam.sockfd, SOL_SOCKET, SO_SNDBUF, & (mParam.sendBufferSize), sizeof(uint32_t)) == -1) {
            SOCKETWRAPPER_ASSERT(0, "%s: set socket sendbuf size (%d) fail %s(%d)", __FUNCTION__, mParam.sendBufferSize, strerror(errno), errno);
        } else {
            SOCKETWRAPPER_LOGW("set socket sendbuf size (%d) success\n", mParam.sendBufferSize);
        }
    } else {
        SOCKETWRAPPER_LOGI("sendBufferSize = 0");
    }

    if(mParam.receiveBufferSize != 0) {
        if(setsockopt(mParam.sockfd, SOL_SOCKET, SO_RCVBUF, & (mParam.receiveBufferSize), sizeof(uint32_t)) == -1) {
            SOCKETWRAPPER_ASSERT(0, "%s: set socket recvbuf size (%d) fail %s(%d)", __FUNCTION__, mParam.receiveBufferSize, strerror(errno), errno);
        } else {
            SOCKETWRAPPER_LOGW("set socket recvbuf size (%d) success\n", mParam.receiveBufferSize);
        }
    } else {
        SOCKETWRAPPER_LOGI("recvBufferSize = 0");
    }



#if 0
    /* set dscp */
    unsigned char opt = (unsigned char) mParam.dscp;

    SOCKETWRAPPER_LOGI("set ToS val: 0x%x\n", opt);

    switch(domain) {
    case AF_INET:

        if(setsockopt(mParam.sockfd, SOL_IP, IP_TOS, &opt, sizeof(opt)) < 0) {
            SOCKETWRAPPER_ASSERT(0, "%s: socket setsockopt dscp failed %s(%d)", __FUNCTION__, strerror(errno), errno);
        }

        break;
    case AF_INET6:

        if(setsockopt(mParam.sockfd, SOL_IP, IPV6_TCLASS, &opt, sizeof(opt)) < 0) {
            SOCKETWRAPPER_ASSERT(0, "%s: socket setsockopt dscp failed %s(%d)", __FUNCTION__, strerror(errno), errno);
        }

        break;
    default:
        SOCKETWRAPPER_ASSERT(0, "%s: uknow domain %d", __FUNCTION__, domain);
        break;
    }

    /* set priority */
    opt = (unsigned char) mParam.priority;
    SOCKETWRAPPER_LOGI("set CoS val: 0x%x\n", opt);

    if(setsockopt(mParam.sockfd, SOL_SOCKET, SO_PRIORITY, &opt, sizeof(opt)) < 0) {
        SOCKETWRAPPER_ASSERT(0, "%s: socket setsockopt pirority failed %s(%d)", __FUNCTION__, strerror(errno), errno);
    }

#endif

    return 0;
}

void * SocketWrapper::receiveThread(void *pParam)
{
    SocketWrapper *pSelf = (SocketWrapper *) pParam;
    int max_fd;
    int ret;
    fd_set readfds;
    int sockfd = pSelf->mParam.sockfd;
    int32_t sn;

    SOCKETWRAPPER_ASSERT(sockfd>=0, "pSelf->mParam.sockfd(%d) < 0", sockfd);

    SOCKETWRAPPER_LOGI("%s:enter",__FUNCTION__);
    struct timeval timeout;
    //struct timeval begin_time,end_time;
    uint32_t diff_time;
    struct timeval last_time,this_time;
    last_time.tv_sec = 0;
    pSelf->mReceiveCount = 0;

    while((pSelf->m_bStarted) && (pSelf->mError==false)) {

        timeout.tv_sec = 0;
        timeout.tv_usec = 1000*100*5;//500ms

        FD_ZERO(&readfds);
        /* add socket to readfds */
        FD_SET(sockfd, &readfds);
        FD_SET(pSelf->mReadPipe, &readfds);

        if(pSelf->mReadPipe > sockfd) {
            max_fd = pSelf->mReadPipe;
        } else {
            max_fd = sockfd;
        }

        gettimeofday(&this_time, (struct timezone *) NULL);

        if(last_time.tv_sec == 0) {
            last_time = this_time;
        } else if(this_time.tv_sec - last_time.tv_sec > 1) {
            SOCKETWRAPPER_LOGI("IMSSOCK DEBUG fd(%d) writeCount(%u) writeFail(%u) writeSize(%lld) receiveCount(%u) receiveSize(%lld)",
                               pSelf->mParam.sockfd, pSelf->mWriteCount, pSelf->mWriteFail,
                               (long long)(pSelf->mSendDataUasage), pSelf->mReceiveCount,
                               (long long)(pSelf->mrecvDataUasage));
            last_time = this_time;
        }

        //gettimeofday(&begin_time, (struct timezone *) NULL);

        ret = select(max_fd+1, &readfds, NULL, NULL, &timeout);
        //SOCKETWRAPPER_LOGI("%s:ret=%d",__FUNCTION__, ret);

        //gettimeofday(&end_time, (struct timezone *) NULL);
        //diff_time = (end_time.tv_sec- begin_time.tv_sec)*1000+(end_time.tv_usec-begin_time.tv_usec)/1000;

        if(ret >0) {
            /* check if sockfd is in readfds set which makes select() wakeup */
            if(FD_ISSET(sockfd, &readfds)) {
                sp<ABuffer> buffer = new ABuffer(MAX_MTU_SIZE);
                ret = pSelf->readSock(sockfd, buffer);

                /* return buffer size */
                if(ret > 0) {
                    if(pSelf->mRxCb != NULL) {
                        sn = (int32_t) getRtpSN(buffer);
                        buffer->setInt32Data(sn);

                        pSelf->mRxCb(pSelf->mCookie, buffer);
                    }
                } else {
                    SOCKETWRAPPER_LOGE("%s:recevie empty buffer ret=%d",__FUNCTION__, ret);
                }

            } else if(FD_ISSET(pSelf->mReadPipe, &readfds)) {
                char buffer[80];
                SOCKETWRAPPER_LOGI("%s:wakeup by pipe", __FUNCTION__);
                /* clean read pipe */
                read(pSelf->mReadPipe, buffer, sizeof(buffer));
            }
        } else if(ret == 0) {
            //SOCKETWRAPPER_LOGI("%s:timeout", __FUNCTION__);
        } else {
            SOCKETWRAPPER_LOGE("%s:%s(%d)", __FUNCTION__, strerror(errno), errno);
        }

    }

    SOCKETWRAPPER_LOGI("%s:leave ERROR=%d",__FUNCTION__,pSelf->mError);
    return 0;
}

