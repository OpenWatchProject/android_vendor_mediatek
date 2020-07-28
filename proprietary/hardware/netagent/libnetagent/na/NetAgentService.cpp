/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "NetAgentService.h"
#include "NetlinkEventHandler.h"
#include <cutils/properties.h>

/*****************************************************************************
 ** Defines
 ******************************************************************************/
#define NA_LOG_TAG "NetAgentService"
#define UNUSED(x) ((void)(x))
#define SIOCSTXQSTATE (SIOCDEVPRIVATE + 0)  // start/stop ccmni tx queue.

/*****************************************************************************
 * Class NetAgentService
 *****************************************************************************/
pthread_mutex_t NetAgentService::sInitMutex = PTHREAD_MUTEX_INITIALIZER;
NetAgentService* NetAgentService::sInstance = NULL;

const char* NetAgentService::CCMNI_IFNAME_CCMNI = "ccmni";

//pthread_condattr_t NetAgentService::s_cond_attr;
//pthread_cond_t NetAgentService::s_cond;
//pthread_mutex_t NetAgentService::s_mutex;
struct thread_args {
    NetAgentService *instance;
    NA_ARP_INFO *arp;
    struct result_naptr_in_netagent* result_list;
};

NetAgentService::NetAgentService() {
    init();
}

void NetAgentService::init() {
    mReaderThread = 0;
    mEventThread = 0;
    sock_fd = 0;
    sock6_fd = 0;
    m_pNetAgentIoObj = NULL;
    m_pNetAgentReqInfo = NULL;
    mRouteSock = 0;
    m_pRouteHandler = NULL;
    mIfChgForIPV6Count = 0;
    pthread_mutex_init(&mDispatchMutex, NULL);
    pthread_cond_init(&mDispatchCond, NULL);
    m_lTransIntfId.clear();
    isMultiHomingFeatureSupport = false;

    //Initialize thread of NAPTR
    pthread_condattr_init(&s_cond_attr);
    pthread_condattr_setclock(&s_cond_attr, CLOCK_MONOTONIC);
    pthread_cond_init(&s_cond, &s_cond_attr);
    pthread_mutex_init(&s_mutex, NULL);

    // Initialize NetAgent IO Socket.
    NA_INIT(m_pNetAgentIoObj);
    if (m_pNetAgentIoObj != NULL) {
        startEventLoop();
        startReaderLoop();
        startNetlinkEventHandler();
        // TODO: Enable it when modem is ready for AT+EPDNHOCFG and AT+EPDN
        syncCapabilityToModem();
        syncNAPTRCapabilityToModem();
    } else {
        NA_LOG_E("[%s] init NetAgent io socket fail", __FUNCTION__);
    }
}

NetAgentService::~NetAgentService() {
    if (NA_DEINIT(m_pNetAgentIoObj) != NETAGENT_IO_RET_SUCCESS ) {
        NA_LOG_E("[%s] deinit NetAgent io socket fail", __FUNCTION__);
    }

    NetAgentReqInfo *pTmp = NULL;
    while (m_pNetAgentReqInfo != NULL) {
       pTmp = m_pNetAgentReqInfo;
       m_pNetAgentReqInfo = m_pNetAgentReqInfo->pNext;
       freeNetAgentCmdObj(pTmp);
       FREEIF(pTmp);
    }

    if (m_pRouteHandler != NULL) {
        if (m_pRouteHandler->stop() < 0) {
            NA_LOG_E("[%s] Unable to stop route NetlinkEventHandler: %s",
                    __FUNCTION__, strerror(errno));
        }
        delete m_pRouteHandler;
        m_pRouteHandler = NULL;
    }

    sInstance = NULL;
    m_lTransIntfId.clear();
}

NetAgentService* NetAgentService::getInstance() {
    if (sInstance != NULL) {
        return sInstance;
    }
    return NULL;
}

bool NetAgentService::createNetAgentService() {
    pthread_mutex_lock(&sInitMutex);
    if (sInstance == NULL) {
        sInstance = new NetAgentService();
        if (sInstance == NULL) {
            NA_LOG_E("[%s] new NetAgentService fail", __FUNCTION__);
            pthread_mutex_unlock(&sInitMutex);
            return false;
        } else if (sInstance->m_pNetAgentIoObj == NULL) {
            delete sInstance;
            sInstance = NULL;
            pthread_mutex_unlock(&sInitMutex);
            return false;
        }
    }
    pthread_mutex_unlock(&sInitMutex);
    return true;
}

void NetAgentService::startEventLoop(void) {
    int ret;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    ret = pthread_create(&mEventThread, &attr, NetAgentService::eventThreadStart, this);

    if (ret != 0) {
        NA_LOG_E("[%s] failed to create event thread ret:%d", __FUNCTION__, ret);
    } else {
        NA_LOG_D("[%s] create event thread OK ret:%d, mEventThread:%ld",
                __FUNCTION__, ret, mEventThread);
    }
}

void *NetAgentService::eventThreadStart(void *arg) {
    NetAgentService *me = reinterpret_cast<NetAgentService *>(arg);
    me->runEventLoop();
    return NULL;
}

void NetAgentService::runEventLoop() {
    while (1) {
        NetAgentReqInfo *pReq = NULL;

        pthread_mutex_lock(&mDispatchMutex);
        pReq = dequeueReqInfo();
        if (pReq != NULL) {
            pthread_mutex_unlock(&mDispatchMutex);
            handleEvent(pReq);
            FREEIF(pReq);
        } else {
            pthread_cond_wait(&mDispatchCond, &mDispatchMutex);
            pthread_mutex_unlock(&mDispatchMutex);
        }
    }
}

void NetAgentService::startReaderLoop(void) {
    int ret;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    ret = pthread_create(&mReaderThread, &attr, NetAgentService::readerThreadStart, this);

    if (ret != 0) {
        NA_LOG_E("[%s] failed to create reader thread ret:%d", __FUNCTION__, ret);
    } else {
        NA_LOG_D("[%s] create reader thread OK ret:%d, mReaderThread:%ld",
                __FUNCTION__, ret, mReaderThread);
    }
}

void *NetAgentService::readerThreadStart(void *arg) {
    NetAgentService *me = reinterpret_cast<NetAgentService *>(arg);
    me->runReaderLoop();
    return NULL;
}

void NetAgentService::runReaderLoop() {
    while (1) {
        void *pNetAgentCmdObj = NULL;

        // Receive URC reported from DDM.
        NA_CMD_RECV(m_pNetAgentIoObj, pNetAgentCmdObj);
        if (pNetAgentCmdObj != NULL) {
            enqueueReqInfo(pNetAgentCmdObj, REQUEST_TYPE_DDM);
        } else {
            NA_LOG_E("[%s] recv urc fail", __FUNCTION__);
        }
    }
}

void NetAgentService::handleEvent(NetAgentReqInfo* pReqInfo) {
    switch (pReqInfo->cmdType) {
        case NETAGENT_IO_CMD_IFUP:
            configureNetworkInterface(pReqInfo, ENABLE);
            break;
        case NETAGENT_IO_CMD_IFDOWN:
            configureNetworkInterface(pReqInfo, DISABLE);
            break;
        case NETAGENT_IO_CMD_IFCHG:
            configureNetworkInterface(pReqInfo, UPDATE);
            break;
        case NETAGENT_IO_CMD_IPUPDATE:
            updateIpv6GlobalAddress(pReqInfo);
            break;
        case NETAGENT_IO_CMD_IFSTATE:
            configureNetworkTransmitState(pReqInfo);
            break;
        case NETAGENT_IO_CMD_SETMTU:
            configureMTUSize(pReqInfo);
            break;
        case NETAGENT_IO_CMD_IPADD:
            configureIpAdd(pReqInfo);
            break;
        case NETAGENT_IO_CMD_IPDEL:
            configureIpDel(pReqInfo);
            break;
        case NETAGENT_IO_CMD_SYNC_CAPABILITY:
            setCapabilityToModem(pReqInfo);
            break;
        case NETAGENT_IO_CMD_PDNHO:
            handlePdnHandoverControl(pReqInfo);
            break;
        case NETAGENT_IO_CMD_IPCHG:
            updatePdnHandoverAddr(pReqInfo);
            break;
        case NETAGENT_IO_CMD_NAPTR_CAPABILITY:
            setNAPTRCapabilityToModem(pReqInfo);
            break;
        case NETAGENT_IO_CMD_NAPTR_QUERY:
            NA_LOG_D("[%s] Enter NETAGENT_IO_CMD_NAPTR_QUERY event", __FUNCTION__);
            queryNAPTR(pReqInfo);
            break;
        case NETAGENT_IO_CMD_ARP_QUERY:
            queryArp(pReqInfo);
            break;
        case NETAGENT_IO_CMD_ARP_RESULT:
            sendArpResult(pReqInfo);
            break;
        case NETAGENT_IO_CMD_TCP_RSVN:
        case NETAGENT_IO_CMD_UDP_RSVN:
            reserveTcpUdpPort(pReqInfo);
            break;
        case NETAGENT_IO_CMD_SPI_RSVN:
            reserveSpi(pReqInfo);
            break;
        default:
            break;
    }
    freeNetAgentCmdObj(pReqInfo);
}

NetAgentReqInfo *NetAgentService::createNetAgentReqInfo(void* obj, REQUEST_TYPE reqType, NA_CMD cmd) {
    NetAgentReqInfo* pNewReqInfo = NULL;
    pNewReqInfo = (NetAgentReqInfo *)calloc(1, sizeof(NetAgentReqInfo));
    if (pNewReqInfo == NULL) {
        NA_LOG_E("[%s] can't allocate NetAgentReqInfo", __FUNCTION__);
        return NULL;
    }

    pNewReqInfo->pNext = NULL;
    pNewReqInfo->pNetAgentCmdObj = obj;
    pNewReqInfo->reqType = reqType;
    pNewReqInfo->cmdType = cmd;
    return pNewReqInfo;
}

void NetAgentService::enqueueReqInfo(void* obj, REQUEST_TYPE reqType) {
    NetAgentReqInfo *pNew = NULL;
    NetAgentReqInfo *pCurrent = NULL;
    NA_CMD cmd;

    if (getCommand(obj, reqType, &cmd) < 0) {
        NA_LOG_E("[%s] get command fail", __FUNCTION__);
        return;
    }

    pNew = createNetAgentReqInfo(obj, reqType, cmd);
    if (pNew == NULL) {
        NA_LOG_E("[%s] create NetAgentReqInfo fail", __FUNCTION__);
        return;
    }

    pthread_mutex_lock(&mDispatchMutex);
    if (m_pNetAgentReqInfo == NULL) { /* No pending */
        m_pNetAgentReqInfo = pNew;
        pthread_cond_broadcast(&mDispatchCond);
    } else {
        pCurrent = m_pNetAgentReqInfo;
        while(pCurrent != NULL) {
            if (pCurrent->pNext == NULL) {
                pCurrent->pNext = pNew;
                break;
            }
            pCurrent = pCurrent->pNext;
        }
    }
    pthread_mutex_unlock(&mDispatchMutex);
}

NetAgentReqInfo *NetAgentService::dequeueReqInfo() {
    NetAgentReqInfo *pCurrent = m_pNetAgentReqInfo;

    if (pCurrent != NULL) {
        m_pNetAgentReqInfo = pCurrent->pNext;
    }
    return pCurrent;
}

void NetAgentService::syncCapabilityToModem() {
    NA_LOG_D("[%s]", __FUNCTION__);

    NetEventReqInfo *pNetEventObj = (NetEventReqInfo *)calloc(1, sizeof(NetEventReqInfo));

    if (pNetEventObj == NULL) {
        NA_LOG_E("[%s] can't allocate rild event obj", __FUNCTION__);
        return;
    }
    pNetEventObj->cmd = NETAGENT_IO_CMD_SYNC_CAPABILITY;
    enqueueReqInfo(pNetEventObj, REQUEST_TYPE_NETAGENT);
}

void NetAgentService::setCapabilityToModem(NetAgentReqInfo* pReqInfo) {
    UNUSED(pReqInfo);
    void *pNetAgentCmdObj = NA_CMD_SYNC_CAPABILITY_ALLOC();
    if (NA_CMD_SEND(m_pNetAgentIoObj, pNetAgentCmdObj) != NETAGENT_IO_RET_SUCCESS) {
        NA_LOG_E("[%s] fail", __FUNCTION__);
    }
    NA_CMD_FREE(pNetAgentCmdObj);
}

void NetAgentService::syncNAPTRCapabilityToModem() {
    NA_LOG_D("[%s]", __FUNCTION__);

    NetEventReqInfo *pNetEventObj = (NetEventReqInfo *)calloc(1, sizeof(NetEventReqInfo));

    if (pNetEventObj == NULL) {
        NA_LOG_E("[%s] can't allocate rild event obj", __FUNCTION__);
        return;
    }
    pNetEventObj->cmd = NETAGENT_IO_CMD_NAPTR_CAPABILITY;
    enqueueReqInfo(pNetEventObj, REQUEST_TYPE_NETAGENT);
}

void NetAgentService::setNAPTRCapabilityToModem(NetAgentReqInfo* pReqInfo) {
    UNUSED(pReqInfo);
    void *pNetAgentCmdObj = NA_CMD_SYNC_NAPTR_CAPABILITY_ALLOC();
    if (NA_CMD_SEND(m_pNetAgentIoObj, pNetAgentCmdObj) != NETAGENT_IO_RET_SUCCESS) {
        NA_LOG_E("[%s] fail", __FUNCTION__);
    }
    NA_CMD_FREE(pNetAgentCmdObj);
}

void NetAgentService::setNwIntfDown(const char *interfaceName) {
    ifc_reset_connections(interfaceName, RESET_ALL_ADDRESSES);
    ifc_remove_default_route(interfaceName);
    ifc_disable(interfaceName);
}

void NetAgentService::resetNwIntfMtu(const char *interfaceName) {
    char *cmd = NULL;
    NA_LOG_I("[%s] reset mtu size for interface %s", __FUNCTION__, interfaceName);
    asprintf(&cmd, "ifconfig %s mtu %d", interfaceName, 1500);
    if (cmd != NULL) {
        NA_LOG_D("[%s] cmd = %s", __FUNCTION__, cmd);
        system(cmd);
        free(cmd);
    } else {
        NA_LOG_E("[%s] cmd is NULL", __FUNCTION__);
    }
}

