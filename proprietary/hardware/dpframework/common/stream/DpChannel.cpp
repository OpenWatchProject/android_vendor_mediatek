#include "DpChannel.h"
#include "DpPortAdapt.h"
#include "DpProfiler.h"

DpChannel::DpChannel()
    : m_currentState(STATE_UNLOCKED),
      m_pSourcePort(0),
      m_pExtraPara(NULL),
      m_paraSize(0),
      m_VEncInserted(0),
      m_pPABuffer(NULL),
      m_pIsp2MdpPara(NULL),
      m_Isp2MdpParaSize(0)
{
    m_endTime.tv_sec = 0;
    m_endTime.tv_usec = 0;


    for (int i = 0; i < MAX_OUTPUT_PORT_NUM; i++)
    {
        m_srcXOffset[i]    = 0;
        m_srcXSubpixel[i]  = 0;
        m_srcYOffset[i]    = 0;
        m_srcYSubpixel[i]  = 0;
        m_srcCropWidth[i]  = 0;
        m_srcCropWidthSubpixel[i]  = 0;
        m_srcCropHeight[i] = 0;
        m_srcCropHeightSubpixel[i] = 0;
        m_dstXOffset[i] = 0;
        m_dstYOffset[i] = 0;
        m_dstRoiWidth[i] = 0;
        m_dstRoiHeight[i] = 0;
    }
}


DpChannel::~DpChannel()
{
    PortIterator iterator;

    delete m_pSourcePort;
    m_pSourcePort = NULL;

    for(iterator = m_outPortList.begin(); iterator != m_outPortList.end() ; iterator++)
    {
        delete &(*iterator);
    }

    free(m_pExtraPara);
    m_pExtraPara = NULL;

    free(m_pIsp2MdpPara);
    m_pIsp2MdpPara = NULL;

}


