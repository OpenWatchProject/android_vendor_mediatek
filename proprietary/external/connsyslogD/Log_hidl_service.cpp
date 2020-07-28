#include <hidl/LegacySupport.h>

#include "Log_hidl_service.h"
extern bool setStoragePath(char* path);
namespace vendor {
namespace mediatek {
namespace hardware {
namespace log {
namespace V1_0 {
namespace implementation {

using ::android::wp;
using ::android::status_t;
//using ::android::hardware::hidl_death_recipient;
using ::android::hidl::base::V1_0::IBase;

bool LogHidlService::commandHandler(const char* cmd) {

    LOGI("consyslogger receive HIDL command %s", cmd);
    int ret = 1;
    if (!strncmp(cmd, "deep_stop", strlen("deep_stop"))) {
        ret = executeCommand(OP_DEEP_STOP_LOGGING);
              
    } else if (!strncmp(cmd, "deep_start", strlen("deep_start"))) {
        ret = executeCommand(OP_DEEP_START_LOGGING);
        if (ret == 0) {
            LOGE("HIDL_DEEP_START_LOGGING, errno:%d, %s", errno, strerror(errno));
        }
   
    }  else if (!strncmp(cmd, "logsize=", strlen("logsize="))) {
        char temp[64];
        memset(temp, '\0', 64);
        strncpy(temp, &cmd[strlen("logsize=")], sizeof(temp) - 1);
        temp[sizeof(temp) - 1] = '\0';
        ret = executeCommand(OP_SET_RECYCLE_SIZE, atoi(temp));
    
    } else if (!strncmp(cmd, "autostart=", strlen("autostart="))) {
        ret = executeCommand(OP_SET_AUTOSTART_LOGGING_MODE,
                cmd[strlen("autostart=")] - '0');

    } else if (!strncmp(cmd,"set_storage_path", strlen("set_storage_path"))) {
        char temp[256];
        memset(temp, '\0', 256);
        strncpy(temp, &cmd[strlen("set_storage_path,")], sizeof(temp) - 1);
        temp[sizeof(temp) - 1] = '\0';
        ret = setStoragePath(temp);

    } else if (!strncmp(cmd,"set_btfw_log_level,",strlen("set_btfw_log_level,"))) {
        ret = executeCommand(OP_SET_DEBUG_LEVEL,cmd[strlen("set_btfw_log_level,")] - '0');
             
    } else {
        LOGE("hidl unkown cmd %s", cmd);
        return 0;
    }
    
    char _buff[256] = {0};
    sprintf(_buff, "%s,%d", cmd, ret);

    LOGD("callbackToClient() = %s", _buff);
    if (mLogCallback == nullptr) {
    	LOGI("[%s] mLogCallback is null", mName);
    	return false;
    }
    ret = mLogCallback->callbackToClient(_buff);     
    return ret;
}
//LbsHidlService
LogHidlService::LogHidlService(const char* name):
        mLogHidlDeathRecipient(new LogHidlDeathRecipient(this)) {
    
	LOGI("LogHidlService name = [%s]", name);
    status_t status;
    status = this->registerAsService(name);
    if(status != 0) {
    	LOGD("registerAsService() for name=[%s] failed status=[%d]",name, status);
    } else {
    	LOGD("registerAsService() for name=[%s] successful status=[%d]",name, status);
    }
    mLogCallback = nullptr;
    strncpy(this->mName, name,strlen(name));
    LOGD("LogHidlService Done!");
}

LogHidlService::~LogHidlService() {
	LOGI("[%s] ~LogHidlService()", mName);
}

Return<void> LogHidlService::setCallback(const sp<ILogCallback>& callback) {
	LOGD("[%s] setCallback()", mName);
    mLogCallback = callback;
    mLogCallback->linkToDeath(mLogHidlDeathRecipient, 0);
    return Void();
}

Return<bool> LogHidlService::sendToServer(const hidl_string& data) {
    
    LOGI("[%s] sendToServer() data = [%s]!!!!", mName, data.c_str());
    bool ret = commandHandler(data.c_str());
   /* if (mLogCallback == nullptr) {
    	LOGI("[%s] mLogCallback is null", mName);
    	return false;
    }
    ret = mLogCallback->callbackToClient(data); */ 
    return ret;
}

void LogHidlService::handleHidlDeath() {
    LOGD("handleHidlDeath():client [%s] died. ", mName);
    mLogCallback = nullptr;

}

bool LogHidlService::sendToClient(const int msgId,const char * msgdata) {
     if (mLogCallback == nullptr) {
    	LOGI("[%s] mLogCallback is null", mName);
    	return false;
    }
    const char *msg;
    switch (msgId) {
        case MSG_FAIL_WRITE_FILE: {
            msg = "FAIL_WRITEFILE";
            break;
        }
        case MSG_SDCARD_NO_EXIST: {
            msg = "SDCARD_NOTEXIST";
            break;
        }
        case MSG_SDCARD_IS_FULL: {
            msg = "SDCARD_FULL";
            break;
        }
        case MSG_SDCARD_NO_LOG_FILE: {
            msg = "LOGFILE_NOTEXIST";
            break;
        }
        default: {
            msg = msgdata;
            break;
        }
    }
    return mLogCallback->callbackToClient(msg);
}
}  // implementation
}  // namespace V1_0
}  // namespace log
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor

