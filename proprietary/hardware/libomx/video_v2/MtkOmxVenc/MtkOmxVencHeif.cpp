#include <sched.h>
#include <assert.h>
#include <signal.h>
#include <cutils/log.h>
#include <utils/Trace.h>
#include <cutils/properties.h>
#include "properties.h"

#include "osal_utils.h"
#include "MtkOmxVenc.h"
#include "MtkOmx2V4L2.h"
#include "OMX_IndexExt.h"

#include <semaphore.h>

#undef LOG_TAG
#define LOG_TAG "MtkOmxVencHeif"

#ifdef ATRACE_TAG
#undef ATRACE_TAG
#define ATRACE_TAG ATRACE_TAG_VIDEO
#endif//ATRACE_TAG

void MtkOmxVenc::MtkOmxVencHeif()
{
    IN_FUNC();
    mHeifOn = OMX_FALSE;
    mGridTotalCount = 0;
    mGridIndex = 0;
    memset(&mImageGrid, 0, sizeof(mImageGrid));

    OUT_FUNC();
}
void MtkOmxVenc::deMtkOmxVencHeif()
{
    IN_FUNC();

    OUT_FUNC();
}
void MtkOmxVenc::ComponentInitHeif()
{
    IN_FUNC();

    InitOMXParams(&mImageGrid);

    OUT_FUNC();
}
void MtkOmxVenc::ComponentDeInitHeif()
{
    IN_FUNC();

    OUT_FUNC();
}

// before stream on: InitHW
void MtkOmxVenc::Heif_OnInit(video_encode_param* param)
{
    IN_FUNC();

    INIT_SEMAPHORE(mHeifSem);

    mHeifOn = mImageGrid.bEnabled;
    mGridTotalCount = mImageGrid.nGridRows * mImageGrid.nGridCols;
    mGridOffset = 0;
    mInputCnt = 0;
    mOutputCnt = 0;

    MTK_OMX_LOGD_ENG("[%s] heif on:%d  total grid: %d",
        __func__, mHeifOn, mGridTotalCount);

    OUT_FUNC();
}

// before stream on: after InitHW, in case you need to set runtime config
OMX_ERRORTYPE MtkOmxVenc::Heif_SetMode()
{
    IN_FUNC();

    // Set grid mode
    if (mImageGrid.bEnabled == OMX_TRUE)
    {
        int heif_grid_size = (mImageGrid.nTileWidth << 16) + (mImageGrid.nTileHeight);
        MTK_OMX_LOGD_ENG("set grid w:%d h:%d heif_grid_size:%d",(heif_grid_size>>16),(heif_grid_size&0xffff),heif_grid_size);
        int nRet = ioctl_runtime_config(V4L2_CID_MPEG_MTK_ENCODE_GRID_SIZE, heif_grid_size);
        if (-1 == nRet)
        {
            MTK_OMX_LOGE("[ERROR] set grid size fail");
        }
    }

    OUT_FUNC();
    return OMX_ErrorNone;
}

// after stream off: DeInitHW
void MtkOmxVenc::Heif_OnDeInit()
{
    IN_FUNC();

    DESTROY_SEMAPHORE(mHeifSem);

    OUT_FUNC();
}

bool MtkOmxVenc::Heif_Enabled()
{
    return mHeifOn;
}

// At the end of HandleEmpty/FillThisBuffer
void MtkOmxVenc::Heif_EmptyThisBuffer(OMX_BUFFERHEADERTYPE* pInputBuf)
{
    if(!Heif_Enabled()) return;
    if(pInputBuf == NULL) return;

    IN_FUNC();

    OUT_FUNC();
}

void MtkOmxVenc::Heif_FillThisBuffer(OMX_BUFFERHEADERTYPE* pOutputBuf)
{
    if(!Heif_Enabled()) return;
    if(pOutputBuf == NULL) return;

    IN_FUNC();
    ATRACE_BEGIN("Heif_FillThisBuffer");

    // Heif_CollectBsBuffer() would only be called once since
    // we only encode single frame buffer. To trigger post processing
    // for grid spliting, we have to reserve output buffer and signal semephore here.
    int output_idx = DequeueBufferAdvance(&mFillThisBufQ);

    OMX_BUFFERHEADERTYPE* pOutputBufferHeaders = EncodeT_Output_GetBufferHeaderPtr(output_idx);
    VAL_UINT32_T mOutputBufferIndex = EncodeT_Output_GetBufferIndex(pOutputBufferHeaders);

    LOCK(mHeifOutputLock);
    mHeifOutputBufList.push(mOutputBufferIndex);
    UNLOCK(mHeifOutputLock);

    MTK_OMX_LOGD_ENG("[%s] SIGNAL mHeifSem",__func__);
    SIGNAL(mHeifSem);

    ATRACE_END();

    OUT_FUNC();
}

// before generating v4l2_buffer from omx buffer (may idendical to Heif_UpdateInputV4L2Buffer)
void MtkOmxVenc::Heif_BufferModifyBeforeEnc(OMX_BUFFERHEADERTYPE** ppInputBuf)
{
    if(!Heif_Enabled()) return;

    IN_FUNC();

    OUT_FUNC();
}

// before qbuf, set NULL to skip enque buffer to kernel
void MtkOmxVenc::Heif_CollectBsBuffer(OMX_BUFFERHEADERTYPE** ppOutputBuf)
{
    if(!Heif_Enabled()) return;
    if(ppOutputBuf == NULL || *ppOutputBuf == NULL) return;

    IN_FUNC();
    ATRACE_BEGIN("Heif_CollectBsBuffer");
    // Need to queue 2 bs buffer to kernel for header/frame.
    if (mInputCnt < 2)
    {
        mInputCnt++;
        ATRACE_END();
        return;
    }

    // Dequeue the reset N-2 bs buffer for later usage.
    // Remember set outputptr to NULL

    VAL_UINT32_T mOutputBufferIndex = EncodeT_Output_GetBufferIndex(*ppOutputBuf);

    LOCK(mHeifOutputLock);
    mHeifOutputBufList.push(mOutputBufferIndex);
    UNLOCK(mHeifOutputLock);

    MTK_OMX_LOGD_ENG("[%s] SIGNAL mHeifSem",__func__);
    SIGNAL(mHeifSem);

    *ppOutputBuf = NULL;
    ATRACE_END();
    OUT_FUNC();
}

// before qbuf: after v4l2_buffer is generated
void MtkOmxVenc::Heif_UpdateInputV4L2Buffer(OMX_BUFFERHEADERTYPE *pInputBuf, struct v4l2_buffer* qbuf)
{
    if(!Heif_Enabled()) return;
    if(pInputBuf == NULL || qbuf == NULL) return;
    if(pInputBuf->nFlags & OMX_BUFFERFLAG_EOS && pInputBuf->nFilledLen == 0) return;

    IN_FUNC();

#ifndef MTK_AutoGridMode
    if (mGridIndex < (mGridTotalCount-1)) // still need this YUV to encode next grid
    {
        // only necessary for 1st frame buffer
        qbuf->flags |= V4L2_BUF_FLAG_NO_CACHE_CLEAN;
        qbuf->flags |= V4L2_BUF_FLAG_NO_CACHE_INVALIDATE;
        MTK_OMX_LOGD_ENG("[%s] Marked as no clean/invalidate %d of %d",
            __func__, mGridIndex, mGridTotalCount);
    }
#endif

    OUT_FUNC();
}
void MtkOmxVenc::Heif_UpdateOutputV4L2Buffer(OMX_BUFFERHEADERTYPE *pOutputBuf, struct v4l2_buffer* qbuf)
{
    if(!Heif_Enabled()) return;
    if(pOutputBuf == NULL) return;
    if(qbuf == NULL) return;

    IN_FUNC();

    OUT_FUNC();
}

// after qbuf, before notify Deque thread (set *ppInputBuf to NULL to avoid EBD after dqbuf)
void MtkOmxVenc::Heif_OnEnqueBuffer(OMX_BUFFERHEADERTYPE** ppInputBuf)
{
    if(!Heif_Enabled()) return;
    if(ppInputBuf == NULL || *ppInputBuf == NULL) return;

    IN_FUNC();

#ifndef MTK_AutoGridMode
    if (mGridIndex < (mGridTotalCount-1)) // still need this YUV to encode next grid
    {
        QueueBufferAdvance(&mEmptyThisBufQ, *ppInputBuf);
        *ppInputBuf = 0; // Not going to deque
        mGridIndex++;
    }

    MTK_OMX_LOGD_ENG("[%s] GridIndex %d of %d",
        __func__, mGridIndex, mGridTotalCount);
#endif

    OUT_FUNC();
}

// after qbuf, before continue to enque any others bs buffer
void MtkOmxVenc::Heif_OnEnqueOutputBuffer(OMX_BUFFERHEADERTYPE** ppOutputBuf)
{
    if(!Heif_Enabled()) return;
    if(ppOutputBuf  == NULL || *ppOutputBuf == NULL) return;

    IN_FUNC();

    OUT_FUNC();
}

