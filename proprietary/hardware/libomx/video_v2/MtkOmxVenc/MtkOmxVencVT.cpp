#include <signal.h>
#include <cutils/log.h>

#include <utils/Trace.h>
#include <utils/AndroidThreads.h>

#include "osal_utils.h"
#include "MtkOmxVenc.h"
#include "MtkOmx2V4L2.h"

#include <cutils/properties.h>
#include <sched.h>
#include <assert.h>

#undef LOG_TAG
#define LOG_TAG "MtkOmxVencVT"

#include "OMX_IndexExt.h"

#ifdef ATRACE_TAG
#undef ATRACE_TAG
#define ATRACE_TAG ATRACE_TAG_VIDEO
#endif//ATRACE_TAG

#include <utils/Trace.h>
//#include <utils/AndroidThreads.h>

void MtkOmxVenc::MtkOmxVencVT()
{
    mIsViLTE = (OMX_BOOL) MtkVenc::IsViLTE();
    mPrepareToResolutionChange = OMX_FALSE;
#if 0
    u4EncodeWidth = 0;
    u4EncodeHeight = 0;
    nFilledLen = 0;
#endif
    mLastQueuedFrameTimeStamp = 0;
    mGotSLI = OMX_FALSE;
}

void MtkOmxVenc::ComponentInitVT()
{
    //nFilledLen = mInputPortDef.format.video.nFrameWidth * mInputPortDef.format.video.nFrameHeight * 1.5;
    bDynamicFps = MtkVenc::DynamicFps();
}

OMX_BOOL MtkOmxVenc::VT_OnInit(video_encode_param* param)
{
    if (mIsViLTE)
    {
        param->prepend_hdr = 1; // no need to dequeu header
    }
    pParam = param;

    return OMX_TRUE;
}

