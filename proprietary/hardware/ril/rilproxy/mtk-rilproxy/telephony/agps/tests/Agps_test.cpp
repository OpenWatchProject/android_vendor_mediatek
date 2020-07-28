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

#define LOG_TAG "Vector_test"


#include <cutils/log.h>
#include <gtest/gtest.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <poll.h>
#include "RfxMainThread.h"
#include "RfxObject.h"
#include "RfxRootController.h"
#include "RfxMessage.h"
#include "agps/RpAgpsSlotController.h"
#include "agps/RpAgpsNSlotController.h"
#include "agps/RpAgpsLog.h"
#include "agps/RpAgpsdAdapter.h"
#include "modecontroller/RpCdmaLteModeController.h"

#define MOCK_C2K_RILD   "/data/agps_supl/mock_c2k_rild"
#define MOCK_AGPSD      "c2kagpsdTest"
#define RILP_AGPS       "c2kagpsd2Test"

#define POLL_CHECK(fd, timeout) \
    { \
        int _ret = poll_wait(fd, timeout);\
        if(_ret == -1) {\
            AGPS_LOGE("%s line=%d poll_wait error reason=[%s]\n",\
                __FUNCTION__, __LINE__, strerror(errno));\
            return -1;\
        } else if(_ret == 0) {\
            AGPS_LOGE("%s line=%d  poll_wait timeout\n", __FUNCTION__, __LINE__);\
            return -1;\
        } \
    }
int c2kfd = -1;
int c2k_client_fd = -1;
int agpsd_fd = -1;

static int poll_wait(int fd, int timeout) {
    struct pollfd poll_fd[1];
    int ret;

    memset(poll_fd, 0, sizeof(poll_fd));
    poll_fd[0].fd      = fd;
    poll_fd[0].events  = POLLIN;
    poll_fd[0].revents = 0;

    ret = poll(poll_fd, 1, timeout);
    if(ret > 0) {
        if(poll_fd[0].revents & POLLIN ||
            poll_fd[0].revents & POLLERR) {
            ret = 1 << 0;
        }
    }
    return ret;
}

static int bindUdpSocket(const char* path) {
    int sockfd;
    struct sockaddr_un soc_addr;

    sockfd = socket(PF_LOCAL, SOCK_DGRAM, 0);
    if(sockfd < 0) {
        AGPS_LOGE("socket failed reason=[%s]\n", strerror(errno));
        return -1;
    }

    strcpy(soc_addr.sun_path, path);
    soc_addr.sun_family = AF_UNIX;

    unlink(soc_addr.sun_path);
    int ret = ::bind(sockfd, (struct sockaddr *)&soc_addr, sizeof(soc_addr));
    if( ret < 0) {
        AGPS_LOGE("bind failed path=[%s] reason=[%s]\n", path, strerror(errno));
        return -1;
    }

    chmod(path, 0660);

    return sockfd;
}

static int sendMsgToRilp(const char *name, int event, int data) {
    struct sockaddr_un addr;
    int socketfd;
    size_t namelen;
    int addr_len;
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    addr.sun_path[0] = 0;
    namelen = strlen(name);
    if (namelen + 1 > sizeof(addr.sun_path))
    {
        AGPS_LOGE("name error");
        return -1;
    }
    memcpy(&addr.sun_path[1], name, namelen);
    socketfd = socket(AF_UNIX, SOCK_DGRAM, 0);

    if (socketfd < 0) {
        AGPS_LOGE("create socket fail");
        return -1;
    }
    addr_len = namelen + offsetof(struct sockaddr_un, sun_path) + 1;

    if(connect(socketfd, (struct sockaddr *) &addr, addr_len) < 0) {
        AGPS_LOGE("Connect Error[%s]",
            strerror(errno));
        return -1;
    }
    char buf[128];
    int offset = 0;
    RpAgpsdAdapter::putInt32(buf, &offset, event);
    RpAgpsdAdapter::putInt32(buf, &offset, data);
    int ret = RpAgpsdAdapter::safeWrite(socketfd, buf, offset);
    return ret;
}

