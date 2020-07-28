#include "DpMutex.h"
#include "DpVEncStream.h"
#include "DpStream.h"
#include "DpChannel.h"
#include "DpTimer.h"
#include "DpMemory.h"
#include "tpipe_config.h"
#ifndef BASIC_PACKAGE
#if CONFIG_FOR_OS_ANDROID
#include "PQSessionManager.h"
#endif // CONFIG_FOR_OS_ANDROID
#endif // BASIC_PACKAGE


/* Venc Modify + */
#if defined(MTK_SLOW_MOTION_HEVC_SUPPORT)
#include "val_types_public.h"
#include "venc_drv_if_public.h"
#include <cutils/properties.h>
#include "venc_drv_hevc_if.h"
#elif defined(MTK_SLOW_MOTION_H264_SUPPORT)
#include "val_types_public.h"
#include "venc_drv_if_public.h"
#include <cutils/properties.h>
#endif // MTK_SLOW_MOTION_HEVC_SUPPORT


#define VENC_ROUND_N(X, N)   (((X) + ((N)-1)) & (~((N)-1)))    //only for N is exponential of 2
/* Venc Modify - */

extern void VENCPostProc(DpVEnc_Config* pVEnc, uint32_t numReadBack, uint32_t* pReadBack);

static uint32_t s_PqCount = 0;
static DpMutex  s_PqCountMutex;

DpVEncStream::DpVEncStream(ISPStreamType type)
    : m_streamType(type),
      m_frameChange(false),
      m_pStream(NULL),
      m_pChannel(new DpChannel()),
      m_channelID(-1),
      m_pListMutex(new DpMutex()),
      m_pBufferMutex(new DpMutex()),
      m_pFrameMutex(new DpMutex()),
      m_pSrcPool(new DpBasicBufferPool()),
      m_srcFormat(DP_COLOR_RGB888),
      m_srcWidth(-1),
      m_srcHeight(-1),
      m_srcYPitch(-1),
      m_srcUVPitch(-1),
      m_srcFlush(true),
      m_srcSecure(DP_SECURE_NONE),
      m_srcProfile(DP_PROFILE_FULL_BT601),
      m_jpegEnc_filesize(0),
      m_configFrameCount(0),
      m_dequeueFrameCount(0),
      m_encodeFrameWidth(0),
      m_encodeFrameHeight(0),
      m_numPABuffer(0)
{
    int32_t index;

    switch(m_streamType)
    {
        case ISP_CC_STREAM:
        case ISP_IC_STREAM:
            m_pStream = new DpStream(STREAM_ISP_VR);
            m_pStream->setSyncMode(false);
            break;
        case ISP_ZSD_STREAM:
            m_pStream = new DpStream(STREAM_ISP_ZSD);
            m_pStream->setSyncMode(false);
            break;
        case ISP_VSS_STREAM:
            m_pStream = new DpStream(STREAM_ISP_VSS);
            m_pStream->setSyncMode(true);
            break;
        default:
            assert(0);
            break;
    }

    assert(NULL != m_pStream);

#ifndef BASIC_PACKAGE
    m_pqSupport = DpDriver::getInstance()->getPQSupport();
#endif

    m_pBufferMutex->lock();
    for (index = 0; index < ISP_MAX_OUTPUT_PORT_NUM; index++)
    {
        m_pDstPool[index]     = new DpBasicBufferPool();
        m_dstFormat[index]    = DP_COLOR_RGB888;
        m_dstPlane[index]     = -1;
        m_dstWidth[index]     = -1;
        m_dstHeight[index]    = -1;
        m_dstYPitch[index]    = -1;
        m_dstUVPitch[index]   = -1;
        m_rotation[index]     = 0;
        m_flipStatus[index]   = false;
        m_sharpness[index]    = 0;
        m_ditherStatus[index] = false;
        m_dstEnable[index]    = false;
        m_dstWasEnabled[index] = false;
        m_dstPortType[index]  = 0;
        m_dstFlush[index]     = true;
        m_dstProfile[index]   = DP_PROFILE_BT601;

        m_dstBufferList[index].clear();

        m_multiSrcXStart[index]    = 0;
        m_multiSrcXSubpixel[index] = 0;
        m_multiSrcYStart[index]    = 0;
        m_multiSrcYSubpixel[index] = 0;
        m_multiCropWidth[index]    = 0;
        m_multiCropHeight[index]   = 0;
        m_multiCropChange[index]   = false;

#ifndef BASIC_PACKAGE
        if (0 != m_pqSupport) {
            m_PqID[index] = getPqID();
        }
#endif // BASIC_PACKAGE

        m_pDstMatrix[index] = NULL;
    }
    m_srcBufferList.clear();
    m_srcBufferStartTimeList.clear();
    m_pBufferMutex->unlock();

    memset(&m_VEnc_cfg, 0, sizeof(m_VEnc_cfg));
    memset(m_PABuffer, 0, sizeof(m_PABuffer));
    memset(m_ValueBuffer, 0, sizeof(m_ValueBuffer));
    memset(m_yuvBuffer, 0, sizeof(m_yuvBuffer));

    m_pListMutex->lock();
    m_frameDoneList.clear();
    m_jobIDList.clear();
    m_pListMutex->unlock();

    memset(m_PqConfig, 0, sizeof(m_PqConfig));
    memset(m_PqParam, 0, sizeof(m_PqParam));
}


DpVEncStream::~DpVEncStream()
{
    int32_t index;

    m_pBufferMutex->lock();
    m_srcBufferList.clear();
    m_srcBufferStartTimeList.clear();
    for (index = 0; index < ISP_MAX_OUTPUT_PORT_NUM; index++)
    {
        delete m_pDstPool[index];
        m_pDstPool[index] = NULL;

        m_dstBufferList[index].clear();

#ifndef BASIC_PACKAGE
        if (0 != m_pqSupport) {
#if CONFIG_FOR_OS_ANDROID
            PQSessionManager::getInstance()->destroyPQSession(m_PqID[index]);
#endif // CONFIG_FOR_OS_ANDROID
        }
#endif // BASIC_PACKAGE

        delete m_pDstMatrix[index];
        m_pDstMatrix[index] = NULL;
    }
    m_pBufferMutex->unlock();

    m_pListMutex->lock();
    m_frameDoneList.clear();
    m_jobIDList.clear();
    m_pListMutex->unlock();

    delete m_pSrcPool;
    m_pSrcPool = NULL;

    delete m_pStream;
    m_pStream = NULL;

    delete m_pChannel;
    m_pChannel = NULL;

    delete m_pFrameMutex;
    m_pFrameMutex = NULL;

    delete m_pListMutex;
    m_pListMutex = NULL;

    delete m_pBufferMutex;
    m_pBufferMutex = NULL;

}


DP_STATUS_ENUM DpVEncStream::queueSrcBuffer(void     *pVA,
                                            uint32_t size)
{
    DP_TRACE_CALL();
    DP_STATUS_ENUM status;
    int32_t        curSourceBuffer;
    DpTimeValue    initTime;

    DPLOGI("==============queue src buffer begin:==============\n");

    if (NULL == pVA)
    {
        DPLOGE("DpVEncStream: invalid buffer base address");
        return DP_STATUS_INVALID_PARAX;
    }

#if 0
    if(-1 != m_srcBuffer)
    {
        m_pSrcPool->unregisterBuffer(m_srcBuffer);
        m_srcBuffer = -1;
    }
#endif

    status = m_pSrcPool->registerBuffer(&pVA,
                                        &size,
                                        1,
                                        -1,
                                        &curSourceBuffer);

    if ((-1 == curSourceBuffer) || (status != DP_STATUS_RETURN_SUCCESS))
    {
        DPLOGE("DpVEnc: register src buffer failed\n");
        return DP_STATUS_OPERATION_FAILED;
    }

    m_pBufferMutex->lock();
    m_srcBufferList.push_back(curSourceBuffer);
    DP_TIMER_GET_CURRENT_TIME(initTime)
    m_srcBufferStartTimeList.push_back(initTime);
    m_pBufferMutex->unlock();

    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpVEncStream: queue src buffer failed(%d)\n", status);
        return status;
    }

    DPLOGI("==============queue src buffer end:==============\n");

    return status;
}


DP_STATUS_ENUM DpVEncStream::queueSrcBuffer(void     *pVA,
                                            uint32_t MVA,
                                            uint32_t size)
{
    DP_TRACE_CALL();
    DP_STATUS_ENUM status;
    int32_t        curSourceBuffer;
    DpTimeValue    initTime;

    DPLOGI("==============queue src buffer begin:==============\n");

    if ((NULL == pVA) || (0 == MVA))
    {
        DPLOGE("DpVEncStream: invalid buffer base address\n");
        return DP_STATUS_INVALID_PARAX;
    }

#if 0
    if(-1 != m_srcBuffer)
    {
        m_pSrcPool->unregisterBuffer(m_srcBuffer);
        m_srcBuffer = -1;
    }
#endif

    status = m_pSrcPool->registerBuffer(&pVA,
                                        &MVA,
                                        &size,
                                        1,
                                        -1,
                                        &curSourceBuffer);

    if ((-1 == curSourceBuffer) || (status != DP_STATUS_RETURN_SUCCESS))
    {
        DPLOGE("DpVEnc: register src buffer failed\n");
        return DP_STATUS_OPERATION_FAILED;
    }

    m_pBufferMutex->lock();
    m_srcBufferList.push_back(curSourceBuffer);
    DP_TIMER_GET_CURRENT_TIME(initTime)
    m_srcBufferStartTimeList.push_back(initTime);
    m_pBufferMutex->unlock();

    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpVEncStream: queue src buffer failed(%d)\n", status);
        return status;
    }

    DPLOGI("==============queue src buffer end:==============\n");

    return status;
}


DP_STATUS_ENUM DpVEncStream::queueSrcBuffer(void     **pVAList,
                                            uint32_t *pSizeList,
                                            int32_t  planeNum)
{
    DP_STATUS_ENUM status;
    int32_t        curSourceBuffer;
    DpTimeValue    initTime;

    DPLOGI("==============queue src buffer begin:==============\n");

    if((planeNum < 0) || (planeNum > 3))
    {
        DPLOGE("DpVEncStream: error argument - planceNumber: %d (should <= 3)\n", planeNum);
        return DP_STATUS_INVALID_PARAX;
    }

#if 0
    if(-1 != m_srcBuffer)
    {
        m_pSrcPool->unregisterBuffer(m_srcBuffer);
        m_srcBuffer = -1;
    }
#endif

    status = m_pSrcPool->registerBuffer(pVAList,
                                        pSizeList,
                                        planeNum,
                                        -1,
                                        &curSourceBuffer);

    if ((-1 == curSourceBuffer) || (status != DP_STATUS_RETURN_SUCCESS))
    {
        DPLOGE("DpVEnc: register src buffer failed\n");
        return DP_STATUS_OPERATION_FAILED;
    }

    m_pBufferMutex->lock();
    m_srcBufferList.push_back(curSourceBuffer);
    DP_TIMER_GET_CURRENT_TIME(initTime)
    m_srcBufferStartTimeList.push_back(initTime);
    m_pBufferMutex->unlock();

    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpVEncStream: queue src buffer failed(%d)\n", status);
        return status;
    }

    DPLOGI("==============queue src buffer end:==============\n");

    return status;
}


DP_STATUS_ENUM DpVEncStream::queueSrcBuffer(uint32_t *pMVAList,
                                            uint32_t *pSizeList,
                                            int32_t  planeNum)
{
    DP_STATUS_ENUM status;
    int32_t        curSourceBuffer;
    DpTimeValue    initTime;

    DPLOGI("==============queue src buffer begin:==============\n");

    if((planeNum < 0) || (planeNum > 3))
    {
        DPLOGE("DpVEncStream: error argument - planceNumber: %d (should <= 3)\n", planeNum);
        return DP_STATUS_INVALID_PARAX;
    }

#if 0
    if(-1 != m_srcBuffer)
    {
        m_pSrcPool->unregisterBuffer(m_srcBuffer);
        m_srcBuffer = -1;
    }
#endif

    status = m_pSrcPool->registerBuffer(pMVAList,
                                        pSizeList,
                                        planeNum,
                                        -1,
                                        &curSourceBuffer);

    if ((-1 == curSourceBuffer) || (status != DP_STATUS_RETURN_SUCCESS))
    {
        DPLOGE("DpVEnc: register src buffer failed\n");
        return DP_STATUS_OPERATION_FAILED;
    }

    m_pBufferMutex->lock();
    m_srcBufferList.push_back(curSourceBuffer);
    DP_TIMER_GET_CURRENT_TIME(initTime)
    m_srcBufferStartTimeList.push_back(initTime);
    m_pBufferMutex->unlock();

    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpVEncStream: queue src buffer failed(%d)\n", status);
        return status;
    }

    DPLOGI("==============queue src buffer end:==============\n");

    return status;
}


DP_STATUS_ENUM DpVEncStream::queueSrcBuffer(void     **pVAList,
                                            uint32_t *pMVAList,
                                            uint32_t *pSizeList,
                                            int32_t  planeNum)
{
    DP_STATUS_ENUM status;
    int32_t        curSourceBuffer;
    DpTimeValue    initTime;

    DPLOGI("==============queue src buffer begin:==============\n");

    if((planeNum < 0) || (planeNum > 3))
    {
        DPLOGE("DpVEncStream: error argument - planceNumber: %d (should <= 3)\n", planeNum);
        return DP_STATUS_INVALID_PARAX;
    }

#if 0
    if(-1 != m_srcBuffer)
    {
        m_pSrcPool->unregisterBuffer(m_srcBuffer);
        m_srcBuffer = -1;
    }
#endif

    status = m_pSrcPool->registerBuffer(pVAList,
                                        pMVAList,
                                        pSizeList,
                                        planeNum,
                                        -1,
                                        &curSourceBuffer);

    if ((-1 == curSourceBuffer) || (status != DP_STATUS_RETURN_SUCCESS))
    {
        DPLOGE("DpVEnc: register src buffer failed\n");
        return DP_STATUS_OPERATION_FAILED;
    }

    m_pBufferMutex->lock();
    m_srcBufferList.push_back(curSourceBuffer);
    DP_TIMER_GET_CURRENT_TIME(initTime)
    m_srcBufferStartTimeList.push_back(initTime);
    m_pBufferMutex->unlock();

    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpVEncStream: queue src buffer failed(%d)\n", status);
        return status;
    }

    DPLOGI("==============queue src buffer end:==============\n");

    return status;
}


DP_STATUS_ENUM DpVEncStream::queueSrcBuffer(int32_t  fileDesc,
                                            uint32_t *pSizeList,
                                            uint32_t planeNum)
{

    DP_STATUS_ENUM status;
    int32_t        curSourceBuffer;
    DpTimeValue    initTime;

    DPLOGI("==============queue src buffer begin:==============\n");

    if((planeNum < 0) || (planeNum > 3))
    {
        DPLOGE("DpVEncStream: error argument - planceNumber: %d (should <= 3)\n", planeNum);
        return DP_STATUS_INVALID_PARAX;
    }

#if 0
    if(-1 != m_srcBuffer)
    {
        m_pSrcPool->unregisterBuffer(m_srcBuffer);
        m_srcBuffer = -1;
    }
#endif

    status = m_pSrcPool->registerBufferFD(fileDesc,
                                          pSizeList,
                                          planeNum,
                                          -1,
                                          &curSourceBuffer);

    if ((-1 == curSourceBuffer) || (status != DP_STATUS_RETURN_SUCCESS))
    {
        DPLOGE("DpVEnc: register src buffer failed\n");
        return DP_STATUS_OPERATION_FAILED;
    }

    m_pBufferMutex->lock();
    m_srcBufferList.push_back(curSourceBuffer);
    DP_TIMER_GET_CURRENT_TIME(initTime)
    m_srcBufferStartTimeList.push_back(initTime);
    m_pBufferMutex->unlock();

    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpVEncStream: queue src buffer failed(%d)\n", status);
        return status;
    }

    DPLOGI("==============queue src buffer end:==============\n");

    return status;
}


