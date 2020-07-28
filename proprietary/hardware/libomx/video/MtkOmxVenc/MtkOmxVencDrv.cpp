
#include "MtkOmxVenc.h"

#include <cutils/log.h>
#include <utils/Trace.h>

#include <cutils/properties.h>
#include <ui/gralloc_extra.h>
#include <HardwareAPI.h>

#undef LOG_TAG
#define LOG_TAG "MtkOmxVenc"

#include "OMX_IndexExt.h"

#ifdef ATRACE_TAG
#undef ATRACE_TAG
#define ATRACE_TAG ATRACE_TAG_VIDEO
#endif//ATRACE_TAG
#ifdef MTK_DUM_SEC_ENC
/* -------------------------------------------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <uree/system.h>
#include <uree/mem.h>
#include <tz_cross/ta_test.h>
#include <tz_cross/ta_mem.h>
VAL_UINT32_T MtkOmxVenc::Dump_buf_sec_and_normal(OMX_U32 hSec_buf, OMX_U8* pTemp_Buf,OMX_U32 ui4_sz, unsigned  dir)
{
    TZ_RESULT ret;
    UREE_SESSION_HANDLE uree_mem_session;
    UREE_SESSION_HANDLE uree_dump_ta_test;
    UREE_SHAREDMEM_PARAM  shm_param;
    UREE_SHAREDMEM_HANDLE shm_handle_src;
    unsigned int cmd;
    MTEEC_PARAM param[4];

    /* check input parameters */
    if( NULL == pTemp_Buf )
    {
        ALOGE("MtkOmxVenc ERROR: NULL pointer for normal buffer\n");
        return -1;
    }
        /* create memory and mtee img prot inf gen sessions */
    ret = UREE_CreateSession(TZ_TA_MEM_UUID, &uree_mem_session);
    if (ret != TZ_RESULT_SUCCESS)
    {
        ALOGE("MtkOmxVenc ERROR:fail to creat memory session (%s)\n", TZ_GetErrorString(ret));
        return -2;
    }
    ret = UREE_CreateSession(TZ_TA_TEST_UUID, &uree_dump_ta_test);
    if (ret != TZ_RESULT_SUCCESS)
    {
        ALOGE("MtkOmxVenc ERROR: fail to creat test ta session (%s)\n", TZ_GetErrorString(ret));

        ret = UREE_CloseSession(uree_mem_session);
        return -3;
    }

    /* register share memory handles */
    shm_param.buffer = (void *) pTemp_Buf;
    shm_param.size = ui4_sz;
    ret = UREE_RegisterSharedmem(uree_mem_session, &shm_handle_src, &shm_param);
    if (ret != TZ_RESULT_SUCCESS)
    {
        ALOGE("MtkOmxVenc ERROR: fail to register share memory for normal buffer (%s)\n", TZ_GetErrorString(ret));
        ret = UREE_CloseSession(uree_dump_ta_test);
        ret = UREE_CloseSession(uree_mem_session);
        return -4;
    }

    /* perform operation */
    cmd = ( dir == 0 )? TZCMD_TEST_CP_SBUF2NBUF : TZCMD_TEST_CP_NBUF2SBUF;
    param[0].value.a = hSec_buf;
    param[0].value.b = 0;
    param[1].memref.handle = (uint32_t) shm_handle_src;
    param[1].memref.offset = 0;
    param[1].memref.size = ui4_sz;
    param[2].value.a = ui4_sz;
    param[2].value.b = 0;
    ret = UREE_TeeServiceCall( uree_dump_ta_test,
                               cmd,
                               TZ_ParamTypes3(TZPT_VALUE_INPUT, TZPT_MEMREF_INOUT, TZPT_VALUE_INPUT),
                               param );
    if (ret != TZ_RESULT_SUCCESS)
    {
         ALOGE("MtkOmxVenc ERROR: fail to invoke function for test ta (%s)\n", TZ_GetErrorString(ret));
         ret = UREE_UnregisterSharedmem(uree_mem_session, shm_handle_src);
         ret = UREE_CloseSession(uree_dump_ta_test);
         ret = UREE_CloseSession(uree_mem_session);
         return -5;
    }

    /* un-register share memory handles */
    ret = UREE_UnregisterSharedmem(uree_mem_session, shm_handle_src);
    if (ret != TZ_RESULT_SUCCESS)
    {
        ALOGE("MtkOmxVenc ERROR: fail to un-register share memory for normal buffer (%s)\n", TZ_GetErrorString(ret));
        ret = UREE_CloseSession(uree_dump_ta_test);
        ret = UREE_CloseSession(uree_mem_session);
        return -6;
    }

    MTK_OMX_LOGE("MtkOmxVenc :Free sec dump tmp handle\n");
    ret = UREE_CloseSession(uree_dump_ta_test);
    if (ret != TZ_RESULT_SUCCESS)
    {
        ALOGE("MtkOmxVenc ERROR: fail to close test ta session (%d)\n", ret);
        ret = UREE_CloseSession(uree_mem_session);
        return -7;
    }

    MTK_OMX_LOGE("MtkOmxVenc :Free sec dump tmp session\n");
    ret = UREE_CloseSession(uree_mem_session);
    if (ret != TZ_RESULT_SUCCESS)
    {
        ALOGE("MtkOmxVenc ERROR: fail to close memory session (%d)\n", ret);
        return -8;
    }
    return 0;
}
#endif

OMX_BOOL MtkOmxVenc::InitVideoEncodeHW()
{
    VAL_BOOL_T  bRet = VAL_FALSE;
    VENC_DRV_MRESULT_T  rRet = VENC_DRV_MRESULT_OK;

    // query driver capability
    bRet = QueryDriverEnc();
    if (VAL_FALSE == bRet)
    {
        return OMX_FALSE;
    }

    if (mCodecId == MTK_VENC_CODEC_ID_AVC)
    {
        if (checkH264MultiSlice(mInputPortDef.format.video.nFrameWidth, mInputPortDef.format.video.nFrameHeight))
        {
            mCodecId = MTK_VENC_CODEC_ID_AVC_VGA;   // It's confusing, it means AVC w/ multi-slice support
        }
    }
    else if (mCodecId == MTK_VENC_CODEC_ID_MPEG4_SHORT)
    {
        if (OMX_TRUE == IsConstantBitrate())
        {
            mCodecId = MTK_VENC_CODEC_ID_H263_VT;
        }
    }

    // create driver
    rRet = eVEncDrvCreate(&mDrvHandle, drvCodecId());
    if (VENC_DRV_MRESULT_FAIL == rRet)
    {
        MTK_OMX_LOGE("[ERROR] cannot create enc driver");
        return OMX_FALSE;
    }

    VAL_RECORD_SIZE_T   rDrvRecordSize;
    // set record size for emi setting
    if (mInputScalingMode)
    {
        rDrvRecordSize.u4FrmWidth = mScaledWidth;
        rDrvRecordSize.u4FrmHeight = mScaledHeight;
        rDrvRecordSize.u4BufWidth = VENC_ROUND_N(mScaledWidth, 16);
        rDrvRecordSize.u4BufHeight = VENC_ROUND_N(mScaledHeight, 16);
    }
    else if (mRotationType.nRotation == 90 || mRotationType.nRotation == 270 )
    {
        rDrvRecordSize.u4FrmWidth = mInputPortDef.format.video.nFrameHeight;
        rDrvRecordSize.u4FrmHeight = mInputPortDef.format.video.nFrameWidth;
        rDrvRecordSize.u4BufWidth = VENC_ROUND_N(mInputPortDef.format.video.nFrameHeight, 16);
        rDrvRecordSize.u4BufHeight = VENC_ROUND_N( mInputPortDef.format.video.nFrameWidth, 16);
    }
    else
    {
        rDrvRecordSize.u4FrmWidth = mInputPortDef.format.video.nFrameWidth;
        rDrvRecordSize.u4FrmHeight = mInputPortDef.format.video.nFrameHeight;
        rDrvRecordSize.u4BufWidth = mInputPortDef.format.video.nStride;
        rDrvRecordSize.u4BufHeight = mInputPortDef.format.video.nSliceHeight;
    }
    MTK_OMX_LOGD("@@ [MtkOmxVenc::Encode%s] FrameWidth=%lu, FrameHeight=%lu, BufWidth=%lu, BufHeight=%lu\n",
                 codecName(), rDrvRecordSize.u4FrmWidth, rDrvRecordSize.u4FrmHeight,
                 rDrvRecordSize.u4BufWidth, rDrvRecordSize.u4BufHeight);

    rRet = eVEncDrvSetParam(mDrvHandle, VENC_DRV_SET_TYPE_RECORD_SIZE, &rDrvRecordSize, VAL_NULL);
    if (VENC_DRV_MRESULT_FAIL == rRet)
    {
        MTK_OMX_LOGE("[ERROR] set record size for emi setting fail");
    }

    // init driver
    rRet = eVEncDrvInit(mDrvHandle);
    if (VENC_DRV_MRESULT_FAIL == rRet)
    {
        MTK_OMX_LOGE("[ERROR] cannot init enc driver");
        // release driver
        if (VENC_DRV_MRESULT_FAIL == eVEncDrvRelease(mDrvHandle, drvCodecId()))
        {
            MTK_OMX_LOGE("[ERROR] eVEncDrvRelease failed");
        }
        return OMX_FALSE;
    }

    // if Live Photo
    if (mIsLivePhoto)
    {
        VENC_DRV_SCENARIO_T eIn = VENC_DRV_SCENARIO_LIVEPHOTO_EFFECT;
        rRet = eVEncDrvSetParam(mDrvHandle, VENC_DRV_SET_TYPE_SCENARIO, (VAL_VOID_T *)&eIn, VAL_NULL);
        if (VENC_DRV_MRESULT_FAIL == rRet)
        {
            MTK_OMX_LOGE("[ERROR] set venc scenario fail");
        }
    }

    return OMX_TRUE;
}

OMX_BOOL MtkOmxVenc::DeInitVideoEncodeHW()
{
    MTK_OMX_LOGD("+DeInitVideoEncodeHW");

    if (mEncoderInitCompleteFlag == OMX_TRUE)
    {
        //statistic OFF.
        if (VENC_DRV_MRESULT_FAIL == eVEncDrvSetParam(mDrvHandle, VENC_DRV_SET_TYPE_STATISTIC_OFF, VAL_NULL, VAL_NULL))
        {
            MTK_OMX_LOGE("[ERROR] cannot set param");
        }

        VENC_DRV_MRESULT_T mReturn = VENC_DRV_MRESULT_OK;
        OMX_U32 uGetResetBWCEnable = 0;
        mReturn = eVEncDrvGetParam((VAL_HANDLE_T)NULL, VENC_DRV_GET_TYPE_RESET_BWC_ENABLE, (VAL_VOID_T *)&mChipName, (VAL_VOID_T *)&uGetResetBWCEnable);
        MTK_OMX_LOGD("uGetResetBWCEnable%d", uGetResetBWCEnable);

        if (uGetResetBWCEnable)
        {
            VENC_DRV_MRESULT_T  rRet = VENC_DRV_MRESULT_OK;
            VAL_RECORD_SIZE_T   rDrvRecordSize;
            // set record size for emi setting
            rDrvRecordSize.u4FrmWidth = 0;
            rDrvRecordSize.u4FrmHeight = 0;
            rDrvRecordSize.u4BufWidth = 0;
            rDrvRecordSize.u4BufHeight = 0;
            MTK_OMX_LOGD("Rest BWC (%lu, %lu, %lu, %lu)",
                    rDrvRecordSize.u4FrmWidth, rDrvRecordSize.u4FrmHeight,
                    rDrvRecordSize.u4BufWidth, rDrvRecordSize.u4BufHeight);

            rRet = eVEncDrvSetParam(mDrvHandle, VENC_DRV_SET_TYPE_RECORD_SIZE, &rDrvRecordSize, VAL_NULL);
            if (VENC_DRV_MRESULT_FAIL == rRet)
            {
                MTK_OMX_LOGE("[ERROR] set record size for emi setting fail");
            }
        }

        // deinit driver
        if (VENC_DRV_MRESULT_FAIL == eVEncDrvDeInit(mDrvHandle))
        {
            MTK_OMX_LOGE("[ERROR] eVEncDrvDeInit failed");
            return OMX_FALSE;
        }

        // release driver
        if (VENC_DRV_MRESULT_FAIL == eVEncDrvRelease(mDrvHandle, drvCodecId()))
        {
            MTK_OMX_LOGE("[ERROR] eVEncDrvRelease failed");
            return OMX_FALSE;
        }

        mExtensions.onDeInit();

        mDrvHandle = (unsigned int)NULL;
        mEncoderInitCompleteFlag = OMX_FALSE;
    }

    MTK_OMX_LOGD("-DeInitVideoEncodeHW");
    return OMX_TRUE;
}

