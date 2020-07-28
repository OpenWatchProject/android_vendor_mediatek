#include <vendor/mediatek/hardware/log/1.0/ILog.h>
#include <string.h>
#include "ConsysLog.h"
#include "GlbDefine.h"
#include "ConsysLogger.h"

namespace vendor {
namespace mediatek {
namespace hardware {
namespace log {
namespace V1_0 {
namespace implementation {

using ::android::sp;
using ::android::wp;

using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::hardware::hidl_string;
using ::android::hidl::base::V1_0::IBase;
using ::android::hardware::hidl_death_recipient;
    
class LogHidlService: public ILog {
public:
	LogHidlService(const char* name);
	~LogHidlService();
    virtual Return<void> setCallback(const sp<ILogCallback>& callback) override;

    virtual Return<bool> sendToServer(const hidl_string& data) override;
    bool commandHandler(const char* cmd);
    class LogHidlDeathRecipient : public hidl_death_recipient {
        public:
        LogHidlDeathRecipient(const sp<LogHidlService> log) : mLog(log) {
        }
        virtual void serviceDied(uint64_t cookie, const wp<IBase>& who) override {
            //UNUSED(cookie);
            //UNUSED(who);
            mLog->handleHidlDeath();
        }
        private:
        sp<LogHidlService> mLog;   
    };
    virtual void handleHidlDeath();
    bool sendToClient(const int msgId,const char* msgdata);
private:
	char mName[64];
	sp<ILogCallback> mLogCallback;
	sp<LogHidlDeathRecipient> mLogHidlDeathRecipient;
};

}  // implementation
}  // namespace V1_0
}  // namespace log
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor
