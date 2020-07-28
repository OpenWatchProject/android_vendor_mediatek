#include "DpEngineBase.h"
#include "DpEngineType.h"
#include "DpTileScaler.h"

#if CONFIG_FOR_OS_WINDOWS

extern EngineReg WPEIReg;
extern EngineReg WPEOReg;
extern EngineReg WPEI2Reg;
extern EngineReg WPEO2Reg;
extern EngineReg IMGIReg;
extern EngineReg IMG2OReg;
extern EngineReg CAMINReg;
extern EngineReg CAMIN2Reg;
extern EngineReg CAMIN3eg;
extern EngineReg CAMIN4Reg;
extern EngineReg RDMA0Reg;
extern EngineReg RDMA1Reg;
extern EngineReg RDMA2Reg;
extern EngineReg RDMA3Reg;
extern EngineReg AAL0Reg;
extern EngineReg AAL1Reg;
extern EngineReg AAL2Reg;
extern EngineReg AAL3Reg;
extern EngineReg CCORR0Reg;
extern EngineReg FG0Reg;
extern EngineReg FG1Reg;
extern EngineReg HDR0Reg;
extern EngineReg HDR1Reg;
extern EngineReg SCL0Reg;
extern EngineReg SCL1Reg;
extern EngineReg SCL2Reg;
extern EngineReg SCL3Reg;
extern EngineReg TDSHP0Reg;
extern EngineReg TDSHP1Reg;
extern EngineReg TDSHP2Reg;
extern EngineReg TDSHP3Reg;
extern EngineReg COLOR0Reg;
extern EngineReg COLOR1Reg;
extern EngineReg WROT0Reg;
extern EngineReg WROT1Reg;
extern EngineReg WROT2Reg;
extern EngineReg WROT3Reg;
extern EngineReg TCC0Reg;
extern EngineReg TCC1Reg;
extern EngineReg TCC2Reg;
extern EngineReg TCC3Reg;
extern EngineReg WDMAReg;
extern EngineReg VENCReg;
extern EngineReg PATH0_SOUTReg;
extern EngineReg PATH1_SOUTReg;
extern EngineReg COLOREXReg;
extern EngineReg OVL0EXReg;
extern EngineReg WDMAEXReg;

typedef struct EngineFactory {
    DpEngineType    type;
    EngineReg       &factory;
} EngineFactory;

static const EngineFactory factory[] =
{
    { tWPEI,        WPEIReg  },
    { tWPEO,        WPEOReg },
    { tWPEI2,       WPEI2Reg  },
    { tWPEO2,       WPEO2Reg },
    { tIMGI,        IMGIReg  },
    { tIMG2O,       IMG2OReg },
    { tCAMIN,       CAMINReg },
    { tCAMIN2,      CAMIN2Reg },
    { tRDMA0,       RDMA0Reg },
    { tRDMA1,       RDMA1Reg },
    { tAAL0,        AAL0Reg },
    { tHDR0,        HDR0Reg },
    { tCCORR0,      CCORR0Reg },
    { tSCL0,        SCL0Reg  },
    { tSCL1,        SCL1Reg  },
    { tSCL2,        SCL2Reg  },
    { tTDSHP0,      TDSHP0Reg },
#ifdef MDP_VERSION_8173
    { tTDSHP1,      TDSHP1Reg },
#endif
    { tCOLOR0,      COLOR0Reg },
    { tWROT0,       WROT0Reg },
    { tWROT1,       WROT1Reg },
    { tWDMA,        WDMAReg },
    { tVENC,        VENCReg },
    { tPATH0_SOUT,  PATH0_SOUTReg },
    { tPATH1_SOUT,  PATH1_SOUTReg },
#ifdef tCAMIN3
    { tCAMIN3,      CAMIN3Reg },
#endif
#ifdef tCAMIN4
    { tCAMIN4,      CAMIN4Reg },
#endif
#ifdef tRDMA2
    { tRDMA2,       RDMA2Reg },
#endif
#ifdef tRDMA3
    { tRDMA3,       RDMA3Reg },
#endif
#ifdef tFG0
    { tFG0,         FG0Reg },
#endif
#ifdef tFG1
    { tFG1,         FG1Reg },
#endif
#ifdef tAAL1
    { tAAL1,        AAL1Reg },
#endif
#ifdef tAAL2
    { tAAL2,        AAL2Reg },
#endif
#ifdef tAAL3
    { tAAL3,        AAL3Reg },
#endif
#ifdef tHDR1
    { tHDR1,        HDR1Reg },
#endif
#ifdef tSCL3
    { tSCL3,        SCL3Reg  },
#endif
#ifdef tTDSHP1
    { tTDSHP1,      TDSHP1Reg },
#endif
#ifdef tTDSHP2
    { tTDSHP2,      TDSHP2Reg },
#endif
#ifdef tTDSHP3
    { tTDSHP3,      TDSHP3Reg },
#endif
#ifdef tCOLOR1
    { tCOLOR1,      COLOR1Reg },
#endif
#ifdef tWROT2
    { tWROT2,       WROT2Reg },
#endif
#ifdef tWROT3
    { tWROT3,       WROT3Reg },
#endif
#ifdef tTCC0
    { tTCC0,        TCC0Reg  },
#endif
#ifdef tTCC1
    { tTCC1,        TCC1Reg  },
#endif
#ifdef tTCC2
    { tTCC2,        TCC2Reg  },
#endif
#ifdef tTCC3
    { tTCC3,        TCC3Reg  },
#endif
#ifdef CONFIG_FOR_SOURCE_PQ
    { tCOLOR_EX,    COLOREXReg },
    { tOVL0_EX,     OVL0EXReg },
    { tWDMA_EX,     WDMAEXReg },
#endif
};

#define DP_ENGINE_MAX_FACTORY_COUNT (sizeof(factory) / sizeof(EngineFactory))

#endif // CONFIG_FOR_OS_WINDOWS

template EngineReg* EngineReg::gHead;

DpEngineBase* DpEngineBase::Factory(DpEngineType type)
{
    DpEngineBase *pEngine;
    const EngineReg *pCurr = EngineReg::Head();

    while(pCurr)
    {
        pEngine = pCurr->factory()(type);
        if(pEngine)
        {
            pEngine->setEngineType(type);
            return pEngine;
        }
        pCurr = pCurr->next();
    }
    return NULL;
}


const char* DpEngineBase::onGetEngineName()
{
    const char *pName;

    DP_GET_ENGINE_NAME(m_engineType, pName);

    return pName;
}


DP_STATUS_ENUM DpEngineBase::bindOutPort(DpPortAdapt *port)
{
    int64_t feature;
    feature = queryFeature();

    if (feature & (eSCL | eTDSHP))
    {
        if (m_outPortCount > 0)
        {
            DP_STATUS_ENUM  status;
            DpColorFormat   format0;
            int32_t         width0;
            int32_t         height0;
            int32_t         YPitch0;
            int32_t         UVPitch0;
            DP_PROFILE_ENUM profile0;
            DpColorFormat   format1;
            int32_t         width1;
            int32_t         height1;
            int32_t         YPitch1;
            int32_t         UVPitch1;
            DP_PROFILE_ENUM profile1;

            status = m_pOutDataPort[0]->getPortInfo(&format0,
                                                    &width0,
                                                    &height0,
                                                    &YPitch0,
                                                    &UVPitch0,
                                                    &profile0);
            if (DP_STATUS_RETURN_SUCCESS != status)
            {
                assert(0);
                return status;
            }

            status = port->getPortInfo(&format1,
                                       &width1,
                                       &height1,
                                       &YPitch1,
                                       &UVPitch1,
                                       &profile1);
            if (DP_STATUS_RETURN_SUCCESS != status)
            {
                assert(0);
                return status;
            }

            if ((width0  != width1) ||
                (height0 != height1) ||
                (YPitch0 != YPitch1))
            {
                assert(0);
                return DP_STATUS_INVALID_PARAX;
            }
        }
    }

    if (feature & (eWMEM | eJPEG | eVENC))
    {
        if (m_outPortCount > 0)
        {
            assert(0);
            return DP_STATUS_INVALID_PARAX;
        }
    }

    m_pOutDataPort[m_outPortCount] = port;
    m_outPortCount++;

    return onBindOutPort(port);
}


DP_STATUS_ENUM DpEngineBase::onPollPort(bool waitPort)
{
    DP_STATUS_ENUM status;
    int64_t feature;

    feature = queryFeature();

    if (feature & eRMEM)
    {
        status = m_pInDataPort->pollStatus(CLIENT_CONSUMER, waitPort);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            return status;
        }
    }

    if (feature & eWMEM)
    {
        status = m_pOutDataPort[0]->pollStatus(CLIENT_PRODUCER, waitPort);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            return status;
        }
    }

    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpEngineBase::onWaitFence()
{
    DP_STATUS_ENUM status;
    int64_t feature;

    feature = queryFeature();

    if (feature & eRMEM)
    {
        status = m_pInDataPort->waitFence(CLIENT_CONSUMER);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            return status;
        }
    }

    if (feature & eWMEM)
    {
        status = m_pOutDataPort[0]->waitFence(CLIENT_PRODUCER);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            return status;
        }
    }

    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpEngineBase::onAbortPollPort()
{

    DP_STATUS_ENUM status;
    int64_t feature;

    feature = queryFeature();

    if (feature & eRMEM)
    {
        status = m_pInDataPort->abortPoll(CLIENT_CONSUMER);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            return status;
        }
    }

    if (feature & eWMEM)
    {
        status = m_pOutDataPort[0]->abortPoll(CLIENT_PRODUCER);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            return status;
        }
    }

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpEngineBase::onFlushBuffer(FLUSH_TYPE_ENUM type)
{
    DP_STATUS_ENUM status;
    int64_t feature;

    feature = queryFeature();

    if (FLUSH_BEFORE_HW_READ == type)
    {
        if (feature & eRMEM)
        {
            if (true == m_pInDataPort->getFlushStatus())
            {
                status = m_pInDataPort->flushBuffer(CLIENT_CONSUMER);
                if (DP_STATUS_RETURN_SUCCESS != status)
                {
                    return status;
                }
            }
        }
        // CPU cache may be dirty on output buffer due to dynamic alloc/free
        if (feature & eWMEM)
        {
            if (true == m_pOutDataPort[0]->getFlushStatus())
            {
                status = m_pOutDataPort[0]->flushBuffer(CLIENT_PRODUCER);
                if (DP_STATUS_RETURN_SUCCESS != status)
                {
                    return status;
                }
            }
        }
    }
    else if (FLUSH_AFTER_HW_WRITE == type)
    {
        if (feature & eWMEM)
        {
            if (true == m_pOutDataPort[0]->getFlushStatus())
            {
                status = m_pOutDataPort[0]->flushBuffer(CLIENT_PRODUCER);
                if (DP_STATUS_RETURN_SUCCESS != status)
                {
                    return status;
                }
            }
        }
    }

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpEngineBase::onFlipBuffer()
{
    DP_STATUS_ENUM status;
    int64_t feature;

    feature = queryFeature();

    if (feature & eRMEM)
    {
        status = m_pInDataPort->flipBuffer(CLIENT_CONSUMER);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            return status;
        }
    }

    if (feature & eWMEM)
    {
        status = m_pOutDataPort[0]->flipBuffer(CLIENT_PRODUCER);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            return status;
        }
    }

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpEngineBase::onDumpBuffer()
{
    DP_STATUS_ENUM status;
    int64_t feature;

    feature = queryFeature();

    if (feature & eRMEM)
    {
        status = m_pInDataPort->dumpBuffer(CLIENT_CONSUMER);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            return status;
        }
    }

    if (feature & eWMEM)
    {
        status = m_pOutDataPort[0]->dumpBuffer(CLIENT_PRODUCER);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            return status;
        }
    }

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpEngineBase::onDumpDebugInfo()
{
    int64_t feature;
    feature = queryFeature();

    DPLOGD("DpEngineBase: engine feature set 0x%08llx\n", feature);

    if (feature & eRMEM)
    {
        if (m_pInDataPort != NULL)
            m_pInDataPort->dumpDebugInfo(CLIENT_CONSUMER);
    }

    if (feature & eWMEM)
    {
        if (m_pOutDataPort[0])
            m_pOutDataPort[0]->dumpDebugInfo(CLIENT_PRODUCER);
    }

    return DP_STATUS_RETURN_SUCCESS;
}

#if !CONFIG_FOR_VERIFY_FPGA

inline DP_STATUS_ENUM DpEngineBase::configFrameISP(DpConfig &config)
{
    DP_STATUS_ENUM status;

    status = m_pInDataPort->getPortInfo(&config.inFormat,
                                        &config.inWidth,
                                        &config.inHeight,
                                        &config.inYPitch,
                                        &config.inUVPitch,
                                        &config.inProfile);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpEngineBase: Get port info failed\n");
        return status;
    }

    config.enISP = true;
    return DP_STATUS_RETURN_SUCCESS;
}

inline DP_STATUS_ENUM DpEngineBase::configFrameRotate(DpConfig &config)
{
    config.rotation = m_pOutDataPort[0]->getRotation();
    return DP_STATUS_RETURN_SUCCESS;
}

inline DP_STATUS_ENUM DpEngineBase::configFrameFlip(DpConfig &config)
{
    config.enFlip = m_pOutDataPort[0]->getFlipStatus();
    return DP_STATUS_RETURN_SUCCESS;
}

inline DP_STATUS_ENUM DpEngineBase::configFrameAlphaRot(DpConfig &config)
{
    DP_STATUS_ENUM status;
    int32_t dummyi;
    DP_PROFILE_ENUM dummy;

    DpColorFormat  sourceFormat;
    int32_t        sourceWidth;
    int32_t        sourceHeight;
    int32_t        sourceCropXOffset;
    int32_t        sourceCropYOffset;
    int32_t        sourceCropWidth;
    int32_t        sourceCropWidthSubpixel;
    int32_t        sourceCropHeight;
    int32_t        sourceCropHeightSubpixel;
    int32_t        sourceCropSubpixX;
    int32_t        sourceCropSubpixY;
    DpColorFormat  targetFormat;
    int32_t        targetWidth;
    int32_t        targetHeight;
    int32_t        rotation;
    uint32_t       sharpness;

    bool           needResizer;

    status = m_pInDataPort->getPortInfo(&sourceFormat,
                                        &sourceWidth,
                                        &sourceHeight,
                                        &dummyi,
                                        &dummyi,
                                        &dummy);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpEngineBase: Get port info failed\n");
        return status;
    }

    status = m_pOutDataPort[0]->getSourceCrop(&sourceCropXOffset,
                                              &sourceCropSubpixX,
                                              &sourceCropYOffset,
                                              &sourceCropSubpixY,
                                              &sourceCropWidth,
                                              &sourceCropWidthSubpixel,
                                              &sourceCropHeight,
                                              &sourceCropHeightSubpixel);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpEngineBase: Get source crop failed\n");
        return status;
    }

    status = m_pOutDataPort[0]->getPortInfo(&targetFormat,
                                            &targetWidth,
                                            &targetHeight,
                                            &dummyi,
                                            &dummyi,
                                            &dummy);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpEngineBase: Get port info failed\n");
        return status;
    }

    sharpness = m_pOutDataPort[0]->getTDSHPStatus();

    rotation = m_pOutDataPort[0]->getRotation();

    if (rotation == 90 || rotation == 270)
    {
        int32_t temp = targetWidth;
        targetWidth = targetHeight;
        targetHeight = temp;
    }

    DPLOGI("DpEngineBase Width  %d %d %d\n", sourceCropWidth, sourceWidth, targetWidth);
    DPLOGI("DpEngineBase Height %d %d %d\n", sourceCropHeight, sourceHeight, targetHeight);
    DPLOGI("DpEngineBase CropSubpixX %d %d\n", sourceCropSubpixX, sourceCropSubpixY);

    if ((config.rootAndLeaf == 2) &&
        (sourceCropWidth == targetWidth) &&
        (sourceCropHeight == targetHeight) &&
        (sourceCropSubpixX == 0) &&
        (sourceCropSubpixY == 0))
    {
        needResizer = false;
    }
    else
    {
        needResizer = true;
    }

    DPLOGI("DpEngineBase needResizer %d\n", needResizer);

    //ARGB8888 in/out and bypass resizer and disable sharpness
    if ( (DP_COLOR_GET_HW_FORMAT(sourceFormat) == 2 || DP_COLOR_GET_HW_FORMAT(sourceFormat) == 3) &&
         (DP_COLOR_GET_HW_FORMAT(targetFormat) == 2 || DP_COLOR_GET_HW_FORMAT(targetFormat) == 3) &&
         needResizer == false &&
         (sourceCropXOffset == 0 && sourceCropYOffset == 0) &&
         sharpness == 0)
    {
        config.enAlphaRot = true;
    }

    return DP_STATUS_RETURN_SUCCESS;
}

