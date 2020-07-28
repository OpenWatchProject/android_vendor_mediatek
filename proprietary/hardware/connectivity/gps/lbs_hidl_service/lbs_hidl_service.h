
#ifndef __LBS_HIDL_SERVICE_H__
#define __LBS_HIDL_SERVICE_H__

#include <vendor/mediatek/hardware/lbs/1.0/ILbs.h>
#include <semaphore.h>
#include "mtk_socket_utils.h"

namespace vendor {
namespace mediatek {
namespace hardware {
namespace lbs {
namespace V1_0 {
namespace implementation {

using ::android::sp;
using ::android::wp;

using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::hardware::hidl_vec;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_death_recipient;
using ::android::hidl::base::V1_0::IBase;

#ifdef UNUSED
#undef UNUSED
#endif
#define UNUSED(expr) do { (void)(expr); } while (0)

class LbsHidlService : public ILbs {
public:
    LbsHidlService(const char* name);
    ~LbsHidlService();

    virtual Return<void> setCallback(const sp<ILbsCallback>& callback) override;
    virtual Return<bool> sendToServer(const hidl_vec<uint8_t>& data) override;
    virtual Return<bool> sendToServerWithCallback(const sp<ILbsCallback>& callback, const hidl_vec<uint8_t>& data) override;

    class LbsHidlDeathRecipient : public hidl_death_recipient {
        public:
        LbsHidlDeathRecipient(const sp<LbsHidlService> lbs) : mLbs(lbs) {
        }
        virtual void serviceDied(uint64_t cookie, const wp<IBase>& who) override {
            UNUSED(cookie);
            UNUSED(who);
            mLbs->handleHidlDeath();
        }
        private:
        sp<LbsHidlService> mLbs;
    };
    virtual void handleHidlDeath();

    virtual bool handleSocketEvent(int fd);

protected:
    char mName[64];
    sp<ILbsCallback> mLbsCallback;
    sp<LbsHidlDeathRecipient> mLbsHidlDeathRecipient;
    sem_t mSem;
};

class UdpHidlService : public LbsHidlService {
public:
    UdpHidlService(const char* name);
    Return<bool> sendToServer(const hidl_vec<uint8_t>& data) override;
protected:
    mtk_socket_fd mSocketLPPe;
};

class AgpsInterfaceHidlService : public LbsHidlService {
public:
    AgpsInterfaceHidlService(const char* name);
    Return<bool> sendToServerWithCallback(const sp<ILbsCallback>& callback, const hidl_vec<uint8_t>& data) override;
};

class AgpsDebugInterfaceHidlService : public LbsHidlService {
public:
    AgpsDebugInterfaceHidlService (const char* name);
    Return<void> setCallback(const sp<ILbsCallback>& callback) override;
    Return<bool> sendToServer(const hidl_vec<uint8_t>& data) override;
    bool handleSocketEvent(int fd) override;
    void handleHidlDeath() override;
    int getSocketFd();
protected:
    int mSocketFd;
    bool mIsExit;
};

class Agps2ApnHidlService : public UdpHidlService {
public:
    Agps2ApnHidlService (const char* name);
    Return<void> setCallback(const sp<ILbsCallback>& callback) override;
    void handleHidlDeath() override;
    int getSocketFd();
protected:
    int mSocketFd;
};

extern int cpp_main();

}  // implementation
}  // namespace V1_0
}  // namespace lbs
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor

#endif  // __LBS_HIDL_SERVICE_H__