#ifdef SUPPORT_NATIVE_HANDLE
void MtkOmxVenc::DumpCorruptedMem(char* name, char* startAddr, int size)
{
    if (NULL == startAddr )
    {
        MTK_OMX_LOGE("NULL startAddr for size %d", size);
        return;
    }
    static int fileNumPostFix = 0;
    char buf[255]="";
    snprintf(buf, sizeof(buf), "/sdcard/venc/venc_file_%d_%s_size_%d.h264", fileNumPostFix, name, size);
    FILE *fp = fopen(buf, "wb");
    if (fp)
    {
        MTK_OMX_LOGE("Dump to %s", buf);
        fwrite((void *)startAddr, 1, size, fp);
        fclose(fp);
    }
    else
    {
        MTK_OMX_LOGE("Cannot open %s", buf);
    }
    ++fileNumPostFix;
}
void MtkOmxVenc::checkMemory(char* startAddr, int bufferSize, char* name)
{
    char* ptemp = startAddr;
    int correuptedSize = 0;
    OMX_BOOL corrupting = OMX_FALSE;
    char* myStartAddr = NULL;
    MTK_OMX_LOGE("%s Check start from %p, size %d", name, ptemp, bufferSize);
#if 1
    OMX_BOOL isDirty = OMX_FALSE;
    for (int i = 0; i < bufferSize; ++i)
    {
        if (*ptemp != CHECK_PATTERN)
        {
            isDirty = OMX_TRUE;
            break;
        }
         ++ptemp;
    }
    if (OMX_TRUE == isDirty)
    {
        MTK_OMX_LOGE("Buffer is dirty, dump %d bytes to check", bufferSize);
        DumpCorruptedMem(name, startAddr, bufferSize);
    }
#else
    for (int i = 0; i < bufferSize; ++i)
    {
        if (*ptemp != 0)
        {
            if (OMX_FALSE == corrupting)
            {
                correuptedSize = 1;
                corrupting = OMX_TRUE;
                myStartAddr = ptemp;
            }
            else
            {
                correuptedSize += 1;
            }
        }
        else
        {
            if (OMX_TRUE == corrupting)
            {
                corrupting = OMX_FALSE;
                MTK_OMX_LOGE("Continuous correupted size: %d bytes", correuptedSize);
                DumpCorruptedMem(name, myStartAddr, correuptedSize);
                myStartAddr = NULL;
            }
        }
        ++ptemp;
    }
    if (OMX_TRUE == corrupting)
    {
        MTK_OMX_LOGE("Continuous correupted size: %d bytes", correuptedSize);
        DumpCorruptedMem(name, myStartAddr, correuptedSize);
    }
#endif
    MTK_OMX_LOGE("%s Check end %p", name, ptemp);
    return;
}
void MtkOmxVenc::DumpBitstream(char* name, char* startAddr, int size)
{
    if (NULL == startAddr )
    {
        MTK_OMX_LOGE("NULL startAddr for size %d", size);
        return;
    }
    static int fileNumPostFix = 0;
    char buf[255] = "";
    snprintf(buf, sizeof(buf), "/sdcard/venc_recorded_bs/bs_%d.h264", fileNumPostFix);
    FILE *fp = fopen(buf, "wb");
    if (fp)
    {
        MTK_OMX_LOGD("Dump to %s", buf);
        fwrite((void *)startAddr, 1, size, fp);
        fclose(fp);
    }
    else
    {
        MTK_OMX_LOGE("Cannot open %s", buf);
    }
    ++fileNumPostFix;
}
void MtkOmxVenc::SetBitstreamSize4Framework(
    OMX_BUFFERHEADERTYPE *pOutputBuf,
    OMX_U8 *aOutputBuf,
    OMX_U32 bistreamSize
    )
{
    //pOutputBuf->nFilledLen = 8;
    pOutputBuf->nFilledLen = bistreamSize;
    native_handle_t *handle = (native_handle_t *)(aOutputBuf);
    handle->data[2] = bistreamSize;
#ifdef COPY_2_CONTIG
#ifdef CHECK_OVERFLOW
    checkMemory((char*)handle->data[7], TITAN_BS_SHIFT, "[HEAD]");
    checkMemory((char*)handle->data[7] + TITAN_BS_SHIFT + bistreamSize, 2 * 1024, "[TAIL]");
    checkMemory((char*)handle->data[7] + handle->data[8] - 1 - 512, 512, "[OUTBOUND]");
#endif
    MTK_OMX_LOGD("Copy from %p to %p", handle->data[7], handle->data[3]);
    memcpy((void*)handle->data[3], (void*)(handle->data[7] + TITAN_BS_SHIFT), bistreamSize);
    MTK_OMX_LOGD("Copy done");
#endif
#ifdef CHECK_OVERFLOW
    checkMemory((char*)handle->data[3] + TITAN_BS_SHIFT - 4 * 1024, 4 * 1024, "[HEAD]");
    checkMemory((char*)handle->data[3] + TITAN_BS_SHIFT + bistreamSize + 128, 2 * 1024, "[TAIL]");
    checkMemory((char*)handle->data[3] + handle->data[8] - 1 - 512, 512, "[OUTBOUND]");
    MTK_OMX_LOGD("Copy from %p to %p", handle->data[3] + TITAN_BS_SHIFT, handle->data[3]);
    memcpy((void*)(handle->data[3] + TITAN_BS_SHIFT), (void*)handle->data[3], bistreamSize);
#endif
#ifdef CHECK_OUTPUT_CONSISTENCY
    MTK_OMX_LOGD("Copy %d bytes from %p to %p", bistreamSize, handle->data[3], handle->data[7]);
    memcpy((void*)handle->data[7], (void*)handle->data[3], bistreamSize);
#endif
    if (0 != mRecordBitstream)
    {
        DumpBitstream("dummy", (char*)handle->data[3], bistreamSize);
    }
    if (0 != mWFDLoopbackMode)
    {
        static int fileNumPostFix = 0;
        char buf[255];
        int bufferSize = 256*1024*1024;
        char* dataBuffer = (char*)MTK_OMX_ALLOC(bufferSize);
        snprintf(buf, sizeof(buf), "/sdcard/venc_recorded_bs/bs_%d.h264", fileNumPostFix);
        MTK_OMX_LOGD("[Loopback] copy from pre-dump %s", buf);
        FILE *fp = fopen(buf, "rb");
        if (fp)
        {
            size_t readSize = fread(dataBuffer, 1, bufferSize, fp);
            MTK_OMX_LOGD("[Loopback]readSize %d", readSize);
            handle->data[2] = readSize;
            memcpy((void*)handle->data[3], (void*)dataBuffer, readSize);
            fclose(fp);
            ++fileNumPostFix;
        }
        else
        {
            MTK_OMX_LOGD("[Loopback] Cannot open file %s", buf);
            fileNumPostFix = 0;
        }
        MTK_OMX_FREE(dataBuffer);
    }
    MTK_OMX_LOGD("handle->data[2](bistream size) is %d", handle->data[2]);
}
#endif
//-------- encoding functions --------
void MtkOmxVenc::EncodeFunc(OMX_BUFFERHEADERTYPE *pInputBuf, OMX_BUFFERHEADERTYPE *pOutputBuf)
{
    VAL_BOOL_T  bResolutionChange = OMX_FALSE;
    VAL_BOOL_T  bEncOneUnitTimeProfile = (mOperationRate <= 30 || (mOperationRate > 30 && mFrameCount % 4 == 0) || mEnableMoreLog);
    m_start_tick = 0;
    m_end_tick = 0;

    m_trans_start_tick = 0;
    m_trans_end_tick = 0;

    if (bEncOneUnitTimeProfile) {
        m_start_tick = getTickCountUs();
    }

    ATRACE_CALL();

    //MTK_OMX_LOGD("%s Encode %lld, %d, 0x%08X, input VA=0x%08X :Offset 0x%08lX, output VA=0x%08X :Offset 0x%08lX",
    //codecName(), pInputBuf->nTimeStamp, (int)pInputBuf->nFilledLen, (unsigned int)pInputBuf->nFlags,
    //(unsigned int)(pInputBuf->pBuffer), pInputBuf->nOffset, (unsigned int)(pOutputBuf->pBuffer),
    //pOutputBuf->nOffset);

    if (pInputBuf->nOffset > pInputBuf->nFilledLen) //check the incorrect access
    {
        MTK_OMX_LOGE("[ERROR] incorrect buffer access");
        return;
    }

    VAL_BOOL_T  bRet = VAL_FALSE;
    VENC_DRV_MRESULT_T  rRet = VENC_DRV_MRESULT_OK;
    VENC_DRV_DONE_RESULT_T  rEncResult;
    rEncResult.eMessage = VENC_DRV_MESSAGE_OK;
    rEncResult.fgIsKeyFrm = VAL_FALSE;

    OMX_U8 *aInputBuf   = pInputBuf->pBuffer + pInputBuf->nOffset;
    OMX_U8 *aOutputBuf = pOutputBuf->pBuffer + pOutputBuf->nOffset;
    OMX_U32 aInputSize  = pInputBuf->nFilledLen;
    OMX_U32 aOutputSize = pOutputBuf->nAllocLen;

    if (OMX_FALSE == mEncoderInitCompleteFlag)
    {
        if (mDoConvertPipeline && OMX_TRUE == NeedConversion())
        {
            // pass the empty frame buffer back to convert
            EncHandleEmptyBufferDone(pInputBuf);
        }
        else
        {
            // put input buffer back bufferQ
            QueueBufferAdvance(mpVencInputBufQ, pInputBuf);
        }

        if (mDummyIdx >= 0)
        {
            QueueOutputBuffer(mDummyIdx);//the VencOutBufQ MUST be REAL OutBufQ!!!
            mDummyIdx = -1;
        }

        bRet = InitVideoEncodeHW();
        if (OMX_FALSE == bRet)
        {
            MTK_OMX_LOGE("[ERROR] cannot init encode driver");
            // report bitstream corrupt error
            mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                   mAppData,
                                   OMX_EventError,
                                   OMX_ErrorBadParameter,
                                   (OMX_U32)NULL,
                                   NULL);
            pOutputBuf->nFilledLen = 0;
            pOutputBuf->nTimeStamp = 0;
            HandleFillBufferDone(pOutputBuf);
            return;
        }

        setDrvParamBeforeHdr();

        int iret = checkSecSwitchInEnc(pInputBuf, pOutputBuf);
        if (iret < 0)
        {
            goto ERR_JUMPOUT;
        }
        else if (iret)
        {
            return;
        }

        // assign timestamp
        pOutputBuf->nTimeStamp = pInputBuf->nTimeStamp;

        // get output buffer mva
        GetVEncDrvBSBuffer(aOutputBuf, aOutputSize);

        if (mIsSecureInst)
        {
            MTK_OMX_LOGD("Get BS buffer: va:0x%lx, pa:0x%lx, sec:0x%lx", mBitStreamBuf.rBSAddr.u4VA,
                         mBitStreamBuf.rBSAddr.u4PA, mBitStreamBuf.rSecMemHandle);
        }

        // encode sequence header.
        rRet = eVEncDrvEncode(mDrvHandle, VENC_DRV_START_OPT_ENCODE_SEQUENCE_HEADER,
                              VAL_NULL, &mBitStreamBuf, &rEncResult);
        if (VENC_DRV_MRESULT_FAIL == rRet)
        {
            MTK_OMX_LOGE("[ERROR] cannot encode Sequence Header");
            // report bitstream corrupt error
            mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                   mAppData,
                                   OMX_EventError,
                                   OMX_ErrorStreamCorrupt,
                                   (OMX_U32)NULL,
                                   NULL);
            pOutputBuf->nFilledLen = 0;
            pOutputBuf->nTimeStamp = 0;
            HandleFillBufferDone(pOutputBuf);
            return;
        }

        MTK_OMX_LOGD("Sequence header size = %lu", rEncResult.prBSBuf->u4BSSize);

        postHdrProcess(aOutputBuf, aOutputSize, &rEncResult);

        mHeaderLen = rEncResult.prBSBuf->u4BSSize;
        // Don't show any config info. to upstream
        // Some Tier-1 customers will ask about this.
        if (OMX_FALSE == mStoreMetaDataInOutBuffers)
        {
            pOutputBuf->nFlags |= OMX_BUFFERFLAG_CODECCONFIG;
        }
        if (OMX_TRUE == mStoreMetaDataInOutBuffers) {
#ifdef SUPPORT_NATIVE_HANDLE
            if (OMX_TRUE == mIsAllocateOutputNativeBuffers)
            {
                SetBitstreamSize4Framework(pOutputBuf, aOutputBuf, mHeaderLen);
            }
            else
#endif
            {
            pOutputBuf->nFilledLen = 8 + 4;
            int *bitstreamLen = (int*)(aOutputBuf + sizeof(struct VideoNativeHandleMetadata));
            *bitstreamLen = mHeaderLen;
            }
        } else {
            pOutputBuf->nFilledLen = mHeaderLen;
        }

        if (OMX_TRUE == mIsMultiSlice)
        {
            pOutputBuf->nFlags |= OMX_BUFFERFLAG_MULTISLICE;
        }
        MTK_OMX_LOGD("mIsMultiSlice = %d", mIsMultiSlice);


        dumpOutputBuffer(pOutputBuf, aOutputBuf, aOutputSize);

        HandleFillBufferDone(pOutputBuf);

        mEncoderInitCompleteFlag = OMX_TRUE;
        return;
    }

    if (mBitRateUpdated == OMX_TRUE && mIsViLTE == OMX_TRUE)
    {
        mPrepareToResolutionChange = OMX_TRUE;
    }

    if (mPrepareToResolutionChange == OMX_TRUE)
    {
        MTK_OMX_LOGD_ENG("mPrepareToResolutionChange %d", mPrepareToResolutionChange);

        if(u4EncodeWidth != 0 && u4EncodeHeight != 0)
        {
            if (pInputBuf->nFilledLen != nFilledLen)
            {
                bResolutionChange = OMX_TRUE;
                mPrepareToResolutionChange = OMX_FALSE;
            }
        }
        else
        {
            // resolution change via bitrate change
            VENC_DRV_ADAPTIVE_INFO_FORMAT_T infoIn =
            {
                .u4Width = mEncDrvSetting.u4Width,
                .u4Height = mEncDrvSetting.u4Height,
                .u4MaxWidth = mInputPortDef.format.video.nFrameWidth,
                .u4MaxHeight = mInputPortDef.format.video.nFrameHeight,
                .u4Bitrate = mConfigBitrate.nEncodeBitrate, // new bitrate
                .u4FrameRate = mEncDrvSetting.eFrameRate
            };

            eVEncDrvGetParam((VAL_HANDLE_T)mDrvHandle, VENC_DRV_GET_TYPE_RESOLUTION_CHANGE,
                (VAL_VOID_T *)&infoIn, (VAL_VOID_T *)&bResolutionChange);

            MTK_OMX_LOGD("VENC_DRV_GET_TYPE_RESOLUTION_CHANGE result %d -> %d x %d", bResolutionChange, infoIn.u4Width, infoIn.u4Height);

            if(bResolutionChange == OMX_TRUE)
            {
                u4EncodeWidth = infoIn.u4Width;
                u4EncodeHeight = infoIn.u4Height;
                mEncDrvSetting.eFrameRate = infoIn.u4FrameRate;
            }

            mPrepareToResolutionChange = OMX_FALSE;
        }
    }

    nFilledLen = pInputBuf->nFilledLen;

    // to-do: vilte
    if (bResolutionChange == OMX_TRUE)
    {
        MTK_OMX_LOGD("resolution change to: %d, %d", u4EncodeWidth, u4EncodeHeight);

        // put input buffer back bufferQ
        QueueBufferAdvance(mpVencInputBufQ, pInputBuf);

        mEncDrvSetting.u4Width = u4EncodeWidth;
        mEncDrvSetting.u4Height = u4EncodeHeight;

        u4EncodeWidth = u4EncodeHeight = 0;

        mEncDrvSetting.u4BufWidth = VENC_ROUND_N(mEncDrvSetting.u4Width, 16);
        mEncDrvSetting.u4BufHeight = VENC_ROUND_N(mEncDrvSetting.u4Height, 16);

        if (mConfigBitrate.nEncodeBitrate != MTK_VENC_DEFAULT_OUTPUT_BITRATE)
        {
            mExtraEncDrvSetting.u4BitRate = mConfigBitrate.nEncodeBitrate;
            MTK_OMX_LOGD("re-set bitrate = %d", mExtraEncDrvSetting.u4BitRate);
        }

        if (VENC_DRV_MRESULT_FAIL == eVEncDrvSetParam(mDrvHandle, VENC_DRV_SET_TYPE_PARAM_ENC, &mEncDrvSetting, VAL_NULL))
        {
            MTK_OMX_LOGE("[ERROR] cannot set param");
            return;
        }

        if (mPrependSPSPPSToIDRFrames == OMX_FALSE)
        {
            // encode sequence header.
            rRet = eVEncDrvEncode(mDrvHandle, VENC_DRV_START_OPT_ENCODE_SEQUENCE_HEADER,
                                  VAL_NULL, &mBitStreamBuf, &rEncResult);
            if (VENC_DRV_MRESULT_FAIL == rRet)
            {
                MTK_OMX_LOGE("[ERROR] cannot encode Sequence Header");
                // report bitstream corrupt error
                mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                       mAppData,
                                       OMX_EventError,
                                       OMX_ErrorStreamCorrupt,
                                       (OMX_U32)NULL,
                                       NULL);
                pOutputBuf->nFilledLen = 0;
                pOutputBuf->nTimeStamp = 0;
                HandleFillBufferDone(pOutputBuf);
                return;
            }
            MTK_OMX_LOGD("Sequence header size = %lu", rEncResult.prBSBuf->u4BSSize);

            postHdrProcess(aOutputBuf, aOutputSize, &rEncResult);

            mHeaderLen = rEncResult.prBSBuf->u4BSSize;
            // Don't show any config info. to upstream
            // Some Tier-1 customers will ask about this.
            if (OMX_FALSE == mStoreMetaDataInOutBuffers)
            {
                pOutputBuf->nFlags |= OMX_BUFFERFLAG_CODECCONFIG;
            }
            if (OMX_TRUE == mStoreMetaDataInOutBuffers) {
                pOutputBuf->nFilledLen = 8 + 4;
                int *bitstreamLen = (int*)(aOutputBuf + sizeof(struct VideoNativeHandleMetadata));
                *bitstreamLen = mHeaderLen;
            } else {
                pOutputBuf->nFilledLen = mHeaderLen;
            }

            if (OMX_TRUE == mIsMultiSlice)
            {
                pOutputBuf->nFlags |= OMX_BUFFERFLAG_MULTISLICE;
            }
            MTK_OMX_LOGD("mIsMultiSlice = %d", mIsMultiSlice);

            pOutputBuf->nTimeStamp = pInputBuf->nTimeStamp;

            dumpOutputBuffer(pOutputBuf, aOutputBuf, aOutputSize);

            HandleFillBufferDone(pOutputBuf);
        }
        else
        {
            QueueBufferAdvance(mpVencOutputBufQ, pOutputBuf);
        }

        return;

    }

    if ((pInputBuf->nFlags & OMX_BUFFERFLAG_EOS) && (pInputBuf->nFilledLen == 0) &&
        //note 8167 hybrid will do special handle
        (!((mIsHybridCodec == OMX_TRUE) && mHaveAVCHybridPlatform) ||
            ((mIsHybridCodec == OMX_TRUE) && mHaveAVCHybridPlatform && (mLastFrameBufHdr == NULL)))
       )   // EOS frame
    {
        MTK_OMX_LOGD("Enc EOS received, TS=%lld, nFilledLen %lu", pInputBuf->nTimeStamp, pOutputBuf->nFilledLen);
        pOutputBuf->nFlags |= OMX_BUFFERFLAG_EOS;
        pOutputBuf->nTimeStamp = pInputBuf->nTimeStamp;
        //oirginal : mLastFrameTimeStamp;
        ////update last time stamp for CTS EncodeDecodeTest.java testEncodeDecodeVideoFromBufferToBufferQCIF test
        pOutputBuf->nFilledLen = 0;

        if (mLastFrameBufHdr != NULL)
        {
            EncHandleEmptyBufferDone(mLastFrameBufHdr);
            mLastFrameBufHdr = NULL;
        }

        EncHandleEmptyBufferDone(pInputBuf);
        HandleFillBufferDone(pOutputBuf);
        if (mDummyIdx >= 0)
        {
            QueueOutputBuffer(mDummyIdx);//the VencOutBufQ MUST be REAL OutBufQ!!!
            mDummyIdx = -1;
        }
    }
    else    // encode normal frame
    {
        //added by bin.liu
        //fix cts issue ALPS03040612 of miss last FBD
        OMX_BOOL fgSendEOSForDummy = OMX_FALSE;
        if (mHaveAVCHybridPlatform && (mIsHybridCodec))
        {
            if (pInputBuf->nFlags & OMX_BUFFERFLAG_EOS)
            {
              fgSendEOSForDummy = OMX_TRUE;
              MTK_OMX_LOGD("Enc EOS received, TS=%lld, mDummyIdx %d", pInputBuf->nTimeStamp,mDummyIdx);
            }
            else if (mWFDMode != OMX_TRUE)
            {
                if (mDummyIdx >= 0)
                {
                    QueueOutputBuffer(mDummyIdx);//the VencOutBufQ MUST be REAL OutBufQ!!!
                    mDummyIdx = -1;
                }
            }
        }
        //if switching happen, don't need do things below.
        int iret = checkSecSwitchInEnc(pInputBuf, pOutputBuf);
        if (iret < 0)
        {
            goto ERR_JUMPOUT;
        }
        else if (iret)
        {
            return;
        }

        if (false == mDoConvertPipeline)
        {
            WaitFence(aInputBuf, OMX_TRUE);
        }

        if (OMX_TRUE == NeedConversion() && false == mDoConvertPipeline && mPartNum == 0)
        {
            //do color convert
            ATRACE_BEGIN("ColorConvert");
            m_trans_start_tick = getTickCountMs();
            if (colorConvert(aInputBuf, aInputSize, mCnvtBuffer, mCnvtBufferSize) <= 0)
            {
                MTK_OMX_LOGE("Color Convert fail!!");
            }
            m_trans_end_tick = getTickCountMs();
            ATRACE_END();
        }
        else
        {
            m_trans_start_tick = 0;
            m_trans_end_tick = 0;
        }
        getLatencyToken(pInputBuf, aInputBuf);//for WFD Latency profiling

        if (OMX_TRUE == mStoreMetaDataInBuffers)
        {
            OMX_U8 *aInputBuf = pInputBuf->pBuffer + pInputBuf->nOffset;
            OMX_U32 handle = 0;
            ge_smvr_info_t mQVrInfo;
            int retVal = 0;

            GetMetaHandleFromBufferPtr(aInputBuf, &handle);

            retVal = gralloc_extra_query((buffer_handle_t)handle, GRALLOC_EXTRA_GET_SMVR_INFO, &mQVrInfo);

            // update ts from gralloc
            if(handle != NULL && GRALLOC_EXTRA_OK == retVal) {
                int index = (mQVrInfo.frame_count)? (mFrameCount % mQVrInfo.frame_count) : 0;
                OMX_TICKS currentGeTimeStamp = mQVrInfo.timestamp[0] / 1000UL;
                OMX_TICKS currentCam2FwTimeStamp = mQVrInfo.timestamp_cam2fw / 1000UL;
                OMX_TICKS currentFw2EncTimeStamp = pInputBuf->nTimeStamp;

                pInputBuf->nTimeStamp = currentGeTimeStamp - (currentCam2FwTimeStamp - currentFw2EncTimeStamp);

                MTK_OMX_LOGD_ENG("currentGeTs %llu currentCam2FwTs %llu currentFw2EncTimeStamp%llu videoTs %llu",
                    currentGeTimeStamp, currentCam2FwTimeStamp, currentFw2EncTimeStamp, pInputBuf->nTimeStamp);
            } else {
                MTK_OMX_LOGD("gralloc_extra_query ts fail handle 0x%x retVal %d", handle, retVal);
            }
        }

        // assign timestamp
        if (mIsHybridCodec == OMX_TRUE)
        {
            pOutputBuf->nTimeStamp = mLastFrameTimeStamp;
        }
        else
        {
            pOutputBuf->nTimeStamp = pInputBuf->nTimeStamp;
        }

        GetVEncDrvBSBuffer(aOutputBuf, aOutputSize);
        if (OMX_FALSE == GetVEncDrvFrmBuffer(aInputBuf, aInputSize)){
            goto ERR_JUMPOUT;
        }

        if (mSubFrameTimeStamp != 0 && mOperationRate >= 240)
        {
            if (mSubFrameLastTimeStamp > 0)
            {
                if (mOperationRate > 0)
                {
                    pOutputBuf->nTimeStamp = mLastFrameTimeStamp + (mSubFrameTimeStamp - mSubFrameLastTimeStamp) * (mOperationRate/mEncDrvSetting.eFrameRate);
                }
            }
            else
            {
                if (mOperationRate > 0)
                {
                    pOutputBuf->nTimeStamp = mLastFrameTimeStamp + mSubFrameTimeStamp;
                }
            }
            mSubFrameLastTimeStamp = mSubFrameTimeStamp;
        }

        SetVEncDrvFrmBufferFlag(pInputBuf);

        if (mIsSecureInst)
        {
            MTK_OMX_LOGD_ENG("Get BS buffer: va:0x%lx, pa:0x%lx, sec:0x%lx. Get Frm buffer: va:0x%lx, pa:0x%lx, sec:0x%lx",
                         mBitStreamBuf.rBSAddr.u4VA, mBitStreamBuf.rBSAddr.u4PA,
                         mBitStreamBuf.rSecMemHandle, mFrameBuf.rFrmBufAddr.u4VA,
                         mFrameBuf.rFrmBufAddr.u4PA, mFrameBuf.rSecMemHandle);
        }

        if (mIDRIntervalinSec > 0 && pOutputBuf->nTimeStamp > mLastIDRTimeStamp)
        {
            if (pOutputBuf->nTimeStamp - mLastIDRTimeStamp > (OMX_TICKS)mIDRIntervalinSec * 1000000)
            {
                mForceIFrame = OMX_TRUE;
            }
        }

        preEncProcess();

        setDrvParamBeforeEnc();

        mExtensions.UpdateInputBuffer(pInputBuf, &mFrameBuf);
        mExtensions.UpdateOutputBuffer(pOutputBuf, &mBitStreamBuf);

        dumpInputBuffer(pInputBuf, (OMX_U8 *)mFrameBuf.rFrmBufAddr.u4VA,
                        mEncDrvSetting.u4BufWidth * mEncDrvSetting.u4BufHeight * 3 / 2);

        //for MPEG4
        mFrameBuf.rTimeStamp.u4TimeStamp[0] = (VAL_UINT32_T)(pInputBuf->nTimeStamp >> 32);
        mFrameBuf.rTimeStamp.u4TimeStamp[1] = (VAL_UINT32_T)(pInputBuf->nTimeStamp &  0xFFFFFFFF);

        {
            char value[PROPERTY_VALUE_MAX];
            VAL_BOOL_T bDynamicFPS = 0;
            property_get("vendor.mtk.omx.venc.dynamicfps", value, "1");
            bDynamicFPS = atoi(value);
            if (bDynamicFPS == 1)
                setDynamicFrameRate(pInputBuf);
        }
        if(mHaveAVCHybridPlatform && mIsHybridCodec && (pInputBuf->nFlags & OMX_BUFFERFLAG_TRIGGER_OUTPUT)){
            MTK_OMX_LOGD("OMX_BUFFERFLAG_TRIGGER_OUTPUT, get the last outputbuffer");
            rRet = eVEncDrvEncode(mDrvHandle, VENC_DRV_START_OPT_ENCODE_FINAL, &mFrameBuf, &mBitStreamBuf, &rEncResult);
            //mForceIFrame = OMX_TRUE;
        }else{
            rRet = eVEncDrvEncode(mDrvHandle, VENC_DRV_START_OPT_ENCODE_FRAME, &mFrameBuf, &mBitStreamBuf, &rEncResult);
        }

        // when covert pipeline on, but no need convert,
        // keep input buffer until encode done, then EBD.
        // (input buffer header store in pInputBuf->pMarkData)
        if (mDoConvertPipeline && pInputBuf->pMarkData != 0)
        {
            HandleEmptyBufferDone((OMX_BUFFERHEADERTYPE *)pInputBuf->pMarkData);
        }

        if (VENC_DRV_MRESULT_FAIL == rRet)
        {
            MTK_OMX_LOGE("## ENCODE ERROR !!!");
            // report bitstream corrupt error
            mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                   mAppData,
                                   OMX_EventError,
                                   OMX_ErrorStreamCorrupt,
                                   (OMX_U32)NULL,
                                   NULL);
            goto ERR_JUMPOUT;
        }

        setDrvParamAfterEnc();

        if (mLastTimeStamp == 0)
            mLastTimeStamp = pInputBuf->nTimeStamp;

        if (pInputBuf->nTimeStamp - mLastTimeStamp > 1000000)
        {
            mLastTimeStamp = pInputBuf->nTimeStamp;
            mSlotBitCount = rEncResult.prBSBuf->u4BSSize;
        }
        else
        {
            mSlotBitCount += rEncResult.prBSBuf->u4BSSize;
        }

        //MTK_OMX_LOGD("%s Encode %lld, %d, 0x%08X, input VA=0x%08X :Offset 0x%08lX, output VA=0x%08X :Offset 0x%08lX",
        //codecName(), pInputBuf->nTimeStamp, (int)pInputBuf->nFilledLen, (unsigned int)pInputBuf->nFlags,
        if (bEncOneUnitTimeProfile)
        {
            m_end_tick = getTickCountUs();
            MTK_OMX_LOGD("%s EncTime=%lld (us), RGB_2_YUV=%lld, FrameCount= %d, buf timestamp=%lld (%lld) IsKey(%d), Size(%lu)  Slotbitcount(%d) : "
                         "in VA=0x%08X, offset=0x%08x, len=%d, flags=0x%08x : out VA=0x%08X, offset=0x%08x"
                         "sec:0x%lx, 0x%lx, pending=%d",
                         codecName(), m_end_tick - m_start_tick, m_trans_end_tick - m_trans_start_tick, mFrameCount,
                         pOutputBuf->nTimeStamp / 1000, pOutputBuf->nTimeStamp, rEncResult.fgIsKeyFrm,
                         rEncResult.prBSBuf->u4BSSize, mSlotBitCount*8, (unsigned int)pInputBuf->pBuffer, pInputBuf->nOffset,
                         (int)pInputBuf->nFilledLen, (unsigned int)pInputBuf->nFlags,
                         pOutputBuf->pBuffer, pOutputBuf->nOffset, mBitStreamBuf.rSecMemHandle, mFrameBuf.rSecMemHandle,
                         mEmptyThisBufQ.mPendingNum);
        }

        //pOutputBuf->nFlags |= OMX_BUFFERFLAG_ENDOFFRAME;
        if (rEncResult.eMessage == VENC_DRV_MESSAGE_PARTIAL)
        {
            //MTK_OMX_LOGD("Get partial frame: %d", mPartNum);//do nothing
            ++mPartNum;
        }
        else if (rEncResult.eMessage == VENC_DRV_MESSAGE_TIMEOUT)
        {
            EncHandleEmptyBufferDone(pInputBuf);

            pOutputBuf->nFilledLen = 0;
            pOutputBuf->nOffset = 0;
            pOutputBuf->nFlags = 0;
            QueueBufferAdvance(mpVencOutputBufQ, pOutputBuf);
            if (mDummyIdx >= 0)
            {
                QueueOutputBuffer(mDummyIdx);//the VencOutBufQ MUST be REAL OutBufQ!!!
                mDummyIdx = -1;
            }
            MTK_OMX_LOGD("Enc Time Out");
            //aee_system_warning("264enc",NULL,DB_OPT_DEFAULT,"timeout");
            goto ERR_JUMPOUT;
        }
        else
        {
            //default
            pOutputBuf->nFlags |= OMX_BUFFERFLAG_ENDOFFRAME;
            if (pInputBuf->nFlags & OMX_BUFFERFLAG_EOS)
            {
                if(!((mHaveAVCHybridPlatform) && (mIsHybridCodec))) //fix cts issue ALPS03040612 of miss last FBD
                {
                   pOutputBuf->nFlags |= OMX_BUFFERFLAG_EOS;
                   MTK_OMX_LOGD("Enc EOS received, TS=%lld, nFilledLen %lu", pInputBuf->nTimeStamp, rEncResult.prBSBuf->u4BSSize);
                }
            }
            mPartNum = 0;
            if (mWFDMode == OMX_TRUE && mEnableDummy == OMX_TRUE)
            {
                if (mIsSecureSrc)
                {
                    mSendDummyNAL = false;
                    if (mDummyIdx >= 0)
                    {
                        QueueOutputBuffer(mDummyIdx);//the VencOutBufQ MUST be REAL OutBufQ!!!
                        mDummyIdx = -1;
                    }
                }
                else if (mDummyIdx >= 0)
                    {
                       if(fgSendEOSForDummy)
                       {
                        	mSendDummyNAL = false;
                       }
                       else
                       {
                       mSendDummyNAL = true;
                    }
                }
            }
            pOutputBuf->nTickCount = pInputBuf->nTickCount;
        }

        if (VAL_TRUE == rEncResult.fgIsKeyFrm)
        {
            mLastIDRTimeStamp = pOutputBuf->nTimeStamp;

            pOutputBuf->nFlags |= OMX_BUFFERFLAG_SYNCFRAME;

            if (OMX_TRUE == mForceFullIFrame)
            {
                mForceFullIFrame = OMX_FALSE;

                if (OMX_TRUE == mForceFullIFramePrependHeader && mPrependSPSPPSToIDRFrames == OMX_FALSE)
                {
                    mForceFullIFramePrependHeader = OMX_FALSE;

                    VAL_UINT32_T enable = 0;
                    if (VENC_DRV_MRESULT_FAIL == eVEncDrvSetParam(mDrvHandle, VENC_DRV_SET_TYPE_PREPEND_HEADER, &enable, VAL_NULL))
                    {
                        MTK_OMX_LOGE("[ERROR] set prepend header fail");
                    }
                }
            }
        }

        if (OMX_TRUE == mIsMultiSlice)
        {
            pOutputBuf->nFlags |= OMX_BUFFERFLAG_MULTISLICE;
        }

        if (mIsHybridCodec == OMX_TRUE)
        {
            mLastFrameTimeStamp = pInputBuf->nTimeStamp;
        }
        else
        {
            mLastFrameTimeStamp = pOutputBuf->nTimeStamp;
        }


        if (mStoreMetaDataInOutBuffers) {
#ifdef SUPPORT_NATIVE_HANDLE
            if (OMX_TRUE == mIsAllocateOutputNativeBuffers)
            {
                SetBitstreamSize4Framework(pOutputBuf, aOutputBuf, rEncResult.prBSBuf->u4BSSize);
            }
            else
#endif
            {
            pOutputBuf->nFilledLen = 8 + 4;
            int *bitstreamLen = (int*)(aOutputBuf + sizeof(struct VideoNativeHandleMetadata));
            *bitstreamLen = rEncResult.prBSBuf->u4BSSize;
            }
        } else {
            pOutputBuf->nFilledLen = rEncResult.prBSBuf->u4BSSize;
        }

        dumpOutputBuffer(pOutputBuf, aOutputBuf, aOutputSize);

        mFrameCount++;

        mExtensions.DequeOnGetInputBuffer(pInputBuf, &mFrameBuf);
        mExtensions.DequeOnGetOutputBuffer(pOutputBuf, &mBitStreamBuf);

        if (rEncResult.eMessage == VENC_DRV_MESSAGE_PARTIAL)
        {
            //QueueInputBuffer(findBufferHeaderIndex(MTK_OMX_INPUT_PORT, pInputBuf));
            QueueBufferAdvance(mpVencInputBufQ, pInputBuf);
        }
        else
        {
            //default
            if (mIsHybridCodec == OMX_TRUE)
            {
                if (mIsTimeLapseMode == OMX_FALSE)
                {
                    if (mLastFrameBufHdr != NULL)
                    {
                        EncHandleEmptyBufferDone(mLastFrameBufHdr);
                    }
                    if(mHaveAVCHybridPlatform && mIsHybridCodec && (pInputBuf->nFlags & OMX_BUFFERFLAG_TRIGGER_OUTPUT)){
                        MTK_OMX_LOGD("The mLastFrameBufHdr is NULL");
                        mLastFrameBufHdr = NULL;
                    }else{
                        mLastFrameBufHdr = pInputBuf;
                    }
                }
                else
                {
                    //MTK_OMX_LOGD("@@ -- first time return input (0x%08X)", (unsigned int)pInputBuf);
                    EncHandleEmptyBufferDone(pInputBuf);
                }
            }
            else
            {
                //handle multi frame in a input buffer
                mSubFrameIndex++;

                if (mSubFrameIndex < mSubFrameTotalCount)
                {
                    QueueBufferAdvance(mpVencInputBufQ, pInputBuf);
                }
                else
                {
                    EncHandleEmptyBufferDone(pInputBuf);
                    mSubFrameIndex = 0;
                }
            }
        }
        //HandleEmptyBufferDone(pInputBuf);

        if (mIsHybridCodec == OMX_TRUE && rEncResult.prBSBuf->u4BSSize == 0)
        {
            // only for hybrid
            pOutputBuf->nFilledLen = 0;
            pOutputBuf->nOffset = 0;
            pOutputBuf->nFlags = 0;
            QueueBufferAdvance(mpVencOutputBufQ, pOutputBuf);
            MTK_OMX_LOGD("Enc Empty output \n");
            //need not send dummy when real frame size 0
            if (mDummyIdx >= 0)
            {
                QueueOutputBuffer(mDummyIdx);//the VencOutBufQ MUST be REAL OutBufQ!!!
                mDummyIdx = -1;
            }
        }
        else
        {
            if (mIsHybridCodec && (pOutputBuf->nFlags & OMX_BUFFERFLAG_EOS)) {
                pOutputBuf->nFlags &= (0 ^ OMX_BUFFERFLAG_EOS);
                mMeetHybridEOS = OMX_TRUE;
                MTK_OMX_LOGD("hybrid meet EOS, flag:0x%x", pOutputBuf->nFlags);
            }
            HandleFillBufferDone(pOutputBuf);
        }

        //added by bin.liu
        //fix cts issue ALPS03040612 of miss last FBD
        if(((mHaveAVCHybridPlatform) && (mIsHybridCodec)) && (mDummyIdx >= 0) && fgSendEOSForDummy)
        {
            OMX_BUFFERHEADERTYPE    *pDummyOutputBufHdr = mOutputBufferHdrs[mDummyIdx];
            OMX_U8                  *pDummyOutputBuf = pDummyOutputBufHdr->pBuffer + pDummyOutputBufHdr->nOffset;

            //ATRACE_BEGIN("EncDummyNAL");
            // encode sequence header.
            //GetVEncDrvBSBuffer(pDummyOutputBuf, pDummyOutputBufHdr->nAllocLen);

            pDummyOutputBufHdr->nFlags |= OMX_BUFFERFLAG_EOS;
            pDummyOutputBufHdr->nFilledLen = 0;
            pDummyOutputBufHdr->nOffset = 0;
            HandleFillBufferDone(pDummyOutputBufHdr);
            mDummyIdx = -1;
            MTK_OMX_LOGD("Extra EOS received, TS=%lld, nFilledLen %lu", pInputBuf->nTimeStamp, rEncResult.prBSBuf->u4BSSize);
        }

        //Bruce send a single dummy NAL for reducing HE dongle latency when WFD
        if (mSendDummyNAL == true && mDummyIdx >= 0)
        {
            OMX_BUFFERHEADERTYPE    *pDummyOutputBufHdr = mOutputBufferHdrs[mDummyIdx];
            OMX_U8                  *pDummyOutputBuf = pDummyOutputBufHdr->pBuffer + pDummyOutputBufHdr->nOffset;

            ATRACE_BEGIN("EncDummyNAL");
            // encode sequence header.
            GetVEncDrvBSBuffer(pDummyOutputBuf, pDummyOutputBufHdr->nAllocLen);

            //TODO : h264 only give pps
            if (VENC_DRV_MRESULT_FAIL == eVEncDrvEncode(mDrvHandle,
                                                        VENC_DRV_START_OPT_ENCODE_SEQUENCE_HEADER_H264_PPS,
                                                        VAL_NULL, &mBitStreamBuf, &rEncResult))
            {
                MTK_OMX_LOGE("[ERROR] cannot encode Sequence Header");
                HandleFillBufferDone(pDummyOutputBufHdr);
                mSendDummyNAL = false;
                return;
            }

	     //added for fixed wfd issue.
	     if (mIsHybridCodec == OMX_TRUE)
	     {
			pDummyOutputBufHdr->nTimeStamp  = pOutputBuf->nTimeStamp;
	     }
	     else
	     {
            pDummyOutputBufHdr->nTimeStamp = mLastFrameTimeStamp;
	     }
            pDummyOutputBufHdr->nFlags |= OMX_BUFFERFLAG_ENDOFFRAME;
            pDummyOutputBufHdr->nFlags |= OMX_BUFFERFLAG_DUMMY_NALU;

            if (mStoreMetaDataInOutBuffers) {
#ifdef SUPPORT_NATIVE_HANDLE
                if (OMX_TRUE == mIsAllocateOutputNativeBuffers)
                {
                    SetBitstreamSize4Framework(pOutputBuf, aOutputBuf, rEncResult.prBSBuf->u4BSSize);
                }
                else
#endif
                {
                pDummyOutputBufHdr->nFilledLen = 8 + 4;
                int *bitstreamLen = (int*)(pDummyOutputBuf + sizeof(struct VideoNativeHandleMetadata));
                *bitstreamLen = rEncResult.prBSBuf->u4BSSize;
                }
            } else {
                pDummyOutputBufHdr->nFilledLen = rEncResult.prBSBuf->u4BSSize;
            }
            pDummyOutputBufHdr->nTickCount = pInputBuf->nTickCount;

            HandleFillBufferDone(pDummyOutputBufHdr);
            mDummyIdx = -1;

            mSendDummyNAL = false;
            ATRACE_END();
            return;
        }

        if (mWaitPart == 1 && mPartNum == 0)
        {
            SIGNAL_COND(mPartCond);
        }
    }
    return;