DP_STATUS_ENUM DpVEncStream::dequeueSrcBuffer()
{
    DP_TRACE_CALL();
    DP_STATUS_ENUM status;
    DpTimeValue    begin;
    DpTimeValue    end;
    int32_t        diff;
    int32_t        curSourceBuffer;

    DPLOGI("==============dequeue src buffer begin:==============\n");

    m_pBufferMutex->lock();
    curSourceBuffer = m_srcBufferList.front();
    begin = m_srcBufferStartTimeList.front();
    m_srcBufferList.erase(m_srcBufferList.begin());
    m_srcBufferStartTimeList.erase(m_srcBufferStartTimeList.begin());
    m_pBufferMutex->unlock();

    if (-1 == curSourceBuffer)
    {
        DPLOGE("DpVEncStream: empty src buffer of port\n");
        return DP_STATUS_INVALID_BUFFER;
    }

    status = waitComplete();
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpVEncStream: wait stream failed(%d)\n", status);
        m_pStream->stopStream();

        return status;
    }

    if(false == m_pStream->getSyncMode()) //Async mode!
    {
        status = m_pSrcPool->asyncReleaseBuffer(curSourceBuffer);
        DPLOGI("DpVEncStream:SrcBuffer releaseBuffer %d status: %d \n", curSourceBuffer, status);
    }

    m_pSrcPool->unregisterBuffer(curSourceBuffer);

    DP_TIMER_GET_CURRENT_TIME(end);

    DP_TIMER_GET_DURATION_IN_MS(begin,
                                end,
                                diff);

    if (diff > 30)
    {
        DPLOGW("DpVEnc: %p, src %d ms\n", this, diff);
    }
    else
    {
        DPLOGI("DpVEnc: %p, src %d ms\n", this, diff);
    }

    DPLOGI("==============dequeue src buffer end:==============\n");

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpVEncStream::setSrcConfig(DpColorFormat srcFormat,
                                          int32_t       srcWidth,
                                          int32_t       srcHeight,
                                          int32_t       srcPitch,
                                          bool          doFlush)
{
    DP_TRACE_CALL();

    if ((srcWidth  <= 0) ||
        (srcHeight <= 0) ||
        (srcPitch  <= 0))
    {
        DPLOGE("DpVEncStream: invalid source width(%d), height(%d) or pitch(%d)\n", srcWidth, srcHeight, srcPitch);
        return DP_STATUS_INVALID_PARAX;
    }

    if (srcPitch < ((srcWidth * DP_COLOR_BITS_PER_PIXEL(srcFormat)) >> 3))
    {
        DPLOGE("DpVEncStream: pitch(%d) is less than width(%d)\n", srcPitch, srcWidth);
        return DP_STATUS_INVALID_PARAX;
    }

    // check for frame change
    if ((m_srcFormat != srcFormat) ||
        (m_srcWidth  != srcWidth) ||
        (m_srcHeight != srcHeight) ||
        (m_srcYPitch != srcPitch) ||
        (m_srcFlush  != doFlush))
    {
        m_frameChange = true;
    }

    m_srcFormat  = srcFormat;
    m_srcWidth   = srcWidth;
    m_srcHeight  = srcHeight;
    m_srcYPitch  = srcPitch;
    m_srcUVPitch = 0;
    m_srcFlush   = doFlush;
    m_srcProfile = DP_PROFILE_FULL_BT601;

    DPLOGI("src flush status %d\n", doFlush);

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpVEncStream::setSrcConfig(int32_t           width,
                                          int32_t           height,
                                          int32_t           YPitch,
                                          int32_t           UVPitch,
                                          DpColorFormat     format,
                                          DP_PROFILE_ENUM   profile,
                                          DpInterlaceFormat,
                                          DpRect*,
                                          bool              doFlush,
                                          DpSecure          secure)
{
    DP_TRACE_CALL();

    if ((width  <= 0) ||
        (height <= 0) ||
        (YPitch <= 0))
    {
        DPLOGE("DpVEncStream: invalid source width(%d), height(%d) or pitch(%d)\n", width, height, YPitch);
        return DP_STATUS_INVALID_PARAX;
    }

    if (YPitch < ((width * DP_COLOR_BITS_PER_PIXEL(format)) >> 3))
    {
        DPLOGE("DpVEncStream: pitch(%d) is less than width(%d)\n", YPitch, width);
        return DP_STATUS_INVALID_PARAX;
    }

    // check for frame change
    if ((m_srcFormat  != format) ||
        (m_srcWidth   != width) ||
        (m_srcHeight  != height) ||
        (m_srcYPitch  != YPitch) ||
        (m_srcUVPitch != UVPitch) ||
        (m_srcProfile != profile) ||
        (m_srcFlush   != doFlush))
    {
        m_frameChange = true;
    }

    m_srcFormat  = format;
    m_srcWidth   = width;
    m_srcHeight  = height;
    m_srcYPitch  = YPitch;
    m_srcUVPitch = UVPitch;
    m_srcFlush   = doFlush;
    m_srcProfile = profile;

    DPLOGI("src profile (%d) flush status %d\n", profile, doFlush);

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpVEncStream::setSrcCrop(int32_t XCropStart,
                                        int32_t XSubpixel,
                                        int32_t YCropStart,
                                        int32_t YSubpixel,
                                        int32_t cropWidth,
                                        int32_t cropHeight)
{
    DP_TRACE_CALL();

    for (int i = 0; i < ISP_MAX_OUTPUT_PORT_NUM; i++)
    {
        if ((m_multiSrcXStart[i]    != XCropStart) ||
            (m_multiSrcXSubpixel[i] != XSubpixel)  ||
            (m_multiSrcYStart[i]    != YCropStart) ||
            (m_multiSrcYSubpixel[i] != YSubpixel)  ||
            (m_multiCropWidth[i]    != cropWidth)  ||
            (m_multiCropHeight[i]   != cropHeight))
        {
            m_multiCropChange[i] = true;
            m_frameChange = true;
        }

        m_multiSrcXStart[i]    = XCropStart;
        m_multiSrcXSubpixel[i] = XSubpixel;
        m_multiSrcYStart[i]    = YCropStart;
        m_multiSrcYSubpixel[i] = YSubpixel;
        m_multiCropWidth[i]    = cropWidth;
        m_multiCropHeight[i]   = cropHeight;
    }

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpVEncStream::setSrcCrop(int32_t portIndex,
                                        int32_t XCropStart,
                                        int32_t XSubpixel,
                                        int32_t YCropStart,
                                        int32_t YSubpixel,
                                        int32_t cropWidth,
                                        int32_t cropHeight)
{
    DP_TRACE_CALL();

    if (portIndex >= ISP_MAX_OUTPUT_PORT_NUM)
        return DP_STATUS_INVALID_PORT;

    if ((m_multiSrcXStart[portIndex]    != XCropStart) ||
        (m_multiSrcXSubpixel[portIndex] != XSubpixel)  ||
        (m_multiSrcYStart[portIndex]    != YCropStart) ||
        (m_multiSrcYSubpixel[portIndex] != YSubpixel)  ||
        (m_multiCropWidth[portIndex]    != cropWidth)  ||
        (m_multiCropHeight[portIndex]   != cropHeight))
    {
        m_multiCropChange[portIndex] = true;
        m_frameChange = true;
    }

    m_multiSrcXStart[portIndex]    = XCropStart;
    m_multiSrcXSubpixel[portIndex] = XSubpixel;
    m_multiSrcYStart[portIndex]    = YCropStart;
    m_multiSrcYSubpixel[portIndex] = YSubpixel;
    m_multiCropWidth[portIndex]    = cropWidth;
    m_multiCropHeight[portIndex]   = cropHeight;

    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpVEncStream::setSrcCrop(int32_t portIndex,
                                        int32_t XStart,
                                        int32_t XSubpixel,
                                        int32_t YStart,
                                        int32_t YSubpixel,
                                        int32_t cropWidth,
                                        int32_t cropWidthSubpixel,
                                        int32_t cropHeight,
                                        int32_t cropHeightSubpixel)
{
    return setSrcCrop(portIndex, XStart, XSubpixel, YStart, YSubpixel, cropWidth, cropHeight);
}

DP_STATUS_ENUM DpVEncStream::queueDstBuffer(int32_t  portIndex,
                                            void     **pVAList,
                                            uint32_t *pSizeList,
                                            int32_t  planeNum)
{
    DP_TRACE_CALL();
    int32_t dstBuffer = -1;
    DP_STATUS_ENUM status;

    DPLOGI("==============queue dst buffer begin:==============\n");

    if((planeNum < 0) || (planeNum > 3))
    {
        DPLOGE("DpVEncStream: error argument - planceNumber: %d (should <= 3)\n", planeNum);
        return DP_STATUS_INVALID_PARAX;
    }

    if (portIndex >= ISP_MAX_OUTPUT_PORT_NUM)
    {
        DPLOGE("DpVEncStream: error argument - invalid output port index: %d\n", portIndex);
        return DP_STATUS_INVALID_PORT;
    }

#if 0
    if(-1 != m_dstBuffer[portIndex])
    {
        m_pDstPool[portIndex]->cancelBuffer(m_dstBuffer[portIndex]);
        m_pDstPool[portIndex]->unregisterBuffer(m_dstBuffer[portIndex]);
        m_dstBuffer[portIndex] = -1;
    }
#endif

    status = m_pDstPool[portIndex]->registerBuffer(pVAList,
                                                   pSizeList,
                                                   planeNum,
                                                   -1,
                                                   &dstBuffer);

    m_pBufferMutex->lock();
    m_dstBufferList[portIndex].push_back(dstBuffer);
    m_pBufferMutex->unlock();

    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpVEncStream: queue dst%d buffer failed(%d)\n", portIndex, status);
        return status;
    }

    m_dstPlane[portIndex] = planeNum;

    DPLOGI("==============queue dst buffer end:==============\n");

    return status;
}


DP_STATUS_ENUM DpVEncStream::queueDstBuffer(int32_t  portIndex,
                                            int32_t  fileDesc,
                                            uint32_t *pSizeList,
                                            int32_t  planeNum)
{
    DP_TRACE_CALL();
    int32_t dstBuffer = -1;
    DP_STATUS_ENUM status;

    DPLOGI("==============queue dst buffer begin:==============\n");

    if((planeNum < 0) || (planeNum > 3))
    {
        DPLOGE("DpVEncStream: error argument - planceNumber: %d (should <= 3)\n", planeNum);
        return DP_STATUS_INVALID_PARAX;
    }

    if (portIndex >= ISP_MAX_OUTPUT_PORT_NUM)
    {
        DPLOGE("DpVEncStream: error argument - invalid output port index: %d\n", portIndex);
        return DP_STATUS_INVALID_PORT;
    }

#if 0
    if(-1 != m_dstBuffer[portIndex])
    {
        m_pDstPool[portIndex]->cancelBuffer(m_dstBuffer[portIndex]);
        m_pDstPool[portIndex]->unregisterBuffer(m_dstBuffer[portIndex]);
        m_dstBuffer[portIndex] = -1;
    }
#endif

    status = m_pDstPool[portIndex]->registerBufferFD(fileDesc,
                                                     pSizeList,
                                                     planeNum,
                                                     -1,
                                                     &dstBuffer);

    m_pBufferMutex->lock();
    m_dstBufferList[portIndex].push_back(dstBuffer);
    m_pBufferMutex->unlock();

    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpVEncStream: queue dst%d buffer failed(%d)\n", portIndex, status);
        return status;
    }

    m_dstPlane[portIndex] = planeNum;

    DPLOGI("==============queue dst buffer end:==============\n");

    return status;
}


DP_STATUS_ENUM DpVEncStream::queueDstBuffer(int32_t  portIndex,
                                            uint32_t *pMVAList,
                                            uint32_t *pSizeList,
                                            int32_t  planeNum)
{
    DP_TRACE_CALL();
    int32_t dstBuffer = -1;
    DP_STATUS_ENUM status;

    DPLOGI("==============queue dst buffer begin:==============\n");

    if((planeNum < 0) || (planeNum > 3))
    {
        DPLOGE("DpVEncStream: error argument - planceNumber: %d (should <= 3)\n", planeNum);
        return DP_STATUS_INVALID_PARAX;
    }

    if (portIndex >= ISP_MAX_OUTPUT_PORT_NUM)
    {
        DPLOGE("DpVEncStream: error argument - invalid output port index: %d\n", portIndex);
        return DP_STATUS_INVALID_PORT;
    }
#if 0
    if(-1 != m_dstBuffer[portIndex])
    {
        m_pDstPool[portIndex]->cancelBuffer(m_dstBuffer[portIndex]);
        m_pDstPool[portIndex]->unregisterBuffer(m_dstBuffer[portIndex]);
        m_dstBuffer[portIndex] = -1;
    }
#endif

    status = m_pDstPool[portIndex]->registerBuffer(pMVAList,
                                                   pSizeList,
                                                   planeNum,
                                                   -1,
                                                   &dstBuffer);

    m_pBufferMutex->lock();
    m_dstBufferList[portIndex].push_back(dstBuffer);
    m_pBufferMutex->unlock();

    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpVEncStream: queue dst%d buffer failed(%d)\n", portIndex, status);
        return status;
    }

    m_dstPlane[portIndex] = planeNum;

    DPLOGI("==============queue dst buffer end:%d ==============\n",dstBuffer);

    return status;
}


DP_STATUS_ENUM DpVEncStream::queueDstBuffer(int32_t  portIndex,
                                            void     **pVAList,
                                            uint32_t *pMVAList,
                                            uint32_t *pSizeList,
                                            int32_t  planeNum)
{
    DP_TRACE_CALL();
    int32_t dstBuffer = -1;
    DP_STATUS_ENUM status;

    DPLOGI("==============queue dst buffer begin:==============\n");

    if((planeNum < 0) || (planeNum > 3))
    {
        DPLOGE("DpVEncStream: error argument - planceNumber: %d (should <= 3)\n", planeNum);
        return DP_STATUS_INVALID_PARAX;
    }

    if (portIndex >= ISP_MAX_OUTPUT_PORT_NUM)
    {
        DPLOGE("DpVEncStream: error argument - invalid output port index: %d\n", portIndex);
        return DP_STATUS_INVALID_PORT;
    }
#if 0
    if(-1 != m_dstBuffer[portIndex])
    {
        m_pDstPool[portIndex]->cancelBuffer(m_dstBuffer[portIndex]);
        m_pDstPool[portIndex]->unregisterBuffer(m_dstBuffer[portIndex]);
        m_dstBuffer[portIndex] = -1;
    }
#endif

    status = m_pDstPool[portIndex]->registerBuffer(pVAList,
                                                   pMVAList,
                                                   pSizeList,
                                                   planeNum,
                                                   -1,
                                                   &dstBuffer);

    m_pBufferMutex->lock();
    m_dstBufferList[portIndex].push_back(dstBuffer);
    m_pBufferMutex->unlock();

    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpVEncStream: queue dst%d buffer failed(%d)\n", portIndex, status);
        return status;
    }

    m_dstPlane[portIndex] = planeNum;

    DPLOGI("==============queue dst buffer end:%d ==============\n",dstBuffer);

    return status;
}