inline DP_STATUS_ENUM DpEngineBase::configFrameRead(DpCommand&, DpConfig &config)
{
    DP_STATUS_ENUM status;
    int32_t dummyi;
    DP_PROFILE_ENUM dummy;

    DPLOGI("DpEngineBase: PROC_RMEM start\n");

    status = m_pInDataPort->getHWAddress(CLIENT_CONSUMER,
                                         m_engineType,
                                         config.memAddr,
                                         config.memSize);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpEngineBase: get input memory hardware base failed\n");
        return status;
    }

    DPLOGI("DpEngineBase: memory base0 0x%08x\n", config.memAddr[0]);
    DPLOGI("DpEngineBase: memory base1 0x%08x\n", config.memAddr[1]);
    DPLOGI("DpEngineBase: memory base2 0x%08x\n", config.memAddr[2]);

    status = m_pInDataPort->getPortInfo(&config.inFormat,
                                        &config.inWidth,
                                        &config.inHeight,
                                        &config.inYPitch,
                                        &config.inUVPitch,
                                        &config.inProfile);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpEngineBase: Get port info failed\n");
        return status;
    }

    // block format error check
    if (DP_COLOR_GET_BLOCK_MODE(config.inFormat))
    {
        int32_t plane = DP_COLOR_GET_PLANE_COUNT(config.inFormat);

        if ((config.inWidth  & 0x0f) ||
            (config.inHeight & 0x1f))
        {
            DPLOGE("invalid block format setting, width %d, height %d\n", config.inWidth, config.inHeight);
            DPLOGE("width should be 16 align, height should be 32 align\n");
            return DP_STATUS_INVALID_PARAX;
        }

        // 16-byte error
        // secure handle will not check
        if ((m_pInDataPort->getSecureStatus() == DP_SECURE_NONE) &&
            ((0 != (config.memAddr[0] & 0x0f)) ||
            (plane > 1 && 0 != (config.memAddr[1] & 0x0f)) ||
            (plane > 2 && 0 != (config.memAddr[2] & 0x0f))))
        {
            DPLOGE("invalid block format setting, buffer %u %u %u\n", config.memAddr[0], config.memAddr[1], config.memAddr[2]);
            DPLOGE("buffer should be 16 align\n");
            return DP_STATUS_INVALID_BUFFER;
        }

        // 128-byte warning for performance
        if ((m_pInDataPort->getSecureStatus() == DP_SECURE_NONE) &&
            ((0 != (config.memAddr[0] & 0x7f)) ||
            (plane > 1 && 0 != (config.memAddr[1] & 0x7f)) ||
            (plane > 2 && 0 != (config.memAddr[2] & 0x7f))))
        {
            DPLOGW("warning: block format setting, buffer %u %u %u\n", config.memAddr[0], config.memAddr[1], config.memAddr[2]);
        }
    }

    if ((DP_COLOR_GET_PLANE_COUNT(config.inFormat) > 1) && (config.inUVPitch <= 0))
    {
        config.inUVPitch = DP_COLOR_GET_MIN_UV_PITCH(config.inFormat, config.inWidth);
    }

    if (((1 == DP_COLOR_GET_PLANE_COUNT(config.inFormat)) &&
         (0 == config.memAddr[0])) ||
        ((2 == DP_COLOR_GET_PLANE_COUNT(config.inFormat)) &&
         ((0 == config.memAddr[1]) || (0 == config.memAddr[0]))) ||
        ((3 == DP_COLOR_GET_PLANE_COUNT(config.inFormat)) &&
         ((0 == config.memAddr[2]) || (0 == config.memAddr[1]) || (0 == config.memAddr[0]))))
    {
        DPLOGE("DpEngineBase: buffer plane number error, color format = %d, plane = %d\n",
            config.inFormat, DP_COLOR_GET_PLANE_COUNT(config.inFormat));
        assert(0);
        return DP_STATUS_INVALID_BUFFER;
    }

    if (0 != DP_COLOR_GET_H_SUBSAMPLE(config.inFormat))
    {
        config.inWidth &= ~1;
    }

    if (0 != DP_COLOR_GET_V_SUBSAMPLE(config.inFormat))
    {
        config.inHeight &= ~1;
    }

    if (0 != DP_COLOR_GET_UFP_ENABLE(config.inFormat))
    {
        uint32_t u4PIC_SIZE_Y     = config.inWidth * config.inHeight;
        uint32_t u4UFO_LEN_SIZE_Y = ((((u4PIC_SIZE_Y + 255) >> 8) + 63 + (16*8)) >> 6) << 6;
        //uint32_t u4UFO_LEN_SIZE_C = (((u4UFO_LEN_SIZE_Y >> 1) + 15 + (16*8)) >> 4) << 4;
        uint32_t u4PIC_SIZE_Y_BS;
        uint32_t u4PIC_SIZE_C_BS;
        uint32_t u4PIC_SIZE_BS;
        if (0 != DP_COLOR_GET_10BIT_PACKED(config.inFormat)) // 10bit
        {
            if (0 != DP_COLOR_GET_10BIT_JUMP_MODE(config.inFormat))
            {
                u4PIC_SIZE_Y_BS  = (((u4PIC_SIZE_Y * 5 >> 2) + 511) >> 9) << 9;
                u4PIC_SIZE_BS    = ((u4PIC_SIZE_Y_BS + u4UFO_LEN_SIZE_Y + 4095) >> 12) << 12;
                u4PIC_SIZE_C_BS  = (((u4PIC_SIZE_Y * 5 >> 3) + 63) >> 6) << 6;
            }
            else
            {
                u4PIC_SIZE_Y_BS  = (((u4PIC_SIZE_Y * 5 >> 2) + 4095) >> 12) << 12;
                u4PIC_SIZE_C_BS  = u4PIC_SIZE_Y * 5 >> 3;
                u4PIC_SIZE_BS    = ((u4PIC_SIZE_Y_BS + u4PIC_SIZE_C_BS + 511) >> 9) << 9;
            }
        }
        else // 8bit
        {
            if (0 != DP_COLOR_GET_AUO_MODE(config.inFormat))
            {
                u4UFO_LEN_SIZE_Y = u4UFO_LEN_SIZE_Y << 1;
                //u4UFO_LEN_SIZE_C = u4UFO_LEN_SIZE_C << 1;
                u4PIC_SIZE_Y_BS  = ((u4PIC_SIZE_Y + 511) >> 9) << 9;
                u4PIC_SIZE_BS    = ((u4PIC_SIZE_Y_BS + u4UFO_LEN_SIZE_Y + 4095) >> 12) << 12;
                u4PIC_SIZE_C_BS  = (((u4PIC_SIZE_Y >> 1) + 63) >> 6) << 6;
            }
            else
            {
                u4PIC_SIZE_Y_BS  = ((u4PIC_SIZE_Y + 4095) >> 12) << 12;
                u4PIC_SIZE_C_BS  = u4PIC_SIZE_Y >> 1;
                u4PIC_SIZE_BS    = ((u4PIC_SIZE_Y_BS + u4PIC_SIZE_C_BS + 511) >> 9) << 9;
            }
        }

        config.enUFODec = true;
        DPLOGI("DpEngineBase: RDMA UFO is enabled\n");
        if (0 != DP_COLOR_GET_10BIT_JUMP_MODE(config.inFormat) ||
            0 != DP_COLOR_GET_AUO_MODE(config.inFormat))
        {
            // Y YL C CL
            config.memUFOLenAddr[0] = config.memAddr[0] + u4PIC_SIZE_Y_BS;
            config.memAddr[1] = config.memAddr[0] + u4PIC_SIZE_BS;
            config.memUFOLenAddr[1] = config.memAddr[1] + u4PIC_SIZE_C_BS;
        }
        else
        {
            // Y C YL CL
            config.memAddr[1] = config.memAddr[0] + u4PIC_SIZE_Y_BS;
            config.memUFOLenAddr[0] = config.memAddr[0] + u4PIC_SIZE_BS;
            config.memUFOLenAddr[1] = config.memUFOLenAddr[0] + u4UFO_LEN_SIZE_Y;
        }
        DPLOGI("DpEngineBase: memAddr base1 0x%08x\n", config.memAddr[1]);
        DPLOGI("DpEngineBase: memUFOLenAddr base0 0x%08x\n", config.memUFOLenAddr[0]);
        DPLOGI("DpEngineBase: memUFOLenAddr base1 0x%08x\n", config.memUFOLenAddr[1]);
    }

    if (config.rootAndLeaf == 2 // 1in/1out
        && ((STREAM_BITBLT == config.scenario)
    #ifdef CONFIG_FOR_SOURCE_PQ
        || (STREAM_COLOR_BITBLT == config.scenario)
    #endif
        || (STREAM_GPU_BITBLT == config.scenario)
        || (STREAM_DUAL_BITBLT == config.scenario)
        || (STREAM_2ND_BITBLT == config.scenario)))
    {
        status = m_pOutDataPort[0]->getSourceCrop(&config.inXOffset,
                                                  &config.inXSubpixel,
                                                  &config.inYOffset,
                                                  &config.inYSubpixel,
                                                  &config.inCropWidth,
                                                  &config.inCropWidthSubpixel,
                                                  &config.inCropHeight,
                                                  &config.inCropHeightSubpixel);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            return status;
        }

        if ((config.inCropWidth != config.inWidth) ||
            (config.inCropHeight != config.inHeight))
        {
            config.enRDMACrop = true;//((feature & eISP) == 0);
        }
    }
    else
    {
        config.inXOffset = 0;
        config.inYOffset = 0;
        config.inCropWidth = config.inWidth;
        config.inCropHeight = config.inHeight;
    }

    if ((90  == config.rotation) ||
        (270 == config.rotation))   // RROT
    {
        int32_t length      = config.inWidth;
        int32_t offset      = config.inXOffset;
        int32_t crop        = config.inCropWidth;

        config.inWidth      = config.inHeight;
        config.inXOffset    = config.inYOffset;
        config.inCropWidth  = config.inCropHeight;

        config.inHeight     = length;
        config.inYOffset    = offset;
        config.inCropHeight = crop;
    }
    // for RDMA filter
    status = m_pOutDataPort[0]->getPortInfo(&config.outFormat,
                                            &dummyi,
                                            &dummyi,
                                            &dummyi,
                                            &dummyi,
                                            &dummy);

    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpEngineBase: Get OutDataPort info failed\n");
        return status;
    }

    //config.outFormat  = config.inFormat;
    config.outWidth   = config.inWidth;
    config.outHeight  = config.inHeight;
    config.outYPitch  = config.inYPitch;
    config.outUVPitch = config.inUVPitch;

    if (config.inCropWidth + config.inXOffset > config.inWidth)
    {
        config.inCropWidth = config.inWidth - config.inXOffset;
    }
    if (config.inCropHeight + config.inYOffset > config.inHeight)
    {
        config.inCropHeight = config.inHeight - config.inYOffset;
    }

    if (config.enRDMACrop)
    {
        config.outWidth  = config.inCropWidth + config.inXOffset;
        config.outHeight = config.inCropHeight + config.inYOffset;
    }