OMX_ERRORTYPE MtkOmxVenc::VT_ResolutionChange(OMX_BUFFERHEADERTYPE *pInputBuf)
{
    IN_FUNC();
    if(!mIsViLTE) { OUT_FUNC(); return OMX_ErrorNotReady;}

    video_resolutionchange rResChange = {0};
    VAL_BOOL_T  bResolutionChange = OMX_FALSE;

    //flag is raised only when set bitrate
    if (mPrepareToResolutionChange == OMX_TRUE)
    {
        VT_GetResolutionChange(&rResChange);
        mPrepareToResolutionChange = OMX_FALSE;
    }

    if(rResChange.resolutionchange == 0)
    {
        OUT_FUNC();
        return OMX_ErrorNotReady;
    }

    // I-Frame after resolution change
    pInputBuf->nFlags |= OMX_BUFFERFLAG_SYNCFRAME;

    // put input buffer back bufferQ
    QueueBufferAdvance(&mEmptyThisBufQ, pInputBuf);
    SIGNAL(mEncodeSem);

    MTK_OMX_LOGD_ENG("resolution change to: %d, %d", rResChange.width, rResChange.height);
#if 0
    if(mEncodeFlags & ENCODE_START)
    {
        mEncodeFlags = ENCODE_FLUSHING;
        startDeque();
    }
    else
    {
        startFlush();
    }
    waitFlushDone(); // Frame & BS are at OMX queue

    MTK_OMX_LOGD_ENG("resolution change to: %d, %d", u4EncodeWidth, u4EncodeHeight);

    mEncParam.width = u4EncodeWidth;
    mEncParam.height = u4EncodeHeight;

    EncSettingEncCommon();

    if (mConfigBitrate.nEncodeBitrate != MTK_VENC_DEFAULT_OUTPUT_BITRATE)
    {
        mOutputPortDef.format.video.nBitrate = mConfigBitrate.nEncodeBitrate;
        MTK_OMX_LOGD_ENG("re-set bitrate = %d", mConfigBitrate.nEncodeBitrate);
    }

    //do not deinit driver when resolution change in vilte mode
    //keep this record if stream on fail

    DeInitVideoEncodeHW();

    mEncodeFlags = ENCODE_START;
#else
    startFlush();
    waitFlushDone(); // Frame & BS are at OMX queue

    // request buffers first to let kernel's buffer clean up
    ioctl_stream_off(3);

    // to-do: content coupled with mResChangeWidth, mResChangeHeight
    // need to decouple them
    mResChangeWidth = rResChange.width;
    mResChangeHeight = rResChange.height;

    // part of InitVideoEncodeHW
    EncSettingEnc();

    // clear req buffers before set fmt
    ioctl_clear_req_bufs();

    ioctl_set_param(pParam);
    ioctl_set_fmt(pParam);
    ioctl_set_fix_scale(pParam);

    ioctl_req_bufs();
    ioctl_query_in_dmabuf();
    ioctl_query_out_dmabuf();
    ioctl_stream_on();
#endif

    OUT_FUNC();
    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::VT_SetScenario(bool on)
{
    IN_FUNC();

    if(!mIsViLTE && on)
    {
        mIsViLTE = true;
        //reduce ViLTE mem usage
        InputBufferHeadersResize(MTK_VENC_DEFAULT_INPUT_BUFFER_COUNT >> 1);
        OutputBufferHeadersResize(MTK_VENC_DEFAULT_OUTPUT_BUFFER_COUNT >> 1);

        MTK_OMX_LOGD("@@ ViLTE : reduce input/output buffer count to %d/%d", mInputPortDef.nBufferCountActual, mOutputPortDef.nBufferCountActual);
    }
    else if(mIsViLTE && !on)
    {
        mIsViLTE = false;
        //recover mem usage
        InputBufferHeadersResize(MTK_VENC_DEFAULT_INPUT_BUFFER_COUNT);
        OutputBufferHeadersResize(MTK_VENC_DEFAULT_OUTPUT_BUFFER_COUNT);

        MTK_OMX_LOGD("@@ ViLTE : recover input/output buffer count to %d/%d", mInputPortDef.nBufferCountActual, mOutputPortDef.nBufferCountActual);
    }

    OUT_FUNC();
    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::VT_GetResolutionChange(video_resolutionchange* pResolutionChange)
{
    venc_resolution_change rVencResolutionChange;
    IN_FUNC();
    if (!pResolutionChange)
        return OMX_ErrorNotReady;

    pResolutionChange->resolutionchange = 0;
    if (!ioctl_runtime_query(V4L2_CID_MPEG_MTK_RESOLUTION_CHANGE, (int*)&rVencResolutionChange)) {
        pResolutionChange->width = rVencResolutionChange.width;
        pResolutionChange->height = rVencResolutionChange.height;
        pResolutionChange->framerate = rVencResolutionChange.framerate;
        pResolutionChange->resolutionchange = rVencResolutionChange.resolutionchange;
        MTK_OMX_LOGD("@@ ViLTE : get resolutionchange w/h/bw/bh/reschange %d/%d/%d",
            pResolutionChange->width, pResolutionChange->height,
            pResolutionChange->resolutionchange);
    }else {
        MTK_OMX_LOGE("@@ ViLTE : get resolutionchange fail\n");
    }


    OUT_FUNC();
    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::VT_DumpOutputBuffer(OMX_U8 *aOutputBuf, OMX_U32 aOutputSize)
{
    IN_FUNC();

    if (mIsViLTE)
    {
        VAL_BOOL_T bDump = MtkVenc::DumpVtBs();
        char name[128];
        if (bDump)
        {
            snprintf(name, sizeof(name), "/sdcard/vilte_venc_bs_%d.bin", gettid());
            dumpBuffer(name, (unsigned char *)aOutputBuf, aOutputSize);
        }
    }

    OUT_FUNC();
    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::VT_BufferModifyBeforeEnc(OMX_BUFFERHEADERTYPE** ppInputBuf)
{
    IN_FUNC();
    if(!mIsViLTE) { OUT_FUNC(); return OMX_ErrorNotReady;}
    if (!ppInputBuf || !*ppInputBuf) return OMX_ErrorNotReady;
    OMX_BUFFERHEADERTYPE* pInputBuf = *ppInputBuf;

    // Resolution change check
    if(OMX_ErrorNone == VT_ResolutionChange(pInputBuf))
    {
        // resolution change happens, set Buffer to NULL to avoid Q_BUF
        *ppInputBuf = 0;
        OUT_FUNC();
        return OMX_ErrorNone;
    }

    // resolution change not happens, need to continue enqueu buffer
    // modify buffer info
    VAL_UINT32_T u4TimeDiff;
    VAL_UINT32_T u4AdjustedFrameRate;
    VAL_UINT32_T u4Count,u4Sum;
    const VAL_UINT32_T u4FrameRateUpperBound = 30;
    const VAL_UINT32_T u4FrameRateLowerBound = 10;

    if(!bDynamicFps) { OUT_FUNC(); return OMX_ErrorNotReady;}
    if(pInputBuf && pInputBuf->nTimeStamp < mLastQueuedFrameTimeStamp) { OUT_FUNC(); return OMX_ErrorNotReady;}

    u4TimeDiff = (VAL_UINT32_T)(pInputBuf->nTimeStamp - mLastQueuedFrameTimeStamp);

    if (u4TimeDiff > 260000)
    {
        MTK_OMX_LOGE("setDynamicFrameRate obnormal time diff timeStamp %lld lastframe timestamp %lld\n",pInputBuf->nTimeStamp,mLastQueuedFrameTimeStamp);
        MTK_OMX_LOGE("setDynamicFrameRate camera switch event detected by OMX\n");

        mInputTimeStampDiffs.clear();
        mInputTimeStampDiffs.setCapacity(MTK_VENC_TIMESTAMP_WINDOW_SIZE);
        OUT_FUNC(); return OMX_ErrorNotReady;
    }

    mInputTimeStampDiffs.push_front(u4TimeDiff);

    u4Sum = 0;
    u4Count = 0;

    for(Vector<VAL_UINT32_T>::iterator pDiff=mInputTimeStampDiffs.begin();
        pDiff != mInputTimeStampDiffs.end(); ++pDiff)
    {
        if(*pDiff != 0)
        {
            u4Sum += *pDiff;
            u4Count ++;
        }
    }

    if(u4Sum == 0 || u4Count == 0) return OMX_ErrorNotReady;

    u4AdjustedFrameRate = (1000000 + ((u4Sum/u4Count)/2))/(u4Sum/u4Count);

    if(u4AdjustedFrameRate > u4FrameRateUpperBound)
    {
        u4AdjustedFrameRate = u4FrameRateUpperBound;
    }
    if(u4AdjustedFrameRate < u4FrameRateLowerBound)
    {
        u4AdjustedFrameRate = u4FrameRateLowerBound;
    }

    // TODO: set to v4l2 set_ext DETECTED_FRAMERATE (input fps)
    ioctl_runtime_config(V4L2_CID_MPEG_MTK_ENCODE_DETECTED_FRAMERATE, u4AdjustedFrameRate);

    if (OMX_TRUE == mGotSLI)
    {
        mGotSLI = OMX_FALSE;
        OMX_U32 RFS_ErrFrm = mSLI.SliceLoss[0] & 0x3F;
        if (-1 == ioctl_runtime_config(V4L2_CID_MPEG_MTK_ENCODE_RFS_ON, RFS_ErrFrm))
        {
            MTK_OMX_LOGE("[ERROR] set qp %u fail", mQP);
        }

        return OMX_ErrorNone;
    }

    OUT_FUNC(); return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::VT_OnEnqueBuffer(OMX_BUFFERHEADERTYPE** ppInputBuf)
{
    if (!mIsViLTE) return OMX_ErrorNotReady;

    if(ppInputBuf && *ppInputBuf)
        mLastQueuedFrameTimeStamp = (*ppInputBuf)->nTimeStamp;

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::VT_SetMode()
{
    int nRet = 0;
    // if VILTE
    if (!mIsViLTE) return OMX_ErrorNone;

    nRet = ioctl_runtime_config(V4L2_CID_MPEG_MTK_ENCODE_SCENARIO, 5);
    if (-1 == nRet)
    {
        MTK_OMX_LOGE("[ERROR] set venc vilte scenario fail");
    }

    //Set Max width/height
    nRet = ioctl_runtime_config(V4L2_CID_MPEG_MTK_MAX_WIDTH, mInputPortDef.format.video.nFrameWidth);
    if (-1 == nRet)
    {
        MTK_OMX_LOGE("[ERROR] set max width fail");
    }

    nRet = ioctl_runtime_config(V4L2_CID_MPEG_MTK_MAX_HEIGHT, mInputPortDef.format.video.nFrameHeight );
    if (-1 == nRet)
    {
        MTK_OMX_LOGE("[ERROR] set max height fail");
    }

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::VT_SetConfig(OMX_INDEXTYPE nConfigIndex, OMX_PTR pCompConfig)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;

    switch (nConfigIndex)
    {
        case OMX_IndexConfigVideoBitrate:
            mPrepareToResolutionChange = OMX_TRUE;
            break;
        case OMX_IndexVendorMtkOmxVencSeResolutionChange:
            err = HandleSetConfigVendorResolutionChange((OMX_VIDEO_PARAM_RESOLUTION *) pCompConfig);
            break;
        case OMX_IndexVendorMtkOmxSliceLossIndication:
            err = HandleSetConfigVendorSliceLossIndication((OMX_CONFIG_SLICE_LOSS_INDICATION *) pCompConfig);
            break;
        default:
            err = OMX_ErrorUnsupportedIndex;
    }
    return err;
}

OMX_ERRORTYPE MtkOmxVenc::VT_SetParameter(OMX_INDEXTYPE nParamIndex, OMX_PTR pCompParam)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;

    switch (nParamIndex)
    {
        case OMX_IndexParamVideoBitrate:
            err = HandleVTSetVideoBitrate((OMX_VIDEO_PARAM_BITRATETYPE *)pCompParam);
            break;
        default:
            err = OMX_ErrorUnsupportedIndex;
    }
    return err;
}

struct ExtensionIndex {
    const OMX_STRING name;
    const OMX_INDEXTYPE index;
};

struct ExtensionIndex vtExt[] = {
    {"OMX.MTK.index.param.video.SlicelossIndication", OMX_IndexVendorMtkOmxSliceLossIndication}
};

OMX_ERRORTYPE MtkOmxVenc::VT_GetExtensionIndex(OMX_STRING parameterName, OMX_INDEXTYPE *pIndexType)
{
    int extCount = sizeof(vtExt)/sizeof(struct ExtensionIndex);
    for(int i=0; i<extCount; i++)
    {
        if(!strncmp(parameterName,vtExt[i].name,strlen(vtExt[i].name)))
        {
            *pIndexType = (OMX_INDEXTYPE) vtExt[i].index;
            return OMX_ErrorNone;
        }
    }
    return OMX_ErrorUnsupportedIndex;
}

OMX_ERRORTYPE MtkOmxVenc::VT_SetConfigSli(OMX_CONFIG_SLICE_LOSS_INDICATION* pSLI)
{
    mGotSLI = OMX_TRUE;

    MTK_OMX_LOGV_ENG("[SLI][%d][%d]start:%d, count:%d", pSLI->nSliceCount,
        pSLI->SliceLoss[0] & 0x3F,
        pSLI->SliceLoss[0] >> 19,
        ((pSLI->SliceLoss[0] >> 6) & 0x1FFF));

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::VT_SetConfigResolutionChange(OMX_VIDEO_PARAM_RESOLUTION* config)
{
#if 0
    if (mPrepareToResolutionChange == OMX_TRUE)
    {
        MTK_OMX_LOGE("resolution change error!!!");
        return OMX_ErrorUndefined;
    }

    if (u4EncodeWidth == config->nFrameWidth && u4EncodeHeight == config->nFrameHeight)
    {
        MTK_OMX_LOGV_ENG("MtkOmxVenc::SetConfig -> resolution change error, the same resolution %d, %d", config->nFrameWidth, config->nFrameHeight);
        return OMX_ErrorBadParameter;
    }
    else
    {
        mPrepareToResolutionChange = OMX_TRUE;
        u4EncodeWidth = config->nFrameWidth;
        u4EncodeHeight = config->nFrameHeight;

        MTK_OMX_LOGV_ENG("MtkOmxVenc::SetConfig -> resolution change to %d, %d", u4EncodeWidth, u4EncodeHeight);
    }
#endif
    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleVTSetVideoBitrate(OMX_VIDEO_PARAM_BITRATETYPE* pBitrateType)
{
    if(pBitrateType->eControlRate == OMX_Video_ControlRateConstantSkipFrames)
    {
        VT_SetScenario(true);
    }
    else
    {
        VT_SetScenario(false);
    }

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleSetConfigVendorSliceLossIndication(OMX_CONFIG_SLICE_LOSS_INDICATION* pSLI)
{
    memcpy(&mSLI, pSLI, sizeof(OMX_CONFIG_SLICE_LOSS_INDICATION));

    VT_SetConfigSli(pSLI);

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleSetConfigVendorResolutionChange(OMX_VIDEO_PARAM_RESOLUTION* config)
{
    return VT_SetConfigResolutionChange(config);
}