ERR_JUMPOUT:
    if (mDummyIdx >= 0)
    {
        QueueOutputBuffer(mDummyIdx);//the VencOutBufQ MUST be REAL OutBufQ!!!
        mDummyIdx = -1;
    }
    EncHandleEmptyBufferDone(pInputBuf);
    HandleFillBufferDone(pOutputBuf);
    if (mWaitPart == 1 && mPartNum == 0)
    {
        SIGNAL_COND(mPartCond);
    }
    return;
}

void MtkOmxVenc::ConvertVideo(OMX_BUFFERHEADERTYPE *pInputBuf, OMX_BUFFERHEADERTYPE *pOutputBuf)
{
    ATRACE_BEGIN("ColorConvert");
    int64_t _start_tick = 0;
    int64_t _end_tick = 0;

    _start_tick = getTickCountMs();
    MTK_OMX_LOGD_ENG("Convert %lld, %d, 0x%08X, input VA=0x%08X :Offset 0x%08lX, output VA=0x%08X :Offset 0x%08lX",
                 pInputBuf->nTimeStamp, (int)pInputBuf->nFilledLen, (unsigned int)pInputBuf->nFlags,
                 (unsigned int)(pInputBuf->pBuffer), pInputBuf->nOffset, (unsigned int)(pOutputBuf->pBuffer),
                 pOutputBuf->nOffset);

    //check MetaMode input format
    mInputMetaDataFormat = CheckOpaqueFormat(pInputBuf);
    if (0xFFFFFFFF == mGrallocWStride)
    {
        mGrallocWStride = CheckGrallocWStride(pInputBuf);
    }


    if (pInputBuf->nOffset > pInputBuf->nFilledLen) //check the incorrect access
    {
        MTK_OMX_LOGE("[ERROR] incorrect buffer access");
        return;
    }

    VENC_DRV_DONE_RESULT_T  rEncResult;
    rEncResult.eMessage = VENC_DRV_MESSAGE_OK;
    VENC_DRV_PARAM_ENC_EXTRA_T rEncoderExtraConfig;

    OMX_U8 *aInputBuf   = pInputBuf->pBuffer + pInputBuf->nOffset;
    OMX_U8 *aOutputBuf = pOutputBuf->pBuffer + pOutputBuf->nOffset;
    OMX_U32 aInputSize  = pInputBuf->nFilledLen;
    OMX_U32 aOutputSize = pOutputBuf->nAllocLen;
    VAL_RECORD_SIZE_T   rDrvRecordSize;

    if (pInputBuf->nFlags & OMX_BUFFERFLAG_EOS)    // EOS frame
    {
        MTK_OMX_LOGD("Convert EOS received, TS=%lld", pInputBuf->nTimeStamp);
        pOutputBuf->nFlags = pInputBuf->nFlags;
        pOutputBuf->nTimeStamp = mLastFrameTimeStamp;
        pOutputBuf->nFilledLen = 0;

        HandleEmptyBufferDone(pInputBuf);
        CnvtHandleFillBufferDone(pOutputBuf);
    }
    else    // conver frame
    {
        // assign timestamp
        pOutputBuf->nTimeStamp = pInputBuf->nTimeStamp;
        // for SEC VIDEO PATH [
        // check there are secure convert buffers, if no, allocate here.
        if (checkSecSwitchInCC(pInputBuf, pOutputBuf))
        {
            return;
        }
        // ] SEC VIDEO PATH

        WaitFence(aInputBuf, OMX_TRUE);

        if (OMX_TRUE == NeedConversion())
        {
            if (OMX_FALSE == mEncoderInitCompleteFlag)
            {
                MTK_OMX_LOGD("Need Convert but Encoder not init");
                // queue input buffer back inputBufQ
                QueueBufferAdvance(mpConvertInputBufQ, pInputBuf);
                // pass an empty frame buffer to encoder for init
                CnvtHandleFillBufferDone(pOutputBuf);

                sched_yield();
                int i = 0;
                while (OMX_FALSE == mEncoderInitCompleteFlag)
                {
                    if (i++ > 200)
                    {
                        MTK_OMX_LOGE("Wait Encoder init complete timeout");
                        break;
                    }

                    MTK_OMX_LOGD("Wait Encoder init complete...");
                    SLEEP_MS(5);
                }
                return;
            }

            MTK_OMX_LOGD_ENG("Need Convert %lld, %d, 0x%08X, input VA=0x%08X :Offset 0x%08lX, output VA=0x%08X :Offset 0x%08lX",
                 pInputBuf->nTimeStamp, (int)pInputBuf->nFilledLen, (unsigned int)pInputBuf->nFlags,
                 (unsigned int)(pInputBuf->pBuffer), pInputBuf->nOffset, (unsigned int)(pOutputBuf->pBuffer),
                 pOutputBuf->nOffset);

            if (colorConvert(aInputBuf, aInputSize, aOutputBuf, aOutputSize) <= 0)
            {
                MTK_OMX_LOGE("## CONVERT ERROR !!!");
                // report bitstream corrupt error
                mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                       mAppData,
                                       OMX_EventError,
                                       OMX_ErrorStreamCorrupt,
                                       (OMX_U32)NULL,
                                       NULL);
            }

            // assign flag
            pOutputBuf->nFlags = pInputBuf->nFlags;
            pOutputBuf->nFilledLen = pOutputBuf->nAllocLen;
            pOutputBuf->pMarkData = 0;
            _end_tick = getTickCountMs();
            MTK_OMX_LOGD("RGB_2_YUV=%lld, buf timestamp=%lld, s:%d, out:0x%x, 0x%x",
                         _end_tick - _start_tick, pInputBuf->nTimeStamp, mIsSecureSrc,
                         pOutputBuf->pMarkData, pOutputBuf->nFlags);

            mLastFrameTimeStamp = pOutputBuf->nTimeStamp;
            //mFrameCount++;

            HandleEmptyBufferDone(pInputBuf);
            CnvtHandleFillBufferDone(pOutputBuf);
        }
        else//get YV12
        {
            MTK_OMX_LOGD_ENG("No Need Convert %lld, %d, 0x%08X, input VA=0x%08X :Offset 0x%08lX, output VA=0x%08X :Offset 0x%08lX",
                 pInputBuf->nTimeStamp, (int)pInputBuf->nFilledLen, (unsigned int)pInputBuf->nFlags,
                 (unsigned int)(pInputBuf->pBuffer), pInputBuf->nOffset, (unsigned int)(pOutputBuf->pBuffer),
                 pOutputBuf->nOffset);

            pOutputBuf->nFlags = pInputBuf->nFlags;
            pOutputBuf->nFlags |= OMX_BUFFERFLAG_COLORCONVERT_NEEDRETURN;
            pOutputBuf->nFilledLen = 8;
            pOutputBuf->pMarkData = pInputBuf;

            memcpy(aOutputBuf, aInputBuf, 8);
            CnvtHandleFillBufferDone(pOutputBuf);
            //MTK_OMX_LOGD("convert out: 0x%x, 0x%x", pOutputBuf->pMarkData, pOutputBuf->nFlags);
        }
    }
    ATRACE_END();
    return;
}

/* for DirectLink Meta Mode + */
OMX_BOOL MtkOmxVenc::DLMetaModeParseandSendBufInfo(VAL_BufInfo *a_prBufInfo)
{
    VAL_ULONG_T ulFrameWidth = mInputPortDef.format.video.nFrameWidth;
    VAL_ULONG_T ulFrameHeight = mInputPortDef.format.video.nFrameHeight;
    VAL_ULONG_T ulFrameSize = (ulFrameWidth * ulFrameHeight * 3) >> 1;

    if (mSeqHdrEncoded == VAL_FALSE)
    {
        a_prBufInfo->fgIsConfigData = 1;
    }
    else
    {
        a_prBufInfo->fgIsConfigData = 0;
    }

    a_prBufInfo->u4BSVA = mFrameBuf.rFrmBufAddr.u4VA;
    a_prBufInfo->fgBSStatus = *((VAL_UINT8_T *)(mFrameBuf.rFrmBufAddr.u4VA + ulFrameSize - 6));
    a_prBufInfo->fgIsKeyFrame = *((VAL_UINT8_T *)(mFrameBuf.rFrmBufAddr.u4VA + ulFrameSize - 5));
    a_prBufInfo->u4BSSize = *((VAL_UINT32_T *)(mFrameBuf.rFrmBufAddr.u4VA + ulFrameSize - 4));

    MTK_OMX_LOGD("[DLMetaModeParseandSendBufInfo] ConfigData(0x%x), BSVA(0x%x), BSStatus(0x%x), "
                 "IsKeyFrame(0x%x), BSSize(0x%x)"
                 "FrameWidth(%d), FrameHeight(%d), FrameSize(%d)",
                 a_prBufInfo->fgIsConfigData,
                 a_prBufInfo->u4BSVA,
                 a_prBufInfo->fgBSStatus,
                 a_prBufInfo->fgIsKeyFrame,
                 a_prBufInfo->u4BSSize,
                 ulFrameWidth,
                 ulFrameHeight,
                 ulFrameSize
                );

    if (a_prBufInfo->u4BSSize > mBitStreamBuf.rBSAddr.u4Size)
    {
        MTK_OMX_LOGE("[ERROR] [DLMetaModeParseandSendBufInfo] EncodedBSSize(%d) > BSBufSize(%d)",
                     a_prBufInfo->u4BSSize,
                     mBitStreamBuf.rBSAddr.u4Size
                    );

        return OMX_FALSE;
    }

    memcpy((void *)mBitStreamBuf.u4BSStartVA, (void *)a_prBufInfo->u4BSVA, a_prBufInfo->u4BSSize);
    mBitStreamBuf.u4BSSize = a_prBufInfo->u4BSSize;

    return OMX_TRUE;
}

OMX_BOOL MtkOmxVenc::DLMetaModeEnable()
{
    if ((OMX_TRUE == mStoreMetaDataInBuffers) &&
        (((unsigned int)mInputPortDef.format.video.eColorFormat) == OMX_MTK_COLOR_FormatBitStream))
    {
        return OMX_TRUE;
    }
    else
    {
        return OMX_FALSE;
    }
}