void NetAgentService::nwIntfIoctlInit() {
    if (sock_fd > 0) {
        close(sock_fd);
    }

    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0) {
        NA_LOG_E("[%s] couldn't create IP socket: errno=%d", __FUNCTION__, errno);
    }

    if (sock6_fd > 0) {
        close(sock6_fd);
    }

    sock6_fd = socket(AF_INET6, SOCK_DGRAM, 0);
    if (sock6_fd < 0) {
        sock6_fd = -errno;    /* save errno for later */
        NA_LOG_E("[%s] couldn't create IPv6 socket: errno=%d", __FUNCTION__, errno);
    }
}

void NetAgentService::nwIntfIoctlDeInit() {
    close(sock_fd);
    close(sock6_fd);
    sock_fd = 0;
    sock6_fd = 0;
}

/* For setting IFF_UP: nwIntfSetFlags(s, &ifr, IFF_UP, 0) */
/* For setting IFF_DOWN: nwIntfSetFlags(s, &ifr, 0, IFF_UP) */
void NetAgentService::nwIntfSetFlags(int s, struct ifreq *ifr, int set, int clr) {
    int ret = 0;

    ret = ioctl(s, SIOCGIFFLAGS, ifr);
    if (ret < 0) {
        NA_LOG_E("[%s] error in set SIOCGIFFLAGS:%d - %d:%s",
                __FUNCTION__, ret, errno, strerror(errno));
        return;
    }

    ifr->ifr_flags = (ifr->ifr_flags & (~clr)) | set;
    ret = ioctl(s, SIOCSIFFLAGS, ifr);
    if (ret < 0) {
        NA_LOG_E("[%s] error in set SIOCSIFFLAGS:%d - %d:%s",
                __FUNCTION__, ret, errno, strerror(errno));
    }
}

inline void NetAgentService::nwIntfInitSockAddrIn(struct sockaddr_in *sin, const char *addr) {
    sin->sin_family = AF_INET;
    sin->sin_port = 0;
    sin->sin_addr.s_addr = inet_addr(addr);
}

void NetAgentService::nwIntfSetAddr(int s, struct ifreq *ifr, const char *addr) {
    int ret = 0;

    NA_LOG_D("[%s] configure IPv4 adress : %s", __FUNCTION__, addr);
    nwIntfInitSockAddrIn((struct sockaddr_in *) &ifr->ifr_addr, addr);
    ret = ioctl(s, SIOCSIFADDR, ifr);
    if (ret < 0) {
        NA_LOG_E("[%s] error in set SIOCSIFADDR:%d - %d:%s",
                __FUNCTION__, ret, errno, strerror(errno));
    }
}

void NetAgentService::nwIntfSetIpv6Addr(int s, struct ifreq *ifr, const char *addr) {
    struct in6_ifreq ifreq6;
    int ret = 0;

    NA_LOG_D("[%s] configure IPv6 adress : %s", __FUNCTION__, addr);
    ret = ioctl(s, SIOCGIFINDEX, ifr);
    if (ret < 0) {
        NA_LOG_E("[%s] error in set SIOCGIFINDEX:%d - %d:%s",
                __FUNCTION__, ret, errno, strerror(errno));
        return;
    }

    // ret: -1, error occurs, ret: 0, invalid address, ret: 1, success;
    ret = inet_pton(AF_INET6, addr, &ifreq6.ifr6_addr);
    if (ret <= 0) {
        NA_LOG_E("[%s] ipv6 address: %s, inet_pton ret: %d", __FUNCTION__, addr, ret);
        return;
    }
    ifreq6.ifr6_prefixlen = 64;
    ifreq6.ifr6_ifindex = ifr->ifr_ifindex;

    ret = ioctl(s, SIOCSIFADDR, &ifreq6);
    if (ret < 0) {
        NA_LOG_E("[%s] error in set SIOCSIFADDR:%d - %d:%s",
                __FUNCTION__, ret, errno, strerror(errno));
    }
}

const char* NetAgentService::getCcmniInterfaceName() {
    // FIXME: To get ccmni interface name by slot ID.
    return getCcmniInterfaceName(0);
}

// Get CCMNI interface name by slot ID.
// TODO: the slot index should be used for specific projects such as DSDA.
const char* NetAgentService::getCcmniInterfaceName(int rid) {
    UNUSED(rid);
    return CCMNI_IFNAME_CCMNI;
}

//Configure the IP address to the CCMNI interface.
void NetAgentService::configureNetworkInterface(NetAgentReqInfo* pReqInfo, STATUS config) {
    struct ifreq ifr;
    unsigned int interfaceId = 0;
    NA_ADDR_TYPE addrType;
    NA_IFST state;
    char addressV4[MAX_IPV4_ADDRESS_LENGTH] = {0};
    char addressV6[MAX_IPV6_ADDRESS_LENGTH] = {0};
    char *reason = NULL;

    NA_GET_IFST_STATE(config, state);

    if (NA_GET_IF_ID(pReqInfo->pNetAgentCmdObj, &interfaceId) != NETAGENT_IO_RET_SUCCESS) {
        NA_LOG_E("[%s] fail to get interface id", __FUNCTION__);
        return;
    }

    if (config == ENABLE) {
        NA_LOG_D("[%s] push transIntfId %d to the list", __FUNCTION__, interfaceId);
        m_lTransIntfId.push_back(interfaceId);
    }
    interfaceId %= TRANSACTION_ID_OFFSET;

    if (NA_GET_ADDR_TYPE(pReqInfo->pNetAgentCmdObj, &addrType) != NETAGENT_IO_RET_SUCCESS) {
        NA_LOG_E("[%s] fail to get addr type", __FUNCTION__);
        return;
    }

    if (config == UPDATE) {
        if (NA_GET_IP_CHANGE_REASON(pReqInfo->pNetAgentCmdObj, &reason) != NETAGENT_IO_RET_SUCCESS) {
            NA_LOG_E("[%s] fail to get IP change reason", __FUNCTION__);
            reason = NULL;
        }

        NA_LOG_I("[%s] update interface %d, addr type : %s(%d), ip change reason: %s", __FUNCTION__,
                interfaceId, addrTypeToString(addrType), addrType, reason != NULL ? reason : "");
    } else {
        NA_LOG_D("[%s] interface %d to %s, addr type : %s(%d)", __FUNCTION__,
                interfaceId, config ? "UP" : "DOWN", addrTypeToString(addrType), addrType);
    }

    memset(&ifr, 0, sizeof(struct ifreq));
    snprintf(ifr.ifr_name, IFNAMSIZ, "%s%d", getCcmniInterfaceName(), interfaceId);

    nwIntfIoctlInit();

    if (config == ENABLE || config == UPDATE) {
        if (config == ENABLE) {
            // set the network interface down first before up
            // to prevent from unknown exception causing not close related
            // dev file description
            NA_LOG_D("[%s] set network interface down before up", __FUNCTION__);
            setNwIntfDown(ifr.ifr_name);
        } else {
            NA_LOG_D("[%s] reset connections", __FUNCTION__);
            ifc_reset_connections(ifr.ifr_name, addrType);
        }

        switch (addrType) {
            case NETAGENT_IO_ADDR_TYPE_IPv4:
                getIpv4Address(pReqInfo->pNetAgentCmdObj, addressV4);
                break;
            case NETAGENT_IO_ADDR_TYPE_IPv6:
                if (config == UPDATE) {
                    mIfChgForIPV6Count++;
                }
                getIpv6Address(pReqInfo->pNetAgentCmdObj, addressV6);
                break;
            case NETAGENT_IO_ADDR_TYPE_IPv4v6:
                if (config == UPDATE) {
                    mIfChgForIPV6Count++;
                }
                getIpv4v6Address(pReqInfo->pNetAgentCmdObj, addressV4, addressV6);
                break;
            default:
                NA_LOG_E("[%s] get addr type fail", __FUNCTION__);
                break;
        }

        if (strlen(addressV4) > 0) {
            if (config == ENABLE) {
                nwIntfSetFlags(sock_fd, &ifr, IFF_UP, 0);
            }
            nwIntfSetAddr(sock_fd, &ifr, addressV4);
        }

        if (strlen(addressV6) > 0) {
            if (config == ENABLE) {
                configureRSTimes(interfaceId);
                configureIPv6AddrGenMode(interfaceId);
                nwIntfSetFlags(sock6_fd, &ifr, IFF_UP, 0);
            }
            nwIntfSetIpv6Addr(sock6_fd, &ifr, addressV6);
        }
    } else {
        setNwTxqState(interfaceId, 1);
        setNwIntfDown(ifr.ifr_name);
        resetNwIntfMtu(ifr.ifr_name);
    }

    nwIntfIoctlDeInit();

    if (config == UPDATE) {
        // Send ipupdate confirm to DDM.
        if (strlen(addressV4) > 0) {
            if (addrType == NETAGENT_IO_ADDR_TYPE_IPv4
                    || addrType == NETAGENT_IO_ADDR_TYPE_IPv4v6) {
                unsigned int addrV4_;
                if (NA_GET_ADDR_V4(pReqInfo->pNetAgentCmdObj, &addrV4_) == NETAGENT_IO_RET_SUCCESS) {
                    confirmIpUpdate(interfaceId,
                            NETAGENT_IO_ADDR_TYPE_IPv4,
                            &addrV4_,
                            INVALID_IPV6_PREFIX_LENGTH);
                } else {
                    NA_LOG_E("[%s] error occurs when get addressV4", __FUNCTION__);
                }
            } else {
                NA_LOG_E("[%s] not to confirm ipupdate for invalid addrType", __FUNCTION__);
            }
        } else {
            NA_LOG_E("[%s] not to confirm ipupdate for wrong address", __FUNCTION__);
        }
    } else {
        // Interface up/down done confirm with DDM.
        confirmInterfaceState(interfaceId, state, addrType);
    }

    if (config == DISABLE) {
        NA_LOG_D("[%s] remove transIntfId %d from the list and last ReqInfo", __FUNCTION__,
                getTransIntfId(interfaceId));
        m_lTransIntfId.remove(getTransIntfId(interfaceId));
        if (m_pRouteHandler != NULL) {
            m_pRouteHandler->removeLastReqInfo(interfaceId);
        }
        clearPdnHandoverInfo(interfaceId);
        clearPdnIpInfo(interfaceId);
    }
}

void NetAgentService::configureMTUSize(NetAgentReqInfo* pReqInfo) {
    struct ifreq ifr;
    unsigned int interfaceId = 0;
    unsigned int mtuSize = 0;
    char mtu[MAX_MTU_SIZE_LENGTH] = {0};
    char *cmd = NULL;

    if (NA_GET_IF_ID(pReqInfo->pNetAgentCmdObj, &interfaceId) != NETAGENT_IO_RET_SUCCESS) {
        NA_LOG_E("[%s] fail to get interface id", __FUNCTION__);
        return;
    }
    NA_LOG_D("[%s] TransIntfId %d request to set mtu size", __FUNCTION__, interfaceId);

    interfaceId %= TRANSACTION_ID_OFFSET;
    if (NA_GET_MTU_SIZE(pReqInfo->pNetAgentCmdObj, &mtuSize) != NETAGENT_IO_RET_SUCCESS) {
        NA_LOG_E("[%s] fail to get mtu size", __FUNCTION__);
        return;
    }

    memset(&ifr, 0, sizeof(struct ifreq));
    sprintf(ifr.ifr_name, "%s%d", getCcmniInterfaceName(), interfaceId);

    sprintf(mtu, "%d", mtuSize);
    NA_LOG_D("[%s] get mtu size %d from URC", __FUNCTION__, mtuSize);

    asprintf(&cmd, "ifconfig %s mtu %s", ifr.ifr_name, mtu);
    system(cmd);
    free(cmd);
}

void NetAgentService::configureIpAdd(NetAgentReqInfo* pReqInfo) {
    int result = -1;
    struct ifreq ifr;
    unsigned int interfaceId = 0;
    NA_ADDR_TYPE addrType;
    char addressV6[MAX_IPV6_ADDRESS_LENGTH] = {0};
    unsigned int addrV6_binary[4] = {0};

    // get interface id
    if (NA_GET_IF_ID(pReqInfo->pNetAgentCmdObj, &interfaceId) != NETAGENT_IO_RET_SUCCESS) {
        NA_LOG_E("[%s] fail to get interface id", __FUNCTION__);
        return;
    }
    interfaceId %= TRANSACTION_ID_OFFSET;

    // get address type
    if (NA_GET_ADDR_TYPE(pReqInfo->pNetAgentCmdObj, &addrType) != NETAGENT_IO_RET_SUCCESS) {
        NA_LOG_E("[%s] fail to get addr type: %d", __FUNCTION__, addrType);
        return;
    }

    // get address
    if (addrType == NETAGENT_IO_ADDR_TYPE_IPv6) {
        getIpv6Address(pReqInfo->pNetAgentCmdObj, addressV6);
    } else {
        NA_LOG_E("[%s] not handle for this addr type", __FUNCTION__);
        return;
    }

    // print log
    NA_LOG_D("[%s] add ip addr to %d, addr type : %s(%d), %s", __FUNCTION__,
            interfaceId, addrTypeToString(addrType), addrType, addressV6);

    memset(&ifr, 0, sizeof(struct ifreq));
    snprintf(ifr.ifr_name, IFNAMSIZ, "%s%d", getCcmniInterfaceName(), interfaceId);

    if (strlen(addressV6) > 0) {
        // check if link local address
        if (!isIpv6Global(addressV6)) {
            // disable RS
            //configureRSTimes(interfaceId, 0);
            // set link local address to kernel
            ifc_add_address(ifr.ifr_name, addressV6, IPV6_REFIX_LENGTH);
            NA_LOG_D("[%s] add link local address: %s", __FUNCTION__, addressV6);
            //string to binary (addressV6 -> addrV6_ll)
            if (convertIpv6ToBinary(addrV6_binary, addressV6) < 0) {
                NA_LOG_E("[%s] fail to convert ipv6 address to binary", __FUNCTION__);
                return;
            }
            if (isMultiHomingFeatureSupport) {
                result = 99;
            } else {
                result = 0;
            }
            confirmIpAddOrDel(interfaceId, NETAGENT_IO_CMD_IPADD, result, addrType, addrV6_binary, INVALID_IPV6_PREFIX_LENGTH);
        } else {
            NA_LOG_E("[%s] is not link local address", __FUNCTION__);
            return;
        }
    } else {
        NA_LOG_E("[%s] not get address", __FUNCTION__);
        return;
    }
}

void NetAgentService::configureIpDel(NetAgentReqInfo* pReqInfo) {
    int result = -1;
    struct ifreq ifr;
    unsigned int interfaceId = 0;
    NA_ADDR_TYPE addrType;
    char addressV4[MAX_IPV4_ADDRESS_LENGTH] = {0};
    char addressV6[MAX_IPV6_ADDRESS_LENGTH] = {0};
    unsigned int addrV6_binary[4] = {0};

    // get interface id
    if (NA_GET_IF_ID(pReqInfo->pNetAgentCmdObj, &interfaceId) != NETAGENT_IO_RET_SUCCESS) {
        NA_LOG_E("[%s] fail to get interface id", __FUNCTION__);
        return;
    }
    interfaceId %= TRANSACTION_ID_OFFSET;

    // get address type
    if (NA_GET_ADDR_TYPE(pReqInfo->pNetAgentCmdObj, &addrType) != NETAGENT_IO_RET_SUCCESS) {
        NA_LOG_E("[%s] fail to get addr type", __FUNCTION__);
        return;
    }

    // get address
    if (addrType == NETAGENT_IO_ADDR_TYPE_IPv6) {
        getIpv6Address(pReqInfo->pNetAgentCmdObj, addressV6);
    } else {
        NA_LOG_E("[%s] not handle for this addr type", __FUNCTION__);
        return;
    }

    // print log
    NA_LOG_D("[%s] del ip addr to %d, addr type : %s(%d), %s", __FUNCTION__,
            interfaceId, addrTypeToString(addrType), addrType, addressV6);

    memset(&ifr, 0, sizeof(struct ifreq));
    snprintf(ifr.ifr_name, IFNAMSIZ, "%s%d", getCcmniInterfaceName(), interfaceId);

    if (strlen(addressV6) > 0) {
        ifc_del_address(ifr.ifr_name, addressV6, IPV6_REFIX_LENGTH);
        NA_LOG_D("[%s] del IP address: %s", __FUNCTION__, addressV6);

        //string to binary (addressV6 -> addrV6_ll)
        if (convertIpv6ToBinary(addrV6_binary, addressV6) < 0) {
            NA_LOG_E("[%s] fail to convert ipv6 address to binary", __FUNCTION__);
            return;
        }
        if (isMultiHomingFeatureSupport) {
            result = 99;
        } else {
            result = 0;
        }
        recordPdnIpInfo(interfaceId, addrType,addressV4, addressV6);
        confirmIpAddOrDel(interfaceId, NETAGENT_IO_CMD_IPDEL, result, addrType, addrV6_binary, INVALID_IPV6_PREFIX_LENGTH);
    } else {
        NA_LOG_E("[%s] not get address", __FUNCTION__);
        return;
    }
}


void NetAgentService::configureRSTimes(int interfaceId) {
    char *cmd = NULL;
    char rs_times[PROPERTY_VALUE_MAX] = { 0 };
    property_get("persist.vendor.ril.rs_times", rs_times, "3");
    asprintf(&cmd, "echo %s > /proc/sys/net/ipv6/conf/%s%d/router_solicitations",
            rs_times, getCcmniInterfaceName(), interfaceId);
    if (cmd != NULL) {
        NA_LOG_D("[%s] cmd = %s", __FUNCTION__, cmd);
        system(cmd);
        free(cmd);
    } else {
        NA_LOG_E("[%s] cmd is NULL", __FUNCTION__);
    }
}

void NetAgentService::configureRSTimes(int interfaceId, int times) {
    char *cmd = NULL;
    asprintf(&cmd, "echo %d > /proc/sys/net/ipv6/conf/%s%d/router_solicitations",
            times, getCcmniInterfaceName(), interfaceId);
    if (cmd != NULL) {
        NA_LOG_D("[%s] cmd = %s", __FUNCTION__, cmd);
        system(cmd);
        free(cmd);
    } else {
        NA_LOG_E("[%s] cmd is NULL", __FUNCTION__);
    }
}

void NetAgentService::queryArp(NetAgentReqInfo* pReqInfo) {
        /*+EWIFIMAC: <ifname>, <ip>*/
        thread_args * args;
        NA_LOG_D("[%s] Enter NETAGENT_IO_CMD_ARP_QUERY event", __FUNCTION__);
        NA_ARP_INFO *arp = (NA_ARP_INFO *)
                calloc(1, sizeof(NA_ARP_INFO));

        if (!arp) {
            NA_LOG_E("[%s] fail to alloc ARP info", __FUNCTION__);
            return;
        }
        if ((NA_GET_ARP(pReqInfo->pNetAgentCmdObj, arp)) != NETAGENT_IO_RET_SUCCESS) {
            NA_LOG_E("[%s] fail to get ARP info", __FUNCTION__);
            free(arp);
            return;
        }
        args = (thread_args*) calloc(1, sizeof(thread_args));
        if (!args) {
            NA_LOG_E("[%s] fail to alloc thread_args info", __FUNCTION__);
            free(arp);
            return;
        }
        args->instance = this;
        args->arp = arp;
        pthread_t mArpThread;
        pthread_create(&mArpThread, NULL, NetAgentService::queryArpThread, args);
}

void * NetAgentService::queryArpThread(void *arg) {
        thread_args *args = (thread_args *) arg;
        NA_ARP_INFO * arp = args->arp;
        void *pNetAgentCmdObj = 0;
        pNetAgentCmdObj = NA_CMD_ARP_RESULT_ALLOC(arp);
        if (pNetAgentCmdObj) {
            //enque the result, thus, at cmd sent in same thread.
            args->instance->enqueueReqInfo(pNetAgentCmdObj, REQUEST_TYPE_DDM);
        }
        free (arp);
        free (args);
        return NULL;
}

void NetAgentService::sendArpResult(NetAgentReqInfo* pReqInfo) {
        if (NA_CMD_SEND(m_pNetAgentIoObj, pReqInfo->pNetAgentCmdObj) != NETAGENT_IO_RET_SUCCESS) {
            NA_LOG_E("[%s] send ARP Result info fail", __FUNCTION__);
        }
}

void NetAgentService::queryNAPTR(NetAgentReqInfo* pReqInfo) {
        /*+ENAPTR: <trans_id(not same as transferid)>,<mod_id>,<fqdn>*/
        int ret = 0;
        NA_LOG_D("[%s] Enter NETAGENT_IO_CMD_NAPTR_QUERY event", __FUNCTION__);
        NA_NAPTR_INFO *m_naptr = (NA_NAPTR_INFO *)calloc(1, sizeof(NA_NAPTR_INFO));
        memset(m_naptr, 0, sizeof(NA_NAPTR_INFO));
        if ((NA_GET_NAPTR(pReqInfo->pNetAgentCmdObj, m_naptr)) != NETAGENT_IO_RET_SUCCESS) {
            NA_LOG_E("[%s] fail to get NAPTR info", __FUNCTION__);
            free(m_naptr);
            //should add at cmd to handle the urc fail
            return;
        }

        NA_LOG_D("[%s] get NAPTR trans_id: %d, moduleid: %s, fqdn: %s from URC", __FUNCTION__,
                       m_naptr->trans_id, m_naptr->mod_id, m_naptr->fqdn);
        m_naptrMap[m_naptr->trans_id] = m_naptr;

        struct result_naptr_in_netagent* result_list = NULL;
        result_list = (result_naptr_in_netagent*)calloc(1, sizeof(result_naptr_in_netagent));
        memset(result_list, 0, sizeof(result_naptr_in_netagent));

        result_list->trans_id = m_naptr->trans_id;
        strncpy(result_list->mod_id, m_naptr->mod_id, sizeof(result_list->mod_id)-1);

        thread_args args;
        args.instance = this;
        args.result_list = result_list;
        struct timespec ts;
        if (clock_gettime(CLOCK_MONOTONIC, &ts) != -1) {
            ts.tv_sec += 5;
        }

        pthread_t mNAPTRThread;
        pthread_mutex_lock(&s_mutex);
        pthread_create(&mNAPTRThread, NULL, NetAgentService::naptrThreadStart, &args);
        ret = pthread_cond_timedwait(&s_cond, &s_mutex, &ts);
        pthread_mutex_unlock(&s_mutex);
        if (ret == ETIMEDOUT) {
             NA_LOG_E("[%s] Force to quit child thread due to timeout", __FUNCTION__);
             result_list->result = 0;
             respondNAPTRinfo(NETAGENT_IO_CMD_NAPTR_SEND, result_list);
             pthread_kill(mNAPTRThread , 0);
        }
}

void *NetAgentService::naptrThreadStart(void *arg) {
    thread_args *args =(thread_args*)arg;
    NA_LOG_D("[%s] Threads created", __FUNCTION__);
    args->instance->getNAPTRinfo(args->result_list);
    return NULL;
}

void NetAgentService::getNAPTRinfo(struct result_naptr_in_netagent *result_list) {
    NA_LOG_D("[%s] Entering getNAPTRinfo", __FUNCTION__);

    unsigned int* ret_val = NULL;

    struct records_naptr* ptr = NULL, *result_getrecords = NULL;
    if (result_list == NULL) {
        NA_LOG_D("[%s] Exiting getNAPTRinfo due to result_list==NULL", __FUNCTION__);
        return;
        }
    NA_LOG_D("[%s] result_list->trans_id=%d, m_naptrMap[result_list->trans_id]->fqdn=%s ",
        __FUNCTION__,result_list->trans_id, m_naptrMap[result_list->trans_id]->fqdn);
    int result = 0;


    result = mtk_aes_getrecords(m_naptrMap[result_list->trans_id]->fqdn, NULL, NULL, &result_getrecords);

    if (result != 1) {
        NA_LOG_E("[%s] mtk_aes_getrecords error! result = %d", __FUNCTION__, result);
        result_list->result = 0; //for MD part
        result_list->flags = "";
        result_list->service = "";
        result_list->regexp = "";
        NA_LOG_D("[%s] AT+ENAPTR= %d, %s, %d, %d, %d, %s, %s, %s ,%s ",
            __FUNCTION__,result_list->trans_id, result_list->mod_id,result_list->result,
            result_list->order,result_list->pref,result_list->flags,result_list->service,
            result_list->regexp,result_list->fqdn);
        respondNAPTRinfo(NETAGENT_IO_CMD_NAPTR_SEND, result_list);

        pthread_mutex_lock(&s_mutex);
        pthread_cond_signal(&s_cond);
        pthread_mutex_unlock(&s_mutex);
        pthread_exit((void *)ret_val);

    }
    else {
        ptr = result_getrecords;
    // AT+ENAPTR=<trans_id>,<mod_id>,<result>,<order>,<pref>,<flags>,<service>,<regexp>,<replacement>
        do {
            result_list->result = 1;
            result_list->order = ptr->order;
            result_list->pref = ptr->pref;

            if (ptr->flags == NULL) {
                result_list->flags = "";
            } else {
                result_list->flags = ptr->flags;
            }

            if (ptr->service == NULL) {
                result_list->service = "";
            } else {
                result_list->service = ptr->service;
            }

            if (ptr->regexp == NULL) {
                result_list->regexp = "";
            } else {
                result_list->regexp = ptr->regexp;
            }

            strncpy(result_list->fqdn, ptr->fqdn, sizeof(result_list->fqdn)-1);
            NA_LOG_D("[%s] AT+ENAPTR= %d, %s, %d, %d, %d, %s, %s, %s ,%s",
                __FUNCTION__,result_list->trans_id, result_list->mod_id,result_list->result,
                result_list->order,result_list->pref,result_list->flags,result_list->service,
                result_list->regexp,result_list->fqdn);
            respondNAPTRinfo(NETAGENT_IO_CMD_NAPTR_SEND, result_list);
            ptr = ptr->next;

        } while(ptr);

        //when ptr=NULL, means all records are received and send
        //send an end command to MD
        result_list->result = 1;
        result_list->order = 0;
        result_list->pref = 0;
        result_list->flags = "";
        result_list->service = "";
        result_list->regexp = "";
        memset(result_list->fqdn, 0, sizeof(result_list->fqdn));
        NA_LOG_D("[%s] AT+ENAPTR= %d, %s, %d, %d, %d, %s, %s, %s ,%s ",
            __FUNCTION__,result_list->trans_id, result_list->mod_id,result_list->result,
            result_list->order,result_list->pref,result_list->flags,result_list->service,
            result_list->regexp,result_list->fqdn);
        respondNAPTRinfo(NETAGENT_IO_CMD_NAPTR_SEND, result_list);

        mtk_aes_getrecords_free(result_getrecords);

        clearNAPTRMapInfo(result_list->trans_id);
        free(result_list);

        pthread_mutex_lock(&s_mutex);
        pthread_cond_signal(&s_cond);
        pthread_mutex_unlock(&s_mutex);
        pthread_exit((void *)ret_val);
    }
}

void NetAgentService::respondNAPTRinfo(netagent_io_cmd_e cmd, struct result_naptr_in_netagent* result_list) {
    void *pNetAgentCmdObj = 0;
    pNetAgentCmdObj = NA_CMD_NAPTR_ALLOC(cmd, result_list);
    if (NA_CMD_SEND(m_pNetAgentIoObj, pNetAgentCmdObj) != NETAGENT_IO_RET_SUCCESS) {
        NA_LOG_E("[%s] send NAPTR info fail", __FUNCTION__);
    }
    NA_CMD_FREE(pNetAgentCmdObj);
}

bool NetAgentService::clearNAPTRMapInfo(unsigned int trans_id) {
    if (m_naptrMap.count(trans_id) > 0) {
        NA_NAPTR_INFO *m_naptr = m_naptrMap[trans_id];
        m_naptrMap.erase(trans_id);
        if (m_naptr != NULL) {
            free(m_naptr);
        }
        return true;
    }
    return false;
}

