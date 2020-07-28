#include <vendor/mediatek/hardware/log/1.0/ILog.h>
#include <string.h>

namespace vendor {
namespace mediatek {
namespace hardware {
namespace log {
namespace V1_0 {
namespace implementation {

using ::android::sp;
using ::android::hardware::Return;
using ::android::hardware::hidl_string;

class AeeHidlService: public ILog {
public:
    AeeHidlService(const char* name);
    ~AeeHidlService();
    virtual Return<void> setCallback(const sp<ILogCallback>& callback) override;

    virtual Return<bool> sendToServer(const hidl_string& data) override;
private:
    char mName[64];
    sp<ILogCallback> mLogCallback;
};

//extern int cpp_main();

}  // implementation
}  // namespace V1_0
}  // namespace log
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor
