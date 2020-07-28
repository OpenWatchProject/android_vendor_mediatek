#include "DpBlitStream.h"
#include "DpStream.h"
#include "DpChannel.h"
#include "DpBufferPool.h"
#include "DpPlatform.h"

#if CONFIG_FOR_OS_ANDROID
#ifndef BASIC_PACKAGE
#include "PQSessionManager.h"
#endif // BASIC_PACKAGE
#endif // CONFIG_FOR_OS_ANDROID

static uint32_t s_PqCount = 0;
static DpMutex  s_PqCountMutex;

bool DpBlitStream::queryHWSupport(uint32_t         srcWidth,
                                  uint32_t         srcHeight,
                                  uint32_t         dstWidth,
                                  uint32_t         dstHeight,
                                  int32_t          Orientation,
                                  DpColorFormat    srcFormat,
                                  DpColorFormat    dstFormat,
                                  DpPqParam        *PqParam,
                                  DpRect           *srcCrop)
{
    uint32_t tmpDstWidth  = dstWidth;
    uint32_t tmpDstHeight = dstHeight;

    //contain rotation 90 and 270
    if ((DpOrientation)Orientation & ROT_90)
    {
        dstWidth  = tmpDstHeight;
        dstHeight = tmpDstWidth;
    }

    // Temporarily
    if ((srcWidth / dstWidth) > 20)
    {
        DPLOGE("DpBlitStream:: exceed HW limitation, srcWidth %d, dstWidth %d\n", srcWidth, dstWidth);
        return false;
    }

    if ((srcHeight / dstHeight) > 255)
    {
        DPLOGE("DpBlitStream:: exceed HW limitation, srcHeight %d, dstHeight %d\n", srcHeight, dstHeight);
        return false;
    }

    // Temporarily
    if ((dstWidth / srcWidth) > 32)
    {
        DPLOGE("DpBlitStream:: exceed HW limitation, dstWidth %d, srcWidth %d\n", dstWidth, srcWidth);
        return false;
    }

    if ((dstHeight / srcHeight) > 32)
    {
        DPLOGE("DpBlitStream:: exceed HW limitation, dstHeight %d, srcHeight %d\n", dstHeight, srcHeight);
        return false;
    }

    if (DP_COLOR_GET_H_SUBSAMPLE(srcFormat) && (srcWidth & 0x1))
    {
        DPLOGE("DpBlitStream:: invalid src width alignment\n");
        return false;
    }

    if (DP_COLOR_GET_V_SUBSAMPLE(srcFormat) && (srcHeight & 0x1))
    {
        DPLOGE("DpBlitStream:: invalid src height alignment\n");
        return false;
    }

    if (DP_COLOR_GET_H_SUBSAMPLE(dstFormat) && (dstWidth & 0x1))
    {
        DPLOGE("DpBlitStream:: invalid dst width alignment\n");
        return false;
    }

    if (DP_COLOR_GET_V_SUBSAMPLE(dstFormat) && (dstHeight & 0x1))
    {
        DPLOGE("DpBlitStream:: invalid dst height alignment\n");
        return false;
    }

    if ((srcWidth == dstWidth) && (srcHeight == dstHeight) &&
        (DP_COLOR_GET_HW_FORMAT(srcFormat) == 2 || DP_COLOR_GET_HW_FORMAT(srcFormat) == 3) &&
        (DP_COLOR_GET_HW_FORMAT(dstFormat) == 2 || DP_COLOR_GET_HW_FORMAT(dstFormat) == 3))
    {
        if (!DMA_SUPPORT_ALPHA_ROT)
        {
            DPLOGE("DpBlitStream:: unsupport alpha rotation on this platform\n");
            return false;
        }
        else
        {
            if (srcWidth < 9)
            {
                DPLOGE("DpBlitStream:: exceed HW limitation, srcWidth %d < 9\n", srcWidth);
                return false;
            }
        }
    }

    if (NULL != srcCrop && NULL != PqParam)
    {
#ifdef SUPPORT_VIDEO_DRE
        if (srcCrop->w < 48 &&
            MEDIA_VIDEO == PqParam->scenario &&
	        1 == PqParam->enable)
        {
            return false;
        }
#else
        DP_UNUSED(PqParam);
        DP_UNUSED(srcCrop);
#endif
    }

    return true;
}


DpBlitStream::DpBlitStream()
    : m_pStream(new DpStream(STREAM_BITBLT)),
      m_pChannel(new DpChannel()),
      m_channelID(-1),
      m_pSrcPool(new DpBasicBufferPool()),
      m_pDstPool(new DpBasicBufferPool()),
      m_srcBuffer(-1),
      m_srcWidth(-1),
      m_srcHeight(-1),
      m_srcYPitch(-1),
      m_srcUVPitch(-1),
      m_srcFormat(DP_COLOR_RGB888),
      m_srcProfile(DP_PROFILE_BT601),
      m_srcSecure(DP_SECURE_NONE),
      m_srcFlush(true),
      m_dstBuffer(-1),
      m_dstWidth(-1),
      m_dstHeight(-1),
      m_dstYPitch(-1),
      m_dstUVPitch(-1),
      m_dstFormat(DP_COLOR_RGB888),
      m_dstProfile(DP_PROFILE_BT601),
      m_dstSecure(DP_SECURE_NONE),
      m_dstFlush(true),
//#ifdef CONFIG_FOR_SOURCE_PQ
      m_pPqStream(NULL),
      m_pPqChannel(NULL),
      m_pPqPool(NULL),
      m_pqBuffer(-1),
//#endif
      m_cropXStart(-1),
      m_cropYStart(-1),
      m_cropWidth(-1),
      m_cropHeight(-1),
      m_cropSubPixelX(0),
      m_cropSubPixelY(0),
      m_targetXStart(-1),
      m_targetYStart(-1),
      m_roiWidth(-1),
      m_roiHeight(-1),
      m_rotation(0),
      m_frameChange(false),
      m_flipStatus(false),
      m_ditherStatus(false),
      m_userID(DP_BLIT_GENERAL_USER),
      m_PqID(0),
      m_pqSupport(0),
      mTdshp(0),
      m_pConvertBlk_NV12(NULL)
{
    memset(&m_PqConfig, 0, sizeof(m_PqConfig));
    memset(&m_PqParam, 0, sizeof(m_PqParam));

#ifndef BASIC_PACKAGE
    m_pqSupport = DpDriver::getInstance()->getPQSupport();
#endif
#ifndef BASIC_PACKAGE
    if (0 != m_pqSupport) {
        m_PqID = getPqID();
    }
#endif // BASIC_PACKAGE
}


DpBlitStream::~DpBlitStream()
{
    DPLOGI("DpBlitStream::destruct DpBlitStream object begin\n");

    delete m_pDstPool;
    m_pDstPool = NULL;

    delete m_pSrcPool;
    m_pSrcPool = NULL;

    delete m_pStream;
    m_pStream = NULL;

    delete m_pChannel;
    m_pChannel = NULL;

#ifdef CONFIG_FOR_SOURCE_PQ
    delete m_pPqPool;
    m_pPqPool = NULL;

    delete m_pPqStream;
    m_pPqStream = NULL;

    delete m_pPqChannel;
    m_pPqChannel = NULL;
#endif

#if CONFIG_FOR_OS_ANDROID
#ifndef BASIC_PACKAGE
    if (0 != m_pqSupport) {
        PQSessionManager::getInstance()->destroyPQSession(m_PqID);
    }
#endif // BASIC_PACKAGE
#endif // CONFIG_FOR_OS_ANDROID
    DPLOGI("DpBlitStream::destruct DpBlitStream object end\n");
}


DP_STATUS_ENUM DpBlitStream::setSrcBuffer(void     *pVABase,
                                          uint32_t size)
{
    DP_STATUS_ENUM status;

    DPLOGI("DpBlitStream: register source buffer with VA begin\n");

    if(-1 != m_srcBuffer)
    {
        m_pSrcPool->unregisterBuffer(m_srcBuffer);
        m_srcBuffer = -1;
    }

    status = m_pSrcPool->registerBuffer(&pVABase,
                                        &size,
                                        1,
                                        -1,
                                        &m_srcBuffer);

    DPLOGI("DpBlitStream: register source buffer with VA end(%d)", status);

    return status;
}


DP_STATUS_ENUM DpBlitStream::setSrcBuffer(void     **pVABaseList,
                                          uint32_t *pSizeList,
                                          uint32_t planeNumber)
{
    DP_STATUS_ENUM status;

    DPLOGI("DpBlitStream: register source buffer with VA begin\n");

    if(-1 != m_srcBuffer)
    {
        m_pSrcPool->unregisterBuffer(m_srcBuffer);
        m_srcBuffer = -1;
    }

    status = m_pSrcPool->registerBuffer(pVABaseList,
                                        pSizeList,
                                        planeNumber,
                                        -1,
                                        &m_srcBuffer);

    DPLOGI("DpBlitStream: register source buffer with VA end(%d)", status);

    return status;
}


DP_STATUS_ENUM DpBlitStream::setSrcBuffer(void**   pVABaseList,
                                          void**   pMVABaseList,
                                          uint32_t *pSizeList,
                                          uint32_t planeNumber)
{
    DP_STATUS_ENUM status;

    DPLOGI("DpBlitStream: register source buffer with MVA begin\n");

    if(-1 != m_srcBuffer)
    {
        m_pSrcPool->unregisterBuffer(m_srcBuffer);
        m_srcBuffer = -1;
    }

    if (pVABaseList == NULL)
    {
        status = m_pSrcPool->registerBuffer((uint32_t*)pMVABaseList,
                                        pSizeList,
                                        planeNumber,
                                        -1,
                                        &m_srcBuffer);
    }
    else
    {
        status = m_pSrcPool->registerBuffer(pVABaseList,
                                       (uint32_t*)pMVABaseList,
                                        pSizeList,
                                        planeNumber,
                                        -1,
                                        &m_srcBuffer);
    }

    DPLOGI("DpBlitStream: register source buffer with MVA end(%d)", status);

    return status;
}


DP_STATUS_ENUM DpBlitStream::setSrcBuffer(int32_t  fileDesc,
                                          uint32_t *pSizeList,
                                          uint32_t planeNumber)
{
    DP_STATUS_ENUM status;

    DPLOGI("DpBlitStream: register source buffer with FD begin\n");

    if(-1 != m_srcBuffer)
    {
        m_pSrcPool->unregisterBuffer(m_srcBuffer);
        m_srcBuffer = -1;
    }

    status = m_pSrcPool->registerBufferFD(fileDesc,
                                          pSizeList,
                                          planeNumber,
                                          -1,
                                          &m_srcBuffer);

    DPLOGI("DpBlitStream: register source buffer with FD end(%d)", status);

    return status;
}


DP_STATUS_ENUM DpBlitStream::setSrcConfig(int32_t           width,
                                          int32_t           height,
                                          DpColorFormat     format,
                                          DpInterlaceFormat,
                                          DpRect            *pROI)
{
    if ((width <= 0) || (height <= 0))
    {
        DPLOGE("DpBlitStream: invalid source width(%d), height(%d)\n", width, height);
        return DP_STATUS_INVALID_PARAX;
    }

    // check for frame change
    if ((m_srcWidth  != width)  ||
        (m_srcHeight != height) ||
        (m_srcFormat != format))
    {
        m_frameChange = true;
    }

    if (NULL != pROI)
    {
        if ((m_cropXStart    != pROI->x) ||
            (m_cropYStart    != pROI->y) ||
            (m_cropWidth     != pROI->w) ||
            (m_cropHeight    != pROI->h) ||
            (m_cropSubPixelX != pROI->sub_x) ||
            (m_cropSubPixelY != pROI->sub_y))
        {
            m_frameChange = true;
        }

        m_cropXStart    = pROI->x;
        m_cropYStart    = pROI->y;
        m_cropWidth     = pROI->w;
        m_cropHeight    = pROI->h;
        m_cropSubPixelX = pROI->sub_x;
        m_cropSubPixelY = pROI->sub_y;
    }
    else
    {
        if ((m_cropXStart    != 0)      ||
            (m_cropYStart    != 0)      ||
            (m_cropWidth     != width)  ||
            (m_cropHeight    != height) ||
            (m_cropSubPixelX != 0)      ||
            (m_cropSubPixelY != 0))
        {
            m_frameChange = true;
        }

        m_cropXStart    = 0;
        m_cropYStart    = 0;
        m_cropWidth     = width;
        m_cropHeight    = height;
        m_cropSubPixelX = 0;
        m_cropSubPixelY = 0;
    }

    m_srcWidth   = width;
    m_srcHeight  = height;
    m_srcFormat  = format;
    m_srcYPitch  = DP_COLOR_GET_MIN_Y_PITCH(format, width);
    m_srcUVPitch = DP_COLOR_GET_MIN_UV_PITCH(format, width);
    m_srcProfile = DP_PROFILE_BT601;
    m_srcFlush   = true;

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpBlitStream::setSrcConfig(int32_t           width,
                                          int32_t           height,
                                          int32_t           YPitch,
                                          int32_t           UVPitch,
                                          DpColorFormat     format,
                                          DP_PROFILE_ENUM   profile,
                                          DpInterlaceFormat,
                                          DpRect            *pROI,
                                          DpSecure          secure,
                                          bool              doFlush)
{
    if ((width <= 0) || (height <= 0) || (YPitch <= 0))
    {
        DPLOGE("DpBlitStream: invalid source width(%d), height(%d), Ypitch(%d)\n", width, height, YPitch);
        return DP_STATUS_INVALID_PARAX;
    }

    if (YPitch < DP_COLOR_GET_MIN_Y_PITCH(format, width))
    {
        DPLOGE("DpBlitStream: source Y pitch(%d) is less than min Y pitch(%d) for width(%d)\n", YPitch, DP_COLOR_GET_MIN_Y_PITCH(format, width), width);
        return DP_STATUS_INVALID_PARAX;
    }

    if (DP_COLOR_GET_PLANE_COUNT(format) > 1)
    {
        if (UVPitch < DP_COLOR_GET_MIN_UV_PITCH(format, width))
        {
            DPLOGE("DpBlitStream: source UV pitch(%d) is less than min UV pitch(%d) for width(%d)\n", UVPitch, DP_COLOR_GET_MIN_UV_PITCH(format, width), width);
            return DP_STATUS_INVALID_PARAX;
        }
    }

    if (DP_STATUS_RETURN_SUCCESS != m_pSrcPool->setSecureMode(secure))
    {
        return DP_STATUS_UNKNOWN_ERROR;
    }

    // check for frame change
    if ((m_srcWidth   != width)   ||
        (m_srcHeight  != height)  ||
        (m_srcFormat  != format)  ||
        (m_srcYPitch  != YPitch)  ||
        (m_srcUVPitch != UVPitch) ||
        (m_srcProfile != profile) ||
        (m_srcSecure  != secure)  ||
        (m_srcFlush   != doFlush))
    {
        m_frameChange = true;
    }

    if (NULL != pROI)
    {
        if ((m_cropXStart != pROI->x) ||
            (m_cropYStart != pROI->y) ||
            (m_cropWidth  != pROI->w) ||
            (m_cropHeight != pROI->h) ||
            (m_cropSubPixelX != pROI->sub_x) ||
            (m_cropSubPixelY != pROI->sub_y))
        {
            m_frameChange = true;
        }

        m_cropXStart    = pROI->x;
        m_cropYStart    = pROI->y;
        m_cropWidth     = pROI->w;
        m_cropHeight    = pROI->h;
        m_cropSubPixelX = pROI->sub_x;
        m_cropSubPixelY = pROI->sub_y;
    }
    else
    {
        if ((m_cropXStart != 0)      ||
            (m_cropYStart != 0)      ||
            (m_cropWidth  != width)  ||
            (m_cropHeight != height) ||
            (m_cropSubPixelX != 0)   ||
            (m_cropSubPixelY != 0))
        {
            m_frameChange = true;
        }

        m_cropXStart    = 0;
        m_cropYStart    = 0;
        m_cropWidth     = width;
        m_cropHeight    = height;
        m_cropSubPixelX = 0;
        m_cropSubPixelY = 0;
    }

    m_srcFormat  = format;
    m_srcWidth   = width;
    m_srcHeight  = height;
    m_srcYPitch  = YPitch;
    m_srcUVPitch = UVPitch;
    m_srcProfile = profile;
    m_srcSecure  = secure;
    m_srcFlush   = doFlush;

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpBlitStream::setDstBuffer(void     *pVABase,
                                          uint32_t size)
{
    DP_STATUS_ENUM status;

    DPLOGI("DpBlitStream: register target buffer with VA begin\n");

    if(-1 != m_dstBuffer)
    {
        m_pDstPool->cancelBuffer(m_dstBuffer);
        m_pDstPool->unregisterBuffer(m_dstBuffer);
        m_dstBuffer = -1;
    }

    status = m_pDstPool->registerBuffer(&pVABase,
                                        &size,
                                        1,
                                        -1,
                                        &m_dstBuffer);

    DPLOGI("DpBlitStream: register target buffer with VA end(%d)\n", status);

    return status;
}


DP_STATUS_ENUM DpBlitStream::setDstBuffer(void     **pVABaseList,
                                          uint32_t *pSizeList,
                                          uint32_t planeNumber)
{
    DP_STATUS_ENUM status;

    DPLOGI("DpBlitStream: register target buffer with VA begin\n");

    if(-1 != m_dstBuffer)
    {
        m_pDstPool->cancelBuffer(m_dstBuffer);
        m_pDstPool->unregisterBuffer(m_dstBuffer);
        m_dstBuffer = -1;
    }

    status = m_pDstPool->registerBuffer(pVABaseList,
                                        pSizeList,
                                        planeNumber,
                                        -1,
                                        &m_dstBuffer);

    DPLOGI("DpBlitStream: register target buffer with VA end(%d)\n", status);

    return status;
}


DP_STATUS_ENUM DpBlitStream::setDstBuffer(void**   pVABaseList,
                                          void**   pMVABaseList,
                                          uint32_t *pSizeList,
                                          uint32_t planeNumber)
{
    DP_STATUS_ENUM status;

    DPLOGI("DpBlitStream: register target buffer with MVA begin\n");

    if(-1 != m_dstBuffer)
    {
        m_pDstPool->cancelBuffer(m_dstBuffer);
        m_pDstPool->unregisterBuffer(m_dstBuffer);
        m_dstBuffer = -1;
    }

    if (pVABaseList == NULL)
    {
        status = m_pDstPool->registerBuffer((uint32_t*)pMVABaseList,
                                        pSizeList,
                                        planeNumber,
                                        -1,
                                        &m_dstBuffer);
    }
    else
    {
        status = m_pDstPool->registerBuffer(pVABaseList,
                                       (uint32_t*)pMVABaseList,
                                        pSizeList,
                                        planeNumber,
                                        -1,
                                        &m_dstBuffer);
    }

    DPLOGI("DpBlitStream: register target buffer with MVA end(%d)\n", status);

    return status;
}


DP_STATUS_ENUM DpBlitStream::setDstBuffer(int32_t  fileDesc,
                                          uint32_t *pSizeList,
                                          uint32_t planeNumber)
{
    DP_STATUS_ENUM status;

    DPLOGI("DpBlitStream: register target buffer with FD(%d) begin", fileDesc);

    if (-1 != m_dstBuffer)
    {
        m_pDstPool->cancelBuffer(m_dstBuffer);
        m_pDstPool->unregisterBuffer(m_dstBuffer);
        m_dstBuffer = -1;
    }

    status = m_pDstPool->registerBufferFD(fileDesc,
                                          pSizeList,
                                          planeNumber,
                                          -1,
                                          &m_dstBuffer);

    DPLOGI("DpBlitStream: register target buffer with FD end(%d)", status);

    return status;
}


DP_STATUS_ENUM DpBlitStream::setDstConfig(int32_t           width,
                                          int32_t           height,
                                          DpColorFormat     format,
                                          DpInterlaceFormat,
                                          DpRect            *pROI)
{
    if ((width <= 0) || (height <= 0))
    {
        DPLOGE("DpBlitStream: invalid target width(%d), height(%d)\n", width, height);
        return DP_STATUS_INVALID_PARAX;
    }

    if (NULL != pROI)
    {
        // roi_width must equal to width or width -1
        if (!(pROI->w == width) && !(pROI->w == width - 1))
        {
            DPLOGE("invalid width ROI setting. width = %d, roi_width = %d\n", width, pROI->w);
            return DP_STATUS_INVALID_PARAX;
        }
        // roi_height must equal to height or height -1
        if (!(pROI->h == height) && !(pROI->h == height - 1))
        {
            DPLOGE("invalid height ROI setting. height = %d, roi_height = %d\n", height, pROI->h);
            return DP_STATUS_INVALID_PARAX;
        }

        // ROI offset must be aligned
        if (DP_COLOR_GET_H_SUBSAMPLE(format) && (pROI->x & 0x1))
        {
            DPLOGE("invalid ROI x offset alignment\n");
            return DP_STATUS_INVALID_X_ALIGN;
        }
        if (DP_COLOR_GET_V_SUBSAMPLE(format) && (pROI->y & 0x1))
        {
            DPLOGE("invalid ROI y offset alignment\n");
            return DP_STATUS_INVALID_Y_ALIGN;
        }
    }

    // check for frame change
    if ((m_dstWidth   != width)  ||
        (m_dstHeight  != height) ||
        (m_dstFormat  != format))
    {
        m_frameChange = true;
    }

    if (NULL != pROI)
    {
        if ((m_targetXStart != pROI->x) ||
            (m_targetYStart != pROI->y) ||
            (m_roiWidth     != pROI->w) ||
            (m_roiHeight    != pROI->h))
        {
            m_frameChange = true;
        }

        m_targetXStart = pROI->x;
        m_targetYStart = pROI->y;
        m_roiWidth     = pROI->w;
        m_roiHeight    = pROI->h;
    }
    else
    {
        if ((m_targetXStart != 0)     ||
            (m_targetYStart != 0)     ||
            (m_roiWidth     != width) ||
            (m_roiHeight    != height))
        {
            m_frameChange = true;
        }

        m_targetXStart = 0;
        m_targetYStart = 0;
        m_roiWidth     = width;
        m_roiHeight    = height;
    }

    m_dstWidth     = width;
    m_dstHeight    = height;

    m_dstYPitch  = DP_COLOR_GET_MIN_Y_PITCH(format,  width);
    m_dstUVPitch = DP_COLOR_GET_MIN_UV_PITCH(format, width);
    m_dstFormat  = format;
    m_dstProfile = DP_PROFILE_BT601;
    m_dstFlush   = true;

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpBlitStream::setDstConfig(int32_t           width,
                                          int32_t           height,
                                          int32_t           YPitch,
                                          int32_t           UVPitch,
                                          DpColorFormat     format,
                                          DP_PROFILE_ENUM   profile,
                                          DpInterlaceFormat,
                                          DpRect            *pROI,
                                          DpSecure          secure,
                                          bool              doFlush)
{
    if ((width <= 0) || (height <= 0) || (YPitch <= 0))
    {
        DPLOGE("DpBlitStream: invalid target width(%d), height(%d), YPitch(%d)\n", width, height, YPitch);
        return DP_STATUS_INVALID_PARAX;
    }

    if (YPitch < DP_COLOR_GET_MIN_Y_PITCH(format, width))
    {
        DPLOGE("DpBlitStream: target Y pitch(%d) is less than min Y pitch(%d) for width(%d)\n", YPitch, DP_COLOR_GET_MIN_Y_PITCH(format, width), width);
        return DP_STATUS_INVALID_PARAX;
    }

    if (DP_COLOR_GET_PLANE_COUNT(format) > 1)
    {
        if (UVPitch < DP_COLOR_GET_MIN_UV_PITCH(format, width))
        {
            DPLOGE("DpBlitStream: target UV pitch(%d) is less than min UV pitch(%d) for width(%d)\n", UVPitch, DP_COLOR_GET_MIN_UV_PITCH(format, width), width);
            return DP_STATUS_INVALID_PARAX;
        }
    }

    if (NULL != pROI)
    {
        // roi_width must equal to width or width -1
        if (!(pROI->w == width) && !(pROI->w == width - 1))
        {
            DPLOGE("invalid width ROI setting. width = %d, roi_width = %d\n", width, pROI->w);
            return DP_STATUS_INVALID_PARAX;
        }
        // roi_height must equal to height or height -1
        if (!(pROI->h == height) && !(pROI->h == height - 1))
        {
            DPLOGE("invalid height ROI setting. height = %d, roi_height = %d\n", height, pROI->h);
            return DP_STATUS_INVALID_PARAX;
        }

        // ROI offset must be aligned
        if (DP_COLOR_GET_H_SUBSAMPLE(format) && (pROI->x & 0x1))
        {
            DPLOGE("invalid ROI x offset alignment\n");
            return DP_STATUS_INVALID_X_ALIGN;
        }
        if (DP_COLOR_GET_V_SUBSAMPLE(format) && (pROI->y & 0x1))
        {
            DPLOGE("invalid ROI y offset alignment\n");
            return DP_STATUS_INVALID_Y_ALIGN;
        }
    }

    if (DP_STATUS_RETURN_SUCCESS != m_pDstPool->setSecureMode(secure))
    {
        return DP_STATUS_UNKNOWN_ERROR;
    }

    // check for frame change
    if ((m_dstWidth   != width)   ||
        (m_dstHeight  != height)  ||
        (m_dstFormat  != format)  ||
        (m_dstYPitch  != YPitch)  ||
        (m_dstUVPitch != UVPitch) ||
        (m_dstProfile != profile) ||
        (m_dstSecure  != secure)  ||
        (m_dstFlush   != doFlush))
    {
        m_frameChange = true;
    }

    if (NULL != pROI)
    {
        if ((m_targetXStart != pROI->x) ||
            (m_targetYStart != pROI->y) ||
            (m_roiWidth     != pROI->w) ||
            (m_roiHeight    != pROI->h))
        {
            m_frameChange = true;
        }

        m_targetXStart = pROI->x;
        m_targetYStart = pROI->y;
        m_roiWidth     = pROI->w;
        m_roiHeight    = pROI->h;
    }
    else
    {
        if ((m_targetXStart != 0)     ||
            (m_targetYStart != 0)     ||
            (m_roiWidth     != width) ||
            (m_roiHeight    != height))
        {
            m_frameChange = true;
        }

        m_targetXStart = 0;
        m_targetYStart = 0;
        m_roiWidth     = width;
        m_roiHeight    = height;
    }

    m_dstWidth     = width;
    m_dstHeight    = height;

    m_dstYPitch  = YPitch;
    m_dstUVPitch = UVPitch;
    m_dstFormat  = format;
    m_dstProfile = profile;
    m_dstFlush   = doFlush;
    m_dstSecure  = secure;

    return DP_STATUS_RETURN_SUCCESS;
}

// Compatible to 6589
DP_STATUS_ENUM DpBlitStream::setOrientation(uint32_t transform)
{
    uint32_t flip;
    uint32_t rot;

    flip = 0;
    rot  = 0;

    // operate on FLIP_H, FLIP_V and ROT_90 respectively
    // to achieve the final orientation
    if (FLIP_H & transform)
    {
        flip ^= 1;
    }

    if (FLIP_V & transform)
    {
        // FLIP_V is equivalent to a 180-degree rotation with a horizontal flip
        rot += 180;
        flip ^= 1;
    }

    if (ROT_90 & transform)
    {
        rot += 90;
    }

    if ((m_flipStatus != ((0 != flip)? true: false)) ||
        (m_rotation   != (rot % 360)))
    {
        m_frameChange = true;
    }

    m_flipStatus = (0 != flip) ? true: false;
    m_rotation   = rot %= 360;

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpBlitStream::invalidate(struct timeval *endTime)
{
    DP_STATUS_ENUM status;
    void           *pBase[3] = {NULL, NULL, NULL};
    uint32_t       size[3] = {0, 0, 0};
    DpTimeValue    begin;
    DpTimeValue    end;
    int32_t        diff;

#ifdef CONFIG_FOR_SOURCE_PQ
    void           *pPqBase[3] = {NULL, NULL, NULL};
    uint32_t       Pqsize[3] = {0, 0, 0};
#endif
    DpBufferPoolBase *pDstPool;
    DpColorFormat     dstFormat;
    int32_t dstYPitch, dstUVPitch;
    int32_t targetXStart, targetYStart;
    int32_t enableLog = DpDriver::getInstance()->getEnableLog();
    char bufferInfoStr[256] = "";
    bool enHDR = false;

    DP_TRACE_CALL();
    DP_TIMER_GET_CURRENT_TIME(begin);

    status = m_pChannel->setEndTime(endTime);

    if ((NULL == m_pSrcPool) || (NULL == m_pDstPool))
    {
        return DP_STATUS_INVALID_BUFFER;
    }
    m_pStream->setPQReadback(m_PqConfig.enDC);
    m_pStream->setHDRReadback(m_PqConfig.enHDR);
    uint32_t tdshp = mTdshp ?
        mTdshp :
        (m_PqConfig.enDC || m_PqConfig.enSharp
#ifndef CONFIG_FOR_SOURCE_PQ
        || m_PqConfig.enColor
#endif
        );

    if (DpDriver::getInstance()->getDisableFrameChange())
    {
        m_frameChange = true;
    }

    if (m_frameChange)
    {
        status = m_pStream->resetStream();
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            DPLOGE("DpBlitStream: reset stream object failed %d\n", status);
            return status;
        }

        status = m_pChannel->resetChannel();
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            DPLOGE("DpBlitStream: reset stream channel failed %d\n", status);
            return status;
        }
        enHDR = m_PqConfig.enHDR;
#if defined(HDR_MT6779)
        enHDR = enHDR || m_PqConfig.enCcorr;
#endif

        status = m_pChannel->setSourcePort(PORT_MEMORY,
                                           m_srcFormat,
                                           m_srcWidth,
                                           m_srcHeight,
                                           m_srcYPitch,
                                           m_srcUVPitch,
                                           enHDR,
                                           m_PqConfig.enDRE, // No DRE in Blit
                                           m_pSrcPool,
                                           m_srcProfile,
                                           m_srcSecure,
                                           (DP_SECURE_NONE != m_srcSecure)? false: m_srcFlush);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            DPLOGE("DpBlitStream: set source port failed %d\n", status);
            return status;
        }

#ifdef CONFIG_FOR_SOURCE_PQ
        if ((m_PqConfig.enColor) && (m_srcSecure == DP_SECURE_NONE) && (m_dstSecure == DP_SECURE_NONE))
        {
            if (NULL == m_pPqPool)
            {
                m_pPqPool = new DpAutoBufferPool();
            }
            else
            {
                DPLOGI("DpBlitStream::release temp buffer\n");
                m_pPqPool->destroyBuffer();
            }

            dstFormat = DP_COLOR_YUYV;
            dstYPitch = DP_COLOR_GET_MIN_Y_PITCH(dstFormat, m_dstWidth);
            dstUVPitch = DP_COLOR_GET_MIN_UV_PITCH(dstFormat, m_dstWidth);
            targetXStart = 0;
            targetYStart = 0;
            pDstPool = m_pPqPool;

            m_pPqPool->createBuffer(dstFormat,
                                m_dstWidth,
                                m_dstHeight,
                                dstYPitch,
                                1);
        }
        else
#endif
        {
            dstFormat = m_dstFormat;
            dstYPitch = m_dstYPitch;
            dstUVPitch = m_dstUVPitch;
            targetXStart = m_targetXStart;
            targetYStart = m_targetYStart;
            pDstPool = m_pDstPool;
        }

#ifdef BASIC_PACKAGE
        uint32_t videoID = m_PqParam.u.video.id;
#else
        PQSessionManager* pPQSessionManager = PQSessionManager::getInstance();
        uint32_t videoID = pPQSessionManager->findVideoID(m_PqParam.u.video.id);
#endif // BASIC_PACKAGE
        uint64_t PQSessionID = ((static_cast<uint64_t>(m_PqID) << 32) | videoID);

        status = m_pChannel->addTargetPort(0,
                                           PORT_MEMORY,
                                           dstFormat,
                                           m_dstWidth,
                                           m_dstHeight,
                                           dstYPitch,
                                           dstUVPitch,
                                           m_rotation,
                                           m_flipStatus? true: false,
                                           PQSessionID,
                                           tdshp,
                                           m_ditherStatus? true: false,
                                           pDstPool,
                                           m_dstProfile,
                                           m_dstSecure,
                                           (DP_SECURE_NONE != m_dstSecure)? false: m_dstFlush);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            DPLOGE("DpBlitStream: add target port failed %d\n", status);
            return status;
        }

#if 0
        if (0 != (tdshp & 0xFFFF0000))
        {
            m_pStream->setPQReadback(true);
        }
#endif

        status = m_pChannel->setSourceCrop(0,
                                           m_cropXStart,
                                           m_cropSubPixelX,
                                           m_cropYStart,
                                           m_cropSubPixelY,
                                           m_cropWidth,
                                           0,
                                           m_cropHeight,
                                           0,
                                           true);

        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            DPLOGE("DpBlitStream: setSourceCrop failed %d\n", status);
            return status;
        }

        status = m_pChannel->setTargetROI(0,
                                          targetXStart,
                                          targetYStart,
                                          m_roiWidth,
                                          m_roiHeight);

        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            DPLOGE("DpBlitStream: setTargetROI failed %d\n", status);
            return status;
        }

        m_pStream->addChannel(m_pChannel, &m_channelID);
    }

    if (enableLog)
    {
        memset(bufferInfoStr, '\0', sizeof(bufferInfoStr));
        m_pSrcPool->dumpBufferInfo(bufferInfoStr, sizeof(bufferInfoStr));
        DPLOGD("DpBlit: in: (%d, %d, %d, %d, C%d%s%s%s%s%s%s, P%d), crop: (%d, %d, %d, %d, %d, %d), sec%d %s\n",
            m_srcWidth, m_srcHeight, m_srcYPitch, m_srcUVPitch,
            DP_COLOR_GET_UNIQUE_ID(m_srcFormat),
            DP_COLOR_GET_SWAP_ENABLE(m_srcFormat) ? "s" : "",
            DP_COLOR_GET_BLOCK_MODE(m_srcFormat) ? "b" : "",
            DP_COLOR_GET_INTERLACED_MODE(m_srcFormat) ? "i" : "",
            DP_COLOR_GET_UFP_ENABLE(m_srcFormat) ? "u" : "",
            DP_COLOR_GET_10BIT_TILE_MODE(m_srcFormat) ? "t" :
            DP_COLOR_GET_10BIT_PACKED(m_srcFormat) ? "p" :
            DP_COLOR_GET_10BIT_LOOSE(m_srcFormat) ? "l" : "",
            DP_COLOR_GET_10BIT_JUMP_MODE(m_srcFormat) ? "j" : "",
            m_srcProfile,
            m_cropXStart, m_cropYStart, m_cropWidth, m_cropHeight, m_cropSubPixelX, m_cropSubPixelY, m_srcSecure,
            bufferInfoStr);

        memset(bufferInfoStr, '\0', sizeof(bufferInfoStr));
        m_pDstPool->dumpBufferInfo(bufferInfoStr, sizeof(bufferInfoStr));
        DPLOGD("DpBlit: out: (%d, %d, %d, %d, C%d%s%s%s%s, P%d), misc: (X:%d, Y:%d, R:%d, F:%d, S:%d, D:%d), sec%d %s\n",
            m_dstWidth, m_dstHeight, m_dstYPitch, m_dstUVPitch,
            DP_COLOR_GET_UNIQUE_ID(m_dstFormat),
            DP_COLOR_GET_SWAP_ENABLE(m_dstFormat) ? "s" : "",
            DP_COLOR_GET_BLOCK_MODE(m_dstFormat) ? "b" : "",
            DP_COLOR_GET_INTERLACED_MODE(m_dstFormat) ? "i" : "",
            DP_COLOR_GET_UFP_ENABLE(m_dstFormat) ? "u" : "", m_dstProfile,
            m_targetXStart, m_targetYStart, m_rotation, m_flipStatus ? 1 : 0,
            tdshp, m_ditherStatus ? 1 : 0, m_dstSecure,
            bufferInfoStr);
    }

    // dequeue source buffer
    m_pSrcPool->dequeueBuffer(&m_srcBuffer, pBase, size);

    // queue and trigger the source buffer
    m_pSrcPool->queueBuffer(m_srcBuffer);

    DPLOGI("DpBlitStream::start stream\n");

#ifdef CONFIG_FOR_SOURCE_PQ
    if ((m_PqConfig.enColor) && (m_srcSecure == DP_SECURE_NONE) && (m_dstSecure == DP_SECURE_NONE))
    {
        m_pPqPool->activateBuffer();
    }
#endif

    if (false == m_frameChange)
    {
        DPLOGI("DpBlit: config frame only!\n");
        m_pStream->setConfigFlag(DpStream::CONFIG_FRAME_ONLY);
    }
    else
    {
        DPLOGI("DpBlit: config all!\n");
        m_pStream->setConfigFlag(DpStream::CONFIG_ALL);
    }

    status = m_pStream->startStream(m_frameChange);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpBlitStream::start stream failed: %d\n", status);
        m_pStream->stopStream();
        m_frameChange = true;
        return status;
    }

    DPLOGI("DpBlitStream::wait stream idle\n");

    status = m_pStream->waitStream();

    if (STREAM_DUAL_BITBLT == m_pStream->getScenario())
    {
        DPLOGD("DpBlitStream::dual pipe setHistogram\n");
        status = m_pStream->setHistogram();
    }

    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpBlitStream::waitstream failed: %d\n", status);
        m_pStream->stopStream();
        m_frameChange = true;
        return status;
    }

#ifdef CONFIG_FOR_SOURCE_PQ
    if ((m_PqConfig.enColor) && (m_srcSecure == DP_SECURE_NONE) && (m_dstSecure == DP_SECURE_NONE))
    {
        m_pPqPool->acquireBuffer(&m_pqBuffer, pPqBase, Pqsize);
        m_pPqPool->releaseBuffer(m_pqBuffer);

        DPLOGD("DpBlitStream: pq_process %d\n", m_PqConfig.enColor);
        pq_process();
    }
#endif

    // acqure and release destination buffer, this is to reset output buffer state
    m_pDstPool->acquireBuffer(&m_dstBuffer, pBase, size);

    m_pDstPool->releaseBuffer(m_dstBuffer);

    DPLOGI("DpBlitStream::stop stream operation\n");
    m_pStream->stopStream();

    DPLOGI("DpBlitStream::return with success status\n");

    DP_TIMER_GET_CURRENT_TIME(end);

    DP_TIMER_GET_DURATION_IN_MS(begin,
                                end,
                                diff);

    if (diff > 30)
    {
        DPLOGW("DpBlit: time %d ms, type %d, pq %d\n", diff, m_frameChange, m_PqConfig.enColor);
    }
    else
    {
        DPLOGI("DpBlit: time %d ms, type %d, pq %d\n", diff, m_frameChange, m_PqConfig.enColor);
    }

    m_frameChange = false;

    return DP_STATUS_RETURN_SUCCESS;
}

#ifdef CONFIG_FOR_SOURCE_PQ
#ifndef BASIC_PACKAGE
DP_STATUS_ENUM DpBlitStream::pq_process()
{
    DP_STATUS_ENUM status;
    int32_t  channelID = 0;
    void     *pBase[3] = {NULL, NULL, NULL};
    uint32_t size[3] = {0, 0, 0};
    DpColorFormat     dstFormat = DP_COLOR_YUYV;
    uint32_t dstYPitch  = DP_COLOR_GET_MIN_Y_PITCH(dstFormat, m_dstWidth);
    uint32_t dstUVPitch = DP_COLOR_GET_MIN_UV_PITCH(dstFormat, m_dstWidth);

    if (NULL == m_pPqStream)
    {
        m_pPqStream = new DpStream(STREAM_COLOR_BITBLT);
    }

    if (NULL == m_pPqChannel)
    {
        m_pPqChannel = new DpChannel();
    }

    if (DpDriver::getInstance()->getDisableFrameChange())
    {
        m_frameChange = true;
    }

    if (m_frameChange)
    {
        status = m_pPqStream->resetStream();
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            DPLOGE("pq_process: reset stream object failed %d\n", status);
            return status;
        }

        status = m_pPqChannel->resetChannel();
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            DPLOGE("pq_process: reset stream channel failed %d\n", status);
            return status;
        }

        status = m_pPqChannel->setSourcePort(PORT_MEMORY,
                                           dstFormat,
                                           m_dstWidth,
                                           m_dstHeight,
                                           dstYPitch,
                                           dstUVPitch,
                                           false,
                                           false,
                                           m_pPqPool,
                                           m_dstProfile,
                                           m_dstSecure,
                                           m_dstFlush);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            DPLOGE("pq_process: set source port failed %d\n", status);
            return status;
        }

        status = m_pPqChannel->addTargetPort(0,
                                           PORT_MEMORY,
                                           m_dstFormat,
                                           m_dstWidth,
                                           m_dstHeight,
                                           m_dstYPitch,
                                           m_dstUVPitch,
                                           0,
                                           false,
                                           0,
                                           0,
                                           false,
                                           m_pDstPool,
                                           m_dstProfile,
                                           m_dstSecure,
                                           m_dstFlush);

        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            DPLOGE("pq_process: add target port failed %d\n", status);
            return status;
        }

        status = m_pPqChannel->setSourceCrop(0,
                                           0,
                                           0,
                                           0,
                                           0,
                                           m_dstWidth,
                                           0,
                                           m_dstHeight,
                                           0,
                                           true);

        status = m_pPqChannel->setTargetROI(0,
                                          m_targetXStart,
                                          m_targetYStart,
                                          m_roiWidth,
                                          m_roiHeight);

        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            DPLOGE("pq_process::setTargetROI failed %d\n", status);
            return status;
        }

        m_pPqStream->addChannel(m_pPqChannel, &channelID);
    }

    m_pPqPool->dequeueBuffer(&m_pqBuffer, pBase, size);
    m_pPqPool->queueBuffer(m_pqBuffer);

    DPLOGI("pq_process::start stream\n");
    status = m_pPqStream->startStream(m_frameChange);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("pq_process::start stream failed: %d\n", status);
        m_pPqStream->stopStream();
        return status;
    }

    DPLOGI("pq_process::wait stream idle\n");
    status = m_pPqStream->waitStream();
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("pq_process::waitstream failed: %d\n", status);
        m_pPqStream->stopStream();
        return status;
    }

    m_pPqStream->stopStream();

    return status;
}
#endif //BASIC_PACKAGE
#endif // CONFIG_FOR_SOURCE_PQ