void NetAgentService::reserveTcpUdpPort(NetAgentReqInfo* pReqInfo) {
    unsigned int transactionId = 0;
    NA_CMD cmd;
    unsigned int rsvnAction = 0;
    int interfaceId = 0;
    NA_ADDR_TYPE addrType;
    char addressV4[MAX_IPV4_ADDRESS_LENGTH] = {0};
    char addressV6[MAX_IPV6_ADDRESS_LENGTH] = {0};
    int port = 0;
    int socketDomain = 0;
    int socketType = 0;
    int socketFD = -1;
    char ifrName[IFNAMSIZ] = {0};
    struct sockaddr_in socketAddrV4;
    struct sockaddr_in6 socketAddrV6;
    unsigned int response = 0;

    if (NA_GET_IF_ID(pReqInfo->pNetAgentCmdObj, &transactionId) != NETAGENT_IO_RET_SUCCESS) {
        NA_LOG_E("[%s] fail to get interface id", __FUNCTION__);
        return;
    }
    if (NA_CMD_TYPE(pReqInfo->pNetAgentCmdObj, &cmd) != NETAGENT_IO_RET_SUCCESS) {
        NA_LOG_E("[%s] fail to get cmd type", __FUNCTION__);
        return;
    }
    if (NA_GET_RSVN_ACTION(pReqInfo->pNetAgentCmdObj, &rsvnAction) != NETAGENT_IO_RET_SUCCESS) {
        NA_LOG_E("[%s] fail to get rsvn action", __FUNCTION__);
        return;
    }
    if (NA_GET_RSVN_IF_ID(pReqInfo->pNetAgentCmdObj, &interfaceId) != NETAGENT_IO_RET_SUCCESS) {
        NA_LOG_E("[%s] fail to get rsvn interfaceId", __FUNCTION__);
        return;
    }
    if (NA_GET_ADDR_TYPE(pReqInfo->pNetAgentCmdObj, &addrType) != NETAGENT_IO_RET_SUCCESS) {
        NA_LOG_E("[%s] fail to get rsvn addr type", __FUNCTION__);
        return;
    }
    if (NA_GET_RSVN_PORT(pReqInfo->pNetAgentCmdObj, &port) != NETAGENT_IO_RET_SUCCESS) {
        NA_LOG_E("[%s] fail to get rsvn port", __FUNCTION__);
        return;
    }

    // 0: alloc, 1: free
    if (rsvnAction == 1) {
        if (m_rsvnFdMap.count(port) > 0) {
            socketFD = m_rsvnFdMap[port];
            close(socketFD);
            m_rsvnFdMap.erase(port);
            response = 1;
        }
    } else {
        if (addrType == NETAGENT_IO_ADDR_TYPE_IPv4) {
            socketDomain = AF_INET;
        } else {
            socketDomain = AF_INET6;
        }
        if (cmd == NETAGENT_IO_CMD_TCP_RSVN) {
            socketType = SOCK_STREAM;
        } else {
            socketType = SOCK_DGRAM;
        }
        if ((socketFD = socket(socketDomain, socketType, 0)) < 0) {
            NA_LOG_E("[%s] Unable to create socket: %s", __FUNCTION__, strerror(errno));
        } else {
            if (interfaceId != -1) {
                if (interfaceId == 99) {
                    strncpy(ifrName, WIFI_IF_NAME, IFNAMSIZ-1);
                } else {
                    snprintf(ifrName, IFNAMSIZ, "%s%d", getCcmniInterfaceName(), interfaceId);
                }
                if (setsockopt(socketFD, SOL_SOCKET, SO_BINDTODEVICE, ifrName, strlen(ifrName)) < 0) {
                    NA_LOG_E("[%s] Unable to set SO_BINDTODEVICE option: %s",
                            __FUNCTION__, strerror(errno));
                    close(socketFD);
                    socketFD = -1;
                }
            }
            if (socketFD >= 0) {
                if (addrType == NETAGENT_IO_ADDR_TYPE_IPv4) {
                    memset(&socketAddrV4, 0, sizeof(socketAddrV4));
                    socketAddrV4.sin_family = AF_INET;
                    socketAddrV4.sin_port = htons(port);
                    getIpv4Address(pReqInfo->pNetAgentCmdObj, addressV4);
                    socketAddrV4.sin_addr.s_addr = inet_addr(addressV4);
                    if (bind(socketFD, (struct sockaddr *) &socketAddrV4, sizeof(socketAddrV4)) < 0) {
                        NA_LOG_E("[%s] Unable to bind socket with v4 addresss: %s",
                                __FUNCTION__, strerror(errno));
                        close(socketFD);
                        socketFD = -1;
                    }
                } else {
                    memset(&socketAddrV6, 0, sizeof(socketAddrV6));
                    socketAddrV6.sin6_family = AF_INET6;
                    socketAddrV6.sin6_port = htons(port);
                    getIpv6Address(pReqInfo->pNetAgentCmdObj, addressV6);
                    inet_pton(AF_INET6, addressV6, &(socketAddrV6.sin6_addr));
                    if (bind(socketFD, (struct sockaddr *) &socketAddrV6, sizeof(socketAddrV6)) < 0) {
                        NA_LOG_E("[%s] Unable to bind socket with v6 address: %s",
                                __FUNCTION__, strerror(errno));
                        close(socketFD);
                        socketFD = -1;
                    }
                }
            }
        }
        if (socketFD >= 0) {
            m_rsvnFdMap[port] = socketFD;
            response = 1;
        }
    }

    confirmTcpUdpRsvn(transactionId, cmd, rsvnAction, response);
}

int NetAgentService::nanl_open(struct nanl_handle *nah, int protocol) {
    int sndbuf = 32768;
    int rcvbuf = 1024 * 1024;
    int one = 1;
    socklen_t addr_len;

    memset(nah, 0, sizeof(*nah));

    nah->proto = protocol;
    nah->fd = socket(AF_NETLINK, SOCK_RAW | SOCK_CLOEXEC, protocol);
    if (nah->fd < 0) {
        NA_LOG_E("[%s] Cannot open netlink socket: %s(%d)", __FUNCTION__, strerror(errno), errno);
        return -1;
    }
    if (setsockopt(nah->fd, SOL_SOCKET, SO_SNDBUF, &sndbuf, sizeof(sndbuf)) < 0) {
        NA_LOG_E("[%s] Fail to SO_SNDBUF: %s(%d)", __FUNCTION__, strerror(errno), errno);
        return -1;
    }
    if (setsockopt(nah->fd, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(rcvbuf)) < 0) {
        NA_LOG_E("[%s] Fail to SO_RCVBUF: %s(%d)", __FUNCTION__, strerror(errno), errno);
        return -1;
    }
    /* Older kernels may no support extended ACK reporting */
    setsockopt(nah->fd, SOL_NETLINK, NETLINK_EXT_ACK, &one, sizeof(one));

    memset(&nah->local, 0, sizeof(nah->local));
    nah->local.nl_family = AF_NETLINK;

    if (bind(nah->fd, (struct sockaddr *)&nah->local, sizeof(nah->local)) < 0) {
        NA_LOG_E("[%s] Cannot bind netlink socket: %s(%d)", __FUNCTION__, strerror(errno), errno);
        return -1;
    }
    addr_len = sizeof(nah->local);
    if (getsockname(nah->fd, (struct sockaddr *)&nah->local, &addr_len) < 0) {
        NA_LOG_E("[%s] Cannot getsockname: %s(%d)", __FUNCTION__, strerror(errno), errno);
        return -1;
    }
    if (addr_len != sizeof(nah->local)) {
        NA_LOG_E("[%s] Wrong address length: %d", __FUNCTION__, addr_len);
        return -1;
    }
    if (nah->local.nl_family != AF_NETLINK) {
        NA_LOG_E("[%s] Wrong address family: %d", __FUNCTION__, nah->local.nl_family);
        return -1;
    }
    nah->seq = time(NULL);
    return 0;
}

int NetAgentService::nanl_talk(struct nanl_handle *nah, struct nlmsghdr *n,
        struct nlmsghdr *answer, size_t maxlen) {
    int status;
    unsigned int seq = 0;
    struct nlmsghdr *h;
    struct sockaddr_nl nladdr = { .nl_family = AF_NETLINK };
    struct iovec iov = {
        .iov_base = n,
        .iov_len = n->nlmsg_len
    };
    struct msghdr msg = {
        .msg_name = &nladdr,
        .msg_namelen = sizeof(nladdr),
        .msg_iov = &iov,
        .msg_iovlen = 1,
    };
    char buf[32768] = {};

    n->nlmsg_seq = seq = ++nah->seq;

    if (answer == NULL) {
        n->nlmsg_flags |= NLM_F_ACK;
    }
    status = sendmsg(nah->fd, &msg, 0);
    if (status < 0) {
        NA_LOG_E("[%s] Cannot talk to rtnetlink: %s(%d)", __FUNCTION__, strerror(errno), errno);
        return -1;
    }

    iov.iov_base = buf;
    while (1) {
        iov.iov_len = sizeof(buf);
        status = recvmsg(nah->fd, &msg, 0);

        if (status < 0) {
            if (errno == EINTR || errno == EAGAIN) {
                continue;
            }
            NA_LOG_E("[%s] netlink receive error: %s(%d)", __FUNCTION__, strerror(errno), errno);
            return -1;
        }
        if (status == 0) {
            NA_LOG_E("[%s] EOF on netlink", __FUNCTION__);
            return -1;
        }
        if (msg.msg_namelen != sizeof(nladdr)) {
            NA_LOG_E("[%s] sender address length == %d", __FUNCTION__, msg.msg_namelen);
            return -1;
        }
        for (h = (struct nlmsghdr *)buf; status >= (int)sizeof(*h); ) {
            int len = h->nlmsg_len;
            int l = len - sizeof(*h);

            if (l < 0 || len > status) {
                if (msg.msg_flags & MSG_TRUNC) {
                    NA_LOG_E("[%s] Truncated message", __FUNCTION__);
                    return -1;
                }
                NA_LOG_E("[%s] !!!malformed message: len=%d", __FUNCTION__, len);
                return -1;
            }

            if (nladdr.nl_pid != 0 ||h->nlmsg_pid != nah->local.nl_pid ||h->nlmsg_seq != seq) {
                /* Don't forget to skip that message. */
                status -= NLMSG_ALIGN(len);
                h = (struct nlmsghdr *)((char *)h + NLMSG_ALIGN(len));
                continue;
            }

            if (h->nlmsg_type == NLMSG_ERROR) {
                struct nlmsgerr *err = (struct nlmsgerr *)NLMSG_DATA(h);

                if (l < (int)sizeof(struct nlmsgerr)) {
                    NA_LOG_E("[%s] ERROR truncated", __FUNCTION__);
                } else if (!err->error) {
                    if (answer) {
                        memcpy(answer, h, ((maxlen) < (h->nlmsg_len) ? (maxlen) : (h->nlmsg_len)));
                    }
                    return 0;
                }

                /*if (rtnl->proto != NETLINK_SOCK_DIAG && show_rtnl_err) {
                    rtnl_talk_error(h, err, errfn);
                }*/

                errno = -err->error;
                NA_LOG_E("[%s] NLMSG_ERROR: %s(%d)", __FUNCTION__, strerror(errno), errno);
                return -1;
            }

            if (answer) {
                memcpy(answer, h, ((maxlen) < (h->nlmsg_len) ? (maxlen) : (h->nlmsg_len)));
                return 0;
            }

            NA_LOG_E("[%s] Unexpected reply!!!", __FUNCTION__);

            status -= NLMSG_ALIGN(len);
            h = (struct nlmsghdr *)((char *)h + NLMSG_ALIGN(len));
        }

        if (msg.msg_flags & MSG_TRUNC) {
            NA_LOG_E("[%s] Message truncated", __FUNCTION__);
            continue;
        }

        if (status) {
            NA_LOG_E("[%s] !!!Remnant of size %d", __FUNCTION__, status);
            return -1;
        }
    }
}

void NetAgentService::nanl_close(struct nanl_handle *nah) {
    if (nah->fd >= 0) {
        close(nah->fd);
        nah->fd = -1;
    }
}

