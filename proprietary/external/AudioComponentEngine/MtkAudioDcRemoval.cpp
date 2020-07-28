#define LOG_TAG  "MTKAudioDCRemoval"

#define MTK_LOG_ENABLE 1
#include <cutils/compiler.h>


#include "MtkAudioDcRemoval.h"
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

MtkAudioDcRemove::MtkAudioDcRemove()
    : mHandle(NULL), p_internal_buf(NULL), mSamplerate(0), mBitProcess(DCREMOVE_BIT16)
{
}

MtkAudioDcRemove::~MtkAudioDcRemove()
{
    close();
}

ACE_ERRID MtkAudioDcRemove::init(uint32_t channel, uint32_t samplerate, uint32_t dcrMode, uint32_t dBit)
{
    Mutex::Autolock _l(&mLock);
    unsigned int internal_buf_size_in_bytes = 0;

    DCR_GetBufferSize(&internal_buf_size_in_bytes);
    if (!mHandle)
    {
        if (internal_buf_size_in_bytes > 0)
        {
            p_internal_buf = new signed char[internal_buf_size_in_bytes];
        }
        if (NULL == p_internal_buf)
        {
            ALOGE("[DCR] Fail to new p_internal_buf, required size = %u", internal_buf_size_in_bytes);
            return ACE_NOT_INIT;
        }
        mHandle = DCR_Open(p_internal_buf, channel, samplerate, dcrMode);
    }
    else
    {
        mHandle = DCR_ReConfig(mHandle, channel, samplerate, dcrMode);
    }
    if (!mHandle)
    {
        ALOGE("[DCR] Fail to get DCR Handle");
        if (p_internal_buf != NULL)
        {
            delete[] p_internal_buf;
            p_internal_buf = NULL;
        }
        return ACE_NOT_INIT;
    }
    mBitProcess = dBit;
    mSamplerate = samplerate;
    return ACE_SUCCESS;
}

ACE_ERRID  MtkAudioDcRemove::close(void)
{
    Mutex::Autolock _l(&mLock);
    ALOGV("DcRemove::deinit");
    if (mHandle)
    {
        DCR_Close(mHandle);
        if (p_internal_buf != NULL)
        {
            delete[] p_internal_buf;
            p_internal_buf = NULL;
        }
    }
    return ACE_SUCCESS;
}

ACE_ERRID MtkAudioDcRemove::process(void *pInputBuffer, uint32_t *InputSampleCount, void *pOutputBuffer, uint32_t *OutputSampleCount)
{
    Mutex::Autolock _l(&mLock);
#ifdef ENABLE_DC_REMOVE
    if (mHandle)
    {
        uint32_t inputBufSize  = *InputSampleCount;
        uint32_t outputBufSize = *InputSampleCount;

#ifdef DUMP_DCR_DEBUG
        FILE *pDumpDcrIn;
        AudiocheckAndCreateDirectory("/sdcard/mtklog/audio_dump/before_dcr.pcm");
        pDumpDcrIn = fopen("/sdcard/mtklog/audio_dump/before_dcr.pcm", "ab");
        if (pDumpDcrIn == NULL) { ALOGW("Fail to Open pDumpDcrIn"); }
        fwrite(pInputBuffer, sizeof(int), outputBufSize / sizeof(int), pDumpDcrIn);
        fclose(pDumpDcrIn);
#endif

        if (mBitProcess == DCREMOVE_BIT24)
        {
            if (mSamplerate <= 48000)
            {
                *OutputSampleCount = DCR_Process_24(mHandle, (int *)pInputBuffer, &inputBufSize, (int *)pOutputBuffer, &outputBufSize);
            }
            else
            {
                //ALOGD("DCR_Process_24 High Coef Precision");
                *OutputSampleCount = DCR_Process_24_High_Precision(mHandle, (int *)pInputBuffer, &inputBufSize, (int *)pOutputBuffer, &outputBufSize);
            }
        }
        else
        {
            *OutputSampleCount = DCR_Process(mHandle, (short *)pInputBuffer, &inputBufSize, (short *)pOutputBuffer, &outputBufSize);
        }

        //ALOGD("DcRemove::process inputBufSize = %d,outputBufSize=%d,outputBytes=%d ", inputBufSize, outputBufSize, outputBytes);

#ifdef DUMP_DCR_DEBUG
        FILE *pDumpDcrOut;
        AudiocheckAndCreateDirectory("/sdcard/mtklog/audio_dump/after_dcr.pcm");
        pDumpDcrOut = fopen("/sdcard/mtklog/audio_dump/after_dcr.pcm", "ab");
        if (pDumpDcrOut == NULL) { ALOGW("Fail to Open pDumpDcrOut"); }
        fwrite(pOutputBuffer, sizeof(int), outputBufSize / sizeof(int), pDumpDcrOut);
        fclose(pDumpDcrOut);
#endif
        return ACE_SUCCESS;
    }
    //SXLOGW("DcRemove::process Dcr not initialized");
#endif
    return ACE_NOT_INIT;
}

/*unused function*/

ACE_ERRID MtkAudioDcRemove::setParameter(uint32_t paramID __unused, void *param __unused)
{
    return ACE_SUCCESS;
}
ACE_ERRID MtkAudioDcRemove::getParameter(uint32_t paramID __unused, void *param __unused)
{
    return ACE_SUCCESS;
}
ACE_ERRID MtkAudioDcRemove::open(void)
{
    return ACE_SUCCESS;
}
ACE_ERRID MtkAudioDcRemove::resetBuffer(void)
{
    return ACE_SUCCESS;
}


}