DP_STATUS_ENUM DpBlitStream::setUser(uint32_t eID)
{
    DP_STATUS_ENUM status;

    switch (eID)
    {
    case DP_BLIT_GPU:
    case DP_BLIT_HWC2:
        status = m_pStream->setScenario(STREAM_GPU_BITBLT);
        break;
    case DP_BLIT_HWC_120FPS:
        status = m_pStream->setScenario(STREAM_DUAL_BITBLT);
        break;
    case DP_BLIT_ADDITIONAL_DISPLAY:
        status = m_pStream->setScenario(STREAM_2ND_BITBLT);
        break;
    default:
        DPLOGE("DpBlitStream: unrecognizable user %d\n", eID);
        status = DP_STATUS_INVALID_PARAX;
    }

    if (DP_STATUS_RETURN_SUCCESS == status)
    {
        m_userID = (DpBlitUser)eID;
    }
    return status;
}

#ifndef BASIC_PACKAGE
uint32_t DpBlitStream::getPqID()
{
    AutoMutex lock(s_PqCountMutex);
    s_PqCount = (s_PqCount+1) & 0xFFFFFFF;

    DPLOGI("DpBlitStream::s_PqCount %x\n", s_PqCount);

    return (s_PqCount | DP_BLITSTREAM);
}
#endif // BASIC_PACKAGE

DP_STATUS_ENUM DpBlitStream::setPQParameter(const DpPqParam &pqParam)
{
#ifndef BASIC_PACKAGE
    if (0 != m_pqSupport) {
        DpPqConfig pqConfig;

        if (pqParam.scenario != MEDIA_VIDEO &&
            pqParam.scenario != MEDIA_PICTURE &&
            pqParam.scenario != MEDIA_VIDEO_CODEC)
        {
            DPLOGE("DpBlitStream: setPQParameter scenario %d\n", pqParam.scenario);
            return DP_STATUS_INVALID_PARAX;
        }

#if CONFIG_FOR_OS_ANDROID
        PQSessionManager* pPQSessionManager = PQSessionManager::getInstance();
        uint32_t videoID = pPQSessionManager->findVideoID(pqParam.u.video.id);
        uint64_t PQSessionID = (static_cast<uint64_t>(m_PqID) << 32) | videoID;

        PQSession* pPQsession = pPQSessionManager->createPQSession(PQSessionID);

#ifdef SUPPORT_VIDEO_DRE
        /*set dre off if secure video playback*/
        if (m_srcSecure == DP_SECURE && pqParam.scenario == MEDIA_VIDEO)
        {
            pPQsession->setSVP(true);
            DPLOGD("DpBlitStream::set dre off when playing svp video\n");
        }
#endif
        pPQsession->setPQparam(&pqParam);

        DpPqConfig* pDpPqConfig;
        pPQsession->getDpPqConfig(&pDpPqConfig);
        pqConfig = *pDpPqConfig;

        DPLOGI("DpBlitStream: pPQsession id %llx created\n", PQSessionID);
        DPLOGI("DpBlitStream: setPQParameter id %x enable %d scenario %d\n", m_PqID, pqParam.enable, pqParam.scenario);
        DPLOGI("DpBlitStream: getPQConfig sharp %d DC %d color %d\n", pqConfig.enSharp, pqConfig.enDC, pqConfig.enColor);

        if ((m_PqConfig.enColor != pqConfig.enColor) ||
            (m_PqConfig.enDC != pqConfig.enDC) ||
            (m_PqConfig.enSharp != pqConfig.enSharp) ||
            (m_PqConfig.enHDR != pqConfig.enHDR) ||
            (m_PqConfig.enCcorr != pqConfig.enCcorr) ||
            (m_PqConfig.enDRE != pqConfig.enDRE))
        {
            memcpy(&m_PqConfig, &pqConfig, sizeof(m_PqConfig));
            m_frameChange = true;
        }

        if (m_PqParam.u.video.id != pqParam.u.video.id)
        {
            m_frameChange = true;
        }

        memcpy(&m_PqParam, &pqParam, sizeof(pqParam));

#ifdef SUPPORT_VIDEO_DRE
        /* set dre setting for enDRE == 1 Video scenario*/
        DPLOGI("DpBlitStream: setPQParameter scenario[%d], enDRE[%d]\n",
            m_PqParam.scenario, m_PqConfig.enDRE);
        if (m_PqParam.scenario == MEDIA_VIDEO && m_PqConfig.enDRE == 1)
        {
            DPLOGI("DpBlitStream: setPQParameter set VIDEO DRE setting\n");
            m_PqParam.u.video.dpDREParam.SRAMId = DpDREParam::DRESRAM::SRAM00;
            m_PqParam.u.video.dpDREParam.cmd = DpDREParam::Cmd::Default | DpDREParam::Cmd::Initialize;
            m_PqParam.u.video.dpDREParam.userId = (((unsigned long long)MEDIA_VIDEO) << 32);
        }
#endif
#endif // CONFIG_FOR_OS_ANDROID
    }
#endif // BASIC_PACKAGE
    return DP_STATUS_RETURN_SUCCESS;
}