#if defined(HDR_MT6799)
    config.pqSessionId = m_pOutDataPort[0]->getPqIdStatus();
    DPLOGI("DpEngineBase: configFrameRead m_pOutDataPort[0]->getPqIdStatus() = %llx\n",m_pOutDataPort[0]->getPqIdStatus());
#endif

    DPLOGI("DpEngineBase: PROC_RMEM done\n");
    return DP_STATUS_RETURN_SUCCESS;
}

inline DP_STATUS_ENUM DpEngineBase::configFrameRingBuf(DpConfig &config)
{
    DP_STATUS_ENUM status;
    BUFFER_TYPE_ENUM bufType;

    status = m_pInDataPort->getBufferType(&bufType);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpEngineBase: get buffer type failed %d\n", status);
        return status;
    }

    config.enRing = bool(BUFFER_RING == bufType);
    return DP_STATUS_RETURN_SUCCESS;
}

inline DP_STATUS_ENUM DpEngineBase::configFrameAal(DpConfig &config)
{
    DP_STATUS_ENUM status;

    int32_t dummyi;
    DP_PROFILE_ENUM dummy;

    DPLOGI("DpEngineBase: PROC_AAL start\n");

    status = m_pInDataPort->getPortInfo(&config.inFormat,
                                        &config.inWidth,
                                        &config.inHeight,
                                        &dummyi,
                                        &dummyi,
                                        &dummy);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpEngineBase: Get port info failed\n");
        return status;
    }

    if (0 != DP_COLOR_GET_H_SUBSAMPLE(config.inFormat))
    {
        config.inWidth &= ~1;
    }

    if (0 != DP_COLOR_GET_V_SUBSAMPLE(config.inFormat))
    {
        config.inHeight &= ~1;
    }

    if (config.enRDMACrop)
    {
        status = m_pOutDataPort[0]->getSourceCrop(&config.inXOffset,
                                                  &config.inXSubpixel,
                                                  &config.inYOffset,
                                                  &config.inYSubpixel,
                                                  &config.inCropWidth,
                                                  &config.inCropWidthSubpixel,
                                                  &config.inCropHeight,
                                                  &config.inCropHeightSubpixel);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            return status;
        }

        if (config.inCropWidth + config.inXOffset > config.inWidth)
        {
            config.inCropWidth = config.inWidth - config.inXOffset;
        }
        if (config.inCropHeight + config.inYOffset > config.inHeight)
        {
            config.inCropHeight = config.inHeight - config.inYOffset;
        }

        config.inWidth  = config.inCropWidth + config.inXOffset;
        config.inHeight = config.inCropHeight + config.inYOffset;
    }
    else
    {
        config.inXOffset = 0;
        config.inYOffset = 0;
        config.inCropWidth = config.inWidth;
        config.inCropHeight = config.inHeight;
    }

    if (config.inFormat == DP_COLOR_420_BLKI)
    {
        config.inYOffset = config.inYOffset >> 1;
        config.inCropHeight = config.inCropHeight >> 1;
        config.inHeight = config.inHeight >> 1;
    }

    if ((90  == config.rotation) ||
        (270 == config.rotation))   // RROT
    {
        int32_t length      = config.inWidth;
        int32_t offset      = config.inXOffset;
        int32_t crop        = config.inCropWidth;

        config.inWidth      = config.inHeight;
        config.inXOffset    = config.inYOffset;
        config.inCropWidth  = config.inCropHeight;

        config.inHeight     = length;
        config.inYOffset    = offset;
        config.inCropHeight = crop;
    }

    config.outWidth   = config.inWidth;
    config.outHeight  = config.inHeight;

    config.pqSessionId = m_pOutDataPort[0]->getPqIdStatus();
    DPLOGI("DpEngineBase: PROC_AAL, getPqIdStatus() = %llx\n", m_pOutDataPort[0]->getPqIdStatus());

    DPLOGI("DpEngineBase: PROC_AAL done\n");
    return DP_STATUS_RETURN_SUCCESS;
}

