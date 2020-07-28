#define LOG_TAG  "AudioMTKDCRemoval"

#define MTK_LOG_ENABLE 1
#include <cutils/compiler.h>


#include "AudioMTKDcRemoval.h"
#include <cutils/log.h>


#define ENABLE_DC_REMOVE
//#define DUMP_DCR_DEBUG
#ifdef DUMP_DCR_DEBUG
#include <sys/types.h>
#include <sys/stat.h>
int AudiocheckAndCreateDirectory(const char *pC)
{
    char tmp[PATH_MAX];
    int i = 0;
    while (*pC)
    {
        tmp[i] = *pC;
        if (*pC == '/' && i)
        {
            tmp[i] = '\0';
            if (access(tmp, F_OK) != 0)
            {
                if (mkdir(tmp, 0770) == -1)
                {
                    ALOGE("AudioDumpPCM: mkdir error! %s\n", (char *)strerror(errno));
                    return -1;
                }
            }
            tmp[i] = '/';
        }
        i++;
        pC++;
    }
    return 0;
}
#endif   // DUMP_DCR_DEBUG


namespace android
{

DcRemove::DcRemove()
    : mHandle(NULL), p_internal_buf(NULL), mBitProcess(DCREMOVE_BIT16), mSamplerate (0)
{
}
DcRemove::~DcRemove()
{
    close();
}

status_t  DcRemove::init(uint32_t channel, uint32_t samplerate, uint32_t dcrMode, uint32_t dBit)
{
    Mutex::Autolock _l(&mLock);
    unsigned int internal_buf_size_in_bytes = 0;
    DCR_GetBufferSize(&internal_buf_size_in_bytes);
    if (!mHandle)
    {
        p_internal_buf = (signed char *)malloc(internal_buf_size_in_bytes);
        if (NULL == p_internal_buf)
        {
            SLOGW("Fail to malloc p_internal_buf");
            return NO_INIT;
        }
        mHandle = DCR_Open(p_internal_buf, channel, samplerate, dcrMode);
    }
    else
    {
        mHandle = DCR_ReConfig(mHandle, channel, samplerate, dcrMode);
    }
    if (!mHandle)
    {
        SLOGW("Fail to get DCR Handle");
        if(p_internal_buf != NULL)
        {
            free(p_internal_buf);
            p_internal_buf = NULL;
        }
        return NO_INIT;
    }
    mBitProcess = dBit;
    mSamplerate = samplerate;
    return NO_ERROR;
}

status_t  DcRemove::close()
{
    Mutex::Autolock _l(&mLock);
    SLOGV("DcRemove::deinit");
    if (mHandle)
    {
        DCR_Close(mHandle);
        if(p_internal_buf != NULL)
        {
            free(p_internal_buf);
            p_internal_buf = NULL;
        }
    }
    return NO_ERROR;
}

size_t DcRemove::process(const void *inbuffer, size_t bytes, void *outbuffer)
{
    Mutex::Autolock _l(&mLock);
#ifdef ENABLE_DC_REMOVE
    if (mHandle)
    {
        size_t outputBytes = 0;
        uint32_t inputBufSize  = bytes;
        uint32_t outputBufSize = bytes;

#ifdef DUMP_DCR_DEBUG
        FILE *pDumpDcrIn;
        AudiocheckAndCreateDirectory("/sdcard/mtklog/audio_dump/before_dcr.pcm");
        pDumpDcrIn = fopen("/sdcard/mtklog/audio_dump/before_dcr.pcm", "ab");
        if (pDumpDcrIn == NULL) ALOGW("Fail to Open pDumpDcrIn");
        fwrite(inbuffer, sizeof(int), outputBufSize/sizeof(int), pDumpDcrIn);
        fclose(pDumpDcrIn);
#endif

#if 0
#ifdef MTK_HD_AUDIO_ARCHITECTURE
        outputBytes = DCR_Process_24(mHandle, (int *)inbuffer, &inputBufSize, (int *)outbuffer, &outputBufSize);
        //ALOGD("DCR_Process_24");
#else
        outputBytes = DCR_Process(mHandle, (short *)inbuffer, &inputBufSize, (short *)outbuffer, &outputBufSize);
        //ALOGD("DCR_Process");
#endif
#else
        if (mBitProcess == DCREMOVE_BIT24)
        {
            if(mSamplerate <=48000)
            {
                outputBytes = DCR_Process_24(mHandle, (int *)inbuffer, &inputBufSize, (int *)outbuffer, &outputBufSize);
            }
            else
            {
                //ALOGD("DCR_Process_24 High Coef Precision");
                //memcpy(outbuffer, inbuffer, bytes);
                outputBytes = DCR_Process_24_High_Precision(mHandle, (int *)inbuffer, &inputBufSize, (int *)outbuffer, &outputBufSize);
            }
        }
        else
            outputBytes = DCR_Process(mHandle, (short *)inbuffer, &inputBufSize, (short *)outbuffer, &outputBufSize);
#endif
        //ALOGD("DcRemove::process inputBufSize = %d,outputBufSize=%d,outputBytes=%d ", inputBufSize, outputBufSize, outputBytes);

#ifdef DUMP_DCR_DEBUG
        FILE *pDumpDcrOut;
        AudiocheckAndCreateDirectory("/sdcard/mtklog/audio_dump/after_dcr.pcm");
        pDumpDcrOut = fopen("/sdcard/mtklog/audio_dump/after_dcr.pcm", "ab");
        if (pDumpDcrOut == NULL) ALOGW("Fail to Open pDumpDcrOut");
        fwrite(outbuffer, sizeof(int), outputBufSize/sizeof(int), pDumpDcrOut);
        fclose(pDumpDcrOut);
#endif
        return outputBytes;
    }
    //SXLOGW("DcRemove::process Dcr not initialized");
#endif
    return 0;
}

}