void MtkOmxVenc::DLMetaModeEncodeVideo(OMX_BUFFERHEADERTYPE *pInputBuf, OMX_BUFFERHEADERTYPE *pOutputBuf)
{
    int64_t _start_tick = 0;
    int64_t _end_tick = 0;

    _start_tick = getTickCountMs();

    MTK_OMX_LOGD("DLMetaModeEncodeVideo");

    ATRACE_CALL();

    MTK_OMX_LOGD("DL MetaMode %lld, %d, 0x%08X (0x%08X, 0x%08X)",
                 pInputBuf->nTimeStamp, (int)pInputBuf->nFilledLen, (unsigned int)pInputBuf->nFlags,
                 (unsigned int)(pInputBuf->pBuffer), (unsigned int)(pOutputBuf->pBuffer));

    VAL_BufInfo rBufInfo;

    OMX_U8 *aInputBuf = pInputBuf->pBuffer + pInputBuf->nOffset;
    OMX_U8 *aOutputBuf = pOutputBuf->pBuffer + pOutputBuf->nOffset;
    OMX_U32 aInputSize = pInputBuf->nFilledLen;
    OMX_U32 aOutputSize = pOutputBuf->nAllocLen;

    if (OMX_FALSE == mSeqHdrEncoded)
    {
        // assign timestamp
        pOutputBuf->nTimeStamp = pInputBuf->nTimeStamp;

        // get mBitStreamBuf from meta buffer handle
        GetVEncDrvBSBuffer(aOutputBuf, aOutputSize);

        // get mFrameBuf from meta buffer handle
        GetVEncDrvFrmBuffer(aInputBuf, aInputSize);

        DLMetaModeParseandSendBufInfo(&rBufInfo);

        if ((VAL_TRUE != rBufInfo.fgBSStatus) || (VAL_TRUE != rBufInfo.fgIsConfigData))
        {
            MTK_OMX_LOGE("[ERROR] cannot get encoded Sequence Header %d, %d",
                         rBufInfo.fgBSStatus, rBufInfo.fgIsConfigData);

            // report bitstream corrupt error
            mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                   mAppData,
                                   OMX_EventError,
                                   OMX_ErrorStreamCorrupt,
                                   (OMX_U32)NULL,
                                   NULL);

            pOutputBuf->nFilledLen = 0;
            pOutputBuf->nTimeStamp = 0;

            EncHandleEmptyBufferDone(pInputBuf);
            HandleFillBufferDone(pOutputBuf);

            return;
        }

        if (mDumpDLBS)
        {
            MTK_OMX_LOGD("Sequence header = 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x",
                         *((VAL_UINT8_T *)(mBitStreamBuf.u4BSStartVA + 0)),
                         *((VAL_UINT8_T *)(mBitStreamBuf.u4BSStartVA + 1)),
                         *((VAL_UINT8_T *)(mBitStreamBuf.u4BSStartVA + 2)),
                         *((VAL_UINT8_T *)(mBitStreamBuf.u4BSStartVA + 3)),
                         *((VAL_UINT8_T *)(mBitStreamBuf.u4BSStartVA + 4)),
                         *((VAL_UINT8_T *)(mBitStreamBuf.u4BSStartVA + 5)),
                         *((VAL_UINT8_T *)(mBitStreamBuf.u4BSStartVA + 6)),
                         *((VAL_UINT8_T *)(mBitStreamBuf.u4BSStartVA + 7)),
                         *((VAL_UINT8_T *)(mBitStreamBuf.u4BSStartVA + 8)),
                         *((VAL_UINT8_T *)(mBitStreamBuf.u4BSStartVA + 9))
                        );
        }

        MTK_OMX_LOGD("Sequence header size = %d", rBufInfo.u4BSSize);

        pOutputBuf->nFlags |= OMX_BUFFERFLAG_CODECCONFIG;
        pOutputBuf->nFilledLen = rBufInfo.u4BSSize;

#if 1 // dump bs
        {
            char value[PROPERTY_VALUE_MAX];
            VAL_BOOL_T bDumpLog = 0;

            property_get("vendor.mtk.omx.venc.dl.dump.bs", value, "0");
            bDumpLog = atoi(value);

            if (bDumpLog)
            {
                char buf[255];
                snprintf(buf, sizeof(buf), "/sdcard/DLvenc%d.bs", gettid());
                FILE *fp = fopen(buf, "ab");
                if (fp)
                {
                    fwrite((void *)mBitStreamBuf.u4BSStartVA, 1, mBitStreamBuf.u4BSSize, fp);
                    fclose(fp);
                }
            }
        }
#endif

        EncHandleEmptyBufferDone(pInputBuf);
        HandleFillBufferDone(pOutputBuf);

        mSeqHdrEncoded = OMX_TRUE;

        return;
    }

    if (pInputBuf->nFlags & OMX_BUFFERFLAG_EOS)    // EOS frame
    {
        MTK_OMX_LOGD("DL Enc EOS received, TS=%lld", pInputBuf->nTimeStamp);

        pOutputBuf->nFlags |= OMX_BUFFERFLAG_EOS;
        pOutputBuf->nTimeStamp = mLastFrameTimeStamp;
        pOutputBuf->nFilledLen = 0;

        EncHandleEmptyBufferDone(pInputBuf);
        HandleFillBufferDone(pOutputBuf);
    }
    else    // encode normal frame
    {
        // assign timestamp
        pOutputBuf->nTimeStamp = pInputBuf->nTimeStamp;

        // get mBitStreamBuf from meta buffer handle
        GetVEncDrvBSBuffer(aOutputBuf, aOutputSize);

        // get mFrameBuf from meta buffer handle
        GetVEncDrvFrmBuffer(aInputBuf, aInputSize);

        mFrameBuf.rTimeStamp.u4TimeStamp[0] = (VAL_UINT32_T)(pInputBuf->nTimeStamp >> 32);
        mFrameBuf.rTimeStamp.u4TimeStamp[1] = (VAL_UINT32_T)(pInputBuf->nTimeStamp &  0xFFFFFFFF);

        DLMetaModeParseandSendBufInfo(&rBufInfo);

        if (VAL_TRUE != rBufInfo.fgBSStatus)
        {
            MTK_OMX_LOGE("## DL ENCODE ERROR !!!");

            // report bitstream corrupt error
            mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                   mAppData,
                                   OMX_EventError,
                                   OMX_ErrorStreamCorrupt,
                                   (OMX_U32)NULL,
                                   NULL);
        }
        else
        {
            MTK_OMX_LOGD("ENCODE OK IsKey(%d), Size(%d)", rBufInfo.fgIsKeyFrame, rBufInfo.u4BSSize);

            if (mDumpDLBS)
            {
                MTK_OMX_LOGD("DL BS = 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x",
                             *((VAL_UINT8_T *)(mBitStreamBuf.u4BSStartVA + 0)),
                             *((VAL_UINT8_T *)(mBitStreamBuf.u4BSStartVA + 1)),
                             *((VAL_UINT8_T *)(mBitStreamBuf.u4BSStartVA + 2)),
                             *((VAL_UINT8_T *)(mBitStreamBuf.u4BSStartVA + 3)),
                             *((VAL_UINT8_T *)(mBitStreamBuf.u4BSStartVA + 4)),
                             *((VAL_UINT8_T *)(mBitStreamBuf.u4BSStartVA + 5)),
                             *((VAL_UINT8_T *)(mBitStreamBuf.u4BSStartVA + 6)),
                             *((VAL_UINT8_T *)(mBitStreamBuf.u4BSStartVA + 7)),
                             *((VAL_UINT8_T *)(mBitStreamBuf.u4BSStartVA + 8)),
                             *((VAL_UINT8_T *)(mBitStreamBuf.u4BSStartVA + 9))
                            );
            }

#if 1 // dump bs
            {
                char value[PROPERTY_VALUE_MAX];
                VAL_BOOL_T bDumpLog = 0;

                property_get("vendor.mtk.omx.venc.dl.dump.bs", value, "0");
                bDumpLog = atoi(value);

                if (bDumpLog)
                {
                    char buf[255]="";
                    snprintf(buf, sizeof(buf), "/sdcard/DLvenc%d.bs", gettid());
                    FILE *fp = fopen(buf, "ab");
                    if (fp)
                    {
                        fwrite((void *)mBitStreamBuf.u4BSStartVA, 1, mBitStreamBuf.u4BSSize, fp);
                        fclose(fp);
                    }
                }
            }
#endif
        }

        _end_tick = getTickCountMs();
        MTK_OMX_LOGD("EncTime=%lld", _end_tick - _start_tick);

        pOutputBuf->nFlags |= OMX_BUFFERFLAG_ENDOFFRAME;

        if (VAL_TRUE == rBufInfo.fgIsKeyFrame)
        {
            pOutputBuf->nFlags |= OMX_BUFFERFLAG_SYNCFRAME;
        }

        mLastFrameTimeStamp = pOutputBuf->nTimeStamp;

        pOutputBuf->nFilledLen = rBufInfo.u4BSSize;

        VENC_DRV_MRESULT_T mReturn = VENC_DRV_MRESULT_OK;
        OMX_U32 uIsHEVCHW = 0;
        mReturn = eVEncDrvGetParam((VAL_HANDLE_T)NULL, VENC_DRV_GET_TYPE_IS_HEVC_HW_PLATFORM, (VAL_VOID_T *)&mChipName, (VAL_VOID_T *)&uIsHEVCHW);
        //MTK_OMX_LOGD("uIsHEVCHW %d", uIsHEVCHW);
        if ((mCodecId == MTK_VENC_CODEC_ID_HEVC) && uIsHEVCHW)
        {
            //for HEVC HW solution
            pOutputBuf->nOffset = 3;   // skip NAL prefix from HEVC Encoder
            pOutputBuf->nFilledLen -= 3;
        }

        mFrameCount++;

        EncHandleEmptyBufferDone(pInputBuf);
        HandleFillBufferDone(pOutputBuf);

    }

    return;
}
/* for DirectLink Meta Mode - */

//-------- switch case for common code --------
VENC_DRV_VIDEO_FORMAT_T MtkOmxVenc::drvCodecId()
{
    switch (mCodecId)
    {
        case MTK_VENC_CODEC_ID_AVC:
            //if MT6577 define MTK_HYBRID_MULTI_THREAD :
            //if (mInputPortDef.format.video.nFrameWidth * mInputPortDef.format.video.nFrameHeight) > (640 * 480))
            //return VENC_DRV_VIDEO_FORMAT_H264_VGA;
            //MTK_OMX_LOGE("drvCodecId mIsSecureInst %d\n",mIsSecureInst);
            if (OMX_TRUE == mIsSecureInst/* && (OMX_TRUE == mSetWFDMode || OMX_TRUE == mWFDMode)*/)
            {
                MTK_OMX_LOGD("set to sec driver \n");
                if(mTeeEncType == INHOUSE_TEE)
                {
                    if(!mHaveAVCHybridPlatform || mStoreMetaDataInOutBuffers){//only wfd+hdcp mode use inhouse encoder
                        MTK_OMX_LOGE("set to VENC_DRV_VIDEO_FORMAT_INHOUSE_H264SEC driver \n");
                        return VENC_DRV_VIDEO_FORMAT_INHOUSE_H264SEC;
                    }else{
                        MTK_OMX_LOGE("mStoreMetaDataInOutBuffers is 0 set to VENC_DRV_VIDEO_FORMAT_H264 driver \n");
                        return VENC_DRV_VIDEO_FORMAT_H264;
                    }
                }
                else
            {
                return VENC_DRV_VIDEO_FORMAT_H264SEC;
            }
            }
            else
            {
                //MTK_OMX_LOGE("set to normal driver \n");
                return VENC_DRV_VIDEO_FORMAT_H264;
            }
            break;
        case MTK_VENC_CODEC_ID_AVC_VGA:
        {
            return VENC_DRV_VIDEO_FORMAT_H264_VGA;
        }
        break;
        case MTK_VENC_CODEC_ID_MPEG4:
        case MTK_VENC_CODEC_ID_MPEG4_1080P:
            //if MT6577 define MTK_HYBRID_MULTI_THREAD :
            //if (mInputPortDef.format.video.nFrameWidth * mInputPortDef.format.video.nFrameHeight) > (1280* 720))
            //return VENC_DRV_VIDEO_FORMAT_MPEG4_1080P;
            return VENC_DRV_VIDEO_FORMAT_MPEG4;
        case MTK_VENC_CODEC_ID_MPEG4_SHORT:
            return VENC_DRV_VIDEO_FORMAT_H263;
        case MTK_VENC_CODEC_ID_H263_VT:
            return VENC_DRV_VIDEO_FORMAT_H263VT;
        case MTK_VENC_CODEC_ID_HEVC:
            return VENC_DRV_VIDEO_FORMAT_HEVC;
        case MTK_VENC_CODEC_ID_VP8:
            return VENC_DRV_VIDEO_FORMAT_VP8;
        default:
            MTK_OMX_LOGE("unsupported codec %d", mCodecId);
            return VENC_DRV_VIDEO_FORMAT_MAX;
    }
}

const char *MtkOmxVenc::codecName()
{
    switch (mCodecId)
    {
        case MTK_VENC_CODEC_ID_AVC:
        case MTK_VENC_CODEC_ID_AVC_VGA:
            return "AVC";
        case MTK_VENC_CODEC_ID_MPEG4:
        case MTK_VENC_CODEC_ID_MPEG4_SHORT:
        case MTK_VENC_CODEC_ID_MPEG4_1080P:
        case MTK_VENC_CODEC_ID_H263_VT:
            return "MP4";
        case MTK_VENC_CODEC_ID_HEVC:
            return "HEVC";
        case MTK_VENC_CODEC_ID_VP8:
            return "VP8";
        default:
            MTK_OMX_LOGE("unsupported codec %d", mCodecId);
            return "NON";
    }
}

VAL_BOOL_T MtkOmxVenc::QueryDriverEnc()
{
    switch (mCodecId)
    {
        case MTK_VENC_CODEC_ID_AVC:
        case MTK_VENC_CODEC_ID_AVC_VGA:
            return QueryDriverH264Enc();
        case MTK_VENC_CODEC_ID_MPEG4:
        case MTK_VENC_CODEC_ID_MPEG4_SHORT:
        case MTK_VENC_CODEC_ID_MPEG4_1080P:
        case MTK_VENC_CODEC_ID_H263_VT:
            return QueryDriverMPEG4Enc();
        case MTK_VENC_CODEC_ID_HEVC:
            return QueryDriverHEVCEnc();
        case MTK_VENC_CODEC_ID_VP8:
            return QueryDriverVP8Enc();
        default:
            MTK_OMX_LOGE("unsupported codec %d", mCodecId);
            return VAL_FALSE;
    }
}

VAL_BOOL_T MtkOmxVenc::EncSettingEnc()
{
    switch (mCodecId)
    {
        case MTK_VENC_CODEC_ID_AVC:
        case MTK_VENC_CODEC_ID_AVC_VGA:
            return EncSettingH264Enc();
        case MTK_VENC_CODEC_ID_MPEG4:
        case MTK_VENC_CODEC_ID_MPEG4_SHORT:
        case MTK_VENC_CODEC_ID_MPEG4_1080P:
        case MTK_VENC_CODEC_ID_H263_VT:
            return EncSettingMPEG4Enc();
        case MTK_VENC_CODEC_ID_HEVC:
            return EncSettingHEVCEnc();
        case MTK_VENC_CODEC_ID_VP8:
            return EncSettingVP8Enc();
        default:
            MTK_OMX_LOGE("unsupported codec %d", mCodecId);
            return VAL_FALSE;
    }
}

//-------- encode drv query --------
VAL_BOOL_T MtkOmxVenc::QueryDriverMPEG4Enc()
{
    //TODO : refine all Query Function
    //eVideoFormat = drvCodecId();
    //u4Profile = Omx2DriverXXXProfileMap(mMpeg4Type.profile);
    //u4Level = Omx2DriverXXXLevelMap(mMpeg4Type.level);
    //eResolution = VENC_DRV_RESOLUTION_UNKNOWN;
    //u4Width = mInputPortDef.format.video.nFrameWidth;
    //u4Height = mInputPortDef.format.video.nFrameHeight;
    //u4BitRate = mOutputPortDef.format.video.nBitrate; //or mBitrateType.nTargetBitrate?
    VENC_DRV_QUERY_VIDEO_FORMAT_T tQuery;
    VENC_DRV_QUERY_VIDEO_FORMAT_T *pQueryInfo = &tQuery;

    pQueryInfo->eVideoFormat = VENC_DRV_VIDEO_FORMAT_MPEG4;
    pQueryInfo->u4Profile = VENC_DRV_MPEG_VIDEO_PROFILE_MPEG4_SIMPLE;
    pQueryInfo->eLevel = VENC_DRV_VIDEO_LEVEL_3;
    pQueryInfo->eResolution = VENC_DRV_RESOLUTION_UNKNOWN;
    if (mInputScalingMode)
    {
        pQueryInfo->u4Width = mScaledWidth;
        pQueryInfo->u4Height = mScaledHeight;
    }
    else if (mRotationType.nRotation == 90 || mRotationType.nRotation == 270)
    {
        pQueryInfo->u4Width = mInputPortDef.format.video.nFrameHeight;
        pQueryInfo->u4Height = mInputPortDef.format.video.nFrameWidth;
    }
    else
    {
        pQueryInfo->u4Width = mInputPortDef.format.video.nFrameWidth;
        pQueryInfo->u4Height = mInputPortDef.format.video.nFrameHeight;
    }
    pQueryInfo->u4Bitrate = 0; //10Mbps

    if (VENC_DRV_MRESULT_FAIL == eVEncDrvQueryCapability(VENC_DRV_QUERY_TYPE_VIDEO_FORMAT, pQueryInfo, VAL_NULL))
    {
        MTK_OMX_LOGE("[ERROR] cannot support MPEG4 D1 SP encoder !!!");
        return VAL_FALSE;
    }
    MTK_OMX_LOGD("VAL_CHIP_ID: %d", mChipName);
    return VAL_TRUE;
}

VAL_BOOL_T MtkOmxVenc::QueryDriverH264Enc()
{
    VENC_DRV_QUERY_VIDEO_FORMAT_T tQuery;
    VENC_DRV_QUERY_VIDEO_FORMAT_T *pQueryInfo = &tQuery;

    pQueryInfo->eVideoFormat = VENC_DRV_VIDEO_FORMAT_H264;
    pQueryInfo->u4Profile = Omx2DriverH264ProfileMap(mAvcType.eProfile);
    pQueryInfo->eLevel = (VENC_DRV_VIDEO_LEVEL_T)Omx2DriverH264LevelMap(mAvcType.eLevel);
    pQueryInfo->eResolution = VENC_DRV_RESOLUTION_UNKNOWN;
    if (mInputScalingMode)
    {
        pQueryInfo->u4Width = mScaledWidth;
        pQueryInfo->u4Height = mScaledHeight;
    }
    else if (mRotationType.nRotation == 90 || mRotationType.nRotation == 270)
    {
        pQueryInfo->u4Width = mInputPortDef.format.video.nFrameHeight;
        pQueryInfo->u4Height = mInputPortDef.format.video.nFrameWidth;
    }
    else
    {
        pQueryInfo->u4Width = mInputPortDef.format.video.nFrameWidth;
        pQueryInfo->u4Height = mInputPortDef.format.video.nFrameHeight;
    }
    pQueryInfo->u4Bitrate = 0; //2Mbps

    if (VENC_DRV_MRESULT_FAIL == eVEncDrvQueryCapability(VENC_DRV_QUERY_TYPE_VIDEO_FORMAT, pQueryInfo, VAL_NULL))
    {

        VENC_DRV_MRESULT_T mReturn = VENC_DRV_MRESULT_OK;
        OMX_U32 uMpeg4SWPlatform = 0;
        mReturn = eVEncDrvGetParam((VAL_HANDLE_T)NULL, VENC_DRV_GET_TYPE_MPEG4_SW_PLATFORM, (VAL_VOID_T *)&mChipName, (VAL_VOID_T *)&uMpeg4SWPlatform);
        //MTK_OMX_LOGD("uMpeg4SWPlatform %d", uMpeg4SWPlatform);
        MTK_OMX_LOGE("[ERROR] cannot support H.264 (%dx%d) encoder", pQueryInfo->u4Width, pQueryInfo->u4Height);
        //if (VAL_CHIP_NAME_MT6572 == mChipName || VAL_CHIP_NAME_DENALI_2 == mChipName || VAL_CHIP_NAME_MT6580 == mChipName || VAL_CHIP_NAME_MT6570 == mChipName)
        if(uMpeg4SWPlatform)
        {
            // for CTS
            MTK_OMX_LOGE("[ERROR] for low-cost chip, we still support H.264 (%dx%d) encoder", pQueryInfo->u4Width, pQueryInfo->u4Height);
        }
        else
        {
            return VAL_FALSE;
        }
    }

    MTK_OMX_LOGD("VAL_CHIP_ID: %d", mChipName);

    return VAL_TRUE;
}

VAL_BOOL_T MtkOmxVenc::QueryDriverHEVCEnc()
{
    VENC_DRV_QUERY_VIDEO_FORMAT_T tQuery;
    VENC_DRV_QUERY_VIDEO_FORMAT_T *pQueryInfo = &tQuery;

    pQueryInfo->eVideoFormat = VENC_DRV_VIDEO_FORMAT_HEVC;
    pQueryInfo->u4Profile = Omx2DriverHEVCProfileMap(mHevcType.eProfile);
    pQueryInfo->eLevel = (VENC_DRV_VIDEO_LEVEL_T)Omx2DriverHEVCLevelMap(mHevcType.eLevel);
    pQueryInfo->eResolution = VENC_DRV_RESOLUTION_UNKNOWN;
    if (mInputScalingMode)
    {
        pQueryInfo->u4Width = mScaledWidth;
        pQueryInfo->u4Height = mScaledHeight;
    }
    else if (mRotationType.nRotation == 90 || mRotationType.nRotation == 270)
    {
        pQueryInfo->u4Width = mInputPortDef.format.video.nFrameHeight;
        pQueryInfo->u4Height = mInputPortDef.format.video.nFrameWidth;
    }
    else
    {
        pQueryInfo->u4Width = mInputPortDef.format.video.nFrameWidth;
        pQueryInfo->u4Height = mInputPortDef.format.video.nFrameHeight;
    }
    pQueryInfo->u4Bitrate = 0;

    if (VENC_DRV_MRESULT_FAIL == eVEncDrvQueryCapability(VENC_DRV_QUERY_TYPE_VIDEO_FORMAT, pQueryInfo, VAL_NULL))
    {
        MTK_OMX_LOGE("[ERROR] cannot support HEVC encoder");
        return VAL_FALSE;
    }

    MTK_OMX_LOGD("VAL_CHIP_ID: %d", mChipName);

    return VAL_TRUE;
}

VAL_BOOL_T MtkOmxVenc::QueryDriverVP8Enc()
{
    VENC_DRV_QUERY_VIDEO_FORMAT_T tQuery;
    VENC_DRV_QUERY_VIDEO_FORMAT_T *pQueryInfo = &tQuery;

    pQueryInfo->eVideoFormat = VENC_DRV_VIDEO_FORMAT_VP8;
    pQueryInfo->u4Profile = VENC_DRV_MS_VIDEO_PROFILE_MAX;
    pQueryInfo->eLevel = VENC_DRV_VIDEO_LEVEL_MAX;
    pQueryInfo->eResolution = VENC_DRV_RESOLUTION_SUPPORT_1080P;
    if (mInputScalingMode)
    {
        pQueryInfo->u4Width = mScaledWidth;
        pQueryInfo->u4Height = mScaledHeight;
    }
    else if (mRotationType.nRotation == 90 || mRotationType.nRotation == 270)
    {
        pQueryInfo->u4Width = mInputPortDef.format.video.nFrameHeight;
        pQueryInfo->u4Height = mInputPortDef.format.video.nFrameWidth;
    }
    else
    {
        pQueryInfo->u4Width = mInputPortDef.format.video.nFrameWidth;
        pQueryInfo->u4Height = mInputPortDef.format.video.nFrameHeight;
    }
    pQueryInfo->u4Bitrate = 0;

    if (VENC_DRV_MRESULT_FAIL == eVEncDrvQueryCapability(VENC_DRV_QUERY_TYPE_VIDEO_FORMAT, pQueryInfo, VAL_NULL))
    {
        MTK_OMX_LOGE("[ERROR] cannot support VP8 encoder");
        return VAL_FALSE;
    }

    MTK_OMX_LOGD("VAL_CHIP_ID: %d", mChipName);

    return VAL_TRUE;
}