// after acquire deque buffer, before calling Empty/FillBufferDone
void MtkOmxVenc::Heif_DequeOnGetInputBuffer(OMX_BUFFERHEADERTYPE* pInputBuf, struct v4l2_buffer* dqbuf)
{
    if(!Heif_Enabled()) return;
    if(pInputBuf == NULL) return;
    if(dqbuf == NULL) return;

    IN_FUNC();

    OUT_FUNC();
}
void MtkOmxVenc::Heif_DequeOnGetOutputBuffer(OMX_BUFFERHEADERTYPE* pOutputBuf, struct v4l2_buffer* dqbuf)
{
    if(!Heif_Enabled()) return;
    if(pOutputBuf == NULL) return;
    if(dqbuf == NULL) return;
    ATRACE_BEGIN("Heif_DequeOutput");

    if (pOutputBuf->nFilledLen == 0)
    {
        MTK_OMX_LOGE("EOS! return all bs buffers to FTBQ.");
        // return all reserved bs buffer as EOS
        LOCK(mHeifOutputLock);
        while(!mHeifOutputBufList.empty())
        {
            int output_idx = mHeifOutputBufList[0];
            mHeifOutputBufList.removeAt(0);
            OMX_BUFFERHEADERTYPE* pHeifOutputBuf = EncodeT_Output_GetBufferHeaderPtr(output_idx);
            QueueBufferAdvance(&mFillThisBufQ, pHeifOutputBuf);
        }
        UNLOCK(mHeifOutputLock);
        ATRACE_END();
        return;
    }

    IN_FUNC();
//#ifdef MTK_AutoGridMode
    OMX_U32 GridCnt = 0;
    OMX_U32 GridNum = 0, GridSize = 0;
    OMX_U8 *buffer = pOutputBuf->pBuffer;
    OMX_BOOL done = OMX_FALSE;
    OMX_U32 *GridInfoStart = (OMX_U32*)(buffer + pOutputBuf->nFilledLen - MTK_GRID_INFO_SIZE);

    MTK_OMX_LOGD_ENG("buffer:0x%.8x GridInfoStart:0x%.8x FilledLen:%d nAllocLen:%d", buffer, GridInfoStart, pOutputBuf->nFilledLen, pOutputBuf->nAllocLen);

    if (!mOutputCnt) {
        MTK_OMX_LOGD_ENG("no need to handle header.");
        mOutputCnt++;
        ATRACE_END();
        return;
    }

    memcpy(&mGridInfo, GridInfoStart, MTK_GRID_INFO_SIZE);

{
    char value[PROPERTY_VALUE_MAX];
    OMX_U32 u4DumpGridInfo = 0;
    property_get("vendor.mtk.omx.venc.dumpgridinfo", value, "0");
    u4DumpGridInfo = atoi(value);
    if (u4DumpGridInfo)
    {
        for (int i=0; i < mGridTotalCount ;i++)
        {
            GridNum = mGridInfo[(i<<1)];
            GridSize = mGridInfo[(i<<1) + 1];
            MTK_OMX_LOGD_ENG("[%s] GridIndex %d of %d; GridNum:%d GridSize:0x%.8x(%d)",
                __func__, i, mGridTotalCount,GridNum,GridSize,GridSize);
        }
    }
}
    GridCnt = 0;
    while (!done)
    {
        // wait Heif output buffer sem
        int val = 0;
        GridSize = mGridInfo[(GridCnt<<1) + 1];

        sem_getvalue(&mHeifSem, &val);
        MTK_OMX_LOGD_ENG("[%s] semVal:%d mHeifOutputBufList size:%zu", __func__, val, mHeifOutputBufList.size());
        WAIT(mHeifSem);

        if (mHeifOutputBufList.isEmpty())
        {
            MTK_OMX_LOGD_ENG("[%s] mHeifOutputBufList is empty.", __func__);
            break;
        }

        // set specific grid from bs buffer
        LOCK(mHeifOutputLock);
        int output_idx = mHeifOutputBufList[0];
        mHeifOutputBufList.removeAt(0);
        UNLOCK(mHeifOutputLock);

        OMX_BUFFERHEADERTYPE* pHeifOutputBuf = EncodeT_Output_GetBufferHeaderPtr(output_idx);
        pHeifOutputBuf->pBuffer = pOutputBuf->pBuffer;
        pHeifOutputBuf->nOffset = mGridOffset;
        pHeifOutputBuf->nFilledLen = GridSize;
        pHeifOutputBuf->nTimeStamp = pOutputBuf->nTimeStamp;
        pHeifOutputBuf->nFlags = OMX_BUFFERFLAG_SYNCFRAME;

        mGridOffset += GridSize;
        GridCnt++;

        done = (OMX_BOOL)(GridCnt == mGridTotalCount);
        if (done) pHeifOutputBuf->nFlags |= OMX_BUFFERFLAG_ENDOFFRAME;

        MTK_OMX_LOGD_ENG("[%s] current %d Grid start addr: 0x%.8x Grid size: 0x%.8x(%d) flag:%X PTS:%lld",
            __func__, GridCnt, mGridOffset, GridSize, GridSize, pHeifOutputBuf->nFlags, pOutputBuf->nTimeStamp);

        // FBD
        HandleFillBufferDone(pHeifOutputBuf);

        MTK_OMX_LOGD_ENG("[%s] done:%d next %d Grid start addr:0x%.8x",
            __func__, done, GridCnt, mGridOffset);

    }

//#endif
    ATRACE_END();

    OUT_FUNC();
}

OMX_ERRORTYPE MtkOmxVenc::Heif_SetConfig(OMX_INDEXTYPE nConfigIndex, OMX_PTR pCompConfig)
{
    if(!Heif_Enabled()) return OMX_ErrorNotReady;

    OMX_ERRORTYPE err = OMX_ErrorNone;

    switch (nConfigIndex)
    {
        //case xxx:
        //    break;
        default:
            err = OMX_ErrorUnsupportedIndex;
    }
    return err;
}

OMX_ERRORTYPE MtkOmxVenc::Heif_GetConfig(OMX_INDEXTYPE nConfigIndex, OMX_PTR pCompConfig)
{
    if(!Heif_Enabled()) return OMX_ErrorNotReady;

    OMX_ERRORTYPE err = OMX_ErrorNone;

    switch (nConfigIndex)
    {
        //case xxx:
        //    break;
        default:
            err = OMX_ErrorUnsupportedIndex;
    }
    return err;
}

OMX_ERRORTYPE MtkOmxVenc::Heif_GetParameter(OMX_INDEXTYPE nParamIndex, OMX_PTR pCompParam)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;

    switch (nParamIndex)
    {
        case OMX_IndexParamVideoAndroidImageGrid:
            err = HandleGetVideoImgeGrid((OMX_VIDEO_PARAM_ANDROID_IMAGEGRIDTYPE *)pCompParam);
            break;
        default:
            err = OMX_ErrorUnsupportedIndex;
    }
    return err;
}

OMX_ERRORTYPE MtkOmxVenc::Heif_SetParameter(OMX_INDEXTYPE nParamIndex, OMX_PTR pCompParam)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;

    switch (nParamIndex)
    {
        case OMX_IndexParamVideoAndroidImageGrid:
            err = HandleSetVideoImgeGrid((OMX_VIDEO_PARAM_ANDROID_IMAGEGRIDTYPE *)pCompParam);
            break;
        default:
            err = OMX_ErrorUnsupportedIndex;
    }
    return err;
}

OMX_ERRORTYPE MtkOmxVenc::Heif_GetExtensionIndex(OMX_STRING parameterName, OMX_INDEXTYPE *pIndexType)
{
    return OMX_ErrorUnsupportedIndex;
}

OMX_ERRORTYPE MtkOmxVenc::HandleSetVideoImgeGrid(OMX_VIDEO_PARAM_ANDROID_IMAGEGRIDTYPE* pImageGrid)
{
    if ((pImageGrid->nPortIndex != mOutputPortFormat.nPortIndex) ||
        (mCodecId != MTK_VENC_CODEC_ID_HEIF))
    {
        return OMX_ErrorBadPortIndex;
    }

    MTK_OMX_LOGD("SetParameter ImageGrid Enable:%d TileWidth:%d TileHeight:%d GridRows:%d GridCols:%d",
        pImageGrid->bEnabled, pImageGrid->nTileWidth, pImageGrid->nTileHeight,
        pImageGrid->nGridRows, pImageGrid->nGridCols);

    memcpy(&mImageGrid, pImageGrid, sizeof(mImageGrid));

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::HandleGetVideoImgeGrid(OMX_VIDEO_PARAM_ANDROID_IMAGEGRIDTYPE *pImageGrid)
{
    if ((pImageGrid->nPortIndex != mOutputPortFormat.nPortIndex) ||
        (mCodecId != MTK_VENC_CODEC_ID_HEIF))
    {
        return OMX_ErrorBadPortIndex;
    }

    MTK_OMX_LOGD("GetParameter ImageGrid Enable:%d TileWidth:%d TileHeight:%d GridRows:%d GridCols:%d",
        mImageGrid.bEnabled, mImageGrid.nTileWidth, mImageGrid.nTileHeight,
        mImageGrid.nGridRows, mImageGrid.nGridCols);

    memcpy(pImageGrid, &mImageGrid, sizeof(mImageGrid));

    return OMX_ErrorNone;
}