static int createAgpsdTcpServer(const char *name) {
    struct sockaddr_un addr;
    int socketfd;
    size_t namelen;
    int addr_len;
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    addr.sun_path[0] = 0;
    namelen = strlen(name);
    if (namelen + 1 > sizeof(addr.sun_path))
    {
        AGPS_LOGE("name error");
        return -1;
    }
    memcpy(&addr.sun_path[1], name, namelen);
    socketfd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (socketfd < 0) {
        AGPS_LOGE("create socket fail");
        return -1;
    }
    addr_len = namelen + offsetof(struct sockaddr_un, sun_path) + 1;
    if (::bind(socketfd, (struct sockaddr *)&addr, addr_len) < 0)
    {
        AGPS_LOGE("bind socket fail");
        return -1;
    }
    if (listen(socketfd, 5) < 0)
    {
        AGPS_LOGE("listen fail");
        return -1;
    }
    return socketfd;
}


static int connectUdpSocket(const char* path) {
    int sockfd = socket(PF_LOCAL, SOCK_DGRAM, 0);
    if(sockfd < 0) {
        AGPS_LOGE("socket failed reason=[%s]\n", strerror(errno));
        return -1;
    }
    struct sockaddr_un soc_addr;
    strcpy(soc_addr.sun_path, path);
    soc_addr.sun_family = AF_UNIX;
    if (connect(sockfd, (struct sockaddr *)&soc_addr,sizeof(soc_addr)) < 0) {
        AGPS_LOGE("connect failed reason=[%s]\n", strerror(errno));
        close(sockfd);
        return -1;
    }

    return sockfd;
}



int getNewConnect(int sockfd) {
    int newsockfd;
    struct sockaddr_un cli_addr;
    socklen_t clilen;

    clilen = sizeof(cli_addr);
    POLL_CHECK(sockfd, 1000);
    newsockfd = accept(sockfd,
                (struct sockaddr *) &cli_addr,
                &clilen);
    if (newsockfd < 0) {
       AGPS_LOGE("accept failed [%s]\n", strerror(errno));
       return -1;
    }

    return newsockfd;
}


int getNewConnectWithOutPoll(int sockfd) {
    int newsockfd;
    struct sockaddr_un cli_addr;
    socklen_t clilen;

    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd,
                (struct sockaddr *) &cli_addr,
                &clilen);
    if (newsockfd < 0) {
       AGPS_LOGE("accept failed [%s]\n", strerror(errno));
       return -1;
    }

    return newsockfd;
}

static int32_t agpsdRead(int clientFd, int32_t *data) {
     char buf[128];
     int offset = 0;
     int ret;
     ret = RpAgpsdAdapter::safeRead(clientFd, buf, sizeof(buf));
     if (ret > 0) {
         int32_t event= RpAgpsdAdapter::getInt32(buf, &offset);
         *data = RpAgpsdAdapter::getInt32(buf, &offset);
         AGPS_LOGD("agpsdRead event = %d, data = %d\n", event, *data);
         return event;
     } else {
         return -1;
     }
}

static int c2kRildRead(char *buf) {
    char buff[2048];
    int ret = 0;
    uint8_t dataLength[4];

    POLL_CHECK(c2kfd, 1000);
    ret = RpAgpsdAdapter::safeRead(c2kfd, (void*)buff, sizeof(buff));
    if(ret < 0) {
        AGPS_LOGE("safe_read failed\n");
        return -1;
    }
    memcpy(&dataLength, buff, sizeof(dataLength));
    int size =  ((dataLength[2] << 8) & 0xFF00) | dataLength[3];
    AGPS_LOGD("c2kRildRead (size = %d)", size);
    ret = RpAgpsdAdapter::safeRead(c2kfd, (void*)buff, sizeof(buff));
    if(ret < 0) {
        AGPS_LOGE("safe_read failed\n");
        return -1;
    }
    AGPS_LOGD("c2kRildRead (readSize = %d)", ret);
    memcpy(buf, buff, size);
    return size;
}

static RpAgpsNSlotController* getNSlotController() {
    RfxRootController *root = RFX_OBJ_GET_INSTANCE(RfxRootController);
    RpAgpsNSlotController *nsCtrl =
            (RpAgpsNSlotController *)root->findController(-1,
                    RFX_OBJ_CLASS_INFO(RpAgpsNSlotController));
    return nsCtrl;
}

static RpAgpsSlotController *getSlotController(int i) {
    RfxRootController *root = RFX_OBJ_GET_INSTANCE(RfxRootController);
    RpAgpsSlotController* sCtrl =
        (RpAgpsSlotController *)root->findController(i,
                RFX_OBJ_CLASS_INFO(RpAgpsSlotController));
    return sCtrl;
}




namespace android {

class AgpsTestEnv : public testing::Environment {
public:
    virtual void SetUp() {
        AGPS_LOGD("Global SetUp");
        agpsd_fd = createAgpsdTcpServer(MOCK_AGPSD);
        // Create Mock C2K RILD
        c2kfd = bindUdpSocket(MOCK_C2K_RILD);
        // Launch Rfx Main Thread
        RfxMainThread::init();
        // Wait Looper util it is ready
        RfxMainThread::waitLooper();
        // Create the socket that is used to send data to Mock C2K RILD
        c2k_client_fd = connectUdpSocket(MOCK_C2K_RILD);
        // Set the Client Socket to RilAdapter
        RFX_OBJ_GET_INSTANCE(RfxRilAdapter)->setSocket(0, RADIO_TECH_GROUP_C2K, c2k_client_fd);
        // RfxMainThread will sleep 20s before init controller
        sleep(1);
    }
    virtual void TearDown() {
        AGPS_LOGD("Global TearDown");
        RFX_OBJ_GET_INSTANCE(RfxRilAdapter)->closeSocket(0, RADIO_TECH_GROUP_C2K);
        close(c2k_client_fd);
        close(c2kfd);
        close(agpsd_fd);
    }
};

static ::testing::Environment * const env = ::testing::AddGlobalTestEnvironment(new AgpsTestEnv());


class AgpsTest : public testing::Test {
protected:
    virtual void SetUp() {}
    virtual void TearDown() {}
public:
};

TEST_F(AgpsTest, AgpsControllerCreateCheck) {
    // Check Non Slot Controller is ready
    ASSERT_TRUE(getNSlotController() != NULL);

    // Check Slot Controller is ready
    for (int i = 0; i < RFX_SLOT_COUNT; i++) {
        ASSERT_TRUE(getSlotController(i) != NULL);
    }
}

// SEE ALSO: VIA_GPS_Event in via-ril.h
typedef enum {
    REQUEST_DATA_CONNECTION = 0,
    CLOSE_DATA_CONNECTION,
    GPS_START,
    GPS_FIX_RESULT,
    GPS_STOP,
} GpsEventEnum;

// SEE ALSO: VIA_GPS_Event_data in via-ril.h
typedef struct {
    GpsEventEnum event;
    int gpsStatus;
} GpsEventStruct;


// Dummy message that is used to trigger postEmit in main thread
#define RIL_UNOSL_DUMMY_MSG      0xF250F250

#define DIRTY_DATA   0x250

// SEE ALSO: ril_oem.c  rilOemUnsolicited "^GPSTCPCONNREQ"
void requestDataConnection() {
    Parcel *p = new Parcel();
    p->writeInt32(RESPONSE_UNSOLICITED);       // Message Type
    p->writeInt32(RIL_LOCAL_C2K_UNSOL_VIA_GPS_EVENT);   // urc ID, convert from 1052 to 5004 in rild
    p->writeInt32(2);                          // int data number
    p->writeInt32(REQUEST_DATA_CONNECTION);    // gps event type, request data connection
    p->writeInt32(0);                          // always 0
    sp<RfxMessage> msg = RfxMessage::obtainUrc(
        RFX_SLOT_ID_0,
        RIL_LOCAL_C2K_UNSOL_VIA_GPS_EVENT,
        RADIO_TECH_GROUP_C2K,
        p);
    RfxMainThread::enqueueMessage(msg);
}

void requestCloseConnection() {
    Parcel *p = new Parcel();
    p->writeInt32(RESPONSE_UNSOLICITED);       // Message Type
    p->writeInt32(RIL_LOCAL_C2K_UNSOL_VIA_GPS_EVENT);   // urc ID, convert from 1052 to 5004 in rild
    p->writeInt32(2);                          // int data number
    p->writeInt32(CLOSE_DATA_CONNECTION);      // gps event type, close data connection
    p->writeInt32(0);                          // always 0
    sp<RfxMessage> msg = RfxMessage::obtainUrc(
        RFX_SLOT_ID_0,
        RIL_LOCAL_C2K_UNSOL_VIA_GPS_EVENT,
        RADIO_TECH_GROUP_C2K,
        p);
    RfxMainThread::enqueueMessage(msg);
}

void sendDummyMessage() {
    sp<RfxMessage> msg = RfxMessage::obtainUrc(
        RFX_SLOT_ID_0,
        RIL_UNOSL_DUMMY_MSG
        );
    RfxMainThread::enqueueMessage(msg);
}

void setDataConnectionState(int slot, int state) {
    sendDummyMessage();
    RpAgpsSlotController * ctrl = getSlotController(slot);
    ctrl->getStatusManager()->setIntValue(
        RFX_STATUS_KEY_DATA_CONNECTION,
        state);
}

void setServiceState(int slot, int state, int dataRadioTech) {
    int voiceRegState;
    int dataRegState = 1;
    int voiceRadioTech = RADIO_TECH_1xRTT;
    int cdmaNetworkExist = 1;
    if (state == RpAgpsNSlotController::IN_SERVICE) {
        voiceRegState = 1;
    } else {
        voiceRegState = 4;
    }
    sendDummyMessage();
    RpAgpsSlotController * ctrl = getSlotController(slot);
    ctrl->getStatusManager()->setServiceStateValue(
        RFX_STATUS_KEY_SERVICE_STATE,
        RfxNwServiceState(voiceRegState, dataRegState, voiceRadioTech,
        dataRadioTech, cdmaNetworkExist));
}

void setCardType(int slot, int type) {
    sendDummyMessage();
    RpAgpsSlotController * ctrl = getSlotController(slot);

    ctrl->getStatusManager()->setIntValue(
        RFX_STATUS_KEY_CARD_TYPE,
        type);
}

void setCDMASlot(int slot)
{
    sendDummyMessage();
    RpAgpsSlotController * ctrl = getSlotController(slot);
    ctrl->getNonSlotScopeStatusManager()->setIntValue(
        RFX_STATUS_KEY_CDMA_SOCKET_SLOT,
        slot);
}

void setOp09(int slot, bool op09) {
    RpAgpsSlotController * ctrl = getSlotController(slot);
    ctrl->setTestMode(true);
    ctrl->setOp09(op09);
}

#define NO_DATA         0
#define DATA_ON_SLOT0   1
#define DATA_ON_SLOT1   2


void setDefaultDataSlot(int slot) {
    RpAgpsNSlotController *ctrl = getNSlotController();
    ctrl->setTestMode(true);
    ctrl->setDefaultDataSlot(slot);
}

void getC2kRildRecvData(Parcel *p) {
    char buffer[1024];
    int size = c2kRildRead(buffer);
    p->setData((uint8_t *)buffer, size);
}


void checkAgpsTcpConnInd(Parcel *p, int eConnected) {
    status_t status;
    int32_t request = DIRTY_DATA;
    int32_t token   = DIRTY_DATA;
    int32_t count   = DIRTY_DATA;
    int connected   = DIRTY_DATA;
    status = p->readInt32(&request);
    status = p->readInt32 (&token);
    status = p->readInt32 (&count);
    status = p->readInt32 (&connected);
    ASSERT_TRUE(request == RIL_LOCAL_C2K_REQUEST_AGPS_TCP_CONNIND);
    ASSERT_TRUE(count == 1);
    ASSERT_TRUE(connected == eConnected);
}

TEST_F(AgpsTest, RequestDataConnection_NoDataConnectionTest) {
    setCDMASlot(RFX_SLOT_ID_1);
    setCDMASlot(RFX_SLOT_ID_0);
    RpCdmaLteModeController::setCdmaSocketSlotId(RFX_SLOT_ID_0);
    setDataConnectionState(RFX_SLOT_ID_0, DATA_STATE_CONNECTED);
    setDataConnectionState(RFX_SLOT_ID_0, DATA_STATE_DISCONNECTED);
    requestDataConnection();
    Parcel q;
    getC2kRildRecvData(&q);
    checkAgpsTcpConnInd(&q, 0);
}


TEST_F(AgpsTest, RequestDataConnection_DataConnectedNonOP09Load) {
    setDataConnectionState(RFX_SLOT_ID_0, DATA_STATE_CONNECTED);
    setOp09(RFX_SLOT_ID_0, false);
    requestDataConnection();
    Parcel q;
    getC2kRildRecvData(&q);
    checkAgpsTcpConnInd(&q, 1);
}


TEST_F(AgpsTest, RequestDataConnection_DataConnectedOP09LoadAndSetupAPNSucceedAndCloseAPN) {
    RFX_OBJ_GET_INSTANCE(RpAgpsdAdapter)->setTestMode(true);
    setDataConnectionState(RFX_SLOT_ID_0, DATA_STATE_CONNECTED);
    setOp09(RFX_SLOT_ID_0, true);
    setCardType(RFX_SLOT_ID_0, RFX_CARD_TYPE_RUIM);
    requestDataConnection();
    int clientFd = getNewConnect(agpsd_fd);
    ASSERT_TRUE(clientFd > 0);
    int event   = DIRTY_DATA;
    int apnType = DIRTY_DATA;
    event = agpsdRead(clientFd, &apnType);
    ASSERT_TRUE(event == EVENT_AGPS_SET_APN);
    ASSERT_TRUE(apnType == APN_TYPE_CTWAP);
    sendMsgToRilp(RILP_AGPS, EVENT_SET_APN_RESULT, 1);
    Parcel q;
    getC2kRildRecvData(&q);
    checkAgpsTcpConnInd(&q, 1);
    requestCloseConnection();
    clientFd = getNewConnect(agpsd_fd);
    ASSERT_TRUE(clientFd > 0);
    event   = DIRTY_DATA;
    apnType = DIRTY_DATA;
    event = agpsdRead(clientFd, &apnType);
    ASSERT_TRUE(event == EVENT_AGPS_DESTROY_APN);
    ASSERT_TRUE(apnType == APN_TYPE_CTWAP);
}


TEST_F(AgpsTest, RequestDataConnection_DataConnectedOP09LoadAndSetupAPNFailAndCloseAPN) {
    RFX_OBJ_GET_INSTANCE(RpAgpsdAdapter)->setTestMode(true);
    setDataConnectionState(RFX_SLOT_ID_0, DATA_STATE_CONNECTED);
    setOp09(RFX_SLOT_ID_0, true);
    requestDataConnection();
    int clientFd = getNewConnect(agpsd_fd);
    ASSERT_TRUE(clientFd > 0);
    int event   = DIRTY_DATA;
    int apnType = DIRTY_DATA;
    event = agpsdRead(clientFd, &apnType);
    ASSERT_TRUE(event == EVENT_AGPS_SET_APN);
    ASSERT_TRUE(apnType == APN_TYPE_CTWAP);
    sendMsgToRilp(RILP_AGPS, EVENT_SET_APN_RESULT, 0);
    Parcel q;
    getC2kRildRecvData(&q);
    checkAgpsTcpConnInd(&q, 0);

    requestCloseConnection();
    clientFd = getNewConnect(agpsd_fd);
    ASSERT_TRUE(clientFd > 0);
    event   = DIRTY_DATA;
    apnType = DIRTY_DATA;
    event = agpsdRead(clientFd, &apnType);
    ASSERT_TRUE(event == EVENT_AGPS_DESTROY_APN);
    ASSERT_TRUE(apnType == APN_TYPE_CTWAP);
}




TEST_F(AgpsTest, ServiceStateChanges) {
    setCDMASlot(RFX_SLOT_ID_1);
    setCDMASlot(RFX_SLOT_ID_0);
    setDefaultDataSlot(DATA_ON_SLOT0);
    setServiceState(RFX_SLOT_ID_0,
        RpAgpsNSlotController::IN_SERVICE,
        RADIO_TECH_1xRTT);

    int clientFd = getNewConnect(agpsd_fd);
    ASSERT_TRUE(clientFd > 0);
    int event   = DIRTY_DATA;
    int type = DIRTY_DATA;
    event = agpsdRead(clientFd, &type);
    ASSERT_TRUE(event == EVENT_AGPS_NETWORK_TYPE);
    ASSERT_EQ(type, RpAgpsNSlotController::NETWORK_TYPE_1xRTT);

    clientFd = getNewConnect(agpsd_fd);
    ASSERT_TRUE(clientFd > 0);
    event   = DIRTY_DATA;
    int state = DIRTY_DATA;
    event = agpsdRead(clientFd, &state);
    ASSERT_TRUE(event == EVENT_AGPS_CDMA_PHONE_STATUS);
    ASSERT_TRUE(state == RpAgpsNSlotController::IN_SERVICE);

    clientFd = getNewConnect(agpsd_fd);
    ASSERT_TRUE(clientFd > 0);
    event   = DIRTY_DATA;
    state = DIRTY_DATA;
    event = agpsdRead(clientFd, &state);
    ASSERT_TRUE(event == EVENT_AGPS_MOBILE_DATA_STATUS);
    ASSERT_TRUE(state == RpAgpsNSlotController::DATA_ENABLED_ON_CDMA_LTE);

    /*
     * LTEA is MTK defined ps rat type
     * We need to treat it as LTE
     * See CR ALPS02459797
     */

    setServiceState(RFX_SLOT_ID_0,
        RpAgpsNSlotController::IN_SERVICE,
        RpAgpsNSlotController::RIL_RADIO_TECHNOLOGY_LTEA);

    clientFd = getNewConnect(agpsd_fd);
    ASSERT_TRUE(clientFd > 0);
    event   = DIRTY_DATA;
    type = DIRTY_DATA;
    event = agpsdRead(clientFd, &type);
    ASSERT_TRUE(event == EVENT_AGPS_NETWORK_TYPE);
    ASSERT_EQ(type, RpAgpsNSlotController::NETWORK_TYPE_LTE);


    /* If no C card is inserted before, RFX_STATUS_KEY_CDMA_SOCKET_SLOT will 0
     * we need to add card type condition in getCDMASlot
     * see CR ALPS02428668
     */
    setCardType(RFX_SLOT_ID_0, RFX_CARD_TYPE_USIM);
    RpCdmaLteModeController::setCdmaSocketSlotId(RFX_SLOT_ID_0);
    setCDMASlot(RFX_SLOT_ID_0);

    setServiceState(RFX_SLOT_ID_0,
        RpAgpsNSlotController::IN_SERVICE,
        RADIO_TECH_UMTS);

    clientFd = getNewConnect(agpsd_fd);
    ASSERT_TRUE(clientFd > 0);
    event   = DIRTY_DATA;
    type = DIRTY_DATA;
    event = agpsdRead(clientFd, &type);
    ASSERT_TRUE(event == EVENT_AGPS_NETWORK_TYPE);
    ASSERT_EQ(type, RpAgpsNSlotController::NETWORK_TYPE_UMTS);

    clientFd = getNewConnect(agpsd_fd);
    ASSERT_TRUE(clientFd > 0);
    event   = DIRTY_DATA;
    state = DIRTY_DATA;
    event = agpsdRead(clientFd, &state);
    ASSERT_TRUE(event == EVENT_AGPS_CDMA_PHONE_STATUS);
    ASSERT_TRUE(state == RpAgpsNSlotController::OUT_OF_SERVICE);

    clientFd = getNewConnect(agpsd_fd);
    ASSERT_TRUE(clientFd > 0);
    event   = DIRTY_DATA;
    state = DIRTY_DATA;
    event = agpsdRead(clientFd, &state);
    ASSERT_TRUE(event == EVENT_AGPS_MOBILE_DATA_STATUS);
    ASSERT_TRUE(state == RpAgpsNSlotController::DATA_ENABLED_ON_GSM);

    /*
     * MTK defined some ps rat type for HSPAP
     * We need to treat them as HSPAP
     * Added when CR ALPS02459797
     */

    setServiceState(RFX_SLOT_ID_0,
        RpAgpsNSlotController::IN_SERVICE,
        RpAgpsNSlotController::RIL_RADIO_TECHNOLOGY_DC_DPA);

    clientFd = getNewConnect(agpsd_fd);
    ASSERT_TRUE(clientFd > 0);
    event   = DIRTY_DATA;
    type = DIRTY_DATA;
    event = agpsdRead(clientFd, &type);
    ASSERT_TRUE(event == EVENT_AGPS_NETWORK_TYPE);
    ASSERT_EQ(type, RpAgpsNSlotController::NETWORK_TYPE_HSPAP);
}


TEST_F(AgpsTest, PeerSocketCloseBeforeWhenWrite) {
    setCDMASlot(RFX_SLOT_ID_1);
    setDefaultDataSlot(DATA_ON_SLOT1);
    setCardType(RFX_SLOT_ID_1, RFX_CARD_TYPE_CSIM);
    setServiceState(RFX_SLOT_ID_1,
        RpAgpsNSlotController::IN_SERVICE,
        RADIO_TECH_LTE);
    int clientFd = getNewConnectWithOutPoll(agpsd_fd);
    int clientFd2 = getNewConnectWithOutPoll(agpsd_fd);
    close(clientFd2);
    close(clientFd);
}


TEST_F(AgpsTest, AgpsControllerDestroyCheck) {
    sleep(1);
    // Close the controllers
    RpAgpsNSlotController *ctrl = getNSlotController();
    RFX_OBJ_CLOSE(ctrl);
    for (int i = 0; i < RFX_SLOT_COUNT; i++) {
        RpAgpsSlotController *ctrl = getSlotController(i);
        RFX_OBJ_CLOSE(ctrl);
    }
    // Check Non Slot Controller is destroyed
    ASSERT_TRUE(getNSlotController() == NULL);

    // Check the Slot Controller is destroyed
    for (int i = 0; i < RFX_SLOT_COUNT; i++) {
        ASSERT_TRUE(getSlotController(i) == NULL);
    }

    sleep(1);
}


} // namespace android