inline DP_STATUS_ENUM DpEngineBase::configFrameCcorr(DpConfig &config)
{
    DP_STATUS_ENUM status;

    int32_t dummyi;
    DP_PROFILE_ENUM dummy;

    DPLOGI("DpEngineBase: PROC_CCORR start\n");

    status = m_pInDataPort->getPortInfo(&config.inFormat,
                                        &config.inWidth,
                                        &config.inHeight,
                                        &dummyi,
                                        &dummyi,
                                        &dummy);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpEngineBase: Get port info failed\n");
        return status;
    }

    if (0 != DP_COLOR_GET_H_SUBSAMPLE(config.inFormat))
    {
        config.inWidth &= ~1;
    }

    if (0 != DP_COLOR_GET_V_SUBSAMPLE(config.inFormat))
    {
        config.inHeight &= ~1;
    }

    if (config.enRDMACrop)
    {
        status = m_pOutDataPort[0]->getSourceCrop(&config.inXOffset,
                                                  &config.inXSubpixel,
                                                  &config.inYOffset,
                                                  &config.inYSubpixel,
                                                  &config.inCropWidth,
                                                  &config.inCropWidthSubpixel,
                                                  &config.inCropHeight,
                                                  &config.inCropHeightSubpixel);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            return status;
        }

        if (config.inCropWidth + config.inXOffset > config.inWidth)
        {
            config.inCropWidth = config.inWidth - config.inXOffset;
        }
        if (config.inCropHeight + config.inYOffset > config.inHeight)
        {
            config.inCropHeight = config.inHeight - config.inYOffset;
        }

        config.inWidth  = config.inCropWidth + config.inXOffset;
        config.inHeight = config.inCropHeight + config.inYOffset;
    }
    else
    {
        config.inXOffset = 0;
        config.inYOffset = 0;
        config.inCropWidth = config.inWidth;
        config.inCropHeight = config.inHeight;
    }

    if (config.inFormat == DP_COLOR_420_BLKI)
    {
        config.inYOffset = config.inYOffset >> 1;
        config.inCropHeight = config.inCropHeight >> 1;
        config.inHeight = config.inHeight >> 1;
    }

    if ((90  == config.rotation) ||
        (270 == config.rotation))   // RROT
    {
        int32_t length      = config.inWidth;
        int32_t offset      = config.inXOffset;
        int32_t crop        = config.inCropWidth;

        config.inWidth      = config.inHeight;
        config.inXOffset    = config.inYOffset;
        config.inCropWidth  = config.inCropHeight;

        config.inHeight     = length;
        config.inYOffset    = offset;
        config.inCropHeight = crop;
    }

    config.outWidth   = config.inWidth;
    config.outHeight  = config.inHeight;

    DPLOGI("DpEngineBase: PROC_CCORR done\n");

    return DP_STATUS_RETURN_SUCCESS;
}

inline DP_STATUS_ENUM DpEngineBase::configFrameHdr(DpConfig &config)
{
    DP_STATUS_ENUM status;

    int32_t dummyi;
    DP_PROFILE_ENUM dummy;

    DPLOGI("DpEngineBase: PROC_HDR start\n");

    status = m_pInDataPort->getPortInfo(&config.inFormat,
                                        &config.inWidth,
                                        &config.inHeight,
                                        &dummyi,
                                        &dummyi,
                                        &dummy);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpEngineBase: Get port info failed\n");
        return status;
    }

    if (0 != DP_COLOR_GET_H_SUBSAMPLE(config.inFormat))
    {
        config.inWidth &= ~1;
    }

    if (0 != DP_COLOR_GET_V_SUBSAMPLE(config.inFormat))
    {
        config.inHeight &= ~1;
    }

    if (config.enRDMACrop)
    {
        status = m_pOutDataPort[0]->getSourceCrop(&config.inXOffset,
                                                  &config.inXSubpixel,
                                                  &config.inYOffset,
                                                  &config.inYSubpixel,
                                                  &config.inCropWidth,
                                                  &config.inCropWidthSubpixel,
                                                  &config.inCropHeight,
                                                  &config.inCropHeightSubpixel);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            return status;
        }

        if (config.inCropWidth + config.inXOffset > config.inWidth)
        {
            config.inCropWidth = config.inWidth - config.inXOffset;
        }
        if (config.inCropHeight + config.inYOffset > config.inHeight)
        {
            config.inCropHeight = config.inHeight - config.inYOffset;
        }

        config.inWidth  = config.inCropWidth + config.inXOffset;
        config.inHeight = config.inCropHeight + config.inYOffset;
    }
    else
    {
        config.inXOffset = 0;
        config.inYOffset = 0;
        config.inCropWidth = config.inWidth;
        config.inCropHeight = config.inHeight;
    }

    if (config.inFormat == DP_COLOR_420_BLKI)
    {
        config.inYOffset = config.inYOffset >> 1;
        config.inCropHeight = config.inCropHeight >> 1;
        config.inHeight = config.inHeight >> 1;
    }

    if ((90  == config.rotation) ||
        (270 == config.rotation))   // RROT
    {
        int32_t length      = config.inWidth;
        int32_t offset      = config.inXOffset;
        int32_t crop        = config.inCropWidth;

        config.inWidth      = config.inHeight;
        config.inXOffset    = config.inYOffset;
        config.inCropWidth  = config.inCropHeight;

        config.inHeight     = length;
        config.inYOffset    = offset;
        config.inCropHeight = crop;
    }

    config.outWidth   = config.inWidth;
    config.outHeight  = config.inHeight;

    config.pqSessionId = m_pOutDataPort[0]->getPqIdStatus();
    DPLOGI("DpEngineBase: PROC_HDR, getPqIdStatus() = %llx\n", m_pOutDataPort[0]->getPqIdStatus());

    DPLOGI("DpEngineBase: PROC_HDR done\n");
    return DP_STATUS_RETURN_SUCCESS;
}

inline DP_STATUS_ENUM DpEngineBase::configFrameScale(DpConfig &config)
{
    DP_STATUS_ENUM status;
    int32_t rotation;

    DP_PROFILE_ENUM dummy;
    int32_t dummyi;

    status = m_pInDataPort->getPortInfo(&config.inFormat,
                                        &config.inWidth,
                                        &config.inHeight,
                                        &config.inYPitch,
                                        &config.inUVPitch,
                                        &dummy);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpEngineBase: Get port info failed\n");
        return status;
    }

    if (0 != DP_COLOR_GET_H_SUBSAMPLE(config.inFormat))
    {
        config.inWidth &= ~1;
    }

    if (0 != DP_COLOR_GET_V_SUBSAMPLE(config.inFormat))
    {
        config.inHeight &= ~1;
    }

    status = m_pOutDataPort[0]->getSourceCrop(&config.inXOffset,
                                              &config.inXSubpixel,
                                              &config.inYOffset,
                                              &config.inYSubpixel,
                                              &config.inCropWidth,
                                              &config.inCropWidthSubpixel,
                                              &config.inCropHeight,
                                              &config.inCropHeightSubpixel);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        return status;
    }

    if (config.inCropWidth + config.inXOffset > config.inWidth)
    {
        config.inCropWidth = config.inWidth - config.inXOffset;
    }
    if (config.inCropHeight + config.inYOffset > config.inHeight)
    {
        config.inCropHeight = config.inHeight - config.inYOffset;
    }

    if (config.enRDMACrop)
    {
        config.inWidth  = config.inCropWidth + config.inXOffset;
        config.inHeight = config.inCropHeight + config.inYOffset;
    }

    if (config.inFormat == DP_COLOR_420_BLKI)
    {
        config.inYOffset = config.inYOffset >> 1;
        config.inCropHeight = config.inCropHeight >> 1;
        config.inHeight = config.inHeight >> 1;
    }

    if ((90  == config.rotation) ||
        (270 == config.rotation))   // RROT
    {
        int32_t length      = config.inWidth;
        int32_t offset      = config.inXOffset;
        int32_t subpixel    = config.inXSubpixel;
        int32_t crop        = config.inCropWidth;

        config.inWidth      = config.inHeight;
        config.inXOffset    = config.inYOffset;
        config.inXSubpixel  = config.inYSubpixel;
        config.inCropWidth  = config.inCropHeight;

        config.inHeight     = length;
        config.inYOffset    = offset;
        config.inYSubpixel  = subpixel;
        config.inCropHeight = crop;
    }

    if (((true  == config.enFlip) && ((0   == config.rotation) || (270 == config.rotation))) ||
        ((false == config.enFlip) && ((90  == config.rotation) || (180 == config.rotation))))
    {
        // RROT RIGHT_TO_LEFT
        int32_t offset;
        int32_t subpixel;

        if (config.inXSubpixel)
        {
            offset = config.inWidth - config.inCropWidth - config.inXOffset - 1;
            subpixel = (1 << TILE_SCALER_SUBPIXEL_SHIFT) - config.inXSubpixel;
        }
        else
        {
            offset = config.inWidth - config.inCropWidth - config.inXOffset;
            subpixel = 0;
        }

        config.inXOffset = offset;
        config.inXSubpixel = subpixel;
    }

    if ((180 == config.rotation) ||
        (270 == config.rotation))
    {
        // RROT BOTTOM_TO_TOP
        int32_t offset;
        int32_t subpixel;

        if (config.inYSubpixel)
        {
            offset = config.inHeight - config.inCropHeight - config.inYOffset - 1;
            subpixel = (1 << TILE_SCALER_SUBPIXEL_SHIFT) - config.inYSubpixel;
        }
        else
        {
            offset = config.inHeight - config.inCropHeight - config.inYOffset;
            subpixel = 0;
        }

        config.inYOffset = offset;
        config.inYSubpixel = subpixel;
    }

    rotation = m_pOutDataPort[0]->getRotation();
    if ((0   == config.rotation) &&
        ((90  == rotation) ||
         (270 == rotation)))    // WROT
    {
        status = m_pOutDataPort[0]->getPortInfo(&config.outFormat,
                                                &config.outHeight,
                                                &config.outWidth,
                                                &config.outYPitch,
                                                &config.outUVPitch,
                                                &dummy);

        if (0 != DP_COLOR_GET_H_SUBSAMPLE(config.outFormat))
        {
            config.outWidth &= ~1;  // WROT HW constraint
            config.outHeight &= ~1;
        }
        else if (0 != DP_COLOR_GET_V_SUBSAMPLE(config.outFormat))
        {
            config.outWidth &= ~1;
        }

        status = m_pOutDataPort[0]->getTargetROI(&dummyi,
                                                 &dummyi,
                                                 &config.outRoiHeight,
                                                 &config.outRoiWidth);
    }
    else
    {
        status = m_pOutDataPort[0]->getPortInfo(&config.outFormat,
                                                &config.outWidth,
                                                &config.outHeight,
                                                &config.outYPitch,
                                                &config.outUVPitch,
                                                &dummy);

        if (0 != DP_COLOR_GET_H_SUBSAMPLE(config.outFormat))
        {
            config.outWidth &= ~1;
        }
        if (0 != DP_COLOR_GET_V_SUBSAMPLE(config.outFormat))
        {
            config.outHeight &= ~1;
        }

        status = m_pOutDataPort[0]->getTargetROI(&dummyi,
                                                 &dummyi,
                                                 &config.outRoiWidth,
                                                 &config.outRoiHeight);
    }

    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        return status;
    }

    config.pqSessionId = m_pOutDataPort[0]->getPqIdStatus();
    DPLOGI("DpEngineBase: configFrameScale m_pOutDataPort[0]->getPqIdStatus() = %llx\n",m_pOutDataPort[0]->getPqIdStatus());

    return DP_STATUS_RETURN_SUCCESS;
}

