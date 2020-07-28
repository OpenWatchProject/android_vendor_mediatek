#ifndef __DP_PORT_ADAPT_H__
#define __DP_PORT_ADAPT_H__

#include "DpDataType.h"
#include "DpBufferPool.h"

// Minimum Y pitch that is acceptable by HW
#define DP_COLOR_GET_MIN_Y_PITCH(color, width)                                              \
    (((DP_COLOR_BITS_PER_PIXEL(color) * width) + 4) >> 3)

// Minimum UV pitch that is acceptable by HW
#define DP_COLOR_GET_MIN_UV_PITCH(color, width)                                             \
    ((1 == DP_COLOR_GET_PLANE_COUNT(color))? 0:                                             \
     (((0 == DP_COLOR_IS_UV_COPLANE(color)) || (1 == DP_COLOR_GET_BLOCK_MODE(color)))?      \
      (DP_COLOR_GET_MIN_Y_PITCH(color, width)  >> DP_COLOR_GET_H_SUBSAMPLE(color)):         \
      ((DP_COLOR_GET_MIN_Y_PITCH(color, width) >> DP_COLOR_GET_H_SUBSAMPLE(color)) * 2)))

// Minimum Y plane size that is necessary in buffer
#define DP_COLOR_GET_MIN_Y_SIZE(color, width, height)                                       \
    ((1 == DP_COLOR_GET_BLOCK_MODE(color))?                                                 \
     (((DP_COLOR_BITS_PER_PIXEL(color) * width) >> 8) * height):                            \
     (DP_COLOR_GET_MIN_Y_PITCH(color, width) * height))

// Minimum UV plane size that is necessary in buffer
#define DP_COLOR_GET_MIN_UV_SIZE(color, width, height)                                                  \
    (((1 < DP_COLOR_GET_PLANE_COUNT(color)) && (1 == DP_COLOR_GET_BLOCK_MODE(color)))?                  \
     (((DP_COLOR_BITS_PER_PIXEL(color) * width) >> 8) * (height >> DP_COLOR_GET_V_SUBSAMPLE(color))):   \
     (DP_COLOR_GET_MIN_UV_PITCH(color, width) * (height >> DP_COLOR_GET_V_SUBSAMPLE(color))))

class DpChannel;

enum PORT_TYPE_ENUM
{
    PORT_MEMORY,
    PORT_MDP = PORT_MEMORY,
    PORT_JPEG,
    PORT_ISP,
    PORT_VENC,
    PORT_WPE,
    PORT_MDP2
};

class DpPortAdapt
{
public:
    DpPortAdapt(int32_t          portId,
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
                DP_PROFILE_ENUM  profile = DP_PROFILE_BT601,
                DpSecure         isSecure = DP_SECURE_NONE,
                bool             doFlush = true,
                DpColorMatrix    *matrix = NULL,
                void*            Enc_cfg = NULL);

    ~DpPortAdapt();

    inline DP_STATUS_ENUM getPortId(int32_t *pPortId) const;

    inline DP_STATUS_ENUM getPortType(PORT_TYPE_ENUM *pPortType) const;

    inline DpBufferPoolBase* getBufferPool() const;

    inline DP_STATUS_ENUM getBufferType(BUFFER_TYPE_ENUM *pBufType) const;

    inline int32_t getRotation() const;

    inline DpSecure getSecureStatus() const;

    inline bool getFlushStatus() const;

    inline bool getFlipStatus() const;

    inline uint64_t getPqIdStatus() const;

    inline uint32_t getTDSHPStatus() const;

    inline bool getHDRStatus() const;

    inline bool getAALStatus() const;

    inline bool getDitherStatus() const;

    inline DpColorMatrix *getColorMatrix() const;

    DP_STATUS_ENUM bindClient(CLIENT_TYPE_ENUM client);

    DP_STATUS_ENUM pollStatus(CLIENT_TYPE_ENUM client,
                              bool             waitPort = false);

    DP_STATUS_ENUM abortPoll(CLIENT_TYPE_ENUM client);

    DP_STATUS_ENUM getPortInfo(DpColorFormat   *pFormat,
                               int32_t         *pWidth,
                               int32_t         *pHeight,
                               int32_t         *pYPitch,
                               int32_t         *pUVPitch,
                               DP_PROFILE_ENUM *pProfile = 0,
                               bool            *pDitherStatus = 0) const;

    DP_STATUS_ENUM getSourceCrop(int32_t *pSrcXOffset,
                                 int32_t *pSrcXSubpixel,
                                 int32_t *pSrcYOffset,
                                 int32_t *pSrcYSubpixel,
                                 int32_t *pSrcCropWidth,
                                 int32_t *pSrcCropWidthSubpixel,
                                 int32_t *pSrcCropHeight,
                                 int32_t *pSrcCropHeightSubpixel) const;

    DP_STATUS_ENUM getTargetROI(int32_t *pDstXOffset,
                                int32_t *pDstYOffset,
                                int32_t *pDstRoiWidth,
                                int32_t *pDstRoiHeight) const;

    DP_STATUS_ENUM getHWAddress(CLIENT_TYPE_ENUM client,
                                DpEngineType     type,
                                uint32_t         base[3],
                                uint32_t         size[3]);

    DP_STATUS_ENUM getSWAddress(CLIENT_TYPE_ENUM client,
                                void             *pBase[3],
                                uint32_t         size[3]);

    DP_STATUS_ENUM flushBuffer(CLIENT_TYPE_ENUM client);

    DP_STATUS_ENUM flipBuffer(CLIENT_TYPE_ENUM client);

    DP_STATUS_ENUM dumpBuffer(CLIENT_TYPE_ENUM client);

    DP_STATUS_ENUM dumpDebugInfo(CLIENT_TYPE_ENUM client);

    //JPEG engine
    DP_STATUS_ENUM getJPEGConfig(DpJPEGEnc_Config* jpegEnc_cfg) const;

    //VENC engine
    DP_STATUS_ENUM getVENCConfig(DpVEnc_Config** VEnc_cfg) const;

    // Hole for ISP engine
    DP_STATUS_ENUM getParameter(void *pPara, int32_t *pSize, uint32_t *pPABuffer = NULL) const;

    // Hole for ISP engine met
    DP_STATUS_ENUM getIsp2MdpParam(void *pPara, int32_t *pSize) const;

    // Hole for expectedEndTime
    struct timeval getEndTime() const;

    DP_STATUS_ENUM waitFence(CLIENT_TYPE_ENUM client);

private:

#if CONFIG_FOR_OS_WINDOWS
    #define DUMP_ROOT   ".\\out\\"
#else
    #define DUMP_ROOT   (DpDriver::getInstance()->getdumpBufferFolder())
#endif  // CONFIG_FOR_OS_WINDOWS
    static int32_t        s_dumpID;
    static DpMutex        s_dumpMutex;

    static inline int32_t getDumpID();

    const int32_t         m_portID;
    int32_t               m_dumpIndex;
    int32_t               m_dumpID;
    DpCondition           m_producer;
    DpCondition           m_consumer;
    DpChannel             *m_pChannel;
    const PORT_TYPE_ENUM  m_portType;
    DpBufferPoolBase      *m_pPortPool;
    const DpColorFormat   m_portFormat;
    const int32_t         m_portWidth;
    const int32_t         m_portHeight;
    const int32_t         m_portYPitch;
    const int32_t         m_portUVPitch;
    const int32_t         m_rotation;
    const bool            m_flipStatus;
    const uint64_t        m_pqIdStatus;
    const uint32_t        m_TDSHPStatus;
    const bool            m_HDRStatus;
    const bool            m_AALStatus;
    const bool            m_ditherStatus;
    const DP_PROFILE_ENUM m_portProfile;
    const DpSecure        m_secureStatus;
    const bool            m_flushStatus;
    int32_t               m_userCount[2];
    int32_t               m_activeID[2];
    DpMutex               m_activeMutex[2];
    int32_t               m_activeOut[2];
    DpJPEGEnc_Config      m_jpegEnc_cfg;
    DpVEnc_Config         *m_pVEnc_cfg;
    DpColorMatrix         *m_pColorMatrix;

    DP_STATUS_ENUM getActiveBuffer(CLIENT_TYPE_ENUM client, bool waitBuf);
};


inline DP_STATUS_ENUM DpPortAdapt::getPortId(int32_t *pPortId) const
{
    if (NULL == pPortId)
    {
        return DP_STATUS_INVALID_PARAX;
    }

    *pPortId = m_portID;

    return DP_STATUS_RETURN_SUCCESS;
}

inline DP_STATUS_ENUM DpPortAdapt::getPortType(PORT_TYPE_ENUM *pPortType) const
{
    if (NULL == pPortType)
    {
        return DP_STATUS_INVALID_PARAX;
    }

    *pPortType = m_portType;

    return DP_STATUS_RETURN_SUCCESS;
}

inline DP_STATUS_ENUM DpPortAdapt::getBufferType(BUFFER_TYPE_ENUM *pBufType) const
{
    DP_STATUS_ENUM status;

    status = m_pPortPool->getBufferType(pBufType);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpPortAdapt: get port buffer type failed %d\n", status);
    }

    return status;
}

inline DpBufferPoolBase* DpPortAdapt::getBufferPool() const
{
    return m_pPortPool;
}

inline int32_t DpPortAdapt::getRotation() const
{
    return m_rotation;
}

inline DpSecure DpPortAdapt::getSecureStatus() const
{
    return m_secureStatus;
}

inline bool DpPortAdapt::getFlushStatus() const
{
    return m_flushStatus;
}

inline bool DpPortAdapt::getFlipStatus() const
{
    return m_flipStatus;
}

inline uint64_t DpPortAdapt::getPqIdStatus() const
{
    return m_pqIdStatus;
}

inline uint32_t DpPortAdapt::getTDSHPStatus() const
{
    return m_TDSHPStatus;
}

inline bool DpPortAdapt::getHDRStatus() const
{
    return m_HDRStatus;
}

inline bool DpPortAdapt::getAALStatus() const
{
    return m_AALStatus;
}

inline bool DpPortAdapt::getDitherStatus() const
{
    return m_ditherStatus;
}

inline DpColorMatrix *DpPortAdapt::getColorMatrix() const
{
    return m_pColorMatrix;
}

inline int32_t DpPortAdapt::getDumpID()
{
    AutoMutex lock(s_dumpMutex);
    return s_dumpID++;
}

#endif  // __DP_PORT_ADAPT_H__