void NetAgentService::reserveSpi(NetAgentReqInfo* pReqInfo) {
    struct {
        struct nlmsghdr n;
        union {
            struct xfrm_userspi_info xspi;
            struct xfrm_usersa_id xsid;
        };
    } req = {
        .n.nlmsg_flags = NLM_F_REQUEST,
    };
    unsigned int transactionId;
    unsigned int action;
    NA_ADDR_TYPE addrType;
    unsigned int protocol = 0;
    unsigned int mode = 0;
    unsigned int v4Address;
    unsigned int v6Address[4];
    char v6AddressString[MAX_IPV6_ADDRESS_LENGTH] = {0};
    unsigned int min = 0;
    unsigned int max = 0;
    struct nanl_handle nah;

    if (NA_GET_IF_ID(pReqInfo->pNetAgentCmdObj, &transactionId) != NETAGENT_IO_RET_SUCCESS) {
        NA_LOG_E("[%s] fail to get interface id", __FUNCTION__);
        return;
    }
    if (NA_GET_RSVN_ACTION(pReqInfo->pNetAgentCmdObj, &action) != NETAGENT_IO_RET_SUCCESS) {
        NA_LOG_E("[%s] fail to get rsvn action", __FUNCTION__);
        return;
    }
    if (NA_GET_ADDR_TYPE(pReqInfo->pNetAgentCmdObj, &addrType) != NETAGENT_IO_RET_SUCCESS) {
        NA_LOG_E("[%s] fail to get rsvn addr type", __FUNCTION__);
        return;
    }
    if (NA_GET_SPI_PROTOCOL(pReqInfo->pNetAgentCmdObj, &protocol) != NETAGENT_IO_RET_SUCCESS) {
        NA_LOG_E("[%s] fail to get spi protocol", __FUNCTION__);
        return;
    }
    if (NA_GET_SPI_MODE(pReqInfo->pNetAgentCmdObj, &mode) != NETAGENT_IO_RET_SUCCESS) {
        NA_LOG_E("[%s] fail to get spi mode", __FUNCTION__);
        return;
    }

    // 0: alloc, 1: free
    if (action == 1) {
        req.n.nlmsg_len = NLMSG_LENGTH(sizeof(req.xsid));
        req.n.nlmsg_type = XFRM_MSG_DELSA;
        req.xsid.spi = htonl(mode);
        req.xsid.proto = protocol;
        memset(&req.xsid.daddr, 0, sizeof(req.xsid.daddr));
        if (addrType == NETAGENT_IO_ADDR_TYPE_IPv4) {
            req.xsid.family = AF_INET;
            if (NA_GET_SPI_DST_ADDR(pReqInfo->pNetAgentCmdObj, &v4Address) != NETAGENT_IO_RET_SUCCESS) {
                NA_LOG_E("[%s] fail to get spi dst v4 addr", __FUNCTION__);
                return;
            }
            memcpy(&req.xsid.daddr, &v4Address, sizeof(v4Address));
        } else {
            req.xsid.family = AF_INET6;
            if (NA_GET_SPI_DST_ADDR(pReqInfo->pNetAgentCmdObj, v6Address) != NETAGENT_IO_RET_SUCCESS) {
                NA_LOG_E("[%s] fail to get spi dst v6 addr", __FUNCTION__);
                return;
            }
            convertIpv6ToString(v6AddressString, v6Address);
            convertIpv6ToBinary((unsigned int *)&req.xsid.daddr, v6AddressString);
        }
    } else {
        req.n.nlmsg_len = NLMSG_LENGTH(sizeof(req.xspi));
        req.n.nlmsg_type = XFRM_MSG_ALLOCSPI;
        memset(&req.xspi.info.saddr, 0, sizeof(req.xspi.info.saddr));
        memset(&req.xspi.info.id.daddr, 0, sizeof(req.xspi.info.id.daddr));
        if (addrType == NETAGENT_IO_ADDR_TYPE_IPv4) {
            req.xspi.info.family = AF_INET;
            if (NA_GET_SPI_SRC_ADDR(pReqInfo->pNetAgentCmdObj, &v4Address) != NETAGENT_IO_RET_SUCCESS) {
                NA_LOG_E("[%s] fail to get spi src v4 addr", __FUNCTION__);
                return;
            }
            memcpy(&req.xspi.info.saddr, &v4Address, sizeof(v4Address));
            if (NA_GET_SPI_DST_ADDR(pReqInfo->pNetAgentCmdObj, &v4Address) != NETAGENT_IO_RET_SUCCESS) {
                NA_LOG_E("[%s] fail to get spi dst v4 addr", __FUNCTION__);
                return;
            }
            memcpy(&req.xspi.info.id.daddr, &v4Address, sizeof(v4Address));
        } else {
            req.xspi.info.family = AF_INET6;
            if (NA_GET_SPI_SRC_ADDR(pReqInfo->pNetAgentCmdObj, v6Address) != NETAGENT_IO_RET_SUCCESS) {
                NA_LOG_E("[%s] fail to get spi src v6 addr", __FUNCTION__);
                return;
            }
            convertIpv6ToString(v6AddressString, v6Address);
            convertIpv6ToBinary((unsigned int *)&req.xspi.info.saddr, v6AddressString);
            if (NA_GET_SPI_DST_ADDR(pReqInfo->pNetAgentCmdObj, v6Address) != NETAGENT_IO_RET_SUCCESS) {
                NA_LOG_E("[%s] fail to get spi dst v6 addr", __FUNCTION__);
                return;
            }
            convertIpv6ToString(v6AddressString, v6Address);
            convertIpv6ToBinary((unsigned int *)&req.xspi.info.id.daddr, v6AddressString);
        }
        req.xspi.info.id.proto = protocol;
        req.xspi.info.mode = mode;
        if (NA_GET_SPI_MIN_MAX(pReqInfo->pNetAgentCmdObj, &min, &max) != NETAGENT_IO_RET_SUCCESS) {
            NA_LOG_E("[%s] fail to get spi min/max", __FUNCTION__);
            return;
        }
        req.xspi.min = min;
        req.xspi.max = max;
    }

    if (nanl_open(&nah, NETLINK_XFRM) < 0) {
        NA_LOG_E("[%s] fail to nanl_open()", __FUNCTION__);
        confirmSpi(transactionId, action, 0);
    } else {
        if (action == 1) {
            if (nanl_talk(&nah, &req.n, NULL, 0) < 0) {
                NA_LOG_E("[%s] fail to nanl_talk()", __FUNCTION__);
                confirmSpi(transactionId, action, 0);
            } else {
                confirmSpi(transactionId, action, 1);
            }
        } else {
            char res_buf[NLMSG_BUF_SIZE] = {};
            struct nlmsghdr *res_n = (struct nlmsghdr *)res_buf;

            if (nanl_talk(&nah, &req.n, res_n, sizeof(res_buf)) < 0) {
                NA_LOG_E("[%s] fail to nanl_talk()", __FUNCTION__);
                confirmSpi(transactionId, action, 0);
            } else {
                struct xfrm_usersa_info *xsinfo = (struct xfrm_usersa_info *)NLMSG_DATA(res_n);
                unsigned int spi = ntohl(xsinfo->id.spi);
                NA_LOG_D("[%s] spi: %d", __FUNCTION__, spi);
                confirmSpi(transactionId, action, spi);
            }
        }
    }
    nanl_close(&nah);
}

void NetAgentService::configureIPv6AddrGenMode(int interfaceId) {
    char optr[PROPERTY_VALUE_MAX] = {0};

    property_get("persist.vendor.operator.optr", optr, "");

    if (0 == strcasecmp(optr, "OP07")) {
        const char UNINITIALIZED[] = "uninitialized";
        const char KEY_STABLE_SECRET[] = "persist.vendor.radio.stable_secret";
        char stable_secret[PROPERTY_VALUE_MAX] = { 0 };
        int fd = 0;
        const char *PATH_RANDOM_IPV6_ADDRESS = "/dev/random";
        in6_addr addr = {};
        char strRandomIPv6Address[INET6_ADDRSTRLEN] = {0};
        int ret = 0;
        char *pathStableSecret = NULL;

        property_get(KEY_STABLE_SECRET, stable_secret, UNINITIALIZED);

        // Check if the random secret from property is initialized.
        if (0 == strcasecmp(stable_secret, UNINITIALIZED)) {
            // Read the random ipv6 address from '/dev/random'.
            fd = open(PATH_RANDOM_IPV6_ADDRESS, O_RDONLY);
            if (fd < 0) {
                NA_LOG_E("[%s] Failed to open '%s': %s", __FUNCTION__, PATH_RANDOM_IPV6_ADDRESS,
                        strerror(errno));
                return;
            }
            ret = read(fd, &addr, sizeof(addr));
            if (ret <= 0) {
                NA_LOG_E("[%s] Failed to read '%s': %s", __FUNCTION__, PATH_RANDOM_IPV6_ADDRESS,
                        strerror(errno));
                close(fd);
                return;
            }
            if (NULL == inet_ntop(AF_INET6, &addr, strRandomIPv6Address, INET6_ADDRSTRLEN)) {
                NA_LOG_E("[%s] Failed to 'inet_ntop': %s", __FUNCTION__, strerror(errno));
                close(fd);
                return;
            }
            NA_LOG_I("[%s] randomIPv6Address = %s", __FUNCTION__, strRandomIPv6Address);
            close(fd);

            // Store the random secret to the persist property.
            property_set(KEY_STABLE_SECRET, strRandomIPv6Address);
        } else {
            NA_LOG_I("[%s] Old secret is initialized: %s", __FUNCTION__, stable_secret);

            // Copy the random secret from the persist property.
            strncpy(strRandomIPv6Address, stable_secret, INET6_ADDRSTRLEN);
        }

        // Store the random secret to '/proc/sys/net/ipv6/conf/ccmniX/stable_secret'.
        asprintf(&pathStableSecret, "/proc/sys/net/ipv6/conf/%s%d/stable_secret",
                getCcmniInterfaceName(), interfaceId);
        if (NULL == pathStableSecret) {
            NA_LOG_E("[%s] pathStableSecret is NULL", __FUNCTION__);
            return;
        }
        fd = open(pathStableSecret, O_CLOEXEC | O_WRONLY);
        if (fd < 0) {
            NA_LOG_E("[%s] Failed to open '%s': %s", __FUNCTION__, pathStableSecret,
                    strerror(errno));
            free(pathStableSecret);
            return;
        }
        ret = write(fd, strRandomIPv6Address, strlen(strRandomIPv6Address));
        if (ret <= 0) {
            NA_LOG_E("[%s] Failed to write '%s': %s", __FUNCTION__, pathStableSecret,
                    strerror(errno));
            free(pathStableSecret);
            close(fd);
            return;
        }
        free(pathStableSecret);
        close(fd);
    }
}

void NetAgentService::updateIpv6GlobalAddress(NetAgentReqInfo* pReqInfo) {
    char address[INET6_ADDRSTRLEN] = "";
    int ipv6PrefixLength = INVALID_IPV6_PREFIX_LENGTH;
    unsigned int addrV6_[4];
    unsigned int interfaceId = INVALID_INTERFACE_ID;
    ACTION action = ACTION_UNKNOWN;
    NA_RA flag = NETAGENT_IO_NO_RA_INITIAL;
    NA_CMD cmd;
    int result = -1;

    if (m_pRouteHandler == NULL) {
        NA_LOG_E("[%s] m_pRouteHandler is NULL", __FUNCTION__);
        return;
    }

    if (m_pRouteHandler->getAddress(pReqInfo->pNetAgentCmdObj, address) != NETLINK_RET_SUCCESS) {
        NA_LOG_E("[%s] fail to get address", __FUNCTION__);
        return;
    }

    if (m_pRouteHandler->getIpv6PrefixLength(pReqInfo->pNetAgentCmdObj, &ipv6PrefixLength) != NETLINK_RET_SUCCESS) {
        NA_LOG_E("[%s] fail to get ipv6PrefixLength", __FUNCTION__);
        return;
    }

    if (m_pRouteHandler->getInterfaceId(pReqInfo->pNetAgentCmdObj, &interfaceId) != NETLINK_RET_SUCCESS) {
        NA_LOG_E("[%s] fail to get interfaceId", __FUNCTION__);
        return;
    }

    if (m_pRouteHandler->getAction(pReqInfo->pNetAgentCmdObj, &action) != NETLINK_RET_SUCCESS) {
        NA_LOG_E("[%s] fail to get action", __FUNCTION__);
        return;
    }

    if (isIpv6Global(address)) {
        if (m_pRouteHandler->hasLastReqInfoChanged(pReqInfo->pNetAgentCmdObj) == NETLINK_RET_REQ_INFO_NO_CHANGED
                && mIfChgForIPV6Count == 0) {
            NA_LOG_I("[%s] pReqInfo is not changed, un-answered ifchg is 0, no need to notify DDM", __FUNCTION__);
            return;
        }

        if (action == ACTION_ADDR_REMOVED) {
            if (isRemovedIPv6RequestByModem(interfaceId, address) == true){
                reomvePdnIpInfo(interfaceId, address);
                NA_LOG_D("[%s] Don't notify ip be removed is request from modem", __FUNCTION__);
                return;
            }

            if (isNeedNotifyIPv6RemovedToModem(interfaceId, address) == false) {
                if (m_pRouteHandler->setLastReqInfo(pReqInfo->pNetAgentCmdObj) != NETLINK_RET_SUCCESS) {
                    NA_LOG_E("[%s] fail to set last pReqInfo", __FUNCTION__);
                }
                NA_LOG_I("[%s] Don't notify ho source ip be removed to modem", __FUNCTION__);
                return;
            }

            if (isMultiHomingFeatureSupport) {
                // No need to clean up ipv6 address for modem as it will send ipdel message later.
            } else {
                memset(&address, 0, sizeof(address));
                strncpy(address, NULL_IPV6_ADDRESS, strlen(NULL_IPV6_ADDRESS));
                ipv6PrefixLength = INVALID_IPV6_PREFIX_LENGTH;
            }
        }

        if (convertIpv6ToBinary(addrV6_, address) < 0) {
            NA_LOG_E("[%s] fail to convert ipv6 address to binary", __FUNCTION__);
            return;
        }

        if (isMultiHomingFeatureSupport) {
            cmd = (action == ACTION_ADDR_UPDATED) ? NETAGENT_IO_CMD_IPADD : NETAGENT_IO_CMD_IPDEL;
            result = 99;
            confirmIpAddOrDel(interfaceId, cmd, result, NETAGENT_IO_ADDR_TYPE_IPv6, addrV6_, ipv6PrefixLength);
        } else {
            confirmIpUpdate(interfaceId, NETAGENT_IO_ADDR_TYPE_IPv6, addrV6_, ipv6PrefixLength);
        }

        if (m_pRouteHandler->setLastReqInfo(pReqInfo->pNetAgentCmdObj) != NETLINK_RET_SUCCESS) {
            NA_LOG_E("[%s] fail to set last pReqInfo", __FUNCTION__);
        }
    } else if (action == ACTION_ADDR_REMOVED && isNoRA(address, &flag)) {
        confirmNoRA(interfaceId, flag);
    }
}

bool NetAgentService::isNoRA(const char *ipv6Addr, NA_RA *flag) {
    if (ipv6Addr) {
        if (strncasecmp("FE80::5A:5A:5A:22", ipv6Addr, strlen("FE80::5A:5A:5A:22")) == 0) {
            *flag = NETAGENT_IO_NO_RA_REFRESH;
            return true;
        } else if (strncasecmp("FE80::5A:5A:5A:23", ipv6Addr, strlen("FE80::5A:5A:5A:23")) == 0) {
            *flag = NETAGENT_IO_NO_RA_INITIAL;
            return true;
        }
    } else {
        NA_LOG_E("[%s] input ipv6 address is null!!", __FUNCTION__);
    }
    return false;
}

