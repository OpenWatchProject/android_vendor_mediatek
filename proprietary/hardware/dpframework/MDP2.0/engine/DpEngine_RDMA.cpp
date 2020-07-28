#include "DpTileEngine.h"
#include "DpEngineType.h"
#include "DpBufferPool.h"
#include "DpPlatform.h"
#include "mdp_reg_rdma.h"
#if defined(HDR_MT6799)
#include "mdp_reg_hdr.h"
#include "PQSessionManager.h"
#include <PQAlgorithmFactory.h>
#endif
#include "tile_mdp_reg.h"
#if ESL_SETTING
#include "DpESLControl.h"
#endif
#include "DpEngineSetting_RDMA.h"
#include <sys/types.h>
#include <unistd.h>
#ifdef RDMA0_RSZ1_SRAM_SHARING
#include "mmsys_reg_base.h"
#define PRZ_ENABLE                   (MDP_RSZ1_BASE + 0x0 )
#define PRZ_ENABLE_MASK              (0x010001)

void DpSramSharingControl(DpCommand &command)
{
    // Disable RSZ1 engine
    MM_REG_WRITE(command, PRZ_ENABLE, 0x00000000, 0x00000001);
}
#else
#define DpSramSharingControl(command)                      \
do {                                                       \
                                                           \
} while (0)
#endif

//#define JPEGDEC_CMDQ

//--------------------------------------------------------
// RDMA driver engine
//--------------------------------------------------------
class DpEngine_RDMA: public DpTileEngine
{
public:
    DpEngine_RDMA(uint32_t identifier)
        : DpTileEngine(identifier),
          m_useSimpleMode(true),
          m_sourceColorFormat(m_data.m_sourceColorFormat),
          m_sourceImagePitchY(0),
          m_sourceImagePitchUV(0),
          m_isRingBufferMode(m_data.m_isRingBufferMode),
          m_pRingBufferPool(m_data.m_pRingBufferPool),
          m_ringYStartLine(0),
          m_ringYEndLine(0),
          m_isVideoBlockMode(m_data.m_isVideoBlockMode),
          m_videoBlockWidth(0),
          m_videoBlockMaskW(0),
          m_videoBlockShiftW(m_data.m_videoBlockShiftW),
          m_videoBlockHeight(0),
          m_videoBlockMaskH(0),
          m_videoBlockShiftH(m_data.m_videoBlockShiftH),
          m_frameModeShift(0),
          m_horizontalShiftUV(0),
          m_verticalShiftUV(0),
          m_bitsPerPixelY(0),
          m_bitsPerPixelUV(0),
          m_enableUFO(0),
          m_cropOffsetX(m_data.m_cropOffsetX),
          m_cropOffsetY(m_data.m_cropOffsetY),
          m_cropWidth(m_data.m_cropWidth),
          m_cropHeight(m_data.m_cropHeight),
          m_outHistXLeft(0),
          m_outHistYTop(0),
          m_lastOutVertical(0),
          m_lastOutHorizontal(0),
          m_prevPABufferIndex(0),
          m_prevReadbackCount(0)
#ifdef JPEGDEC_CMDQ
          , pJpegDecCMDQ(NULL)
#endif
    {
        m_data.m_identifier = m_identifier;
        m_data.m_pPullDataFunc = funcPullBufferData;
        m_data.m_pFitConstraintFunc = funcFitConstraint;
        m_pData = &m_data;
        memset(m_regLabel, -1, sizeof(m_regLabel));
    }

    ~DpEngine_RDMA()
    {
    }

    virtual int64_t getEngineFlag()
    {
#ifdef RDMA0_RSZ1_SRAM_SHARING
        if (getEngineType() == tRDMA0)
        {
            return (1LL << tRDMA0) | (1LL << tSCL1);
        }
#endif
        return DpEngineBase::getEngineFlag();
    }

private:
    MDP_RDMA_DATA       m_data;

    bool                m_useSimpleMode;
    DpColorFormat       &m_sourceColorFormat;
    uint32_t            m_sourceImagePitchY;
    uint32_t            m_sourceImagePitchUV;
    bool                &m_isRingBufferMode;
    DpRingBufferPool    *&m_pRingBufferPool;
    int32_t             m_ringYStartLine;
    int32_t             m_ringYEndLine;
    bool                &m_isVideoBlockMode;
    uint32_t            m_videoBlockWidth;
    uint32_t            m_videoBlockMaskW;
    uint32_t            &m_videoBlockShiftW;
    uint32_t            m_videoBlockHeight;
    uint32_t            m_videoBlockMaskH;
    uint32_t            &m_videoBlockShiftH;
    uint32_t            m_frameModeShift;
    uint32_t            m_horizontalShiftUV;
    uint32_t            m_verticalShiftUV;

    uint32_t            m_bitsPerPixelY;
    uint32_t            m_bitsPerPixelUV;
    uint32_t            m_enableUFO;

    uint32_t            &m_cropOffsetX;
    uint32_t            &m_cropOffsetY;
    uint32_t            &m_cropWidth;
    uint32_t            &m_cropHeight;

    int32_t             m_outHistXLeft;
    int32_t             m_outHistYTop;
    int32_t             m_lastOutVertical;
    int32_t             m_lastOutHorizontal;

#ifdef JPEGDEC_CMDQ
    uint32_t            fJpegDecBaseAddr[3];
    JpegDecCMDQ         *pJpegDecCMDQ ;
#endif

    int32_t        m_regLabel[MAX_NUM_READBACK_PA_BUFFER];

    uint32_t       m_prevPABufferIndex;
    uint32_t       m_prevReadbackCount;

    DP_STATUS_ENUM mapColorFormat(DP_PROFILE_ENUM colorProfile,
                                  DpColorFormat   colorFormat,
                                  uint32_t        &sourceFormat,
                                  uint32_t        &sourceSwap,
                                  uint32_t        &blockMode,
                                  uint32_t        &fieldMode,
                                  uint32_t        &block10Bit,
                                  uint32_t        &blockTileMode,
                                  uint32_t        &colorTran,
                                  uint32_t        &matrixSel)
    {
        // Default value
        colorTran    = 0;
        matrixSel    = 0;

        sourceFormat = DP_COLOR_GET_HW_FORMAT(colorFormat);
        sourceSwap   = DP_COLOR_GET_SWAP_ENABLE(colorFormat);
        blockMode    = DP_COLOR_GET_BLOCK_MODE(colorFormat);
        fieldMode    = DP_COLOR_GET_INTERLACED_MODE(colorFormat);
        block10Bit   = DP_COLOR_GET_10BIT_PACKED(colorFormat);
        blockTileMode= DP_COLOR_GET_10BIT_TILE_MODE(colorFormat);

        m_useSimpleMode    = true;
        m_isVideoBlockMode = blockMode ? true : false;
        m_frameModeShift   = fieldMode ? 1 : 0;

        switch (colorFormat)
        {
            case DP_COLOR_GREY:
                // Y only
                m_bitsPerPixelY    = 8;
                m_bitsPerPixelUV   = 0;
                m_horizontalShiftUV = 0;
                m_verticalShiftUV   = 0;
                break;
            case DP_COLOR_RGB565:
            case DP_COLOR_BGR565:
                m_bitsPerPixelY    = 16;
                m_bitsPerPixelUV   = 0;
                m_horizontalShiftUV = 0;
                m_verticalShiftUV   = 0;
                colorTran           = 1;
                break;
            case DP_COLOR_RGB888:
            case DP_COLOR_BGR888:
                m_bitsPerPixelY    = 24;
                m_bitsPerPixelUV   = 0;
                m_horizontalShiftUV = 0;
                m_verticalShiftUV   = 0;
                colorTran           = 1;
                break;
            case DP_COLOR_RGBA8888:
            case DP_COLOR_BGRA8888:
            case DP_COLOR_ARGB8888:
            case DP_COLOR_ABGR8888:
#ifdef HW_SUPPORT_10BIT_PATH
            case DP_COLOR_RGBA1010102:
            case DP_COLOR_BGRA1010102:
#endif // HW_SUPPORT_10BIT_PATH
                m_bitsPerPixelY    = 32;
                m_bitsPerPixelUV   = 0;
                m_horizontalShiftUV = 0;
                m_verticalShiftUV   = 0;
                colorTran           = 1;
#ifdef RDMA_P010_CHANGE_SETTING
                if ((DP_COLOR_RGBA1010102 == colorFormat ||
                    DP_COLOR_BGRA1010102 == colorFormat) &&
                    !m_pInDataPort->getHDRStatus())
                {
                    DPLOGE("DpEngine_RDMA: this platform needs HDR to support RGBA1010102\n");
                    return DP_STATUS_INVALID_FORMAT;
                }
#endif
                break;
            case DP_COLOR_UYVY:
            case DP_COLOR_VYUY:
            case DP_COLOR_YUYV:
            case DP_COLOR_YVYU:
            case DP_COLOR_RGB565_RAW:
                // YUV422, 1 plane
                m_useSimpleMode     = RDMA_USE_SIMPLE_MODE;
                m_bitsPerPixelY    = 16;
                m_bitsPerPixelUV   = 0;
                m_horizontalShiftUV = 0;
                m_verticalShiftUV   = 0;
                break;
            case DP_COLOR_I420:
            case DP_COLOR_YV12:
                // YUV420, 3 plane
                m_bitsPerPixelY    = 8;
                m_bitsPerPixelUV   = 8;
                m_horizontalShiftUV = 1;
                m_verticalShiftUV   = 1;
                break;
            case DP_COLOR_I422:
            case DP_COLOR_YV16:
                // YUV422, 3 plane
                m_useSimpleMode     = RDMA_USE_SIMPLE_MODE;
                m_bitsPerPixelY    = 8;
                m_bitsPerPixelUV   = 8;
                m_horizontalShiftUV = 1;
                m_verticalShiftUV   = 0;
                break;
            case DP_COLOR_I444:
            case DP_COLOR_YV24:
                // YUV444, 3 plane
                m_bitsPerPixelY    = 8;
                m_bitsPerPixelUV   = 8;
                m_horizontalShiftUV = 0;
                m_verticalShiftUV   = 0;
                break;
            case DP_COLOR_NV12:
            case DP_COLOR_NV21:
                // YUV420, 2 plane
                m_bitsPerPixelY    = 8;
                m_bitsPerPixelUV   = 16;
                m_horizontalShiftUV = 1;
                m_verticalShiftUV   = 1;
                break;
            case DP_COLOR_420_BLKP_UFO:
            case DP_COLOR_420_BLKP_UFO_AUO:
            case DP_COLOR_420_BLKP:
                m_videoBlockWidth   = 16;
                m_videoBlockMaskW   = 0x0F;
                m_videoBlockShiftW  = 4;
                m_videoBlockHeight  = 32;
                m_videoBlockMaskH   = 0x1F;
                m_videoBlockShiftH  = 5;

                m_bitsPerPixelY    = 8;
                m_bitsPerPixelUV   = 16;
                m_horizontalShiftUV = 1;
                m_verticalShiftUV   = 1;
                break;
            case DP_COLOR_420_BLKI:
                // Calculate field height
                m_inFrameHeight     /= 2;
                m_outFrameHeight    /= 2;

                m_videoBlockWidth   = 16;
                m_videoBlockMaskW   = 0x0F;
                m_videoBlockShiftW  = 4;
                m_videoBlockHeight  = (32 / 2);  // Calculate field block height
                m_videoBlockMaskH   = 0x0F;
                m_videoBlockShiftH  = 4;

                m_bitsPerPixelY    = 8;
                m_bitsPerPixelUV   = 16;
                m_horizontalShiftUV = 1;
                m_verticalShiftUV   = 1;
                break;
            case DP_COLOR_422_BLKP:
                m_videoBlockWidth   = 16;
                m_videoBlockMaskW   = 0x0F;
                m_videoBlockShiftW  = 4;
                m_videoBlockHeight  = 32;
                m_videoBlockMaskH   = 0x1F;
                m_videoBlockShiftH  = 5;

                m_bitsPerPixelY    = 16;
                m_bitsPerPixelUV   = 0;
                m_horizontalShiftUV = 0;
                m_verticalShiftUV   = 0;
                break;
            case DP_COLOR_NV16:
            case DP_COLOR_NV61:
                // YUV422, 2 plane
                m_useSimpleMode     = RDMA_USE_SIMPLE_MODE;
                m_bitsPerPixelY    = 8;
                m_bitsPerPixelUV   = 16;
                m_horizontalShiftUV = 1;
                m_verticalShiftUV   = 0;
                break;
            case DP_COLOR_NV24:
            case DP_COLOR_NV42:
                // YUV444, 2 plane
                m_bitsPerPixelY    = 8;
                m_bitsPerPixelUV   = 16;
                m_horizontalShiftUV = 0;
                m_verticalShiftUV   = 0;
                break;
#if 0
            case DP_COLOR_UYVY_10P:
            case DP_COLOR_UYVY_10L:
            case DP_COLOR_VYUY_10L:
            case DP_COLOR_YUYV_10L:
            case DP_COLOR_YVYU_10L:
                // YUV422, 1 plane, 10bit
                //m_useSimpleMode     = false;
                m_bitsPerPixelY    = 20;
                m_bitsPerPixelUV   = 0;
                m_horizontalShiftUV = 0;
                m_verticalShiftUV   = 0;
                break;
            case DP_COLOR_NV21_10P:
                // YUV420, 2 plane, 10bit
                m_bitsPerPixelY    = 10;
                m_bitsPerPixelUV   = 20;
                m_horizontalShiftUV = 1;
                m_verticalShiftUV   = 1;
                break;
#endif // 0
#ifdef HW_SUPPORT_10BIT_PATH
            case DP_COLOR_NV12_10L:
            case DP_COLOR_NV21_10L:
                // P010 YUV420, 2 plane 10bit
                m_bitsPerPixelY    = 16;
                m_bitsPerPixelUV   = 32;
                m_horizontalShiftUV = 1;
                m_verticalShiftUV   = 1;
                break;
#endif // HW_SUPPORT_10BIT_PATH
            case DP_COLOR_420_BLKP_UFO_10_H:
            case DP_COLOR_420_BLKP_UFO_10_V:
            case DP_COLOR_420_BLKP_UFO_10_H_JUMP:
            case DP_COLOR_420_BLKP_UFO_10_V_JUMP:
            case DP_COLOR_420_BLKP_10_H:
            case DP_COLOR_420_BLKP_10_V:
            case DP_COLOR_420_BLKP_10_H_JUMP:
            case DP_COLOR_420_BLKP_10_V_JUMP:
                m_videoBlockWidth   = 16;
                m_videoBlockMaskW   = 0x0F;
                m_videoBlockShiftW  = 4;
                m_videoBlockHeight  = 32;
                m_videoBlockMaskH   = 0x1F;
                m_videoBlockShiftH  = 5;

                m_bitsPerPixelY    = 10;
                m_bitsPerPixelUV   = 20;
                m_horizontalShiftUV = 1;
                m_verticalShiftUV   = 1;
                break;
#if 0
            case DP_COLOR_NV16_10L:
            case DP_COLOR_NV61_10L:
                // YUV422, 2 plane, 10bit loose
                //m_useSimpleMode     = false;
                m_bitsPerPixelY    = 10;
                m_bitsPerPixelUV   = 20;
                m_horizontalShiftUV = 1;
                m_verticalShiftUV   = 0;
                break;
            case DP_COLOR_YV12_10L:
            case DP_COLOR_I420_10L:
                // YUV420, 3 plane, 10bit loose
                m_bitsPerPixelY    = 10;
                m_bitsPerPixelUV   = 10;
                m_horizontalShiftUV = 1;
                m_verticalShiftUV   = 1;
                break;
#endif // 0
            default:
                DPLOGE("DpEngine_RDMA: unsupported color format\n");
                assert(0);
                return DP_STATUS_INVALID_FORMAT;
        }

        /*
         * 4'b0000: RGB to JPEG
         * 4'b0010: RGB to BT601
         * 4'b0011: RGB to BT709
         * 4'b0100: JPEG to RGB
         * 4'b0110: BT601 to RGB
         * 4'b0111: BT709 to RGB
         * 4'b1000: JPEG to BT601
         * 4'b1001: JPEG to BT709
         * 4'b1010: BT601 to JPEG
         * 4'b1011: BT709 to JPEG
         * 4'b1100: BT709 to BT601
         * 4'b1101: BT601 to BT709
         */
        if (DP_PROFILE_BT2020      == colorProfile ||
            DP_PROFILE_FULL_BT709  == colorProfile ||
            DP_PROFILE_FULL_BT2020 == colorProfile)
        {
            colorProfile = DP_PROFILE_BT709;
        }

        if (1 == colorTran)
        {
            if (DP_PROFILE_BT601 == colorProfile)
            {
                matrixSel = 2;
            }
            else if (DP_PROFILE_BT709 == colorProfile)
            {
                matrixSel = 3;
            }
            else if (DP_PROFILE_JPEG == colorProfile)
            {
                matrixSel = 0;
            }
            else
            {
                DPLOGE("DpEngineRDMA: unknown color conversion\n");
                assert(0);
                return DP_STATUS_INVALID_PARAX;
            }
        }

        return DP_STATUS_RETURN_SUCCESS;
    }

    DP_STATUS_ENUM onInitEngine(DpCommand&);

    DP_STATUS_ENUM onDeInitEngine(DpCommand&);

    DP_STATUS_ENUM onConfigFrame(DpCommand&, DpConfig&);

    DP_STATUS_ENUM onReconfigFrame(DpCommand&, DpConfig&);

    DP_STATUS_ENUM onConfigTile(DpCommand&);

    DP_STATUS_ENUM onAdvanceTile(DpCommand&);

    DP_STATUS_ENUM onConfigHDRTile(DpCommand&);

    DP_STATUS_ENUM onWaitEvent(DpCommand&);

    DP_STATUS_ENUM onPostProc(DpCommand &);

    DP_STATUS_ENUM onUpdatePMQOS(DpCommand&, uint32_t&, uint32_t&, int32_t&, struct timeval&);

    int64_t onQueryFeature()
    {
#if DMA_SUPPORT_ALPHA_ROT
        return eRMEM | eRING | eALPHA_ROT;
#else
        return eRMEM | eRING;
#endif
    }

    DP_STATUS_ENUM onReconfigTiles(DpCommand &command);
};


// register factory function
static DpEngineBase* RDMA0Factory(DpEngineType type)
{
    if (tRDMA0 == type)
    {
        return new DpEngine_RDMA(0);
    }
    return NULL;
};

// register factory function
static DpEngineBase* RDMA1Factory(DpEngineType type)
{
    if (tRDMA1 == type)
    {
        return new DpEngine_RDMA(1);
    }
    return NULL;
};

#ifdef tRDMA2
// register factory function
static DpEngineBase* RDMA2Factory(DpEngineType type)
{
    if (tRDMA2 == type)
    {
        return new DpEngine_RDMA(2);
    }
    return NULL;
};
#endif

#ifdef tRDMA3
// register factory function
static DpEngineBase* RDMA3Factory(DpEngineType type)
{
    if (tRDMA3 == type)
    {
        return new DpEngine_RDMA(3);
    }
    return NULL;
};
#endif

// register factory function
EngineReg RDMA0Reg(RDMA0Factory);
EngineReg RDMA1Reg(RDMA1Factory);
#ifdef tRDMA2
EngineReg RDMA2Reg(RDMA2Factory);
#endif
#ifdef tRDMA3
EngineReg RDMA3Reg(RDMA3Factory);
#endif

DP_STATUS_ENUM DpEngine_RDMA::onInitEngine(DpCommand &command)
{
    DpSramSharingControl(command);
    // Reset engine
    MM_REG_WRITE(command, MDP_RDMA_RESET, 0x00000001, 0x00000001);
    MM_REG_POLL(command, MDP_RDMA_MON_STA_1, 0x00000100, 0x00000100);
    MM_REG_WRITE(command, MDP_RDMA_RESET, 0x00000000, 0x00000001);

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpEngine_RDMA::onDeInitEngine(DpCommand &)
{
#ifdef JPEGDEC_CMDQ
    if (m_isRingBufferMode && pJpegDecCMDQ)
    {
        delete pJpegDecCMDQ ;
        pJpegDecCMDQ = NULL;
    }
#endif
    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpEngine_RDMA::onConfigFrame(DpCommand &command,
                                            DpConfig  &config)
{
    DP_STATUS_ENUM status;
    uint32_t       ringBuffer;
    uint32_t       sourceFormat;
    uint32_t       sourceSwap;
    uint32_t       filterMode;
    uint32_t       blockMode;
    uint32_t       fieldMode;
    uint32_t       block10Bit;
    uint32_t       blockTileMode;
    uint32_t       colorTran;
    uint32_t       matrixSel;
    uint32_t       secureRegAddr[3] = {MDP_RDMA_SRC_BASE_0, MDP_RDMA_SRC_BASE_1, MDP_RDMA_SRC_BASE_2};
    uint32_t       offset[3] = {0};
    uint32_t       planeOffset[3] = {0};
    uint32_t       alphaRotation;
    int32_t        index = 0;
    uint32_t       RDMA0_lb_2b_mode     = 0;
    uint32_t       RDMA0_buffer_mode    = 0;
    uint32_t       RDMA0_simple_mode    = 0;
    uint32_t       RDMA0_output_10bit   = 0;

    // Basic image information
    m_sourceColorFormat  = config.inFormat;
    m_sourceImagePitchY  = config.inYPitch;
    m_sourceImagePitchUV = config.inUVPitch;

    m_cropOffsetX = config.inXOffset;
    m_cropOffsetY = config.inYOffset;
    m_cropWidth   = config.inCropWidth;
    m_cropHeight  = config.inCropHeight;

    command.addMMpathLog((m_identifier?"in_2":"in_1"), config.inFormat, DP_MMPATH_FORMAT);
    command.addMMpathLog((m_identifier?"in_2_width":"in_1_width"), config.inCropWidth, DP_MMPATH_OTHER);
    command.addMMpathLog((m_identifier?"in_2_height":"in_1_height"), config.inCropHeight, DP_MMPATH_OTHER);
    command.addMMpathLog((m_identifier?"MDP_RDMA1":"MDP_RDMA0"), 1, DP_MMPATH_OTHER);
    command.addMMpathLog((m_identifier?"in_2":"in_1"), config.memAddr[0], DP_MMPATH_ADDR);
    command.addMMpathLog("pid", (int)getpid(), DP_MMPATH_OTHER);

    command.addMetLog((m_identifier?"MDP_RDMA1__MF_SRC_SIZE":"MDP_RDMA0__MF_SRC_SIZE"),
            (config.inCropWidth << 0) + (config.inCropHeight <<  16));

    // Configure ring buffer
    m_isRingBufferMode   = config.enRing;
    if (m_isRingBufferMode)
    {
        ringBuffer = 1;
        m_pRingBufferPool = static_cast<DpRingBufferPool*>(m_pInDataPort->getBufferPool());
        assert(m_pRingBufferPool != 0);

#ifdef JPEGDEC_CMDQ
        if(m_pRingBufferPool->getEmbeddedJPEG())
        {
            pJpegDecCMDQ = new JpegDecCMDQ();
            if(NULL == pJpegDecCMDQ)
            {
                DPLOGE("new JpegDecCMDQ is failed!!!\n");
                return DP_STATUS_OUT_OF_MEMORY;
            }
        }
#endif
    }
    else
    {
        ringBuffer = 0;
#ifdef JPEGDEC_CMDQ
        pJpegDecCMDQ = NULL ;
#endif
    }

    if ((DP_COLOR_GET_V_SUBSAMPLE(config.inFormat)
        && !DP_COLOR_GET_V_SUBSAMPLE(config.outFormat))
        && !DP_COLOR_GET_BLOCK_MODE(config.inFormat))
    {
        filterMode = 2;//420 to 422 interpolation solution
    }
    else
    {
        filterMode = 3;//config.enRDMACrop ? 3 : 2; // when RSZ uses YUV422 format, RDMA could use V filter unless cropping.
    }

    m_enableUFO = config.enUFODec ? 1 : 0;

    status = mapColorFormat(config.inProfile,
                            config.inFormat,
                            sourceFormat,
                            sourceSwap,
                            blockMode,
                            fieldMode,
                            block10Bit,
                            blockTileMode,
                            colorTran,
                            matrixSel);

    if (config.enAlphaRot)
    {
        alphaRotation = 1;
        colorTran = 0;
    }
    else
    {
        alphaRotation = 0;
    }

    if (config.enBottomField)
    {
        fieldMode |= 0x2;
    }

    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpEngine_RDMA: map color format failed\n");
        return status;
    }

#if defined(HDR_MT6799)
    DPLOGI("DpEngine_RDMA: pqSessionId = %llx \n", config.pqSessionId);
    PQSession* pPQSession = PQSessionManager::getInstance()->getPQSession(config.pqSessionId);

    if (m_identifier == 0)
    {
        if (pPQSession != NULL)
        {
            MM_REG_WRITE(command, MDP_RDMA_TRANSFORM_0, (1         << 12), 0x00001000); //hdr enable

            PQAlgorithmFactory* pPQAlgorithmFactory = PQAlgorithmFactory::getInstance();

            PQHDRAdaptor* pPQHDRAdaptor = PQAlgorithmFactory::getInstance()->getHDR(m_identifier);
            dcHandle* currDC_p = pPQSession->getDCHandle();

            currDC_p->isHDRContent = pPQHDRAdaptor->calRegs(pPQSession, command, config);

            DPLOGI("DpEngine_RDMA: configFrame() hdrEnable = %d\n", currDC_p->isHDRContent);
        }
        else
        {
            MM_REG_WRITE(command, MDP_RDMA_TRANSFORM_0, (0         << 12), 0x00001000); //hdr disable
        }
    }
    else
    {
        if (pPQSession != NULL)
        {
            dcHandle* currDC_p = pPQSession->getDCHandle();

            currDC_p->isHDRContent = false;
        }
    }
#endif
    if (block10Bit) // || config.enDither)
    {
        if (DP_COLOR_IS_YUV(config.inFormat))
        {
            matrixSel = 0xF;
            colorTran = 1;
        }
        MM_REG_WRITE(command, MDP_RDMA_RESV_DUMMY_0, 0x00000007, 0x00000007);
    }
    else
    {
        MM_REG_WRITE(command, MDP_RDMA_RESV_DUMMY_0, 0x00000000, 0x00000007);
    }

    // Setup smi control
    MM_REG_WRITE(command, MDP_RDMA_GMCIF_CON, (1 <<  0) +
                                              (7 <<  4) + //burst type to 8
                                              (1 << 16),  //enable pre-ultra
                                              0x00030071);

// Setup source frame info
#if RDMA_SUPPORT_10BIT
#ifdef RDMA_P010_CHANGE_SETTING
    if ((DP_COLOR_NV12_10L == config.inFormat) ||
        (DP_COLOR_NV21_10L == config.inFormat))
    {
        sourceFormat = 6;
    }
    else if ((DP_COLOR_RGBA1010102 == config.inFormat) ||
        (DP_COLOR_BGRA1010102 == config.inFormat))
    {
        sourceFormat = 11;
        colorTran = 0;
    }
#endif

    MM_REG_WRITE(command, MDP_RDMA_SRC_CON, (sourceFormat <<  0) +
                                            //(cosite       <<  8) +
                                            (filterMode   <<  9) +
                                            (m_enableUFO  << 11) +
                                            (fieldMode    << 12) +
                                            (sourceSwap   << 14) +
                                            (blockMode    << 15) +
                                            (1            << 19) + //UFO reverse output
                                            (block10Bit   << 22) + //10-bit block mode
                                            (blockTileMode<< 23) + //HEVC tile mode
                                            (ringBuffer   << 24) +
                                            (alphaRotation<< 25) , 0x03C8FE0F);
#if HW_SUPPORT_EXTEND_UFO_FORMAT
    uint32_t auo = 0, jump = 0;
    if (block10Bit)
    {
        jump = DP_COLOR_GET_10BIT_JUMP_MODE(config.inFormat);
    }
    else
    {
        auo = DP_COLOR_GET_AUO_MODE(config.inFormat);
    }
    MM_REG_WRITE(command, MDP_RDMA_COMP_CON, (m_enableUFO  << 31) +
                                             (auo          << 29) +
                                             (jump         << 28) , 0xB0000000);
#endif

    command.addMetLog((m_identifier?"MDP_RDMA1__SRC_CON":"MDP_RDMA0__SRC_CON"),
                                  (sourceFormat <<  0) +
                                  //(cosite       <<  8) +
                                  (filterMode   <<  9) +
                                  (m_enableUFO  << 11) +
                                  (fieldMode    << 12) +
                                  (sourceSwap   << 14) +
                                  (blockMode    << 15) +
                                  (1            << 19) + //UFO reverse output
                                  (block10Bit   << 22) + //10-bit block mode
                                  (blockTileMode<< 23) + //HEVC tile mode
                                  (ringBuffer   << 24) +
                                  (alphaRotation<< 25));

    if (m_enableUFO)
    {
        // Setup source buffer base
        MM_REG_WRITE(command, MDP_RDMA_UFO_DEC_LENGTH_BASE_Y, config.memUFOLenAddr[0], 0xFFFFFFFF, &m_frameConfigLabel[index++]);
        MM_REG_WRITE(command, MDP_RDMA_UFO_DEC_LENGTH_BASE_C, config.memUFOLenAddr[1], 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    }
#else
    MM_REG_WRITE(command, MDP_RDMA_SRC_CON, (sourceFormat <<  0) +
                                            //(cosite       <<  8) +
                                            (filterMode   <<  9) +
                                            //(enableUFO    << 11) +
                                            (fieldMode    << 12) +
                                            (sourceSwap   << 14) +
                                            (blockMode    << 15) +
                                            //(1            << 19) + //UFO reverse output
                                            (ringBuffer   << 24), 0x0100F60F);

    command.addMetLog((m_identifier?"MDP_RDMA1__SRC_CON":"MDP_RDMA0__SRC_CON"),
                                 (sourceFormat <<  0) +
                                 //(cosite       <<  8) +
                                 (filterMode   <<  9) +
                                 //(enableUFO    << 11) +
                                 (fieldMode    << 12) +
                                 (sourceSwap   << 14) +
                                 (blockMode    << 15) +
                                 //(1            << 19) + //UFO reverse output
                                 (ringBuffer   << 24));

#endif

    RDMA0_lb_2b_mode     = m_isVideoBlockMode ? 0 : 1;
    RDMA0_simple_mode    = m_useSimpleMode ? 1 : 0;

#ifdef HW_SUPPORT_10BIT_PATH
    // 10 bit format
    if (DP_COLOR_GET_10BIT_PACKED(config.outFormat) ||
        DP_COLOR_GET_10BIT_LOOSE(config.outFormat) ||
        DP_COLOR_GET_10BIT_PACKED(config.inFormat) ||
        DP_COLOR_GET_10BIT_LOOSE(config.inFormat) ||
        config.scenario == STREAM_ISP_IC ||
        config.scenario == STREAM_ISP_VR ||
        config.scenario == STREAM_ISP_ZSD ||
        config.scenario == STREAM_ISP_IP ||
        config.scenario == STREAM_ISP_VSS ||
        config.scenario == STREAM_ISP_ZSD_SLOW ||
        config.scenario == STREAM_WPE ||
        config.scenario == STREAM_WPE2)
    {
        RDMA0_output_10bit = 1;
    }
#endif // HW_SUPPORT_10BIT_PATH
    MM_REG_WRITE(command, MDP_RDMA_CON, (RDMA0_lb_2b_mode   << 12) +
                                        (RDMA0_buffer_mode  <<  8) +
                                        (RDMA0_output_10bit <<  5) +
                                        (RDMA0_simple_mode  <<  4), 0x00001130);
    planeOffset[0] = 0;
    planeOffset[1] = config.memAddr[1];
    planeOffset[2] = config.memAddr[2];

    command.secure(m_engineType,
                   getEngineFlag(),
                   secureRegAddr,
                   m_pInDataPort->getSecureStatus(),
                   config.memAddr[0],
                   offset,
                   config.memSize,
                   planeOffset);

    command.frameSrcInfo(config.inFormat,
                         config.inWidth,
                         config.inHeight,
                         config.inYPitch,
                         config.inUVPitch,
                         config.memAddr,
                         config.memSize,
                         m_pInDataPort->getSecureStatus());

#ifdef JPEGDEC_CMDQ
    if(m_isRingBufferMode){
        fJpegDecBaseAddr[0] = config.memAddr[0];
        fJpegDecBaseAddr[1] = config.memAddr[1];
        fJpegDecBaseAddr[2] = config.memAddr[2];
    }
#endif

    // Setup source buffer base
    MM_REG_WRITE(command, MDP_RDMA_SRC_BASE_0, config.memAddr[0], 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_RDMA_SRC_BASE_1, config.memAddr[1], 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_RDMA_SRC_BASE_2, config.memAddr[2], 0xFFFFFFFF, &m_frameConfigLabel[index++]);

    // Setup source buffer end
    MM_REG_WRITE(command, MDP_RDMA_SRC_END_0, config.memAddr[0] + config.memSize[0], 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_RDMA_SRC_END_1, config.memAddr[1] + config.memSize[1], 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_RDMA_SRC_END_2, config.memAddr[2] + config.memSize[2], 0xFFFFFFFF, &m_frameConfigLabel[index++]);

    // Setup source frame pitch
    MM_REG_WRITE(command, MDP_RDMA_MF_BKGD_SIZE_IN_BYTE, m_sourceImagePitchY,  0x001FFFFF);
    MM_REG_WRITE(command, MDP_RDMA_SF_BKGD_SIZE_IN_BYTE, m_sourceImagePitchUV, 0x001FFFFF);

    // Setup color transform
    MM_REG_WRITE(command, MDP_RDMA_TRANSFORM_0, (matrixSel << 24) +
                                                (0         << 20) + //internal matrix table
                                                (colorTran << 16), 0x0F110000);

    if (false == m_useSimpleMode)
    {
        // Setup internal SRAM
        MM_REG_WRITE(command, MDP_RDMA_MB_BASE, 0x0000, 0x0FFFF);
        MM_REG_WRITE(command, MDP_RDMA_SB_BASE, 0x1200, 0x0FFFF);
    }

#if ESL_SETTING
    setESLRDMA(command, m_identifier, config.inFormat);
#else
    // Setup ultra threshold
    MM_REG_WRITE(command, MDP_RDMA_DMAULTRA_CON_0, 0x08080808, 0xFFFFFFFF);
    MM_REG_WRITE(command, MDP_RDMA_DMAULTRA_CON_1, 0x04040404, 0xFFFFFFFF);
    MM_REG_WRITE(command, MDP_RDMA_DMAULTRA_CON_2, 0x04040404, 0xFFFFFFFF);
    MM_REG_WRITE(command, MDP_RDMA_DMABUF_CON_0, (7 << 24) + 0x24, 0x700007F);
    MM_REG_WRITE(command, MDP_RDMA_DMABUF_CON_1, (3 << 24) + 0x13, 0x700003F);
    MM_REG_WRITE(command, MDP_RDMA_DMABUF_CON_2, (3 << 24) + 0x13, 0x700001F);
#endif

    m_outHistXLeft      = 0;
    m_outHistYTop       = 0;
    m_lastOutVertical   = 0;
    m_lastOutHorizontal = 0;
    command.addMetLog((m_identifier?"MDP_RDMA1__EN":"MDP_RDMA0__EN"), 1);
    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpEngine_RDMA::onReconfigFrame(DpCommand &command,
                                              DpConfig  &config)
{
    DP_STATUS_ENUM status;
    uint32_t       secureRegAddr[3] = {MDP_RDMA_SRC_BASE_0, MDP_RDMA_SRC_BASE_1, MDP_RDMA_SRC_BASE_2};
    uint32_t       offset[3] = {0};
    uint32_t       planeOffset[3] = {0};
    int32_t        index = 0;

    DPLOGI("DpEngine_RDMA: onReconfigFrame");

    planeOffset[0] = 0;
    planeOffset[1] = config.memAddr[1];
    planeOffset[2] = config.memAddr[2];

    command.secure(m_engineType,
                   getEngineFlag(),
                   secureRegAddr,
                   m_pInDataPort->getSecureStatus(),
                   config.memAddr[0],
                   offset,
                   config.memSize,
                   planeOffset);

#if RDMA_SUPPORT_10BIT
    if (m_enableUFO)
    {
        // Setup source buffer base
        MM_REG_WRITE(command, MDP_RDMA_UFO_DEC_LENGTH_BASE_Y, config.memUFOLenAddr[0], 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
        MM_REG_WRITE(command, MDP_RDMA_UFO_DEC_LENGTH_BASE_C, config.memUFOLenAddr[1], 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
    }
#endif

    MM_REG_WRITE(command, MDP_RDMA_SRC_BASE_0, config.memAddr[0], 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_RDMA_SRC_BASE_1, config.memAddr[1], 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_RDMA_SRC_BASE_2, config.memAddr[2], 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);

    // Setup source buffer end
    MM_REG_WRITE(command, MDP_RDMA_SRC_END_0, config.memAddr[0] + config.memSize[0], 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_RDMA_SRC_END_1, config.memAddr[1] + config.memSize[1], 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
    MM_REG_WRITE(command, MDP_RDMA_SRC_END_2, config.memAddr[2] + config.memSize[2], 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);

    command.addMMpathLog((m_identifier?"in_2":"in_1"), config.inFormat, DP_MMPATH_FORMAT);
    command.addMMpathLog((m_identifier?"in_2_width":"in_1_width"), config.inCropWidth, DP_MMPATH_OTHER);
    command.addMMpathLog((m_identifier?"in_2_height":"in_1_height"), config.inCropHeight, DP_MMPATH_OTHER);
    command.addMMpathLog((m_identifier?"MDP_RDMA1":"MDP_RDMA0"), 1, DP_MMPATH_OTHER);
    command.addMMpathLog((m_identifier?"in_2":"in_1"), config.memAddr[0], DP_MMPATH_ADDR);
    command.addMMpathLog("pid", (int)getpid(), DP_MMPATH_OTHER);

    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpEngine_RDMA::onConfigTile(DpCommand &command)
{
    DP_STATUS_ENUM status;
    uint32_t       RDMA0_lb_2b_mode     = m_isVideoBlockMode ? 0 : 1;
    uint32_t       RDMA0_buffer_mode    = 0;
    uint32_t       RDMA0_src_offset_0;
    uint32_t       RDMA0_src_offset_1;
    uint32_t       RDMA0_src_offset_2;
    uint32_t       RDMA0_mf_src_w;
    uint32_t       RDMA0_mf_src_h;
    uint32_t       RDMA0_mf_clip_w;
    uint32_t       RDMA0_mf_clip_h;
    uint32_t       RDMA0_mf_offset_w_1;
    uint32_t       RDMA0_mf_offset_h_1;

    // Enable engine
    MM_REG_WRITE(command, MDP_RDMA_EN, 0x00000001, 0x00000001);

    if (m_isVideoBlockMode)
    {
        m_inTileXLeft = ((m_inTileXLeft >> m_videoBlockShiftW) << m_videoBlockShiftW);  // Alignment X left in block boundary
        m_inTileYTop  = ((m_inTileYTop  >> m_videoBlockShiftH) << m_videoBlockShiftH);  // Alignment Y top  in block boundary
    }

#ifdef JPEGDEC_CMDQ
    if (m_isRingBufferMode && pJpegDecCMDQ &&
        (m_outHorizontal == 0))
    {
        DPLOGW("CMDQ_JPEGDEC: RDMA call jpeg, x_idx %d, addr %x %x %x !!",m_outHorizontal, fJpegDecBaseAddr[0], fJpegDecBaseAddr[1], fJpegDecBaseAddr[2]);
        if(m_inTileYTop == 0)
        {
            DpColorFormat format ;
            int32_t width ;
            int32_t height ;
            int32_t stride_Y ;
            int32_t stride_C ;
            int32_t mcu_h ;
            int32_t mcu_col ;
            uint32_t fmt ;

            m_pRingBufferPool->getBufferInfo(&format, &width, &height, &stride_Y, &stride_C, &mcu_h, &mcu_col);
            DPLOGW("CMDQ_JPEGDEC: getBufInfo fmt %x, width/height %d %d, stride %d %d, mcu_h %d, mcu_col %d!!", format, width, height, stride_Y, stride_C, mcu_h, mcu_col);
            fmt = (format == DP_COLOR_YV12 || format == DP_COLOR_I420)? 0 :
                  (format == DP_COLOR_YV16 || format == DP_COLOR_I422)? 1 :
                  (format == DP_COLOR_I444)? 2 : 3 ;

            pJpegDecCMDQ->setRingBufBase(fJpegDecBaseAddr[0], fJpegDecBaseAddr[1], fJpegDecBaseAddr[2]) ; //decode to YUV
            pJpegDecCMDQ->setRingBufInfo(width, height, fmt,
                                    m_pRingBufferPool->getRingBufferMCUCount(),
                                    mcu_h, stride_Y, stride_C, mcu_col);
        }

        pJpegDecCMDQ->jpeg_decode( command,m_inTileYTop,m_inTileYBottom ) ;
    }
#endif

    if (false == m_useSimpleMode)
    {
        RdmaInput inParam;
        RdmaOutput outParam;
        memset(&inParam, 0, sizeof(RdmaInput));
        memset(&outParam, 0, sizeof(RdmaOutput));

        inParam.RDMA0_lb_2b_mode = RDMA0_lb_2b_mode;
        inParam.RDMA0_buffer_mode = RDMA0_buffer_mode;
        inParam.identifier = m_identifier;
        inParam.colorFormat = m_sourceColorFormat;
        inParam.inTileXRight = m_inTileXRight;
        inParam.inTileXLeft = m_inTileXLeft;

        calRDMASetting(&inParam, &outParam);

        MM_REG_WRITE(command, MDP_RDMA_MB_DEPTH, outParam.RDMA0_mb_depth, 0x0000007F);

        MM_REG_WRITE(command, MDP_RDMA_MF_PAR, (outParam.RDMA0_mf_sb   << 12) +
                                               (outParam.RDMA0_mf_jump <<  0), 0x1FFFF3FF);

        MM_REG_WRITE(command, MDP_RDMA_MB_CON, (outParam.RDMA0_mb_lp   << 16) +
                                               (outParam.RDMA0_mb_pps  <<  0), 0x3FFF1FFF);

        MM_REG_WRITE(command, MDP_RDMA_SB_DEPTH, outParam.RDMA0_sb_depth, 0x0000007F);

        MM_REG_WRITE(command, MDP_RDMA_SF_PAR, (outParam.RDMA0_sf_sb   << 12) +
                                               (outParam.RDMA0_sf_jump <<  0), 0x1FFFF3FF);

        MM_REG_WRITE(command, MDP_RDMA_SB_CON, (outParam.RDMA0_sb_lp   << 16) +
                                               (outParam.RDMA0_sb_pps  <<  0), 0x3FFF1FFF);
    }

    if (false == m_isVideoBlockMode)
    {
        if (m_isRingBufferMode)
        {
            uint32_t       RDMA0_src_offset_w_0;
            uint32_t       RDMA0_src_offset_w_1;
            uint32_t       RDMA0_src_offset_w_2;

            RDMA0_src_offset_w_0 = m_inTileXLeft * m_bitsPerPixelY >> 3;
            MM_REG_WRITE(command, MDP_RDMA_SRC_OFFSET_W_0, RDMA0_src_offset_w_0, 0x0000FFFF);

            RDMA0_src_offset_w_1 = (m_inTileXLeft >> m_horizontalShiftUV) * m_bitsPerPixelUV >> 3;
            MM_REG_WRITE(command, MDP_RDMA_SRC_OFFSET_W_1, RDMA0_src_offset_w_1, 0x0000FFFF);

            RDMA0_src_offset_w_2 = (m_inTileXLeft >> m_horizontalShiftUV) * m_bitsPerPixelUV >> 3;
            MM_REG_WRITE(command, MDP_RDMA_SRC_OFFSET_W_2, RDMA0_src_offset_w_2, 0x0000FFFF);

            DPLOGI("DpEngine_RDMA: window offset Y: 0x%08x, U: 0x%08x, V: 0x%08x\n", RDMA0_src_offset_w_0, RDMA0_src_offset_w_1, RDMA0_src_offset_w_2);

            status = m_pRingBufferPool->mapBufferLine(m_inTileYTop,
                                                      m_inTileYBottom,
                                                      &m_ringYStartLine,
                                                      &m_ringYEndLine);
            if (DP_STATUS_RETURN_SUCCESS != status)
            {
                return status;
            }
        }
        else
        {
            m_ringYStartLine = m_inTileYTop;
        }

        // Set Y pixel offset
        RDMA0_src_offset_0  = (m_inTileXLeft * m_bitsPerPixelY >> 3) + m_ringYStartLine * m_sourceImagePitchY;
        MM_REG_WRITE(command, MDP_RDMA_SRC_OFFSET_0, RDMA0_src_offset_0, 0xFFFFFFFF);

        // Set U pixel offset
        RDMA0_src_offset_1  = ((m_inTileXLeft    >> m_horizontalShiftUV) * m_bitsPerPixelUV >> 3) +
                              (m_ringYStartLine >> m_verticalShiftUV) * m_sourceImagePitchUV;
        MM_REG_WRITE(command, MDP_RDMA_SRC_OFFSET_1, RDMA0_src_offset_1, 0xFFFFFFFF);

        // Set V pixel offset
        RDMA0_src_offset_2  = ((m_inTileXLeft    >> m_horizontalShiftUV) * m_bitsPerPixelUV >> 3) +
                              (m_ringYStartLine >> m_verticalShiftUV) * m_sourceImagePitchUV;
        MM_REG_WRITE(command, MDP_RDMA_SRC_OFFSET_2, RDMA0_src_offset_2, 0xFFFFFFFF);

        DPLOGI("DpEngine_RDMA: pixel offset Y: 0x%08x, U: 0x%08x, V: 0x%08x\n", RDMA0_src_offset_0, RDMA0_src_offset_1, RDMA0_src_offset_2);

        // Set source size
        RDMA0_mf_src_w      = m_inTileXRight   - m_inTileXLeft + 1;
        RDMA0_mf_src_h      = m_inTileYBottom  - m_inTileYTop  + 1;
        MM_REG_WRITE(command, MDP_RDMA_MF_SRC_SIZE, (RDMA0_mf_src_h << 16) +
                                                    (RDMA0_mf_src_w <<  0), 0x1FFF1FFF);

        // Set target size
        RDMA0_mf_clip_w     = m_outTileXRight  - m_outTileXLeft + 1;
        RDMA0_mf_clip_h     = m_outTileYBottom - m_outTileYTop + 1;
        MM_REG_WRITE(command, MDP_RDMA_MF_CLIP_SIZE, (RDMA0_mf_clip_h << 16) +
                                                     (RDMA0_mf_clip_w <<  0), 0x1FFF1FFF);

        // Set crop offset
        RDMA0_mf_offset_w_1 = m_lumaXOffset;
        RDMA0_mf_offset_h_1 = m_lumaYOffset;
        MM_REG_WRITE(command, MDP_RDMA_MF_OFFSET_1, (RDMA0_mf_offset_h_1 << 16) +
                                                    (RDMA0_mf_offset_w_1 <<  0), 0x003F001F);
    }
    else
    {
        // Set Y pixel offset
        RDMA0_src_offset_0 = (m_inTileXLeft * (m_videoBlockHeight << m_frameModeShift) * m_bitsPerPixelY >> 3) +
                             (m_inTileYTop >> m_videoBlockShiftH) * m_sourceImagePitchY;
        MM_REG_WRITE(command, MDP_RDMA_SRC_OFFSET_0, RDMA0_src_offset_0, 0xFFFFFFFF);

#if RDMA_SUPPORT_10BIT
        // Set 10bit UFO mode
        if (DP_COLOR_GET_10BIT_PACKED(m_sourceColorFormat) && m_enableUFO)
        {
            MM_REG_WRITE(command, MDP_RDMA_SRC_OFFSET_0_P, (RDMA0_src_offset_0 << 2) / 5, 0xFFFFFFFF);

            // Set 10bit source frame pitch
            MM_REG_WRITE(command, MDP_RDMA_MF_BKGD_SIZE_IN_PXL, (m_sourceImagePitchY << 2) / 5, 0x001FFFFF);
        }
#endif

        // Set U pixel offset
        RDMA0_src_offset_1 = ((m_inTileXLeft >> m_horizontalShiftUV) * ((m_videoBlockHeight >> m_verticalShiftUV) << m_frameModeShift) * m_bitsPerPixelUV >> 3) +
                             (m_inTileYTop >> m_videoBlockShiftH) * m_sourceImagePitchUV;
        MM_REG_WRITE(command, MDP_RDMA_SRC_OFFSET_1, RDMA0_src_offset_1, 0xFFFFFFFF);

        // Set V pixel offset
        RDMA0_src_offset_2 = ((m_inTileXLeft >> m_horizontalShiftUV) * ((m_videoBlockHeight >> m_verticalShiftUV) << m_frameModeShift) * m_bitsPerPixelUV >> 3) +
                             (m_inTileYTop >> m_videoBlockShiftH) * m_sourceImagePitchUV;
        MM_REG_WRITE(command, MDP_RDMA_SRC_OFFSET_2, RDMA0_src_offset_2, 0xFFFFFFFF);

        DPLOGI("DpEngine_RDMA: pixel offset Y: 0x%08x, U: 0x%08x, V: 0x%08x\n", RDMA0_src_offset_0, RDMA0_src_offset_1, RDMA0_src_offset_2);

        // Set source size
        RDMA0_mf_src_w      = m_inTileXRight  - m_inTileXLeft + 1;
        RDMA0_mf_src_h      = (m_inTileYBottom - m_inTileYTop + 1) << m_frameModeShift;
        MM_REG_WRITE(command, MDP_RDMA_MF_SRC_SIZE, (RDMA0_mf_src_h << 16) +
                                                    (RDMA0_mf_src_w <<  0), 0x1FFF1FFF);

        // Set target size
        RDMA0_mf_clip_w     = m_outTileXRight - m_outTileXLeft + 1;
        RDMA0_mf_clip_h     = (m_outTileYBottom - m_outTileYTop + 1) << m_frameModeShift;
        MM_REG_WRITE(command, MDP_RDMA_MF_CLIP_SIZE, (RDMA0_mf_clip_h << 16) +
                                                     (RDMA0_mf_clip_w <<  0), 0x1FFF1FFF);

        // Set crop offset
        RDMA0_mf_offset_w_1 = (m_outTileXLeft - m_inTileXLeft);
        RDMA0_mf_offset_h_1 = (m_outTileYTop  - m_inTileYTop) << m_frameModeShift;
        MM_REG_WRITE(command, MDP_RDMA_MF_OFFSET_1, (RDMA0_mf_offset_h_1 << 16) +
                                                    (RDMA0_mf_offset_w_1 <<  0), 0x003F001F);
    }

#if RDMA_UPSAMPLE_REPEAT_ONLY
    if(RDMA0_mf_src_w > 320)
    {
       MM_REG_WRITE(command, MDP_RDMA_RESV_DUMMY_0, 0x00000004, 0x00000004);
    }
#endif
#if defined(HDR_MT6799)
    onConfigHDRTile(command);
#endif
    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpEngine_RDMA::onAdvanceTile(DpCommand&)
{
    // FIXME: check tile order
    if (m_lastOutHorizontal != m_outHorizontal)
    {
        m_outHistXLeft      = m_outHorizontal ? (m_outTileXRight + 1) : 0;
        m_lastOutHorizontal = m_outHorizontal;
    }
    if (m_lastOutVertical != m_outVertical)
    {
        m_outHistYTop       = m_outVertical ? (m_outTileYBottom + 1) : 0;
        m_lastOutVertical   = m_outVertical;
    }

    DPLOGI("DpEngine_RDMA: m_outHistXLeft %d, m_outHistYTop %d\n",m_outHistXLeft,m_outHistYTop );

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpEngine_RDMA::onConfigHDRTile(DpCommand &command)
{
#if defined(HDR_MT6799)
    if (m_identifier != 0)
        return DP_STATUS_RETURN_SUCCESS;

    uint32_t HDR_tile_h;
    uint32_t HDR_tile_v;
    uint32_t HDR_in_hsize;
    uint32_t HDR_in_vsize;
    uint32_t HDR_out_end_x;
    uint32_t HDR_out_end_y;
    uint32_t HDR_hist_left;
    uint32_t HDR_hist_top;
    uint32_t HDR_hist_begin_x;
    uint32_t HDR_hist_begin_y;
    uint32_t HDR_hist_end_x;
    uint32_t HDR_hist_end_y;
    uint32_t HDR_first_tile;
    uint32_t HDR_last_tile;

    // Set tile position
    HDR_tile_h = m_outTileXLeft;
    HDR_tile_v = m_outTileYTop;
    MM_REG_WRITE(command, MDP_HDR_TILE_POS, HDR_tile_v << 16 |
                                            HDR_tile_h <<  0, 0x1FFF1FFF);

    HDR_out_end_x = m_outTileXRight - m_outTileXLeft;
    HDR_out_end_y = m_outTileYBottom - m_outTileYTop;

    // Set source size
    HDR_in_hsize = HDR_out_end_x + 1;
    HDR_in_vsize = HDR_out_end_y + 1;
    MM_REG_WRITE(command, MDP_HDR_SIZE_0, HDR_in_vsize << 16 |
                                          HDR_in_hsize <<  0, 0x1FFF1FFF);

    // Set crop region
    MM_REG_WRITE(command, MDP_HDR_SIZE_1, HDR_out_end_x << 16 |
                                          0             <<  0, 0x1FFF1FFF);
    MM_REG_WRITE(command, MDP_HDR_SIZE_2, HDR_out_end_y << 16 |
                                          0             <<  0, 0x1FFF1FFF);

    HDR_hist_left = (m_outTileXLeft > m_outHistXLeft) ? m_outTileXLeft : m_outHistXLeft;
    HDR_hist_top  = (m_outTileYTop  > m_outHistYTop)  ? m_outTileYTop  : m_outHistYTop;

    // Set histogram window
    HDR_hist_begin_x = HDR_hist_left - m_outTileXLeft;
    HDR_hist_begin_y = HDR_hist_top  - m_outTileYTop; // Decided by FW
    MM_REG_WRITE(command, MDP_HDR_HIST_CTRL_0, HDR_hist_begin_x, 0x00001FFF); // only config x in tile

    HDR_hist_end_x = HDR_out_end_x;
    HDR_hist_end_y = HDR_out_end_y; // Decided by FW
    MM_REG_WRITE(command, MDP_HDR_HIST_CTRL_1, HDR_hist_end_x, 0x00001FFF); //only config x in tile

    if (m_outTileXLeft == 0)
    {
        HDR_first_tile = 1;
    }
    else
    {
        HDR_first_tile = 0;
    }
    if (m_cropWidth == (m_outTileXRight + 1))
    {
        HDR_last_tile = 1;
    }
    else
    {
        HDR_last_tile = 0;
    }
    MM_REG_WRITE(command, MDP_HDR_TOP, (HDR_first_tile << 4) |
                                       (HDR_last_tile << 5), 0x00000030);
    MM_REG_WRITE(command, MDP_HDR_HIST_ADDR, (HDR_first_tile << 9), 0x00000200); //enable hist_clr_en

    DPLOGI("m_cropHeight = %u, m_outTileYBottom = %u, m_outTileYTop = %u\n", m_cropHeight, m_outTileYBottom, m_outTileYTop);
    DPLOGI("m_cropOffsetY = %u, m_inTileYBottom = %u, m_inTileYTop = %u\n", m_cropOffsetY, m_inTileYBottom, m_inTileYTop);

    DPLOGI("m_cropWidth = %u, m_outTileXRight = %u, m_inTileXRight = %u\n", m_cropWidth, m_outTileXRight, m_inTileXRight);
    DPLOGI("m_cropOffsetX = %u, m_inTileXLeft = %u, m_outTileXLeft = %u\n", m_cropOffsetX, m_inTileXLeft, m_outTileXLeft);

    DPLOGI("HDR_hist_end_x = %u, HDR_hist_begin_x = %u\n", HDR_hist_end_x, HDR_hist_begin_x);
    DPLOGI("HDR_hist_end_y = %u, HDR_hist_begin_y = %u\n", HDR_hist_end_y, HDR_hist_begin_y);
    DPLOGI("HDR_in_hsize = %u, HDR_in_vsize = %u\n", HDR_in_hsize, HDR_in_vsize);
#else
    DP_UNUSED(command);
#endif
    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpEngine_RDMA::onWaitEvent(DpCommand &command)
{
    if (0 == m_identifier)
    {
        MM_REG_WAIT(command, DpCommand::RDMA0_FRAME_DONE);
    }
    else
    {
        MM_REG_WAIT(command, DpCommand::RDMA1_FRAME_DONE);
    }

    // Disable RDMA
    MM_REG_WRITE(command, MDP_RDMA_EN, 0x00000000, 0x00000001);

    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpEngine_RDMA::onPostProc(DpCommand &command)
{
    command.addMetLog("tileOverhead", m_inTileAccumulation * 100 / m_inFrameWidth -100);

#ifdef HDR_MT6799
    uint32_t index;
    bool pq_readback;
    bool hdr_readback;
    int32_t dre_readback;
    uint64_t engineFlag;
    uint32_t VEncFlag;
    uint32_t counter = 0;
    uint32_t* readbackPABuffer = NULL;
    uint32_t readbackPABufferIndex = 0;

    command.getReadbackStatus(pq_readback, hdr_readback, dre_readback, engineFlag, VEncFlag);

    if ((((engineFlag >> tVENC) & 0x1) && VEncFlag) || !(hdr_readback))
    {
        DPLOGI("DpEngine_RDMA::onReconfigTiles : VENC and no readback do nothing\n");
        return DP_STATUS_RETURN_SUCCESS;
    }
    readbackPABuffer = command.getReadbackPABuffer(readbackPABufferIndex);

    if (readbackPABuffer == NULL)
    {
        DPLOGW("DpEngine_RDMA::onPostProc : readbackPABuffer has been destroyed readbackPABuffer = %p, readbackPABufferIndex = %d\n", readbackPABuffer, readbackPABufferIndex);
        return DP_STATUS_RETURN_SUCCESS;
    }
    MM_REG_READ_BEGIN(command);

#ifdef SUPPORT_HDR
    if (hdr_readback)
    {
        DPLOGI("DpEngine_RDMA::onPostProc:hdr_readback\n");
        for (int i = 0; i < 57; i++)
        {
            MM_REG_READ(command, MDP_HDR0_BASE + 0x88, readbackPABuffer[((readbackPABufferIndex + counter) & (MAX_NUM_READBACK_PA_BUFFER - 1))], &m_regLabel[counter]);
            counter++;
        }
        MM_REG_READ(command, MDP_HDR0_BASE + 0xA4, readbackPABuffer[((readbackPABufferIndex + counter) & (MAX_NUM_READBACK_PA_BUFFER - 1))], &m_regLabel[counter]);
        counter++;
    }
#endif //SUPPORT_HDR

    MM_REG_READ_END(command);

    //for general case,  only set RDMA0 for all project
    command.setNumReadbackPABuffer(counter, tRDMA0);

    m_prevPABufferIndex = readbackPABufferIndex;
    m_prevReadbackCount = counter;

    DPLOGI("DpEngine_RDMA::onPostProc: counter:%d\n", counter);

#endif //HDR_MT6799
    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpEngine_RDMA::onUpdatePMQOS(DpCommand &command, uint32_t& totalPixel, uint32_t& totalDataSzie, int32_t& engineType, struct timeval& endtime)
{
    DP_STATUS_ENUM  status;
    uint32_t        plane = 0;
    uint32_t        data = 0;
    uint32_t        pixelCount = 0;
    int32_t         dummy;
    int32_t         SrcCropWidth;

    plane = DP_COLOR_GET_PLANE_COUNT(m_sourceColorFormat);

    data = DP_COLOR_GET_MIN_Y_SIZE(m_sourceColorFormat, m_inTileAccumulation, m_inFrameHeight);
    data += (plane > 1) ? DP_COLOR_GET_MIN_UV_SIZE(m_sourceColorFormat, m_inTileAccumulation, m_inFrameHeight) : 0;
    data += (plane > 2) ? DP_COLOR_GET_MIN_UV_SIZE(m_sourceColorFormat, m_inTileAccumulation, m_inFrameHeight) : 0;

    if (1 == DP_COLOR_GET_UFP_ENABLE(m_sourceColorFormat))
    {
        data = (data * 7) / 10; //UFO case
        DPLOGD("DpEngine_RDMA::UFO case data = %d\n", data);
    }

    totalDataSzie += data;

    pixelCount = m_inTileAccumulation * m_inFrameHeight;

    endtime = m_pInDataPort->getEndTime();

    if (pixelCount > totalPixel)
    {
        totalPixel = pixelCount;
    }

    status = m_pOutDataPort[0]->getSourceCrop(&dummy,
                                              &dummy,
                                              &dummy,
                                              &dummy,
                                              &SrcCropWidth,
                                              &dummy,
                                              &dummy,
                                              &dummy);

    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpEngine_RDMA: getSourceCrop failed\n");
        return status;
    }

    command.addMMpathLog("tileOverhead", m_inTileAccumulation * 1000 / SrcCropWidth -1000, DP_MMPATH_TILEOVERHEAD);

    engineType = m_engineType;

    DPLOGI("DpEngine_RDMA::updatePMQOS totalDataSzie = %d totalPixel = %d\n", totalDataSzie, totalPixel);

    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpEngine_RDMA::onReconfigTiles(DpCommand &command)
{
    if (command.getSyncMode()) //old version pq readback must remove
    {
        DPLOGI("DpEngine_RDMA::onReconfigTiles : SyncMode do nothing\n");
        return DP_STATUS_RETURN_SUCCESS;
    }

#ifdef HDR_MT6799
    uint32_t index;
    bool pq_readback;
    bool hdr_readback;
    int32_t dre_readback;
    uint64_t engineFlag;
    uint32_t VEncFlag;
    uint32_t counter = 0;
    uint32_t* readbackPABuffer = NULL;
    uint32_t readbackPABufferIndex = 0;

    command.getReadbackStatus(pq_readback, hdr_readback, dre_readback, engineFlag, VEncFlag);

    if ((((engineFlag >> tVENC) & 0x1) && VEncFlag) || !(hdr_readback))
    {
        DPLOGI("DpEngine_RDMA::onReconfigTiles : VENC and no readback do nothing\n");
        return DP_STATUS_RETURN_SUCCESS;
    }

    readbackPABuffer = command.getReadbackPABuffer(readbackPABufferIndex);
    command.setNumReadbackPABuffer(m_prevReadbackCount, tRDMA0);
    if (readbackPABufferIndex == m_prevPABufferIndex)
    {
        DPLOGI("DpEngine_RDMA::onReconfigTiles : PABufferIndex no change do nothing\n");
        return DP_STATUS_RETURN_SUCCESS;
    }

    if (readbackPABuffer == NULL)
    {
        DPLOGW("DpEngine_RDMA::onReconfigTiles : readbackPABuffer has been destroyed readbackPABuffer = %p, readbackPABufferIndex = %d\n", readbackPABuffer, readbackPABufferIndex);
        return DP_STATUS_RETURN_SUCCESS;
    }

#ifdef SUPPORT_HDR
    if (hdr_readback)
    {
        DPLOGI("DpEngine_RDMA::onReconfigTiles:hdr_readback\n");
        for (int i = 0; i < 57; i++)
        {
            MM_REG_READ(command, MDP_HDR0_BASE + 0x88, readbackPABuffer[((readbackPABufferIndex + counter) & (MAX_NUM_READBACK_PA_BUFFER - 1))], NULL, m_regLabel[counter]);
            counter++;
        }
        MM_REG_READ(command, MDP_HDR0_BASE + 0xA4, readbackPABuffer[((readbackPABufferIndex + counter) & (MAX_NUM_READBACK_PA_BUFFER - 1))], NULL, m_regLabel[counter]);
        counter++;

    }
#endif // SUPPORT_HDR

    //for general case,  only set RDMA0 for all project
    m_prevPABufferIndex = readbackPABufferIndex;
#endif //HDR_MT6799
    return DP_STATUS_RETURN_SUCCESS;
}