inline DP_STATUS_ENUM DpEngineBase::configFrameSharpness(DpConfig &config)
{
    DP_STATUS_ENUM status;
    int32_t rotation;

    DpColorFormat dummycf;
    int32_t dummyi;
    DP_PROFILE_ENUM dummy;

    rotation = m_pOutDataPort[0]->getRotation();
    if ((0   == config.rotation) &&
        ((90  == rotation) ||
         (270 == rotation)))    // WROT
    {
        status = m_pOutDataPort[0]->getPortInfo(&dummycf,
                                                &config.outHeight,
                                                &config.outWidth,
                                                &dummyi,
                                                &dummyi,
                                                &dummy);
    }
    else
    {
        status = m_pOutDataPort[0]->getPortInfo(&dummycf,
                                                &config.outWidth,
                                                &config.outHeight,
                                                &dummyi,
                                                &dummyi,
                                                &dummy);
    }

    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        return status;
    }

    status = m_pInDataPort->getPortInfo(&config.inFormat,
                                        &dummyi,
                                        &dummyi,
                                        &dummyi,
                                        &dummyi,
                                        &dummy);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        return status;
    }

    status = m_pOutDataPort[0]->getSourceCrop(&dummyi,
                                              &dummyi,
                                              &dummyi,
                                              &dummyi,
                                              &config.inCropWidth,
                                              &config.inCropWidthSubpixel,
                                              &config.inCropHeight,
                                              &config.inCropHeightSubpixel);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        return status;
    }

    if ((90  == config.rotation) ||
        (270 == config.rotation))   // RROT
    {
        int32_t crop        = config.inCropWidth;
        config.inCropWidth  = config.inCropHeight;
        config.inCropHeight = crop;
    }

    config.inWidth   = config.outWidth;
    config.inHeight  = config.outHeight;
    config.parTDSHP = m_pOutDataPort[0]->getTDSHPStatus();
    config.pqSessionId = m_pOutDataPort[0]->getPqIdStatus();
    DPLOGI("DpEngineBase: m_pOutDataPort[0]->getPqIdStatus() = %llx\n", m_pOutDataPort[0]->getPqIdStatus());

    return DP_STATUS_RETURN_SUCCESS;
}

inline DP_STATUS_ENUM DpEngineBase::configFrameColor(DpConfig &config)
{
    DP_STATUS_ENUM status;
    int32_t rotation;

    DpColorFormat dummycf;
    int32_t dummyi;
    DP_PROFILE_ENUM dummy;

    rotation = m_pOutDataPort[0]->getRotation();
    if ((0   == config.rotation) &&
        ((90  == rotation) ||
         (270 == rotation)))    // WROT
    {
        status = m_pOutDataPort[0]->getPortInfo(&dummycf,
                                                &config.outHeight,
                                                &config.outWidth,
                                                &dummyi,
                                                &dummyi,
                                                &dummy);
    }
    else
    {
        status = m_pOutDataPort[0]->getPortInfo(&dummycf,
                                                &config.outWidth,
                                                &config.outHeight,
                                                &dummyi,
                                                &dummyi,
                                                &dummy);
    }

    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        return status;
    }

    status = m_pInDataPort->getPortInfo(&config.inFormat,
                                        &dummyi,
                                        &dummyi,
                                        &dummyi,
                                        &dummyi,
                                        &dummy);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        return status;
    }

    status = m_pOutDataPort[0]->getSourceCrop(&dummyi,
                                              &dummyi,
                                              &dummyi,
                                              &dummyi,
                                              &config.inCropWidth,
                                              &config.inCropWidthSubpixel,
                                              &config.inCropHeight,
                                              &config.inCropHeightSubpixel);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        return status;
    }

    if ((90  == config.rotation) ||
        (270 == config.rotation))   // RROT
    {
        int32_t crop        = config.inCropWidth;
        config.inCropWidth  = config.inCropHeight;
        config.inCropHeight = crop;
    }

    config.inWidth   = config.outWidth;
    config.inHeight  = config.outHeight;

    return DP_STATUS_RETURN_SUCCESS;
}

inline DP_STATUS_ENUM DpEngineBase::configFrameDither(DpConfig &config)
{
    config.enDither = m_pOutDataPort[0]->getDitherStatus();
    return DP_STATUS_RETURN_SUCCESS;
}

inline DP_STATUS_ENUM DpEngineBase::configFrameMout(DpConfig &config)
{
    DP_STATUS_ENUM status;
    int32_t rotation;

    rotation = m_pOutDataPort[0]->getRotation();
    if ((0   == config.rotation) &&
        ((90  == rotation) ||
         (270 == rotation)))    // WROT
    {
        status = m_pOutDataPort[0]->getPortInfo(&config.outFormat,
                                                &config.outHeight,
                                                &config.outWidth,
                                                &config.outYPitch,
                                                &config.outUVPitch,
                                                &config.outProfile);
    }
    else
    {
        status = m_pOutDataPort[0]->getPortInfo(&config.outFormat,
                                                &config.outWidth,
                                                &config.outHeight,
                                                &config.outYPitch,
                                                &config.outUVPitch,
                                                &config.outProfile);
    }

    config.inFormat  = config.outFormat;
    config.inWidth   = config.outWidth;
    config.inHeight  = config.outHeight;
    config.inYPitch  = config.outYPitch;
    config.inUVPitch = config.outUVPitch;

    DPLOGI("DpEngineBase: PROC_MDPMOUT done\n");
    return DP_STATUS_RETURN_SUCCESS;
}

inline DP_STATUS_ENUM DpEngineBase::configFrameWrite(DpCommand&, DpConfig &config)
{
    DP_STATUS_ENUM status;
    int64_t feature;
    int32_t rotation;

    int32_t dummyi;
    PORT_TYPE_ENUM portType;

    DPLOGI("DpEngineBase: PROC_WMEM start\n");

    feature = queryFeature();

    status = m_pOutDataPort[0]->getPortType(&portType);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpEngineBase: getPortType failed\n");
        return status;
    }

    if (PORT_VENC == portType)
    {
        DpVEnc_Config* pVEncConfig = NULL;

        status = m_pOutDataPort[0]->getVENCConfig(&pVEncConfig);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            DPLOGE("DpEngineBase: get VENCConfig failed\n");
            return status;
        }

        config.memAddr[0] = pVEncConfig->memYUVMVAAddr[0];
        config.memAddr[1] = pVEncConfig->memYUVMVAAddr[1];
        config.memAddr[2] = pVEncConfig->memYUVMVAAddr[2];

        config.memSize[0] = pVEncConfig->memYUVMVASize[0];
        config.memSize[1] = pVEncConfig->memYUVMVASize[1];
        config.memSize[2] = pVEncConfig->memYUVMVASize[2];
    }
    else
    {
        status = m_pOutDataPort[0]->getHWAddress(CLIENT_PRODUCER,
                                                 m_engineType,
                                                 config.memAddr,
                                                 config.memSize);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            DPLOGE("DpEngineBase: get hardware base failed\n");
            return status;
        }
    }

    DPLOGI("DpEngineBase: memory base0 0x%08x\n", config.memAddr[0]);
    DPLOGI("DpEngineBase: memory base1 0x%08x\n", config.memAddr[1]);
    DPLOGI("DpEngineBase: memory base2 0x%08x\n", config.memAddr[2]);

    status = m_pInDataPort->getPortInfo(&config.inFormat,
                                        &dummyi,
                                        &dummyi,
                                        &dummyi,
                                        &dummyi,
                                        &config.inProfile);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpEngineBase: Get port info failed\n");
        return status;
    }

    rotation = m_pOutDataPort[0]->getRotation();
    if ((feature & eROT) &&
        ((90  == rotation) ||
         (270 == rotation)))    // WROT
    {
        status = m_pOutDataPort[0]->getPortInfo(&config.outFormat,
                                                &config.outHeight,
                                                &config.outWidth,
                                                &config.outYPitch,
                                                &config.outUVPitch,
                                                &config.outProfile);

        if (0 != DP_COLOR_GET_H_SUBSAMPLE(config.outFormat))
        {
            config.outWidth &= ~1;  // WROT HW constraint
            config.outHeight &= ~1;
        }
        else if (0 != DP_COLOR_GET_V_SUBSAMPLE(config.outFormat))
        {
            config.outWidth &= ~1;
        }

        if ((DP_COLOR_GET_PLANE_COUNT(config.outFormat) > 1) && (config.outUVPitch <= 0))
        {
            config.outUVPitch = DP_COLOR_GET_MIN_UV_PITCH(config.outFormat, config.outHeight);
        }
    }
    else
    {
        status = m_pOutDataPort[0]->getPortInfo(&config.outFormat,
                                                &config.outWidth,
                                                &config.outHeight,
                                                &config.outYPitch,
                                                &config.outUVPitch,
                                                &config.outProfile);

        if (0 != DP_COLOR_GET_H_SUBSAMPLE(config.outFormat))
        {
            config.outWidth &= ~1;
        }
        if (0 != DP_COLOR_GET_V_SUBSAMPLE(config.outFormat))
        {
            config.outHeight &= ~1;
        }

        if ((DP_COLOR_GET_PLANE_COUNT(config.outFormat) > 1) && (config.outUVPitch <= 0))
        {
            config.outUVPitch = DP_COLOR_GET_MIN_UV_PITCH(config.outFormat, config.outWidth);
        }
    }

    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpEngineBase: get port info failed\n");
        return status;
    }

    status = m_pOutDataPort[0]->getTargetROI(&config.outXStart,
                                             &config.outYStart,
                                             &config.outRoiWidth,
                                             &config.outRoiHeight);

    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpEngineBase: get target roi info failed\n");
        return status;
    }

    if (STREAM_DUAL_BITBLT == config.scenario)
    {
        config.enWROTCrop = true;

        if (config.pipeID == 0)
        {
            config.outXOffset = 0;
            config.outCropWidth = config.outWidth >> 1;
            if (config.outCropWidth & 1)
            {
                config.outCropWidth++;
            }
        }
        else
        {
            config.outXOffset = config.outWidth >> 1;
            config.outCropWidth = config.outWidth >> 1;
            if (config.outXOffset & 1)
            {
                config.outXOffset++;
                config.outCropWidth--;
            }
        }
    }

    config.pColorMatrix = m_pOutDataPort[0]->getColorMatrix();

    if (((1 == DP_COLOR_GET_PLANE_COUNT(config.outFormat)) &&
         (0 == config.memAddr[0])) ||
        ((2 == DP_COLOR_GET_PLANE_COUNT(config.outFormat)) &&
         ((0 == config.memAddr[1]) || (0 == config.memAddr[0]))) ||
        ((3 == DP_COLOR_GET_PLANE_COUNT(config.outFormat)) &&
         ((0 == config.memAddr[2]) || (0 == config.memAddr[1]) || (0 == config.memAddr[0]))))
    {
        DPLOGE("DpEngineBase: buffer plane number error, color format = %d, plane = %d\n",
            config.outFormat, DP_COLOR_GET_PLANE_COUNT(config.outFormat));
        assert(0);
        return DP_STATUS_INVALID_BUFFER;
    }

    config.inWidth   = config.outWidth;
    config.inHeight  = config.outHeight;
    config.inYPitch  = config.outYPitch;
    config.inUVPitch = config.outUVPitch;

    DPLOGI("DpEngineBase: PROC_WMEM done\n");
    return DP_STATUS_RETURN_SUCCESS;
}

inline DP_STATUS_ENUM DpEngineBase::configFrameJPEG(DpConfig &config)
{
    DP_STATUS_ENUM status;
    uint32_t tmpSize[3] = {0};
    void*    tmpBase[3] = {0};

    DPLOGI("DpEngineBase: PROC_JPEG start\n");

    status = m_pOutDataPort[0]->getJPEGConfig(&config.jpegEnc_cfg);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        return status;
    }

    DPLOGI("DpEngineBase:PROC_JPEG Get SW Address\n");

    status = m_pOutDataPort[0]->getSWAddress(CLIENT_PRODUCER,
                                             tmpBase,
                                             tmpSize);
    if ((DP_STATUS_RETURN_SUCCESS != status) || (tmpSize[0] != config.memSize[0]))
    {
        DPLOGE("DpEngineBase: get software base failed or size is not correct\n");
        return status;
    }

    config.jpegEnc_cfg.memSWAddr[0] = tmpBase[0];
    config.jpegEnc_cfg.memSWAddr[1] = tmpBase[1];
    config.jpegEnc_cfg.memSWAddr[2] = tmpBase[2];

    DPLOGI("DpEngineBase: memory SW base0 %p\n", config.jpegEnc_cfg.memSWAddr[0]);
    DPLOGI("DpEngineBase: memory SW base1 %p\n", config.jpegEnc_cfg.memSWAddr[1]);
    DPLOGI("DpEngineBase: memory SW base2 %p\n", config.jpegEnc_cfg.memSWAddr[2]);

    DPLOGI("DpEngineBase: PROC_JPEG done\n");
    return DP_STATUS_RETURN_SUCCESS;
}

