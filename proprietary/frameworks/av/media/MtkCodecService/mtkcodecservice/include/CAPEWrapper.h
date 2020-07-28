#ifndef CAPEWRAPPER_H
#define CAPEWRAPPER_H

#ifdef MTK_LOG_ENABLE
#undef MTK_LOG_ENABLE
#endif
#define MTK_LOG_ENABLE 1
#include "IMtkCodec.h"
#include "ape_decoder_exp.h"
#include <cutils/log.h>

#include <vendor/mediatek/hardware/mtkcodecservice/1.1/IMtkCodecService.h>
#include <android/hidl/memory/1.0/IMemory.h>
#include <hwbinder/IInterface.h>
#include <hwbinder/IBinder.h>
#include <hidl/Status.h>


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


#define LOGD SLOGD
#define LOGE SLOGE
#define LOGV SLOGV

typedef struct tMTKAPEDecoderExternal
{

    unsigned char       *pInputBuffer;
    int                 inputBufferUsedLength;
    int                 outputFrameSize;
    unsigned char       *pOutputBuffer;

} mtkAPEDecoderExternal;

class CAPEWrapper:public RefBase
{
public:
	CAPEWrapper();
	status_t Init(const apeInitParams& params);
	status_t DeInit();
	status_t DoCodec(const apeDecParams& params_in, apeDecRetParams& params_out,
        const sp<IMemory>& in_mem, sp<IMemory>& out_mem);
	status_t Reset(const apeResetParams& params);
    status_t Create(apeCreateParams& params);
    status_t Destory();
	int32_t getOutSize(){return (int32_t)out_size;}

private:
	ape_decoder_handle apeHandle;
	unsigned int working_BUF_size, in_size, out_size;
	void *pWorking_BUF;
	void *pTempBuff;
	bool pTempBuffEnabled;
	bool bTempBuffFlag;  /// to indicate need copy buffer first after pTempBuffEnabled is enabled.
	unsigned int Tempbuffersize;
	bool mSourceRead;
	mtkAPEDecoderExternal mApeConfig;
	struct ape_decoder_init_param ape_param;
	bool mNewInBufferRequired;
	bool    mNewOutBufRequired; //required
	unsigned char *ptemp;
};

#endif