//-------- encode drv setting --------
void MtkOmxVenc::EncSettingDrvResolution(void)
{

    if (mInputScalingMode)
    {
        mEncDrvSetting.u4Width = mScaledWidth;
        mEncDrvSetting.u4Height = mScaledHeight;
    }
    else if (mRotationType.nRotation == 90 || mRotationType.nRotation == 270)
    {
        mEncDrvSetting.u4Width = mInputPortDef.format.video.nFrameHeight;
        mEncDrvSetting.u4Height = mInputPortDef.format.video.nFrameWidth;
    }
    else
    {
        mEncDrvSetting.u4Width = mInputPortDef.format.video.nFrameWidth;
        mEncDrvSetting.u4Height = mInputPortDef.format.video.nFrameHeight;
    }
    //if need color convert we can decide the buffer size, after concider the GPU limitation.
    if (NeedConversion())
    {
        if (mInputScalingMode)
        {
            mEncDrvSetting.u4BufWidth = VENC_ROUND_N(mScaledWidth, 16);
            mEncDrvSetting.u4BufHeight = VENC_ROUND_N(mScaledHeight, 16);
        }
        else if (mRotationType.nRotation == 90 || mRotationType.nRotation == 270)
        {
            mEncDrvSetting.u4BufWidth = VENC_ROUND_N(mInputPortDef.format.video.nFrameHeight, 16);
            mEncDrvSetting.u4BufHeight = VENC_ROUND_N(mInputPortDef.format.video.nFrameWidth, 16);
        }
        else
        {
            mEncDrvSetting.u4BufWidth = VENC_ROUND_N(mInputPortDef.format.video.nFrameWidth, 16);
            mEncDrvSetting.u4BufHeight = VENC_ROUND_N(mInputPortDef.format.video.nFrameHeight, 16);
        }
        //always give even height input to driver
        mEncDrvSetting.u4Height &= (~0x01);
    }
    else
    {
        //if meta mode the stride is decided by handle, not user
        mEncDrvSetting.u4BufWidth = (mStoreMetaDataInBuffers) ? mGrallocWStride : mInputPortDef.format.video.nStride;
        mEncDrvSetting.u4BufHeight = mInputPortDef.format.video.nSliceHeight;
    }

    return;
}

VAL_BOOL_T MtkOmxVenc::EncSettingMPEG4Enc()
{
    VENC_DRV_PARAM_ENC_T           *pmEncDrvSetting;
    VENC_DRV_PARAM_ENC_EXTRA_T      rEncoderExtraConfig;
    OMX_U32                         _frameRate = 0;

    memset(&rEncoderExtraConfig, 0, sizeof(VENC_DRV_PARAM_ENC_EXTRA_T));
    //get default settings.
    if (VENC_DRV_MRESULT_FAIL == eVEncDrvGetParam(mDrvHandle, VENC_DRV_GET_TYPE_PARAM_ENC, VAL_NULL, &mEncDrvSetting))
    {
        MTK_OMX_LOGE("[ERROR] cannot get param");
        return VAL_FALSE;
    }

    VAL_UINT32_T u4PeriodOfIntraFrames = 30;

    //TODO : refine setting
    //u4Profile = Omx2DriverXXXXProfileMap(mMpeg4Type.profile);
    //u4Level = Omx2DriverXXXXProfileMap(mMpeg4Type.level);
    mEncDrvSetting.eVEncFormat = CheckFormatToDrv();

    mExtraEncDrvSetting.u4BitRate = mOutputPortDef.format.video.nBitrate;
    mExtraEncDrvSetting.u4FrameRateQ16 = (mInputPortDef.format.video.xFramerate >> 16);
    mExtraEncDrvSetting.u4IntraFrameRate = u4PeriodOfIntraFrames;

    mEncDrvSetting.u4Profile = VENC_DRV_MPEG_VIDEO_PROFILE_MPEG4_ADVANCED_SIMPLE;
    mEncDrvSetting.u4Level = VENC_DRV_VIDEO_LEVEL_5;
    EncSettingDrvResolution();
    mEncDrvSetting.u4NumPFrm = u4PeriodOfIntraFrames - 1;
    mEncDrvSetting.u4NumBFrm = 0;
    mEncDrvSetting.eFrameRate = (VENC_DRV_FRAME_RATE_T)(mInputPortDef.format.video.xFramerate >> 16);
    mEncDrvSetting.fgInterlace = VAL_FALSE;
    mEncDrvSetting.fgUseMCI = (mIsMCIMode == OMX_TRUE) ? VAL_TRUE : VAL_FALSE;
    mEncDrvSetting.pvExtraEnc = &mExtraEncDrvSetting;

    mExtensions.onInit(&mEncDrvSetting, mDrvHandle);

    if (VENC_DRV_MRESULT_FAIL == eVEncDrvSetParam(mDrvHandle, VENC_DRV_SET_TYPE_PARAM_ENC, &mEncDrvSetting, VAL_NULL))
    {
        MTK_OMX_LOGE("[ERROR] cannot set param");
        return VAL_FALSE;
    }
#ifdef ANDROID_ICS
    if (OMX_TRUE == mIsTimeLapseMode)
    {
        if (VENC_DRV_MRESULT_FAIL == eVEncDrvSetParam(mDrvHandle, VENC_DRV_SET_TYPE_TIME_LAPSE, &mEncDrvSetting, VAL_NULL))
        {
            MTK_OMX_LOGE("[ERROR] cannot set param");
            return VAL_FALSE;
        }
    }
#endif
    VAL_UINT32_T u4InitQP = 4;
    if (OMX_TRUE == mIsWhiteboardEffectMode)
    {
        u4InitQP = 7;
        if (VENC_DRV_MRESULT_FAIL == eVEncDrvSetParam(mDrvHandle, VENC_DRV_SET_TYPE_INIT_QP, &u4InitQP, VAL_NULL))
        {
            MTK_OMX_LOGE("[ERROR] cannot set param");
            return VAL_FALSE;
        }
    }

    MTK_OMX_LOGD("Encoding Drv Setting: Format=%d, Profile=%lu, Level=%lu, Width=%lu, Height=%lu, BufWidth=%lu, "
                 "BufHeight=%lu, NumPFrm=%lu, NumBFrm=%lu, Framerate=%d, Interlace=%d",
                 mEncDrvSetting.eVEncFormat, mEncDrvSetting.u4Profile, mEncDrvSetting.u4Level, mEncDrvSetting.u4Width,
                 mEncDrvSetting.u4Height, mEncDrvSetting.u4BufWidth, mEncDrvSetting.u4BufHeight,
                 mEncDrvSetting.u4NumPFrm, mEncDrvSetting.u4NumBFrm, mEncDrvSetting.eFrameRate,
                 mEncDrvSetting.fgInterlace);

    return VAL_TRUE;
}


VAL_BOOL_T MtkOmxVenc::EncSettingH264Enc()
{
    char value[PROPERTY_VALUE_MAX];
    VAL_UINT32_T u4PeriodOfIntraFrames = 30;

    //get default settings.
    if (VENC_DRV_MRESULT_FAIL == eVEncDrvGetParam(mDrvHandle, VENC_DRV_GET_TYPE_PARAM_ENC, VAL_NULL, &mEncDrvSetting))
    {
        MTK_OMX_LOGE("[ERROR] cannot get param");
        return VAL_FALSE;
    }

    // If the multi-slice is supported, set OMX_BUFFERFLAG_MULTISLICE flag
    mIsMultiSlice = (mEncDrvSetting.fgMultiSlice == VAL_TRUE) ? OMX_TRUE : OMX_FALSE;

    mEncDrvSetting.eVEncFormat = CheckFormatToDrv();

    mExtraEncDrvSetting.u4BitRate = mOutputPortDef.format.video.nBitrate;
    mExtraEncDrvSetting.u4FrameRateQ16 = mInputPortDef.format.video.xFramerate;
    if (mAvcType.nPFrames == 0xffffffff)  //initial value
    {
        mExtraEncDrvSetting.u4IntraFrameRate = mInputPortDef.format.video.xFramerate >> 16 ;
    }
    else if (mAvcType.nPFrames == 0)  //all I farme
    {
        mExtraEncDrvSetting.u4IntraFrameRate = 1;
    }
    else
    {
        mExtraEncDrvSetting.u4IntraFrameRate = mAvcType.nPFrames + 1;
    }
    //mExtraEncDrvSetting.u4IntraFrameRate = u4PeriodOfIntraFrames;

    if (OMX_TRUE == IsConstantBitrate())
    {
        mExtraEncDrvSetting.fgBitrateMode = true; // constant bitrate
    }
    else
    {
        mExtraEncDrvSetting.fgBitrateMode = false;
    }

    if (mIsViLTE)
    {
        property_get("vendor.mtk.venc.i.interval", value, "0");
        mIDRIntervalinSec = atoi(value);

        u4PeriodOfIntraFrames = mExtraEncDrvSetting.u4IntraFrameRate;
        MTK_OMX_LOGD("venc set interval: %d sec, driver I rate : %d framecount", mIDRIntervalinSec, u4PeriodOfIntraFrames);
    }

    mEncDrvSetting.u4Profile = Omx2DriverH264ProfileMap(mAvcType.eProfile);
    mEncDrvSetting.u4Level = Omx2DriverH264LevelMap(mAvcType.eLevel);
    EncSettingDrvResolution();
    mEncDrvSetting.u4NumPFrm = u4PeriodOfIntraFrames - 1;
    mEncDrvSetting.u4NumBFrm = 0;
    mEncDrvSetting.eFrameRate = (VENC_DRV_FRAME_RATE_T)(mInputPortDef.format.video.xFramerate >> 16);
    mEncDrvSetting.fgInterlace = VAL_FALSE;
    mEncDrvSetting.pvExtraEnc = &mExtraEncDrvSetting;
    mEncDrvSetting.fgMBAFF = checkMBAFF();

    mExtensions.onInit(&mEncDrvSetting, mDrvHandle);

    if (VENC_DRV_MRESULT_FAIL == eVEncDrvSetParam(mDrvHandle, VENC_DRV_SET_TYPE_PARAM_ENC, &mEncDrvSetting, VAL_NULL))
    {
        MTK_OMX_LOGE("[ERROR] cannot set param");
        return VAL_FALSE;
    }

// TODO:yxz for hybrid
    VENC_DRV_MRESULT_T mReturn = VENC_DRV_MRESULT_OK;
    OMX_U32 uAVCHybridPlatform = 0;
    mReturn = eVEncDrvGetParam((VAL_HANDLE_T)NULL, VENC_DRV_GET_TYPE_AVC_HYBRID_PLATFORM, (VAL_VOID_T *)&mChipName, (VAL_VOID_T *)&uAVCHybridPlatform);
    MTK_OMX_LOGD_ENG("uAVCHybridPlatform %d", uAVCHybridPlatform);

    if (uAVCHybridPlatform)
    {
        //after ComponentInit, query once for AVC hybrid platform is enough
        mHaveAVCHybridPlatform = OMX_TRUE;
        mIsHybridCodec = OMX_TRUE;
    }

    MTK_OMX_LOGD("Encoding: Format = %d, Profile = %lu, Level = %lu, Width = %lu, Height = %lu, BufWidth = %lu, "
                 "BufHeight = %lu, NumPFrm = %lu, NumBFrm = %lu, Framerate = %d, Interlace = %d"
                 "FrameRateQ16=%d, IntraFrameRate=%d, fgMBAFF=%d",
                 mEncDrvSetting.eVEncFormat, mEncDrvSetting.u4Profile, mEncDrvSetting.u4Level, mEncDrvSetting.u4Width,
                 mEncDrvSetting.u4Height, mEncDrvSetting.u4BufWidth, mEncDrvSetting.u4BufHeight,
                 mEncDrvSetting.u4NumPFrm, mEncDrvSetting.u4NumBFrm, mEncDrvSetting.eFrameRate,
                 mEncDrvSetting.fgInterlace, mExtraEncDrvSetting.u4FrameRateQ16,
                 mExtraEncDrvSetting.u4IntraFrameRate, mEncDrvSetting.fgMBAFF);

    return VAL_TRUE;
}

VAL_BOOL_T MtkOmxVenc::EncSettingHEVCEnc()
{

    //get default settings.
    if (VENC_DRV_MRESULT_FAIL == eVEncDrvGetParam(mDrvHandle, VENC_DRV_GET_TYPE_PARAM_ENC, VAL_NULL, &mEncDrvSetting))
    {
        MTK_OMX_LOGE("[ERROR] cannot get param");
        return VAL_FALSE;
    }

    mEncDrvSetting.eVEncFormat = CheckFormatToDrv();
    mExtraEncDrvSetting.u4BitRate = mOutputPortDef.format.video.nBitrate;
    mExtraEncDrvSetting.u4FrameRateQ16 = (mInputPortDef.format.video.xFramerate >> 16);
    mExtraEncDrvSetting.u4IntraFrameRate = mHevcType.nKeyFrameInterval;
    mExtraEncDrvSetting.fgBitrateMode = mSetConstantBitrateMode;

    mEncDrvSetting.u4Profile = Omx2DriverHEVCProfileMap(mHevcType.eProfile);
    mEncDrvSetting.u4Level = Omx2DriverHEVCLevelMap(mHevcType.eLevel);
    EncSettingDrvResolution();
    mEncDrvSetting.u4NumPFrm = mHevcType.nKeyFrameInterval - 1;
    mEncDrvSetting.u4NumBFrm = 0;
    //added for 8163 software HEVC
    VENC_DRV_MRESULT_T mReturn = VENC_DRV_MRESULT_OK;
    OMX_U32 uIsHEVCSWPlatform = 0;
    mReturn = eVEncDrvGetParam((VAL_HANDLE_T)NULL, VENC_DRV_GET_TYPE_IS_HEVC_SW_SPECIFIC_PLATFORM, (VAL_VOID_T *)&mChipName, (VAL_VOID_T *)&uIsHEVCSWPlatform);
    //MTK_OMX_LOGD("uIsHEVCSWPlatform %d", uIsHEVCSWPlatform);

    if (uIsHEVCSWPlatform)
    {
        mInputPortDef.format.video.xFramerate = VENC_DRV_FRAME_RATE_20 << 16;
    }
    mEncDrvSetting.eFrameRate = (VENC_DRV_FRAME_RATE_T)(mInputPortDef.format.video.xFramerate >> 16);
    mEncDrvSetting.fgInterlace = VAL_FALSE;
    mEncDrvSetting.pvExtraEnc = &mExtraEncDrvSetting;

    mExtensions.onInit(&mEncDrvSetting, mDrvHandle);

    if (VENC_DRV_MRESULT_FAIL == eVEncDrvSetParam(mDrvHandle, VENC_DRV_SET_TYPE_PARAM_ENC, &mEncDrvSetting, VAL_NULL))
    {
        MTK_OMX_LOGE("[ERROR] cannot set param");
        return VAL_FALSE;
    }

    MTK_OMX_LOGD("Encoding: Format = %d, Profile = %lu, Level = %lu, Width = %lu, Height = %lu, BufWidth = %lu, "
                 "BufHeight = %lu, NumPFrm = %lu, NumBFrm = %lu, Framerate = %d, Interlace = %d, "
                 "FrameRateQ16=%d, IntraFrameRate=%d",
                 mEncDrvSetting.eVEncFormat, mEncDrvSetting.u4Profile, mEncDrvSetting.u4Level, mEncDrvSetting.u4Width,
                 mEncDrvSetting.u4Height, mEncDrvSetting.u4BufWidth, mEncDrvSetting.u4BufHeight,
                 mEncDrvSetting.u4NumPFrm, mEncDrvSetting.u4NumBFrm, mEncDrvSetting.eFrameRate,
                 mEncDrvSetting.fgInterlace,mExtraEncDrvSetting.u4FrameRateQ16,mExtraEncDrvSetting.u4IntraFrameRate);

    return VAL_TRUE;
}

VAL_BOOL_T MtkOmxVenc::EncSettingVP8Enc()
{
    VAL_UINT32_T u4PeriodOfIntraFrames = 30;

    //get default settings.
    if (VENC_DRV_MRESULT_FAIL == eVEncDrvGetParam(mDrvHandle, VENC_DRV_GET_TYPE_PARAM_ENC, VAL_NULL, &mEncDrvSetting))
    {
        MTK_OMX_LOGE("[ERROR] cannot get param");
        return VAL_FALSE;
    }

    mEncDrvSetting.eVEncFormat = CheckFormatToDrv();

    mExtraEncDrvSetting.u4BitRate = (mOutputPortDef.format.video.nBitrate / 1000);
    mExtraEncDrvSetting.u4FrameRateQ16 = (mInputPortDef.format.video.xFramerate >> 16);
    mExtraEncDrvSetting.u4IntraFrameRate = u4PeriodOfIntraFrames;

    mEncDrvSetting.u4Profile = VENC_DRV_MS_VIDEO_PROFILE_MAX;
    mEncDrvSetting.u4Level = VENC_DRV_VIDEO_LEVEL_MAX;
    EncSettingDrvResolution();
    mEncDrvSetting.u4NumPFrm = u4PeriodOfIntraFrames - 1;
    mEncDrvSetting.u4NumBFrm = 0;
    mEncDrvSetting.eFrameRate = (VENC_DRV_FRAME_RATE_T)(mInputPortDef.format.video.xFramerate >> 16);
    mEncDrvSetting.fgInterlace = VAL_FALSE;
    mEncDrvSetting.pvExtraEnc = &mExtraEncDrvSetting;

    mExtensions.onInit(&mEncDrvSetting, mDrvHandle);

    if (VENC_DRV_MRESULT_FAIL == eVEncDrvSetParam(mDrvHandle, VENC_DRV_SET_TYPE_PARAM_ENC, &mEncDrvSetting, VAL_NULL))
    {
        MTK_OMX_LOGE("[ERROR][%s] cannot set param", __FUNCTION__);
        return VAL_FALSE;
    }

    MTK_OMX_LOGD("Encoding: Format = %d, Profile = %lu, Level = %lu, Width = %lu, Height = %lu, BufWidth = %lu, "
                 "BufHeight = %lu, NumPFrm = %lu, NumBFrm = %lu, Framerate = %d, Interlace = %d",
                 mEncDrvSetting.eVEncFormat, mEncDrvSetting.u4Profile, mEncDrvSetting.u4Level, mEncDrvSetting.u4Width,
                 mEncDrvSetting.u4Height, mEncDrvSetting.u4BufWidth, mEncDrvSetting.u4BufHeight,
                 mEncDrvSetting.u4NumPFrm, mEncDrvSetting.u4NumBFrm, mEncDrvSetting.eFrameRate,
                 mEncDrvSetting.fgInterlace);

    return VAL_TRUE;
}

//-------- map omx profile & level to drv profile & level --------
VAL_UINT32_T MtkOmxVenc::Omx2DriverH264ProfileMap(OMX_VIDEO_AVCPROFILETYPE eProfile)
{
    switch (eProfile)
    {
        case OMX_VIDEO_AVCProfileBaseline:
            return VENC_DRV_H264_VIDEO_PROFILE_BASELINE;
            break;
        case OMX_VIDEO_AVCProfileMain:
            return VENC_DRV_H264_VIDEO_PROFILE_MAIN;
            break;
        case OMX_VIDEO_AVCProfileExtended:
            return VENC_DRV_H264_VIDEO_PROFILE_EXTENDED;
            break;
        case OMX_VIDEO_AVCProfileHigh:
            return VENC_DRV_H264_VIDEO_PROFILE_HIGH;
            break;
        case OMX_VIDEO_AVCProfileHigh10:
            return VENC_DRV_H264_VIDEO_PROFILE_HIGH_10;
            break;
        case OMX_VIDEO_AVCProfileHigh422:
            return VENC_DRV_H264_VIDEO_PROFILE_HIGH422;
            break;
        case OMX_VIDEO_AVCProfileHigh444:
            return VENC_DRV_H264_VIDEO_PROFILE_HIGH444;
            break;
        default:
            MTK_OMX_LOGE("[ERROR] Unsupported H264 Profile");
            return VENC_DRV_H264_VIDEO_PROFILE_BASELINE;
            break;
    }
}

VAL_UINT32_T MtkOmxVenc::Omx2DriverH264LevelMap(OMX_VIDEO_AVCLEVELTYPE eLevel)
{
    switch (eLevel)
    {
        case OMX_VIDEO_AVCLevel1:
            return VENC_DRV_VIDEO_LEVEL_1;
            break;
        case OMX_VIDEO_AVCLevel1b:
            return VENC_DRV_VIDEO_LEVEL_1b;
            break;
        case OMX_VIDEO_AVCLevel11:
            return VENC_DRV_VIDEO_LEVEL_1_1;
            break;
        case OMX_VIDEO_AVCLevel12:
            return VENC_DRV_VIDEO_LEVEL_1_2;
            break;
        case OMX_VIDEO_AVCLevel13:
            return VENC_DRV_VIDEO_LEVEL_1_3;
            break;
        case OMX_VIDEO_AVCLevel2:
            return VENC_DRV_VIDEO_LEVEL_2;
            break;
        case OMX_VIDEO_AVCLevel21:
            return VENC_DRV_VIDEO_LEVEL_2_1;
            break;
        case OMX_VIDEO_AVCLevel22:
            return VENC_DRV_VIDEO_LEVEL_2_2;
            break;
        case OMX_VIDEO_AVCLevel3:
            return VENC_DRV_VIDEO_LEVEL_3;
            break;
        case OMX_VIDEO_AVCLevel31:
            return VENC_DRV_VIDEO_LEVEL_3_1;
            break;
        case OMX_VIDEO_AVCLevel32:
            return VENC_DRV_VIDEO_LEVEL_3_2;
            break;
        case OMX_VIDEO_AVCLevel4:
            return VENC_DRV_VIDEO_LEVEL_4;
            break;
        case OMX_VIDEO_AVCLevel41:
            return VENC_DRV_VIDEO_LEVEL_4_1;
            break;
        case OMX_VIDEO_AVCLevel42:
            return VENC_DRV_VIDEO_LEVEL_4_2;
            break;
        case OMX_VIDEO_AVCLevel5:
            return VENC_DRV_VIDEO_LEVEL_5;
            break;
        case OMX_VIDEO_AVCLevel51:
            return VENC_DRV_VIDEO_LEVEL_5_1;
            break;
        default:
            MTK_OMX_LOGE("[ERROR] Unsupported H264 Level");
            return VENC_DRV_VIDEO_LEVEL_3_1;
            break;
    }
}

VAL_UINT32_T MtkOmxVenc::Omx2DriverMPEG4ProfileMap(OMX_VIDEO_MPEG4PROFILETYPE eProfile)
{
    switch (eProfile)
    {
        case OMX_VIDEO_MPEG4ProfileSimple:
            return VENC_DRV_MPEG_VIDEO_PROFILE_MPEG4_SIMPLE;
            break;
        default:
            MTK_OMX_LOGE("[ERROR] Unsupported MPEG4 Profile");
            return VENC_DRV_MPEG_VIDEO_PROFILE_MPEG4_SIMPLE;
            break;
    }
}

