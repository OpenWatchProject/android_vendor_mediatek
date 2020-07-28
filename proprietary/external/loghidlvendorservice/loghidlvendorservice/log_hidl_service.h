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
#define UNUSED(x) (void)(x)//eliminate "warning: unused parameter"

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

typedef struct __CLIENT {
    int fd;
    struct sockaddr addr;
} CLIENT;

class LogHidlService: public ILog {
public:
	LogHidlService(const char* name);
	~LogHidlService();
    virtual Return<void> setCallback(const sp<ILogCallback>& callback) override;

    virtual Return<bool> sendToServer(const hidl_string& data) override;

    class LogHidlDeathRecipient : public hidl_death_recipient {
        public:
        LogHidlDeathRecipient(const sp<LogHidlService> log) : mLog(log) {
        }
        virtual void serviceDied(uint64_t cookie, const wp<IBase>& who) override {
            UNUSED(cookie);
            UNUSED(who);
            mLog->handleHidlDeath();
        }
        private:
        sp<LogHidlService> mLog;   
    };
    virtual void handleHidlDeath();

    void init();
//for socket
  void initSocketServer();
  static void *socketListener(void *p);
  void runListener();
  bool SocketServerhandleMessage(int fd);
  bool shouldtimeout;
  CLIENT  client[10];
  pthread_mutex_t mlock;
  timeval timeout;
  int clientConnect;
  void SendMessageToSocketClient(const char* msg);

public:
	int m_socketID;
	int m_threadID;
	int m_stop;
	pthread_t  m_thread;

	char m_Name[64];
	sp<ILogCallback> mLogCallback;
	sp<LogHidlDeathRecipient> mLogHidlDeathRecipient;

};
extern int cpp_main();
}  // implementation
}  // namespace V1_0
}  // namespace log
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor
