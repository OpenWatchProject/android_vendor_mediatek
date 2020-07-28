#ifndef BP_MTKCODECSERVICE_H_
#define BP_MTKCODECSERVICE_H_

#include <vendor/mediatek/hardware/mtkcodecservice/1.1/IMtkCodecService.h>
#include <hwbinder/Parcel.h>
#include <android/hidl/memory/1.0/IMemory.h>
#include <hwbinder/IInterface.h>
#include <hwbinder/IBinder.h>
#include <hidl/Status.h>
#include <android/hidl/allocator/1.0/IAllocator.h>
#include <hidlmemory/mapping.h>

using vendor::mediatek::hardware::mtkcodecservice::V1_1::IMtkCodecService;
using vendor::mediatek::hardware::mtkcodecservice::V1_1::IMtkCodecServiceCallback;
using vendor::mediatek::hardware::mtkcodecservice::V1_1::apeCreateParams;
using vendor::mediatek::hardware::mtkcodecservice::V1_1::apeInitParams;
using vendor::mediatek::hardware::mtkcodecservice::V1_1::apeDecParams;
using vendor::mediatek::hardware::mtkcodecservice::V1_1::apeDecRetParams;
using vendor::mediatek::hardware::mtkcodecservice::V1_1::apeResetParams;

using ::android::hidl::memory::V1_0::IMemory;
using ::android::status_t;
using ::android::hardware::Parcel;
using ::android::hardware::hidl_memory;

namespace android{


enum OPERATION 
{  
    INIT= 1,
    RESET,
    DEINIT,
    DOCODEC,
    CREATE,
    DESTROY,
    COMMAND
};
enum CODECTYPE
{
    ENCODE,
    DECODE
};


class BpMtkCodecService : public RefBase{
public:
    BpMtkCodecService();

    status_t Create(apeCreateParams &params);
    status_t Destroy();
    status_t Init(apeInitParams &params);
    status_t DeInit();
    status_t DoCodec(apeDecParams &params_in,apeDecRetParams &params_out,uint8_t* pOutBuffer);
    status_t Reset(apeResetParams &params);
    status_t setCallback(IMtkCodecServiceCallback callback __unused) {return OK;}

    bool IsSrvReady(){
        return mSrvReady;
    }

    int32_t GetCodecId() {
        return mCodecId;
    }

protected:
    ~BpMtkCodecService();
private:
    sp<IMtkCodecService> service;
    bool mSrvReady;
    int32_t mCodecId;
};



}


#endif
