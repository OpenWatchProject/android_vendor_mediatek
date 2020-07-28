#ifndef __DP_CHANNEL_H__
#define __DP_CHANNEL_H__

#include "DpDataType.h"
#include "DpMutex.h"
#include "DpBufferPool.h"
#include "DpPortAdapt.h"
#include "DpList.h"

#define MAX_OUTPUT_PORT_NUM     4
#define DP_MAX_SINGLE_PABUFFER_COUNT    4
#define SUPPOSE_30_FPS    33333

class DpChannel
{
private:
    typedef ListPtr<DpPortAdapt> PortList;

    enum CHANNEL_STATE_ENUM
    {
        STATE_LOCKED,
        STATE_UNLOCKED
    };

public:
    typedef PortList::iterator PortIterator;

    DpChannel();

    ~DpChannel();

    DP_STATUS_ENUM lockChannel();

    DP_STATUS_ENUM unlockChannel();

    DP_STATUS_ENUM setSourcePort(PORT_TYPE_ENUM   type,
                                 DpColorFormat    format,
                                 int32_t          width,
                                 int32_t          height,
                                 int32_t          YPitch,
                                 int32_t          UVPitch,
                                 bool             enHDR,
                                 bool             enDRE,
                                 DpBufferPoolBase *pool,
                                 DP_PROFILE_ENUM  profile = DP_PROFILE_BT601,
                                 DpSecure         isSecure = DP_SECURE_NONE,
                                 bool             doFlush = true);

    DP_STATUS_ENUM addTargetPort(int32_t          portId,
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
                                 DP_PROFILE_ENUM  profile = DP_PROFILE_BT601,
                                 DpSecure         isSecure = DP_SECURE_NONE,
                                 bool             doFlush = true,
                                 DpColorMatrix    *matrix = NULL,
                                 void*            Enc_cfg = NULL);

    inline DpPortAdapt* getSourcePort();

    inline PortIterator beginOutPort();

    inline PortIterator endOutPort();

    inline uint32_t getOutPortNumber() const;

    inline DP_STATUS_ENUM setSourceCrop(int32_t portIndex,
                                        int32_t srcXOffset,
                                        int32_t srcYOffset,
                                        int32_t srcCropWidth,
                                        int32_t srcCropHeignt);

    DP_STATUS_ENUM setSourceCrop(int32_t portIndex,
                                 int32_t srcXOffset,
                                 int32_t srcXSubpixel,
                                 int32_t srcYOffset,
                                 int32_t srcYSubpixel,
                                 int32_t srcCropWidth,
                                 int32_t srcCropWidthSubpixel,
                                 int32_t srcCropHeight,
                                 int32_t srcCropHeightSubpixel,
                                 bool    bVerify = false);

    DP_STATUS_ENUM getSourceCrop(int32_t portIndex,
                                 int32_t *pSrcXOffset,
                                 int32_t *pSrcXSubpixel,
                                 int32_t *pSrcYOffset,
                                 int32_t *pSrcYSubpixel,
                                 int32_t *pSrcCropWidth,
                                 int32_t *pSrcCropWidthSubpixel,
                                 int32_t *pSrcCropHeignt,
                                 int32_t *pSrcCropHeigntSubpixel) const;

    DP_STATUS_ENUM setTargetROI(int32_t portIndex,
                                int32_t dstXOffset,
                                int32_t dstYOffset,
                                int32_t dstRoiWidth,
                                int32_t dstRoiHeight);

    DP_STATUS_ENUM getTargetROI(int32_t portIndex,
                                int32_t *pDstXOffset,
                                int32_t *pDstYOffset,
                                int32_t *pDstRoiWidth,
                                int32_t *pDstRoiHeight) const;

    // Hole for ISP engine
    DP_STATUS_ENUM setParameter(void    *pPara,
                                int32_t size,
                                uint32_t *pPABuffer = NULL);

    // Hole for ISP engine
    DP_STATUS_ENUM getParameter(void    *pPara,
                                int32_t *pSize,
                                uint32_t *pPABuffer = NULL) const;

    // Hole for ISP engine for met
    DP_STATUS_ENUM setIsp2MdpParam(void    *pPara,
                                   int32_t size);

    // Hole for ISP engine for met
    DP_STATUS_ENUM getIsp2MdpParam(void    *pPara,
                                   int32_t *pSize) const;

    // Hole for expectedEndTime
    DP_STATUS_ENUM setEndTime(struct timeval *endTime = NULL);

    // Hole for expectedEndTime
    inline struct timeval getEndTime() const;

    DP_STATUS_ENUM resetChannel();

    inline uint32_t getVENCInserted() const;

private:
    mutable DpMutex     m_channelMutex;
    CHANNEL_STATE_ENUM  m_currentState;
    DpPortAdapt         *m_pSourcePort;
    PortList            m_outPortList;
    int32_t             m_srcXOffset[MAX_OUTPUT_PORT_NUM];
    int32_t             m_srcXSubpixel[MAX_OUTPUT_PORT_NUM];
    int32_t             m_srcYOffset[MAX_OUTPUT_PORT_NUM];
    int32_t             m_srcYSubpixel[MAX_OUTPUT_PORT_NUM];
    int32_t             m_srcCropWidth[MAX_OUTPUT_PORT_NUM];
    int32_t             m_srcCropWidthSubpixel[MAX_OUTPUT_PORT_NUM];
    int32_t             m_srcCropHeight[MAX_OUTPUT_PORT_NUM];
    int32_t             m_srcCropHeightSubpixel[MAX_OUTPUT_PORT_NUM];
    void                *m_pExtraPara;
    int32_t             m_paraSize;
    int32_t             m_dstXOffset[MAX_OUTPUT_PORT_NUM];
    int32_t             m_dstYOffset[MAX_OUTPUT_PORT_NUM];
    int32_t             m_dstRoiWidth[MAX_OUTPUT_PORT_NUM];
    int32_t             m_dstRoiHeight[MAX_OUTPUT_PORT_NUM];
    uint32_t            m_VEncInserted;
    uint32_t            *m_pPABuffer;
    struct timeval      m_endTime;
    void                *m_pIsp2MdpPara;
    int32_t             m_Isp2MdpParaSize;

};


inline DpPortAdapt* DpChannel::getSourcePort()
{
    return m_pSourcePort;
}

inline DpChannel::PortIterator DpChannel::beginOutPort()
{
    return m_outPortList.begin();
}

inline DpChannel::PortIterator DpChannel::endOutPort()
{
    return m_outPortList.end();
}

inline uint32_t DpChannel::getOutPortNumber() const
{
    return m_outPortList.size();
}

inline DP_STATUS_ENUM DpChannel::setSourceCrop(int32_t portIndex,
                                               int32_t srcXOffset,
                                               int32_t srcYOffset,
                                               int32_t srcCropWidth,
                                               int32_t srcCropHeight)
{
    return setSourceCrop(portIndex, srcXOffset, 0, srcYOffset, 0, srcCropWidth, 0, srcCropHeight, 0, true);
}

inline uint32_t DpChannel::getVENCInserted() const
{
    return m_VEncInserted;
}

inline struct timeval DpChannel::getEndTime() const
{
    return m_endTime;
}
#endif // __DP_CHANNEL_H_
