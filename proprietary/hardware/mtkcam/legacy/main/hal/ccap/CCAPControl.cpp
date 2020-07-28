#include "CCAPControl.h"

#define LOG_TAG "CCAPControl"

#include <dlfcn.h>
#include <log/log.h>

#include <android/hidl/memory/1.0/IMemory.h>
#include <hidlmemory/mapping.h>

#define MY_LOGI(fmt, arg...)    ALOGI("[%s:%d] " fmt, __FUNCTION__, __LINE__, ##arg)
#define MY_LOGD(fmt, arg...)    ALOGD("[%s:%d] " fmt, __FUNCTION__, __LINE__, ##arg)
#define MY_LOGE(fmt, arg...)    ALOGE("[%s:%d] " fmt, __FUNCTION__, __LINE__, ##arg)

using ::android::hidl::memory::V1_0::IMemory;

#define CCAP_LIB_PATH "libccap.so"
static void *CcapHwhndl = NULL;
static int (*func_ccap_init)() = NULL;
static void (*func_ccap_adb_op)() = NULL;
static void (*func_ccap_atci_op)(const void*, void*, const int) = NULL;
static void (*func_ccap_const)(int*, int*, int*, int*, int*) = NULL;

namespace vendor {
namespace mediatek {
namespace hardware {
namespace camera {
namespace ccap {
namespace V1_0 {
namespace implementation {

// Methods from ::vendor::mediatek::hardware::camera::ccap::V1_0::ICCAPControl follow.
Return<int32_t> CCAPControl::intf_ccap_atci_op(const hidl_memory& req, const hidl_memory& cnf, int32_t cct_op_legacy) {

    MY_LOGI("+");
    (void)cct_op_legacy;
    if(CcapHwhndl == NULL)
    {
        MY_LOGE("CcapHwhndl is null");
        return -1;
    }

    func_ccap_atci_op = (void(*)(const void*, void*, const int)) dlsym(CcapHwhndl, "intf_ccap_atci_op");
    if (func_ccap_atci_op == NULL)
    {
        MY_LOGE("intf_ccap_atci_op is NULL");
        return -1;
    }

    sp<IMemory> memoryReq = mapMemory(req);
    sp<IMemory> memoryCnf = mapMemory(cnf);
    void* pReg = static_cast<void*>(static_cast<void*>(memoryReq->getPointer()));
    void* pCnf = static_cast<void*>(static_cast<void*>(memoryCnf->getPointer()));
    memoryReq->update();
    memoryCnf->update();

    const int* iReq = static_cast<const int*>(pReg);
    for(int i=0; i<30; i+=5)
    {
        MY_LOGI("(%d), (%d), (%d), (%d), (%d)", *(iReq+i), *(iReq+i+1), *(iReq+i+2), *(iReq+i+3), *(iReq+i+4));
    }

    func_ccap_atci_op(pReg, pCnf, -1);

    memoryReq->commit();
    memoryCnf->commit();
    MY_LOGI("-");

    return 0;
}

Return<int32_t> CCAPControl::intf_ccap_init() {

    MY_LOGI("+");
    if(CcapHwhndl == NULL)
    {
        MY_LOGE("CcapHwhndl is null");
        return -1;
    }

    func_ccap_init = (int(*)()) dlsym(CcapHwhndl, "intf_ccap_init");
    if (func_ccap_init == NULL)
    {
        MY_LOGE("intf_ccap_init is NULL");
        return -1;
    }

    if(func_ccap_init() == 0)
    {
        MY_LOGE("func_ccap_init fail");
        return -1;
    };
    MY_LOGI("-");

    return 0;
}

Return<int32_t> CCAPControl::intf_ccap_adb_op() {

    MY_LOGI("+");
    if(CcapHwhndl == NULL)
    {
        MY_LOGE("CcapHwhndl is null");
        return -1;
    }

    func_ccap_adb_op = (void(*)()) dlsym(CcapHwhndl, "intf_ccap_adb_op");
    if (func_ccap_adb_op == NULL)
    {
        MY_LOGE("intf_ccap_adb_op is NULL");
        return -1;
    }

    func_ccap_adb_op();
    MY_LOGI("-");

    return 0;
}

Return<int32_t> CCAPControl::intf_ccap_const(const hidl_memory& constVal) {

    MY_LOGI("+");
    if(CcapHwhndl == NULL)
    {
        CcapHwhndl = dlopen(CCAP_LIB_PATH, RTLD_NOW);
        if(CcapHwhndl == NULL)
        {
            MY_LOGE("dlopen libccap.so failed");
            return -1;
        }
    }

    func_ccap_const = (void(*)(int*, int*, int*, int*, int*)) dlsym(CcapHwhndl, "intf_ccap_const");
    if (func_ccap_const == NULL)
    {
        MY_LOGE("intf_ccap_const is NULL");
        return -1;
    }

    sp<IMemory> memory = mapMemory(constVal);
    if (memory == NULL)
    {
        MY_LOGE("hidl_memory is NULL");
        return -1;
    }
    int32_t* data = static_cast<int32_t*>(static_cast<void*>(memory->getPointer()));
    memory->update();
    // data[0]: sizeof_FT_CCT_REQ, data[1]: sizeof_FT_CCT_CNF, data[2]: FT_CCT_OP_END
    func_ccap_const(data, (data + 1), (data + 2), NULL, NULL);
    memory->commit();

    MY_LOGI("[0]=%d, [1]=%d, [2]=%d -", data[0], data[1], data[2]);

    return 0;
}


// Methods from ::android::hidl::base::V1_0::IBase follow.

ICCAPControl* HIDL_FETCH_ICCAPControl(const char* /* name */) {
    return new CCAPControl();
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace ccap
}  // namespace camera
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor
