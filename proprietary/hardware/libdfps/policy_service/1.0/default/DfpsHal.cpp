#define LOG_TAG "DfpsPassthrough"

#include "DfpsHal.h"

#include <inttypes.h>
#include <type_traits>
#include <log/log.h>

#include "dfps/dynamicfps_defs.h"

#include "FpsPolicyClient.h"
#include "FpsInfoClient.h"
#include "FpsVsyncClient.h"

namespace vendor {
namespace mediatek {
namespace hardware{
namespace dfps {
namespace V1_0 {
namespace implementation {

using android::hardware::Void;
using android::wp;

typedef android::hidl::base::V1_0::IBase HInterface;

class FpsPolicyDeathRecipient : public ::android::hardware::hidl_death_recipient {
public:
    FpsPolicyDeathRecipient(hidl_string name, uint64_t sequence, const wp<DfpsHal>& hal) {
        extendObjectLifetime(OBJECT_LIFETIME_WEAK);
        mName = name;
        mSequence = sequence;
        mHal = hal;
    }

    ~FpsPolicyDeathRecipient() {
    }

    void serviceDied(uint64_t, const wp<HInterface>&) {
        ALOGE("FpsPolicy(%" PRIx64 ") %s is died!!", mSequence, mName.c_str());
        sp<DfpsHal> hal = mHal.promote();
        if (hal != NULL) {
            hal->removePolicyPackage(mSequence);
            hal->unregPolicy(mSequence, DFPS_FLAG_BINDER_DIED);
        }
    }
private:
    hidl_string mName;
    uint64_t mSequence;
    wp<DfpsHal> mHal;
};

class FpsInfoDeathRecipient : public ::android::hardware::hidl_death_recipient {
public:
    FpsInfoDeathRecipient(uint64_t id, const wp<DfpsHal>& hal) {
        extendObjectLifetime(OBJECT_LIFETIME_WEAK);
        mId = id;
        mHal = hal;
    }

    ~FpsInfoDeathRecipient() {
    }

    void serviceDied(uint64_t, const wp<HInterface>&) {
        ALOGE("FpsInfo(%" PRIx64 ") is died!!", mId);
        sp<DfpsHal> hal = mHal.promote();
        if (hal != NULL) {
            hal->removeInfoPackage(mId);
            hal->unregInfo(mId, DFPS_FLAG_BINDER_DIED);
        }
    }
private:
    uint64_t mId;
    wp<DfpsHal> mHal;
};

class FpsVsyncDeathRecipient : public ::android::hardware::hidl_death_recipient {
public:
    FpsVsyncDeathRecipient() {
        extendObjectLifetime(OBJECT_LIFETIME_WEAK);
    }

    ~FpsVsyncDeathRecipient() {
    }

    void serviceDied(uint64_t, const wp<HInterface>&) {
        ALOGE("FpsVsync is died !!");
    }
private:
};

DfpsHal::DfpsHal(const hw_module_t* module)
{
    hw_device_t* device = nullptr;
    int error = module->methods->open(module, DFPS_DYNAMIC_FPS, &device);
    if (error != 0) {
        ALOGE("Failed to open DFPS device (%s), aborting", strerror(-error));
        abort();
    }

    mDevice = reinterpret_cast<dfps_device_t*>(device);
    initDispatch();
    mClient = nullptr;
}

DfpsHal::~DfpsHal() {
}

template<typename T>
void DfpsHal::initDispatch(dfps_function_descriptor_t desc, T* outPfn)
{
    auto pfn = mDevice->getFunction(mDevice, desc);
    if (!pfn) {
        LOG_ALWAYS_FATAL("failed to get hwcomposer2 function %d", desc);
    }

    *outPfn = reinterpret_cast<T>(pfn);
}

void DfpsHal::initDispatch()
{
    initDispatch(DFPS_FUNCTION_REG_POLICY, &mDispatch.regPolicy);
    initDispatch(DFPS_FUNCTION_UNREG_POLICY, &mDispatch.unregPolicy);
    initDispatch(DFPS_FUNCTION_SET_FPS, &mDispatch.setFps);
    initDispatch(DFPS_FUNCTION_SET_SINGLE_LAYER, &mDispatch.setSingleLayer);
    initDispatch(DFPS_FUNCTION_SET_NUMBER_DISPLAY,&mDispatch.setNumberDisplay);
    initDispatch(DFPS_FUNCTION_SET_HWC_INFO, &mDispatch.setHwcInfo);
    initDispatch(DFPS_FUNCTION_SET_INPUT_WINDOWS, &mDispatch.setInputWindows);
    initDispatch(DFPS_FUNCTION_REG_INFO, &mDispatch.regInfo);
    initDispatch(DFPS_FUNCTION_UNREG_INFO, &mDispatch.unregInfo);
    initDispatch(DFPS_FUNCTION_GET_PANEL_INFO, &mDispatch.getPanelInfo);
    initDispatch(DFPS_FUNCTION_SET_FOREGROUND_INFO, &mDispatch.setForegroundInfo);
    initDispatch(DFPS_FUNCTION_GET_FPS_RANGE, &mDispatch.getFpsRange);
    initDispatch(DFPS_FUNCTION_SET_WINDOW_FLAG, &mDispatch.setWindowFlag);
    initDispatch(DFPS_FUNCTION_ENABLE_TRACKER, &mDispatch.enableTracker);
    initDispatch(DFPS_FUNCTION_FORBID_ADJUST_VSYNC, &mDispatch.forbidAdjustVsync);
    initDispatch(DFPS_FUNCTION_DUMP, &mDispatch.dump);
    initDispatch(DFPS_FUNCTION_REGISTER_CALLBACK, &mDispatch.registerCallback);
}

Return<void> DfpsHal::createPolicyClient(createPolicyClient_cb hidl_cb)
{
    Error err = Error::NONE;
    sp<FpsPolicyClient> client = new FpsPolicyClient(*this);

    hidl_cb(err, client);

    return Void();
}

Return<void> DfpsHal::createInfoClient(createInfoClient_cb hidl_cb)
{
    Error err = Error::NONE;
    sp<FpsInfoClient> client = new FpsInfoClient(*this);

    hidl_cb(err, client);

    return Void();
}

Return<void> DfpsHal::createVsyncClient(createVsyncClient_cb hidl_cb)
{
    Error err = Error::NONE;
    sp<FpsVsyncClient> client;
    {
        std::lock_guard<std::mutex> lock(mClientMutex);
        if (mClient == nullptr) {
            client = new FpsVsyncClient(*this);
            mClient = client;
        } else {
            err = Error::NO_RESOURCES;
        }
    }

    hidl_cb(err, client);

    return Void();
}

Return<void> DfpsHal::dumpDebugInfo(dumpDebugInfo_cb hidl_cb)
{
    uint32_t len = 0;
    mDispatch.dump(mDevice, &len, nullptr);

    std::vector<char> buf(len + 1);
    mDispatch.dump(mDevice, &len, buf.data());
    buf.resize(len + 1);
    buf[len] = '\0';

    hidl_string buf_reply;
    buf_reply.setToExternal(buf.data(), len);
    hidl_cb(buf_reply);

    return Void();
}

Error DfpsHal::regPolicy(const sp<IFpsPolicy>& policy, hidl_string name, FpsPolicyInfo info, int32_t *receiveFd)
{
    int32_t err = 0;
    *receiveFd = -1;
    if (policy != NULL) {
        sp<FpsPolicyDeathRecipient> fpdr = new FpsPolicyDeathRecipient(name, info.sequence, this);
        policy->linkToDeath(fpdr, 0);
        dfps_fps_policy_info_t policyInfo;
        transferFpsPolicyInfo(&policyInfo, info);
        err = mDispatch.regPolicy(mDevice, name.c_str(), name.size(), policyInfo, receiveFd);
        PolicyPackage package = {info.sequence, policy, fpdr};
        {
            std::lock_guard<std::mutex> lock(mClientMutex);
            mPolicyPackageList.push_back(package);
        }
    } else {
        ALOGE("register policy with a null IFpsPolicy");
        err = -EINVAL;
    }

    return static_cast<Error>(err);
}

Error DfpsHal::unregPolicy(uint64_t sequence)
{
    int32_t err = mDispatch.unregPolicy(mDevice, sequence, DFPS_FLAG_NONE);

    return static_cast<Error>(err);
}

void DfpsHal::unregPolicy(uint64_t sequence, int32_t flag)
{
    mDispatch.unregPolicy(mDevice, sequence, flag);
}

Error DfpsHal::setFps(FpsPolicyInfo info)
{
    dfps_fps_policy_info_t policyInfo;
    transferFpsPolicyInfo(&policyInfo, info);
    int32_t err = mDispatch.setFps(mDevice, policyInfo);

    return static_cast<Error>(err);
}

Error DfpsHal::getPanelInfo(PanelInfo *info)
{
    dfps_panel_info_t panelInfo;
    int32_t err = mDispatch.getPanelInfo(mDevice, &panelInfo);
    transferPanelInfo(info, panelInfo);

    return static_cast<Error>(err);
}

Error DfpsHal::getFpsRange(FpsRange *range)
{
    dfps_fps_range_t rangeInfo;
    rangeInfo.index = range->index;
    int32_t err = mDispatch.getFpsRange(mDevice, &rangeInfo);
    transferFpsRange(range, rangeInfo);

    return static_cast<Error>(err);
}

Error DfpsHal::regInfo(const sp<IFpsInfo>& info, uint64_t id)
{
    int32_t err = 0;
    if (info != NULL) {
        sp<FpsInfoDeathRecipient> fidr = new FpsInfoDeathRecipient(id, this);
        info->linkToDeath(fidr, 0);
        err = mDispatch.regInfo(mDevice, id);
        InfoPackage package = {id, info, fidr};
        {
            std::lock_guard<std::mutex> lock(mClientMutex);
            mInfoPackageList.push_back(package);
        }
    } else {
        ALOGE("register info with a null IFpsInfo");
    }

    return static_cast<Error>(err);
}

Error DfpsHal::unregInfo(uint64_t id)
{
    int32_t err = mDispatch.unregInfo(mDevice, id, DFPS_FLAG_NONE);

    return static_cast<Error>(err);
}

void DfpsHal::unregInfo(uint64_t id, int32_t flag)
{
    mDispatch.unregInfo(mDevice, id, flag);
}

Error DfpsHal::setSingleLayer(int32_t single)
{
    int32_t err = mDispatch.setSingleLayer(mDevice, single);;

    return static_cast<Error>(err);
}

Error DfpsHal::setNumberDisplay(int32_t num)
{
    int32_t err = mDispatch.setNumberDisplay(mDevice, num);;

    return static_cast<Error>(err);
}

Error DfpsHal::setHwcInfo(HwcInfo info)
{
    dfps_hwc_info_t hwcInfo;
    transferHwcInfo(&hwcInfo, info);
    int32_t err = mDispatch.setHwcInfo(mDevice, hwcInfo);

    return static_cast<Error>(err);
}

Error DfpsHal::setInputWindows(hidl_string name, SimpleInputWindowInfo info)
{
    dfps_simple_input_window_info_t simpleInfo;
    transferSimpleInputWindowInfo(&simpleInfo, info);
    int32_t err = mDispatch.setInputWindows(mDevice, name.c_str(), name.size(), simpleInfo);;

    return static_cast<Error>(err);
}

Error DfpsHal::setForegroundInfo(int32_t pid, hidl_string packageName)
{
    int32_t err = mDispatch.setForegroundInfo(mDevice, pid, packageName.c_str(), packageName.size());

    return static_cast<Error>(err);
}

Error DfpsHal::setWindowFlag(int32_t flag, int32_t mask)
{
    int32_t err = mDispatch.setWindowFlag(mDevice, flag, mask);

    return static_cast<Error>(err);
}

Error DfpsHal::enableTracker(int32_t enable)
{
    int32_t err = mDispatch.enableTracker(mDevice, enable);

    return static_cast<Error>(err);
}

Error DfpsHal::forbidAdjustingVsync(int32_t forbid)
{
    int32_t err = mDispatch.forbidAdjustVsync(mDevice, forbid);

    return static_cast<Error>(err);
}

void DfpsHal::transferFpsPolicyInfo(dfps_fps_policy_info_t *dst, FpsPolicyInfo& src)
{
    dst->sequence = src.sequence;
    dst->fps = src.fps;
    dst->pid = src.pid;
    dst->api = src.api;
    dst->mode = src.mode;
    dst->target_pid = src.targetPid;
    dst->gl_context_id = src.glContextId;
    dst->flag = src.flag;
}

void DfpsHal::transferPanelInfo(PanelInfo *dst, dfps_panel_info_t& src)
{
    dst->support120 = src.support_120;
    dst->support90 = src.support_90;
    dst->num = src.num;
}

void DfpsHal::transferFpsRange(FpsRange *dst, dfps_fps_range_t &src)
{
    dst->index = src.index;
    dst->min = src.min_fps;
    dst->max = src.max_fps;
}

void DfpsHal::transferSimpleInputWindowInfo(dfps_simple_input_window_info_t *dst, SimpleInputWindowInfo& src)
{
    dst->pid = src.ownerPid;
}

void DfpsHal::transferHwcInfo(dfps_hwc_info_t *dst, HwcInfo& src)
{
    dst->single_layer = src.singleLayer;
    dst->num_display = src.numDisplay;
}

sp<FpsVsyncClient> DfpsHal::getVsyncClient()
{
    std::lock_guard<std::mutex> lock(mClientMutex);
    return (mClient != nullptr) ? mClient.promote() : nullptr;
}

void DfpsHal::regVsyncClient(const sp<IFpsVsync>& vsync)
{
    sp<FpsVsyncDeathRecipient> fvdr = new FpsVsyncDeathRecipient();
    vsync->linkToDeath(fvdr, 0);
}

void DfpsHal::removeVsyncClient()
{
    std::lock_guard<std::mutex> lock(mClientMutex);
    mClient = nullptr;
}

void DfpsHal::swVsyncChangeHook(dfps_callback_data_t callbackData, int32_t mode, int32_t fps)
{
    auto hal = reinterpret_cast<DfpsHal*>(callbackData);
    auto client = hal->getVsyncClient();
    if (client != nullptr) {
        client->onSwVsyncChange(mode, fps);
    }
}

void DfpsHal::enableCallback(bool enable)
{
    if (enable) {
        mDispatch.registerCallback(mDevice, DFPS_CALLBACK_SW_VSYNC_CHANGE, this, reinterpret_cast<dfps_function_pointer_t>(swVsyncChangeHook));
    } else {
        mDispatch.registerCallback(mDevice, DFPS_CALLBACK_SW_VSYNC_CHANGE, this, nullptr);
    }
}

void DfpsHal::dumpInfo(uint32_t* len, char* string)
{
    mDispatch.dump(mDevice, len, string);
}

void DfpsHal::removeInfoPackage(uint64_t id)
{
    std::lock_guard<std::mutex> lock(mClientMutex);
    for (auto it = mInfoPackageList.begin(); it != mInfoPackageList.end(); ) {
        if ((*it).mId == id) {
            mInfoPackageList.erase(it);
            ALOGE("patio: remove info package: %" PRIx64, id);
            break;
        }
    }
}

void DfpsHal::removePolicyPackage(uint64_t sequence)
{
    std::lock_guard<std::mutex> lock(mClientMutex);
    for (auto it = mPolicyPackageList.begin(); it != mPolicyPackageList.end(); ) {
        if ((*it).mSequence == sequence) {
            mPolicyPackageList.erase(it);
            ALOGE("patio: remove policy package: %" PRIx64, sequence);
            break;
        }
    }
}

IFpsPolicyService* HIDL_FETCH_IFpsPolicyService(const char*)
{
    const hw_module_t* module = nullptr;
    int err = hw_get_module(DFPS_HARDWARE_MODULE_ID, &module);
    if (err) {
        ALOGE("failed to get dfps module");
        return nullptr;
    }

    return new DfpsHal(module);
}

}; // namespace implementation
}; // namespace V1_0
}; // namespace dfps
}; // namespace hardware
}; // namespace mediatek
}; // namespace vendor
