#include <log/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <cutils/properties.h>
#include <string>
#include <signal.h>


#include "aee_hidl_service.h"
namespace vendor {
namespace mediatek {
namespace hardware {
namespace log {
namespace V1_0 {
namespace implementation {

using ::android::wp;
using ::android::status_t;
using ::android::hardware::Void;
using ::android::hidl::base::V1_0::IBase;

AeeHidlService::AeeHidlService(const char* name) {
    status_t status;
    status = this->registerAsService(name);
    if(status != 0) {
        ALOGE("registerAsService() for name=[%s] failed status=[%d]\n", name, status);
    } else {
        ALOGE("registerAsService() for name=[%s] successful status=[%d]\n", name, status);
    }
    if (strlen(name) < sizeof(this->mName))
        strncpy(this->mName, name, strlen(name));
    else
        ALOGE("service name too long: %s", name);
}

AeeHidlService::~AeeHidlService() {
}

Return<void> AeeHidlService::setCallback(const sp<ILogCallback>& callback) {
    mLogCallback = callback;
    return Void();
}

static void signal_handler(int sig) {
    ALOGE("sigalrm:%d handler, aee hidl service exit.\n", sig);
    exit(1);
}

Return<bool> AeeHidlService::sendToServer(const hidl_string& data) {
    bool ret = false;
    char *cmd[128];
    int pos = -1;
    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));
    sa.sa_flags = SA_RESETHAND;
    sa.sa_handler = signal_handler;
    sigaction(SIGALRM, &sa, NULL);

    sprintf((char *)cmd,"%s",data.c_str());
    std::string cmdline(data.c_str());
    pos = cmdline.find("=");
    if (pos > 0) {
        std::string property = cmdline.substr(0, pos);
        std::string value = cmdline.substr(pos+1);
        property_set(property.c_str(), value.c_str());
    } else {
        ALOGE("invalid comand: %s", data.c_str());
    }
    alarm(60);
    return ret;
}
}  // implementation
}  // namespace V1_0
}  // namespace log
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor

