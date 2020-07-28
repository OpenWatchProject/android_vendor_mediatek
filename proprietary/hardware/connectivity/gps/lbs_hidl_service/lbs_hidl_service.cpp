#include <android/log.h>
#include <unistd.h>    // usleep
#include <sys/epoll.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include <cutils/properties.h>

#include <hidl/LegacySupport.h>

#include "lbs_hidl_service.h"

namespace vendor {
namespace mediatek {
namespace hardware {
namespace lbs {
namespace V1_0 {
namespace implementation {

using ::android::wp;
using ::android::status_t;
using ::android::hardware::hidl_death_recipient;
using ::android::hidl::base::V1_0::IBase;

char g_ver[] = "1.06";
// 1.01 add agps_apn <-> lbs hidl service
// 1.02 add mnld -> lbs hidl service
// 1.03 add synchronization for HIDL callback from server to client
// 1.04 add HAL callback return value check
// 1.05 add handler timer to restart
// 1.06 fix AgpsDebugInterface stability issue when the LocationEM is killed by abnormal scenario

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "lbs_hidl_service", __VA_ARGS__);
#define LOGE  LOGD

#define LBS_HANDLER_TIMEOUT        (30 * 1000)
#define MAX_EPOLL_EVENT 50
typedef void (* timer_callback)(sigval_t sig);
int g_epfd = epoll_create(MAX_EPOLL_EVENT);
int g_log_epfd = epoll_create(MAX_EPOLL_EVENT);
int handleCase = 0;

// LPPe Service
sp<LbsHidlService> hidlLPPeAgps;
sp<LbsHidlService> hidlLPPeWlan;
sp<LbsHidlService> hidlLPPeBT;
sp<LbsHidlService> hidlLPPeSensor;
sp<LbsHidlService> hidlLPPeNetwork;
sp<LbsHidlService> hidlLPPeIpAddr;
sp<LbsHidlService> hidlLPPeLbs;
sp<LbsHidlService> hidlApn2Agps;
sp<LbsHidlService> hidlMnld2NlpUtils;
sp<LbsHidlService> hidlMnld2DebugService;
sp<LbsHidlService> hidlDebugService2Mnld;
sp<LbsHidlService> hidlMeta2Mnld;
sp<LbsHidlService> hidlMnld2Mtklogger;
sp<LbsHidlService> hidlMtklogger2Mnld;
sp<LbsHidlService> hidlLbsLogv2s;


// LocationEM, CMCC AGPS Settings, OP01
sp<LbsHidlService> hidlAgpsInterface;
sp<AgpsDebugInterfaceHidlService> hidlAgpsDebugInterface;
sp<Agps2ApnHidlService> hidlAgps2Apn;

static void crash_to_debug();

void covertVector2Array(std::vector<uint8_t> in, char* out) {
    int size = in.size();
    for(int i = 0; i < size; i++) {
        out[i] = in.at(i);
    }
}

void covertArray2Vector(const char* in, int len, std::vector<uint8_t>& out) {
    out.clear();
    for(int i = 0; i < len; i++) {
        out.push_back(in[i]);
    }
}

void dump_vector(std::vector<uint8_t>& vec) {
    LOGD("dump_vector() size=%d\n", (int)vec.size());
    for(int i = 0; i < (int)vec.size(); i++) {
        LOGD(" i=%d %02x\n", i, vec.at(i) & 0xff);
    }
}

void dump_buff(const char* buff, int len) {
    int i = 0;
    LOGD("dump_buff() len=%d\n", len);
    for(i = 0; i < len; i++) {
        LOGD(" i=%d %02x\n", i, buff[i] & 0xff);
    }
}

static void msleep(int interval) {
    usleep(interval * 1000);
}

//-1 failed
int epoll_add_fd(int epfd, int fd) {
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.data.fd = fd;
    ev.events = EPOLLIN;
    //don't set the fd to edge trigger
    //the some event like accept may be lost if two or more clients are connecting to server at the same time
    //level trigger is preferred to avoid event lost
    //do not set EPOLLOUT due to it will always trigger when write is available
    if(epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        LOGE("epoll_add_fd() epoll_ctl() failure reason=[%s]  epfd=%d fd=%d\n", strerror(errno), epfd, fd);
        return -1;
    }
    return 0;
}

//LbsHidlService
LbsHidlService::LbsHidlService(const char* name) :
        mLbsHidlDeathRecipient(new LbsHidlDeathRecipient(this)) {

    safe_strncpy(mName, name, sizeof(mName));
    status_t status;
    status = this->registerAsService(mName);
    if(status != 0) {
        LOGE("[%s] LbsHidlService() registerAsService() for name=[%s] failed status=[%d]",
            g_ver, mName, status);
    }
    mLbsCallback = nullptr;
    sem_init(&mSem, 0, 1);
}

LbsHidlService::~LbsHidlService() {
    LOGD("[%s] ~LbsHidlService()", g_ver);
    sem_destroy(&mSem);
}

Return<void> LbsHidlService::setCallback(const sp<ILbsCallback>& callback) {
    LOGD("[%s][%s] setCallback()", g_ver, mName);
    mLbsCallback = callback;
    mLbsCallback->linkToDeath(mLbsHidlDeathRecipient, 0);
    return Void();
}

Return<bool> LbsHidlService::sendToServer(const hidl_vec<uint8_t>& data) {
    std::vector<uint8_t> tmp = data;
    bool ret = false;
    LOGE("[%s][%s] sendToServer() size=[%d] TBD!!!!", g_ver, mName, (int)tmp.size());
    dump_vector(tmp);
    return ret;
}

Return<bool> LbsHidlService::sendToServerWithCallback(const sp<ILbsCallback>& callback, const hidl_vec<uint8_t>& data) {
    UNUSED(callback);
    std::vector<uint8_t> tmp = data;
    bool ret = false;
    LOGE("[%s][%s] sendToServerWithCallback() size=[%d] TBD!!!", g_ver, mName, (int)tmp.size());
    dump_vector(tmp);
    return ret;
}

void LbsHidlService::handleHidlDeath() {
    sem_wait(&mSem);
    LOGD("[%s][%s] handleHidlDeath()", g_ver, mName);
    mLbsCallback = nullptr;
    sem_post(&mSem);
}

bool LbsHidlService::handleSocketEvent(int fd) {
    int _ret;
    char _buff[16 * 1024] = {0};

    _ret = read(fd, _buff, sizeof(_buff));
    LOGD("[%s][%s] handleSocketEvent() ret=[%d]", g_ver, mName, _ret);
    if(_ret == -1) {
        LOGE("[%s][%s] handleSocketEvent() read() failed, fd=%d err=[%s]%d",
            g_ver, mName, fd, strerror(errno), errno);
        return false;
    }
    sem_wait(&mSem);
    if(mLbsCallback != nullptr) {
        std::vector<uint8_t> tmp;
        covertArray2Vector(_buff, _ret, tmp);
        auto ret = mLbsCallback->callbackToClient(tmp);
        if (!ret.isOk()) {
            LOGE("[%s][%s] handleSocketEvent() callbackToClient() failed", g_ver, mName);
        }
    }
    sem_post(&mSem);
    return true;
}

//UdpHidlService
UdpHidlService::UdpHidlService(const char* name) : LbsHidlService(name) {
    mtk_socket_client_init_local(&mSocketLPPe, mName, SOCK_NS_ABSTRACT);
}

Return<bool> UdpHidlService::sendToServer(const hidl_vec<uint8_t>& data) {
    std::vector<uint8_t> tmp = data;
    LOGD("[%s][%s] sendToServer() size=[%d]", g_ver, mName, (int)tmp.size());


    if(!mtk_socket_client_connect(&mSocketLPPe)) {
        LOGE("[%s][%s] sendToServer() mtk_socket_client_connect() failed", g_ver, mName);
        return false;
    }
    //dump_vector(tmp);
    char _buff[16 * 1024] = {0};
    int _offset = (int)tmp.size();
    covertVector2Array(data, _buff);
    //dump_buff(_buff, _offset);
    int _ret = mtk_socket_write(mSocketLPPe.fd, _buff, _offset);
    if(_ret == -1) {
        LOGE("[%s][%s] sendToServer() mtk_socket_write() failed", g_ver, mName);
        return false;
    }
    mtk_socket_client_close(&mSocketLPPe);
    return true;
}

//AgpsInterfaceHidlService
AgpsInterfaceHidlService::AgpsInterfaceHidlService(const char* name) : LbsHidlService(name) {
}

Return<bool> AgpsInterfaceHidlService::sendToServerWithCallback(const sp<ILbsCallback>& callback, const hidl_vec<uint8_t>& data) {
    std::vector<uint8_t> tmp = data;
    char buff[16 * 1024] = {0};
    bool ret = true;
    int read_len = 0;
    LOGD("[%s][%s] sendToServerWithCallback() size=%d", g_ver, mName, (int)tmp.size());
    int fd = mtk_socket_tcp_connect_local("/dev/socket/agpsd2", SOCK_NS_FILESYSTEM);
    if(fd < 0) {
        LOGE("[%s][%s] mtk_socket_tcp_connect_local() failed", g_ver, mName);
        return false;
    }
    covertVector2Array(data, buff);
    read_len = mtk_socket_write(fd, buff, (int)tmp.size());
    if(read_len == -1) {
        ret = false;
    }
    memset(buff, 0, sizeof(buff));
    read_len = 0;
    if(ret) {
        read_len = read(fd, buff, sizeof(buff));
    }
    close(fd);
    covertArray2Vector(buff, read_len, tmp);
    auto _ret = callback->callbackToClient(tmp);
    if (!_ret.isOk()) {
        LOGE("[%s][%s] sendToServerWithCallback() callbackToClient() failed", g_ver, mName);
    }
    return ret;
}

//AgpsDebugInterfaceHidlService
AgpsDebugInterfaceHidlService::AgpsDebugInterfaceHidlService(const char* name) : LbsHidlService(name) {
    mSocketFd = -1;
    mIsExit = true;
}

Return<void> AgpsDebugInterfaceHidlService::setCallback(const sp<ILbsCallback>& callback) {
    LOGD("[%s][%s] setCallback()", g_ver, mName);
    if(mSocketFd != -1) {
        LOGE("[%s][%s] setCallback() mSocketFd is not -1", g_ver, mName);
        crash_to_debug();
    }
    mLbsCallback = callback;
    mLbsCallback->linkToDeath(mLbsHidlDeathRecipient, 0);
    mSocketFd = mtk_socket_tcp_connect_local("/dev/socket/agpsd3", SOCK_NS_FILESYSTEM);
    if(mSocketFd < 0) {
        LOGE("[%s][%s] setCallback() mtk_socket_tcp_connect_local() failed", g_ver, mName);
        return Void();
    }
    epoll_add_fd(g_epfd, mSocketFd);
    mIsExit = false;
    return Void();
}

Return<bool> AgpsDebugInterfaceHidlService::sendToServer(const hidl_vec<uint8_t>& data) {
    std::vector<uint8_t> tmp = data;
    LOGD("[%s][%s] sendToServer() size=[%d]", g_ver, mName, (int)tmp.size());
    char buff[16 * 1024] = {0};
    int read_len = 0;
    covertVector2Array(data, buff);
    read_len = mtk_socket_write(mSocketFd, buff, (int)tmp.size());
    // we don't care the error happens in this scenario
    UNUSED(read_len);
    mIsExit = true;
    return true;
}

bool AgpsDebugInterfaceHidlService::handleSocketEvent(int fd) {
    int _ret;
    char _buff[16 * 1024] = {0};

    _ret = read(fd, _buff, sizeof(_buff));
    LOGD("[%s][%s] handleSocketEvent() ret=[%d] isExit=[%d]", g_ver, mName, _ret, mIsExit);
    if(mIsExit) {
        LOGD("[%s][%s] handleSocketEvent() close mSocketFd: %d", g_ver, mName, mSocketFd);
        close(mSocketFd);
        mSocketFd = -1;
    } else if (_ret <= 0) {
        LOGE("[%s][%s] handleSocketEvent() read() failed, fd=%d err=[%s]%d",
            g_ver, mName, fd, strerror(errno), errno);
        close(mSocketFd);
        mSocketFd = -1;
        while(mSocketFd < 0) {
            msleep(500);
            mSocketFd = mtk_socket_tcp_connect_local("/dev/socket/agpsd3", SOCK_NS_FILESYSTEM);
            LOGD("[%s][%s] handleSocketEvent() mtk_socket_tcp_connect_local() mSocketFd=[%d]",
                g_ver, mName, mSocketFd);
            if(mSocketFd > 0) {
                epoll_add_fd(g_epfd, mSocketFd);
                break;
            }
        }
        LOGD("[%s][%s] handleSocketEvent() re-connect done", g_ver, mName);
    } else {
        sem_wait(&mSem);
        if(mLbsCallback != nullptr) {
            std::vector<uint8_t> tmp;
            covertArray2Vector(_buff, _ret, tmp);
            auto ret = mLbsCallback->callbackToClient(tmp);
            if (!ret.isOk()) {
                LOGE("[%s][%s] handleSocketEvent() callbackToClient() failed", g_ver, mName);
            }
        }
        sem_post(&mSem);
    }
    return true;
}

void AgpsDebugInterfaceHidlService::handleHidlDeath() {
    LbsHidlService::handleHidlDeath();

    if(mSocketFd > 0) {
        char buff[16 * 1024] = {0};
        int len = 0;
        len = mtk_socket_write(mSocketFd, buff, 16);
        // we don't care the error happens in this scenario
        UNUSED(len);
        mIsExit = true;
    }
}

int AgpsDebugInterfaceHidlService::getSocketFd() {
    return mSocketFd;
}


//Agps2ApnHidlService,  lbs hidl service -> agps_apn
Agps2ApnHidlService::Agps2ApnHidlService(const char* name) : UdpHidlService(name) {
    mSocketFd = -1;
}

Return<void> Agps2ApnHidlService::setCallback(const sp<ILbsCallback>& callback) {
    LbsHidlService::setCallback(callback);

    mSocketFd = mtk_socket_server_bind_local("mtk_agps2framework", SOCK_NS_ABSTRACT);
    if(mSocketFd < 0) {
        LOGE("[%s][%s] setCallback() mtk_socket_server_bind_local(mtk_agps2framework) failed", g_ver, mName);
        return Void();
    }
    epoll_add_fd(g_epfd, mSocketFd);
    return Void();
}

void Agps2ApnHidlService::handleHidlDeath() {
    LbsHidlService::handleHidlDeath();

    if (mSocketFd < 0) {
        LOGE("[%s][%s] handleHidlDeath() error mSocketFd: %d", g_ver, mName, mSocketFd);
        return;
    }

    LOGD("[%s][%s] handleHidlDeath(), close mSocketFd: %d", g_ver, mName, mSocketFd);
    close(mSocketFd);
    mSocketFd = -1;
}

int Agps2ApnHidlService::getSocketFd() {
    return mSocketFd;
}

/*************************************************
* Timer
**************************************************/
// -1 means failure

timer_t init_timer_id(timer_callback cb, int id) {
    struct sigevent sevp;
    timer_t timerid;

    memset(&sevp, 0, sizeof(sevp));
    sevp.sigev_value.sival_int = id;
    sevp.sigev_notify = SIGEV_THREAD;
    sevp.sigev_notify_function = cb;

    if (timer_create(CLOCK_MONOTONIC, &sevp, &timerid) == -1) {
        LOGE("timer_create  failed reason=[%s]", strerror(errno));
        return (timer_t)-1;
    }
    LOGE("successfully timer created id= %ld", (long)timerid);
    return timerid;
}

// -1 means failure
timer_t init_timer(timer_callback cb) {
    return init_timer_id(cb, 0);
}

// -1 means failure
int start_timer(timer_t timerid, int milliseconds) {
    struct itimerspec expire;
    expire.it_interval.tv_sec = 0;
    expire.it_interval.tv_nsec = 0;
    expire.it_value.tv_sec = milliseconds/1000;
    expire.it_value.tv_nsec = (milliseconds%1000)*1000000;
    return timer_settime(timerid, 0, &expire, NULL);
}

// -1 means failure
int stop_timer(timer_t timerid) {
    return start_timer(timerid, 0);
}

// -1 means failure
int deinit_timer(timer_t timerid) {
    if (timer_delete(timerid) == -1) {
        // errno
        return -1;
    }
    return 0;
}


bool lbs_timeout_restart_enabled(void) {
    char result[PROPERTY_VALUE_MAX] = {0};

    if((property_get("ro.vendor.mtk_hidl_consolidation", result, NULL) != 0) && (result[0] == '1')) {
        LOGD("LBS_timeout disabled because hidl service consolidation enabled!!!");
        return false;
    } else {
        LOGD("LBS_timeout enabled because hidl service consolidation disabled!!!");
        return true;
    }
}

bool lbs_timeout_ne_enabled(void) {
    char result[PROPERTY_VALUE_MAX] = {0};

    if((property_get("vendor.debug.gps.mnld.ne", result, NULL) != 0) && (result[0] == '1')) {
        LOGD("NE enabled!!!");
        return true;
    } else {
        LOGD("NE disabled!!!");
        return false;
    }
}

static void crash_to_debug() {
    int* crash = 0;
    *crash = 100;
}

static void lbs_control_thread_timeout(sigval_t sig) {
    UNUSED(sig);

    LOGE("lbs_control timeout handleCase = %d", handleCase);
    if (lbs_timeout_restart_enabled()) {
        if (lbs_timeout_ne_enabled() == false) {
            LOGE("lbs_control_thread_timeout() _exit.");
            _exit(0);
        } else {
            LOGE("lbs_control_thread_timeout() crash here for debugging");
            crash_to_debug();
        }
    }
}

static void* lbs_hidl_thread(void *arg) {
    UNUSED(arg);
    struct epoll_event events[MAX_EPOLL_EVENT];
    int fd_lppe_wlan         = mtk_socket_server_bind_local("mtk_lppe_socket_wlan", SOCK_NS_ABSTRACT);
    int fd_lppe_bt           = mtk_socket_server_bind_local("mtk_lppe_socket_bt", SOCK_NS_ABSTRACT);
    int fd_lppe_sensor       = mtk_socket_server_bind_local("mtk_lppe_socket_sensor", SOCK_NS_ABSTRACT);
    int fd_lppe_network      = mtk_socket_server_bind_local("mtk_lppe_socket_network", SOCK_NS_ABSTRACT);
    int fd_lppe_ipaddr       = mtk_socket_server_bind_local("mtk_lppe_socket_ipaddr", SOCK_NS_ABSTRACT);
    int fd_lppe_lbs          = mtk_socket_server_bind_local("mtk_lppe_socket_lbs", SOCK_NS_ABSTRACT);
    int fd_mnld2nlputils     = mtk_socket_server_bind_local("mtk_mnld2nlputils", SOCK_NS_ABSTRACT);
    int fd_mnld2debugService = mtk_socket_server_bind_local("mtk_mnld2debugService", SOCK_NS_ABSTRACT);
    int fd_mnld2mtklogger    = mtk_socket_server_bind_local("mtk_mnld2mtklogger", SOCK_NS_ABSTRACT);
    timer_t hdlr_timer       = init_timer(lbs_control_thread_timeout);

    epoll_add_fd(g_epfd, fd_lppe_wlan);
    epoll_add_fd(g_epfd, fd_lppe_bt);
    epoll_add_fd(g_epfd, fd_lppe_sensor);
    epoll_add_fd(g_epfd, fd_lppe_network);
    epoll_add_fd(g_epfd, fd_lppe_ipaddr);
    epoll_add_fd(g_epfd, fd_lppe_lbs);
    epoll_add_fd(g_epfd, fd_mnld2nlputils);
    epoll_add_fd(g_epfd, fd_mnld2debugService);
    epoll_add_fd(g_epfd, fd_mnld2mtklogger);

    while(1) {
        int i;
        int n;

        n = epoll_wait(g_epfd, events, MAX_EPOLL_EVENT , -1);
        if(n == -1) {
            if(errno == EINTR) {
                continue;
            } else {
                LOGE("[%s] lbs_hidl_thread() epoll_wait() failure reason=[%s]\n", g_ver, strerror(errno));
                return NULL;
            }
        }

        start_timer(hdlr_timer, LBS_HANDLER_TIMEOUT);
        for(i = 0; i < n; i++) {
            if(events[i].data.fd == fd_lppe_wlan && events[i].events & EPOLLIN) {
                handleCase = 1;
                hidlLPPeWlan->handleSocketEvent(fd_lppe_wlan);
            } else if(events[i].data.fd == fd_lppe_bt && events[i].events & EPOLLIN) {
                handleCase = 2;
                hidlLPPeBT->handleSocketEvent(fd_lppe_bt);
            } else if(events[i].data.fd == fd_lppe_sensor && events[i].events & EPOLLIN) {
                handleCase = 3;
                hidlLPPeSensor->handleSocketEvent(fd_lppe_sensor);
            } else if(events[i].data.fd == fd_lppe_network && events[i].events & EPOLLIN) {
                handleCase = 4;
                hidlLPPeNetwork->handleSocketEvent(fd_lppe_network);
            } else if(events[i].data.fd == fd_lppe_ipaddr && events[i].events & EPOLLIN) {
                handleCase = 5;
                hidlLPPeIpAddr->handleSocketEvent(fd_lppe_ipaddr);
            } else if(events[i].data.fd == fd_lppe_lbs && events[i].events & EPOLLIN) {
                handleCase = 6;
                hidlLPPeLbs->handleSocketEvent(fd_lppe_lbs);
            } else if(events[i].data.fd == hidlAgpsDebugInterface->getSocketFd() && events[i].events & EPOLLIN) {
                handleCase = 7;
                hidlAgpsDebugInterface->handleSocketEvent(hidlAgpsDebugInterface->getSocketFd());
            } else if(events[i].data.fd == hidlAgps2Apn->getSocketFd() && events[i].events & EPOLLIN) {
                handleCase = 8;
                hidlAgps2Apn->handleSocketEvent(hidlAgps2Apn->getSocketFd());
            } else if(events[i].data.fd == fd_mnld2nlputils && events[i].events & EPOLLIN) {
                handleCase = 9;
                hidlMnld2NlpUtils->handleSocketEvent(fd_mnld2nlputils);
            } else if(events[i].data.fd == fd_mnld2debugService && events[i].events & EPOLLIN) {
                handleCase = 10;
                hidlMnld2DebugService->handleSocketEvent(fd_mnld2debugService);
            } else if(events[i].data.fd == fd_mnld2mtklogger && events[i].events & EPOLLIN) {
                handleCase = 11;
                hidlMnld2Mtklogger->handleSocketEvent(fd_mnld2mtklogger);
            } else {
                LOGE("[%s] lbs_hidl_thread() fd=%d is unhandled", g_ver, events[i].data.fd);
            }
        }
        stop_timer(hdlr_timer);
    }
    return NULL;
}

static void* lbs_hidl_log_thread(void *arg) {
    UNUSED(arg);
    struct epoll_event events[MAX_EPOLL_EVENT];
    int fd_lbs_log_v2s       = mtk_socket_server_bind_local("mtk_lbs_log_v2s", SOCK_NS_ABSTRACT);

    epoll_add_fd(g_log_epfd, fd_lbs_log_v2s);

    while(1) {
        int i;
        int n;

        n = epoll_wait(g_log_epfd, events, MAX_EPOLL_EVENT , -1);
        if(n == -1) {
            if(errno == EINTR) {
                continue;
            } else {
                LOGE("[%s] lbs_hidl_log_thread() epoll_wait() failure reason=[%s]\n", g_ver, strerror(errno));
                return NULL;
            }
        }

        for(i = 0; i < n; i++) {
            if(events[i].data.fd == fd_lbs_log_v2s && events[i].events & EPOLLIN) {
                hidlLbsLogv2s->handleSocketEvent(fd_lbs_log_v2s);
            } else {
                LOGE("[%s] lbs_hidl_log_thread() fd=%d is unhandled", g_ver, events[i].data.fd);
                crash_to_debug();
            }
        }
    }
    return NULL;
}

int cpp_main() {
    LOGD("[%s] lbs hidl service is running", g_ver);

    ::android::hardware::configureRpcThreadpool(20, true);

    hidlLPPeAgps      = new UdpHidlService("mtk_lppe_socket_agps");
    hidlLPPeWlan      = new UdpHidlService("mtk_lppe_socket_wlan");
    hidlLPPeBT        = new UdpHidlService("mtk_lppe_socket_bt");
    hidlLPPeSensor    = new UdpHidlService("mtk_lppe_socket_sensor");
    hidlLPPeNetwork   = new UdpHidlService("mtk_lppe_socket_network");
    hidlLPPeIpAddr    = new UdpHidlService("mtk_lppe_socket_ipaddr");
    hidlLPPeLbs       = new UdpHidlService("mtk_lppe_socket_lbs");

    // agps_apn -> lbs hidl service
    hidlApn2Agps = new UdpHidlService("mtk_framework2agps");
    // lbs hidl service -> agps_apn
    hidlAgps2Apn = new Agps2ApnHidlService("mtk_agps2framework");
    // mnld -> lbs hidl service
    hidlMnld2NlpUtils = new UdpHidlService("mtk_mnld2nlputils");
    // mnld -> debug service
    hidlMnld2DebugService = new UdpHidlService("mtk_mnld2debugService");
    // debug service  -> mnld
    hidlDebugService2Mnld = new UdpHidlService("mtk_debugService2mnld");
    // meta  -> mnld
    hidlMeta2Mnld = new UdpHidlService("mtk_meta2mnld");

    hidlAgpsInterface = new AgpsInterfaceHidlService("AgpsInterface");
    hidlAgpsDebugInterface = new AgpsDebugInterfaceHidlService("AgpsDebugInterface");

    // mnld -> mtklogger
    hidlMnld2Mtklogger = new UdpHidlService("mtk_mnld2mtklogger");
    // mtklogger -> mnld
    hidlMtklogger2Mnld = new UdpHidlService("mtk_mtklogger2mnld");
    // vendor -> system lbs log
    hidlLbsLogv2s= new UdpHidlService("mtk_lbs_log_v2s");

    // ... add more LBS HIDL Service here

    pthread_t pthread1;
    pthread_t pthread2;
    pthread_create(&pthread1, NULL, lbs_hidl_thread, NULL);
    pthread_create(&pthread2, NULL, lbs_hidl_log_thread, NULL);

    return 0;
}

}  // implementation
}  // namespace V1_0
}  // namespace lbs
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor

