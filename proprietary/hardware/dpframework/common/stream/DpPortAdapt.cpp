#include <sys/stat.h>
#include <errno.h>

#include "DpPortAdapt.h"
#include "DpChannel.h"
#include "DpWriteBMP.h"
#include "DpWriteBin.h"

int32_t DpPortAdapt::s_dumpID = 0;
DpMutex DpPortAdapt::s_dumpMutex;

DpPortAdapt::DpPortAdapt(int32_t          portId,
                         DpChannel        *pChannel,
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
                         bool             enHDR,
                         bool             enDRE,
                         bool             doDither,
                         DpBufferPoolBase *pool,
                         DP_PROFILE_ENUM  profile,
                         DpSecure         isSecure,
                         bool             doFlush,
                         DpColorMatrix    *matrix,
                         void*            Enc_cfg)
    : m_portID(portId),
      m_dumpIndex(0),
      m_dumpID(getDumpID()),
      m_pChannel(pChannel),
      m_portType(type),
      m_pPortPool(pool),
      m_portFormat(format),
      m_portWidth(width),
      m_portHeight(height),
      m_portYPitch(YPitch),
      m_portUVPitch(UVPitch),
      m_rotation(rotation),
      m_flipStatus(doFlip),
      m_pqIdStatus(pqSessionId),
      m_TDSHPStatus(parPQ),
      m_HDRStatus(enHDR),
      m_AALStatus(enDRE),
      m_ditherStatus(doDither),
      m_portProfile(profile),
      m_secureStatus(isSecure),
      m_flushStatus(doFlush),
      m_pColorMatrix(matrix)
{
    m_activeID[CLIENT_PRODUCER] = -1;
    m_activeID[CLIENT_CONSUMER] = -1;

    m_activeOut[CLIENT_PRODUCER] = 0;
    m_activeOut[CLIENT_CONSUMER] = 0;

    m_userCount[CLIENT_PRODUCER] = 0;
    m_userCount[CLIENT_CONSUMER] = 0;

    if (Enc_cfg && (type == PORT_JPEG))
    {
        memcpy(&m_jpegEnc_cfg, Enc_cfg, sizeof(m_jpegEnc_cfg));
    }
    else
    {
        memset(&m_jpegEnc_cfg, 0x0, sizeof(m_jpegEnc_cfg));
    }

    if (Enc_cfg && (type == PORT_VENC))
    {
        m_pVEnc_cfg = (DpVEnc_Config *)Enc_cfg;
    }
    else
    {
        m_pVEnc_cfg = NULL;
    }
}


DpPortAdapt::~DpPortAdapt()
{
}


DP_STATUS_ENUM DpPortAdapt::getVENCConfig(DpVEnc_Config** ppVEnc_cfg) const
{
    if (NULL == ppVEnc_cfg)
    {
        return DP_STATUS_INVALID_PARAX;
    }
    if (NULL == m_pVEnc_cfg)
    {
        return DP_STATUS_INVALID_PORT;
    }

    *ppVEnc_cfg = m_pVEnc_cfg;
    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpPortAdapt::getJPEGConfig(DpJPEGEnc_Config* jpegEnc_cfg) const
{
    memcpy(jpegEnc_cfg,&m_jpegEnc_cfg,sizeof(m_jpegEnc_cfg));
    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpPortAdapt::bindClient(CLIENT_TYPE_ENUM client)
{
    m_activeMutex[client].lock();
    m_userCount[client]++;
    m_activeMutex[client].unlock();

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpPortAdapt::pollStatus(CLIENT_TYPE_ENUM client,
                                       bool             waitPort)
{
    DP_STATUS_ENUM status;

    // Default status
    status = DP_STATUS_RETURN_SUCCESS;

    DPLOGI("DpPortAdapt: poll port status begin\n");

    m_activeMutex[client].lock();

    if (-1 == m_activeID[client])
    {
        DPLOGI("DpPortAdapt: get active buffer begin\n");
        status = getActiveBuffer(client, waitPort);
        DPLOGI("DpPortAdapt: get active buffer end\n");
    }
    else  //if (eRing == m_poolType)
    {
        // If we don't have active buffer, we may need to check buffer status
        DPLOGI("DpPortAdapt: poll buffer begin\n");
        status = m_pPortPool->pollBufferStatus(client,
                                               m_activeID[client],
                                               0,
                                               m_portWidth,
                                               0,
                                               m_portHeight,
                                               waitPort);
        DPLOGI("DpPortAdapt: poll buffer end\n");
    }

    m_activeMutex[client].unlock();

    DPLOGI("DpPortAdapt: poll port status end\n");

    return status;
}


DP_STATUS_ENUM DpPortAdapt::abortPoll(CLIENT_TYPE_ENUM client)
{
    DP_STATUS_ENUM status;

    if (m_pPortPool != NULL)
    {
        status = m_pPortPool->abortPoll(client);

        m_activeMutex[client].lock();
        m_activeID[client] = -1;
        m_activeMutex[client].unlock();
        return status;
    }

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpPortAdapt::getPortInfo(DpColorFormat  *pFormat,
                                       int32_t         *pWidth,
                                       int32_t         *pHeight,
                                       int32_t         *pYPitch,
                                       int32_t         *pUVPitch,
                                       DP_PROFILE_ENUM *pProfile,
                                       bool            *pDitherStatus) const
{
    if ((NULL == pFormat) ||
        (NULL == pWidth)  ||
        (NULL == pHeight) ||
        (NULL == pYPitch) ||
        (NULL == pUVPitch))
    {
        return DP_STATUS_INVALID_PARAX;
    }

    *pFormat = m_portFormat;
    *pWidth  = m_portWidth;
    *pHeight = m_portHeight;
    *pYPitch = m_portYPitch;
    *pUVPitch = m_portUVPitch;

    if (NULL != pProfile)
    {
        *pProfile = m_portProfile;
    }

    if (NULL != pDitherStatus)
    {
        *pDitherStatus = m_ditherStatus;
    }

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpPortAdapt::getSourceCrop(int32_t *pSrcXOffset,
                                          int32_t *pSrcXSubpixel,
                                          int32_t *pSrcYOffset,
                                          int32_t *pSrcYSubpixel,
                                          int32_t *pSrcCropWidth,
                                          int32_t *pSrcCropWidthSubpixel,
                                          int32_t *pSrcCropHeight,
                                          int32_t *pSrcCropHeightSubpixel) const
{
    DP_STATUS_ENUM status;

    if ((NULL == pSrcXOffset) ||
        (NULL == pSrcXSubpixel) ||
        (NULL == pSrcYOffset) ||
        (NULL == pSrcYSubpixel) ||
        (NULL == pSrcCropWidth) ||
        (NULL == pSrcCropWidthSubpixel) ||
        (NULL == pSrcCropHeight) ||
        (NULL == pSrcCropHeightSubpixel))
    {
        return DP_STATUS_INVALID_PARAX;
    }

    status = DP_STATUS_RETURN_SUCCESS;

    if (NULL == m_pChannel)
    {
        *pSrcXOffset    = 0;
        *pSrcXSubpixel  = 0;
        *pSrcYOffset    = 0;
        *pSrcYSubpixel  = 0;
        *pSrcCropWidth  = 0;
        *pSrcCropWidthSubpixel  = 0;
        *pSrcCropHeight = 0;
        *pSrcCropHeightSubpixel = 0;
    }
    else
    {
        status = m_pChannel->getSourceCrop(m_portID,
                                           pSrcXOffset,
                                           pSrcXSubpixel,
                                           pSrcYOffset,
                                           pSrcYSubpixel,
                                           pSrcCropWidth,
                                           pSrcCropWidthSubpixel,
                                           pSrcCropHeight,
                                           pSrcCropHeightSubpixel);
    }

    return status;
}

DP_STATUS_ENUM DpPortAdapt::getTargetROI(int32_t *pDstXOffset,
                                         int32_t *pDstYOffset,
                                         int32_t *pDstRoiWidth,
                                         int32_t *pDstRoiHeight) const

{
    DP_STATUS_ENUM status;

    if ((NULL == pDstXOffset) ||
        (NULL == pDstYOffset) ||
        (NULL == pDstRoiWidth) ||
        (NULL == pDstRoiHeight))
    {
        return DP_STATUS_INVALID_PARAX;
    }

    status = DP_STATUS_RETURN_SUCCESS;

    if (NULL == m_pChannel)
    {
        *pDstXOffset = 0;
        *pDstYOffset = 0;
    }
    else
    {
        status = m_pChannel->getTargetROI(m_portID,
                                          pDstXOffset,
                                          pDstYOffset,
                                          pDstRoiWidth,
                                          pDstRoiHeight);
    }

    return status;
}


DP_STATUS_ENUM DpPortAdapt::getHWAddress(CLIENT_TYPE_ENUM client,
                                        DpEngineType      type,
                                        uint32_t          base[3],
                                        uint32_t          size[3])
{
    DP_STATUS_ENUM status;
    int32_t        bufID;

    m_activeMutex[client].lock();

    memset(base, 0, sizeof(uint32_t)*3);
    memset(size, 0, sizeof(uint32_t)*3);

    if (-1 == m_activeID[client])
    {
        status = getActiveBuffer(client, false);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            m_activeMutex[client].unlock();
            return status;
        }
    }

    bufID = m_activeID[client];
    m_activeMutex[client].unlock();

    return m_pPortPool->getHWBaseWithID(bufID,
                                          type,
                                          base,
                                          size);
}


DP_STATUS_ENUM DpPortAdapt::getSWAddress(CLIENT_TYPE_ENUM client,
                                         void             *pBase[3],
                                         uint32_t         size[3])
{
    DP_STATUS_ENUM status;
    int32_t        bufID;

    m_activeMutex[client].lock();

    if (-1 == m_activeID[client])
    {
        status = getActiveBuffer(client, false);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            m_activeMutex[client].unlock();
            return status;
        }
    }

    bufID = m_activeID[client];
    m_activeMutex[client].unlock();

    return m_pPortPool->getSWBaseWithID(bufID,
                                        pBase,
                                        size);
}



DP_STATUS_ENUM DpPortAdapt::flipBuffer(CLIENT_TYPE_ENUM client)
{
    DP_STATUS_ENUM status = DP_STATUS_RETURN_SUCCESS;
    DPLOGI("DpPortAdapt: flip buffer begin\n");

    m_activeMutex[client].lock();

    if (-1 == m_activeID[client])
    {
        status = DP_STATUS_INVALID_BUFFER;
    }
    else
    {
        m_activeOut[client]--;

        if (m_activeOut[client] > 0)
        {
            if (CLIENT_PRODUCER == client)
            {
                m_producer.wait(m_activeMutex[client]);
            }
            else
            {
                m_consumer.wait(m_activeMutex[client]);
            }
        }
        else
        {
            if (CLIENT_PRODUCER == client)
            {
                m_pPortPool->queueBuffer(m_activeID[client]);
                m_activeID[client] = -1;
                m_producer.broadcast();
            }
            else
            {
                m_pPortPool->releaseBuffer(m_activeID[client]);
                m_activeID[client] = -1;
                m_consumer.broadcast();
            }
        }
    }

    m_activeMutex[client].unlock();

    DPLOGI("DpPortAdapt: flip buffer end\n");
    return status;
}


DP_STATUS_ENUM DpPortAdapt::flushBuffer(CLIENT_TYPE_ENUM client)
{
    DP_STATUS_ENUM status;
    int32_t        bufID;

    DPLOGI("DpPortAdapt: flush buffer begin\n");

    m_activeMutex[client].lock();
    bufID = m_activeID[client];
    m_activeMutex[client].unlock();

    if ((NULL == m_pPortPool) || (-1 == bufID))
    {
        DPLOGE("DpPortAdapt: flush buffer failed\n");
        return DP_STATUS_INVALID_BUFFER;
    }

    if (CLIENT_PRODUCER == client)
    {
        DPLOGI("DpPortAdapt: flush producer buffer\n");
        status = m_pPortPool->flushWriteBuffer(bufID);
    }
    else
    {
        DPLOGI("DpPortAdapt: flush consumer buffer\n");
        status = m_pPortPool->flushReadBuffer(bufID);
    }

    DPLOGI("DpPortAdapt: flush buffer end\n");

    return status;
}


DP_STATUS_ENUM DpPortAdapt::dumpBuffer(CLIENT_TYPE_ENUM client)
{
    int32_t     bufID;
    char        name[256] = "";

    m_activeMutex[client].lock();
    bufID = m_activeID[client];
    m_activeMutex[client].unlock();

    if ((NULL == m_pPortPool) || (-1 == bufID))
    {
        DPLOGE("DpPortAdapt: check buffer ID failed\n");
        return DP_STATUS_INVALID_BUFFER;
    }

    sprintf(name, "%05d_%02d-%s", m_dumpID, m_dumpIndex, CLIENT_PRODUCER == client ? "o" : "i");
    m_pPortPool->dumpBuffer(bufID, m_portFormat, m_portWidth, m_portHeight, m_portYPitch, m_portUVPitch, name);

    m_dumpIndex++;

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpPortAdapt::dumpDebugInfo(CLIENT_TYPE_ENUM client)
{
    if (m_pPortPool != NULL)
    {
        return m_pPortPool->dumpDebugInfo(client);
    }
    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpPortAdapt::getActiveBuffer(CLIENT_TYPE_ENUM client,
                                            bool             waitBuf)
{
    DP_STATUS_ENUM   status;
    int32_t          bufID;
    void             *pVABase[3];
    uint32_t         bufSize[3];
    BUFFER_TYPE_ENUM bufType;
    int32_t          plane;
    uint32_t         realSize;

    // Default value
    status = DP_STATUS_UNKNOWN_ERROR;

    if (CLIENT_PRODUCER == client)
    {
        status = m_pPortPool->dequeueBuffer(&bufID,
                                            pVABase,
                                            bufSize,
                                            waitBuf);
    }
    else
    {
        status = m_pPortPool->acquireBuffer(&bufID,
                                            pVABase,
                                            bufSize,
                                            waitBuf);
    }

    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        //DPLOGE("DpPortAdapt: get active buffer for client failed %d\n", status);
        return status;
    }

    status = m_pPortPool->getBufferType(&bufType);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpPortAdapt: get port buffer type failed %d\n", status);
        return status;
    }

    if (BUFFER_RING != bufType)
    {
        //realSize = m_portYPitch * m_portHeight;
        realSize = DP_COLOR_GET_MIN_Y_SIZE(m_portFormat, m_portWidth, m_portHeight);

        if (bufSize[0] < realSize)
        {
            DPLOGE("DpPortAdapt: incorrect plane 0 size %d for pitch %d and height %d\n", bufSize[0], m_portYPitch, m_portHeight);
            return DP_STATUS_INVALID_BUFFER;
        }

        plane  = DP_COLOR_GET_PLANE_COUNT(m_portFormat);
        if (plane > 1)
        {
            //realSize = (m_portUVPitch * (m_portHeight >> DP_COLOR_GET_V_SUBSAMPLE(m_portFormat)));
            realSize = DP_COLOR_GET_MIN_UV_SIZE(m_portFormat, m_portWidth, m_portHeight);

            if (bufSize[1] < realSize)
            {
                DPLOGE("DpPortAdapt: incorrect plane 1 size %d for pitch %d and height %d\n",
                            bufSize[1], m_portUVPitch, m_portHeight >> DP_COLOR_GET_V_SUBSAMPLE(m_portFormat));
                return DP_STATUS_INVALID_BUFFER;
            }
        }

        if (plane > 2)
        {
            if (bufSize[2] < realSize)
            {
                DPLOGE("DpPortAdapt: incorrect plane 2 size %d for pitch %d and height %d\n",
                            bufSize[2], m_portUVPitch, m_portHeight >> DP_COLOR_GET_V_SUBSAMPLE(m_portFormat));
                return DP_STATUS_INVALID_BUFFER;
            }
        }
    }

    // DP_STATUS_RETURN_SUCCESS == status
    m_activeID[client]  = bufID;
    m_activeOut[client] = m_userCount[client];

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpPortAdapt::getParameter(void *pPara, int32_t *pSize, uint32_t *pPABuffer) const
{
    return m_pChannel->getParameter(pPara, pSize, pPABuffer);
}

DP_STATUS_ENUM DpPortAdapt::getIsp2MdpParam(void *pPara, int32_t *pSize) const
{
    return m_pChannel->getIsp2MdpParam(pPara, pSize);
}

struct timeval DpPortAdapt::getEndTime() const
{
    return m_pChannel->getEndTime();
}

DP_STATUS_ENUM DpPortAdapt::waitFence(CLIENT_TYPE_ENUM client)
{
    DPLOGI("DpPortAdapt: waitFence begin\n");

    DP_STATUS_ENUM status;
    BUFFER_TYPE_ENUM bufType;
    int32_t        bufID;

    m_activeMutex[client].lock();
    bufID = m_activeID[client];
    m_activeMutex[client].unlock();

    if ((NULL == m_pPortPool) || (-1 == bufID))
    {
        DPLOGE("DpPortAdapt: waitFence failed\n");
        return DP_STATUS_INVALID_BUFFER;
    }

    status = m_pPortPool->getBufferType(&bufType);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpPortAdapt: get port buffer type failed %d\n", status);
        return status;
    }

    if (BUFFER_RING != bufType)
    {
        status = m_pPortPool->waitBufferFence(bufID);

        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            DPLOGE("DpPortAdapt: waitFence buffID %d failed %d\n", bufID, status);
        }
    }

    DPLOGI("DpPortAdapt: waitFence end\n");

    return status;
}

