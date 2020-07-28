#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "CAPEWrapper"
#endif

#include "CAPEWrapper.h"
//#include <binder/IMemory.h>
#include "OMX_Core.h"
#include <audio_utils/primitives.h>


CAPEWrapper::CAPEWrapper():apeHandle(NULL),working_BUF_size(0),in_size(0),out_size(0),pWorking_BUF(NULL),
                                                        pTempBuff(NULL),pTempBuffEnabled(false),bTempBuffFlag(false),Tempbuffersize(0),
                                                        mSourceRead(false),mNewInBufferRequired(true),mNewOutBufRequired(true),ptemp(NULL)
{
    memset(&mApeConfig, 0, sizeof(mApeConfig));
}

status_t CAPEWrapper::Create(apeCreateParams& params)
{
    ape_decoder_get_mem_size(&in_size, &working_BUF_size, &out_size);
    LOGD("create:in_size:%d,working_BUF_size:%d,out_size:%d",in_size,working_BUF_size,out_size);
    //reply->writeInt32(in_size);
    //reply->writeInt32(out_size);
	params.in_size = in_size;
	params.out_size = out_size;
    ptemp = (unsigned char*)malloc(out_size*sizeof(char));
    if (ptemp == NULL) {
        ALOGE("ptemp malloc failed for size: %u", out_size);
        return NO_MEMORY;
    }
    memset(ptemp, 0, out_size);
    return OK;
}

status_t CAPEWrapper::Destory()
{
    apeHandle = NULL;

    if (pTempBuff != NULL)
    {
        free(pTempBuff);
        pTempBuff = NULL;
    }

    if (pWorking_BUF != NULL)
    {
        free(pWorking_BUF);
        pWorking_BUF = NULL;
    }
    if (ptemp != NULL)
    {
        free(ptemp);
        ptemp = NULL;
    }
    return OK;
}

status_t CAPEWrapper::Init(const apeInitParams& params)
{
    ape_param.blocksperframe = params.blocksperframe;//para.readInt32();
    ape_param.bps = params.bps;//para.readInt32();
    ape_param.channels = params.channels;//para.readInt32();
    ape_param.compressiontype = params.compressiontype;//para.readInt32();
    ape_param.fileversion = params.fileversion;//para.readInt32();
    ape_param.finalframeblocks = params.finalframeblocks;//para.readInt32();
    ape_param.totalframes = params.totalframes;//para.readInt32();
    LOGD("Init:blocksperframe:%d,bps:%d,channels:%d,compressiontype:%d,fileversion:%d,finalframeblocks:%d,totalframes:%d",
        ape_param.blocksperframe,ape_param.bps,ape_param.channels,ape_param.compressiontype,
        ape_param.fileversion,ape_param.finalframeblocks,ape_param.totalframes);

    if (pTempBuff == NULL)
    {
        pTempBuff = (unsigned char *)malloc(in_size*sizeof(char));
        if (pTempBuff == NULL) {
            ALOGE("pTempBuff malloc failed");
            goto malloc_fail;
        }
    }
    if (pWorking_BUF == NULL)
    {
        pWorking_BUF = (unsigned char *)malloc(working_BUF_size*sizeof(char));
        if (pWorking_BUF == NULL) {
            ALOGE("pWorking_BUF malloc failed");
            goto malloc_fail;
        }
    }

    if (apeHandle == NULL)
    {
        apeHandle = ape_decoder_init(pWorking_BUF, &ape_param);
        if (apeHandle == NULL) {
            ALOGE("ape_decoder_init failed");
            return NO_INIT;
        }
    }
    mNewInBufferRequired=mNewOutBufRequired=true;
    return OK;

malloc_fail:
    if (pTempBuff != NULL)
    {
        free(pTempBuff);
        pTempBuff = NULL;
    }

    if (pWorking_BUF != NULL)
    {
        free(pWorking_BUF);
        pWorking_BUF = NULL;
    }
    return NO_MEMORY;
}

status_t CAPEWrapper::DeInit()
{
    return OK;
}