VAL_UINT32_T MtkOmxVenc::Omx2DriverMPEG4LevelMap(OMX_VIDEO_MPEG4LEVELTYPE eLevel)
{
    switch (eLevel)
    {
        case OMX_VIDEO_MPEG4Level0:
            return VENC_DRV_VIDEO_LEVEL_0;
            break;
        case OMX_VIDEO_MPEG4Level0b:
            return VENC_DRV_VIDEO_LEVEL_0;
            break;
        case OMX_VIDEO_MPEG4Level1:
            return VENC_DRV_VIDEO_LEVEL_1;
            break;
        case OMX_VIDEO_MPEG4Level2:
            return VENC_DRV_VIDEO_LEVEL_2;
            break;
        case OMX_VIDEO_MPEG4Level3:
            return VENC_DRV_VIDEO_LEVEL_3;
            break;
        case OMX_VIDEO_MPEG4Level4:
            return VENC_DRV_VIDEO_LEVEL_4;
            break;
        case OMX_VIDEO_MPEG4Level4a:
            return VENC_DRV_VIDEO_LEVEL_4;
            break;
        case OMX_VIDEO_MPEG4Level5:
            return VENC_DRV_VIDEO_LEVEL_5;
            break;
        default:
            MTK_OMX_LOGE("[ERROR] Unsupported MPEG4 Level");
            return VENC_DRV_VIDEO_LEVEL_4;
            break;
    }
}

VAL_UINT32_T MtkOmxVenc::Omx2DriverH263ProfileMap(OMX_VIDEO_H263PROFILETYPE eProfile)
{
    switch (eProfile)
    {
        case OMX_VIDEO_H263ProfileBaseline:
            return VENC_DRV_MPEG_VIDEO_PROFILE_H263_0;
            break;
        default:
            MTK_OMX_LOGE("[ERROR] Unsupported H263 Profile");
            return VENC_DRV_MPEG_VIDEO_PROFILE_H263_0;
            break;
    }
}

VAL_UINT32_T MtkOmxVenc::Omx2DriverH263LevelMap(OMX_VIDEO_H263LEVELTYPE eLevel)
{
    switch (eLevel)
    {
        case OMX_VIDEO_H263Level10:
            return VENC_DRV_VIDEO_LEVEL_1;
            break;
        case OMX_VIDEO_H263Level20:
            return VENC_DRV_VIDEO_LEVEL_2;
            break;
        case OMX_VIDEO_H263Level30:
            return VENC_DRV_VIDEO_LEVEL_3;
            break;
        case OMX_VIDEO_H263Level40:
            return VENC_DRV_VIDEO_LEVEL_4;
            break;
        case OMX_VIDEO_H263Level50:
            return VENC_DRV_VIDEO_LEVEL_5;
            break;
        case OMX_VIDEO_H263Level60:
            return VENC_DRV_VIDEO_LEVEL_6;
            break;
        case OMX_VIDEO_H263Level70:
            return VENC_DRV_VIDEO_LEVEL_7;
            break;
        default:
            MTK_OMX_LOGE("[ERROR] Unsupported H263 Level");
            return VENC_DRV_VIDEO_LEVEL_1;
            break;
    }
}

VAL_UINT32_T MtkOmxVenc::Omx2DriverHEVCProfileMap(OMX_VIDEO_HEVCPROFILETYPE eProfile)
{
    VENC_DRV_MRESULT_T mReturn = VENC_DRV_MRESULT_OK;
    OMX_U32 uIsHEVCSWPlatform = 0;
    mReturn = eVEncDrvGetParam((VAL_HANDLE_T)NULL, VENC_DRV_GET_TYPE_IS_HEVC_SW_SPECIFIC_PLATFORM, (VAL_VOID_T *)&mChipName, (VAL_VOID_T *)&uIsHEVCSWPlatform);
    MTK_OMX_LOGD("uIsHEVCSWPlatform %d", uIsHEVCSWPlatform);

    if (uIsHEVCSWPlatform)
    {
        return VENC_DRV_MPEG_VIDEO_PROFILE_MPEG4_SIMPLE;

    }
    else
    {
        switch (eProfile)
        {
            case OMX_VIDEO_HEVCProfileMain:
                return VENC_DRV_HEVC_VIDEO_PROFILE_MAIN;
                break;
            default:
                MTK_OMX_LOGE("[ERROR] Unsupported HEVC Profile, %x", eProfile);
                return OMX_VIDEO_HEVCProfileMain;
                break;
        }
    }
}

VAL_UINT32_T MtkOmxVenc::Omx2DriverHEVCLevelMap(OMX_VIDEO_HEVCLEVELTYPE eLevel)
{
    switch (eLevel)
    {
        case OMX_VIDEO_HEVCMainTierLevel1:
            return VENC_DRV_VIDEO_LEVEL_1;
            break;
        case OMX_VIDEO_HEVCHighTierLevel1:
            return VENC_DRV_VIDEO_LEVEL_1;
            break;
        case OMX_VIDEO_HEVCMainTierLevel2:
            return VENC_DRV_VIDEO_LEVEL_2;
            break;
        case OMX_VIDEO_HEVCHighTierLevel2:
            return VENC_DRV_VIDEO_LEVEL_2;
            break;
        case OMX_VIDEO_HEVCMainTierLevel21:
            return VENC_DRV_VIDEO_LEVEL_2_1;
            break;
        case OMX_VIDEO_HEVCHighTierLevel21:
            return VENC_DRV_VIDEO_LEVEL_2_1;
            break;
        case OMX_VIDEO_HEVCMainTierLevel3:
            return VENC_DRV_VIDEO_LEVEL_3;
            break;
        case OMX_VIDEO_HEVCHighTierLevel3:
            return VENC_DRV_VIDEO_LEVEL_3;
            break;
        case OMX_VIDEO_HEVCMainTierLevel31:
            return VENC_DRV_VIDEO_LEVEL_3_1;
            break;
        case OMX_VIDEO_HEVCHighTierLevel31:
            return VENC_DRV_VIDEO_LEVEL_3_1;
            break;
        case OMX_VIDEO_HEVCMainTierLevel4:
            return VENC_DRV_VIDEO_LEVEL_4;
            break;
        case OMX_VIDEO_HEVCHighTierLevel4:
            return VENC_DRV_VIDEO_LEVEL_4;
            break;
        case OMX_VIDEO_HEVCMainTierLevel41:
            return VENC_DRV_VIDEO_LEVEL_4_1;
            break;
        case OMX_VIDEO_HEVCHighTierLevel41:
            return VENC_DRV_VIDEO_LEVEL_4_1;
            break;
        case OMX_VIDEO_HEVCMainTierLevel5:
            return VENC_DRV_VIDEO_LEVEL_5;
            break;
        case OMX_VIDEO_HEVCHighTierLevel5:
            return VENC_DRV_VIDEO_LEVEL_5;
            break;

        default:
            MTK_OMX_LOGE("[ERROR] Unsupported HEVC Level, %x", eLevel);
            return VENC_DRV_VIDEO_LEVEL_5;
            break;
    }
}

//-------- init omx encode parameters --------
OMX_BOOL MtkOmxVenc::InitEncParams()
{
    //strcpy((char *)mCompRole, "video_encoder.h263");//add in InitXXXEncParas()
    // init input port format
    mInputPortFormat.nPortIndex         = MTK_OMX_INPUT_PORT;
    mInputPortFormat.nIndex             = 0;
    mInputPortFormat.eCompressionFormat = OMX_VIDEO_CodingUnused;
    mInputPortFormat.eColorFormat       = OMX_MTK_COLOR_FormatYV12;
    mInputPortFormat.xFramerate         = (30 << 16);

    // init output port format
    mOutputPortFormat.nPortIndex    = MTK_OMX_OUTPUT_PORT;
    mOutputPortFormat.nIndex        = 0;
    //mOutputPortFormat.eCompressionFormat = OMX_VIDEO_CodingH263;//add in InitXXXEncParas()
    mOutputPortFormat.eColorFormat  = OMX_COLOR_FormatUnused;
    mOutputPortFormat.xFramerate    = (30 << 16);

    // init input port definition
    mInputPortDef.nPortIndex                            = MTK_OMX_INPUT_PORT;
    mInputPortDef.eDir                                  = OMX_DirInput;
    mInputPortDef.eDomain                               = OMX_PortDomainVideo;
    mInputPortDef.format.video.pNativeRender            = NULL;
    mInputPortDef.format.video.nFrameWidth              = 176;
    mInputPortDef.format.video.nFrameHeight             = 144;
    mInputPortDef.format.video.nStride                  = 176;
    mInputPortDef.format.video.nSliceHeight             = 144;
    mInputPortDef.format.video.nBitrate                 = 64000;
    mInputPortDef.format.video.xFramerate               = (30 << 16);
    mInputPortDef.format.video.cMIMEType                = (OMX_STRING)"raw";
    mInputPortDef.format.video.bFlagErrorConcealment    = OMX_FALSE;
    mInputPortDef.format.video.eCompressionFormat       = OMX_VIDEO_CodingUnused;
    mInputPortDef.format.video.eColorFormat             = OMX_MTK_COLOR_FormatYV12;

    mInputPortDef.nBufferCountActual                    = MTK_VENC_DEFAULT_INPUT_BUFFER_COUNT;
    mInputPortDef.nBufferCountMin                       = 1;
    mInputPortDef.nBufferSize                           = MTK_VENC_DEFAULT_INPUT_BUFFER_SIZE;
    mInputPortDef.bEnabled                              = OMX_TRUE;
    mInputPortDef.bPopulated                            = OMX_FALSE;

    // init output port definition
    mOutputPortDef.nPortIndex                           = MTK_OMX_OUTPUT_PORT;
    mOutputPortDef.eDomain                              = OMX_PortDomainVideo;
    //mOutputPortDef.format.video.cMIMEType = (OMX_STRING)"video/h263";
    mOutputPortDef.format.video.pNativeRender           = 0;
    mOutputPortDef.format.video.bFlagErrorConcealment   = OMX_FALSE;
    //mOutputPortDef.format.video.eCompressionFormat = OMX_VIDEO_CodingH263;
    mOutputPortDef.format.video.eColorFormat            = OMX_COLOR_FormatUnused;
    mOutputPortDef.format.video.nFrameWidth             = 176;
    mOutputPortDef.format.video.nStride                 = 176;
    mOutputPortDef.format.video.nFrameHeight            = 144;
    mOutputPortDef.format.video.nSliceHeight            = 144;
    mOutputPortDef.format.video.nBitrate                = MTK_VENC_DEFAULT_OUTPUT_BITRATE;
    mOutputPortDef.format.video.xFramerate              = (15 << 16);
    mOutputPortDef.eDir                                 = OMX_DirOutput;

    char value[PROPERTY_VALUE_MAX];
    property_get("vendor.mtk.omx.venc.bufcount", value, "8");
    mOutputPortDef.nBufferCountActual                   = atoi(value);
    MTK_OMX_LOGD_ENG("[TEST] mtk.omx.venc.bufcount set count to %d", mOutputPortDef.nBufferCountActual);
    //mOutputPortDef.nBufferCountActual                   = MTK_VENC_DEFAULT_OUTPUT_BUFFER_COUNT;
    mOutputPortDef.nBufferCountMin                      = 1;
    mOutputPortDef.nBufferSize                          = MTK_VENC_DEFAULT_OUTPUT_BUFFER_SIZE_MPEG4;
    mOutputPortDef.bEnabled                             = OMX_TRUE;
    mOutputPortDef.bPopulated                           = OMX_FALSE;

    // init bitrate type
    mBitrateType.nPortIndex     = MTK_OMX_OUTPUT_PORT;
    mBitrateType.eControlRate   = OMX_Video_ControlRateConstant;
    mBitrateType.nTargetBitrate = 64000;

    // init frame rate type
    mFrameRateType.nPortIndex       = MTK_OMX_OUTPUT_PORT;
    mFrameRateType.xEncodeFramerate = (30 << 16);

    //init quantization type
    mQuantizationType.nPortIndex    = MTK_OMX_OUTPUT_PORT;
    mQuantizationType.nQpI          = 0;
    mQuantizationType.nQpP          = 0;
    mQuantizationType.nQpB          = 0;

    // init macro block size
    MTK_OMX_MEMSET(&mVbsmcType, 0x00, sizeof(OMX_VIDEO_PARAM_VBSMCTYPE));
    mVbsmcType.nPortIndex   = MTK_OMX_OUTPUT_PORT;
    mVbsmcType.b16x16       = OMX_TRUE;

    // init motion vector type
    mMvType.nPortIndex      = MTK_OMX_OUTPUT_PORT;
    mMvType.eAccuracy       = OMX_Video_MotionVectorQuarterPel;
    mMvType.bUnrestrictedMVs = OMX_TRUE;
    mMvType.sXSearchRange   = 16;
    mMvType.sYSearchRange   = 16;

    // init intra refresh type
    mIntraRefreshType.nPortIndex    = MTK_OMX_OUTPUT_PORT;
    mIntraRefreshType.eRefreshMode  = OMX_VIDEO_IntraRefreshCyclic;
    mIntraRefreshType.nCirMBs       = 0;

    // init error correction type
    mErrorCorrectionType.nPortIndex             = MTK_OMX_OUTPUT_PORT;
    mErrorCorrectionType.bEnableDataPartitioning = OMX_FALSE;

    // init bitrate config
    mConfigBitrate.nPortIndex       = MTK_OMX_OUTPUT_PORT;
    mConfigBitrate.nEncodeBitrate   = MTK_VENC_DEFAULT_OUTPUT_BITRATE;

    // init intra refresh config
    mConfigIntraRefreshVopType.nPortIndex       = MTK_OMX_OUTPUT_PORT;
    mConfigIntraRefreshVopType.IntraRefreshVOP  = OMX_FALSE;

    // init rotate
    mRotationType.nRotation = 0;

    // allocate input buffer headers address array
    mInputBufferHdrs = (OMX_BUFFERHEADERTYPE **)MTK_OMX_ALLOC(
                           sizeof(OMX_BUFFERHEADERTYPE *)*mInputPortDef.nBufferCountActual);
    MTK_OMX_MEMSET(mInputBufferHdrs, 0x00, sizeof(OMX_BUFFERHEADERTYPE *)*mInputPortDef.nBufferCountActual);

    // allocate output buffer headers address array
    mOutputBufferHdrs = (OMX_BUFFERHEADERTYPE **)MTK_OMX_ALLOC(
                            sizeof(OMX_BUFFERHEADERTYPE *)*mOutputPortDef.nBufferCountActual);
    MTK_OMX_MEMSET(mOutputBufferHdrs, 0x00, sizeof(OMX_BUFFERHEADERTYPE *)*mOutputPortDef.nBufferCountActual);

    return OMX_TRUE;
}

OMX_BOOL MtkOmxVenc::InitH263EncParams()
{
    // init common part
    InitEncParams();

    strncpy((char *)mCompRole, "video_encoder.h263", sizeof(mCompRole));

    // init output port format
    mOutputPortFormat.eCompressionFormat = OMX_VIDEO_CodingH263;

    // init output port definition
    mOutputPortDef.format.video.cMIMEType           = (OMX_STRING)"video/h263";
    mOutputPortDef.format.video.eCompressionFormat  = OMX_VIDEO_CodingH263;

    // init h263 type
    MTK_OMX_MEMSET(&mH263Type, 0x00, sizeof(OMX_VIDEO_PARAM_H263TYPE));
    mH263Type.nPortIndex                = MTK_OMX_OUTPUT_PORT;
    mH263Type.eProfile                  = OMX_VIDEO_H263ProfileBaseline;
    mH263Type.eLevel                    = OMX_VIDEO_H263Level10;
    mH263Type.bPLUSPTYPEAllowed         = OMX_FALSE;
    mH263Type.nAllowedPictureTypes      = OMX_VIDEO_PictureTypeI | OMX_VIDEO_PictureTypeP;
    mH263Type.bForceRoundingTypeToZero  = OMX_TRUE;
    mH263Type.nPictureHeaderRepetition  = 0;
    mH263Type.nGOBHeaderInterval        = 0;

    // init profile level type
    mProfileLevelType.nPortIndex    = MTK_OMX_OUTPUT_PORT;
    mProfileLevelType.nProfileIndex = 0;
    mProfileLevelType.eProfile      = OMX_VIDEO_H263ProfileBaseline;
    mProfileLevelType.eLevel        = OMX_VIDEO_H263Level10;

    return OMX_TRUE;
}

OMX_BOOL MtkOmxVenc::InitMpeg4EncParams()
{
    // init common part
    InitEncParams();

    strncpy((char *)mCompRole, "video_encoder.mpeg4", sizeof(mCompRole));

    // init output port format
    mOutputPortFormat.eCompressionFormat = OMX_VIDEO_CodingMPEG4;

    // init output port definition
    mOutputPortDef.format.video.cMIMEType = (OMX_STRING)"video/mpeg4";
    mOutputPortDef.format.video.eCompressionFormat = OMX_VIDEO_CodingMPEG4;

    // init mpeg4 type
    MTK_OMX_MEMSET(&mMpeg4Type, 0x00, sizeof(OMX_VIDEO_PARAM_MPEG4TYPE));
    mMpeg4Type.nPortIndex           = MTK_OMX_OUTPUT_PORT;
    mMpeg4Type.eProfile             = OMX_VIDEO_MPEG4ProfileSimple;
    mMpeg4Type.eLevel               = OMX_VIDEO_MPEG4Level3;
    mMpeg4Type.nPFrames             = 10;
    mMpeg4Type.nBFrames             = 0;
    mMpeg4Type.nMaxPacketSize       = 255;
    mMpeg4Type.nAllowedPictureTypes = OMX_VIDEO_PictureTypeI | OMX_VIDEO_PictureTypeP;
    mMpeg4Type.bGov                 = OMX_FALSE;

    // init profile level type
    mProfileLevelType.nPortIndex    = MTK_OMX_OUTPUT_PORT;
    mProfileLevelType.nProfileIndex = 0;
    mProfileLevelType.eProfile      = OMX_VIDEO_MPEG4ProfileSimple;
    mProfileLevelType.eLevel        = OMX_VIDEO_MPEG4Level3;

    return OMX_TRUE;
}

OMX_BOOL MtkOmxVenc::InitAvcEncParams()
{
    // init common part
    InitEncParams();

    strncpy((char *)mCompRole, "video_encoder.avc", sizeof(mCompRole));

    // init output port format
    mOutputPortFormat.eCompressionFormat = OMX_VIDEO_CodingAVC;

    // init output port definition
    mOutputPortDef.format.video.cMIMEType = (OMX_STRING)"video/avc";
    mOutputPortDef.format.video.eCompressionFormat = OMX_VIDEO_CodingAVC;

    //mOutputPortDef.nBufferSize = MTK_VENC_DEFAULT_OUTPUT_BUFFER_SIZE_AVC;
    {
        //static bool bDump = true;
        int bufK;
        char value[PROPERTY_VALUE_MAX];
        bufK = property_get("vendor.mtk.omxvenc.bufk", value, "0");
        bufK = atoi(value);
        MTK_OMX_LOGE("bufk = %d", bufK);
        if (bufK == 0)
        {
            property_get("ro.vendor.mtk_config_max_dram_size", value, "0x40000000");
            if (0 == strcmp(value, "0x20000000"))
            {
                mOutputPortDef.nBufferSize = MTK_VENC_DEFAULT_OUTPUT_BUFFER_SIZE_AVC / 2; // For LCA project
            }
            else
            {
                mOutputPortDef.nBufferSize = MTK_VENC_DEFAULT_OUTPUT_BUFFER_SIZE_AVC;
            }
        }
        else
        {
            mOutputPortDef.nBufferSize = bufK * 1024;
        }
    }

    // init avc type
    MTK_OMX_MEMSET(&mAvcType, 0x00, sizeof(OMX_VIDEO_PARAM_AVCTYPE));
    mAvcType.nPortIndex             = MTK_OMX_OUTPUT_PORT;
    mAvcType.eProfile               = defaultAvcProfile(mChipName);
    mAvcType.eLevel                 = defaultAvcLevel(mChipName);
    mAvcType.nPFrames               = 0xFFFFFFFF; //Default value
    mAvcType.nBFrames               = 0; // B frames are not supported
    mAvcType.nAllowedPictureTypes   = OMX_VIDEO_PictureTypeI | OMX_VIDEO_PictureTypeP;
    mAvcType.nRefFrames             = 1;
    mAvcType.eLoopFilterMode        = OMX_VIDEO_AVCLoopFilterEnable;
    mAvcType.bEnableFMO             = OMX_FALSE;
    mAvcType.bFrameMBsOnly          = OMX_TRUE;
    mAvcType.bMBAFF                 = OMX_FALSE;
    mAvcType.bEntropyCodingCABAC    = OMX_FALSE;
    mAvcType.bWeightedPPrediction   = OMX_FALSE;
    mAvcType.bDirect8x8Inference    = OMX_FALSE;

    // init profile level type
    mProfileLevelType.nPortIndex    = MTK_OMX_OUTPUT_PORT;
    mProfileLevelType.nProfileIndex = 0;
    mProfileLevelType.eProfile      = defaultAvcProfile(mChipName);
    mProfileLevelType.eLevel        = defaultAvcLevel(mChipName);

    //for SECURE VIDEO PATH
    InitSecEncParams();

    return OMX_TRUE;
}

OMX_BOOL MtkOmxVenc::InitHevcEncParams()
{
    // init common part
    InitEncParams();

    strncpy((char *)mCompRole, "video_encoder.hevc", sizeof(mCompRole));

    // init output port format
    mOutputPortFormat.eCompressionFormat = OMX_VIDEO_CodingHEVC;

    // init output port definition
    mOutputPortDef.format.video.cMIMEType           = (OMX_STRING)"video/hevc";
    mOutputPortDef.format.video.eCompressionFormat  = OMX_VIDEO_CodingHEVC;

    //mOutputPortDef.nBufferSize = MTK_VENC_DEFAULT_OUTPUT_BUFFER_SIZE_HEVC;
    {
        //static bool bDump = true;
        int bufK;
        char value[PROPERTY_VALUE_MAX];
        bufK = property_get("vendor.mtk.omxenc.bufk", value, "0");
        bufK = atoi(value);
        MTK_OMX_LOGE("bufk = %d", bufK);
        if (bufK == 0)
        {
            mOutputPortDef.nBufferSize = MTK_VENC_DEFAULT_OUTPUT_BUFFER_SIZE_HEVC;
        }
        else
        {
            mOutputPortDef.nBufferSize = bufK * 1024;
        }
    }

    // init hevc type
    MTK_OMX_MEMSET(&mHevcType, 0x00, sizeof(OMX_VIDEO_PARAM_HEVCTYPE));
    mHevcType.nPortIndex = MTK_OMX_OUTPUT_PORT;
    mHevcType.eProfile  = defaultHevcProfile(mChipName);
    mHevcType.eLevel    = defaultHevcLevel(mChipName);

    // init profile level type
    mProfileLevelType.nPortIndex    = MTK_OMX_OUTPUT_PORT;
    mProfileLevelType.nProfileIndex = 0;
    mProfileLevelType.eProfile      = defaultHevcProfile(mChipName);
    mProfileLevelType.eLevel        = defaultHevcLevel(mChipName);

    return OMX_TRUE;
}

