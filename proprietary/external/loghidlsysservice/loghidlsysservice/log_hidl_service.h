#include <vendor/mediatek/hardware/log/1.0/ILog.h>
#include <unistd.h>
#include <pthread.h>

#include <string.h>
#include <utils.h>
#include <cutils/sockets.h>
#include <sys/socket.h>
#include <cutils/sockets.h>
#include <unistd.h>

#ifdef UNUSED
#undef UNUSED
#endif
#define UNUSED(x) (x)//eliminate "warning: unused parameter"

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

class LogHidlService;

class LogHidlCallback: public ILogCallback
{
public:
	LogHidlCallback(LogHidlService* log) : mLog(log){
     }
	~LogHidlCallback(){}

	virtual Return<bool> callbackToClient(const hidl_string& data) override;

private:
    LogHidlService* mLog;
};

class LogHidlDeathRecipient : public hidl_death_recipient {
        public:
        LogHidlDeathRecipient(LogHidlService* log) : mLog(log) {
        }
        virtual void serviceDied(uint64_t /*cookie*/, const wp<IBase>& /*who*/) override;
        
        private:
        LogHidlService* mLog;
    };


class LogHidlService
{
public:
	LogHidlService(const char* name);
	~LogHidlService();

    void InitService();
	bool InitHidl();

    int initSocket();
    void deinitSocket();
    static void *wait_msg(void *p);
    static void *newThreadSendMsg(void *p);
    void SendMsgToSocketServer(const char* name);
    void SocketServerDead();
    void handleHidlDeath();
private:
    char m_Name[64];
	sp<ILog>  m_LogHidl;
	sp<LogHidlCallback> m_Callback;
    sp<LogHidlDeathRecipient> m_Recipient;

    int m_socketID;
    int m_stop;
    int m_threadID;
    char m_socketMsg[256];
    pthread_t  m_thread;   
};

extern int cpp_main();
}  // implementation
}  // namespace V1_0
}  // namespace log
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor
