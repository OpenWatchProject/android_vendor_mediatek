
#ifndef __FM_HIDL_SERVICE_H__
#define __FM_HIDL_SERVICE_H__

#include <vendor/mediatek/hardware/fm/1.0/IFmRadio.h>
#include <hidl/Status.h>
#include <hidl/HidlSupport.h>
#include <hidl/HidlTransportSupport.h>
#include <semaphore.h>

#include <android-base/logging.h>
#include <utils/Looper.h>
#include <utils/StrongPointer.h>
#include <log/log.h>
#include <cutils/list.h>
//#include "mtk_socket_utils.h"

#define LOG_TAG "FmHidlService"

namespace vendor {
namespace mediatek {
namespace hardware {
namespace fm {
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

#define FM_SCAN_MAX_CH_SIZE 200

enum FmCommandType {
    CMD_EXIT = 0,
    CMD_ENABLE,
    CMD_DISABLE,
    CMD_CONFIG,
    CMD_STEP,
    CMD_SEEK_UP,
    CMD_SEEK_DOWN,
    CMD_SCAN_UP,
    CMD_SCAN_DOWN,
    CMD_PRE_SCAN,
    CMD_TUNE,
    CMD_CANCEL_SEARCH,
};

struct FmCommand {
    struct listnode node;
    FmCommandType type;
    Control ctrl;
    struct timespec ts;
};

class FmHidlService : public IFmRadio {
public:

    class FmHidlDeathRecipient : public hidl_death_recipient {
    public:
        FmHidlDeathRecipient(const sp<FmHidlService> fmHidl) : mFmHidl(fmHidl) {
        }
        virtual void serviceDied(uint64_t cookie, const wp<IBase>& who) override {
            UNUSED(cookie);
            UNUSED(who);
            mFmHidl->handleHidlDeath();
        }
    private:
            sp<FmHidlService> mFmHidl;
        };
    virtual void handleHidlDeath();


    FmHidlService();
    ~FmHidlService();

    Return<Result> enable(const sp<IFmRadioCallback>& callback);

    Return<Result> disable();

    Return<Result> startSearch(Direction dir);

    Return<Result> cancelSearch();

    Return<Result> setControl(Control id, uint32_t value);

    Return<Result> getControl(Control id);

    Return<void> getFreq(getFreq_cb _hidl_cb);

    Return<Result> setFreq(uint32_t freq);

    Return<void> getRssi(getRssi_cb _hidl_cb);

    Return<Result> setBand(uint32_t low, uint32_t high);

    Return<Result> setRdsOnOff(uint32_t onOff);

    Return<void> getRdsStatus(getRdsStatus_cb _hidl_cb);

    Return<void> getRadioText(getRadioText_cb _hidl_cb);

    Return<void> getPrgmServices(getPrgmServices_cb _hidl_cb);

    Return<void> getPrgmId(getPrgmId_cb _hidl_cb);

    Return<void> getPrgmType(getPrgmType_cb _hidl_cb);

    Return<void> getAfInfo(getAfInfo_cb _hidl_cb);

private:
    static void* fm_hidl_thread(void *context);
    static void* fm_rds_thread(void *context);
    int sendCommand(FmCommandType type, unsigned int delay_ms, void *param);

protected:
    sp<IFmRadioCallback> mFmCallback;
    sp<FmHidlDeathRecipient> mFmHidlDeathRecipient;
    listnode mFmCommandList;
    int mTuneFreq;
    int mFmFd;
    int mScanChlCount;
    int mAfList[50];
    unsigned int mScanDwell;
    SearchMode mSearchMode;
    Region mRegion;
    Deemphasis mEmphasis;
    Spacing mSpacing;
    Rds mRds;
    bool mEnabled;
    bool mSearching;
    bool mFmTunned;
    bool mInPwrMode;
    bool mSearchCancelled;
    bool mRdsOn;
    bool mRdsReady;
    unsigned short mLastRdsStatus;
    unsigned short mScanTable[FM_SCAN_MAX_CH_SIZE];
    pthread_t mCommandThread;
    pthread_t mRdsThread;
    pthread_cond_t mCommandCond;
    pthread_cond_t mRdsCond;
    pthread_mutex_t mCommandLock;
    pthread_mutex_t mRdsLock;
};

//extern "C" IFmRadio* HIDL_FETCH_IFmRadio(const char* name);

}  // implementation
}  // namespace V1_0
}  // namespace fm
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor

#endif  // __FM_HIDL_SERVICE_H__