DP_STATUS_ENUM DpVEncStream::setDstConfig(int32_t       portIndex,
                                          DpColorFormat dstFormat,
                                          int32_t       dstWidth,
                                          int32_t       dstHeight,
                                          int32_t       dstPitch,
                                          bool          doFlush)
{
    DP_TRACE_CALL();

    if ((dstWidth  <= 0) ||
        (dstHeight <= 0) ||
        (dstPitch  <= 0))
    {
        DPLOGE("DpVEncStream: error argument - width(%d), height(%d) or pitch(%d)\n", dstWidth, dstHeight, dstPitch);
        return DP_STATUS_INVALID_PARAX;
    }

    if (portIndex >= ISP_MAX_OUTPUT_PORT_NUM)
    {
        DPLOGE("DpVEncStream: error argument - invalid output port index: %d\n", portIndex);
        return DP_STATUS_INVALID_PORT;
    }

    if (dstPitch < ((dstWidth * DP_COLOR_BITS_PER_PIXEL(dstFormat)) >> 3))
    {
        DPLOGE("DpVEncStream: pitch(%d) is less than width(%d)\n", dstPitch, dstWidth);
        return DP_STATUS_INVALID_PARAX;
    }

    // check for frame change
    if ((m_dstFormat[portIndex] != dstFormat) ||
        (m_dstWidth[portIndex]  != dstWidth) ||
        (m_dstHeight[portIndex] != dstHeight) ||
        (m_dstYPitch[portIndex] != dstPitch) ||
        (m_dstFlush[portIndex]  != doFlush))
    {
        m_frameChange = true;
    }

    m_dstFormat[portIndex]  = dstFormat;
    m_dstWidth[portIndex]   = dstWidth;
    m_dstHeight[portIndex]  = dstHeight;
    m_dstYPitch[portIndex]  = dstPitch;
    m_dstUVPitch[portIndex] = 0;
    m_dstEnable[portIndex]  = true;
    m_dstFlush[portIndex]   = doFlush;
    m_dstProfile[portIndex] = DP_PROFILE_BT601;

    DPLOGI("dst %d flush status %d\n", portIndex, doFlush);

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpVEncStream::setDstConfig(int32_t           portIndex,
                                          int32_t           width,
                                          int32_t           height,
                                          int32_t           YPitch,
                                          int32_t           UVPitch,
                                          DpColorFormat     format,
                                          DP_PROFILE_ENUM   profile,
                                          DpInterlaceFormat,
                                          DpRect*,
                                          bool              doFlush,
                                          DpSecure          secure)
{
    DP_TRACE_CALL();

    if ((width  <= 0) ||
        (height <= 0) ||
        (YPitch <= 0))
    {
        DPLOGE("DpVEncStream: error argument - width(%d), height(%d) or pitch(%d)\n", width, height, YPitch);
        return DP_STATUS_INVALID_PARAX;
    }

    if (portIndex >= ISP_MAX_OUTPUT_PORT_NUM)
    {
        DPLOGE("DpVEncStream: error argument - invalid output port index: %d\n", portIndex);
        return DP_STATUS_INVALID_PORT;
    }

    if (YPitch < ((width * DP_COLOR_BITS_PER_PIXEL(format)) >> 3))
    {
        DPLOGE("DpVEncStream: pitch(%d) is less than width(%d)\n", YPitch, width);
        return DP_STATUS_INVALID_PARAX;
    }

    // check for frame change
    if ((m_dstFormat[portIndex]  != format) ||
        (m_dstWidth[portIndex]   != width) ||
        (m_dstHeight[portIndex]  != height) ||
        (m_dstYPitch[portIndex]  != YPitch) ||
        (m_dstUVPitch[portIndex] != UVPitch) ||
        (m_dstProfile[portIndex] != profile) ||
        (m_dstFlush[portIndex]   != doFlush))
    {
        m_frameChange = true;
    }

    m_dstFormat[portIndex]  = format;
    m_dstWidth[portIndex]   = width;
    m_dstHeight[portIndex]  = height;
    m_dstYPitch[portIndex]  = YPitch;
    m_dstUVPitch[portIndex] = UVPitch;
    m_dstEnable[portIndex]  = true;
    m_dstFlush[portIndex]   = doFlush;
    m_dstProfile[portIndex] = profile;

    DPLOGI("dst %d flush status %d\n", portIndex, doFlush);
    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpVEncStream::setPortType(int32_t        portIndex,
                                         uint32_t       portType,
                                         DpVEnc_Config* VEnc_cfg)
{
    DP_TRACE_CALL();

    if (portIndex >= ISP_MAX_OUTPUT_PORT_NUM)
    {
        DPLOGE("DpVEncStream: invalid port index %d\n", portIndex);
        return DP_STATUS_INVALID_PORT;
    }

    if (m_dstPortType[portIndex] != portType)
    {
        m_dstPortType[portIndex] = portType;
        m_frameChange = true;
    }

    if (portType == PORT_VENC)
    {
        if (VEnc_cfg)
        {
            uint32_t BufferIndex = m_configFrameCount&0x7;
            //memcpy(&m_VEnc_cfg,VEnc_cfg,sizeof(m_VEnc_cfg));
            m_VEnc_cfg.memYUVMVAAddr[0] = m_yuvBuffer[BufferIndex]->mapHWAddress(tWDMA, 3);
            m_VEnc_cfg.memYUVMVAAddr[1] = m_VEnc_cfg.memYUVMVAAddr[0] + m_encodeFrameWidth*m_encodeFrameHeight;
            m_VEnc_cfg.memYUVMVAAddr[2] = m_VEnc_cfg.memYUVMVAAddr[1] + ((m_encodeFrameWidth*m_encodeFrameHeight) >> 2);

            m_VEnc_cfg.memYUVSWAddr[0] = m_yuvBuffer[BufferIndex]->mapSWAddress();
            m_VEnc_cfg.memYUVSWAddr[1] = (uint8_t*)m_VEnc_cfg.memYUVSWAddr[0] + m_encodeFrameWidth*m_encodeFrameHeight;
            m_VEnc_cfg.memYUVSWAddr[2] = (uint8_t*)m_VEnc_cfg.memYUVSWAddr[1] + ((m_encodeFrameWidth*m_encodeFrameHeight) >> 2);

            m_VEnc_cfg.memYUVMVASize[0] = (m_encodeFrameWidth*m_encodeFrameHeight);
            m_VEnc_cfg.memYUVMVASize[1] = (m_encodeFrameWidth*m_encodeFrameHeight)>>2;
            m_VEnc_cfg.memYUVMVASize[2] = (m_encodeFrameWidth*m_encodeFrameHeight)>>2;
        }
        else
        {
            DPLOGE("DpVEncStream: error argument - invalid VEnc parameter index: %d\n", portIndex);
            return DP_STATUS_INVALID_PARAX;
        }
    }
    else if (portType == PORT_ISP)
    {
        m_dstEnable[portIndex] = true;
    }

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpVEncStream::setRotation(int32_t portIndex,
                                         int32_t rotation)
{
    DP_TRACE_CALL();

    if (portIndex >= ISP_MAX_OUTPUT_PORT_NUM)
    {
        DPLOGE("DpVEncStream: error argument - invalid output port index: %d\n", portIndex);
        return DP_STATUS_INVALID_PORT;
    }

    if (m_rotation[portIndex] != rotation)
    {
        m_rotation[portIndex] = rotation;
        m_frameChange = true;
    }

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpVEncStream::setFlipStatus(int32_t portIndex,
                                           bool    flipStatus)
{
    DP_TRACE_CALL();

    if (portIndex >= ISP_MAX_OUTPUT_PORT_NUM)
    {
        DPLOGE("DpVEncStream: error argument - invalid output port index: %d\n", portIndex);
        return DP_STATUS_INVALID_PORT;
    }

    if (m_flipStatus[portIndex] != flipStatus)
    {
        m_flipStatus[portIndex] = flipStatus;
        m_frameChange = true;
    }

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpVEncStream::setParameter(ISP_TPIPE_CONFIG_STRUCT &extraPara, uint32_t hint)
{
    DP_TRACE_CALL();

    if (0 == (hint & HINT_ISP_TPIPE_NO_CHANGE))
    {
        m_frameChange = true;
    }

    return m_pChannel->setParameter(&extraPara,
                                    sizeof(ISP_TPIPE_CONFIG_STRUCT));
}

DP_STATUS_ENUM DpVEncStream::setParameter(ISP_TPIPE_CONFIG_STRUCT &extraPara, ISP2MDP_STRUCT *isp2mdp, uint32_t hint){
    DP_STATUS_ENUM status;
    status = setParameter(extraPara, hint);
    if (status != DP_STATUS_RETURN_SUCCESS)
        return status;
    return m_pChannel->setIsp2MdpParam(isp2mdp, sizeof(ISP2MDP_STRUCT));
}

DP_STATUS_ENUM DpVEncStream::setPortMatrix(int32_t portIndex, DpColorMatrix *matrix)
{
    DP_TRACE_CALL();

    if (portIndex >= ISP_MAX_OUTPUT_PORT_NUM)
    {
        DPLOGE("DpVEncStream: error argument - invalid output port index: %d\n", portIndex);
        return DP_STATUS_INVALID_PORT;
    }

    if (NULL == matrix || !matrix->enable)
    {
        if (NULL != m_pDstMatrix[portIndex])
        {
            delete m_pDstMatrix[portIndex];
            m_pDstMatrix[portIndex] = NULL;
            m_frameChange = true;
        }
    }
    else if (NULL == m_pDstMatrix[portIndex])
    {
        m_pDstMatrix[portIndex] = new DpColorMatrix(*matrix);
        m_frameChange = true;
    }
    else
    {
        *m_pDstMatrix[portIndex] = *matrix;
        //m_frameChange = true; // no need
    }

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpVEncStream::startStream(struct timeval *endTime)
{
    DP_TRACE_CALL();
    DP_STATUS_ENUM status;
    int32_t        index;
    void           *pBase[3];
    uint32_t       size[3];
    int32_t        curSourceBuffer;
    int32_t        enableLog = DpDriver::getInstance()->getEnableLog();
    char           bufferInfoStr[256] = "";

#if 0
    if (-1 == m_srcBuffer)
    {
        DPLOGE("DpVEncStream: src buffer not ready\n");
        return DP_STATUS_INVALID_BUFFER;
    }
#endif

    DPLOGI("DpVEnc: start 0x%08x\n", this);
    status = m_pChannel->setEndTime(endTime);

    if (!m_srcBufferStartTimeList.empty())
        DP_TIMER_GET_CURRENT_TIME(m_srcBufferStartTimeList.front());

    for (index = 0; index < ISP_MAX_OUTPUT_PORT_NUM; index++)
    {
        if (m_dstWasEnabled[index] != m_dstEnable[index])
        {
            m_frameChange = true;
            break;
        }
    }

    if (m_pStream->getScenario() == STREAM_ISP_VSS ||
        m_pStream->getScenario() == STREAM_ISP_ZSD_SLOW)
    {
        m_frameChange = true;
    }

    if (m_frameChange)
    {
        status = m_pStream->resetStream();
        assert(DP_STATUS_RETURN_SUCCESS == status);

        status = m_pChannel->resetChannel();
        assert(DP_STATUS_RETURN_SUCCESS == status);

        // Add source port
        status = m_pChannel->setSourcePort(PORT_MEMORY,
                                           m_srcFormat,
                                           m_srcWidth,
                                           m_srcHeight,
                                           m_srcYPitch,
                                           m_srcUVPitch,
                                           false, // No HDR in VEnc
                                           false, // No DRE in VEnc
                                           m_pSrcPool,
                                           m_srcProfile,
                                           m_srcSecure,
                                           (DP_SECURE_NONE != m_srcSecure)? false: m_srcFlush);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            DPLOGE("DpVEncStream: set source port failed\n");
            return status;
        }

        if (enableLog)
        {
            memset(bufferInfoStr, '\0', sizeof(bufferInfoStr));
            m_pSrcPool->dumpBufferInfo(bufferInfoStr, sizeof(bufferInfoStr));
            DPLOGD("DpVEnc: in: (%d, %d, %d, %d, C%d%s%s%s%s, P%d) %s\n",
                m_srcWidth, m_srcHeight, m_srcYPitch, m_srcUVPitch,
                DP_COLOR_GET_UNIQUE_ID(m_srcFormat),
                DP_COLOR_GET_SWAP_ENABLE(m_srcFormat) ? "s" : "",
                DP_COLOR_GET_BLOCK_MODE(m_srcFormat) ? "b" : "",
                DP_COLOR_GET_INTERLACED_MODE(m_srcFormat) ? "i" : "",
                DP_COLOR_GET_UFP_ENABLE(m_srcFormat) ? "u" : "", m_srcProfile,
                bufferInfoStr);
        }

        m_pSrcPool->activateBuffer();

        for (index = 0; index < ISP_MAX_OUTPUT_PORT_NUM; index++)
        {
            if (false == m_dstEnable[index])
            {
                continue;
            }

            // Error check for the target buffer
#if 0
            if (-1 == m_dstBuffer[index])
            {
                DPLOGE("DpVEncStream: dst buffer not ready\n");
                return DP_STATUS_INVALID_BUFFER;
            }
#endif
            //if (m_PqConfig[index].enDC || m_PqConfig[index].enSharp)
            //{
            //    m_sharpness[index] = m_PqConfig[index].enSharp;
            //}
            //m_pStream->setPQReadback(m_PqConfig[index].enDC);
            uint32_t tdshp = m_sharpness[index] ?
                m_sharpness[index] :
                (m_PqConfig[index].enDC || m_PqConfig[index].enSharp || m_PqConfig[index].enColor);

            if (enableLog)
            {
                memset(bufferInfoStr, '\0', sizeof(bufferInfoStr));
                m_pDstPool[index]->dumpBufferInfo(bufferInfoStr, sizeof(bufferInfoStr));
                DPLOGD("DpVEnc: out%d: (%d, %d, %d, %d, C%d%s, P%d), misc: (R:%d, F:%d, S:%d, D:%d, T:%d) %s\n",
                    index, m_dstWidth[index], m_dstHeight[index], m_dstYPitch[index], m_dstUVPitch[index],
                    DP_COLOR_GET_UNIQUE_ID(m_dstFormat[index]), DP_COLOR_GET_SWAP_ENABLE(m_dstFormat[index]) ? "s" : "", m_dstProfile[index],
                    m_rotation[index], m_flipStatus[index] ? 1 : 0, tdshp, m_ditherStatus[index] ? 1 : 0, m_dstPortType[index],
                    bufferInfoStr);
            }

            if (m_dstPlane[index] != DP_COLOR_GET_PLANE_COUNT(m_dstFormat[index]))
            {
                DPLOGE("DpVEncStream: invalid color format: %d, %d\n", m_dstPlane[index], DP_COLOR_GET_PLANE_COUNT(m_dstFormat[index]));
                assert(0);
                return DP_STATUS_INVALID_FORMAT;
            }

            switch ((PORT_TYPE_ENUM)m_dstPortType[index])
            {
            case PORT_MEMORY:
            case PORT_VENC:
            case PORT_ISP:
                {
#ifdef BASIC_PACKAGE
                    uint32_t videoID = m_PqParam[index].u.video.id;
#else
                    PQSessionManager* pPQSessionManager = PQSessionManager::getInstance();
                    uint32_t videoID = pPQSessionManager->findVideoID(m_PqParam[index].u.video.id);
#endif // BASIC_PACKAGE
                    uint64_t PqSessionID = (static_cast<uint64_t>(m_PqID[index]) << 32) | videoID;

                    status = m_pChannel->addTargetPort(index,
                                                       (PORT_TYPE_ENUM)m_dstPortType[index],
                                                       m_dstFormat[index],
                                                       m_dstWidth[index],
                                                       m_dstHeight[index],
                                                       m_dstYPitch[index],
                                                       m_dstUVPitch[index],
                                                       m_rotation[index],
                                                       m_flipStatus[index],
                                                       PqSessionID,
                                                       tdshp,
                                                       m_ditherStatus[index],
                                                       m_pDstPool[index],
                                                       m_dstProfile[index],
                                                       m_srcSecure,
                                                       (DP_SECURE_NONE != m_srcSecure) ? false : m_dstFlush[index],
                                                       m_pDstMatrix[index],
                                                       (PORT_VENC == (PORT_TYPE_ENUM)m_dstPortType[index]) ? &m_VEnc_cfg : NULL);
                }
                break;
            default:
                DPLOGE("DpVEncStream: Port Type is not supported\n");
                return DP_STATUS_INVALID_PORT;
            }

            if (DP_STATUS_RETURN_SUCCESS != status)
            {
                DPLOGE("DpVEncStream: add target port failed\n");
                return status;
            }

            if (true == m_multiCropChange[index])
            {
                if (enableLog)
                {
                    DPLOGD("DpVEnc: crop%d: (%d, %d, %d, %d, %d, %d)\n",
                        index, m_multiSrcXStart[index], m_multiSrcYStart[index], m_multiCropWidth[index], m_multiCropHeight[index], m_multiSrcXSubpixel[index], m_multiSrcYSubpixel[index]);
                }

                status = m_pChannel->setSourceCrop(index,
                                                   m_multiSrcXStart[index],
                                                   m_multiSrcXSubpixel[index],
                                                   m_multiSrcYStart[index],
                                                   m_multiSrcYSubpixel[index],
                                                   m_multiCropWidth[index],
                                                   0,
                                                   m_multiCropHeight[index],
                                                   0);
                if (DP_STATUS_RETURN_SUCCESS != status)
                {
                    DPLOGE("DpVEncStream: set source crop failed\n");
                    return status;
                }
            }

            m_pDstPool[index]->activateBuffer();
        }

        status = m_pStream->addChannel(m_pChannel,
                                       &m_channelID);
        assert(DP_STATUS_RETURN_SUCCESS == status);
    }

    if (false == m_frameChange)
    {
        DPLOGI("DpIsp: config frame only!\n");
        m_pStream->setConfigFlag(DpStream::CONFIG_FRAME_ONLY);
    }
    else
    {
        DPLOGI("DpIsp: config all!\n");
        m_pStream->setConfigFlag(DpStream::CONFIG_ALL);
    }

    status = m_pStream->startStream(m_frameChange);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpVEncStream: start ISP stream failed(%d)\n", status);
        m_frameChange = true;
        return status;
    }

    m_frameChange = false;

    status = m_pSrcPool->dequeueBuffer(&curSourceBuffer, pBase, size);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpVEncStream: dequeue source buffer failed(%d)\n", status);
        return status;
    }

    // Queue and trigger the source buffer
    DPLOGI("DpVEncStream:: enqueue source buffer: %d!!\n",curSourceBuffer);
    status = m_pSrcPool->queueBuffer(curSourceBuffer);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpVEncStream: queue source buffer failed(%d)\n", status);
        return status;
    }

    if(m_pStream->getSyncMode())
    {
        DPLOGW("DpVEncStream: in synchronize mode, back to caller directly\n");
        return status;
    }

    status = waitSubmit();
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpVEncStream: waitSubmit is failed in asynchronize mode (%d)\n", status);
        m_frameChange = true;
        return status;
    }
    DPLOGI("DpVEncStream: waitSubmit is done in asynchronize mode\n");

    if (VENC_ENABLE_FLAG == m_pChannel->getVENCInserted())
    {
        m_pFrameMutex->lock();
        m_configFrameCount++;
        m_pFrameMutex->unlock();
    }

    if (NULL != m_VEnc_cfg.pVEncCommander)
    {
        m_VEnc_cfg.pVEncCommander->reset();
    }

    return status;
}


DP_STATUS_ENUM DpVEncStream::getJPEGFilesize(uint32_t* filesize)
{
    if (NULL == filesize)
    {
        DPLOGE("DpVEncStream::getJPEGFilesize *filesize == NULL\n");
        return DP_STATUS_INVALID_PARAX;
    }

    *filesize = m_jpegEnc_filesize;

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpVEncStream::waitComplete()
{
    DP_TRACE_CALL();
    DP_STATUS_ENUM status = DP_STATUS_RETURN_SUCCESS;
    DpReadbackRegs readBackRegs;
    DpJobID        _jobID = 0;
    DpJobID        extJobID = 0;
    DpJobID        numJobID = 0;
    const uint32_t *readBackValues = NULL;
    uint32_t       readBackNum = 0;
    uint32_t       index = 0;

    if (m_pStream->getSyncMode())
    {
        DPLOGI("DpVEncStream: Wait complete in synchronized mode\n");
        status = m_pStream->waitStream();
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            DPLOGE("DpVEncStream: Can not complete in synchronized mode (%d)\n", status);
            m_frameChange = true;
        }
        readBackValues = m_pStream->getReadbackValues(readBackNum);
        if (NULL != readBackValues)
        {
            m_jpegEnc_filesize = readBackValues[0];
        }
        else
        {
            DPLOGW("DpVEncStream: getReadbackValues is NULL\n");
        }
        return status;
    }

    DPLOGI("DpVEncStream: Wait complete in asynchronized mode: %x\n", this);

    memset(&readBackRegs, 0, sizeof(readBackRegs));

    m_pListMutex->lock();
    if (!m_frameDoneList.empty())
    {
        _jobID = m_frameDoneList.front();
        m_pListMutex->unlock();

        if (_jobID)
        {
            DPLOGI("DpVEncStream: Job ID : %llu is in Framedone List!!!\n", _jobID);
            return DP_STATUS_RETURN_SUCCESS;
        }
        else
        {
            DPLOGE("DpVEncStream: Invalid Job ID : %llu is in Framedone List!!!\n", _jobID);
            assert(0);
            return DP_STATUS_INVALID_PARAX;
        }
    }

    if (1 < m_jobIDList.size())
    {
        numJobID = m_jobIDList.front();
        _jobID = m_jobIDList[1];
        if(numJobID == 2)
        {
            extJobID = m_jobIDList[2];
        }
        m_pListMutex->unlock();

        status = DpDriver::getInstance()->waitFramedone(_jobID, readBackRegs);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            DPLOGE("DpVEncStream: wait ISP/MDP complete stream failed(%d)\n", status);
            m_pStream->dumpDebugStream();
            m_frameChange = true;
            return status;
        }

        if (numJobID > 1) // Means VENC dedicated!!!
        {
            uint32_t offset = (m_dequeueFrameCount & 0xF) << 2;
            readBackRegs.m_num = DP_MAX_SINGLE_PABUFFER_COUNT;
            memcpy(readBackRegs.m_regs, m_PABuffer + offset, sizeof(uint32_t) * DP_MAX_SINGLE_PABUFFER_COUNT);
            readBackRegs.m_engineFlag = (1 << tVENC);
            status = DpDriver::getInstance()->waitFramedone(extJobID, readBackRegs);
            if (DP_STATUS_RETURN_SUCCESS != status)
            {
                DPLOGE("DpVEncStream: wait VENC complete stream failed(%d)\n", status);
                m_pStream->dumpDebugStream();
                m_frameChange = true;
                return status;
            }
            memcpy(m_ValueBuffer, readBackRegs.m_values, sizeof(uint32_t) * DP_MAX_SINGLE_PABUFFER_COUNT);

            VENCPostProc(&m_VEnc_cfg, m_numPABuffer, m_ValueBuffer);
            m_pFrameMutex->lock();
            m_dequeueFrameCount++;
            m_pFrameMutex->unlock();

        }

        m_pListMutex->lock();
        m_frameDoneList.push_back(_jobID);
        m_jobIDList.erase(m_jobIDList.begin());
        for(index = 0; index < numJobID; index++)
        {
            m_jobIDList.erase(m_jobIDList.begin());
        }
        m_pListMutex->unlock();
        DPLOGI("DpVEncStream: Job ID : %x is in JobID List!!!\n",_jobID);

        m_jpegEnc_filesize = readBackRegs.m_jpegEnc_filesize;

        return DP_STATUS_RETURN_SUCCESS;
    }
    m_pListMutex->unlock();

    DPLOGE("DpVEncStream: Both JobID and Framedone List are empty!!!\n");
    assert(0);
    return DP_STATUS_INVALID_PARAX;
}


DP_STATUS_ENUM DpVEncStream::waitSubmit()
{
    DP_TRACE_CALL();
    DP_STATUS_ENUM status;
    DpTimeValue    begin;
    DpJobID        jobID = 0;
    DpJobID        extJobID = 0;
    DpJobID        numJobID = 0;

    DP_TIMER_GET_CURRENT_TIME(begin);

    status = m_pStream->waitStream();
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpVEncStream: wait submit stream failed(%d)\n", status);
        m_pStream->dumpDebugStream();
        m_frameChange = true;
        return status;
    }

    numJobID = m_pStream->getAsyncJob(0); //Means getting number of JOB
    jobID = m_pStream->getAsyncJob(1);
    if(numJobID > 1)
    {
        extJobID = m_pStream->getAsyncJob(2);
    }
    m_pListMutex->lock();
    m_jobIDList.push_back(numJobID);
    m_jobIDList.push_back(jobID);
    if(numJobID > 1)
    {
        m_jobIDList.push_back(extJobID);
    }
    m_pListMutex->unlock();
    DPLOGI("DpVEncStream: submit stream done %d\n", numJobID);
    DPLOGI("DpVEncStream: submit stream done %llx\n", jobID);
    DPLOGI("DpVEncStream: submit stream done %llx\n", extJobID);

    return DP_STATUS_RETURN_SUCCESS;
}