status_t CAPEWrapper::DoCodec(const apeDecParams& params_in, apeDecRetParams& params_out,
        const sp<IMemory>& in_mem, sp<IMemory>& out_mem)
{
//param order: input buf offset, input flag, input buf, output buf
    //input buffer offset
    int inOffset = params_in.inOffset;//para.readInt32();
    //inputbuf flag
    int iflag = params_in.inFlag;//para.readInt32();
    //sp<IMemory> inputmem = interface_cast<IMemory>(para.readStrongBinder());
    //sp<IMemory> outputmem = interface_cast<IMemory>(para.readStrongBinder());
    unsigned char * pinputbuf = static_cast<unsigned char *>(static_cast<void*>(in_mem->getPointer()));//inputmem->pointer();
    int inAlloclen = in_mem->getSize();
    unsigned char * poutputbuf = static_cast<unsigned char *>(static_cast<void*>(out_mem->getPointer()));//outputmem->pointer();
    int32_t consumeBS = APE_ERR_EOS;
    bool decoderEosErr __unused = false;
    LOGV("Docodec+,inOffset:%d,iflag:%d,inAllocLen:%d",inOffset,iflag,inAlloclen);

    if (pTempBuffEnabled == false)
    {
        LOGV("tmpbuf enabled:F");
        if (mSourceRead == true)
        {
            LOGD("buffer mSourceRead done in_offset %d, in_filllen %d", inOffset, inAlloclen);
            mApeConfig.pInputBuffer = (uint8_t *)pTempBuff;
            memset(pTempBuff, 0, in_size);    
        }
        else
        {
            LOGV("cfg.inputbuf=inpubuf+offset:%d",inOffset);    
            mApeConfig.pInputBuffer = (uint8_t *)pinputbuf + inOffset;    
        }
    }
    else
    {
        LOGV("tmpbuf enabled:T");
        mApeConfig.pInputBuffer = (uint8_t *)pTempBuff;
        if (bTempBuffFlag == true)
        {
            //ALOGD("tembufflag:T,Tempbuffersize:%d,in_size:%d", Tempbuffersize, in_size);
			if(in_mem->getPointer() != NULL)
			{
            memcpy((void *)((OMX_U8 *)pTempBuff + Tempbuffersize), pinputbuf, in_size - Tempbuffersize);
            bTempBuffFlag = false;
			}
			else
			{
				return APE_ERR_EOS;
			}
        }
    }
    mApeConfig.pOutputBuffer = poutputbuf;

        if (ape_param.bps == 24)
        {
            if((in_mem->getPointer() != NULL) && (out_mem->getPointer() != NULL))
            {
                consumeBS = ape_decoder_decode(apeHandle, mApeConfig.pInputBuffer, (int *)&mApeConfig.inputBufferUsedLength,
                               ptemp, (int *)&mApeConfig.outputFrameSize);
            }
			else
            {
                consumeBS = APE_ERR_CRC;
            }
            float * fdest = (float *)mApeConfig.pOutputBuffer;
            int numSamples = mApeConfig.outputFrameSize / 3;
            memcpy_to_float_from_p24(fdest, (const uint8_t *)ptemp, numSamples);
        }
        else
		{
			if((in_mem->getPointer() != NULL) && (out_mem->getPointer() != NULL))
            {
	            consumeBS = ape_decoder_decode(apeHandle, mApeConfig.pInputBuffer, (int *)&mApeConfig.inputBufferUsedLength,
	                               mApeConfig.pOutputBuffer, (int *)&mApeConfig.outputFrameSize);
            }
			else
			{
                consumeBS = APE_ERR_CRC ;
			}
        }

    LOGV("decode: pTempBuffEnabled %d, consumeBS %d,in_used %d out_len %d",
           pTempBuffEnabled, consumeBS, mApeConfig.inputBufferUsedLength, mApeConfig.outputFrameSize);
    
//consumeBS should be returned to the MtkOmxApeDec component...>>>
    if (consumeBS == APE_ERR_CRC)
    {
        LOGD("APEDEC_INVALID_Frame CRC ERROR code %d", consumeBS);                    
        mApeConfig.outputFrameSize = 0;
        mNewInBufferRequired = OMX_TRUE;
        memset(mApeConfig.pOutputBuffer, 0, mApeConfig.outputFrameSize);
        mApeConfig.inputBufferUsedLength = 0;    
    }
    else if (consumeBS == APE_ERR_EOS)
    {
        mSourceRead = false;
        mNewInBufferRequired = OMX_FALSE;
        mNewOutBufRequired = OMX_FALSE;
        LOGV("Decode Frame ERROR EOS");
    }
    else
    {
        mNewOutBufRequired = OMX_TRUE;
        if (pTempBuffEnabled == true)
        {
            LOGV("tmpbuf true");
            mNewInBufferRequired = OMX_FALSE;
    //tempbuffer was decoded to end;why inputBufferUsedLength > Tempbuffersize?
            if (mApeConfig.inputBufferUsedLength >= (int)Tempbuffersize) ///&& (mSourceRead == false))
            {
                LOGV("tmpbuf true:1,in used len:%d>temsize:%d",mApeConfig.inputBufferUsedLength,Tempbuffersize);
                mApeConfig.inputBufferUsedLength -= Tempbuffersize;
                pTempBuffEnabled = false;
                Tempbuffersize = 0;
            }
    //tempbuffer suplus
            else
            {
                LOGV("tmpbuf true:2");
                Tempbuffersize -= mApeConfig.inputBufferUsedLength;
                memmove(pTempBuff, (void *)((OMX_U8 *)pTempBuff + mApeConfig.inputBufferUsedLength), Tempbuffersize);
                if (mSourceRead == true)
                    memset((void *)((OMX_U8 *)pTempBuff + Tempbuffersize), 0, in_size - Tempbuffersize);
                else
				{				
                    if(in_mem->getPointer() != NULL)
                    {
                    memcpy((void *)((OMX_U8 *)pTempBuff + Tempbuffersize), pinputbuf + inOffset, in_size - Tempbuffersize);
                    }
					else
                    {
                        return APE_ERR_EOS;
                    }
                }
                mApeConfig.inputBufferUsedLength = 0;
            }
        }
        else
        {
         //input buffer not docodec end,copy suplus data in input buffer to temp buffer
            LOGV("tempbuf false:alloclen:%d,offset:%d,inUsedLen:%d",inAlloclen, inOffset, mApeConfig.inputBufferUsedLength);
            if ((inAlloclen - inOffset - mApeConfig.inputBufferUsedLength) <= (int)in_size)
            {
                LOGV("tempbuf false1");
                pTempBuffEnabled = true;
                bTempBuffFlag = true;
                memset(pTempBuff, 0, in_size);
                
                if ((iflag & OMX_BUFFERFLAG_EOS))
                {
                    mNewInBufferRequired = false;
                    mSourceRead = true;
                }
                else
                {
                    mNewInBufferRequired = true;
                }

                Tempbuffersize = inAlloclen - inOffset - mApeConfig.inputBufferUsedLength;
				if(in_mem->getPointer() != NULL)
				{
                memcpy(pTempBuff, (uint8_t *)(pinputbuf + inOffset + mApeConfig.inputBufferUsedLength), Tempbuffersize);
				}
				else
				{
					return APE_ERR_EOS;
				}
            }
            else
            {
                LOGV("tembuf false:2");
                mNewInBufferRequired = false;
            }
        }
    }

//output param order: consumeBS, outputFrameSize, inputBufferUsedLength, mNewInputBufferRequired, mNewOutBufRequired
    params_out.consumeBS = consumeBS;//replay->writeInt32(consumeBS);
    if (ape_param.bps == 24)
        mApeConfig.outputFrameSize = mApeConfig.outputFrameSize/3 * sizeof (float);
    params_out.outputFrameSize = mApeConfig.outputFrameSize;//replay->writeInt32(mApeConfig.outputFrameSize);
    params_out.inputBufUsedLen = mApeConfig.inputBufferUsedLength;//replay->writeInt32(mApeConfig.inputBufferUsedLength);
    params_out.newInBufRequired = mNewInBufferRequired;//replay->writeInt32(mNewInBufferRequired);
    params_out.newOutBufRequired = mNewOutBufRequired;//replay->writeInt32(mNewOutBufRequired);
    LOGV("Docodec-,consumeBS:%d,outputFrameSize:%d,inputBufferUsedLength:%d,mNewInBufferRequired:%d,mNewOutBufRequired:%d",
        consumeBS,mApeConfig.outputFrameSize,mApeConfig.inputBufferUsedLength,mNewInBufferRequired,mNewOutBufRequired);
    return OK;
}

status_t CAPEWrapper::Reset(const apeResetParams& params)
{
    LOGD("reset+");
    int seekbyte = params.seekbyte;//para.readInt32();
    int newframe = params.newframe;//para.readInt32();
    ape_decoder_reset(apeHandle, seekbyte, newframe);
    pTempBuffEnabled = false;
    bTempBuffFlag = false;
    Tempbuffersize = 0;
    //CR:ALPS01450062
    mSourceRead = false;
    if (ptemp != NULL)
        memset(ptemp, 0, out_size);
    LOGD("reset-");
    return OK;
}

