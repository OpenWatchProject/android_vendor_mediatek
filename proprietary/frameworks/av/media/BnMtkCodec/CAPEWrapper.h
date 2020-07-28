#ifndef CAPEWRAPPER_H
#define CAPEWRAPPER_H

#ifdef MTK_LOG_ENABLE
#undef MTK_LOG_ENABLE
#endif
#define MTK_LOG_ENABLE 1
#include "IMtkCodec.h"
#include "ape_decoder_exp.h"
#include <cutils/log.h>

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

class CAPEWrapper:public IMtkCodec
{
public:
	CAPEWrapper();
	status_t Init(const Parcel &para);
	status_t DeInit(const Parcel &para);
	status_t DoCodec(const Parcel &para, Parcel *replay);
	status_t Reset(const Parcel &para);
        status_t Create(const Parcel &para, Parcel *replay);
        status_t Destroy(const Parcel &para);
	
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