DP_STATUS_ENUM DpVEncStream::dequeueDstBuffer(int32_t portIndex,
                                              void    **pVABase,
                                              bool    waitBuf)
{
    DP_TRACE_CALL();
    DP_STATUS_ENUM status;
    uint32_t       size[3];
    int32_t        bufID;
    DpTimeValue    begin;
    DpTimeValue    end;
    int32_t        diff;
    int32_t        dstBuffer = -1;

    DPLOGI("==============dequeue dst buffer begin:==============\n");

    DP_TIMER_GET_CURRENT_TIME(begin);

    if (portIndex >= ISP_MAX_OUTPUT_PORT_NUM)
    {
        DPLOGE("DpVEncStream: invalid output port index: %d\n", portIndex);
        return DP_STATUS_INVALID_PORT;
    }

    status = DP_STATUS_RETURN_SUCCESS;

    if (true == waitBuf)
    {
        DPLOGI("DpVEncStream: goto wait stream\n");
        status = waitComplete();

        m_pBufferMutex->lock();
        dstBuffer = m_dstBufferList[portIndex].front();
        m_dstBufferList[portIndex].erase(m_dstBufferList[portIndex].begin());
        m_pBufferMutex->unlock();

        if (-1 == dstBuffer)
        {
            DPLOGE("DpVEncStream: empty dst buffer of port %d\n", portIndex);
            return DP_STATUS_INVALID_BUFFER;
        }
    }else
    {
        if(false == m_pStream->getSyncMode()) //Async mode!
        {
            DPLOGE("DpVEncStream: NOT supporting to nonblocking dequeueDstBuffer in async mode %d\n", portIndex);
            assert(0);
            return DP_STATUS_INVALID_PARAX;
        }
    }

    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpVEncStream: wait stream failed(%d)\n", status);
        m_pStream->stopStream();

        return status;
    }

#if 1
    if(false == m_pStream->getSyncMode()) //Async mode!
    {
        if(true == m_dstFlush[portIndex])
        {
            status = m_pDstPool[portIndex]->flushWriteBuffer(dstBuffer);
            DPLOGI("DpVEncStream:Port[%d] flushWriteBuffer %d status: %d \n", portIndex, dstBuffer, status);
        }
        status = m_pDstPool[portIndex]->queueBuffer(dstBuffer);
        DPLOGI("DpVEncStream:Port[%d] queueBuffer %d status: %d \n", portIndex, dstBuffer, status);
    }
#endif

    status = m_pDstPool[portIndex]->acquireBuffer(&bufID,
                                                  pVABase,
                                                  size,
                                                  waitBuf);

    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        if (true == waitBuf)
        {
            DPLOGE("DpVEncStream: acquire dst buffer failed(%d)\n", status);
        }

        return status;
    }

    if(bufID != dstBuffer)
    {
        DPLOGE("DpVEncStream: Not identical bufID(%d) != dstBuffer(%d)\n", bufID,dstBuffer);
    }

    if( (false == waitBuf) && (true == m_pStream->getSyncMode()))
    {
        //Sync Mode
        const uint32_t *readBackValues = NULL;
        uint32_t       readBackNum = 0;
        readBackValues = m_pStream->getReadbackValues(readBackNum);

        m_jpegEnc_filesize = (NULL != readBackValues) ? readBackValues[0] : 0;
        DPLOGD("DpVEncStream: JPEG size: %d in %p\n", m_jpegEnc_filesize, readBackValues);
    }

    if (false == m_pStream->getSyncMode()) //Async mode!
    {
        status = m_pDstPool[portIndex]->asyncReleaseBuffer(bufID);
        DPLOGI("DpVEncStream: dequeueDstBuffer releaseBuffer %d status: %d \n",bufID,status);
    }
    else
    {
        m_pDstPool[portIndex]->releaseBuffer(bufID);
    }

    m_pDstPool[portIndex]->unregisterBuffer(bufID);

    DP_TIMER_GET_CURRENT_TIME(end);

    DP_TIMER_GET_DURATION_IN_MS(begin,
                                end,
                                diff);

    if (diff > 30)
    {
        DPLOGW("DpVEnc: %p, scenario %d, dst%d %d ms\n", this, m_pStream->getScenario(), portIndex, diff);
    }
    else
    {
        DPLOGI("DpVEnc: %p, scenario %d, dst%d %d ms\n", this, m_pStream->getScenario(), portIndex, diff);
    }

    DPLOGI("==============dequeue dst buffer end:==============\n");
    return status;
}


DP_STATUS_ENUM DpVEncStream::dequeueFrameEnd(uint32_t*)
{
    DP_TRACE_CALL();
    DpJobID jobID;

    if(true == m_pStream->getSyncMode()) //Sync mode!
    {
        DPLOGI("bypass dequeueFrameEnd in Sync mode\n");
        return DP_STATUS_RETURN_SUCCESS;
    }

    DPLOGI("==============dequeue frame end:==============\n");
    m_pListMutex->lock();
    if (m_frameDoneList.empty())
    {
        DPLOGE("============== dequeue frame end: Failed due to empty framedone List ==============\n");
    }
    assert(false == m_frameDoneList.empty());
    jobID = m_frameDoneList.front();
    m_frameDoneList.erase(m_frameDoneList.begin());
    m_pListMutex->unlock();

    m_jpegEnc_filesize = 0;
    DPLOGI("FRAME: %X  - dequeue end\n",jobID);
    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpVEncStream::stopStream()
{
    DP_TRACE_CALL();
    DP_STATUS_ENUM status = DP_STATUS_RETURN_SUCCESS;
    int32_t        index;

    DPLOGI("=================stop stream begin:=================\n");

    DPLOGI("DpVEncStream: stop 0x%08x\n", this);

    m_pStream->stopStream();

    m_pSrcPool->activateBuffer();

    for (index = 0; index < ISP_MAX_OUTPUT_PORT_NUM; index++)
    {
        if (true == m_dstEnable[index])
        {
            m_pDstPool[index]->activateBuffer();
        }
        //m_dstPortType[index] = 0;

        m_dstWasEnabled[index] = m_dstEnable[index];
        m_dstEnable[index]     = false;
    }

    DPLOGI("=================stop stream end:=================\n");

    return status;
}

/* Venc Modify + */
#if defined(MTK_SLOW_MOTION_HEVC_SUPPORT)
DP_STATUS_ENUM DpVEncStream::startVideoRecord_HEVC(uint32_t width, uint32_t height, uint32_t framerate)
{
    VAL_CHAR_T  icvalue[PROPERTY_VALUE_MAX];

    VAL_UINT32_T u4FrameWidth = width;
    VAL_UINT32_T u4FrameHeight = height;
    VAL_UINT32_T u4FrameRate;
    VAL_UINT32_T u4BitRate;
    VAL_UINT32_T u4BitRateKbps;
    VAL_UINT32_T u4EncoderFormat;
    VAL_UINT32_T u4PeriodOfIntraFrames;
    VAL_BOOL_T fgDLUnitTestMode;
    VAL_UINT32_T _current_tid = gettid();

    VENC_DRV_QUERY_VIDEO_FORMAT_T rqInfo;
    VENC_DRV_PARAM_ENC_T rDrvEncPrm;
    VENC_DRV_PARAM_ENC_EXTRA_T rEncoderExtraConfig;
    VENC_DRV_SCENARIO_T eScenario;

    //NOT Used in Real Case
    property_get("vendor.dl.vr.set.unit.test.mode", icvalue, "0");
    fgDLUnitTestMode = (VAL_BOOL_T) atoi(icvalue);
    if (fgDLUnitTestMode == VAL_TRUE)
    {
        DPLOGD("[tid: %d][startVideoRecord_HEVC] ==== Unit Test Mode ====\n", _current_tid);
        property_set("vendor.dl.vr.set.frame.rate", "120");
        property_set("vendor.dl.vr.set.bit.rate", "43000000");
        property_set("vendor.dl.vr.set.encoder", "4");
        property_set("vendor.dl.vr.set.iframes.interval", "1");
    }

    // get FrameRate from Camera
    u4FrameRate = framerate;  //[ToDo : Need to get FrameRate from Camera]
    DPLOGD("[tid: %d][startVideoRecord_HEVC] FrameRate = %d\n", _current_tid, u4FrameRate);

    // get BitRate from StageFrightRecorder
    if (u4FrameRate == 120)
    {
        property_get("vendor.dl.vr.set.bit.rate", icvalue, "43000000");    // 43Mbps
    }
    else if (u4FrameRate == 180)
    {
        property_get("vendor.dl.vr.set.bit.rate", icvalue, "40000000");    // 40Mbps
    }
    else
    {
        property_get("vendor.dl.vr.set.bit.rate", icvalue, "43000000");    // 43Mbps
    }

    u4BitRate = (VAL_UINT32_T) atoi(icvalue);
    u4BitRateKbps = u4BitRate/1000;
    DPLOGD("[tid: %d][startVideoRecord_HEVC] BitRate = %d, BitRateKbps = %d\n", _current_tid, u4BitRate, u4BitRateKbps);

    // get EncoderFormat from StageFrightRecorder
    property_get("vendor.dl.vr.set.encoder", icvalue, "4");    // Hevc encoder
    u4EncoderFormat = (VAL_UINT32_T) atoi(icvalue);
    DPLOGD("[tid: %d][startVideoRecord_HEVC] EncoderFormat = %d\n", _current_tid, u4EncoderFormat);

    // get PeriodOfIntraFrames from StageFrightRecorder
    property_get("vendor.dl.vr.set.iframes.interval", icvalue, "1");   // 1 I-frame/sec
    u4PeriodOfIntraFrames = (VAL_UINT32_T) atoi(icvalue);
    u4PeriodOfIntraFrames = u4PeriodOfIntraFrames * u4FrameRate;
    DPLOGD("[tid: %d][startVideoRecord_HEVC] PeriodOfIntraFrames = %d\n", _current_tid, u4PeriodOfIntraFrames);

    // query capability
    memset(&rqInfo, 0, sizeof(VENC_DRV_QUERY_VIDEO_FORMAT_T));
    rqInfo.eVideoFormat = VENC_DRV_VIDEO_FORMAT_HEVC;
    rqInfo.u4Width = u4FrameWidth;
    rqInfo.u4Height = u4FrameHeight;
    if (VENC_DRV_MRESULT_FAIL == eVEncDrvQueryCapability(VENC_DRV_QUERY_TYPE_VIDEO_FORMAT, &rqInfo, VAL_NULL))
    {
        DPLOGE("[tid: %d][startVideoRecord][ERROR] Cannot support HEVC encoder\n", _current_tid);
        return DP_STATUS_UNKNOWN_ERROR;
    }

    // create driver
    if (VENC_DRV_MRESULT_FAIL == eVEncDrvCreate(&m_VEnc_cfg.rVencDrvHandle, VENC_DRV_VIDEO_FORMAT_HEVC))
    {
        DPLOGE("[tid: %d][startVideoRecord][ERROR] cannot create HEVC enc driver\n", _current_tid);
        return DP_STATUS_UNKNOWN_ERROR;
    }

    // init driver
    if (VENC_DRV_MRESULT_FAIL == eVEncDrvInit(m_VEnc_cfg.rVencDrvHandle))
    {
        DPLOGE("[tid: %d][startVideoRecord][ERROR] cannot init HEVC enc driver\n", _current_tid);
        goto startVideoRecord_HEVC_Release_Driver_ERROR;
    }

    // set slow motion
    eScenario = VENC_DRV_SCENARIO_CAMERA_REC_SLOW_MOTION;
    if (VENC_DRV_MRESULT_FAIL == eVEncDrvSetParam(m_VEnc_cfg.rVencDrvHandle, VENC_DRV_SET_TYPE_SCENARIO, (VAL_VOID_T*)&eScenario, VAL_NULL))
    {
        DPLOGE("[tid: %d][startVideoRecord][ERROR] set venc scenario fail\n", _current_tid);
        goto startVideoRecord_HEVC_Deinit_Release_Driver_ERROR;
    }

    // set encoder parameter
    rEncoderExtraConfig.u4IntraFrameRate = u4PeriodOfIntraFrames;
    rEncoderExtraConfig.u4BitRate = u4BitRateKbps;
    rEncoderExtraConfig.u4FrameRateQ16 = u4FrameRate;

    rDrvEncPrm.eVEncFormat = VENC_DRV_YUV_FORMAT_YV12;
    rDrvEncPrm.u4Profile = VENC_DRV_HEVC_VIDEO_PROFILE_MAIN;
    rDrvEncPrm.u4Level = VENC_DRV_VIDEO_LEVEL_5;
    rDrvEncPrm.u4Width = u4FrameWidth;
    rDrvEncPrm.u4Height = u4FrameHeight;
    rDrvEncPrm.u4BufWidth = VENC_ROUND_N(u4FrameWidth, 16);
    rDrvEncPrm.u4BufHeight = VENC_ROUND_N(u4FrameHeight, 16);
    rDrvEncPrm.u4NumPFrm = u4PeriodOfIntraFrames - 1;
    rDrvEncPrm.u4NumBFrm = 0;
    rDrvEncPrm.eFrameRate = (VENC_DRV_FRAME_RATE_T)u4FrameRate;
    rDrvEncPrm.fgInterlace = VAL_FALSE;
    rDrvEncPrm.pvExtraEnc = &rEncoderExtraConfig;

    if (VENC_DRV_MRESULT_FAIL == eVEncDrvSetParam(m_VEnc_cfg.rVencDrvHandle, VENC_DRV_SET_TYPE_PARAM_ENC, &rDrvEncPrm, VAL_NULL))
    {
        DPLOGE("[tid: %d][startVideoRecord][ERROR] cannot set param VENC_DRV_SET_TYPE_PARAM_ENC\n", _current_tid);
        goto startVideoRecord_HEVC_Deinit_Release_Driver_ERROR;
    }

    if (VENC_DRV_MRESULT_FAIL == eVEncDrvSetParam(m_VEnc_cfg.rVencDrvHandle, VENC_DRV_SET_TYPE_SLOW_MOTION_LOCK_HW, VAL_NULL, VAL_NULL))
    {
        DPLOGE("[tid: %d][startVideoRecord][ERROR] cannot set param VENC_DRV_SET_TYPE_SLOW_MOTION_LOCK_HW\n", _current_tid);
        goto startVideoRecord_HEVC_Deinit_Release_Driver_ERROR;
    }

    return DP_STATUS_RETURN_SUCCESS;

startVideoRecord_HEVC_Deinit_Release_Driver_ERROR:
    eVEncDrvDeInit(m_VEnc_cfg.rVencDrvHandle);

startVideoRecord_HEVC_Release_Driver_ERROR:
    eVEncDrvRelease(m_VEnc_cfg.rVencDrvHandle, VENC_DRV_VIDEO_FORMAT_HEVC);

    return DP_STATUS_UNKNOWN_ERROR;
}

DP_STATUS_ENUM DpVEncStream::stopVideoRecord_HEVC()
{
    VAL_UINT32_T _current_tid = gettid();

    // UnLock HW
    if (VENC_DRV_MRESULT_FAIL == eVEncDrvSetParam(m_VEnc_cfg.rVencDrvHandle, VENC_DRV_SET_TYPE_SLOW_MOTION_UNLOCK_HW, VAL_NULL, VAL_NULL))
    {
        DPLOGE("[tid: %d][stopVideoRecord_HEVC][ERROR] cannot set param VENC_DRV_SET_TYPE_SLOW_MOTION_UNLOCK_HW\n", _current_tid);
        goto stopVideoRecord_HEVC_Deinit_Release_Driver_ERROR;
    }

    eVEncDrvDeInit(m_VEnc_cfg.rVencDrvHandle);
    eVEncDrvRelease(m_VEnc_cfg.rVencDrvHandle, VENC_DRV_VIDEO_FORMAT_HEVC);

    return DP_STATUS_RETURN_SUCCESS;

stopVideoRecord_HEVC_Deinit_Release_Driver_ERROR:
    eVEncDrvDeInit(m_VEnc_cfg.rVencDrvHandle);
    eVEncDrvRelease(m_VEnc_cfg.rVencDrvHandle, VENC_DRV_VIDEO_FORMAT_HEVC);

    return DP_STATUS_UNKNOWN_ERROR;
}
#elif defined(MTK_SLOW_MOTION_H264_SUPPORT)
DP_STATUS_ENUM DpVEncStream::startVideoRecord_H264(uint32_t width, uint32_t height, uint32_t framerate)
{
    VAL_CHAR_T  icvalue[PROPERTY_VALUE_MAX];

    VAL_UINT32_T u4FrameWidth = width;
    VAL_UINT32_T u4FrameHeight = height;
    VAL_UINT32_T u4FrameRate;
    VAL_UINT32_T u4BitRate;
    VAL_UINT32_T u4BitRateKbps;
    VAL_UINT32_T u4EncoderFormat;
    VAL_UINT32_T u4PeriodOfIntraFrames;
    VAL_BOOL_T fgDLUnitTestMode;
    VAL_UINT32_T _current_tid = gettid();

    VENC_DRV_QUERY_VIDEO_FORMAT_T rqInfo;
    VENC_DRV_PARAM_ENC_T rDrvEncPrm;
    VENC_DRV_PARAM_ENC_EXTRA_T rEncoderExtraConfig;
    VENC_DRV_SCENARIO_T eScenario;

    //NOT Used in Real Case
    property_get("vendor.dl.vr.set.unit.test.mode", icvalue, "0");
    fgDLUnitTestMode = (VAL_BOOL_T) atoi(icvalue);
    if (fgDLUnitTestMode == VAL_TRUE)
    {
        DPLOGD("[tid: %d][startVideoRecord_H264] ==== Unit Test Mode ====\n", _current_tid);
        property_set("vendor.dl.vr.set.frame.rate", "120");
        property_set("vendor.dl.vr.set.bit.rate", "18000000");
        property_set("vendor.dl.vr.set.encoder", "4");
        property_set("vendor.dl.vr.set.iframes.interval", "1");
    }

    // get FrameRate from Camera
    u4FrameRate = framerate;  //[ToDo : Need to get FrameRate from Camera]

    // get BitRate from StageFrightRecorder
    property_get("vendor.dl.vr.set.bit.rate", icvalue, "18000000");    // 60Mbps
    u4BitRate = (VAL_UINT32_T) atoi(icvalue);
    u4BitRateKbps = u4BitRate/1000;
    DPLOGD("[tid: %d][startVideoRecord_H264] BitRate = %d, BitRateKbps = %d\n", _current_tid, u4BitRate, u4BitRateKbps);

    // get EncoderFormat from StageFrightRecorder
    property_get("vendor.dl.vr.set.encoder", icvalue, "3");    // H.264 encoder
    u4EncoderFormat = (VAL_UINT32_T) atoi(icvalue);
    DPLOGD("[tid: %d][startVideoRecord_H264] EncoderFormat = %d\n", _current_tid, u4EncoderFormat);

    // get PeriodOfIntraFrames from StageFrightRecorder
    property_get("vendor.dl.vr.set.iframes.interval", icvalue, "1");   // 1 I-frame/sec
    u4PeriodOfIntraFrames = (VAL_UINT32_T) atoi(icvalue);
    u4PeriodOfIntraFrames = u4PeriodOfIntraFrames * u4FrameRate;
    DPLOGD("[tid: %d][startVideoRecord_H264] PeriodOfIntraFrames = %d\n", _current_tid, u4PeriodOfIntraFrames);

    // query capability
    memset(&rqInfo, 0, sizeof(VENC_DRV_QUERY_VIDEO_FORMAT_T));
    rqInfo.eVideoFormat = VENC_DRV_VIDEO_FORMAT_H264;
    rqInfo.u4Width = u4FrameWidth;
    rqInfo.u4Height = u4FrameHeight;
    if (VENC_DRV_MRESULT_FAIL == eVEncDrvQueryCapability(VENC_DRV_QUERY_TYPE_VIDEO_FORMAT, &rqInfo, VAL_NULL))
    {
        DPLOGE("[tid: %d][startVideoRecord][ERROR] Cannot support H264 encoder\n", _current_tid);
        return DP_STATUS_UNKNOWN_ERROR;
    }

    // create driver
    if (VENC_DRV_MRESULT_FAIL == eVEncDrvCreate(&m_VEnc_cfg.rVencDrvHandle, VENC_DRV_VIDEO_FORMAT_H264))
    {
        DPLOGE("[tid: %d][startVideoRecord][ERROR] cannot create H264 enc driver\n", _current_tid);
        return DP_STATUS_UNKNOWN_ERROR;
    }

    // init driver
    if (VENC_DRV_MRESULT_FAIL == eVEncDrvInit(m_VEnc_cfg.rVencDrvHandle))
    {
        DPLOGE("[tid: %d][startVideoRecord][ERROR] cannot init H264 enc driver\n", _current_tid);
        goto startVideoRecord_H264_Release_Driver_ERROR;
    }

    // set slow motion
    eScenario = VENC_DRV_SCENARIO_CAMERA_REC_SLOW_MOTION;//VENC_DRV_SCENARIO_CAMERA_REC_SLOW_MOTION;
    if (VENC_DRV_MRESULT_FAIL == eVEncDrvSetParam(m_VEnc_cfg.rVencDrvHandle, VENC_DRV_SET_TYPE_SCENARIO, (VAL_VOID_T*)&eScenario, VAL_NULL))
    {
        DPLOGE("[tid: %d][startVideoRecord][ERROR] set venc scenario fail\n", _current_tid);
        goto startVideoRecord_H264_Deinit_Release_Driver_ERROR;
    }

    // set encoder parameter
    rEncoderExtraConfig.u4IntraFrameRate = u4PeriodOfIntraFrames;
    rEncoderExtraConfig.u4BitRate = u4BitRate;
    rEncoderExtraConfig.u4FrameRateQ16 = u4FrameRate << 16;

    rDrvEncPrm.eVEncFormat = VENC_DRV_YUV_FORMAT_YV12;
    rDrvEncPrm.u4Profile = VENC_DRV_H264_VIDEO_PROFILE_HIGH;
    rDrvEncPrm.u4Level = VENC_DRV_VIDEO_LEVEL_4_1;
    rDrvEncPrm.u4Width = u4FrameWidth;
    rDrvEncPrm.u4Height = u4FrameHeight;
    rDrvEncPrm.u4BufWidth = VENC_ROUND_N(u4FrameWidth, 16);
    rDrvEncPrm.u4BufHeight = VENC_ROUND_N(u4FrameHeight, 16);
    rDrvEncPrm.u4NumPFrm = u4PeriodOfIntraFrames - 1;
    rDrvEncPrm.u4NumBFrm = 0;
    rDrvEncPrm.eFrameRate = (VENC_DRV_FRAME_RATE_T)u4FrameRate;
    rDrvEncPrm.fgInterlace = VAL_FALSE;
    rDrvEncPrm.pvExtraEnc = &rEncoderExtraConfig;

    if (VENC_DRV_MRESULT_FAIL == eVEncDrvSetParam(m_VEnc_cfg.rVencDrvHandle, VENC_DRV_SET_TYPE_PARAM_ENC, &rDrvEncPrm, VAL_NULL))
    {
        DPLOGE("[tid: %d][startVideoRecord][ERROR] cannot set param VENC_DRV_SET_TYPE_PARAM_ENC\n", _current_tid);
        goto startVideoRecord_H264_Deinit_Release_Driver_ERROR;
    }

    if (VENC_DRV_MRESULT_FAIL == eVEncDrvSetParam(m_VEnc_cfg.rVencDrvHandle, VENC_DRV_SET_TYPE_SLOW_MOTION_LOCK_HW, VAL_NULL, VAL_NULL))
    {
        DPLOGE("[tid: %d][startVideoRecord][ERROR] cannot set param VENC_DRV_SET_TYPE_SLOW_MOTION_LOCK_HW\n", _current_tid);
        goto startVideoRecord_H264_Deinit_Release_Driver_ERROR;
    }

    return DP_STATUS_RETURN_SUCCESS;

startVideoRecord_H264_Deinit_Release_Driver_ERROR:
    eVEncDrvDeInit(m_VEnc_cfg.rVencDrvHandle);

startVideoRecord_H264_Release_Driver_ERROR:
    eVEncDrvRelease(m_VEnc_cfg.rVencDrvHandle, VENC_DRV_VIDEO_FORMAT_H264);

    return DP_STATUS_UNKNOWN_ERROR;
}

DP_STATUS_ENUM DpVEncStream::stopVideoRecord_H264()
{
    VAL_UINT32_T _current_tid = gettid();

    // UnLock HW
    if (VENC_DRV_MRESULT_FAIL == eVEncDrvSetParam(m_VEnc_cfg.rVencDrvHandle, VENC_DRV_SET_TYPE_SLOW_MOTION_UNLOCK_HW, VAL_NULL, VAL_NULL))
    {
        DPLOGE("[tid: %d][stopVideoRecord_H264][ERROR] cannot set param VENC_DRV_SET_TYPE_SLOW_MOTION_UNLOCK_HW\n", _current_tid);
        goto stopVideoRecord_H264_Deinit_Release_Driver_ERROR;
    }

    eVEncDrvDeInit(m_VEnc_cfg.rVencDrvHandle);
    eVEncDrvRelease(m_VEnc_cfg.rVencDrvHandle, VENC_DRV_VIDEO_FORMAT_H264);

    return DP_STATUS_RETURN_SUCCESS;

stopVideoRecord_H264_Deinit_Release_Driver_ERROR:
    eVEncDrvDeInit(m_VEnc_cfg.rVencDrvHandle);
    eVEncDrvRelease(m_VEnc_cfg.rVencDrvHandle, VENC_DRV_VIDEO_FORMAT_H264);

    return DP_STATUS_UNKNOWN_ERROR;
}
#endif // MTK_SLOW_MOTION_HEVC_SUPPORT
/* Venc Modify - */

DP_STATUS_ENUM DpVEncStream::startVideoRecord(uint32_t width, uint32_t height, uint32_t framerate)
{
    DP_TRACE_CALL();
    uint32_t index;

    for(index = 0; index < YUV_MAX_BUFFER_NUM;index++)
    {
        if(NULL == m_yuvBuffer[index])
        {
            m_yuvBuffer[index] = DpMemory::Factory(DP_MEMORY_ION, -1, (width*height*2));
        }else
        {
            DPLOGE("DpVEncStream:[startVideoRecord]m_yuvBuffer[%d] is NOT 0 \n", index);
        }
    }

    m_encodeFrameWidth = width;
    m_encodeFrameHeight = height;
    //hevc_enc_init();
    //m_VEnc_cfg

    /* Venc Modify + */
#if defined(MTK_SLOW_MOTION_HEVC_SUPPORT)
    VAL_UINT32_T _current_tid = gettid();
    if (DP_STATUS_RETURN_SUCCESS != startVideoRecord_HEVC(width, height, framerate))
    {
        DPLOGE("[tid: %d][startVideoRecord][ERROR] startVideoRecord_HEVC\n", _current_tid);
        return DP_STATUS_UNKNOWN_ERROR;
    }
#elif defined(MTK_SLOW_MOTION_H264_SUPPORT)
    VAL_UINT32_T _current_tid = gettid();
    if (DP_STATUS_RETURN_SUCCESS != startVideoRecord_H264(width, height, framerate))
    {
        DPLOGE("[tid: %d][startVideoRecord][ERROR] startVideoRecord_H264\n", _current_tid);
        return DP_STATUS_UNKNOWN_ERROR;
    }
#else
    DP_UNUSED(framerate);
#endif // MTK_SLOW_MOTION_HEVC_SUPPORT
    /* Venc Modify - */

    DpDriver::getInstance()->allocatePABuffer(MAX_NUM_READBACK_REGS, m_PABuffer);
    m_numPABuffer = 0;
    m_VEnc_cfg.pNumPABuffer = &m_numPABuffer;
    m_VEnc_cfg.pPABuffer = m_PABuffer;

    m_pFrameMutex->lock();
    m_configFrameCount = 0;
    m_dequeueFrameCount = 0;
    m_pFrameMutex->unlock();

    m_VEnc_cfg.pConfigFrameCount = &m_configFrameCount;
    m_VEnc_cfg.pDequeueFrameCount = &m_dequeueFrameCount;
    m_VEnc_cfg.pFrameMutex= m_pFrameMutex;

    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpVEncStream::stopVideoRecord()
{
    DP_TRACE_CALL();
    //hevc_enc_deInit();
    uint32_t index;

#if defined(MTK_SLOW_MOTION_HEVC_SUPPORT)
    VAL_UINT32_T _current_tid = gettid();
    if (DP_STATUS_RETURN_SUCCESS != stopVideoRecord_HEVC())
    {
        DPLOGE("[tid: %d][stopVideoRecord_HEVC][ERROR] stopVideoRecord_HEVC\n", _current_tid);
        return DP_STATUS_UNKNOWN_ERROR;
    }
#elif defined(MTK_SLOW_MOTION_H264_SUPPORT)
    VAL_UINT32_T _current_tid = gettid();
    if (DP_STATUS_RETURN_SUCCESS != stopVideoRecord_H264())
    {
        DPLOGE("[tid: %d][stopVideoRecord_H264][ERROR] stopVideoRecord_H264\n", _current_tid);
        return DP_STATUS_UNKNOWN_ERROR;
    }
#endif // MTK_SLOW_MOTION_HEVC_SUPPORT

    DpDriver::getInstance()->releasePABuffer(MAX_NUM_READBACK_REGS, m_PABuffer);

    DPLOGI("configFrameCount: %d , dequeueFramewCount: %d",m_configFrameCount,m_dequeueFrameCount);

    for(index = 0; index < YUV_MAX_BUFFER_NUM;index++)
    {
        delete m_yuvBuffer[index];
        m_yuvBuffer[index] = NULL;
    }

    delete m_VEnc_cfg.pVEncCommander;
    m_VEnc_cfg.pVEncCommander = NULL;

    return DP_STATUS_RETURN_SUCCESS;
}

#ifndef BASIC_PACKAGE
uint32_t DpVEncStream::getPqID()
{
    AutoMutex lock(s_PqCountMutex);
    s_PqCount = (s_PqCount+1) & 0xFFFFFFF;

    DPLOGI("DpVEncStream::s_PqCount %x\n", s_PqCount);

    return (s_PqCount | DP_VENCSTREAM);
}
#endif // BASIC_PACKAGE

DP_STATUS_ENUM DpVEncStream::setPQParameter(int32_t portIndex, const DpPqParam &pqParam)
{
    DP_TRACE_CALL();
#ifndef BASIC_PACKAGE
    if (0 != m_pqSupport) {
        DpPqConfig pqConfig;

        if (pqParam.scenario != MEDIA_ISP_PREVIEW)
        {
            DPLOGE("DpVEncStream: setPQParameter scenario %d\n", pqParam.scenario);
            return DP_STATUS_INVALID_PARAX;
        }

        if (portIndex >= ISP_MAX_OUTPUT_PORT_NUM)
        {
            DPLOGE("DpVEncStream: error argument - invalid output port index: %d\n", portIndex);
            return DP_STATUS_INVALID_PORT;
        }

#if CONFIG_FOR_OS_ANDROID
        PQSessionManager* pPQSessionManager = PQSessionManager::getInstance();
        uint32_t videoID = pPQSessionManager->findVideoID(pqParam.u.video.id);
        uint64_t PqSessionID = (static_cast<uint64_t>(m_PqID[portIndex]) << 32) | videoID;

        PQSession* pPQsession = pPQSessionManager->createPQSession(PqSessionID);
        pPQsession->setPQparam(&pqParam);

        DpPqConfig* pDpPqConfig;
        pPQsession->getDpPqConfig(&pDpPqConfig);
        pqConfig = *pDpPqConfig;

        DPLOGI("DpVEncStream: pPQsession id %llx created\n", PqSessionID);
        DPLOGI("DpVEncStream: setPQParameter id %x enable %d scenario %d iso %d\n", m_PqID[portIndex], pqParam.enable, pqParam.scenario, pqParam.u.isp.iso);
        DPLOGI("DpVEncStream: getPQConfig sharp %d DC %d color %d\n", pqConfig.enSharp, pqConfig.enDC, pqConfig.enColor);

        if ((m_PqConfig[portIndex].enColor != pqConfig.enColor) ||
            (m_PqConfig[portIndex].enDC != pqConfig.enDC) ||
            (m_PqConfig[portIndex].enSharp != pqConfig.enSharp))
        {
            memcpy(&m_PqConfig[portIndex], &pqConfig, sizeof(pqConfig));
            m_frameChange = true;
        }

        if (m_PqParam[portIndex].u.video.id != pqParam.u.video.id)
        {
            m_frameChange = true;
        }

        memcpy(&m_PqParam[portIndex], &pqParam, sizeof(pqParam));
#endif // CONFIG_FOR_OS_ANDROID
    }
#endif // BASIC_PACKAGE
    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpVEncStream::setPQParameter(int32_t portIndex, const DpPqParam *pqParam)
{
    DpPqParam localDpPqParam;
    localDpPqParam.enable = false;
    localDpPqParam.scenario = MEDIA_ISP_PREVIEW;
    if (pqParam == NULL)
    {
        DPLOGI("DpVEncStream: setPQParameter pqParam is NULL, set scenario to MEDIA_ISP_PREVIEW\n");
        pqParam = &localDpPqParam;
    }
    return setPQParameter(portIndex, *pqParam);
}

DP_STATUS_ENUM DpVEncStream::setCallback(void (*callback)(void *), void * data)
{
    //Only for API sync to DpIspStream
    return DP_STATUS_RETURN_SUCCESS;
}