inline DP_STATUS_ENUM DpEngineBase::configFrameVEnc(DpConfig &config)
{
    DP_STATUS_ENUM status;
    uint32_t tmpSize[3] = {0};
    void*    tmpBase[3] = {0};

    DPLOGI("DpEngineBase: PROC_VENC start\n");

    status = m_pOutDataPort[0]->getVENCConfig(&config.pVEnc_cfg);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        return status;
    }

    status = m_pOutDataPort[0]->getHWAddress(CLIENT_PRODUCER,
                                             m_engineType,
                                             config.memAddr,
                                             config.memSize);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpEngineBase: get output memory hardware base failed\n");
        return status;
    }

    DPLOGI("DpEngineBase: memory base0 0x%08x\n", config.memAddr[0]);
    DPLOGI("DpEngineBase: memory base1 0x%08x\n", config.memAddr[1]);
    DPLOGI("DpEngineBase: memory base2 0x%08x\n", config.memAddr[2]);

    status = m_pOutDataPort[0]->getPortInfo(&config.outFormat,
                                            &config.outWidth,
                                            &config.outHeight,
                                            &config.outYPitch,
                                            &config.outUVPitch,
                                            &config.outProfile);

    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpEngineBase: get port info failed\n");
        return status;
    }

    if ((DP_COLOR_GET_PLANE_COUNT(config.outFormat) > 1) && (config.outUVPitch <= 0))
    {
        config.outUVPitch = DP_COLOR_GET_MIN_UV_PITCH(config.outFormat, config.outWidth);
    }

    if (((1 == DP_COLOR_GET_PLANE_COUNT(config.outFormat)) &&
         (0 == config.memAddr[0])) ||
        ((2 == DP_COLOR_GET_PLANE_COUNT(config.outFormat)) &&
         ((0 == config.memAddr[1]) || (0 == config.memAddr[0]))) ||
        ((3 == DP_COLOR_GET_PLANE_COUNT(config.outFormat)) &&
         ((0 == config.memAddr[2]) || (0 == config.memAddr[1]) || (0 == config.memAddr[0]))))
    {
        DPLOGE("DpEngineBase: buffer plane number error, color format = 0x%08x, plane = %d\n",
            uint32_t(config.outFormat), DP_COLOR_GET_PLANE_COUNT(config.outFormat));
        assert(0);
        return DP_STATUS_INVALID_BUFFER;
    }

    if (0 != DP_COLOR_GET_H_SUBSAMPLE(config.outFormat))
    {
        config.outWidth &= ~1;
    }

    if (0 != DP_COLOR_GET_V_SUBSAMPLE(config.outFormat))
    {
        config.outHeight &= ~1;
    }

    DPLOGI("DpEngineBase: memory size0 0x%08x\n", config.memSize[0]);
    DPLOGI("DpEngineBase: memory size1 0x%08x\n", config.memSize[1]);
    DPLOGI("DpEngineBase: memory size2 0x%08x\n", config.memSize[2]);

    status = m_pOutDataPort[0]->getSWAddress(CLIENT_PRODUCER, tmpBase, tmpSize);
    if ((DP_STATUS_RETURN_SUCCESS != status) || (tmpSize[0] != config.memSize[0]))
    {
        DPLOGE("DpEngineBase: get software base failed tmpSize[0]: %d, tmpBase[0]: %p\n",tmpSize[0],tmpBase[0]);
        DPLOGE("DpEngineBase: get software base failed or size is not correct: %d\n",status);
        return status;
    }

    config.pVEnc_cfg->memOutputSWAddr[0] = tmpBase[0];
    config.pVEnc_cfg->memOutputSWAddr[1] = tmpBase[1];
    config.pVEnc_cfg->memOutputSWAddr[2] = tmpBase[2];

    DPLOGI("DpEngineBase: memory SW base0 %p\n", config.pVEnc_cfg->memOutputSWAddr[0]);
    DPLOGI("DpEngineBase: memory SW base1 %p\n", config.pVEnc_cfg->memOutputSWAddr[1]);
    DPLOGI("DpEngineBase: memory SW base2 %p\n", config.pVEnc_cfg->memOutputSWAddr[2]);

    config.inFormat  = config.outFormat;
    config.inWidth   = config.outWidth;
    config.inHeight  = config.outHeight;
    config.inYPitch  = config.outYPitch;
    config.inUVPitch = config.outUVPitch;

    DPLOGI("DpEngineBase: PROC_VENC done\n");
    return DP_STATUS_RETURN_SUCCESS;
}

#endif // CONFIG_FOR_VERIFY_FPGA

DP_STATUS_ENUM DpEngineBase::configFrame(DpCommand &command,
                                         DpConfig  &config)
{
    DP_TRACE_CALL();

    DP_STATUS_ENUM status;
    int64_t feature;

    feature = queryFeature();

    /***************************************************************************
     * If rotate on input, use TARGET coordination which is
     * AFTER rotation, so fix input orientation;
     * If rotate on output, use SOURCE coordination which is
     * BEFORE or IN rotation, so fix output orientation.
     *
     * To judge the situation, use
     * (0 != config.rotation) => IN or AFTER rotation: RMEM in, SCL in
     * (0 == config.rotation) => BEFORE rotation:      SCL out, TDSHP out, MDPMOUT out
     * (feature & eROT)       => IN rotation:          WMEM out
     **************************************************************************/

    if (feature & eROT)
    {
        configFrameRotate(config);
    }

    if (feature & eFLIP)
    {
        configFrameFlip(config);
    }

    if (feature & eISP)
    {
        configFrameISP(config);
    }

    if (feature & eALPHA_ROT)
    {
        configFrameAlphaRot(config);
    }

    if ((feature & eRMEM) && !(feature & eISP))
    {
        status = configFrameRead(command, config);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            return status;
        }
    }

    if (feature & eRING)
    {
        status = configFrameRingBuf(config);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            return status;
        }
    }

    if (feature & eAAL)
    {
        status = configFrameAal(config);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            return status;
        }
    }

    if (feature & eCCORR)
    {
        status = configFrameCcorr(config);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            return status;
        }
    }

    if (feature & eHDR)
    {
        status = configFrameHdr(config);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            return status;
        }
    }

    if (feature & eSCL)
    {
        status = configFrameScale(config);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            return status;
        }
    }

    if (feature & eTDSHP)
    {
        status = configFrameSharpness(config);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            return status;
        }
    }

    if (feature & eCOLOR)
    {
        status = configFrameColor(config);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            return status;
        }
    }

    if (feature & eDITHER)
    {
        configFrameDither(config);
    }

    if (feature & eMDPSOUT)
    {
        status = configFrameMout(config);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            return status;
        }
    }

    if ((feature & eWMEM) && !(feature & (eISP | eVENC)))
    {
        status = configFrameWrite(command, config);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            return status;
        }
    }

    //Get JPEG port working buffer
    if (feature & eJPEG)
    {
        status = configFrameJPEG(config);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            return status;
        }
    }

    //Get VENC port working buffer
    if (feature & eVENC)
    {
        status = configFrameVEnc(config);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            return status;
        }
    }

    // Setup frame info
    m_inFrameWidth   = config.inWidth;
    m_inFrameHeight  = config.inHeight;
    m_outFrameWidth  = config.outWidth;
    m_outFrameHeight = config.outHeight;

    onSetTileOrder(config.inCalOrder, config.outCalOrder, config.inStreamOrder, config.outDumpOrder);

    status = onConfigFrame(command, config);
    //m_engineState = eConfig;

    return status;
}


DP_STATUS_ENUM DpEngineBase::reconfigFrame(DpCommand &command,
                                           DpConfig  &config)
{
    DP_TRACE_CALL();

    DP_STATUS_ENUM status;
    int64_t feature;

    feature = queryFeature();

    /***************************************************************************
     * If rotate on input, use TARGET coordination which is
     * AFTER rotation, so fix input orientation;
     * If rotate on output, use SOURCE coordination which is
     * BEFORE or IN rotation, so fix output orientation.
     *
     * To judge the situation, use
     * (0 != config.rotation) => IN or AFTER rotation: RMEM in, SCL in
     * (0 == config.rotation) => BEFORE rotation:      SCL out, TDSHP out, MDPMOUT out
     * (feature & eROT)       => IN rotation:          WMEM out
     **************************************************************************/

    if (feature & eROT)
    {
        configFrameRotate(config);
    }

    if (feature & eFLIP)
    {
        configFrameFlip(config);
    }

    if (feature & eISP)
    {
        configFrameISP(config);
    }

    if ((feature & eRMEM) && !(feature & eISP))
    {
        status = configFrameRead(command, config);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            return status;
        }
    }

    if (feature & eAAL)
    {
        status = configFrameAal(config);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            return status;
        }
    }

    if (feature & eHDR)
    {
        status = configFrameHdr(config);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            return status;
        }
    }

    if (feature & eTDSHP)
    {
        status = configFrameSharpness(config);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            return status;
        }
    }

    if ((feature & eWMEM) && !(feature & (eISP | eVENC)))
    {
        status = configFrameWrite(command, config);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            return status;
        }
    }

    status = onReconfigFrame(command, config);
    return status;
}