DP_STATUS_ENUM DpChannel::setSourcePort(PORT_TYPE_ENUM    type,
                                        DpColorFormat     format,
                                        int32_t           width,
                                        int32_t           height,
                                        int32_t           YPitch,
                                        int32_t           UVPitch,
                                        bool              enHDR,
                                        bool              enDRE,
                                        DpBufferPoolBase  *pool,
                                        DP_PROFILE_ENUM   profile,
                                        DpSecure          isSecure,
                                        bool              doFlush)
{
    DP_STATUS_ENUM status;
    DpPortAdapt    *pSource;

    DP_TRACE_CALL();

    if (PORT_MEMORY != type)
    {
        DPLOGE("DpChannel: unsupported source port type\n");
        return DP_STATUS_INVALID_PARAX;
    }

    if ((width   < 0) ||
        (height  < 0) ||
        (YPitch  < DP_COLOR_GET_MIN_Y_PITCH(format, width)) ||
        (UVPitch < DP_COLOR_GET_MIN_UV_PITCH(format, width)))
    {
        DPLOGE("DpChannel: invalid width(%d), height(%d), YPitch(%d) or UVPitch(%d)\n", width, height, YPitch, UVPitch);
        return DP_STATUS_INVALID_PARAX;
    }

    if (pool != NULL)
    {
        status = pool->pollBufferStatus(CLIENT_PRODUCER, false);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            DPLOGE("DpChannel: poll buffer status failed(%d)\n", status);
            return status;
        }
    }

    AutoMutex lock(m_channelMutex);

    if (STATE_LOCKED == m_currentState)
    {
        DPLOGE("DpChannel: source port is in locked state\n");
        return DP_STATUS_INVALID_STATE;
    }

    delete m_pSourcePort;
    m_pSourcePort = NULL;

    pSource = new DpPortAdapt(0,
                              this,
                              type,
                              format,
                              width,
                              height,
                              YPitch,
                              UVPitch,
                              0,      // No Rotate
                              false,  // No flip
                              0,      // No id
                              false,  // No sharpness
                              enHDR,
                              enDRE,
                              false,  // No dither
                              pool,
                              profile,
                              isSecure,
                              doFlush);
    if (NULL == pSource)
    {
        DPLOGE("DpChannel: can't allocate source port object\n");
        return DP_STATUS_OUT_OF_MEMORY;
    }

    m_pSourcePort = pSource;

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpChannel::addTargetPort(int32_t          portId,
                                        PORT_TYPE_ENUM   type,
                                        DpColorFormat    format,
                                        int32_t          width,
                                        int32_t          height,
                                        int32_t          YPitch,
                                        int32_t          UVPitch,
                                        int32_t          rotation,
                                        bool             doFlip,
                                        uint64_t         pqSessionId,
                                        uint32_t         parPQ,
                                        bool             doDither,
                                        DpBufferPoolBase *pool,
                                        DP_PROFILE_ENUM  profile,
                                        DpSecure         isSecure,
                                        bool             doFlush,
                                        DpColorMatrix    *matrix,
                                        void*            Enc_cfg)
{
    DP_STATUS_ENUM status;
    DpPortAdapt    *pTarget;

    DP_TRACE_CALL();

    if ((width   < 0) ||
        (height  < 0) ||
        (YPitch  < DP_COLOR_GET_MIN_Y_PITCH(format, width)) ||
        (UVPitch < DP_COLOR_GET_MIN_UV_PITCH(format, width)))
    {
        DPLOGE("DpChannel: invalid width(%d), height(%d), YPitch(%d) or UVPitch(%d)\n", width, height, YPitch, UVPitch);
        return DP_STATUS_INVALID_PARAX;
    }

    if (pool != NULL)
    {
        status = pool->pollBufferStatus(CLIENT_PRODUCER, false);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            DPLOGE("DpChannel: poll buffer status failed(%d)\n", status);
            return status;
        }
    }

    AutoMutex lock(m_channelMutex);

    if (PORT_VENC == type)
    {
        m_VEncInserted = VENC_ENABLE_FLAG;
    }

    if (STATE_LOCKED == m_currentState)
    {
        DPLOGE("DpChannel: target port is in locked state\n");
        return DP_STATUS_INVALID_STATE;
    }

    pTarget = new DpPortAdapt(portId,
                              this,
                              type,
                              format,
                              width,
                              height,
                              YPitch,
                              UVPitch,
                              rotation,
                              doFlip,
                              pqSessionId,
                              parPQ,
                              false, // No HDR
                              false, // No DRE
                              doDither,
                              pool,
                              profile,
                              isSecure,
                              doFlush,
                              matrix,
                              Enc_cfg);
    if (NULL == pTarget)
    {
        DPLOGE("DpChannel: can't allocate target port object\n");
        return DP_STATUS_OUT_OF_MEMORY;
    }

    m_outPortList.push_back(pTarget);

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpChannel::setSourceCrop(int32_t portIndex,
                                        int32_t srcXOffset,
                                        int32_t srcXSubpixel,
                                        int32_t srcYOffset,
                                        int32_t srcYSubpixel,
                                        int32_t srcCropWidth,
                                        int32_t srcCropWidthSubpixel,
                                        int32_t srcCropHeight,
                                        int32_t srcCropHeightSubpixel,
                                        bool    bVerify)
{
    DP_STATUS_ENUM  status;
    DpColorFormat   format;
    int32_t         width;
    int32_t         height;
    int32_t         YPitch;
    int32_t         UVPitch;
    DP_PROFILE_ENUM profile;

    if ((srcCropWidth  <= 0) ||
        (srcCropHeight <= 0))
    {
        DPLOGE("DpChannel: invalid crop width(%d) or height(%d)\n", srcCropWidth, srcCropHeight);
        return DP_STATUS_INVALID_PARAX;
    }

    AutoMutex lock(m_channelMutex);

    status = m_pSourcePort->getPortInfo(&format,
                                        &width,
                                        &height,
                                        &YPitch,
                                        &UVPitch,
                                        &profile);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpChannel: query port info failed(%d)\n", status);
        return status;
    }

    if (((srcXOffset + srcCropWidth) > width) ||
        ((srcYOffset + srcCropHeight) > height))
    {
        DPLOGW("DpChannel: invalid source crop setting, should be checked by ISP driver\n");
        if (bVerify)
        {
            return DP_STATUS_INVALID_PARAX;
        }
    }

    m_srcXOffset[portIndex]    = srcXOffset;
    m_srcXSubpixel[portIndex]  = srcXSubpixel;
    m_srcYOffset[portIndex]    = srcYOffset;
    m_srcYSubpixel[portIndex]  = srcYSubpixel;
    m_srcCropWidth[portIndex]  = srcCropWidth;
    m_srcCropWidthSubpixel[portIndex]  = srcCropWidthSubpixel;
    m_srcCropHeight[portIndex] = srcCropHeight;
    m_srcCropHeightSubpixel[portIndex] = srcCropHeightSubpixel;

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpChannel::getSourceCrop(int32_t portIndex,
                                        int32_t *pSrcXOffset,
                                        int32_t *pSrcXSubpixel,
                                        int32_t *pSrcYOffset,
                                        int32_t *pSrcYSubpixel,
                                        int32_t *pSrcCropWidth,
                                        int32_t *pSrcCropWidthSubpixel,
                                        int32_t *pSrcCropHeight,
                                        int32_t *pSrcCropHeightSubpixel) const
{
    if ((0 == pSrcXOffset) ||
        (0 == pSrcXSubpixel) ||
        (0 == pSrcYOffset) ||
        (0 == pSrcYSubpixel) ||
        (0 == pSrcCropWidth) ||
        (0 == pSrcCropWidthSubpixel) ||
        (0 == pSrcCropHeight) ||
        (0 == pSrcCropHeightSubpixel))
    {
        DPLOGE("DpChannel: invalid parameter to get source crop settings\n");
        DPLOGE("DpChannel: pXOffset(%p), pXSubpixel(%p),\n", pSrcXOffset, pSrcXSubpixel);
        DPLOGE("DpChannel: pYOffset(%p), pYSubpixel(%p),\n", pSrcYOffset, pSrcYSubpixel);
        DPLOGE("DpChannel: pCropWidth(%p), pCropHeight(%p)\n", pSrcCropWidth, pSrcCropHeight);
        return DP_STATUS_INVALID_PARAX;
    }

    AutoMutex lock(m_channelMutex);

    *pSrcXOffset    = m_srcXOffset[portIndex];
    *pSrcXSubpixel  = m_srcXSubpixel[portIndex];
    *pSrcYOffset    = m_srcYOffset[portIndex];
    *pSrcYSubpixel  = m_srcYSubpixel[portIndex];
    *pSrcCropWidth  = m_srcCropWidth[portIndex];
    *pSrcCropWidthSubpixel  = m_srcCropWidthSubpixel[portIndex];
    *pSrcCropHeight = m_srcCropHeight[portIndex];
    *pSrcCropHeightSubpixel = m_srcCropHeightSubpixel[portIndex];

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpChannel::lockChannel()
{
    AutoMutex lock(m_channelMutex);

    m_currentState = STATE_LOCKED;

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpChannel::unlockChannel()
{
    AutoMutex lock(m_channelMutex);

    m_currentState = STATE_UNLOCKED;

    return DP_STATUS_RETURN_SUCCESS;
}


// Hole for ISP engine
DP_STATUS_ENUM DpChannel::setParameter(void    *pPara,
                                       int32_t size,
                                       uint32_t *pPABuffer)
{
    AutoMutex lock(m_channelMutex);

    if (m_paraSize != size)
    {
        free(m_pExtraPara);

        m_pExtraPara = malloc(sizeof(uint8_t) * size);
        if (NULL == m_pExtraPara)
        {
            return DP_STATUS_OUT_OF_MEMORY;
        }

        m_paraSize   = size;
    }

    memcpy(m_pExtraPara, pPara, size);
    m_paraSize = size;
    m_pPABuffer = pPABuffer;

    return DP_STATUS_RETURN_SUCCESS;
}

// Hole for ISP engine
DP_STATUS_ENUM DpChannel::getParameter(void    *pPara,
                                       int32_t *pSize,
                                       uint32_t *pPABuffer) const
{
    if ((NULL == pPara) ||
        (NULL == pSize))
    {
        return DP_STATUS_INVALID_PARAX;
    }

    AutoMutex lock(m_channelMutex);

    memcpy(pPara, m_pExtraPara, m_paraSize);
    *pSize = m_paraSize;

    if ((m_pPABuffer != NULL) && (pPABuffer != NULL))
    {
        memcpy(pPABuffer, m_pPABuffer, DP_MAX_SINGLE_PABUFFER_COUNT * sizeof(uint32_t));

        DPLOGI("DpChannel: pPABuffer = %x\n", pPABuffer[0]);
    }


    return DP_STATUS_RETURN_SUCCESS;
}


// Hole for ISP engine for met
DP_STATUS_ENUM DpChannel::setIsp2MdpParam(void    *pPara,
                                          int32_t size)
{
    AutoMutex lock(m_channelMutex);

    if (m_Isp2MdpParaSize != size)
    {
        free(m_pIsp2MdpPara);

        m_pIsp2MdpPara = malloc(sizeof(uint8_t) * size);
        if (NULL == m_pIsp2MdpPara)
        {
            return DP_STATUS_OUT_OF_MEMORY;
        }

        m_Isp2MdpParaSize = size;
    }
    memcpy(m_pIsp2MdpPara, pPara, size);

    return DP_STATUS_RETURN_SUCCESS;
}

// Hole for ISP engine for met
DP_STATUS_ENUM DpChannel::getIsp2MdpParam(void    *pPara,
                                          int32_t *pSize) const
{
    if ((NULL == pPara) ||
        (NULL == pSize))
    {
        return DP_STATUS_INVALID_PARAX;
    }

    AutoMutex lock(m_channelMutex);

    memcpy(pPara, m_pIsp2MdpPara, m_Isp2MdpParaSize);
    *pSize = m_Isp2MdpParaSize;
    return DP_STATUS_RETURN_SUCCESS;
}

// Hole for expectedEndTime
DP_STATUS_ENUM DpChannel::setEndTime(struct timeval *endTime)
{
    AutoMutex lock(m_channelMutex);
    if (endTime == NULL)
    {
        DP_TIMER_GET_CURRENT_TIME(m_endTime);
        DPLOGI("DpChannel: m_endTime.tv_usec %lld ms\n", m_endTime.tv_usec);

        m_endTime.tv_usec += SUPPOSE_30_FPS; // suppose 30fps

        DPLOGI("DpChannel: m_endTime.tv_usec ++ %lld ms\n", m_endTime.tv_usec);
    }
    else
    {
        m_endTime = *endTime;
    }

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpChannel::resetChannel()
{
    DP_TRACE_CALL();
    PortIterator iterator;

    delete m_pSourcePort;
    m_pSourcePort = NULL;

    for(iterator = m_outPortList.begin(); iterator != m_outPortList.end(); iterator++)
    {
        delete &(*iterator);
    }
    m_outPortList.clear();

    for (int i = 0; i < MAX_OUTPUT_PORT_NUM; i++)
    {
        m_srcXOffset[i]    = 0;
        m_srcXSubpixel[i]  = 0;
        m_srcYOffset[i]    = 0;
        m_srcYSubpixel[i]  = 0;
        m_srcCropWidth[i]  = 0;
        m_srcCropWidthSubpixel[i]  = 0;
        m_srcCropHeight[i] = 0;
        m_srcCropHeightSubpixel[i] = 0;
    }

    m_VEncInserted  = 0;

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpChannel::setTargetROI(int32_t portIndex,
                                       int32_t dstXOffset,
                                       int32_t dstYOffset,
                                       int32_t dstRoiWidth,
                                       int32_t dstRoiHeight)
{
    DP_STATUS_ENUM  status = DP_STATUS_RETURN_SUCCESS;
    DpColorFormat   format;
    int32_t         width;
    int32_t         height;
    int32_t         YPitch;
    int32_t         UVPitch;
    DP_PROFILE_ENUM profile;
    PortIterator    iterator;

    DP_TRACE_CALL();

    if ((dstRoiWidth <= 0) ||
        (dstRoiHeight <= 0))
    {
        DPLOGE("DpChannel: invalid target roi_width(%d) or roi_height(%d)\n", dstRoiWidth, dstRoiHeight);
        return DP_STATUS_INVALID_PARAX;
    }

    AutoMutex lock(m_channelMutex);

    for (iterator = m_outPortList.begin(); iterator != m_outPortList.end(); iterator++)
    {
        status = iterator->getPortInfo(&format,
                                       &width,
                                       &height,
                                       &YPitch,
                                       &UVPitch,
                                       &profile);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            DPLOGE("DpChannel: query port info failed(%d)\n", status);
            return status;
        }
    }

#if 0
    if( ((dstXOffset + dstWidth)  > width) ||
        ((dstYOffset + dstHeight) > height) )
    {
        DPLOGE("DpChannel: invalid source crop setting\n");
        return DP_STATUS_INVALID_PARAX;
    }
#endif

    m_dstXOffset[portIndex] = dstXOffset;
    m_dstYOffset[portIndex] = dstYOffset;
    m_dstRoiWidth[portIndex]   = dstRoiWidth;
    m_dstRoiHeight[portIndex]  = dstRoiHeight;

    return status;
}


DP_STATUS_ENUM DpChannel::getTargetROI(int32_t portIndex,
                                       int32_t *pDstXOffset,
                                       int32_t *pDstYOffset,
                                       int32_t *pDstRoiWidth,
                                       int32_t *pDstRoiHeight) const
{
    if ((0 == pDstXOffset) ||
        (0 == pDstYOffset) ||
        (0 == pDstRoiWidth) ||
        (0 == pDstRoiHeight))
    {
        DPLOGE("DpChannel: invalid parameter to get target crop settings\n");
        DPLOGE("DpChannel: pXOffset(%p), pYOffset(%p), pDstRoiWidth(%p), pDstRoiHeight(%p)\n", pDstXOffset, pDstYOffset, pDstRoiWidth, pDstRoiHeight);
        return DP_STATUS_INVALID_PARAX;
    }

    AutoMutex lock(m_channelMutex);

    *pDstXOffset = m_dstXOffset[portIndex];
    *pDstYOffset = m_dstYOffset[portIndex];
    *pDstRoiWidth = m_dstRoiWidth[portIndex];
    *pDstRoiHeight = m_dstRoiHeight[portIndex];

    return DP_STATUS_RETURN_SUCCESS;
}