int32_t DpBlitStream::queryPaddingSide(uint32_t transform)
{
    /* padding side bit rule
    *
    * bit0: left
    * bit1: top
    * bit2: right
    * bit3: bottom
    *
    */
        int32_t padding = 0;
        uint32_t flip = 0;
        uint32_t rot  = 0;

        // operate on FLIP_H, FLIP_V and ROT_90 respectively
        // to achieve the final orientation
        if (FLIP_H & transform)
        {
            flip ^= 1;
        }

        if (FLIP_V & transform)
        {
            // FLIP_V is equivalent to a 180-degree rotation with a horizontal flip
            rot += 180;
            flip ^= 1;
        }

        if (ROT_90 & transform)
        {
            rot += 90;
        }

        rot %= 360;

        if (flip)
        {
            if (rot == 0)
                padding = 0b1001;
            else if (rot == 90)
                padding = 0b1100;
            else if (rot == 180)
                padding = 0b0110;
            else
                padding = 0b0011;
        }
        else
        {
            if (rot == 0)
                padding = 0b1100;
            else if (rot == 90)
                padding = 0b1001;
            else if (rot == 180)
                padding = 0b0011;
            else
                padding = 0b0110;
        }

        DPLOGD("DpBlitStream: rot(%d) flip(%d) padding(%x)\n", rot, flip, padding);

        return padding;
}