void NetAgentService::handlePdnHandoverControl(NetAgentReqInfo* pReqInfo) {
    unsigned int tranId = INVALID_INTERFACE_ID;
    unsigned int interfaceId = INVALID_INTERFACE_ID;
    char addressV4[MAX_IPV4_ADDRESS_LENGTH] = {0};
    char addressV6[MAX_IPV6_ADDRESS_LENGTH] = {0};
    NA_PDN_HO_INFO hoInfo;

    if (NA_GET_IF_ID(pReqInfo->pNetAgentCmdObj, &tranId) != NETAGENT_IO_RET_SUCCESS) {
        NA_LOG_E("[%s] fail to get interface id", __FUNCTION__);
        return;
    }
    interfaceId = tranId % TRANSACTION_ID_OFFSET;

    if (NA_GET_PDN_HO_INFO(pReqInfo->pNetAgentCmdObj, &hoInfo) != NETAGENT_IO_RET_SUCCESS) {
        NA_LOG_E("[%s] fail to get handover info", __FUNCTION__);
        return;
    }

    NA_ADDR_TYPE addrType = hoInfo.addr_type;
    if (NETAGENT_IO_HO_STATE_START == hoInfo.hostate) {
        NA_LOG_D("[%s] tid: %d, hostate: %s, result: %s, src_ran: %s, tgt_ran: %s",
                __FUNCTION__, tranId, hoStateToString(hoInfo.hostate),
                hoResultToString(hoInfo.is_succ), ranTypeToString(hoInfo.src_ran),
                ranTypeToString(hoInfo.tgt_ran));
        if (NETAGENT_IO_HO_RESULT_SUCCESS == hoInfo.is_succ) {
            switch (addrType) {
                case NETAGENT_IO_ADDR_TYPE_IPv4:
                    getIpv4Address(pReqInfo->pNetAgentCmdObj, addressV4);
                    break;
                case NETAGENT_IO_ADDR_TYPE_IPv6:
                    getIpv6Address(pReqInfo->pNetAgentCmdObj, addressV6);
                    break;
                case NETAGENT_IO_ADDR_TYPE_IPv4v6:
                    getIpv4v6Address(pReqInfo->pNetAgentCmdObj, addressV4, addressV6);
                    break;
                default:
                    // No address
                    break;
            }
            recordPdnHandoverInfo(interfaceId, addrType, addressV4, addressV6);
        }

    } else if (NETAGENT_IO_HO_STATE_STOP == hoInfo.hostate) {
        bool needFlushIpsecPolicy =
                hoInfo.is_succ == NETAGENT_IO_HO_RESULT_SUCCESS &&
                hoInfo.src_ran == NETAGENT_IO_HO_RAN_WIFI &&
                hoInfo.tgt_ran == NETAGENT_IO_HO_RAN_MOBILE;

        NA_LOG_D("[%s] tid: %d, hostate: %s, result: %s, src_ran: %s, tgt_ran: %s, flush_ipsec: %d",
                __FUNCTION__, tranId, hoStateToString(hoInfo.hostate),
                hoResultToString(hoInfo.is_succ), ranTypeToString(hoInfo.src_ran),
                ranTypeToString(hoInfo.tgt_ran), needFlushIpsecPolicy);

        if (needFlushIpsecPolicy) {
            clearIpsec(interfaceId);
        }

        confirmPdnHandoverControl(tranId);
    }
}

NetAgentPdnInfo *NetAgentService::recordPdnHandoverInfo(
        unsigned int interfaceId, NA_ADDR_TYPE addrType, char *addressV4, char *addressV6) {

    NetAgentPdnInfo *pPdnSrcInfo = getPdnHandoverInfo(interfaceId);
    if (pPdnSrcInfo == NULL) {
        pPdnSrcInfo = (NetAgentPdnInfo *)calloc(1, sizeof(NetAgentPdnInfo));
        if (pPdnSrcInfo == NULL) {
            NA_LOG_E("[%s] can't allocate NetAgentPdnInfo", __FUNCTION__);
            return NULL;
        }
    }

    pPdnSrcInfo->interfaceId = interfaceId;
    pPdnSrcInfo->addrType = addrType;

    switch (addrType) {
        case NETAGENT_IO_ADDR_TYPE_IPv4:
            memcpy(pPdnSrcInfo->addressV4, addressV4, MAX_IPV4_ADDRESS_LENGTH);
            NA_LOG_D("[%s] interfaceId: %d, addrType: %s, addressV4: %s",
                    __FUNCTION__, interfaceId, addrTypeToString(addrType), addressV4);
            break;
        case NETAGENT_IO_ADDR_TYPE_IPv6:
            memcpy(pPdnSrcInfo->addressV6, addressV6, MAX_IPV6_ADDRESS_LENGTH);
            NA_LOG_D("[%s] interfaceId: %d, addrType: %s, addressV6: %s",
                    __FUNCTION__, interfaceId, addrTypeToString(addrType), addressV4);
            break;
        case NETAGENT_IO_ADDR_TYPE_IPv4v6:
            memcpy(pPdnSrcInfo->addressV4, addressV4, MAX_IPV4_ADDRESS_LENGTH);
            memcpy(pPdnSrcInfo->addressV6, addressV6, MAX_IPV6_ADDRESS_LENGTH);
            NA_LOG_D("[%s] interfaceId: %d, addrType: %s, addressV4: %s, addressV6: %s",
                    __FUNCTION__, interfaceId, addrTypeToString(addrType), addressV4, addressV6);
            break;
        default:
            // No address, shall not go to here.
            break;
    }
    m_pdnHoInfoMap[interfaceId] = pPdnSrcInfo;

    return pPdnSrcInfo;
}

NetAgentPdnInfo *NetAgentService::getPdnHandoverInfo(unsigned int interfaceId) {
    if (m_pdnHoInfoMap.count(interfaceId) > 0) {
        return m_pdnHoInfoMap[interfaceId];
    }
    return NULL;
}

bool NetAgentService::clearPdnHandoverInfo(unsigned int interfaceId) {
    if (m_pdnHoInfoMap.count(interfaceId) > 0) {
        NetAgentPdnInfo *pPdnSrcInfo = m_pdnHoInfoMap[interfaceId];
        m_pdnHoInfoMap.erase(interfaceId);
        if (pPdnSrcInfo != NULL) {
            free(pPdnSrcInfo);
        }
        return true;
    }

    return false;
}

void NetAgentService::clearIpsec(unsigned int interfaceId) {
    NetAgentPdnInfo *pPdnSrcInfo = getPdnHandoverInfo(interfaceId);
    if (pPdnSrcInfo != NULL) {
        switch (pPdnSrcInfo->addrType) {
            case NETAGENT_IO_ADDR_TYPE_IPv4:
                NA_FLUSH_IPSEC_POLICY(pPdnSrcInfo->addressV4, NETAGENT_IO_ADDR_TYPE_IPv4);
                break;
            case NETAGENT_IO_ADDR_TYPE_IPv6:
                NA_FLUSH_IPSEC_POLICY(pPdnSrcInfo->addressV6, NETAGENT_IO_ADDR_TYPE_IPv6);
                break;
            case NETAGENT_IO_ADDR_TYPE_IPv4v6:
                NA_FLUSH_IPSEC_POLICY(pPdnSrcInfo->addressV4, NETAGENT_IO_ADDR_TYPE_IPv4);
                NA_FLUSH_IPSEC_POLICY(pPdnSrcInfo->addressV6, NETAGENT_IO_ADDR_TYPE_IPv6);
                break;
            default:
                // No address
                break;
        }
    } else {
        NA_LOG_E("[%s] Can't find NetAgentPdnInfo for tid: %d", __FUNCTION__, interfaceId);
    }
}

void NetAgentService::updatePdnHandoverAddr(NetAgentReqInfo* pReqInfo) {
    struct ifreq ifr;
    unsigned int interfaceId = INVALID_INTERFACE_ID;
    NA_ADDR_TYPE addrType;
    char addressV4[MAX_IPV4_ADDRESS_LENGTH] = {0};
    char addressV6[MAX_IPV6_ADDRESS_LENGTH] = {0};
    char *reason = NULL;
    unsigned int addrV4_;
    unsigned int addrV6_[4];

    if (NA_GET_IF_ID(pReqInfo->pNetAgentCmdObj, &interfaceId) != NETAGENT_IO_RET_SUCCESS) {
        NA_LOG_E("[%s] fail to get interface id", __FUNCTION__);
        return;
    }
    interfaceId %= TRANSACTION_ID_OFFSET;

    if (NA_GET_ADDR_TYPE(pReqInfo->pNetAgentCmdObj, &addrType) != NETAGENT_IO_RET_SUCCESS) {
        NA_LOG_E("[%s] fail to get addr type", __FUNCTION__);
        return;
    }

    if (NA_GET_IP_CHANGE_REASON(pReqInfo->pNetAgentCmdObj, &reason) != NETAGENT_IO_RET_SUCCESS) {
        NA_LOG_E("[%s] fail to get IP change reason", __FUNCTION__);
        reason = NULL;
    }

    NA_LOG_I("[%s] update interface %d, addr type : %s(%d), reason: %s", __FUNCTION__,
                interfaceId, addrTypeToString(addrType), addrType, reason != NULL ? reason : "");

    memset(&ifr, 0, sizeof(struct ifreq));
    snprintf(ifr.ifr_name, IFNAMSIZ, "%s%d", getCcmniInterfaceName(), interfaceId);

    // add new interface address into kernel
    switch (addrType) {
        case NETAGENT_IO_ADDR_TYPE_IPv4:
            getIpv4Address(pReqInfo->pNetAgentCmdObj, addressV4);
            ifc_add_address(ifr.ifr_name, addressV4, IPV4_REFIX_LENGTH);
            NA_LOG_D("[%s] add addressV4: %s", __FUNCTION__, addressV4);
            if (NA_GET_ADDR_V4(pReqInfo->pNetAgentCmdObj, &addrV4_) != NETAGENT_IO_RET_SUCCESS) {
                NA_LOG_I("[%s] fail to get addrV4", __FUNCTION__);
            }
            break;
        case NETAGENT_IO_ADDR_TYPE_IPv6:
            getIpv6Address(pReqInfo->pNetAgentCmdObj, addressV6);
            ifc_add_address(ifr.ifr_name, addressV6, IPV6_REFIX_LENGTH);
            NA_LOG_D("[%s] add addressV6: %s", __FUNCTION__, addressV6);
            if (NA_GET_ADDR_V6(pReqInfo->pNetAgentCmdObj, addrV6_) != NETAGENT_IO_RET_SUCCESS) {
                NA_LOG_I("[%s] fail to get addrV4", __FUNCTION__);
            }
            break;
        case NETAGENT_IO_ADDR_TYPE_IPv4v6:
            getIpv4v6Address(pReqInfo->pNetAgentCmdObj, addressV4, addressV6);
            ifc_add_address(ifr.ifr_name, addressV4, IPV4_REFIX_LENGTH);
            ifc_add_address(ifr.ifr_name, addressV6, IPV6_REFIX_LENGTH);
            NA_LOG_D("[%s] add addressV4: %s, addressV6: %s", __FUNCTION__, addressV4, addressV6);
            if (NA_GET_ADDR_V4(pReqInfo->pNetAgentCmdObj, &addrV4_) != NETAGENT_IO_RET_SUCCESS) {
                NA_LOG_I("[%s] fail to get addrV4", __FUNCTION__);
            }
            if (NA_GET_ADDR_V6(pReqInfo->pNetAgentCmdObj, addrV6_) != NETAGENT_IO_RET_SUCCESS) {
                NA_LOG_I("[%s] fail to get addrV4", __FUNCTION__);
            }
            break;
        default:
            NA_LOG_E("[%s] get addr type fail", __FUNCTION__);
            break;
    }

    NetAgentPdnInfo *pPdnSrcInfo = getPdnHandoverInfo(interfaceId);
    if (pPdnSrcInfo == NULL) {
        NA_LOG_E("[%s] Can't find NetAgentPdnInfo for tid: %d", __FUNCTION__, interfaceId);
        return;
    }

    // del old interface address into kernel
    switch (pPdnSrcInfo->addrType) {
        case NETAGENT_IO_ADDR_TYPE_IPv4:
            ifc_del_address(ifr.ifr_name, pPdnSrcInfo->addressV4, IPV4_REFIX_LENGTH);
            NA_LOG_D("[%s] remove addressV4: %s", __FUNCTION__, pPdnSrcInfo->addressV4);
            break;
        case NETAGENT_IO_ADDR_TYPE_IPv6:
            ifc_del_address(ifr.ifr_name, pPdnSrcInfo->addressV6, IPV6_REFIX_LENGTH);
            NA_LOG_D("[%s] remove addressV6: %s", __FUNCTION__, pPdnSrcInfo->addressV6);
            break;
        case NETAGENT_IO_ADDR_TYPE_IPv4v6:
            if(addrType == NETAGENT_IO_ADDR_TYPE_IPv4) {
                ifc_del_address(ifr.ifr_name, pPdnSrcInfo->addressV4, IPV4_REFIX_LENGTH);
                NA_LOG_D("[%s] remove addressV4: %s because only addressV4 change",
                        __FUNCTION__, pPdnSrcInfo->addressV4);
            } else if(addrType == NETAGENT_IO_ADDR_TYPE_IPv6) {
                ifc_del_address(ifr.ifr_name, pPdnSrcInfo->addressV6, IPV6_REFIX_LENGTH);
                NA_LOG_D("[%s] remove addressV6: %s because only addressV6 change",
                        __FUNCTION__, pPdnSrcInfo->addressV6);
            } else {
                ifc_del_address(ifr.ifr_name, pPdnSrcInfo->addressV4, IPV4_REFIX_LENGTH);
                ifc_del_address(ifr.ifr_name, pPdnSrcInfo->addressV6, IPV6_REFIX_LENGTH);
                NA_LOG_D("[%s] remove addressV4: %s, addressV6: %s",
                        __FUNCTION__, pPdnSrcInfo->addressV4, pPdnSrcInfo->addressV6);
            }
            break;
        default:
            // No address, shall not go to here.
            break;
    }

    //send comfirm to modem
    switch (addrType) {
        case NETAGENT_IO_ADDR_TYPE_IPv4:
        case NETAGENT_IO_ADDR_TYPE_IPv4v6:
            confirmIpUpdate(interfaceId,
                    NETAGENT_IO_ADDR_TYPE_IPv4,
                    &addrV4_,
                    INVALID_IPV6_PREFIX_LENGTH);
            break;
        default:
            // No address, shall not go to here.
            break;
    }
}