OMX_BOOL MtkOmxVenc::InitVP8EncParams()
{
    // init common part
    InitEncParams();

    strncpy((char *)mCompRole, "video_encoder.vp8", sizeof(mCompRole));

    // init output port format
    mOutputPortFormat.eCompressionFormat = OMX_VIDEO_CodingVP8;

    // init output port definition
    mOutputPortDef.format.video.cMIMEType           = (OMX_STRING)"video/x-vnd.on2.vp8";
    mOutputPortDef.format.video.eCompressionFormat  = OMX_VIDEO_CodingVP8;

    //mOutputPortDef.nBufferSize = MTK_VENC_DEFAULT_OUTPUT_BUFFER_SIZE_HEVC;
    {
        //static bool bDump = true;
        int bufK;
        char value[PROPERTY_VALUE_MAX];
        bufK = property_get("vendor.mtk.omxenc.bufk", value, "0");
        bufK = atoi(value);
        MTK_OMX_LOGE("bufk = %d", bufK);
        if (bufK == 0)
        {
            mOutputPortDef.nBufferSize = MTK_VENC_DEFAULT_OUTPUT_BUFFER_SIZE_VP8;
        }
        else
        {
            mOutputPortDef.nBufferSize = bufK * 1024;
        }
    }

    // init VP8 type
    MTK_OMX_MEMSET(&mVp8Type, 0x00, sizeof(OMX_VIDEO_PARAM_VP8TYPE));
    mVp8Type.nPortIndex           = MTK_OMX_OUTPUT_PORT;
    mVp8Type.eProfile             = OMX_VIDEO_VP8ProfileUnknown;
    mVp8Type.eLevel               = OMX_VIDEO_VP8LevelUnknown;

    // init profile level type
    mProfileLevelType.nPortIndex    = MTK_OMX_OUTPUT_PORT;
    mProfileLevelType.nProfileIndex = 0;
    mProfileLevelType.eProfile      = OMX_VIDEO_VP8ProfileUnknown;
    mProfileLevelType.eLevel        = OMX_VIDEO_VP8LevelUnknown;

    return OMX_TRUE;
}

OMX_BOOL MtkOmxVenc::InitSecEncParams(void)
{
    // check secure environment [
    int svp = 0;
    char value[PROPERTY_VALUE_MAX];

    property_get("ro.vendor.mtk_sec_video_path_support", value, "0");
    svp = atoi(value);
    mTeeEncType = NONE_TEE;
    if (svp)
    {
        property_get("ro.vendor.mtk_trustonic_tee_support", value, "0");
        //MTK_OMX_LOGE("%s ro.vendor.mtk_trustonic_tee_support %s",__FUNCTION__,value);
        if (atoi(value))
        {
            mTeeEncType = NONE_TEE; // disable for phone no more use SVR: TRUSTONIC_TEE;
        }
        else
        {
            VENC_DRV_MRESULT_T mReturn = VENC_DRV_MRESULT_OK;
            OMX_U32 uIsInHouseReady = 0;
            mReturn = eVEncDrvGetParam((VAL_HANDLE_T)NULL, VENC_DRV_GET_TYPE_IS_INHOUSE_TEE, (VAL_VOID_T *)&mChipName, (VAL_VOID_T *)&uIsInHouseReady);
            //MTK_OMX_LOGD("uIsInHouseReady %d", uIsInHouseReady);

           //if(mChipName == VAL_CHIP_NAME_MT8163 || mChipName == VAL_CHIP_NAME_MT8173 || mChipName == VAL_CHIP_NAME_MT8167)
           if(uIsInHouseReady)
           {
               property_get("ro.vendor.mtk_venc_inhouse_ready", value, "0");
               MTK_OMX_LOGE("%s ro.vendor.mtk_venc_inhouse_ready %s",__FUNCTION__,value);
               if (atoi(value))
               {
                 mTeeEncType = INHOUSE_TEE;
               }
           }
           else
           {
               property_get("ro.vendor.mtk_microtrust_tee_support", value, "0");
               if (atoi(value))
               {//Microtrust TEE
                   mTeeEncType = NONE_TEE; // disable for phone no more use SVR: MICROTRUST_TEE
               }
           }
        }
    }

    mIsSecureInst = (svp && mTeeEncType != NONE_TEE) ? OMX_TRUE : OMX_FALSE;
    if (mIsSecureInst) {
        mEnableDummy = OMX_FALSE;
    }
    // ] check secure environment
    MTK_OMX_LOGD("%s svp support: %d, type: %d", __FUNCTION__, svp, mTeeEncType);

    // setup basic secure variables.
    if (OMX_TRUE == mIsSecureInst)
    {
        if (mTeeEncType == TRUSTONIC_TEE || mTeeEncType == MICROTRUST_TEE)
        {
            memset(&tlc, 0, sizeof(tlc));
            property_get("ro.vendor.mtk_tee_gp_support", value, "0");
            int supportGP = atoi(value);
            if (1 == supportGP){
                tlc.tlcLib = dlopen("/vendor/lib/AVCSecureVencCA.so", RTLD_LAZY);
            }
            else{
                tlc.tlcLib = dlopen("/vendor/lib/libMtkH264SecVencTLCLib.so", RTLD_LAZY);
            }

            if (tlc.tlcLib != NULL) {
                tlc.tlcHandleCreate = (void *(*)())dlsym(tlc.tlcLib, "MtkH264SecVenc_tlcHandleCreate");
                tlc.tlcHandleRelease = (void (*)(void *))dlsym(tlc.tlcLib, "MtkH264SecVenc_tlcHandleRelease");

                tlc.tlcDumpSecMem = (int (*)(void *, uint32_t, void *, uint32_t))dlsym(tlc.tlcLib,
                                                                                       "MtkH264SecVenc_tlcDumpSecMem");
                tlc.tlcFillSecMem = (int (*)(void *, void *, uint32_t, uint32_t))dlsym(tlc.tlcLib,
                                                                                       "MtkH264SecVenc_tlcFillSecMem");
                bHasSecTlc = true;
            }
            else {
                MTK_OMX_LOGE("[ERROR] don't get secure lib %d", supportGP);
                bHasSecTlc = false;
            }
        }
        //else if (mTeeEncType == INHOUSE_TEE)//...
    }

    return mIsSecureInst;
}

OMX_BOOL MtkOmxVenc::DeInitSecEncParams(void)
{
    //TODO: free mTestSecInput ?

    DeInitSecCnvtBuffer();

    if (bHasSecTlc)
    {
        dlclose(tlc.tlcLib);
    }
    return OMX_TRUE;
}

//-------- sub functions of encoding function --------
bool MtkOmxVenc::setDrvParamBeforeHdr(void)
{
    VAL_BOOL_T          bRet = VAL_FALSE;
    VENC_DRV_MRESULT_T  rRet = VENC_DRV_MRESULT_OK;

    //for MPEG4
    if (MTK_VENC_CODEC_ID_MPEG4 == mCodecId ||
        MTK_VENC_CODEC_ID_MPEG4_SHORT == mCodecId ||
        MTK_VENC_CODEC_ID_MPEG4_1080P == mCodecId ||
        MTK_VENC_CODEC_ID_H263_VT == mCodecId)
    {
        VAL_UINT32_T omx_tids = 0;
        omx_tids = ((mVencThreadTid << 16) | (mVencEncThreadTid & 0xFFFF));
        rRet = eVEncDrvSetParam(mDrvHandle, VENC_DRV_SET_TYPE_SET_OMX_TIDS, (VAL_VOID_T *)&omx_tids, NULL);
        if (VENC_DRV_MRESULT_FAIL == rRet)
        {
            MTK_OMX_LOGD("[WARN] set tids fail");
        }
    }

    if (mSetWFDMode == OMX_TRUE)
    {
        mSetWFDMode = OMX_FALSE;
        rRet = eVEncDrvSetParam(mDrvHandle, VENC_DRV_SET_TYPE_WFD_MODE, VAL_NULL, VAL_NULL);
        if (VENC_DRV_MRESULT_FAIL == rRet)
        {
            MTK_OMX_LOGE("[ERROR] set WFD mode fail");
        }
        mWFDMode = OMX_TRUE;
    }

    if (mSetStreamingMode == OMX_TRUE)
    {
        mSetStreamingMode = OMX_FALSE;
        rRet = eVEncDrvSetParam(mDrvHandle, VENC_DRV_SET_TYPE_WFD_MODE, VAL_NULL, VAL_NULL);
        if (VENC_DRV_MRESULT_FAIL == rRet)
        {
            MTK_OMX_LOGE("[ERROR] set WFD mode fail");
        }
    }

    if (mEnableNonRefP == OMX_TRUE)
    {
        int enable = 1;
        rRet = eVEncDrvSetParam(mDrvHandle, VENC_DRV_SET_TYPE_NONREFP, &enable, VAL_NULL);
        if (VENC_DRV_MRESULT_FAIL == rRet)
        {
            MTK_OMX_LOGE("[ERROR] enable non ref p fail");
        }
    }

    // if VILTE
    if (mIsViLTE)
    {
        VENC_DRV_SCENARIO_T eIn = VENC_DRV_SCENARIO_VILTE_REC;
        rRet = eVEncDrvSetParam(mDrvHandle, VENC_DRV_SET_TYPE_SCENARIO, (VAL_VOID_T *)&eIn, VAL_NULL);
        if (VENC_DRV_MRESULT_FAIL == rRet)
        {
            MTK_OMX_LOGE("[ERROR] set venc scenario(0x%x) fail", eIn);
        }
        mWFDMode = OMX_FALSE;
        mEnableDummy = OMX_FALSE;
    }

    if (mOperationRate !=0)
    {
        unsigned int operationrate = mOperationRate;
        rRet = eVEncDrvSetParam(mDrvHandle, VENC_DRV_SET_TYPE_OPERATION_RATE, (VAL_VOID_T *)&operationrate, VAL_NULL);
        if (VENC_DRV_MRESULT_FAIL == rRet)
        {
            MTK_OMX_LOGE("[ERROR] set venc operation rate(%d) fail", operationrate);
        }
    }
    // encoding settings.
    bRet = EncSettingEnc();
    if (VAL_FALSE == bRet)
    {
        MTK_OMX_LOGE("[ERROR] EncSettingEnc fail");
        return false;
    }

    // statistic settings. (from MPEG4)
    if (VENC_DRV_MRESULT_FAIL == eVEncDrvSetParam(mDrvHandle, VENC_DRV_SET_TYPE_STATISTIC_ON, VAL_NULL, VAL_NULL))
    {
        MTK_OMX_LOGE("[ERROR] cannot set param");
    }
    return true;
}

bool MtkOmxVenc::postHdrProcess(OMX_U8 *aOutputBuf, OMX_U32 aOutputSize, VENC_DRV_DONE_RESULT_T *rEncResult)
{
    (void)(aOutputSize);
    if (MTK_VENC_CODEC_ID_AVC == mCodecId || MTK_VENC_CODEC_ID_AVC_VGA == mCodecId)
    {
        //for MTK_CAM_STEREO_CAMERA_SUPPORT
        OMX_U32 SideBySideSEI[4] = { 0x01000000, 0x81072d06, 0x03000081, 0x80200100 };
        OMX_U32 TopAndBottomSEI[4] = { 0x01000000, 0x82072d06, 0x03000001, 0x80200100 };
        switch (m3DVideoRecordMode)
        {
            case OMX_VIDEO_H264FPA_FRAMESEQUENCE:
                MTK_OMX_LOGE("Frame sequence of FPA is not support\n");
                break;
            case OMX_VIDEO_H264FPA_SIDEBYSIDE:
                memcpy(aOutputBuf + rEncResult->prBSBuf->u4BSSize, SideBySideSEI, sizeof(OMX_U32) * 4);
                rEncResult->prBSBuf->u4BSSize += sizeof(OMX_U32) * 4;
                break;
            case OMX_VIDEO_H264FPA_TOPANDBOTTOM:
                memcpy(aOutputBuf + rEncResult->prBSBuf->u4BSSize, TopAndBottomSEI, sizeof(OMX_U32) * 4);
                rEncResult->prBSBuf->u4BSSize += sizeof(OMX_U32) * 4;
                break;
            case OMX_VIDEO_H264FPA_NONE:
            default:
                break;
        }
    }
    return true;
}

bool MtkOmxVenc::preEncProcess(void)
{
    if (OMX_TRUE == mIsSecureInst && mIsSecureSrc)
    {
        if ((mDrawStripe == true) || (mDrawBlack == OMX_TRUE))
        {
            MTK_OMX_LOGD("Cannot modify frame buffer data when secure path enabled\n");
        }
    }
    else
    {
        if (mDrawStripe == true)
        {
            DrawYUVStripeLine((OMX_U8 *)mFrameBuf.rFrmBufAddr.u4VA);
        }

        if (mDrawBlack == OMX_TRUE)//for Miracast test case SIGMA 5.1.11 workaround
        {
            int YSize = mInputPortDef.format.video.nStride * mInputPortDef.format.video.nSliceHeight;
            memset((unsigned char *)mFrameBuf.rFrmBufAddr.u4VA, 0, YSize);
            memset((unsigned char *)((VAL_UINT32_T)mFrameBuf.rFrmBufAddr.u4VA + YSize),
                   0x80, mFrameBuf.rFrmBufAddr.u4Size - YSize);
        }
    }
    return true;
}

bool MtkOmxVenc::setDynamicFrameRate(OMX_BUFFERHEADERTYPE *pInputBuf)
{
    VAL_UINT32_T u4TimeDiff;
    VAL_UINT32_T u4AdjustedFrameRate;
    VAL_UINT32_T i,u4Count,u4Sum;
    VAL_UINT32_T u4FrameRateUpperBound = 30;
    VAL_UINT32_T u4FrameRateLowerBound = 10;

    VENC_DRV_MRESULT_T  rRet = VENC_DRV_MRESULT_OK;

    if (!mIsViLTE)
    {
        return false;
    }

    if (mLastFrameTimeStamp == 0)
    {
        return false;
    }

    if (pInputBuf->nTimeStamp < mLastFrameTimeStamp)
    {
        MTK_OMX_LOGE("setDynamicFrameRate invalid time stamp sequence\n");
        return false;
    }

    if (pInputBuf->nFlags & OMX_BUFFERFLAG_CAMERASWITCH)
    {
        for (i = 0 ;i < MTK_VENC_TIMESTAMP_WINDOW_SIZE ;i++)
        {
            mInputTimeStampDiffs[i] = 0;
        }
        MTK_OMX_LOGD("setDynamicFrameRate camera switch event\n");
        return false;
    }
    //MTK_OMX_LOGD("setDynamicFrameRate timeStamp %lld lastframe timestamp %lld\n",pInputBuf->nTimeStamp,mLastFrameTimeStamp);
    u4TimeDiff = pInputBuf->nTimeStamp - mLastFrameTimeStamp;

    if (u4TimeDiff > 100000)
    {
        MTK_OMX_LOGE("setDynamicFrameRate obnormal time diff timeStamp %lld lastframe timestamp %lld\n",pInputBuf->nTimeStamp,mLastFrameTimeStamp);
        for (i = 0 ;i < MTK_VENC_TIMESTAMP_WINDOW_SIZE ;i++)
        {
            mInputTimeStampDiffs[i] = 0;
        }
        MTK_OMX_LOGE("setDynamicFrameRate camera switch event detected by OMX\n");
        return false;
    }

    //MTK_OMX_LOGD("setDynamicFrameRate u4TimeDiff %d\n",u4TimeDiff);

    for (i =(MTK_VENC_TIMESTAMP_WINDOW_SIZE - 1) ;i>0;i--)
    {
        mInputTimeStampDiffs[i] = mInputTimeStampDiffs[i-1];
    }
    mInputTimeStampDiffs[0] = u4TimeDiff;


    u4Sum = 0;
    u4Count = 0;
    for (i = 0 ;i < MTK_VENC_TIMESTAMP_WINDOW_SIZE ;i++)
    {
        if (mInputTimeStampDiffs[i] !=0)
        {
            u4Sum += mInputTimeStampDiffs[i];
            u4Count++;

        }
    }

    if ((u4Sum == 0) || (u4Count == 0))
    {
        MTK_OMX_LOGE("setDynamicFrameRate invalid time stamp record\n");
        return false;
    }

    // timestamp diff not fill up diff slot will use original timestamp
    if(u4Count == MTK_VENC_TIMESTAMP_WINDOW_SIZE)
        u4AdjustedFrameRate = (1000000 + ((u4Sum/u4Count)/2))/(u4Sum/u4Count);
    else
        u4AdjustedFrameRate = mFrameRateType.xEncodeFramerate >> 16;

    if (u4AdjustedFrameRate > u4FrameRateUpperBound)
    {
        u4AdjustedFrameRate = u4FrameRateUpperBound;
        MTK_OMX_LOGD("setDynamicFrameRate limit frame rate to upper bound\n");
    }

    if (u4AdjustedFrameRate < u4FrameRateLowerBound)
    {
        u4AdjustedFrameRate = u4FrameRateLowerBound;
        MTK_OMX_LOGD("setDynamicFrameRate limit frame rate to lowner bound\n");
    }

    MTK_OMX_LOGD_ENG("setDynamicFrameRate frame rate %d\n",u4AdjustedFrameRate);

    rRet = eVEncDrvSetParam(mDrvHandle, VENC_DRV_SET_TYPE_DETECTED_FRAMERATE,
                                (VAL_VOID_T *)&u4AdjustedFrameRate, VAL_NULL);
    if (VENC_DRV_MRESULT_FAIL == rRet)
    {
        MTK_OMX_LOGE("[ERROR] setDynamicFrameRate cannot set param framerate");
    }

    return true;
}

bool MtkOmxVenc::setDrvParamBeforeEnc(void)
{
    VAL_BOOL_T          bRet = VAL_FALSE;
    VENC_DRV_MRESULT_T  rRet = VENC_DRV_MRESULT_OK;

#ifdef ANDROID_ICS
    // For Force Intra (Begin)
    if (OMX_TRUE == mForceIFrame || OMX_TRUE == mForceFullIFrame)
    {
        mForceIFrame = OMX_FALSE;
        rRet = eVEncDrvSetParam(mDrvHandle, VENC_DRV_SET_TYPE_FORCE_INTRA_ON, VAL_NULL, VAL_NULL);
        if (VENC_DRV_MRESULT_FAIL == rRet)
        {
            MTK_OMX_LOGE("[ERROR] cannot set param");
        }
    }
    // For Force Intra (End)

    // Dynamic bitrate adjustment [
    if (OMX_TRUE == mBitRateUpdated)
    {
        VENC_DRV_PARAM_ENC_EXTRA_T rEncoderExtraConfig = VENC_DRV_PARAM_ENC_EXTRA_T();
        mBitRateUpdated = OMX_FALSE;
        rEncoderExtraConfig.u4BitRate = mConfigBitrate.nEncodeBitrate;//please note the unit is bps
        rRet = eVEncDrvSetParam(mDrvHandle, VENC_DRV_SET_TYPE_ADJUST_BITRATE,
                                (VAL_VOID_T *)&rEncoderExtraConfig, VAL_NULL);
        if (VENC_DRV_MRESULT_FAIL == rRet)
        {
            MTK_OMX_LOGE("[ERROR] cannot set param bitrate");
        }
    }
    // ]
    // Dynamic framerate adjustment [
    if (OMX_TRUE == mFrameRateUpdated)
    {
        int frameRate = 30;
        mFrameRateUpdated = OMX_FALSE;
        frameRate = mFrameRateType.xEncodeFramerate >> 16; //only support int
        rRet = eVEncDrvSetParam(mDrvHandle, VENC_DRV_SET_TYPE_ADJUST_FRAMERATE,
                                (VAL_VOID_T *)&frameRate, VAL_NULL);
        if (VENC_DRV_MRESULT_FAIL == rRet)
        {
            MTK_OMX_LOGE("[ERROR] cannot set param framerate");
        }
    }
    // ]
#endif//ANDROID_ICS

    if (OMX_TRUE == mSetIInterval)
    {
        mSetIInterval = OMX_FALSE;
        rRet = eVEncDrvSetParam(mDrvHandle, VENC_DRV_SET_TYPE_I_FRAME_INTERVAL,
                                (VAL_VOID_T *)&mIInterval, VAL_NULL);
        if (VENC_DRV_MRESULT_FAIL == rRet)
        {
            MTK_OMX_LOGE("[ERROR] cannot set param I interval");
        }
    }

    if (OMX_TRUE == mSetIDRInterval)
    {
        mSetIDRInterval = OMX_FALSE;
        rRet = eVEncDrvSetParam(mDrvHandle, VENC_DRV_SET_TYPE_IDR_FRAME_INTERVAL,
                                (VAL_VOID_T *)&mIDRInterval, VAL_NULL);
        if (VENC_DRV_MRESULT_FAIL == rRet)
        {
            MTK_OMX_LOGE("[ERROR] cannot set param IDR interval");
        }
    }


    if (mSkipFrame)
    {
        mSkipFrame = 0;
        rRet = eVEncDrvSetParam(mDrvHandle, VENC_DRV_SET_TYPE_SKIP_FRAME, VAL_NULL, VAL_NULL);
        if (VENC_DRV_MRESULT_FAIL == rRet)
        {
            MTK_OMX_LOGE("[ERROR] cannot skip frame");
        }
    }

    if (OMX_TRUE == mPrependSPSPPSToIDRFramesNotify)
    {
        mPrependSPSPPSToIDRFramesNotify = OMX_FALSE;
        VAL_UINT32_T enable = mPrependSPSPPSToIDRFrames;
        if (VENC_DRV_MRESULT_FAIL == eVEncDrvSetParam(mDrvHandle, VENC_DRV_SET_TYPE_PREPEND_HEADER, &enable, VAL_NULL))
        {
            MTK_OMX_LOGE("[ERROR] set prepend header fail");
        }
    }

    if (OMX_TRUE == mSetQP)
    {
        mSetQP = OMX_FALSE;
        if (VENC_DRV_MRESULT_FAIL == eVEncDrvSetParam(mDrvHandle, VENC_DRV_SET_TYPE_CONFIG_QP, (VAL_VOID_T *)&mQP, VAL_NULL))
        {
            MTK_OMX_LOGE("[ERROR] set qp %u fail", mQP);
        }
    }

    if (OMX_TRUE == mGotSLI)
    {
        mGotSLI = OMX_FALSE;
        OMX_U32 RFS_ErrFrm = mSLI.SliceLoss[0] & 0x3F;
        if (VENC_DRV_MRESULT_FAIL == eVEncDrvSetParam(mDrvHandle, VENC_DRV_SET_TYPE_RFS_ON, &RFS_ErrFrm, VAL_NULL))
        {
            MTK_OMX_LOGE("[ERROR] set RFS on fail");
        }
    }

    return true;
}

