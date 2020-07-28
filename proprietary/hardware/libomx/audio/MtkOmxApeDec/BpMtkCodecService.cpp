#define LOG_TAG "BpMtkCodecService"

#include "BpMtkCodecService.h"
#include <cutils/log.h>

namespace android{

//using vendor::mediatek::hardware::mtkcodecservice::V1_1::IMtkCodecService;

const char *gServiceName __unused = "vendor.mediatek.hardware.IMtkCodecService";

BpMtkCodecService::BpMtkCodecService() : mCodecId(0)
{
    ALOGD("BpMtkCodecService ctor");
    service = IMtkCodecService::getService(/*gServiceName*/);
    ALOGD("BpMtkCodecService get getService Done");
    if (service == NULL)
    {
        mSrvReady = false;
        ALOGD("mtk.codecservice is NULL");
    }
    else
    {
        mSrvReady = true;
        ALOGD("mtk.codecservice success");
    }
}

BpMtkCodecService::~BpMtkCodecService()
{
    ALOGD("~BpMtkCodecService");
}

status_t BpMtkCodecService::Create(apeCreateParams &params)
{
    ALOGD("BpMtkCodecService::create");
    if (mSrvReady)
    {
        service->Create([&](const apeCreateParams &param_create){
            memcpy(&params,&param_create,sizeof(apeCreateParams));
        });
        mCodecId = params.codec_id;
        return OK;
    }
    else
    {
        ALOGE("create:service error:%d",DEAD_OBJECT);
        return DEAD_OBJECT;
    }
}

status_t BpMtkCodecService::Destroy()
{
    ALOGD("BpMtkCodecService::Destroy");
    if (mSrvReady)
    {
        service->Destory(GetCodecId());
        {
            return OK;
        }
    }
    else
    {
        ALOGE("destroy:service error:%d",DEAD_OBJECT);
        return DEAD_OBJECT;
    }
}

status_t BpMtkCodecService::Init(apeInitParams &params)
{
    ALOGD("BpMtkCodecService::Init");
    service->Init(params, GetCodecId());
    return OK;
}

status_t BpMtkCodecService::DeInit()
{
    ALOGD("BpMtkCodecService::DeInit");
    service->DeInit(GetCodecId());
    return OK;
}

status_t BpMtkCodecService::DoCodec(apeDecParams &params_in,apeDecRetParams &params_out,
    uint8_t* pOutBuffer)
{
    sp<IMemory> mapped_memory_output;
    service->Decode(params_in, GetCodecId(), [&](const apeDecRetParams &dec_params_out){
        params_out.consumeBS = dec_params_out.consumeBS;
        params_out.outputFrameSize = dec_params_out.outputFrameSize;
        params_out.inputBufUsedLen = dec_params_out.inputBufUsedLen;
        params_out.newInBufRequired = dec_params_out.newInBufRequired;
        params_out.newOutBufRequired = dec_params_out.newOutBufRequired;
        mapped_memory_output = mapMemory(dec_params_out.out_mem);
        uint8_t *data_output = static_cast<uint8_t*>(static_cast<void*>(mapped_memory_output->getPointer()));
        memcpy(pOutBuffer,data_output,params_out.outputFrameSize);
    });

    return OK;
}

status_t BpMtkCodecService::Reset(apeResetParams &params)
{
    ALOGD("BpMtkCodecService::Reset");
    service->Reset(params, GetCodecId());
    return OK;
}


}