/**
When IPv6 address is removed from kernel via netlink notification,
AP doesn't need to relay it to modem if delAddr is the same address
of interface before handover.

@param interfaceId for ccmni
@param delAddr IPv6 address be removed from kernel
*/
bool NetAgentService::isNeedNotifyIPv6RemovedToModem(unsigned int interfaceId, char* delAddr) {

    unsigned int addrV6_[4];
    NetAgentPdnInfo *pPdnSrcInfo = getPdnHandoverInfo(interfaceId);

    if (pPdnSrcInfo == NULL) {
        NA_LOG_E("[%s] can not found PdnSrcInfo", __FUNCTION__);
        return true;
    }

    switch (pPdnSrcInfo->addrType) {
        /**
        adjust the format of IP address to be consistent and comparable
        EX 2001:2001:0:0:1::11 -> 2001:0000:0000:0001:0000:0000:0000:0011
        */
        case NETAGENT_IO_ADDR_TYPE_IPv6:
        case NETAGENT_IO_ADDR_TYPE_IPv4v6:
            if (convertIpv6ToBinary(addrV6_, delAddr) < 0) {
                NA_LOG_E("[%s] fail to convert ipv6 address to binary", __FUNCTION__);
                return true;
             }

            if (convertIpv6ToString(delAddr, addrV6_) < 0) {
                NA_LOG_E("[%s] error occurs when converting ipv6 to string", __FUNCTION__);
                return true;
            }

            NA_LOG_I("[%s] compare PdnSrcInfo IPv6:%s, Netlink removed IPv6:%s", __FUNCTION__, pPdnSrcInfo->addressV6, delAddr);
            if (strncmp(delAddr, pPdnSrcInfo->addressV6, strlen(delAddr)) == 0) {
                return false;
            }
            break;
        default:
            // No address, shall not go to here.
            break;
    }
    return true;
}

void NetAgentService::confirmInterfaceState(unsigned int interfaceId, NA_IFST state, NA_ADDR_TYPE addrType) {
    void *pNetAgentCmdObj = 0;

    int nTransIntfId = getTransIntfId(interfaceId);
    if (INVALID_TRANS_INTF_ID != nTransIntfId) {
        pNetAgentCmdObj = NA_CMD_IFST_ALLOC(nTransIntfId, state, addrType);
        if (NA_CMD_SEND(m_pNetAgentIoObj, pNetAgentCmdObj) != NETAGENT_IO_RET_SUCCESS) {
            NA_LOG_E("[%s] send Ifst confirm fail", __FUNCTION__);
        }
        NA_CMD_FREE(pNetAgentCmdObj);
    } else {
        NA_LOG_I("[%s] ignore to send Ifst confirm", __FUNCTION__);
    }
}

void NetAgentService::confirmIpUpdate(unsigned int interfaceId, NA_ADDR_TYPE addrType, unsigned int* addr, int ipv6PrefixLength) {
    void *pNetAgentCmdObj = 0;

    int nTransIntfId = getTransIntfId(interfaceId);
    if (INVALID_TRANS_INTF_ID != nTransIntfId) {
        pNetAgentCmdObj = NA_CMD_IPUPDATE_ALLOC(nTransIntfId, addrType, addr, ipv6PrefixLength);
        if (NA_CMD_SEND(m_pNetAgentIoObj, pNetAgentCmdObj) != NETAGENT_IO_RET_SUCCESS) {
            NA_LOG_E("[%s] send IpUpdate confirm fail", __FUNCTION__);
        }
        if (mIfChgForIPV6Count > 0 && addrType == NETAGENT_IO_ADDR_TYPE_IPv6) {
            mIfChgForIPV6Count--;
        }
        NA_CMD_FREE(pNetAgentCmdObj);
    } else {
        NA_LOG_D("[%s] ignore to send ip update event", __FUNCTION__);
    }
}

void NetAgentService::confirmNoRA(unsigned int interfaceId, NA_RA flag) {
    void *pNetAgentCmdObj = 0;

    int nTransIntfId = getTransIntfId(interfaceId);
    if (INVALID_TRANS_INTF_ID != nTransIntfId) {
        pNetAgentCmdObj = NA_CMD_RA_ALLOC(nTransIntfId, flag);
        if (NA_CMD_SEND(m_pNetAgentIoObj, pNetAgentCmdObj) != NETAGENT_IO_RET_SUCCESS) {
            NA_LOG_E("[%s] send NoRA confirm fail", __FUNCTION__);
        }
        NA_CMD_FREE(pNetAgentCmdObj);
    } else {
        NA_LOG_I("[%s] ignore to send no RA event", __FUNCTION__);
    }
}

void NetAgentService::confirmPdnHandoverControl(unsigned int tranId) {
    NA_LOG_D("[%s] tranId %d", __FUNCTION__, tranId);

    void *pNetAgentCmdObj = NA_CMD_PDNHO_ALLOC(tranId);
    if (NA_CMD_SEND(m_pNetAgentIoObj, pNetAgentCmdObj) != NETAGENT_IO_RET_SUCCESS) {
        NA_LOG_E("[%s] send PDN handover confirm fail", __FUNCTION__);
    }
    NA_CMD_FREE(pNetAgentCmdObj);
}

void NetAgentService::confirmIpAddOrDel(unsigned int interfaceId, netagent_io_cmd_e cmd, int result, NA_ADDR_TYPE addrType, unsigned int* addr, int ipv6PrefixLength) {
    void *pNetAgentCmdObj = 0;

    int nTransIntfId = getTransIntfId(interfaceId);
    if (INVALID_TRANS_INTF_ID != nTransIntfId) {
        pNetAgentCmdObj = NA_CMD_IP_ADD_DEL_ALLOC(nTransIntfId, cmd, result, addrType, addr, ipv6PrefixLength);
        if (NA_CMD_SEND(m_pNetAgentIoObj, pNetAgentCmdObj) != NETAGENT_IO_RET_SUCCESS) {
           NA_LOG_E("[%s] send IpUpdate confirm fail", __FUNCTION__);
        }
        NA_CMD_FREE(pNetAgentCmdObj);
    } else {
        NA_LOG_E("[%s] can't get transaction id, ignore to send cinfirm, cmd:%d", __FUNCTION__, cmd);
    }
}

void NetAgentService::confirmTcpUdpRsvn(unsigned int transactionId, NA_CMD cmd,
        unsigned int rsvnAction, unsigned int response) {
    void *pNetAgentCmdObj = NA_CMD_RSVN_ALLOC(transactionId, cmd, rsvnAction, response);
    if (NA_CMD_SEND(m_pNetAgentIoObj, pNetAgentCmdObj) != NETAGENT_IO_RET_SUCCESS) {
        NA_LOG_E("[%s] send TcpUdpRsvn confirm fail", __FUNCTION__);
    }
    NA_CMD_FREE(pNetAgentCmdObj);
}

void NetAgentService::confirmSpi(unsigned int transactionId,
        unsigned int spiAction, unsigned int response) {
    void *pNetAgentCmdObj = NA_CMD_SPI_ALLOC(transactionId, spiAction, response);
    if (NA_CMD_SEND(m_pNetAgentIoObj, pNetAgentCmdObj) != NETAGENT_IO_RET_SUCCESS) {
        NA_LOG_E("[%s] send spi confirm fail", __FUNCTION__);
    }
    NA_CMD_FREE(pNetAgentCmdObj);
}

void NetAgentService::startNetlinkEventHandler(void) {
    if ((m_pRouteHandler = setupSocket(&mRouteSock, NETLINK_ROUTE,
                                      RTMGRP_IPV6_IFADDR | RTMGRP_IPV6_PREFIX,
                                      NetlinkListener::NETLINK_FORMAT_BINARY)) == NULL) {
        NA_LOG_E("[%s] setup socket fail", __FUNCTION__);
    }
}

NetlinkEventHandler *NetAgentService::setupSocket(int *sock, int netlinkFamily,
        int groups, int format) {

    struct sockaddr_nl nladdr;
    int sz = 64 * 1024;
    int on = 1;

    memset(&nladdr, 0, sizeof(nladdr));
    nladdr.nl_family = AF_NETLINK;
    nladdr.nl_pid = getpid();
    nladdr.nl_groups = groups;

    if ((*sock = socket(PF_NETLINK, SOCK_DGRAM | SOCK_CLOEXEC, netlinkFamily)) < 0) {
        NA_LOG_E("[%s] Unable to create netlink socket: %s", __FUNCTION__, strerror(errno));
        return NULL;
    }

    if (setsockopt(*sock, SOL_SOCKET, SO_RCVBUFFORCE, &sz, sizeof(sz)) < 0) {
        NA_LOG_E("[%s] Unable to set uevent socket SO_RCVBUFFORCE option: %s",
                __FUNCTION__, strerror(errno));
        close(*sock);
        return NULL;
    }

    if (setsockopt(*sock, SOL_SOCKET, SO_PASSCRED, &on, sizeof(on)) < 0) {
        NA_LOG_E("[%s] Unable to set uevent socket SO_PASSCRED option: %s",
                __FUNCTION__, strerror(errno));
        close(*sock);
        return NULL;
    }

    if (bind(*sock, (struct sockaddr *) &nladdr, sizeof(nladdr)) < 0) {
        NA_LOG_E("[%s] Unable to bind netlink socket: %s", __FUNCTION__, strerror(errno));
        close(*sock);
        return NULL;
    }

    NetlinkEventHandler *handler = new NetlinkEventHandler(this, *sock, format);
    if (handler == NULL) {
        NA_LOG_E("[%s] new NetlinkEventHandler fail", __FUNCTION__);
        close(*sock);
        return NULL;
    }

    if (handler->start() < 0) {
        NA_LOG_E("[%s] Unable to start NetlinkEventHandler: %s", __FUNCTION__, strerror(errno));
        delete handler;
        handler = NULL;
        close(*sock);
        return NULL;
    }

    return handler;
}

bool NetAgentService::isIpv6Global(const char *ipv6Addr) {
    if (ipv6Addr) {
        struct sockaddr_in6 sa;
        int ret = 0;

        if (strncasecmp("FE80", ipv6Addr, strlen("FE80")) == 0) {
            NA_LOG_I("[%s] not global", __FUNCTION__);
            return false;
        }

        // ret: -1, error occurs, ret: 0, invalid address, ret: 1, success;
        ret = inet_pton(AF_INET6, ipv6Addr, &(sa.sin6_addr));
        if (ret <= 0) {
            NA_LOG_E("[%s] ipv6 address: %s, inet_pton ret: %d", __FUNCTION__, ipv6Addr, ret);
            return false;
        }

        if (IN6_IS_ADDR_MULTICAST(&sa.sin6_addr)) {
            NA_LOG_I("[%s] multi-cast", __FUNCTION__);
            if (IN6_IS_ADDR_MC_GLOBAL(&sa.sin6_addr)) {
                NA_LOG_D("[%s] global", __FUNCTION__);
                return true;
            } else {
                NA_LOG_I("[%s] not global", __FUNCTION__);
            }
        } else {
            if (IN6_IS_ADDR_LINKLOCAL(&sa.sin6_addr)) {
                NA_LOG_I("[%s] link-local", __FUNCTION__);
            } else if (IN6_IS_ADDR_SITELOCAL(&sa.sin6_addr)) {
                NA_LOG_I("[%s] site-local", __FUNCTION__);
            } else if (IN6_IS_ADDR_V4MAPPED(&sa.sin6_addr)) {
                NA_LOG_I("[%s] v4mapped", __FUNCTION__);
            } else if (IN6_IS_ADDR_V4COMPAT(&sa.sin6_addr)) {
                NA_LOG_I("[%s] v4compat", __FUNCTION__);
            } else if (IN6_IS_ADDR_LOOPBACK(&sa.sin6_addr)) {
                NA_LOG_I("[%s] host", __FUNCTION__);
            } else if (IN6_IS_ADDR_UNSPECIFIED(&sa.sin6_addr)) {
                NA_LOG_I("[%s] unspecified", __FUNCTION__);
            } else if (_IN6_IS_ULA(&sa.sin6_addr)) {
                NA_LOG_D("[%s] uni-local", __FUNCTION__);
                return true;
            } else {
                NA_LOG_D("[%s] global", __FUNCTION__);
                return true;
            }
        }
    } else {
        NA_LOG_E("[%s] input ipv6 address is null!!", __FUNCTION__);
    }
    return false;
}

int NetAgentService::getCommand(void* obj, REQUEST_TYPE reqType, NA_CMD *cmd) {
    if (reqType == REQUEST_TYPE_DDM) {
        if (NA_CMD_TYPE(obj, cmd) != NETAGENT_IO_RET_SUCCESS) {
            NA_LOG_E("[%s] get %s command fail", __FUNCTION__, reqTypeToString(reqType));
            return -1;
        }
    } else if (reqType == REQUEST_TYPE_NETLINK) {
        if (m_pRouteHandler->getCommandType(obj, cmd) != NETLINK_RET_SUCCESS) {
            NA_LOG_E("[%s] get %s command fail", __FUNCTION__, reqTypeToString(reqType));
            return -1;
        }
    } else if (reqType == REQUEST_TYPE_NETAGENT) {
        NetEventReqInfo *pNetEventReqInfo = (NetEventReqInfo *)obj;
        *cmd = pNetEventReqInfo->cmd;
    } else {
        NA_LOG_E("[%s] request is %s(%d)", __FUNCTION__, reqTypeToString(reqType), reqType);
        return -1;
    }
    return 0;
}

void NetAgentService::getIpv4Address(void *obj, char *addressV4) {
    unsigned int addrV4_;
    if (NA_GET_ADDR_V4(obj, &addrV4_) == NETAGENT_IO_RET_SUCCESS) {
        if (addrV4_ != 0) {
            if (convertIpv4ToString(addressV4, &addrV4_) < 0) {
                NA_LOG_E("[%s] error occurs when converting ipv4 to string", __FUNCTION__);
            }
        } else {
            NA_LOG_I("[%s] IPv4 address lost after IRAT", __FUNCTION__);
        }
    } else {
        NA_LOG_E("[%s] error occurs when parsing addressV4", __FUNCTION__);
    }
}

void NetAgentService::getIpv6Address(void *obj, char *addressV6) {
    unsigned int addrV6_[4];
    if (NA_GET_ADDR_V6(obj, addrV6_) == NETAGENT_IO_RET_SUCCESS) {
        if (!(addrV6_[0] == 0 && addrV6_[1] == 0 && addrV6_[2] == 0 && addrV6_[3] == 0)) {
            if (convertIpv6ToString(addressV6, addrV6_) < 0) {
                NA_LOG_E("[%s] error occurs when converting ipv6 to string", __FUNCTION__);
            }
        } else {
            NA_LOG_I("[%s] IPv6 address lost after IRAT", __FUNCTION__);
        }
    } else {
        NA_LOG_E("[%s] error occurs when parsing addressV6", __FUNCTION__);
    }
}

void NetAgentService::getIpv4v6Address(void *obj, char *addressV4, char *addressV6) {
    getIpv4Address(obj, addressV4);
    getIpv6Address(obj, addressV6);
}

int NetAgentService::convertIpv6ToBinary(unsigned int *output, char *input) {
    int ret = 1;
    struct in6_addr v6Address;
    memset(&v6Address, 0, sizeof(v6Address));
    // ret: -1, error occurs, ret: 0, invalid address, ret: 1, success;
    ret = inet_pton(AF_INET6, input, &v6Address);
    if (ret >= 0) {
        memcpy(output, &v6Address, 16);
        return 0;
    }
    return -1;
}

int NetAgentService::convertIpv4ToString(char *output, unsigned int *input) {
    unsigned char *address = reinterpret_cast<unsigned char *>(input);
    if (output == NULL || address == NULL) {
        NA_LOG_E("[%s] null occurs on output = %s or addressV4 = %s", __FUNCTION__, output, address);
        return -1;
    }
    sprintf(output, "%d.%d.%d.%d", *address, *(address+1), *(address+2), *(address+3));
    return 0;
}

int NetAgentService::convertIpv6ToString(char *output, unsigned int *input) {
    unsigned char *address = reinterpret_cast<unsigned char *>(input);
    if (output == NULL || address == NULL) {
        NA_LOG_E("[%s] null occurs on output = %s or addressV6 = %s", __FUNCTION__, output, address);
        return -1;
    }
    sprintf(output, "%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X",
            *address, *(address+1), *(address+2), *(address+3),
            *(address+4), *(address+5), *(address+6), *(address+7),
            *(address+8), *(address+9), *(address+10), *(address+11),
            *(address+12), *(address+13), *(address+14), *(address+15));
    return 0;
}

void NetAgentService::freeNetAgentCmdObj(NetAgentReqInfo *pReqInfo) {
    if (pReqInfo->reqType == REQUEST_TYPE_DDM) {
        NA_CMD_FREE(pReqInfo->pNetAgentCmdObj);
    } else if (pReqInfo->reqType == REQUEST_TYPE_NETLINK) {
        m_pRouteHandler->freeNetlinkEventObj(pReqInfo->pNetAgentCmdObj);
    } else if (pReqInfo->reqType == REQUEST_TYPE_NETAGENT) {
        FREEIF(pReqInfo->pNetAgentCmdObj);
    }
}

void NetAgentService::setNetworkTransmitState(int state, int transIntfId,
        const sp<NetActionBase>& action) {
    if (isTransIntfIdMatched(transIntfId)) {
        NetEventReqInfo *pNetEventObj = (NetEventReqInfo *)calloc(1, sizeof(NetEventReqInfo));
        if (pNetEventObj == NULL) {
            NA_LOG_E("[%s] can't allocate rild event obj", __FUNCTION__);
            action->ack(false);
            return;
        }

        pNetEventObj->cmd = NETAGENT_IO_CMD_IFSTATE;
        pNetEventObj->action = action;
        pNetEventObj->parameter.snts.state = state;
        pNetEventObj->parameter.snts.interfaceId = transIntfId % TRANSACTION_ID_OFFSET;

        enqueueReqInfo(pNetEventObj, REQUEST_TYPE_NETAGENT);
    } else {
        action->ack(false);
    }
}

void NetAgentService::configureNetworkTransmitState(NetAgentReqInfo* pReqInfo) {
    NetEventReqInfo *pNetEventObj = (NetEventReqInfo *)pReqInfo->pNetAgentCmdObj;
    setNwTxqState(pNetEventObj->parameter.snts.interfaceId, pNetEventObj->parameter.snts.state);
    pNetEventObj->action->ack(true);
}

void NetAgentService::setNwTxqState(int interfaceId, int state) {
    struct ifreq ifr;
    int ret, ctl_sock;

    // Tclose timer(sec.) << 16 | 0 : stop uplink data transfer with Tclose timer
    // 1 : start uplink data transfer
    memset(&ifr, 0, sizeof(struct ifreq));
    snprintf(ifr.ifr_name, IFNAMSIZ, "%s", getNetworkInterfaceName(interfaceId));

    // The following is used to start/stop ccmni tx queue. @{
    ifr.ifr_name[IFNAMSIZ - 1] = 0;
    ifr.ifr_ifru.ifru_ivalue = state;

    ctl_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (ctl_sock < 0) {
        NA_LOG_E("[%s] fail to create ctl socket: errno=%d", __FUNCTION__, errno);
        return;
    }

    ret = ioctl(ctl_sock, SIOCSTXQSTATE, &ifr);
    if (ret < 0) {
        NA_LOG_E("[%s] error in set SIOCSTXQSTATE:%d - %d:%s",
                __FUNCTION__, ret, errno, strerror(errno));
    } else {
        NA_LOG_I("[%s] set state as %d, ret: %d", __FUNCTION__, state, ret);
    }

    close(ctl_sock);
    // @}
}

char* NetAgentService::getNetworkInterfaceName(int interfaceId) {
    char* ret = NULL;
    ret = ccci_get_node_name(static_cast<CCCI_USER>(USR_NET_0 + interfaceId), MD_SYS1);
    return ret;
}

bool NetAgentService::isTransIntfIdMatched(int transIntfId) {
    for (std::list<int>::iterator it = m_lTransIntfId.begin(); it != m_lTransIntfId.end(); ++it) {
        if (*it == transIntfId) {
            return true;
        }
    }
    NA_LOG_I("[%s] transIntfId %d is not matched", __FUNCTION__, transIntfId);
    return false;
}

int NetAgentService::getTransIntfId(int interfaceId) {
    if (interfaceId == INVALID_INTERFACE_ID) {
        NA_LOG_E("[%s] invalid interfaceId", __FUNCTION__);
        return INVALID_TRANS_INTF_ID;
    }

    for (std::list<int>::iterator it = m_lTransIntfId.begin(); it != m_lTransIntfId.end(); it++) {
        if (((*it) % TRANSACTION_ID_OFFSET) == interfaceId) {
            return *it;
        }
    }
    return INVALID_TRANS_INTF_ID;
}

const char *NetAgentService::cmdToString(NA_CMD cmd) {
    switch (cmd) {
        case NETAGENT_IO_CMD_IFST: return "IFST";
        case NETAGENT_IO_CMD_IPUPDATE: return "IPUPDATE";
        case NETAGENT_IO_CMD_IFUP: return "IFUP";
        case NETAGENT_IO_CMD_IFDOWN: return "IFDOWN";
        case NETAGENT_IO_CMD_IFCHG: return "IFCHG";
        case NETAGENT_IO_CMD_IFSTATE: return "IFSTATE";
        case NETAGENT_IO_CMD_SETMTU: return "SETMTU";
        case NETAGENT_IO_CMD_SYNC_CAPABILITY: return "SYNCCAP";
        case NETAGENT_IO_CMD_PDNHO: return "PDNHO";
        case NETAGENT_IO_CMD_IPCHG: return "IPCHG";
        default: return "UNKNOWN";
    }
}

const char *NetAgentService::addrTypeToString(NA_ADDR_TYPE addrType) {
    switch (addrType) {
        case NETAGENT_IO_ADDR_TYPE_IPv4: return "IPV4";
        case NETAGENT_IO_ADDR_TYPE_IPv6: return "IPV6";
        case NETAGENT_IO_ADDR_TYPE_IPv4v6: return "IPV4V6";
        default: return "UNKNOWN";
    }
}

const char *NetAgentService::reqTypeToString(REQUEST_TYPE reqType) {
    switch (reqType) {
        case REQUEST_TYPE_DDM: return "DDM";
        case REQUEST_TYPE_NETLINK: return "NETLINK";
        case REQUEST_TYPE_NETAGENT: return "NETAGENT";
        default: return "UNKNOWN";
    }
}

const char *NetAgentService::ranTypeToString(NA_RAN_TYPE ranType) {
    switch (ranType) {
        case NETAGENT_IO_HO_RAN_MOBILE: return "MOBILE";
        case NETAGENT_IO_HO_RAN_WIFI: return "WIFI";
        default: return "UNKNOWN";
    }
}

const char *NetAgentService::hoStateToString(int state) {
    switch (state) {
        case 0: return "START";
        case 1: return "STOP";
        default: return "UNKNOWN";
    }
}

const char *NetAgentService::hoResultToString(int result) {
    switch (result) {
        case 0: return "FAIL";
        case 1: return "SUCCESS";
        default: return "UNKNOWN";
    }
}

void NetAgentService::recordPdnIpInfo(
        unsigned int interfaceId, NA_ADDR_TYPE addrType, char *addressV4, char *addressV6) {

    NetAgentIpInfo *pPdnIpInfo;

    pPdnIpInfo = (NetAgentIpInfo *)calloc(1, sizeof(NetAgentIpInfo));
    if (pPdnIpInfo == NULL) {
        NA_LOG_E("[%s] can't allocate NetAgentIpInfo", __FUNCTION__);
        return;
    }

    pPdnIpInfo->interfaceId = interfaceId;
    pPdnIpInfo->addrType = addrType;
    switch (addrType) {
        // only save addressV6 now
        case NETAGENT_IO_ADDR_TYPE_IPv6:
            memcpy(pPdnIpInfo->addressV6, addressV6, MAX_IPV6_ADDRESS_LENGTH);
            NA_LOG_D("[%s] interfaceId: %d, addrType: %d, addressV6: %s",
                    __FUNCTION__, interfaceId, addrType, addressV6);
            break;
        default:
            // No address, shall not go to here.
            NA_LOG_E("[%s] No address, shall not go to here.", __FUNCTION__);
            free(pPdnIpInfo);
            return;
    }
    m_IpInfoList.push_back(*pPdnIpInfo);
    free(pPdnIpInfo);
}

bool NetAgentService::clearPdnIpInfo(unsigned int interfaceId) {
    NetAgentIpInfoList::iterator i = m_IpInfoList.begin();

    while (i != m_IpInfoList.end()) {
        NetAgentIpInfo ipInfo = *i;
        NA_LOG_D("[%s] interfaceId: %d, addressV6: %s",
                __FUNCTION__, ipInfo.interfaceId, ipInfo.addressV6);
        if (ipInfo.interfaceId == interfaceId) {
            m_IpInfoList.erase(i++);
        } else {
            ++i;
        }
    }
    return true;
}

bool NetAgentService::isRemovedIPv6RequestByModem(unsigned int interfaceId, char *delAddr) {
    NetAgentIpInfoList::iterator i;
    unsigned int addrV6_[4];

    for (i = m_IpInfoList.begin (); i != m_IpInfoList.end (); i++) {
        NetAgentIpInfo ipInfo = *i;
        if (ipInfo.interfaceId == interfaceId) {
            // compare PdnIpInfo IPv6 and Netlink removed IPv6:
            if (convertIpv6ToBinary(addrV6_, delAddr) < 0) {
                NA_LOG_E("[%s] fail to convert ipv6 address to binary", __FUNCTION__);
                return false;
            }

            if (convertIpv6ToString(delAddr, addrV6_) < 0) {
                NA_LOG_E("[%s] error occurs when converting ipv6 to string", __FUNCTION__);
                return false;
            }

            NA_LOG_I("[%s] compare NetAgentIpInfo IPv6:%s, Netlink removed IPv6:%s",
                    __FUNCTION__, ipInfo.addressV6, delAddr);
            if (strncmp(delAddr, ipInfo.addressV6, strlen(delAddr)) == 0) {
                return true;
            }
        }
    }
    return false;
}

bool NetAgentService::reomvePdnIpInfo(unsigned int interfaceId, char *addressV6){
    NetAgentIpInfoList::iterator i;
    unsigned int addrV6_[4];

    for (i = m_IpInfoList.begin (); i != m_IpInfoList.end (); i++) {
        NetAgentIpInfo ipInfo = *i;
        if (ipInfo.interfaceId == interfaceId) {
            // compare PdnIpInfo IPv6 and Netlink removed IPv6:
            if (convertIpv6ToBinary(addrV6_, addressV6) < 0) {
                NA_LOG_E("[%s] fail to convert ipv6 address to binary", __FUNCTION__);
                return false;
            }

            if (convertIpv6ToString(addressV6, addrV6_) < 0) {
                NA_LOG_E("[%s] error occurs when converting ipv6 to string", __FUNCTION__);
                return false;
            }

            NA_LOG_D("[%s] remove NetAgentIpInfo interfaceId: %d, addressV6: %s",
                    __FUNCTION__, ipInfo.interfaceId, ipInfo.addressV6);
            if (strncmp(addressV6, ipInfo.addressV6, strlen(addressV6)) == 0) {
                m_IpInfoList.erase(i++);
                return true;
            }
        }
    }
    return false;
}

void NetAgentService::setMultiHomingFeatureSupport(int supportVersion) {
    if (supportVersion >= 1) {
        isMultiHomingFeatureSupport = true;
    }
}

// Test mode start.
void NetAgentService::setTransactionInterfaceId(int transIntfId) {
    NA_LOG_D("[%s] transIntfId = %d", __FUNCTION__, transIntfId);
    if (transIntfId == INVALID_TRANS_INTF_ID) {
        return;
    }
    m_lTransIntfId.push_back(transIntfId);
}

void NetAgentService::removeTransactionInterfaceId(int transIntfId) {
    NA_LOG_D("[%s] transIntfId = %d", __FUNCTION__, transIntfId);
    if (transIntfId == INVALID_TRANS_INTF_ID) {
        return;
    }
    m_lTransIntfId.remove(transIntfId);
}

void NetAgentService::removeAllTransactionInterfaceId() {
    NA_LOG_D("[%s] X", __FUNCTION__);
    m_lTransIntfId.clear();
}
// Test mode end.