bool MtkOmxVenc::setDrvParamAfterEnc(void)
{
    //VAL_BOOL_T          bRet = VAL_FALSE;
    VENC_DRV_MRESULT_T  rRet = VENC_DRV_MRESULT_OK;

#ifdef ANDROID_ICS
    // Disable Force Intra
    rRet = eVEncDrvSetParam(mDrvHandle, VENC_DRV_SET_TYPE_FORCE_INTRA_OFF, VAL_NULL, VAL_NULL);
    if (VENC_DRV_MRESULT_FAIL == rRet)
    {
        MTK_OMX_LOGE("[ERROR] cannot set param");
    }
#endif//ANDROID_ICS
    return true;
}

void MtkOmxVenc::normalSecureSwitchHndling(OMX_BUFFERHEADERTYPE *pInputBuf, OMX_BUFFERHEADERTYPE *pOutputBuf)
{
    mIsSecureSrc = (OMX_TRUE == mIsSecureSrc) ? OMX_FALSE : OMX_TRUE;
    MTK_OMX_LOGD("set secure src:%d", mIsSecureSrc);

    //Notify Encode Thread wait PortReconfig
    mPortReconfigInProgress = OMX_TRUE; //Encode Thread need to wait Port Re-config

    //Put current frame back to Queue
    pOutputBuf->nTimeStamp = mLastFrameTimeStamp;
    pOutputBuf->nFilledLen = 0;
    if (mDoConvertPipeline)
    {
        EncHandleEmptyBufferDone(pInputBuf);
    }
    else
    {
        QueueBufferAdvance(mpVencInputBufQ, pInputBuf);
    }
    HandleFillBufferDone(pOutputBuf);
    if (mDummyIdx >= 0)
    {
        QueueOutputBuffer(mDummyIdx);//the VencOutBufQ MUST be REAL OutBufQ!!!
        mDummyIdx = -1;
    }

    if (mDoConvertPipeline)  //switch internal bufQ to secure or normal
    {
        //clear C-out Q
        LOCK(mpConvertOutputBufQ->mBufQLock);
        mpConvertOutputBufQ->Clear();
        mpConvertOutputBufQ->mPendingNum = 0;
        UNLOCK(mpConvertOutputBufQ->mBufQLock);
        //both C-out & E-in bufHdrQ should be EMPTY!!
        MTK_OMX_LOGD("C-out:%d, E-in:%d", mpConvertOutputBufQ->Size(), mpVencInputBufQ->Size());
        int i;
        if (mIsSecureSrc)
        {
            // check there are secure convert buffers, if no, allocate here.
            if (checkSecCnvtBufferNum() <= 0)
            {
                InitSecCnvtBuffer(CONVERT_MAX_BUFFER);
            }
            //normal to secure
            for (i = 0; i < CONVERT_MAX_BUFFER; ++i)
            {
                //memset(mConvertOutputBufferHdrs[i], 0, sizeof(OMX_BUFFERHEADERTYPE));
                *(buffer_handle_t *)(mConvertOutputBufferHdrs[i]->pBuffer + 4) = mSecConvertBufs[i]->handle;
                mConvertOutputBufferHdrs[i]->nAllocLen      = 8;
                mConvertOutputBufferHdrs[i]->nFlags         = 0;
                mConvertOutputBufferHdrs[i]->nTimeStamp     = 0;
                mConvertOutputBufferHdrs[i]->nFilledLen     = 0;
                mConvertOutputBufferHdrs[i]->nOffset        = 0;
                mConvertOutputBufferHdrs[i]->pMarkData      = 0;


                //memset(mVencInputBufferHdrs[i], 0, sizeof(OMX_BUFFERHEADERTYPE));
                *(buffer_handle_t *)(mVencInputBufferHdrs[i]->pBuffer + 4) = mSecConvertBufs[i]->handle;
                mVencInputBufferHdrs[i]->nAllocLen      = 8;
                mVencInputBufferHdrs[i]->nFlags         = 0;
                mVencInputBufferHdrs[i]->nTimeStamp     = 0;
                mVencInputBufferHdrs[i]->nFilledLen     = 0;
                mVencInputBufferHdrs[i]->nOffset        = 0;
                mVencInputBufferHdrs[i]->pMarkData      = 0;
            }
        }
        else
        {
            //secure to normal
            for (i = 0; i < CONVERT_MAX_BUFFER; ++i)
            {
                //memset(mConvertOutputBufferHdrs[i], 0, sizeof(OMX_BUFFERHEADERTYPE));
                mConvertOutputBufferHdrs[i]->nAllocLen      = mCnvtBufferSize;
                mConvertOutputBufferHdrs[i]->nFlags         = 0;
                mConvertOutputBufferHdrs[i]->nTimeStamp     = 0;
                mConvertOutputBufferHdrs[i]->nFilledLen     = 0;
                mConvertOutputBufferHdrs[i]->nOffset        = 0;
                mConvertOutputBufferHdrs[i]->pMarkData      = 0;

                //memset(mVencInputBufferHdrs[i], 0, sizeof(OMX_BUFFERHEADERTYPE));
                mVencInputBufferHdrs[i]->nAllocLen      = mCnvtBufferSize;
                mVencInputBufferHdrs[i]->nFlags         = 0;
                mVencInputBufferHdrs[i]->nTimeStamp     = 0;
                mVencInputBufferHdrs[i]->nFilledLen     = 0;
                mVencInputBufferHdrs[i]->nOffset        = 0;
                mVencInputBufferHdrs[i]->pMarkData      = 0;
            }
        }
        LOCK(mpConvertOutputBufQ->mBufQLock);
        //fill convert output bufq
        for (i = 0; i < CONVERT_MAX_BUFFER; ++i)
        {
            mpConvertOutputBufQ->Push(i);
            ++mpConvertOutputBufQ->mPendingNum;
        }
        UNLOCK(mpConvertOutputBufQ->mBufQLock);
    }
    else//reset convert buffer
    {
        //convert buffer for secure path is not required now
#if 0
        if (mIsSecureSrc)
        {
            // check there are secure convert buffers, if no, allocate here.
            if (checkSecCnvtBufferNum() <= 0)
            {
                InitSecCnvtBuffer(1);
            }
            //normal to secure, set handle to buffer
            *(buffer_handle_t *)(mCnvtBuffer + 4) = mSecConvertBufs[0]->handle;
        }
#endif
    }
    //Notify that Output buffers' format goes to reconfig
    MTK_OMX_LOGD("--- OMX_EventPortSettingsChanged ---");
    mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                           mAppData,
                           OMX_EventPortSettingsChanged,
                           MTK_OMX_OUTPUT_PORT,
                           OMX_IndexVendorMtkOmxVencSwitchWFDSecureOut,
                           NULL);
    ++mReconfigCount;
    mFrameCount = 0;
    return;
}

int MtkOmxVenc::checkSecSwitchInEnc(OMX_BUFFERHEADERTYPE *pInputBuf, OMX_BUFFERHEADERTYPE *pOutputBuf)
{
    int inputbuffertype = 0;
    if (OMX_TRUE == mStoreMetaDataInOutBuffers)
    {
        if ((OMX_TRUE == mIsSecureInst) && mDoConvertPipeline)
        {
            //inputbuffertype = ((unsigned int)pInputBuf->pMarkData == 0xc07f1900) ? 1 : 0;
            inputbuffertype = ((pInputBuf->nFlags & OMX_BUFFERFLAG_SECUREBUF) != 0);
            MTK_OMX_LOGD("get convert in: %d, 0x%x, 0x%x",
                         inputbuffertype, pInputBuf->pMarkData, pInputBuf->nFlags);
        }
        else
        {
            if ((OMX_TRUE == mIsSecureInst) && (OMX_FALSE == isBufferSec(pInputBuf->pBuffer + pInputBuf->nOffset, pInputBuf->nFilledLen, &inputbuffertype)))
            {
                MTK_OMX_LOGE("[ERROR] Input frame buffer type is not normal nor secure\n");
                return -1;
            }
        }
    }

    //Src Buffer Type = 0 Normal Buffer
    //Src Buffer Type = 1 Secure Buffer
    //Src Buffer Type = 2 Secure FD, not support
    //mIsSecureSrc = FALSE current Working buffers for Normal Path
    //mIsSecureSrc = TRUE current Working buffers for Secure Path
    // for TRUSTONIC TEE SUPPORT only [
    if (mIsSecureSrc && mTeeEncType == TRUSTONIC_TEE && !bHasSecTlc)
    {
        MTK_OMX_LOGE("[ERROR] Don't support secure input!\n");
        return -1;
    }
    // ]

    if (((1 == inputbuffertype) && (OMX_FALSE == mIsSecureSrc)) ||
        ((0 == inputbuffertype) && (OMX_TRUE == mIsSecureSrc)))
    {
        normalSecureSwitchHndling(pInputBuf, pOutputBuf);
        return 1;//return switching happen
    }
    return 0;
}

int MtkOmxVenc::checkSecSwitchInCC(OMX_BUFFERHEADERTYPE *pInputBuf, OMX_BUFFERHEADERTYPE *pOutputBuf)
{
    return 0;
}

bool MtkOmxVenc::checkMBAFF(void)
{
    char value[PROPERTY_VALUE_MAX];
    property_get("vendor.mtk.omxvenc.mbaff", value, "2");
    OMX_U32 u4MBAFF = (OMX_U32)atoi(value);
    MTK_OMX_LOGD("MBAFF 0x%x", u4MBAFF);
    if (u4MBAFF != 2)
    {
        return u4MBAFF;
    }
    else
    {
        bool fgMBAFF = mAvcType.bMBAFF;
        if (!fgMBAFF ||
            mEncDrvSetting.u4Profile == VENC_DRV_H264_VIDEO_PROFILE_BASELINE ||
            mWFDMode == OMX_TRUE)
        {
            return false;
        }

        // Chip list
        if (0)
        {
            if (mEncDrvSetting.u4BufWidth * mEncDrvSetting.u4BufHeight >= 1920 * 1088 &&
                mEncDrvSetting.u4BufHeight % 32 == 0)
            {
                return true;
            }
        }
    }
    return false;
}

bool MtkOmxVenc::checkH264MultiSlice(VAL_UINT32_T u4FrameWidth, VAL_UINT32_T u4FrameHeight)
{
    VENC_DRV_MRESULT_T mReturn = VENC_DRV_MRESULT_OK;
    OMX_U32 uGetMultiSliceType = 0;
    mReturn = eVEncDrvGetParam((VAL_HANDLE_T)NULL, VENC_DRV_GET_TYPE_MULTISLICE_TYPE, (VAL_VOID_T *)&mChipName, (VAL_VOID_T *)&uGetMultiSliceType);
    MTK_OMX_LOGD("uGetMultiSliceType %d", uGetMultiSliceType);

    switch (uGetMultiSliceType)
    {
        // sw encoder
        //D2
        case VENC_DRV_MULTISLICE_TYPE_ONE:
        {
            return mIsViLTE;
        }
        break;
        //80
        case VENC_DRV_MULTISLICE_TYPE_TWO:
        {
            // Enable multi-slice encoding only when the recorded size >= 720p.
            return (u4FrameWidth * u4FrameHeight >= 1280 * 720) ? true : false;
        }
        break;
        //70
        case VENC_DRV_MULTISLICE_TYPE_THREE:
        {
            // Enable multi-slice encoding only when the recorded size >= HVGA
            return (u4FrameWidth * u4FrameHeight >= 480 * 320) ? true : false;
        }
        break;
        default:
            break;
    }
    return false;
}

bool MtkOmxVenc::dumpInputBuffer(OMX_BUFFERHEADERTYPE *pInputBuf, OMX_U8 *aInputBuf, OMX_U32 aInputSize)
{
    (void)(pInputBuf);

    char name[128];
    int width = mInputPortDef.format.video.nFrameWidth;
    int height = mInputPortDef.format.video.nFrameHeight;
    int size = 0;
    if (OMX_TRUE == mIsSecureSrc)
    {
        if (OMX_TRUE == mDumpInputFrame)
        {
            if (OMX_FALSE == NeedConversion())
            {
                int hSecHandle = 0;
                buffer_handle_t _handle = *((buffer_handle_t *)(aInputBuf + 4));
                gralloc_extra_query(_handle, GRALLOC_EXTRA_GET_SECURE_HANDLE_HWC, &hSecHandle);

                snprintf(name, sizeof(name), "/sdcard/input_%u_%u_%d_s.yuv", width, height, gettid());
                size = width * height * 3 / 2;
                dumpSecBuffer(name, hSecHandle, size);
            }
            //MTK_OMX_LOGD("Cannot dump frame buffer data when secure path enabled\n");
        }
    }
    else
    {
        //if(1)// (mRTDumpInputFrame == OMX_TRUE)
        {
            char value[PROPERTY_VALUE_MAX];
            property_get("vendor.mtk.omxvenc.dump", value, "0");
            mDumpInputFrame = (OMX_BOOL) atoi(value);
        }

        if (OMX_TRUE == mDumpInputFrame)
        {
            if (OMX_FALSE == mDumpCts) {
                snprintf(name, sizeof(name), "/sdcard/input_%u_%u_%u_%d.yuv", mEncDrvSetting.u4BufWidth, mEncDrvSetting.u4BufHeight, aInputSize, gettid());
            } else {
                snprintf(name, sizeof(name), "/sdcard/vdump/Venc_input_%d_%d_t%05d.yuv", width, height, gettid());
            }

            dumpBuffer(name, (unsigned char *)aInputBuf, aInputSize);
        }
    }
    return true;
}

bool MtkOmxVenc::dumpOutputBuffer(OMX_BUFFERHEADERTYPE *pOutputBuf, OMX_U8 *aOutputBuf, OMX_U32 aOutputSize)
{
    char name[128];
    if (mIsSecureSrc == OMX_TRUE && (mDumpFlag & DUMP_SECURE_OUTPUT_Flag) && bHasSecTlc)
    {
        int hSecHandle = 0;
        buffer_handle_t _handle = *((buffer_handle_t *)(aOutputBuf + 4));
        gralloc_extra_query(_handle, GRALLOC_EXTRA_GET_SECURE_HANDLE, &hSecHandle);

        snprintf(name, sizeof(name), "/sdcard/enc_dump_%d_s.h264", gettid());
        dumpSecBuffer(name, hSecHandle, mBitStreamBuf.u4BSSize);
    }
    else
    {
        char value[PROPERTY_VALUE_MAX];
        VAL_BOOL_T bDumpLog = 0;
        property_get("vendor.mtk.omx.venc.dump.bs", value, "0");
        bDumpLog = atoi(value);

        aOutputBuf = pOutputBuf->pBuffer + pOutputBuf->nOffset;
        aOutputSize = pOutputBuf->nFilledLen;

        if (bDumpLog)
        {
            snprintf(name, sizeof(name), "/sdcard/enc_dump_bitstream%d.bin", gettid());
            if (mIsSecureSrc == OMX_FALSE)
            {
                dumpBuffer(name, (unsigned char *)aOutputBuf, aOutputSize);
            }
       #ifdef MTK_DUM_SEC_ENC
            else if(INHOUSE_TEE == mTeeEncType)
            {
                  int hSecHandle = 0;
                  buffer_handle_t _handle = *((buffer_handle_t *)(aOutputBuf + 4));
                  gralloc_extra_query(_handle, GRALLOC_EXTRA_GET_SECURE_HANDLE, &hSecHandle);
                  dumpSecBuffer(name, hSecHandle, mBitStreamBuf.u4BSSize);
            }
       #endif
        }

        if (mIsViLTE)
        {
            VAL_BOOL_T bDump = 0;

            property_get("vendor.mtk.omx.venc.vilte.dump.bs", value, "0");
            bDump = atoi(value);
            if (bDump)
            {
                snprintf(name, sizeof(name), "/sdcard/vilte_venc_bs_%d.bin", gettid());
                dumpBuffer(name, (unsigned char *)aOutputBuf, aOutputSize);
            }
        }
    }
    return true;
}

bool MtkOmxVenc::dumpSecBuffer(char *name, int hSecHandle, int size)
{
    int i, tmpSize = 0;
    unsigned char *buf, *tmp;
    if (mTeeEncType == TRUSTONIC_TEE)
    {
        tmpSize = 64 << 10;//64K
        buf = (unsigned char *)memalign(512, size);
        if(buf == NULL) return false;

        tmp = buf;
        void *tlcHandle = tlc.tlcHandleCreate();
        for (i = 0; i < (size / tmpSize); ++i)
        {
            tlc.tlcDumpSecMem(tlcHandle, ((uint32_t)hSecHandle + (i * tmpSize)),
                              tmp + tmpSize, tmpSize);
        }
        if (size  % tmpSize)
        {
            tlc.tlcDumpSecMem(tlcHandle, ((uint32_t)hSecHandle + (i * tmpSize)),
                              tmp + (i * tmpSize), size % tmpSize);
        }
        dumpBuffer(name, buf, size);
        free(buf);
        tlc.tlcHandleRelease(tlcHandle);
    }
#ifdef MTK_DUM_SEC_ENC
    else if(INHOUSE_TEE == mTeeEncType)
    {
         MTK_OMX_LOGD("INHOUSE dump");
         MTK_OMX_LOGE(" %s line %d \n", __FUNCTION__,__LINE__);
         if(pTmp_buf == 0)
         {
            pTmp_buf =(unsigned char *)memalign(512, size);
            if(pTmp_buf == NULL) return false;
            if(pTmp_buf)
            Tmp_bufsz = size;
         }
         else if(size > Tmp_bufsz)
         {
             free(pTmp_buf);
             pTmp_buf =(unsigned char *)memalign(512, size);
             if(pTmp_buf == NULL) return false;
             if(pTmp_buf)
             Tmp_bufsz = size;
         }
         MTK_OMX_LOGE(" %s line %d,szie %d \n", __FUNCTION__,__LINE__,size);
         Dump_buf_sec_and_normal(hSecHandle, pTmp_buf, size, 0);
         dumpBuffer(name, pTmp_buf, size);
    }
#endif
    else
    {
        MTK_OMX_LOGD("Don't support dump input");
    }
    return true;
}

bool MtkOmxVenc::dumpBuffer(char *name, unsigned char *data, int size)
{
    if(name == NULL || data == NULL || size == 0) return false;

    FILE *fp = fopen(name, "ab");
    char newFile[256]={0};
    if(!fp)
    {
         char *ptsrc =NULL;
         if(0==memcmp("/data/",name, 6))
         {
            ptsrc =(char *)(name +6);
            memcpy(newFile,"/sdcard/",8);
            if (strlen(ptsrc) < 256)
            {
                memcpy(newFile+8,ptsrc, strlen(ptsrc));
            }
            else
            {
                MTK_OMX_LOGE("open file %s fail: %d %s, file path too long", name, errno, strerror(errno));
                return OMX_TRUE;
            }
         }
         else  if(0==memcmp("/sdcard/",name, 8))
         {
            ptsrc =(char *)(name +8);
            memcpy(newFile,"/data/",6);
            if (strlen(ptsrc) < 256)
            {
                memcpy(newFile+6,ptsrc, strlen(ptsrc));
            }
            else
            {
                MTK_OMX_LOGE("open file %s fail: %d %s, file path too long", name, errno, strerror(errno));
                return OMX_TRUE;
            }
         }
         else
         {
             MTK_OMX_LOGE("open file %s fail: %d %s,and new file not support", name, errno, strerror(errno));
             return true;
         }
         MTK_OMX_LOGE("open file %s fail,and switch to %s\n", name, newFile);
         fp = fopen(newFile, "ab");
    }

    if (fp)
    {
        fwrite((void *)data, 1, size, fp);
        fclose(fp);
    }
    else
    {
        MTK_OMX_LOGE("open file %s fail: %d %s", name, errno, strerror(errno));
    }
    return true;
}

bool MtkOmxVenc::dumpYUVBuffer(char *name, unsigned char *y, unsigned char *u, unsigned char *v,
                               int width, int height)
{
    int ySize = width * height;
    int uvSize = ySize >> 2;
    FILE *fp = fopen(name, "ab");
    if (fp)
    {
        fwrite((void *)y, 1, ySize, fp);
        fwrite((void *)u, 1, uvSize, fp);
        fwrite((void *)v, 1, uvSize, fp);
        fclose(fp);
    }
    else
    {
        MTK_OMX_LOGE("open file %s fail: %d %s", name, errno, strerror(errno));
    }
    return true;
}

OMX_BOOL MtkOmxVenc::IsConstantBitrate(void)
{
    return mSetConstantBitrateMode;
}

OMX_BOOL MtkOmxVenc::encodeHybridEOS(OMX_BUFFERHEADERTYPE *pOutputBuf)
{
    if (OMX_FALSE == mMeetHybridEOS) {
        MTK_OMX_LOGE("Can do encodeHybridEOS when not meet EOS");
        return OMX_FALSE;
    }
    mMeetHybridEOS = OMX_FALSE;

    pOutputBuf->nFlags &= OMX_BUFFERFLAG_EOS;
    pOutputBuf->nFlags &= OMX_BUFFERFLAG_ENDOFFRAME;
    pOutputBuf->nTimeStamp = mLastFrameTimeStamp;

    //do final encode here
    VENC_DRV_MRESULT_T rRet = VENC_DRV_MRESULT_OK;
    VENC_DRV_DONE_RESULT_T rEncResult;
    rEncResult.eMessage = VENC_DRV_MESSAGE_OK;
    OMX_U8 *aOutputBuf = pOutputBuf->pBuffer + pOutputBuf->nOffset;
    OMX_U32 aOutputSize = pOutputBuf->nAllocLen;
    GetVEncDrvBSBuffer(aOutputBuf, aOutputSize);
    rRet = eVEncDrvEncode(mDrvHandle, VENC_DRV_START_OPT_ENCODE_FINAL,
            VAL_NULL, &mBitStreamBuf, &rEncResult);
    if (rRet != VENC_DRV_MRESULT_OK) {
        MTK_OMX_LOGE("encode final fail %d", rRet);
        rEncResult.prBSBuf->u4BSSize = 0;
    }
    pOutputBuf->nFilledLen = rEncResult.prBSBuf->u4BSSize;

    HandleFillBufferDone(pOutputBuf);
    return OMX_TRUE;
}

